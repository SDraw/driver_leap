#include "stdafx.h"
#include "Overlay/CCursorOverlay.h"
#include "Utils/Utils.h"

const glm::mat4 g_identityMat4(1.f);

CCursorOverlay::CCursorOverlay(bool p_left)
{
    m_overlayHandle = vr::k_ulOverlayHandleInvalid;
    m_isLeft = p_left;
    m_position = glm::vec3(0.f);
    m_rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
    m_transformUpdate = false;
}

bool CCursorOverlay::Create()
{
    if(m_overlayHandle == vr::k_ulOverlayHandleInvalid)
    {
        std::string l_name("leap_control.cursor.");
        l_name.append(m_isLeft ? "left" : "right");
        if(vr::VROverlay()->CreateOverlay(l_name.c_str(), "Finger overlay", &m_overlayHandle) == vr::EVROverlayError::VROverlayError_None)
        {
            std::string l_path(QApplication::applicationDirPath().toStdString());
            l_path.append("/../../../resources/textures/cursor.png");

            vr::VROverlay()->SetOverlayFromFile(m_overlayHandle, l_path.c_str());
            vr::VROverlay()->SetOverlayColor(m_overlayHandle, 0.f, 1.f, 0.f);
            vr::VROverlay()->SetOverlayWidthInMeters(m_overlayHandle, 0.006625f);
            vr::VROverlay()->SetOverlaySortOrder(m_overlayHandle, 1);
            vr::VROverlay()->ShowOverlay(m_overlayHandle);
        }
    }

    return (m_overlayHandle != vr::k_ulOverlayHandleInvalid);
}

void CCursorOverlay::Destroy()
{
    if(m_overlayHandle != vr::k_ulOverlayHandleInvalid)
    {
        vr::VROverlay()->DestroyOverlay(m_overlayHandle);
        m_overlayHandle = vr::k_ulOverlayHandleInvalid;
    }
}

void CCursorOverlay::SetPosition(const glm::vec3 & p_pos)
{
    m_position = p_pos;
    m_transformUpdate = true;
}

void CCursorOverlay::SetRotation(const glm::quat &p_rot)
{
    m_rotation = p_rot;
    m_transformUpdate = true;
}

void CCursorOverlay::Update()
{
    if(m_overlayHandle != vr::k_ulOverlayHandleInvalid)
    {
        if(m_transformUpdate)
        {
            glm::mat4 l_matrix = glm::translate(g_identityMat4, m_position) * glm::toMat4(m_rotation);
            ConvertMatrix(l_matrix, m_matrix);
            vr::VROverlay()->SetOverlayTransformAbsolute(m_overlayHandle, vr::TrackingUniverseRawAndUncalibrated, &m_matrix);
            m_transformUpdate = false;
        }
    }
}
