@echo off
cls
SET mypath=%~dp0

REM detect Steam install directory
for /f "tokens=2*" %%A in ('reg query hku /e /c /v /f SteamPath /t REG_SZ /s 2^>^NUL') DO (
	set SteamPath=%%B
	goto :start
)
:start

REM unregister driver
"%SteamPath%\steamapps\common\SteamVR\bin\win32\vrpathreg.exe" removedriver "%mypath:~0,-1%\leap"

REM always return with 0 to not interrupt uninstall
exit /b 0
