#include "stdafx.h"

#include "Core/CDriverConfig.h"

#include "Utils/Utils.h"

extern char g_modulePath[];

const std::vector<std::string> g_settingNames
{
    "trackingLevel", "handsReset", "velocity",
    "rootOffset", "rootAngle", "handsOffset", "handsRotationOffset"
};

enum ConfigSetting : size_t
{
    CS_TrackingLevel = 0U,
    CS_HandsReset,
    CS_Velocity,
    CS_RootOffset,
    CS_RootAngle,
    CS_HandsOffset,
    CS_HandsRotationOffset
};

const std::vector<std::string> g_trackingLevels
{
    "partial", "full"
};

unsigned char CDriverConfig::ms_trackingLevel = CDriverConfig::TL_Partial;
bool CDriverConfig::ms_handsReset = false;
bool CDriverConfig::ms_useVelocity = false;
glm::vec3 CDriverConfig::ms_rootOffset = glm::vec3(0.f);
float CDriverConfig::ms_rootAngle = 0.f;
glm::vec3 CDriverConfig::ms_handsOffset = glm::vec3(0.f);
glm::vec3 CDriverConfig::ms_handsRotationOffset = glm::vec3(0.f);

void CDriverConfig::Load()
{
    std::string l_path(g_modulePath);
    l_path.erase(l_path.begin() + l_path.rfind('\\'), l_path.end());
    l_path.append("\\..\\..\\resources\\settings.xml");

    pugi::xml_document *l_document = new pugi::xml_document();
    if(l_document->load_file(l_path.c_str()))
    {
        const pugi::xml_node l_root = l_document->child("settings");
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
                        case ConfigSetting::CS_TrackingLevel:
                        {
                            const size_t l_tableIndex = ReadEnumVector(l_attribValue.as_string(), g_trackingLevels);
                            if(l_tableIndex != std::numeric_limits<size_t>::max()) ms_trackingLevel = static_cast<unsigned char>(l_tableIndex);
                        } break;
                        case ConfigSetting::CS_HandsReset:
                            ms_handsReset = l_attribValue.as_bool(false);
                            break;
                        case ConfigSetting::CS_Velocity:
                            ms_useVelocity = l_attribValue.as_bool(false);
                            break;
                        case ConfigSetting::CS_RootOffset:
                        {
                            std::stringstream l_stream(l_attribValue.as_string("0.0 0.0 0.0"));
                            l_stream >> ms_rootOffset.x >> ms_rootOffset.y >> ms_rootOffset.z;
                        } break;
                        case ConfigSetting::CS_RootAngle:
                            ms_rootAngle = l_attribValue.as_float(0.f);
                            break;
                        case ConfigSetting::CS_HandsOffset:
                        {
                            std::stringstream l_stream(l_attribValue.as_string("0.0 0.0 0.0"));
                            l_stream >> ms_handsOffset.x >> ms_handsOffset.y >> ms_handsOffset.z;
                        } break;
                        case ConfigSetting::CS_HandsRotationOffset:
                        {
                            std::stringstream l_stream(l_attribValue.as_string("0.0 0.0 0.0"));
                            l_stream >> ms_handsRotationOffset.x >> ms_handsRotationOffset.y >> ms_handsRotationOffset.z;
                        } break;
                    }
                }
            }
        }
    }
    delete l_document;
}

unsigned char CDriverConfig::GetTrackingLevel()
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

float CDriverConfig::GetRootAngle()
{
    return ms_rootAngle;
}

const glm::vec3& CDriverConfig::GetHandsOffset()
{
    return ms_handsOffset;
}

const glm::vec3& CDriverConfig::GetHandsRotationOffset()
{
    return ms_handsRotationOffset;
}
