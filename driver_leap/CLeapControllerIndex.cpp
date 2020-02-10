#include "stdafx.h"

#include "CLeapControllerIndex.h"

#include "CControllerButton.h"
#include "CDriverConfig.h"
#include "CGestureMatcher.h"
#include "Utils.h"

extern const glm::vec3 g_axisX;
extern const glm::vec3 g_axisY;
extern const glm::mat4 g_identityMatrix;
extern const vr::VRBoneTransform_t g_openHandGesture[];
extern const glm::vec4 g_zeroPoint;

enum IndexButton : size_t
{
    IB_SysClick = 0U,
    IB_SysTouch,
    IB_TriggerClick,
    IB_TriggerValue,
    IB_TrackpadX,
    IB_TrackpadY,
    IB_TrackpadTouch,
    IB_TrackpadForce,
    IB_GripTouch,
    IB_GripForce,
    IB_GripValue,
    IB_ThumbstickClick,
    IB_ThumbstickTouch,
    IB_ThumbstickX,
    IB_ThumbstickY,
    IB_AClick,
    IB_ATouch,
    IB_BClick,
    IB_BTouch,
    IB_FingerIndex,
    IB_FingerMiddle,
    IB_FingerRing,
    IB_FingerPinky,

    IB_Count
};

CLeapControllerIndex::CLeapControllerIndex(unsigned char f_hand)
{
    m_hand = f_hand % CH_Count;
    m_serialNumber.assign((m_hand == CH_Left) ? "LHR-E217CD00" : "LHR-E217CD01");

    for(size_t i = 0U; i < HSB_Count; i++) m_boneTransform[i] = g_openHandGesture[i];
    m_skeletonHandle = vr::k_ulInvalidInputComponentHandle;

    if(m_hand == CH_Right)
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

}
CLeapControllerIndex::~CLeapControllerIndex()
{
}

vr::EVRInitError CLeapControllerIndex::Activate(uint32_t unObjectId)
{
    vr::EVRInitError l_resultError = vr::VRInitError_Driver_Failed;

    if(m_trackedDevice == vr::k_unTrackedDeviceIndexInvalid)
    {
        m_trackedDevice = unObjectId;
        m_propertyContainer = ms_propertyHelpers->TrackedDeviceToPropertyContainer(m_trackedDevice);

        // Properties
        ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_TrackingSystemName_String, "lighthouse");
        ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_SerialNumber_String, m_serialNumber.c_str());
        ms_propertyHelpers->SetBoolProperty(m_propertyContainer, vr::Prop_WillDriftInYaw_Bool, false);
        ms_propertyHelpers->SetBoolProperty(m_propertyContainer, vr::Prop_DeviceIsWireless_Bool, true);
        ms_propertyHelpers->SetBoolProperty(m_propertyContainer, vr::Prop_DeviceIsCharging_Bool, false);
        ms_propertyHelpers->SetFloatProperty(m_propertyContainer, vr::Prop_DeviceBatteryPercentage_Float, 1.f); // Always charged

        vr::HmdMatrix34_t l_matrix = { -1.f, 0.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f };
        ms_propertyHelpers->SetProperty(m_propertyContainer, vr::Prop_StatusDisplayTransform_Matrix34, &l_matrix, sizeof(vr::HmdMatrix34_t), vr::k_unHmdMatrix34PropertyTag);

        ms_propertyHelpers->SetBoolProperty(m_propertyContainer, vr::Prop_Firmware_UpdateAvailable_Bool, false);
        ms_propertyHelpers->SetBoolProperty(m_propertyContainer, vr::Prop_Firmware_ManualUpdate_Bool, false);
        ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_Firmware_ManualUpdateURL_String, "https://developer.valvesoftware.com/wiki/SteamVR/HowTo_Update_Firmware");
        ms_propertyHelpers->SetBoolProperty(m_propertyContainer, vr::Prop_DeviceProvidesBatteryStatus_Bool, true);
        ms_propertyHelpers->SetBoolProperty(m_propertyContainer, vr::Prop_DeviceCanPowerOff_Bool, true);
        ms_propertyHelpers->SetInt32Property(m_propertyContainer, vr::Prop_DeviceClass_Int32, vr::TrackedDeviceClass_Controller);
        ms_propertyHelpers->SetBoolProperty(m_propertyContainer, vr::Prop_Firmware_ForceUpdateRequired_Bool, false);
        ms_propertyHelpers->SetBoolProperty(m_propertyContainer, vr::Prop_Identifiable_Bool, true);
        ms_propertyHelpers->SetBoolProperty(m_propertyContainer, vr::Prop_Firmware_RemindUpdate_Bool, false);
        ms_propertyHelpers->SetInt32Property(m_propertyContainer, vr::Prop_Axis0Type_Int32, vr::k_eControllerAxis_TrackPad);
        ms_propertyHelpers->SetInt32Property(m_propertyContainer, vr::Prop_Axis1Type_Int32, vr::k_eControllerAxis_Trigger);
        ms_propertyHelpers->SetInt32Property(m_propertyContainer, vr::Prop_ControllerRoleHint_Int32, (m_hand == CH_Left) ? vr::TrackedControllerRole_LeftHand : vr::TrackedControllerRole_RightHand);
        ms_propertyHelpers->SetBoolProperty(m_propertyContainer, vr::Prop_HasDisplayComponent_Bool, false);
        ms_propertyHelpers->SetBoolProperty(m_propertyContainer, vr::Prop_HasCameraComponent_Bool, false);
        ms_propertyHelpers->SetBoolProperty(m_propertyContainer, vr::Prop_HasDriverDirectModeComponent_Bool, false);
        ms_propertyHelpers->SetBoolProperty(m_propertyContainer, vr::Prop_HasVirtualDisplayComponent_Bool, false);
        ms_propertyHelpers->SetInt32Property(m_propertyContainer, vr::Prop_ControllerHandSelectionPriority_Int32, 0);
        ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_ModelNumber_String, (m_hand == CH_Left) ? "Knuckles Left" : "Knuckles Right");
        ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_RenderModelName_String, (m_hand == CH_Left) ? "{indexcontroller}valve_controller_knu_1_0_left" : "{indexcontroller}valve_controller_knu_1_0_right");
        ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_ManufacturerName_String, "Valve");
        ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_TrackingFirmwareVersion_String, "1562916277 watchman@ValveBuilder02 2019-07-12 FPGA 538(2.26/10/2) BL 0 VRC 1562916277 Radio 1562882729");
        ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_HardwareRevision_String, "product 17 rev 14.1.9 lot 2019/4/20 0");
        ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_ConnectedWirelessDongle_String, "C2F75F5986-FYI"); // Changed
        ms_propertyHelpers->SetUint64Property(m_propertyContainer, vr::Prop_HardwareRevision_Uint64, 286130441U);
        ms_propertyHelpers->SetUint64Property(m_propertyContainer, vr::Prop_FirmwareVersion_Uint64, 1562916277U);
        ms_propertyHelpers->SetUint64Property(m_propertyContainer, vr::Prop_FPGAVersion_Uint64, 538U);
        ms_propertyHelpers->SetUint64Property(m_propertyContainer, vr::Prop_VRCVersion_Uint64, 1562916277U);
        ms_propertyHelpers->SetUint64Property(m_propertyContainer, vr::Prop_RadioVersion_Uint64, 1562882729U);
        ms_propertyHelpers->SetUint64Property(m_propertyContainer, vr::Prop_DongleVersion_Uint64, 1558748372U);
        ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_Firmware_ProgrammingTarget_String, (m_hand == CH_Left) ? "LHR-E217CD00" : "LHR-E217CD01"); // Changed
        ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_ResourceRoot_String, "indexcontroller");
        ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_RegisteredDeviceType_String, (m_hand == CH_Left) ? "valve/index_controllerLHR-E217CD00" : "valve/index_controllerLHR-E217CD01"); // Changed
        ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_InputProfilePath_String, "{indexcontroller}/input/index_controller_profile.json");
        ms_propertyHelpers->SetInt32Property(m_propertyContainer, vr::Prop_Axis2Type_Int32, vr::k_eControllerAxis_Trigger);
        ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceOff_String, (m_hand == CH_Left) ? "{indexcontroller}/icons/left_controller_status_off.png" : "{indexcontroller}/icons/right_controller_status_off.png");
        ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceSearching_String, (m_hand == CH_Left) ? "{indexcontroller}/icons/left_controller_status_searching.gif" : "{indexcontroller}/icons/right_controller_status_searching.gif");
        ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceSearchingAlert_String, (m_hand == CH_Left) ? "{indexcontroller}/icons/left_controller_status_searching_alert.gif" : "{indexcontroller}/icons//right_controller_status_searching_alert.gif");
        ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceReady_String, (m_hand == CH_Left) ? "{indexcontroller}/icons/left_controller_status_ready.png" : "{indexcontroller}/icons//right_controller_status_ready.png");
        ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceReadyAlert_String, (m_hand == CH_Left) ? "{indexcontroller}/icons/left_controller_status_ready_alert.png" : "{indexcontroller}/icons//right_controller_status_ready_alert.png");
        ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceNotReady_String, (m_hand == CH_Left) ? "{indexcontroller}/icons/left_controller_status_error.png" : "{indexcontroller}/icons//right_controller_status_error.png");
        ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceStandby_String, (m_hand == CH_Left) ? "{indexcontroller}/icons/left_controller_status_off.png" : "{indexcontroller}/icons//right_controller_status_off.png");
        ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceAlertLow_String, (m_hand == CH_Left) ? "{indexcontroller}/icons/left_controller_status_ready_low.png" : "{indexcontroller}/icons//right_controller_status_ready_low.png");
        ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_ControllerType_String, "knuckles");

        // Input
        if(m_buttons.empty())
        {
            for(size_t i = 0U; i < IB_Count; i++) m_buttons.push_back(new CControllerButton());
        }

        ms_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/system/click", &m_buttons[IB_SysClick]->GetHandleRef());
        m_buttons[IB_SysClick]->SetInputType(CControllerButton::IT_Boolean);

        ms_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/system/touch", &m_buttons[IB_SysTouch]->GetHandleRef());
        m_buttons[IB_SysTouch]->SetInputType(CControllerButton::IT_Boolean);

        ms_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/trigger/click", &m_buttons[IB_TriggerClick]->GetHandleRef());
        m_buttons[IB_TriggerClick]->SetInputType(CControllerButton::IT_Boolean);

        ms_driverInput->CreateScalarComponent(m_propertyContainer, "/input/trigger/value", &m_buttons[IB_TriggerValue]->GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
        m_buttons[IB_TriggerValue]->SetInputType(CControllerButton::IT_Float);

        ms_driverInput->CreateScalarComponent(m_propertyContainer, "/input/trackpad/x", &m_buttons[IB_TrackpadX]->GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
        m_buttons[IB_TrackpadX]->SetInputType(CControllerButton::IT_Float);

        ms_driverInput->CreateScalarComponent(m_propertyContainer, "/input/trackpad/y", &m_buttons[IB_TrackpadY]->GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
        m_buttons[IB_TrackpadY]->SetInputType(CControllerButton::IT_Float);

        ms_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/trackpad/touch", &m_buttons[IB_TrackpadTouch]->GetHandleRef());
        m_buttons[IB_TrackpadTouch]->SetInputType(CControllerButton::IT_Boolean);

        ms_driverInput->CreateScalarComponent(m_propertyContainer, "/input/trackpad/force", &m_buttons[IB_TrackpadForce]->GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
        m_buttons[IB_TrackpadForce]->SetInputType(CControllerButton::IT_Float);

        ms_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/grip/touch", &m_buttons[IB_GripTouch]->GetHandleRef());
        m_buttons[IB_GripTouch]->SetInputType(CControllerButton::IT_Boolean);

        ms_driverInput->CreateScalarComponent(m_propertyContainer, "/input/grip/force", &m_buttons[IB_GripForce]->GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
        m_buttons[IB_GripForce]->SetInputType(CControllerButton::IT_Float);

        ms_driverInput->CreateScalarComponent(m_propertyContainer, "/input/grip/value", &m_buttons[IB_GripValue]->GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
        m_buttons[IB_GripValue]->SetInputType(CControllerButton::IT_Float);

        ms_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/thumbstick/click", &m_buttons[IB_ThumbstickClick]->GetHandleRef());
        m_buttons[IB_ThumbstickClick]->SetInputType(CControllerButton::IT_Boolean);

        ms_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/thumbstick/touch", &m_buttons[IB_ThumbstickTouch]->GetHandleRef());
        m_buttons[IB_ThumbstickTouch]->SetInputType(CControllerButton::IT_Boolean);

        ms_driverInput->CreateScalarComponent(m_propertyContainer, "/input/thumbstick/x", &m_buttons[IB_ThumbstickX]->GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
        m_buttons[IB_ThumbstickX]->SetInputType(CControllerButton::IT_Float);

        ms_driverInput->CreateScalarComponent(m_propertyContainer, "/input/thumbstick/y", &m_buttons[IB_ThumbstickY]->GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
        m_buttons[IB_ThumbstickY]->SetInputType(CControllerButton::IT_Float);

        ms_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/a/click", &m_buttons[IB_AClick]->GetHandleRef());
        m_buttons[IB_AClick]->SetInputType(CControllerButton::IT_Boolean);

        ms_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/a/touch", &m_buttons[IB_ATouch]->GetHandleRef());
        m_buttons[IB_ATouch]->SetInputType(CControllerButton::IT_Boolean);

        ms_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/b/click", &m_buttons[IB_BClick]->GetHandleRef());
        m_buttons[IB_BClick]->SetInputType(CControllerButton::IT_Boolean);

        ms_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/b/touch", &m_buttons[IB_BTouch]->GetHandleRef());
        m_buttons[IB_BTouch]->SetInputType(CControllerButton::IT_Boolean);

        ms_driverInput->CreateScalarComponent(m_propertyContainer, "/input/finger/index", &m_buttons[IB_FingerIndex]->GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
        m_buttons[IB_FingerIndex]->SetInputType(CControllerButton::IT_Float);

        ms_driverInput->CreateScalarComponent(m_propertyContainer, "/input/finger/middle", &m_buttons[IB_FingerMiddle]->GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
        m_buttons[IB_FingerMiddle]->SetInputType(CControllerButton::IT_Float);

        ms_driverInput->CreateScalarComponent(m_propertyContainer, "/input/finger/ring", &m_buttons[IB_FingerRing]->GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
        m_buttons[IB_FingerRing]->SetInputType(CControllerButton::IT_Float);

        ms_driverInput->CreateScalarComponent(m_propertyContainer, "/input/finger/pinky", &m_buttons[IB_FingerPinky]->GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
        m_buttons[IB_FingerPinky]->SetInputType(CControllerButton::IT_Float);

        vr::EVRSkeletalTrackingLevel l_trackingLevel = ((CDriverConfig::GetTrackingLevel() == CDriverConfig::TL_Partial) ? vr::VRSkeletalTracking_Partial : vr::VRSkeletalTracking_Full);
        switch(m_hand)
        {
            case CH_Left:
                ms_driverInput->CreateSkeletonComponent(m_propertyContainer, "/input/skeleton/left", "/skeleton/hand/left", "/pose/raw", l_trackingLevel, nullptr, 0U, &m_skeletonHandle);
                break;
            case CH_Right:
                ms_driverInput->CreateSkeletonComponent(m_propertyContainer, "/input/skeleton/right", "/skeleton/hand/right", "/pose/raw", l_trackingLevel, nullptr, 0U, &m_skeletonHandle);
                break;
        }

        ms_driverInput->CreateHapticComponent(m_propertyContainer, "/output/haptic", &m_haptic);

        l_resultError = vr::VRInitError_None;
    }

    return l_resultError;
}

bool CLeapControllerIndex::MixHandState(bool f_state)
{
    bool l_result = f_state;
    if((m_gameProfile == GP_VRChat) && !CDriverConfig::IsVRChatHandsResetEnabled()) l_result = true;
    return l_result;
}

void CLeapControllerIndex::UpdateGestures(const Leap::Frame &f_frame)
{
    std::vector<float> l_scores;
    if(CGestureMatcher::GetGestures(f_frame, ((m_hand == CH_Left) ? CGestureMatcher::GH_LeftHand : CGestureMatcher::GH_RightHand), l_scores))
    {
        switch(m_gameProfile)
        {
            case GP_Default:
            {
                if(CDriverConfig::IsInputEnabled())
                {
                    if(CDriverConfig::IsMenuEnabled())
                    {
                        m_buttons[IB_SysClick]->SetState(l_scores[CGestureMatcher::GT_Timeout] >= 0.5f);
                        m_buttons[IB_SysTouch]->SetState(l_scores[CGestureMatcher::GT_Timeout] >= 0.25f);
                    }
                    if(CDriverConfig::IsGripEnabled())
                    {
                        m_buttons[IB_GripTouch]->SetState(l_scores[CGestureMatcher::GT_LowerFist] >= 0.75f);
                        m_buttons[IB_GripValue]->SetValue(l_scores[CGestureMatcher::GT_LowerFist]);
                        m_buttons[IB_GripForce]->SetValue(l_scores[CGestureMatcher::GT_LowerFist] >= 0.75f ? (l_scores[CGestureMatcher::GT_LowerFist] - 0.75f) / 0.25f : 0.f);
                    }
                    if(CDriverConfig::IsTouchpadEnabled())
                    {
                        if(CDriverConfig::IsTouchpadTouchEnabled())
                        {
                            if(l_scores[CGestureMatcher::GT_Thumbpress] >= 0.5f)
                            {
                                m_buttons[IB_TrackpadTouch]->SetState(true);
                                m_buttons[IB_TrackpadForce]->SetValue(l_scores[CGestureMatcher::GT_Thumbpress] >= 0.75f ? (l_scores[CGestureMatcher::GT_Thumbpress] - 0.75f) / 0.25f : 0.f);
                                if(CDriverConfig::IsTouchpadAxesEnabled())
                                {
                                    m_buttons[IB_TrackpadX]->SetValue(l_scores[CGestureMatcher::GT_TouchpadAxisX]);
                                    m_buttons[IB_TrackpadY]->SetValue(l_scores[CGestureMatcher::GT_TouchpadAxisY]);
                                }
                            }
                            else
                            {
                                m_buttons[IB_TrackpadTouch]->SetState(false);
                                m_buttons[IB_TrackpadForce]->SetValue(0.f);
                            }
                        }
                    }
                    if(CDriverConfig::IsTriggerEnabled())
                    {
                        m_buttons[IB_TriggerClick]->SetState(l_scores[CGestureMatcher::GT_TriggerFinger] >= 0.75f);
                        m_buttons[IB_TriggerValue]->SetValue(l_scores[CGestureMatcher::GT_TriggerFinger]);
                    }
                    if(CDriverConfig::IsThumbstickEnabled())
                    {
                        m_buttons[IB_ThumbstickTouch]->SetState(l_scores[CGestureMatcher::GT_ThumbIndexCrossTouch] >= 0.5f);
                        m_buttons[IB_ThumbstickClick]->SetState(l_scores[CGestureMatcher::GT_ThumbIndexCrossTouch] >= 0.75f);

                        // Keyboard thumbstick direction when NumLock is active
                        if((GetKeyState(VK_NUMLOCK) & 0xFFFF) != 0)
                        {
                            if(GetAsyncKeyState((m_hand == CH_Left) ? VK_RIGHT : VK_NUMPAD6) & 0x8000) m_buttons[IB_ThumbstickX]->SetValue(1.f);
                            else if(GetAsyncKeyState((m_hand == CH_Left) ? VK_LEFT : VK_NUMPAD4) & 0x8000) m_buttons[IB_ThumbstickX]->SetValue(-1.f);
                            else m_buttons[IB_ThumbstickX]->SetValue(0.f);

                            if(GetAsyncKeyState((m_hand == CH_Left) ? VK_UP : VK_NUMPAD8) & 0x8000) m_buttons[IB_ThumbstickY]->SetValue(1.f);
                            else if(GetAsyncKeyState((m_hand == CH_Left) ? VK_DOWN : VK_NUMPAD2) & 0x8000) m_buttons[IB_ThumbstickY]->SetValue(-1.f);
                            else m_buttons[IB_ThumbstickY]->SetValue(0.f);
                        }
                    }
                    if(CDriverConfig::IsButtonAEnabled())
                    {
                        m_buttons[IB_ATouch]->SetState(l_scores[CGestureMatcher::GT_ThumbMiddleTouch] >= 0.5f);
                        m_buttons[IB_AClick]->SetState(l_scores[CGestureMatcher::GT_ThumbMiddleTouch] >= 0.75f);
                    }
                    if(CDriverConfig::IsButtonBEnabled())
                    {
                        m_buttons[IB_BTouch]->SetState(l_scores[CGestureMatcher::GT_ThumbPinkyTouch] >= 0.5f);
                        m_buttons[IB_BClick]->SetState(l_scores[CGestureMatcher::GT_ThumbPinkyTouch] >= 0.75f);
                    }
                }
            } break;

            case GP_VRChat:
            {
                if(CDriverConfig::IsInputEnabled())
                {
                    if(CDriverConfig::IsTouchpadEnabled())
                    {
                        if(CDriverConfig::IsTouchpadTouchEnabled())
                        {
                            if(l_scores[CGestureMatcher::GT_Thumbpress] >= 0.5f)
                            {
                                m_buttons[IB_TrackpadTouch]->SetState(true);
                                m_buttons[IB_TrackpadForce]->SetValue(l_scores[CGestureMatcher::GT_Thumbpress] >= 0.75f ? (l_scores[CGestureMatcher::GT_Thumbpress] - 0.75f) / 0.25f : 0.f);
                            }
                            else
                            {
                                m_buttons[IB_TrackpadTouch]->SetState(false);
                                m_buttons[IB_TrackpadForce]->SetValue(0.f);
                            }
                        }
                        if(CDriverConfig::IsTriggerEnabled())
                        {
                            m_buttons[IB_TriggerClick]->SetState(l_scores[CGestureMatcher::GT_TriggerFinger] >= 0.75f);
                            m_buttons[IB_TriggerValue]->SetValue(l_scores[CGestureMatcher::GT_TriggerFinger]);
                        }
                        if(CDriverConfig::IsGripEnabled())
                        {
                            m_buttons[IB_GripTouch]->SetState(l_scores[CGestureMatcher::GT_LowerFist] >= 0.85f);
                            m_buttons[IB_GripValue]->SetValue(l_scores[CGestureMatcher::GT_LowerFist]);
                            m_buttons[IB_GripForce]->SetValue(l_scores[CGestureMatcher::GT_LowerFist] >= 0.85f ? (l_scores[CGestureMatcher::GT_LowerFist] - 0.85f) / 0.15f : 0.f);
                        }
                        if(CDriverConfig::IsButtonBEnabled())
                        {
                            m_buttons[IB_BTouch]->SetState(l_scores[CGestureMatcher::GT_Timeout] >= 0.5f);
                            m_buttons[IB_BClick]->SetState(l_scores[CGestureMatcher::GT_Timeout] >= 0.75f);
                        }
                    }
                }
            } break;
        }

        // Update skeleton
        if(CDriverConfig::IsSkeletonEnabled())
        {
            m_buttons[IB_FingerIndex]->SetValue(l_scores[CGestureMatcher::GT_IndexFingerBend]);
            m_buttons[IB_FingerMiddle]->SetValue(l_scores[CGestureMatcher::GT_MiddleFingerBend]);
            m_buttons[IB_FingerRing]->SetValue(l_scores[CGestureMatcher::GT_RingFingerBend]);
            m_buttons[IB_FingerPinky]->SetValue(l_scores[CGestureMatcher::GT_PinkyFingerBend]);

            for(Leap::Hand l_hand : f_frame.hands())
            {
                if(l_hand.isValid())
                {
                    if((l_hand.isLeft() && (m_hand == CH_Left)) || (l_hand.isRight() && (m_hand == CH_Right)))
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
                                    if(m_hand == CH_Left) l_result = glm::rotate(l_result, glm::pi<float>(), g_axisX);
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
                        const glm::mat4 l_wristMat = glm::translate(g_identityMatrix, l_position) * glm::mat4_cast(l_rotation);

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
                        if(m_hand == CH_Left) FixAuxBoneTransformation(l_position, l_rotation);
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
                        if(m_hand == CH_Left) FixAuxBoneTransformation(l_position, l_rotation);
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
                        if(m_hand == CH_Left) FixAuxBoneTransformation(l_position, l_rotation);
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
                        if(m_hand == CH_Left) FixAuxBoneTransformation(l_position, l_rotation);
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
                        if(m_hand == CH_Left) FixAuxBoneTransformation(l_position, l_rotation);
                        ConvertVector3(l_position, m_boneTransform[HSB_Aux_PinkyFinger].position);
                        ConvertQuaternion(l_rotation, m_boneTransform[HSB_Aux_PinkyFinger].orientation);

                        break;
                    }
                }
            }
        }
    }
}

void CLeapControllerIndex::UpdateInputInternal()
{
    ms_driverInput->UpdateSkeletonComponent(m_skeletonHandle, vr::VRSkeletalMotionRange_WithController, m_boneTransform, HSB_Count);
    ms_driverInput->UpdateSkeletonComponent(m_skeletonHandle, vr::VRSkeletalMotionRange_WithoutController, m_boneTransform, HSB_Count);
}
