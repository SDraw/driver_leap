#pragma once
#include "CLeapHandController.h"

class CLeapHandControllerIndex final : public CLeapHandController
{
    vr::VRInputComponentHandle_t m_skeletonHandle;
    enum HandSkeletonBone : size_t
    {
        HSB_Root = 0U,
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
        HSB_Aux_Thumb,
        HSB_Aux_IndexFinger,
        HSB_Aux_MiddleFinger,
        HSB_Aux_RingFinger,
        HSB_Aux_PinkyFinger,

        HSB_Count
    };
    vr::VRBoneTransform_t m_boneTransform[HSB_Count];

    // CLeapHandController
    vr::EVRInitError Activate(uint32_t unObjectId);
    void UpdateGestures(const Leap::Frame &f_frame);
    void UpdateInputInternal();
    bool MixHandState(bool f_state);

    CLeapHandControllerIndex(const CLeapHandControllerIndex &that) = delete;
    CLeapHandControllerIndex& operator=(const CLeapHandControllerIndex &that) = delete;
public:
    explicit CLeapHandControllerIndex(unsigned char f_hand);
    ~CLeapHandControllerIndex();
};

