#pragma once

class CGestureMatcher
{
public:

    enum WhichHand : size_t
    {
        WH_LeftHand = 0U,
        WH_RightHand
    };

    enum GestureType : size_t
    {
        // Default
        GT_TriggerFinger = 0U,
        GT_LowerFist,
        GT_Pinch,
        GT_Thumbpress,
        GT_Victory,
        GT_ThumbUp,
        GT_ThumbInward,
        GT_ThumbMiddleTouch,
        GT_ThumbPinkyTouch,

        // Hand orientation
        GT_FlatHandPalmUp,
        GT_FlatHandPalmDown,
        GT_FlatHandPalmAway,
        GT_FlatHandPalmTowards,

        // Two-handed
        GT_Timeout,
        GT_TouchpadAxisX,
        GT_TouchpadAxisY,
        GT_ThumbIndexCrossTouch,

        // VRChat specific
        GT_VRChatPoint,
        GT_VRChatRockOut,
        GT_VRChatSpreadHand,
        GT_VRChatGun,
        GT_VRChatThumbsUp,
        GT_VRChatVictory,

        // Utility
        GT_IndexFingerBend,
        GT_MiddleFingerBend,
        GT_RingFingerBend,
        GT_PinkyFingerBend,

        GT_GesturesCount,
        GT_Invalid = 0xFFU
    };

    static bool GetGestures(const Leap::Frame &f_frame, WhichHand f_which, std::vector<float> &f_result);
    static void GetGestureName(GestureType f_gesture, std::string &f_name);
protected:
    static float MapRange(float input, float minimum, float maximum);
    static void Merge(float &result, float value);
};
