#include "stdafx.h"

#include "CLeapController.h"
#include "CControllerButton.h"

#include "CDriverConfig.h"
#include "CGestureMatcher.h"
#include "Utils.h"

vr::IVRServerDriverHost *CLeapController::ms_driverHost = nullptr;
vr::IVRDriverInput *CLeapController::ms_driverInput = nullptr;
double CLeapController::ms_headPosition[] = { .0, .0, .0 };
vr::HmdQuaternion_t CLeapController::ms_headRotation = { 1.0, .0, .0, .0 };
vr::CVRPropertyHelpers *CLeapController::ms_propertyHelpers = nullptr;

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
    m_pose.qDriverFromHeadRotation = { 1.0, .0, .0, .0 };
    m_pose.result = vr::TrackingResult_Uninitialized;
    m_pose.shouldApplyHeadModel = false;
    m_pose.willDriftInYaw = false;

    m_controllerHand = CH_Left;
    m_gameProfile = GP_Default;
    m_gripOffset = glm::quat(1.f, 0.f, 0.f, 0.f);
    m_specialMode = false;
}
CLeapController::~CLeapController()
{
    for(auto l_button : m_buttons) delete l_button;

}

// vr::ITrackedDeviceServerDriver
void CLeapController::Deactivate()
{
    ResetControls();
    m_trackedDevice = vr::k_unTrackedDeviceIndexInvalid;
}

void* CLeapController::GetComponent(const char* pchComponentNameAndVersion)
{
    void *l_result = nullptr;
    if(!strcmp(pchComponentNameAndVersion, vr::ITrackedDeviceServerDriver_Version)) l_result = this;
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
    if(m_trackedDevice != vr::k_unTrackedDeviceIndexInvalid) ms_driverHost->TrackedDevicePoseUpdated(m_trackedDevice, m_pose, sizeof(vr::DriverPose_t));
}

bool CLeapController::MixHandState(bool f_state)
{
    return f_state;
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

void CLeapController::SwitchSpecialMode()
{
    m_specialMode = !m_specialMode;
}

void CLeapController::Update(const Leap::Frame &f_frame)
{
    if(m_trackedDevice != vr::k_unTrackedDeviceIndexInvalid)
    {
        if(m_pose.deviceIsConnected)
        {
            UpdateTransformation(f_frame);
            ms_driverHost->TrackedDevicePoseUpdated(m_trackedDevice, m_pose, sizeof(vr::DriverPose_t));

            UpdateGestures(f_frame);
            UpdateInput();
        }
        else ms_driverHost->TrackedDevicePoseUpdated(m_trackedDevice, m_pose, sizeof(vr::DriverPose_t));
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
                    ms_driverInput->UpdateBooleanComponent(l_button->GetHandle(), l_button->GetState(), .0);
                    break;
                case CControllerButton::IT_Float:
                    ms_driverInput->UpdateScalarComponent(l_button->GetHandle(), l_button->GetValue(), .0);
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
    const Leap::HandList &l_hands = f_frame.hands();
    for(auto l_hand : l_hands)
    {
        if(l_hand.isValid())
        {
            if(((m_controllerHand == CH_Left) && l_hand.isLeft()) || ((m_controllerHand == CH_Right) && l_hand.isRight()))
            {
                switch(CDriverConfig::GetOrientationMode())
                {
                    case CDriverConfig::OM_HMD:
                    {
                        std::memcpy(&m_pose.qWorldFromDriverRotation, &ms_headRotation, sizeof(vr::HmdQuaternion_t));
                        std::memcpy(m_pose.vecWorldFromDriverTranslation, ms_headPosition, sizeof(double) * 3U);

                        Leap::Vector l_position = l_hand.palmPosition();
                        m_pose.vecPosition[0] = -0.001*l_position.x;
                        m_pose.vecPosition[1] = -0.001*l_position.z;
                        m_pose.vecPosition[2] = -0.001*l_position.y - 0.15; // ?

                        Leap::Vector l_velocity = l_hand.palmVelocity();
                        m_pose.vecVelocity[0] = -0.001*l_velocity.x;
                        m_pose.vecVelocity[1] = -0.001*l_velocity.z;
                        m_pose.vecVelocity[2] = -0.001*l_velocity.y;

                        Leap::Vector l_handDirection = l_hand.direction();
                        l_handDirection /= l_handDirection.magnitude();

                        Leap::Vector l_palmNormal = l_hand.palmNormal();
                        l_palmNormal /= l_palmNormal.magnitude();

                        Leap::Vector l_leapCross = l_handDirection.cross(l_palmNormal);
                        switch(m_controllerHand)
                        {
                            case CH_Left:
                                l_palmNormal *= -1.f;
                                break;
                            case CH_Right:
                                l_leapCross *= -1.f;
                                break;
                        }

                        glm::mat3 l_rotMat(
                            l_palmNormal.x, l_palmNormal.z, l_palmNormal.y,
                            l_leapCross.x, l_leapCross.z, l_leapCross.y,
                            l_handDirection.x, l_handDirection.z, l_handDirection.y
                            );
                        glm::quat l_finalRot = glm::quat_cast(l_rotMat);
                        l_finalRot *= m_gripOffset;

                        m_pose.qRotation.x = l_finalRot.x;
                        m_pose.qRotation.y = l_finalRot.y;
                        m_pose.qRotation.z = l_finalRot.z;
                        m_pose.qRotation.w = l_finalRot.w;
                    } break;
                    case CDriverConfig::OM_Desktop:
                    {
                        // Controller follows HMD position only
                        std::memcpy(m_pose.vecWorldFromDriverTranslation, ms_headPosition, sizeof(double) * 3U);

                        Leap::Vector l_position = l_hand.palmPosition();
                        m_pose.vecPosition[0] = 0.001*l_position.x + CDriverConfig::GetDesktopRootX();
                        m_pose.vecPosition[1] = 0.001*l_position.y + CDriverConfig::GetDesktopRootY();
                        m_pose.vecPosition[2] = 0.001*l_position.z + CDriverConfig::GetDesktopRootZ();

                        Leap::Vector l_velocity = l_hand.palmVelocity();
                        m_pose.vecVelocity[0] = 0.001*l_velocity.x;
                        m_pose.vecVelocity[1] = 0.001*l_velocity.y;
                        m_pose.vecVelocity[2] = 0.001*l_velocity.z;

                        Leap::Vector l_handDirection = -l_hand.direction();
                        l_handDirection /= l_handDirection.magnitude();

                        Leap::Vector l_palmNormal = -l_hand.palmNormal();
                        l_palmNormal /= l_palmNormal.magnitude();

                        Leap::Vector l_leapCross = -l_handDirection.cross(l_palmNormal);
                        switch(m_controllerHand)
                        {
                            case CH_Left:
                                l_palmNormal *= -1.f;
                                break;
                            case CH_Right:
                                l_leapCross *= -1.f;
                                break;
                        }

                        glm::mat3 l_rotMat(
                            l_palmNormal.x, l_palmNormal.y, l_palmNormal.z,
                            l_leapCross.x, l_leapCross.y, l_leapCross.z,
                            l_handDirection.x, l_handDirection.y, l_handDirection.z
                            );
                        glm::quat l_finalRot = glm::quat_cast(l_rotMat);
                        l_finalRot *= m_gripOffset;

                        m_pose.qRotation.x = l_finalRot.x;
                        m_pose.qRotation.y = l_finalRot.y;
                        m_pose.qRotation.z = l_finalRot.z;
                        m_pose.qRotation.w = l_finalRot.w;
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
        m_pose.result = vr::TrackingResult_Running_OutOfRange;
    }
    m_pose.poseIsValid = MixHandState(l_handFound);
}

void CLeapController::SetInterfaces(vr::IVRServerDriverHost *f_host, vr::IVRDriverInput *f_input, vr::CVRPropertyHelpers *f_helpers)
{
    ms_driverHost = f_host;
    ms_driverInput = f_input;
    ms_propertyHelpers = f_helpers;
}

void CLeapController::UpdateHMDCoordinates()
{
    vr::TrackedDevicePose_t l_hmdPose;
    ms_driverHost->GetRawTrackedDevicePoses(0.f, &l_hmdPose, 1U); // HMD has device ID 0
    if(l_hmdPose.bPoseIsValid)
    {
        glm::mat4 l_rotMat(1.f);
        ConvertMatrix(l_hmdPose.mDeviceToAbsoluteTracking, l_rotMat);
        glm::quat l_headRot = glm::quat_cast(l_rotMat);
        ms_headRotation.x = l_headRot.x;
        ms_headRotation.y = l_headRot.y;
        ms_headRotation.z = l_headRot.z;
        ms_headRotation.w = l_headRot.w;

        for(size_t i = 0U; i < 3U; i++) ms_headPosition[i] = l_hmdPose.mDeviceToAbsoluteTracking.m[i][3];
    }
}
