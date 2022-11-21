#include "stdafx.h"

#include "Devices/CLeapController/CLeapController.h"
#include "Devices/CLeapController/CControllerButton.h"

#include "Core/CDriverConfig.h"
#include "Utils/Utils.h"

const glm::quat g_reverseRotation(0.f, 0.f, 0.70106769f, -0.70106769f);
const glm::quat g_rotateHalfPiZ(0.70106769f, 0.f, 0.f, 0.70106769f);
const glm::quat g_rotateHalfPiZN(0.70106769f, 0.f, 0.f, -0.70106769f);
const vr::HmdQuaternion_t g_vrZeroRotation = { 1.0, .0, .0, .0 };
const glm::vec3 g_positionInversions[] =
{
    { 1.f, 1.f, 1.f },
    { -1.f, 1.f, 1.f }
};
const glm::vec3 g_rotationInversions[] =
{
    { 1.f, 1.f, 1.f },
    { 1.f, -1.f, -1.f }
};

double CLeapController::ms_headPosition[] = { .0, .0, .0 };
vr::HmdQuaternion_t CLeapController::ms_headRotation = { 1.0, .0, .0, .0 };

CLeapController::CLeapController()
{
    m_propertyContainer = vr::k_ulInvalidPropertyContainer;
    m_trackedDevice = vr::k_unTrackedDeviceIndexInvalid;
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
    m_pose.qDriverFromHeadRotation = g_vrZeroRotation;
    m_pose.qRotation = g_vrZeroRotation;
    m_pose.qWorldFromDriverRotation = g_vrZeroRotation;
    m_pose.result = vr::TrackingResult_Uninitialized;
    m_pose.shouldApplyHeadModel = false;
    m_pose.willDriftInYaw = false;

    m_hand = CH_Left;
    m_type = CT_Invalid;
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

void CLeapController::EnterStandby()
{
}

void* CLeapController::GetComponent(const char* pchComponentNameAndVersion)
{
    void *l_result = nullptr;
    if(!strcmp(pchComponentNameAndVersion, vr::ITrackedDeviceServerDriver_Version)) l_result = dynamic_cast<vr::ITrackedDeviceServerDriver*>(this);
    return l_result;
}

void CLeapController::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
{
}

vr::DriverPose_t CLeapController::GetPose()
{
    return m_pose;
}

// CLeapController
bool CLeapController::IsEnabled() const
{
    return m_pose.deviceIsConnected;
}

void CLeapController::SetEnabled(bool p_state)
{
    m_pose.deviceIsConnected = p_state;
    if(m_trackedDevice != vr::k_unTrackedDeviceIndexInvalid) vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_trackedDevice, m_pose, sizeof(vr::DriverPose_t));
}

const std::string& CLeapController::GetSerialNumber() const
{
    return m_serialNumber;
}

void CLeapController::ResetControls()
{
    for(auto l_button : m_buttons)
    {
        l_button->SetValue(0.f);
        l_button->SetState(false);
    }
}

void CLeapController::RunFrame(const LEAP_HAND *p_hand)
{
    if(m_trackedDevice != vr::k_unTrackedDeviceIndexInvalid)
    {
        if(m_pose.deviceIsConnected)
        {
            UpdateTransformation(p_hand);
            vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_trackedDevice, m_pose, sizeof(vr::DriverPose_t));

            UpdateGestures(p_hand);
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

void CLeapController::UpdateTransformation(const LEAP_HAND *p_hand)
{
    m_pose.poseIsValid = (p_hand != nullptr);

    if(p_hand)
    {
        std::memcpy(m_pose.vecWorldFromDriverTranslation, ms_headPosition, sizeof(double) * 3U);
        std::memcpy(&m_pose.qWorldFromDriverRotation, &ms_headRotation, sizeof(vr::HmdQuaternion_t));

        // Root offset
        glm::quat l_headRot(ms_headRotation.w, ms_headRotation.x, ms_headRotation.y, ms_headRotation.z);
        glm::vec3 l_rootLocalOffset = l_headRot * CDriverConfig::GetRootOffset();

        m_pose.vecWorldFromDriverTranslation[0] += l_rootLocalOffset.x;
        m_pose.vecWorldFromDriverTranslation[1] += l_rootLocalOffset.y;
        m_pose.vecWorldFromDriverTranslation[2] += l_rootLocalOffset.z;

        // Root angle
        glm::quat l_appliedRot = l_headRot * glm::quat(glm::vec3(CDriverConfig::GetRootAngle(), 0.f, 0.f));

        m_pose.qWorldFromDriverRotation.w = l_appliedRot.w;
        m_pose.qWorldFromDriverRotation.x = l_appliedRot.x;
        m_pose.qWorldFromDriverRotation.y = l_appliedRot.y;
        m_pose.qWorldFromDriverRotation.z = l_appliedRot.z;

        // Velocity
        if(CDriverConfig::IsVelocityUsed())
        {
            glm::vec3 l_resultVelocity(-0.001f * p_hand->palm.velocity.x, -0.001f * p_hand->palm.velocity.z, -0.001f * p_hand->palm.velocity.y);
            l_resultVelocity = l_appliedRot * l_resultVelocity;
            m_pose.vecVelocity[0] = l_resultVelocity.x;
            m_pose.vecVelocity[1] = l_resultVelocity.y;
            m_pose.vecVelocity[2] = l_resultVelocity.z;
        }

        // Rotation
        glm::quat l_localRotation(p_hand->palm.orientation.w, p_hand->palm.orientation.x, p_hand->palm.orientation.y, p_hand->palm.orientation.z);
        l_localRotation = g_reverseRotation * l_localRotation;
        l_localRotation *= ((m_hand == CH_Left) ? g_rotateHalfPiZN : g_rotateHalfPiZ);
        l_localRotation = glm::normalize(l_localRotation);

        // Offset position
        glm::vec3 l_localPos(-0.001f * p_hand->arm.next_joint.x, -0.001f * p_hand->arm.next_joint.z, -0.001f * p_hand->arm.next_joint.y);
        l_localPos += (l_localRotation * (CDriverConfig::GetHandsOffset() * g_positionInversions[m_hand]));

        m_pose.vecPosition[0] = l_localPos.x;
        m_pose.vecPosition[1] = l_localPos.y;
        m_pose.vecPosition[2] = l_localPos.z;

        // Offset rotation
        l_localRotation = l_localRotation * (glm::quat(CDriverConfig::GetHandsRotationOffset() * g_rotationInversions[m_hand]));

        m_pose.qRotation.x = l_localRotation.x;
        m_pose.qRotation.y = l_localRotation.y;
        m_pose.qRotation.z = l_localRotation.z;
        m_pose.qRotation.w = l_localRotation.w;
        m_pose.result = vr::TrackingResult_Running_OK;
    }
    else
    {
        for(size_t i = 0U; i < 3U; i++) m_pose.vecVelocity[i] = .0;
        if(CDriverConfig::IsHandsResetEnabled()) m_pose.result = vr::TrackingResult_Running_OutOfRange;
        else
        {
            m_pose.result = vr::TrackingResult_Running_OK;
            m_pose.poseIsValid = true;
        }
    }
}

void CLeapController::ActivateInternal()
{
}

void CLeapController::UpdateGestures(const LEAP_HAND *p_hand)
{
}

void CLeapController::UpdateInputInternal()
{
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
