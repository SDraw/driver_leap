#pragma once

class CDriverLogHelper
{
    static vr::IDriverLog *ms_pLogFile;
public:
    static bool InitDriverLog(vr::IDriverLog* pDriverLog);
    static void CleanupDriverLog();
    static void DriverLogVarArgs(const char* pMsgFormat, va_list args);
    static void DriverLog(const char* pMsgFormat, ...);
};
