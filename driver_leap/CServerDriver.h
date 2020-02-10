#pragma once

class CLeapController;
class CRelayDevice;

class CLeapListener final : public Leap::Listener
{
    virtual void onInit(const Leap::Controller &controller);
    virtual void onLogMessage(const Leap::Controller &controller, Leap::MessageSeverity severity, int64_t timestamp, const char *msg);
};

class CServerDriver final : public vr::IServerTrackedDeviceProvider
{
    vr::IVRServerDriverHost* m_driverHost;
    static const char* const ms_interfaces[];

    Leap::Controller *m_leapController;
    CLeapListener m_leapListener;

    bool m_connectionState;
    bool m_firstConnection;
    std::vector<CLeapController*> m_controllers;
    CRelayDevice *m_relayDevice;

    bool m_monitorLaunched;
    PROCESS_INFORMATION m_monitorInfo;

    CServerDriver(const CServerDriver &that) = delete;
    CServerDriver& operator=(const CServerDriver &that) = delete;

    void ProcessLeapControllerPause();

    // vr::IServerTrackedDeviceProvider
    void Cleanup();
    void EnterStandby() {};
    const char* const* GetInterfaceVersions();
    vr::EVRInitError Init(vr::IVRDriverContext *pDriverContext);
    void LeaveStandby() {};
    void RunFrame();
    bool ShouldBlockStandbyMode();
public:
    CServerDriver();
    ~CServerDriver();

    void ProcessExternalMessage(const char *f_message);
};
