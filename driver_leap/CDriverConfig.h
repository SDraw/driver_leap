#pragma once

class CDriverConfig final
{
    static unsigned char ms_emulatedController;
    static bool ms_leftHand;
    static bool ms_rightHand;
    static unsigned char ms_orientation;
    static bool ms_skeleton;
    static unsigned char ms_trackingLevel;
    static glm::vec3 ms_desktopOffset;
    static glm::vec3 ms_leftHandOffset;
    static glm::quat ms_leftHandOffsetRotation;
    static glm::vec3 ms_rightHandOffset;
    static glm::quat ms_rightHandOffsetRotation;
    static bool ms_input;
    static bool ms_menu;
    static bool ms_applicationMenu;
    static bool ms_trigger;
    static bool ms_grip;
    static bool ms_touchpad;
    static bool ms_touchpadTouch;
    static bool ms_touchpadPress;
    static bool ms_touchpadAxes;
    static bool ms_buttonA;
    static bool ms_buttonB;
    static bool ms_thumbstick;
    static bool ms_handsReset;

    CDriverConfig() = delete;
    ~CDriverConfig() = delete;
    CDriverConfig(const CDriverConfig &that) = delete;
    CDriverConfig& operator=(const CDriverConfig &that) = delete;
public:
    enum EmulatedController : unsigned char
    {
        EC_Vive = 0U,
        EC_Index
    };
    enum OrientationMode : unsigned char
    {
        OM_HMD = 0U,
        OM_Desktop
    };
    enum TrackingLevel : unsigned char
    {
        TL_Partial = 0U,
        TL_Full
    };

    static void Load();

    static unsigned char GetEmulatedController();
    static bool IsLeftHandEnabled();
    static bool IsRightHandEnabled();
    static unsigned char GetOrientationMode();
    static bool IsSkeletonEnabled();
    static unsigned char GetTrackingLevel();

    static const glm::vec3& GetDesktopOffset();
    static const glm::vec3& GetLeftHandOffset();
    static const glm::quat& GetLeftHandOffsetRotation();
    static const glm::vec3& GetRightHandOffset();
    static const glm::quat& GetRightHandOffsetRotation();

    static bool IsInputEnabled();
    static bool IsMenuEnabled();
    static bool IsApplicationMenuEnabled();
    static bool IsTriggerEnabled();
    static bool IsGripEnabled();
    static bool IsTouchpadEnabled();
    static bool IsTouchpadTouchEnabled();
    static bool IsTouchpadPressEnabled();
    static bool IsTouchpadAxesEnabled();
    static bool IsButtonAEnabled();
    static bool IsButtonBEnabled();
    static bool IsThumbstickEnabled();

    static bool IsHandsResetEnabled();
};
