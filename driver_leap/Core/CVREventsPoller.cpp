#include "stdafx.h"
#include "Core/CVREventsPoller.h"

bool CVREventsPoller::ms_dashboardOpened = false;

bool CVREventsPoller::IsDashboardOpened()
{
    return ms_dashboardOpened;
}

void CVREventsPoller::PollEvents()
{
    vr::VREvent_t l_event{ 0 };
    while(vr::VRServerDriverHost()->PollNextEvent(&l_event, sizeof(vr::VREvent_t)))
    {
        switch(l_event.eventType)
        {
            case vr::VREvent_DashboardActivated:
                ms_dashboardOpened = true;
                break;
            
            case vr::VREvent_DashboardDeactivated:
                ms_dashboardOpened = false;
                break;
        }
    }
}
