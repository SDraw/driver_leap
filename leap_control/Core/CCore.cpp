#include "stdafx.h"
#include "Core/CCore.h"
#include "Managers/COpenGLManager.h"
#include "Managers/CVRManager.h"
#include "Managers/COverlayManager.h"
#include "Managers/CLeapManager.h"
#include "Managers/CSettingsManager.h"
#include "Ui/leap_control.h"

CCore::CCore(int &argc, char **argv) : QApplication(argc, argv)
{
    m_window = nullptr;
    m_timer = nullptr;
}

void CCore::Launch()
{
    CSettingsManager::GetInstance()->Load();

    if(!CLeapManager::GetInstance()->Init())
        return;
    if(!COpenGLManager::GetInstance()->Init())
        return;
    if(!CVRManager::GetInstance()->Init())
        return;

    COverlayManager::GetInstance()->CreateOverlays();

    m_window = new leap_control();
    if(!CSettingsManager::GetInstance()->GetStartMinimized())
        m_window->show();

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &CCore::UpdateLoop);
    m_timer->start(16); // 60 FPS lesgo!

    QApplication::exec();

    m_timer->stop();
    delete m_timer;
    m_timer = nullptr;

    delete m_window;
    m_window = nullptr;

    COverlayManager::GetInstance()->DestroyOverlays();
    CVRManager::GetInstance()->Terminate();
    COpenGLManager::GetInstance()->Terminate();
    CLeapManager::GetInstance()->Terminate();
    CSettingsManager::GetInstance()->Save();
}

void CCore::UpdateLoop()
{
    CVRManager::GetInstance()->Update();
    if(CVRManager::GetInstance()->IsExitPolled())
        m_window->close();
    CLeapManager::GetInstance()->Update();
    COverlayManager::GetInstance()->Update();
}
