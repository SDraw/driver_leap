#include "stdafx.h"
#include "CServerDriver.h"
#include "CDriverLogHelper.h"
#include "CConfigHelper.h"
#include "CLeapHandController.h"
#include "Utils.h"

extern char g_moduleFileName[];

void CLeapListener::onInit(const Leap::Controller &controller)
{
    controller.setPolicy(Leap::Controller::POLICY_OPTIMIZE_HMD);
}
void CLeapListener::onLogMessage(const Leap::Controller &controller, Leap::MessageSeverity severity, int64_t timestamp, const char *msg)
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
    m_leapMonitorLaunched = false;
}

CServerDriver::~CServerDriver()
{
}

vr::EVRInitError CServerDriver::Init(vr::IVRDriverContext *pDriverContext)
{
    CConfigHelper::LoadConfig();

    VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
    CDriverLogHelper::InitDriverLog(vr::VRDriverLog());

    m_driverHost = vr::VRServerDriverHost();

    // Generate VR controllers, serials are stored for whole VR session
    if(CConfigHelper::IsLeftHandEnabled())
    {
        CLeapHandController *l_leftHandController = new CLeapHandController(m_driverHost, CLeapHandController::CHA_Left);
        m_handControllers.push_back(l_leftHandController);
        if(m_driverHost) m_driverHost->TrackedDeviceAdded(l_leftHandController->GetSerialNumber(), vr::ETrackedDeviceClass::TrackedDeviceClass_Controller, l_leftHandController);
    }
    if(CConfigHelper::IsRightHandEnabled())
    {
        CLeapHandController *l_rightHandController = new CLeapHandController(m_driverHost, CLeapHandController::CHA_Right);
        m_handControllers.push_back(l_rightHandController);
        if(m_driverHost) m_driverHost->TrackedDeviceAdded(l_rightHandController->GetSerialNumber(), vr::ETrackedDeviceClass::TrackedDeviceClass_Controller, l_rightHandController);
    }

    m_leapController = new Leap::Controller();
    m_leapController->addListener(m_leapListener);

    LaunchLeapMonitor();

    return vr::VRInitError_None;
}

void CServerDriver::Cleanup()
{
    CDriverLogHelper::CleanupDriverLog();

    if(m_leapMonitorLaunched)
    {
        PostThreadMessage(m_processInfo.dwThreadId, WM_QUIT, 0, 0);
        m_leapMonitorLaunched = false;
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

            Leap::Frame l_frame = m_leapController->frame();
            if(l_frame.isValid())
            {
                for(auto iter : m_handControllers) iter->Update(l_frame);
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
    if(!m_leapMonitorLaunched)
    {
        std::string path(g_moduleFileName);
        path.erase(path.begin() + path.rfind('\\'), path.end());

        m_leapMonitorLaunched = true;
        STARTUPINFOA sInfoProcess = { 0 };
        sInfoProcess.cb = sizeof(STARTUPINFOW);
        CreateProcessA((path + "\\leap_monitor.exe").c_str(), NULL, NULL, NULL, FALSE, 0, NULL, path.c_str(), &sInfoProcess, &m_processInfo);
    }
}
