#include "stdafx.h"

#include "Core/CServerDriver.h"

std::wstring g_modulePath;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID)
{
    switch(ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        {
            g_modulePath.resize(2048U);
            unsigned long l_length = GetModuleFileNameW(hModule, &g_modulePath[0], 2048U);
            g_modulePath.resize(l_length);
            g_modulePath.erase(g_modulePath.begin() + g_modulePath.rfind(L'\\'), g_modulePath.end());
        } break;
        case DLL_THREAD_ATTACH: case DLL_THREAD_DETACH: case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

CServerDriver g_serverDriver;

extern "C" __declspec(dllexport) void* HmdDriverFactory(const char *pInterfaceName, int *pReturnCode)
{
    void *l_result = nullptr;
    if(!strcmp(vr::IServerTrackedDeviceProvider_Version, pInterfaceName)) l_result = dynamic_cast<vr::IServerTrackedDeviceProvider*>(&g_serverDriver);
    else
    {
        if(pReturnCode) *pReturnCode = vr::VRInitError_Init_InterfaceNotFound;
    }
    return l_result;
}
