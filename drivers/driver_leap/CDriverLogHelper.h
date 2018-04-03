#pragma once

class CDriverLogHelper
{
    static vr::IVRDriverLog *ms_pLogFile;
public:
    static bool InitDriverLog(vr::IVRDriverLog* pDriverLog);
    static void CleanupDriverLog();
    static void DriverLogVarArgs(const char* pMsgFormat, va_list args);
    static void DriverLog(const char* pMsgFormat, ...);
};
