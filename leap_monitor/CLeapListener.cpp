#include "stdafx.h"

#include "CLeapListener.h"
#include "CLeapMonitor.h"

CLeapListener::CLeapListener(CLeapMonitor *f_monitor)
{
    m_monitor = f_monitor;
}

CLeapListener::~CLeapListener()
{
}

void CLeapListener::onConnect(const Leap::Controller &controller)
{
    m_monitor->SendNotification("Controller connected");
}

void CLeapListener::onDisconnect(const Leap::Controller &controller)
{
    m_monitor->SendNotification("Controller disconnected");
}

void CLeapListener::onLogMessage(const Leap::Controller &controller, Leap::MessageSeverity severity, int64_t timestamp, const char *msg)
{
    if(severity <= Leap::MESSAGE_CRITICAL) m_monitor->SendNotification(msg);
}

void CLeapListener::onServiceConnect(const Leap::Controller &controller)
{
    m_monitor->SendNotification("Service connected");
}

void CLeapListener::onServiceDisconnect(const Leap::Controller &controller)
{
    m_monitor->SendNotification("Service disconnected");
}
