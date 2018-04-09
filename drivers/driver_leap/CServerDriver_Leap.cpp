#include "stdafx.h"
#include "CServerDriver_Leap.h"
#include "CDriverLogHelper.h"
#include "CConfigHelper.h"
#include "CLeapHmdLatest.h"
#include "Utils.h"

extern char g_ModuleFileName[];

void CServerDriver_Leap::onInit(const Leap::Controller& controller)
{
    CDriverLogHelper::DriverLog("CServerDriver_Leap::onInit()\n");

    if(&controller == m_Controller)
    {
        // Set config for Leap controller
        Leap::Config configuraton = m_Controller->config();
        if(configuraton.getInt32("background_app_mode") != 2)
        {
            configuraton.setInt32("background_app_mode", 2);
            configuraton.save();
        }
        m_Controller->setPolicy(Leap::Controller::POLICY_OPTIMIZE_HMD);
        m_Controller->setPolicy(Leap::Controller::POLICY_BACKGROUND_FRAMES);
        m_Controller->setPolicy((Leap::Controller::PolicyFlag)(15));
        m_Controller->setPolicy((Leap::Controller::PolicyFlag)(23));

        // Generate VR controllers, serials are stored for whole VR session
        while(m_vecVRControllers.size() < 2U)
        {
            char buf[256];
            int base = 0;
            int i = m_vecVRControllers.size();
            GenerateSerialNumber(buf, sizeof(buf), base, i);

            CLeapHmdLatest *l_leapHmd = new CLeapHmdLatest(m_pDriverHost, base, i);
            m_vecVRControllers.push_back(l_leapHmd);
            if(m_pDriverHost)
            {
                m_pDriverHost->TrackedDeviceAdded(l_leapHmd->GetSerialNumber(), vr::ETrackedDeviceClass::TrackedDeviceClass_Controller, l_leapHmd);
            }
        }
    }
}

void CServerDriver_Leap::onConnect(const Leap::Controller& controller)
{
    CDriverLogHelper::DriverLog("CServerDriver_Leap::onConnect()\n");
}

void CServerDriver_Leap::onDisconnect(const Leap::Controller& controller)
{
    CDriverLogHelper::DriverLog("CServerDriver_Leap::onDisconnect()\n");
}

void CServerDriver_Leap::onExit(const Leap::Controller& controller)
{
    CDriverLogHelper::DriverLog("CServerDriver_Leap::onExit()\n");
}

void CServerDriver_Leap::onFrame(const Leap::Controller& controller)
{
}

void CServerDriver_Leap::onFocusGained(const Leap::Controller& controller)
{
    //    DriverLog("CServerDriver_Leap::onFocusGained()\n");
}

void CServerDriver_Leap::onFocusLost(const Leap::Controller& controller)
{
    //    DriverLog("CServerDriver_Leap::onFocusLost()\n");
}

void CServerDriver_Leap::onServiceConnect(const Leap::Controller& controller)
{
    CDriverLogHelper::DriverLog("CServerDriver_Leap::onServiceConnect()\n");
}

void CServerDriver_Leap::onServiceDisconnect(const Leap::Controller& controller)
{
    CDriverLogHelper::DriverLog("CServerDriver_Leap::onServiceDisconnect()\n");
}

void CServerDriver_Leap::onDeviceChange(const Leap::Controller& controller)
{
    CDriverLogHelper::DriverLog("CServerDriver_Leap::onDeviceChange()\n");
}

void CServerDriver_Leap::onServiceChange(const Leap::Controller& controller)
{
    CDriverLogHelper::DriverLog("CServerDriver_Leap::onServiceChange()\n");
}

void CServerDriver_Leap::onDeviceFailure(const Leap::Controller& controller)
{
    CDriverLogHelper::DriverLog("CServerDriver_Leap::onDeviceFailure()\n");
}

void CServerDriver_Leap::onLogMessage(const Leap::Controller& controller, Leap::MessageSeverity severity, int64_t timestamp, const char* msg)
{
    CDriverLogHelper::DriverLog("CServerDriver_Leap::onLogMessage(%d): %s\n", (int)severity, msg);
}


CServerDriver_Leap::CServerDriver_Leap()
    : m_bLaunchedLeapMonitor(false)
{
    m_pDriverHost = nullptr;
    m_Controller = nullptr;
    memset(&m_pInfoStartedProcess, 0, sizeof(PROCESS_INFORMATION));
}

CServerDriver_Leap::~CServerDriver_Leap()
{
    CDriverLogHelper::DriverLog("CServerDriver_Leap::~CServerDriver_Leap()\n");
}

vr::EVRInitError CServerDriver_Leap::Init(vr::IVRDriverContext *pDriverContext)
{
    vr::EVRInitError eError = vr::InitServerDriverContext(pDriverContext);
    if(eError != vr::VRInitError_None) return eError;

    CDriverLogHelper::InitDriverLog(vr::VRDriverLog());
    CDriverLogHelper::DriverLog("CServerDriver_Leap::Init()\n");
    CConfigHelper::LoadConfig();

    m_pDriverHost = vr::VRServerDriverHost();

    m_Controller = new Leap::Controller;
    m_Controller->addListener(*this);

    LaunchLeapMonitor();

    return vr::VRInitError_None;
}

void CServerDriver_Leap::Cleanup()
{
    CDriverLogHelper::DriverLog("CServerDriver_Leap::Cleanup()\n");
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
        m_Controller->removeListener(*this);
        delete m_Controller;
        m_Controller = NULL;
    }

    vr::CleanupDriverContext();
    m_pDriverHost = nullptr;
}

void CServerDriver_Leap::RunFrame()
{
    if(m_vecVRControllers.size() == 2U) CLeapHmdLatest::RealignCoordinates(m_vecVRControllers[0], m_vecVRControllers[1]);

    if(m_Controller)
    {
        if(m_Controller->isConnected())
        {
            Leap::Frame frame = m_Controller->frame();

            for(auto iter : m_vecVRControllers)
            {
                if(iter->IsActivated()) iter->Update(frame);
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

void CServerDriver_Leap::LaunchLeapMonitor()
{
    if(m_bLaunchedLeapMonitor) return;

    CDriverLogHelper::DriverLog("CServerDriver_Leap::LaunchLeapMonitor()\n");

    std::string path(g_ModuleFileName);
    path.erase(path.begin() + path.rfind('\\'), path.end());
    CDriverLogHelper::DriverLog("leap_monitor path: %s\n", path.c_str());

    m_bLaunchedLeapMonitor = true;
    STARTUPINFOA sInfoProcess = { 0 };
    sInfoProcess.cb = sizeof(STARTUPINFOW);
    BOOL okay = CreateProcessA((path + "\\leap_monitor.exe").c_str(), NULL, NULL, NULL, FALSE, 0, NULL, path.c_str(), &sInfoProcess, &m_pInfoStartedProcess);
    CDriverLogHelper::DriverLog("start leap_monitor: %d %8x\n", okay, GetLastError());
}
