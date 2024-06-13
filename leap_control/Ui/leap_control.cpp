#include "stdafx.h"
#include "Ui/leap_control.h"
#include "Managers/CSettingsManager.h"
#include "Managers/COverlayManager.h"
#include "Managers/CVRManager.h"
#include "Utils/Utils.h"

leap_control::leap_control(QWidget *parent) : QWidget(parent)
{
    ui.setupUi(this);
    this->setWindowFlag(Qt::WindowMaximizeButtonHint, false);

    // General
    ui.m_trackingLevelComboBox->setCurrentIndex(CSettingsManager::GetInstance()->GetTrackingLevel());
    connect(ui.m_trackingLevelComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &leap_control::OnTrackingLevelChange);

    ui.m_handsResetCheckBox->setChecked(CSettingsManager::GetInstance()->GetHandsReset());
    connect(ui.m_handsResetCheckBox, &QCheckBox::stateChanged, this, &leap_control::OnHandsResetChange);

    ui.m_useVelocityCheckBox->setChecked(CSettingsManager::GetInstance()->GetUseVelocity());
    connect(ui.m_useVelocityCheckBox, &QCheckBox::stateChanged, this, &leap_control::OnUseVelocityChange);

    ui.m_startMinimizedCheckBox->setChecked(CSettingsManager::GetInstance()->GetStartMinimized());
    connect(ui.m_startMinimizedCheckBox, &QCheckBox::stateChanged, this, &leap_control::OnStartMinimizedChanged);

    // Input
    ui.m_useTriggerGripCheckBox->setChecked(CSettingsManager::GetInstance()->GetUseTriggerGrip());
    connect(ui.m_useTriggerGripCheckBox, &QCheckBox::stateChanged, this, &leap_control::OnUseTriggerGripChange);

    ui.m_triggerModeComboBox->setCurrentIndex(CSettingsManager::GetInstance()->GetTriggerMode());
    connect(ui.m_triggerModeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &leap_control::OnTriggerModeChange);

    ui.m_useControllerInputCheckBox->setChecked(CSettingsManager::GetInstance()->GetUseControllerInput());
    connect(ui.m_useControllerInputCheckBox, &QCheckBox::stateChanged, this, &leap_control::OnUseControllerInputChange);

    // Root
    ui.m_rootOffsetSliderX->setValue(InvProgressLerp(CSettingsManager::GetInstance()->GetRootOffset().x, -1.f, 1.f));
    ui.m_rootOffsetSliderX->setToolTip(QString("X: %1").arg(CSettingsManager::GetInstance()->GetRootOffset().x));
    connect(ui.m_rootOffsetSliderX, &QSlider::valueChanged, this, &leap_control::OnRootOffsetXChanged);

    ui.m_rootOffsetSliderY->setValue(InvProgressLerp(CSettingsManager::GetInstance()->GetRootOffset().y, -1.f, 1.f));
    ui.m_rootOffsetSliderY->setToolTip(QString("Y: %1").arg(CSettingsManager::GetInstance()->GetRootOffset().y));
    connect(ui.m_rootOffsetSliderY, &QSlider::valueChanged, this, &leap_control::OnRootOffsetYChanged);

    ui.m_rootOffsetSliderZ->setValue(InvProgressLerp(CSettingsManager::GetInstance()->GetRootOffset().z, -1.f, 1.f));
    ui.m_rootOffsetSliderZ->setToolTip(QString("Z: %1").arg(CSettingsManager::GetInstance()->GetRootOffset().z));
    connect(ui.m_rootOffsetSliderZ, &QSlider::valueChanged, this, &leap_control::OnRootOffsetZChanged);

    ui.m_rootAngleSliderX->setValue(InvProgressLerp(CSettingsManager::GetInstance()->GetRootAngle().x, -180.f, 180.f));
    ui.m_rootAngleSliderX->setToolTip(QString("X: %1").arg(CSettingsManager::GetInstance()->GetRootAngle().x));
    connect(ui.m_rootAngleSliderX, &QSlider::valueChanged, this, &leap_control::OnRootAngleXChanged);

    ui.m_rootAngleSliderY->setValue(InvProgressLerp(CSettingsManager::GetInstance()->GetRootAngle().y, -180.f, 180.f));
    ui.m_rootAngleSliderY->setToolTip(QString("Y: %1").arg(CSettingsManager::GetInstance()->GetRootAngle().y));
    connect(ui.m_rootAngleSliderY, &QSlider::valueChanged, this, &leap_control::OnRootAngleYChanged);

    ui.m_rootAngleSliderZ->setValue(InvProgressLerp(CSettingsManager::GetInstance()->GetRootAngle().z, -180.f, 180.f));
    ui.m_rootAngleSliderZ->setToolTip(QString("Z: %1").arg(CSettingsManager::GetInstance()->GetRootAngle().z));
    connect(ui.m_rootAngleSliderZ, &QSlider::valueChanged, this, &leap_control::OnRootAngleZChanged);

    // Overlays
    ui.m_showOverlaysCheckBox->setChecked(CSettingsManager::GetInstance()->GetShowOverlays());
    connect(ui.m_showOverlaysCheckBox, &QCheckBox::stateChanged, this, &leap_control::OnShowOverlaysChange);

    ui.m_overlaySizeSlider->setValue(InvProgressLerp(CSettingsManager::GetInstance()->GetOverlaySize(), 0.1f, 0.5f));
    ui.m_overlaySizeSlider->setToolTip(QString("%1").arg(CSettingsManager::GetInstance()->GetOverlaySize()));
    connect(ui.m_overlaySizeSlider, &QSlider::valueChanged, this, &leap_control::OnOverlaySizeChange);

    ui.m_overlayOffsetSliderX->setValue(InvProgressLerp(CSettingsManager::GetInstance()->GetOverlayOffset().x, -0.5f, 0.5f));
    ui.m_overlayOffsetSliderX->setToolTip(QString("X: %1").arg(CSettingsManager::GetInstance()->GetOverlayOffset().x));
    connect(ui.m_overlayOffsetSliderX, &QSlider::valueChanged, this, &leap_control::OnOverlayOffsetXChanged);

    ui.m_overlayOffsetSliderY->setValue(InvProgressLerp(CSettingsManager::GetInstance()->GetOverlayOffset().y, -0.5f, 0.5f));
    ui.m_overlayOffsetSliderY->setToolTip(QString("Y: %1").arg(CSettingsManager::GetInstance()->GetOverlayOffset().y));
    connect(ui.m_overlayOffsetSliderY, &QSlider::valueChanged, this, &leap_control::OnOverlayOffsetYChanged);

    ui.m_overlayOffsetSliderZ->setValue(InvProgressLerp(CSettingsManager::GetInstance()->GetOverlayOffset().z, -0.5f, 0.5f));
    ui.m_overlayOffsetSliderZ->setToolTip(QString("Z: %1").arg(CSettingsManager::GetInstance()->GetOverlayOffset().z));
    connect(ui.m_overlayOffsetSliderZ, &QSlider::valueChanged, this, &leap_control::OnOverlayOffsetZChanged);

    ui.m_overlayAngleSliderX->setValue(InvProgressLerp(CSettingsManager::GetInstance()->GetOverlayAngle().x, -180.f, 180.f));
    ui.m_overlayAngleSliderX->setToolTip(QString("X: %1").arg(CSettingsManager::GetInstance()->GetOverlayAngle().x));
    connect(ui.m_overlayAngleSliderX, &QSlider::valueChanged, this, &leap_control::OnOverlayAngleXChanged);

    ui.m_overlayAngleSliderY->setValue(InvProgressLerp(CSettingsManager::GetInstance()->GetOverlayAngle().y, -180.f, 180.f));
    ui.m_overlayAngleSliderY->setToolTip(QString("Y: %1").arg(CSettingsManager::GetInstance()->GetOverlayAngle().y));
    connect(ui.m_overlayAngleSliderY, &QSlider::valueChanged, this, &leap_control::OnOverlayAngleYChanged);

    ui.m_overlayAngleSliderZ->setValue(InvProgressLerp(CSettingsManager::GetInstance()->GetOverlayAngle().z, -180.f, 180.f));
    ui.m_overlayAngleSliderZ->setToolTip(QString("Z: %1").arg(CSettingsManager::GetInstance()->GetOverlayAngle().z));
    connect(ui.m_overlayAngleSliderZ, &QSlider::valueChanged, this, &leap_control::OnOverlayAngleZChanged);

    connect(ui.m_rootOffsetResetButton, &QPushButton::clicked, this, &leap_control::OnRootOffsetReset);
    connect(ui.m_rootAngleResetButton, &QPushButton::clicked, this, &leap_control::OnRootAngleReset);
    connect(ui.m_overlaySizeResetButton, &QPushButton::clicked, this, &leap_control::OnOverlaySizeReset);
    connect(ui.m_overlayOffsetResetButton, &QPushButton::clicked, this, &leap_control::OnOverlayOffsetReset);
    connect(ui.m_overlayAngleResetButton, &QPushButton::clicked, this, &leap_control::OnOverlayAngleReset);
    connect(ui.m_saveButton, &QPushButton::clicked, this, &leap_control::OnSave);

    m_trayIcon.setIcon(QIcon(":/leap_control/leap_control.ico"));
    m_trayIcon.setToolTip("Leap Control\n(Double-click to show/hide)");
    m_trayIcon.show();
    connect(&m_trayIcon, &QSystemTrayIcon::activated, this, &leap_control::OnTrayClick);
}

void leap_control::OnTrackingLevelChange(int p_item)
{
    CSettingsManager::GetInstance()->SetSetting(CSettingsManager::ST_TrackingLevel, p_item);
}
void leap_control::OnHandsResetChange(int p_state)
{
    if(p_state != Qt::PartiallyChecked)
    {
        CSettingsManager::GetInstance()->SetSetting(CSettingsManager::ST_HandsReset, (p_state == Qt::Checked));

        std::string l_message("handsReset ");
        l_message.append(std::to_string((p_state == Qt::Checked) ? 1 : 0));
        CVRManager::GetInstance()->SendStationMessage(l_message);
    }
}
void leap_control::OnUseVelocityChange(int p_state)
{
    if(p_state != Qt::PartiallyChecked)
    {
        CSettingsManager::GetInstance()->SetSetting(CSettingsManager::ST_UseVelocity, (p_state == Qt::Checked));

        std::string l_message("useVelocity ");
        l_message.append(std::to_string((p_state == Qt::Checked) ? 1 : 0));
        CVRManager::GetInstance()->SendStationMessage(l_message);
    }
}
void leap_control::OnStartMinimizedChanged(int p_state)
{
    if(p_state != Qt::PartiallyChecked)
        CSettingsManager::GetInstance()->SetSetting(CSettingsManager::ST_StartMinimized, (p_state == Qt::Checked));
}

void leap_control::OnUseTriggerGripChange(int p_state)
{
    if(p_state != Qt::PartiallyChecked)
    {
        CSettingsManager::GetInstance()->SetSetting(CSettingsManager::ST_UseTriggerGrip, (p_state == Qt::Checked));

        std::string l_message("useTriggerGrip ");
        l_message.append(std::to_string((p_state == Qt::Checked) ? 1 : 0));
        CVRManager::GetInstance()->SendStationMessage(l_message);
    }
}
void leap_control::OnTriggerModeChange(int p_item)
{
    CSettingsManager::GetInstance()->SetSetting(CSettingsManager::ST_TriggerMode, p_item);

    std::string l_message("triggerMode ");
    l_message.append(std::to_string(p_item));
    CVRManager::GetInstance()->SendStationMessage(l_message);
}
void leap_control::OnUseControllerInputChange(int p_state)
{
    if(p_state != Qt::PartiallyChecked)
    {
        CSettingsManager::GetInstance()->SetSetting(CSettingsManager::ST_UseControllerInput, (p_state == Qt::Checked));

        std::string l_message("useControllerInput ");
        l_message.append(std::to_string((p_state == Qt::Checked) ? 1 : 0));
        CVRManager::GetInstance()->SendStationMessage(l_message);
    }
}

void leap_control::OnRootOffsetXChanged(int p_value)
{
    float l_value = ProgressLerp(p_value, -1.f, 1.f);
    ui.m_rootOffsetSliderX->setToolTip(QString("X: %1").arg(l_value));
    CSettingsManager::GetInstance()->SetSetting(CSettingsManager::ST_RootOffsetX, l_value);

    std::string l_message("rootOffsetX ");
    l_message.append(std::to_string(l_value));
    CVRManager::GetInstance()->SendStationMessage(l_message);
}
void leap_control::OnRootOffsetYChanged(int p_value)
{
    float l_value = ProgressLerp(p_value, -1.f, 1.f);
    ui.m_rootOffsetSliderY->setToolTip(QString("Y: %1").arg(l_value));
    CSettingsManager::GetInstance()->SetSetting(CSettingsManager::ST_RootOffsetY, l_value);

    std::string l_message("rootOffsetY ");
    l_message.append(std::to_string(l_value));
    CVRManager::GetInstance()->SendStationMessage(l_message);
}
void leap_control::OnRootOffsetZChanged(int p_value)
{
    float l_value = ProgressLerp(p_value, -1.f, 1.f);
    ui.m_rootOffsetSliderZ->setToolTip(QString("Z: %1").arg(l_value));
    CSettingsManager::GetInstance()->SetSetting(CSettingsManager::ST_RootOffsetZ, l_value);

    std::string l_message("rootOffsetZ ");
    l_message.append(std::to_string(l_value));
    CVRManager::GetInstance()->SendStationMessage(l_message);
}
void leap_control::OnRootOffsetReset(bool p_checked)
{
    int l_progress = InvProgressLerp(0.f, -1.f, 1.f);
    ui.m_rootOffsetSliderX->setValue(l_progress);
    ui.m_rootOffsetSliderY->setValue(l_progress);
    ui.m_rootOffsetSliderZ->setValue(l_progress);
    OnRootOffsetXChanged(l_progress);
    OnRootOffsetYChanged(l_progress);
    OnRootOffsetZChanged(l_progress);
}

void leap_control::OnRootAngleXChanged(int p_value)
{
    float l_value = ProgressLerp(p_value, -180.f, 180.f);
    ui.m_rootAngleSliderX->setToolTip(QString("X: %1").arg(l_value));
    CSettingsManager::GetInstance()->SetSetting(CSettingsManager::ST_RootAngleX, l_value);

    std::string l_message("rootAngleX ");
    l_message.append(std::to_string(l_value));
    CVRManager::GetInstance()->SendStationMessage(l_message);
}
void leap_control::OnRootAngleYChanged(int p_value)
{
    float l_value = ProgressLerp(p_value, -180.f, 180.f);
    ui.m_rootAngleSliderY->setToolTip(QString("Y: %1").arg(l_value));
    CSettingsManager::GetInstance()->SetSetting(CSettingsManager::ST_RootAngleY, l_value);

    std::string l_message("rootAngleY ");
    l_message.append(std::to_string(l_value));
    CVRManager::GetInstance()->SendStationMessage(l_message);
}
void leap_control::OnRootAngleZChanged(int p_value)
{
    float l_value = ProgressLerp(p_value, -180.f, 180.f);
    ui.m_rootAngleSliderZ->setToolTip(QString("Z: %1").arg(l_value));
    CSettingsManager::GetInstance()->SetSetting(CSettingsManager::ST_RootAngleZ, l_value);

    std::string l_message("rootAngleZ ");
    l_message.append(std::to_string(l_value));
    CVRManager::GetInstance()->SendStationMessage(l_message);
}
void leap_control::OnRootAngleReset(bool p_checked)
{
    int l_progress = InvProgressLerp(0.f, -180.f, 180.f);
    ui.m_rootAngleSliderX->setValue(l_progress);
    ui.m_rootAngleSliderY->setValue(l_progress);
    ui.m_rootAngleSliderZ->setValue(l_progress);
    OnRootAngleXChanged(l_progress);
    OnRootAngleYChanged(l_progress);
    OnRootAngleZChanged(l_progress);
}

void leap_control::OnShowOverlaysChange(int p_state)
{
    if(p_state != Qt::PartiallyChecked)
    {
        CSettingsManager::GetInstance()->SetSetting(CSettingsManager::ST_ShowOverlays, (p_state == Qt::Checked));
        COverlayManager::GetInstance()->SetOverlaysActive(p_state == Qt::Checked);
    }
}

void leap_control::OnOverlaySizeChange(int p_value)
{
    float l_value = ProgressLerp(p_value, 0.1f, 0.5f);
    ui.m_overlaySizeSlider->setToolTip(QString("%1").arg(l_value));
    CSettingsManager::GetInstance()->SetSetting(CSettingsManager::ST_OverlaySize, l_value);
    COverlayManager::GetInstance()->SetOverlaysWidth(l_value);
}
void leap_control::OnOverlaySizeReset(bool p_checked)
{
    int l_progress = InvProgressLerp(0.128f, 0.1f, 0.5f);
    ui.m_overlaySizeSlider->setValue(l_progress);
    OnOverlaySizeChange(l_progress);
}

void leap_control::OnOverlayOffsetXChanged(int p_value)
{
    float l_value = ProgressLerp(p_value, -0.5f, 0.5f);
    ui.m_overlayOffsetSliderX->setToolTip(QString("X: %1").arg(l_value));
    CSettingsManager::GetInstance()->SetSetting(CSettingsManager::ST_OverlayOffsetX, l_value);
}
void leap_control::OnOverlayOffsetYChanged(int p_value)
{
    float l_value = ProgressLerp(p_value, -0.5f, 0.5f);
    ui.m_overlayOffsetSliderY->setToolTip(QString("Y: %1").arg(l_value));
    CSettingsManager::GetInstance()->SetSetting(CSettingsManager::ST_OverlayOffsetY, l_value);
}
void leap_control::OnOverlayOffsetZChanged(int p_value)
{
    float l_value = ProgressLerp(p_value, -0.5f, 0.5f);
    ui.m_overlayOffsetSliderZ->setToolTip(QString("Z: %1").arg(l_value));
    CSettingsManager::GetInstance()->SetSetting(CSettingsManager::ST_OverlayOffsetZ, l_value);
}
void leap_control::OnOverlayOffsetReset(bool p_checked)
{
    int l_progress = InvProgressLerp(0.f, -1.f, 1.f);
    ui.m_overlayOffsetSliderX->setValue(l_progress);
    ui.m_overlayOffsetSliderY->setValue(l_progress);
    ui.m_overlayOffsetSliderZ->setValue(l_progress);
    OnOverlayOffsetXChanged(l_progress);
    OnOverlayOffsetYChanged(l_progress);
    OnOverlayOffsetZChanged(l_progress);
}

void leap_control::OnOverlayAngleXChanged(int p_value)
{
    float l_value = ProgressLerp(p_value, -180.f, 180.f);
    ui.m_overlayAngleSliderX->setToolTip(QString("X: %1").arg(l_value));
    CSettingsManager::GetInstance()->SetSetting(CSettingsManager::ST_OverlayAngleX, l_value);
}
void leap_control::OnOverlayAngleYChanged(int p_value)
{
    float l_value = ProgressLerp(p_value, -180.f, 180.f);
    ui.m_overlayAngleSliderY->setToolTip(QString("Y: %1").arg(l_value));
    CSettingsManager::GetInstance()->SetSetting(CSettingsManager::ST_OverlayAngleY, l_value);
}
void leap_control::OnOverlayAngleZChanged(int p_value)
{
    float l_value = ProgressLerp(p_value, -180.f, 180.f);
    ui.m_overlayAngleSliderZ->setToolTip(QString("Z: %1").arg(l_value));
    CSettingsManager::GetInstance()->SetSetting(CSettingsManager::ST_OverlayAngleZ, l_value);
}
void leap_control::OnOverlayAngleReset(bool p_checked)
{
    int l_progress = InvProgressLerp(0.f, -180.f, 180.f);
    ui.m_overlayAngleSliderX->setValue(l_progress);
    ui.m_overlayAngleSliderY->setValue(l_progress);
    ui.m_overlayAngleSliderZ->setValue(l_progress);
    OnOverlayAngleXChanged(l_progress);
    OnOverlayAngleYChanged(l_progress);
    OnOverlayAngleZChanged(l_progress);
}

void leap_control::OnSave(bool p_checked)
{
    CSettingsManager::GetInstance()->Save();
}

void leap_control::hideEvent(QHideEvent * p_event)
{
    this->hide(); // This is weirdly named method, ngl
    p_event->accept();
}

void leap_control::OnTrayClick(QSystemTrayIcon::ActivationReason p_reason)
{
    if(p_reason == QSystemTrayIcon::ActivationReason::DoubleClick)
    {
        if(this->isHidden())
        {
            this->show();
            this->setWindowState(Qt::WindowActive);
        }
        else
            this->hide();
    }
}
