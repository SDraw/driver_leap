//========= Copyright Valve Corporation ============//
//
// driver_leap.cpp : Defines the client and server interfaces used by the SteamVR runtime.
//

#include "pch.h"
#include "driver_leap.h"

#include <sstream>

#ifdef _WIN32
#include <mmsystem.h>  // for timeBeginPeriod()
#pragma comment(lib, "winmm.lib")
#endif

#include <algorithm>

#define HMD_DLL_EXPORT extern "C" __declspec( dllexport )

CServerDriver_Leap g_ServerTrackedDeviceProvider;
CClientDriver_Leap g_ClientTrackedDeviceProvider;

// strangely, swapping the order here doesn't fix the swapped controllers in Audioshield
#define LEFT_CONTROLLER 0
#define RIGHT_CONTROLLER 1

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
// Listener interface in Server Provider
//==================================================================================================

/**
 * Called once, when this Listener object is newly added to a Controller.
 *
 * \include Listener_onInit.txt
 *
 * @param controller The Controller object invoking this callback function.
 * @since 1.0
 */
void CServerDriver_Leap::onInit(const Controller&controller)
{
    DriverLog("CServerDriver_Leap::onInit()\n");
}

/**
 * Called when the Controller object connects to the Leap Motion software and
 * the Leap Motion hardware device is plugged in,
 * or when this Listener object is added to a Controller that is already connected.
 *
 * When this callback is invoked, Controller::isServiceConnected is true,
 * Controller::devices() is not empty, and, for at least one of the Device objects in the list,
 * Device::isStreaming() is true.
 *
 * \include Listener_onConnect.txt
 *
 * @param controller The Controller object invoking this callback function.
 * @since 1.0
 */
void CServerDriver_Leap::onConnect(const Controller&controller)
{
    DriverLog("CServerDriver_Leap::onConnect()\n");
}

/**
 * Called when the Controller object disconnects from the Leap Motion software or
 * the Leap Motion hardware is unplugged.
 * The controller can disconnect when the Leap Motion device is unplugged, the
 * user shuts the Leap Motion software down, or the Leap Motion software encounters an
 * unrecoverable error.
 *
 * \include Listener_onDisconnect.txt
 *
 * Note: When you launch a Leap-enabled application in a debugger, the
 * Leap Motion library does not disconnect from the application. This is to allow
 * you to step through code without losing the connection because of time outs.
 *
 * @param controller The Controller object invoking this callback function.
 * @since 1.0
 */
void CServerDriver_Leap::onDisconnect(const Controller&controller)
{
    DriverLog("CServerDriver_Leap::onDisconnect()\n");
}

/**
 * Called when this Listener object is removed from the Controller
 * or the Controller instance is destroyed.
 *
 * \include Listener_onExit.txt
 *
 * @param controller The Controller object invoking this callback function.
 * @since 1.0
 */
void CServerDriver_Leap::onExit(const Controller&controller)
{
    DriverLog("CServerDriver_Leap::onExit()\n");
}

/**
 * Called when a new frame of hand and finger tracking data is available.
 * Access the new frame data using the Controller::frame() function.
 *
 * \include Listener_onFrame.txt
 *
 * Note, the Controller skips any pending onFrame events while your
 * onFrame handler executes. If your implementation takes too long to return,
 * one or more frames can be skipped. The Controller still inserts the skipped
 * frames into the frame history. You can access recent frames by setting
 * the history parameter when calling the Controller::frame() function.
 * You can determine if any pending onFrame events were skipped by comparing
 * the ID of the most recent frame with the ID of the last received frame.
 *
 * @param controller The Controller object invoking this callback function.
 * @since 1.0
 */
void CServerDriver_Leap::onFrame(const Controller&controller)
{
}

/**
 * Called when this application becomes the foreground application.
 *
 * Only the foreground application receives tracking data from the Leap
 * Motion Controller. This function is only called when the controller
 * object is in a connected state.
 *
 * \include Listener_onFocusGained.txt
 *
 * @param controller The Controller object invoking this callback function.
 * @since 1.0
 */
void CServerDriver_Leap::onFocusGained(const Controller&controller)
{
//    DriverLog("CServerDriver_Leap::onFocusGained()\n");
}

/**
 * Called when this application loses the foreground focus.
 *
 * Only the foreground application receives tracking data from the Leap
 * Motion Controller. This function is only called when the controller
 * object is in a connected state.
 *
 * \include Listener_onFocusLost.txt
 *
 * @param controller The Controller object invoking this callback function.
 * @since 1.0
 */
void CServerDriver_Leap::onFocusLost(const Controller&controller)
{
//    DriverLog("CServerDriver_Leap::onFocusLost()\n");
}

// onServiceConnect/onServiceDisconnect are for connection established/lost.
// in normal course of events onServiceConnect will get called once after onInit
// and onServiceDisconnect will not get called. disconnect notification only happens
// if service stops running or something else bad happens to disconnect controller from service.
/**
 * Called when the Leap Motion daemon/service connects to your application Controller.
 *
 * \include Listener_onServiceConnect.txt
 *
 * @param controller The Controller object invoking this callback function.
 * @since 1.2
 */
void CServerDriver_Leap::onServiceConnect(const Controller&controller)
{
    DriverLog("CServerDriver_Leap::onServiceConnect()\n");
}

/**
 * Called if the Leap Motion daemon/service disconnects from your application Controller.
 *
 * Normally, this callback is not invoked. It is only called if some external event
 * or problem shuts down the service or otherwise interrupts the connection.
 *
 * \include Listener_onServiceDisconnect.txt
 *
 * @param controller The Controller object invoking this callback function.
 * @since 1.2
 */
void CServerDriver_Leap::onServiceDisconnect(const Controller&controller)
{
    DriverLog("CServerDriver_Leap::onServiceDisconnect()\n");
}

/**
 * Called when a Leap Motion controller is plugged in, unplugged, or the device changes state.
 *
 * State changes include entering or leaving robust mode and low resource mode.
 * Note that there is no direct way to query whether the device is in these modes,
 * although you can use Controller::isLightingBad() to check if there are environmental
 * IR lighting problems.
 *
 * \include Listener_onDeviceChange.txt
 *
 * @param controller The Controller object invoking this callback function.
 * @since 1.2
 */
void CServerDriver_Leap::onDeviceChange(const Controller&controller)
{
    DriverLog("CServerDriver_Leap::onDeviceChange()\n");

    if (controller.isConnected())
    {
        bool backgroundModeAllowed = controller.config().getInt32("background_app_mode") == 2;
        if (!backgroundModeAllowed) {
            // TODO: Show dialog to request permission to allow background mode apps
            bool userPermission = true;
            if (userPermission) {
                controller.config().setInt32("background_app_mode", 2);
                controller.config().save();
            }
        }

        controller.setPolicy(Leap::Controller::POLICY_OPTIMIZE_HMD);
        controller.setPolicy(Leap::Controller::POLICY_BACKGROUND_FRAMES);

        // make sure we always get background frames even when we lose the focus to another
        // Leap-enabled application
        controller.setPolicy((Leap::Controller::PolicyFlag)(15));

        // allow other background applications to receive frames even when SteamVR has the focus.
        controller.setPolicy((Leap::Controller::PolicyFlag)(23));

        ScanForNewControllers(true);
    }
    else
    {
        for (auto it = m_vecControllers.begin(); it != m_vecControllers.end(); ++it)
            delete (*it);
        m_vecControllers.clear();
    }
}

/**
 * Called when new images are available.
 * Access the new frame data using the Controller::images() function.
 *
 * \include Listener_onImages.txt
 *
 * @param controller The Controller object invoking this callback function.
 * @since 2.2.1
 */
void CServerDriver_Leap::onImages(const Controller&controller)
{
}

/**
 * Called when the Leap Motion service is paused or resumed or when a
 * controller policy is changed.
 *
 * The service can change states because the computer user changes settings
 * in the Leap Motion Control Panel application or because an application
 * connected to the service triggers a change. Any application can pause or
 * unpause the service, but only runtime policy changes you make apply to your
 * own application.
 *
 * \include Listener_onServiceChange.txt
 *
 * You can query the pause state of the controller with Controller::isPaused().
 * You can check the state of those policies you are interested in with
 * Controller::isPolicySet().
 *
 * @param controller The Controller object invoking this callback function.
 * @since 3.0
 */
void CServerDriver_Leap::onServiceChange(const Controller&controller)
{
    DriverLog("CServerDriver_Leap::onServiceChange()\n");
}

/**
 * Called when a Leap Motion controller device is plugged into the client
 * computer, but fails to operate properly.
 *
 * Get the list containing all failed devices using Controller::failedDevices().
 * The members of this list provide the device pnpID and reason for failure.
 *
 * \include Listener_onDeviceFailure.txt
 *
 * @param controller The Controller object invoking this callback function.
 * @since 3.0
 */
void CServerDriver_Leap::onDeviceFailure(const Controller&controller)
{
    DriverLog("CServerDriver_Leap::onDeviceFailure()\n");
}

/**
 * Called when the service emits a log message to report an error, warning, or
 * status change.
 *
 * Log message text is provided as ASCII-encoded english.
 *
 * @param controller The Controller object invoking this callback function.
 * @param severity The severity of the error, if known.
 * @param timestamp The timestamp of the error in microseconds.
 * (Use Controller::now() - timestamp to compute the age of the message.)
 * @param msg The log message.
 * @since 3.0
 */
void CServerDriver_Leap::onLogMessage(const Controller&controller, MessageSeverity severity, int64_t timestamp, const char* msg)
{
    DriverLog("CServerDriver_Leap::onLogMessage(%d): %s\n", (int)severity, msg);
}


//==================================================================================================
// Server Provider
//==================================================================================================

CServerDriver_Leap::CServerDriver_Leap()
    : m_bLaunchedLeapMonitor( false )
{
//    DriverLog not yet initialized at this point.
//    DriverLog("CServerDriver_Leap::CServerDriver_Leap()\n");
}

CServerDriver_Leap::~CServerDriver_Leap()
{
    DriverLog("CServerDriver_Leap::~CServerDriver_Leap()\n");
    Cleanup();
}

vr::EVRInitError CServerDriver_Leap::Init( vr::IDriverLog * pDriverLog, vr::IServerDriverHost * pDriverHost, const char * pchUserDriverConfigDir, const char * pchDriverInstallDir )
{
    InitDriverLog( pDriverLog );
    DriverLog("CServerDriver_Leap::Init()\n");

    m_pDriverHost = pDriverHost;
    m_strDriverInstallDir = pchDriverInstallDir;

    m_Controller = new Controller;

    Controller &controller = *m_Controller;

    m_Controller->addListener(*this);

    return vr::VRInitError_None;
}

void CServerDriver_Leap::Cleanup()
{
    DriverLog("CServerDriver_Leap::Cleanup()\n");

    // send a termination message to the leap monitor companion application
    if (m_bLaunchedLeapMonitor)
    {
        // Ask leap_monitor to shut down.
        PostThreadMessage(m_pInfoStartedProcess.dwThreadId, WM_QUIT, 0, 0);
        m_bLaunchedLeapMonitor = false;
    }

    // clean up our Leap::Controller object
    if (m_Controller)
    {
        m_Controller->removeListener(*this);
        delete m_Controller;
        m_Controller = NULL;
    }

    // clean up any controller objects we've created
    for (auto it = m_vecControllers.begin(); it != m_vecControllers.end(); ++it)
        delete (*it);
    m_vecControllers.clear();
}

uint32_t CServerDriver_Leap::GetTrackedDeviceCount()
{
    return m_vecControllers.size();
}

vr::ITrackedDeviceServerDriver * CServerDriver_Leap::GetTrackedDeviceDriver( uint32_t unWhich, const char *pchInterfaceVersion )
{
    // don't return anything if that's not the interface version we have
    if ( 0 != stricmp( pchInterfaceVersion, vr::ITrackedDeviceServerDriver_Version ) )
    {
        DriverLog( "FindTrackedDeviceDriver for version %s, which we don't support.\n",
            pchInterfaceVersion );
        return NULL;
    }

    if ( unWhich < m_vecControllers.size() )
        return m_vecControllers[unWhich];

    return nullptr;
}

vr::ITrackedDeviceServerDriver * CServerDriver_Leap::FindTrackedDeviceDriver( const char * pchId, const char *pchInterfaceVersion )
{
    // don't return anything if that's not the interface version we have
    if ( 0 != stricmp( pchInterfaceVersion, vr::ITrackedDeviceServerDriver_Version ) )
    {
        DriverLog( "FindTrackedDeviceDriver for version %s, which we don't support.\n",
            pchInterfaceVersion );
        return NULL;
    }

    for ( auto it = m_vecControllers.begin(); it != m_vecControllers.end(); ++it )
    {
        if ( 0 == strcmp( ( *it )->GetSerialNumber(), pchId ) )
        {
            return *it;
        }
    }
    return nullptr;
}

void CServerDriver_Leap::RunFrame()
{
    if (m_vecControllers.size() == 2)
    {
        m_vecControllers[0]->RealignCoordinates(m_vecControllers[0], m_vecControllers[1]);
    }

    if (m_Controller)
    {
        if (m_Controller->isConnected())
        {
            Frame frame = m_Controller->frame();

            // update the controllers
            for (auto it = m_vecControllers.begin(); it != m_vecControllers.end(); ++it)
            {
                CLeapHmdLatest *pLeap = *it;
                if (pLeap->IsActivated())
                {
                    // Returns true if this is new data (so we can sleep for long interval)
                    if (!pLeap->Update(frame))
                    {
                        // not updated?
                    }
                }
            }
        }
    }
}

bool CServerDriver_Leap::ShouldBlockStandbyMode()
{
    return false;
}

void CServerDriver_Leap::EnterStandby()
{
    DriverLog("CServerDriver_Leap::EnterStandby()\n");
}

void CServerDriver_Leap::LeaveStandby()
{
    DriverLog("CServerDriver_Leap::LeaveStandby()\n");
}

static void GenerateSerialNumber( char *p, int psize, int base, int controller )
{
    char tmp[32];
    _snprintf(tmp, 32, "controller%d", controller);
    _snprintf( p, psize, "leap%d_%s", base, (controller == LEFT_CONTROLLER) ? "lefthand" : (controller == RIGHT_CONTROLLER) ? "righthand" : tmp );
}

void CServerDriver_Leap::ScanForNewControllers( bool bNotifyServer )
{
    while (m_vecControllers.size() < 2)
    {
        char buf[256];
        int base = 0;
        int i = m_vecControllers.size();
        GenerateSerialNumber( buf, sizeof( buf ), base, i );
        if ( !FindTrackedDeviceDriver( buf, vr::ITrackedDeviceServerDriver_Version ) )
        {
            DriverLog( "added new device %s\n", buf );
            m_vecControllers.push_back( new CLeapHmdLatest( m_pDriverHost, base, i ) );
            if ( bNotifyServer && m_pDriverHost )
            {
                m_pDriverHost->TrackedDeviceAdded( m_vecControllers.back()->GetSerialNumber() );
            }
        }
    }
}

// The leap_monitor is a companion program which will tell us the pose of the HMD.
void CServerDriver_Leap::LaunchLeapMonitor( const char * pchDriverInstallDir )
{
    if ( m_bLaunchedLeapMonitor )
        return;

    DriverLog("CServerDriver_Leap::LaunchLeapMonitor()\n");

    m_bLaunchedLeapMonitor = true;

    std::ostringstream ss;

    ss << pchDriverInstallDir << "\\bin\\";
#if defined( _WIN64 )
    ss << "win64";
#elif defined( _WIN32 )
    ss << "win32";
#else
#error Do not know how to launch leap_monitor
#endif
    DriverLog( "leap_monitor path: %s\n", ss.str().c_str() );

#if defined( _WIN32 )
    STARTUPINFOA sInfoProcess = { 0 };
    sInfoProcess.cb = sizeof(STARTUPINFOW);
//    sInfoProcess.dwFlags = STARTF_USESHOWWINDOW;
//    sInfoProcess.wShowWindow = SW_SHOWDEFAULT;
    BOOL okay = CreateProcessA( (ss.str() + "\\leap_monitor.exe").c_str(), NULL, NULL, NULL, FALSE, 0, NULL, ss.str().c_str(), &sInfoProcess, &m_pInfoStartedProcess );
    DriverLog( "start leap_monitor okay: %d %08x\n", okay, GetLastError() );
#else
#error Do not know how to launch leap_monitor
#endif
}

/** Launch leap_monitor if needed (requested by devices as they activate) */
void CServerDriver_Leap::LaunchLeapMonitor()
{
    LaunchLeapMonitor( m_strDriverInstallDir.c_str() );
}

//==================================================================================================
// Client Provider
//==================================================================================================

CClientDriver_Leap::CClientDriver_Leap()
{
}

CClientDriver_Leap::~CClientDriver_Leap()
{
}

vr::EVRInitError CClientDriver_Leap::Init( vr::IDriverLog * pDriverLog, vr::IClientDriverHost * pDriverHost, const char * pchUserDriverConfigDir, const char * pchDriverInstallDir )
{
    InitDriverLog( pDriverLog );
    DriverLog("CClientDriver_Leap::Init()\n");
    m_pDriverHost = pDriverHost;
    return vr::VRInitError_None;
}

void CClientDriver_Leap::Cleanup()
{
    DriverLog("CClientDriver_Leap::Cleanup()\n");
}

bool CClientDriver_Leap::BIsHmdPresent( const char * pchUserConfigDir )
{
    return false;
}

vr::EVRInitError CClientDriver_Leap::SetDisplayId( const char * pchDisplayId )
{
    return vr::VRInitError_None;
    //return vr::VRInitError_Driver_HmdUnknown;
}

vr::HiddenAreaMesh_t CClientDriver_Leap::GetHiddenAreaMesh( vr::EVREye eEye )
{
    return vr::HiddenAreaMesh_t();
}

uint32_t CClientDriver_Leap::GetMCImage( uint32_t * pImgWidth, uint32_t * pImgHeight, uint32_t * pChannels, void * pDataBuffer, uint32_t unBufferLen )
{
    return uint32_t();
}

//==================================================================================================
// Device Driver
//==================================================================================================

const std::chrono::milliseconds CLeapHmdLatest::k_TrackingLatency( -30 );

CLeapHmdLatest::CLeapHmdLatest( vr::IServerDriverHost * pDriverHost, int base, int n )
    : m_pDriverHost( pDriverHost )
    , m_nBase( base )
    , m_nId( n )
    , m_bCalibrated( true )
    , m_pAlignmentPartner( NULL )
    , m_unSteamVRTrackedDeviceId( vr::k_unTrackedDeviceIndexInvalid )
{
    DriverLog("CLeapHmdLatest::CLeapHmdLatest(base=%d, n=%d)\n", base, n);
    
    memset(m_hmdPos, 0, sizeof(m_hmdPos));

    char buf[256];
    GenerateSerialNumber( buf, sizeof( buf ), base, n );
    m_strSerialNumber = buf;

    memset( &m_ControllerState, 0, sizeof( m_ControllerState ) );
    memset( &m_Pose, 0, sizeof( m_Pose ) );
    m_Pose.result = vr::TrackingResult_Uninitialized;

    m_firmware_revision = 0x0001;
    m_hardware_revision = 0x0001;

    // Load config from steamvr.vrsettings
    vr::IVRSettings *settings_;
    settings_ = m_pDriverHost->GetSettings(vr::IVRSettings_Version);

    // Load rendermodel
    char tmp_[256];
    settings_->GetString("leap", (m_nId ==  LEFT_CONTROLLER) ? "renderModel_lefthand" : (m_nId == RIGHT_CONTROLLER) ? "renderModel_righthand" : "renderModel", tmp_, sizeof(tmp_), "vr_controller_vive_1_5");
    m_strRenderModel = tmp_;

    // set the 
    m_gripAngleOffset = settings_->GetFloat("leap", (m_nId == LEFT_CONTROLLER) ? "gripAngleOffset_lefthand" : (m_nId == RIGHT_CONTROLLER) ? "gripAngleOffset_righthand" : "gripAngleOffset", 0.0);
}

CLeapHmdLatest::~CLeapHmdLatest()
{
    DriverLog("CLeapHmdLatest::~CLeapHmdLatest(base=%d, n=%d)\n", m_nBase, m_nId);
}

void *CLeapHmdLatest::GetComponent( const char *pchComponentNameAndVersion )
{
    if ( !stricmp( pchComponentNameAndVersion, vr::IVRControllerComponent_Version ) )
    {
        return ( vr::IVRControllerComponent* )this;
    }
    
    return NULL;
}

vr::EVRInitError CLeapHmdLatest::Activate( uint32_t unObjectId )
{
    DriverLog( "CLeapHmdLatest::Activate: %s is object id %d\n", GetSerialNumber(), unObjectId );
    m_unSteamVRTrackedDeviceId = unObjectId;

    g_ServerTrackedDeviceProvider.LaunchLeapMonitor();

    return vr::VRInitError_None;
}

void CLeapHmdLatest::Deactivate()
{
    DriverLog( "CLeapHmdLatest::Deactivate: %s was object id %d\n", GetSerialNumber(), m_unSteamVRTrackedDeviceId );
    m_unSteamVRTrackedDeviceId = vr::k_unTrackedDeviceIndexInvalid;
}

void CLeapHmdLatest::PowerOff()
{
    DriverLog("CLeapHmdLatest::PowerOff()\n");
    // FIXME Implement
}

void CLeapHmdLatest::DebugRequest( const char * pchRequest, char * pchResponseBuffer, uint32_t unResponseBufferSize )
{
    std::istringstream ss( pchRequest );
    std::string strCmd;

    ss >> strCmd;
    if ( strCmd == "leap:realign_coordinates" )
    {
        // leap_monitor is calling us back with HMD tracking information so we can
        // finish realigning our coordinate system to the HMD's
        float m[3][3], v[3];
        for ( int i = 0; i < 3; ++i )
        {
            for ( int j = 0; j < 3; ++j )
            {
                ss >> m[j][i];
            }
            ss >> v[i];
        }
        FinishRealignCoordinates(m, v);
    }
}

const char * CLeapHmdLatest::GetSerialNumber()
{
    return m_strSerialNumber.c_str();
}

vr::DriverPose_t CLeapHmdLatest::GetPose()
{
    // This is only called at startup to synchronize with the driver.
    // Future updates are driven by our thread calling TrackedDevicePoseUpdated()
    return m_Pose;
}

bool CLeapHmdLatest::GetBoolTrackedDeviceProperty( vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError * pError )
{
    *pError = vr::TrackedProp_ValueNotProvidedByDevice;
    return false;
}

float CLeapHmdLatest::GetFloatTrackedDeviceProperty( vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError * pError )
{
    *pError = vr::TrackedProp_ValueNotProvidedByDevice;
    return 0.0f;
}

int32_t CLeapHmdLatest::GetInt32TrackedDeviceProperty( vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError * pError )
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

uint64_t CLeapHmdLatest::GetUint64TrackedDeviceProperty( vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError * pError )
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
            vr::ButtonMaskFromId( vr::k_EButton_ApplicationMenu) |
            vr::ButtonMaskFromId( vr::k_EButton_System ) |
            vr::ButtonMaskFromId( vr::k_EButton_SteamVR_Touchpad ) |
            vr::ButtonMaskFromId( vr::k_EButton_SteamVR_Trigger) |
            vr::ButtonMaskFromId( vr::k_EButton_Grip );
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

vr::HmdMatrix34_t CLeapHmdLatest::GetMatrix34TrackedDeviceProperty( vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError * pError )
{
    return vr::HmdMatrix34_t();
}

uint32_t CLeapHmdLatest::GetStringTrackedDeviceProperty( vr::ETrackedDeviceProperty prop, char * pchValue, uint32_t unBufferSize, vr::ETrackedPropertyError * pError )
{
    std::ostringstream ssRetVal;

    switch ( prop )
    {
    case vr::Prop_SerialNumber_String:
        ssRetVal << m_strSerialNumber;
        break;

    case vr::Prop_RenderModelName_String:
        // We return the user configured rendermodel here. Defaults to "vr_controller_vive_1_5".
        ssRetVal << m_strRenderModel.c_str();
        break;

    case vr::Prop_ManufacturerName_String:
        ssRetVal << "LeapMotion";
        break;

    case vr::Prop_ModelNumber_String:
        ssRetVal << "Controller";
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

vr::VRControllerState_t CLeapHmdLatest::GetControllerState()
{
    // This is only called at startup to synchronize with the driver.
    // Future updates are driven by our thread calling TrackedDeviceButton*() and TrackedDeviceAxis*()
    return vr::VRControllerState_t();
}

bool CLeapHmdLatest::TriggerHapticPulse( uint32_t unAxisId, uint16_t usPulseDurationMicroseconds )
{
    return true;  // handled -- returning false will cause errors to come out of vrserver
}

void CLeapHmdLatest::SendButtonUpdates( ButtonUpdate ButtonEvent, uint64_t ulMask )
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

void CLeapHmdLatest::UpdateControllerState(Frame &frame)
{
    vr::VRControllerState_t NewState = { 0 };

    bool handFound = false;
    GestureMatcher::WhichHand which = (m_nId == LEFT_CONTROLLER ) ? GestureMatcher::LeftHand :
                                      (m_nId == RIGHT_CONTROLLER) ? GestureMatcher::RightHand :
                                      GestureMatcher::AnyHand;

    float scores[GestureMatcher::NUM_GESTURES];
    handFound = matcher.MatchGestures(frame, which, scores);

    if (handFound)
    {
        // Changing unPacketNum tells anyone polling state that something might have
        // changed.  We don't try to be precise about that here.
        NewState.unPacketNum = m_ControllerState.unPacketNum + 1;

        // system menu mapping (timeout gesture)
        if (scores[GestureMatcher::Timeout] >= 0.5f)
            NewState.ulButtonTouched |= vr::ButtonMaskFromId(vr::k_EButton_System);
        if (scores[GestureMatcher::Timeout] >= 0.5f)
            NewState.ulButtonPressed |= vr::ButtonMaskFromId(vr::k_EButton_System);

        // application menu mapping (Flat hand towards your face gesture)
        if (scores[GestureMatcher::FlatHandPalmTowards] >= 0.8f)
            NewState.ulButtonTouched |= vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu);
        if (scores[GestureMatcher::FlatHandPalmTowards] >= 0.8f)
            NewState.ulButtonPressed |= vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu);

        // digital trigger mapping (fist clenching gesture)
        if (scores[GestureMatcher::TriggerFinger] > 0.5f)
            NewState.ulButtonTouched |= vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger);
        if (scores[GestureMatcher::TriggerFinger] > 0.5f)
            NewState.ulButtonPressed |= vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger);

        // grip mapping (clench fist with middle, index, pinky fingers)
        if (scores[GestureMatcher::LowerFist] >= 0.5f)
            NewState.ulButtonTouched |= vr::ButtonMaskFromId(vr::k_EButton_Grip);
        if (scores[GestureMatcher::LowerFist] >= 0.5f)
            NewState.ulButtonPressed |= vr::ButtonMaskFromId(vr::k_EButton_Grip);

        // touchpad button press mapping (Thumbpress gesture)
        if (scores[GestureMatcher::Thumbpress] >= 0.2f)
            NewState.ulButtonTouched |= vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad);
        if (scores[GestureMatcher::Thumbpress] >= 1.0f)
            NewState.ulButtonPressed |= vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad);

#if 0
        // sixense driver seems to have good deadzone, but add a small one here
        if (fabsf(cd.joystick_x) > 0.03f || fabsf(cd.joystick_y) > 0.03f)
            NewState.ulButtonTouched |= vr::ButtonMaskFromId(vr::k_EButton_StreamVR_Touchpad);
#endif

        // All pressed buttons are touched
        NewState.ulButtonTouched |= NewState.ulButtonPressed;

        uint64_t ulChangedTouched = NewState.ulButtonTouched ^ m_ControllerState.ulButtonTouched;
        uint64_t ulChangedPressed = NewState.ulButtonPressed ^ m_ControllerState.ulButtonPressed;

        SendButtonUpdates(&vr::IServerDriverHost::TrackedDeviceButtonTouched, ulChangedTouched & NewState.ulButtonTouched);
        SendButtonUpdates(&vr::IServerDriverHost::TrackedDeviceButtonPressed, ulChangedPressed & NewState.ulButtonPressed);
        SendButtonUpdates(&vr::IServerDriverHost::TrackedDeviceButtonUnpressed, ulChangedPressed & ~NewState.ulButtonPressed);
        SendButtonUpdates(&vr::IServerDriverHost::TrackedDeviceButtonUntouched, ulChangedTouched & ~NewState.ulButtonTouched);

        NewState.rAxis[0].x = scores[GestureMatcher::TouchpadAxisX];
        NewState.rAxis[0].y = scores[GestureMatcher::TouchpadAxisY];

        NewState.rAxis[1].x = scores[GestureMatcher::TriggerFinger];
        NewState.rAxis[1].y = 0.0f;

        // the touchpad maps to Axis 0 X/Y
        if (NewState.rAxis[0].x != m_ControllerState.rAxis[0].x || NewState.rAxis[0].y != m_ControllerState.rAxis[0].y)
            m_pDriverHost->TrackedDeviceAxisUpdated(m_unSteamVRTrackedDeviceId, 0, NewState.rAxis[0]);

        // trigger maps to Axis 1 X
        if (NewState.rAxis[1].x != m_ControllerState.rAxis[1].x)
            m_pDriverHost->TrackedDeviceAxisUpdated(m_unSteamVRTrackedDeviceId, 1, NewState.rAxis[1]);

        m_ControllerState = NewState;
    }
}

// multiplication of quaternions
vr::HmdQuaternion_t operator*(const vr::HmdQuaternion_t& a, const vr::HmdQuaternion_t& b)
{
    vr::HmdQuaternion_t tmp;

    tmp.w = (b.w * a.w) - (b.x * a.x) - (b.y * a.y) - (b.z * a.z);
    tmp.x = (b.w * a.x) + (b.x * a.w) + (b.y * a.z) - (b.z * a.y);
    tmp.y = (b.w * a.y) + (b.y * a.w) + (b.z * a.x) - (b.x * a.z);
    tmp.z = (b.w * a.z) + (b.z * a.w) + (b.x * a.y) - (b.y * a.x);

    return tmp;
}

// generate a rotation quaternion around an arbitrary axis
vr::HmdQuaternion_t rotate_around_axis(const Vector &v, const float &a)
{
    // Here we calculate the sin( a / 2) once for optimization
    float factor = sinf(a* 0.01745329 / 2.0);

    // Calculate the x, y and z of the quaternion
    float x = v.x * factor;
    float y = v.y * factor;
    float z = v.z * factor;

    // Calcualte the w value by cos( a / 2 )
    float w = cosf(a* 0.01745329 / 2.0);

    float mag = sqrtf(w*w + x*x + y*y + z*z);

    vr::HmdQuaternion_t result = { w / mag, x / mag, y / mag, z / mag };
    return result;
}

// convert a 3x3 rotation matrix into a rotation quaternion
static vr::HmdQuaternion_t CalculateRotation(float(*a)[3]) {

    vr::HmdQuaternion_t q;

    float trace = a[0][0] + a[1][1] + a[2][2];
    if (trace > 0) {
        float s = 0.5f / sqrtf(trace + 1.0f);
        q.w = 0.25f / s;
        q.x = (a[2][1] - a[1][2]) * s;
        q.y = (a[0][2] - a[2][0]) * s;
        q.z = (a[1][0] - a[0][1]) * s;
    }
    else {
        if (a[0][0] > a[1][1] && a[0][0] > a[2][2]) {
            float s = 2.0f * sqrtf(1.0f + a[0][0] - a[1][1] - a[2][2]);
            q.w = (a[2][1] - a[1][2]) / s;
            q.x = 0.25f * s;
            q.y = (a[0][1] + a[1][0]) / s;
            q.z = (a[0][2] + a[2][0]) / s;
        }
        else if (a[1][1] > a[2][2]) {
            float s = 2.0f * sqrtf(1.0f + a[1][1] - a[0][0] - a[2][2]);
            q.w = (a[0][2] - a[2][0]) / s;
            q.x = (a[0][1] + a[1][0]) / s;
            q.y = 0.25f * s;
            q.z = (a[1][2] + a[2][1]) / s;
        }
        else {
            float s = 2.0f * sqrtf(1.0f + a[2][2] - a[0][0] - a[1][1]);
            q.w = (a[1][0] - a[0][1]) / s;
            q.x = (a[0][2] + a[2][0]) / s;
            q.y = (a[1][2] + a[2][1]) / s;
            q.z = 0.25f * s;
        }
    }
    q.x = -q.x;
    q.y = -q.y;
    q.z = -q.z;
    return q;
}

void CLeapHmdLatest::UpdateTrackingState(Frame &frame)
{
    HandList &hands = frame.hands();

    bool handFound = false;
    for (int h = 0; h < hands.count(); h++)
    {
        Hand &hand = hands[h];

        // controller #0 is supposed to be the left hand, controller #1 the right one.
        if (hand.isValid() && (m_nId == LEFT_CONTROLLER && hand.isLeft() ||
                               m_nId == RIGHT_CONTROLLER && hand.isRight()))
        {
            handFound = true;

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
            // In the leap driver, we use it to unify our coordinate system with the HMD.
            m_Pose.qWorldFromDriverRotation = m_hmdRot;
            m_Pose.vecWorldFromDriverTranslation[0] = m_hmdPos[0];
            m_Pose.vecWorldFromDriverTranslation[1] = m_hmdPos[1];
            m_Pose.vecWorldFromDriverTranslation[2] = m_hmdPos[2];

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
            // controller rendermodel along side the Leap model and rotating the Leap model to roughly
            // align the main features like the handle and trigger.
            m_Pose.qDriverFromHeadRotation.w = 1;
            m_Pose.qDriverFromHeadRotation.x = 0; //  -m_hmdRot.x;   this would cancel out the HMD's rotation
            m_Pose.qDriverFromHeadRotation.y = 0; //  -m_hmdRot.y;   but instead we rely on the Leap Motion to
            m_Pose.qDriverFromHeadRotation.z = 0; //  -m_hmdRot.z;   update the hand rotation as the head rotates
            m_Pose.vecDriverFromHeadTranslation[0] = 0;
            m_Pose.vecDriverFromHeadTranslation[1] = 0;
            m_Pose.vecDriverFromHeadTranslation[2] = 0;

            Vector position = hand.palmPosition();

            m_Pose.vecPosition[0] = -0.001*position.x;
            m_Pose.vecPosition[1] = -0.001*position.z;
            m_Pose.vecPosition[2] = -0.001*position.y - 0.15; // assume 15 cm offset from midpoint between eys

            Vector velocity = hand.palmVelocity();

            m_Pose.vecVelocity[0] = -0.001*velocity.x;
            m_Pose.vecVelocity[1] = -0.001*velocity.z;
            m_Pose.vecVelocity[2] = -0.001*velocity.y;

            // Unmeasured.  XXX we currently leave the acceleration at zero
            m_Pose.vecAcceleration[0] = 0.0;
            m_Pose.vecAcceleration[1] = 0.0;
            m_Pose.vecAcceleration[2] = 0.0;

            // get two vectors describing the hand's orientation in space. We need to find a rotation
            // matrix that turns the default coordinate system into the hand's coordinate system
            Vector direction = hand.direction(); direction /= direction.magnitude();
            Vector normal = hand.palmNormal(); normal /= normal.magnitude();
            Vector side = direction.cross(normal);

#if 0
            // This code assumes palms are facing downwards.

            // NOTE: y and z are swapped with respect to the Leap Motion's coordinate system and I list
            //       the vectors in the order in which I expect them to be in the tracking camera's
            //       coordinates system: X = sideways,
            //                           Y = up/down i.e. palm's normal vector
            //                           Z = front/back i.e. hand's pointing direction
            m_Pose.qRotation = CalculateRotation(R);

            float R[3][3] =
            { { side.x,      side.z,      side.y },
            { normal.x,    normal.z,    normal.y },
            { direction.x, direction.z, direction.y } };

#else
            // This code assumes palms are facing inwards as if you were holding controllers.
            // This is why the left hand and the
            // right hands have to use different matrices to compute their rotations.

            float L[3][3] =
            { {-normal.x,  -normal.z,   -normal.y },
            {   side.x,      side.z,      side.y },
            {   direction.x, direction.z, direction.y } };

            float R[3][3] =
            { { normal.x,    normal.z,    normal.y },
              {-side.x,     -side.z,     -side.y      },
              { direction.x, direction.z, direction.y } };

            // now turn this into a Quaternion and we're done.
            if (m_nId == LEFT_CONTROLLER)
                m_Pose.qRotation = CalculateRotation(L);
            else if (m_nId == RIGHT_CONTROLLER)
                m_Pose.qRotation = CalculateRotation(R);

#endif
            // rotate by the specified grip angle (may be useful when using the Vive as a gun grip)
            if (m_gripAngleOffset != 0)
                m_Pose.qRotation = rotate_around_axis(Vector(1.0, 0.0, 0.0), m_gripAngleOffset) * m_Pose.qRotation;

            // Unmeasured.  XXX with no angular velocity, throwing might not work in some games
            m_Pose.vecAngularVelocity[0] = 0.0;
            m_Pose.vecAngularVelocity[1] = 0.0;
            m_Pose.vecAngularVelocity[2] = 0.0;

            // The same argument applies here as to vecAcceleration, and a driver is even
            // less likely to have a valid value for it (since gyros measure angular velocity)
            m_Pose.vecAngularAcceleration[0] = 0.0;
            m_Pose.vecAngularAcceleration[1] = 0.0;
            m_Pose.vecAngularAcceleration[2] = 0.0;

            // this results in the controllers being shown on screen
            m_Pose.result = vr::TrackingResult_Running_OK;

            // the pose validity also depends on HMD tracking data sent to us by the leap_monitor.exe
            m_Pose.poseIsValid = m_bCalibrated;
        }
    }

    if (!handFound)
    {
        m_Pose.result = vr::TrackingResult_Running_OutOfRange;
        m_Pose.poseIsValid = false;
    }

    // This is very hard to know with this driver, but CServerDriver_Leap::ThreadFunc
    // tries to reduce latency as much as possible.  There is processing in the Leap Motion SDK,
    // though, which causes additional unknown latency.  This time is used to know how much
    // extrapolation (via velocity and angular velocity) should be done when predicting poses.
    m_Pose.poseTimeOffset = -0.016f;

    // when we get here, the Leap Motion is connected
    m_Pose.deviceIsConnected = true;

    // These should always be false from any modern driver.  These are for Oculus DK1-like
    // rotation-only tracking.  Support for that has likely rotted in vrserver.
    m_Pose.willDriftInYaw = false;
    m_Pose.shouldApplyHeadModel = false;

    // This call posts this pose to shared memory, where all clients will have access to it the next
    // moment they want to predict a pose.
    m_pDriverHost->TrackedDevicePoseUpdated(m_unSteamVRTrackedDeviceId, m_Pose);
}

bool CLeapHmdLatest::IsActivated() const
{
    return m_unSteamVRTrackedDeviceId != vr::k_unTrackedDeviceIndexInvalid;
}

bool CLeapHmdLatest::HasControllerId( int nBase, int nId )
{
    return nBase == m_nBase && nId == m_nId;
}

/** Process sixenseControllerData.  Return true if it's new to help caller manage sleep durations */
bool CLeapHmdLatest::Update(Frame &frame)
{
    UpdateTrackingState(frame);
    UpdateControllerState(frame);

    return true;
}

// Alignment of the coordinate system of driver_leap with the HMD:
void CLeapHmdLatest::RealignCoordinates( CLeapHmdLatest * pLeapA, CLeapHmdLatest * pLeapB )
{
    if ( pLeapA->m_unSteamVRTrackedDeviceId == vr::k_unTrackedDeviceIndexInvalid )
        return;

    pLeapA->m_pAlignmentPartner = pLeapB;
    pLeapB->m_pAlignmentPartner = pLeapA;

    // Ask leap_monitor to tell us HMD pose
    static vr::VREvent_Data_t nodata = { 0 };
    pLeapA->m_pDriverHost->VendorSpecificEvent( pLeapA->m_unSteamVRTrackedDeviceId,
        (vr::EVREventType) (vr::VREvent_VendorSpecific_Reserved_Start + 0), nodata,
        -std::chrono::duration_cast<std::chrono::seconds>( k_TrackingLatency ).count() );
}

// leap_monitor called us back with the HMD information
void CLeapHmdLatest::FinishRealignCoordinates(float(*m)[3], float *v )
{
    CLeapHmdLatest * pLeapA = this;
    CLeapHmdLatest * pLeapB = m_pAlignmentPartner;

    if ( !pLeapA || !pLeapB )
        return;

    vr::HmdQuaternion_t q = CalculateRotation(m);
    pLeapA->UpdateHmdPose(v, q);
    pLeapB->UpdateHmdPose(v, q);
}

void CLeapHmdLatest::UpdateHmdPose(float *v, vr::HmdQuaternion_t q)
{
    memcpy(m_hmdPos, &v[0], sizeof(m_hmdPos));
    m_hmdRot = q;
    m_bCalibrated = true;
}
