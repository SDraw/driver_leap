#pragma once

class CControllerButton;
class CLeapHand;

class CLeapIndexController : public vr::ITrackedDeviceServerDriver
{
    enum SkeletonBone : size_t
    {
        SB_Root = 0U,
        SB_Wrist,
        SB_Thumb0,
        SB_Thumb1,
        SB_Thumb2,
        SB_Thumb3, // Last, no effect
        SB_IndexFinger0,
        SB_IndexFinger1,
        SB_IndexFinger2,
        SB_IndexFinger3,
        SB_IndexFinger4, // Last, no effect
        SB_MiddleFinger0,
        SB_MiddleFinger1,
        SB_MiddleFinger2,
        SB_MiddleFinger3,
        SB_MiddleFinger4, // Last, no effect
        SB_RingFinger0,
        SB_RingFinger1,
        SB_RingFinger2,
        SB_RingFinger3,
        SB_RingFinger4, // Last, no effect
        SB_PinkyFinger0,
        SB_PinkyFinger1,
        SB_PinkyFinger2,
        SB_PinkyFinger3,
        SB_PinkyFinger4, // Last, no effect
        SB_Aux_Thumb,
        SB_Aux_IndexFinger,
        SB_Aux_MiddleFinger,
        SB_Aux_RingFinger,
        SB_Aux_PinkyFinger,

        SB_Count
    };
    enum HandFinger : size_t
    {
        HF_Thumb = 0U,
        HF_Index,
        HF_Middle,
        HF_Ring,
        HF_Pinky,

        HF_Count
    };

    static double ms_headPosition[3U];
    static vr::HmdQuaternion_t ms_headRotation;

    uint32_t m_trackedDevice;
    vr::PropertyContainerHandle_t m_propertyContainer;
    vr::DriverPose_t m_pose;
    vr::VRInputComponentHandle_t m_haptic;
    vr::VRBoneTransform_t m_boneTransform[SB_Count];
    vr::VRInputComponentHandle_t m_skeletonHandle;

    bool m_isLeft;
    std::string m_serialNumber;
    std::vector<CControllerButton*> m_buttons;
    glm::vec3 m_position;
    glm::quat m_rotation;

    CLeapIndexController(const CLeapIndexController &that) = delete;
    CLeapIndexController& operator=(const CLeapIndexController &that) = delete;

    void UpdatePose(const CLeapHand *p_hand);
    void UpdateSkeletalInput(const CLeapHand *p_hand);
    void UpdateInput(const CLeapHand *p_hand);

    void ResetControls();
    void ChangeBoneOrientation(glm::quat &p_rot) const;
    void ChangeBonePosition(glm::vec3 &p_pos) const;
    void FixMetacarpalBone(glm::quat &p_rot) const;

    void ProcessExternalInput(const char *p_message);

    static void ChangeAuxTransformation(glm::vec3 &p_pos, glm::quat &p_rot);
    static size_t GetFingerBoneIndex(size_t p_id);

    // vr::ITrackedDeviceServerDriver
    vr::EVRInitError Activate(uint32_t unObjectId);
    void Deactivate();
    void EnterStandby();
    void* GetComponent(const char* pchComponentNameAndVersion);
    void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize);
    vr::DriverPose_t GetPose();
public:
    enum IndexButton : size_t
    {
        IB_SystemClick = 0U,
        IB_SystemTouch,
        IB_TriggerClick,
        IB_TriggerTouch,
        IB_TriggerValue,
        IB_TrackpadX,
        IB_TrackpadY,
        IB_TrackpadTouch,
        IB_TrackpadForce,
        IB_GripTouch,
        IB_GripForce,
        IB_GripValue,
        IB_ThumbstickClick,
        IB_ThumbstickTouch,
        IB_ThumbstickX,
        IB_ThumbstickY,
        IB_AClick,
        IB_ATouch,
        IB_BClick,
        IB_BTouch,
        IB_FingerIndex,
        IB_FingerMiddle,
        IB_FingerRing,
        IB_FingerPinky,

        IB_Count
    };

    explicit CLeapIndexController(bool p_left);
    ~CLeapIndexController();

    void RunFrame(const CLeapHand *p_hand);

    const std::string& GetSerialNumber() const;

    void SetEnabled(bool p_state);
    void SetButtonState(size_t p_button, bool p_state);
    void SetButtonValue(size_t p_button, float p_value);

    static void UpdateHMDCoordinates();
};
