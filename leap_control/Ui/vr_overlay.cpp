#include "stdafx.h"
#include "Ui/vr_overlay.h"

vr_overlay::vr_overlay(QWidget *parent) : QWidget(parent)
{
    ui.setupUi(this);

    m_onThumbstick = false;
    m_onTouchpad = false;
    m_onButtonA = false;
    m_onButtonB = false;
    m_onButtonS = false;
}

void vr_overlay::Mirror()
{
    int l_mainWidth = this->width();
    Mirror(ui.m_thumbstickArea, l_mainWidth);
    Mirror(ui.m_touchpadArea, l_mainWidth);
    Mirror(ui.m_buttonA, l_mainWidth);
    Mirror(ui.m_buttonB, l_mainWidth);
    Mirror(ui.m_buttonSys, l_mainWidth);
    Mirror(ui.m_pressureArea, l_mainWidth);
}

void vr_overlay::Update(const glm::vec2 & p_pos, float p_pressure)
{
    auto l_cursorSize = ui.m_cursor->size();
    ui.m_cursor->move(p_pos.x - l_cursorSize.width() / 2, p_pos.y - l_cursorSize.height() / 2);

    // Check for areas with axes
    m_onThumbstick = false;
    QRect l_rect(ui.m_thumbstickArea->pos(), ui.m_thumbstickArea->size()); // ui.m_thumbstickArea->rect() returns only size with zero position, no bueno
    m_onThumbstick = l_rect.contains(p_pos.x, p_pos.y);
    if(m_onThumbstick)
    {
        auto l_pos = ui.m_thumbstickArea->pos();
        auto l_size = ui.m_thumbstickArea->size();
        l_pos.setX(p_pos.x - l_pos.x());
        l_pos.setY(p_pos.y - l_pos.y());
        m_thumbstickAxis.x = ((static_cast<float>(l_pos.x()) / static_cast<float>(l_size.width())) *2.f) - 1.0f;
        m_thumbstickAxis.y = -(((static_cast<float>(l_pos.y()) / static_cast<float>(l_size.height())) *2.f) - 1.0f);

        l_size = ui.m_thumbstickCursor->size();
        l_pos.setX(l_pos.x() - l_size.width() / 2);
        l_pos.setY(l_pos.y() - l_size.height() / 2);
        ui.m_thumbstickCursor->move(l_pos);
    }

    l_rect.moveTo(ui.m_touchpadArea->pos());
    l_rect.setSize(ui.m_touchpadArea->size());
    m_onTouchpad = l_rect.contains(p_pos.x, p_pos.y);
    if(m_onTouchpad)
    {
        auto l_pos = ui.m_touchpadArea->pos();
        auto l_size = ui.m_touchpadArea->size();
        l_pos.setX(p_pos.x - l_pos.x());
        l_pos.setY(p_pos.y - l_pos.y());
        m_touchpadAxis.x = ((static_cast<float>(l_pos.x()) / static_cast<float>(l_size.width())) *2.f) - 1.0f;
        m_touchpadAxis.y = -(((static_cast<float>(l_pos.y()) / static_cast<float>(l_size.height())) *2.f) - 1.0f);

        l_size = ui.m_touchpadCursor->size();
        l_pos.setX(l_pos.x() - l_size.width() / 2);
        l_pos.setY(l_pos.y() - l_size.height() / 2);
        ui.m_touchpadCursor->move(l_pos);
    }

    l_rect.moveTo(ui.m_buttonA->pos());
    l_rect.setSize(ui.m_buttonA->size());
    m_onButtonA = l_rect.contains(p_pos.x, p_pos.y);
    if(m_onButtonA)
        ui.m_buttonA->setStyleSheet(QString("QFrame{background-color:#%1;border-radius:10px;}").arg((p_pressure > 0.75f) ? "ffa500" : ((p_pressure > 0.5f) ? "00ff00" : "23262e")));
    else
        ui.m_buttonA->setStyleSheet("QFrame{background-color:#23262e;border-radius:10px;}");

    l_rect.moveTo(ui.m_buttonB->pos());
    l_rect.setSize(ui.m_buttonB->size());
    m_onButtonB = l_rect.contains(p_pos.x, p_pos.y);
    if(m_onButtonB)
        ui.m_buttonB->setStyleSheet(QString("QFrame{background-color:#%1;border-radius:10px;}").arg((p_pressure > 0.75f) ? "ffa500" : ((p_pressure > 0.5f) ? "00ff00" : "23262e")));
    else
        ui.m_buttonB->setStyleSheet("QFrame{background-color:#23262e;border-radius:10px;}");

    l_rect.moveTo(ui.m_buttonSys->pos());
    l_rect.setSize(ui.m_buttonSys->size());
    m_onButtonS = l_rect.contains(p_pos.x, p_pos.y);
    if(m_onButtonS)
        ui.m_buttonSys->setStyleSheet(QString("QFrame{background-color:#%1;border-radius:10px;}").arg((p_pressure > 0.75f) ? "ffa500" : ((p_pressure > 0.5f) ? "00ff00" : "23262e")));
    else
        ui.m_buttonSys->setStyleSheet("QFrame{background-color:#23262e;border-radius:10px;}");

    ui.m_pressureRectangle->resize(10, static_cast<int>(p_pressure * 320.f));
    ui.m_pressureRectangle->setStyleSheet(QString("QFrame{background-color:#%1;border-radius:2px;}").arg((p_pressure > 0.75f) ? "ffa500" : ((p_pressure > 0.5f) ? "00ff00" : "3399ff")));
}

bool vr_overlay::IsOnThumbstick() const
{
    return m_onThumbstick;
}

bool vr_overlay::IsOnTouchpad() const
{
    return m_onTouchpad;
}

bool vr_overlay::IsOnButtonA() const
{
    return m_onButtonA;
}

bool vr_overlay::IsOnButtonB() const
{
    return m_onButtonB;
}

bool vr_overlay::IsOnButtonSys() const
{
    return m_onButtonS;
}

const glm::vec2 & vr_overlay::GetThumbstickAxis() const
{
    return m_thumbstickAxis;
}

const glm::vec2 & vr_overlay::GetTouchpadAxis() const
{
    return m_touchpadAxis;
}

void vr_overlay::Mirror(QWidget *p_frame, int p_mainWidth)
{
    QPoint l_pos = p_frame->pos();
    QSize l_size = p_frame->size();
    int l_newX = p_mainWidth - (l_pos.x() + l_size.width());
    l_pos.setX(l_newX);
    p_frame->move(l_pos);
}
