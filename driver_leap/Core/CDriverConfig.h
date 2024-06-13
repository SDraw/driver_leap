#pragma once

class CDriverConfig final
{
    static int ms_trackingLevel;
    static bool ms_handsReset;
    static bool ms_useVelocity;
    static bool ms_useTriggerGrip;
    static int ms_triggerMode;
    static bool ms_useControllerInput;
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
        CS_UseTriggerGrip,
        CS_TriggerMode,
        CS_UseControllerInput,
        CS_RootOffsetX,
        CS_RootOffsetY,
        CS_RootOffsetZ,
        CS_RootAngleX,
        CS_RootAngleY,
        CS_RootAngleZ
    };

    enum TrackingLevel : int
    {
        TL_Partial = 0,
        TL_Full
    };

    enum TriggerMode : int
    {
        TM_FingerBend = 0,
        TM_Pinch
    };

    static void Load();

    static int GetTrackingLevel();
    static bool IsHandsResetEnabled();
    static bool IsVelocityUsed();
    static bool IsTriggerGripUsed();
    static int GetTriggerMode();
    static bool IsControllerInputUsed();
    static const glm::vec3& GetRootOffset();
    static const glm::vec3& GetRootAngle();

    static void ProcessExternalSetting(const char *p_message);
};
