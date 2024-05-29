#include "stdafx.h"
#include "Overlay/CHandOverlay.h"
#include "Overlay/CRenderTarget.h"
#include "Managers/COpenGLManager.h"
#include "Ui/vr_overlay.h"
#include "Utils/CButton.h"
#include "Utils/Utils.h"

enum ButtonName
{
    BN_A,
    BN_B,
    BN_System,
    BN_Thumbstick,
    BN_Touchpad
};

const glm::mat4 g_identityMat4(1.f);
const glm::vec2 g_zeroVec2(0.f);

const float g_touchPressure = 0.5f;
const float g_clickPressure = 0.75f;

CHandOverlay::CHandOverlay(bool p_left)
{
    m_renderTarget = nullptr;
    m_ui = nullptr;
    m_overlayHandle = vr::k_ulOverlayHandleInvalid;
    m_texture.eColorSpace = vr::EColorSpace::ColorSpace_Auto;
    m_texture.eType = vr::ETextureType::TextureType_OpenGL;
    m_texture.handle = nullptr;
    m_isLeft = p_left;
    m_width = 0.125f;
    m_alpha = 0.5f;
    m_position = glm::vec3(0.f);
    m_rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
    m_matrix = g_identityMat4;
    m_transformUpdate = false;
    m_pressure = 0.f;
    m_interacted = false;
    m_locked = false;
    m_visible = true;
}

bool CHandOverlay::Create()
{
    if(m_overlayHandle == vr::k_ulOverlayHandleInvalid)
    {
        m_ui = new vr_overlay();
        if(!m_isLeft)
            m_ui->Mirror();

        std::string l_name("leap_control.handOverlay.");
        l_name.append(m_isLeft ? "left" : "right");
        if(vr::VROverlay()->CreateOverlay(l_name.c_str(), "Hand overlay", &m_overlayHandle) == vr::EVROverlayError::VROverlayError_None)
        {
            vr::VROverlay()->SetOverlayWidthInMeters(m_overlayHandle, m_width);
            vr::VROverlay()->SetOverlayInputMethod(m_overlayHandle, vr::VROverlayInputMethod_None);
            vr::VROverlay()->SetOverlayAlpha(m_overlayHandle, m_alpha);
            vr::VROverlay()->ShowOverlay(m_overlayHandle);

            m_renderTarget = new CRenderTarget();
            m_renderTarget->Create(m_ui->size().width(), m_ui->size().height());
            m_renderTarget->AddWidget(m_ui);
            m_texture.handle = reinterpret_cast<void*>(static_cast<uintptr_t>(m_renderTarget->GetTextureID()));
        }

        m_buttons.push_back(new CButton(CButton::BT_Button, "a"));
        m_buttons.push_back(new CButton(CButton::BT_Button, "b"));
        m_buttons.push_back(new CButton(CButton::BT_Button, "system"));
        m_buttons.push_back(new CButton(CButton::BT_Axis, "thumbstick"));
        m_buttons.push_back(new CButton(CButton::BT_Axis, "touchpad"));
    }

    return (m_overlayHandle != vr::k_ulOverlayHandleInvalid);
}

void CHandOverlay::Destroy()
{
    if(m_overlayHandle != vr::k_ulOverlayHandleInvalid)
    {
        vr::VROverlay()->DestroyOverlay(m_overlayHandle);
        m_overlayHandle = vr::k_ulOverlayHandleInvalid;
    }

    if(m_renderTarget)
    {
        m_renderTarget->Destroy();
        m_renderTarget = nullptr;
    }

    //delete m_ui;
    m_ui = nullptr;

    m_texture.handle = nullptr;

    for(auto l_button : m_buttons)
        delete l_button;
    m_buttons.clear();
}

void CHandOverlay::SetPosition(const glm::vec3 & p_pos)
{
    m_position = p_pos;
    m_transformUpdate = true;
}

void CHandOverlay::SetRotation(const glm::quat &p_rot)
{
    m_rotation = p_rot;
    m_transformUpdate = true;
}

bool CHandOverlay::IsLocked() const
{
    return m_locked;
}

void CHandOverlay::SetLocked(bool p_state)
{
    m_locked = p_state;
}

bool CHandOverlay::IsInteracted() const
{
    return m_interacted;
}

void CHandOverlay::SetVisible(bool p_state)
{
    m_visible = p_state;
}

void CHandOverlay::SetWidth(float p_width)
{
    m_width = glm::clamp(p_width, 0.1f, 0.5f);

    if(m_overlayHandle != vr::k_ulOverlayHandleInvalid)
        vr::VROverlay()->SetOverlayWidthInMeters(m_overlayHandle, m_width);
}

void CHandOverlay::ResetInput()
{
    if(!m_buttons.empty())
    {
        m_buttons[ButtonName::BN_A]->SetState(CButton::BS_None);
        m_buttons[ButtonName::BN_B]->SetState(CButton::BS_None);
        m_buttons[ButtonName::BN_System]->SetState(CButton::BS_None);
        m_buttons[ButtonName::BN_Thumbstick]->SetState(CButton::BS_None);
        m_buttons[ButtonName::BN_Thumbstick]->SetAxis(g_zeroVec2);
        m_buttons[ButtonName::BN_Touchpad]->SetState(CButton::BS_None);
        m_buttons[ButtonName::BN_Touchpad]->SetAxis(g_zeroVec2);
    }
}

const std::vector<CButton*>& CHandOverlay::GetButtons() const
{
    return m_buttons;
}

float CHandOverlay::GetPressure() const
{
    return m_pressure;
}

void CHandOverlay::Update(const glm::vec3 &p_cursor)
{
    if(m_overlayHandle != vr::k_ulOverlayHandleInvalid)
    {
        if(m_transformUpdate)
        {
            m_matrix = glm::translate(g_identityMat4, m_position) * glm::toMat4(m_rotation);
            ConvertMatrix(m_matrix, m_vrMatrix);
            vr::VROverlay()->SetOverlayTransformAbsolute(m_overlayHandle, vr::TrackingUniverseRawAndUncalibrated, &m_vrMatrix);
            m_transformUpdate = false;
        }

        if(!m_locked && m_visible)
        {
            for(auto l_button : m_buttons)
                l_button->ResetUpdate();

            glm::vec3 l_localPos = glm::inverse(m_matrix) * glm::vec4(p_cursor, 1.f);
            float l_halfWidth = m_width * 0.5f;
            if(IsInRange(l_localPos.x, -l_halfWidth, l_halfWidth) && IsInRange(l_localPos.y, -l_halfWidth, l_halfWidth) && IsInRange(l_localPos.z, -0.025f, l_halfWidth))
            {
                m_interacted = true;
                glm::vec2 l_cursorPos;
                l_cursorPos.x = (l_localPos.x + l_halfWidth) / m_width * static_cast<float>(m_ui->size().width());
                l_cursorPos.y = (-l_localPos.y + l_halfWidth) / m_width * static_cast<float>(m_ui->size().height());

                if(l_localPos.z < 0.f)
                    l_localPos.z = 0.f;
                m_pressure = 1.f - glm::clamp(l_localPos.z / (m_width * 0.25f), 0.f, 1.f);

                m_ui->Update(l_cursorPos, m_pressure);
            }
            else
                ResetInteraction();
        }
        else
            ResetInteraction();

        m_alpha = glm::mix(m_alpha, ((m_locked || !m_visible) ? 0.f : (((m_pressure > 0.f) ? 1.f : 0.5f))), 0.25f);

        if(m_alpha > 0.f)
        {
            m_renderTarget->Update();
            vr::VROverlay()->SetOverlayTexture(m_overlayHandle, &m_texture);
        }
        vr::VROverlay()->SetOverlayAlpha(m_overlayHandle, m_alpha);

        // Buttons
        if(m_ui->IsOnButtonA())
            m_buttons[ButtonName::BN_A]->SetState((m_pressure > g_clickPressure) ? CButton::BS_Clicked : ((m_pressure > g_touchPressure) ? CButton::BS_Touched : CButton::BS_None));
        else
            m_buttons[ButtonName::BN_A]->SetState(CButton::BS_None);

        if(m_ui->IsOnButtonB())
            m_buttons[ButtonName::BN_B]->SetState((m_pressure > g_clickPressure) ? CButton::BS_Clicked : ((m_pressure > g_touchPressure) ? CButton::BS_Touched : CButton::BS_None));
        else
            m_buttons[ButtonName::BN_B]->SetState(CButton::BS_None);

        if(m_ui->IsOnButtonSys())
            m_buttons[ButtonName::BN_System]->SetState((m_pressure > g_clickPressure) ? CButton::BS_Clicked : ((m_pressure > g_touchPressure) ? CButton::BS_Touched : CButton::BS_None));
        else
            m_buttons[ButtonName::BN_System]->SetState(CButton::BS_None);

        if(m_ui->IsOnThumbstick() && (m_pressure > g_touchPressure))
        {
            m_buttons[ButtonName::BN_Thumbstick]->SetState((m_pressure > g_clickPressure) ? CButton::BS_Clicked : CButton::BS_Touched);
            m_buttons[ButtonName::BN_Thumbstick]->SetAxis(m_ui->GetThumbstickAxis());
        }
        else
        {
            m_buttons[ButtonName::BN_Thumbstick]->SetState(CButton::BS_None);
            m_buttons[ButtonName::BN_Thumbstick]->SetAxis(g_zeroVec2);
        }

        if(m_ui->IsOnTouchpad() && (m_pressure > g_touchPressure))
        {
            m_buttons[ButtonName::BN_Touchpad]->SetState((m_pressure > g_clickPressure) ? CButton::BS_Clicked : CButton::BS_Touched);
            m_buttons[ButtonName::BN_Touchpad]->SetAxis(m_ui->GetTouchpadAxis());
        }
        else
        {
            m_buttons[ButtonName::BN_Touchpad]->SetState(CButton::BS_None);
            m_buttons[ButtonName::BN_Touchpad]->SetAxis(g_zeroVec2);
        }
    }
}

void CHandOverlay::ResetInteraction()
{
    m_pressure = 0.f;
    m_interacted = false;
}
