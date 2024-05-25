#pragma once

class CCursorOverlay
{
    vr::VROverlayHandle_t m_overlayHandle;

    bool m_isLeft;

    glm::vec3 m_position;
    glm::quat m_rotation;
    vr::HmdMatrix34_t m_matrix;
    bool m_transformUpdate;

    CCursorOverlay(const CCursorOverlay &that) = delete;
    CCursorOverlay& operator=(const CCursorOverlay &that) = delete;
public:
    explicit CCursorOverlay(bool p_left);
    ~CCursorOverlay() = default;

    bool Create();
    void Destroy();

    void SetPosition(const glm::vec3 &p_pos);
    void SetRotation(const glm::quat &p_rot);

    void Update();
};

