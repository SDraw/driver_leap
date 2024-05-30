#include "stdafx.h"
#include "Managers/CSettingsManager.h"
#include "Utils/Utils.h"

const std::vector<std::string> g_settingNames
{
    "trackingLevel",
    "handsReset",
    "useVelocity",
    "showOverlays",
    "rootOffsetX",
    "rootOffsetY",
    "rootOffsetZ",
    "rootAngleX",
    "rootAngleY",
    "rootAngleZ",
    "overlayOffsetX",
    "overlayOffsetY",
    "overlayOffsetZ",
    "overlayAngleX",
    "overlayAngleY",
    "overlayAngleZ",
    "startMinimized",
    "overlaySize"
};

CSettingsManager* CSettingsManager::ms_instance = nullptr;

CSettingsManager::CSettingsManager()
{
    m_trackingLevel = 0;
    m_handsReset = false;
    m_useVelocity = false;
    m_showOverlays = true;
    m_rootOffset = glm::vec3(0.f);
    m_rootAngle = glm::vec3(0.f);
    m_overlayOffset = glm::vec3(0.f);
    m_overlayAngle = glm::vec3(0.f);
    m_startMinimized = false;
    m_overlaySize = 0.128f;
}

CSettingsManager* CSettingsManager::GetInstance()
{
    if(!ms_instance)
        ms_instance = new CSettingsManager();

    return ms_instance;
}

void CSettingsManager::Load()
{
    std::wstring l_path(QApplication::applicationDirPath().toStdWString());
    l_path.append(L"/../../../resources/settings.xml");

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
                        case ST_TrackingLevel:
                            m_trackingLevel = std::abs(l_attribValue.as_int(0)) % 2;
                            break;

                        case ST_HandsReset:
                            m_handsReset = l_attribValue.as_bool(false);
                            break;

                        case ST_UseVelocity:
                            m_useVelocity = l_attribValue.as_bool(false);
                            break;

                        case ST_ShowOverlays:
                            m_showOverlays = l_attribValue.as_bool(true);
                            break;

                        case ST_RootOffsetX:
                            m_rootOffset.x = glm::clamp(l_attribValue.as_float(0.f), -1.f, 1.f);
                            break;

                        case ST_RootOffsetY:
                            m_rootOffset.y = glm::clamp(l_attribValue.as_float(0.f), -1.f, 1.f);
                            break;

                        case ST_RootOffsetZ:
                            m_rootOffset.z = glm::clamp(l_attribValue.as_float(0.f), -1.f, 1.f);
                            break;

                        case ST_RootAngleX:
                            m_rootAngle.x = glm::clamp(l_attribValue.as_float(0.f), -180.f, 180.f);
                            break;

                        case ST_RootAngleY:
                            m_rootAngle.y = glm::clamp(l_attribValue.as_float(0.f), -180.f, 180.f);
                            break;

                        case ST_RootAngleZ:
                            m_rootAngle.z = glm::clamp(l_attribValue.as_float(0.f), -180.f, 180.f);
                            break;

                        case ST_OverlayOffsetX:
                            m_overlayOffset.x = glm::clamp(l_attribValue.as_float(0.f), -0.5f, 0.5f);
                            break;

                        case ST_OverlayOffsetY:
                            m_overlayOffset.y = glm::clamp(l_attribValue.as_float(0.f), -0.5f, 0.5f);
                            break;

                        case ST_OverlayOffsetZ:
                            m_overlayOffset.z = glm::clamp(l_attribValue.as_float(0.f), -0.5f, 0.5f);
                            break;

                        case ST_OverlayAngleX:
                            m_overlayAngle.x = glm::clamp(l_attribValue.as_float(0.f), -180.f, 180.f);
                            break;

                        case ST_OverlayAngleY:
                            m_overlayAngle.y = glm::clamp(l_attribValue.as_float(0.f), -180.f, 180.f);
                            break;

                        case ST_OverlayAngleZ:
                            m_overlayAngle.z = glm::clamp(l_attribValue.as_float(0.f), -180.f, 180.f);
                            break;

                        case ST_StartMinimized:
                            m_startMinimized = l_attribValue.as_bool(false);
                            break;

                        case ST_OverlaySize:
                            m_overlaySize = glm::clamp(l_attribValue.as_float(0.128f), 0.1f, 0.5f);
                            break;
                    }
                }
            }
        }
    }
}

void CSettingsManager::Save()
{
    std::wstring l_path(QApplication::applicationDirPath().toStdWString());
    l_path.append(L"/../../../resources/settings.xml");

    pugi::xml_document l_document;
    auto l_root = l_document.append_child("settings");
    for(size_t i = 0U; i < SettingType::Count; i++)
    {
        auto l_node = l_root.append_child("setting");
        auto l_nameAttrib = l_node.append_attribute("name");
        l_nameAttrib.set_value(g_settingNames[i].c_str());

        auto l_valueAttrib = l_node.append_attribute("value");
        switch(i)
        {
            case ST_TrackingLevel:
                l_valueAttrib.set_value(m_trackingLevel);
                break;

            case ST_HandsReset:
                l_valueAttrib.set_value(m_handsReset);
                break;

            case ST_UseVelocity:
                l_valueAttrib.set_value(m_useVelocity);
                break;

            case ST_ShowOverlays:
                l_valueAttrib.set_value(m_showOverlays);
                break;

            case ST_RootOffsetX:
                l_valueAttrib.set_value(m_rootOffset.x);
                break;

            case ST_RootOffsetY:
                l_valueAttrib.set_value(m_rootOffset.y);
                break;

            case ST_RootOffsetZ:
                l_valueAttrib.set_value(m_rootOffset.z);
                break;

            case ST_RootAngleX:
                l_valueAttrib.set_value(m_rootAngle.x);
                break;

            case ST_RootAngleY:
                l_valueAttrib.set_value(m_rootAngle.y);
                break;

            case ST_RootAngleZ:
                l_valueAttrib.set_value(m_rootAngle.z);
                break;

            case ST_OverlayOffsetX:
                l_valueAttrib.set_value(m_overlayOffset.x);
                break;

            case ST_OverlayOffsetY:
                l_valueAttrib.set_value(m_overlayOffset.y);
                break;

            case ST_OverlayOffsetZ:
                l_valueAttrib.set_value(m_overlayOffset.z);
                break;

            case ST_OverlayAngleX:
                l_valueAttrib.set_value(m_overlayAngle.x);
                break;

            case ST_OverlayAngleY:
                l_valueAttrib.set_value(m_overlayAngle.y);
                break;

            case ST_OverlayAngleZ:
                l_valueAttrib.set_value(m_overlayAngle.z);
                break;

            case ST_StartMinimized:
                l_valueAttrib.set_value(m_startMinimized);
                break;

            case ST_OverlaySize:
                l_valueAttrib.set_value(m_overlaySize);
                break;
        }
    }

    l_document.save_file(l_path.c_str());
}

int CSettingsManager::GetTrackingLevel() const
{
    return m_trackingLevel;
}

bool CSettingsManager::GetHandsReset() const
{
    return m_handsReset;
}

bool CSettingsManager::GetUseVelocity() const
{
    return m_useVelocity;
}

bool CSettingsManager::GetShowOverlays() const
{
    return m_showOverlays;
}

bool CSettingsManager::GetStartMinimized() const
{
    return m_startMinimized;
}

const glm::vec3 & CSettingsManager::GetRootOffset() const
{
    return m_rootOffset;
}

const glm::vec3& CSettingsManager::GetRootAngle() const
{
    return m_rootAngle;
}

const glm::vec3 & CSettingsManager::GetOverlayOffset() const
{
    return m_overlayOffset;
}

const glm::vec3 & CSettingsManager::GetOverlayAngle() const
{
    return m_overlayAngle;
}

float CSettingsManager::GetOverlaySize() const
{
    return m_overlaySize;
}

void CSettingsManager::SetSetting(SettingType p_setting, int p_value)
{
    switch(p_setting)
    {
        case ST_TrackingLevel:
            m_trackingLevel = std::abs(p_value) % 2;
            break;
    }
}
void CSettingsManager::SetSetting(SettingType p_setting, bool p_value)
{
    switch(p_setting)
    {
        case ST_HandsReset:
            m_handsReset = p_value;
            break;

        case ST_UseVelocity:
            m_useVelocity = p_value;
            break;

        case ST_ShowOverlays:
            m_showOverlays = p_value;
            break;

        case ST_StartMinimized:
            m_startMinimized = p_value;
            break;
    }
}
void CSettingsManager::SetSetting(SettingType p_setting, float p_value)
{
    switch(p_setting)
    {
        case ST_RootOffsetX:
            m_rootOffset.x = glm::clamp(p_value, -1.f, 1.f);
            break;

        case ST_RootOffsetY:
            m_rootOffset.y = glm::clamp(p_value, -1.f, 1.f);
            break;

        case ST_RootOffsetZ:
            m_rootOffset.z = glm::clamp(p_value, -1.f, 1.f);
            break;

        case ST_RootAngleX:
            m_rootAngle.x = glm::clamp(p_value, -180.f, 180.f);
            break;

        case ST_RootAngleY:
            m_rootAngle.y = glm::clamp(p_value, -180.f, 180.f);
            break;

        case ST_RootAngleZ:
            m_rootAngle.z = glm::clamp(p_value, -180.f, 180.f);
            break;

        case ST_OverlayOffsetX:
            m_overlayOffset.x = glm::clamp(p_value, -0.5f, 0.5f);
            break;

        case ST_OverlayOffsetY:
            m_overlayOffset.y = glm::clamp(p_value, -0.5f, 0.5f);
            break;

        case ST_OverlayOffsetZ:
            m_overlayOffset.z = glm::clamp(p_value, -0.5f, 0.5f);
            break;

        case ST_OverlayAngleX:
            m_overlayAngle.x = glm::clamp(p_value, -180.f, 180.f);
            break;

        case ST_OverlayAngleY:
            m_overlayAngle.y = glm::clamp(p_value, -180.f, 180.f);
            break;

        case ST_OverlayAngleZ:
            m_overlayAngle.z = glm::clamp(p_value, -180.f, 180.f);
            break;

        case ST_OverlaySize:
            m_overlaySize = glm::clamp(p_value, 0.1f, 0.5f);
            break;
    }
}


