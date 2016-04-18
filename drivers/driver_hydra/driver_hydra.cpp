//========= Copyright Valve Corporation ============//
//
// driver_hydra.cpp : Defines the client and server interfaces used by the SteamVR runtime.
//

#include "pch.h"
#include "driver_hydra.h"
#include <sixense.h>
#include <sixense_math.hpp>

#include <sstream>

#ifdef _WIN32
#include <mmsystem.h>  // for timeBeginPeriod()
#pragma comment(lib, "winmm.lib")
#endif

#define HMD_DLL_EXPORT extern "C" __declspec( dllexport )

CServerDriver_Hydra g_ServerTrackedDeviceProvider;
CClientDriver_Hydra g_ClientTrackedDeviceProvider;

HMD_DLL_EXPORT
void *HmdDriverFactory( const char *pInterfaceName, int *pReturnCode )
{
	if ( 0 == strcmp( vr::IServerTrackedDeviceProvider_Version, pInterfaceName ) )
	{
		return &g_ServerTrackedDeviceProvider;
	}
	if ( 0 == strcmp( vr::IClientTrackedDeviceProvider_Version, pInterfaceName ) )
	{
		return &g_ClientTrackedDeviceProvider;
	}

	if ( pReturnCode )
		*pReturnCode = vr::VRInitError_Init_InterfaceNotFound;

	return NULL;
}

//==================================================================================================
// Logging helpers
//==================================================================================================

static vr::IDriverLog * s_pLogFile = NULL;

static bool InitDriverLog( vr::IDriverLog *pDriverLog )
{
	if ( s_pLogFile )
		return false;
	s_pLogFile = pDriverLog;
	return s_pLogFile != NULL;
}

static void CleanupDriverLog()
{
	s_pLogFile = NULL;
}

static void DriverLogVarArgs( const char *pMsgFormat, va_list args )
{
	char buf[1024];
#if defined( WIN32 )
	vsprintf_s( buf, pMsgFormat, args );
#else
	vsnprintf( buf, sizeof( buf ), pMsgFormat, args );
#endif

	if ( s_pLogFile )
		s_pLogFile->Log( buf );
}

/** Provides printf-style debug logging via the vr::IDriverLog interface provided by SteamVR
* during initialization.  Client logging ends up in vrclient_appname.txt and server logging
* ends up in vrserver.txt.
*/
static void DriverLog( const char *pMsgFormat, ... )
{
	va_list args;
	va_start( args, pMsgFormat );

	DriverLogVarArgs( pMsgFormat, args );

	va_end( args );
}

//==================================================================================================
// Server Provider
//==================================================================================================

CServerDriver_Hydra::CServerDriver_Hydra()
	: m_bStopRequested( false )
	, m_bLaunchedHydraMonitor( false )
{
}

CServerDriver_Hydra::~CServerDriver_Hydra()
{
	// 10/10/2015 benj:  vrserver is exiting without calling Cleanup() to balance Init()
	// causing std::thread to call std::terminate
	Cleanup();
}

vr::EVRInitError CServerDriver_Hydra::Init( vr::IDriverLog * pDriverLog, vr::IServerDriverHost * pDriverHost, const char * pchUserDriverConfigDir, const char * pchDriverInstallDir )
{
	InitDriverLog( pDriverLog );
	m_pDriverHost = pDriverHost;
	m_strDriverInstallDir = pchDriverInstallDir;

	if ( sixenseInit() != SIXENSE_SUCCESS )
		return vr::VRInitError_Driver_Failed;

	// Will not immediately detect controllers at this point.  Sixense driver must be initializing
	// in its own thread...  It's okay to dynamically detect devices later, but if controllers are
	// the only devices (e.g. requireHmd=false) we must have GetTrackedDeviceCount() != 0 before returning.
	for ( int i = 0; i < 20; ++i )
	{
		ScanForNewControllers( false );
		if ( GetTrackedDeviceCount() )
			break;
		Sleep( 100 );
	}

	m_Thread = std::thread( ThreadEntry, this );

	return vr::VRInitError_None;
}

void CServerDriver_Hydra::Cleanup()
{
	if ( m_Thread.joinable() )
	{
		m_bStopRequested = true;
		m_Thread.join();
		sixenseExit();
	}
}

uint32_t CServerDriver_Hydra::GetTrackedDeviceCount()
{
	scope_lock lock( m_Mutex );

	return m_vecControllers.size();
}

vr::ITrackedDeviceServerDriver * CServerDriver_Hydra::GetTrackedDeviceDriver( uint32_t unWhich, const char *pchInterfaceVersion )
{
	// don't return anything if that's not the interface version we have
	if ( 0 != stricmp( pchInterfaceVersion, vr::ITrackedDeviceServerDriver_Version ) )
	{
		DriverLog( "FindTrackedDeviceDriver for version %s, which we don't support.\n",
			pchInterfaceVersion );
		return NULL;
	}

	scope_lock lock( m_Mutex );

	if ( unWhich < m_vecControllers.size() )
		return m_vecControllers[unWhich];

	return nullptr;
}

vr::ITrackedDeviceServerDriver * CServerDriver_Hydra::FindTrackedDeviceDriver( const char * pchId, const char *pchInterfaceVersion )
{
	// don't return anything if that's not the interface version we have
	if ( 0 != stricmp( pchInterfaceVersion, vr::ITrackedDeviceServerDriver_Version ) )
	{
		DriverLog( "FindTrackedDeviceDriver for version %s, which we don't support.\n",
			pchInterfaceVersion );
		return NULL;
	}

	scope_lock lock( m_Mutex );

	for ( auto it = m_vecControllers.begin(); it != m_vecControllers.end(); ++it )
	{
		if ( 0 == strcmp( ( *it )->GetSerialNumber(), pchId ) )
		{
			return *it;
		}
	}
	return nullptr;
}

void CServerDriver_Hydra::RunFrame()
{
	// This would be an appropriate place to call ScanForNewControllers( true ),
	// and put entries in a processing list for ThreadFunc().  However, due to the
	// modal nature of the sixense API, we can't scan for devices without locking
	// against ThreadFunc anyway, so it's easier to do it all in there.
}

bool CServerDriver_Hydra::ShouldBlockStandbyMode()
{
	return false;
}

void CServerDriver_Hydra::EnterStandby()
{
}

void CServerDriver_Hydra::LeaveStandby()
{
}

static void GenerateSerialNumber( char *p, int psize, int base, int controller )
{
	_snprintf( p, psize, "hydra%d_controller%d", base, controller );
}

void CServerDriver_Hydra::ThreadEntry( CServerDriver_Hydra * pDriver )
{
	pDriver->ThreadFunc();
}

void CServerDriver_Hydra::ThreadFunc()
{
	// We know the sixense SDK thread is running at "60 FPS", but we don't know when
	// those frames are.  To minimize latency, we sleep for slightly less than the
	// target rate, and detect when the frame has not advanced to wait a bit longer.
	auto longInterval = std::chrono::milliseconds( 16 );
	auto retryInterval = std::chrono::milliseconds( 2 );
	auto scanInterval = std::chrono::seconds( 1 );
	auto pollDeadline = std::chrono::steady_clock::now();
	auto scanDeadline = std::chrono::steady_clock::now() + scanInterval;

#ifdef _WIN32
	// Request at least 2ms timing granularity for the life of this process
	timeBeginPeriod( 2 );
#endif

	while ( !m_bStopRequested )
	{
		// Check for new controllers here because sixense API is modal
		// (e.g. sixenseSetActiveBase()) so it can't happen in parallel with pose updates
		if ( pollDeadline > scanDeadline )
		{
			ScanForNewControllers( true );
			scanDeadline += scanInterval;
		}

		bool bAnyActivated = false;
		bool bAllUpdated = true;
		for ( int base = 0; base < sixenseGetMaxBases(); ++base )
		{
			if ( !sixenseIsBaseConnected( base ) )
				continue;

			sixenseAllControllerData acd;

			sixenseSetActiveBase( base );
			if ( sixenseGetAllNewestData( &acd ) != SIXENSE_SUCCESS )
				continue;
			for ( int id = 0; id < sixenseGetMaxControllers(); ++id )
			{
				for ( auto it = m_vecControllers.begin(); it != m_vecControllers.end(); ++it )
				{
					CHydraHmdLatest *pHydra = *it;
					if ( pHydra->IsActivated() && pHydra->HasControllerId( base, id ) )
					{
						bAnyActivated = true;
						// Returns true if this is new data (so we can sleep for long interval)
						if ( !pHydra->Update( acd.controllers[id] ) )
						{
							bAllUpdated = false;
						}
						break;
					}
				}
			}
		}

		CheckForChordedSystemButtons();

		// If everyone just got new data, we can wait about 1/60s, else try again soon
		pollDeadline += !bAnyActivated ? scanInterval :
			               bAllUpdated ? longInterval : retryInterval;
		std::this_thread::sleep_until( pollDeadline );
	}

#ifdef _WIN32
	timeEndPeriod( 2 );
#endif
}

void CServerDriver_Hydra::ScanForNewControllers( bool bNotifyServer )
{
	for ( int base = 0; base < sixenseGetMaxBases(); ++base )
	{
		if ( sixenseIsBaseConnected( base ) )
		{
			sixenseSetActiveBase( base );
			for ( int i = 0; i < sixenseGetMaxControllers(); ++i )
			{
				if ( sixenseIsControllerEnabled( i ) )
				{
					char buf[256];
					GenerateSerialNumber( buf, sizeof( buf ), base, i );
					scope_lock lock( m_Mutex );
					if ( !FindTrackedDeviceDriver( buf, vr::ITrackedDeviceServerDriver_Version ) )
					{
						DriverLog( "added new device %s\n", buf );
						m_vecControllers.push_back( new CHydraHmdLatest( m_pDriverHost, base, i ) );
						if ( bNotifyServer && m_pDriverHost )
						{
							m_pDriverHost->TrackedDeviceAdded( m_vecControllers.back()->GetSerialNumber() );
						}
					}
				}
			}
		}
	}
}

void CServerDriver_Hydra::CheckForChordedSystemButtons()
{
	std::vector<CHydraHmdLatest *> vecHeldSystemButtons;

	for ( auto it = m_vecControllers.begin(); it != m_vecControllers.end(); ++it )
	{
		CHydraHmdLatest *pHydra = *it;

		if ( pHydra->IsHoldingSystemButton() )
		{
			vecHeldSystemButtons.push_back( pHydra );
		}
	}
	// If two or more system buttons are pressed together, treat them as a chord
	// requesting a realignment of the coordinate system
	if ( vecHeldSystemButtons.size() >= 2 )
	{
		if ( vecHeldSystemButtons.size() == 2 )
		{
			CHydraHmdLatest::RealignCoordinates( vecHeldSystemButtons[0], vecHeldSystemButtons[1] );
		}

		for ( auto it = vecHeldSystemButtons.begin(); it != vecHeldSystemButtons.end(); ++it )
		{
			( *it )->ConsumeSystemButtonPress();
		}
	}
}

// The hydra_monitor is a companion program which can display overlay prompts for us
// and tell us the pose of the HMD at the moment we want to calibrate.
void CServerDriver_Hydra::LaunchHydraMonitor( const char * pchDriverInstallDir )
{
	if ( m_bLaunchedHydraMonitor )
		return;

	m_bLaunchedHydraMonitor = true;

	std::ostringstream ss;

	ss << pchDriverInstallDir << "\\bin\\";
#if defined( _WIN64 )
	ss << "win64";
#elif defined( _WIN32 )
	ss << "win32";
#else
#error Do not know how to launch hydra_monitor
#endif
	DriverLog( "hydra_monitor path: %s\n", ss.str().c_str() );

#if defined( _WIN32 )
	STARTUPINFOA sInfoProcess = { 0 };
	sInfoProcess.cb = sizeof( STARTUPINFOW );
	PROCESS_INFORMATION pInfoStartedProcess;
	BOOL okay = CreateProcessA( (ss.str() + "\\hydra_monitor.exe").c_str(), NULL, NULL, NULL, FALSE, 0, NULL, ss.str().c_str(), &sInfoProcess, &pInfoStartedProcess );
	DriverLog( "start hydra_monitor okay: %d %08x\n", okay, GetLastError() );
#else
#error Do not know how to launch hydra_monitor
#endif
}

/** Launch hydra_monitor if needed (requested by devices as they activate) */
void CServerDriver_Hydra::LaunchHydraMonitor()
{
	LaunchHydraMonitor( m_strDriverInstallDir.c_str() );
}

//==================================================================================================
// Client Provider
//==================================================================================================

CClientDriver_Hydra::CClientDriver_Hydra()
{
}

CClientDriver_Hydra::~CClientDriver_Hydra()
{
}

vr::EVRInitError CClientDriver_Hydra::Init( vr::IDriverLog * pDriverLog, vr::IClientDriverHost * pDriverHost, const char * pchUserDriverConfigDir, const char * pchDriverInstallDir )
{
	InitDriverLog( pDriverLog );
	m_pDriverHost = pDriverHost;
	return vr::VRInitError_None;
}

void CClientDriver_Hydra::Cleanup()
{
}

bool CClientDriver_Hydra::BIsHmdPresent( const char * pchUserConfigDir )
{
	return false;
}

vr::EVRInitError CClientDriver_Hydra::SetDisplayId( const char * pchDisplayId )
{
	return vr::VRInitError_None;
	//return vr::VRInitError_Driver_HmdUnknown;
}

vr::HiddenAreaMesh_t CClientDriver_Hydra::GetHiddenAreaMesh( vr::EVREye eEye )
{
	return vr::HiddenAreaMesh_t();
}

uint32_t CClientDriver_Hydra::GetMCImage( uint32_t * pImgWidth, uint32_t * pImgHeight, uint32_t * pChannels, void * pDataBuffer, uint32_t unBufferLen )
{
	return uint32_t();
}

//==================================================================================================
// Device Driver
//==================================================================================================

const std::chrono::milliseconds CHydraHmdLatest::k_SystemButtonChordingDelay( 150 );
const std::chrono::milliseconds CHydraHmdLatest::k_SystemButtonPulsingDuration( 100 );
const float CHydraHmdLatest::k_fScaleSixenseToMeters = 0.001;  // sixense driver in mm

CHydraHmdLatest::CHydraHmdLatest( vr::IServerDriverHost * pDriverHost, int base, int n )
	: m_pDriverHost( pDriverHost )
	, m_nBase( base )
	, m_nId( n )
	, m_ucPoseSequenceNumber( 0 )
	, m_eHemisphereTrackingState( k_eHemisphereTrackingDisabled )
	, m_bCalibrated( false )
	, m_pAlignmentPartner( NULL )
	, m_eSystemButtonState( k_eIdle )
	, m_unSteamVRTrackedDeviceId( vr::k_unTrackedDeviceIndexInvalid )
{
	char buf[256];
	GenerateSerialNumber( buf, sizeof( buf ), base, n );
	m_strSerialNumber = buf;

	memset( &m_ControllerState, 0, sizeof( m_ControllerState ) );
	memset( &m_Pose, 0, sizeof( m_Pose ) );
	m_Pose.result = vr::TrackingResult_Calibrating_InProgress;

	sixenseControllerData cd;
	sixenseGetNewestData( m_nId, &cd );
	m_firmware_revision = cd.firmware_revision;
	m_hardware_revision = cd.hardware_revision;
}

CHydraHmdLatest::~CHydraHmdLatest()
{
}

void *CHydraHmdLatest::GetComponent( const char *pchComponentNameAndVersion )
{
	if ( !stricmp( pchComponentNameAndVersion, vr::IVRControllerComponent_Version ) )
	{
		return ( vr::IVRControllerComponent* )this;
	}
	
	return NULL;
}

vr::EVRInitError CHydraHmdLatest::Activate( uint32_t unObjectId )
{
	DriverLog( "CHydraHmdLatest::Activate: %s is object id %d\n", GetSerialNumber(), unObjectId );
	m_unSteamVRTrackedDeviceId = unObjectId;

	g_ServerTrackedDeviceProvider.LaunchHydraMonitor();

	return vr::VRInitError_None;
}

void CHydraHmdLatest::Deactivate()
{
	DriverLog( "CHydraHmdLatest::Deactivate: %s was object id %d\n", GetSerialNumber(), m_unSteamVRTrackedDeviceId );
	m_unSteamVRTrackedDeviceId = vr::k_unTrackedDeviceIndexInvalid;
}

void CHydraHmdLatest::DebugRequest( const char * pchRequest, char * pchResponseBuffer, uint32_t unResponseBufferSize )
{
	std::istringstream ss( pchRequest );
	std::string strCmd;

	ss >> strCmd;
	if ( strCmd == "hydra:realign_coordinates" )
	{
		// hydra_monitor is calling us back with HMD tracking information so we can
		// finish realigning our coordinate system to the HMD's
		float m[3][3], v[3];
		for ( int i = 0; i < 3; ++i )
		{
			for ( int j = 0; j < 3; ++j )
			{
				// Note the transpose, because sixenseMath::Matrix3 and vr::HmdMatrix34_t disagree on row/col major
				ss >> m[j][i];
			}
			ss >> v[i];
		}
		sixenseMath::Matrix3 matRot( m );
		sixenseMath::Vector3 matPos( v );

		FinishRealignCoordinates( matRot, matPos );
	}
}

const char * CHydraHmdLatest::GetSerialNumber()
{
	return m_strSerialNumber.c_str();
}

vr::DriverPose_t CHydraHmdLatest::GetPose()
{
	// This is only called at startup to synchronize with the driver.
	// Future updates are driven by our thread calling TrackedDevicePoseUpdated()
	return m_Pose;
}

bool CHydraHmdLatest::GetBoolTrackedDeviceProperty( vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError * pError )
{
	*pError = vr::TrackedProp_ValueNotProvidedByDevice;
	return false;
}

float CHydraHmdLatest::GetFloatTrackedDeviceProperty( vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError * pError )
{
	*pError = vr::TrackedProp_ValueNotProvidedByDevice;
	return 0.0f;
}

int32_t CHydraHmdLatest::GetInt32TrackedDeviceProperty( vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError * pError )
{
	int32_t nRetVal = 0;
	vr::ETrackedPropertyError error = vr::TrackedProp_UnknownProperty;
	switch ( prop )
	{
	case vr::Prop_DeviceClass_Int32:
		nRetVal = vr::TrackedDeviceClass_Controller;
		error = vr::TrackedProp_Success;
		break;

	case vr::Prop_Axis0Type_Int32:
		nRetVal = vr::k_eControllerAxis_Joystick;
		error = vr::TrackedProp_Success;
		break;

	case vr::Prop_Axis1Type_Int32:
		nRetVal = vr::k_eControllerAxis_Trigger;
		error = vr::TrackedProp_Success;
		break;

	case vr::Prop_Axis2Type_Int32:
	case vr::Prop_Axis3Type_Int32:
	case vr::Prop_Axis4Type_Int32:
		error = vr::TrackedProp_ValueNotProvidedByDevice;
		break;
	}

	*pError = error;
	return nRetVal;
}

uint64_t CHydraHmdLatest::GetUint64TrackedDeviceProperty( vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError * pError )
{
	uint64_t ulRetVal = 0;
	vr::ETrackedPropertyError error = vr::TrackedProp_ValueNotProvidedByDevice;

	switch ( prop )
	{
	case vr::Prop_CurrentUniverseId_Uint64:
	case vr::Prop_PreviousUniverseId_Uint64:
		error = vr::TrackedProp_ValueNotProvidedByDevice;
		break;

	case vr::Prop_SupportedButtons_Uint64:
		ulRetVal = 
			vr::ButtonMaskFromId( vr::k_EButton_System ) |
			vr::ButtonMaskFromId( vr::k_EButton_Axis0 ) |
			vr::ButtonMaskFromId( vr::k_EButton_Axis1 ) |
			vr::ButtonMaskFromId( k_EButton_Button1 ) |
			vr::ButtonMaskFromId( k_EButton_Button2 ) |
			vr::ButtonMaskFromId( k_EButton_Button3 ) |
			vr::ButtonMaskFromId( k_EButton_Button4 ) |
			vr::ButtonMaskFromId( k_EButton_Bumper );
		error = vr::TrackedProp_Success;
		break;

	case vr::Prop_HardwareRevision_Uint64:
		ulRetVal = m_hardware_revision;
		error = vr::TrackedProp_Success;
		break;

	case vr::Prop_FirmwareVersion_Uint64:
		ulRetVal = m_firmware_revision;
		error = vr::TrackedProp_Success;
		break;

	}

	*pError = error;
	return ulRetVal;
}

vr::HmdMatrix34_t CHydraHmdLatest::GetMatrix34TrackedDeviceProperty( vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError * pError )
{
	return vr::HmdMatrix34_t();
}

uint32_t CHydraHmdLatest::GetStringTrackedDeviceProperty( vr::ETrackedDeviceProperty prop, char * pchValue, uint32_t unBufferSize, vr::ETrackedPropertyError * pError )
{
	std::ostringstream ssRetVal;

	switch ( prop )
	{
	case vr::Prop_SerialNumber_String:
		ssRetVal << m_strSerialNumber;
		break;

	case vr::Prop_RenderModelName_String:
		// The {hydra} syntax lets us refer to rendermodels that are installed
		// in the driver's own resources/rendermodels directory.  The driver can
		// still refer to SteamVR models like "generic_hmd".
		ssRetVal << "{hydra}hydra_controller";
		break;

	case vr::Prop_ManufacturerName_String:
		ssRetVal << "Razer";
		break;

	case vr::Prop_ModelNumber_String:
		ssRetVal << "Hydra";
		break;

	case vr::Prop_TrackingFirmwareVersion_String:
		ssRetVal << "cd.firmware_revision=" << m_firmware_revision;
		break;

	case vr::Prop_HardwareRevision_String:
		ssRetVal << "cd.hardware_revision=" << m_hardware_revision;
		break;
	}

	std::string sRetVal = ssRetVal.str();
	if ( sRetVal.empty() )
	{
		*pError = vr::TrackedProp_ValueNotProvidedByDevice;
		return 0;
	}
	else if ( sRetVal.size() + 1 > unBufferSize )
	{
		*pError = vr::TrackedProp_BufferTooSmall;
		return sRetVal.size() + 1;  // caller needs to know how to size buffer
	}
	else
	{
		_snprintf( pchValue, unBufferSize, sRetVal.c_str() );
		*pError = vr::TrackedProp_Success;
		return sRetVal.size() + 1;
	}
}

vr::VRControllerState_t CHydraHmdLatest::GetControllerState()
{
	// This is only called at startup to synchronize with the driver.
	// Future updates are driven by our thread calling TrackedDeviceButton*() and TrackedDeviceAxis*()
	return vr::VRControllerState_t();
}

bool CHydraHmdLatest::TriggerHapticPulse( uint32_t unAxisId, uint16_t usPulseDurationMicroseconds )
{
	// this doesn't actually work on the Hydra...
	// also this would need to be interlocked with ThreadFunc because the API is modal
#if 0
	sixenseSetActiveBase( m_nBase );
	return ( sixenseTriggerVibration( m_nId, ( usPulseDurationMicroseconds + 100000 - 1 ) / 100000, 0 ) == SIXENSE_SUCCESS );
#else
	return true;  // handled -- returning false will cause errors to come out of vrserver
#endif
}

void CHydraHmdLatest::SendButtonUpdates( ButtonUpdate ButtonEvent, uint64_t ulMask )
{
	if ( !ulMask )
		return;

	for ( int i = 0; i< vr::k_EButton_Max; i++ )
	{
		vr::EVRButtonId button = ( vr::EVRButtonId )i;

		uint64_t bit = ButtonMaskFromId( button );

		if ( bit & ulMask )
		{
			( m_pDriverHost->*ButtonEvent )( m_unSteamVRTrackedDeviceId, button, 0.0 );
		}
	}
}

void CHydraHmdLatest::UpdateControllerState( sixenseControllerData & cd )
{
	vr::VRControllerState_t NewState = { 0 };

	// Changing unPacketNum tells anyone polling state that something might have
	// changed.  We don't try to be precise about that here.
	NewState.unPacketNum = m_ControllerState.unPacketNum + 1;

	if ( cd.buttons & SIXENSE_BUTTON_1 )
		NewState.ulButtonPressed |= vr::ButtonMaskFromId( k_EButton_Button1 );
	if ( cd.buttons & SIXENSE_BUTTON_2 )
		NewState.ulButtonPressed |= vr::ButtonMaskFromId( k_EButton_Button2 );
	if ( cd.buttons & SIXENSE_BUTTON_3 )
		NewState.ulButtonPressed |= vr::ButtonMaskFromId( k_EButton_Button3 );
	if ( cd.buttons & SIXENSE_BUTTON_4 )
		NewState.ulButtonPressed |= vr::ButtonMaskFromId( k_EButton_Button4 );
	if ( cd.buttons & SIXENSE_BUTTON_BUMPER )
		NewState.ulButtonPressed |= vr::ButtonMaskFromId( k_EButton_Bumper );
	if ( cd.buttons & SIXENSE_BUTTON_START )
		NewState.ulButtonPressed |= vr::ButtonMaskFromId( vr::k_EButton_System );
	if ( cd.buttons & SIXENSE_BUTTON_JOYSTICK)
		NewState.ulButtonPressed |= vr::ButtonMaskFromId( vr::k_EButton_Axis0 );
	if ( cd.trigger > 0.1f )
		NewState.ulButtonTouched |= vr::ButtonMaskFromId( vr::k_EButton_Axis1 );
	if ( cd.trigger > 0.8f )
		NewState.ulButtonPressed |= vr::ButtonMaskFromId( vr::k_EButton_Axis1 );
	// sixense driver seems to have good deadzone, but add a small one here
	if ( fabsf( cd.joystick_x ) > 0.03f || fabsf( cd.joystick_y ) > 0.03f )
		NewState.ulButtonTouched |= vr::ButtonMaskFromId( vr::k_EButton_Axis0 );

	// All pressed buttons are touched
	NewState.ulButtonTouched |= NewState.ulButtonPressed;

	uint64_t ulChangedTouched = NewState.ulButtonTouched ^ m_ControllerState.ulButtonTouched;
	uint64_t ulChangedPressed = NewState.ulButtonPressed ^ m_ControllerState.ulButtonPressed;

	SendButtonUpdates( &vr::IServerDriverHost::TrackedDeviceButtonTouched, ulChangedTouched & NewState.ulButtonTouched );
	SendButtonUpdates( &vr::IServerDriverHost::TrackedDeviceButtonPressed, ulChangedPressed & NewState.ulButtonPressed );
	SendButtonUpdates( &vr::IServerDriverHost::TrackedDeviceButtonUnpressed, ulChangedPressed & ~NewState.ulButtonPressed );
	SendButtonUpdates( &vr::IServerDriverHost::TrackedDeviceButtonUntouched, ulChangedTouched & ~NewState.ulButtonTouched );

	NewState.rAxis[0].x = cd.joystick_x;
	NewState.rAxis[0].y = cd.joystick_y;
	NewState.rAxis[1].x = cd.trigger;
	NewState.rAxis[1].y = 0.0f;

	if ( NewState.rAxis[0].x != m_ControllerState.rAxis[0].x || NewState.rAxis[0].y != m_ControllerState.rAxis[0].y )
		m_pDriverHost->TrackedDeviceAxisUpdated( m_unSteamVRTrackedDeviceId, 0, NewState.rAxis[0] );
	if ( NewState.rAxis[1].x != m_ControllerState.rAxis[1].x )
		m_pDriverHost->TrackedDeviceAxisUpdated( m_unSteamVRTrackedDeviceId, 1, NewState.rAxis[1] );

	m_ControllerState = NewState;
}

void CHydraHmdLatest::UpdateTrackingState( sixenseControllerData & cd )
{
	using namespace sixenseMath;

	// This is very hard to know with this driver, but CServerDriver_Hydra::ThreadFunc
	// tries to reduce latency as much as possible.  There is filtering in the Sixense SDK,
	// though, which causes additional unknown latency.  This time is used to know how much
	// extrapolation (via velocity and angular velocity) should be done when predicting poses.
	m_Pose.poseTimeOffset = -0.016f;

	// The "driver" coordinate system is the one that vecPosition is in.  This is whatever
	// coordinates the driver naturally produces for position and orientation.  The "world"
	// coordinate system is the one that is presented to vrserver.  This should include
	// fixing any tilt to the world (caused by a tilted camera, for example) and can include
	// any other useful transformation for the driver (e.g. the driver is tracking from a
	// secondary camera, but uses this transform to move this object into the primary camera
	// coordinate system to be consistent with other objects).
	//
	// This transform is multiplied on the left of the predicted "driver" pose.  That becomes
	// the vr::TrackingUniverseRawAndUncalibrated origin, which is then further offset for
	// floor height and tracking space center by the chaperone system to produce both the
	// vr::TrackingUniverseSeated and vr::TrackingUniverseStanding spaces.
	//
	// In the hydra driver, we use it to unify our coordinate system with the HMD.
	m_Pose.qWorldFromDriverRotation.w = m_WorldFromDriverRotation[3];
	m_Pose.qWorldFromDriverRotation.x = m_WorldFromDriverRotation[0];
	m_Pose.qWorldFromDriverRotation.y = m_WorldFromDriverRotation[1];
	m_Pose.qWorldFromDriverRotation.z = m_WorldFromDriverRotation[2];
	m_Pose.vecWorldFromDriverTranslation[0] = m_WorldFromDriverTranslation[0];
	m_Pose.vecWorldFromDriverTranslation[1] = m_WorldFromDriverTranslation[1];
	m_Pose.vecWorldFromDriverTranslation[2] = m_WorldFromDriverTranslation[2];

	// The "head" coordinate system defines a natural point for the object.  While the "driver"
	// space may be chosen for mechanical, eletrical, or mathematical convenience (e.g. being
	// the location of the IMU), the "head" should be a point meaningful to the user.  For HMDs,
	// it's the point directly between the user's eyes.  The origin of this coordinate system
	// is the origin used for the rendermodel.
	//
	// This transform is multiplied on the right side of the "driver" pose.
	//
	// This transform was inadvertently left at identity for the GDC 2015 controllers, creating
	// a defacto standard "head" position for controllers at the location of the IMU for that
	// particular controller.  We will remedy that later by adding other, explicitly named and
	// chosen spaces.  For now, mimicking that point in this driver lets us run content authored
	// for the HTC Vive Developer Edition controller.  This was done by loading an existing
	// controller rendermodel along side the Hydra model and rotating the Hydra model to roughly
	// align the main features like the handle and trigger.
	m_Pose.qDriverFromHeadRotation.w = 0.945519f;
	m_Pose.qDriverFromHeadRotation.x = 0.325568f;
	m_Pose.qDriverFromHeadRotation.y = 0.0f;
	m_Pose.qDriverFromHeadRotation.z = 0.0f;
	m_Pose.vecDriverFromHeadTranslation[0] =  0.000f;
	m_Pose.vecDriverFromHeadTranslation[1] =  0.06413f;
	m_Pose.vecDriverFromHeadTranslation[2] = -0.08695f;

	Vector3 pos = Vector3( cd.pos ) * k_fScaleSixenseToMeters;
	m_Pose.vecPosition[0] = pos[0];
	m_Pose.vecPosition[1] = pos[1];
	m_Pose.vecPosition[2] = pos[2];

	// Note that using first-order derivatives of position is a terrible
	// way to actually supply velocity to the driver.  Good prediction is
	// required to compensate for unavoidable system latencies, and having
	// a good velocity estimate is key to that.  Any serious driver should
	// be doing sensor fusion with an IMU, and should have some decent
	// notion of instantaneous velocity.
	// (Also I have no idea what shenanigans are going on in this sixense_utils
	// class, so I hope it's reasonable)
	m_Velocity.update( &cd );
	Vector3 vel = m_Velocity.getVelocity() * k_fScaleSixenseToMeters;

	// The tradeoff here is that setting a valid velocity causes the controllers
	// to jitter, but the controllers feel much more "alive" and lighter.
	// The jitter while stationary is more annoying than the laggy feeling caused
	// by disabling velocity (which effectively disables prediction for rendering).
	// Even the Hydra (without IMU) could probably produce a better velocity here
	// with a different filter on top of the raw position.  Perhaps someone feels
	// like writing one??
	vel *= 0.0f;  // XXX with no velocity, throwing might not work in some games
	m_Pose.vecVelocity[0] = vel[0];
	m_Pose.vecVelocity[1] = vel[1];
	m_Pose.vecVelocity[2] = vel[2];

	// True acceleration is highly volatile, so it's not really reasonable to
	// extrapolate much from it anyway.  Passing it as 0 from any driver should
	// be fine.
	m_Pose.vecAcceleration[0] = 0.0;
	m_Pose.vecAcceleration[1] = 0.0;
	m_Pose.vecAcceleration[2] = 0.0;

	m_Pose.qRotation.w = cd.rot_quat[3];
	m_Pose.qRotation.x = cd.rot_quat[0];
	m_Pose.qRotation.y = cd.rot_quat[1];
	m_Pose.qRotation.z = cd.rot_quat[2];

	// Unmeasured.  XXX with no angular velocity, throwing might not work in some games
	m_Pose.vecAngularVelocity[0] = 0.0;
	m_Pose.vecAngularVelocity[1] = 0.0;
	m_Pose.vecAngularVelocity[2] = 0.0;

	// The same argument applies here as to vecAcceleration, and a driver is even
	// less likely to have a valid value for it (since gyros measure angular velocity)
	m_Pose.vecAngularAcceleration[0] = 0.0;
	m_Pose.vecAngularAcceleration[1] = 0.0;
	m_Pose.vecAngularAcceleration[2] = 0.0;

	// Don't show user any controllers until they have hemisphere tracking and
	// do the calibration gesture.  hydra_monitor should be prompting with an overlay
	if ( m_eHemisphereTrackingState != k_eHemisphereTrackingEnabled )
		m_Pose.result = vr::TrackingResult_Uninitialized;
	else if ( !m_bCalibrated )
		m_Pose.result = vr::TrackingResult_Calibrating_InProgress;
	else
		m_Pose.result = vr::TrackingResult_Running_OK;

	m_Pose.poseIsValid = m_bCalibrated;
	m_Pose.deviceIsConnected = true;

	// These should always be false from any modern driver.  These are for Oculus DK1-like
	// rotation-only tracking.  Support for that has likely rotted in vrserver.
	m_Pose.willDriftInYaw = false;
	m_Pose.shouldApplyHeadModel = false;

	// This call posts this pose to shared memory, where all clients will have access to it the next
	// moment they want to predict a pose.
	m_pDriverHost->TrackedDevicePoseUpdated( m_unSteamVRTrackedDeviceId, m_Pose );
}

void CHydraHmdLatest::DelaySystemButtonForChording( sixenseControllerData & cd )
{
	// Delay sending system button to vrserver while we see if it is being
	// chorded with the other system button to reset the coordinate system
	if ( cd.buttons & SIXENSE_BUTTON_START )
	{
		switch ( m_eSystemButtonState )
		{
		case k_eIdle:
			m_eSystemButtonState = k_eWaiting;
			m_SystemButtonDelay = std::chrono::steady_clock::now() + k_SystemButtonChordingDelay;
			cd.buttons &= ~SIXENSE_BUTTON_START;
			break;

		case k_eWaiting:
			if ( std::chrono::steady_clock::now() >= m_SystemButtonDelay )
			{
				m_eSystemButtonState = k_eSent;
				// leave button state set, will reach vrserver
			}
			else
			{
				cd.buttons &= ~SIXENSE_BUTTON_START;
			}
			break;

		case k_eSent:
			// still held down, nothing to do
			break;

		case k_ePulsed:
			// user re-pressed within 1 frame, just ignore lift
			m_eSystemButtonState = k_eSent;
			break;

		case k_eBlocked:
			// was consumed by chording gesture -- never send until released
			cd.buttons &= ~SIXENSE_BUTTON_START;
			break;
		}
	}
	else
	{
		switch ( m_eSystemButtonState )
		{
		case k_eIdle:
		case k_eSent:
		case k_eBlocked:
			m_eSystemButtonState = k_eIdle;
			break;

		case k_eWaiting:
			// user pressed and released the button within the timeout, so
			// send a quick pulse to the application
			m_eSystemButtonState = k_ePulsed;
			m_SystemButtonDelay = std::chrono::steady_clock::now() + k_SystemButtonPulsingDuration;
			cd.buttons |= SIXENSE_BUTTON_START;
			break;

		case k_ePulsed:
			// stretch fake pulse so client sees it
			if ( std::chrono::steady_clock::now() >= m_SystemButtonDelay )
			{
				m_eSystemButtonState = k_eIdle;
			}
			cd.buttons |= SIXENSE_BUTTON_START;
			break;
		}
	}
}

bool CHydraHmdLatest::IsHoldingSystemButton() const
{
	return m_eSystemButtonState == k_eWaiting;
}

void CHydraHmdLatest::ConsumeSystemButtonPress()
{
	if ( m_eSystemButtonState == k_eWaiting )
	{
		m_eSystemButtonState = k_eBlocked;
	}
}

bool CHydraHmdLatest::IsActivated() const
{
	return m_unSteamVRTrackedDeviceId != vr::k_unTrackedDeviceIndexInvalid;
}

bool CHydraHmdLatest::HasControllerId( int nBase, int nId )
{
	return nBase == m_nBase && nId == m_nId;
}

// Initially block all button presses, stealing the first one to mean
// that the controller is pointing at the base and we should tell the Sixense SDK
bool CHydraHmdLatest::WaitingForHemisphereTracking( sixenseControllerData & cd )
{
	switch ( m_eHemisphereTrackingState )
	{
	case k_eHemisphereTrackingDisabled:
		if ( cd.buttons || cd.trigger > 0.8f )
		{
			// First button press
			m_eHemisphereTrackingState = k_eHemisphereTrackingButtonDown;
		}
		return true;

	case k_eHemisphereTrackingButtonDown:
		if ( !cd.buttons && cd.trigger < 0.1f )
		{
			// Buttons released (so they won't leak into application), go!
			sixenseAutoEnableHemisphereTracking( m_nId );
			m_eHemisphereTrackingState = k_eHemisphereTrackingEnabled;
		}
		return true;

	case k_eHemisphereTrackingEnabled:
	default:
		return false;
	}
}

/** Process sixenseControllerData.  Return true if it's new to help caller manage sleep durations */
bool CHydraHmdLatest::Update( sixenseControllerData & cd )
{
	if ( m_ucPoseSequenceNumber == cd.sequence_number || !IsActivated() )
		return false;
	m_ucPoseSequenceNumber = cd.sequence_number;

	UpdateTrackingState( cd );

	// Block all buttons until initial press confirms hemisphere
	if ( WaitingForHemisphereTracking( cd ) )
		return true;

	DelaySystemButtonForChording( cd );
	UpdateControllerState( cd );
	return true;
}

// User initiated manual alignment of the coordinate system of driver_hydra with the HMD:
//
// The user has put two controllers on either side of her head, near the shoulders.  We
// assume that the HMD is roughly in between them (so the exact distance apart doesn't
// matter as long as the pose is symmetrical) and we align the HMD's coordinate system
// using the line between the controllers (again, exact position is not important, only
// symmetry).
void CHydraHmdLatest::RealignCoordinates( CHydraHmdLatest * pHydraA, CHydraHmdLatest * pHydraB )
{
	if ( pHydraA->m_unSteamVRTrackedDeviceId == vr::k_unTrackedDeviceIndexInvalid )
		return;

	pHydraA->m_pAlignmentPartner = pHydraB;
	pHydraB->m_pAlignmentPartner = pHydraA;

	// Ask hydra_monitor to tell us HMD pose
	static vr::VREvent_Data_t nodata = { 0 };
	pHydraA->m_pDriverHost->VendorSpecificEvent( pHydraA->m_unSteamVRTrackedDeviceId,
		(vr::EVREventType) (vr::VREvent_VendorSpecific_Reserved_Start + 0), nodata,
		-std::chrono::duration_cast<std::chrono::seconds>( k_SystemButtonChordingDelay ).count() );
}

// hydra_monitor called us back with the HMD information
// (Note we should probably cache pose at the moment of the chording, but we just use current here)
void CHydraHmdLatest::FinishRealignCoordinates( sixenseMath::Matrix3 & matHmdRotation, sixenseMath::Vector3 & vecHmdPosition )
{
	using namespace sixenseMath;

	CHydraHmdLatest * pHydraA = this;
	CHydraHmdLatest * pHydraB = m_pAlignmentPartner;

	if ( !pHydraA || !pHydraB )
		return;

	// Assign left/right arbitrarily for a second
	Vector3 posLeft( pHydraA->m_Pose.vecPosition[0], pHydraA->m_Pose.vecPosition[1], pHydraA->m_Pose.vecPosition[2] );
	Vector3 posRight( pHydraB->m_Pose.vecPosition[0], pHydraB->m_Pose.vecPosition[1], pHydraB->m_Pose.vecPosition[2] );

	Vector3 posCenter = ( posLeft + posRight ) * 0.5f;
	Vector3 posDiff = posRight - posLeft;
	
	// Choose arbitrary controller for hint about which one is on the right:
	// Assume controllers are roughly upright, so +X vector points across body.
	Quat q1( pHydraA->m_Pose.qRotation.x, pHydraA->m_Pose.qRotation.y, pHydraA->m_Pose.qRotation.z, pHydraA->m_Pose.qRotation.w );
	Vector3 rightProbe = q1 * Vector3( 1, 0, 0 );
	if ( rightProbe * posDiff < 0 ) // * is dot product
	{
		std::swap( posLeft, posRight );
		posDiff = posDiff * -1.0f;
	}

	// Find a vector pointing forward relative to the hands, so we can rotate
	// that to match forward for the head.  Use -Y by right hand rule.
	Vector3 hydraFront = posDiff ^ Vector3( 0, -1, 0 ); // ^ is cross product
	Vector3 hmdFront = matHmdRotation * Vector3( 0, 0, -1 ); // -Z implicitly forward

	// Project both "front" vectors onto the XZ plane (we only care about yaw,
	// because we assume the HMD space is Y up, and hydra space is also Y up,
	// assuming the base is level).
	hydraFront[1] = 0.0f;
	hmdFront[1] = 0.0f;

	// Rotation is what makes the hydraFront point toward hmdFront
	Quat rotation = Quat::rotation( hydraFront, hmdFront );

	// Adjust for the natural pose of HMD vs controllers
	Vector3 vecAlignPosition = vecHmdPosition + Vector3( 0, -0.100f, -0.100f );
	Vector3 translation = vecAlignPosition - rotation * posCenter;

	// Note that it is very common for all objects from a given driver to share
	// the same world transforms, because they are in the same driver space and
	// the same world space.
	pHydraA->m_WorldFromDriverTranslation = translation;
	pHydraA->m_WorldFromDriverRotation = rotation;
	pHydraA->m_bCalibrated = true;
	pHydraB->m_WorldFromDriverTranslation = translation;
	pHydraB->m_WorldFromDriverRotation = rotation;
	pHydraB->m_bCalibrated = true;
}
