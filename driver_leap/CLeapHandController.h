#pragma once

enum EControllerButtonInputType : unsigned char
{
    CBIT_None = 0U,
    CBIT_Boolean,
    CBIT_Float
};
class CControllerButton
{
    vr::VRInputComponentHandle_t m_handle;
    float m_value;
    bool m_state;
    EControllerButtonInputType m_inputType;
    bool m_updated;
public:
    CControllerButton();
    ~CControllerButton();

    inline vr::VRInputComponentHandle_t GetHandle() const { return m_handle; }
    inline vr::VRInputComponentHandle_t& GetHandleRef() { return m_handle; }

    inline void SetInputType(EControllerButtonInputType f_type) { m_inputType = f_type; }
    inline EControllerButtonInputType GetInputType() const { return m_inputType; }

    void SetValue(float f_value);
    inline float GetValue() const { return m_value; }

    void SetState(bool f_state);
    inline bool GetState() const { return m_state; }

    inline bool IsUpdated() const { return m_updated; }
    inline void ResetUpdate() { m_updated = false; }
};

class CLeapHandController : public vr::ITrackedDeviceServerDriver
{
    vr::IVRServerDriverHost *m_driverHost;
    vr::IVRDriverInput *m_driverInput;

    int m_id;
    std::string m_serialNumber;
    vr::PropertyContainerHandle_t m_propertyContainer;
    uint32_t m_trackedDeviceID;

    vr::DriverPose_t m_pose;
    glm::quat m_gripAngleOffset;

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
    CControllerButton m_buttons[CB_Count];

    static double ms_headPos[3];
    static vr::HmdQuaternion_t ms_headRot;

    void UpdateGestures(Leap::Frame &frame);
    void UpdateTrasnformation(Leap::Frame &frame);
    void UpdateButtonInput();

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

    const char* GetSerialNumber() const;
    void SetAsDisconnected();

    void Update(Leap::Frame& frame);
    static void UpdateHMDCoordinates(vr::IVRServerDriverHost *f_host);
};