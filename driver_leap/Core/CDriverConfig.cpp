#include "stdafx.h"
#include "Core/CDriverConfig.h"
#include "Utils/Utils.h"

extern std::wstring g_modulePath;

const std::vector<std::string> g_settingNames
{
    "trackingLevel",
    "handsReset",
    "useVelocity",
    "dashboardSmooth",
    "useTriggerGrip",
    "triggerMode",
    "triggerThreshold",
    "gripThreshold",
    "pinchLimitMin",
    "pinchLimitMax",
    "useControllerInput",
    "rootOffsetX",
    "rootOffsetY",
    "rootOffsetZ",
    "rootAngleX",
    "rootAngleY",
    "rootAngleZ"
};

int CDriverConfig::ms_trackingLevel = CDriverConfig::TL_Full;
bool CDriverConfig::ms_handsReset = false;
bool CDriverConfig::ms_useVelocity = false;
float CDriverConfig::ms_dashboardSmooth = 1.f;
bool CDriverConfig::ms_useTriggerGrip = true;
int CDriverConfig::ms_triggerMode = CDriverConfig::TM_FingerBend;
float CDriverConfig::ms_triggerThreshold = 0.75f;
float CDriverConfig::ms_gripThreshold = 0.75f;
glm::vec2 CDriverConfig::ms_pinchLimits(0.02f, 0.05f);
bool CDriverConfig::ms_useControllerInput = false;
glm::vec3 CDriverConfig::ms_rootOffset(0.f);
glm::vec3 CDriverConfig::ms_rootAngle(0.f);

void CDriverConfig::Load()
{
    std::wstring l_path(g_modulePath);
    l_path.append(L"\\..\\..\\resources\\settings.xml");

    pugi::xml_document l_document;
    if(l_document.load_file(l_path.c_str()))
    {
        const pugi::xml_node l_root = l_document.child("settings");
        if(l_root)
        {
            for(pugi::xml_node l_node = l_root.child("setting"); l_node; l_node = l_node.next_sibling("setting"))
            {
                const pugi::xml_attribute l_attribName = l_node.attribute("name");
                const pugi::xml_attribute l_attribValue = l_node.attribute("value");
                if(l_attribName && l_attribValue)
                {
                    switch(ReadEnumVector(l_attribName.as_string(), g_settingNames))
                    {
                        case CS_TrackingLevel:
                            ms_trackingLevel = glm::clamp<int>(l_attribValue.as_int(TL_Full), TL_Partial, TL_Full);
                            break;

                        case CS_HandsReset:
                            ms_handsReset = l_attribValue.as_bool(false);
                            break;

                        case CS_UseVelocity:
                            ms_useVelocity = l_attribValue.as_bool(false);
                            break;

                        case CS_DashboardSmooth:
                            ms_dashboardSmooth = glm::clamp(l_attribValue.as_float(1.f), 0.01f, 1.f);
                            break;

                        case CS_UseTriggerGrip:
                            ms_useTriggerGrip = l_attribValue.as_bool(true);
                            break;

                        case CS_TriggerMode:
                            ms_triggerMode = glm::clamp<int>(l_attribValue.as_int(TM_FingerBend), TM_FingerBend, TM_Pinch);
                            break;

                        case CS_TriggerThreshold:
                            ms_triggerThreshold = glm::clamp(l_attribValue.as_float(0.75f), 0.1f, 1.f);
                            break;

                        case CS_GripThreshold:
                            ms_gripThreshold = glm::clamp(l_attribValue.as_float(0.75f), 0.1f, 1.f);
                            break;

                        case CS_PinchLimitMin:
                            ms_pinchLimits.x = glm::clamp(l_attribValue.as_float(0.02f), 0.01f, 0.1f);
                            break;

                        case CS_PinchLimitMax:
                            ms_pinchLimits.y = glm::clamp(l_attribValue.as_float(0.05f), 0.01f, 0.1f);
                            break;

                        case CS_UseControllerInput:
                            ms_useControllerInput = l_attribValue.as_bool(false);
                            break;

                        case CS_RootOffsetX:
                            ms_rootOffset.x = glm::clamp(l_attribValue.as_float(0.f), -1.f, 1.f);
                            break;

                        case CS_RootOffsetY:
                            ms_rootOffset.y = glm::clamp(l_attribValue.as_float(0.f), -1.f, 1.f);
                            break;

                        case CS_RootOffsetZ:
                            ms_rootOffset.z = glm::clamp(l_attribValue.as_float(0.f), -1.f, 1.f);
                            break;

                        case CS_RootAngleX:
                            ms_rootAngle.x = glm::clamp(l_attribValue.as_float(0.f), -180.f, 180.f);
                            break;

                        case CS_RootAngleY:
                            ms_rootAngle.y = glm::clamp(l_attribValue.as_float(0.f), -180.f, 180.f);
                            break;

                        case CS_RootAngleZ:
                            ms_rootAngle.z = glm::clamp(l_attribValue.as_float(0.f), -180.f, 180.f);
                            break;
                    }
                }
            }
        }
    }
}

int CDriverConfig::GetTrackingLevel()
{
    return ms_trackingLevel;
}

bool CDriverConfig::IsHandsResetEnabled()
{
    return ms_handsReset;
}

bool CDriverConfig::IsVelocityUsed()
{
    return ms_useVelocity;
}

float CDriverConfig::GetDashboardSmooth()
{
    return ms_dashboardSmooth;
}

bool CDriverConfig::IsTriggerGripUsed()
{
    return ms_useTriggerGrip;
}

int CDriverConfig::GetTriggerMode()
{
    return ms_triggerMode;
}

float CDriverConfig::GetTriggerThreshold()
{
    return ms_triggerThreshold;
}

float CDriverConfig::GetGripThreshold()
{
    return ms_gripThreshold;
}

const glm::vec2& CDriverConfig::GetPinchLimits()
{
    return ms_pinchLimits;
}

bool CDriverConfig::IsControllerInputUsed()
{
    return ms_useControllerInput;
}

const glm::vec3& CDriverConfig::GetRootOffset()
{
    return ms_rootOffset;
}

const glm::vec3& CDriverConfig::GetRootAngle()
{
    return ms_rootAngle;
}

void CDriverConfig::ProcessExternalSetting(const char *p_message)
{
    // Message format: "{setting} {value}"
    std::vector<std::string> l_chunks;
    SplitString(p_message, ' ', l_chunks);

    if(l_chunks.size() >= 2U)
    {
        size_t l_index = 0U;
        if(ReadEnumVector(l_chunks[0U], g_settingNames, l_index))
        {
            ParsedValue l_value;
            switch(l_index)
            {
                case CS_HandsReset:
                {
                    if(TryParse(l_chunks[1U], l_value.m_int))
                        ms_handsReset = (l_value.m_int == 1);
                } break;

                case CS_UseVelocity:
                {
                    if(TryParse(l_chunks[1U], l_value.m_int))
                        ms_useVelocity = (l_value.m_int == 1);
                } break;

                case CS_RootAngleX:
                {
                    if(TryParse(l_chunks[1U], l_value.m_float))
                        ms_rootAngle.x = glm::clamp(l_value.m_float, -180.f, 180.f);
                } break;

                case CS_RootAngleY:
                {
                    if(TryParse(l_chunks[1U], l_value.m_float))
                        ms_rootAngle.y = glm::clamp(l_value.m_float, -180.f, 180.f);
                } break;

                case CS_RootAngleZ:
                {
                    if(TryParse(l_chunks[1U], l_value.m_float))
                        ms_rootAngle.z = glm::clamp(l_value.m_float, -180.f, 180.f);
                } break;

                case CS_RootOffsetX:
                {
                    if(TryParse(l_chunks[1U], l_value.m_float))
                        ms_rootOffset.x = glm::clamp(l_value.m_float, -1.f, 1.f);
                } break;

                case CS_RootOffsetY:
                {
                    if(TryParse(l_chunks[1U], l_value.m_float))
                        ms_rootOffset.y = glm::clamp(l_value.m_float, -1.f, 1.f);
                } break;

                case CS_RootOffsetZ:
                {
                    if(TryParse(l_chunks[1U], l_value.m_float))
                        ms_rootOffset.z = glm::clamp(l_value.m_float, -1.f, 1.f);
                } break;

                case CS_UseControllerInput:
                {
                    if(TryParse(l_chunks[1U], l_value.m_int))
                        ms_useControllerInput = (l_value.m_int == 1);
                } break;

                case CS_UseTriggerGrip:
                {
                    if(TryParse(l_chunks[1U], l_value.m_int))
                        ms_useTriggerGrip = (l_value.m_int == 1);
                } break;

                case CS_TriggerMode:
                {
                    if(TryParse(l_chunks[1U], l_value.m_int))
                        ms_triggerMode = glm::clamp<int>(l_value.m_int, TL_Partial, TL_Full);
                } break;

                case CS_TriggerThreshold:
                {
                    if(TryParse(l_chunks[1U], l_value.m_float))
                        ms_triggerThreshold = glm::clamp(l_value.m_float, 0.1f, 1.f);
                } break;

                case CS_GripThreshold:
                {
                    if(TryParse(l_chunks[1U], l_value.m_float))
                        ms_triggerThreshold = glm::clamp(l_value.m_float, 0.1f, 1.f);
                } break;

                case CS_PinchLimitMin:
                {
                    if(TryParse(l_chunks[1U], l_value.m_float))
                        ms_pinchLimits.x = glm::clamp(l_value.m_float, 0.01f, 0.1f);
                } break;

                case CS_PinchLimitMax:
                {
                    if(TryParse(l_chunks[1U], l_value.m_float))
                        ms_pinchLimits.y = glm::clamp(l_value.m_float, 0.01f, 0.1f);
                } break;

                case CS_DashboardSmooth:
                {
                    if(TryParse(l_chunks[1U], l_value.m_float))
                        ms_dashboardSmooth = glm::clamp(l_value.m_float, 0.01f, 1.f);
                } break;
            }
        }
    }
}
