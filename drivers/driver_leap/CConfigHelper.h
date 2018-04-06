#pragma once

class CConfigHelper
{
    static bool ms_menu;
    static bool ms_applicationMenu;
    static bool ms_trigger;
    static bool ms_grip;
    static bool ms_touchpad;
    static bool ms_touchpadAxes;
public:
    static void LoadConfig();
    static inline bool IsMenuEnabled() { return ms_menu; }
    static inline bool IsApplicationMenuEnabled() { return ms_applicationMenu; }
    static inline bool IsTriggerEnabled() { return ms_trigger; }
    static inline bool IsGripEnabled() { return ms_grip; }
    static inline bool IsTouchpadEnabled() { return ms_touchpad; }
    static inline bool IsTouchpadAxesEnabled() { return ms_touchpadAxes; }
};