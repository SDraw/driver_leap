#pragma once

void ConvertMatrix(const vr::HmdMatrix34_t &f_matVR, glm::mat4 &f_mat);
void ConvertQuaternion(const glm::quat &f_glmQuat, vr::HmdQuaternionf_t &f_vrQuat);
void ConvertQuaternion(const vr::HmdQuaternionf_t &f_vrQuat, glm::quat &f_glmQuat);
void ConvertVector3(const vr::HmdVector4_t &f_vrVec, glm::vec3 &f_glmVec);
void ConvertVector3(const glm::vec3 &f_glmVec, vr::HmdVector4_t &f_vrVec);

void SwitchBoneAxes(glm::quat &f_rot);
void FixAuxTransformation(glm::vec3 &f_pos, glm::quat &f_rot);

size_t ReadEnumVector(const std::string &f_val, const std::vector<std::string> &f_vec);
size_t ReadEnumVector(const char *f_val, const std::vector<std::string> &f_vec);
