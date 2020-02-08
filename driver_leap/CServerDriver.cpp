#include "stdafx.h"

#include "CServerDriver.h"
#include "CLeapControllerVive.h"
#include "CLeapControllerIndex.h"
#include "CRelayDevice.h"

#include "CDriverConfig.h"
#include "CDriverLog.h"
#include "Utils.h"

extern char g_modulePath[];

// CLeapListener
void CLeapListener::onInit(const Leap::Controller &controller)
{
}
void CLeapListener::onLogMessage(const Leap::Controller &controller, Leap::MessageSeverity severity, int64_t timestamp, const char *msg)
{
    CDriverLog::Log("(%d) - %s\n", static_cast<int>(severity), msg);
}

// CServerDriver
const std::vector<std::string> g_debugRequests
{
    "profile", "game", "setting"
};
enum DebugRequest : size_t
{
    DR_Profile = 0U,
    DR_Game,
    DR_Setting
};

const std::vector<std::string> g_gameProfiles
{
    "Default", "VRChat"
};

const std::vector<std::string> g_gameCommands
{
    "special_mode"
};
enum GameCommand : size_t
{
    CC_SpecialMode = 0U
};

const std::vector<std::string> g_settingCommands
{
    "left_hand", "right_hand"
};
enum SettingCommand : size_t
{
    SC_LeftHand = 0U,
    SC_RightHand
};

const char* const CServerDriver::ms_interfaces[] = {
    vr::ITrackedDeviceServerDriver_Version,
    vr::IServerTrackedDeviceProvider_Version,
    nullptr
};

CServerDriver::CServerDriver()
{
    m_driverHost = nullptr;
    m_connectionState = false;
    m_leapController = nullptr;
    m_relayDevice = nullptr;
    m_monitorLaunched = false;
    m_monitorInfo = { 0 };
}
CServerDriver::~CServerDriver()
{
}

// vr::IServerTrackedDeviceProvider
void CServerDriver::Cleanup()
{
    CDriverLog::Cleanup();

    if(m_monitorLaunched)
    {
        PostThreadMessageA(m_monitorInfo.dwThreadId, WM_QUIT, 0, 0);
        m_monitorLaunched = false;
    }

    for(auto l_controller : m_controllers) delete l_controller;
    delete m_relayDevice;
    m_controllers.clear();
    CLeapController::SetInterfaces(nullptr, nullptr, nullptr);

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

vr::EVRInitError CServerDriver::Init(vr::IVRDriverContext *pDriverContext)
{
    vr::EVRInitError l_vrError = vr::VRInitError_None;
    CDriverConfig::LoadConfig();

    if(CDriverConfig::IsEnabled())
    {
        VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
        CDriverLog::Initialize(vr::VRDriverLog());

        m_driverHost = vr::VRServerDriverHost();
        CLeapController::SetInterfaces(m_driverHost, vr::VRDriverInput(), vr::VRProperties());

        // Relay device for events from leap_motinor
        m_relayDevice = new CRelayDevice(this);
        m_driverHost->TrackedDeviceAdded(m_relayDevice->GetSerialNumber().c_str(), vr::TrackedDeviceClass_TrackingReference, m_relayDevice);

        switch(CDriverConfig::GetEmulatedController())
        {
            case CDriverConfig::EC_Vive:
            {
                m_controllers.push_back(new CLeapControllerVive(CLeapController::CH_Left));
                m_controllers.push_back(new CLeapControllerVive(CLeapController::CH_Right));
            } break;
            case CDriverConfig::EC_Index:
            {
                m_controllers.push_back(new CLeapControllerIndex(CLeapController::CH_Left));
                m_controllers.push_back(new CLeapControllerIndex(CLeapController::CH_Right));
            } break;
        }

        for(auto l_controller : m_controllers)
        {
            l_controller->SetEnabled((l_controller->GetHand() == CLeapController::CH_Left) ? CDriverConfig::IsLeftHandEnabled() : CDriverConfig::IsRightHandEnabled());
            m_driverHost->TrackedDeviceAdded(l_controller->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, l_controller);
        }

        m_leapController = new Leap::Controller();
        m_leapController->addListener(m_leapListener);
        m_leapController->setPolicy(Leap::Controller::POLICY_ALLOW_PAUSE_RESUME);
        if(CDriverConfig::GetOrientationMode() == CDriverConfig::OM_HMD) m_leapController->setPolicy(Leap::Controller::POLICY_OPTIMIZE_HMD);
        m_leapController->setPaused(!CDriverConfig::IsLeftHandEnabled() && !CDriverConfig::IsRightHandEnabled());
        m_connectionState = true;

        if(!m_monitorLaunched)
        {
            std::string l_path(g_modulePath);
            l_path.erase(l_path.begin() + l_path.rfind('\\'), l_path.end());

            std::string l_appPath(l_path);
            l_appPath.append("\\leap_monitor.exe");

            STARTUPINFOA l_infoProcess = { 0 };
            l_infoProcess.cb = sizeof(STARTUPINFOA);
            m_monitorLaunched = CreateProcessA(l_appPath.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, l_path.c_str(), &l_infoProcess, &m_monitorInfo);
        }
    }
    else l_vrError = vr::VRInitError_Driver_NotLoaded;

    return l_vrError;
}

void CServerDriver::RunFrame()
{
    CLeapController::UpdateHMDCoordinates();
    if(m_leapController)
    {
        bool l_connectionState = m_leapController->isConnected();
        if(m_connectionState != l_connectionState)
        {
            m_connectionState = l_connectionState;
            for(auto l_handController : m_controllers) l_handController->SetEnabled(m_connectionState);
        }
        if(m_connectionState)
        {
            Leap::Frame l_frame = m_leapController->frame();
            if(l_frame.isValid())
            {
                for(auto l_handController : m_controllers) l_handController->Update(l_frame);
            }
        }
    }
    m_relayDevice->Update();
}

bool CServerDriver::ShouldBlockStandbyMode()
{
    return false;
}

// CServerDriver
void CServerDriver::ProcessLeapControllerPause()
{
    if(m_leapController)
    {
        bool l_pause = true;

        for(auto l_controller : m_controllers)
        {
            if(l_controller->GetEnabled())
            {
                l_pause = false;
                break;
            }
        }

        m_leapController->setPaused(l_pause);
    }
}

void CServerDriver::ProcessExternalMessage(const char *f_message)
{
    std::stringstream l_stream(f_message);
    std::string l_event;

    l_stream >> l_event;
    if(!l_stream.fail() && !l_event.empty())
    {
        switch(ReadEnumVector(l_event, g_debugRequests))
        {
            case DebugRequest::DR_Profile:
            {
                std::string l_profile;
                l_stream >> l_profile;

                if(!l_stream.fail() && !l_profile.empty())
                {
                    CLeapController::GameProfile l_newProfile;
                    switch(ReadEnumVector(l_profile, g_gameProfiles))
                    {
                        case CLeapController::GP_Default:
                            l_newProfile = CLeapController::GP_Default;
                            break;
                        case CLeapController::GP_VRChat:
                            l_newProfile = CLeapController::GP_VRChat;
                            break;
                        default:
                            l_newProfile = CLeapController::GP_Default;
                            break;
                    }

                    for(auto l_controller : m_controllers) l_controller->SetGameProfile(l_newProfile);
                }
            } break;

            case DR_Game:
            {
                std::string l_gameCommand;
                l_stream >> l_gameCommand;
                if(!l_stream.fail() && !l_gameCommand.empty())
                {
                    switch(ReadEnumVector(l_gameCommand, g_gameCommands))
                    {
                        case CC_SpecialMode:
                        {
                            for(auto l_controller : m_controllers) l_controller->SwitchSpecialMode();
                        } break;
                    }
                }
            } break;

            case DR_Setting:
            {
                std::string l_setting;
                l_stream >> l_setting;
                if(!l_stream.fail() && !l_setting.empty())
                {
                    switch(ReadEnumVector(l_setting, g_settingCommands))
                    {
                        case SC_LeftHand:
                        {
                            for(auto l_controller : m_controllers)
                            {
                                if(l_controller->GetHand() == CLeapController::CH_Left)
                                {
                                    l_controller->SetEnabled(!l_controller->GetEnabled());
                                    ProcessLeapControllerPause();
                                    break;
                                }
                            }
                        } break;
                        case SC_RightHand:
                        {
                            for(auto l_controller : m_controllers)
                            {
                                if(l_controller->GetHand() == CLeapController::CH_Right)
                                {
                                    l_controller->SetEnabled(!l_controller->GetEnabled());
                                    ProcessLeapControllerPause();
                                    break;
                                }
                            }
                        } break;
                    }
                }
            } break;
        }
    }
}
