#pragma once

class CLeapHmdLatest : public vr::ITrackedDeviceServerDriver, public vr::IVRControllerComponent
{
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

    bool Update(Leap::Frame& frame);
    const char* GetSerialNumber() const;

    static void RealignCoordinates(CLeapHmdLatest* pLeapA, CLeapHmdLatest* pLeapB);
    void FinishRealignCoordinates(float(*m)[3], float *v);
    void UpdateHmdPose(float* v, const vr::HmdQuaternion_t& q);
private:
    static const std::chrono::milliseconds k_TrackingLatency;

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

    CLeapHmdLatest *m_pAlignmentPartner;

    unsigned short m_firmware_revision;
    unsigned short m_hardware_revision;
    uint32_t m_unSteamVRTrackedDeviceId;
    std::string m_strRenderModel;

    Leap::Vector m_gripAngleOffset;
};