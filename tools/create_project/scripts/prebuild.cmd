@echo off

REM ---------------------------------------------------------------
REM -- Pre-Build Script
REM ---------------------------------------------------------------
REM
REM Generate file with proper revision number
REM
REM Expected parameters
REM    Root folder

if "%~1"=="" goto error_input

if not exist "%~1/.svn/" GOTO error_working_copy

echo Generating revision number

SubWCRev.exe "%~1" "%~1/base/internal_version.h.tpl" "%~1/base/internal_version.h"

if not %errorlevel% == 0 goto error_subwcrev
goto done

:error_output
@echo Invalid root folder (%~1)!
goto done

:error_working_copy
echo Not a working copy, skipping...
exit /B0

:error_subwcrev
echo SubWCRev not found or invalid command line, skipping...
exit /B0

:done
exit /B0
