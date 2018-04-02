//========= Copyright Valve Corporation ============//
//
// driver_leap.cpp : Defines the client and server interfaces used by the SteamVR runtime.
//
#include "stdafx.h"
#include "CServerDriver_Leap.h"
#include "CClientDriver_Leap.h"

#define HMD_DLL_EXPORT extern "C" __declspec( dllexport )

CServerDriver_Leap g_ServerTrackedDeviceProvider;
CClientDriver_Leap g_ClientTrackedDeviceProvider;

HMD_DLL_EXPORT
void *HmdDriverFactory(const char *pInterfaceName, int *pReturnCode)
{
    if(!strcmp(vr::IServerTrackedDeviceProvider_Version, pInterfaceName)) return &g_ServerTrackedDeviceProvider;
    if(!strcmp(vr::IClientTrackedDeviceProvider_Version, pInterfaceName)) return &g_ClientTrackedDeviceProvider;

    if(pReturnCode) *pReturnCode = vr::VRInitError_Init_InterfaceNotFound;

    return NULL;
}
