#pragma once

class CDriverConfig final
{
    static int ms_trackingLevel;
    static bool ms_handsReset;
    static bool ms_useVelocity;
    static glm::vec3 ms_rootOffset;
    static glm::vec3 ms_rootAngle;

    CDriverConfig() = delete;
    ~CDriverConfig() = delete;
    CDriverConfig(const CDriverConfig &that) = delete;
    CDriverConfig& operator=(const CDriverConfig &that) = delete;
public:
    enum ConfigSetting : size_t
    {
        CS_TrackingLevel = 0U,
        CS_HandsReset,
        CS_UseVelocity,
        CS_RootOffsetX,
        CS_RootOffsetY,
        CS_RootOffsetZ,
        CS_RootAngleX,
        CS_RootAngleY,
        CS_RootAngleZ
    };

    enum TrackingLevel : int
    {
        TL_Partial = 0U,
        TL_Full
    };

    static void Load();

    static int GetTrackingLevel();
    static bool IsHandsResetEnabled();
    static bool IsVelocityUsed();
    static const glm::vec3& GetRootOffset();
    static const glm::vec3& GetRootAngle();

    static void ProcessExternalSetting(const char *p_message);
};
