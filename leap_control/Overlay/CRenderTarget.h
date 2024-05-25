#pragma once

class CRenderTarget
{
    QOffscreenSurface *m_surface;
    QGraphicsScene *m_scene;
    QOpenGLFramebufferObject *m_fbo;
    QOpenGLPaintDevice *m_paintDevice;
    QPainter *m_painter;

    std::vector<QWidget*> m_widgets;

    CRenderTarget(const CRenderTarget &that) = delete;
    CRenderTarget& operator=(const CRenderTarget &that) = delete;
public:
    CRenderTarget();
    ~CRenderTarget() = default;

    bool Create(int p_width, int p_height);
    void Destroy();

    GLuint GetTextureID() const;

    void AddWidget(QWidget *p_widget);

    void Update();
};

