#include "stdafx.h"

#include "CLeapControllerVive.h"

#include "CControllerButton.h"
#include "CDriverConfig.h"
#include "CGestureMatcher.h"

enum ViveButton : size_t
{
    VB_SysClick = 0U,
    VB_TriggerClick,
    VB_TriggerValue,
    VB_TrackpadX,
    VB_TrackpadY,
    VB_TrackpadTouch,
    VB_TrackpadClick,
    VB_GripClick,
    VB_AppMenuClick,

    VB_Count
};

CLeapControllerVive::CLeapControllerVive(unsigned char f_hand)
{
    m_hand = (f_hand % CH_Count);
    m_serialNumber.assign((m_hand == CH_Left) ? "LHR-F94B3BD8" : "LHR-F94B3BD9");
}
CLeapControllerVive::~CLeapControllerVive()
{
}

void CLeapControllerVive::ActivateInternal()
{
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
    ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_ModelNumber_String, "Vive. Controller MV");
    ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_SerialNumber_String, m_serialNumber.c_str()); // Changed
    ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_RenderModelName_String, "vr_controller_vive_1_5");
    ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_ManufacturerName_String, "HTC");
    ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_TrackingFirmwareVersion_String, "1533720215 htcvrsoftware@firmware-win32 2018-08-08 FPGA 262(1.6/0/0) BL 0 VRC 1533720214 Radio 1532585738");
    ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_HardwareRevision_String, "product 129 rev 1.5.0 lot 2000/0/0 0");
    ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_ConnectedWirelessDongle_String, "1E8092840E"); // Changed
    ms_propertyHelpers->SetUint64Property(m_propertyContainer, vr::Prop_HardwareRevision_Uint64, 2164327680U);
    ms_propertyHelpers->SetUint64Property(m_propertyContainer, vr::Prop_FirmwareVersion_Uint64, 1533720215U);
    ms_propertyHelpers->SetUint64Property(m_propertyContainer, vr::Prop_FPGAVersion_Uint64, 262U);
    ms_propertyHelpers->SetUint64Property(m_propertyContainer, vr::Prop_VRCVersion_Uint64, 1533720214U);
    ms_propertyHelpers->SetUint64Property(m_propertyContainer, vr::Prop_RadioVersion_Uint64, 1532585738U);
    ms_propertyHelpers->SetUint64Property(m_propertyContainer, vr::Prop_DongleVersion_Uint64, 1461100729U);
    ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_ResourceRoot_String, "htc");
    ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_RegisteredDeviceType_String, (m_hand == CH_Left) ? "htc/vive_controllerLHR-F94B3BD8" : "htc/vive_controllerLHR-F94B3BD9"); // Changed
    ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_InputProfilePath_String, "{htc}/input/vive_controller_profile.json");
    ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceOff_String, "{htc}/icons/controller_status_off.png");
    ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceSearching_String, "{htc}/icons/controller_status_searching.gif");
    ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceSearchingAlert_String, "{htc}/icons/controller_status_searching_alert.gif");
    ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceReady_String, "{htc}/icons/controller_status_ready.png");
    ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceReadyAlert_String, "{htc}/icons/controller_status_ready_alert.png");
    ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceNotReady_String, "{htc}/icons/controller_status_error.png");
    ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceStandby_String, "{htc}/icons/controller_status_off.png");
    ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceAlertLow_String, "{htc}/icons/controller_status_ready_low.png");
    ms_propertyHelpers->SetStringProperty(m_propertyContainer, vr::Prop_ControllerType_String, "vive_controller");

    // Input
    if(m_buttons.empty())
    {
        for(size_t i = 0U; i < VB_Count; i++) m_buttons.push_back(new CControllerButton());
    }

    ms_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/system/click", &m_buttons[VB_SysClick]->GetHandleRef());
    m_buttons[VB_SysClick]->SetInputType(CControllerButton::IT_Boolean);

    ms_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/trigger/click", &m_buttons[VB_TriggerClick]->GetHandleRef());
    m_buttons[VB_TriggerClick]->SetInputType(CControllerButton::IT_Boolean);

    ms_driverInput->CreateScalarComponent(m_propertyContainer, "/input/trigger/value", &m_buttons[VB_TriggerValue]->GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
    m_buttons[VB_TriggerValue]->SetInputType(CControllerButton::IT_Float);

    ms_driverInput->CreateScalarComponent(m_propertyContainer, "/input/trackpad/x", &m_buttons[VB_TrackpadX]->GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
    m_buttons[VB_TrackpadX]->SetInputType(CControllerButton::IT_Float);

    ms_driverInput->CreateScalarComponent(m_propertyContainer, "/input/trackpad/y", &m_buttons[VB_TrackpadY]->GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
    m_buttons[VB_TrackpadY]->SetInputType(CControllerButton::IT_Float);

    ms_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/trackpad/touch", &m_buttons[VB_TrackpadTouch]->GetHandleRef());
    m_buttons[VB_TrackpadTouch]->SetInputType(CControllerButton::IT_Boolean);

    ms_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/trackpad/click", &m_buttons[VB_TrackpadClick]->GetHandleRef());
    m_buttons[VB_TrackpadClick]->SetInputType(CControllerButton::IT_Boolean);

    ms_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/grip/click", &m_buttons[VB_GripClick]->GetHandleRef());
    m_buttons[VB_GripClick]->SetInputType(CControllerButton::IT_Boolean);

    ms_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/application_menu/click", &m_buttons[VB_AppMenuClick]->GetHandleRef());
    m_buttons[VB_AppMenuClick]->SetInputType(CControllerButton::IT_Boolean);

    ms_driverInput->CreateHapticComponent(m_propertyContainer, "/output/haptic", &m_haptic);
}

bool CLeapControllerVive::MixHandState(bool f_state)
{
    bool l_result = f_state;
    if(m_gameProfile == GP_VRChat) l_result = true; // VRChat changes controls if one controller is out of range or disconnected
    return l_result;
}

void CLeapControllerVive::UpdateGestures(const Leap::Frame &f_frame)
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
                    if(CDriverConfig::IsMenuEnabled()) m_buttons[VB_SysClick]->SetState(l_scores[CGestureMatcher::GT_Timeout] >= 0.25f);
                    if(CDriverConfig::IsApplicationMenuEnabled()) m_buttons[VB_AppMenuClick]->SetState(l_scores[CGestureMatcher::GT_FlatHandPalmTowards] >= 0.8f);

                    if(CDriverConfig::IsTriggerEnabled())
                    {
                        m_buttons[VB_TriggerClick]->SetState(l_scores[CGestureMatcher::GT_TriggerFinger] >= 0.75f);
                        m_buttons[VB_TriggerValue]->SetValue(l_scores[CGestureMatcher::GT_TriggerFinger]);
                    }

                    if(CDriverConfig::IsGripEnabled()) m_buttons[VB_GripClick]->SetState(l_scores[CGestureMatcher::GT_LowerFist] >= 0.5f);

                    if(CDriverConfig::IsTouchpadEnabled())
                    {
                        if(CDriverConfig::IsTouchpadAxesEnabled())
                        {
                            m_buttons[VB_TrackpadX]->SetValue(l_scores[CGestureMatcher::GT_TouchpadAxisX]);
                            m_buttons[VB_TrackpadY]->SetValue(l_scores[CGestureMatcher::GT_TouchpadAxisY]);
                        }
                        if(CDriverConfig::IsTouchpadTouchEnabled()) m_buttons[VB_TrackpadTouch]->SetState(l_scores[CGestureMatcher::GT_Thumbpress] >= 0.5f);
                        if(CDriverConfig::IsTouchpadPressEnabled()) m_buttons[VB_TrackpadClick]->SetState(l_scores[CGestureMatcher::GT_Thumbpress] >= 0.1f);
                    }
                }
            } break;

            case GP_VRChat:
            {
                if(CDriverConfig::IsInputEnabled())
                {
                    if(m_specialMode)
                    {
                        m_buttons[VB_AppMenuClick]->SetState(false);
                        m_buttons[VB_TrackpadX]->SetValue(0.f);
                        m_buttons[VB_TrackpadY]->SetValue(0.f);
                        m_buttons[VB_TrackpadTouch]->SetState(false);
                        m_buttons[VB_TriggerValue]->SetValue((l_scores[CGestureMatcher::GT_LowerFist] >= 0.85f) ? 0.85f : 0.f);
                        m_buttons[VB_TriggerClick]->SetState(l_scores[CGestureMatcher::GT_LowerFist] >= 0.85f);
                        m_buttons[VB_GripClick]->SetState(false);
                    }
                    else
                    {
                        m_buttons[VB_AppMenuClick]->SetState(l_scores[CGestureMatcher::GT_Timeout] >= 0.75f);

                        glm::vec2 l_trackpadAxis(0.f);
                        if(l_scores[CGestureMatcher::GT_VRChatPoint] >= 0.75f)
                        {
                            l_trackpadAxis.x = 0.0f;
                            l_trackpadAxis.y = 1.0f;
                        }
                        else if(l_scores[CGestureMatcher::GT_VRChatThumbsUp] >= 0.75f)
                        {
                            l_trackpadAxis.x = -0.95f;
                            l_trackpadAxis.y = 0.31f;
                        }
                        else if(l_scores[CGestureMatcher::GT_VRChatVictory] >= 0.75f)
                        {
                            l_trackpadAxis.x = 0.95f;
                            l_trackpadAxis.y = 0.31f;
                        }
                        else if(l_scores[CGestureMatcher::GT_VRChatGun] >= 0.75f)
                        {
                            l_trackpadAxis.x = -0.59f;
                            l_trackpadAxis.y = -0.81f;
                        }
                        else if(l_scores[CGestureMatcher::GT_VRChatRockOut] >= 0.75f)
                        {
                            l_trackpadAxis.x = 0.59f;
                            l_trackpadAxis.y = -0.81f;
                        }
                        if(m_hand == CH_Left) l_trackpadAxis.x *= -1.f;
                        m_buttons[VB_TrackpadX]->SetValue(l_trackpadAxis.x);
                        m_buttons[VB_TrackpadY]->SetValue(l_trackpadAxis.y);
                        m_buttons[VB_TrackpadTouch]->SetState((l_trackpadAxis.x != 0.f) || (l_trackpadAxis.y != 0.f));

                        m_buttons[VB_TriggerValue]->SetValue(l_scores[CGestureMatcher::GT_LowerFist]);
                        m_buttons[VB_TriggerClick]->SetState(l_scores[CGestureMatcher::GT_LowerFist] >= 0.5f);
                        m_buttons[VB_GripClick]->SetState(l_scores[CGestureMatcher::GT_VRChatSpreadHand] >= 0.75f);
                    }
                }
            } break;
        }
    }
}
