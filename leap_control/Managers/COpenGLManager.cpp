#include "stdafx.h"
#include "Managers/COpenGLManager.h"

COpenGLManager* COpenGLManager::ms_instance = nullptr;

COpenGLManager* COpenGLManager::GetInstance()
{
    if(!ms_instance)
        ms_instance = new COpenGLManager();

    return ms_instance;
}

COpenGLManager::COpenGLManager()
{
    m_glContext = nullptr;
}

bool COpenGLManager::Init()
{
    if(!m_glContext)
    {
        QSurfaceFormat l_format;
        l_format.setMajorVersion(4);
        l_format.setMinorVersion(1);
        l_format.setProfile(QSurfaceFormat::CoreProfile);
        l_format.setRenderableType(QSurfaceFormat::RenderableType::OpenGL);
        l_format.setAlphaBufferSize(8);

        m_glContext = new QOpenGLContext();
        m_glContext->setFormat(l_format);
        if(!m_glContext->create())
        {
            delete m_glContext;
            m_glContext = nullptr;

            QMessageBox::critical(nullptr, "Leap Control", "Unable to create OpenGL 4.1 Core context");
        }
    }

    return (m_glContext != nullptr);
}

void COpenGLManager::Terminate()
{
    if(m_glContext)
    {
        delete m_glContext;
        m_glContext = nullptr;
    }
}

QOpenGLContext* COpenGLManager::GetContext() const
{
    return m_glContext;
}
