#pragma once

class CLeapMonitor;

class CLeapListener : public Leap::Listener
{
    CLeapMonitor *m_monitor = nullptr;

    virtual void onInit(const Leap::Controller &controller);
    virtual void onLogMessage(const Leap::Controller &controller, Leap::MessageSeverity severity, int64_t timestamp, const char *msg);
public:
    inline void SetMonitor(CLeapMonitor *f_monitor) { m_monitor = f_monitor; }
};

class CLeapMonitor
{
    vr::IVRSystem *m_vrSystem;
    vr::IVROverlay *m_vrOverlay;
    vr::IVRNotifications *m_vrNotifications;
    vr::VROverlayHandle_t m_overlayHandle;
    vr::VRNotificationId m_notificationID;
    std::set<uint32_t> m_setLeapDevices;
    uint32_t m_lastApplication;

    Leap::Controller *m_leapController;
    CLeapListener m_leapListener;

    bool Init();
    void MainLoop();
    void Shutdown();

    /** Keep track of which devices are using driver_leap */
    void UpdateTrackedDevice(uint32_t unTrackedDeviceIndex);
    void UpdateApplicationKey(const char *f_appKey);
public:
    explicit CLeapMonitor();
    ~CLeapMonitor();

    void Run();

    void SendNotification(const std::string &f_text);
};