#pragma once

class CControllerButton;

class CLeapHandController : public vr::ITrackedDeviceServerDriver
{
    static vr::IVRServerDriverHost *ms_driverHost;

    // HMD transformation
    static double ms_headPos[3];
    static vr::HmdQuaternion_t ms_headRot;

    vr::DriverPose_t m_pose;
    glm::quat m_gripAngleOffset;

    void UpdateTransformation(const Leap::Frame &f_frame);
    void UpdateInput();
    void ResetControls();

    // vr::ITrackedDeviceServerDriver
    void Deactivate();
    void* GetComponent(const char* pchComponentNameAndVersion);
    void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize);
    vr::DriverPose_t GetPose();
    void EnterStandby() {};

    CLeapHandController(const CLeapHandController &that) = delete;
    CLeapHandController& operator=(const CLeapHandController &that) = delete;
public:
    enum EControllerHandAssignment : unsigned char
    {
        CHA_Left = 0U,
        CHA_Right
    };

    CLeapHandController();
    virtual ~CLeapHandController();

    inline const std::string& GetSerialNumber() const { return m_serialNumber; }
    void SetConnectionState(bool f_state);
    void Update(const Leap::Frame& f_frame);

    static void SetInterfaces(vr::IVRServerDriverHost *f_host, vr::IVRDriverInput *f_input, vr::CVRPropertyHelpers *f_helpers);
    static void UpdateHMDCoordinates();
protected:
    static vr::IVRDriverInput *ms_driverInput;
    static vr::CVRPropertyHelpers *ms_propertyHelpers;

    std::string m_serialNumber;
    vr::PropertyContainerHandle_t m_propertyContainer;
    uint32_t m_trackedDeviceID;
    vr::VRInputComponentHandle_t m_haptic;

    unsigned char m_handAssigment;
    std::vector<CControllerButton*> m_buttons;
    enum GameProfile : size_t
    {
        GP_Default = 0U,
        GP_VRChat
    } m_gameProfile;
    struct GameSpecialMode
    {
        bool m_vrchatDrawingMode = false;
    } m_gameSpecialModes;

    virtual void UpdateGestures(const Leap::Frame &f_frame) = 0;
    virtual void UpdateInputInternal() {}
    virtual bool MixHandState(bool f_state);

    // vr::ITrackedDeviceServerDriver
    virtual vr::EVRInitError Activate(uint32_t unObjectId) = 0;
};