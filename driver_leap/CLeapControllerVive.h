#pragma once

#include "CLeapController.h"

class CLeapControllerVive final : public CLeapController
{
    CLeapControllerVive(const CLeapControllerVive &that) = delete;
    CLeapControllerVive& operator=(const CLeapControllerVive &that) = delete;

    // CLeapController
    void ActivateInternal() override;
    void UpdateGestures(const Leap::Frame &f_frame) override;
public:
    explicit CLeapControllerVive(unsigned char f_hand);
    ~CLeapControllerVive();
};
