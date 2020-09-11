#include "stdafx.h"

#include "CLeapController.h"
#include "CControllerButton.h"

#include "CDriverConfig.h"
#include "CGestureMatcher.h"
#include "Utils.h"

const glm::quat g_reverseRotation(0.f, 0.f, 0.70106769f, -0.70106769f);
const glm::quat g_rotateHalfPiZ(0.70106769f, 0.f, 0.f, 0.70106769f);
const glm::quat g_rotateHalfPiZN(0.70106769f, 0.f, 0.f, -0.70106769f);
const vr::HmdQuaternion_t g_vrZeroRotation = { 1.0, .0, .0, .0 };

double CLeapController::ms_headPosition[] = { .0, .0, .0 };
vr::HmdQuaternion_t CLeapController::ms_headRotation = { 1.0, .0, .0, .0 };

CLeapController::CLeapController()
{
    m_haptic = vr::k_ulInvalidPropertyContainer;
    m_propertyContainer = vr::k_ulInvalidPropertyContainer;
    m_trackedDevice = vr::k_unTrackedDeviceIndexInvalid;

    m_pose = { 0 };
    m_pose.deviceIsConnected = false;
    for(size_t i = 0U; i < 3U; i++)
    {
        m_pose.vecAcceleration[i] = .0;
        m_pose.vecAngularAcceleration[i] = .0;
        m_pose.vecAngularVelocity[i] = .0;
        m_pose.vecDriverFromHeadTranslation[i] = .0;
    }
    m_pose.poseTimeOffset = -0.016;
    m_pose.qDriverFromHeadRotation = g_vrZeroRotation;
    m_pose.qRotation = g_vrZeroRotation;
    m_pose.qWorldFromDriverRotation = g_vrZeroRotation;
    m_pose.result = vr::TrackingResult_Uninitialized;
    m_pose.shouldApplyHeadModel = false;
    m_pose.willDriftInYaw = false;

    m_hand = CH_Left;
    m_gameProfile = GP_Default;
}
CLeapController::~CLeapController()
{
    for(auto l_button : m_buttons) delete l_button;
}

// vr::ITrackedDeviceServerDriver
vr::EVRInitError CLeapController::Activate(uint32_t unObjectId)
{
    vr::EVRInitError l_resultError = vr::VRInitError_Driver_Failed;

    if(m_trackedDevice == vr::k_unTrackedDeviceIndexInvalid)
    {
        m_trackedDevice = unObjectId;
        m_propertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer(m_trackedDevice);

        ActivateInternal();

        l_resultError = vr::VRInitError_None;
    }

    return l_resultError;
}
void CLeapController::Deactivate()
{
    ResetControls();
    m_trackedDevice = vr::k_unTrackedDeviceIndexInvalid;
}

void* CLeapController::GetComponent(const char* pchComponentNameAndVersion)
{
    void *l_result = nullptr;
    if(!strcmp(pchComponentNameAndVersion, vr::ITrackedDeviceServerDriver_Version)) l_result = dynamic_cast<vr::ITrackedDeviceServerDriver*>(this);
    return l_result;
}

vr::DriverPose_t CLeapController::GetPose()
{
    return m_pose;
}

// CLeapController
bool CLeapController::GetEnabled() const
{
    return m_pose.deviceIsConnected;
}

void CLeapController::SetEnabled(bool f_state)
{
    m_pose.deviceIsConnected = f_state;
    if(m_trackedDevice != vr::k_unTrackedDeviceIndexInvalid) vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_trackedDevice, m_pose, sizeof(vr::DriverPose_t));
}

void CLeapController::ResetControls()
{
    for(auto l_button : m_buttons)
    {
        l_button->SetValue(0.f);
        l_button->SetState(false);
    }
}

void CLeapController::SetGameProfile(GameProfile f_profile)
{
    if(m_gameProfile != f_profile)
    {
        m_gameProfile = f_profile;
        ResetControls();
    }
}

void CLeapController::RunFrame(const Leap::Frame &f_frame)
{
    if(m_trackedDevice != vr::k_unTrackedDeviceIndexInvalid)
    {
        if(m_pose.deviceIsConnected)
        {
            UpdateTransformation(f_frame);
            vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_trackedDevice, m_pose, sizeof(vr::DriverPose_t));

            UpdateGestures(f_frame);
            UpdateInput();
        }
        else vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_trackedDevice, m_pose, sizeof(vr::DriverPose_t));
    }
}

void CLeapController::UpdateInput()
{
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
    UpdateInputInternal();
}

void CLeapController::UpdateTransformation(const Leap::Frame &f_frame)
{
    bool l_handFound = false;
    const Leap::HandList l_hands = f_frame.hands();
    for(const auto l_hand : l_hands)
    {
        if(l_hand.isValid())
        {
            if(((m_hand == CH_Left) && l_hand.isLeft()) || ((m_hand == CH_Right) && l_hand.isRight()))
            {
                switch(CDriverConfig::GetOrientationMode())
                {
                    case CDriverConfig::OM_HMD:
                    {
                        std::memcpy(m_pose.vecWorldFromDriverTranslation, ms_headPosition, sizeof(double) * 3U);
                        std::memcpy(&m_pose.qWorldFromDriverRotation, &ms_headRotation, sizeof(vr::HmdQuaternion_t));

                        const Leap::Vector l_position = l_hand.palmPosition();
                        const glm::vec3 &l_handOffset = ((m_hand == CH_Left) ? CDriverConfig::GetLeftHandOffset() : CDriverConfig::GetRightHandOffset());
                        m_pose.vecPosition[0] = -0.001f*l_position.x + l_handOffset.x;
                        m_pose.vecPosition[1] = -0.001f*l_position.z + l_handOffset.y;
                        m_pose.vecPosition[2] = -0.001f*l_position.y + l_handOffset.z;

                        const Leap::Vector l_velocity = l_hand.palmVelocity();
                        glm::vec3 l_resultVelocity(-0.001f*l_velocity.x, -0.001f*l_velocity.z, -0.001f*l_velocity.y);
                        const glm::quat l_headRotation(ms_headRotation.w, ms_headRotation.x, ms_headRotation.y, ms_headRotation.z);
                        l_resultVelocity = l_headRotation*l_resultVelocity;
                        m_pose.vecVelocity[0] = l_resultVelocity.x;
                        m_pose.vecVelocity[1] = l_resultVelocity.y;
                        m_pose.vecVelocity[2] = l_resultVelocity.z;

                        const Leap::Quaternion l_handOrientation = l_hand.orientation();
                        glm::quat l_rotation(l_handOrientation.w, l_handOrientation.x, l_handOrientation.y, l_handOrientation.z);
                        l_rotation = g_reverseRotation*l_rotation;
                        l_rotation *= ((m_hand == CH_Left) ? g_rotateHalfPiZN : g_rotateHalfPiZ);
                        l_rotation *= ((m_hand == CH_Left) ? CDriverConfig::GetLeftHandOffsetRotation() : CDriverConfig::GetRightHandOffsetRotation());

                        m_pose.qRotation.x = l_rotation.x;
                        m_pose.qRotation.y = l_rotation.y;
                        m_pose.qRotation.z = l_rotation.z;
                        m_pose.qRotation.w = l_rotation.w;
                    } break;
                    case CDriverConfig::OM_Desktop:
                    {
                        // Controller follows HMD position only
                        std::memcpy(m_pose.vecWorldFromDriverTranslation, ms_headPosition, sizeof(double) * 3U);
                        std::memcpy(&m_pose.qWorldFromDriverRotation, &g_vrZeroRotation, sizeof(vr::HmdQuaternion_t));

                        const glm::vec3 &l_offset = CDriverConfig::GetDesktopOffset();
                        m_pose.vecWorldFromDriverTranslation[0U] += l_offset.x;
                        m_pose.vecWorldFromDriverTranslation[1U] += l_offset.y;
                        m_pose.vecWorldFromDriverTranslation[2U] += l_offset.z;

                        const Leap::Vector l_position = l_hand.palmPosition();
                        const glm::vec3 &l_handOffset = ((m_hand == CH_Left) ? CDriverConfig::GetLeftHandOffset() : CDriverConfig::GetRightHandOffset());
                        m_pose.vecPosition[0] = 0.001f*l_position.x + l_handOffset.x;
                        m_pose.vecPosition[1] = 0.001f*l_position.y + l_handOffset.y;
                        m_pose.vecPosition[2] = 0.001f*l_position.z + l_handOffset.z;

                        const Leap::Vector l_velocity = l_hand.palmVelocity();
                        m_pose.vecVelocity[0] = 0.001f*l_velocity.x;
                        m_pose.vecVelocity[1] = 0.001f*l_velocity.y;
                        m_pose.vecVelocity[2] = 0.001f*l_velocity.z;

                        const Leap::Quaternion l_handOrientation = l_hand.orientation();
                        glm::quat l_rotation(l_handOrientation.w, l_handOrientation.x, l_handOrientation.y, l_handOrientation.z);
                        l_rotation *= ((m_hand == CH_Left) ? g_rotateHalfPiZN : g_rotateHalfPiZ);
                        l_rotation *= ((m_hand == CH_Left) ? CDriverConfig::GetLeftHandOffsetRotation() : CDriverConfig::GetRightHandOffsetRotation());

                        m_pose.qRotation.x = l_rotation.x;
                        m_pose.qRotation.y = l_rotation.y;
                        m_pose.qRotation.z = l_rotation.z;
                        m_pose.qRotation.w = l_rotation.w;
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

        if(CDriverConfig::IsHandsResetEnabled()) m_pose.result = vr::TrackingResult_Running_OutOfRange;
        else
        {
            m_pose.result = vr::TrackingResult_Running_OK;
            l_handFound = true;
        }
    }

    m_pose.poseIsValid = l_handFound;
}

void CLeapController::UpdateHMDCoordinates()
{
    vr::TrackedDevicePose_t l_hmdPose;
    vr::VRServerDriverHost()->GetRawTrackedDevicePoses(0.f, &l_hmdPose, 1U); // HMD has device ID 0
    if(l_hmdPose.bPoseIsValid)
    {
        glm::mat4 l_rotMat(1.f);
        ConvertMatrix(l_hmdPose.mDeviceToAbsoluteTracking, l_rotMat);

        const glm::quat l_headRot = glm::quat_cast(l_rotMat);
        ms_headRotation.x = l_headRot.x;
        ms_headRotation.y = l_headRot.y;
        ms_headRotation.z = l_headRot.z;
        ms_headRotation.w = l_headRot.w;

        for(size_t i = 0U; i < 3U; i++) ms_headPosition[i] = l_hmdPose.mDeviceToAbsoluteTracking.m[i][3];
    }
}
