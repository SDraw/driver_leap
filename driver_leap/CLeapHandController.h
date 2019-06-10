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

    enum EControllerHandAssignment : unsigned char
    {
        CHA_Left = 0U,
        CHA_Right
    };
    unsigned char m_id;
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
        CB_GripForce,
        CB_GripTouch,
        CB_GripValue,
        CB_AppMenuClick,
        CB_TriggerClick,
        CB_TriggerValue,
        CB_TrackpadX,
        CB_TrackpadY,
        CB_TrackpadClick,
        CB_TrackpadTouch,
        CB_TrackpadForce,
        CB_ThumbstickClick,
        CB_ThumbstickTouch,
        CB_ThumbstickX,
        CB_ThumbstickY,
        CB_IndexAClick,
        CB_IndexATouch,
        CB_IndexBClick,
        CB_IndexBTouch,
        CB_FingerIndex,
        CB_FingerMiddle,
        CB_FingerRing,
        CB_FingerPinky,

        CB_Count
    };
    CControllerButton m_buttons[CB_Count];

    static double ms_headPos[3];
    static vr::HmdQuaternion_t ms_headRot;

    // Index
    vr::VRInputComponentHandle_t m_skeletonHandle;
    enum EHandSkeletonBone : size_t
    {
	    HSB_Root = 0,
	    HSB_Wrist,
	    HSB_Thumb0,
	    HSB_Thumb1,
	    HSB_Thumb2,
	    HSB_Thumb3,
	    HSB_IndexFinger0,
	    HSB_IndexFinger1,
	    HSB_IndexFinger2,
	    HSB_IndexFinger3,
	    HSB_IndexFinger4,
	    HSB_MiddleFinger0,
	    HSB_MiddleFinger1,
	    HSB_MiddleFinger2,
	    HSB_MiddleFinger3,
	    HSB_MiddleFinger4,
	    HSB_RingFinger0,
	    HSB_RingFinger1,
	    HSB_RingFinger2,
	    HSB_RingFinger3,
	    HSB_RingFinger4,
	    HSB_PinkyFinger0,
	    HSB_PinkyFinger1,
	    HSB_PinkyFinger2,
	    HSB_PinkyFinger3,
	    HSB_PinkyFinger4,
	    HSB_Aux_Thumb, // Not used yet
	    HSB_Aux_IndexFinger, // Not used yet
	    HSB_Aux_MiddleFinger, // Not used yet
	    HSB_Aux_RingFinger, // Not used yet
	    HSB_Aux_PinkyFinger, // Not used yet
	    HSB_Count
    };
    vr::VRBoneTransform_t m_boneTransform[HSB_Count];

    void UpdateGestures(const Leap::Frame &frame);
    void UpdateTrasnformation(const Leap::Frame &frame);
    void UpdateButtonInput();

    void ProcessViveDefaultProfileGestures(const std::vector<float> &l_scores);
    void ProcessViveVRChatProfileGestures(const std::vector<float> &l_scores);
    void ProcessIndexGestures(const Leap::Frame &frame, const std::vector<float> &l_scores);

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

    void Update(const Leap::Frame& frame);
    static void UpdateHMDCoordinates(vr::IVRServerDriverHost *f_host);
};