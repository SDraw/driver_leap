#include "stdafx.h"

#include "CLeapMonitor.h"

#include "Utils.h"

// CLeapListener
void CLeapListener::onConnect(const Leap::Controller &controller)
{
    if(m_monitor.load())
    {
        const std::string l_message("Controller connected");
        m_monitor.load()->SendNotification(l_message);
    }
}

void CLeapListener::onDisconnect(const Leap::Controller &controller)
{
    if(m_monitor.load())
    {
        const std::string l_message("Controller disconnected");
        m_monitor.load()->SendNotification(l_message);
    }
}

void CLeapListener::onInit(const Leap::Controller &controller)
{
}

void CLeapListener::onLogMessage(const Leap::Controller &controller, Leap::MessageSeverity severity, int64_t timestamp, const char *msg)
{
    if(severity <= Leap::MESSAGE_CRITICAL)
    {
        if(m_monitor.load())
        {
            const std::string l_message(msg);
            m_monitor.load()->SendNotification(l_message);
        }
    }
}

void CLeapListener::onServiceConnect(const Leap::Controller &controller)
{
    if(m_monitor.load())
    {
        const std::string l_message("Service connected");
        m_monitor.load()->SendNotification(l_message);
    }
}

void CLeapListener::onServiceDisconnect(const Leap::Controller &controller)
{
    if(m_monitor.load())
    {
        const std::string l_message("Service disconnected");
        m_monitor.load()->SendNotification(l_message);
    }
}

void CLeapListener::SetMonitor(CLeapMonitor *f_monitor)
{
    m_monitor.store(f_monitor);
}
// ----

const std::vector<std::string> g_steamAppKeys
{
    "steam.app.438100", "system.generated.vrchat.exe" // VRChat
};
enum SteamAppID : size_t
{
    SAI_VRChat = 0U,
    SAI_VRChatNoSteam
};

const std::string g_profileName[2U]
{
    "Default", "VRChat"
};

CLeapMonitor::CLeapMonitor()
{
    m_initialized = false;

    m_vrApplications = nullptr;
    m_vrDebug = nullptr;
    m_vrOverlay = nullptr;
    m_vrNotifications = nullptr;
    m_vrSystem = nullptr;
    m_notificationID = 0U;
    m_overlayHandle = vr::k_ulOverlayHandleInvalid;

    m_leapController = nullptr;

    m_gameProfile = GP_Default;
    m_relayDevice = vr::k_unTrackedDeviceIndexInvalid;
    m_specialHotkey = false;
    m_leftHotkey = false;
    m_rightHotkey = false;
    m_reloadHotkey = false;
}
CLeapMonitor::~CLeapMonitor()
{
}

bool CLeapMonitor::Initialize()
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
        m_overlayHandle = vr::k_ulOverlayHandleInvalid;

        vr::VR_Shutdown();

        m_vrSystem = nullptr;
        m_vrDebug = nullptr;
        m_vrApplications = nullptr;
        m_vrOverlay = nullptr;
        m_vrNotifications = nullptr;
        m_notificationID = 0U;

        m_leapController = nullptr;

        m_gameProfile = GP_Default;
        m_specialHotkey = false;
    }
}

void CLeapMonitor::Run()
{
    if(m_initialized)
    {
        const std::chrono::milliseconds l_monitorInterval(11U); // ~90 FPS
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
            while(m_vrSystem->PollNextEvent(&l_event, sizeof(vr::VREvent_t)))
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
                    case vr::VREvent_SceneApplicationStateChanged:
                    {
                        vr::EVRSceneApplicationState l_appState = m_vrApplications->GetSceneApplicationState();
                        switch(l_appState)
                        {
                            case vr::EVRSceneApplicationState_Starting:
                            {
                                char l_appKey[vr::k_unMaxApplicationKeyLength];
                                if(m_vrApplications->GetStartingApplication(l_appKey, vr::k_unMaxApplicationKeyLength) == vr::VRApplicationError_None) UpdateGameProfile(l_appKey);
                            } break;
                            case vr::EVRSceneApplicationState_None:
                                UpdateGameProfile(""); // Revert to default
                                break;
                        }
                    } break;
                }
                if(l_quitEvent) break;
            }

            // Process special combinations if NumLock is active
            if((GetKeyState(VK_NUMLOCK) & 0xFFFF) != 0)
            {
                bool l_hotkeyState = ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && (GetAsyncKeyState(0x58) & 0x8000)); // Ctrl+X 
                if(m_specialHotkey != l_hotkeyState)
                {
                    m_specialHotkey = l_hotkeyState;
                    if(m_specialHotkey)
                    {
                        SendCommand("game special_mode");
                        const std::string l_message("Special mode toggled");
                        SendNotification(l_message);
                    }
                }

                l_hotkeyState = ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && (GetAsyncKeyState(0x4F) & 0x8000)); // Ctrl+O
                {
                    if(m_leftHotkey != l_hotkeyState)
                    {
                        m_leftHotkey = l_hotkeyState;
                        if(m_leftHotkey)
                        {
                            SendCommand("setting left_hand");
                            const std::string l_message("Left hand toggled");
                            SendNotification(l_message);
                        }
                    }
                }

                l_hotkeyState = ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && (GetAsyncKeyState(0x50) & 0x8000)); // Ctrl+P
                {
                    if(m_rightHotkey != l_hotkeyState)
                    {
                        m_rightHotkey = l_hotkeyState;
                        if(m_rightHotkey)
                        {
                            SendCommand("setting right_hand");
                            const std::string l_message("Right hand toggled");
                            SendNotification(l_message);
                        }
                    }
                }

                l_hotkeyState = ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && (GetAsyncKeyState(0xDC) & 0x8000)); // Ctrl+§
                {
                    if(m_reloadHotkey != l_hotkeyState)
                    {
                        m_reloadHotkey = l_hotkeyState;
                        if(m_reloadHotkey)
                        {
                            SendCommand("setting reload_config");
                            const std::string l_message("Configuration reloaded");
                            SendNotification(l_message);
                        }
                    }
                }
            }

            std::this_thread::sleep_for(l_monitorInterval);
        }
    }
}

void CLeapMonitor::AddTrackedDevice(uint32_t unTrackedDeviceIndex)
{
    if(m_vrSystem->GetUint64TrackedDeviceProperty(unTrackedDeviceIndex, vr::Prop_VendorSpecific_Reserved_Start) == 0x4C4D6F74696F6E) m_relayDevice = unTrackedDeviceIndex;
}

void CLeapMonitor::RemoveTrackedDevice(uint32_t unTrackedDeviceIndex)
{
    if(m_relayDevice == unTrackedDeviceIndex) m_relayDevice = vr::k_unTrackedDeviceIndexInvalid;
}

void CLeapMonitor::SendCommand(const char *f_char)
{
    if(m_relayDevice != vr::k_unTrackedDeviceIndexInvalid)
    {
        char l_response[32U];
        m_vrDebug->DriverDebugRequest(m_relayDevice, f_char, l_response, 32U);
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

void CLeapMonitor::UpdateGameProfile(const char *f_appKey)
{
    GameProfile l_newProfile;
    switch(ReadEnumVector(f_appKey, g_steamAppKeys))
    {
        case SAI_VRChat: case SAI_VRChatNoSteam:
            l_newProfile = GP_VRChat;
            break;
        default:
            l_newProfile = GP_Default;
            break;
    }
    if(m_gameProfile != l_newProfile)
    {
        m_gameProfile = l_newProfile;

        std::string l_command("profile ");
        l_command.append(g_profileName[m_gameProfile]);
        SendCommand(l_command.c_str());

        std::string l_notifyText("Game profile has been changed to '");
        l_notifyText.append(g_profileName[m_gameProfile]);
        l_notifyText.push_back('\'');
        SendNotification(l_notifyText);
    }
}
