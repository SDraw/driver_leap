#pragma once

class CLeapHmdLatest;
class CServerDriver_Leap : public vr::IServerTrackedDeviceProvider, public Leap::Listener
{

    vr::IVRServerDriverHost* m_pDriverHost;
    static const char* const ms_interfaces[];

    bool m_bLaunchedLeapMonitor;
    PROCESS_INFORMATION m_pInfoStartedProcess;

    std::vector<CLeapHmdLatest*> m_vecVRControllers;
    Leap::Controller *m_Controller;
    bool m_updatedVRConnectivity;

    void LaunchLeapMonitor();
public:
    CServerDriver_Leap();
    virtual ~CServerDriver_Leap();

    // vr::IServerTrackedDeviceProvider
    virtual vr::EVRInitError Init(vr::IVRDriverContext *pDriverContext);
    virtual void Cleanup();
    virtual const char* const* GetInterfaceVersions() { return ms_interfaces; }
    virtual void RunFrame();

    virtual bool ShouldBlockStandbyMode();
    virtual void EnterStandby();
    virtual void LeaveStandby();

    // Leap::Listener
    virtual void onInit(const Leap::Controller&);
    virtual void onLogMessage(const Leap::Controller&, Leap::MessageSeverity severity, int64_t timestamp, const char* msg);
};
