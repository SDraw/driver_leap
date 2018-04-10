#pragma once

class CLeapHmdLatest : public vr::ITrackedDeviceServerDriver, public vr::IVRControllerComponent
{
    typedef void (vr::IVRServerDriverHost::*ButtonUpdate)(uint32_t unWhichDevice, vr::EVRButtonId eButtonId, double eventTimeOffset);

    void SendButtonUpdates(ButtonUpdate ButtonEvent, uint64_t ulMask);
    void UpdateControllerState(Leap::Frame &frame);
    void UpdateTrackingState(Leap::Frame &frame);

    vr::IVRServerDriverHost *m_pDriverHost;

    int m_nId;
    std::string m_strSerialNumber;

    vr::DriverPose_t m_Pose;
    vr::VRControllerState_t m_ControllerState;

    bool m_bCalibrated;
    float m_hmdPos[3];
    vr::HmdQuaternion_t m_hmdRot;
    bool m_connected;

    unsigned short m_firmware_revision;
    unsigned short m_hardware_revision;
    uint32_t m_unSteamVRTrackedDeviceId;
    std::string m_strRenderModel;

    Leap::Vector m_gripAngleOffset;
public:
    CLeapHmdLatest(vr::IVRServerDriverHost* pDriverHost, int n);
    virtual ~CLeapHmdLatest();

    // vr::ITrackedDeviceServerDriver
    virtual vr::EVRInitError Activate(uint32_t unObjectId) override;
    virtual void Deactivate() override;
    void* GetComponent(const char* pchComponentNameAndVersion) override;
    virtual void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) override;
    virtual vr::DriverPose_t GetPose() override;
    virtual void EnterStandby() override;

    // vr::IVRControllerComponent
    virtual vr::VRControllerState_t GetControllerState() override;
    virtual bool TriggerHapticPulse(uint32_t unAxisId, uint16_t usPulseDurationMicroseconds) override;

    void Update(Leap::Frame& frame);
    const char* GetSerialNumber() const;

    void RealignCoordinates();
    void SetAsDisconnected();
};