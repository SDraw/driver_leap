#pragma once

void ConvertMatrix(const vr::HmdMatrix34_t &p_matVR, glm::mat4 &p_mat);
template<class T, class U> void ConvertQuaternion(const T &p_quatA, U &p_quatB)
{
    p_quatB.x = p_quatA.x;
    p_quatB.y = p_quatA.y;
    p_quatB.z = p_quatA.z;
    p_quatB.w = p_quatA.w;
}
void ConvertVector3(const vr::HmdVector4_t &p_vrVec, glm::vec3 &p_glmVec);
void ConvertVector3(const glm::vec3 &p_glmVec, vr::HmdVector4_t &p_vrVec);

size_t ReadEnumVector(const std::string &p_val, const std::vector<std::string> &p_vec);
size_t ReadEnumVector(const char *p_val, const std::vector<std::string> &p_vec);
