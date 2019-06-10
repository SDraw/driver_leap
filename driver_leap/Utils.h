#pragma once

int ReadEnumVector(const std::string &f_val, const std::vector<std::string> &f_vec);
void ConvertMatrix(const vr::HmdMatrix34_t &f_matVR, glm::mat4 &f_mat);
void ConvertMatrix(const Leap::Matrix &f_leapMat, glm::mat4 &f_mat);
void ConvertQuaternion(const glm::quat &f_glmQuat, vr::HmdQuaternionf_t &f_vrQuat);
void ConvertQuaternion(const vr::HmdQuaternionf_t &f_vrQuat, glm::quat &f_glmQuat);

void ConvertVector3(const glm::vec3 &f_glmVec, vr::HmdQuaternionf_t &f_vrQuat);
void ConvertQuaternion(const vr::HmdQuaternionf_t &f_vrQuat, glm::quat& f_glmQuat);
