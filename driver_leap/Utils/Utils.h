#pragma once

void ConvertMatrix(const vr::HmdMatrix34_t &p_matVR, glm::mat4 &p_mat);
template<class T, class U> void ConvertQuaternion(const T &p_src, U &p_dst)
{
    p_dst.x = p_src.x;
    p_dst.y = p_src.y;
    p_dst.z = p_src.z;
    p_dst.w = p_src.w;
}
void ConvertVector3(const vr::HmdVector4_t &p_vrVec, glm::vec3 &p_glmVec);
void ConvertVector3(const glm::vec3 &p_glmVec, vr::HmdVector4_t &p_vrVec);

size_t ReadEnumVector(const std::string &p_val, const std::vector<std::string> &p_vec);
size_t ReadEnumVector(const char *p_val, const std::vector<std::string> &p_vec);

bool ReadEnumVector(const std::string &p_val, const std::vector<std::string> &p_vec, size_t &p_index);
bool ReadEnumVector(const char *p_val, const std::vector<std::string> &p_vec, size_t &p_index);

void SplitString(const std::string& p_text, const char p_separator, std::vector<std::string>& p_result);
void SplitString(const char *p_text, const char p_separator, std::vector<std::string>& p_result);
float NormalizeRange(float p_val, float p_min, float p_max);

bool TryParse(const std::string &p_string, int &p_value);
bool TryParse(const std::string &p_string, float &p_value);
