#pragma once

class CLeapHandController;

class CLeapListener final : public Leap::Listener
{
    virtual void onInit(const Leap::Controller &controller);
    virtual void onLogMessage(const Leap::Controller &controller, Leap::MessageSeverity severity, int64_t timestamp, const char *msg);
};

class CServerDriver final : public vr::IServerTrackedDeviceProvider
{
    vr::IVRServerDriverHost* m_driverHost;
    static const char* const ms_interfaces[];

    bool m_leapMonitorLaunched;
    PROCESS_INFORMATION m_processInfo;

    std::vector<CLeapHandController*> m_handControllers;
    Leap::Controller *m_leapController;
    CLeapListener m_leapListener;
    bool m_controllerState;

    void LaunchLeapMonitor();

    // vr::IServerTrackedDeviceProvider
    vr::EVRInitError Init(vr::IVRDriverContext *pDriverContext);
    void Cleanup();
    const char* const* GetInterfaceVersions();
    void RunFrame();
    bool ShouldBlockStandbyMode();
    void EnterStandby() {};
    void LeaveStandby() {};

    CServerDriver(const CServerDriver &that) = delete;
    CServerDriver& operator=(const CServerDriver &that) = delete;
public:
    CServerDriver();
    ~CServerDriver();
};
