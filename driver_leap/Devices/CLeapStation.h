#pragma once

class CLeapIndexController;
class CLeapStation final : public vr::ITrackedDeviceServerDriver
{
    vr::DriverPose_t m_pose;
    vr::PropertyContainerHandle_t m_propertyHandle;
    uint32_t m_trackedDevice;

    std::string m_serialNumber;

    CLeapIndexController *m_leftController;
    CLeapIndexController *m_rightController;

    CLeapStation(const CLeapStation &that) = delete;
    CLeapStation& operator=(const CLeapStation &that) = delete;
    void ProcessExternalMessage(const char *p_message);

    // vr::ITrackedDeviceServerDriver
    vr::EVRInitError Activate(uint32_t unObjectId);
    void Deactivate();
    void EnterStandby();
    void* GetComponent(const char* pchComponentNameAndVersion);
    void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize);
    vr::DriverPose_t GetPose();
public:
    enum TrackingState : unsigned char
    {
        TS_Connected = 0U,
        TS_Search
    };

    CLeapStation();
    ~CLeapStation() = default;

    const std::string& GetSerialNumber() const;

    void SetControllers(CLeapIndexController *p_left, CLeapIndexController *p_right);

    void SetTrackingState(TrackingState p_state);

    void RunFrame();
};
