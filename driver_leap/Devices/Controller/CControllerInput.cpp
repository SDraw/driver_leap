#include "stdafx.h"
#include "Devices/Controller/CControllerInput.h"
#include "Devices/Controller/CLeapIndexController.h"

CControllerInput::CControllerInput()
{
    m_deviceCount = 0;
}

CControllerInput::~CControllerInput()
{
    JslDisconnectAndDisposeAll();
}

bool CControllerInput::IsConnected()
{
    // NOTE: Joycons work as a pair, so we check if both are still alive, otherwise this check will fail.
    if((m_devices[JS_TYPE_JOYCON_LEFT].connected && m_devices[JS_TYPE_JOYCON_RIGHT].connected) || m_devices[JS_TYPE_DS4].connected)
        return true;

    m_deviceCount = JslConnectDevices();

    std::vector<int> l_handles(m_deviceCount);
    JslGetConnectedDeviceHandles(l_handles.data(), m_deviceCount);

    for(auto l_handle : l_handles)
    {
        int l_type = JslGetControllerType(l_handle);
        m_devices[l_type].connected = true;
        m_devices[l_type].handle = l_handle;
    }

    // For other controllers, just one of them is enough
    if(m_devices[JS_TYPE_DS4].connected)
        return true;

    // For joycons, we need both of them connected
    return (m_devices[JS_TYPE_JOYCON_LEFT].connected && m_devices[JS_TYPE_JOYCON_RIGHT].connected);
}

void CControllerInput::Update(CLeapIndexController *p_left, CLeapIndexController *p_right)
{
    if(m_devices[JS_TYPE_JOYCON_LEFT].connected)
    {
        JOY_SHOCK_STATE l_state = JslGetSimpleState(m_devices[JS_TYPE_JOYCON_LEFT].handle);

        // Home
        p_left->SetButtonState(CLeapIndexController::IndexButton::IB_SystemTouch, l_state.buttons & JSMASK_CAPTURE);
        p_left->SetButtonState(CLeapIndexController::IndexButton::IB_SystemClick, l_state.buttons & JSMASK_CAPTURE);

        // A
        p_left->SetButtonState(CLeapIndexController::IndexButton::IB_AClick, l_state.buttons & JSMASK_DOWN);
        p_left->SetButtonState(CLeapIndexController::IndexButton::IB_ATouch, l_state.buttons & JSMASK_DOWN);

        // B
        p_left->SetButtonState(CLeapIndexController::IndexButton::IB_BClick, l_state.buttons & JSMASK_UP);
        p_left->SetButtonState(CLeapIndexController::IndexButton::IB_BTouch, l_state.buttons & JSMASK_UP);

        // Joystick XY
        p_left->SetButtonState(CLeapIndexController::IndexButton::IB_ThumbstickClick, l_state.buttons & JSMASK_LCLICK);
        p_left->SetButtonState(CLeapIndexController::IndexButton::IB_ThumbstickTouch, (l_state.stickLX > 0.f) && (l_state.stickLY > 0.f));
        p_left->SetButtonValue(CLeapIndexController::IndexButton::IB_ThumbstickX, l_state.stickLX);
        p_left->SetButtonValue(CLeapIndexController::IndexButton::IB_ThumbstickY, l_state.stickLY);

        // Trigger
        p_left->SetButtonState(CLeapIndexController::IndexButton::IB_TriggerClick, l_state.buttons & JSMASK_ZL);
        p_left->SetButtonState(CLeapIndexController::IndexButton::IB_TriggerTouch, l_state.lTrigger > 0.f);
        p_left->SetButtonValue(CLeapIndexController::IndexButton::IB_TriggerValue, l_state.lTrigger);

        // Grip
        p_left->SetButtonState(CLeapIndexController::IndexButton::IB_GripTouch, l_state.buttons & JSMASK_L);
        p_left->SetButtonValue(CLeapIndexController::IndexButton::IB_GripValue, (l_state.buttons & JSMASK_L) ? 1.f : 0.f);
        p_left->SetButtonValue(CLeapIndexController::IndexButton::IB_GripForce, (l_state.buttons & JSMASK_L) ? 1.f : 0.f);

        // Trackpad/touchpad
        p_left->SetButtonState(CLeapIndexController::IndexButton::IB_TrackpadTouch, l_state.buttons & JSMASK_SL);
        p_left->SetButtonValue(CLeapIndexController::IndexButton::IB_TrackpadForce, ((l_state.buttons & JSMASK_SL) && (l_state.buttons & JSMASK_SR)) ? 1.f : 0.f);
    }

    if(m_devices[JS_TYPE_JOYCON_RIGHT].connected)
    {
        JOY_SHOCK_STATE l_state = JslGetSimpleState(m_devices[JS_TYPE_JOYCON_RIGHT].handle);

        // Home
        p_right->SetButtonState(CLeapIndexController::IndexButton::IB_SystemTouch, l_state.buttons & JSMASK_HOME);
        p_right->SetButtonState(CLeapIndexController::IndexButton::IB_SystemClick, l_state.buttons & JSMASK_HOME);

        // A
        p_right->SetButtonState(CLeapIndexController::IndexButton::IB_AClick, l_state.buttons & JSMASK_S);
        p_right->SetButtonState(CLeapIndexController::IndexButton::IB_ATouch, l_state.buttons & JSMASK_S);

        // B
        p_right->SetButtonState(CLeapIndexController::IndexButton::IB_BClick, l_state.buttons & JSMASK_N);
        p_right->SetButtonState(CLeapIndexController::IndexButton::IB_BTouch, l_state.buttons & JSMASK_N);

        // Joystick XY
        p_right->SetButtonState(CLeapIndexController::IndexButton::IB_ThumbstickClick, l_state.buttons & JSMASK_RCLICK);
        p_right->SetButtonState(CLeapIndexController::IndexButton::IB_ThumbstickTouch, (l_state.stickRX > 0.f) && (l_state.stickRY > 0.f));
        p_right->SetButtonValue(CLeapIndexController::IndexButton::IB_ThumbstickX, l_state.stickRX);
        p_right->SetButtonValue(CLeapIndexController::IndexButton::IB_ThumbstickY, l_state.stickRY);

        // Trigger
        p_right->SetButtonState(CLeapIndexController::IndexButton::IB_TriggerClick, l_state.buttons & JSMASK_ZR);
        p_right->SetButtonState(CLeapIndexController::IndexButton::IB_TriggerTouch, l_state.rTrigger > 0.f);
        p_right->SetButtonValue(CLeapIndexController::IndexButton::IB_TriggerValue, l_state.rTrigger);

        // Grip
        p_right->SetButtonState(CLeapIndexController::IndexButton::IB_GripTouch, l_state.buttons & JSMASK_R);
        p_right->SetButtonValue(CLeapIndexController::IndexButton::IB_GripValue, (l_state.buttons & JSMASK_R) ? 1.0f : 0.0f);
        p_right->SetButtonValue(CLeapIndexController::IndexButton::IB_GripForce, (l_state.buttons & JSMASK_R) ? 1.0f : 0.0f);

        // Trackpad/touchpad
        p_left->SetButtonState(CLeapIndexController::IndexButton::IB_TrackpadTouch, l_state.buttons & JSMASK_SR);
        p_left->SetButtonValue(CLeapIndexController::IndexButton::IB_TrackpadForce, ((l_state.buttons & JSMASK_SR) && (l_state.buttons & JSMASK_SL)) ? 1.f : 0.f);
    }

    if(m_devices[JS_TYPE_DS4].connected)
    {
        JOY_SHOCK_STATE l_state = JslGetSimpleState(m_devices[JS_TYPE_DS4].handle);

        // Home
        p_left->SetButtonState(CLeapIndexController::IndexButton::IB_SystemTouch, l_state.buttons & JSMASK_HOME);
        p_left->SetButtonState(CLeapIndexController::IndexButton::IB_SystemClick, l_state.buttons & JSMASK_HOME);

        // A
        p_left->SetButtonState(CLeapIndexController::IndexButton::IB_AClick, l_state.buttons & JSMASK_DOWN);
        p_left->SetButtonState(CLeapIndexController::IndexButton::IB_ATouch, l_state.buttons & JSMASK_DOWN);

        // B
        p_left->SetButtonState(CLeapIndexController::IndexButton::IB_BClick, l_state.buttons & JSMASK_UP);
        p_left->SetButtonState(CLeapIndexController::IndexButton::IB_BTouch, l_state.buttons & JSMASK_UP);

        // Joystick XY
        p_left->SetButtonState(CLeapIndexController::IndexButton::IB_ThumbstickClick, l_state.buttons & JSMASK_LCLICK);
        p_left->SetButtonState(CLeapIndexController::IndexButton::IB_ThumbstickTouch, (l_state.stickLX > 0.f) && (l_state.stickLY > 0.f));
        p_left->SetButtonValue(CLeapIndexController::IndexButton::IB_ThumbstickX, l_state.stickLX);
        p_left->SetButtonValue(CLeapIndexController::IndexButton::IB_ThumbstickY, l_state.stickLY);

        // Trigger
        p_left->SetButtonState(CLeapIndexController::IndexButton::IB_TriggerClick, l_state.buttons & JSMASK_ZL);
        p_left->SetButtonState(CLeapIndexController::IndexButton::IB_TriggerTouch, l_state.lTrigger > 0.f);
        p_left->SetButtonValue(CLeapIndexController::IndexButton::IB_TriggerValue, l_state.lTrigger);

        // Grip
        p_left->SetButtonState(CLeapIndexController::IndexButton::IB_GripTouch, l_state.buttons & JSMASK_L);
        p_left->SetButtonValue(CLeapIndexController::IndexButton::IB_GripValue, (l_state.buttons & JSMASK_L) ? 1.f : 0.f);
        p_left->SetButtonValue(CLeapIndexController::IndexButton::IB_GripForce, (l_state.buttons & JSMASK_L) ? 1.f : 0.f);

        // Home
        p_right->SetButtonState(CLeapIndexController::IndexButton::IB_SystemTouch, l_state.buttons & JSMASK_HOME);
        p_right->SetButtonState(CLeapIndexController::IndexButton::IB_SystemClick, l_state.buttons & JSMASK_HOME);

        // A
        p_right->SetButtonState(CLeapIndexController::IndexButton::IB_AClick, l_state.buttons & JSMASK_S);
        p_right->SetButtonState(CLeapIndexController::IndexButton::IB_ATouch, l_state.buttons & JSMASK_S);

        // B
        p_right->SetButtonState(CLeapIndexController::IndexButton::IB_BClick, l_state.buttons & JSMASK_N);
        p_right->SetButtonState(CLeapIndexController::IndexButton::IB_BTouch, l_state.buttons & JSMASK_N);

        // Joystick XY
        p_right->SetButtonState(CLeapIndexController::IndexButton::IB_ThumbstickClick, l_state.buttons & JSMASK_RCLICK);
        p_right->SetButtonState(CLeapIndexController::IndexButton::IB_ThumbstickTouch, (l_state.stickRX > 0.f) && (l_state.stickRY > 0.f));
        p_right->SetButtonValue(CLeapIndexController::IndexButton::IB_ThumbstickX, l_state.stickRX);
        p_right->SetButtonValue(CLeapIndexController::IndexButton::IB_ThumbstickY, l_state.stickRY);

        // Trigger
        p_right->SetButtonState(CLeapIndexController::IndexButton::IB_TriggerClick, l_state.buttons & JSMASK_ZR);
        p_right->SetButtonState(CLeapIndexController::IndexButton::IB_TriggerTouch, l_state.rTrigger > 0.f);
        p_right->SetButtonValue(CLeapIndexController::IndexButton::IB_TriggerValue, l_state.rTrigger);

        // Grip
        p_right->SetButtonState(CLeapIndexController::IndexButton::IB_GripTouch, l_state.buttons & JSMASK_R);
        p_right->SetButtonValue(CLeapIndexController::IndexButton::IB_GripValue, (l_state.buttons & JSMASK_R) ? 1.f : 0.f);
        p_right->SetButtonValue(CLeapIndexController::IndexButton::IB_GripForce, (l_state.buttons & JSMASK_R) ? 1.f : 0.f);
    }

    for(int i = 0, j = static_cast<int>(m_devices.size()); i < j; i++)
        m_devices[i].connected = JslStillConnected(m_devices[i].handle);
}
