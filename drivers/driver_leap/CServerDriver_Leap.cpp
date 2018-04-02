#include "stdafx.h"
#include "CServerDriver_Leap.h"
#include "CDriverLogHelper.h"
#include "CLeapHmdLatest.h"
#include "Utils.h"

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
void CServerDriver_Leap::onInit(const Leap::Controller& controller)
{
    CDriverLogHelper::DriverLog("CServerDriver_Leap::onInit()\n");
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
void CServerDriver_Leap::onConnect(const Leap::Controller& controller)
{
    CDriverLogHelper::DriverLog("CServerDriver_Leap::onConnect()\n");
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
void CServerDriver_Leap::onDisconnect(const Leap::Controller& controller)
{
    CDriverLogHelper::DriverLog("CServerDriver_Leap::onDisconnect()\n");
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
void CServerDriver_Leap::onExit(const Leap::Controller& controller)
{
    CDriverLogHelper::DriverLog("CServerDriver_Leap::onExit()\n");
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
void CServerDriver_Leap::onFrame(const Leap::Controller& controller)
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
void CServerDriver_Leap::onFocusGained(const Leap::Controller& controller)
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
void CServerDriver_Leap::onFocusLost(const Leap::Controller& controller)
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
void CServerDriver_Leap::onServiceConnect(const Leap::Controller& controller)
{
    CDriverLogHelper::DriverLog("CServerDriver_Leap::onServiceConnect()\n");
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
void CServerDriver_Leap::onServiceDisconnect(const Leap::Controller& controller)
{
    CDriverLogHelper::DriverLog("CServerDriver_Leap::onServiceDisconnect()\n");
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
void CServerDriver_Leap::onDeviceChange(const Leap::Controller& controller)
{
    CDriverLogHelper::DriverLog("CServerDriver_Leap::onDeviceChange()\n");

    if(controller.isConnected())
    {
        Leap::Config configuraton = controller.config();
        bool backgroundModeAllowed = (configuraton.getInt32("background_app_mode") == 2);
        if(!backgroundModeAllowed)
        {
            // TODO: Show dialog to request permission to allow background mode apps
            /*bool userPermission = true;
            if (userPermission) {*/
            configuraton.setInt32("background_app_mode", 2);
            configuraton.save();
            //}
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
        for(auto it = m_vecControllers.begin(); it != m_vecControllers.end(); ++it)
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
void CServerDriver_Leap::onImages(const Leap::Controller& controller)
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
void CServerDriver_Leap::onServiceChange(const Leap::Controller& controller)
{
    CDriverLogHelper::DriverLog("CServerDriver_Leap::onServiceChange()\n");
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
void CServerDriver_Leap::onDeviceFailure(const Leap::Controller& controller)
{
    CDriverLogHelper::DriverLog("CServerDriver_Leap::onDeviceFailure()\n");
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
void CServerDriver_Leap::onLogMessage(const Leap::Controller& controller, Leap::MessageSeverity severity, int64_t timestamp, const char* msg)
{
    CDriverLogHelper::DriverLog("CServerDriver_Leap::onLogMessage(%d): %s\n", (int)severity, msg);
}


//==================================================================================================
// Server Provider
//==================================================================================================

CServerDriver_Leap::CServerDriver_Leap()
    : m_bLaunchedLeapMonitor(false)
{
    //    DriverLog not yet initialized at this point.
    m_pDriverHost = nullptr;
    m_Controller = nullptr;
    memset(&m_pInfoStartedProcess, 0, sizeof(PROCESS_INFORMATION));
    //    DriverLog("CServerDriver_Leap::CServerDriver_Leap()\n");
}

CServerDriver_Leap::~CServerDriver_Leap()
{
    CDriverLogHelper::DriverLog("CServerDriver_Leap::~CServerDriver_Leap()\n");
    Cleanup();
}

vr::EVRInitError CServerDriver_Leap::Init(vr::IDriverLog* pDriverLog, vr::IServerDriverHost* pDriverHost, const char* pchUserDriverConfigDir, const char* pchDriverInstallDir)
{
    CDriverLogHelper::InitDriverLog(pDriverLog);
    CDriverLogHelper::DriverLog("CServerDriver_Leap::Init()\n");

    m_pDriverHost = pDriverHost;
    m_strDriverInstallDir = pchDriverInstallDir;

    m_Controller = new Leap::Controller;

    m_Controller->addListener(*this);

    return vr::VRInitError_None;
}

void CServerDriver_Leap::Cleanup()
{
    CDriverLogHelper::DriverLog("CServerDriver_Leap::Cleanup()\n");

    // send a termination message to the leap monitor companion application
    if(m_bLaunchedLeapMonitor)
    {
        // Ask leap_monitor to shut down.
        PostThreadMessage(m_pInfoStartedProcess.dwThreadId, WM_QUIT, 0, 0);
        m_bLaunchedLeapMonitor = false;
    }

    // clean up our Leap::Controller object
    if(m_Controller)
    {
        m_Controller->removeListener(*this);
        delete m_Controller;
        m_Controller = NULL;
    }

    // clean up any controller objects we've created
    for(auto it = m_vecControllers.begin(); it != m_vecControllers.end(); ++it)
        delete (*it);
    m_vecControllers.clear();
}

uint32_t CServerDriver_Leap::GetTrackedDeviceCount()
{
    return m_vecControllers.size();
}

vr::ITrackedDeviceServerDriver* CServerDriver_Leap::GetTrackedDeviceDriver(uint32_t unWhich)
{
    if(unWhich < m_vecControllers.size())
        return m_vecControllers[unWhich];

    return nullptr;
}

vr::ITrackedDeviceServerDriver* CServerDriver_Leap::FindTrackedDeviceDriver(const char* pchId)
{
    for(auto it = m_vecControllers.begin(); it != m_vecControllers.end(); ++it)
    {
        if(0 == strcmp((*it)->GetSerialNumber(), pchId))
        {
            return *it;
        }
    }
    return nullptr;
}

void CServerDriver_Leap::RunFrame()
{
    if(m_vecControllers.size() == 2)  m_vecControllers[0]->RealignCoordinates(m_vecControllers[0], m_vecControllers[1]);

    if(m_Controller)
    {
        if(m_Controller->isConnected())
        {
            Leap::Frame frame = m_Controller->frame();

            // update the controllers
            for(auto it = m_vecControllers.begin(); it != m_vecControllers.end(); ++it)
            {
                CLeapHmdLatest *pLeap = *it;
                if(pLeap->IsActivated())
                {
                    // Returns true if this is new data (so we can sleep for long interval)
                    if(!pLeap->Update(frame))
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
    CDriverLogHelper::DriverLog("CServerDriver_Leap::EnterStandby()\n");
}

void CServerDriver_Leap::LeaveStandby()
{
    CDriverLogHelper::DriverLog("CServerDriver_Leap::LeaveStandby()\n");
}

void CServerDriver_Leap::ScanForNewControllers(bool bNotifyServer)
{
    while(m_vecControllers.size() < 2)
    {
        char buf[256];
        int base = 0;
        int i = m_vecControllers.size();
        GenerateSerialNumber(buf, sizeof(buf), base, i);
        if(!FindTrackedDeviceDriver(buf))
        {
            CDriverLogHelper::DriverLog("added new device %s\n", buf);
            m_vecControllers.push_back(new CLeapHmdLatest(m_pDriverHost, base, i));
            if(bNotifyServer && m_pDriverHost)
            {
                m_pDriverHost->TrackedDeviceAdded(m_vecControllers.back()->GetSerialNumber());
            }
        }
    }
}

// The leap_monitor is a companion program which will tell us the pose of the HMD.
void CServerDriver_Leap::LaunchLeapMonitor(const char* pchDriverInstallDir)
{
    if(m_bLaunchedLeapMonitor)
        return;

    CDriverLogHelper::DriverLog("CServerDriver_Leap::LaunchLeapMonitor()\n");

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
    CDriverLogHelper::DriverLog("leap_monitor path: %s\n", ss.str().c_str());

#if defined( _WIN32 )
    STARTUPINFOA sInfoProcess = { 0 };
    sInfoProcess.cb = sizeof(STARTUPINFOW);
    //    sInfoProcess.dwFlags = STARTF_USESHOWWINDOW;
    //    sInfoProcess.wShowWindow = SW_SHOWDEFAULT;
    BOOL okay = CreateProcessA((ss.str() + "\\leap_monitor.exe").c_str(), NULL, NULL, NULL, FALSE, 0, NULL, ss.str().c_str(), &sInfoProcess, &m_pInfoStartedProcess);
    CDriverLogHelper::DriverLog("start leap_monitor okay: %d %08x\n", okay, GetLastError());
#else
#error Do not know how to launch leap_monitor
#endif
}

/** Launch leap_monitor if needed (requested by devices as they activate) */
void CServerDriver_Leap::LaunchLeapMonitor()
{
    LaunchLeapMonitor(m_strDriverInstallDir.c_str());
}
