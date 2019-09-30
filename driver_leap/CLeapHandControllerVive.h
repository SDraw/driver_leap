#pragma once
#include "CLeapHandController.h"

class CLeapHandControllerVive final : public CLeapHandController
{
    // CLeapHandController
    vr::EVRInitError Activate(uint32_t unObjectId);
    void UpdateGestures(const Leap::Frame &f_frame);
    bool MixHandState(bool f_state);

    CLeapHandControllerVive(const CLeapHandControllerVive &that) = delete;
    CLeapHandControllerVive& operator=(const CLeapHandControllerVive &that) = delete;
public:
    CLeapHandControllerVive(unsigned char f_hand);
    ~CLeapHandControllerVive();
};

