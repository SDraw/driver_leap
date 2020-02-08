#include "stdafx.h"
#include "CControllerButton.h"

CControllerButton::CControllerButton()
{
    m_handle = vr::k_ulInvalidInputComponentHandle;
    m_inputType = IT_Boolean;
    m_state = false;
    m_value = 0.f;
    m_updated = false;
}
CControllerButton::~CControllerButton()
{
}

void CControllerButton::SetState(bool f_state)
{
    if(m_inputType == IT_Boolean)
    {
        if(m_state != f_state)
        {
            m_state = f_state;
            m_updated = true;
        }
    }
}

void CControllerButton::SetValue(float f_value)
{
    if(m_inputType == IT_Float)
    {
        if(m_value != f_value)
        {
            m_value = f_value;
            m_updated = true;
        }
    }
}
