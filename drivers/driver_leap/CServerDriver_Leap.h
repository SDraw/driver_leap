#pragma once

class CLeapHmdLatest;
class CServerDriver_Leap : public vr::IServerTrackedDeviceProvider, public Leap::Listener
{

    vr::IVRServerDriverHost* m_pDriverHost;

    bool m_bLaunchedLeapMonitor;
    PROCESS_INFORMATION m_pInfoStartedProcess;

    std::vector<CLeapHmdLatest*> m_vecVRControllers;
    Leap::Controller* m_Controller;

    void LaunchLeapMonitor();
public:
    CServerDriver_Leap();
    virtual ~CServerDriver_Leap();

    // vr::IServerTrackedDeviceProvider
    virtual vr::EVRInitError Init(vr::IVRDriverContext *pDriverContext) override;
    virtual void Cleanup() override;
    virtual const char* const* GetInterfaceVersions() override { return vr::k_InterfaceVersions; }
    virtual void RunFrame() override;

    virtual bool ShouldBlockStandbyMode() override;
    virtual void EnterStandby() override;
    virtual void LeaveStandby() override;

    // Leap::Listener
    void onInit(const Leap::Controller&);
    void onConnect(const Leap::Controller&);
    void onDisconnect(const Leap::Controller&);
    void onExit(const Leap::Controller&);
    void onServiceConnect(const Leap::Controller&);
    void onServiceDisconnect(const Leap::Controller&);
    void onDeviceChange(const Leap::Controller&);
    void onServiceChange(const Leap::Controller&);
    void onDeviceFailure(const Leap::Controller&);
    void onLogMessage(const Leap::Controller&, Leap::MessageSeverity severity, int64_t timestamp, const char* msg);
};
