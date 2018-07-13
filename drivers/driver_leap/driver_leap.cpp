#include "stdafx.h"
#include "CServerDriver_Leap.h"

#define HMD_DLL_EXPORT extern "C" __declspec( dllexport )

CServerDriver_Leap g_ServerTrackedDeviceProvider;

HMD_DLL_EXPORT void *HmdDriverFactory(const char *pInterfaceName, int *pReturnCode)
{
    if(!strcmp(vr::IServerTrackedDeviceProvider_Version, pInterfaceName)) return &g_ServerTrackedDeviceProvider;

    if(pReturnCode) *pReturnCode = vr::VRInitError_Init_InterfaceNotFound;

    return nullptr;
}
