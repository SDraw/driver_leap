#include "stdafx.h"
#include "CDriverLogHelper.h"

vr::IVRDriverLog* CDriverLogHelper::ms_pLogFile = nullptr;

bool CDriverLogHelper::InitDriverLog(vr::IVRDriverLog* pDriverLog)
{
    if(ms_pLogFile) return false;
    ms_pLogFile = pDriverLog;
    return ms_pLogFile != NULL;
}

void CDriverLogHelper::CleanupDriverLog()
{
    ms_pLogFile = nullptr;
}

void CDriverLogHelper::DriverLogVarArgs(const char *pMsgFormat, va_list args)
{
    char buf[1024];
#if defined( WIN32 )
    vsprintf_s(buf, pMsgFormat, args);
#else
    vsnprintf( buf, sizeof( buf ), pMsgFormat, args );
#endif

    if(ms_pLogFile) ms_pLogFile->Log(buf);
}

void CDriverLogHelper::DriverLog(const char *pMsgFormat, ...)
{
    va_list args;
    va_start(args, pMsgFormat);

    DriverLogVarArgs(pMsgFormat, args);

    va_end(args);
}
