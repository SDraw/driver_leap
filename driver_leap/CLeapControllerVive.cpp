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
    vr::VRProperties()->SetInt32Property(m_propertyContainer, vr::Prop_ControllerRoleHint_Int32, (m_hand == CH_Left) ? vr::TrackedControllerRole_LeftHand : vr::TrackedControllerRole_RightHand);
    vr::VRProperties()->SetBoolProperty(m_propertyContainer, vr::Prop_HasDisplayComponent_Bool, false);
    vr::VRProperties()->SetBoolProperty(m_propertyContainer, vr::Prop_HasCameraComponent_Bool, false);
    vr::VRProperties()->SetBoolProperty(m_propertyContainer, vr::Prop_HasDriverDirectModeComponent_Bool, false);
    vr::VRProperties()->SetBoolProperty(m_propertyContainer, vr::Prop_HasVirtualDisplayComponent_Bool, false);
    vr::VRProperties()->SetInt32Property(m_propertyContainer, vr::Prop_ControllerHandSelectionPriority_Int32, 0);
    vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_ModelNumber_String, "Vive. Controller MV");
    vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_SerialNumber_String, m_serialNumber.c_str()); // Changed
    vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_RenderModelName_String, "vr_controller_vive_1_5");
    vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_ManufacturerName_String, "HTC");
    vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_TrackingFirmwareVersion_String, "1533720215 htcvrsoftware@firmware-win32 2018-08-08 FPGA 262(1.6/0/0) BL 0 VRC 1533720214 Radio 1532585738");
    vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_HardwareRevision_String, "product 129 rev 1.5.0 lot 2000/0/0 0");
    vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_ConnectedWirelessDongle_String, "1E8092840E"); // Changed
    vr::VRProperties()->SetUint64Property(m_propertyContainer, vr::Prop_HardwareRevision_Uint64, 2164327680U);
    vr::VRProperties()->SetUint64Property(m_propertyContainer, vr::Prop_FirmwareVersion_Uint64, 1533720215U);
    vr::VRProperties()->SetUint64Property(m_propertyContainer, vr::Prop_FPGAVersion_Uint64, 262U);
    vr::VRProperties()->SetUint64Property(m_propertyContainer, vr::Prop_VRCVersion_Uint64, 1533720214U);
    vr::VRProperties()->SetUint64Property(m_propertyContainer, vr::Prop_RadioVersion_Uint64, 1532585738U);
    vr::VRProperties()->SetUint64Property(m_propertyContainer, vr::Prop_DongleVersion_Uint64, 1461100729U);
    vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_ResourceRoot_String, "htc");
    vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_RegisteredDeviceType_String, (m_hand == CH_Left) ? "htc/vive_controllerLHR-F94B3BD8" : "htc/vive_controllerLHR-F94B3BD9"); // Changed
    vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_InputProfilePath_String, "{htc}/input/vive_controller_profile.json");
    vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceOff_String, "{htc}/icons/controller_status_off.png");
    vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceSearching_String, "{htc}/icons/controller_status_searching.gif");
    vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceSearchingAlert_String, "{htc}/icons/controller_status_searching_alert.gif");
    vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceReady_String, "{htc}/icons/controller_status_ready.png");
    vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceReadyAlert_String, "{htc}/icons/controller_status_ready_alert.png");
    vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceNotReady_String, "{htc}/icons/controller_status_error.png");
    vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceStandby_String, "{htc}/icons/controller_status_off.png");
    vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceAlertLow_String, "{htc}/icons/controller_status_ready_low.png");
    vr::VRProperties()->SetStringProperty(m_propertyContainer, vr::Prop_ControllerType_String, "vive_controller");

    // Input
    if(m_buttons.empty())
    {
        for(size_t i = 0U; i < VB_Count; i++) m_buttons.push_back(new CControllerButton());
    }

    vr::VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/system/click", &m_buttons[VB_SysClick]->GetHandleRef());
    m_buttons[VB_SysClick]->SetInputType(CControllerButton::IT_Boolean);

    vr::VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/trigger/click", &m_buttons[VB_TriggerClick]->GetHandleRef());
    m_buttons[VB_TriggerClick]->SetInputType(CControllerButton::IT_Boolean);

    vr::VRDriverInput()->CreateScalarComponent(m_propertyContainer, "/input/trigger/value", &m_buttons[VB_TriggerValue]->GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
    m_buttons[VB_TriggerValue]->SetInputType(CControllerButton::IT_Float);

    vr::VRDriverInput()->CreateScalarComponent(m_propertyContainer, "/input/trackpad/x", &m_buttons[VB_TrackpadX]->GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
    m_buttons[VB_TrackpadX]->SetInputType(CControllerButton::IT_Float);

    vr::VRDriverInput()->CreateScalarComponent(m_propertyContainer, "/input/trackpad/y", &m_buttons[VB_TrackpadY]->GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
    m_buttons[VB_TrackpadY]->SetInputType(CControllerButton::IT_Float);

    vr::VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/trackpad/touch", &m_buttons[VB_TrackpadTouch]->GetHandleRef());
    m_buttons[VB_TrackpadTouch]->SetInputType(CControllerButton::IT_Boolean);

    vr::VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/trackpad/click", &m_buttons[VB_TrackpadClick]->GetHandleRef());
    m_buttons[VB_TrackpadClick]->SetInputType(CControllerButton::IT_Boolean);

    vr::VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/grip/click", &m_buttons[VB_GripClick]->GetHandleRef());
    m_buttons[VB_GripClick]->SetInputType(CControllerButton::IT_Boolean);

    vr::VRDriverInput()->CreateBooleanComponent(m_propertyContainer, "/input/application_menu/click", &m_buttons[VB_AppMenuClick]->GetHandleRef());
    m_buttons[VB_AppMenuClick]->SetInputType(CControllerButton::IT_Boolean);

    vr::VRDriverInput()->CreateHapticComponent(m_propertyContainer, "/output/haptic", &m_haptic);
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
            } break;
        }
    }
}
