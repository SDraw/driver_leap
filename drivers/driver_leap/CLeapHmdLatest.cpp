#include "stdafx.h"
#include "CLeapHmdLatest.h"
#include "CDriverLogHelper.h"
#include "CConfigHelper.h"
#include "CGestureMatcher.h"
#include "Utils.h"

const Leap::Vector g_AxisX(1.f, 0.f, 0.f);
const Leap::Vector g_AxisY(0.f, 1.f, 0.f);
const Leap::Vector g_AxisZ(0.f, 0.f, 1.f);

extern char g_ModuleFileName[];
const vr::VREvent_Data_t g_EmptyVREventData = { 0 };
const long long g_VRTrackingLatency = -std::chrono::duration_cast<std::chrono::seconds>(std::chrono::milliseconds(-30)).count();

const std::vector<std::string> g_SteamAppKeysTable = {
    "steam.app.438100" // VRChat
};
#define STEAM_APPKEY_VRCHAT 0U

const std::vector<std::string> g_DebugRequestStringTable = {
    "leap:realign_coordinates",
    "app_key"
};
#define CONTROLLER_DEBUGREQUEST_REALIGN 0U
#define CONTROLLER_DEBUGREQUEST_APPKEY 1U

CLeapHmdLatest::CLeapHmdLatest(vr::IVRServerDriverHost* pDriverHost, int n)
{
    CDriverLogHelper::DriverLog("CLeapHmdLatest::CLeapHmdLatest(n=%d)\n", n);

    m_pDriverHost = pDriverHost;
    m_driverInput = nullptr;
    m_nId = n;
    m_unSteamVRTrackedDeviceId = vr::k_unTrackedDeviceIndexInvalid;

    char buf[256];
    GenerateSerialNumber(buf, sizeof(buf), n);
    m_strSerialNumber.assign(buf);
    m_propertyContainer = vr::k_ulInvalidPropertyContainer;
    m_connected = true;

    m_gripAngleOffset.x = CConfigHelper::GetGripOffsetX();
    m_gripAngleOffset.y = CConfigHelper::GetGripOffsetY();
    m_gripAngleOffset.z = CConfigHelper::GetGripOffsetZ();
    if(m_nId == RIGHT_CONTROLLER)
    {
        // Only X axis isn't inverted for right controller
        m_gripAngleOffset.y *= -1.f;
        m_gripAngleOffset.z *= -1.f;
    }

    std::memset(&m_Pose, 0, sizeof(vr::DriverPose_t));
    m_Pose.qDriverFromHeadRotation.w = 1;
    m_Pose.qDriverFromHeadRotation.x = 0;
    m_Pose.qDriverFromHeadRotation.y = 0;
    m_Pose.qDriverFromHeadRotation.z = 0;
    m_Pose.vecDriverFromHeadTranslation[0] = 0;
    m_Pose.vecDriverFromHeadTranslation[1] = 0;
    m_Pose.vecDriverFromHeadTranslation[2] = 0;
    m_Pose.vecAngularVelocity[0] = 0.0;
    m_Pose.vecAngularVelocity[1] = 0.0;
    m_Pose.vecAngularVelocity[2] = 0.0;
    m_Pose.vecAcceleration[0] = 0.0;
    m_Pose.vecAcceleration[1] = 0.0;
    m_Pose.vecAcceleration[2] = 0.0;
    m_Pose.vecAngularAcceleration[0] = 0.0;
    m_Pose.vecAngularAcceleration[1] = 0.0;
    m_Pose.vecAngularAcceleration[2] = 0.0;
    m_Pose.poseTimeOffset = -0.016f;
    m_Pose.willDriftInYaw = false;
    m_Pose.shouldApplyHeadModel = false;
    m_Pose.result = vr::TrackingResult_Uninitialized;

    std::fill_n(m_hmdPos, 3U, 0.f);
    m_hmdRot = { 1.0, .0, .0, .0 };
    m_gameProfile = GP_Default;
}

CLeapHmdLatest::~CLeapHmdLatest()
{
    CDriverLogHelper::DriverLog("CLeapHmdLatest::~CLeapHmdLatest(n=%d)\n", m_nId);
}

void* CLeapHmdLatest::GetComponent(const char* pchComponentNameAndVersion)
{
    return NULL;
}

vr::EVRInitError CLeapHmdLatest::Activate(uint32_t unObjectId)
{
    CDriverLogHelper::DriverLog("CLeapHmdLatest::Activate: %s is object id %d\n", GetSerialNumber(), unObjectId);
    m_unSteamVRTrackedDeviceId = unObjectId;

    vr::CVRPropertyHelpers *l_vrProperties = vr::VRProperties();
    m_propertyContainer = l_vrProperties->TrackedDeviceToPropertyContainer(m_unSteamVRTrackedDeviceId);

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
    l_modelLabel.append(std::to_string(m_nId));
    l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_ModeLabel_String, l_modelLabel.c_str());

    l_vrProperties->SetInt32Property(m_propertyContainer, vr::Prop_ControllerRoleHint_Int32, (m_nId == LEFT_CONTROLLER) ? vr::TrackedControllerRole_LeftHand : vr::TrackedControllerRole_RightHand);
    l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_ManufacturerName_String, "HTC");

    std::string l_path(g_ModuleFileName);
    l_path.erase(l_path.begin() + l_path.rfind('\\'), l_path.end());
    l_path.append("\\profile.json");
    l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_InputProfilePath_String, l_path.c_str());

    l_vrProperties->SetUint64Property(m_propertyContainer, vr::Prop_HardwareRevision_Uint64, 1313);
    l_vrProperties->SetUint64Property(m_propertyContainer, vr::Prop_FirmwareVersion_Uint64, 1315);
    l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_ModelNumber_String, "LeapMotion");
    l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_SerialNumber_String, m_strSerialNumber.c_str());
    l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_RenderModelName_String, "vr_controller_vive_1_5");

    vr::HmdMatrix34_t matrix = { 0.f };
    l_vrProperties->SetProperty(m_propertyContainer, vr::Prop_CameraToHeadTransform_Matrix34, &matrix, sizeof(vr::HmdMatrix34_t), vr::k_unHmdMatrix34PropertyTag);

    m_driverInput = vr::VRDriverInput();
    m_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/system/click", &m_buttons[CB_SysClick].m_handle);
    m_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/grip/click", &m_buttons[CB_GripClick].m_handle);
    m_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/application_menu/click", &m_buttons[CB_AppMenuClick].m_handle);
    m_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/trigger/click", &m_buttons[CB_TriggerClick].m_handle);
    m_driverInput->CreateScalarComponent(m_propertyContainer, "/input/trigger/value", &m_buttons[CB_TriggerValue].m_handle, vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
    m_driverInput->CreateScalarComponent(m_propertyContainer, "/input/trackpad/x", &m_buttons[CB_TrackpadX].m_handle, vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
    m_driverInput->CreateScalarComponent(m_propertyContainer, "/input/trackpad/y", &m_buttons[CB_TrackpadY].m_handle, vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
    m_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/trackpad/click", &m_buttons[CB_TrackpadClick].m_handle);
    m_driverInput->CreateBooleanComponent(m_propertyContainer, "/input/trackpad/touch", &m_buttons[CB_TrackpadTouch].m_handle);

    return vr::VRInitError_None;
}

void CLeapHmdLatest::Deactivate()
{
    CDriverLogHelper::DriverLog("CLeapHmdLatest::Deactivate: %s was object id %d\n", GetSerialNumber(), m_unSteamVRTrackedDeviceId);
    m_unSteamVRTrackedDeviceId = vr::k_unTrackedDeviceIndexInvalid;
}

void CLeapHmdLatest::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
{
    std::istringstream ss(pchRequest);
    std::string strCmd;

    ss >> strCmd;
    switch(ReadEnumVector(strCmd, g_DebugRequestStringTable))
    {
        case CONTROLLER_DEBUGREQUEST_REALIGN:
        {
            float m[3][3], v[3];
            for(int i = 0; i < 3; ++i)
            {
                for(int j = 0; j < 3; ++j)
                {
                    ss >> m[j][i];
                }
                ss >> v[i];
            }

            vr::HmdQuaternion_t q = CalculateRotation(m);
            memcpy(m_hmdPos, &v[0], sizeof(m_hmdPos));
            m_hmdRot = q;
        } break;
        case CONTROLLER_DEBUGREQUEST_APPKEY:
        {
            std::string l_appKey;
            ss >> l_appKey;
            switch(ReadEnumVector(l_appKey, g_SteamAppKeysTable))
            {
                case STEAM_APPKEY_VRCHAT:
                    m_gameProfile = GP_VRChat;
                    break;
                default:
                    m_gameProfile = GP_Default;
            }
            ResetControls();
        } break;
    }
}

const char* CLeapHmdLatest::GetSerialNumber() const
{
    return m_strSerialNumber.c_str();
}

vr::DriverPose_t CLeapHmdLatest::GetPose()
{
    return m_Pose;
}


void CLeapHmdLatest::EnterStandby()
{
    CDriverLogHelper::DriverLog("CLeapHmdLatest::EnterStandby()\n");
}

void CLeapHmdLatest::UpdateControllerState(Leap::Frame& frame)
{
    bool handFound = false;
    CGestureMatcher::WhichHand which = ((m_nId == LEFT_CONTROLLER) ? CGestureMatcher::LeftHand : CGestureMatcher::RightHand);

    float scores[CGestureMatcher::NUM_GESTURES] = { 0.f };
    handFound = CGestureMatcher::MatchGestures(frame, which, scores);

    if(handFound)
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
    }
}

void CLeapHmdLatest::UpdateTrackingState(Leap::Frame &frame)
{
    Leap::HandList &hands = frame.hands();

    bool handFound = false;
    for(int h = 0; h < hands.count(); h++)
    {
        Leap::Hand &hand = hands[h];

        if(hand.isValid() && ((m_nId == LEFT_CONTROLLER && hand.isLeft()) || (m_nId == RIGHT_CONTROLLER && hand.isRight())))
        {
            handFound = true;

            m_Pose.qWorldFromDriverRotation = m_hmdRot;
            m_Pose.vecWorldFromDriverTranslation[0] = m_hmdPos[0];
            m_Pose.vecWorldFromDriverTranslation[1] = m_hmdPos[1];
            m_Pose.vecWorldFromDriverTranslation[2] = m_hmdPos[2];

            Leap::Vector position = hand.palmPosition();

            m_Pose.vecPosition[0] = -0.001*position.x;
            m_Pose.vecPosition[1] = -0.001*position.z;
            m_Pose.vecPosition[2] = -0.001*position.y - 0.15;

            Leap::Vector velocity = hand.palmVelocity();

            m_Pose.vecVelocity[0] = -0.001*velocity.x;
            m_Pose.vecVelocity[1] = -0.001*velocity.z;
            m_Pose.vecVelocity[2] = -0.001*velocity.y;

            Leap::Vector direction = hand.direction();
            direction /= direction.magnitude();

            Leap::Vector normal = hand.palmNormal();
            normal /= normal.magnitude();

            Leap::Vector side = direction.cross(normal);

            switch(m_nId)
            {
                case LEFT_CONTROLLER:
                {
                    float L[3][3] =
                    { { -normal.x, -normal.z, -normal.y },
                    { side.x, side.z, side.y },
                    { direction.x, direction.z, direction.y } };
                    m_Pose.qRotation = CalculateRotation(L);
                } break;
                case RIGHT_CONTROLLER:
                {

                    float R[3][3] =
                    { { normal.x, normal.z, normal.y },
                    { -side.x, -side.z, -side.y },
                    { direction.x, direction.z, direction.y } };
                    m_Pose.qRotation = CalculateRotation(R);
                } break;
            }

            if(m_gripAngleOffset.x != 0.f)
                m_Pose.qRotation = rotate_around_axis(g_AxisX, m_gripAngleOffset.x) * m_Pose.qRotation;
            if(m_gripAngleOffset.y != 0.f)
                m_Pose.qRotation = rotate_around_axis(g_AxisY, m_gripAngleOffset.y) * m_Pose.qRotation;
            if(m_gripAngleOffset.z != 0.f)
                m_Pose.qRotation = rotate_around_axis(g_AxisZ, m_gripAngleOffset.z) * m_Pose.qRotation;

            m_Pose.result = vr::TrackingResult_Running_OK;
        }
    }

    if(!handFound) m_Pose.result = vr::TrackingResult_Running_OutOfRange;
    m_Pose.poseIsValid = handFound;

    if(!m_Pose.deviceIsConnected) m_Pose.deviceIsConnected = true;

    m_pDriverHost->TrackedDevicePoseUpdated(m_unSteamVRTrackedDeviceId, m_Pose, sizeof(vr::DriverPose_t));
}

void CLeapHmdLatest::Update(Leap::Frame &frame)
{
    UpdateTrackingState(frame);
    UpdateControllerState(frame);
}

void CLeapHmdLatest::RealignCoordinates()
{
    if(m_unSteamVRTrackedDeviceId == vr::k_unTrackedDeviceIndexInvalid) return;

    m_pDriverHost->VendorSpecificEvent(m_unSteamVRTrackedDeviceId, (vr::EVREventType) (vr::VREvent_VendorSpecific_Reserved_Start + 0), g_EmptyVREventData, g_VRTrackingLatency);
}

void CLeapHmdLatest::SetAsDisconnected()
{
    if(m_unSteamVRTrackedDeviceId == vr::k_unTrackedDeviceIndexInvalid) return;

    m_Pose.deviceIsConnected = false;
    m_pDriverHost->TrackedDevicePoseUpdated(m_unSteamVRTrackedDeviceId, m_Pose, sizeof(vr::DriverPose_t));
}

void CLeapHmdLatest::ProcessDefaultProfileGestures(float *l_scores)
{
    if(CConfigHelper::IsMenuEnabled())
    {
        bool l_state = (l_scores[CGestureMatcher::Timeout] >= 0.25f);
        SControllerButton &l_button = m_buttons[CB_SysClick];
        if(l_button.m_state != l_state)
        {
            l_button.m_state = l_state;
            m_driverInput->UpdateBooleanComponent(l_button.m_handle, l_state, .0);
        }
    }

    if(CConfigHelper::IsApplicationMenuEnabled())
    {
        bool l_state = (l_scores[CGestureMatcher::FlatHandPalmTowards] >= 0.8f);
        SControllerButton &l_button = m_buttons[CB_AppMenuClick];
        if(l_button.m_state != l_state)
        {
            l_button.m_state = l_state;
            m_driverInput->UpdateBooleanComponent(l_button.m_handle, l_state, .0);
        }
    }

    if(CConfigHelper::IsTriggerEnabled())
    {
        bool l_state = (l_scores[CGestureMatcher::TriggerFinger] >= 0.5f);
        SControllerButton &l_button1 = m_buttons[CB_TriggerClick];
        if(l_button1.m_state != l_state)
        {
            l_button1.m_state = l_state;
            m_driverInput->UpdateBooleanComponent(l_button1.m_handle, l_state, .0);
        }

        SControllerButton &l_button2 = m_buttons[CB_TriggerValue];
        if(l_button2.m_value != l_scores[CGestureMatcher::TriggerFinger])
        {
            l_button2.m_value = l_scores[CGestureMatcher::TriggerFinger];
            m_driverInput->UpdateScalarComponent(l_button2.m_handle, l_button2.m_value, .0);
        }
    }

    if(CConfigHelper::IsGripEnabled())
    {
        bool l_state = (l_scores[CGestureMatcher::LowerFist] >= 0.5f);
        SControllerButton &l_button = m_buttons[CB_GripClick];
        if(l_button.m_state != l_state)
        {
            l_button.m_state = l_state;
            m_driverInput->UpdateBooleanComponent(l_button.m_handle, l_state, .0);
        }
    }

    if(CConfigHelper::IsTouchpadEnabled())
    {
        if(CConfigHelper::IsTouchpadAxesEnabled())
        {
            SControllerButton &l_button1 = m_buttons[CB_TrackpadX];
            if(l_button1.m_value != l_scores[CGestureMatcher::TouchpadAxisX])
            {
                l_button1.m_value = l_scores[CGestureMatcher::TouchpadAxisX];
                m_driverInput->UpdateScalarComponent(l_button1.m_handle, l_button1.m_value, .0);
            }

            SControllerButton &l_button2 = m_buttons[CB_TrackpadY];
            if(l_button2.m_value != l_scores[CGestureMatcher::TouchpadAxisY])
            {
                l_button2.m_value = l_scores[CGestureMatcher::TouchpadAxisY];
                m_driverInput->UpdateScalarComponent(l_button2.m_handle, l_button2.m_value, .0);
            }
        }

        if(CConfigHelper::IsTouchpadTouchEnabled())
        {
            bool l_state = (l_scores[CGestureMatcher::Thumbpress] >= 0.5f);
            SControllerButton &l_button = m_buttons[CB_TrackpadTouch];
            if(l_button.m_state != l_state)
            {
                l_button.m_state = l_state;
                m_driverInput->UpdateBooleanComponent(l_button.m_handle, l_state, .0);
            }
        }

        if(CConfigHelper::IsTouchpadPressEnabled())
        {
            bool l_state = (l_scores[CGestureMatcher::Thumbpress] >= 0.9f);
            SControllerButton &l_button = m_buttons[CB_TrackpadClick];
            if(l_button.m_state != l_state)
            {
                l_button.m_state = l_state;
                m_driverInput->UpdateBooleanComponent(l_button.m_handle, l_state, .0);
            }
        }
    }
}
void CLeapHmdLatest::ProcessVRChatProfileGestures(float *l_scores)
{
    // VRChat profile ignores control restrictions
    bool l_state = (l_scores[CGestureMatcher::Timeout] >= 0.75f);
    SControllerButton &l_button1 = m_buttons[CB_AppMenuClick];
    if(l_button1.m_state != l_state)
    {
        l_button1.m_state = l_state;
        m_driverInput->UpdateBooleanComponent(l_button1.m_handle, l_state, .0);
    }

    l_state = (l_scores[CGestureMatcher::LowerFist] >= 0.5f);
    SControllerButton &l_button2 = m_buttons[CB_TriggerClick];
    if(l_button2.m_state != l_state)
    {
        l_button2.m_state = l_state;
        m_driverInput->UpdateBooleanComponent(l_button2.m_handle, l_state, .0);
    }

    l_state = (l_scores[CGestureMatcher::VRChat_SpreadHand] >= 0.75f);
    SControllerButton &l_button3 = m_buttons[CB_GripClick];
    if(l_button3.m_state != l_state)
    {
        l_button3.m_state = l_state;
        m_driverInput->UpdateBooleanComponent(l_button3.m_handle, l_state, .0);
    }

    vr::VRControllerAxis_t l_trackpadAxis = { 0.f };
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
    if(m_nId == LEFT_CONTROLLER) l_trackpadAxis.x *= -1.f;

    SControllerButton &l_button4 = m_buttons[CB_TrackpadX];
    if(l_button4.m_value != l_trackpadAxis.x)
    {
        l_button4.m_value = l_trackpadAxis.x;
        m_driverInput->UpdateScalarComponent(l_button4.m_handle, l_button4.m_value, .0);
    }

    SControllerButton &l_button5 = m_buttons[CB_TrackpadY];
    if(l_button5.m_value != l_trackpadAxis.y)
    {
        l_button5.m_value = l_trackpadAxis.y;
        m_driverInput->UpdateScalarComponent(l_button5.m_handle, l_button5.m_value, .0);
    }

    l_state = (l_trackpadAxis.x != 0.f || l_trackpadAxis.y != 0.f);
    SControllerButton &l_button6 = m_buttons[CB_TrackpadTouch];
    if(l_button6.m_state != l_state)
    {
        l_button6.m_state = l_state;
        m_driverInput->UpdateBooleanComponent(l_button6.m_handle, l_state, .0);
    }

    SControllerButton &l_button7 = m_buttons[CB_TriggerValue];
    if(l_button7.m_value != l_scores[CGestureMatcher::LowerFist])
    {
        l_button7.m_value = l_scores[CGestureMatcher::LowerFist];
        m_driverInput->UpdateScalarComponent(l_button7.m_handle, l_button7.m_value, .0);
    }
}

void CLeapHmdLatest::ResetControls()
{
    m_buttons[CB_SysClick].m_state = false;
    m_driverInput->UpdateBooleanComponent(m_buttons[CB_SysClick].m_handle, false, .0);
    m_buttons[CB_GripClick].m_state = false;
    m_driverInput->UpdateBooleanComponent(m_buttons[CB_GripClick].m_handle, false, .0);
    m_buttons[CB_AppMenuClick].m_state = false;
    m_driverInput->UpdateBooleanComponent(m_buttons[CB_AppMenuClick].m_handle, false, .0);
    m_buttons[CB_TriggerClick].m_state = false;
    m_driverInput->UpdateBooleanComponent(m_buttons[CB_TriggerClick].m_handle, false, .0);
    m_buttons[CB_TriggerValue].m_value = 0.f;
    m_driverInput->UpdateScalarComponent(m_buttons[CB_TriggerValue].m_handle, 0.f, .0);
    m_buttons[CB_TrackpadX].m_value = 0.f;
    m_driverInput->UpdateScalarComponent(m_buttons[CB_TrackpadX].m_handle, 0.f, .0);
    m_buttons[CB_TrackpadY].m_value = 0.f;
    m_driverInput->UpdateScalarComponent(m_buttons[CB_TrackpadY].m_handle, 0.f, .0);
    m_buttons[CB_TrackpadClick].m_state = false;
    m_driverInput->UpdateBooleanComponent(m_buttons[CB_TrackpadClick].m_handle, false, .0);
    m_buttons[CB_TrackpadTouch].m_state = false;
    m_driverInput->UpdateBooleanComponent(m_buttons[CB_TrackpadTouch].m_handle, false, .0);
}
