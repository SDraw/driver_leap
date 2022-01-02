#pragma once

#include "Devices/CLeapController/CLeapController.h"

class CLeapControllerIndex final : public CLeapController
{
    enum HandSkeletonBone : size_t
    {
        HSB_Root = 0U,
        HSB_Wrist,
        HSB_Thumb0,
        HSB_Thumb1,
        HSB_Thumb2,
        HSB_Thumb3, // Last, no effect
        HSB_IndexFinger0,
        HSB_IndexFinger1,
        HSB_IndexFinger2,
        HSB_IndexFinger3,
        HSB_IndexFinger4, // Last, no effect
        HSB_MiddleFinger0,
        HSB_MiddleFinger1,
        HSB_MiddleFinger2,
        HSB_MiddleFinger3,
        HSB_MiddleFinger4, // Last, no effect
        HSB_RingFinger0,
        HSB_RingFinger1,
        HSB_RingFinger2,
        HSB_RingFinger3,
        HSB_RingFinger4, // Last, no effect
        HSB_PinkyFinger0,
        HSB_PinkyFinger1,
        HSB_PinkyFinger2,
        HSB_PinkyFinger3,
        HSB_PinkyFinger4, // Last, no effect
        HSB_Aux_Thumb,
        HSB_Aux_IndexFinger,
        HSB_Aux_MiddleFinger,
        HSB_Aux_RingFinger,
        HSB_Aux_PinkyFinger,

        HSB_Count
    };

    vr::VRBoneTransform_t m_boneTransform[HSB_Count];
    vr::VRInputComponentHandle_t m_skeletonHandle;

    CLeapControllerIndex(const CLeapControllerIndex &that) = delete;
    CLeapControllerIndex& operator=(const CLeapControllerIndex &that) = delete;

    void ChangeBoneOrientation(glm::quat &p_rot);
    static void ChangeAuxTransformation(glm::vec3 &p_pos, glm::quat &p_rot);
    static size_t GetFingerBoneIndex(size_t p_id);

    // CLeapController
    void ActivateInternal() override;
    void UpdateGestures(const LEAP_HAND *p_hand) override;
    void UpdateInputInternal() override;
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

    explicit CLeapControllerIndex(unsigned char p_hand);
    ~CLeapControllerIndex();

    void SetButtonState(size_t p_button, bool p_state);
    void SetButtonValue(size_t p_button, float p_value);
};
