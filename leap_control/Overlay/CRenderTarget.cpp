#include "stdafx.h"
#include "Overlay/CRenderTarget.h"
#include "Managers/COpenGLManager.h"

CRenderTarget::CRenderTarget()
{
    m_surface = nullptr;
    m_scene = nullptr;
    m_fbo = nullptr;
    m_paintDevice = nullptr;
    m_painter = nullptr;
}

bool CRenderTarget::Create(int p_width, int p_height)
{
    if(!m_surface)
    {
        m_surface = new QOffscreenSurface();
        m_surface->create();
        COpenGLManager::GetInstance()->GetContext()->makeCurrent(m_surface);

        QOpenGLFramebufferObjectFormat l_format;
        l_format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        m_fbo = new QOpenGLFramebufferObject(p_width, p_height, l_format);

        m_paintDevice = new QOpenGLPaintDevice(m_fbo->size());
        m_painter = new QPainter(m_paintDevice);
        m_painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

        m_scene = new QGraphicsScene();
    }
    return (m_surface != nullptr);
}

void CRenderTarget::Destroy()
{
    if(m_surface)
    {
        m_widgets.clear();

        delete m_scene;
        m_scene = nullptr;

        delete m_painter;
        m_painter = nullptr;

        delete m_paintDevice;
        m_paintDevice = nullptr;

        delete m_fbo;
        m_fbo = nullptr;

        m_surface->destroy();
        delete m_surface;
        m_surface = nullptr;
    }
}

GLuint CRenderTarget::GetTextureID() const
{
    return (m_fbo ? m_fbo->texture() : 0U);
}

void CRenderTarget::AddWidget(QWidget * p_widget)
{
    m_widgets.push_back(p_widget);
    if(m_scene)
        m_scene->addWidget(p_widget);
}

void CRenderTarget::Update()
{
    if(m_surface)
        COpenGLManager::GetInstance()->GetContext()->makeCurrent(m_surface);
    if(m_fbo)
        m_fbo->bind();
    if(m_painter && m_fbo)
        m_painter->fillRect(0, 0, m_fbo->size().width(), m_fbo->height(), Qt::transparent);
    if(m_scene)
        m_scene->render(m_painter);
    if(m_fbo)
        m_fbo->release();
}
