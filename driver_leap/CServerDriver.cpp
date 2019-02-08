#include "stdafx.h"
#include "CServerDriver.h"
#include "CDriverLogHelper.h"
#include "CConfigHelper.h"
#include "CLeapHandController.h"
#include "Utils.h"

extern char g_moduleFileName[];

void CLeapListener::onInit(const Leap::Controller& controller)
{
    controller.setPolicy(Leap::Controller::POLICY_OPTIMIZE_HMD);
}

void CLeapListener::onLogMessage(const Leap::Controller& controller, Leap::MessageSeverity severity, int64_t timestamp, const char* msg)
{
    CDriverLogHelper::DriverLog("(%d) - %s\n", static_cast<int>(severity), msg);
}


const char* const CServerDriver::ms_interfaces[] = {
    vr::ITrackedDeviceServerDriver_Version,
    vr::IServerTrackedDeviceProvider_Version,
    nullptr
};

CServerDriver::CServerDriver()
{
    m_driverHost = nullptr;
    m_leapController = nullptr;
    memset(&m_processInfo, 0, sizeof(PROCESS_INFORMATION));
    m_updatedVRConnectivity = false;
    m_bLaunchedLeapMonitor = false;
}

CServerDriver::~CServerDriver()
{
}

vr::EVRInitError CServerDriver::Init(vr::IVRDriverContext *pDriverContext)
{
    VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
    CDriverLogHelper::InitDriverLog(vr::VRDriverLog());
    CConfigHelper::LoadConfig();

    m_driverHost = vr::VRServerDriverHost();
    // Generate VR controllers, serials are stored for whole VR session
    for(int i = 0; i < 2; i++)
    {
        CLeapHandController *l_handController = new CLeapHandController(m_driverHost, i);
        m_handControllers.push_back(l_handController);
        if(m_driverHost) m_driverHost->TrackedDeviceAdded(l_handController->GetSerialNumber(), vr::ETrackedDeviceClass::TrackedDeviceClass_Controller, l_handController);
    }

    m_leapController = new Leap::Controller();
    m_leapController->addListener(m_leapListener);

    LaunchLeapMonitor();

    return vr::VRInitError_None;
}

void CServerDriver::Cleanup()
{
    CDriverLogHelper::CleanupDriverLog();

    if(m_bLaunchedLeapMonitor)
    {
        PostThreadMessage(m_processInfo.dwThreadId, WM_QUIT, 0, 0);
        m_bLaunchedLeapMonitor = false;
    }

    for(auto iter : m_handControllers) delete iter;
    m_handControllers.clear();

    if(m_leapController)
    {
        m_leapController->removeListener(m_leapListener);
        delete m_leapController;
        m_leapController = nullptr;
    }

    m_driverHost = nullptr;
}

const char* const* CServerDriver::GetInterfaceVersions()
{
    return ms_interfaces;
}

void CServerDriver::RunFrame()
{
    CLeapHandController::UpdateHMDCoordinates(m_driverHost);

    if(m_leapController)
    {
        if(m_leapController->isConnected())
        {
            m_updatedVRConnectivity = false;

            Leap::Frame frame = m_leapController->frame();
            if(frame.isValid())
            {
                for(auto iter : m_handControllers) iter->Update(frame);
            }
        }
        else
        {
            if(!m_updatedVRConnectivity)
            {
                for(auto iter : m_handControllers) iter->SetAsDisconnected();
                m_updatedVRConnectivity = true;
            }
        }
    }
}

bool CServerDriver::ShouldBlockStandbyMode()
{
    return false;
}

void CServerDriver::LaunchLeapMonitor()
{
    if(m_bLaunchedLeapMonitor) return;

    std::string path(g_moduleFileName);
    path.erase(path.begin() + path.rfind('\\'), path.end());

    m_bLaunchedLeapMonitor = true;
    STARTUPINFOA sInfoProcess = { 0 };
    sInfoProcess.cb = sizeof(STARTUPINFOW);
    CreateProcessA((path + "\\leap_monitor.exe").c_str(), NULL, NULL, NULL, FALSE, 0, NULL, path.c_str(), &sInfoProcess, &m_processInfo);
}
