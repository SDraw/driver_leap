#include "stdafx.h"
#include "CLeapHandController.h"
#include "CControllerButton.h"
#include "CDriverConfig.h"
#include "CGestureMatcher.h"
#include "Utils.h"

extern char g_moduleFileName[];

const std::vector<std::string> g_gameProfiles
{
    "Default", "VRChat"
};

const std::vector<std::string> g_debugRequests
{
    "profile", "game"
};
enum DebugRequest : size_t
{
    DR_Profile = 0U,
    DR_Game
};

const std::vector<std::string> g_gameList
{
    "vrchat"
};
enum GameList : size_t
{
    GL_VRChat = 0U
};

const std::vector<std::string> g_gameCommands
{
    "drawing_mode" // VRChat
};
enum GameCommand : size_t
{
    CC_DrawingMode = 0U // VRChat
};

vr::IVRServerDriverHost *CLeapHandController::ms_driverHost = nullptr;
vr::IVRDriverInput *CLeapHandController::ms_driverInput = nullptr;
vr::CVRPropertyHelpers *CLeapHandController::ms_propertyHelpers = nullptr;
double CLeapHandController::ms_headPos[] = { .0, .0, .0 };
vr::HmdQuaternion_t CLeapHandController::ms_headRot = { 1.0, .0, .0, .0 };

CLeapHandController::CLeapHandController()
{
    m_trackedDeviceID = vr::k_unTrackedDeviceIndexInvalid;
    m_propertyContainer = vr::k_ulInvalidPropertyContainer;

    glm::vec3 l_eulerOffsetRot(CDriverConfig::GetRotationOffsetX(), CDriverConfig::GetRotationOffsetY(), CDriverConfig::GetRotationOffsetZ());
    if(m_handAssigment == CHA_Right)
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
}
CLeapHandController::~CLeapHandController()
{
    for(auto l_button : m_buttons) delete l_button;

}

void CLeapHandController::Deactivate()
{
    ResetControls();
    m_trackedDeviceID = vr::k_unTrackedDeviceIndexInvalid;
}

void* CLeapHandController::GetComponent(const char* pchComponentNameAndVersion)
{
    void *l_result = nullptr;
    if(!strcmp(pchComponentNameAndVersion, vr::ITrackedDeviceServerDriver_Version)) l_result = this;
    return l_result;
}

void CLeapHandController::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
{
    if(m_trackedDeviceID != vr::k_unTrackedDeviceIndexInvalid)
    {
        std::stringstream l_stream(pchRequest);
        std::string l_command;

        l_stream >> l_command;
        if(!l_stream.fail() && !l_command.empty())
        {
            switch(ReadEnumVector(l_command, g_debugRequests))
            {
                case DebugRequest::DR_Profile:
                {
                    std::string l_profile;
                    l_stream >> l_profile;

                    if(!l_stream.fail() && !l_profile.empty())
                    {
                        GameProfile l_newProfile;
                        switch(ReadEnumVector(l_profile, g_gameProfiles))
                        {
                            case GP_Default:
                                l_newProfile = GP_Default;
                                break;
                            case GP_VRChat:
                                l_newProfile = GP_VRChat;
                                break;
                            default:
                                l_newProfile = GP_Default;
                                break;
                        }
                        if(m_gameProfile != l_newProfile)
                        {
                            m_gameProfile = l_newProfile;
                            ResetControls();
                        }
                    }
                }

                case DR_Game:
                {
                    std::string l_game;
                    l_stream >> l_game;
                    if(!l_stream.fail() && !l_game.empty())
                    {
                        switch(ReadEnumVector(l_game, g_gameList))
                        {
                            case GL_VRChat:
                            {
                                std::string l_gameCommand;
                                l_stream >> l_gameCommand;
                                if(!l_stream.fail() && !l_game.empty())
                                {
                                    switch(ReadEnumVector(l_gameCommand, g_gameCommands))
                                    {
                                        case CC_DrawingMode:
                                        {
                                            m_gameSpecialModes.m_vrchatDrawingMode = !m_gameSpecialModes.m_vrchatDrawingMode;
                                        } break;
                                    }
                                }
                            } break;
                        }
                    }
                } break;
            }
        }
    }
}

vr::DriverPose_t CLeapHandController::GetPose()
{
    return m_pose;
}

void CLeapHandController::SetConnectionState(bool f_state)
{
    if(m_trackedDeviceID != vr::k_unTrackedDeviceIndexInvalid)
    {
        m_pose.deviceIsConnected = f_state;
        ms_driverHost->TrackedDevicePoseUpdated(m_trackedDeviceID, m_pose, sizeof(vr::DriverPose_t));
    }
}

void CLeapHandController::Update(const Leap::Frame &f_frame)
{
    if(m_trackedDeviceID != vr::k_unTrackedDeviceIndexInvalid)
    {
        UpdateTransformation(f_frame);
        UpdateGestures(f_frame);
        UpdateInput();
    }
}

void CLeapHandController::UpdateTransformation(const Leap::Frame &f_frame)
{
    bool l_handFound = false;
    Leap::HandList &l_hands = f_frame.hands();
    for(auto l_hand : l_hands)
    {
        if(l_hand.isValid())
        {
            if(((m_handAssigment == CHA_Left) && l_hand.isLeft()) || ((m_handAssigment == CHA_Right) && l_hand.isRight()))
            {
                switch(CDriverConfig::GetOrientationMode())
                {
                    case CDriverConfig::OM_HMD:
                    {
                        std::memcpy(&m_pose.qWorldFromDriverRotation, &ms_headRot, sizeof(vr::HmdQuaternion_t));
                        std::memcpy(m_pose.vecWorldFromDriverTranslation, ms_headPos, sizeof(double) * 3U);

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
                        switch(m_handAssigment)
                        {
                            case CHA_Left:
                                l_palmNormal *= -1.f;
                                break;
                            case CHA_Right:
                                l_leapCross *= -1.f;
                                break;
                        }

                        glm::mat3 l_rotMat(
                            l_palmNormal.x, l_palmNormal.z, l_palmNormal.y,
                            l_leapCross.x, l_leapCross.z, l_leapCross.y,
                            l_handDirection.x, l_handDirection.z, l_handDirection.y
                            );
                        glm::quat l_finalRot = glm::quat_cast(l_rotMat);
                        l_finalRot *= m_gripAngleOffset;

                        m_pose.qRotation.x = l_finalRot.x;
                        m_pose.qRotation.y = l_finalRot.y;
                        m_pose.qRotation.z = l_finalRot.z;
                        m_pose.qRotation.w = l_finalRot.w;
                    } break;
                    case CDriverConfig::OM_Desktop:
                    {
                        // Controller follows HMD
                        m_pose.qWorldFromDriverRotation.x = .0;
                        m_pose.qWorldFromDriverRotation.y = .0;
                        m_pose.qWorldFromDriverRotation.z = .0;
                        m_pose.qWorldFromDriverRotation.w = 1.0;
                        m_pose.vecWorldFromDriverTranslation[0U] = CDriverConfig::GetDesktopRootX();
                        m_pose.vecWorldFromDriverTranslation[1U] = CDriverConfig::GetDesktopRootY();
                        m_pose.vecWorldFromDriverTranslation[2U] = CDriverConfig::GetDesktopRootZ();

                        Leap::Vector l_position = l_hand.palmPosition();
                        m_pose.vecPosition[0] = 0.001*l_position.x;
                        m_pose.vecPosition[1] = 0.001*l_position.y;
                        m_pose.vecPosition[2] = 0.001*l_position.z;

                        Leap::Vector l_velocity = l_hand.palmVelocity();
                        m_pose.vecVelocity[0] = 0.001*l_velocity.x;
                        m_pose.vecVelocity[1] = 0.001*l_velocity.y;
                        m_pose.vecVelocity[2] = 0.001*l_velocity.z;

                        Leap::Vector l_handDirection = -l_hand.direction();
                        l_handDirection /= l_handDirection.magnitude();

                        Leap::Vector l_palmNormal = -l_hand.palmNormal();
                        l_palmNormal /= l_palmNormal.magnitude();

                        Leap::Vector l_leapCross = -l_handDirection.cross(l_palmNormal);
                        switch(m_handAssigment)
                        {
                            case CHA_Left:
                                l_palmNormal *= -1.f;
                                break;
                            case CHA_Right:
                                l_leapCross *= -1.f;
                                break;
                        }

                        glm::mat3 l_rotMat(
                            l_palmNormal.x, l_palmNormal.y, l_palmNormal.z,
                            l_leapCross.x, l_leapCross.y, l_leapCross.z,
                            l_handDirection.x, l_handDirection.y, l_handDirection.z
                            );
                        glm::quat l_finalRot = glm::quat_cast(l_rotMat);
                        l_finalRot *= m_gripAngleOffset;

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

    ms_driverHost->TrackedDevicePoseUpdated(m_trackedDeviceID, m_pose, sizeof(vr::DriverPose_t));
}

bool CLeapHandController::MixHandState(bool f_state)
{
    return f_state;
}

void CLeapHandController::UpdateInput()
{
    for(auto l_button : m_buttons)
    {
        if(l_button->IsUpdated())
        {
            switch(l_button->GetInputType())
            {
                case CControllerButton::CBIT_Boolean:
                    ms_driverInput->UpdateBooleanComponent(l_button->GetHandle(), l_button->GetState(), .0);
                    break;
                case CControllerButton::CBIT_Float:
                    ms_driverInput->UpdateScalarComponent(l_button->GetHandle(), l_button->GetValue(), .0);
                    break;
            }
            l_button->ResetUpdate();
        }
    }
    UpdateInputInternal();
}

void CLeapHandController::ResetControls()
{
    for(auto l_button : m_buttons)
    {
        l_button->SetValue(0.f);
        l_button->SetState(false);
    }
}

void CLeapHandController::SetInterfaces(vr::IVRServerDriverHost *f_host, vr::IVRDriverInput *f_input, vr::CVRPropertyHelpers *f_helpers)
{
    ms_driverHost = f_host;
    ms_driverInput = f_input;
    ms_propertyHelpers = f_helpers;
}
void CLeapHandController::UpdateHMDCoordinates()
{
    vr::TrackedDevicePose_t l_hmdPose;
    ms_driverHost->GetRawTrackedDevicePoses(0.f, &l_hmdPose, 1U); // HMD has device ID 0
    if(l_hmdPose.bPoseIsValid)
    {
        glm::mat4 l_rotMat(1.f);
        ConvertMatrix(l_hmdPose.mDeviceToAbsoluteTracking, l_rotMat);
        glm::quat l_headRot = glm::quat_cast(l_rotMat);
        ms_headRot.x = l_headRot.x;
        ms_headRot.y = l_headRot.y;
        ms_headRot.z = l_headRot.z;
        ms_headRot.w = l_headRot.w;

        for(size_t i = 0U; i < 3U; i++) ms_headPos[i] = l_hmdPose.mDeviceToAbsoluteTracking.m[i][3];
    }
}
