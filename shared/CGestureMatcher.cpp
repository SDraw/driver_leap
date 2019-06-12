#include "stdafx.h"
#include "CGestureMatcher.h"

const Leap::Vector CGestureMatcher::RightVector = Leap::Vector(-1, 0, 0);
const Leap::Vector CGestureMatcher::InVector = Leap::Vector(0, 1, 0);
const Leap::Vector CGestureMatcher::UpVector = Leap::Vector(0, 0, -1);

bool CGestureMatcher::MatchGestures(const Leap::Frame &frame, WhichHand which, std::vector<float> &result,
    const Leap::Vector& right, const Leap::Vector& in, const Leap::Vector& up)
{
    bool success = false;
    result.resize(NUM_GESTURES, 0.f);

    // Go through the hands in the dataset
    Leap::HandList &hands = frame.hands();

    for(int h = 0; h < hands.count(); h++)
    {
        Leap::Hand hand = hands[h];

        // these are the conditions under which we do not want
        // to evaluate a hand from the Frame dataset.
        if(!hand.isValid()) continue;
        if(which == RightHand && hand.isLeft()) continue;
        if(which == LeftHand && hand.isRight()) continue;

        Leap::Hand otherhand;
        if(hands.count() == 2) otherhand = hands[(h + 1) % 2];

        // okay, found a hand we want to look at.
        success = true;

        // stores the bend angles per finger and per joint
        float bends[5][3] = { { 0.f } };

        // total bend angle of a finger
        float sumbend[5] = { 0.f };

        Leap::Vector fingerdir[5] = { { 0.f, 0.f, 0.f } };
        Leap::Vector fingertip[5] = { { 0.f, 0.f, 0.f } };
        bool extended[5] = { false };

        // Evaluate bending of all fingers
        const Leap::FingerList &fingers = hand.fingers();
        for(auto fl = fingers.begin(); fl != fingers.end(); ++fl)
        {
            const Leap::Finger &finger = *fl;

            int f = finger.type(); // thumb, index, middle, ring, pinky
            if(finger.isValid())
            {
                fingertip[f] = finger.tipPosition();
                extended[f] = finger.isExtended();

                // go through the finger's bones:
                // metacarpal, proximal, intermediate, distal
                Leap::Vector prev_direction;
                for(int b = 0; b < 4; b++)
                {
                    Leap::Bone &bone = finger.bone(Leap::Bone::Type(b));
                    Leap::Vector direction = -bone.direction(); // for some reaason bone directions point handinwards?

                    if(b == Leap::Bone::TYPE_DISTAL)
                        fingerdir[f] = direction;

                    if(b > 0)
                    {
                        // get the bend angle of each finger joint
                        bends[f][b - 1] = 57.2957795f * direction.angleTo(prev_direction); // in degrees

                        // also sum up the total
                        sumbend[f] += bends[f][b - 1];
                    }
                    prev_direction = direction;
                }
            }
        }

        // trigger figure gesture means the bend angles of the upper two joints
        // of the index finger exceed 70 degrees.
        float triggerbend = bends[Leap::Finger::TYPE_INDEX][1] + bends[Leap::Finger::TYPE_INDEX][2];
        float trigger = maprange(triggerbend, 70.0, 100.0);
        merge(result[TriggerFinger], trigger);

        // lower first gesture means clenching middle, ring, pinky fingers beyond 90 degrees
        float grip = maprange((sumbend[Leap::Finger::TYPE_MIDDLE] + sumbend[Leap::Finger::TYPE_RING] + sumbend[Leap::Finger::TYPE_PINKY]) / 3, 90.0, 180.0);
        merge(result[LowerFist], grip);

        // pinch gesture means pinching the index and thumb (distance closer than 30mm)
        float pinch = maprange(hand.pinchDistance(), 40, 30);
        merge(result[Pinch], pinch);

        // Thumbpress gesture means that the thumb points the direction of the pinky
        Leap::Vector palmNormal = hand.palmNormal();
        Leap::Vector direction = hand.direction();
        Leap::Vector pinkyside;
        if(hand.isRight())
            pinkyside = palmNormal.cross(direction);
        else
            pinkyside = direction.cross(palmNormal);
        merge(result[Thumbpress], maprange(pinkyside.dot(fingerdir[Leap::Finger::TYPE_THUMB]), 0.0f, 0.6f));


        // *UNRELIABLE* ILY gesture means pinky and index finger extended, middle and ring finger curled up
        // Thumb doesn't matter. It's easier to point it inwards for many people.
        merge(result[ILY], std::min(maprange((sumbend[Leap::Finger::TYPE_PINKY] + sumbend[Leap::Finger::TYPE_INDEX]) / 2, 50.0, 40.0),
            maprange((sumbend[Leap::Finger::TYPE_MIDDLE] + sumbend[Leap::Finger::TYPE_RING]) / 2, 120.0, 150.0)));

        // *UNRELIABLE* Flipping the Bird: You know how to flip a bird.
        merge(result[FlippingTheBird], std::min(maprange(sumbend[Leap::Finger::TYPE_MIDDLE], 50.0, 40.0),
            maprange((sumbend[Leap::Finger::TYPE_INDEX] + sumbend[Leap::Finger::TYPE_RING] + sumbend[Leap::Finger::TYPE_PINKY]) / 3, 120.0, 150.0)));

        // Victory gesture: make a nice V sign with your index and middle finger
        float angle = fingerdir[Leap::Finger::TYPE_INDEX].angleTo(fingerdir[Leap::Finger::TYPE_MIDDLE]);
        merge(result[Victory], std::min(std::min(maprange((sumbend[Leap::Finger::TYPE_INDEX] + sumbend[Leap::Finger::TYPE_MIDDLE]) / 2, 50.0, 40.0),
            maprange((sumbend[Leap::Finger::TYPE_PINKY] + sumbend[Leap::Finger::TYPE_RING]) / 2, 120.0, 150.0)),
            maprange(57.2957795f * fingerdir[Leap::Finger::TYPE_INDEX].angleTo(fingerdir[Leap::Finger::TYPE_MIDDLE]), 10.0, 20.0)));

        // FlatHand gestures
        float flatHand = maprange((sumbend[Leap::Finger::TYPE_THUMB] + sumbend[Leap::Finger::TYPE_INDEX] + sumbend[Leap::Finger::TYPE_MIDDLE] + sumbend[Leap::Finger::TYPE_RING] + sumbend[Leap::Finger::TYPE_PINKY]) / 5, 50.0, 40.0);
        merge(result[FlatHandPalmUp], std::min(flatHand, maprange((up).dot(palmNormal), 0.8f, 0.95f)));
        merge(result[FlatHandPalmDown], std::min(flatHand, maprange((-up).dot(palmNormal), 0.8f, 0.95f)));
        merge(result[FlatHandPalmAway], std::min(flatHand, maprange((in).dot(palmNormal), 0.8f, 0.95f)));
        merge(result[FlatHandPalmTowards], std::min(flatHand, maprange((-in).dot(palmNormal), 0.8f, 0.95f)));

        // ThumbsUp/Inward gestures
        Leap::Vector inward = hand.isLeft() ? right : -right;
        float fistHand = maprange((sumbend[Leap::Finger::TYPE_INDEX] + sumbend[Leap::Finger::TYPE_MIDDLE] + sumbend[Leap::Finger::TYPE_RING] + sumbend[Leap::Finger::TYPE_PINKY]) / 5, 120.0, 150.0);
        float straightThumb = maprange(sumbend[Leap::Finger::TYPE_THUMB], 50.0, 40.0);
        merge(result[ThumbUp], std::min(fistHand, std::min(straightThumb, maprange((up).dot(fingerdir[Leap::Finger::TYPE_THUMB]), 0.8f, 0.95f))));
        merge(result[ThumbInward], std::min(fistHand, std::min(straightThumb, maprange((inward).dot(fingerdir[Leap::Finger::TYPE_THUMB]), 0.8f, 0.95f))));

        // VRChat gestures
        result[VRChat_Point] = (!extended[Leap::Finger::TYPE_THUMB] && extended[Leap::Finger::TYPE_INDEX] && !extended[Leap::Finger::TYPE_MIDDLE] && !extended[Leap::Finger::TYPE_RING] && !extended[Leap::Finger::TYPE_PINKY]) ? 1.f : 0.f;
        result[VRChat_Gun] = (extended[Leap::Finger::TYPE_THUMB] && extended[Leap::Finger::TYPE_INDEX] && !extended[Leap::Finger::TYPE_MIDDLE] && !extended[Leap::Finger::TYPE_RING] && !extended[Leap::Finger::TYPE_PINKY]) ? 1.f : 0.f;
        result[VRChat_SpreadHand] = (extended[Leap::Finger::TYPE_THUMB] && extended[Leap::Finger::TYPE_INDEX] && extended[Leap::Finger::TYPE_MIDDLE] && extended[Leap::Finger::TYPE_RING] && extended[Leap::Finger::TYPE_PINKY] && (hand.grabAngle() < Leap::PI / 6.f)) ? 1.f : 0.f;
        result[VRChat_ThumbsUp] = (extended[Leap::Finger::TYPE_THUMB] && !extended[Leap::Finger::TYPE_INDEX] && !extended[Leap::Finger::TYPE_MIDDLE] && !extended[Leap::Finger::TYPE_RING] && !extended[Leap::Finger::TYPE_PINKY]) ? 1.f : 0.f;
        result[VRChat_RockOut] = (!extended[Leap::Finger::TYPE_THUMB] && extended[Leap::Finger::TYPE_INDEX] && !extended[Leap::Finger::TYPE_MIDDLE] && !extended[Leap::Finger::TYPE_RING] && extended[Leap::Finger::TYPE_PINKY]) ? 1.f : 0.f;
        result[VRChat_Victory] = (!extended[Leap::Finger::TYPE_THUMB] && extended[Leap::Finger::TYPE_INDEX] && extended[Leap::Finger::TYPE_MIDDLE] && !extended[Leap::Finger::TYPE_RING] && !extended[Leap::Finger::TYPE_PINKY]) ? 1.f : 0.f;

        // Index controller
        result[IndexFinger_Index] = maprange(sumbend[Leap::Finger::TYPE_INDEX], 90.f, 180.f);
        result[IndexFinger_Middle] = maprange(sumbend[Leap::Finger::TYPE_MIDDLE], 90.f, 180.f);
        result[IndexFinger_Ring] = maprange(sumbend[Leap::Finger::TYPE_RING], 90.f, 180.f);
        result[IndexFinger_Pinky] = maprange(sumbend[Leap::Finger::TYPE_PINKY], 90.f, 180.f);

        // Find index finger on same hand
        for(Leap::Finger l_fingerA : hand.fingers())
        {
            if(l_fingerA.isValid())
            {
                if(l_fingerA.type() == Leap::Finger::TYPE_THUMB)
                {
                    for(Leap::Finger l_fingerB : hand.fingers())
                    {
                        if(l_fingerB.isValid())
                        {
                            if(l_fingerB.type() == Leap::Finger::TYPE_INDEX)
                            {
                                Leap::Vector l_vec = l_fingerA.tipPosition() - l_fingerB.tipPosition();
                                float l_length = l_vec.magnitude();
                                result[IndexButtonA] = (l_length <= 35.f) ? std::max((35.f - l_length) / 20.f, 1.f) : 0.f;
                            }
                            if(l_fingerB.type() == Leap::Finger::TYPE_PINKY)
                            {
                                Leap::Vector l_vec = l_fingerA.tipPosition() - l_fingerB.tipPosition();
                                float l_length = l_vec.magnitude();
                                result[IndexButtonB] = (l_length <= 35.f) ? std::max((35.f - l_length) / 20.f, 1.f) : 0.f;
                            }
                        }
                    }
                    break;
                }
            }
        }

        if(otherhand.isValid()) // two handed gestures really need two hands
        {
            // Timeout gesture. Note that only the lower hand forming the T shape will register the gesture.
            // TODO: might also validate that the lower hand points upward
            // TODO: might as well check that the other hand is also flat
            merge(result[Timeout], std::min(flatHand,  // I reuse the flatHand metric from above
                std::min(maprange(hand.direction().dot(-otherhand.palmNormal()), 0.8f, 0.95f),
                maprange(fingertip[Leap::Finger::TYPE_INDEX].distanceTo(otherhand.palmPosition()), 80.0f, 60.0f))
                ));

            // Touchpad emulation
            Leap::Finger otherIndex = otherhand.fingers().fingerType(Leap::Finger::TYPE_INDEX)[0];
            if(otherIndex.isValid())
            {
                // we need the index finger direction and the other hand's palm to face opposing directions
                if(otherIndex.direction().dot(hand.palmNormal()) < 0)
                {
                    // Origin o of the plane is the hand's palm position
                    Leap::Vector o = hand.palmPosition();

                    // sideways vector on the hand
                    Leap::Vector uvec = direction.cross(palmNormal) * hand.palmWidth() / 2;

                    // vvec going from palm towards the hand's pointing direction
                    Leap::Vector vvec = hand.direction() * hand.palmWidth() / 2;

                    // p and d form a line originating at the index finger's tip
                    Leap::Vector p = otherIndex.tipPosition();
                    Leap::Vector d = otherIndex.direction();

                    //  solve this linear equation system
                    //  p0       d0     o0       uvec0       vvec0
                    //  p1 + n * d1  =  o1 + u * uvec1 + v * vvec1
                    //  p2       d2     o2       uvec1       vvec2

                    //  u         u0 v0 d0       p0   o0
                    //  v =  inv( u1 v1 d1 ) * ( p1 - o1 )
                    //  n         u2 v2 d2       p2   o2

                    glm::mat3 A(uvec.x, vvec.x, d.x,
                        uvec.y, vvec.y, d.y,
                        uvec.z, vvec.z, d.z);

                    glm::mat3 invA = glm::inverse(A);
                    glm::vec3 R = glm::vec3(p.x - o.x, p.y - o.y, p.z - o.z)*invA;

                    // u and v are in R.x and R.y respectively and are expected to be within -1...+1
                    // when the index finger points into the palm area
                    // n is contained in R.z and represents the distance in mm between the index finger tip and the palm

                    // compute length of u,v vector in plane
                    glm::vec2 uv(R);
                    float length = glm::length(uv);

                    // ignore if are we pointing way out of bounds
                    if(length < 5.0)
                    {
                        // limit vector to remain inside unit circle
                        if(length > 1.0) uv /= length;
                        result[TouchpadAxisX] = uv.x;
                        result[TouchpadAxisY] = uv.y;
                    }
                }
            }

            // Index thumbstick
            for(Leap::Finger l_fingerA : hand.fingers())
            {
                if(l_fingerA.isValid())
                {
                    if(l_fingerA.type() == Leap::Finger::TYPE_THUMB)
                    {
                        // Find index finger on another hand
                        for(Leap::Finger l_fingerB : otherhand.fingers())
                        {
                            if(l_fingerB.isValid())
                            {
                                if(l_fingerB.type() == Leap::Finger::TYPE_INDEX)
                                {
                                    Leap::Vector l_vec = l_fingerA.tipPosition() - l_fingerB.tipPosition();
                                    float l_length = l_vec.magnitude();
                                    result[IndexThumbstick] = (l_length <= 35.f) ? std::max((35.f - l_length) / 20.f, 1.f) : 0.f;

                                    break;
                                }
                            }
                        }

                        break;
                    }
                }
            }
        }
    }
    return success;
}
