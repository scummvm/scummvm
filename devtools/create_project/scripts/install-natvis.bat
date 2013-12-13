@echo off
echo Installing Visual Studio debugger integration...

REM On 2000 & XP, the folder is "My Documents" but VS 2012 is not supported on those OSes
SET DOCUMENTS="%USERPROFILE%\Documents"
set FOUND=0

REM Set current folder
cd /d %~dp0

:INSTALL_VS11
SET FOLDER="%DOCUMENTS%\Visual Studio 2012"
IF EXIST %FOLDER% (
    echo     Visual Studio 2012
    copy scummvm.natvis %FOLDER%\Visualizers
    IF NOT %ERRORLEVEL% == 0 GOTO FAILED
    SET FOUND=1
)

:INSTALL_VS12
SET FOLDER="%DOCUMENTS%\Visual Studio 2013"
IF EXIST %FOLDER% (
    echo     Visual Studio 2013
    copy scummvm.natvis %FOLDER%\Visualizers
    IF NOT %ERRORLEVEL% == 0 GOTO FAILED
    SET FOUND=1
)

IF %FOUND% == 1 goto SUCCESS
echo Failed to find Visual Studio user folder.

:SUCCESS
echo.
echo Done!
goto END

:FAILED
echo Failed to install visualization file

:END
pause
