#pragma once

class CLeapPoller;
class CLeapManager
{
    static CLeapManager* ms_instance;

    CLeapPoller *m_leapPoller;
    LEAP_TRACKING_EVENT *m_trackingEvent;

    glm::vec3 m_leftIndexTipPosition;
    glm::vec3 m_rightIndexTipPosition;

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

    const glm::vec3& GetLeftIndexTipPosition() const;
    const glm::vec3& GetRightIndexTipPosition() const;

    bool IsLeftHandVisible() const;
    bool IsRightHandVisible() const;

    static void ConvertPosition(const LEAP_VECTOR & p_src, glm::vec3 & p_dst);
};
