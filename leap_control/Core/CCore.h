#pragma once

class leap_control;

class CCore : public QApplication
{
    leap_control *m_window;
    QTimer *m_timer;

    CCore(const CCore &that) = delete;
    CCore& operator=(const CCore &that) = delete;

    void UpdateLoop();
public:
    CCore(int &argc, char **argv);
    ~CCore() = default;

    void Launch();
};

