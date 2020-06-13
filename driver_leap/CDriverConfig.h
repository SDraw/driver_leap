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

    static void LoadConfig();

    static inline unsigned char GetEmulatedController() { return ms_emulatedController; }
    static inline bool IsLeftHandEnabled() { return ms_leftHand; }
    static inline bool IsRightHandEnabled() { return ms_rightHand; }
    static inline unsigned char GetOrientationMode() { return ms_orientation; }
    static inline bool IsSkeletonEnabled() { return ms_skeleton; }
    static inline unsigned char GetTrackingLevel() { return ms_trackingLevel; }

    static inline const glm::vec3& GetDesktopOffset() { return ms_desktopOffset; }
    static inline const glm::vec3& GetLeftHandOffset() { return ms_leftHandOffset; }
    static inline const glm::quat& GetLeftHandOffsetRotation() { return ms_leftHandOffsetRotation; }
    static inline const glm::vec3& GetRightHandOffset() { return ms_rightHandOffset; }
    static inline const glm::quat& GetRightHandOffsetRotation() { return ms_rightHandOffsetRotation; }

    static inline bool IsInputEnabled() { return ms_input; }
    static inline bool IsMenuEnabled() { return ms_menu; }
    static inline bool IsApplicationMenuEnabled() { return ms_applicationMenu; }
    static inline bool IsTriggerEnabled() { return ms_trigger; }
    static inline bool IsGripEnabled() { return ms_grip; }
    static inline bool IsTouchpadEnabled() { return ms_touchpad; }
    static inline bool IsTouchpadTouchEnabled() { return ms_touchpadTouch; }
    static inline bool IsTouchpadPressEnabled() { return ms_touchpadPress; }
    static inline bool IsTouchpadAxesEnabled() { return ms_touchpadAxes; }
    static inline bool IsButtonAEnabled() { return ms_buttonA; }
    static inline bool IsButtonBEnabled() { return ms_buttonB; }
    static inline bool IsThumbstickEnabled() { return ms_thumbstick; }
    static inline bool IsHandsResetEnabled() { return ms_handsReset; }
};
