#pragma once

//========= Copyright Valve Corporation ============//

#include <openvr_driver.h>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

#include "Leap.h"
#include "GestureMatcher.h"

using namespace Leap;

class CLeapHmdLatest;

class CServerDriver_Leap : public vr::IServerTrackedDeviceProvider, public Listener
{
public:
    CServerDriver_Leap();
    virtual ~CServerDriver_Leap();

    // Inherited via IServerTrackedDeviceProvider
    virtual vr::EVRInitError Init( vr::IDriverLog * pDriverLog, vr::IServerDriverHost * pDriverHost, const char * pchUserDriverConfigDir, const char * pchDriverInstallDir ) override;
    virtual void Cleanup() override;
    virtual uint32_t GetTrackedDeviceCount() override;
    virtual vr::ITrackedDeviceServerDriver * GetTrackedDeviceDriver( uint32_t unWhich, const char *pchInterfaceVersion ) override;
    virtual vr::ITrackedDeviceServerDriver * FindTrackedDeviceDriver( const char * pchId, const char *pchInterfaceVersion ) override;
    virtual void RunFrame() override;

    virtual bool ShouldBlockStandbyMode() override;
    virtual void EnterStandby() override;
    virtual void LeaveStandby() override;

    void LaunchLeapMonitor();

    // Leap::Listener interface
    void onInit(const Controller&);
    void onConnect(const Controller&);
    void onDisconnect(const Controller&);
    void onExit(const Controller&);
    void onFrame(const Controller&);
    void onFocusGained(const Controller&);
    void onFocusLost(const Controller&);
    void onServiceConnect(const Controller&);
    void onServiceDisconnect(const Controller&);
    void onDeviceChange(const Controller&);
    void onImages(const Controller&);
    void onServiceChange(const Controller&);
    void onDeviceFailure(const Controller&);
    void onLogMessage(const Controller&, MessageSeverity severity, int64_t timestamp, const char* msg);


private:
    void ScanForNewControllers( bool bNotifyServer );

    void LaunchLeapMonitor( const char * pchDriverInstallDir );

    vr::IServerDriverHost* m_pDriverHost;
    std::string m_strDriverInstallDir;

    bool m_bLaunchedLeapMonitor;
    PROCESS_INFORMATION m_pInfoStartedProcess;

    // SteamVR's tracked controller objects
    std::vector< CLeapHmdLatest * > m_vecControllers;

    // Leap Motion's Controller object
    Controller *m_Controller;

    // a mutex for thread safety (Leap::Listener callbacks arrive from different threads)
//    std::recursive_mutex m_Mutex;
//    typedef std::lock_guard<std::recursive_mutex> scope_lock;
};

class CClientDriver_Leap : public vr::IClientTrackedDeviceProvider
{
public:
    CClientDriver_Leap();
    virtual ~CClientDriver_Leap();

    // Inherited via IClientTrackedDeviceProvider
    virtual vr::EVRInitError Init( vr::IDriverLog * pDriverLog, vr::IClientDriverHost * pDriverHost, const char * pchUserDriverConfigDir, const char * pchDriverInstallDir ) override;
    virtual void Cleanup() override;
    virtual bool BIsHmdPresent( const char * pchUserConfigDir ) override;
    virtual vr::EVRInitError SetDisplayId( const char * pchDisplayId ) override;
    virtual vr::HiddenAreaMesh_t GetHiddenAreaMesh( vr::EVREye eEye ) override;
    virtual uint32_t GetMCImage( uint32_t *pImgWidth, uint32_t *pImgHeight, uint32_t *pChannels, void *pDataBuffer, uint32_t unBufferLen ) override;

private:
    vr::IClientDriverHost* m_pDriverHost;

};

class CLeapHmdLatest : public vr::ITrackedDeviceServerDriver, public vr::IVRControllerComponent
{
public:
    CLeapHmdLatest( vr::IServerDriverHost * pDriverHost, int base, int n );
    virtual ~CLeapHmdLatest();

    // Implementation of vr::ITrackedDeviceServerDriver
    virtual vr::EVRInitError Activate( uint32_t unObjectId ) override;
    virtual void Deactivate() override;
    virtual void PowerOff() override;
    void *GetComponent( const char *pchComponentNameAndVersion ) override;
    virtual void DebugRequest( const char * pchRequest, char * pchResponseBuffer, uint32_t unResponseBufferSize ) override;
    virtual vr::DriverPose_t GetPose() override;
    virtual bool GetBoolTrackedDeviceProperty( vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError * pError ) override;
    virtual float GetFloatTrackedDeviceProperty( vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError * pError ) override;
    virtual int32_t GetInt32TrackedDeviceProperty( vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError * pError ) override;
    virtual uint64_t GetUint64TrackedDeviceProperty( vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError * pError ) override;
    virtual vr::HmdMatrix34_t GetMatrix34TrackedDeviceProperty( vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *pError ) override;
    virtual uint32_t GetStringTrackedDeviceProperty( vr::ETrackedDeviceProperty prop, char * pchValue, uint32_t unBufferSize, vr::ETrackedPropertyError * pError ) override;

    // Implementation of vr::IVRControllerComponent
    virtual vr::VRControllerState_t GetControllerState() override;
    virtual bool TriggerHapticPulse( uint32_t unAxisId, uint16_t usPulseDurationMicroseconds ) override;

    bool IsActivated() const;
    bool HasControllerId( int nBase, int nId );
    bool Update(Frame &frame);
    const char *GetSerialNumber();

    static void RealignCoordinates( CLeapHmdLatest * pLeapA, CLeapHmdLatest * pLeapB );
    void FinishRealignCoordinates( float (*m)[3], float *v );
    void UpdateHmdPose(float *v, vr::HmdQuaternion_t q);

    uint32_t GetDeviceId() { return m_unSteamVRTrackedDeviceId; }

private:
    static const std::chrono::milliseconds k_TrackingLatency;

    typedef void ( vr::IServerDriverHost::*ButtonUpdate )( uint32_t unWhichDevice, vr::EVRButtonId eButtonId, double eventTimeOffset );

    void SendButtonUpdates( ButtonUpdate ButtonEvent, uint64_t ulMask );
    void UpdateControllerState(Frame &frame);
    void UpdateTrackingState(Frame &frame);

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

    // a helper object to identify hand poses
    GestureMatcher matcher;
};