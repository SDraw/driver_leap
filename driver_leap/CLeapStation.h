#pragma once

class CServerDriver;

class CLeapStation final : public vr::ITrackedDeviceServerDriver
{
    vr::DriverPose_t m_pose;
    vr::PropertyContainerHandle_t m_propertyHandle;
    uint32_t m_trackedDevice;

    CServerDriver *m_serverDriver;
    std::string m_serialNumber;

    CLeapStation(const CLeapStation &that) = delete;
    CLeapStation& operator=(const CLeapStation &that) = delete;

    // vr::ITrackedDeviceServerDriver
    vr::EVRInitError Activate(uint32_t unObjectId);
    void Deactivate();
    void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize);
    void EnterStandby() {}
    void* GetComponent(const char* pchComponentNameAndVersion);
    vr::DriverPose_t GetPose();
protected:
    explicit CLeapStation(CServerDriver *f_server);
    ~CLeapStation();

    inline const std::string& GetSerialNumber() const { return m_serialNumber; }

    void RunFrame();

    friend CServerDriver;
};
