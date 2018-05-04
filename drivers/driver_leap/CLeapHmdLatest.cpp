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

CLeapHmdLatest::CLeapHmdLatest(vr::IVRServerDriverHost* pDriverHost, int n)
    : m_pDriverHost(pDriverHost)
    , m_nId(n)
    , m_unSteamVRTrackedDeviceId(vr::k_unTrackedDeviceIndexInvalid)
{
    CDriverLogHelper::DriverLog("CLeapHmdLatest::CLeapHmdLatest(n=%d)\n", n);

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
    m_ControllerState = { 0 };
    m_hmdRot = { 1.0, .0, .0, .0 };
}

CLeapHmdLatest::~CLeapHmdLatest()
{
    CDriverLogHelper::DriverLog("CLeapHmdLatest::~CLeapHmdLatest(n=%d)\n", m_nId);
}

void* CLeapHmdLatest::GetComponent(const char* pchComponentNameAndVersion)
{
    if(!stricmp(pchComponentNameAndVersion, vr::IVRControllerComponent_Version))
    {
        return (vr::IVRControllerComponent*)this;
    }

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
    l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_InputProfileName_String, l_path.c_str());

    l_vrProperties->SetUint64Property(m_propertyContainer, vr::Prop_HardwareRevision_Uint64, 1313);
    l_vrProperties->SetUint64Property(m_propertyContainer, vr::Prop_FirmwareVersion_Uint64, 1315);
    l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_ModelNumber_String, "LeapMotion");
    l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_SerialNumber_String, m_strSerialNumber.c_str());
    l_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_RenderModelName_String, "vr_controller_vive_1_5");

    vr::HmdMatrix34_t matrix = { 0.f };
    l_vrProperties->SetProperty(m_propertyContainer, vr::Prop_CameraToHeadTransform_Matrix34, &matrix, sizeof(vr::HmdMatrix34_t), vr::k_unHmdMatrix34PropertyTag);

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
    if(!strCmd.compare("leap:realign_coordinates"))
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

vr::VRControllerState_t CLeapHmdLatest::GetControllerState()
{
    return m_ControllerState;
}

bool CLeapHmdLatest::TriggerHapticPulse(uint32_t unAxisId, uint16_t usPulseDurationMicroseconds)
{
    return true;
}

void CLeapHmdLatest::SendButtonUpdates(ButtonUpdate ButtonEvent, uint64_t ulMask)
{
    if(!ulMask) return;

    for(int i = 0; i < vr::k_EButton_Max; i++)
    {
        vr::EVRButtonId button = (vr::EVRButtonId)i;
        uint64_t bit = ButtonMaskFromId(button);
        if(bit & ulMask) (m_pDriverHost->*ButtonEvent)(m_unSteamVRTrackedDeviceId, button, 0.0);
    }
}

void CLeapHmdLatest::UpdateControllerState(Leap::Frame& frame)
{
    bool handFound = false;
    CGestureMatcher::WhichHand which = ((m_nId == LEFT_CONTROLLER) ? CGestureMatcher::LeftHand : CGestureMatcher::RightHand);

    float scores[CGestureMatcher::NUM_GESTURES] = { 0.f };
    handFound = CGestureMatcher::MatchGestures(frame, which, scores);

    if(handFound)
    {
        vr::VRControllerState_t NewState = { 0 };
        NewState.unPacketNum = m_ControllerState.unPacketNum + 1;

        switch(CConfigHelper::GetGameProfile())
        {
            case CConfigHelper::GP_Default:
                ProcessDefaultProfileGestures(NewState, scores);
                break;
            case CConfigHelper::GP_VRChat:
                ProcessVRChatProfileGestures(NewState, scores);
                break;
        }

        NewState.ulButtonTouched |= NewState.ulButtonPressed;
        uint64_t ulChangedTouched = NewState.ulButtonTouched ^ m_ControllerState.ulButtonTouched;
        uint64_t ulChangedPressed = NewState.ulButtonPressed ^ m_ControllerState.ulButtonPressed;
        SendButtonUpdates(&vr::IVRServerDriverHost::TrackedDeviceButtonTouched, ulChangedTouched & NewState.ulButtonTouched);
        SendButtonUpdates(&vr::IVRServerDriverHost::TrackedDeviceButtonPressed, ulChangedPressed & NewState.ulButtonPressed);
        SendButtonUpdates(&vr::IVRServerDriverHost::TrackedDeviceButtonUnpressed, ulChangedPressed & ~NewState.ulButtonPressed);
        SendButtonUpdates(&vr::IVRServerDriverHost::TrackedDeviceButtonUntouched, ulChangedTouched & ~NewState.ulButtonTouched);

        std::memcpy(&m_ControllerState, &NewState, sizeof(vr::VRControllerState_t));
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

void CLeapHmdLatest::ProcessDefaultProfileGestures(vr::VRControllerState_t &l_state, float *l_scores)
{
    if(CConfigHelper::IsMenuEnabled())
    {
        if(l_scores[CGestureMatcher::Timeout] >= 0.25f)
        {
            l_state.ulButtonTouched |= vr::ButtonMaskFromId(vr::k_EButton_System);
            if(l_scores[CGestureMatcher::Timeout] >= 0.5f) l_state.ulButtonPressed |= vr::ButtonMaskFromId(vr::k_EButton_System);
        }
    }

    if(CConfigHelper::IsApplicationMenuEnabled())
    {
        if(l_scores[CGestureMatcher::FlatHandPalmTowards] >= 0.4f)
        {
            l_state.ulButtonTouched |= vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu);
            if(l_scores[CGestureMatcher::FlatHandPalmTowards] >= 0.8f) l_state.ulButtonPressed |= vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu);
        }
    }

    if(CConfigHelper::IsTriggerEnabled())
    {
        if(l_scores[CGestureMatcher::TriggerFinger] >= 0.25f)
        {
            l_state.ulButtonTouched |= vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger);
            if(l_scores[CGestureMatcher::TriggerFinger] >= 0.5f) l_state.ulButtonPressed |= vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger);
        }
    }

    if(CConfigHelper::IsGripEnabled())
    {
        if(l_scores[CGestureMatcher::LowerFist] >= 0.25f)
        {
            l_state.ulButtonTouched |= vr::ButtonMaskFromId(vr::k_EButton_Grip);
            if(l_scores[CGestureMatcher::LowerFist] >= 0.5f) l_state.ulButtonPressed |= vr::ButtonMaskFromId(vr::k_EButton_Grip);
        }
    }

    if(CConfigHelper::IsTouchpadEnabled())
    {
        if(CConfigHelper::IsTouchpadTouchEnabled())
        {
            if(l_scores[CGestureMatcher::Thumbpress] >= 0.5f)
            {
                l_state.ulButtonTouched |= vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad);
                if(CConfigHelper::IsTouchpadPressEnabled())
                {
                    if(l_scores[CGestureMatcher::Thumbpress] >= 0.9f) l_state.ulButtonPressed |= vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad);
                }
            }
        }
    }

    l_state.rAxis[0].x = l_scores[CGestureMatcher::TouchpadAxisX];
    l_state.rAxis[0].y = l_scores[CGestureMatcher::TouchpadAxisY];
    l_state.rAxis[1].x = l_scores[CGestureMatcher::TriggerFinger];
    l_state.rAxis[1].y = 0.0f;

    if(CConfigHelper::IsTouchpadEnabled() && CConfigHelper::IsTouchpadTouchEnabled() && CConfigHelper::IsTouchpadAxesEnabled())
    {
        if(l_state.rAxis[0].x != m_ControllerState.rAxis[0].x || l_state.rAxis[0].y != m_ControllerState.rAxis[0].y)
            m_pDriverHost->TrackedDeviceAxisUpdated(m_unSteamVRTrackedDeviceId, 0, l_state.rAxis[0]);
    }
    if(l_state.rAxis[1].x != m_ControllerState.rAxis[1].x)
        m_pDriverHost->TrackedDeviceAxisUpdated(m_unSteamVRTrackedDeviceId, 1, l_state.rAxis[1]);
}
void CLeapHmdLatest::ProcessVRChatProfileGestures(vr::VRControllerState_t &l_state, float *l_scores)
{
    // VRChat profile ignores control restrictions
    if(l_scores[CGestureMatcher::Timeout] >= 0.75f)
    {
        l_state.ulButtonTouched |= vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu);
        l_state.ulButtonPressed |= vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu);
    }
    if(l_scores[CGestureMatcher::LowerFist] >= 0.5f)
    {
        l_state.ulButtonTouched |= vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger);
        l_state.ulButtonPressed |= vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger);
    }
    if(l_scores[CGestureMatcher::VRChat_SpreadHand] >= 0.75f)
    {
        l_state.ulButtonTouched |= vr::ButtonMaskFromId(vr::k_EButton_Grip);
        l_state.ulButtonPressed |= vr::ButtonMaskFromId(vr::k_EButton_Grip);
    }

    l_state.rAxis[0].x = 0.f;
    l_state.rAxis[0].y = 0.f;
    if(l_scores[CGestureMatcher::VRChat_Point] >= 0.75f)
    {
        l_state.rAxis[0].x = 0.0f;
        l_state.rAxis[0].y = 1.0f;
    }
    else if(l_scores[CGestureMatcher::VRChat_ThumbsUp] >= 0.75f)
    {
        l_state.rAxis[0].x = -0.95f;
        l_state.rAxis[0].y = 0.31f;
    }
    else if(l_scores[CGestureMatcher::VRChat_Victory] >= 0.75f)
    {
        l_state.rAxis[0].x = 0.95f;
        l_state.rAxis[0].y = 0.31f;
    }
    else if(l_scores[CGestureMatcher::VRChat_Gun] >= 0.75f)
    {
        l_state.rAxis[0].x = -0.59f;
        l_state.rAxis[0].y = -0.81f;
    }
    else if(l_scores[CGestureMatcher::VRChat_RockOut] >= 0.75f)
    {
        l_state.rAxis[0].x = 0.59f;
        l_state.rAxis[0].y = -0.81f;
    }
    if(l_state.rAxis[0].x != 0.f || l_state.rAxis[0].y != 0.f)
    {
        l_state.ulButtonTouched |= vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad);
        if(m_nId == LEFT_CONTROLLER) l_state.rAxis[0].x *= -1.f;
    }

    l_state.rAxis[1].x = l_scores[CGestureMatcher::LowerFist];
    l_state.rAxis[1].y = 0.0f;

    if(l_state.rAxis[0].x != m_ControllerState.rAxis[0].x || l_state.rAxis[0].y != m_ControllerState.rAxis[0].y)
            m_pDriverHost->TrackedDeviceAxisUpdated(m_unSteamVRTrackedDeviceId, 0, l_state.rAxis[0]);
    if (l_state.rAxis[1].x != m_ControllerState.rAxis[1].x)
        m_pDriverHost->TrackedDeviceAxisUpdated(m_unSteamVRTrackedDeviceId, 1, l_state.rAxis[1]);
}
