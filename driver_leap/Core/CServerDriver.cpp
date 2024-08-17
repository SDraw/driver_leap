#include "stdafx.h"
#include "Core/CServerDriver.h"
#include "Leap/CLeapPoller.h"
#include "Leap/CLeapFrame.h"
#include "Devices/Controller/CLeapIndexController.h"
#include "Devices/Controller/CControllerInput.h"
#include "Devices/CLeapStation.h"
#include "Core/CDriverConfig.h"
#include "Core/CVREventsPoller.h"
#include "Utils/Utils.h"

extern std::wstring g_modulePath;

const char* const CServerDriver::ms_interfaces[]
{
    vr::ITrackedDeviceServerDriver_Version,
        vr::IServerTrackedDeviceProvider_Version,
        nullptr
};

CServerDriver::CServerDriver()
{
    m_leapPoller = nullptr;
    m_leapFrame = nullptr;
    m_connectionState = false;
    m_leftController = nullptr;
    m_rightController = nullptr;
    m_leapStation = nullptr;
    m_controllerInput = nullptr;
}

// vr::IServerTrackedDeviceProvider
vr::EVRInitError CServerDriver::Init(vr::IVRDriverContext *pDriverContext)
{
    VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
    CDriverConfig::Load();

    m_leapStation = new CLeapStation();
    vr::VRServerDriverHost()->TrackedDeviceAdded(m_leapStation->GetSerialNumber().c_str(), vr::TrackedDeviceClass_TrackingReference, m_leapStation);

    m_leftController = new CLeapIndexController(true);
    m_rightController = new CLeapIndexController(false);
    m_controllerInput = new CControllerInput();

    vr::VRServerDriverHost()->TrackedDeviceAdded(m_leftController->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, m_leftController);
    vr::VRServerDriverHost()->TrackedDeviceAdded(m_rightController->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, m_rightController);

    m_leapStation->SetControllers(m_leftController, m_rightController);

    m_leapFrame = new CLeapFrame();
    m_leapPoller = new CLeapPoller();
    m_leapPoller->Start();
#ifndef LEAP_ORION
    m_leapPoller->SetTrackingMode(_eLeapTrackingMode::eLeapTrackingMode_HMD);
    m_leapPoller->SetPolicy(eLeapPolicyFlag::eLeapPolicyFlag_OptimizeHMD, eLeapPolicyFlag::eLeapPolicyFlag_OptimizeScreenTop);
#else
    m_leapPoller->SetPolicy(eLeapPolicyFlag::eLeapPolicyFlag_OptimizeHMD);
#endif

    // Start leap_control
    std::wstring l_appPath(g_modulePath);
    l_appPath.append(L"\\leap_control\\leap_control.exe");

    STARTUPINFOW l_infoProcess = { 0 };
    PROCESS_INFORMATION l_monitorInfo = { 0 };
    l_infoProcess.cb = sizeof(STARTUPINFOA);
    CreateProcessW(l_appPath.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, g_modulePath.c_str(), &l_infoProcess, &l_monitorInfo);

    return vr::VRInitError_None;
}

void CServerDriver::Cleanup()
{
    delete m_leftController;
    m_leftController = nullptr;

    delete m_rightController;
    m_rightController = nullptr;

    delete m_leapStation;
    m_leapStation = nullptr;

    m_leapPoller->Stop();
    delete m_leapPoller;
    m_leapPoller = nullptr;

    delete m_leapFrame;
    m_leapFrame = nullptr;

    delete m_controllerInput;
    m_controllerInput = nullptr;

    m_connectionState = false;

    VR_CLEANUP_SERVER_DRIVER_CONTEXT();
}

const char* const* CServerDriver::GetInterfaceVersions()
{
    return ms_interfaces;
}

void CServerDriver::RunFrame()
{
    CLeapIndexController::UpdateHMDCoordinates();
    CVREventsPoller::PollEvents();

    if(m_connectionState != m_leapPoller->IsConnected())
    {
        m_connectionState = m_leapPoller->IsConnected();
        m_leapStation->SetTrackingState(m_connectionState ? CLeapStation::TS_Connected : CLeapStation::TS_Search);
        m_leftController->SetEnabled(m_connectionState);
        m_rightController->SetEnabled(m_connectionState);

#ifndef LEAP_ORION
        m_leapPoller->SetTrackingMode(_eLeapTrackingMode::eLeapTrackingMode_HMD);
        m_leapPoller->SetPolicy(eLeapPolicyFlag::eLeapPolicyFlag_OptimizeHMD, eLeapPolicyFlag::eLeapPolicyFlag_OptimizeScreenTop);
#else
        m_leapPoller->SetPolicy(eLeapPolicyFlag::eLeapPolicyFlag_OptimizeHMD);
#endif
    }

    if(m_connectionState && m_leapPoller->GetFrame(m_leapFrame->GetEvent()))
        m_leapFrame->Update();

    if(CDriverConfig::IsControllerInputUsed() && m_controllerInput->IsConnected())
        m_controllerInput->Update(m_leftController, m_rightController);

    // Update devices
    m_leftController->RunFrame(m_leapFrame->GetLeftHand());
    m_rightController->RunFrame(m_leapFrame->GetRightHand());
    m_leapStation->RunFrame();
}

bool CServerDriver::ShouldBlockStandbyMode()
{
    return false;
}

void CServerDriver::EnterStandby()
{
}

void CServerDriver::LeaveStandby()
{
}
