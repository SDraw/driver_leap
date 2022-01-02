#include "stdafx.h"

#include "Utils/CGestureMatcher.h"

const float g_pi = glm::pi<float>();
const float g_piHalf = g_pi * 0.5f;
const float g_piQuarter = g_pi * 0.25f;
extern const glm::mat4 g_identityMatrix;

void CGestureMatcher::GetGestures(const LEAP_HAND *p_hand, std::vector<float> &p_result)
{
    p_result.resize(HG_Count, 0.f);

    // Finger bends
    float l_fingerBend[5U] = { 0.f };
    for(size_t i = 0U; i < 5U; i++)
    {
        const LEAP_DIGIT &l_finger = p_hand->digits[i];
        glm::vec3 l_prevDirection;
        const size_t l_startBoneIndex = ((i == 0U) ? 1U : 0U);
        for(size_t j = l_startBoneIndex; j < 4U; j++)
        {
            const LEAP_BONE &l_bone = l_finger.bones[j];
            glm::vec3 l_direction(l_bone.next_joint.x - l_bone.prev_joint.x, l_bone.next_joint.y - l_bone.prev_joint.y, l_bone.next_joint.z - l_bone.prev_joint.z);
            l_direction = glm::normalize(l_direction);
            if(j > l_startBoneIndex) l_fingerBend[i] += glm::acos(glm::dot(l_direction, l_prevDirection));
            l_prevDirection = l_direction;
        }
    }

    for(size_t i = 0U; i < 5U; i++) p_result[HG_ThumbBend + i] = NormalizeRange(l_fingerBend[i], (i == 0U) ? 0.f : g_piQuarter, (i == 0U) ? g_piQuarter : g_pi);

    // Simple gestures
    p_result[HG_Trigger] = p_result[HG_IndexBend];
    p_result[HG_Grab] = NormalizeRange((l_fingerBend[2U] + l_fingerBend[3U] + l_fingerBend[4U]) / 3.f, g_piHalf, g_pi);
}

float CGestureMatcher::NormalizeRange(float p_val, float p_min, float p_max)
{
    const float l_mapped = (p_val - p_min) / (p_max - p_min);
    return glm::clamp(l_mapped, 0.f, 1.f);
}
