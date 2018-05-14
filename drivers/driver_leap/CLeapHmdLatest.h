#pragma once

class CLeapHmdLatest : public vr::ITrackedDeviceServerDriver
{
    vr::IVRServerDriverHost *m_pDriverHost;
    vr::IVRDriverInput *m_driverInput;

    int m_nId;
    std::string m_strSerialNumber;
    vr::PropertyContainerHandle_t m_propertyContainer;
    uint32_t m_unSteamVRTrackedDeviceId;
    bool m_connected;

    vr::DriverPose_t m_Pose;
    float m_hmdPos[3];
    vr::HmdQuaternion_t m_hmdRot;
    Leap::Vector m_gripAngleOffset;

    enum EGameProfile
    {
        GP_Default = 0U,
        GP_VRChat
    };
    EGameProfile m_gameProfile;

    enum EControllerButton
    {
        CB_SysClick = 0U,
        CB_GripClick,
        CB_AppMenuClick,
        CB_TriggerClick,
        CB_TriggerValue,
        CB_TrackpadX,
        CB_TrackpadY,
        CB_TrackpadClick,
        CB_TrackpadTouch,

        CB_Count
    };
    struct SControllerButton
    {
        vr::VRInputComponentHandle_t m_handle = vr::k_ulInvalidInputComponentHandle;
        union
        {
            float m_value;
            bool m_state;
        };
    };
    SControllerButton m_buttons[CB_Count];

    void UpdateControllerState(Leap::Frame &frame);
    void UpdateTrackingState(Leap::Frame &frame);

    void ProcessDefaultProfileGestures(float *l_scores);
    void ProcessVRChatProfileGestures(float *l_scores);

    void ResetControls();
public:
    CLeapHmdLatest(vr::IVRServerDriverHost* pDriverHost, int n);
    virtual ~CLeapHmdLatest();

    // vr::ITrackedDeviceServerDriver
    virtual vr::EVRInitError Activate(uint32_t unObjectId) override;
    virtual void Deactivate() override;
    void* GetComponent(const char* pchComponentNameAndVersion) override;
    virtual void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) override;
    virtual vr::DriverPose_t GetPose() override;
    virtual void EnterStandby() override;

    void Update(Leap::Frame& frame);
    const char* GetSerialNumber() const;

    void RealignCoordinates();
    void SetAsDisconnected();
};