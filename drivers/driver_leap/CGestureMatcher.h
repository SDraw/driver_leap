#pragma once

/**
 * Hand gesture recognizer that will return an array of matches against
 * a list of predefined finger and hand poses.
 */
class CGestureMatcher
{
public:

    enum WhichHand
    {
        LeftHand,
        RightHand
    };

    enum GestureType
    {
        // Finger gestures (these would not throw your hand's orientation off much)
        TriggerFinger,           // bend your index finger as if pulling a trigger
        LowerFist,               // grab with your middle, ring, pinky fingers
        Pinch,                   // pinch with your thumb and index fingers
        Thumbpress,              // point the thumb towards the direction of your pinky

        // Hand gestures (these would significantly change the orientation of your hand)
        FlatHandPalmUp,          // flat hand, palm points upwards (relative to alignment of Leap!)
        FlatHandPalmDown,        // flat hand, palm points downwards (relative to alignment of Leap!)
        FlatHandPalmAway,        // flat hand, palm points away from self (relative to alignment of Leap!)
        FlatHandPalmTowards,     // flat hand, palm points towards self (relative to alignment of Leap!)
        ThumbUp,                 // thumb points up, remaining fingers form a fist
        ThumbInward,             // thumb points towards the left for the right hand and vice versa

        // VRChat gestures
        VRChat_Point,
        VRChat_RockOut,
        VRChat_SpreadHand,
        VRChat_Gun,
        VRChat_ThumbsUp,
        VRChat_Victory,

        // Two handed gestures
        Timeout,                 // both Hands form a T shape, signals a Timeout in sports
        TouchpadAxisX,           // Touchpad emulation: index finger of other hand points towards palm 
        TouchpadAxisY,           // Touchpad emulation: index finger of other hand points towards palm 

        NUM_GESTURES,

        INVALID_GESTURE = -1
    };

    // default orientation vectors with respect to the Leap's coordinate system
    // in Head Mounted Mode.
    static const Leap::Vector RightVector;
    static const Leap::Vector InVector;
    static const Leap::Vector UpVector;

    /**
     * Perform gesture detection and quantification for the specified hand.
     * If AnyHand is specified, the gesture classifications will be merged together (typically std::max)
     */
    static bool MatchGestures(const Leap::Frame &frame, WhichHand which, float(&result)[NUM_GESTURES],
        const Leap::Vector& right = RightVector, const Leap::Vector& in = InVector, const Leap::Vector &up = UpVector);
protected:

    // some utility functions

    static float maprange(float input, float minimum, float maximum)
    {
        float mapped = (input - minimum) / (maximum - minimum);
        return std::max(std::min(mapped, 1.0f), 0.0f);
    }

    static void merge(float &result, float value)
    {
        result = std::max(result, value);
    }
};
