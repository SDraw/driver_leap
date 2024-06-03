#include "stdafx.h"
#include "CControllerInput.h"
#include "JoyShockLibrary.h"

CControllerInput::CControllerInput()
{
    m_deviceCount = -1;
}

CControllerInput::~CControllerInput()
{
    JslDisconnectAndDisposeAll();
}

bool CControllerInput::IsConnected()
{
    // NOTE: Joycons work as a pair, so we check if both are still alive, otherwise this check will fail.
    if ((m_devices[ControllerType::CONTROLLER_JOYCON_LEFT].connected &&
        m_devices[ControllerType::CONTROLLER_JOYCON_RIGHT].connected) ||
        m_devices[ControllerType::CONTROLLER_JOYCON_DS4].connected)
    {
        return true;
    }

    m_deviceCount = JslConnectDevices();

    std::vector<int> handles(m_deviceCount);
    JslGetConnectedDeviceHandles(handles.data(), m_deviceCount);

    for (auto handle : handles)
    {
        int type = JslGetControllerType(handle);
        m_devices[type].connected = true;
        m_devices[type].handle = handle;
    }

    // For other controllers, just one of them is enough
    if (m_devices[ControllerType::CONTROLLER_JOYCON_DS4].connected)
        return true;

    // For joycons, we need both of them connected
    return m_devices[ControllerType::CONTROLLER_JOYCON_LEFT].connected && m_devices[ControllerType::CONTROLLER_JOYCON_RIGHT].connected;
}

void CControllerInput::Update(CLeapIndexController* left, CLeapIndexController* right)
{
    if (m_devices[ControllerType::CONTROLLER_JOYCON_LEFT].connected)
    {
        JOY_SHOCK_STATE state = JslGetSimpleState(m_devices[ControllerType::CONTROLLER_JOYCON_LEFT].handle);

        // Home
        left->SetButtonState(CLeapIndexController::IndexButton::IB_SystemTouch, state.buttons & 0x20000);
        left->SetButtonState(CLeapIndexController::IndexButton::IB_SystemClick, state.buttons & 0x20000);

        // A
        left->SetButtonState(CLeapIndexController::IndexButton::IB_AClick, state.buttons & 0x00002);
        left->SetButtonState(CLeapIndexController::IndexButton::IB_ATouch, state.buttons & 0x00002);

        // B
        left->SetButtonState(CLeapIndexController::IndexButton::IB_BClick, state.buttons & 0x00001);
        left->SetButtonState(CLeapIndexController::IndexButton::IB_BClick, state.buttons & 0x00001);

        // Joystick XY
        left->SetButtonState(CLeapIndexController::IndexButton::IB_ThumbstickClick, state.buttons & 0x00040);
        left->SetButtonState(CLeapIndexController::IndexButton::IB_ThumbstickTouch, state.stickLX > 0 && state.stickLY > 0);
        left->SetButtonValue(CLeapIndexController::IndexButton::IB_ThumbstickX, state.stickLX);
        left->SetButtonValue(CLeapIndexController::IndexButton::IB_ThumbstickY, state.stickLY);

        // Trigger
        left->SetButtonState(CLeapIndexController::IndexButton::IB_TriggerClick, state.buttons & 0x00400);
        left->SetButtonValue(CLeapIndexController::IndexButton::IB_TriggerValue, state.lTrigger);

        // Grip
        left->SetButtonState(CLeapIndexController::IndexButton::IB_GripTouch, state.buttons & 0x00100);
        left->SetButtonValue(CLeapIndexController::IndexButton::IB_GripValue, state.buttons & 0x00100 ? 1.0f : 0.0f);
        left->SetButtonValue(CLeapIndexController::IndexButton::IB_GripForce, state.buttons & 0x00100 ? 1.0f : 0.0f);
    }

    if (m_devices[ControllerType::CONTROLLER_JOYCON_RIGHT].connected)
    {
        JOY_SHOCK_STATE state = JslGetSimpleState(m_devices[ControllerType::CONTROLLER_JOYCON_RIGHT].handle);

        // Home
        right->SetButtonState(CLeapIndexController::IndexButton::IB_SystemTouch, state.buttons & 0x10000);
        right->SetButtonState(CLeapIndexController::IndexButton::IB_SystemClick, state.buttons & 0x10000);

        // A
        right->SetButtonState(CLeapIndexController::IndexButton::IB_AClick, state.buttons & 0x01000);
        right->SetButtonState(CLeapIndexController::IndexButton::IB_AClick, state.buttons & 0x01000);

        // B
        right->SetButtonState(CLeapIndexController::IndexButton::IB_BClick, state.buttons & 0x08000);
        right->SetButtonState(CLeapIndexController::IndexButton::IB_BClick, state.buttons & 0x08000);

        // Joystick XY
        right->SetButtonState(CLeapIndexController::IndexButton::IB_ThumbstickClick, state.buttons & 0x00080);
        right->SetButtonState(CLeapIndexController::IndexButton::IB_ThumbstickTouch, state.stickRX > 0 && state.stickRY > 0);
        right->SetButtonValue(CLeapIndexController::IndexButton::IB_ThumbstickX, state.stickRX);
        right->SetButtonValue(CLeapIndexController::IndexButton::IB_ThumbstickY, state.stickRY);

        // Trigger
        right->SetButtonState(CLeapIndexController::IndexButton::IB_TriggerClick, state.buttons & 0x00800);
        right->SetButtonValue(CLeapIndexController::IndexButton::IB_TriggerValue, state.rTrigger);

        // Grip
        right->SetButtonState(CLeapIndexController::IndexButton::IB_GripTouch, state.buttons & 0x00200);
        right->SetButtonValue(CLeapIndexController::IndexButton::IB_GripValue, state.buttons & 0x00200 ? 1.0f : 0.0f);
        right->SetButtonValue(CLeapIndexController::IndexButton::IB_GripForce, state.buttons & 0x00200 ? 1.0f : 0.0f);
    }

    if (m_devices[ControllerType::CONTROLLER_JOYCON_DS4].connected)
    {
        JOY_SHOCK_STATE state = JslGetSimpleState(m_devices[ControllerType::CONTROLLER_JOYCON_DS4].handle);

        // Home
        left->SetButtonState(CLeapIndexController::IndexButton::IB_SystemTouch, state.buttons & 0x10000);
        left->SetButtonState(CLeapIndexController::IndexButton::IB_SystemClick, state.buttons & 0x10000);

        // A
        left->SetButtonState(CLeapIndexController::IndexButton::IB_AClick, state.buttons & 0x00002);
        left->SetButtonState(CLeapIndexController::IndexButton::IB_ATouch, state.buttons & 0x00002);

        // B
        left->SetButtonState(CLeapIndexController::IndexButton::IB_BClick, state.buttons & 0x00001);
        left->SetButtonState(CLeapIndexController::IndexButton::IB_BClick, state.buttons & 0x00001);

        // Joystick XY
        left->SetButtonState(CLeapIndexController::IndexButton::IB_ThumbstickClick, state.buttons & 0x00040);
        left->SetButtonState(CLeapIndexController::IndexButton::IB_ThumbstickTouch, state.stickLX > 0 && state.stickLY > 0);
        left->SetButtonValue(CLeapIndexController::IndexButton::IB_ThumbstickX, state.stickLX);
        left->SetButtonValue(CLeapIndexController::IndexButton::IB_ThumbstickY, state.stickLY);

        // Trigger
        left->SetButtonState(CLeapIndexController::IndexButton::IB_TriggerClick, state.buttons & 0x00400);
        left->SetButtonValue(CLeapIndexController::IndexButton::IB_TriggerValue, state.lTrigger);

        // Grip
        left->SetButtonState(CLeapIndexController::IndexButton::IB_GripTouch, state.buttons & 0x00100);
        left->SetButtonValue(CLeapIndexController::IndexButton::IB_GripValue, state.buttons & 0x00100 ? 1.0f : 0.0f);
        left->SetButtonValue(CLeapIndexController::IndexButton::IB_GripForce, state.buttons & 0x00100 ? 1.0f : 0.0f);

        // Home
        right->SetButtonState(CLeapIndexController::IndexButton::IB_SystemTouch, state.buttons & 0x10000);
        right->SetButtonState(CLeapIndexController::IndexButton::IB_SystemClick, state.buttons & 0x10000);

        // A
        right->SetButtonState(CLeapIndexController::IndexButton::IB_AClick, state.buttons & 0x01000);
        right->SetButtonState(CLeapIndexController::IndexButton::IB_AClick, state.buttons & 0x01000);

        // B
        right->SetButtonState(CLeapIndexController::IndexButton::IB_BClick, state.buttons & 0x08000);
        right->SetButtonState(CLeapIndexController::IndexButton::IB_BClick, state.buttons & 0x08000);

        // Joystick XY
        right->SetButtonState(CLeapIndexController::IndexButton::IB_ThumbstickClick, state.buttons & 0x00080);
        right->SetButtonState(CLeapIndexController::IndexButton::IB_ThumbstickTouch, state.stickRX > 0 && state.stickRY > 0);
        right->SetButtonValue(CLeapIndexController::IndexButton::IB_ThumbstickX, state.stickRX);
        right->SetButtonValue(CLeapIndexController::IndexButton::IB_ThumbstickY, state.stickRY);

        // Trigger
        right->SetButtonState(CLeapIndexController::IndexButton::IB_TriggerClick, state.buttons & 0x00800);
        right->SetButtonValue(CLeapIndexController::IndexButton::IB_TriggerValue, state.rTrigger);

        // Grip
        right->SetButtonState(CLeapIndexController::IndexButton::IB_GripTouch, state.buttons & 0x00200);
        right->SetButtonValue(CLeapIndexController::IndexButton::IB_GripValue, state.buttons & 0x00200 ? 1.0f : 0.0f);
        right->SetButtonValue(CLeapIndexController::IndexButton::IB_GripForce, state.buttons & 0x00200 ? 1.0f : 0.0f);
    }

    for (size_t i = 0U; i < m_devices.size(); i++)
    {
        m_devices[i].connected = JslStillConnected(m_devices[i].handle);
    }
}