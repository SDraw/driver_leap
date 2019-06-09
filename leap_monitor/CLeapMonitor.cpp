#include "stdafx.h"

#include "CLeapMonitor.h"

const std::chrono::milliseconds k_MonitorInterval(100U);

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

bool g_keyPress[256U];
bool IsKeyDown(int key)
{
    return (GetAsyncKeyState(key) & 0x8000) != 0;
}

void CLeapMonitor::MainLoop()
{
    for(size_t i = 0U; i < 256U; i++) g_keyPress[i] = false;
    bool l_quitEvent = false;
    while(!l_quitEvent)
    {
        // System messages
        MSG msg = { 0 };
        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if(msg.message == WM_QUIT) break;

        // VR messages
        vr::VREvent_t Event;
        while(vr::VRSystem()->PollNextEvent(&Event, sizeof(Event)))
        {
            switch(Event.eventType)
            {
                case vr::VREvent_Quit:
                    l_quitEvent = true;
                    break;
                case vr::VREvent_TrackedDeviceActivated: case vr::VREvent_TrackedDeviceUpdated:
                    UpdateTrackedDevice(Event.trackedDeviceIndex);
                    break;
                case vr::VREvent_SceneApplicationChanged:
                {
                    char l_appKeyNew[vr::k_unMaxApplicationKeyLength];
                    vr::VRApplications()->GetApplicationKeyByProcessId(Event.data.process.pid, l_appKeyNew, sizeof(l_appKeyNew));
                    UpdateApplicationKey(l_appKeyNew);
                } break;
            }
            if(l_quitEvent) break;
        }

        std::this_thread::sleep_for(k_MonitorInterval);
    }
}

void CLeapMonitor::Shutdown()
{
    vr::VR_Shutdown();
}

void CLeapMonitor::UpdateTrackedDevice(uint32_t unTrackedDeviceIndex)
{
    char rgchTrackingSystemName[vr::k_unMaxPropertyStringSize];
    vr::ETrackedPropertyError eError;

    vr::VRSystem()->GetStringTrackedDeviceProperty(unTrackedDeviceIndex, vr::Prop_TrackingSystemName_String, rgchTrackingSystemName, sizeof(rgchTrackingSystemName), &eError);
    if(eError == vr::TrackedProp_Success)
    {
        if(!strcmp(rgchTrackingSystemName, "leap")) m_setLeapDevices.insert(unTrackedDeviceIndex);
    }
}

bool CLeapMonitor::IsLeapDevice(uint32_t unTrackedDeviceIndex)
{
    return (m_setLeapDevices.count(unTrackedDeviceIndex) != 0);
}

void CLeapMonitor::UpdateApplicationKey(const char *f_appKey)
{
    if(!m_setLeapDevices.empty())
    {
        char l_response[32];
        std::string l_data("app_key ");
        l_data.append(f_appKey);
        for(auto iter : m_setLeapDevices) vr::VRSystem()->DriverDebugRequest(iter, l_data.c_str(), l_response, sizeof(l_response));
    }
}
