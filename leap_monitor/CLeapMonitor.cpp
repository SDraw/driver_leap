#include "stdafx.h"

#include "CLeapMonitor.h"

void CLeapListener::onInit(const Leap::Controller &controller)
{
    controller.setPolicy(Leap::Controller::POLICY_OPTIMIZE_HMD);
}
void CLeapListener::onLogMessage(const Leap::Controller &controller, Leap::MessageSeverity severity, int64_t timestamp, const char *msg)
{
    if(severity != Leap::MESSAGE_INFORMATION)
    {
        std::string l_message(msg);
        m_monitor->SendNotification(l_message);
    }
}

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
    m_vrSystem = vr::VR_Init(&eVRInitError, vr::VRApplication_Background);
    if(eVRInitError != vr::VRInitError_None) return false;

    m_vrOverlay = vr::VROverlay();
    m_vrOverlay->CreateOverlay("leap_monitor_overlay", "Leap Motion Monitor", &m_overlayHandle);
    m_vrNotifications = vr::VRNotifications();
    m_notificationID = 0U;
    m_lastApplication = 0U;

    // Keep track of which devices use driver_leap
    for(int i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i) UpdateTrackedDevice(i);

    m_leapController = new Leap::Controller();
    m_leapController->addListener(m_leapListener);
    m_leapListener.SetMonitor(this);

    return true;
}

void CLeapMonitor::MainLoop()
{
    const std::chrono::milliseconds l_monitorInterval(100U);
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
                case vr::VREvent_ApplicationTransitionNewAppLaunchComplete:
                {
                    if(m_lastApplication != Event.data.process.pid)
                    {
                        m_lastApplication = Event.data.process.pid;
                        char l_appKeyNew[vr::k_unMaxApplicationKeyLength];
                        vr::VRApplications()->GetApplicationKeyByProcessId(Event.data.process.pid, l_appKeyNew, sizeof(l_appKeyNew));
                        UpdateApplicationKey(l_appKeyNew);
                    }
                } break;
            }
            if(l_quitEvent) break;
        }

        std::this_thread::sleep_for(l_monitorInterval);
    }
}

void CLeapMonitor::Shutdown()
{
    m_leapListener.SetMonitor(nullptr);
    if(m_notificationID) m_vrNotifications->RemoveNotification(m_notificationID);
    m_vrOverlay->DestroyOverlay(m_overlayHandle);

    m_leapController->removeListener(m_leapListener);
    delete m_leapController;
    m_leapController = nullptr;

    vr::VR_Shutdown();
}

void CLeapMonitor::SendNotification(const std::string &f_text)
{
    if(!f_text.empty())
    {
        if(m_notificationID) m_vrNotifications->RemoveNotification(m_notificationID);
        m_vrNotifications->CreateNotification(m_overlayHandle, 500U, vr::EVRNotificationType_Transient, f_text.c_str(), vr::EVRNotificationStyle_None, nullptr, &m_notificationID);
    }
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

void CLeapMonitor::UpdateApplicationKey(const char *f_appKey)
{
    bool l_notify = false;
    std::string l_notifyText;

    if(!m_setLeapDevices.empty())
    {
        char l_response[32] = { 0 };
        std::string l_data("app_key ");
        l_data.append(f_appKey);
        for(auto iter : m_setLeapDevices) vr::VRSystem()->DriverDebugRequest(iter, l_data.c_str(), l_response, sizeof(l_response));
        if(!l_notify)
        {
            size_t l_responseLength = static_cast<size_t>(l_response[0U]);
            if(l_responseLength != 0U)
            {
                l_notifyText.append("Game profile has been changed to '");
                l_notifyText.append(&l_response[1U],l_responseLength);
                l_notifyText.push_back('\'');

                l_notify = true;
            }
        }
    }

    if(l_notify) SendNotification(l_notifyText);
}
