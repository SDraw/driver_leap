#pragma once

class CVREventsPoller
{
    static bool ms_dashboardOpened;

    CVREventsPoller() = delete;
    ~CVREventsPoller() = delete;
    CVREventsPoller(const CVREventsPoller &that) = delete;
    CVREventsPoller& operator=(const CVREventsPoller &that) = delete;
public:
    static bool IsDashboardOpened();

    static void PollEvents();
};
