#pragma once

class CSettingsManager
{
    static CSettingsManager* ms_instance;

    int m_trackingLevel;
    bool m_handsReset;
    bool m_useVelocity;
    bool m_showOverlays;
    glm::vec3 m_rootOffset;
    glm::vec3 m_rootAngle;
    glm::vec3 m_overlayOffset;
    glm::vec3 m_overlayAngle;
    bool m_startMinimized;
    float m_overlaySize;
    bool m_useControllerInput;

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
        ST_ShowOverlays,
        ST_RootOffsetX,
        ST_RootOffsetY,
        ST_RootOffsetZ,
        ST_RootAngleX,
        ST_RootAngleY,
        ST_RootAngleZ,
        ST_OverlayOffsetX,
        ST_OverlayOffsetY,
        ST_OverlayOffsetZ,
        ST_OverlayAngleX,
        ST_OverlayAngleY,
        ST_OverlayAngleZ,
        ST_StartMinimized,
        ST_OverlaySize,
        ST_UseControllerInput,

        Count
    };

    static CSettingsManager* GetInstance();

    void Load();
    void Save();

    int GetTrackingLevel() const;
    bool GetHandsReset() const;
    bool GetUseVelocity() const;
    bool GetShowOverlays() const;
    bool GetStartMinimized() const;
    const glm::vec3& GetRootOffset() const;
    const glm::vec3& GetRootAngle() const;
    const glm::vec3& GetOverlayOffset() const;
    const glm::vec3& GetOverlayAngle() const;
    float GetOverlaySize() const;
    bool GetUseControllerInput() const;

    void SetSetting(SettingType p_setting, int p_value);
    void SetSetting(SettingType p_setting, bool p_value);
    void SetSetting(SettingType p_setting, float p_value);
};

