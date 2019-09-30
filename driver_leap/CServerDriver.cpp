#include "stdafx.h"
#include "CServerDriver.h"
#include "CDriverConfig.h"
#include "CDriverLog.h"
#include "CLeapHandControllerVive.h"
#include "CLeapHandControllerIndex.h"

extern char g_moduleFilePath[];

void CLeapListener::onInit(const Leap::Controller &controller)
{
}
void CLeapListener::onLogMessage(const Leap::Controller &controller, Leap::MessageSeverity severity, int64_t timestamp, const char *msg)
{
    CDriverLog::Log("(%d) - %s\n", static_cast<int>(severity), msg);
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
    std::memset(&m_processInfo, 0, sizeof(PROCESS_INFORMATION));
    m_controllerState = false;
    m_leapMonitorLaunched = false;
}

CServerDriver::~CServerDriver()
{
}

// vr::IServerTrackedDeviceProvider
vr::EVRInitError CServerDriver::Init(vr::IVRDriverContext *pDriverContext)
{
    CDriverConfig::LoadConfig();

    VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
    CDriverLog::Init(vr::VRDriverLog());

    m_driverHost = vr::VRServerDriverHost();
    CLeapHandController::SetInterfaces(m_driverHost, vr::VRDriverInput(), vr::VRProperties());

    if(CDriverConfig::IsLeftHandEnabled())
    {
        CLeapHandController *l_controller = nullptr;
        switch(CDriverConfig::GetEmulatedController())
        {
            case CDriverConfig::EC_Vive:
                l_controller = new CLeapHandControllerVive(CLeapHandController::CHA_Left);
                break;
            case CDriverConfig::EC_Index:
                l_controller = new CLeapHandControllerIndex(CLeapHandController::CHA_Left);
                break;
        }
        if(m_driverHost && l_controller)
        {
            m_handControllers.push_back(l_controller);
            m_driverHost->TrackedDeviceAdded(l_controller->GetSerialNumber().c_str(), vr::ETrackedDeviceClass::TrackedDeviceClass_Controller, l_controller);
        }
    }
    if(CDriverConfig::IsRightHandEnabled())
    {
        CLeapHandController *l_controller = nullptr;
        switch(CDriverConfig::GetEmulatedController())
        {
            case CDriverConfig::EC_Vive:
                l_controller = new CLeapHandControllerVive(CLeapHandController::CHA_Right);
                break;
            case CDriverConfig::EC_Index:
                l_controller = new CLeapHandControllerIndex(CLeapHandController::CHA_Right);
                break;
        }
        if(m_driverHost && l_controller)
        {
            m_handControllers.push_back(l_controller);
            m_driverHost->TrackedDeviceAdded(l_controller->GetSerialNumber().c_str(), vr::ETrackedDeviceClass::TrackedDeviceClass_Controller, l_controller);
        }
    }

    m_leapController = new Leap::Controller();
    m_leapController->addListener(m_leapListener);
    if(CDriverConfig::GetOrientationMode() == CDriverConfig::OM_HMD) m_leapController->setPolicy(Leap::Controller::POLICY_OPTIMIZE_HMD);

    LaunchLeapMonitor();

    return vr::VRInitError_None;
}

void CServerDriver::Cleanup()
{
    CDriverLog::Cleanup();

    if(m_leapMonitorLaunched)
    {
        PostThreadMessageA(m_processInfo.dwThreadId, WM_QUIT, 0, 0);
        m_leapMonitorLaunched = false;
    }

    for(auto l_controller : m_handControllers) delete l_controller;
    m_handControllers.clear();
    CLeapHandController::SetInterfaces(nullptr, nullptr, nullptr);

    if(m_leapController)
    {
        m_leapController->removeListener(m_leapListener);
        delete m_leapController;
        m_leapController = nullptr;
    }

    m_driverHost = nullptr;

    VR_CLEANUP_SERVER_DRIVER_CONTEXT();
}

const char* const* CServerDriver::GetInterfaceVersions()
{
    return ms_interfaces;
}

void CServerDriver::RunFrame()
{
    CLeapHandController::UpdateHMDCoordinates();
    if(m_leapController)
    {
        bool l_controllerState = m_leapController->isConnected();
        if(m_controllerState != l_controllerState)
        {
            m_controllerState = l_controllerState;
            for(auto l_handController : m_handControllers) l_handController->SetConnectionState(m_controllerState);
        }
        if(m_controllerState)
        {
            Leap::Frame l_frame = m_leapController->frame();
            if(l_frame.isValid())
            {
                for(auto l_handController : m_handControllers) l_handController->Update(l_frame);
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
        std::string l_path(g_moduleFilePath);
        l_path.erase(l_path.begin() + l_path.rfind('\\'), l_path.end());

        std::string l_appPath(l_path);
        l_appPath.append("\\leap_monitor.exe");

        STARTUPINFOA l_infoProcess = { 0 };
        l_infoProcess.cb = sizeof(STARTUPINFOA);
        m_leapMonitorLaunched = CreateProcessA(l_appPath.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, l_path.c_str(), &l_infoProcess, &m_processInfo);
    }
}
