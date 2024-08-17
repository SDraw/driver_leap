#pragma once

class CDriverConfig final
{
    union ParsedValue
    {
        bool m_bool;
        int m_int;
        float m_float;
        double m_double;
    };

    static int ms_trackingLevel;
    static bool ms_handsReset;
    static bool ms_useVelocity;
    static float ms_dashboardSmooth;
    static bool ms_useTriggerGrip;
    static int ms_triggerMode;
    static float ms_triggerThreshold;
    static float ms_gripThreshold;
    static glm::vec2 ms_pinchLimits;
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
        CS_DashboardSmooth,
        CS_UseTriggerGrip,
        CS_TriggerMode,
        CS_TriggerThreshold,
        CS_GripThreshold,
        CS_PinchLimitMin,
        CS_PinchLimitMax,
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
    static float GetDashboardSmooth();
    static bool IsTriggerGripUsed();
    static int GetTriggerMode();
    static float GetTriggerThreshold();
    static float GetGripThreshold();
    static const glm::vec2& GetPinchLimits();
    static bool IsControllerInputUsed();
    static const glm::vec3& GetRootOffset();
    static const glm::vec3& GetRootAngle();

    static void ProcessExternalSetting(const char *p_message);
    static void ProcessExternalSetting(const std::string &p_message);
};
