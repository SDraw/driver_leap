#pragma once

class CDriverConfig final
{
    static unsigned char ms_trackingLevel;
    static bool ms_handsReset;
    static bool ms_useVelocity;
    static glm::vec3 ms_rootOffset;
    static float ms_rootAngle;
    static glm::vec3 ms_handsOffset;
    static glm::vec3 ms_handsRotationOffset;

    CDriverConfig() = delete;
    ~CDriverConfig() = delete;
    CDriverConfig(const CDriverConfig &that) = delete;
    CDriverConfig& operator=(const CDriverConfig &that) = delete;
public:
    enum TrackingLevel : unsigned char
    {
        TL_Partial = 0U,
        TL_Full
    };

    static void Load();

    static unsigned char GetTrackingLevel();
    static bool IsHandsResetEnabled();
    static bool IsVelocityUsed();

    static const glm::vec3& GetRootOffset();
    static float GetRootAngle();
    static const glm::vec3& GetHandsOffset();
    static const glm::vec3& GetHandsRotationOffset();
};
