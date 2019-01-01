#include "stdafx.h"
#include "CServerDriver_Leap.h"
#include "CDriverLogHelper.h"
#include "CConfigHelper.h"
#include "CLeapHmdLatest.h"
#include "Utils.h"

const char* const CServerDriver_Leap::ms_interfaces[] = {
    vr::ITrackedDeviceServerDriver_Version,
    vr::IServerTrackedDeviceProvider_Version,
    nullptr
};

extern char g_ModuleFileName[];

void CServerDriver_Leap::onInit(const Leap::Controller& controller)
{
    if(&controller == m_Controller)
    {
        // Set config for Leap controller
        m_Controller->setPolicy(Leap::Controller::POLICY_OPTIMIZE_HMD);

        // Generate VR controllers, serials are stored for whole VR session
        while(m_vecVRControllers.size() < 2U)
        {
            CLeapHmdLatest *l_leapHmd = new CLeapHmdLatest(m_pDriverHost, m_vecVRControllers.size());
            m_vecVRControllers.push_back(l_leapHmd);
            if(m_pDriverHost)
            {
                m_pDriverHost->TrackedDeviceAdded(l_leapHmd->GetSerialNumber(), vr::ETrackedDeviceClass::TrackedDeviceClass_Controller, l_leapHmd);
            }
        }
    }
}

void CServerDriver_Leap::onLogMessage(const Leap::Controller& controller, Leap::MessageSeverity severity, int64_t timestamp, const char* msg)
{
    if(&controller == m_Controller) CDriverLogHelper::DriverLog("(%d) - %s\n", static_cast<int>(severity), msg);
}


CServerDriver_Leap::CServerDriver_Leap()
    : m_bLaunchedLeapMonitor(false)
{
    m_pDriverHost = nullptr;
    m_Controller = nullptr;
    memset(&m_pInfoStartedProcess, 0, sizeof(PROCESS_INFORMATION));
    m_updatedVRConnectivity = false;
}

CServerDriver_Leap::~CServerDriver_Leap()
{
}

vr::EVRInitError CServerDriver_Leap::Init(vr::IVRDriverContext *pDriverContext)
{
    VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
    CDriverLogHelper::InitDriverLog(vr::VRDriverLog());
    CConfigHelper::LoadConfig();

    m_pDriverHost = vr::VRServerDriverHost();

    m_Controller = new Leap::Controller();
    m_Controller->addListener(*this);

    LaunchLeapMonitor();

    return vr::VRInitError_None;
}

void CServerDriver_Leap::Cleanup()
{
    CDriverLogHelper::CleanupDriverLog();

    if(m_bLaunchedLeapMonitor)
    {
        PostThreadMessage(m_pInfoStartedProcess.dwThreadId, WM_QUIT, 0, 0);
        m_bLaunchedLeapMonitor = false;
    }

    for(auto iter : m_vecVRControllers) delete iter;
    m_vecVRControllers.clear();

    if(m_Controller)
    {
        // There is no 'delete m_Controller' due to regular SteamVR crash at this line
        // Can be problem in virtual destructor of Leap::Controller, can't be fixed with derived class with working destructor
        m_Controller->removeListener(*this);
        m_Controller = nullptr;
    }

    m_pDriverHost = nullptr;
}

void CServerDriver_Leap::RunFrame()
{
    for(auto iter : m_vecVRControllers) iter->RealignCoordinates();

    if(m_Controller)
    {
        if(m_Controller->isConnected())
        {
            m_updatedVRConnectivity = false;

            Leap::Frame frame = m_Controller->frame();
            for(auto iter : m_vecVRControllers) iter->Update(frame);
        }
        else
        {
            if(!m_updatedVRConnectivity)
            {
                for(auto iter : m_vecVRControllers) iter->SetAsDisconnected();
                m_updatedVRConnectivity = true;
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
}

void CServerDriver_Leap::LeaveStandby()
{
}

void CServerDriver_Leap::LaunchLeapMonitor()
{
    if(m_bLaunchedLeapMonitor) return;

    std::string path(g_ModuleFileName);
    path.erase(path.begin() + path.rfind('\\'), path.end());

    m_bLaunchedLeapMonitor = true;
    STARTUPINFOA sInfoProcess = { 0 };
    sInfoProcess.cb = sizeof(STARTUPINFOW);
    CreateProcessA((path + "\\leap_monitor.exe").c_str(), NULL, NULL, NULL, FALSE, 0, NULL, path.c_str(), &sInfoProcess, &m_pInfoStartedProcess);
}
