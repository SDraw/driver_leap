#include "stdafx.h"
#include "CLeapHandController.h"
#include "CConfigHelper.h"
#include "CGestureMatcher.h"
#include "CDriverLogHelper.h"
#include "Utils.h"

extern char g_moduleFileName[];

const std::vector<std::string> g_gameProfiles
{
    "Default", "VRChat"
};

const std::vector<std::string> g_debugRequests
{
    "profile", "game"
};
enum DebugRequest : size_t
{
    DR_Profile = 0U,
    DR_Game
};

const std::vector<std::string> g_gameList
{
    "vrchat"
};
enum GameList : size_t
{
    GL_VRChat = 0U
};

const std::vector<std::string> g_gameCommands
{
    "drawing_mode" // VRChat
};
enum GameCommand : size_t
{
    CC_DrawingMode = 0U // VRChat
};

extern const vr::VRBoneTransform_t g_openHandGesture[];
const glm::vec3 g_axisX(1.f, 0.f, 0.f);
const glm::vec3 g_axisY(0.f, 1.f, 0.f);
const glm::mat4 g_identityMatrix(1.f);
const glm::vec4 g_zeroPoint(0.f, 0.f, 0.f, 1.f);

// ----
CControllerButton::CControllerButton()
{
    m_handle = vr::k_ulInvalidInputComponentHandle;
    m_state = false;
    m_value = 0.f;
    m_inputType = ControllerButtonInputType::CBIT_Boolean;
    m_updated = false;
}
CControllerButton::~CControllerButton()
{
}

void CControllerButton::SetValue(float f_value)
{
    if(m_inputType == ControllerButtonInputType::CBIT_Float)
    {
        if(m_value != f_value)
        {
            m_value = f_value;
            m_updated = true;
        }
    }
}
void CControllerButton::SetState(bool f_state)
{
    if(m_inputType == ControllerButtonInputType::CBIT_Boolean)
    {
        if(m_state != f_state)
        {
            m_state = f_state;
            m_updated = true;
        }
    }
}
// ----

double CLeapHandController::ms_headPos[] = { .0, .0, .0 };
vr::HmdQuaternion_t CLeapHandController::ms_headRot = { 1.0, .0, .0, .0 };

CLeapHandController::CLeapHandController(vr::IVRServerDriverHost* f_driverHost, EControllerHandAssignment f_hand)
{
    m_driverHost = f_driverHost;
    m_driverInput = nullptr;
    m_handAssigment = f_hand;
    m_trackedDeviceID = vr::k_unTrackedDeviceIndexInvalid;

    m_serialNumber.assign("leap_");
    m_serialNumber.append((m_handAssigment == CHA_Left) ? "lefthand" : "righthand");
    m_propertyContainer = vr::k_ulInvalidPropertyContainer;

    glm::vec3 l_eulerOffsetRot(CConfigHelper::GetRotationOffsetX(), CConfigHelper::GetRotationOffsetY(), CConfigHelper::GetRotationOffsetZ());
    if(m_handAssigment == CHA_Right)
    {
        // Only X axis isn't inverted for right controller
        l_eulerOffsetRot.y *= -1.f;
        l_eulerOffsetRot.z *= -1.f;
    }
    m_gripAngleOffset = glm::quat(l_eulerOffsetRot);

    m_pose = { 0 };
    m_pose.qDriverFromHeadRotation = { 1.0, .0, .0, .0 };
    for(size_t i = 0U; i < 3U; i++)
    {
        m_pose.vecDriverFromHeadTranslation[i] = .0;
        m_pose.vecAngularVelocity[i] = .0;
        m_pose.vecAcceleration[i] = .0;
        m_pose.vecAngularAcceleration[i] = .0;
    }
    m_pose.poseTimeOffset = -0.016;
    m_pose.willDriftInYaw = false;
    m_pose.shouldApplyHeadModel = false;
    m_pose.result = vr::TrackingResult_Uninitialized;

    // Skeletal data
    m_skeletonHandle = vr::k_ulInvalidInputComponentHandle;
    for(size_t i = 0U; i < HSB_Count; i++) m_boneTransform[i] = g_openHandGesture[i];
    if(m_handAssigment == CHA_Right)
    {
        // Transformation inversion along 0YZ plane
        for(size_t i = 1U; i < HSB_Count; i++)
        {
            m_boneTransform[i].position.v[0] *= -1.f;

            if(i == HSB_Wrist)
            {
                m_boneTransform[i].orientation.y *= -1.f;
                m_boneTransform[i].orientation.z *= -1.f;
            }
            else if(i == HSB_Thumb0 || i == HSB_IndexFinger0 || i == HSB_MiddleFinger0 || i == HSB_RingFinger0 || i == HSB_PinkyFinger0)
            {
                m_boneTransform[i].orientation.y *= -1.f;
                m_boneTransform[i].orientation.z *= -1.f;

                glm::quat l_rot;
                ConvertQuaternion(m_boneTransform[i].orientation, l_rot);
                l_rot = glm::rotate(l_rot, glm::pi<float>(), g_axisX);
                ConvertQuaternion(l_rot, m_boneTransform[i].orientation);
            }
        }
    }

    m_gameProfile = GP_Default;
}
CLeapHandController::~CLeapHandController()
{
}

vr::EVRInitError CLeapHandController::Activate(uint32_t unObjectId)
{
    m_trackedDeviceID = unObjectId;

    vr::CVRPropertyHelpers *l_vrProperties = vr::VRProperties();
    m_propertyContainer = l_vrProperties->TrackedDeviceToPropertyContainer(m_trackedDeviceID);

    // Shared properties
    l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_TrackingSystemName_String, "lighthouse");
    l_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_WillDriftInYaw_Bool, false);
    l_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_DeviceIsWireless_Bool, true);
    l_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_DeviceIsCharging_Bool, false);
    l_vrProperties->SetFloatProperty(m_propertyContainer, vr::Prop_DeviceBatteryPercentage_Float, 1.f); // Always charged

    vr::HmdMatrix34_t l_matrix = { -1.f, 0.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f };
    l_vrProperties->SetProperty(m_propertyContainer, vr::Prop_StatusDisplayTransform_Matrix34, &l_matrix, sizeof(vr::HmdMatrix34_t), vr::k_unHmdMatrix34PropertyTag);

    l_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_Firmware_UpdateAvailable_Bool, false);
    l_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_Firmware_ManualUpdate_Bool, false);
    l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_Firmware_ManualUpdateURL_String, "https://developer.valvesoftware.com/wiki/SteamVR/HowTo_Update_Firmware");
    l_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_DeviceProvidesBatteryStatus_Bool, true);
    l_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_DeviceCanPowerOff_Bool, true);
    l_vrProperties->SetInt32Property(m_propertyContainer, vr::Prop_DeviceClass_Int32, vr::TrackedDeviceClass_Controller);
    l_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_Firmware_ForceUpdateRequired_Bool, false);
    l_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_Identifiable_Bool, true);
    l_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_Firmware_RemindUpdate_Bool, false);
    l_vrProperties->SetInt32Property(m_propertyContainer, vr::Prop_Axis0Type_Int32, vr::k_eControllerAxis_TrackPad);
    l_vrProperties->SetInt32Property(m_propertyContainer, vr::Prop_Axis1Type_Int32, vr::k_eControllerAxis_Trigger);
    l_vrProperties->SetInt32Property(m_propertyContainer, vr::Prop_ControllerRoleHint_Int32, (m_handAssigment == CHA_Left) ? vr::TrackedControllerRole_LeftHand : vr::TrackedControllerRole_RightHand);
    l_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_HasDisplayComponent_Bool, false);
    l_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_HasCameraComponent_Bool, false);
    l_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_HasDriverDirectModeComponent_Bool, false);
    l_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_HasVirtualDisplayComponent_Bool, false);
    l_vrProperties->SetInt32Property(m_propertyContainer, vr::Prop_ControllerHandSelectionPriority_Int32, 0);

    // Specific properties
    switch(CConfigHelper::GetEmulatedController())
    {
        case CConfigHelper::EC_Vive:
        {
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_ModelNumber_String, "Vive. Controller MV");
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_SerialNumber_String, (m_handAssigment == CHA_Left) ? "LHR-F94B3BD8" : "LHR-F94B3BD9"); // Changed
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_RenderModelName_String, "vr_controller_vive_1_5");
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_ManufacturerName_String, "HTC");
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_TrackingFirmwareVersion_String, "1533720215 htcvrsoftware@firmware-win32 2018-08-08 FPGA 262(1.6/0/0) BL 0 VRC 1533720214 Radio 1532585738");
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_HardwareRevision_String, "product 129 rev 1.5.0 lot 2000/0/0 0");
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_ConnectedWirelessDongle_String, "1E8092840E"); // Changed
            l_vrProperties->SetUint64Property(m_propertyContainer, vr::Prop_HardwareRevision_Uint64, 2164327680U);
            l_vrProperties->SetUint64Property(m_propertyContainer, vr::Prop_FirmwareVersion_Uint64, 1533720215U);
            l_vrProperties->SetUint64Property(m_propertyContainer, vr::Prop_FPGAVersion_Uint64, 262U);
            l_vrProperties->SetUint64Property(m_propertyContainer, vr::Prop_VRCVersion_Uint64, 1533720214U);
            l_vrProperties->SetUint64Property(m_propertyContainer, vr::Prop_RadioVersion_Uint64, 1532585738U);
            l_vrProperties->SetUint64Property(m_propertyContainer, vr::Prop_DongleVersion_Uint64, 1461100729U);
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_Firmware_ProgrammingTarget_String, (m_handAssigment == CHA_Left) ? "LHR-F94B3BD8" : "LHR-F94B3BD9"); // Changed
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_ResourceRoot_String, "htc");
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_RegisteredDeviceType_String, (m_handAssigment == CHA_Left) ? "htc/vive_controllerLHR-F94B3BD8" : "htc/vive_controllerLHR-F94B3BD9"); // Changed
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_InputProfilePath_String, "{htc}/input/vive_controller_profile.json");
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceOff_String, "{htc}/icons/controller_status_off.png");
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceSearching_String, "{htc}/icons/controller_status_searching.gif");
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceSearchingAlert_String, "{htc}/icons/controller_status_searching_alert.gif");
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceReady_String, "{htc}/icons/controller_status_ready.png");
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceReadyAlert_String, "{htc}/icons/controller_status_ready_alert.png");
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceNotReady_String, "{htc}/icons/controller_status_error.png");
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceStandby_String, "{htc}/icons/controller_status_off.png");
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceAlertLow_String, "{htc}/icons/controller_status_ready_low.png");
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_ControllerType_String, "vive_controller");
        } break;

        case CConfigHelper::EC_Index:
        {
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_ModelNumber_String, (m_handAssigment == CHA_Left) ? "Knuckles Left" : "Knuckles Right");
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_SerialNumber_String, (m_handAssigment == CHA_Left) ? "LHR-E217CD00" : "LHR-E217CD01"); // Changed
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_RenderModelName_String, (m_handAssigment == CHA_Left) ? "{indexcontroller}valve_controller_knu_1_0_left" : "{indexcontroller}valve_controller_knu_1_0_right");
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_ManufacturerName_String, "Valve");
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_TrackingFirmwareVersion_String, "1562916277 watchman@ValveBuilder02 2019-07-12 FPGA 538(2.26/10/2) BL 0 VRC 1562916277 Radio 1562882729");
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_HardwareRevision_String, "product 17 rev 14.1.9 lot 2019/4/20 0");
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_ConnectedWirelessDongle_String, "C2F75F5986-FYI"); // Changed
            l_vrProperties->SetUint64Property(m_propertyContainer, vr::Prop_HardwareRevision_Uint64, 286130441U);
            l_vrProperties->SetUint64Property(m_propertyContainer, vr::Prop_FirmwareVersion_Uint64, 1562916277U);
            l_vrProperties->SetUint64Property(m_propertyContainer, vr::Prop_FPGAVersion_Uint64, 538U);
            l_vrProperties->SetUint64Property(m_propertyContainer, vr::Prop_VRCVersion_Uint64, 1562916277U);
            l_vrProperties->SetUint64Property(m_propertyContainer, vr::Prop_RadioVersion_Uint64, 1562882729U);
            l_vrProperties->SetUint64Property(m_propertyContainer, vr::Prop_DongleVersion_Uint64, 1558748372U);
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_Firmware_ProgrammingTarget_String, (m_handAssigment == CHA_Left) ? "LHR-E217CD00" : "LHR-E217CD01"); // Changed
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_ResourceRoot_String, "indexcontroller");
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_RegisteredDeviceType_String, (m_handAssigment == CHA_Left) ? "valve/index_controllerLHR-E217CD00" : "valve/index_controllerLHR-E217CD01"); // Changed
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_InputProfilePath_String, "{indexcontroller}/input/index_controller_profile.json");
            l_vrProperties->SetInt32Property(m_propertyContainer, vr::Prop_Axis2Type_Int32, vr::k_eControllerAxis_Trigger);
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceOff_String, (m_handAssigment == CHA_Left) ? "{indexcontroller}/icons/left_controller_status_off.png" : "{indexcontroller}/icons/right_controller_status_off.png");
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceSearching_String, (m_handAssigment == CHA_Left) ? "{indexcontroller}/icons/left_controller_status_searching.gif" : "{indexcontroller}/icons/right_controller_status_searching.gif");
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceSearchingAlert_String, (m_handAssigment == CHA_Left) ? "{indexcontroller}/icons/left_controller_status_searching_alert.gif" : "{indexcontroller}/icons//right_controller_status_searching_alert.gif");
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceReady_String, (m_handAssigment == CHA_Left) ? "{indexcontroller}/icons/left_controller_status_ready.png" : "{indexcontroller}/icons//right_controller_status_ready.png");
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceReadyAlert_String, (m_handAssigment == CHA_Left) ? "{indexcontroller}/icons/left_controller_status_ready_alert.png" : "{indexcontroller}/icons//right_controller_status_ready_alert.png");
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceNotReady_String, (m_handAssigment == CHA_Left) ? "{indexcontroller}/icons/left_controller_status_error.png" : "{indexcontroller}/icons//right_controller_status_error.png");
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceStandby_String, (m_handAssigment == CHA_Left) ? "{indexcontroller}/icons/left_controller_status_off.png" : "{indexcontroller}/icons//right_controller_status_off.png");
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceAlertLow_String, (m_handAssigment == CHA_Left) ? "{indexcontroller}/icons/left_controller_status_ready_low.png" : "{indexcontroller}/icons//right_controller_status_ready_low.png");
            l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_ControllerType_String, "knuckles");
        } break;
    }

    // Hidden property for leap_monitor
    l_vrProperties->SetUint64Property(m_propertyContainer, vr::Prop_VendorSpecific_Reserved_Start, 0x1EA8U);

    // Inputs
    m_driverInput = vr::VRDriverInput();
    switch(CConfigHelper::GetEmulatedController())
    {
        case CConfigHelper::EC_Vive:
        {
            m_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/system/click", &m_buttons[CB_SysClick].GetHandleRef());
            m_buttons[CB_SysClick].SetInputType(ControllerButtonInputType::CBIT_Boolean);

            m_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/grip/click", &m_buttons[CB_GripClick].GetHandleRef());
            m_buttons[CB_GripClick].SetInputType(ControllerButtonInputType::CBIT_Boolean);

            m_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/application_menu/click", &m_buttons[CB_AppMenuClick].GetHandleRef());
            m_buttons[CB_AppMenuClick].SetInputType(ControllerButtonInputType::CBIT_Boolean);

            m_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/trigger/click", &m_buttons[CB_TriggerClick].GetHandleRef());
            m_buttons[CB_TriggerClick].SetInputType(ControllerButtonInputType::CBIT_Boolean);

            m_driverInput->CreateScalarComponent(m_propertyContainer, "/input/trigger/value", &m_buttons[CB_TriggerValue].GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
            m_buttons[CB_TriggerValue].SetInputType(ControllerButtonInputType::CBIT_Float);

            m_driverInput->CreateScalarComponent(m_propertyContainer, "/input/trackpad/x", &m_buttons[CB_TrackpadX].GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
            m_buttons[CB_TrackpadX].SetInputType(ControllerButtonInputType::CBIT_Float);

            m_driverInput->CreateScalarComponent(m_propertyContainer, "/input/trackpad/y", &m_buttons[CB_TrackpadY].GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
            m_buttons[CB_TrackpadY].SetInputType(ControllerButtonInputType::CBIT_Float);

            m_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/trackpad/click", &m_buttons[CB_TrackpadClick].GetHandleRef());
            m_buttons[CB_TrackpadClick].SetInputType(ControllerButtonInputType::CBIT_Boolean);

            m_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/trackpad/touch", &m_buttons[CB_TrackpadTouch].GetHandleRef());
            m_buttons[CB_TrackpadTouch].SetInputType(ControllerButtonInputType::CBIT_Boolean);
        } break;
        case CConfigHelper::EC_Index:
        {
            m_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/system/click", &m_buttons[CB_SysClick].GetHandleRef());
            m_buttons[CB_SysClick].SetInputType(ControllerButtonInputType::CBIT_Boolean);

            m_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/system/touch", &m_buttons[CB_SysTouch].GetHandleRef());
            m_buttons[CB_SysTouch].SetInputType(ControllerButtonInputType::CBIT_Boolean);

            m_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/grip/touch", &m_buttons[CB_GripTouch].GetHandleRef());
            m_buttons[CB_GripTouch].SetInputType(ControllerButtonInputType::CBIT_Boolean);

            m_driverInput->CreateScalarComponent(m_propertyContainer, "/input/grip/force", &m_buttons[CB_GripForce].GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
            m_buttons[CB_GripForce].SetInputType(ControllerButtonInputType::CBIT_Float);

            m_driverInput->CreateScalarComponent(m_propertyContainer, "/input/grip/value", &m_buttons[CB_GripValue].GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
            m_buttons[CB_GripValue].SetInputType(ControllerButtonInputType::CBIT_Float);

            m_driverInput->CreateScalarComponent(m_propertyContainer, "/input/trackpad/x", &m_buttons[CB_TrackpadX].GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
            m_buttons[CB_TrackpadX].SetInputType(ControllerButtonInputType::CBIT_Float);

            m_driverInput->CreateScalarComponent(m_propertyContainer, "/input/trackpad/y", &m_buttons[CB_TrackpadY].GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
            m_buttons[CB_TrackpadY].SetInputType(ControllerButtonInputType::CBIT_Float);

            m_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/trackpad/touch", &m_buttons[CB_TrackpadTouch].GetHandleRef());
            m_buttons[CB_TrackpadTouch].SetInputType(ControllerButtonInputType::CBIT_Boolean);

            m_driverInput->CreateScalarComponent(m_propertyContainer, "/input/trackpad/force", &m_buttons[CB_TrackpadForce].GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
            m_buttons[CB_TrackpadForce].SetInputType(ControllerButtonInputType::CBIT_Float);

            m_driverInput->CreateScalarComponent(m_propertyContainer, "/input/thumbstick/x", &m_buttons[CB_ThumbstickX].GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
            m_buttons[CB_ThumbstickX].SetInputType(ControllerButtonInputType::CBIT_Float);

            m_driverInput->CreateScalarComponent(m_propertyContainer, "/input/thumbstick/y", &m_buttons[CB_ThumbstickY].GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
            m_buttons[CB_ThumbstickY].SetInputType(ControllerButtonInputType::CBIT_Float);

            m_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/thumbstick/click", &m_buttons[CB_ThumbstickClick].GetHandleRef());
            m_buttons[CB_ThumbstickClick].SetInputType(ControllerButtonInputType::CBIT_Boolean);

            m_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/thumbstick/touch", &m_buttons[CB_ThumbstickTouch].GetHandleRef());
            m_buttons[CB_ThumbstickTouch].SetInputType(ControllerButtonInputType::CBIT_Boolean);

            m_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/a/click", &m_buttons[CB_IndexAClick].GetHandleRef());
            m_buttons[CB_IndexAClick].SetInputType(ControllerButtonInputType::CBIT_Boolean);

            m_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/a/touch", &m_buttons[CB_IndexATouch].GetHandleRef());
            m_buttons[CB_IndexATouch].SetInputType(ControllerButtonInputType::CBIT_Boolean);

            m_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/b/click", &m_buttons[CB_IndexBClick].GetHandleRef());
            m_buttons[CB_IndexBClick].SetInputType(ControllerButtonInputType::CBIT_Boolean);

            m_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/b/touch", &m_buttons[CB_IndexBTouch].GetHandleRef());
            m_buttons[CB_IndexBTouch].SetInputType(ControllerButtonInputType::CBIT_Boolean);

            m_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/trigger/click", &m_buttons[CB_TriggerClick].GetHandleRef());
            m_buttons[CB_TriggerClick].SetInputType(ControllerButtonInputType::CBIT_Boolean);

            m_driverInput->CreateScalarComponent(m_propertyContainer, "/input/trigger/value", &m_buttons[CB_TriggerValue].GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
            m_buttons[CB_TriggerValue].SetInputType(ControllerButtonInputType::CBIT_Float);

            m_driverInput->CreateScalarComponent(m_propertyContainer, "/input/finger/index", &m_buttons[CB_FingerIndex].GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
            m_buttons[CB_FingerIndex].SetInputType(ControllerButtonInputType::CBIT_Float);

            m_driverInput->CreateScalarComponent(m_propertyContainer, "/input/finger/middle", &m_buttons[CB_FingerMiddle].GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
            m_buttons[CB_FingerMiddle].SetInputType(ControllerButtonInputType::CBIT_Float);

            m_driverInput->CreateScalarComponent(m_propertyContainer, "/input/finger/ring", &m_buttons[CB_FingerRing].GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
            m_buttons[CB_FingerRing].SetInputType(ControllerButtonInputType::CBIT_Float);

            m_driverInput->CreateScalarComponent(m_propertyContainer, "/input/finger/pinky", &m_buttons[CB_FingerPinky].GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
            m_buttons[CB_FingerPinky].SetInputType(ControllerButtonInputType::CBIT_Float);

            vr::EVRSkeletalTrackingLevel l_trackingLevel = vr::VRSkeletalTracking_Partial;
            switch(CConfigHelper::GetTrackingLevel())
            {
                case CConfigHelper::TL_Partial:
                    l_trackingLevel = vr::VRSkeletalTracking_Partial;
                    break;
                case CConfigHelper::TL_Full:
                    l_trackingLevel = vr::VRSkeletalTracking_Full;
                    break;
            }
            switch(m_handAssigment)
            {
                case CHA_Left:
                    m_driverInput->CreateSkeletonComponent(m_propertyContainer, "/input/skeleton/left", "/skeleton/hand/left", "/pose/raw", l_trackingLevel, nullptr, 0U, &m_skeletonHandle);
                    break;
                case CHA_Right:
                    m_driverInput->CreateSkeletonComponent(m_propertyContainer, "/input/skeleton/right", "/skeleton/hand/right", "/pose/raw", l_trackingLevel, nullptr, 0U, &m_skeletonHandle);
                    break;
            }
        } break;
    }

    // Fake haptic output to remove errors in vrserver log
    m_driverInput->CreateHapticComponent(m_propertyContainer, "/output/haptic", &m_haptic);

    return vr::VRInitError_None;
}

void CLeapHandController::Deactivate()
{
    m_trackedDeviceID = vr::k_unTrackedDeviceIndexInvalid;
}

void* CLeapHandController::GetComponent(const char* pchComponentNameAndVersion)
{
    void *l_result = nullptr;
    if(!strcmp(pchComponentNameAndVersion, vr::ITrackedDeviceServerDriver_Version)) l_result = this;
    return l_result;
}

void CLeapHandController::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
{
    std::stringstream l_stream(pchRequest);
    std::string l_command;

    l_stream >> l_command;
    if(!l_stream.fail() && !l_command.empty())
    {
        switch(ReadEnumVector(l_command, g_debugRequests))
        {
            case DebugRequest::DR_Profile:
            {
                std::string l_profile;
                l_stream >> l_profile;

                if(!l_stream.fail() && !l_profile.empty())
                {
                    GameProfile l_newProfile;
                    switch(ReadEnumVector(l_profile, g_gameProfiles))
                    {
                        case GP_Default:
                            l_newProfile = GP_Default;
                            break;
                        case GP_VRChat:
                            l_newProfile = GP_VRChat;
                            break;
                        default:
                            l_newProfile = GP_Default;
                            break;
                    }
                    if(m_gameProfile != l_newProfile)
                    {
                        m_gameProfile = l_newProfile;
                        ResetControls();
                    }
                }
            }

            case DR_Game:
            {
                std::string l_game;
                l_stream >> l_game;
                if(!l_stream.fail() && !l_game.empty())
                {
                    switch(ReadEnumVector(l_game, g_gameList))
                    {
                        case GL_VRChat:
                        {
                            std::string l_gameCommand;
                            l_stream >> l_gameCommand;
                            if(!l_stream.fail() && !l_game.empty())
                            {
                                switch(ReadEnumVector(l_gameCommand, g_gameCommands))
                                {
                                    case CC_DrawingMode:
                                    {
                                        m_gameSpecialModes.m_vrchatDrawingMode = !m_gameSpecialModes.m_vrchatDrawingMode;
                                    } break;
                                }
                            }
                        } break;
                    }
                }
            } break;
        }
    }
}

vr::DriverPose_t CLeapHandController::GetPose()
{
    return m_pose;
}

const char* CLeapHandController::GetSerialNumber() const
{
    return m_serialNumber.c_str();
}

void CLeapHandController::SetAsDisconnected()
{
    if(m_trackedDeviceID != vr::k_unTrackedDeviceIndexInvalid)
    {
        m_pose.deviceIsConnected = false;
        m_driverHost->TrackedDevicePoseUpdated(m_trackedDeviceID, m_pose, sizeof(vr::DriverPose_t));
    }
}

void CLeapHandController::Update(const Leap::Frame &f_frame)
{
    UpdateTrasnformation(f_frame);
    UpdateGestures(f_frame);
    UpdateInput();
}

void CLeapHandController::UpdateTrasnformation(const Leap::Frame &f_frame)
{
    if(m_trackedDeviceID != vr::k_unTrackedDeviceIndexInvalid)
    {
        bool l_handFound = false;
        Leap::HandList &l_hands = f_frame.hands();
        for(auto l_hand : l_hands)
        {
            if(l_hand.isValid())
            {
                if(((m_handAssigment == CHA_Left) && l_hand.isLeft()) || ((m_handAssigment == CHA_Right) && l_hand.isRight()))
                {
                    switch(CConfigHelper::GetOrientationMode())
                    {
                        case CConfigHelper::OM_HMD:
                        {
                            std::memcpy(&m_pose.qWorldFromDriverRotation, &ms_headRot, sizeof(vr::HmdQuaternion_t));
                            std::memcpy(m_pose.vecWorldFromDriverTranslation, ms_headPos, sizeof(double) * 3U);

                            Leap::Vector l_position = l_hand.palmPosition();
                            m_pose.vecPosition[0] = -0.001*l_position.x;
                            m_pose.vecPosition[1] = -0.001*l_position.z;
                            m_pose.vecPosition[2] = -0.001*l_position.y - 0.15; // ?

                            Leap::Vector l_velocity = l_hand.palmVelocity();
                            m_pose.vecVelocity[0] = -0.001*l_velocity.x;
                            m_pose.vecVelocity[1] = -0.001*l_velocity.z;
                            m_pose.vecVelocity[2] = -0.001*l_velocity.y;

                            Leap::Vector l_handDirection = l_hand.direction();
                            l_handDirection /= l_handDirection.magnitude();

                            Leap::Vector l_palmNormal = l_hand.palmNormal();
                            l_palmNormal /= l_palmNormal.magnitude();

                            Leap::Vector l_leapCross = l_handDirection.cross(l_palmNormal);
                            switch(m_handAssigment)
                            {
                                case CHA_Left:
                                    l_palmNormal *= -1.f;
                                    break;
                                case CHA_Right:
                                    l_leapCross *= -1.f;
                                    break;
                            }

                            glm::mat3 l_rotMat(
                                l_palmNormal.x, l_palmNormal.z, l_palmNormal.y,
                                l_leapCross.x, l_leapCross.z, l_leapCross.y,
                                l_handDirection.x, l_handDirection.z, l_handDirection.y
                                );
                            glm::quat l_finalRot = glm::quat_cast(l_rotMat);
                            l_finalRot *= m_gripAngleOffset;

                            m_pose.qRotation.x = l_finalRot.x;
                            m_pose.qRotation.y = l_finalRot.y;
                            m_pose.qRotation.z = l_finalRot.z;
                            m_pose.qRotation.w = l_finalRot.w;
                        } break;
                        case CConfigHelper::OM_Desktop:
                        {
                            // Head position and rotation are fixed
                            m_pose.qWorldFromDriverRotation.x = .0;
                            m_pose.qWorldFromDriverRotation.y = .0;
                            m_pose.qWorldFromDriverRotation.z = .0;
                            m_pose.qWorldFromDriverRotation.w = 1.0;
                            m_pose.vecWorldFromDriverTranslation[0U] = CConfigHelper::GetDesktopRootX();
                            m_pose.vecWorldFromDriverTranslation[1U] = CConfigHelper::GetDesktopRootY();
                            m_pose.vecWorldFromDriverTranslation[2U] = CConfigHelper::GetDesktopRootZ();

                            Leap::Vector l_position = l_hand.palmPosition();
                            m_pose.vecPosition[0] = 0.001*l_position.x;
                            m_pose.vecPosition[1] = 0.001*l_position.y;
                            m_pose.vecPosition[2] = 0.001*l_position.z;

                            Leap::Vector l_velocity = l_hand.palmVelocity();
                            m_pose.vecVelocity[0] = 0.001*l_velocity.x;
                            m_pose.vecVelocity[1] = 0.001*l_velocity.y;
                            m_pose.vecVelocity[2] = 0.001*l_velocity.z;

                            Leap::Vector l_handDirection = -l_hand.direction();
                            l_handDirection /= l_handDirection.magnitude();

                            Leap::Vector l_palmNormal = -l_hand.palmNormal();
                            l_palmNormal /= l_palmNormal.magnitude();

                            Leap::Vector l_leapCross = -l_handDirection.cross(l_palmNormal);
                            switch(m_handAssigment)
                            {
                                case CHA_Left:
                                    l_palmNormal *= -1.f;
                                    break;
                                case CHA_Right:
                                    l_leapCross *= -1.f;
                                    break;
                            }

                            glm::mat3 l_rotMat(
                                l_palmNormal.x, l_palmNormal.y, l_palmNormal.z,
                                l_leapCross.x, l_leapCross.y, l_leapCross.z,
                                l_handDirection.x, l_handDirection.y, l_handDirection.z
                                );
                            glm::quat l_finalRot = glm::quat_cast(l_rotMat);
                            l_finalRot *= m_gripAngleOffset;

                            m_pose.qRotation.x = l_finalRot.x;
                            m_pose.qRotation.y = l_finalRot.y;
                            m_pose.qRotation.z = l_finalRot.z;
                            m_pose.qRotation.w = l_finalRot.w;
                        } break;
                    }

                    l_handFound = true;
                    m_pose.result = vr::TrackingResult_Running_OK;
                    break;
                }
            }
        }

        if(!l_handFound)
        {
            for(size_t i = 0U; i < 3U; i++) m_pose.vecVelocity[i] = .0;
            m_pose.result = vr::TrackingResult_Calibrating_InProgress;
        }
        m_pose.poseIsValid = ((m_gameProfile == GP_VRChat) ? true : l_handFound);
        if(!m_pose.deviceIsConnected) m_pose.deviceIsConnected = true;

        m_driverHost->TrackedDevicePoseUpdated(m_trackedDeviceID, m_pose, sizeof(vr::DriverPose_t));
    }
}

void CLeapHandController::UpdateGestures(const Leap::Frame& f_frame)
{
    std::vector<float> l_scores;
    if(CGestureMatcher::GetGestures(f_frame, ((m_handAssigment == CHA_Left) ? CGestureMatcher::WH_LeftHand : CGestureMatcher::WH_RightHand), l_scores))
    {
        switch(CConfigHelper::GetEmulatedController())
        {
            case CConfigHelper::EC_Vive:
                ProcessViveGestures(l_scores);
                break;
            case CConfigHelper::EC_Index:
                ProcessIndexGestures(f_frame, l_scores);
                break;
        }
    }
}

void CLeapHandController::ProcessViveGestures(const std::vector<float> &f_scores)
{
    switch(m_gameProfile)
    {
        case GP_Default:
        {
            if(CConfigHelper::IsInputEnabled())
            {
                if(CConfigHelper::IsMenuEnabled()) m_buttons[CB_SysClick].SetState(f_scores[CGestureMatcher::GT_Timeout] >= 0.25f);
                if(CConfigHelper::IsApplicationMenuEnabled()) m_buttons[CB_AppMenuClick].SetState(f_scores[CGestureMatcher::GT_FlatHandPalmTowards] >= 0.8f);

                if(CConfigHelper::IsTriggerEnabled())
                {
                    m_buttons[CB_TriggerClick].SetState(f_scores[CGestureMatcher::GT_TriggerFinger] >= 0.75f);
                    m_buttons[CB_TriggerValue].SetValue(f_scores[CGestureMatcher::GT_TriggerFinger]);
                }

                if(CConfigHelper::IsGripEnabled()) m_buttons[CB_GripClick].SetState(f_scores[CGestureMatcher::GT_LowerFist] >= 0.5f);

                if(CConfigHelper::IsTouchpadEnabled())
                {
                    if(CConfigHelper::IsTouchpadAxesEnabled())
                    {
                        m_buttons[CB_TrackpadX].SetValue(f_scores[CGestureMatcher::GT_TouchpadAxisX]);
                        m_buttons[CB_TrackpadY].SetValue(f_scores[CGestureMatcher::GT_TouchpadAxisY]);
                    }
                    if(CConfigHelper::IsTouchpadTouchEnabled()) m_buttons[CB_TrackpadTouch].SetState(f_scores[CGestureMatcher::GT_Thumbpress] >= 0.5f);
                    if(CConfigHelper::IsTouchpadPressEnabled()) m_buttons[CB_TrackpadClick].SetState(f_scores[CGestureMatcher::GT_Thumbpress] >= 0.1f);
                }
            }
        } break;

        case GP_VRChat:
        {
            if(CConfigHelper::IsInputEnabled())
            {
                if(!m_gameSpecialModes.m_vrchatDrawingMode)
                {
                    m_buttons[CB_AppMenuClick].SetState(f_scores[CGestureMatcher::GT_Timeout] >= 0.75f);

                    glm::vec2 l_trackpadAxis(0.f);
                    if(f_scores[CGestureMatcher::GT_VRChatPoint] >= 0.75f)
                    {
                        l_trackpadAxis.x = 0.0f;
                        l_trackpadAxis.y = 1.0f;
                    }
                    else if(f_scores[CGestureMatcher::GT_VRChatThumbsUp] >= 0.75f)
                    {
                        l_trackpadAxis.x = -0.95f;
                        l_trackpadAxis.y = 0.31f;
                    }
                    else if(f_scores[CGestureMatcher::GT_VRChatVictory] >= 0.75f)
                    {
                        l_trackpadAxis.x = 0.95f;
                        l_trackpadAxis.y = 0.31f;
                    }
                    else if(f_scores[CGestureMatcher::GT_VRChatGun] >= 0.75f)
                    {
                        l_trackpadAxis.x = -0.59f;
                        l_trackpadAxis.y = -0.81f;
                    }
                    else if(f_scores[CGestureMatcher::GT_VRChatRockOut] >= 0.75f)
                    {
                        l_trackpadAxis.x = 0.59f;
                        l_trackpadAxis.y = -0.81f;
                    }
                    if(m_handAssigment == CHA_Left) l_trackpadAxis.x *= -1.f;
                    m_buttons[CB_TrackpadX].SetValue(l_trackpadAxis.x);
                    m_buttons[CB_TrackpadY].SetValue(l_trackpadAxis.y);
                    m_buttons[CB_TrackpadTouch].SetState((l_trackpadAxis.x != 0.f) || (l_trackpadAxis.y != 0.f));

                    m_buttons[CB_TriggerValue].SetValue(f_scores[CGestureMatcher::GT_LowerFist]);
                    m_buttons[CB_TriggerClick].SetState(f_scores[CGestureMatcher::GT_LowerFist] >= 0.5f);
                    m_buttons[CB_GripClick].SetState(f_scores[CGestureMatcher::GT_VRChatSpreadHand] >= 0.75f);
                }
                else
                {
                    m_buttons[CB_AppMenuClick].SetState(false);
                    m_buttons[CB_TrackpadX].SetValue(0.f);
                    m_buttons[CB_TrackpadY].SetValue(0.f);
                    m_buttons[CB_TrackpadTouch].SetState(false);
                    m_buttons[CB_TriggerValue].SetValue((f_scores[CGestureMatcher::GT_LowerFist] >= 0.85f) ? 0.85f : 0.f);
                    m_buttons[CB_TriggerClick].SetState(f_scores[CGestureMatcher::GT_LowerFist] >= 0.85f);
                    m_buttons[CB_GripClick].SetState(false);
                }
            }
        } break;
    }
}

void CLeapHandController::ProcessIndexGestures(const Leap::Frame &f_frame, const std::vector<float> &f_scores)
{
    switch(m_gameProfile)
    {
        case GP_Default:
        {
            if(CConfigHelper::IsInputEnabled())
            {
                if(CConfigHelper::IsMenuEnabled())
                {
                    m_buttons[CB_SysClick].SetState(f_scores[CGestureMatcher::GT_Timeout] >= 0.5f);
                    m_buttons[CB_SysTouch].SetState(f_scores[CGestureMatcher::GT_Timeout] >= 0.25f);
                }
                if(CConfigHelper::IsGripEnabled())
                {
                    m_buttons[CB_GripTouch].SetState(f_scores[CGestureMatcher::GT_LowerFist] >= 0.75f);
                    m_buttons[CB_GripValue].SetValue(f_scores[CGestureMatcher::GT_LowerFist]);
                    m_buttons[CB_GripForce].SetValue(f_scores[CGestureMatcher::GT_LowerFist] >= 0.75f ? (f_scores[CGestureMatcher::GT_LowerFist] - 0.75f) / 0.25f : 0.f);
                }
                if(CConfigHelper::IsTouchpadEnabled())
                {
                    if(CConfigHelper::IsTouchpadTouchEnabled())
                    {
                        if(f_scores[CGestureMatcher::GT_Thumbpress] >= 0.5f)
                        {
                            m_buttons[CB_TrackpadTouch].SetState(true);
                            m_buttons[CB_TrackpadForce].SetValue(f_scores[CGestureMatcher::GT_Thumbpress] >= 0.75f ? (f_scores[CGestureMatcher::GT_Thumbpress] - 0.75f) / 0.25f : 0.f);
                            if(CConfigHelper::IsTouchpadAxesEnabled())
                            {
                                m_buttons[CB_TrackpadX].SetValue(f_scores[CGestureMatcher::GT_TouchpadAxisX]);
                                m_buttons[CB_TrackpadY].SetValue(f_scores[CGestureMatcher::GT_TouchpadAxisY]);
                            }
                        }
                        else
                        {
                            m_buttons[CB_TrackpadTouch].SetState(false);
                            m_buttons[CB_TrackpadForce].SetValue(0.f);
                        }
                    }
                }
                if(CConfigHelper::IsTriggerEnabled())
                {
                    m_buttons[CB_TriggerClick].SetState(f_scores[CGestureMatcher::GT_TriggerFinger] >= 0.75f);
                    m_buttons[CB_TriggerValue].SetValue(f_scores[CGestureMatcher::GT_TriggerFinger]);
                }
                if(CConfigHelper::IsThumbstickEnabled())
                {
                    m_buttons[CB_ThumbstickTouch].SetState(f_scores[CGestureMatcher::GT_ThumbIndexCrossTouch] >= 0.5f);
                    m_buttons[CB_ThumbstickClick].SetState(f_scores[CGestureMatcher::GT_ThumbIndexCrossTouch] >= 0.75f);
                }
                if(CConfigHelper::IsButtonAEnabled())
                {
                    m_buttons[CB_IndexATouch].SetState(f_scores[CGestureMatcher::GT_ThumbMiddleTouch] >= 0.5f);
                    m_buttons[CB_IndexAClick].SetState(f_scores[CGestureMatcher::GT_ThumbMiddleTouch] >= 0.75f);
                }
                if(CConfigHelper::IsButtonBEnabled())
                {
                    m_buttons[CB_IndexBTouch].SetState(f_scores[CGestureMatcher::GT_ThumbPinkyTouch] >= 0.5f);
                    m_buttons[CB_IndexBClick].SetState(f_scores[CGestureMatcher::GT_ThumbPinkyTouch] >= 0.75f);
                }
            }
        } break;
        case GP_VRChat:
        {
            if(CConfigHelper::IsInputEnabled())
            {
                if(CConfigHelper::IsTouchpadEnabled())
                {
                    if(CConfigHelper::IsTouchpadTouchEnabled())
                    {
                        if(f_scores[CGestureMatcher::GT_Thumbpress] >= 0.5f)
                        {
                            m_buttons[CB_TrackpadTouch].SetState(true);
                            m_buttons[CB_TrackpadForce].SetValue(f_scores[CGestureMatcher::GT_Thumbpress] >= 0.75f ? (f_scores[CGestureMatcher::GT_Thumbpress] - 0.75f) / 0.25f : 0.f);
                        }
                        else
                        {
                            m_buttons[CB_TrackpadTouch].SetState(false);
                            m_buttons[CB_TrackpadForce].SetValue(0.f);
                        }
                    }
                    if(CConfigHelper::IsTriggerEnabled())
                    {
                        m_buttons[CB_TriggerClick].SetState(f_scores[CGestureMatcher::GT_TriggerFinger] >= 0.75f);
                        m_buttons[CB_TriggerValue].SetValue(f_scores[CGestureMatcher::GT_TriggerFinger]);
                    }
                    if(CConfigHelper::IsGripEnabled())
                    {
                        m_buttons[CB_GripTouch].SetState(f_scores[CGestureMatcher::GT_LowerFist] >= 0.85f);
                        m_buttons[CB_GripValue].SetValue(f_scores[CGestureMatcher::GT_LowerFist]);
                        m_buttons[CB_GripForce].SetValue(f_scores[CGestureMatcher::GT_LowerFist] >= 0.85f ? (f_scores[CGestureMatcher::GT_LowerFist] - 0.85f) / 0.15f : 0.f);
                    }
                    if(CConfigHelper::IsButtonBEnabled())
                    {
                        m_buttons[CB_IndexBTouch].SetState(f_scores[CGestureMatcher::GT_Timeout] >= 0.5f);
                        m_buttons[CB_IndexBClick].SetState(f_scores[CGestureMatcher::GT_Timeout] >= 0.75f);
                    }
                }
            }
        } break;
    }

    // Update skeleton
    if(CConfigHelper::IsSkeletonEnabled())
    {
        m_buttons[CB_FingerIndex].SetValue(f_scores[CGestureMatcher::GT_IndexFingerBend]);
        m_buttons[CB_FingerMiddle].SetValue(f_scores[CGestureMatcher::GT_MiddleFingerBend]);
        m_buttons[CB_FingerRing].SetValue(f_scores[CGestureMatcher::GT_RingFingerBend]);
        m_buttons[CB_FingerPinky].SetValue(f_scores[CGestureMatcher::GT_PinkyFingerBend]);

        for(Leap::Hand l_hand : f_frame.hands())
        {
            if(l_hand.isValid())
            {
                if((l_hand.isLeft() && (m_handAssigment == CHA_Left)) || (l_hand.isRight() && (m_handAssigment == CHA_Right)))
                {
                    // Update rotations
                    glm::mat4 l_handMat;
                    Leap::Matrix l_leapMat = l_hand.basis();
                    ConvertMatrix(l_leapMat, l_handMat);
                    glm::mat4 l_handMatInv = glm::inverse(l_handMat);

                    for(Leap::Finger l_finger : l_hand.fingers())
                    {
                        if(l_finger.isValid())
                        {
                            size_t l_transformIndex = 0U;
                            switch(l_finger.type())
                            {
                                case Leap::Finger::TYPE_INDEX:
                                    l_transformIndex = HSB_IndexFinger1;
                                    break;
                                case Leap::Finger::TYPE_MIDDLE:
                                    l_transformIndex = HSB_MiddleFinger1;
                                    break;
                                case Leap::Finger::TYPE_PINKY:
                                    l_transformIndex = HSB_PinkyFinger1;
                                    break;
                                case Leap::Finger::TYPE_RING:
                                    l_transformIndex = HSB_RingFinger1;
                                    break;
                                case Leap::Finger::TYPE_THUMB:
                                    l_transformIndex = HSB_Thumb0;
                                    break;
                            }

                            glm::mat4 l_boneMat;
                            glm::quat l_result;

                            // Segment 1
                            l_leapMat = l_finger.bone(Leap::Bone::TYPE_PROXIMAL).basis();
                            ConvertMatrix(l_leapMat, l_boneMat);
                            l_result = l_boneMat*l_handMatInv;
                            if(l_finger.type() == Leap::Finger::TYPE_THUMB)
                            {
                                if(m_handAssigment == CHA_Left) l_result = glm::rotate(l_result, glm::pi<float>(), g_axisX);
                                else
                                {
                                    l_result.w *= -1.f;
                                    l_result.z *= -1.f;
                                    l_result = glm::rotate(l_result, glm::pi<float>(), g_axisY);
                                }
                            }
                            SwitchBoneAxes(l_result);
                            ConvertQuaternion(l_result, m_boneTransform[l_transformIndex].orientation);

                            // Segment 2
                            glm::mat4 l_inversedProximal = glm::inverse(l_boneMat);
                            l_leapMat = l_finger.bone(Leap::Bone::TYPE_INTERMEDIATE).basis();
                            ConvertMatrix(l_leapMat, l_boneMat);
                            l_result = l_boneMat*l_inversedProximal;
                            SwitchBoneAxes(l_result);
                            ConvertQuaternion(l_result, m_boneTransform[l_transformIndex + 1U].orientation);

                            // Segment 3
                            glm::mat4 l_inversedIntermediate = glm::inverse(l_boneMat);
                            l_leapMat = l_finger.bone(Leap::Bone::TYPE_DISTAL).basis();
                            ConvertMatrix(l_leapMat, l_boneMat);
                            l_result = l_boneMat*l_inversedIntermediate;
                            SwitchBoneAxes(l_result);
                            ConvertQuaternion(l_result, m_boneTransform[l_transformIndex + 2U].orientation);
                        }
                    }

                    // Update aux bones
                    glm::vec3 l_position;
                    glm::quat l_rotation;
                    ConvertVector3(m_boneTransform[HSB_Wrist].position, l_position);
                    ConvertQuaternion(m_boneTransform[HSB_Wrist].orientation, l_rotation);
                    glm::mat4 l_wristMat = glm::translate(g_identityMatrix, l_position) * glm::mat4_cast(l_rotation);

                    // Thumb aux
                    glm::mat4 l_chainMat(l_wristMat);
                    for(size_t i = HSB_Thumb0; i < HSB_Thumb3; i++)
                    {
                        ConvertVector3(m_boneTransform[i].position, l_position);
                        ConvertQuaternion(m_boneTransform[i].orientation, l_rotation);
                        l_chainMat = l_chainMat*(glm::translate(g_identityMatrix, l_position)*glm::mat4_cast(l_rotation));
                    }
                    l_position = l_chainMat*g_zeroPoint;
                    l_rotation = glm::quat_cast(l_chainMat);
                    if(m_handAssigment == CHA_Left) FixAuxBoneTransformation(l_position, l_rotation);
                    ConvertVector3(l_position, m_boneTransform[HSB_Aux_Thumb].position);
                    ConvertQuaternion(l_rotation, m_boneTransform[HSB_Aux_Thumb].orientation);

                    // Index aux
                    std::memcpy(&l_chainMat, &l_wristMat, sizeof(glm::mat4));
                    for(size_t i = HSB_IndexFinger0; i < HSB_IndexFinger4; i++)
                    {
                        ConvertVector3(m_boneTransform[i].position, l_position);
                        ConvertQuaternion(m_boneTransform[i].orientation, l_rotation);
                        l_chainMat = l_chainMat*(glm::translate(g_identityMatrix, l_position)*glm::mat4_cast(l_rotation));
                    }
                    l_position = l_chainMat*g_zeroPoint;
                    l_rotation = glm::quat_cast(l_chainMat);
                    if(m_handAssigment == CHA_Left) FixAuxBoneTransformation(l_position, l_rotation);
                    ConvertVector3(l_position, m_boneTransform[HSB_Aux_IndexFinger].position);
                    ConvertQuaternion(l_rotation, m_boneTransform[HSB_Aux_IndexFinger].orientation);

                    // Middle aux
                    std::memcpy(&l_chainMat, &l_wristMat, sizeof(glm::mat4));
                    for(size_t i = HSB_MiddleFinger0; i < HSB_MiddleFinger4; i++)
                    {
                        ConvertVector3(m_boneTransform[i].position, l_position);
                        ConvertQuaternion(m_boneTransform[i].orientation, l_rotation);
                        l_chainMat = l_chainMat*(glm::translate(g_identityMatrix, l_position)*glm::mat4_cast(l_rotation));
                    }
                    l_position = l_chainMat*g_zeroPoint;
                    l_rotation = glm::quat_cast(l_chainMat);
                    if(m_handAssigment == CHA_Left) FixAuxBoneTransformation(l_position, l_rotation);
                    ConvertVector3(l_position, m_boneTransform[HSB_Aux_MiddleFinger].position);
                    ConvertQuaternion(l_rotation, m_boneTransform[HSB_Aux_MiddleFinger].orientation);

                    // Ring aux
                    std::memcpy(&l_chainMat, &l_wristMat, sizeof(glm::mat4));
                    for(size_t i = HSB_RingFinger0; i < HSB_RingFinger4; i++)
                    {
                        ConvertVector3(m_boneTransform[i].position, l_position);
                        ConvertQuaternion(m_boneTransform[i].orientation, l_rotation);
                        l_chainMat = l_chainMat*(glm::translate(g_identityMatrix, l_position)*glm::mat4_cast(l_rotation));
                    }
                    l_position = l_chainMat*g_zeroPoint;
                    l_rotation = glm::quat_cast(l_chainMat);
                    if(m_handAssigment == CHA_Left) FixAuxBoneTransformation(l_position, l_rotation);
                    ConvertVector3(l_position, m_boneTransform[HSB_Aux_RingFinger].position);
                    ConvertQuaternion(l_rotation, m_boneTransform[HSB_Aux_RingFinger].orientation);

                    // Pinky aux
                    std::memcpy(&l_chainMat, &l_wristMat, sizeof(glm::mat4));
                    for(size_t i = HSB_PinkyFinger0; i < HSB_PinkyFinger4; i++)
                    {
                        ConvertVector3(m_boneTransform[i].position, l_position);
                        ConvertQuaternion(m_boneTransform[i].orientation, l_rotation);
                        l_chainMat = l_chainMat*(glm::translate(g_identityMatrix, l_position)*glm::mat4_cast(l_rotation));
                    }
                    l_position = l_chainMat*g_zeroPoint;
                    l_rotation = glm::quat_cast(l_chainMat);
                    if(m_handAssigment == CHA_Left) FixAuxBoneTransformation(l_position, l_rotation);
                    ConvertVector3(l_position, m_boneTransform[HSB_Aux_PinkyFinger].position);
                    ConvertQuaternion(l_rotation, m_boneTransform[HSB_Aux_PinkyFinger].orientation);

                    break;
                }
            }
        }
    }
}

void CLeapHandController::UpdateInput()
{
    for(size_t i = 0U; i < CB_Count; i++)
    {
        CControllerButton &l_button = m_buttons[i];
        if(l_button.IsUpdated())
        {
            switch(l_button.GetInputType())
            {
                case ControllerButtonInputType::CBIT_Boolean:
                    m_driverInput->UpdateBooleanComponent(l_button.GetHandle(), l_button.GetState(), .0);
                    break;
                case ControllerButtonInputType::CBIT_Float:
                    m_driverInput->UpdateScalarComponent(l_button.GetHandle(), l_button.GetValue(), .0);
                    break;
            }
            l_button.ResetUpdate();
        }
    }
    if(CConfigHelper::GetEmulatedController() == CConfigHelper::EC_Index)
    {
        m_driverInput->UpdateSkeletonComponent(m_skeletonHandle, vr::VRSkeletalMotionRange_WithController, m_boneTransform, HSB_Count);
        m_driverInput->UpdateSkeletonComponent(m_skeletonHandle, vr::VRSkeletalMotionRange_WithoutController, m_boneTransform, HSB_Count);
    }
}

void CLeapHandController::ResetControls()
{
    for(size_t i = 0U; i < CB_Count; i++)
    {
        CControllerButton &l_button = m_buttons[i];
        l_button.SetValue(0.f);
        l_button.SetState(false);
    }
}

void CLeapHandController::UpdateHMDCoordinates(vr::IVRServerDriverHost *f_host)
{
    vr::TrackedDevicePose_t l_hmdPose;
    f_host->GetRawTrackedDevicePoses(0.f, &l_hmdPose, 1U); // HMD has device ID 0
    if(l_hmdPose.bPoseIsValid)
    {
        glm::mat4 l_rotMat(1.f);
        ConvertMatrix(l_hmdPose.mDeviceToAbsoluteTracking, l_rotMat);
        glm::quat l_headRot = glm::quat_cast(l_rotMat);
        ms_headRot.x = l_headRot.x;
        ms_headRot.y = l_headRot.y;
        ms_headRot.z = l_headRot.z;
        ms_headRot.w = l_headRot.w;

        for(size_t i = 0U; i < 3U; i++) ms_headPos[i] = l_hmdPose.mDeviceToAbsoluteTracking.m[i][3];
    }
}
