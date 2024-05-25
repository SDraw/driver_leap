#include "stdafx.h"
#include "Utils/Utils.h"

size_t ReadEnumVector(const std::string& p_val, const std::vector<std::string>& p_vec)
{
    size_t l_result = std::numeric_limits<size_t>::max();
    for(auto iter = p_vec.begin(), iterEnd = p_vec.end(); iter != iterEnd; ++iter)
    {
        if(!iter->compare(p_val))
        {
            l_result = std::distance(p_vec.begin(), iter);
            break;
        }
    }
    return l_result;
}

size_t ReadEnumVector(const char* p_val, const std::vector<std::string>& p_vec)
{
    size_t l_result = std::numeric_limits<size_t>::max();
    for(auto iter = p_vec.begin(), iterEnd = p_vec.end(); iter != iterEnd; ++iter)
    {
        if(!iter->compare(p_val))
        {
            l_result = std::distance(p_vec.begin(), iter);
            break;
        }
    }
    return l_result;
}

void ConvertMatrix(const vr::HmdMatrix34_t& p_matVR, glm::mat4& p_mat)
{
    for(int i = 0; i < 4; i++)
    {
        for(int j = 0; j < 3; j++) p_mat[i][j] = p_matVR.m[j][i];
    }
    for(int i = 0; i < 3; i++) p_mat[i][3] = 0.f;
    p_mat[3][3] = 1.f;
}
void ConvertMatrix(const glm::mat4& p_mat, vr::HmdMatrix34_t& p_matVR)
{
    for(int i = 0; i < 4; i++)
    {
        for(int j = 0; j < 3; j++) p_matVR.m[j][i] = p_mat[i][j];
    }
}

bool IsInRange(float p_value, float p_min, float p_max)
{
    return ((p_value >= p_min) && (p_value <= p_max));
}

float ProgressLerp(int p_progress, float p_min, float p_max)
{
    return glm::mix(p_min, p_max, static_cast<float>(p_progress) * 0.01f);
}
int InvProgressLerp(float p_value, float p_min, float p_max)
{
    return glm::clamp(static_cast<int>(((p_value - p_min) / (p_max - p_min)) *100.f), 0, 100);
}
