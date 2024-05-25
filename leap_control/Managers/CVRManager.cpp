#include "stdafx.h"
#include "Managers/CVRManager.h"
#include "Utils/Utils.h"

const glm::vec4 g_pointVec4(0.f, 0.f, 0.f, 1.f);

CVRManager* CVRManager::ms_instance = nullptr;

CVRManager* CVRManager::GetInstance()
{
    if(!ms_instance)
        ms_instance = new CVRManager();

    return ms_instance;
}

CVRManager::CVRManager()
{
    m_vrSystem = nullptr;
    m_leapDevice = vr::k_unTrackedDeviceIndexInvalid;
    m_leftController = vr::k_unTrackedDeviceIndexInvalid;
    m_rightController = vr::k_unTrackedDeviceIndexInvalid;
    m_exitPolled = false;

    m_hmdMatrix = glm::mat4(1.f);
    m_hmdPosition = glm::vec3(0.f);
    m_hmdRotation = glm::quat(1.f, 0.f, 0.f, 0.f);

    m_leftHandMatrix = glm::mat4(1.f);
    m_leftHandPosition = glm::vec3(0.f);
    m_leftHandRotation = glm::quat(1.f, 0.f, 0.f, 0.f);

    m_rightHandMatrix = glm::mat4(1.f);
    m_rightHandPosition = glm::vec3(0.f);
    m_rightHandRotation = glm::quat(1.f, 0.f, 0.f, 0.f);
}

bool CVRManager::Init()
{
    if(!m_vrSystem)
    {
        vr::EVRInitError l_initError;
        m_vrSystem = vr::VR_Init(&l_initError, vr::VRApplication_Overlay);
        if(l_initError != vr::VRInitError_None)
        {
            QString l_errorString("Unable to initialize OpenVR: ");
            l_errorString.append(vr::VR_GetVRInitErrorAsEnglishDescription(l_initError));
            QMessageBox::critical(nullptr, "Leap Control", l_errorString);
        }
        else
        {
            for(uint32_t i = 0; i < vr::k_unMaxTrackedDeviceCount; i++)
            {
                vr::ETrackedPropertyError l_propertyError = vr::TrackedProp_Success;
                uint64_t l_property = m_vrSystem->GetUint64TrackedDeviceProperty(i, vr::Prop_VendorSpecific_Reserved_Start, &l_propertyError);
                if((l_propertyError == vr::TrackedProp_Success) && (l_property == 0x4C4DU)) // "LM"
                {
                    m_leapDevice = i;
                    break;
                }
            }
        }
    }

    return (m_vrSystem != nullptr);
}

void CVRManager::Terminate()
{
    if(m_vrSystem)
    {
        vr::VR_Shutdown();
        m_vrSystem = nullptr;
    }
}

void CVRManager::Update()
{
    if(m_vrSystem)
    {
        m_vrSystem->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseRawAndUncalibrated, 0.f, m_trackedPoses, vr::k_unMaxTrackedDeviceCount);

        vr::VREvent_t l_event{ 0 };
        while(m_vrSystem->PollNextEvent(&l_event, sizeof(vr::VREvent_t)))
        {
            switch(l_event.eventType)
            {
                case vr::VREvent_Quit:
                case vr::VREvent_RestartRequested:
                    m_exitPolled = true;
                    break;

                case vr::VREvent_TrackedDeviceDeactivated:
                {
                    if(m_leftController == l_event.trackedDeviceIndex)
                        m_leftController = vr::k_unTrackedDeviceIndexInvalid;
                    if(m_rightController == l_event.trackedDeviceIndex)
                        m_rightController = vr::k_unTrackedDeviceIndexInvalid;
                }
                break;
            }
        }

        if(m_trackedPoses[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
        {
            ConvertMatrix(m_trackedPoses[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking, m_hmdMatrix);
            m_hmdPosition = m_hmdMatrix * g_pointVec4;
            m_hmdRotation = glm::toQuat(m_hmdMatrix);
        }

        if(m_leftController != vr::k_unTrackedDeviceIndexInvalid)
        {
            if(m_trackedPoses[m_leftController].bPoseIsValid)
            {
                ConvertMatrix(m_trackedPoses[m_leftController].mDeviceToAbsoluteTracking, m_leftHandMatrix);
                m_leftHandPosition = m_leftHandMatrix * g_pointVec4;
                m_leftHandRotation = glm::toQuat(m_leftHandMatrix);
            }
        }
        else
            m_leftController = m_vrSystem->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand);

        if(m_rightController != vr::k_unTrackedDeviceIndexInvalid)
        {
            if(m_trackedPoses[m_rightController].bPoseIsValid)
            {
                ConvertMatrix(m_trackedPoses[m_rightController].mDeviceToAbsoluteTracking, m_rightHandMatrix);
                m_rightHandPosition = m_rightHandMatrix * g_pointVec4;
                m_rightHandRotation = glm::toQuat(m_rightHandMatrix);
            }
        }
        else
            m_rightController = m_vrSystem->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);
    }
}

bool CVRManager::IsExitPolled() const
{
    return m_exitPolled;
}

const glm::vec3 & CVRManager::GetHmdPosition() const
{
    return m_hmdPosition;
}
const glm::quat & CVRManager::GetHmdRotation() const
{
    return m_hmdRotation;
}
const glm::mat4 & CVRManager::GetHmdMatrix() const
{
    return m_hmdMatrix;
}

const glm::vec3 & CVRManager::GetLeftHandPosition() const
{
    return m_leftHandPosition;
}
const glm::quat & CVRManager::GetLeftHandRotation() const
{
    return m_leftHandRotation;
}
const glm::mat4 & CVRManager::GetLeftHandMatrix() const
{
    return m_leftHandMatrix;
}

const glm::vec3 & CVRManager::GetRightHandPosition() const
{
    return m_rightHandPosition;
}
const glm::quat & CVRManager::GetRightHandRotation() const
{
    return m_rightHandRotation;
}
const glm::mat4 & CVRManager::GetRightHandMatrix() const
{
    return m_rightHandMatrix;
}

void CVRManager::SendStationMessage(const std::string &p_message)
{
    if(m_vrSystem && (m_leapDevice != vr::k_unTrackedDeviceIndexInvalid))
    {
        char l_response[32U] = { 0 };
        vr::VRDebug()->DriverDebugRequest(m_leapDevice, p_message.c_str(), l_response, 32U);
    }
}

void CVRManager::SendLeftControllerMessage(const std::string &p_message)
{
    if(m_vrSystem && (m_leftController != vr::k_unTrackedDeviceIndexInvalid))
    {
        char l_response[32U] = { 0 };
        vr::VRDebug()->DriverDebugRequest(m_leftController, p_message.c_str(), l_response, 32U);
    }
}

void CVRManager::SendRightControllerMessage(const std::string &p_message)
{
    if(m_vrSystem && (m_rightController != vr::k_unTrackedDeviceIndexInvalid))
    {
        char l_response[32U] = { 0 };
        vr::VRDebug()->DriverDebugRequest(m_rightController, p_message.c_str(), l_response, 32U);
    }
}
