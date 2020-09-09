#include "stdafx.h"

#include "CRelayDevice.h"
#include "CServerDriver.h"

CRelayDevice::CRelayDevice(CServerDriver *f_server)
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

    m_serialNumber.assign("leap_motion_relay");
    m_serverDriver = f_server;
}
CRelayDevice::~CRelayDevice()
{
}

vr::EVRInitError CRelayDevice::Activate(uint32_t unObjectId)
{
    vr::EVRInitError l_resultError = vr::VRInitError_Driver_CalibrationInvalid;

    if(m_trackedDevice == vr::k_unTrackedDeviceIndexInvalid)
    {
        m_trackedDevice = unObjectId;
        m_propertyHandle = vr::VRProperties()->TrackedDeviceToPropertyContainer(m_trackedDevice);

        vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_ResourceRoot_String, "leap");
        vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_RenderModelName_String, "{leap}leap_motion_1_0");

        vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_SerialNumber_String, m_serialNumber.c_str());
        vr::VRProperties()->SetBoolProperty(m_propertyHandle, vr::Prop_WillDriftInYaw_Bool, false);
        vr::VRProperties()->SetUint64Property(m_propertyHandle, vr::Prop_VendorSpecific_Reserved_Start, 0x4C4D6F74696F6E); // "LMotion", hidden property for leap_monitor

        vr::VRProperties()->SetStringProperty(m_propertyHandle, vr::Prop_NamedIconPathDeviceReady_String, "{leap}/icons/base_status_ready.png");

        m_pose.vecWorldFromDriverTranslation[0U] = 0.1f; // Slightly move to avoid base stations

        l_resultError = vr::VRInitError_None;
    }

    return l_resultError;
}

void CRelayDevice::Deactivate()
{
    m_trackedDevice = vr::k_unTrackedDeviceIndexInvalid;
}

void CRelayDevice::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
{
    if(m_trackedDevice != vr::k_unTrackedDeviceIndexInvalid)
    {
        m_serverDriver->ProcessExternalMessage(pchRequest);
    }
}

void* CRelayDevice::GetComponent(const char* pchComponentNameAndVersion)
{
    void *l_result = nullptr;
    if(!strcmp(pchComponentNameAndVersion, vr::ITrackedDeviceServerDriver_Version)) l_result = dynamic_cast<vr::ITrackedDeviceServerDriver*>(this);
    return l_result;
}

vr::DriverPose_t CRelayDevice::GetPose()
{
    return m_pose;
}

void CRelayDevice::Update()
{
    if(m_trackedDevice != vr::k_unTrackedDeviceIndexInvalid)
    {
        vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_trackedDevice, m_pose, sizeof(vr::DriverPose_t));
    }
}
