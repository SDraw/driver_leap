#pragma once

class CLeapHandController : public vr::ITrackedDeviceServerDriver
{
    vr::IVRServerDriverHost *m_driverHost;
    vr::IVRDriverInput *m_driverInput;

    int m_id;
    std::string m_serialNumber;
    vr::PropertyContainerHandle_t m_propertyContainer;
    uint32_t m_trackedDeviceID;

    vr::DriverPose_t m_pose;
    Leap::Vector m_gripAngleOffset;

    static float ms_headPos[3];
    static vr::HmdQuaternion_t ms_headRot;

    enum EGameProfile
    {
        GP_Default = 0U,
        GP_VRChat
    };
    EGameProfile m_gameProfile;
    bool m_isEnabled;

    enum EControllerButton : size_t
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
    CLeapHandController(vr::IVRServerDriverHost* pDriverHost, int n);
    virtual ~CLeapHandController();

    // vr::ITrackedDeviceServerDriver
    virtual vr::EVRInitError Activate(uint32_t unObjectId);
    virtual void Deactivate();
    void* GetComponent(const char* pchComponentNameAndVersion);
    virtual void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize);
    virtual vr::DriverPose_t GetPose();
    virtual void EnterStandby() {};

    void Update(Leap::Frame& frame);
    const char* GetSerialNumber() const;

    static void UpdateHMDCoordinates(vr::IVRServerDriverHost *f_host);
    void SetAsDisconnected();
};