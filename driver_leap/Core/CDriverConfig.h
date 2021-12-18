#pragma once

class CDriverConfig final
{
    static unsigned char ms_trackingLevel;
    static bool ms_handsReset;
    static bool ms_interpolation;
    static bool ms_useVelocity;

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
    static bool IsInterpolationEnabled();
    static bool IsVelocityUsed();
};
