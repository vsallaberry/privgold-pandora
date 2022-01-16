@echo off

set MYDIR=%~dp0
set PATH=%MYDIR%\bin;%PATH%

%~d0
cd "%MYDIR%\data"
"%MYDIR%\bin\vssetup.exe"

IF NOT ERRORLEVEL 0 pause

