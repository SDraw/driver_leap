#include <windows.h>

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <algorithm>

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"

using namespace std;

// execute a command and return its output
wstring exec(const wstring &cmd) {
    wchar_t buffer[128];
    wstring result = L"";
    shared_ptr<FILE> pipe(_wpopen(cmd.c_str(), L"r"), _pclose);
    if (!pipe) throw runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgetws(buffer, 128, pipe.get()) != NULL)
            result += buffer;
    }
    return result;
}

// read a registry key value of type REG_SZ
bool ReadRegValue(HKEY root, wstring key, wstring name, wstring &value)
{
    HKEY hKey;
    if (RegOpenKeyEx(root, key.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return false;

    DWORD type;
    DWORD cbData;
    if (RegQueryValueEx(hKey, name.c_str(), NULL, &type, NULL, &cbData) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return false;
    }

    if (type != REG_SZ)
    {
        RegCloseKey(hKey);
        return false;
    }

    value = wstring(cbData / sizeof(wchar_t), L'\0');
    if (RegQueryValueEx(hKey, name.c_str(), NULL, NULL, reinterpret_cast<LPBYTE>(&value[0]), &cbData) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);

    size_t firstNull = value.find_first_of(L'\0');
    if (firstNull != string::npos)
        value.resize(firstNull);

    return true;
}

// get the SteamVR installation path
bool SteamVRInstallLocation(wstring &location)
{
    // guess where streamVR is located from its uninstall entry. Is there a better way?
    if (!ReadRegValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Steam App 250820", L"InstallLocation", location))
    {
        // otherwise assume it's in the default Steam library
        if (ReadRegValue(HKEY_CURRENT_USER, L"SOFTWARE\\Valve\\Steam", L"SteamPath", location))
        {
            replace(location.begin(), location.end(), L'/', L'\\');
            location.append(L"\\steamapps\\common\\SteamVR");
            return true;
        }
    }
    else
    {
        return true;
    }
    return false;
}

// get the path to the vrpathreg utility (32bit)
bool SteamVRPathReg(wstring &location)
{
    if (SteamVRInstallLocation(location))
    {
        location.append(L"\\bin\\win32\\vrpathreg.exe");
        return true;
    }
    return false;
}

// get the SteamVR config path from the vrpathreg utility
bool SteamVRConfigPath(wstring &configpath)
{
    wstring location;
    if (SteamVRPathReg(location))
    {
        wstring result = exec(location);
        size_t pos = result.find(L"Config path = ", 0);
        if (pos != wstring::npos)
        {
            pos += 14;
            size_t pos2 = result.find(L"\n", pos);
            configpath = result.substr(pos, pos2 - pos);
            return true;
        }
    }
    return false;
}

// get the SteamVR log path from the vrpathreg utility
bool SteamVRLogPath(wstring &logpath)
{
    wstring location;
    if (SteamVRPathReg(location))
    {
        wstring result = exec(location);
        size_t pos = result.find(L"Log path = ", 0);
        if (pos != wstring::npos)
        {
            pos += 11;
            size_t pos2 = result.find(L"\n", pos);
            logpath = result.substr(pos, pos2 - pos);
            return true;
        }
        return true;
    }
    return false;
}

// get full path to steamvr.vrconfig file
bool SteamVRVRSettingsFile(wstring &configfile)
{
    if (SteamVRConfigPath(configfile))
    {
        configfile.append(L"\\steamvr.vrsettings");
        return true;
    }
    return false;
}

#include <vector>

void MergeObject(rapidjson::Value& target, rapidjson::Value& source, rapidjson::Value::AllocatorType& allocator, rapidjson::Value& backup) {
    if (target.GetType() == source.GetType() ||
        target.GetType() == rapidjson::kTrueType && source.GetType() == rapidjson::kFalseType ||
        target.GetType() == rapidjson::kFalseType && source.GetType() == rapidjson::kTrueType)
    {
        if (target.GetType() == rapidjson::kObjectType || target.GetType() == rapidjson::kArrayType)
        {
            std::vector<rapidjson::Value::MemberIterator> toremove;

            // if the backup copy of the original document has members that are NOT in the source document
            // we delete them from the backup copy
            for (rapidjson::Value::MemberIterator itr = backup.MemberBegin(); itr != backup.MemberEnd(); ++itr)
                if (source.FindMember((*itr).name) == source.MemberEnd())
                    toremove.push_back(itr);

            // erase in reverse order to keep the iterators sane
            for (int i = toremove.size()-1; i >= 0; i--)
                backup.RemoveMember(toremove[i]);

            // add or merge members found in the source document into the target document
            for (rapidjson::Value::MemberIterator itr = source.MemberBegin(); itr != source.MemberEnd(); ++itr)
            {
                rapidjson::Value::MemberIterator dest;
                if ((dest = target.FindMember((*itr).name)) == target.MemberEnd())
                    target.AddMember(itr->name, itr->value, allocator);
                else
                    MergeObject((*dest).value, (*itr).value, allocator, (*backup.FindMember((*itr).name)).value);
            }
        }
        // string, number, boolean and null types simply get replaced by the source
        else if (target.GetType() == rapidjson::kStringType ||
                 target.GetType() == rapidjson::kNumberType ||
                 target.GetType() == rapidjson::kFalseType ||
                 target.GetType() == rapidjson::kTrueType ||
                 target.GetType() == rapidjson::kNullType)
            target = source;
    }
}

bool SubtractObject(rapidjson::Value& target, rapidjson::Value& source)
{
    if (target.GetType() == source.GetType() ||
        target.GetType() == rapidjson::kTrueType && source.GetType() == rapidjson::kFalseType ||
        target.GetType() == rapidjson::kFalseType && source.GetType() == rapidjson::kTrueType)
    {
        if (target.GetType() == rapidjson::kObjectType || target.GetType() == rapidjson::kArrayType)
        {
            std::vector<rapidjson::Value::MemberIterator> toremove;

            // subtract members found in the source document from the target document
            for (rapidjson::Value::MemberIterator itr = source.MemberBegin(); itr != source.MemberEnd(); ++itr)
            {
                rapidjson::Value::MemberIterator dest;
                if ((dest = target.FindMember((*itr).name)) != target.MemberEnd())
                    if (SubtractObject((*dest).value, (*itr).value))
                        toremove.push_back(dest);
            }

            // erase in reverse order to keep the iterators sane
            for (int i = toremove.size() - 1; i >= 0; i--)
                target.RemoveMember(toremove[i]);

            // empty parent members should be deleted
            if (target.MemberCount() == 0) return true;
        }
        // string, number, boolean and null types simply get removed
        else if (target.GetType() == rapidjson::kStringType ||
            target.GetType() == rapidjson::kNumberType ||
            target.GetType() == rapidjson::kFalseType ||
            target.GetType() == rapidjson::kTrueType ||
            target.GetType() == rapidjson::kNullType)
            return true;
    }
    return false;
}

bool MergeJSON(const wstring &targetfile, const wstring &tobemergedin, const wstring &backupfile)
{
    char buffer[65536];

    FILE * pFile = _wfopen(targetfile.c_str(), L"rt");
    rapidjson::Document document;
    {
        rapidjson::FileReadStream is(pFile, buffer, sizeof(buffer));
        document.ParseStream<0>(is);
    }
    fclose(pFile);

    FILE * pFile2 = _wfopen(tobemergedin.c_str(), L"rt");
    rapidjson::Document document2;
    {
        rapidjson::FileReadStream is2(pFile2, buffer, sizeof(buffer));
        document2.ParseStream<0>(is2);
    }
    fclose(pFile2);

    rapidjson::Document backup;
    backup.CopyFrom(document, backup.GetAllocator());

    MergeObject(document, document2, document.GetAllocator(), backup);

    FILE * pFile3 = _wfopen(targetfile.c_str(), L"wt");
    {
        rapidjson::FileWriteStream os(pFile3, buffer, sizeof(buffer));
        rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
        document.Accept(writer);
    }
    fclose(pFile3);

    FILE * pFile4 = _wfopen(backupfile.c_str(), L"wt");
    {
        rapidjson::FileWriteStream os(pFile4, buffer, sizeof(buffer));
        rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
        backup.Accept(writer);
    }
    fclose(pFile4);

    return true;
}

bool UnmergeJSON(const wstring &targetfile, const wstring &tobeunmerged, const wstring &backupfile)
{
    char buffer[65536];

    FILE * pFile = _wfopen(targetfile.c_str(), L"rt");
    rapidjson::Document document;
    {
        rapidjson::FileReadStream is(pFile, buffer, sizeof(buffer));
        document.ParseStream<0>(is);
    }
    fclose(pFile);

    FILE * pFile2 = _wfopen(tobeunmerged.c_str(), L"rt");
    rapidjson::Document document2;
    {
        rapidjson::FileReadStream is2(pFile2, buffer, sizeof(buffer));
        document2.ParseStream<0>(is2);
    }
    fclose(pFile2);

    FILE * pFile3 = _wfopen(backupfile.c_str(), L"rt");
    rapidjson::Document document3;
    {
        rapidjson::FileReadStream is3(pFile3, buffer, sizeof(buffer));
        document3.ParseStream<0>(is3);
    }
    fclose(pFile3);

    SubtractObject(document, document2);
    rapidjson::Document backup;
    backup.CopyFrom(document, backup.GetAllocator());
    MergeObject(document, document3, document.GetAllocator(), backup);

    FILE * pFile4 = _wfopen(targetfile.c_str(), L"wt");
    {
        rapidjson::FileWriteStream os(pFile4, buffer, sizeof(buffer));
        rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
        document.Accept(writer);
    }
    fclose(pFile4);

    return true;
}

// main program
int wmain(int argc, wchar_t* argv[])
{
    if (argc == 4)
    {
        // default to current working directory as the driver path
        wchar_t wcwd[512];
        wchar_t *cwd = _wgetcwd(wcwd, sizeof(wcwd) / sizeof(wchar_t));

        // if the path to the source JSON file contains a backslash,
        // prefer this path over the current working directory for
        // registering the driver path.
        wchar_t *tmp;
        if ((tmp = wcsrchr(argv[2], L'\\')) != NULL)
        {
            cwd = wcsncpy(wcwd, argv[2], tmp - argv[2]);
            wcwd[tmp - argv[2]] = L'\0';
        }

        if (cwd != NULL)
        {
            wstring configfile;
            if (SteamVRVRSettingsFile(configfile))
            {
                if (!wcsicmp(argv[1], L"install"))
                {
                    MergeJSON(configfile, argv[2], argv[3]);

                    wstring location;
                    if (SteamVRPathReg(location))
                    {
                        location.append(L" adddriver \"");
                        location.append(cwd);
                        location.append(L"\"");
                        wstring result = exec(location);
                        wcout << result << endl;
                    }
                }
                else if (!wcsicmp(argv[1], L"uninstall"))
                {
                    UnmergeJSON(configfile, argv[2], argv[3]);

                    wstring location;
                    if (SteamVRPathReg(location))
                    {
                        location.append(L" removedriver \"");
                        location.append(cwd);
                        location.append(L"\"");
                        wstring result = exec(location);
                        wcout << result << endl;
                    }
                }
            }
            else
                wcout << L"Unable to determine location of steamvr.vrsettings file!" << endl;
        }
    }
    else
    {
        wcout << L"Usage: config_tool.exe [un]install jsonfile backupfile" << endl;
    }
    return 0;
}
