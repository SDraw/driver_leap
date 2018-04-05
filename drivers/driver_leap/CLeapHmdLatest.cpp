#include "stdafx.h"
#include "CLeapHmdLatest.h"
#include "CDriverLogHelper.h"
#include "CGestureMatcher.h"
#include "CServerDriver_Leap.h"
#include "Utils.h"

extern CServerDriver_Leap g_ServerTrackedDeviceProvider;

const std::chrono::milliseconds CLeapHmdLatest::k_TrackingLatency(-30);

CLeapHmdLatest::CLeapHmdLatest(vr::IVRServerDriverHost* pDriverHost, int base, int n)
    : m_pDriverHost(pDriverHost)
    , m_nBase(base)
    , m_nId(n)
    , m_bCalibrated(true)
    , m_pAlignmentPartner(nullptr)
    , m_unSteamVRTrackedDeviceId(vr::k_unTrackedDeviceIndexInvalid)
{
    CDriverLogHelper::DriverLog("CLeapHmdLatest::CLeapHmdLatest(base=%d, n=%d)\n", base, n);

    memset(m_hmdPos, 0, sizeof(m_hmdPos));

    char buf[256];
    GenerateSerialNumber(buf, sizeof(buf), base, n);
    m_strSerialNumber = buf;

    memset(&m_ControllerState, 0, sizeof(m_ControllerState));
    memset(&m_Pose, 0, sizeof(m_Pose));
    m_Pose.result = vr::TrackingResult_Uninitialized;

    m_firmware_revision = 0x0001;
    m_hardware_revision = 0x0001;

    vr::IVRSettings *settings_ = vr::VRSettings();

    char tmp_[256];
    settings_->GetString("leap", (m_nId == LEFT_CONTROLLER) ? "renderModel_lefthand" : (m_nId == RIGHT_CONTROLLER) ? "renderModel_righthand" : "renderModel", tmp_, sizeof(tmp_));
    m_strRenderModel = tmp_;

    m_gripAngleOffset = settings_->GetFloat("leap", (m_nId == LEFT_CONTROLLER) ? "gripAngleOffset_lefthand" : (m_nId == RIGHT_CONTROLLER) ? "gripAngleOffset_righthand" : "gripAngleOffset");
    m_hmdRot = vr::HmdQuaternion_t{ .0, .0, .0, .0 };
}

CLeapHmdLatest::~CLeapHmdLatest()
{
    CDriverLogHelper::DriverLog("CLeapHmdLatest::~CLeapHmdLatest(base=%d, n=%d)\n", m_nBase, m_nId);
}

void *CLeapHmdLatest::GetComponent(const char* pchComponentNameAndVersion)
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

    vr::CVRPropertyHelpers *propertyHelpers = vr::VRProperties();
    vr::PropertyContainerHandle_t propertyContainer = propertyHelpers->TrackedDeviceToPropertyContainer(m_unSteamVRTrackedDeviceId);

    propertyHelpers->SetStringProperty(propertyContainer, vr::Prop_SerialNumber_String, m_strSerialNumber.c_str());
    propertyHelpers->SetStringProperty(propertyContainer, vr::Prop_RenderModelName_String, m_strRenderModel.c_str());
    propertyHelpers->SetStringProperty(propertyContainer, vr::Prop_ManufacturerName_String, "LeapMotion");
    propertyHelpers->SetStringProperty(propertyContainer, vr::Prop_ModelNumber_String, "Controller");
    propertyHelpers->SetStringProperty(propertyContainer, vr::Prop_TrackingFirmwareVersion_String, std::to_string(m_firmware_revision).c_str());
    propertyHelpers->SetStringProperty(propertyContainer, vr::Prop_HardwareRevision_String, std::to_string(m_hardware_revision).c_str());
    propertyHelpers->SetInt32Property(propertyContainer, vr::Prop_Axis0Type_Int32, vr::k_eControllerAxis_Joystick);
    propertyHelpers->SetInt32Property(propertyContainer, vr::Prop_Axis1Type_Int32, vr::k_eControllerAxis_Trigger);
    propertyHelpers->SetUint64Property(propertyContainer, vr::Prop_SupportedButtons_Uint64,
        vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu) |
        vr::ButtonMaskFromId(vr::k_EButton_System) |
        vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad) |
        vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger) |
        vr::ButtonMaskFromId(vr::k_EButton_Grip)
        );
    propertyHelpers->SetUint64Property(propertyContainer, vr::Prop_HardwareRevision_Uint64, m_hardware_revision);
    propertyHelpers->SetUint64Property(propertyContainer, vr::Prop_FirmwareVersion_Uint64, m_firmware_revision);

    vr::HmdMatrix34_t matrix = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
    propertyHelpers->SetProperty(propertyContainer, vr::Prop_CameraToHeadTransform_Matrix34, &matrix, sizeof(vr::HmdMatrix34_t), vr::k_unHmdMatrix34PropertyTag);

    g_ServerTrackedDeviceProvider.LaunchLeapMonitor();

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
    if(strCmd == "leap:realign_coordinates")
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
        FinishRealignCoordinates(m, v);
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
    return vr::VRControllerState_t();
}

bool CLeapHmdLatest::TriggerHapticPulse(uint32_t unAxisId, uint16_t usPulseDurationMicroseconds)
{
    return true;
}

void CLeapHmdLatest::SendButtonUpdates(ButtonUpdate ButtonEvent, uint64_t ulMask)
{
    if(!ulMask)
        return;

    for(int i = 0; i < vr::k_EButton_Max; i++)
    {
        vr::EVRButtonId button = (vr::EVRButtonId)i;

        uint64_t bit = ButtonMaskFromId(button);

        if(bit & ulMask)
        {
            (m_pDriverHost->*ButtonEvent)(m_unSteamVRTrackedDeviceId, button, 0.0);
        }
    }
}

void CLeapHmdLatest::UpdateControllerState(Leap::Frame& frame)
{
    bool handFound = false;
    CGestureMatcher::WhichHand which = (m_nId == LEFT_CONTROLLER) ? CGestureMatcher::LeftHand :
        (m_nId == RIGHT_CONTROLLER) ? CGestureMatcher::RightHand :
        CGestureMatcher::AnyHand;

    float scores[CGestureMatcher::NUM_GESTURES];
    handFound = CGestureMatcher::MatchGestures(frame, which, scores);

    if(handFound)
    {
        vr::VRControllerState_t NewState = { 0 };
        NewState.unPacketNum = m_ControllerState.unPacketNum + 1;

        if(scores[CGestureMatcher::Timeout] >= 0.25f)
        {
            NewState.ulButtonTouched |= vr::ButtonMaskFromId(vr::k_EButton_System);
            if(scores[CGestureMatcher::Timeout] >= 0.5f) NewState.ulButtonPressed |= vr::ButtonMaskFromId(vr::k_EButton_System);
        }

        if(scores[CGestureMatcher::FlatHandPalmTowards] >= 0.4f)
        {
            NewState.ulButtonTouched |= vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu);
            if(scores[CGestureMatcher::FlatHandPalmTowards] >= 0.8f) NewState.ulButtonPressed |= vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu);
        }

        if(scores[CGestureMatcher::TriggerFinger] >= 0.25f)
        {
            NewState.ulButtonTouched |= vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger);
            if(scores[CGestureMatcher::TriggerFinger] >= 0.5f) NewState.ulButtonPressed |= vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger);
        }

        if(scores[CGestureMatcher::LowerFist] >= 0.25f)
        {
            NewState.ulButtonTouched |= vr::ButtonMaskFromId(vr::k_EButton_Grip);
            if(scores[CGestureMatcher::LowerFist] >= 0.5f) NewState.ulButtonPressed |= vr::ButtonMaskFromId(vr::k_EButton_Grip);
        }

        if(scores[CGestureMatcher::Thumbpress] >= 0.5f)
        {
            NewState.ulButtonTouched |= vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad);
            if(scores[CGestureMatcher::Thumbpress] >= 0.9f) NewState.ulButtonPressed |= vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad);
        }

        NewState.ulButtonTouched |= NewState.ulButtonPressed;

        uint64_t ulChangedTouched = NewState.ulButtonTouched ^ m_ControllerState.ulButtonTouched;
        uint64_t ulChangedPressed = NewState.ulButtonPressed ^ m_ControllerState.ulButtonPressed;

        SendButtonUpdates(&vr::IVRServerDriverHost::TrackedDeviceButtonTouched, ulChangedTouched & NewState.ulButtonTouched);
        SendButtonUpdates(&vr::IVRServerDriverHost::TrackedDeviceButtonPressed, ulChangedPressed & NewState.ulButtonPressed);
        SendButtonUpdates(&vr::IVRServerDriverHost::TrackedDeviceButtonUnpressed, ulChangedPressed & ~NewState.ulButtonPressed);
        SendButtonUpdates(&vr::IVRServerDriverHost::TrackedDeviceButtonUntouched, ulChangedTouched & ~NewState.ulButtonTouched);

        NewState.rAxis[0].x = scores[CGestureMatcher::TouchpadAxisX];
        NewState.rAxis[0].y = scores[CGestureMatcher::TouchpadAxisY];

        NewState.rAxis[1].x = scores[CGestureMatcher::TriggerFinger];
        NewState.rAxis[1].y = 0.0f;

        if(NewState.rAxis[0].x != m_ControllerState.rAxis[0].x || NewState.rAxis[0].y != m_ControllerState.rAxis[0].y)
            m_pDriverHost->TrackedDeviceAxisUpdated(m_unSteamVRTrackedDeviceId, 0, NewState.rAxis[0]);

        if(NewState.rAxis[1].x != m_ControllerState.rAxis[1].x)
            m_pDriverHost->TrackedDeviceAxisUpdated(m_unSteamVRTrackedDeviceId, 1, NewState.rAxis[1]);

        m_ControllerState = NewState;
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

            m_Pose.qDriverFromHeadRotation.w = 1;
            m_Pose.qDriverFromHeadRotation.x = 0;
            m_Pose.qDriverFromHeadRotation.y = 0;
            m_Pose.qDriverFromHeadRotation.z = 0;
            m_Pose.vecDriverFromHeadTranslation[0] = 0;
            m_Pose.vecDriverFromHeadTranslation[1] = 0;
            m_Pose.vecDriverFromHeadTranslation[2] = 0;

            Leap::Vector position = hand.palmPosition();

            m_Pose.vecPosition[0] = -0.001*position.x;
            m_Pose.vecPosition[1] = -0.001*position.z;
            m_Pose.vecPosition[2] = -0.001*position.y - 0.15;

            Leap::Vector velocity = hand.palmVelocity();

            m_Pose.vecVelocity[0] = -0.001*velocity.x;
            m_Pose.vecVelocity[1] = -0.001*velocity.z;
            m_Pose.vecVelocity[2] = -0.001*velocity.y;

            m_Pose.vecAcceleration[0] = 0.0;
            m_Pose.vecAcceleration[1] = 0.0;
            m_Pose.vecAcceleration[2] = 0.0;

            Leap::Vector direction = hand.direction(); direction /= direction.magnitude();
            Leap::Vector normal = hand.palmNormal(); normal /= normal.magnitude();
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

            if(m_gripAngleOffset != 0.f)
                m_Pose.qRotation = rotate_around_axis(Leap::Vector(1.0, 0.0, 0.0), m_gripAngleOffset) * m_Pose.qRotation;

            m_Pose.vecAngularVelocity[0] = 0.0;
            m_Pose.vecAngularVelocity[1] = 0.0;
            m_Pose.vecAngularVelocity[2] = 0.0;

            m_Pose.vecAngularAcceleration[0] = 0.0;
            m_Pose.vecAngularAcceleration[1] = 0.0;
            m_Pose.vecAngularAcceleration[2] = 0.0;

            m_Pose.result = vr::TrackingResult_Running_OK;
            m_Pose.poseIsValid = m_bCalibrated;
        }
    }

    if(!handFound)
    {
        m_Pose.result = vr::TrackingResult_Running_OutOfRange;
        m_Pose.poseIsValid = false;
    }

    m_Pose.poseTimeOffset = -0.016f;
    m_Pose.deviceIsConnected = true;
    m_Pose.willDriftInYaw = false;
    m_Pose.shouldApplyHeadModel = false;

    m_pDriverHost->TrackedDevicePoseUpdated(m_unSteamVRTrackedDeviceId, m_Pose, sizeof(vr::DriverPose_t));
}

bool CLeapHmdLatest::IsActivated() const
{
    return m_unSteamVRTrackedDeviceId != vr::k_unTrackedDeviceIndexInvalid;
}

bool CLeapHmdLatest::HasControllerId(int nBase, int nId) const
{
    return nBase == m_nBase && nId == m_nId;
}

bool CLeapHmdLatest::Update(Leap::Frame &frame)
{
    UpdateTrackingState(frame);
    UpdateControllerState(frame);

    return true;
}

void CLeapHmdLatest::RealignCoordinates(CLeapHmdLatest* pLeapA, CLeapHmdLatest* pLeapB)
{
    if(pLeapA->m_unSteamVRTrackedDeviceId == vr::k_unTrackedDeviceIndexInvalid)
        return;

    pLeapA->m_pAlignmentPartner = pLeapB;
    pLeapB->m_pAlignmentPartner = pLeapA;

    static vr::VREvent_Data_t nodata = { 0 };
    pLeapA->m_pDriverHost->VendorSpecificEvent(pLeapA->m_unSteamVRTrackedDeviceId,
        (vr::EVREventType) (vr::VREvent_VendorSpecific_Reserved_Start + 0), nodata,
        -std::chrono::duration_cast<std::chrono::seconds>(k_TrackingLatency).count());
}

void CLeapHmdLatest::FinishRealignCoordinates(float(*m)[3], float* v)
{
    CLeapHmdLatest * pLeapA = this;
    CLeapHmdLatest * pLeapB = m_pAlignmentPartner;

    if(!pLeapA || !pLeapB)
        return;

    vr::HmdQuaternion_t q = CalculateRotation(m);
    pLeapA->UpdateHmdPose(v, q);
    pLeapB->UpdateHmdPose(v, q);
}

void CLeapHmdLatest::UpdateHmdPose(float* v, const vr::HmdQuaternion_t& q)
{
    memcpy(m_hmdPos, &v[0], sizeof(m_hmdPos));
    m_hmdRot = q;
    m_bCalibrated = true;
}
