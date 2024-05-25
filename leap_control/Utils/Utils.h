#pragma once

size_t ReadEnumVector(const std::string &p_val, const std::vector<std::string> &p_vec);
size_t ReadEnumVector(const char* p_val, const std::vector<std::string>& p_vec);
void ConvertMatrix(const vr::HmdMatrix34_t &p_matVR, glm::mat4 &p_mat);
void ConvertMatrix(const glm::mat4& p_mat, vr::HmdMatrix34_t& p_matVR);
bool IsInRange(float p_value, float p_min, float p_max);
float ProgressLerp(int p_progress, float p_min, float p_max);
int InvProgressLerp(float p_value, float p_min, float p_max);
