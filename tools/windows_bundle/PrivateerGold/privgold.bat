@echo off

set MYDIR=%~dp0
set PATH=%MYDIR%\bin;%PATH%

REM NOT NEEDED
rem set PYTHONPATH="%MYDIR%\data\modules\builtin"
rem set PYTHONHOME="%PYTHONPATH%"

REM GET PRIVGOLD DIRECTORY
set VEGAHOME=%LOCALAPPDATA%\privgold120
IF NOT EXIST "%VEGAHOME%" mkdir "%VEGAHOME%"

REM GET GL ENGINE FROM CONFIG FILE
for /F "tokens=3" %%i in ('findstr /R /C:"^#set GL-Renderer .*" "%VEGAHOME%\vegastrike.config"') do set GFX=%%i
IF NOT EXIST "%MYDIR%\bin\vegastrike.%GFX%.exe" set GFX=SDL2

REM GO TO DATA DRIVE, THEN TO DATA FOLDER
%~d0
cd "%MYDIR%\data"

REM LAUNCH
set VEGALOG=%VEGAHOME%\vegastrike.log
"%MYDIR%\bin\vegastrike.%GFX%.exe" 1> "%VEGALOG%" 2>&1

