#include "stdafx.h"
#include "CLeapMonitor.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    CLeapMonitor LeapMonitor;
    LeapMonitor.Run();
    return EXIT_SUCCESS;
}
