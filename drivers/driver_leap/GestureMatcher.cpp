
#include "GestureMatcher.h"

const Vector GestureMatcher::RightVector = Vector(-1,  0,  0);
const Vector GestureMatcher::InVector    = Vector( 0,  1,  0);
const Vector GestureMatcher::UpVector    = Vector( 0,  0, -1);

// invert a 3x3 matrix
static void invert_3x3(const float(*A)[3], float(*R)[3]) {
    float det;

    det = A[0][0] * (A[2][2] * A[1][1] - A[2][1] * A[1][2])
        - A[1][0] * (A[2][2] * A[0][1] - A[2][1] * A[0][2])
        + A[2][0] * (A[1][2] * A[0][1] - A[1][1] * A[0][2]);

    R[0][0] = (A[2][2] * A[1][1] - A[2][1] * A[1][2]) / det;
    R[0][1] = -(A[2][2] * A[0][1] - A[2][1] * A[0][2]) / det;
    R[0][2] = (A[1][2] * A[0][1] - A[1][1] * A[0][2]) / det;

    R[1][0] = -(A[2][2] * A[1][0] - A[2][0] * A[1][2]) / det;
    R[1][1] = (A[2][2] * A[0][0] - A[2][0] * A[0][2]) / det;
    R[1][2] = -(A[1][2] * A[0][0] - A[1][0] * A[0][2]) / det;

    R[2][0] = (A[2][1] * A[1][0] - A[2][0] * A[1][1]) / det;
    R[2][1] = -(A[2][1] * A[0][0] - A[2][0] * A[0][1]) / det;
    R[2][2] = (A[1][1] * A[0][0] - A[1][0] * A[0][1]) / det;
}

// compute matrix * column vector product
static Vector matrix_vector(const float(*A)[3], Vector &v)
{
    Vector result;
    result.x = A[0][0] * v.x + A[0][1] * v.y + A[0][2] * v.z;
    result.y = A[1][0] * v.x + A[1][1] * v.y + A[1][2] * v.z;
    result.z = A[2][0] * v.x + A[2][1] * v.y + A[2][2] * v.z;
    return result;
}



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
        Hand hand = hands[h];

        // these are the conditions under which we do not want
        // to evaluate a hand from the Frame dataset.
        if (!hand.isValid()) continue;
        if (which == RightHand && hand.isLeft()) continue;
        if (which == LeftHand && hand.isRight()) continue;

        Hand otherhand;
        if (hands.count() == 2) otherhand = hands[(h + 1) % 2];

        // okay, found a hand we want to look at.
        success = true;

        // stores the bend angles per finger and per joint
        float bends[5][3] = { 0 };

        // total bend angle of a finger
        float sumbend[5] = { 0 };

        Vector fingerdir[5];
        Vector fingertip[5];
        bool extended[5];
        memset(fingerdir, 0, sizeof(fingerdir));
        memset(fingertip, 0, sizeof(fingertip));
        memset(extended, 0, sizeof(extended));

        // Evaluate bending of all fingers
        const FingerList &fingers = hand.fingers();
        for (auto fl = fingers.begin(); fl != fingers.end(); ++fl) {
            const Finger &finger = *fl;

            int f = finger.type(); // thumb, index, middle, ring, pinky
            if (finger.isFinger() && finger.isValid())
            {
                fingertip[f] = finger.tipPosition();
                extended[f] = finger.isExtended();

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
        Vector palmNormal = hand.palmNormal();
        Vector direction = hand.direction();
        Vector pinkyside;
        if (hand.isRight())
            pinkyside = palmNormal.cross(direction);
        else
            pinkyside = direction.cross(palmNormal);
        merge(result[Thumbpress], maprange(pinkyside.dot(fingerdir[Finger::TYPE_THUMB]), 0.0f, 0.6f));


        // *UNRELIABLE* ILY gesture means pinky and index finger extended, middle and ring finger curled up
        // Thumb doesn't matter. It's easier to point it inwards for many people.
        merge(result[ILY], std::min(maprange((sumbend[Finger::TYPE_PINKY] + sumbend[Finger::TYPE_INDEX]) / 2, 50.0, 40.0),
                               maprange((sumbend[Finger::TYPE_MIDDLE] + sumbend[Finger::TYPE_RING]) / 2, 120.0, 150.0)));

        // *UNRELIABLE* Flipping the Bird: You know how to flip a bird.
        merge(result[FlippingTheBird], std::min(maprange(sumbend[Finger::TYPE_MIDDLE], 50.0, 40.0),
                                        maprange((sumbend[Finger::TYPE_INDEX] + sumbend[Finger::TYPE_RING] + sumbend[Finger::TYPE_PINKY]) / 3, 120.0, 150.0)));

        // Victory gesture: make a nice V sign with your index and middle finger
        float angle = fingerdir[Finger::TYPE_INDEX].angleTo(fingerdir[Finger::TYPE_MIDDLE]);
        merge(result[Victory], std::min(std::min(maprange((sumbend[Finger::TYPE_INDEX] + sumbend[Finger::TYPE_MIDDLE]) / 2, 50.0, 40.0),
                                   maprange((sumbend[Finger::TYPE_PINKY] + sumbend[Finger::TYPE_RING]) / 2, 120.0, 150.0)),
                                   maprange(57.2957795f * fingerdir[Finger::TYPE_INDEX].angleTo(fingerdir[Finger::TYPE_MIDDLE]), 10.0, 20.0) ));

        // FlatHand gestures
        float flatHand = maprange((sumbend[Finger::TYPE_THUMB] + sumbend[Finger::TYPE_INDEX] + sumbend[Finger::TYPE_MIDDLE] + sumbend[Finger::TYPE_RING] + sumbend[Finger::TYPE_PINKY]) / 5, 50.0, 40.0);
        merge(result[FlatHandPalmUp]     , std::min(flatHand, maprange(( up).dot(palmNormal), 0.8f, 0.95f)));
        merge(result[FlatHandPalmDown]   , std::min(flatHand, maprange((-up).dot(palmNormal), 0.8f, 0.95f)));
        merge(result[FlatHandPalmAway]   , std::min(flatHand, maprange(( in).dot(palmNormal), 0.8f, 0.95f)));
        merge(result[FlatHandPalmTowards], std::min(flatHand, maprange((-in).dot(palmNormal), 0.8f, 0.95f)));

        // ThumbsUp/Inward gestures
        Vector inward = hand.isLeft() ? right : -right;
        float fistHand = maprange((sumbend[Finger::TYPE_INDEX] + sumbend[Finger::TYPE_MIDDLE] + sumbend[Finger::TYPE_RING] + sumbend[Finger::TYPE_PINKY]) / 5, 120.0, 150.0);
        float straightThumb = maprange(sumbend[Finger::TYPE_THUMB], 50.0, 40.0);
        merge(result[ThumbUp]     , std::min(fistHand, std::min(straightThumb, maprange((    up).dot(fingerdir[Finger::TYPE_THUMB]), 0.8f, 0.95f))));
        merge(result[ThumbInward] , std::min(fistHand, std::min(straightThumb, maprange((inward).dot(fingerdir[Finger::TYPE_THUMB]), 0.8f, 0.95f))));

        if (otherhand.isValid()) // two handed gestures really need two hands
        {
            // Timeout gesture. Note that only the lower hand forming the T shape will register the gesture.
            // TODO: might also validate that the lower hand points upward
            // TODO: might as well check that the other hand is also flat
            merge(result[Timeout], std::min( flatHand,  // I reuse the flatHand metric from above
                                   std::min( maprange(hand.direction().dot(-otherhand.palmNormal()), 0.8f, 0.95f),
                                             maprange(fingertip[Leap::Finger::TYPE_INDEX].distanceTo(otherhand.palmPosition()), 80.0f, 60.0f) )
                                   ));

            // Touchpad emulation
            Finger otherIndex = otherhand.fingers().fingerType(Leap::Finger::TYPE_INDEX)[0];
            if (otherIndex.isFinger() && otherIndex.isValid())
            {
                // we need the index finger direction and the other hand's palm to face opposing directions
                if (otherIndex.direction().dot(hand.palmNormal()) < 0)
                {
                    // Origin o of the plane is the hand's palm position
                    Vector o = hand.palmPosition();

                    // sideways vector on the hand
                    Vector uvec = direction.cross(palmNormal) * hand.palmWidth() / 2;

                    // vvec going from palm towards the hand's pointing direction
                    Vector vvec = hand.direction() * hand.palmWidth() / 2;

                    // p and d form a line originating at the index finger's tip
                    Vector p = otherIndex.tipPosition();
                    Vector d = otherIndex.direction();

                    //  solve this linear equation system
                    //  p0       d0     o0       uvec0       vvec0
                    //  p1 + n * d1  =  o1 + u * uvec1 + v * vvec1
                    //  p2       d2     o2       uvec1       vvec2

                    //  u         u0 v0 d0       p0   o0
                    //  v =  inv( u1 v1 d1 ) * ( p1 - o1 )
                    //  n         u2 v2 d2       p2   o2

                    float A[3][3] = { {uvec.x, vvec.x, d.x},
                                      {uvec.y, vvec.y, d.y},
                                      {uvec.z, vvec.z, d.z} };

                    float invA[3][3];
                    invert_3x3(A, invA);
                    Vector R = matrix_vector(invA, Vector(p.x - o.x, p.y - o.y, p.z - o.z));

                    // u and v are in R.x and R.y respectively and are expected to be within -1...+1
                    // when the index finger points into the palm area
                    // n is contained in R.z and represents the distance in mm between the index finger tip and the palm

                    // compute length of u,v vector in plane
                    float u = R.x;
                    float v = R.y;
                    float length = sqrtf(u*u + v*v);

                    // ignore if are we pointing way out of bounds
                    if (length < 5.0)
                    {
                        // limit vector to remain inside unit circle
                        if (length > 1.0) {
                            u /= length;
                            v /= length;
                        }
                        result[TouchpadAxisX] = u;
                        result[TouchpadAxisY] = v;
                    }
                }
            }
        }

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
