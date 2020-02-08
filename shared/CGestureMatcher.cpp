#include "LeapC++.h"
#include "glm/glm.hpp"

#include "CGestureMatcher.h"

const Leap::Vector g_InVector = Leap::Vector(0, 1, 0);
const Leap::Vector g_RightVector = Leap::Vector(-1, 0, 0);
const Leap::Vector g_UpVector = Leap::Vector(0, 0, -1);

struct FingerData
{
    float m_bend;
    float m_bends[3U];
    Leap::Vector m_direction;
    Leap::Vector m_tipPosition;
    bool m_extended;
    FingerData()
    {
        m_bend = 0.f;
        for(size_t i = 0U; i < 3U; i++) m_bends[i] = 0.f;
        m_extended = false;
    }
};

bool CGestureMatcher::GetGestures(const Leap::Frame &f_frame, GestureHand f_which, std::vector<float> &f_result)
{
    bool l_result = false;
    f_result.resize(GT_GesturesCount, 0.f);

    Leap::HandList l_hands = f_frame.hands();
    for(auto l_hand : l_hands)
    {
        if(l_hand.isValid())
        {
            if((l_hand.isLeft() == (f_which == GH_LeftHand)) || (l_hand.isRight() == (f_which == GH_RightHand)))
            {
                FingerData l_fingerData[5U];
                const Leap::FingerList l_fingers = l_hand.fingers();
                for(auto l_finger : l_fingers)
                {
                    if(l_finger.isValid())
                    {
                        size_t l_index = static_cast<size_t>(l_finger.type());

                        l_fingerData[l_index].m_tipPosition = l_finger.tipPosition();
                        l_fingerData[l_index].m_extended = l_finger.isExtended();

                        Leap::Vector l_prevDirection;
                        for(int i = 0; i < 4; i++)
                        {
                            Leap::Bone l_bone = l_finger.bone(static_cast<Leap::Bone::Type>(i));
                            Leap::Vector l_direction = -l_bone.direction();

                            if(i == Leap::Bone::TYPE_DISTAL) l_fingerData[l_index].m_direction = l_direction;
                            if(i > 0)
                            {
                                l_fingerData[l_index].m_bends[i - 1] = 57.2957795f * l_direction.angleTo(l_prevDirection);
                                l_fingerData[l_index].m_bend += l_fingerData[l_index].m_bends[i - 1];
                            }
                            l_prevDirection = l_direction;
                        }
                    }
                }

                // Trigger
                float l_triggerBend = l_fingerData[Leap::Finger::TYPE_INDEX].m_bends[1U] + l_fingerData[Leap::Finger::TYPE_INDEX].m_bends[2U];
                float l_trigger = MapRange(l_triggerBend, 70.f, 100.f);
                Merge(f_result[GT_TriggerFinger], l_trigger);

                // Lower fist / grip
                float l_grip = MapRange((l_fingerData[Leap::Finger::TYPE_MIDDLE].m_bend + l_fingerData[Leap::Finger::TYPE_RING].m_bend + l_fingerData[Leap::Finger::TYPE_PINKY].m_bend) / 3.f, 90.f, 180.f);
                Merge(f_result[GT_LowerFist], l_grip);

                // Pinch
                float l_pinch = MapRange(l_hand.pinchDistance(), 40.f, 30.f);
                Merge(f_result[GT_Pinch], l_pinch);

                // Thumb press
                Leap::Vector l_palmNormal = l_hand.palmNormal();
                Leap::Vector l_direction = l_hand.direction();
                Leap::Vector l_pinkyside;
                if(f_which == GH_RightHand) l_pinkyside = l_palmNormal.cross(l_direction);
                else l_pinkyside = l_direction.cross(l_palmNormal);
                Merge(f_result[GT_Thumbpress], 1.f - MapRange(l_pinkyside.dot(l_fingerData[Leap::Finger::TYPE_THUMB].m_direction), 0.0f, 0.6f));

                // Victory
                Merge(f_result[GT_Victory], std::min(std::min(MapRange((l_fingerData[Leap::Finger::TYPE_INDEX].m_bend + l_fingerData[Leap::Finger::TYPE_MIDDLE].m_bend) / 2.f, 50.f, 40.f),
                    MapRange((l_fingerData[Leap::Finger::TYPE_PINKY].m_bend + l_fingerData[Leap::Finger::TYPE_RING].m_bend) / 2.f, 120.f, 150.f)),
                    MapRange(57.2957795f * l_fingerData[Leap::Finger::TYPE_INDEX].m_direction.angleTo(l_fingerData[Leap::Finger::TYPE_MIDDLE].m_direction), 10.f, 20.f)));

                // Flat hand gestures
                float l_flatHand = MapRange((l_fingerData[Leap::Finger::TYPE_THUMB].m_bend + l_fingerData[Leap::Finger::TYPE_INDEX].m_bend + l_fingerData[Leap::Finger::TYPE_MIDDLE].m_bend + l_fingerData[Leap::Finger::TYPE_RING].m_bend + l_fingerData[Leap::Finger::TYPE_PINKY].m_bend) / 5.f, 50.f, 40.f);
                Merge(f_result[GT_FlatHandPalmUp], std::min(l_flatHand, MapRange((g_UpVector).dot(l_palmNormal), 0.8f, 0.95f)));
                Merge(f_result[GT_FlatHandPalmDown], std::min(l_flatHand, MapRange((-g_UpVector).dot(l_palmNormal), 0.8f, 0.95f)));
                Merge(f_result[GT_FlatHandPalmAway], std::min(l_flatHand, MapRange((g_InVector).dot(l_palmNormal), 0.8f, 0.95f)));
                Merge(f_result[GT_FlatHandPalmTowards], std::min(l_flatHand, MapRange((-g_InVector).dot(l_palmNormal), 0.8f, 0.95f)));

                // ThumbsUp/Inward gestures (seems broken in new LeapSDK)
                Leap::Vector l_inward = ((f_which == GH_LeftHand) ? g_RightVector : -g_RightVector);
                float l_fistHand = MapRange((l_fingerData[Leap::Finger::TYPE_INDEX].m_bend + l_fingerData[Leap::Finger::TYPE_MIDDLE].m_bend + l_fingerData[Leap::Finger::TYPE_RING].m_bend + l_fingerData[Leap::Finger::TYPE_PINKY].m_bend) / 5.f, 120.f, 150.f);
                float l_straightThumb = MapRange(l_fingerData[Leap::Finger::TYPE_THUMB].m_bend, 50.f, 40.f);
                Merge(f_result[GT_ThumbUp], std::min(l_fistHand, std::min(l_straightThumb, MapRange((g_UpVector).dot(l_fingerData[Leap::Finger::TYPE_THUMB].m_direction), 0.8f, 0.95f))));
                Merge(f_result[GT_ThumbInward], std::min(l_fistHand, std::min(l_straightThumb, MapRange((l_inward).dot(l_fingerData[Leap::Finger::TYPE_THUMB].m_direction), 0.8f, 0.95f))));

                // VRChat gestures
                f_result[GT_VRChatPoint] = (!l_fingerData[Leap::Finger::TYPE_THUMB].m_extended && l_fingerData[Leap::Finger::TYPE_INDEX].m_extended&& !l_fingerData[Leap::Finger::TYPE_MIDDLE].m_extended && !l_fingerData[Leap::Finger::TYPE_RING].m_extended && !l_fingerData[Leap::Finger::TYPE_PINKY].m_extended) ? 1.f : 0.f;
                f_result[GT_VRChatGun] = (l_fingerData[Leap::Finger::TYPE_THUMB].m_extended&& l_fingerData[Leap::Finger::TYPE_INDEX].m_extended&& !l_fingerData[Leap::Finger::TYPE_MIDDLE].m_extended && !l_fingerData[Leap::Finger::TYPE_RING].m_extended && !l_fingerData[Leap::Finger::TYPE_PINKY].m_extended) ? 1.f : 0.f;
                f_result[GT_VRChatSpreadHand] = (l_fingerData[Leap::Finger::TYPE_THUMB].m_extended&& l_fingerData[Leap::Finger::TYPE_INDEX].m_extended&& l_fingerData[Leap::Finger::TYPE_MIDDLE].m_extended && l_fingerData[Leap::Finger::TYPE_RING].m_extended && l_fingerData[Leap::Finger::TYPE_PINKY].m_extended && (l_hand.grabAngle() < Leap::PI / 6.f)) ? 1.f : 0.f;
                f_result[GT_VRChatThumbsUp] = (l_fingerData[Leap::Finger::TYPE_THUMB].m_extended&& !l_fingerData[Leap::Finger::TYPE_INDEX].m_extended&& !l_fingerData[Leap::Finger::TYPE_MIDDLE].m_extended && !l_fingerData[Leap::Finger::TYPE_RING].m_extended && !l_fingerData[Leap::Finger::TYPE_PINKY].m_extended) ? 1.f : 0.f;
                f_result[GT_VRChatRockOut] = (!l_fingerData[Leap::Finger::TYPE_THUMB].m_extended&& l_fingerData[Leap::Finger::TYPE_INDEX].m_extended&& !l_fingerData[Leap::Finger::TYPE_MIDDLE].m_extended && !l_fingerData[Leap::Finger::TYPE_RING].m_extended && l_fingerData[Leap::Finger::TYPE_PINKY].m_extended) ? 1.f : 0.f;
                f_result[GT_VRChatVictory] = (!l_fingerData[Leap::Finger::TYPE_THUMB].m_extended&& l_fingerData[Leap::Finger::TYPE_INDEX].m_extended&& l_fingerData[Leap::Finger::TYPE_MIDDLE].m_extended && !l_fingerData[Leap::Finger::TYPE_RING].m_extended && !l_fingerData[Leap::Finger::TYPE_PINKY].m_extended) ? 1.f : 0.f;

                // Finger bends
                f_result[GT_IndexFingerBend] = MapRange(l_fingerData[Leap::Finger::TYPE_INDEX].m_bend, 90.f, 180.f);
                f_result[GT_MiddleFingerBend] = MapRange(l_fingerData[Leap::Finger::TYPE_MIDDLE].m_bend, 90.f, 180.f);
                f_result[GT_RingFingerBend] = MapRange(l_fingerData[Leap::Finger::TYPE_RING].m_bend, 90.f, 180.f);
                f_result[GT_PinkyFingerBend] = MapRange(l_fingerData[Leap::Finger::TYPE_PINKY].m_bend, 90.f, 180.f);

                float l_length = l_fingerData[Leap::Finger::TYPE_THUMB].m_tipPosition.distanceTo(l_fingerData[Leap::Finger::TYPE_MIDDLE].m_tipPosition);
                f_result[GT_ThumbMiddleTouch] = (l_length <= 35.f) ? std::min((35.f - l_length) / 20.f, 1.f) : 0.f;
                l_length = l_fingerData[Leap::Finger::TYPE_THUMB].m_tipPosition.distanceTo(l_fingerData[Leap::Finger::TYPE_PINKY].m_tipPosition);
                f_result[GT_ThumbPinkyTouch] = (l_length <= 35.f) ? std::min((35.f - l_length) / 20.f, 1.f) : 0.f;

                // Two-handed gestures
                for(auto l_otherHand : l_hands)
                {
                    if(l_otherHand.isValid())
                    {
                        if(((f_which == GH_LeftHand) && l_otherHand.isRight()) || ((f_which == GH_RightHand) && l_otherHand.isLeft()))
                        {
                            Merge(f_result[GT_Timeout], std::min(l_flatHand,  // I reuse the flatHand metric from above
                                std::min(MapRange(l_direction.dot(-l_otherHand.palmNormal()), 0.8f, 0.95f),
                                MapRange(l_fingerData[Leap::Finger::TYPE_INDEX].m_tipPosition.distanceTo(l_otherHand.palmPosition()), 80.0f, 60.0f))
                                ));

                            Leap::FingerList l_otherFingers = l_otherHand.fingers();
                            for(auto l_otherFinger : l_otherFingers)
                            {
                                if(l_otherFinger.isValid())
                                {
                                    // Touchpad emulation
                                    if(l_otherFinger.type() == Leap::Finger::TYPE_INDEX)
                                    {
                                        if(l_otherFinger.direction().dot(l_palmNormal) < 0)
                                        {
                                            Leap::Vector l_uVec = l_direction.cross(l_palmNormal) * (l_hand.palmWidth() / 2.f);
                                            Leap::Vector l_vVec = l_direction * (l_hand.palmWidth() / 2.f);
                                            Leap::Vector l_path = l_otherFinger.tipPosition() - l_hand.palmPosition();
                                            Leap::Vector l_otherFingerDir = l_otherFinger.direction();

                                            glm::mat3 l_matrix(l_uVec.x, l_vVec.x, l_otherFingerDir.x,
                                                l_uVec.y, l_vVec.y, l_otherFingerDir.y,
                                                l_uVec.z, l_vVec.z, l_otherFingerDir.z);
                                            glm::vec2 l_uv = l_path.toVector3<glm::vec3>()*glm::inverse(l_matrix);
                                            l_length = glm::length(l_uv);
                                            if(l_length < 5.f)
                                            {
                                                if(l_length > 1.f) l_uv /= l_length;
                                                f_result[GT_TouchpadAxisX] = l_uv.x;
                                                f_result[GT_TouchpadAxisY] = l_uv.y;
                                            }
                                        }

                                        l_length = l_fingerData[Leap::Finger::TYPE_THUMB].m_tipPosition.distanceTo(l_otherFinger.tipPosition());
                                        f_result[GT_ThumbIndexCrossTouch] = (l_length <= 35.f) ? std::max((35.f - l_length) / 20.f, 1.f) : 0.f;

                                        break;
                                    }
                                }
                            }

                            break;
                        }
                    }
                }

                l_result = true;
                break;
            }
        }
    }
    return l_result;
}

float CGestureMatcher::MapRange(float input, float minimum, float maximum)
{
    float mapped = (input - minimum) / (maximum - minimum);
    return std::max(std::min(mapped, 1.0f), 0.0f);
}
void CGestureMatcher::Merge(float &result, float value)
{
    result = std::max(result, value);
}

void CGestureMatcher::GetGestureName(GestureType f_gesture, std::string &f_name)
{
    switch(f_gesture)
    {
        case GT_TriggerFinger: f_name.assign("TriggerFinger"); break;
        case GT_LowerFist: f_name.assign("LowerFist"); break;
        case GT_Pinch: f_name.assign("Pinch"); break;
        case GT_Thumbpress: f_name.assign("Thumbpress"); break;
        case GT_Victory: f_name.assign("Victory"); break;
        case GT_ThumbUp: f_name.assign("ThumbUp"); break;
        case GT_ThumbInward: f_name.assign("ThumbInward"); break;
        case GT_ThumbMiddleTouch: f_name.assign("ThumbMiddleTouch"); break;
        case GT_ThumbPinkyTouch: f_name.assign("ThumbPinkyTouch"); break;
        case GT_FlatHandPalmUp: f_name.assign("FlatHandPalmUp"); break;
        case GT_FlatHandPalmDown: f_name.assign("FlatHandPalmDown"); break;
        case GT_FlatHandPalmAway: f_name.assign("FlatHandPalmAway"); break;
        case GT_FlatHandPalmTowards: f_name.assign("FlatHandPalmTowards"); break;
        case GT_Timeout: f_name.assign("Timeout"); break;
        case GT_TouchpadAxisX: f_name.assign("TouchpadAxisX"); break;
        case GT_TouchpadAxisY: f_name.assign("TouchpadAxisY"); break;
        case GT_ThumbIndexCrossTouch: f_name.assign("ThumbIndexCrossTouch"); break;
        case GT_VRChatGun: f_name.assign("VRChatGun"); break;
        case GT_VRChatPoint: f_name.assign("VRChatPoint"); break;
        case GT_VRChatRockOut: f_name.assign("VRChatRockOut"); break;
        case GT_VRChatSpreadHand: f_name.assign("VRChatSpreadHand"); break;
        case GT_VRChatThumbsUp: f_name.assign("VRChatThumbsUp"); break;
        case GT_VRChatVictory: f_name.assign("VRChatVictory"); break;
        case GT_IndexFingerBend: f_name.assign("IndexFingerBend"); break;
        case GT_MiddleFingerBend: f_name.assign("MiddleFingerBend"); break;
        case GT_RingFingerBend: f_name.assign("RingFingerBend"); break;
        case GT_PinkyFingerBend: f_name.assign("PinkyFingerBend"); break;
    }
}
