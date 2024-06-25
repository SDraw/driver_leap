#pragma once

class CSettingsManager
{
    static CSettingsManager* ms_instance;

    int m_trackingLevel;
    bool m_handsReset;
    bool m_useVelocity;
    float m_dashboardSmooth;
    bool m_startMinimized;
    bool m_useTriggerGrip;
    int m_triggerMode;
    float m_triggerThreshold;
    float m_gripThreshold;
    glm::vec2 m_pinchLimits;
    bool m_useControllerInput;
    glm::vec3 m_rootOffset;
    glm::vec3 m_rootAngle;
    bool m_showOverlays;
    glm::vec3 m_overlayOffset;
    glm::vec3 m_overlayAngle;
    float m_overlaySize;

    CSettingsManager();
    CSettingsManager(const CSettingsManager &that) = delete;
    CSettingsManager& operator=(const CSettingsManager &that) = delete;
    ~CSettingsManager() = default;
public:
    enum SettingType : size_t
    {
        ST_TrackingLevel = 0U,
        ST_HandsReset,
        ST_UseVelocity,
        ST_DashboardSmooth,
        ST_StartMinimized,
        ST_UseTriggerGrip,
        ST_TriggerMode,
        ST_TriggerThreshold,
        ST_GripThreshold,
        ST_PinchLimitMin,
        ST_PinchLimitMax,
        ST_UseControllerInput,
        ST_RootOffsetX,
        ST_RootOffsetY,
        ST_RootOffsetZ,
        ST_RootAngleX,
        ST_RootAngleY,
        ST_RootAngleZ,
        ST_ShowOverlays,
        ST_OverlaySize,
        ST_OverlayOffsetX,
        ST_OverlayOffsetY,
        ST_OverlayOffsetZ,
        ST_OverlayAngleX,
        ST_OverlayAngleY,
        ST_OverlayAngleZ,
        
        Count
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

    static CSettingsManager* GetInstance();

    void Load();
    void Save();

    int GetTrackingLevel() const;
    bool GetHandsReset() const;
    bool GetUseVelocity() const;
    float GetDashboardSmooth() const;
    bool GetUseTriggerGrip() const;
    int GetTriggerMode() const;
    float GetTriggerThreshold() const;
    float GetGripThreshold() const;
    const glm::vec2& GetPinchLimits() const;
    bool GetUseControllerInput() const;
    bool GetStartMinimized() const;
    const glm::vec3& GetRootOffset() const;
    const glm::vec3& GetRootAngle() const;
    bool GetShowOverlays() const;
    float GetOverlaySize() const;
    const glm::vec3& GetOverlayOffset() const;
    const glm::vec3& GetOverlayAngle() const;

    void SetSetting(SettingType p_setting, int p_value);
    void SetSetting(SettingType p_setting, bool p_value);
    void SetSetting(SettingType p_setting, float p_value);
};

