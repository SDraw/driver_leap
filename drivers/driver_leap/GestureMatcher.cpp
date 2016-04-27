
#include "GestureMatcher.h"

GestureMatcher::GestureMatcher()
{
}

GestureMatcher::~GestureMatcher()
{
}

bool GestureMatcher::MatchGestures(const Frame &frame, WhichHand which, float(&result)[NUM_GESTURES])
{
    // first, set all gesture matches to zero
    bool success = false;
    memset(result, 0, sizeof(result));

    // Go through the hands in the dataset
    HandList &hands = frame.hands();
    for (int h = 0; h < hands.count(); h++)
    {
        Hand &hand = hands[h];

        // these are the conditions under which we do not want
        // to evaluate a hand from the Frame dataset.
        if (!hand.isValid()) continue;
        if (which == RightHand && hand.isLeft()) continue;
        if (which == LeftHand && hand.isRight()) continue;

        // okay, found a hand we want to look at.
        success = true;

        // stores the bend angles per finger and per joint
        float bends[5][3] = { 0 };

        // total bend angle of a finger
        float sumbend[5] = { 0 };

        Vector fingerdir[5];
        memset(fingerdir, 0, sizeof(fingerdir));

        // Evaluate bending of all fingers
        const FingerList &fingers = hand.fingers();
        for (auto fl = fingers.begin(); fl != fingers.end(); ++fl) {
            const Finger &finger = *fl;

            int f = finger.type(); // thumb, index, middle, ring, pinky
            if (finger.isFinger() && finger.isValid())
            {
                // go through the finger's bones:
                // metacarpal, proximal, intermediate, distal
                Vector prev_direction;
                for (int b = 0; b < 4; b++)
                {
                    Leap::Bone &bone = finger.bone(Leap::Bone::Type(b));
                    Vector direction = bone.direction();
                    
                    if (b == Leap::Bone::TYPE_DISTAL)
                        fingerdir[f] = direction;
    
                    if (b > 0)
                    {
                        // get the bend angle of each finger joint
                        bends[f][b-1] = 57.2957795f * direction.angleTo(prev_direction); // in degrees

                        // also sum up the total
                        sumbend[f] += bends[f][b - 1];
                    }
                    prev_direction = direction;
                }
            }
        }

        Vector normal = hand.palmNormal();
        Vector direction = hand.direction();
        Vector side;
        if (hand.isRight())
            side = direction.cross(normal);
        else
            side = normal.cross(direction);
        Vector thumbdir = fingerdir[Finger::TYPE_THUMB];
        float cosalpha = thumbdir.dot(side);

        // trigger figure gesture means the bend angles of the upper two joints
        // of the index finger exceed 70 degrees.
        float triggerbend = bends[Finger::TYPE_INDEX][1] + bends[Finger::TYPE_INDEX][2];
        float trigger = maprange(triggerbend, 70.0, 100.0);
        merge(result[TriggerFinger], trigger);

        // lower first gesture means clenching middle, ring, pinky fingers beyond 90 degrees
        float grip = maprange((sumbend[Leap::Finger::TYPE_MIDDLE] + sumbend[Leap::Finger::TYPE_RING] + sumbend[Leap::Finger::TYPE_PINKY]) / 3, 90.0, 180.0);
        merge(result[LowerFist], grip);

        // pinch gesture means pinching the index and thumb (distance closer than 25mm)
        float pinch = maprange(hand.pinchDistance(), 40, 30);
        merge(result[Pinch], pinch);

        // ThumbInwards gesture means that the thumb points inwards (direction of palm, opposite of thumbs up)
        result[ThumbInwards] = maprange(cosalpha, 0.0, 0.6);
    }

    return success;
}
