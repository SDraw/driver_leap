#pragma once

class CLeapMonitor;

class CLeapListener final : public Leap::Listener
{
    CLeapMonitor *m_monitor = nullptr;

    virtual void onConnect(const Leap::Controller &controller);
    virtual void onDisconnect(const Leap::Controller &controller);
    virtual void onLogMessage(const Leap::Controller &controller, Leap::MessageSeverity severity, int64_t timestamp, const char *msg); // Async
    virtual void onServiceConnect(const Leap::Controller &controller);
    virtual void onServiceDisconnect(const Leap::Controller &controller);
public:
    void SetMonitor(CLeapMonitor *f_monitor);
};

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
