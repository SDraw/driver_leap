#pragma once
#include "CLeapController.h"

class CLeapControllerIndex final : public CLeapController
{
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
    vr::VRInputComponentHandle_t m_skeletonHandle;

    CLeapControllerIndex(const CLeapControllerIndex &that) = delete;
    CLeapControllerIndex& operator=(const CLeapControllerIndex &that) = delete;

    // CLeapController
    void ActivateInternal() override;
    bool MixHandState(bool f_state) override;
    void UpdateGestures(const Leap::Frame &f_frame);
    void UpdateInputInternal() override;
public:
    explicit CLeapControllerIndex(unsigned char f_hand);
    ~CLeapControllerIndex();
};
