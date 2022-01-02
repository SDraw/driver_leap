#pragma once

class CGestureMatcher
{
    static float NormalizeRange(float p_val, float p_min, float p_max);
public:
    enum HandGesture : size_t
    {
        // Simple gestures
        HG_Trigger = 0U,
        HG_Grab,

        // Finger bends
        HG_ThumbBend,
        HG_IndexBend,
        HG_MiddleBend,
        HG_RingBend,
        HG_PinkyBend,

        HG_Count
    };

    static void GetGestures(const LEAP_HAND *p_hand, std::vector<float> &p_result);
};
