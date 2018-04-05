#include "stdafx.h"
#include "CServerDriver_Leap.h"
#include "CDriverLogHelper.h"
#include "CLeapHmdLatest.h"
#include "Utils.h"

extern char g_ModuleFileName[];

void CServerDriver_Leap::onInit(const Leap::Controller& controller)
{
    CDriverLogHelper::DriverLog("CServerDriver_Leap::onInit()\n");
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

    if(controller.isConnected())
    {
        Leap::Config configuraton = controller.config();
        bool backgroundModeAllowed = (configuraton.getInt32("background_app_mode") == 2);
        if(!backgroundModeAllowed)
        {
            configuraton.setInt32("background_app_mode", 2);
            configuraton.save();
        }

        controller.setPolicy(Leap::Controller::POLICY_OPTIMIZE_HMD);
        controller.setPolicy(Leap::Controller::POLICY_BACKGROUND_FRAMES);
        controller.setPolicy((Leap::Controller::PolicyFlag)(15));
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

void CServerDriver_Leap::onImages(const Leap::Controller& controller)
{
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
    Cleanup();
}

vr::EVRInitError CServerDriver_Leap::Init(vr::IVRDriverContext *pDriverContext)
{
    VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
    CDriverLogHelper::InitDriverLog(vr::VRDriverLog());
    CDriverLogHelper::DriverLog("CServerDriver_Leap::Init()\n");

    m_pDriverHost = vr::VRServerDriverHost();

    m_Controller = new Leap::Controller;

    m_Controller->addListener(*this);

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

    if(m_Controller)
    {
        m_Controller->removeListener(*this);
        delete m_Controller;
        m_Controller = NULL;
    }

    for(auto it = m_vecControllers.begin(); it != m_vecControllers.end(); ++it)
        delete (*it);
    m_vecControllers.clear();
}

uint32_t CServerDriver_Leap::GetTrackedDeviceCount()
{
    return m_vecControllers.size();
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
    if(m_vecControllers.size() == 2)  CLeapHmdLatest::RealignCoordinates(m_vecControllers[0], m_vecControllers[1]);

    if(m_Controller)
    {
        if(m_Controller->isConnected())
        {
            Leap::Frame frame = m_Controller->frame();

            for(auto it = m_vecControllers.begin(); it != m_vecControllers.end(); ++it)
            {
                CLeapHmdLatest *pLeap = *it;
                if(pLeap->IsActivated())
                {
                    if(!pLeap->Update(frame))
                    {
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
                m_pDriverHost->TrackedDeviceAdded(m_vecControllers.back()->GetSerialNumber(), vr::ETrackedDeviceClass::TrackedDeviceClass_Controller, m_vecControllers.back());
            }
        }
    }
}

void CServerDriver_Leap::LaunchLeapMonitor()
{
    if(m_bLaunchedLeapMonitor) return;

    CDriverLogHelper::DriverLog("CServerDriver_Leap::LaunchLeapMonitor()\n");

    m_bLaunchedLeapMonitor = true;

    std::string path(g_ModuleFileName);
    path.erase(path.begin() + path.rfind('\\'), path.end());
    CDriverLogHelper::DriverLog("leap_monitor path: %s\n", path.c_str());

    STARTUPINFOA sInfoProcess = { 0 };
    sInfoProcess.cb = sizeof(STARTUPINFOW);
    BOOL okay = CreateProcessA((path + "\\leap_monitor.exe").c_str(), NULL, NULL, NULL, FALSE, 0, NULL, path.c_str(), &sInfoProcess, &m_pInfoStartedProcess);
    CDriverLogHelper::DriverLog("start leap_monitor: %d %8x\n", okay, GetLastError());
}
