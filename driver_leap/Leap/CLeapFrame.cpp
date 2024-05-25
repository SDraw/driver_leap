#include "stdafx.h"
#include "Leap/CLeapFrame.h"
#include "Leap/CLeapHand.h"

CLeapFrame::CLeapFrame()
{
    m_leftHand = new CLeapHand(true);
    m_rightHand = new CLeapHand(false);
    m_lastFrameId = 0U;
}

CLeapFrame::~CLeapFrame()
{
    delete m_leftHand;
    delete m_rightHand;
}

LEAP_TRACKING_EVENT* CLeapFrame::GetEvent()
{
    return &m_event;
}

const CLeapHand* CLeapFrame::GetLeftHand() const
{
    return m_leftHand;
}

const CLeapHand* CLeapFrame::GetRightHand() const
{
    return m_rightHand;
}

void CLeapFrame::Update()
{
    if(m_lastFrameId != m_event.tracking_frame_id)
    {
        bool l_skipLeft = false;
        bool l_skipRight = false;

        for(uint32_t i = 0U; i < m_event.nHands; i++)
        {
            switch(m_event.pHands[i].type)
            {
                case eLeapHandType_Left:
                {
                    if(!l_skipLeft)
                    {
                        m_leftHand->Update(m_event.pHands[i]);
                        l_skipLeft = true;
                    }
                } break;
                case eLeapHandType_Right:
                {
                    if(!l_skipRight)
                    {
                        m_rightHand->Update(m_event.pHands[i]);
                        l_skipRight = true;
                    }
                } break;
            }
        }

        if(!l_skipLeft)
            m_leftHand->Update();
        if(!l_skipRight)
            m_rightHand->Update();

        m_lastFrameId = m_event.tracking_frame_id;
    }
}
