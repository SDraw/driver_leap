#pragma once

class CLeapHand final
{
    bool m_isLeft;
    bool m_visible;
    glm::vec3 m_position;
    glm::quat m_rotation;
    glm::vec3 m_velocity;
    std::array<glm::vec3, 20U> m_bonesPositions;
    std::array<glm::quat, 20U> m_bonesRotations;
    std::array<float, 5U> m_fingersBends;
    float m_grabValue;
public:
    enum Finger : size_t
    {
        Thumb = 0U,
        Index,
        Middle,
        Ring,
        Pinky
    };
    enum FingerBone : size_t
    {
        Metacarpal = 0U,
        Proximal,
        Intermediate,
        Distal
    };

    explicit CLeapHand(bool p_left);
    ~CLeapHand() = default;

    bool IsLeft() const;
    bool IsVisible() const;
    const glm::vec3& GetPosition() const;
    const glm::quat& GetRotation() const;
    const glm::vec3& GetVelocity() const;

    const glm::vec3& GetFingerBonePosition(size_t p_finger, size_t p_bone) const;
    const glm::quat& GetFingerBoneRotation(size_t p_finger, size_t p_bone) const;

    void GetFingerBoneLocalPosition(size_t p_finger, size_t p_bone, glm::vec3 &l_result) const;
    void GetFingerBoneLocalRotation(size_t p_finger, size_t p_bone, glm::quat &l_result) const;
	void GetThumbBoneLocalRotation(size_t p_finger, size_t p_bone, glm::quat &l_result) const;

    float GetFingerBend(size_t p_finger) const;
    float GetGrabValue() const;

    void Update(const LEAP_HAND &p_hand);
    void Update();

    static void ConvertPosition(const LEAP_VECTOR &p_src, glm::vec3 &p_dst);
    static void ConvertRotation(const LEAP_QUATERNION &p_src, glm::quat &p_dst);
};

