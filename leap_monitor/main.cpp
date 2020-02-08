#include "stdafx.h"
#include "CLeapMonitor.h"

#ifdef _DEBUG
int main()
{
#else
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
#endif

    CLeapMonitor *l_monitor = new CLeapMonitor();
    if(l_monitor->Initialize())
    {
        l_monitor->Run();
        l_monitor->Terminate();
    }
    delete l_monitor;

    return EXIT_SUCCESS;
}
