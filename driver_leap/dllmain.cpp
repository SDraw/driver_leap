#include "stdafx.h"
#include "CServerDriver.h"

#define HMD_DLL_EXPORT extern "C" __declspec( dllexport )

char g_moduleFileName[2048];

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID /* lpReserved */)
{
    switch(ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            GetModuleFileNameA(hModule, g_moduleFileName, 2048);
            break;
        case DLL_THREAD_ATTACH: case DLL_THREAD_DETACH: case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

CServerDriver g_serverDriver;

HMD_DLL_EXPORT void* HmdDriverFactory(const char *pInterfaceName, int *pReturnCode)
{
    if(!strcmp(vr::IServerTrackedDeviceProvider_Version, pInterfaceName)) return &g_serverDriver;

    if(pReturnCode) *pReturnCode = vr::VRInitError_Init_InterfaceNotFound;

    return nullptr;
}
