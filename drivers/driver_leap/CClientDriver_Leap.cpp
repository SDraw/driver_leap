#include "stdafx.h"
#include "CClientDriver_Leap.h"
#include "CDriverLogHelper.h"

CClientDriver_Leap::CClientDriver_Leap()
{
    m_pDriverHost = nullptr;
}

CClientDriver_Leap::~CClientDriver_Leap()
{
}

vr::EVRInitError CClientDriver_Leap::Init(vr::EClientDriverMode eDriverMode, vr::IDriverLog* pDriverLog, vr::IClientDriverHost* pDriverHost, const char* pchUserDriverConfigDir, const char* pchDriverInstallDir)
{
    CDriverLogHelper::InitDriverLog(pDriverLog);
    CDriverLogHelper::DriverLog("CClientDriver_Leap::Init()\n");
    m_pDriverHost = pDriverHost;
    return vr::VRInitError_None;
}

void CClientDriver_Leap::Cleanup()
{
    CDriverLogHelper::DriverLog("CClientDriver_Leap::Cleanup()\n");
}

bool CClientDriver_Leap::BIsHmdPresent(const char* pchUserConfigDir)
{
    return false;
}

vr::EVRInitError CClientDriver_Leap::SetDisplayId(const char* pchDisplayId)
{
    return vr::VRInitError_None;
    //return vr::VRInitError_Driver_HmdUnknown;
}

vr::HiddenAreaMesh_t CClientDriver_Leap::GetHiddenAreaMesh(vr::EVREye eEye, vr::EHiddenAreaMeshType type)
{
    return vr::HiddenAreaMesh_t();
}

uint32_t CClientDriver_Leap::GetMCImage(uint32_t* pImgWidth, uint32_t* pImgHeight, uint32_t* pChannels, void* pDataBuffer, uint32_t unBufferLen)
{
    return uint32_t();
}
