
#include "GestureMatcher.h"

const Vector GestureMatcher::RightVector = Vector(-1,  0,  0);
const Vector GestureMatcher::InVector    = Vector( 0,  1,  0);
const Vector GestureMatcher::UpVector    = Vector( 0,  0, -1);


GestureMatcher::GestureMatcher()
{
}

GestureMatcher::~GestureMatcher()
{
}

bool GestureMatcher::MatchGestures(const Frame &frame, WhichHand which, float(&result)[NUM_GESTURES],
                                   Vector right, Vector in, Vector up)
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
                    Bone &bone = finger.bone(Bone::Type(b));
                    Vector direction = -bone.direction(); // for some reaason bone directions point handinwards?
                    
                    if (b == Bone::TYPE_DISTAL)
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

        // trigger figure gesture means the bend angles of the upper two joints
        // of the index finger exceed 70 degrees.
        float triggerbend = bends[Finger::TYPE_INDEX][1] + bends[Finger::TYPE_INDEX][2];
        float trigger = maprange(triggerbend, 70.0, 100.0);
        merge(result[TriggerFinger], trigger);

        // lower first gesture means clenching middle, ring, pinky fingers beyond 90 degrees
        float grip = maprange((sumbend[Finger::TYPE_MIDDLE] + sumbend[Finger::TYPE_RING] + sumbend[Finger::TYPE_PINKY]) / 3, 90.0, 180.0);
        merge(result[LowerFist], grip);

        // pinch gesture means pinching the index and thumb (distance closer than 30mm)
        float pinch = maprange(hand.pinchDistance(), 40, 30);
        merge(result[Pinch], pinch);

        // Thumbpress gesture means that the thumb points the direction of the pinky
        Vector normal = hand.palmNormal();
        Vector direction = hand.direction();
        Vector pinkyside;
        if (hand.isRight())
            pinkyside = normal.cross(direction);
        else
            pinkyside = direction.cross(normal);
        result[Thumbpress] = maprange(pinkyside.dot(fingerdir[Finger::TYPE_THUMB]), 0.0f, 0.6f);


        // *UNRELIABLE* ILY gesture means pinky and index finger extended, middle and ring finger curled up
        // Thumb doesn't matter. It's easier to point it inwards for many people.
        result[ILY] = std::min(maprange((sumbend[Finger::TYPE_PINKY] + sumbend[Finger::TYPE_INDEX]) / 2, 50.0, 40.0),
                               maprange((sumbend[Finger::TYPE_MIDDLE] + sumbend[Finger::TYPE_RING]) / 2, 120.0, 150.0));

        // *UNRELIABLE* Flipping the Bird: You know how to flip a bird.
        result[FlippingTheBird] = std::min(maprange(sumbend[Finger::TYPE_MIDDLE], 50.0, 40.0),
                                           maprange((sumbend[Finger::TYPE_INDEX] + sumbend[Finger::TYPE_RING] + sumbend[Finger::TYPE_PINKY]) / 3, 120.0, 150.0));

        // Victory gesture: make a nice V sign with your index and middle finger
        float angle = fingerdir[Finger::TYPE_INDEX].angleTo(fingerdir[Finger::TYPE_MIDDLE]);
        result[Victory] = std::min(std::min(maprange((sumbend[Finger::TYPE_INDEX] + sumbend[Finger::TYPE_MIDDLE]) / 2, 50.0, 40.0),
                                   maprange((sumbend[Finger::TYPE_PINKY] + sumbend[Finger::TYPE_RING]) / 2, 120.0, 150.0)),
                                   maprange(57.2957795f * fingerdir[Finger::TYPE_INDEX].angleTo(fingerdir[Finger::TYPE_MIDDLE]), 10.0, 20.0) );

        // FlatHand gestures
        float flatHand = maprange((sumbend[Finger::TYPE_THUMB] + sumbend[Finger::TYPE_INDEX] + sumbend[Finger::TYPE_MIDDLE] + sumbend[Finger::TYPE_RING] + sumbend[Finger::TYPE_PINKY]) / 5, 50.0, 40.0);
        Vector palmnormal = hand.palmNormal();
        result[FlatHandPalmUp]      = std::min(flatHand, maprange(( up).dot(palmnormal), 0.8f, 0.95f));
        result[FlatHandPalmDown]    = std::min(flatHand, maprange((-up).dot(palmnormal), 0.8f, 0.95f));
        result[FlatHandPalmAway]    = std::min(flatHand, maprange(( in).dot(palmnormal), 0.8f, 0.95f));
        result[FlatHandPalmTowards] = std::min(flatHand, maprange((-in).dot(palmnormal), 0.8f, 0.95f));

        // ThumbsUp/Inward gestures
        Vector inward = hand.isLeft() ? right : -right;
        float fistHand = maprange((sumbend[Finger::TYPE_INDEX] + sumbend[Finger::TYPE_MIDDLE] + sumbend[Finger::TYPE_RING] + sumbend[Finger::TYPE_PINKY]) / 5, 120.0, 150.0);
        float straightThumb = maprange(sumbend[Finger::TYPE_THUMB], 50.0, 40.0);
        result[ThumbUp]     = std::min(fistHand, std::min(straightThumb, maprange((    up).dot(fingerdir[Finger::TYPE_THUMB]), 0.8f, 0.95f)));
        result[ThumbInward] = std::min(fistHand, std::min(straightThumb, maprange((inward).dot(fingerdir[Finger::TYPE_THUMB]), 0.8f, 0.95f)));

#if 0
        fprintf(stderr, "handdir %f %f %f\n", hand.direction().x, hand.direction().y, hand.direction().z);
        fprintf(stderr, "thumbdir %f %f %f\n", fingerdir[Finger::TYPE_THUMB].x, fingerdir[Finger::TYPE_THUMB].y, fingerdir[Finger::TYPE_THUMB].z);
        fprintf(stderr, "indexdir %f %f %f\n", fingerdir[Finger::TYPE_INDEX].x, fingerdir[Finger::TYPE_INDEX].y, fingerdir[Finger::TYPE_INDEX].z);
        fprintf(stderr, "palmpos %f %f %f\n", hand.palmPosition().x, hand.palmPosition().y, hand.palmPosition().z);
        fprintf(stderr, "palmnormal %f %f %f\n", hand.palmNormal().x, hand.palmNormal().y, hand.palmNormal().z);
#endif
    }

    return success;
}
