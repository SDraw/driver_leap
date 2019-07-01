#include "stdafx.h"

#include "CLeapMonitor.h"

#include "Utils.h"

void CLeapListener::SetMonitor(CLeapMonitor *f_monitor)
{
    m_monitorMutex.lock();
    m_monitor = f_monitor;
    m_monitorMutex.unlock();
}
void CLeapListener::onInit(const Leap::Controller &controller)
{
    controller.setPolicy(Leap::Controller::POLICY_OPTIMIZE_HMD);
}
void CLeapListener::onConnect(const Leap::Controller &controller)
{
    m_monitorMutex.lock();
    if(m_monitor)
    {
        std::string l_message("Controller connected");
        m_monitor->SendNotification(l_message);
    }
    m_monitorMutex.unlock();
}
void CLeapListener::onDisconnect(const Leap::Controller &controller)
{
    m_monitorMutex.lock();
    if(m_monitor)
    {
        std::string l_message("Controller disconnected");
        m_monitor->SendNotification(l_message);
    }
    m_monitorMutex.unlock();
}
void CLeapListener::onServiceConnect(const Leap::Controller &controller)
{
    m_monitorMutex.lock();
    if(m_monitor)
    {
        std::string l_message("Service connected");
        m_monitor->SendNotification(l_message);
    }
    m_monitorMutex.unlock();
}
void CLeapListener::onServiceDisconnect(const Leap::Controller &controller)
{
    m_monitorMutex.lock();
    if(m_monitor)
    {
        std::string l_message("Service disconnected");
        m_monitor->SendNotification(l_message);
    }
    m_monitorMutex.unlock();
}
void CLeapListener::onLogMessage(const Leap::Controller &controller, Leap::MessageSeverity severity, int64_t timestamp, const char *msg)
{
    if(severity <= Leap::MESSAGE_CRITICAL)
    {
        m_monitorMutex.lock();
        if(m_monitor)
        {
            std::string l_message(msg);
            m_monitor->SendNotification(l_message);
        }
        m_monitorMutex.unlock();
    }
}

// ----
const std::vector<std::string> g_steamAppKeys
{
    "steam.app.438100" // VRChat
};
enum SteamAppID : int
{
    SAI_VRChat = 0
};

const std::string g_profileName[2]
{
    "Default", "VRChat"
};

CLeapMonitor::CLeapMonitor()
{
    m_initialized = false;
    m_vrSystem = nullptr;
    m_vrApplications = nullptr;
    m_vrOverlay = nullptr;
    m_vrNotifications = nullptr;
    m_notificationID = 0U;
    m_leapController = nullptr;
    m_gameProfile = GP_Default;
}
CLeapMonitor::~CLeapMonitor()
{
}

bool CLeapMonitor::Init()
{
    if(!m_initialized)
    {
        vr::EVRInitError eVRInitError;
        m_vrSystem = vr::VR_Init(&eVRInitError, vr::VRApplication_Background);
        if(eVRInitError == vr::VRInitError_None)
        {
            m_vrApplications = vr::VRApplications();
            m_vrOverlay = vr::VROverlay();
            m_vrOverlay->CreateOverlay("leap_monitor_overlay", "Leap Motion Monitor", &m_overlayHandle);
            m_vrNotifications = vr::VRNotifications();

            for(int i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i) UpdateTrackedDevice(i);

            m_leapController = new Leap::Controller();
            m_leapController->addListener(m_leapListener);
            m_leapListener.SetMonitor(this);

            m_initialized = true;
        }
    }
    return m_initialized;
}

void CLeapMonitor::Run()
{
    if(m_initialized)
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
                    case vr::VREvent_TrackedDeviceActivated:
                        UpdateTrackedDevice(Event.trackedDeviceIndex);
                        break;
                    case vr::VREvent_ApplicationTransitionNewAppLaunchComplete:
                    {
                        char l_appKeyNew[vr::k_unMaxApplicationKeyLength];
                        m_vrApplications->GetApplicationKeyByProcessId(Event.data.process.pid, l_appKeyNew, vr::k_unMaxApplicationKeyLength);

                        std::string l_appString(l_appKeyNew);
                        switch(ReadEnumVector(l_appString, g_steamAppKeys))
                        {
                            case SAI_VRChat:
                                m_gameProfile = GP_VRChat;
                                break;
                            default:
                                m_gameProfile = GP_Default;
                                break;
                        }
                        UpdateGameProfile();

                        std::string l_notifyText("Game profile has been changed to '");
                        l_notifyText.append(g_profileName[m_gameProfile]);
                        l_notifyText.push_back('\'');
                        SendNotification(l_notifyText);
                    } break;
                }
                if(l_quitEvent) break;
            }

            std::this_thread::sleep_for(l_monitorInterval);
        }
    }
}

void CLeapMonitor::Terminate()
{
    if(m_initialized)
    {
        m_initialized = false;

        m_leapListener.SetMonitor(nullptr);
        m_leapController->removeListener(m_leapListener);
        delete m_leapController;

        if(m_notificationID) m_vrNotifications->RemoveNotification(m_notificationID);
        m_vrOverlay->DestroyOverlay(m_overlayHandle);

        vr::VR_Shutdown();

        m_vrSystem = nullptr;
        m_vrApplications = nullptr;
        m_vrOverlay = nullptr;
        m_vrNotifications = nullptr;
        m_notificationID = 0U;
        m_leapController = nullptr;
        m_gameProfile = GP_Default;

        vr::VR_Shutdown();
    }
}

void CLeapMonitor::SendNotification(const std::string &f_text)
{
    if(m_initialized)
    {
        if(!f_text.empty())
        {
            m_notificationLock.lock();
            if(m_notificationID) m_vrNotifications->RemoveNotification(m_notificationID);
            m_vrNotifications->CreateNotification(m_overlayHandle, 500U, vr::EVRNotificationType_Transient, f_text.c_str(), vr::EVRNotificationStyle_None, nullptr, &m_notificationID);
            m_notificationLock.unlock();
        }
    }
}

void CLeapMonitor::UpdateTrackedDevice(uint32_t unTrackedDeviceIndex)
{
    char rgchTrackingSystemName[vr::k_unMaxPropertyStringSize];
    vr::ETrackedPropertyError eError;

    m_vrSystem->GetStringTrackedDeviceProperty(unTrackedDeviceIndex, vr::Prop_TrackingSystemName_String, rgchTrackingSystemName, vr::k_unMaxPropertyStringSize, &eError);
    if(eError == vr::TrackedProp_Success)
    {
        if(!strcmp(rgchTrackingSystemName, "leap")) m_setLeapDevices.insert(unTrackedDeviceIndex);
    }
}

void CLeapMonitor::UpdateGameProfile()
{
    if(!m_setLeapDevices.empty())
    {
        char l_response[32U];
        std::string l_data("profile ");
        l_data.append(g_profileName[m_gameProfile]);

        for(auto iter : m_setLeapDevices) m_vrSystem->DriverDebugRequest(iter, l_data.c_str(), l_response, 32U);
    }
}
