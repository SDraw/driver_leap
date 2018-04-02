#pragma once

class CLeapHmdLatest : public vr::ITrackedDeviceServerDriver, public vr::IVRControllerComponent
{
public:
    CLeapHmdLatest( vr::IServerDriverHost* pDriverHost, int base, int n );
    virtual ~CLeapHmdLatest();

    // Implementation of vr::ITrackedDeviceServerDriver
    virtual vr::EVRInitError Activate( uint32_t unObjectId ) override;
    virtual void Deactivate() override;
    void *GetComponent( const char* pchComponentNameAndVersion ) override;
    virtual void DebugRequest( const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize ) override;
    virtual vr::DriverPose_t GetPose() override;
    virtual bool GetBoolTrackedDeviceProperty( vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pError ) override;
    virtual float GetFloatTrackedDeviceProperty( vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pError ) override;
    virtual int32_t GetInt32TrackedDeviceProperty( vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pError ) override;
    virtual uint64_t GetUint64TrackedDeviceProperty( vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pError ) override;
    virtual vr::HmdMatrix34_t GetMatrix34TrackedDeviceProperty( vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* pError ) override;
    virtual uint32_t GetStringTrackedDeviceProperty( vr::ETrackedDeviceProperty prop, char* pchValue, uint32_t unBufferSize, vr::ETrackedPropertyError* pError ) override;
    virtual void EnterStandby() override;

    // Implementation of vr::IVRControllerComponent
    virtual vr::VRControllerState_t GetControllerState() override;
    virtual bool TriggerHapticPulse( uint32_t unAxisId, uint16_t usPulseDurationMicroseconds ) override;

    bool IsActivated() const;
    bool HasControllerId( int nBase, int nId ) const;
    bool Update(Leap::Frame& frame);
    const char* GetSerialNumber() const;

    static void RealignCoordinates( CLeapHmdLatest* pLeapA, CLeapHmdLatest* pLeapB );
    void FinishRealignCoordinates( float (*m)[3], float *v );
    void UpdateHmdPose(float* v, const vr::HmdQuaternion_t& q);

    uint32_t GetDeviceId() const { return m_unSteamVRTrackedDeviceId; }

private:
    static const std::chrono::milliseconds k_TrackingLatency;

    typedef void ( vr::IServerDriverHost::*ButtonUpdate )( uint32_t unWhichDevice, vr::EVRButtonId eButtonId, double eventTimeOffset );

    void SendButtonUpdates( ButtonUpdate ButtonEvent, uint64_t ulMask );
    void UpdateControllerState(Leap::Frame &frame);
    void UpdateTrackingState(Leap::Frame &frame);

    // Handle for calling back into vrserver with events and updates
    vr::IServerDriverHost *m_pDriverHost;

    // Which Leap controller
    int m_nBase;
    int m_nId;
    std::string m_strSerialNumber;

    // To main structures for passing state to vrserver
    vr::DriverPose_t m_Pose;
    vr::VRControllerState_t m_ControllerState;

    // Ancillary tracking state
    bool m_bCalibrated;
    float m_hmdPos[3];
    vr::HmdQuaternion_t m_hmdRot;

    // Other controller with from the last realignment
    CLeapHmdLatest *m_pAlignmentPartner;

    // Cached for answering version queries from vrserver
    unsigned short m_firmware_revision;
    unsigned short m_hardware_revision;

    // Assigned by vrserver upon Activate().  The same ID visible to clients
    uint32_t m_unSteamVRTrackedDeviceId;

    // The rendermodel used by the device. Check the contents of "c:\Program Files (x86)\Steam\steamapps\common\SteamVR\resources\rendermodels" for available models.
    std::string m_strRenderModel;

    // with this angle offset you can emulate the angle of a gun grip. Positive values tilt the controller up by N degrees.
    float m_gripAngleOffset;
};