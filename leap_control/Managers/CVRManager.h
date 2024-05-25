#pragma once

class CVRManager
{
    static CVRManager* ms_instance;

    vr::IVRSystem *m_vrSystem;

    uint32_t m_leapDevice;
    uint32_t m_leftController;
    uint32_t m_rightController;
    vr::TrackedDevicePose_t m_trackedPoses[vr::k_unMaxTrackedDeviceCount];

    glm::vec3 m_hmdPosition;
    glm::quat m_hmdRotation;
    glm::mat4 m_hmdMatrix;

    glm::vec3 m_leftHandPosition;
    glm::quat m_leftHandRotation;
    glm::mat4 m_leftHandMatrix;

    glm::vec3 m_rightHandPosition;
    glm::quat m_rightHandRotation;
    glm::mat4 m_rightHandMatrix;

    bool m_exitPolled;

    CVRManager();
    CVRManager(const CVRManager &that) = delete;
    CVRManager& operator=(const CVRManager &that) = delete;
    ~CVRManager() = default;
public:
    static CVRManager* GetInstance();

    bool Init();
    void Terminate();

    void Update();
    bool IsExitPolled() const;

    const glm::vec3& GetHmdPosition() const;
    const glm::quat& GetHmdRotation() const;
    const glm::mat4& GetHmdMatrix() const;

    const glm::vec3& GetLeftHandPosition() const;
    const glm::quat& GetLeftHandRotation() const;
    const glm::mat4& GetLeftHandMatrix() const;

    const glm::vec3& GetRightHandPosition() const;
    const glm::quat& GetRightHandRotation() const;
    const glm::mat4& GetRightHandMatrix() const;

    void SendStationMessage(const std::string &p_message);
    void SendLeftControllerMessage(const std::string &p_message);
    void SendRightControllerMessage(const std::string &p_message);
};
