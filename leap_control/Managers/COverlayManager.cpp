#include "stdafx.h"
#include "Managers/COverlayManager.h"
#include "Managers/CVRManager.h"
#include "Managers/CLeapManager.h"
#include "Managers/CSettingsManager.h"
#include "Overlay/CHandOverlay.h"
#include "Overlay/CCursorOverlay.h"
#include "Utils/CButton.h"
#include "Utils/Utils.h"

const glm::mat4 g_identityMat4(1.f);
const glm::vec4 g_pointVec4(0.f, 0.f, 0.f, 1.f);

COverlayManager* COverlayManager::ms_instance = nullptr;

COverlayManager* COverlayManager::GetInstance()
{
    if(!ms_instance)
        ms_instance = new COverlayManager();

    return ms_instance;
}

COverlayManager::COverlayManager()
{
    m_leftHandOverlay = nullptr;
    m_rightHandOverlay = nullptr;
    m_leftCursorOverlay = nullptr;
    m_rightCursorOverlay = nullptr;
}

void COverlayManager::CreateOverlays()
{
    if(!m_leftHandOverlay)
    {
        m_leftHandOverlay = new CHandOverlay(true);
        m_leftHandOverlay->Create();
        m_leftHandOverlay->SetWidth(CSettingsManager::GetInstance()->GetOverlaySize());
        m_leftHandOverlay->SetVisible(CSettingsManager::GetInstance()->GetShowOverlays());
    }
    if(!m_rightHandOverlay)
    {
        m_rightHandOverlay = new CHandOverlay(false);
        m_rightHandOverlay->Create();
        m_rightHandOverlay->SetWidth(CSettingsManager::GetInstance()->GetOverlaySize());
        m_rightHandOverlay->SetVisible(CSettingsManager::GetInstance()->GetShowOverlays());
    }

    if(!m_leftCursorOverlay)
    {
        m_leftCursorOverlay = new CCursorOverlay(true);
        m_leftCursorOverlay->Create();
    }
    if(!m_rightCursorOverlay)
    {
        m_rightCursorOverlay = new CCursorOverlay(false);
        m_rightCursorOverlay->Create();
    }
}

void COverlayManager::DestroyOverlays()
{
    if(m_leftHandOverlay)
    {
        m_leftHandOverlay->Destroy();
        delete m_leftHandOverlay;
        m_leftHandOverlay = nullptr;
    }
    if(m_rightHandOverlay)
    {
        m_rightHandOverlay->Destroy();
        delete m_rightHandOverlay;
        m_rightHandOverlay = nullptr;
    }

    if(m_leftCursorOverlay)
    {
        m_leftCursorOverlay->Destroy();
        delete m_leftCursorOverlay;
        m_leftCursorOverlay = nullptr;
    }
    if(m_rightCursorOverlay)
    {
        m_rightCursorOverlay->Destroy();
        delete m_rightCursorOverlay;
        m_rightCursorOverlay = nullptr;
    }
}

void COverlayManager::Update()
{
    glm::quat l_hmdRot = CVRManager::GetInstance()->GetHmdRotation();

    if(m_leftCursorOverlay)
    {
        m_leftCursorOverlay->SetPosition(CLeapManager::GetInstance()->GetLeftTipPosition());
        m_leftCursorOverlay->SetRotation(l_hmdRot);
        m_leftCursorOverlay->Update();
    }
    if(m_rightCursorOverlay)
    {
        m_rightCursorOverlay->SetPosition(CLeapManager::GetInstance()->GetRightTipPosition());
        m_rightCursorOverlay->SetRotation(l_hmdRot);
        m_rightCursorOverlay->Update();
    }

    if(m_leftHandOverlay)
    {
        if(m_rightHandOverlay)
            m_leftHandOverlay->SetLocked(m_rightHandOverlay->IsInteracted());

        glm::mat4 l_overlayMat = glm::translate(g_identityMat4, CSettingsManager::GetInstance()->GetOverlayOffset());
        glm::vec3 l_pos = CSettingsManager::GetInstance()->GetOverlayAngle();
        l_overlayMat *= glm::toMat4(glm::quat(glm::radians(l_pos)));
        l_overlayMat = CVRManager::GetInstance()->GetLeftHandMatrix() * l_overlayMat;
        l_pos = l_overlayMat * g_pointVec4;
        glm::quat l_rot = glm::toQuat(l_overlayMat);
        m_leftHandOverlay->SetPosition(l_pos);
        m_leftHandOverlay->SetRotation(l_rot);
        m_leftHandOverlay->Update(CLeapManager::GetInstance()->GetRightTipPosition());

        ProcessButtons(m_leftHandOverlay->GetButtons(), m_leftHandOverlay->GetPressure(), true);
    }
    if(m_rightHandOverlay)
    {
        if(m_leftHandOverlay)
            m_rightHandOverlay->SetLocked(m_leftHandOverlay->IsInteracted());

        glm::vec3 l_pos = CSettingsManager::GetInstance()->GetOverlayOffset();
        l_pos.x *= -1.f;
        glm::mat4 l_overlayMat = glm::translate(g_identityMat4, l_pos);
        l_pos = CSettingsManager::GetInstance()->GetOverlayAngle();
        l_pos.y *= -1.f;
        l_pos.z *= -1.f;
        l_overlayMat *= glm::toMat4(glm::quat(glm::radians(l_pos)));
        l_overlayMat = CVRManager::GetInstance()->GetRightHandMatrix() * l_overlayMat;
        l_pos = l_overlayMat * g_pointVec4;
        glm::quat l_rot = glm::toQuat(l_overlayMat);
        m_rightHandOverlay->SetPosition(l_pos);
        m_rightHandOverlay->SetRotation(l_rot);
        m_rightHandOverlay->Update(CLeapManager::GetInstance()->GetLeftTipPosition());

        ProcessButtons(m_rightHandOverlay->GetButtons(), m_leftHandOverlay->GetPressure(), false);
    }
}

void COverlayManager::SetOverlaysWidth(float p_width)
{
    if(m_leftHandOverlay)
        m_leftHandOverlay->SetWidth(p_width);
    if(m_rightHandOverlay)
        m_rightHandOverlay->SetWidth(p_width);
}

void COverlayManager::SetOverlaysActive(bool p_state)
{
    if(m_leftHandOverlay)
    {
        m_leftHandOverlay->SetVisible(p_state);
        if(!p_state)
            m_leftHandOverlay->ResetInput();
    }
    if(m_rightHandOverlay)
    {
        m_rightHandOverlay->SetVisible(p_state);
        if(!p_state)
            m_rightHandOverlay->ResetInput();
    }
}

void COverlayManager::ProcessButtons(const std::vector<CButton*>& p_buttons, float p_pressure, bool p_left) const
{
    for(auto l_button : p_buttons)
    {
        if(l_button->IsUpdated())
        {
            std::string l_message;
            switch(l_button->GetType())
            {
                case CButton::BT_Button:
                    l_message.append("button ");
                    break;
                case CButton::BT_Axis:
                    l_message.append("axis ");
                    break;
            }

            l_message.append(l_button->GetName());
            l_message.push_back(' ');

            switch(l_button->GetState())
            {
                case CButton::BS_None:
                    l_message.append("none ");
                    break;
                case CButton::BS_Touched:
                    l_message.append("touched ");
                    break;
                case CButton::BS_Clicked:
                    l_message.append("clicked ");
                    break;
            }

            if(l_button->GetType() == CButton::BT_Axis)
            {
                const glm::vec2 &l_axis = l_button->GetAxis();
                l_message.append(std::to_string(l_axis.x));
                l_message.push_back(' ');
                l_message.append(std::to_string(l_axis.y));
                l_message.push_back(' ');
                l_message.append(std::to_string(p_pressure));
            }

            if(p_left)
                CVRManager::GetInstance()->SendLeftControllerMessage(l_message);
            else
                CVRManager::GetInstance()->SendRightControllerMessage(l_message);
        }
    }
}
