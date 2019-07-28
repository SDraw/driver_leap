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
            m_vrDebug = vr::VRDebug();
            m_vrApplications = vr::VRApplications();
            m_vrOverlay = vr::VROverlay();
            m_vrOverlay->CreateOverlay("leap_monitor_overlay", "Leap Motion Monitor", &m_overlayHandle);
            m_vrNotifications = vr::VRNotifications();

            for(uint32_t i = 0U; i < vr::k_unMaxTrackedDeviceCount; i++) AddTrackedDevice(i);

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
            vr::VREvent_t l_event;
            while(vr::VRSystem()->PollNextEvent(&l_event, sizeof(vr::VREvent_t)))
            {
                switch(l_event.eventType)
                {
                    case vr::VREvent_Quit:
                        l_quitEvent = true;
                        break;
                    case vr::VREvent_TrackedDeviceActivated:
                        AddTrackedDevice(l_event.trackedDeviceIndex);
                        break;
                    case vr::VREvent_TrackedDeviceDeactivated:
                        RemoveTrackedDevice(l_event.trackedDeviceIndex);
                        break;
                    case vr::VREvent_ApplicationTransitionNewAppLaunchComplete:
                    {
                        char l_appKey[vr::k_unMaxApplicationKeyLength];
                        m_vrApplications->GetApplicationKeyByProcessId(l_event.data.process.pid, l_appKey, vr::k_unMaxApplicationKeyLength);
                        UpdateGameProfile(l_appKey);
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
        m_vrDebug = nullptr;
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

void CLeapMonitor::AddTrackedDevice(uint32_t unTrackedDeviceIndex)
{
    char rgchTrackingSystemName[vr::k_unMaxPropertyStringSize];
    vr::ETrackedPropertyError eError;

    m_vrSystem->GetStringTrackedDeviceProperty(unTrackedDeviceIndex, vr::Prop_TrackingSystemName_String, rgchTrackingSystemName, vr::k_unMaxPropertyStringSize, &eError);
    if(eError == vr::TrackedProp_Success)
    {
        if(!strcmp(rgchTrackingSystemName, "leap")) m_leapDevices.insert(unTrackedDeviceIndex);
    }
}
void CLeapMonitor::RemoveTrackedDevice(uint32_t unTrackedDeviceIndex)
{
    auto l_searchIter = m_leapDevices.find(unTrackedDeviceIndex);
    if(l_searchIter != m_leapDevices.end()) m_leapDevices.erase(l_searchIter);
}

void CLeapMonitor::UpdateGameProfile(const char *f_appKey)
{
    std::string l_appString(f_appKey);
    GameProfile l_newProfile;
    switch(ReadEnumVector(l_appString, g_steamAppKeys))
    {
        case SAI_VRChat:
            l_newProfile = GP_VRChat;
            break;
        default:
            l_newProfile = GP_Default;
            break;
    }
    if(m_gameProfile != l_newProfile)
    {
        m_gameProfile = l_newProfile;

        if(!m_leapDevices.empty())
        {
            char l_response[32U];
            std::string l_data("profile ");
            l_data.append(g_profileName[m_gameProfile]);

            for(auto l_device : m_leapDevices) m_vrDebug->DriverDebugRequest(l_device, l_data.c_str(), l_response, 32U);
        }

        std::string l_notifyText("Game profile has been changed to '");
        l_notifyText.append(g_profileName[m_gameProfile]);
        l_notifyText.push_back('\'');
        SendNotification(l_notifyText);
    }
}
