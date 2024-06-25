#pragma once

#include <QtWidgets/QWidget>
#include "ui_leap_control.h"

class leap_control : public QWidget
{
    Q_OBJECT
public:
    leap_control(QWidget *parent = nullptr);
    ~leap_control() = default;
private:
    Ui::leap_controlClass ui;
    QSystemTrayIcon m_trayIcon;

    // General
    void OnTrackingLevelChange(int p_item);
    void OnHandsResetChange(int p_state);
    void OnUseVelocityChange(int p_state);
    void OnDashboardSmoothChanged(int p_value);
    void OnDashboardSmoothReset(bool p_checked);
    void OnStartMinimizedChanged(int p_state);

    // Input
    void OnUseTriggerGripChange(int p_state);
    void OnTriggerModeChange(int p_item);
    void OnTriggerThresholdChange(int p_value);
    void OnTriggerThresholdReset(bool p_checked);
    void OnGripThresholdChange(int p_value);
    void OnGripThresholdReset(bool p_checked);
    void OnPinchRangeMinChange(int p_value);
    void OnPinchRangeMaxChange(int p_value);
    void OnPinchRangeReset(bool p_checked);
    void OnUseControllerInputChange(int p_state);

    // Root
    void OnRootOffsetXChanged(int p_value);
    void OnRootOffsetYChanged(int p_value);
    void OnRootOffsetZChanged(int p_value);
    void OnRootOffsetReset(bool p_checked);

    void OnRootAngleXChanged(int p_value);
    void OnRootAngleYChanged(int p_value);
    void OnRootAngleZChanged(int p_value);
    void OnRootAngleReset(bool p_checked);

    // Overlays
    void OnShowOverlaysChange(int p_state);

    void OnOverlaySizeChange(int p_value);
    void OnOverlaySizeReset(bool p_checked);

    void OnOverlayOffsetXChanged(int p_value);
    void OnOverlayOffsetYChanged(int p_value);
    void OnOverlayOffsetZChanged(int p_value);
    void OnOverlayOffsetReset(bool p_checked);

    void OnOverlayAngleXChanged(int p_value);
    void OnOverlayAngleYChanged(int p_value);
    void OnOverlayAngleZChanged(int p_value);
    void OnOverlayAngleReset(bool p_checked);

    // Utils
    void OnSave(bool p_checked);

    void hideEvent(QHideEvent *p_event);
    void OnTrayClick(QSystemTrayIcon::ActivationReason p_reason);
};
