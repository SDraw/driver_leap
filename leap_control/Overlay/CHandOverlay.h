#pragma once

class CRenderTarget;
class vr_overlay;
class CButton;
class CHandOverlay
{
    vr::VROverlayHandle_t m_overlayHandle;
    vr::Texture_t m_texture;

    bool m_isLeft;
    CRenderTarget *m_renderTarget;
    vr_overlay *m_ui;

    float m_width;
    float m_alpha;
    glm::vec3 m_position;
    glm::quat m_rotation;
    glm::mat4 m_matrix;
    vr::HmdMatrix34_t m_vrMatrix;
    bool m_transformUpdate;

    float m_pressure;
    bool m_interacted;
    bool m_locked;
    bool m_visible;

    std::vector<CButton*> m_buttons;

    CHandOverlay(const CHandOverlay &that) = delete;
    CHandOverlay& operator=(const CHandOverlay &that) = delete;

    void ResetInteraction();
public:
    explicit CHandOverlay(bool p_left);
    ~CHandOverlay() = default;

    bool Create();
    void Destroy();

    void SetPosition(const glm::vec3 &p_pos);
    void SetRotation(const glm::quat &p_rot);

    bool IsLocked() const;
    void SetLocked(bool p_state);

    bool IsInteracted() const;
    void SetVisible(bool p_state);

    void SetWidth(float p_width);
    void ResetInput();

    const std::vector<CButton*>& GetButtons() const;
    float GetPressure() const;

    void Update(const glm::vec3 &p_cursor);
};

