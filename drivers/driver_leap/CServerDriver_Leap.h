#pragma once

class CLeapHmdLatest;
class CServerDriver_Leap : public vr::IServerTrackedDeviceProvider, public Leap::Listener
{
    void ScanForNewControllers(bool bNotifyServer);

    void LaunchLeapMonitor(const char* pchDriverInstallDir);

    vr::IServerDriverHost* m_pDriverHost;
    std::string m_strDriverInstallDir;

    bool m_bLaunchedLeapMonitor;
    PROCESS_INFORMATION m_pInfoStartedProcess;

    // SteamVR's tracked controller objects
    std::vector<CLeapHmdLatest*> m_vecControllers;

    // Leap Motion's Controller object
    Leap::Controller* m_Controller;

    // a mutex for thread safety (Leap::Listener callbacks arrive from different threads)
    //    std::recursive_mutex m_Mutex;
    //    typedef std::lock_guard<std::recursive_mutex> scope_lock;
public:
    CServerDriver_Leap();
    virtual ~CServerDriver_Leap();

    // Inherited via IServerTrackedDeviceProvider
    virtual vr::EVRInitError Init(vr::IDriverLog* pDriverLog, vr::IServerDriverHost* pDriverHost, const char* pchUserDriverConfigDir, const char* pchDriverInstallDir) override;
    virtual void Cleanup() override;
    virtual uint32_t GetTrackedDeviceCount() override;
    virtual vr::ITrackedDeviceServerDriver* GetTrackedDeviceDriver(uint32_t unWhich) override;
    virtual vr::ITrackedDeviceServerDriver* FindTrackedDeviceDriver(const char* pchId) override;
    virtual const char* const* GetInterfaceVersions() { return vr::k_InterfaceVersions; }
    virtual void RunFrame() override;

    virtual bool ShouldBlockStandbyMode() override;
    virtual void EnterStandby() override;
    virtual void LeaveStandby() override;

    void LaunchLeapMonitor();

    // Leap::Listener interface
    void onInit(const Leap::Controller&);
    void onConnect(const Leap::Controller&);
    void onDisconnect(const Leap::Controller&);
    void onExit(const Leap::Controller&);
    void onFrame(const Leap::Controller&);
    void onFocusGained(const Leap::Controller&);
    void onFocusLost(const Leap::Controller&);
    void onServiceConnect(const Leap::Controller&);
    void onServiceDisconnect(const Leap::Controller&);
    void onDeviceChange(const Leap::Controller&);
    void onImages(const Leap::Controller&);
    void onServiceChange(const Leap::Controller&);
    void onDeviceFailure(const Leap::Controller&);
    void onLogMessage(const Leap::Controller&, Leap::MessageSeverity severity, int64_t timestamp, const char* msg);
};
