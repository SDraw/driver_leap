#pragma once

int ReadEnumVector(const std::string &f_val, const std::vector<std::string> &f_vec);
void ConvertMatrix(const vr::HmdMatrix34_t &f_matVR, glm::mat4 &f_mat);
void ConvertMatrix(const Leap::Matrix &f_leapMat, glm::mat4 &f_mat);
void ConvertQuaternion(const glm::quat &f_glmQuat, vr::HmdQuaternionf_t &f_vrQuat);
void ConvertQuaternion(const vr::HmdQuaternionf_t &f_vrQuat, glm::quat &f_glmQuat);
void ConvertVector3(const vr::HmdVector4_t &f_vrVec, glm::vec3 &f_glmVec);
void ConvertVector3(const glm::vec3 &f_glmVec, vr::HmdVector4_t &f_vrVec);

void SwitchBoneAxes(glm::quat &f_rot);
void FixAuxBoneTransformation(glm::vec3 &f_pos, glm::quat &f_rot);
