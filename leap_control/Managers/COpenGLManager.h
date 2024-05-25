#pragma once

class COpenGLManager
{
    static COpenGLManager* ms_instance;

    QOpenGLContext *m_glContext;

    COpenGLManager();
    COpenGLManager(const COpenGLManager &that) = delete;
    COpenGLManager& operator=(const COpenGLManager &that) = delete;
    ~COpenGLManager() = default;
public:
    static COpenGLManager* GetInstance();

    QOpenGLContext* GetContext() const;

    bool Init();
    void Terminate();
};

