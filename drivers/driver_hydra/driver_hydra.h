#pragma once

//========= Copyright Valve Corporation ============//

#include <openvr_driver.h>
#include <sixense.h>
#include <sixense_utils/derivatives.hpp>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

class CHydraHmdLatest;

class CServerDriver_Hydra : public vr::IServerTrackedDeviceProvider
{
public:
	CServerDriver_Hydra();
	virtual ~CServerDriver_Hydra();

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

	void LaunchHydraMonitor();

private:
	static void ThreadEntry( CServerDriver_Hydra *pDriver );
	void ThreadFunc();
	void ScanForNewControllers( bool bNotifyServer );
	void CheckForChordedSystemButtons();

	void LaunchHydraMonitor( const char * pchDriverInstallDir );

	vr::IServerDriverHost* m_pDriverHost;
	std::string m_strDriverInstallDir;

	bool m_bLaunchedHydraMonitor;

	std::atomic<bool> m_bStopRequested;
	std::thread m_Thread;
	std::recursive_mutex m_Mutex;
	typedef std::lock_guard<std::recursive_mutex> scope_lock;
	std::vector< CHydraHmdLatest * > m_vecControllers;
};

class CClientDriver_Hydra : public vr::IClientTrackedDeviceProvider
{
public:
	CClientDriver_Hydra();
	virtual ~CClientDriver_Hydra();

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

class CHydraHmdLatest : public vr::ITrackedDeviceServerDriver, public vr::IVRControllerComponent
{
public:
	CHydraHmdLatest( vr::IServerDriverHost * pDriverHost, int base, int n );
	virtual ~CHydraHmdLatest();

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

	static const vr::EVRButtonId k_EButton_Button1 = ( vr::EVRButtonId ) 7;
	static const vr::EVRButtonId k_EButton_Button2 = ( vr::EVRButtonId ) 8;
	static const vr::EVRButtonId k_EButton_Button3 = ( vr::EVRButtonId ) 9;
	static const vr::EVRButtonId k_EButton_Button4 = vr::k_EButton_ApplicationMenu;
	static const vr::EVRButtonId k_EButton_Bumper  = vr::k_EButton_Grip; // Just for demo compatibility

	bool IsActivated() const;
	bool HasControllerId( int nBase, int nId );
	bool Update( sixenseControllerData & cd );
	bool IsHoldingSystemButton() const;
	void ConsumeSystemButtonPress();
	const char *GetSerialNumber();

	static void RealignCoordinates( CHydraHmdLatest * pHydraA, CHydraHmdLatest * pHydraB );
	void FinishRealignCoordinates( sixenseMath::Matrix3 & matHmdRotation, sixenseMath::Vector3 &vecHmdPosition );

private:
	static const float k_fScaleSixenseToMeters;
	static const std::chrono::milliseconds k_SystemButtonChordingDelay;
	static const std::chrono::milliseconds k_SystemButtonPulsingDuration;

	typedef void ( vr::IServerDriverHost::*ButtonUpdate )( uint32_t unWhichDevice, vr::EVRButtonId eButtonId, double eventTimeOffset );

	void SendButtonUpdates( ButtonUpdate ButtonEvent, uint64_t ulMask );
	void UpdateControllerState( sixenseControllerData & cd );
	void UpdateTrackingState( sixenseControllerData & cd );
	void DelaySystemButtonForChording( sixenseControllerData & cd );
	bool WaitingForHemisphereTracking( sixenseControllerData & cd );

	// Handle for calling back into vrserver with events and updates
	vr::IServerDriverHost *m_pDriverHost;

	// Which Hydra controller
	int m_nBase;
	int m_nId;
	std::string m_strSerialNumber;

	// Used to deduplicate state data from the sixense driver
	uint8_t m_ucPoseSequenceNumber;

	// To main structures for passing state to vrserver
	vr::DriverPose_t m_Pose;
	vr::VRControllerState_t m_ControllerState;

	// Ancillary tracking state
	sixenseMath::Vector3 m_WorldFromDriverTranslation;
	sixenseMath::Quat m_WorldFromDriverRotation;
	sixenseUtils::Derivatives m_Velocity;
	enum { k_eHemisphereTrackingDisabled, k_eHemisphereTrackingButtonDown, k_eHemisphereTrackingEnabled } m_eHemisphereTrackingState;
	bool m_bCalibrated;

	// Other controller with from the last realignment
	CHydraHmdLatest *m_pAlignmentPartner;

	// Timeout for system button chording
	std::chrono::steady_clock::time_point m_SystemButtonDelay;
	enum { k_eIdle, k_eWaiting, k_eSent, k_ePulsed, k_eBlocked } m_eSystemButtonState;

	// Cached for answering version queries from vrserver
	unsigned short m_firmware_revision;
	unsigned short m_hardware_revision;

	// Assigned by vrserver upon Activate().  The same ID visible to clients
	uint32_t m_unSteamVRTrackedDeviceId;

};