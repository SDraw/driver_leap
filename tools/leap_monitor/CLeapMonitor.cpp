#include "stdafx.h"

#include "CLeapMonitor.h"

const std::chrono::milliseconds k_MonitorInterval(10);

CLeapMonitor::CLeapMonitor()
{
}
CLeapMonitor::~CLeapMonitor()
{
}

void CLeapMonitor::Run()
{
    if(Init()) MainLoop();
    Shutdown();
}

bool CLeapMonitor::Init()
{
    vr::EVRInitError eVRInitError;
    vr::VR_Init(&eVRInitError, vr::VRApplication_Background);
    if(!vr::VRSystem() || eVRInitError != vr::VRInitError_None) return false;

    // Keep track of which devices use driver_leap
    for(int i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i) UpdateTrackedDevice(i);

    return true;
}

void CLeapMonitor::MainLoop()
{
    while(true)
    {
        std::this_thread::sleep_for(k_MonitorInterval);

#if defined( WIN32 )
        MSG msg = { 0 };
        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if(msg.message == WM_QUIT) break;
#endif

        vr::VREvent_t Event;
        while(vr::VRSystem()->PollNextEvent(&Event, sizeof(Event)))
        {
            switch(Event.eventType)
            {
                case vr::VREvent_Quit:
                    exit(0);
                    // NOTREAHED

                case vr::VREvent_TrackedDeviceActivated:
                case vr::VREvent_TrackedDeviceUpdated:
                    UpdateTrackedDevice(Event.trackedDeviceIndex);
                    break;

                case vr::VREvent_VendorSpecific_Reserved_Start + 0:
                    // User has made the "align" gesture.  The driver can't see the HMD
                    // coordinates, so we forward those from our client view.
                    if(IsLeapDevice(Event.trackedDeviceIndex))
                    {
                        //                        printf("received event vr::VREvent_VendorSpecific_Reserved_Start + 0 for device %d\n", Event.trackedDeviceIndex);
                        TriggerRealignCoordinates(Event);
                    }
                    break;
            }
        }
    }
}

void CLeapMonitor::Shutdown()
{
    vr::VR_Shutdown();
}

bool CLeapMonitor::TriggerRealignCoordinates(const vr::VREvent_t& Event)
{
    vr::TrackedDevicePose_t hmdPose;
    vr::VRSystem()->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseRawAndUncalibrated, -Event.eventAgeSeconds, &hmdPose, 1);
    if(!hmdPose.bPoseIsValid) return false;

    std::ostringstream ss;
    char rgchReplyBuf[256];

    ss << "leap:realign_coordinates";
    for(int i = 0; i < 3; ++i)
    {
        for(int j = 0; j < 4; ++j)
        {
            ss << " " << hmdPose.mDeviceToAbsoluteTracking.m[i][j];
        }
    }
    //        printf("%s\n", ss.str().c_str());
    vr::VRSystem()->DriverDebugRequest(Event.trackedDeviceIndex, ss.str().c_str(), rgchReplyBuf, sizeof(rgchReplyBuf));
    return true;
}

void CLeapMonitor::UpdateTrackedDevice(uint32_t unTrackedDeviceIndex)
{
    char rgchTrackingSystemName[vr::k_unMaxPropertyStringSize];
    vr::ETrackedPropertyError eError;

    vr::VRSystem()->GetStringTrackedDeviceProperty(unTrackedDeviceIndex, vr::Prop_TrackingSystemName_String, rgchTrackingSystemName, sizeof(rgchTrackingSystemName), &eError);
    if(eError == vr::TrackedProp_Success)
    {
        if(!strcmp(rgchTrackingSystemName, "leap"))
        {
            m_setLeapDevices.insert(unTrackedDeviceIndex);
        }
    }
}

bool CLeapMonitor::IsLeapDevice(uint32_t unTrackedDeviceIndex)
{
    return (m_setLeapDevices.count(unTrackedDeviceIndex) != 0);
}
