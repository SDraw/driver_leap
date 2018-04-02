#pragma once

class CClientDriver_Leap : public vr::IClientTrackedDeviceProvider
{
public:
    CClientDriver_Leap();
    virtual ~CClientDriver_Leap();

    // Inherited via IClientTrackedDeviceProvider
    virtual vr::EVRInitError Init(vr::EClientDriverMode eDriverMode, vr::IDriverLog* pDriverLog, vr::IClientDriverHost* pDriverHost, const char* pchUserDriverConfigDir, const char *pchDriverInstallDir) override;
    virtual void Cleanup() override;
    virtual bool BIsHmdPresent(const char* pchUserConfigDir) override;
    virtual vr::EVRInitError SetDisplayId(const char* pchDisplayId) override;
    virtual vr::HiddenAreaMesh_t GetHiddenAreaMesh(vr::EVREye eEye, vr::EHiddenAreaMeshType type) override;
    virtual uint32_t GetMCImage(uint32_t* pImgWidth, uint32_t* pImgHeight, uint32_t* pChannels, void* pDataBuffer, uint32_t unBufferLen) override;
private:
    vr::IClientDriverHost* m_pDriverHost;

};
