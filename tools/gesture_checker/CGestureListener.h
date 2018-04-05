#pragma once

class CGestureListener : public Leap::Listener
{
public:
    virtual void onInit(const Leap::Controller&);
    virtual void onConnect(const Leap::Controller&);
    virtual void onDisconnect(const Leap::Controller&);
    virtual void onExit(const Leap::Controller&);
    virtual void onFrame(const Leap::Controller&);
    virtual void onFocusGained(const Leap::Controller&);
    virtual void onFocusLost(const Leap::Controller&);
    virtual void onDeviceChange(const Leap::Controller&);
    virtual void onServiceConnect(const Leap::Controller&);
    virtual void onServiceDisconnect(const Leap::Controller&);
    virtual void onServiceChange(const Leap::Controller&);
    virtual void onDeviceFailure(const Leap::Controller&);
    virtual void onLogMessage(const Leap::Controller&, Leap::MessageSeverity s, int64_t t, const char* msg);
};