#include "stdafx.h"
#include "CLeapHmdLatest.h"
#include "CDriverLogHelper.h"
#include "GestureMatcher.h"
#include "CServerDriver_Leap.h"
#include "Utils.h"

extern CServerDriver_Leap g_ServerTrackedDeviceProvider;

const std::chrono::milliseconds CLeapHmdLatest::k_TrackingLatency(-30);

CLeapHmdLatest::CLeapHmdLatest(vr::IServerDriverHost* pDriverHost, int base, int n)
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

    // Load config from steamvr.vrsettings
    vr::IVRSettings *settings_;
    settings_ = m_pDriverHost->GetSettings(vr::IVRSettings_Version);

    // Load rendermodel
    char tmp_[256];
    settings_->GetString("leap", (m_nId == LEFT_CONTROLLER) ? "renderModel_lefthand" : (m_nId == RIGHT_CONTROLLER) ? "renderModel_righthand" : "renderModel", tmp_, sizeof(tmp_));
    m_strRenderModel = tmp_;

    // set the 
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
        // leap_monitor is calling us back with HMD tracking information so we can
        // finish realigning our coordinate system to the HMD's
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
    // This is only called at startup to synchronize with the driver.
    // Future updates are driven by our thread calling TrackedDevicePoseUpdated()
    return m_Pose;
}

bool CLeapHmdLatest::GetBoolTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pError)
{
    *pError = vr::TrackedProp_ValueNotProvidedByDevice;
    return false;
}

float CLeapHmdLatest::GetFloatTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pError)
{
    *pError = vr::TrackedProp_ValueNotProvidedByDevice;
    return 0.0f;
}

int32_t CLeapHmdLatest::GetInt32TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pError)
{
    int32_t nRetVal = 0;
    vr::ETrackedPropertyError error = vr::TrackedProp_UnknownProperty;
    switch(prop)
    {
        case vr::Prop_DeviceClass_Int32:
            nRetVal = vr::TrackedDeviceClass_Controller;
            error = vr::TrackedProp_Success;
            break;

        case vr::Prop_Axis0Type_Int32:
            nRetVal = vr::k_eControllerAxis_Joystick;
            error = vr::TrackedProp_Success;
            break;

        case vr::Prop_Axis1Type_Int32:
            nRetVal = vr::k_eControllerAxis_Trigger;
            error = vr::TrackedProp_Success;
            break;

        case vr::Prop_Axis2Type_Int32:
        case vr::Prop_Axis3Type_Int32:
        case vr::Prop_Axis4Type_Int32:
            error = vr::TrackedProp_ValueNotProvidedByDevice;
            break;
    }

    *pError = error;
    return nRetVal;
}

uint64_t CLeapHmdLatest::GetUint64TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pError)
{
    uint64_t ulRetVal = 0;
    vr::ETrackedPropertyError error = vr::TrackedProp_ValueNotProvidedByDevice;

    switch(prop)
    {
        case vr::Prop_CurrentUniverseId_Uint64:
        case vr::Prop_PreviousUniverseId_Uint64:
            error = vr::TrackedProp_ValueNotProvidedByDevice;
            break;

        case vr::Prop_SupportedButtons_Uint64:
            ulRetVal =
                vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu) |
                vr::ButtonMaskFromId(vr::k_EButton_System) |
                vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad) |
                vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger) |
                vr::ButtonMaskFromId(vr::k_EButton_Grip);
            error = vr::TrackedProp_Success;
            break;

        case vr::Prop_HardwareRevision_Uint64:
            ulRetVal = m_hardware_revision;
            error = vr::TrackedProp_Success;
            break;

        case vr::Prop_FirmwareVersion_Uint64:
            ulRetVal = m_firmware_revision;
            error = vr::TrackedProp_Success;
            break;

    }

    *pError = error;
    return ulRetVal;
}

vr::HmdMatrix34_t CLeapHmdLatest::GetMatrix34TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pError)
{
    return vr::HmdMatrix34_t();
}

uint32_t CLeapHmdLatest::GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, char* pchValue, uint32_t unBufferSize, vr::ETrackedPropertyError* pError)
{
    std::ostringstream ssRetVal;

    switch(prop)
    {
        case vr::Prop_SerialNumber_String:
            ssRetVal << m_strSerialNumber;
            break;

        case vr::Prop_RenderModelName_String:
            // We return the user configured rendermodel here. Defaults to "vr_controller_vive_1_5".
            ssRetVal << m_strRenderModel.c_str();
            break;

        case vr::Prop_ManufacturerName_String:
            ssRetVal << "LeapMotion";
            break;

        case vr::Prop_ModelNumber_String:
            ssRetVal << "Controller";
            break;

        case vr::Prop_TrackingFirmwareVersion_String:
            ssRetVal << "cd.firmware_revision=" << m_firmware_revision;
            break;

        case vr::Prop_HardwareRevision_String:
            ssRetVal << "cd.hardware_revision=" << m_hardware_revision;
            break;
    }

    std::string sRetVal = ssRetVal.str();
    if(sRetVal.empty())
    {
        *pError = vr::TrackedProp_ValueNotProvidedByDevice;
        return 0;
    }
    else if(sRetVal.size() + 1 > unBufferSize)
    {
        *pError = vr::TrackedProp_BufferTooSmall;
        return sRetVal.size() + 1;  // caller needs to know how to size buffer
    }
    else
    {
        _snprintf(pchValue, unBufferSize, sRetVal.c_str());
        *pError = vr::TrackedProp_Success;
        return sRetVal.size() + 1;
    }
}

void CLeapHmdLatest::EnterStandby()
{
    CDriverLogHelper::DriverLog("CLeapHmdLatest::EnterStandby()\n");
}

vr::VRControllerState_t CLeapHmdLatest::GetControllerState()
{
    // This is only called at startup to synchronize with the driver.
    // Future updates are driven by our thread calling TrackedDeviceButton*() and TrackedDeviceAxis*()
    return vr::VRControllerState_t();
}

bool CLeapHmdLatest::TriggerHapticPulse(uint32_t unAxisId, uint16_t usPulseDurationMicroseconds)
{
    return true;  // handled -- returning false will cause errors to come out of vrserver
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
    vr::VRControllerState_t NewState = { 0 };

    bool handFound = false;
    GestureMatcher::WhichHand which = (m_nId == LEFT_CONTROLLER) ? GestureMatcher::LeftHand :
        (m_nId == RIGHT_CONTROLLER) ? GestureMatcher::RightHand :
        GestureMatcher::AnyHand;

    float scores[GestureMatcher::NUM_GESTURES];
    handFound = GestureMatcher::MatchGestures(frame, which, scores);

    if(handFound)
    {
        // Changing unPacketNum tells anyone polling state that something might have
        // changed.  We don't try to be precise about that here.
        NewState.unPacketNum = m_ControllerState.unPacketNum + 1;

        // system menu mapping (timeout gesture)
        if(scores[GestureMatcher::Timeout] >= 0.5f)
            NewState.ulButtonTouched |= vr::ButtonMaskFromId(vr::k_EButton_System);
        if(scores[GestureMatcher::Timeout] >= 0.5f)
            NewState.ulButtonPressed |= vr::ButtonMaskFromId(vr::k_EButton_System);

        // application menu mapping (Flat hand towards your face gesture)
        if(scores[GestureMatcher::FlatHandPalmTowards] >= 0.8f)
            NewState.ulButtonTouched |= vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu);
        if(scores[GestureMatcher::FlatHandPalmTowards] >= 0.8f)
            NewState.ulButtonPressed |= vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu);

        // digital trigger mapping (fist clenching gesture)
        if(scores[GestureMatcher::TriggerFinger] > 0.5f)
            NewState.ulButtonTouched |= vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger);
        if(scores[GestureMatcher::TriggerFinger] > 0.5f)
            NewState.ulButtonPressed |= vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger);

        // grip mapping (clench fist with middle, index, pinky fingers)
        if(scores[GestureMatcher::LowerFist] >= 0.5f)
            NewState.ulButtonTouched |= vr::ButtonMaskFromId(vr::k_EButton_Grip);
        if(scores[GestureMatcher::LowerFist] >= 0.5f)
            NewState.ulButtonPressed |= vr::ButtonMaskFromId(vr::k_EButton_Grip);

        // touchpad button press mapping (Thumbpress gesture)
        if(scores[GestureMatcher::Thumbpress] >= 0.2f)
            NewState.ulButtonTouched |= vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad);
        if(scores[GestureMatcher::Thumbpress] >= 1.0f)
            NewState.ulButtonPressed |= vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad);

#if 0
        // sixense driver seems to have good deadzone, but add a small one here
        if (fabsf(cd.joystick_x) > 0.03f || fabsf(cd.joystick_y) > 0.03f)
            NewState.ulButtonTouched |= vr::ButtonMaskFromId(vr::k_EButton_StreamVR_Touchpad);
#endif

        // All pressed buttons are touched
        NewState.ulButtonTouched |= NewState.ulButtonPressed;

        uint64_t ulChangedTouched = NewState.ulButtonTouched ^ m_ControllerState.ulButtonTouched;
        uint64_t ulChangedPressed = NewState.ulButtonPressed ^ m_ControllerState.ulButtonPressed;

        SendButtonUpdates(&vr::IServerDriverHost::TrackedDeviceButtonTouched, ulChangedTouched & NewState.ulButtonTouched);
        SendButtonUpdates(&vr::IServerDriverHost::TrackedDeviceButtonPressed, ulChangedPressed & NewState.ulButtonPressed);
        SendButtonUpdates(&vr::IServerDriverHost::TrackedDeviceButtonUnpressed, ulChangedPressed & ~NewState.ulButtonPressed);
        SendButtonUpdates(&vr::IServerDriverHost::TrackedDeviceButtonUntouched, ulChangedTouched & ~NewState.ulButtonTouched);

        NewState.rAxis[0].x = scores[GestureMatcher::TouchpadAxisX];
        NewState.rAxis[0].y = scores[GestureMatcher::TouchpadAxisY];

        NewState.rAxis[1].x = scores[GestureMatcher::TriggerFinger];
        NewState.rAxis[1].y = 0.0f;

        // the touchpad maps to Axis 0 X/Y
        if(NewState.rAxis[0].x != m_ControllerState.rAxis[0].x || NewState.rAxis[0].y != m_ControllerState.rAxis[0].y)
            m_pDriverHost->TrackedDeviceAxisUpdated(m_unSteamVRTrackedDeviceId, 0, NewState.rAxis[0]);

        // trigger maps to Axis 1 X
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

        // controller #0 is supposed to be the left hand, controller #1 the right one.
        if(hand.isValid() && (m_nId == LEFT_CONTROLLER && hand.isLeft() ||
            m_nId == RIGHT_CONTROLLER && hand.isRight()))
        {
            handFound = true;

            // The "driver" coordinate system is the one that vecPosition is in.  This is whatever
            // coordinates the driver naturally produces for position and orientation.  The "world"
            // coordinate system is the one that is presented to vrserver.  This should include
            // fixing any tilt to the world (caused by a tilted camera, for example) and can include
            // any other useful transformation for the driver (e.g. the driver is tracking from a
            // secondary camera, but uses this transform to move this object into the primary camera
            // coordinate system to be consistent with other objects).
            //
            // This transform is multiplied on the left of the predicted "driver" pose.  That becomes
            // the vr::TrackingUniverseRawAndUncalibrated origin, which is then further offset for
            // floor height and tracking space center by the chaperone system to produce both the
            // vr::TrackingUniverseSeated and vr::TrackingUniverseStanding spaces.
            //
            // In the leap driver, we use it to unify our coordinate system with the HMD.
            m_Pose.qWorldFromDriverRotation = m_hmdRot;
            m_Pose.vecWorldFromDriverTranslation[0] = m_hmdPos[0];
            m_Pose.vecWorldFromDriverTranslation[1] = m_hmdPos[1];
            m_Pose.vecWorldFromDriverTranslation[2] = m_hmdPos[2];

            // The "head" coordinate system defines a natural point for the object.  While the "driver"
            // space may be chosen for mechanical, eletrical, or mathematical convenience (e.g. being
            // the location of the IMU), the "head" should be a point meaningful to the user.  For HMDs,
            // it's the point directly between the user's eyes.  The origin of this coordinate system
            // is the origin used for the rendermodel.
            //
            // This transform is multiplied on the right side of the "driver" pose.
            //
            // This transform was inadvertently left at identity for the GDC 2015 controllers, creating
            // a defacto standard "head" position for controllers at the location of the IMU for that
            // particular controller.  We will remedy that later by adding other, explicitly named and
            // chosen spaces.  For now, mimicking that point in this driver lets us run content authored
            // for the HTC Vive Developer Edition controller.  This was done by loading an existing
            // controller rendermodel along side the Leap model and rotating the Leap model to roughly
            // align the main features like the handle and trigger.
            m_Pose.qDriverFromHeadRotation.w = 1;
            m_Pose.qDriverFromHeadRotation.x = 0; //  -m_hmdRot.x;   this would cancel out the HMD's rotation
            m_Pose.qDriverFromHeadRotation.y = 0; //  -m_hmdRot.y;   but instead we rely on the Leap Motion to
            m_Pose.qDriverFromHeadRotation.z = 0; //  -m_hmdRot.z;   update the hand rotation as the head rotates
            m_Pose.vecDriverFromHeadTranslation[0] = 0;
            m_Pose.vecDriverFromHeadTranslation[1] = 0;
            m_Pose.vecDriverFromHeadTranslation[2] = 0;

            Leap::Vector position = hand.palmPosition();

            m_Pose.vecPosition[0] = -0.001*position.x;
            m_Pose.vecPosition[1] = -0.001*position.z;
            m_Pose.vecPosition[2] = -0.001*position.y - 0.15; // assume 15 cm offset from midpoint between eys

            Leap::Vector velocity = hand.palmVelocity();

            m_Pose.vecVelocity[0] = -0.001*velocity.x;
            m_Pose.vecVelocity[1] = -0.001*velocity.z;
            m_Pose.vecVelocity[2] = -0.001*velocity.y;

            // Unmeasured.  XXX we currently leave the acceleration at zero
            m_Pose.vecAcceleration[0] = 0.0;
            m_Pose.vecAcceleration[1] = 0.0;
            m_Pose.vecAcceleration[2] = 0.0;

            // get two vectors describing the hand's orientation in space. We need to find a rotation
            // matrix that turns the default coordinate system into the hand's coordinate system
            Leap::Vector direction = hand.direction(); direction /= direction.magnitude();
            Leap::Vector normal = hand.palmNormal(); normal /= normal.magnitude();
            Leap::Vector side = direction.cross(normal);

#if 0
            // This code assumes palms are facing downwards.

            // NOTE: y and z are swapped with respect to the Leap Motion's coordinate system and I list
            //       the vectors in the order in which I expect them to be in the tracking camera's
            //       coordinates system: X = sideways,
            //                           Y = up/down i.e. palm's normal vector
            //                           Z = front/back i.e. hand's pointing direction
            m_Pose.qRotation = CalculateRotation(R);

            float R[3][3] =
            { { side.x,      side.z,      side.y },
            { normal.x,    normal.z,    normal.y },
            { direction.x, direction.z, direction.y } };

#else
            // This code assumes palms are facing inwards as if you were holding controllers.
            // This is why the left hand and the
            // right hands have to use different matrices to compute their rotations.

            // now turn this into a Quaternion and we're done.
            if(m_nId == LEFT_CONTROLLER)
            {
                float L[3][3] =
                { { -normal.x, -normal.z, -normal.y },
                { side.x, side.z, side.y },
                { direction.x, direction.z, direction.y } };
                m_Pose.qRotation = CalculateRotation(L);
            }
            else if(m_nId == RIGHT_CONTROLLER)
            {
                float R[3][3] =
                { { normal.x, normal.z, normal.y },
                { -side.x, -side.z, -side.y },
                { direction.x, direction.z, direction.y } };
                m_Pose.qRotation = CalculateRotation(R);
            }

#endif
            // rotate by the specified grip angle (may be useful when using the Vive as a gun grip)
            if(m_gripAngleOffset != 0)
                m_Pose.qRotation = rotate_around_axis(Leap::Vector(1.0, 0.0, 0.0), m_gripAngleOffset) * m_Pose.qRotation;

            // Unmeasured.  XXX with no angular velocity, throwing might not work in some games
            m_Pose.vecAngularVelocity[0] = 0.0;
            m_Pose.vecAngularVelocity[1] = 0.0;
            m_Pose.vecAngularVelocity[2] = 0.0;

            // The same argument applies here as to vecAcceleration, and a driver is even
            // less likely to have a valid value for it (since gyros measure angular velocity)
            m_Pose.vecAngularAcceleration[0] = 0.0;
            m_Pose.vecAngularAcceleration[1] = 0.0;
            m_Pose.vecAngularAcceleration[2] = 0.0;

            // this results in the controllers being shown on screen
            m_Pose.result = vr::TrackingResult_Running_OK;

            // the pose validity also depends on HMD tracking data sent to us by the leap_monitor.exe
            m_Pose.poseIsValid = m_bCalibrated;
        }
    }

    if(!handFound)
    {
        m_Pose.result = vr::TrackingResult_Running_OutOfRange;
        m_Pose.poseIsValid = false;
    }

    // This is very hard to know with this driver, but CServerDriver_Leap::ThreadFunc
    // tries to reduce latency as much as possible.  There is processing in the Leap Motion SDK,
    // though, which causes additional unknown latency.  This time is used to know how much
    // extrapolation (via velocity and angular velocity) should be done when predicting poses.
    m_Pose.poseTimeOffset = -0.016f;

    // when we get here, the Leap Motion is connected
    m_Pose.deviceIsConnected = true;

    // These should always be false from any modern driver.  These are for Oculus DK1-like
    // rotation-only tracking.  Support for that has likely rotted in vrserver.
    m_Pose.willDriftInYaw = false;
    m_Pose.shouldApplyHeadModel = false;

    // This call posts this pose to shared memory, where all clients will have access to it the next
    // moment they want to predict a pose.
    m_pDriverHost->TrackedDevicePoseUpdated(m_unSteamVRTrackedDeviceId, m_Pose);
}

bool CLeapHmdLatest::IsActivated() const
{
    return m_unSteamVRTrackedDeviceId != vr::k_unTrackedDeviceIndexInvalid;
}

bool CLeapHmdLatest::HasControllerId(int nBase, int nId) const
{
    return nBase == m_nBase && nId == m_nId;
}

/** Process sixenseControllerData.  Return true if it's new to help caller manage sleep durations */
bool CLeapHmdLatest::Update(Leap::Frame &frame)
{
    UpdateTrackingState(frame);
    UpdateControllerState(frame);

    return true;
}

// Alignment of the coordinate system of driver_leap with the HMD:
void CLeapHmdLatest::RealignCoordinates(CLeapHmdLatest* pLeapA, CLeapHmdLatest* pLeapB)
{
    if(pLeapA->m_unSteamVRTrackedDeviceId == vr::k_unTrackedDeviceIndexInvalid)
        return;

    pLeapA->m_pAlignmentPartner = pLeapB;
    pLeapB->m_pAlignmentPartner = pLeapA;

    // Ask leap_monitor to tell us HMD pose
    static vr::VREvent_Data_t nodata = { 0 };
    pLeapA->m_pDriverHost->VendorSpecificEvent(pLeapA->m_unSteamVRTrackedDeviceId,
        (vr::EVREventType) (vr::VREvent_VendorSpecific_Reserved_Start + 0), nodata,
        -std::chrono::duration_cast<std::chrono::seconds>(k_TrackingLatency).count());
}

// leap_monitor called us back with the HMD information
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