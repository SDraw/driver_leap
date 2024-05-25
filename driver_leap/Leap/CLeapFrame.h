#pragma once

class CLeapHand;
class CLeapFrame
{
    LEAP_TRACKING_EVENT m_event;
    CLeapHand *m_leftHand;
    CLeapHand *m_rightHand;
    int64_t m_lastFrameId;
public:
    CLeapFrame();
    ~CLeapFrame();

    LEAP_TRACKING_EVENT* GetEvent();
    const CLeapHand* GetLeftHand() const;
    const CLeapHand* GetRightHand() const;

    void Update();
};

