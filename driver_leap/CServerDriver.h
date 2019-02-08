#pragma once

class CLeapHandController;

class CLeapListener : public Leap::Listener
{
    virtual void onInit(const Leap::Controller&);
    virtual void onLogMessage(const Leap::Controller&, Leap::MessageSeverity severity, int64_t timestamp, const char* msg);
};

class CServerDriver : public vr::IServerTrackedDeviceProvider
{
    vr::IVRServerDriverHost* m_driverHost;
    static const char* const ms_interfaces[];

    bool m_bLaunchedLeapMonitor;
    PROCESS_INFORMATION m_processInfo;

    std::vector<CLeapHandController*> m_handControllers;
    Leap::Controller *m_leapController;
    CLeapListener m_leapListener;
    bool m_updatedVRConnectivity;

    void LaunchLeapMonitor();
public:
    CServerDriver();
    virtual ~CServerDriver();

    // vr::IServerTrackedDeviceProvider
    virtual vr::EVRInitError Init(vr::IVRDriverContext *pDriverContext);
    virtual void Cleanup();
    virtual const char* const* GetInterfaceVersions();
    virtual void RunFrame();

    virtual bool ShouldBlockStandbyMode();
    virtual void EnterStandby() {};
    virtual void LeaveStandby() {};
};
