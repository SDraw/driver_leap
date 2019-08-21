#include "stdafx.h"
#include "CConfigHelper.h"
#include "Utils.h"

extern char g_moduleFileName[];

unsigned char CConfigHelper::ms_emulatedController = CConfigHelper::EC_Vive;
bool CConfigHelper::ms_leftHand = true;
bool CConfigHelper::ms_rightHand = true;
unsigned char CConfigHelper::ms_orientation = CConfigHelper::OM_HMD;
float CConfigHelper::ms_desktopRootX = 0.f;
float CConfigHelper::ms_desktopRootY = 0.f;
float CConfigHelper::ms_desktopRootZ = 0.f;
float CConfigHelper::ms_rotationOffsetX = 0.f;
float CConfigHelper::ms_rotationOffsetY = 0.f;
float CConfigHelper::ms_rotationOffsetZ = 0.f;
bool CConfigHelper::ms_skeleton = true;
unsigned char CConfigHelper::ms_trackingLevel = CConfigHelper::TL_Partial;
bool CConfigHelper::ms_input = true;
bool CConfigHelper::ms_menu = true;
bool CConfigHelper::ms_applicationMenu = true;
bool CConfigHelper::ms_trigger = true;
bool CConfigHelper::ms_grip = true;
bool CConfigHelper::ms_touchpad = true;
bool CConfigHelper::ms_touchpadTouch = true;
bool CConfigHelper::ms_touchpadPress = true;
bool CConfigHelper::ms_touchpadAxes = true;
bool CConfigHelper::ms_buttonA = true;
bool CConfigHelper::ms_buttonB = true;
bool CConfigHelper::ms_thumbstick = true;

const std::vector<std::string> g_configAttributeTable
{
    "emulated_controller", "leftHand", "rightHand",
    "orientation", "desktopRoot", "rotationOffset",
    "skeleton", "trackingLevel",
    "input", "menu", "appMenu", "trigger", "grip",
    "touchpad", "touchpadTouch", "touchpadPress", "touchpadAxes",
    "buttonA", "buttonB", "thumbstick"
};
enum ConfigParamIndex : size_t
{
    CPI_EmulatedController = 0U,
    CPI_LeftHand,
    CPI_RightHand,
    CPI_Orientation,
    CPI_DesktopRoot,
    CPI_RotationOffset,
    CPI_Skeleton,
    CPI_TrackingLevel,
    CPI_Input,
    CPI_Menu,
    CPI_ApplicationMenu,
    CPI_Trigger,
    CPI_Grip,
    CPI_Touchpad,
    CPI_TouchpadTouch,
    CPI_TouchpadPress,
    CPI_TouchpadAxes,
    CPI_ButtonA,
    CPI_ButtonB,
    CPI_Thumbstick
};
const std::vector<std::string> g_orientationMode
{
    "hmd", "desktop"
};
const std::vector<std::string> g_emulatedControllersTable
{
    "vive", "index"
};
const std::vector<std::string> g_trackingLevelsTable
{
    "partial", "full"
};

void CConfigHelper::LoadConfig()
{
    std::string l_path(g_moduleFileName);
    l_path.erase(l_path.begin() + l_path.rfind('\\'), l_path.end());
    l_path.append("\\..\\..\\cfg\\control_config.xml");

    pugi::xml_document *l_config = new pugi::xml_document();
    if(l_config->load_file(l_path.c_str()))
    {
        pugi::xml_node l_configRoot = l_config->child("config");
        if(l_configRoot)
        {
            for(pugi::xml_node l_node = l_configRoot.child("param"); l_node; l_node = l_node.next_sibling("param"))
            {
                pugi::xml_attribute l_attribName = l_node.attribute("name");
                if(l_attribName)
                {
                    std::string l_param(l_attribName.as_string());
                    pugi::xml_attribute l_attribValue = l_node.attribute("value");
                    if(l_attribValue)
                    {
                        switch(ReadEnumVector(l_param, g_configAttributeTable))
                        {
                            case ConfigParamIndex::CPI_EmulatedController:
                            {
                                std::string l_emulated = l_attribValue.as_string();
                                size_t l_tableIndex = ReadEnumVector(l_emulated, g_emulatedControllersTable);
                                if(l_tableIndex != std::numeric_limits<size_t>::max()) ms_emulatedController = static_cast<unsigned char>(l_tableIndex);
                            } break;
                            case ConfigParamIndex::CPI_LeftHand:
                                ms_leftHand = l_attribValue.as_bool(true);
                                break;
                            case ConfigParamIndex::CPI_RightHand:
                                ms_rightHand = l_attribValue.as_bool(true);
                                break;
                            case ConfigParamIndex::CPI_Orientation:
                            {
                                std::string l_orientation = l_attribValue.as_string();
                                size_t l_tableIndex = ReadEnumVector(l_orientation, g_orientationMode);
                                if(l_tableIndex != std::numeric_limits<size_t>::max()) ms_orientation = static_cast<unsigned char>(l_tableIndex);
                            } break;
                            case ConfigParamIndex::CPI_DesktopRoot:
                            {
                                std::stringstream l_root(l_attribValue.as_string());
                                l_root >> ms_desktopRootX >> ms_desktopRootY >> ms_desktopRootZ;
                            } break;
                            case ConfigParamIndex::CPI_RotationOffset:
                            {
                                std::stringstream l_offset(l_attribValue.as_string());
                                l_offset >> ms_rotationOffsetX >> ms_rotationOffsetY >> ms_rotationOffsetZ;
                            } break;
                            case ConfigParamIndex::CPI_Skeleton:
                                ms_skeleton = l_attribValue.as_bool(true);
                                break;
                            case ConfigParamIndex::CPI_TrackingLevel:
                            {
                                std::string l_trackingLevel = l_attribValue.as_string();
                                size_t l_tableIndex = ReadEnumVector(l_trackingLevel, g_trackingLevelsTable);
                                if(l_tableIndex != std::numeric_limits<size_t>::max()) ms_trackingLevel = static_cast<unsigned char>(l_tableIndex);
                            } break;
                            case ConfigParamIndex::CPI_Input:
                                ms_input = l_attribValue.as_bool(true);
                                break;
                            case ConfigParamIndex::CPI_Menu:
                                ms_menu = l_attribValue.as_bool(true);
                                break;
                            case ConfigParamIndex::CPI_ApplicationMenu:
                                ms_applicationMenu = l_attribValue.as_bool(true);
                                break;
                            case ConfigParamIndex::CPI_Trigger:
                                ms_trigger = l_attribValue.as_bool(true);
                                break;
                            case ConfigParamIndex::CPI_Grip:
                                ms_grip = l_attribValue.as_bool(true);
                                break;
                            case ConfigParamIndex::CPI_Touchpad:
                                ms_touchpad = l_attribValue.as_bool(true);
                                break;
                            case ConfigParamIndex::CPI_TouchpadTouch:
                                ms_touchpadTouch = l_attribValue.as_bool(true);
                                break;
                            case ConfigParamIndex::CPI_TouchpadPress:
                                ms_touchpadPress = l_attribValue.as_bool(true);
                                break;
                            case ConfigParamIndex::CPI_TouchpadAxes:
                                ms_touchpadAxes = l_attribValue.as_bool(true);
                                break;
                            case ConfigParamIndex::CPI_ButtonA:
                                ms_buttonA = l_attribValue.as_bool(true);
                                break;
                            case ConfigParamIndex::CPI_ButtonB:
                                ms_buttonB = l_attribValue.as_bool(true);
                                break;
                            case ConfigParamIndex::CPI_Thumbstick:
                                ms_thumbstick = l_attribValue.as_bool(true);
                                break;
                        }
                    }
                }
            }
        }
    }
    delete l_config;
}