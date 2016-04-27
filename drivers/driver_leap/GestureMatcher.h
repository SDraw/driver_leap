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
        TODO_ThumbsUp,
        TODO_ThumbsDown,
        TODO_FlippingTheBird,
        TODO_ILY,
        TODO_LiveLongAndProsper,
        TODO_Victory,
        TODO_DiverOkay,
        TODO_PalmUp,
        TODO_Stop,
        TODO_FistBump,

        // Two handed gestures
        TODO_Timeout,

        NUM_GESTURES,

        INVALID_GESTURE = -1
    };

    bool MatchGestures(const Frame &frame, WhichHand which, float (&result)[NUM_GESTURES]);

    static std::string GestureNameFromType(GestureType gesture)
    {
        switch (gesture)
        {
        case TriggerFinger: return "TriggerFinger"; break;
        case LowerFist: return "LowerFist"; break;
        case Pinch: return "Pinch"; break;
        case ThumbInwards: return "ThumbInwards"; break;
        default: return ""; break;
        }
    }

    GestureType GestureTypeFromName(std::string &name)
    {
             if (name.compare("TriggerFinger") == 0) return TriggerFinger;
        else if (name.compare("LowerFist") == 0) return LowerFist;
        else if (name.compare("Pinch") == 0) return Pinch;
        else if (name.compare("ThumbInwards") == 0) return Pinch;
        else return INVALID_GESTURE;
    }

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
};
