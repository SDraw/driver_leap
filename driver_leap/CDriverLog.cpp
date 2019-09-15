#include "stdafx.h"
#include "CDriverLog.h"

vr::IVRDriverLog* CDriverLog::ms_pLogFile = nullptr;

bool CDriverLog::Init(vr::IVRDriverLog* pDriverLog)
{
    if(!ms_pLogFile) ms_pLogFile = pDriverLog;
    return (ms_pLogFile != nullptr);
}

void CDriverLog::Cleanup()
{
    ms_pLogFile = nullptr;
}

void CDriverLog::LogVarArgs(const char *pMsgFormat, va_list args)
{
    char buf[1024];
#if defined( WIN32 )
    vsprintf_s(buf, pMsgFormat, args);
#else
    vsnprintf( buf, sizeof( buf ), pMsgFormat, args );
#endif

    if(ms_pLogFile) ms_pLogFile->Log(buf);
}

void CDriverLog::Log(const char *pMsgFormat, ...)
{
    va_list args;
    va_start(args, pMsgFormat);

    LogVarArgs(pMsgFormat, args);

    va_end(args);
}
