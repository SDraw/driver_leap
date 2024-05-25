#pragma once

class CHandOverlay;
class CCursorOverlay;
class CButton;
class COverlayManager
{
    static COverlayManager* ms_instance;

    CHandOverlay *m_leftHandOverlay;
    CHandOverlay *m_rightHandOverlay;

    CCursorOverlay *m_leftCursorOverlay;
    CCursorOverlay *m_rightCursorOverlay;

    COverlayManager();
    COverlayManager(const COverlayManager &that) = delete;
    COverlayManager& operator=(const COverlayManager &that) = delete;
    ~COverlayManager() = default;

    void ProcessButtons(const std::vector<CButton*> &p_buttons, bool p_left) const;
public:
    static COverlayManager* GetInstance();

    void CreateOverlays();
    void DestroyOverlays();

    void Update();

    void SetOverlaysWidth(float p_width);
    void SetOverlaysActive(bool p_state);
};

