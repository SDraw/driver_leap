#include "stdafx.h"

#include "Core/CDriverConfig.h"

#include "Utils/Utils.h"

extern std::wstring g_modulePath;

const std::vector<std::string> g_settingNames
{
    "trackingLevel",
    "handsReset",
    "useVelocity",
    "rootOffsetX",
    "rootOffsetY",
    "rootOffsetZ",
    "rootAngleX",
    "rootAngleY",
    "rootAngleZ"
};

int CDriverConfig::ms_trackingLevel = CDriverConfig::TL_Partial;
bool CDriverConfig::ms_handsReset = false;
bool CDriverConfig::ms_useVelocity = false;
glm::vec3 CDriverConfig::ms_rootOffset = glm::vec3(0.f);
glm::vec3 CDriverConfig::ms_rootAngle = glm::vec3(0.f);

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
                            ms_trackingLevel = glm::clamp(l_attribValue.as_int(0), static_cast<int>(TL_Partial), static_cast<int>(TL_Full));
                            break;

                        case CS_HandsReset:
                            ms_handsReset = l_attribValue.as_bool(false);
                            break;

                        case CS_UseVelocity:
                            ms_useVelocity = l_attribValue.as_bool(false);
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
            switch(l_index)
            {
                case CS_HandsReset:
                {
                    int l_value = -1;
                    if(TryParse(l_chunks[1U], l_value))
                        ms_handsReset = (l_value == 1);
                } break;

                case CS_UseVelocity:
                {
                    int l_value = -1;
                    if(TryParse(l_chunks[1U], l_value))
                        ms_useVelocity = (l_value == 1);
                } break;

                case CS_RootAngleX:
                {
                    float l_value = 0.f;
                    if(TryParse(l_chunks[1U], l_value))
                        ms_rootAngle.x = l_value;
                } break;

                case CS_RootAngleY:
                {
                    float l_value = 0.f;
                    if(TryParse(l_chunks[1U], l_value))
                        ms_rootAngle.y = l_value;
                } break;

                case CS_RootAngleZ:
                {
                    float l_value = 0.f;
                    if(TryParse(l_chunks[1U], l_value))
                        ms_rootAngle.z = l_value;
                } break;

                case CS_RootOffsetX:
                {
                    float l_value = 0.f;
                    if(TryParse(l_chunks[1U], l_value))
                        ms_rootOffset.x = l_value;
                } break;

                case CS_RootOffsetY:
                {
                    float l_value = 0.f;
                    if(TryParse(l_chunks[1U], l_value))
                        ms_rootOffset.y = l_value;
                } break;

                case CS_RootOffsetZ:
                {
                    float l_value = 0.f;
                    if(TryParse(l_chunks[1U], l_value))
                        ms_rootOffset.z = l_value;
                } break;
            }
        }
    }
}
