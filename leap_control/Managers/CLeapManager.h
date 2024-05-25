#pragma once

class CLeapPoller;
class CLeapManager
{
    static CLeapManager* ms_instance;

    CLeapPoller *m_leapPoller;
    LEAP_TRACKING_EVENT *m_trackingEvent;

    glm::vec3 m_leftTipPosition;
    glm::vec3 m_rightTipPosition;

    bool m_leftHandVisible;
    bool m_rightHandVisible;

    CLeapManager();
    CLeapManager(const CLeapManager &that) = delete;
    CLeapManager& operator=(const CLeapManager &that) = delete;
    ~CLeapManager();
public:
    static CLeapManager* GetInstance();

    bool Init();
    void Terminate();

    void Update();

    const glm::vec3& GetLeftTipPosition() const;
    const glm::vec3& GetRightTipPosition() const;

    bool IsLeftHandVisible() const;
    bool IsRightHandVisible() const;

    static void ConvertPosition(const LEAP_VECTOR & p_src, glm::vec3 & p_dst);
};
