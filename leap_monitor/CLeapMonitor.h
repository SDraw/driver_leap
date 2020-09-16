#pragma once

class CLeapListener;

class CLeapMonitor final
{
    enum GameProfile : unsigned char
    {
        GP_Default = 0U,
        GP_VRChat
    };

    std::atomic<bool> m_active;

    vr::IVRSystem *m_vrSystem;
    vr::VRNotificationId m_notificationID;
    vr::VROverlayHandle_t m_overlayHandle;
    vr::VREvent_t m_event;

    Leap::Controller *m_leapController;
    CLeapListener *m_leapListener;

    GameProfile m_gameProfile;
    std::mutex m_notificationLock;
    uint32_t m_relayDevice;
    bool m_leftHotkey;
    bool m_rightHotkey;
    bool m_reloadHotkey;

    CLeapMonitor(const CLeapMonitor &that) = delete;
    CLeapMonitor& operator=(const CLeapMonitor &that) = delete;

    void SendCommand(const char *f_cmd);
    void UpdateGameProfile(const char *f_appKey);
public:
    CLeapMonitor();
    ~CLeapMonitor();

    bool Initialize();
    void Terminate();
    bool DoPulse();

    void SendNotification(const char *f_text); // Async method
};
