#pragma once

#include "Leap.h"

#include <algorithm>

using namespace Leap;

/**
 * Hand gesture recognizer that will return an array of matches against
 * a list of predefined finger and hand poses.
 */
class GestureMatcher
{
public:
    GestureMatcher();
    ~GestureMatcher();

    enum WhichHand
    {
        AnyHand,
        LeftHand,
        RightHand
    };

    enum GestureType
    {
        // Finger gestures
        TriggerFinger,
        LowerFist,
        Pinch,
        ThumbInwards,

        // Hand gestures
        FlippingTheBird,
        ILY,
        Victory,
        TODO_ThumbsUp,
        TODO_ThumbsDown,
        TODO_LiveLongAndProsper,
        TODO_DiverOkay,
        TODO_PalmUp,
        TODO_Stop,
        TODO_FistBump,

        // Two handed gestures
        TODO_Timeout,

        NUM_GESTURES,

        INVALID_GESTURE = -1
    };

    /**
     * Perform gesture detection and quantification for the specified hand.
     * If AnyHand is specified, the gesture classifications will be merged together (typically std::max)
     */
    bool MatchGestures(const Frame &frame, WhichHand which, float (&result)[NUM_GESTURES]);

    /**
     * Map the GestureType enum to a string name.
     */
    static std::string GestureNameFromType(GestureType gesture)
    {
        switch (gesture)
        {
        case TriggerFinger: return "TriggerFinger"; break;
        case LowerFist: return "LowerFist"; break;
        case Pinch: return "Pinch"; break;
        case ThumbInwards: return "ThumbInwards"; break;
        case ILY: return "ILY"; break;
        case FlippingTheBird: return "FlippingTheBird"; break;
        case Victory: return "Victory"; break;
        default: return ""; break;
        }
    }

    /**
     * Map a string name to the GestureType enum. Case Sensitive!
     * Be sure to check the return code for INVALID_GESTURE
     */
    GestureType GestureTypeFromName(std::string &name)
    {
             if (name.compare("TriggerFinger") == 0) return TriggerFinger;
        else if (name.compare("LowerFist") == 0) return LowerFist;
        else if (name.compare("Pinch") == 0) return Pinch;
        else if (name.compare("ThumbInwards") == 0) return ThumbInwards;
        else if (name.compare("ILY") == 0) return ILY;
        else if (name.compare("FlippingTheBird") == 0) return FlippingTheBird;
        else if (name.compare("Victory") == 0) return Victory;
        else return INVALID_GESTURE;
    }

protected:

    // some utility functions

    float maprange(float input, float minimum, float maximum)
    {
        float mapped = (input - minimum) / (maximum - minimum);
        return std::max(std::min(mapped, 1.0f), 0.0f);
    }

    void merge(float &result, float value)
    {
        result = std::max(result, value);
    }

protected:

    // place state variables here (if any)
};
