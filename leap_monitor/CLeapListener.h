#pragma once

class CLeapMonitor;

class CLeapListener final : public Leap::Listener
{
    CLeapMonitor *m_monitor = nullptr;

    virtual void onConnect(const Leap::Controller &controller);
    virtual void onDisconnect(const Leap::Controller &controller);
    virtual void onLogMessage(const Leap::Controller &controller, Leap::MessageSeverity severity, int64_t timestamp, const char *msg); // Async
    virtual void onServiceConnect(const Leap::Controller &controller);
    virtual void onServiceDisconnect(const Leap::Controller &controller);
public:
    explicit CLeapListener(CLeapMonitor *f_monitor);
    ~CLeapListener();
};

