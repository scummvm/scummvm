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

REM Run the revision script
@call cscript "%~1/tools/create_project/scripts/revision.vbs" %~1 1>NUL
if not %errorlevel% == 0 goto error_script
goto done

:error_output
echo Invalid root folder (%~1)!
goto done

:error_script:
echo An error occured while running the revision script!

:done
exit /B0
