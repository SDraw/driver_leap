#include "stdafx.h"
#include "Devices/Controller/CLeapIndexController.h"
#include "Devices/Controller/CControllerButton.h"
#include "Core/CDriverConfig.h"
#include "Leap/CLeapHand.h"
#include "Utils/Utils.h"

extern const std::array<vr::VRBoneTransform_t, 31U> g_openHandGesture;
const glm::mat4 g_identityMatrix(1.f);
const float g_pi = glm::pi<float>();
const float g_piHalf = g_pi * 0.5f;
const glm::vec4 g_pointVec4(0.f, 0.f, 0.f, 1.f);

// This is ... something ...
const glm::mat4 g_wristOffsetLeft = glm::inverse(glm::translate(g_identityMatrix, glm::vec3(-0.0445230342f, 0.0301547553f, 0.16438961f)) * glm::toMat4(glm::quat(1.50656376e-07f, -1.77612698e-08f, 0.927827835f, -0.373008907f)));
const glm::mat4 g_wristOffsetRight = glm::inverse(glm::translate(g_identityMatrix, glm::vec3(0.0445230342f, 0.0301547553f, 0.16438961f)) * glm::toMat4(glm::quat(1.50656376e-07f, -1.77612698e-08f, -0.927827835f, 0.373008907f)));
const glm::quat g_skeletonOffsetLeft = glm::quat(glm::vec3(g_pi, 0.f, g_piHalf));
const glm::quat g_skeletonOffsetRight = glm::quat(glm::vec3(g_pi, 0.f, -g_piHalf));
const glm::quat g_thumbOffset = glm::quat(glm::vec3(-g_piHalf * 0.5, 0.f, 0.f)) * glm::quat(glm::vec3(-g_piHalf, g_piHalf, -g_piHalf));
const glm::quat g_metacarpalOffset = glm::quat(glm::vec3(-g_piHalf, g_piHalf, 0.f));
const glm::quat g_mirroringOffset = glm::quat(glm::vec3(g_pi, 0.f, 0.f));

double CLeapIndexController::ms_headPosition[] = { .0, .0, .0 };
vr::HmdQuaternion_t CLeapIndexController::ms_headRotation = { 1.0, .0, .0, .0 };

enum TypeName : size_t
{
    TN_Button,
    TN_Axis
};
const std::vector<std::string> g_typeNames =
{
    "button", "axis"
};

enum InputName : size_t
{
    IN_A,
    IN_B,
    IN_System,
    IN_Thumbstick,
    IN_Touchpad,
};
const std::vector<std::string> g_inputNames =
{
    "a", "b", "system", "thumbstick", "touchpad"
};

enum StateName : size_t
{
    ST_None,
    ST_Touched,
    ST_Clicked,
};
const std::vector<std::string> g_stateNames =
{
    "none", "touched", "clicked"
};

CLeapIndexController::CLeapIndexController(bool p_left)
{
    m_isLeft = p_left;
    m_serialNumber.assign(m_isLeft ? "LHR-E217CD00" : "LHR-E217CD01");

    m_trackedDevice = vr::k_unTrackedDeviceIndexInvalid;
    m_propertyContainer = vr::k_ulInvalidPropertyContainer;
    m_skeletonHandle = vr::k_ulInvalidInputComponentHandle;
    m_haptic = vr::k_ulInvalidPropertyContainer;

    m_pose = { 0 };
    m_pose.deviceIsConnected = false;
    for(size_t i = 0U; i < 3U; i++)
    {
        m_pose.vecAcceleration[i] = .0;
        m_pose.vecAngularAcceleration[i] = .0;
        m_pose.vecAngularVelocity[i] = .0;
        m_pose.vecDriverFromHeadTranslation[i] = .0;
    }
    m_pose.poseTimeOffset = .0;
    m_pose.qDriverFromHeadRotation = { 1.0, .0, .0, .0 };
    m_pose.qRotation = { 1.0, .0, .0, .0 };
    m_pose.qWorldFromDriverRotation = { 1.0, .0, .0, .0 };
    m_pose.result = vr::TrackingResult_Uninitialized;
    m_pose.shouldApplyHeadModel = false;
    m_pose.willDriftInYaw = false;

    for(size_t i = 0U; i < SB_Count; i++)
        m_boneTransform[i] = g_openHandGesture[i];

    if(!m_isLeft)
    {
        // Transformation inversion along 0YZ plane
        for(size_t i = 1U; i < SB_Count; i++)
        {
            m_boneTransform[i].position.v[0] *= -1.f;

            switch(i)
            {
                case SB_Wrist:
                {
                    m_boneTransform[i].orientation.y *= -1.f;
                    m_boneTransform[i].orientation.z *= -1.f;
                } break;
                case SB_Thumb0:
                case SB_IndexFinger0:
                case SB_MiddleFinger0:
                case SB_RingFinger0:
                case SB_PinkyFinger0:
                {
                    m_boneTransform[i].orientation.z *= -1.f;
                    std::swap(m_boneTransform[i].orientation.x, m_boneTransform[i].orientation.w);
                    m_boneTransform[i].orientation.w *= -1.f;
                    std::swap(m_boneTransform[i].orientation.y, m_boneTransform[i].orientation.z);
                } break;
            }
        }
    }
}

CLeapIndexController::~CLeapIndexController()
{
    for(auto l_button : m_buttons)
        delete l_button;
}

// vr::ITrackedDeviceServerDriver
vr::EVRInitError CLeapIndexController::Activate(uint32_t unObjectId)
{
    vr::EVRInitError l_resultError = vr::VRInitError_Driver_Failed;

    if(m_trackedDevice == vr::k_unTrackedDeviceIndexInvalid)
    {
        m_trackedDevice = unObjectId;
        m_propertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer(m_trackedDevice);

        vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_TrackingSystemName_String, "lighthouse");
        vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_SerialNumber_String, m_serialNumber.c_str());
        vr::VRProperties()->SetBoolProperty(m_propertyContainer, vr::Prop_WillDriftInYaw_Bool, false);
        vr::VRProperties()->SetBoolProperty(m_propertyContainer, vr::Prop_DeviceIsWireless_Bool, true);
        vr::VRProperties()->SetBoolProperty(m_propertyContainer, vr::Prop_DeviceIsCharging_Bool, false);
        vr::VRProperties()->SetFloatProperty(m_propertyContainer, vr::Prop_DeviceBatteryPercentage_Float, 1.f); // Always charged

        vr::HmdMatrix34_t l_matrix = { -1.f, 0.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f };
        vr::VRProperties()->SetProperty(m_propertyContainer, vr::Prop_StatusDisplayTransform_Matrix34, &l_matrix, sizeof(vr::HmdMatrix34_t), vr::k_unHmdMatrix34PropertyTag);

        vr::VRProperties()->SetBoolProperty(m_propertyContainer, vr::Prop_Firmware_UpdateAvailable_Bool, false);
        vr::VRProperties()->SetBoolProperty(m_propertyContainer, vr::Prop_Firmware_ManualUpdate_Bool, false);
        vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_Firmware_ManualUpdateURL_String, "https://developer.valvesoftware.com/wiki/SteamVR/HowTo_Update_Firmware");
        vr::VRProperties()->SetBoolProperty(m_propertyContainer, vr::Prop_DeviceProvidesBatteryStatus_Bool, true);
        vr::VRProperties()->SetBoolProperty(m_propertyContainer, vr::Prop_DeviceCanPowerOff_Bool, true);
        vr::VRProperties()->SetInt32Property(m_propertyContainer, vr::Prop_DeviceClass_Int32, vr::TrackedDeviceClass_Controller);
        vr::VRProperties()->SetBoolProperty(m_propertyContainer, vr::Prop_Firmware_ForceUpdateRequired_Bool, false);
        vr::VRProperties()->SetBoolProperty(m_propertyContainer, vr::Prop_Identifiable_Bool, true);
        vr::VRProperties()->SetBoolProperty(m_propertyContainer, vr::Prop_Firmware_RemindUpdate_Bool, false);
        vr::VRProperties()->SetInt32Property(m_propertyContainer, vr::Prop_Axis0Type_Int32, vr::k_eControllerAxis_TrackPad);
        vr::VRProperties()->SetInt32Property(m_propertyContainer, vr::Prop_Axis1Type_Int32, vr::k_eControllerAxis_Trigger);
        vr::VRProperties()->SetInt32Property(m_propertyContainer, vr::Prop_ControllerRoleHint_Int32, m_isLeft ? vr::TrackedControllerRole_LeftHand : vr::TrackedControllerRole_RightHand);
        vr::VRProperties()->SetBoolProperty(m_propertyContainer, vr::Prop_HasDisplayComponent_Bool, false);
        vr::VRProperties()->SetBoolProperty(m_propertyContainer, vr::Prop_HasCameraComponent_Bool, false);
        vr::VRProperties()->SetBoolProperty(m_propertyContainer, vr::Prop_HasDriverDirectModeComponent_Bool, false);
        vr::VRProperties()->SetBoolProperty(m_propertyContainer, vr::Prop_HasVirtualDisplayComponent_Bool, false);
        vr::VRProperties()->SetInt32Property(m_propertyContainer, vr::Prop_ControllerHandSelectionPriority_Int32, 0);
        vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_ModelNumber_String, m_isLeft ? "Knuckles Left" : "Knuckles Right");
        vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_RenderModelName_String, m_isLeft ? "{indexcontroller}valve_controller_knu_1_0_left" : "{indexcontroller}valve_controller_knu_1_0_right");
        vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_ManufacturerName_String, "Valve");
        vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_TrackingFirmwareVersion_String, "1562916277 watchman@ValveBuilder02 2019-07-12 FPGA 538(2.26/10/2) BL 0 VRC 1562916277 Radio 1562882729");
        vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_HardwareRevision_String, "product 17 rev 14.1.9 lot 2019/4/20 0");
        vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_ConnectedWirelessDongle_String, "C2F75F5986-DIY"); // Changed
        vr::VRProperties()->SetUint64Property(m_propertyContainer, vr::Prop_HardwareRevision_Uint64, 286130441U);
        vr::VRProperties()->SetUint64Property(m_propertyContainer, vr::Prop_FirmwareVersion_Uint64, 1562916277U);
        vr::VRProperties()->SetUint64Property(m_propertyContainer, vr::Prop_FPGAVersion_Uint64, 538U);
        vr::VRProperties()->SetUint64Property(m_propertyContainer, vr::Prop_VRCVersion_Uint64, 1562916277U);
        vr::VRProperties()->SetUint64Property(m_propertyContainer, vr::Prop_RadioVersion_Uint64, 1562882729U);
        vr::VRProperties()->SetUint64Property(m_propertyContainer, vr::Prop_DongleVersion_Uint64, 1558748372U);
        vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_Firmware_ProgrammingTarget_String, m_isLeft ? "LHR-E217CD00" : "LHR-E217CD01"); // Changed
        vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_ResourceRoot_String, "indexcontroller");
        vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_RegisteredDeviceType_String, m_isLeft ? "valve/index_controllerLHR-E217CD00" : "valve/index_controllerLHR-E217CD01"); // Changed
        vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_InputProfilePath_String, "{indexcontroller}/input/index_controller_profile.json");
        vr::VRProperties()->SetInt32Property(m_propertyContainer, vr::Prop_Axis2Type_Int32, vr::k_eControllerAxis_Trigger);
        vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceOff_String, m_isLeft ? "{indexcontroller}/icons/left_controller_status_off.png" : "{indexcontroller}/icons/right_controller_status_off.png");
        vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceSearching_String, m_isLeft ? "{indexcontroller}/icons/left_controller_status_searching.gif" : "{indexcontroller}/icons/right_controller_status_searching.gif");
        vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceSearchingAlert_String, m_isLeft ? "{indexcontroller}/icons/left_controller_status_searching_alert.gif" : "{indexcontroller}/icons//right_controller_status_searching_alert.gif");
        vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceReady_String, m_isLeft ? "{indexcontroller}/icons/left_controller_status_ready.png" : "{indexcontroller}/icons//right_controller_status_ready.png");
        vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceReadyAlert_String, m_isLeft ? "{indexcontroller}/icons/left_controller_status_ready_alert.png" : "{indexcontroller}/icons//right_controller_status_ready_alert.png");
        vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceNotReady_String, m_isLeft ? "{indexcontroller}/icons/left_controller_status_error.png" : "{indexcontroller}/icons//right_controller_status_error.png");
        vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceStandby_String, m_isLeft ? "{indexcontroller}/icons/left_controller_status_off.png" : "{indexcontroller}/icons//right_controller_status_off.png");
        vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceAlertLow_String, m_isLeft ? "{indexcontroller}/icons/left_controller_status_ready_low.png" : "{indexcontroller}/icons//right_controller_status_ready_low.png");
        vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_ControllerType_String, "knuckles");

        for(size_t i = 0U; i < IB_Count; i++)
            m_buttons.push_back(new CControllerButton());

        vr::VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/system/click", &m_buttons[IB_SystemClick]->GetHandleRef());
        m_buttons[IB_SystemClick]->SetInputType(CControllerButton::IT_Boolean);

        vr::VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/system/touch", &m_buttons[IB_SystemTouch]->GetHandleRef());
        m_buttons[IB_SystemTouch]->SetInputType(CControllerButton::IT_Boolean);

        vr::VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/trigger/click", &m_buttons[IB_TriggerClick]->GetHandleRef());
        m_buttons[IB_TriggerClick]->SetInputType(CControllerButton::IT_Boolean);

        vr::VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/trigger/touch", &m_buttons[IB_TriggerTouch]->GetHandleRef());
        m_buttons[IB_TriggerClick]->SetInputType(CControllerButton::IT_Boolean);

        vr::VRDriverInput()->CreateScalarComponent(m_propertyContainer, "/input/trigger/value", &m_buttons[IB_TriggerValue]->GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
        m_buttons[IB_TriggerValue]->SetInputType(CControllerButton::IT_Float);

        vr::VRDriverInput()->CreateScalarComponent(m_propertyContainer, "/input/trackpad/x", &m_buttons[IB_TrackpadX]->GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
        m_buttons[IB_TrackpadX]->SetInputType(CControllerButton::IT_Float);

        vr::VRDriverInput()->CreateScalarComponent(m_propertyContainer, "/input/trackpad/y", &m_buttons[IB_TrackpadY]->GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
        m_buttons[IB_TrackpadY]->SetInputType(CControllerButton::IT_Float);

        vr::VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/trackpad/touch", &m_buttons[IB_TrackpadTouch]->GetHandleRef());
        m_buttons[IB_TrackpadTouch]->SetInputType(CControllerButton::IT_Boolean);

        vr::VRDriverInput()->CreateScalarComponent(m_propertyContainer, "/input/trackpad/force", &m_buttons[IB_TrackpadForce]->GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
        m_buttons[IB_TrackpadForce]->SetInputType(CControllerButton::IT_Float);

        vr::VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/grip/touch", &m_buttons[IB_GripTouch]->GetHandleRef());
        m_buttons[IB_GripTouch]->SetInputType(CControllerButton::IT_Boolean);

        vr::VRDriverInput()->CreateScalarComponent(m_propertyContainer, "/input/grip/force", &m_buttons[IB_GripForce]->GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
        m_buttons[IB_GripForce]->SetInputType(CControllerButton::IT_Float);

        vr::VRDriverInput()->CreateScalarComponent(m_propertyContainer, "/input/grip/value", &m_buttons[IB_GripValue]->GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
        m_buttons[IB_GripValue]->SetInputType(CControllerButton::IT_Float);

        vr::VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/thumbstick/click", &m_buttons[IB_ThumbstickClick]->GetHandleRef());
        m_buttons[IB_ThumbstickClick]->SetInputType(CControllerButton::IT_Boolean);

        vr::VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/thumbstick/touch", &m_buttons[IB_ThumbstickTouch]->GetHandleRef());
        m_buttons[IB_ThumbstickTouch]->SetInputType(CControllerButton::IT_Boolean);

        vr::VRDriverInput()->CreateScalarComponent(m_propertyContainer, "/input/thumbstick/x", &m_buttons[IB_ThumbstickX]->GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
        m_buttons[IB_ThumbstickX]->SetInputType(CControllerButton::IT_Float);

        vr::VRDriverInput()->CreateScalarComponent(m_propertyContainer, "/input/thumbstick/y", &m_buttons[IB_ThumbstickY]->GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
        m_buttons[IB_ThumbstickY]->SetInputType(CControllerButton::IT_Float);

        vr::VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/a/click", &m_buttons[IB_AClick]->GetHandleRef());
        m_buttons[IB_AClick]->SetInputType(CControllerButton::IT_Boolean);

        vr::VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/a/touch", &m_buttons[IB_ATouch]->GetHandleRef());
        m_buttons[IB_ATouch]->SetInputType(CControllerButton::IT_Boolean);

        vr::VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/b/click", &m_buttons[IB_BClick]->GetHandleRef());
        m_buttons[IB_BClick]->SetInputType(CControllerButton::IT_Boolean);

        vr::VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/b/touch", &m_buttons[IB_BTouch]->GetHandleRef());
        m_buttons[IB_BTouch]->SetInputType(CControllerButton::IT_Boolean);

        vr::VRDriverInput()->CreateScalarComponent(m_propertyContainer, "/input/finger/index", &m_buttons[IB_FingerIndex]->GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
        m_buttons[IB_FingerIndex]->SetInputType(CControllerButton::IT_Float);

        vr::VRDriverInput()->CreateScalarComponent(m_propertyContainer, "/input/finger/middle", &m_buttons[IB_FingerMiddle]->GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
        m_buttons[IB_FingerMiddle]->SetInputType(CControllerButton::IT_Float);

        vr::VRDriverInput()->CreateScalarComponent(m_propertyContainer, "/input/finger/ring", &m_buttons[IB_FingerRing]->GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
        m_buttons[IB_FingerRing]->SetInputType(CControllerButton::IT_Float);

        vr::VRDriverInput()->CreateScalarComponent(m_propertyContainer, "/input/finger/pinky", &m_buttons[IB_FingerPinky]->GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
        m_buttons[IB_FingerPinky]->SetInputType(CControllerButton::IT_Float);

        const vr::EVRSkeletalTrackingLevel l_trackingLevel = ((CDriverConfig::GetTrackingLevel() == CDriverConfig::TL_Partial) ? vr::VRSkeletalTracking_Partial : vr::VRSkeletalTracking_Full);
        if(m_isLeft)
            vr::VRDriverInput()->CreateSkeletonComponent(m_propertyContainer, "/input/skeleton/left", "/skeleton/hand/left", "/pose/raw", l_trackingLevel, nullptr, 0U, &m_skeletonHandle);
        else
            vr::VRDriverInput()->CreateSkeletonComponent(m_propertyContainer, "/input/skeleton/right", "/skeleton/hand/right", "/pose/raw", l_trackingLevel, nullptr, 0U, &m_skeletonHandle);

        vr::VRDriverInput()->CreateHapticComponent(m_propertyContainer, "/output/haptic", &m_haptic);

        l_resultError = vr::VRInitError_None;
    }

    return l_resultError;
}

void CLeapIndexController::Deactivate()
{
    ResetControls();
    m_trackedDevice = vr::k_unTrackedDeviceIndexInvalid;
}

void CLeapIndexController::EnterStandby()
{
}

void* CLeapIndexController::GetComponent(const char* pchComponentNameAndVersion)
{
    void *l_result = nullptr;
    if(!strcmp(pchComponentNameAndVersion, vr::ITrackedDeviceServerDriver_Version)) l_result = dynamic_cast<vr::ITrackedDeviceServerDriver*>(this);
    return l_result;
}

void CLeapIndexController::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
{
    if(m_trackedDevice != vr::k_unTrackedDeviceIndexInvalid)
        ProcessExternalInput(pchRequest);
}

vr::DriverPose_t CLeapIndexController::GetPose()
{
    return m_pose;
}

const std::string& CLeapIndexController::GetSerialNumber() const
{
    return m_serialNumber;
}

void CLeapIndexController::SetEnabled(bool p_state)
{
    m_pose.deviceIsConnected = p_state;
    if(m_trackedDevice != vr::k_unTrackedDeviceIndexInvalid) vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_trackedDevice, m_pose, sizeof(vr::DriverPose_t));
}

void CLeapIndexController::ResetControls()
{
    for(auto l_button : m_buttons)
    {
        l_button->SetValue(0.f);
        l_button->SetState(false);
    }
}

void CLeapIndexController::RunFrame(const CLeapHand *p_hand)
{
    if(m_trackedDevice != vr::k_unTrackedDeviceIndexInvalid)
    {
        if(m_pose.deviceIsConnected)
        {
            UpdatePose(p_hand);
            UpdateSkeletalInput(p_hand);
            UpdateInput(p_hand);
        }
        else vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_trackedDevice, m_pose, sizeof(vr::DriverPose_t));
    }
}

void CLeapIndexController::UpdatePose(const CLeapHand *p_hand)
{
    m_pose.poseIsValid = (p_hand && p_hand->IsVisible());

    if(m_pose.poseIsValid)
    {
        std::memcpy(m_pose.vecWorldFromDriverTranslation, ms_headPosition, sizeof(double) * 3U);
        std::memcpy(&m_pose.qWorldFromDriverRotation, &ms_headRotation, sizeof(vr::HmdQuaternion_t));

        // Root represents Leap Motion mount point on HMD
        // Root offset
        glm::quat l_headRot(ms_headRotation.w, ms_headRotation.x, ms_headRotation.y, ms_headRotation.z);
        glm::vec3 l_globalOffset = l_headRot * CDriverConfig::GetRootOffset();

        m_pose.vecWorldFromDriverTranslation[0] += l_globalOffset.x;
        m_pose.vecWorldFromDriverTranslation[1] += l_globalOffset.y;
        m_pose.vecWorldFromDriverTranslation[2] += l_globalOffset.z;

        // Root angle
        glm::quat l_globalRot = l_headRot * glm::quat(glm::radians(CDriverConfig::GetRootAngle()));
        ConvertQuaternion(l_globalRot, m_pose.qWorldFromDriverRotation);

        // Velocity
        if(CDriverConfig::IsVelocityUsed())
        {
            glm::vec3 l_resultVelocity = l_globalRot * p_hand->GetVelocity();
            m_pose.vecVelocity[0] = l_resultVelocity.x;
            m_pose.vecVelocity[1] = l_resultVelocity.y;
            m_pose.vecVelocity[2] = l_resultVelocity.z;
        }
        else
        {
            m_pose.vecVelocity[0] = 0.f;
            m_pose.vecVelocity[1] = 0.f;
            m_pose.vecVelocity[2] = 0.f;
        }

        // Local transformation of device
        // Rotation
        glm::quat l_poseRotation = p_hand->GetRotation() * (m_isLeft ? g_skeletonOffsetLeft : g_skeletonOffsetRight);
        glm::mat4 l_mat = glm::translate(g_identityMatrix, p_hand->GetPosition()) * glm::toMat4(l_poseRotation);
        l_mat *= (m_isLeft ? g_wristOffsetLeft : g_wristOffsetRight);
        l_poseRotation = glm::toQuat(l_mat);
        m_pose.qRotation.x = l_poseRotation.x;
        m_pose.qRotation.y = l_poseRotation.y;
        m_pose.qRotation.z = l_poseRotation.z;
        m_pose.qRotation.w = l_poseRotation.w;

        // Positon
        glm::vec4 l_posePosition = l_mat * g_pointVec4;
        m_pose.vecPosition[0] = l_posePosition.x;
        m_pose.vecPosition[1] = l_posePosition.y;
        m_pose.vecPosition[2] = l_posePosition.z;

        m_pose.result = vr::TrackingResult_Running_OK;
    }
    else
    {
        for(size_t i = 0U; i < 3U; i++)
            m_pose.vecVelocity[i] = .0;

        if(CDriverConfig::IsHandsResetEnabled()) m_pose.result = vr::TrackingResult_Running_OutOfRange;
        else
        {
            m_pose.result = vr::TrackingResult_Running_OK;
            m_pose.poseIsValid = true;
        }
    }

    vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_trackedDevice, m_pose, sizeof(vr::DriverPose_t));
}

void CLeapIndexController::UpdateInput(const CLeapHand *p_hand)
{
    if (!CDriverConfig::IsControllerInputUsed())
    {
        float l_trigger = p_hand->GetFingerBend(CLeapHand::Finger::Index);
        m_buttons[IB_TriggerValue]->SetValue(l_trigger);
        m_buttons[IB_TriggerTouch]->SetState(l_trigger >= 0.5f);
        m_buttons[IB_TriggerClick]->SetState(l_trigger >= 0.75f);

        float l_grabValue = p_hand->GetGrabValue();
        m_buttons[IB_GripValue]->SetValue(l_grabValue);
        m_buttons[IB_GripTouch]->SetState(l_grabValue >= 0.75f);
        m_buttons[IB_GripForce]->SetValue((l_grabValue >= 0.9f) ? (l_grabValue - 0.9f) * 10.f : 0.f);
    }

    m_buttons[IB_FingerIndex]->SetValue(p_hand->GetFingerBend(CLeapHand::Finger::Index));
    m_buttons[IB_FingerMiddle]->SetValue(p_hand->GetFingerBend(CLeapHand::Finger::Middle));
    m_buttons[IB_FingerRing]->SetValue(p_hand->GetFingerBend(CLeapHand::Finger::Ring));
    m_buttons[IB_FingerPinky]->SetValue(p_hand->GetFingerBend(CLeapHand::Finger::Pinky));

    for(auto l_button : m_buttons)
    {
        if(l_button->IsUpdated())
        {
            switch(l_button->GetInputType())
            {
                case CControllerButton::IT_Boolean:
                    vr::VRDriverInput()->UpdateBooleanComponent(l_button->GetHandle(), l_button->GetState(), .0);
                    break;
                case CControllerButton::IT_Float:
                    vr::VRDriverInput()->UpdateScalarComponent(l_button->GetHandle(), l_button->GetValue(), .0);
                    break;
            }
            l_button->ResetUpdate();
        }
    }
}

void CLeapIndexController::UpdateSkeletalInput(const CLeapHand *p_hand)
{
    // Skeletal structure update
    for(size_t i = 0U; i < 5U; i++)
    {
        size_t l_indexFinger = GetFingerBoneIndex(i);
        for(size_t j = 0U; j < ((i == 0U) ? 3U : 4U); j++)
        {
            glm::quat l_rot;
            p_hand->GetFingerBoneLocalRotation(i, (i == 0U) ? (j + 1U) : j, l_rot);
            ChangeBoneOrientation(l_rot);

            if(j == 0U) // Metacarpal
            {
                if(i == 0U)
                    FixThumbBone(l_rot);
                else
                    FixMetacarpalBone(l_rot);
            }

            ConvertQuaternion(l_rot, m_boneTransform[l_indexFinger + j].orientation);
        }
    }

    // Update aux bones
    glm::vec3 l_position;
    glm::quat l_rotation;
    ConvertVector3(m_boneTransform[SB_Wrist].position, l_position);
    ConvertQuaternion(m_boneTransform[SB_Wrist].orientation, l_rotation);
    const glm::mat4 l_wristMat = glm::translate(g_identityMatrix, l_position) * glm::mat4_cast(l_rotation);
    for(size_t i = HF_Thumb; i < HF_Count; i++)
    {
        glm::mat4 l_chainMat(l_wristMat);
        const size_t l_chainIndex = GetFingerBoneIndex(i);
        for(size_t j = 0U; j < ((i == HF_Thumb) ? 3U : 4U); j++)
        {
            ConvertVector3(m_boneTransform[l_chainIndex + j].position, l_position);
            ConvertQuaternion(m_boneTransform[l_chainIndex + j].orientation, l_rotation);
            l_chainMat = l_chainMat * (glm::translate(g_identityMatrix, l_position)*glm::mat4_cast(l_rotation));
        }
        l_position = l_chainMat * g_pointVec4;
        l_rotation = glm::quat_cast(l_chainMat);
        if(m_isLeft)
            ChangeAuxTransformation(l_position, l_rotation);

        ConvertVector3(l_position, m_boneTransform[SB_Aux_Thumb + i].position);
        ConvertQuaternion(l_rotation, m_boneTransform[SB_Aux_Thumb + i].orientation);
    }

    vr::VRDriverInput()->UpdateSkeletonComponent(m_skeletonHandle, vr::VRSkeletalMotionRange_WithController, m_boneTransform, SB_Count);
    vr::VRDriverInput()->UpdateSkeletonComponent(m_skeletonHandle, vr::VRSkeletalMotionRange_WithoutController, m_boneTransform, SB_Count);
}

void CLeapIndexController::SetButtonState(size_t p_button, bool p_state)
{
    if(p_button < m_buttons.size())
        m_buttons[p_button]->SetState(p_state);
}
void CLeapIndexController::SetButtonValue(size_t p_button, float p_value)
{
    if(p_button < m_buttons.size())
        m_buttons[p_button]->SetValue(p_value);
}

void CLeapIndexController::ChangeBoneOrientation(glm::quat &p_rot) const
{
    std::swap(p_rot.x, p_rot.z);
    p_rot.z *= -1.f;
    if(m_isLeft)
    {
        p_rot.x *= -1.f;
        p_rot.y *= -1.f;
    }
}

void CLeapIndexController::FixThumbBone(glm::quat & p_rot) const
{
    p_rot = g_thumbOffset * p_rot;
    if(m_isLeft)
        p_rot = g_mirroringOffset * p_rot;
}

void CLeapIndexController::FixMetacarpalBone(glm::quat & p_rot) const
{
    p_rot = g_metacarpalOffset * p_rot;
    if(m_isLeft)
        p_rot = g_mirroringOffset * p_rot;
}

void CLeapIndexController::ProcessExternalInput(const char * p_message)
{
    // Message format for buttons: button {name} {state}
    // Message format for axes: axis {name} {state} {x} {y} {pressure}
    // This method is ugly
    std::vector<std::string> l_chunks;
    SplitString(p_message, ' ', l_chunks);

    if(l_chunks.size() >= 3U)
    {
        size_t l_type = 0U;
        size_t l_input = 0U;
        size_t l_state = 0U;
        if(ReadEnumVector(l_chunks[0U], g_typeNames, l_type) && ReadEnumVector(l_chunks[1U], g_inputNames, l_input) && ReadEnumVector(l_chunks[2U], g_stateNames, l_state))
        {
            switch(l_type)
            {
                // Buttons
                case TypeName::TN_Button:
                {
                    switch(l_input)
                    {
                        case InputName::IN_A:
                        {
                            switch(l_state)
                            {
                                case StateName::ST_None:
                                {
                                    m_buttons[IB_ATouch]->SetState(false);
                                    m_buttons[IB_AClick]->SetState(false);
                                } break;
                                case StateName::ST_Touched:
                                {
                                    m_buttons[IB_ATouch]->SetState(true);
                                    m_buttons[IB_AClick]->SetState(false);
                                } break;
                                case StateName::ST_Clicked:
                                {
                                    m_buttons[IB_ATouch]->SetState(true);
                                    m_buttons[IB_AClick]->SetState(true);
                                } break;
                            }
                        } break;

                        case InputName::IN_B:
                        {
                            switch(l_state)
                            {
                                case StateName::ST_None:
                                {
                                    m_buttons[IB_BTouch]->SetState(false);
                                    m_buttons[IB_BClick]->SetState(false);
                                } break;
                                case StateName::ST_Touched:
                                {
                                    m_buttons[IB_BTouch]->SetState(true);
                                    m_buttons[IB_BClick]->SetState(false);
                                } break;
                                case StateName::ST_Clicked:
                                {
                                    m_buttons[IB_BTouch]->SetState(true);
                                    m_buttons[IB_BClick]->SetState(true);
                                } break;
                            }
                        } break;

                        case InputName::IN_System:
                        {
                            switch(l_state)
                            {
                                case StateName::ST_None:
                                {
                                    m_buttons[IB_SystemTouch]->SetState(false);
                                    m_buttons[IB_SystemClick]->SetState(false);
                                } break;
                                case StateName::ST_Touched:
                                {
                                    m_buttons[IB_SystemTouch]->SetState(true);
                                    m_buttons[IB_SystemClick]->SetState(false);
                                } break;
                                case StateName::ST_Clicked:
                                {
                                    m_buttons[IB_SystemTouch]->SetState(true);
                                    m_buttons[IB_SystemClick]->SetState(true);
                                } break;
                            }
                        } break;
                    }
                } break;

                // Axes
                case TypeName::TN_Axis:
                {
                    if(l_chunks.size() >= 6U)
                    {
                        glm::vec3 l_values(0.f);
                        if(TryParse(l_chunks[3U], l_values.x) && TryParse(l_chunks[4U], l_values.y) && TryParse(l_chunks[5U], l_values.z))
                        {
                            switch(l_input)
                            {
                                case InputName::IN_Thumbstick:
                                {
                                    switch(l_state)
                                    {
                                        case StateName::ST_None:
                                        {
                                            m_buttons[IB_ThumbstickTouch]->SetState(false);
                                            m_buttons[IB_ThumbstickClick]->SetState(false);
                                            m_buttons[IB_ThumbstickX]->SetValue(0.f);
                                            m_buttons[IB_ThumbstickY]->SetValue(0.f);
                                        } break;
                                        case StateName::ST_Touched:
                                        {
                                            m_buttons[IB_ThumbstickTouch]->SetState(true);
                                            m_buttons[IB_ThumbstickClick]->SetState(false);
                                            m_buttons[IB_ThumbstickX]->SetValue(l_values.x);
                                            m_buttons[IB_ThumbstickY]->SetValue(l_values.y);
                                        } break;
                                        case StateName::ST_Clicked:
                                        {
                                            m_buttons[IB_ThumbstickTouch]->SetState(true);
                                            m_buttons[IB_ThumbstickClick]->SetState(true);
                                            m_buttons[IB_ThumbstickX]->SetValue(l_values.x);
                                            m_buttons[IB_ThumbstickY]->SetValue(l_values.y);
                                        } break;
                                    }
                                } break;

                                case InputName::IN_Touchpad:
                                {
                                    switch(l_state)
                                    {
                                        case StateName::ST_None:
                                        {
                                            m_buttons[IB_TrackpadTouch]->SetState(false);
                                            m_buttons[IB_TrackpadForce]->SetValue(0.f);
                                            m_buttons[IB_TrackpadX]->SetValue(0.f);
                                            m_buttons[IB_TrackpadY]->SetValue(0.f);
                                        } break;
                                        case StateName::ST_Touched:
                                        {
                                            m_buttons[IB_TrackpadTouch]->SetState(true);
                                            m_buttons[IB_TrackpadForce]->SetValue(0.f);
                                            m_buttons[IB_TrackpadX]->SetValue(l_values.x);
                                            m_buttons[IB_TrackpadY]->SetValue(l_values.y);
                                        } break;
                                        case StateName::ST_Clicked:
                                        {
                                            m_buttons[IB_TrackpadTouch]->SetState(true);
                                            m_buttons[IB_TrackpadForce]->SetValue(NormalizeRange(l_values.z, 0.75f, 1.f));
                                            m_buttons[IB_TrackpadX]->SetValue(l_values.x);
                                            m_buttons[IB_TrackpadY]->SetValue(l_values.y);
                                        } break;
                                    }
                                } break;
                            }
                        }
                    }
                }
            }
        }
    }
}

void CLeapIndexController::ChangeAuxTransformation(glm::vec3 &p_pos, glm::quat &p_rot)
{
    p_pos.y *= -1.f;
    p_pos.z *= -1.f;

    std::swap(p_rot.x, p_rot.w);
    p_rot.w *= -1.f;
    std::swap(p_rot.y, p_rot.z);
    p_rot.y *= -1.f;
}

size_t CLeapIndexController::GetFingerBoneIndex(size_t p_id)
{
    size_t l_result = 0U;
    switch(p_id)
    {
        case HF_Thumb:
            l_result = SB_Thumb0;
            break;
        case HF_Index:
            l_result = SB_IndexFinger0;
            break;
        case HF_Middle:
            l_result = SB_MiddleFinger0;
            break;
        case HF_Ring:
            l_result = SB_RingFinger0;
            break;
        case HF_Pinky:
            l_result = SB_PinkyFinger0;
            break;
    }
    return l_result;
}

void CLeapIndexController::UpdateHMDCoordinates()
{
    vr::TrackedDevicePose_t l_hmdPose;
    vr::VRServerDriverHost()->GetRawTrackedDevicePoses(0.f, &l_hmdPose, 1U); // HMD has device ID 0
    if(l_hmdPose.bPoseIsValid)
    {
        glm::mat4 l_rotMat(1.f);
        ConvertMatrix(l_hmdPose.mDeviceToAbsoluteTracking, l_rotMat);

        const glm::quat l_headRot = glm::quat_cast(l_rotMat);
        ConvertQuaternion(l_headRot, ms_headRotation);

        for(size_t i = 0U; i < 3U; i++)
            ms_headPosition[i] = l_hmdPose.mDeviceToAbsoluteTracking.m[i][3];
    }
}
