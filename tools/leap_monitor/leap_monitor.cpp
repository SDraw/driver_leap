#include "stdafx.h"
#include "CLeapMonitor.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Find resource path
    HMODULE hModule = GetModuleHandleA(NULL);
    char path[MAX_PATH];
    GetModuleFileNameA(hModule, path, MAX_PATH);

    char *snip = strstr(path, "\\bin\\");
    if(snip) *snip = '\0';

    std::string resources = std::string(path) + "\\resources\\overlays\\";

    CLeapMonitor LeapMonitor(resources);

    LeapMonitor.Run();
    return EXIT_SUCCESS;
}
