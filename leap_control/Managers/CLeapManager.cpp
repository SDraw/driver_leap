#include "stdafx.h"
#include "Managers/CLeapManager.h"
#include "Managers/CSettingsManager.h"
#include "Managers/CVRManager.h"
#include "Leap/CLeapPoller.h"

const glm::mat4 g_identityMat4(1.f);
const glm::vec4 g_pointVec4(0.f, 0.f, 0.f, 1.f);

CLeapManager* CLeapManager::ms_instance = nullptr;

CLeapManager* CLeapManager::GetInstance()
{
    if(!ms_instance)
        ms_instance = new CLeapManager();

    return ms_instance;
}

CLeapManager::CLeapManager()
{
    m_leapPoller = nullptr;
    m_trackingEvent = new LEAP_TRACKING_EVENT();
    m_leftIndexTipPosition = glm::vec3(0.f);
    m_leftThumbTipPosition = glm::vec3(0.f);
    m_rightIndexTipPosition = glm::vec3(0.f);
    m_rightThumbTipPosition = glm::vec3(0.f);
    m_leftHandVisible = false;
    m_rightHandVisible = false;
}
CLeapManager::~CLeapManager()
{
    delete m_trackingEvent;
}

bool CLeapManager::Init()
{
    if(!m_leapPoller)
    {
        m_leapPoller = new CLeapPoller();
        m_leapPoller->Start();
    }
    return (m_leapPoller != nullptr);
}

void CLeapManager::Terminate()
{
    if(m_leapPoller)
    {
        m_leapPoller->Stop();
        delete m_leapPoller;
        m_leapPoller = nullptr;
    }
}

void CLeapManager::Update()
{
    if(m_leapPoller && m_leapPoller->GetFrame(m_trackingEvent))
    {
        m_leftHandVisible = false;
        m_rightHandVisible = false;

        for(uint32_t i = 0U; i < m_trackingEvent->nHands; i++)
        {
            switch(m_trackingEvent->pHands[i].type)
            {
                case eLeapHandType_Left:
                {
                    if(!m_leftHandVisible)
                    {
                        ConvertPosition(m_trackingEvent->pHands[i].index.distal.next_joint, m_leftIndexTipPosition);
                        ConvertPosition(m_trackingEvent->pHands[i].thumb.distal.next_joint, m_leftThumbTipPosition);
                        m_leftHandVisible = true;
                    }
                } break;
                case eLeapHandType_Right:
                {
                    if(!m_rightHandVisible)
                    {
                        ConvertPosition(m_trackingEvent->pHands[i].index.distal.next_joint, m_rightIndexTipPosition);
                        ConvertPosition(m_trackingEvent->pHands[i].thumb.distal.next_joint, m_rightThumbTipPosition);
                        m_rightHandVisible = true;
                    }
                } break;
            }
        }

        if(m_leftHandVisible || m_rightHandVisible)
        {
            glm::mat4 l_rootMat = glm::translate(g_identityMat4, CSettingsManager::GetInstance()->GetRootOffset());
            l_rootMat *= glm::toMat4(glm::quat(glm::radians(CSettingsManager::GetInstance()->GetRootAngle())));
            l_rootMat = CVRManager::GetInstance()->GetHmdMatrix() * l_rootMat;

            if(m_leftHandVisible)
            {
                glm::mat4 l_tipMat = l_rootMat * glm::translate(g_identityMat4, m_leftIndexTipPosition);
                m_leftIndexTipPosition = l_tipMat * g_pointVec4;
            }
            if(m_rightHandVisible)
            {
                glm::mat4 l_tipMat = l_rootMat * glm::translate(g_identityMat4, m_rightIndexTipPosition);
                m_rightIndexTipPosition = l_tipMat * g_pointVec4;
            }
        }
    }
}

const glm::vec3 & CLeapManager::GetLeftIndexTipPosition() const
{
    return m_leftIndexTipPosition;
}
const glm::vec3& CLeapManager::GetLeftThumbTipPosition() const
{
    return m_leftThumbTipPosition;
}
const glm::vec3 & CLeapManager::GetRightIndexTipPosition() const
{
    return m_rightIndexTipPosition;
}

const glm::vec3& CLeapManager::GetRightThumbTipPosition() const
{
    return m_rightThumbTipPosition;
}

bool CLeapManager::IsLeftHandVisible() const
{
    return m_leftHandVisible;
}

bool CLeapManager::IsRightHandVisible() const
{
    return m_rightHandVisible;
}

void CLeapManager::ConvertPosition(const LEAP_VECTOR & p_src, glm::vec3 & p_dst)
{
    // In desktop mode: +X - right, +Y - up, -Z - forward (as OpenGL)
    // In HMD mode: -X - right, +Y - forward, -Z - up (same basis, just rotated)
    p_dst.x = -0.001f * p_src.x;
    p_dst.y = -0.001f * p_src.z;
    p_dst.z = -0.001f * p_src.y;
}
