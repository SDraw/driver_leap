#pragma once

#include <QWidget>
#include "ui_vr_overlay.h"

class vr_overlay : public QWidget
{
    Q_OBJECT

public:
    vr_overlay(QWidget *parent = nullptr);
    ~vr_overlay() = default;

    void Mirror();

    void Update(const glm::vec2 &p_pos, float p_pressure);

    bool IsOnThumbstick() const;
    bool IsOnTouchpad() const;
    bool IsOnButtonA() const;
    bool IsOnButtonB() const;
    bool IsOnButtonSys() const;

    const glm::vec2& GetThumbstickAxis() const;
    const glm::vec2& GetTouchpadAxis() const;
private:
    Ui::vr_overlayClass ui;

    bool m_onThumbstick;
    glm::vec2 m_thumbstickAxis;

    bool m_onTouchpad;
    glm::vec2 m_touchpadAxis;

    bool m_onButtonA;
    bool m_onButtonB;
    bool m_onButtonS;

    static void Mirror(QWidget *m_frame, int p_mainWidth);
};
