#pragma once

class CConfigHelper
{
    static unsigned char ms_emulatedController;
    static bool ms_leftHand;
    static bool ms_rightHand;
    static float ms_offsetX;
    static float ms_offsetY;
    static float ms_offsetZ;
    static unsigned char ms_trackingLevel;
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
public:
    enum EmulatedController : unsigned char
    {
        EC_Vive = 0U,
        EC_Index
    };
    enum TrackingLevel : unsigned char
    {
        TL_Partial = 0U,
        TL_Full
    };

    static void LoadConfig();

    static inline bool GetEmulatedController() { return ms_emulatedController; }
    static inline bool IsLeftHandEnabled() { return ms_leftHand; }
    static inline bool IsRightHandEnabled() { return ms_rightHand; }
    static inline float GetOffsetX() { return ms_offsetX; }
    static inline float GetOffsetY() { return ms_offsetY; }
    static inline float GetOffsetZ() { return ms_offsetZ; }
    static inline unsigned char GetTrackingLevel() { return ms_trackingLevel; }
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
};