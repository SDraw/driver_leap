#include "stdafx.h"
#include "CLeapHandController.h"
#include "CConfigHelper.h"
#include "CGestureMatcher.h"
#include "Utils.h"

extern char g_moduleFileName[];

//----
CControllerButton::CControllerButton()
{
    m_handle = vr::k_ulInvalidInputComponentHandle;
    m_state = false;
    m_value = 0.f;
    m_inputType = EControllerButtonInputType::CBIT_Boolean;
    m_updated = false;
}
CControllerButton::~CControllerButton()
{
}

void CControllerButton::SetValue(float f_value)
{
    if(m_inputType == EControllerButtonInputType::CBIT_Float)
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
    if(m_inputType == EControllerButtonInputType::CBIT_Boolean)
    {
        if(m_state != f_state)
        {
            m_state = f_state;
            m_updated = true;
        }
    }
}

//----
const std::vector<std::string> g_steamAppKeysTable = {
    "steam.app.438100" // VRChat
};
#define STEAM_APPKEY_VRCHAT 0U

const std::vector<std::string> g_debugRequestStringTable = {
    "app_key"
};
#define CONTROLLER_DEBUGREQUEST_APPKEY 0U

double CLeapHandController::ms_headPos[] = { .0, .0, .0 };
vr::HmdQuaternion_t CLeapHandController::ms_headRot = { 1.0, .0, .0, .0 };

CLeapHandController::CLeapHandController(vr::IVRServerDriverHost* pDriverHost, int n)
{
    m_driverHost = pDriverHost;
    m_driverInput = nullptr;
    m_id = n;
    m_trackedDeviceID = vr::k_unTrackedDeviceIndexInvalid;

    m_serialNumber.assign("leap_");
    m_serialNumber.append((m_id == LEFT_CONTROLLER) ? "lefthand" : "righthand");
    m_propertyContainer = vr::k_ulInvalidPropertyContainer;

    glm::vec3 l_eulerOffsetRot(CConfigHelper::GetGripOffsetX(), CConfigHelper::GetGripOffsetY(), CConfigHelper::GetGripOffsetZ());
    if(m_id == RIGHT_CONTROLLER)
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

    m_gameProfile = GP_Default;
    m_isEnabled = ((m_id == LEFT_CONTROLLER) ? CConfigHelper::IsLeftControllerEnabled() : CConfigHelper::IsRightControllerEnabled());
}
CLeapHandController::~CLeapHandController()
{
}

vr::EVRInitError CLeapHandController::Activate(uint32_t unObjectId)
{
    m_trackedDeviceID = unObjectId;

    vr::CVRPropertyHelpers *l_vrProperties = vr::VRProperties();
    m_propertyContainer = l_vrProperties->TrackedDeviceToPropertyContainer(m_trackedDeviceID);

    l_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_WillDriftInYaw_Bool, false);
    l_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_DeviceIsWireless_Bool, false);

    l_vrProperties->SetInt32Property(m_propertyContainer, vr::Prop_DeviceClass_Int32, vr::TrackedDeviceClass_Controller);
    l_vrProperties->SetInt32Property(m_propertyContainer, vr::Prop_Axis0Type_Int32, vr::k_eControllerAxis_TrackPad);
    l_vrProperties->SetInt32Property(m_propertyContainer, vr::Prop_Axis1Type_Int32, vr::k_eControllerAxis_Trigger);

    l_vrProperties->SetUint64Property(m_propertyContainer, vr::Prop_SupportedButtons_Uint64,
        vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu) |
        vr::ButtonMaskFromId(vr::k_EButton_System) |
        vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad) |
        vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger) |
        vr::ButtonMaskFromId(vr::k_EButton_Grip)
        );

    std::string l_modelLabel("leap_");
    l_modelLabel.append(std::to_string(m_id));
    l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_ModeLabel_String, l_modelLabel.c_str());

    l_vrProperties->SetInt32Property(m_propertyContainer, vr::Prop_ControllerRoleHint_Int32, (m_id == LEFT_CONTROLLER) ? vr::TrackedControllerRole_LeftHand : vr::TrackedControllerRole_RightHand);
    l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_ManufacturerName_String, "HTC");

    std::string l_path(g_moduleFileName);
    l_path.erase(l_path.begin() + l_path.rfind('\\'), l_path.end());
    l_path.append("\\profile.json");
    l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_InputProfilePath_String, l_path.c_str());

    l_vrProperties->SetUint64Property(m_propertyContainer, vr::Prop_HardwareRevision_Uint64, 1313);
    l_vrProperties->SetUint64Property(m_propertyContainer, vr::Prop_FirmwareVersion_Uint64, 1315);
    l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_ModelNumber_String, "LeapMotion");
    l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_SerialNumber_String, m_serialNumber.c_str());
    l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_RenderModelName_String, "vr_controller_vive_1_5");

    vr::HmdMatrix34_t matrix = { 0.f };
    l_vrProperties->SetProperty(m_propertyContainer, vr::Prop_CameraToHeadTransform_Matrix34, &matrix, sizeof(vr::HmdMatrix34_t), vr::k_unHmdMatrix34PropertyTag);

    m_driverInput = vr::VRDriverInput();

    m_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/system/click", &m_buttons[CB_SysClick].GetHandleRef());
    m_buttons[CB_SysClick].SetInputType(EControllerButtonInputType::CBIT_Boolean);

    m_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/grip/click", &m_buttons[CB_GripClick].GetHandleRef());
    m_buttons[CB_GripClick].SetInputType(EControllerButtonInputType::CBIT_Boolean);

    m_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/application_menu/click", &m_buttons[CB_AppMenuClick].GetHandleRef());
    m_buttons[CB_AppMenuClick].SetInputType(EControllerButtonInputType::CBIT_Boolean);

    m_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/trigger/click", &m_buttons[CB_TriggerClick].GetHandleRef());
    m_buttons[CB_TriggerClick].SetInputType(EControllerButtonInputType::CBIT_Boolean);

    m_driverInput->CreateScalarComponent(m_propertyContainer, "/input/trigger/value", &m_buttons[CB_TriggerValue].GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
    m_buttons[CB_TriggerValue].SetInputType(EControllerButtonInputType::CBIT_Float);

    m_driverInput->CreateScalarComponent(m_propertyContainer, "/input/trackpad/x", &m_buttons[CB_TrackpadX].GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
    m_buttons[CB_TrackpadX].SetInputType(EControllerButtonInputType::CBIT_Float);

    m_driverInput->CreateScalarComponent(m_propertyContainer, "/input/trackpad/y", &m_buttons[CB_TrackpadY].GetHandleRef(), vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
    m_buttons[CB_TrackpadY].SetInputType(EControllerButtonInputType::CBIT_Float);

    m_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/trackpad/click", &m_buttons[CB_TrackpadClick].GetHandleRef());
    m_buttons[CB_TrackpadClick].SetInputType(EControllerButtonInputType::CBIT_Boolean);

    m_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/trackpad/touch", &m_buttons[CB_TrackpadTouch].GetHandleRef());
    m_buttons[CB_TrackpadTouch].SetInputType(EControllerButtonInputType::CBIT_Boolean);

    return vr::VRInitError_None;
}

void CLeapHandController::Deactivate()
{
    m_trackedDeviceID = vr::k_unTrackedDeviceIndexInvalid;
}

void* CLeapHandController::GetComponent(const char* pchComponentNameAndVersion)
{
    if(!strcmp(pchComponentNameAndVersion, vr::ITrackedDeviceServerDriver_Version)) return this;
    return nullptr;
}

void CLeapHandController::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
{
    std::stringstream ss(pchRequest);
    std::string strCmd;

    ss >> strCmd;
    switch(ReadEnumVector(strCmd, g_debugRequestStringTable))
    {
        case CONTROLLER_DEBUGREQUEST_APPKEY:
        {
            std::string l_appKey;
            ss >> l_appKey;

            EGameProfile l_last = m_gameProfile;
            switch(ReadEnumVector(l_appKey, g_steamAppKeysTable))
            {
                case STEAM_APPKEY_VRCHAT:
                    m_gameProfile = GP_VRChat;
                    break;
                default:
                    m_gameProfile = GP_Default;
            }
            if(l_last != m_gameProfile) ResetControls();
        } break;
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
    if(m_trackedDeviceID == vr::k_unTrackedDeviceIndexInvalid) return;

    m_pose.deviceIsConnected = false;
    m_driverHost->TrackedDevicePoseUpdated(m_trackedDeviceID, m_pose, sizeof(vr::DriverPose_t));
}

void CLeapHandController::Update(Leap::Frame &frame)
{
    UpdateTrasnformation(frame);
    UpdateGestures(frame);
}

void CLeapHandController::UpdateGestures(Leap::Frame& frame)
{
    float scores[CGestureMatcher::NUM_GESTURES] = { 0.f };
    if(CGestureMatcher::MatchGestures(frame, ((m_id == LEFT_CONTROLLER) ? CGestureMatcher::LeftHand : CGestureMatcher::RightHand), scores))
    {
        switch(m_gameProfile)
        {
            case GP_Default:
                ProcessDefaultProfileGestures(scores);
                break;
            case GP_VRChat:
                ProcessVRChatProfileGestures(scores);
                break;
        }
        UpdateButtonInput();
    }
}

void CLeapHandController::ProcessDefaultProfileGestures(float *l_scores)
{
    if(CConfigHelper::IsMenuEnabled()) m_buttons[CB_SysClick].SetState(l_scores[CGestureMatcher::Timeout] >= 0.25f);
    if(CConfigHelper::IsApplicationMenuEnabled()) m_buttons[CB_AppMenuClick].SetState(l_scores[CGestureMatcher::FlatHandPalmTowards] >= 0.8f);

    if(CConfigHelper::IsTriggerEnabled())
    {
        m_buttons[CB_TriggerClick].SetState(l_scores[CGestureMatcher::TriggerFinger] >= 0.75f);
        m_buttons[CB_TriggerValue].SetValue(l_scores[CGestureMatcher::TriggerFinger]);
    }

    if(CConfigHelper::IsGripEnabled()) m_buttons[CB_GripClick].SetState(l_scores[CGestureMatcher::LowerFist] >= 0.5f);

    if(CConfigHelper::IsTouchpadEnabled())
    {
        if(CConfigHelper::IsTouchpadAxesEnabled())
        {
            m_buttons[CB_TrackpadX].SetValue(l_scores[CGestureMatcher::TouchpadAxisX]);
            m_buttons[CB_TrackpadY].SetValue(l_scores[CGestureMatcher::TouchpadAxisY]);
        }
        if(CConfigHelper::IsTouchpadTouchEnabled()) m_buttons[CB_TrackpadTouch].SetState(l_scores[CGestureMatcher::Thumbpress] <= 0.5f);
        if(CConfigHelper::IsTouchpadPressEnabled()) m_buttons[CB_TrackpadClick].SetState(l_scores[CGestureMatcher::Thumbpress] <= 0.1f);
    }
}
void CLeapHandController::ProcessVRChatProfileGestures(float *l_scores)
{
    // VRChat profile ignores control restrictions
    m_buttons[CB_AppMenuClick].SetState(l_scores[CGestureMatcher::Timeout] >= 0.75f);

    glm::vec2 l_trackpadAxis(0.f);
    if(l_scores[CGestureMatcher::VRChat_Point] >= 0.75f)
    {
        l_trackpadAxis.x = 0.0f;
        l_trackpadAxis.y = 1.0f;
    }
    else if(l_scores[CGestureMatcher::VRChat_ThumbsUp] >= 0.75f)
    {
        l_trackpadAxis.x = -0.95f;
        l_trackpadAxis.y = 0.31f;
    }
    else if(l_scores[CGestureMatcher::VRChat_Victory] >= 0.75f)
    {
        l_trackpadAxis.x = 0.95f;
        l_trackpadAxis.y = 0.31f;
    }
    else if(l_scores[CGestureMatcher::VRChat_Gun] >= 0.75f)
    {
        l_trackpadAxis.x = -0.59f;
        l_trackpadAxis.y = -0.81f;
    }
    else if(l_scores[CGestureMatcher::VRChat_RockOut] >= 0.75f)
    {
        l_trackpadAxis.x = 0.59f;
        l_trackpadAxis.y = -0.81f;
    }
    if(m_id == LEFT_CONTROLLER) l_trackpadAxis.x *= -1.f;
    m_buttons[CB_TrackpadX].SetValue(l_trackpadAxis.x);
    m_buttons[CB_TrackpadY].SetValue(l_trackpadAxis.y);
    m_buttons[CB_TrackpadTouch].SetState((l_trackpadAxis.x != 0.f) || (l_trackpadAxis.y != 0.f));

    m_buttons[CB_TriggerValue].SetValue(l_scores[CGestureMatcher::LowerFist]);
    m_buttons[CB_TriggerClick].SetState(l_scores[CGestureMatcher::LowerFist] >= 0.5f);
    m_buttons[CB_GripClick].SetState(l_scores[CGestureMatcher::VRChat_SpreadHand] >= 0.75f);
}

void CLeapHandController::UpdateTrasnformation(Leap::Frame &frame)
{
    Leap::HandList &hands = frame.hands();

    bool l_handFound = false;
    for(int h = 0; h < hands.count(); h++)
    {
        Leap::Hand &l_hand = hands[h];

        if(l_hand.isValid() && ((m_id == LEFT_CONTROLLER && l_hand.isLeft()) || (m_id == RIGHT_CONTROLLER && l_hand.isRight())))
        {
            l_handFound = true;

            std::memcpy(&m_pose.qWorldFromDriverRotation, &ms_headRot, sizeof(vr::HmdQuaternion_t));
            std::memcpy(m_pose.vecWorldFromDriverTranslation, ms_headPos, sizeof(double) * 3U);

            Leap::Vector position = l_hand.palmPosition();
            m_pose.vecPosition[0] = -0.001*position.x;
            m_pose.vecPosition[1] = -0.001*position.z;
            m_pose.vecPosition[2] = -0.001*position.y - 0.15; // ?

            Leap::Vector velocity = l_hand.palmVelocity();
            m_pose.vecVelocity[0] = -0.001*velocity.x;
            m_pose.vecVelocity[1] = -0.001*velocity.z;
            m_pose.vecVelocity[2] = -0.001*velocity.y;

            Leap::Vector l_handDirection = l_hand.direction();
            l_handDirection /= l_handDirection.magnitude();

            Leap::Vector l_palmNormal = l_hand.palmNormal();
            l_palmNormal /= l_palmNormal.magnitude();

            Leap::Vector l_leapCross = l_handDirection.cross(l_palmNormal);

            glm::mat3 l_rotMat(
                l_palmNormal.x, l_palmNormal.z, l_palmNormal.y,
                l_leapCross.x, l_leapCross.z, l_leapCross.y,
                l_handDirection.x, l_handDirection.z, l_handDirection.y
                );
            for(size_t i = 0U; i < 3U; i++) l_rotMat[static_cast<size_t>(m_id)][i] *= -1.f; // Reverse normal for left controller (0) and cross product for right controller (1)
            glm::quat l_finalRot(l_rotMat);
            l_finalRot *= m_gripAngleOffset;

            m_pose.qRotation.x = l_finalRot.x;
            m_pose.qRotation.y = l_finalRot.y;
            m_pose.qRotation.z = l_finalRot.z;
            m_pose.qRotation.w = l_finalRot.w;

            m_pose.result = vr::TrackingResult_Running_OK;
            break;
        }
    }

    if(m_isEnabled)
    {
        if(!l_handFound) m_pose.result = vr::TrackingResult_Calibrating_InProgress;
        m_pose.poseIsValid = l_handFound;
    }
    else
    {
        m_pose.result = vr::TrackingResult_Running_OutOfRange;
        m_pose.poseIsValid = false;
    }

    if(!m_pose.deviceIsConnected) m_pose.deviceIsConnected = true;

    m_driverHost->TrackedDevicePoseUpdated(m_trackedDeviceID, m_pose, sizeof(vr::DriverPose_t));
}

void CLeapHandController::UpdateButtonInput()
{
    for(size_t i = 0U; i < CB_Count; i++)
    {
        CControllerButton &l_button = m_buttons[i];
        if(l_button.IsUpdated())
        {
            switch(l_button.GetInputType())
            {
                case EControllerButtonInputType::CBIT_Boolean:
                    m_driverInput->UpdateBooleanComponent(l_button.GetHandle(), l_button.GetState(), .0);
                    break;
                case EControllerButtonInputType::CBIT_Float:
                    m_driverInput->UpdateScalarComponent(l_button.GetHandle(), l_button.GetValue(), .0);
                    break;
            }
            l_button.ResetUpdate();
        }
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
        const auto &l_hmdMat = l_hmdPose.mDeviceToAbsoluteTracking.m;
        glm::mat4 l_rotMat(
            l_hmdMat[0][0], l_hmdMat[1][0], l_hmdMat[2][0], 0.f,
            l_hmdMat[0][1], l_hmdMat[1][1], l_hmdMat[2][1], 0.f,
            l_hmdMat[0][2], l_hmdMat[1][2], l_hmdMat[2][2], 0.f,
            l_hmdMat[0][3], l_hmdMat[1][3], l_hmdMat[2][3], 1.f
            );
        glm::quat l_headRot(l_rotMat);
        ms_headRot.x = l_headRot.x;
        ms_headRot.y = l_headRot.y;
        ms_headRot.z = l_headRot.z;
        ms_headRot.w = l_headRot.w;

        for(int i = 0; i < 3; i++) ms_headPos[i] = l_hmdMat[i][3];
    }
}
