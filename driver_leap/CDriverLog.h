#pragma once

class CDriverLog
{
    static vr::IVRDriverLog *ms_pLogFile;

    static void LogVarArgs(const char* pMsgFormat, va_list args);
public:
    static bool Init(vr::IVRDriverLog* pDriverLog);
    static void Cleanup();
    static void Log(const char* pMsgFormat, ...);
};
