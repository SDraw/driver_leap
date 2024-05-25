#include "stdafx.h"
#include "Utils/CButton.h"

CButton::CButton(ButtonType p_type, std::string p_name)
{
    m_name.assign(p_name);
    m_type = p_type;
    m_updated = false;
}

void CButton::SetState(ButtonState p_state)
{
    if(m_state != p_state)
    {
        m_state = p_state;
        m_updated = true;
    }
}

CButton::ButtonState CButton::GetState() const
{
    return m_state;
}

void CButton::SetAxis(const glm::vec2 & p_value)
{
    if(m_axisValues != p_value)
    {
        m_axisValues = p_value;
        m_updated = true;
    }
}

const glm::vec2 & CButton::GetAxis() const
{
    return m_axisValues;
}

const std::string & CButton::GetName() const
{
    return m_name;
}

CButton::ButtonType CButton::GetType() const
{
    return m_type;
}

bool CButton::IsUpdated() const
{
    return m_updated;
}

void CButton::ResetUpdate()
{
    m_updated = false;
}
