#include "stdafx.h"

#include "CLeapMonitor.h"
#include "CLeapListener.h"

#include "Utils.h"

const std::chrono::milliseconds g_threadDelay(11U);

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
    m_active = false;

    m_vrSystem = nullptr;
    m_notificationID = 0U;
    m_overlayHandle = vr::k_ulOverlayHandleInvalid;

    m_leapController = nullptr;
    m_leapListener = nullptr;

    m_gameProfile = GP_Default;
    m_relayDevice = vr::k_unTrackedDeviceIndexInvalid;
    m_leftHotkey = false;
    m_rightHotkey = false;
    m_reloadHotkey = false;
}
CLeapMonitor::~CLeapMonitor()
{
}

bool CLeapMonitor::Initialize()
{
    if(!m_active)
    {
        vr::EVRInitError eVRInitError;
        m_vrSystem = vr::VR_Init(&eVRInitError, vr::VRApplication_Background);
        if(eVRInitError == vr::VRInitError_None)
        {
            vr::VROverlay()->CreateOverlay("leap_monitor_overlay", "Leap Motion Monitor", &m_overlayHandle);

            for(uint32_t i = 0U; i < vr::k_unMaxTrackedDeviceCount; i++)
            {
                if(m_vrSystem->GetUint64TrackedDeviceProperty(i, vr::Prop_VendorSpecific_Reserved_Start) == 0x4C4D6F74696F6E)
                {
                    m_relayDevice = m_event.trackedDeviceIndex;
                    break;
                }
            }

            m_leapListener = new CLeapListener();
            m_leapListener->SetMonitor(this);

            m_leapController = new Leap::Controller();
            m_leapController->addListener(*m_leapListener);

            m_active = true;
        }
    }
    return m_active;
}

void CLeapMonitor::Terminate()
{
    if(m_active)
    {
        m_active = false;

        m_leapController->removeListener(*m_leapListener);
        m_leapListener->SetMonitor(nullptr);

        delete m_leapController;
        m_leapController = nullptr;

        delete m_leapListener;
        m_leapListener = nullptr;

        if(m_notificationID) vr::VRNotifications()->RemoveNotification(m_notificationID);
        vr::VROverlay()->DestroyOverlay(m_overlayHandle);
        m_overlayHandle = vr::k_ulOverlayHandleInvalid;
        m_notificationID = 0U;

        vr::VR_Shutdown();

        m_vrSystem = nullptr;

        m_gameProfile = GP_Default;
    }

    m_active = false;
}

bool CLeapMonitor::DoPulse()
{
    while(m_vrSystem->PollNextEvent(&m_event, sizeof(vr::VREvent_t)))
    {
        switch(m_event.eventType)
        {
            case vr::VREvent_Quit:
                m_active = false;
                break;
            case vr::VREvent_TrackedDeviceActivated:
            {
                if(m_vrSystem->GetUint64TrackedDeviceProperty(m_event.trackedDeviceIndex, vr::Prop_VendorSpecific_Reserved_Start) == 0x4C4D6F74696F6E) m_relayDevice = m_event.trackedDeviceIndex;
            } break;
            case vr::VREvent_TrackedDeviceDeactivated:
            {
                if(m_relayDevice == m_event.trackedDeviceIndex) m_relayDevice = vr::k_unTrackedDeviceIndexInvalid;
            } break;
            case vr::VREvent_SceneApplicationStateChanged:
            {
                vr::EVRSceneApplicationState l_appState = vr::VRApplications()->GetSceneApplicationState();
                switch(l_appState)
                {
                    case vr::EVRSceneApplicationState_Starting:
                    {
                        char l_appKey[vr::k_unMaxApplicationKeyLength];
                        if(vr::VRApplications()->GetStartingApplication(l_appKey, vr::k_unMaxApplicationKeyLength) == vr::VRApplicationError_None) UpdateGameProfile(l_appKey);
                    } break;
                    case vr::EVRSceneApplicationState_None:
                        UpdateGameProfile(""); // Revert to default
                        break;
                }
            } break;
        }
    }

    // Process special combinations if NumLock is active
    if((GetKeyState(VK_NUMLOCK) & 0xFFFF) != 0)
    {
        if(GetAsyncKeyState(VK_CONTROL) & 0x8000)
        {
            bool l_hotkeyState = (GetAsyncKeyState(0x4F) & 0x8000); // Ctrl+O
            {
                if(m_leftHotkey != l_hotkeyState)
                {
                    m_leftHotkey = l_hotkeyState;
                    if(m_leftHotkey)
                    {
                        SendCommand("setting left_hand");
                        SendNotification("Left hand toggled");
                    }
                }
            }

            l_hotkeyState = (GetAsyncKeyState(0x50) & 0x8000); // Ctrl+P
            {
                if(m_rightHotkey != l_hotkeyState)
                {
                    m_rightHotkey = l_hotkeyState;
                    if(m_rightHotkey)
                    {
                        SendCommand("setting right_hand");
                        SendNotification("Right hand toggled");
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
                        SendNotification("Configuration reloaded");
                    }
                }
            }
        }
    }

    std::this_thread::sleep_for(g_threadDelay);
    return m_active;
}

void CLeapMonitor::SendCommand(const char *f_char)
{
    if(m_relayDevice != vr::k_unTrackedDeviceIndexInvalid)
    {
        char l_response[32U];
        vr::VRDebug()->DriverDebugRequest(m_relayDevice, f_char, l_response, 32U);
    }
}

void CLeapMonitor::SendNotification(const char *f_text)
{
    if(m_active)
    {
        if(std::strlen(f_text) != 0U)
        {
            m_notificationLock.lock();
            if(m_notificationID) vr::VRNotifications()->RemoveNotification(m_notificationID);
            vr::VRNotifications()->CreateNotification(m_overlayHandle, 500U, vr::EVRNotificationType_Transient, f_text, vr::EVRNotificationStyle_None, nullptr, &m_notificationID);
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
        SendNotification(l_notifyText.c_str());
    }
}
