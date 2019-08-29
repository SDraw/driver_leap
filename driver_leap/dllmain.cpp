#include "stdafx.h"
#include "CServerDriver.h"

#define HMD_DLL_EXPORT extern "C" __declspec( dllexport )

char g_moduleFilePath[2048U];

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID /* lpReserved */)
{
    switch(ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            GetModuleFileNameA(hModule, g_moduleFilePath, 2048U);
            break;
        case DLL_THREAD_ATTACH: case DLL_THREAD_DETACH: case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

CServerDriver g_serverDriver;

HMD_DLL_EXPORT void* HmdDriverFactory(const char *pInterfaceName, int *pReturnCode)
{
    void *l_result = nullptr;
    if(!strcmp(vr::IServerTrackedDeviceProvider_Version, pInterfaceName)) l_result = &g_serverDriver;
    else
    {
        if(pReturnCode) *pReturnCode = vr::VRInitError_Init_InterfaceNotFound;
    }
    return l_result;
}
