#include "stdafx.h"
#include "Devices/CLeapStation.h"
#include "Devices/Controller/CLeapIndexController.h"
#include "Core/CDriverConfig.h"
#include "Utils/Utils.h"

enum MessageType : size_t
{
    MT_Power = 0U,
    MT_Setting
};
const std::vector<std::string> g_messageTypes =
{
    "power",
    "setting"
};

enum ControllerHand : size_t
{
    CH_Left = 0U,
    CH_Right
};
const std::vector<std::string> g_controllerHands =
{
    "left", "right"
};

CLeapStation::CLeapStation()
{
    m_pose = { 0 };
    m_pose.deviceIsConnected = true;
    m_pose.poseIsValid = true;
    m_pose.poseTimeOffset = 0.;
    m_pose.qDriverFromHeadRotation = { 1.0, .0, .0, .0 };
    m_pose.qRotation = { 1.0, .0, .0, .0 };
    m_pose.qWorldFromDriverRotation = { 1.0, .0, .0, .0 };
    m_pose.result = vr::TrackingResult_Running_OK;
    m_pose.shouldApplyHeadModel = false;
    for(size_t i = 0U; i < 3U; i++)
    {
        m_pose.vecAcceleration[i] = 0.;
        m_pose.vecAngularAcceleration[i] = 0.;
        m_pose.vecAngularVelocity[i] = 0.;
        m_pose.vecDriverFromHeadTranslation[i] = 0.;
        m_pose.vecPosition[i] = 0.;
        m_pose.vecVelocity[i] = 0.;
    }
    m_pose.willDriftInYaw = false;

    m_propertyHandle = vr::k_ulInvalidPropertyContainer;
    m_trackedDevice = vr::k_unTrackedDeviceIndexInvalid;

    m_serialNumber.assign("leap_motion_station");
    m_leftController = nullptr;
    m_rightController = nullptr;
}

// vr::ITrackedDeviceServerDriver
vr::EVRInitError CLeapStation::Activate(uint32_t unObjectId)
{
    vr::EVRInitError l_resultError = vr::VRInitError_Driver_CalibrationInvalid;

    if(m_trackedDevice == vr::k_unTrackedDeviceIndexInvalid)
    {
        m_trackedDevice = unObjectId;
        m_propertyHandle = vr::VRProperties()->TrackedDeviceToPropertyContainer(m_trackedDevice);

        vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_TrackingSystemName_String, "leap_motion");
        vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_SerialNumber_String, m_serialNumber.c_str());
        vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_ModelNumber_String, m_serialNumber.c_str());
        vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_ManufacturerName_String, "Leap Motion");
        vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_ModeLabel_String, "L");

        vr::VRProperties()->SetInt32Property(m_propertyHandle, vr::Prop_DeviceClass_Int32, vr::TrackedDeviceClass_TrackingReference);
        vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_IsOnDesktop_Bool, false);
        vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_NeverTracked_Bool, false);
        vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_WillDriftInYaw_Bool, false);
        vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_CanWirelessIdentify_Bool, false);

        vr::VRProperties()->SetFloatProperty(m_propertyHandle, vr::Prop_FieldOfViewLeftDegrees_Float, 150.f);
        vr::VRProperties()->SetFloatProperty(m_propertyHandle, vr::Prop_FieldOfViewRightDegrees_Float, 150.f);
        vr::VRProperties()->SetFloatProperty(m_propertyHandle, vr::Prop_FieldOfViewTopDegrees_Float, 120.f);
        vr::VRProperties()->SetFloatProperty(m_propertyHandle, vr::Prop_FieldOfViewBottomDegrees_Float, 120.f);
        vr::VRProperties()->SetFloatProperty(m_propertyHandle, vr::Prop_TrackingRangeMinimumMeters_Float, 0.025f);
        vr::VRProperties()->SetFloatProperty(m_propertyHandle, vr::Prop_TrackingRangeMaximumMeters_Float, 0.6f);

        vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_ResourceRoot_String, "leap");
        vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_RenderModelName_String, "{leap}leap_motion_1_0");

        vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_NamedIconPathDeviceReady_String, "{leap}/icons/base_status_ready.png");
        vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_NamedIconPathDeviceSearching_String, "{leap}/icons/base_status_searching.png");

        vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_HasDisplayComponent_Bool, false);
        vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_HasCameraComponent_Bool, false);
        vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_HasDriverDirectModeComponent_Bool, false);
        vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_HasVirtualDisplayComponent_Bool, false);

        vr::VRProperties()->SetUint64Property(m_propertyHandle, vr::Prop_VendorSpecific_Reserved_Start, 0x4C4DU); // "LM", _hidden_ property for leap_monitor

        m_pose.vecWorldFromDriverTranslation[0U] = 0.1f; // Slightly move to avoid base stations

        l_resultError = vr::VRInitError_None;
    }

    return l_resultError;
}

void CLeapStation::Deactivate()
{
    m_trackedDevice = vr::k_unTrackedDeviceIndexInvalid;
}

void CLeapStation::EnterStandby()
{
}

void* CLeapStation::GetComponent(const char* pchComponentNameAndVersion)
{
    void *l_result = nullptr;
    if(!strcmp(pchComponentNameAndVersion, vr::ITrackedDeviceServerDriver_Version))
        l_result = dynamic_cast<vr::ITrackedDeviceServerDriver*>(this);
    return l_result;
}

void CLeapStation::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
{
    if(m_trackedDevice != vr::k_unTrackedDeviceIndexInvalid)
        ProcessExternalMessage(pchRequest);
}

vr::DriverPose_t CLeapStation::GetPose()
{
    return m_pose;
}

// CLeapStation
const std::string& CLeapStation::GetSerialNumber() const
{
    return m_serialNumber;
}

void CLeapStation::SetControllers(CLeapIndexController * p_left, CLeapIndexController * p_right)
{
    m_leftController = p_left;
    m_rightController = p_right;
}

void CLeapStation::SetTrackingState(TrackingState p_state)
{
    switch(p_state)
    {
        case TS_Connected:
        {
            m_pose.result = vr::TrackingResult_Running_OK;
            m_pose.poseIsValid = true;
        } break;
        case TS_Search:
        {
            m_pose.result = vr::TrackingResult_Calibrating_OutOfRange;
            m_pose.poseIsValid = false;
        } break;
    }
}

void CLeapStation::RunFrame()
{
    if(m_trackedDevice != vr::k_unTrackedDeviceIndexInvalid)
        vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_trackedDevice, m_pose, sizeof(vr::DriverPose_t));
}

void CLeapStation::ProcessExternalMessage(const char *p_message)
{
    std::vector<std::string> l_chunks;
    SplitString(p_message, ' ', l_chunks);

    if(l_chunks.size() > 1U)
    {
        size_t l_messageType = 0U;
        if(ReadEnumVector(l_chunks[0U], g_messageTypes, l_messageType))
        {
            switch(l_messageType)
            {
                case MT_Power:
                {
                    size_t l_hand = 0U;
                    if(ReadEnumVector(l_chunks[1U], g_controllerHands, l_hand))
                    {
                        switch(l_hand)
                        {
                            case CH_Left:
                            {
                                if(m_leftController)
                                    m_leftController->ToggleEnabled();
                            } break;
                            case CH_Right:
                            {
                                if(m_rightController)
                                    m_rightController->ToggleEnabled();
                            } break;
                        }
                    }
                } break;

                case MT_Setting:
                {
                    std::string l_message;
                    for(size_t i = 1U, j = l_chunks.size(); i < j; i++)
                    {
                        l_message.append(l_chunks[i]);
                        l_message.push_back(' ');
                    }
                    CDriverConfig::ProcessExternalSetting(l_message);
                } break;
            }
        }
    }
}
