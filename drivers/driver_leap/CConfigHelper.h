#pragma once

class CConfigHelper
{
    static bool ms_menu;
    static bool ms_applicationMenu;
    static bool ms_trigger;
    static bool ms_grip;
    static bool ms_touchpad;
    static bool ms_touchpadTouch;
    static bool ms_touchpadPress;
    static bool ms_touchpadAxes;
    static float ms_gripOffsetX;
    static float ms_gripOffsetY;
    static float ms_gripOffsetZ;
public:
    static void LoadConfig();
    static inline bool IsMenuEnabled() { return ms_menu; }
    static inline bool IsApplicationMenuEnabled() { return ms_applicationMenu; }
    static inline bool IsTriggerEnabled() { return ms_trigger; }
    static inline bool IsGripEnabled() { return ms_grip; }
    static inline bool IsTouchpadEnabled() { return ms_touchpad; }
    static inline bool IsTouchpadTouchEnabled() { return ms_touchpadTouch; }
    static inline bool IsTouchpadPressEnabled() { return ms_touchpadPress; }
    static inline bool IsTouchpadAxesEnabled() { return ms_touchpadAxes; }
    static inline float GetGripOffsetX() { return ms_gripOffsetX; }
    static inline float GetGripOffsetY() { return ms_gripOffsetY; }
    static inline float GetGripOffsetZ() { return ms_gripOffsetZ; }
};