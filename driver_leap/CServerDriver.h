#pragma once

class CLeapController;
class CLeapStation;

class CServerDriver final : public vr::IServerTrackedDeviceProvider
{
    enum LeapControllerHand : size_t
    {
        LCH_Left = 0U,
        LCH_Right = 1U,

        LCH_Count
    };

    static const char* const ms_interfaces[];

    Leap::Controller *m_leapController;

    bool m_connectionState;
    bool m_firstConnection;
    CLeapController *m_controllers[LCH_Count];
    CLeapStation *m_leapStation;

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
