#pragma once

class CLeapMonitor;

class CLeapListener final : public Leap::Listener
{
    std::atomic<CLeapMonitor*> m_monitor = nullptr;

    virtual void onConnect(const Leap::Controller &controller);
    virtual void onDisconnect(const Leap::Controller &controller);
    virtual void onInit(const Leap::Controller &controller);
    virtual void onLogMessage(const Leap::Controller &controller, Leap::MessageSeverity severity, int64_t timestamp, const char *msg); // Async
    virtual void onServiceConnect(const Leap::Controller &controller);
    virtual void onServiceDisconnect(const Leap::Controller &controller);
public:
    void SetMonitor(CLeapMonitor *f_monitor); // Async method
};

class CLeapMonitor final
{
    enum GameProfile : unsigned char
    {
        GP_Default = 0U,
        GP_VRChat
    };

    std::atomic<bool> m_initialized;

    vr::IVRApplications *m_vrApplications;
    vr::IVRDebug *m_vrDebug;
    vr::IVRNotifications *m_vrNotifications;
    vr::IVROverlay *m_vrOverlay;
    vr::IVRSystem *m_vrSystem;
    vr::VRNotificationId m_notificationID;
    vr::VROverlayHandle_t m_overlayHandle;

    Leap::Controller *m_leapController;
    CLeapListener m_leapListener;

    GameProfile m_gameProfile;
    std::mutex m_notificationLock;
    uint32_t m_relayDevice;
    bool m_specialHotkey;
    bool m_leftHotkey;
    bool m_rightHotkey;

    CLeapMonitor(const CLeapMonitor &that) = delete;
    CLeapMonitor& operator=(const CLeapMonitor &that) = delete;

    void AddTrackedDevice(uint32_t unTrackedDeviceIndex);
    void RemoveTrackedDevice(uint32_t unTrackedDeviceIndex);

    void SendCommand(const char *f_cmd);
    void UpdateGameProfile(const char *f_appKey);
public:
    CLeapMonitor();
    ~CLeapMonitor();

    bool Initialize();
    void Terminate();
    void Run();

    void SendNotification(const std::string &f_text); // Async method
};
