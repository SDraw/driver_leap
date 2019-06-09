#include "stdafx.h"
#include "CConfigHelper.h"
#include "Utils.h"

extern char g_moduleFileName[];

unsigned char CConfigHelper::ms_emulatedController = CConfigHelper::EmulatedController::EC_Vive;
bool CConfigHelper::ms_leftHand = true;
bool CConfigHelper::ms_rightHand = true;
float CConfigHelper::ms_offsetX = 0.f;
float CConfigHelper::ms_offsetY = 0.f;
float CConfigHelper::ms_offsetZ = 0.f;
unsigned char CConfigHelper::ms_trackingLevel = CConfigHelper::TrackingLevel::TL_Partial;
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
    "offsetX", "offsetY", "offsetZ", "trackingLevel"
    "input", "menu", "appMenu", "trigger", "grip",
    "touchpad", "touchpadTouch", "touchpadPress", "touchpadAxes",
    "buttonA", "buttonB", "thumbstick"
};
enum ConfigParamIndex
{
    CPI_EmulatedController = 0,
    CPI_LeftHand,
    CPI_RightHand,
    CPI_OffsetX,
    CPI_OffsetY,
    CPI_OffsetZ,
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
                                int l_tableIndex = ReadEnumVector(l_emulated, g_emulatedControllersTable);
                                if(l_tableIndex != -1) ms_emulatedController = static_cast<unsigned char>(l_tableIndex);
                            } break;
                            case ConfigParamIndex::CPI_LeftHand:
                                ms_leftHand = l_attribValue.as_bool(true);
                                break;
                            case ConfigParamIndex::CPI_RightHand:
                                ms_rightHand = l_attribValue.as_bool(true);
                                break;
                            case ConfigParamIndex::CPI_OffsetX:
                                ms_offsetX = l_attribValue.as_float(0.f);
                                break;
                            case ConfigParamIndex::CPI_OffsetY:
                                ms_offsetY = l_attribValue.as_float(0.f);
                                break;
                            case ConfigParamIndex::CPI_OffsetZ:
                                ms_offsetZ = l_attribValue.as_float(0.f);
                                break;
                            case ConfigParamIndex::CPI_TrackingLevel:
                            {
                                std::string l_trackingLevel = l_attribValue.as_string();
                                int l_tableIndex = ReadEnumVector(l_trackingLevel, g_trackingLevelsTable);
                                if(l_tableIndex != -1) ms_trackingLevel = static_cast<unsigned char>(l_tableIndex);
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