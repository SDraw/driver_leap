#include "stdafx.h"

#include "Core/CDriverConfig.h"

#include "Utils/Utils.h"

extern char g_modulePath[];

const std::vector<std::string> g_settingNames
{
    "skeleton", "trackingLevel",
    "handsReset", "interpolation", "velocity"
};

enum ConfigSetting : size_t
{
    CS_TrackingLevel = 0U,
    CS_HandsReset,
    CS_Interpolation,
    CS_Velocity
};

const std::vector<std::string> g_trackingLevels
{
    "partial", "full"
};

unsigned char CDriverConfig::ms_trackingLevel = CDriverConfig::TL_Partial;
bool CDriverConfig::ms_handsReset = false;
bool CDriverConfig::ms_useVelocity = false;

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
                            break;
                        case ConfigSetting::CS_Velocity:
                            ms_useVelocity = l_attribValue.as_bool(false);
                            break;
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
