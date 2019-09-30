#pragma once

class CDriverLog final
{
    static vr::IVRDriverLog *ms_pLogFile;

    static void LogVarArgs(const char* pMsgFormat, va_list args);

    CDriverLog() = delete;
    ~CDriverLog() = delete;
    CDriverLog(const CDriverLog &that) = delete;
    CDriverLog& operator=(const CDriverLog &that) = delete;
public:
    static bool Init(vr::IVRDriverLog* pDriverLog);
    static void Cleanup();
    static void Log(const char* pMsgFormat, ...);
};
