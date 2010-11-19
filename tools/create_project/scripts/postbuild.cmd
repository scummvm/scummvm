REM @echo off

REM ---------------------------------------------------------------
REM -- Post-Build Script
REM ---------------------------------------------------------------
REM
REM Copy engine data, themes, translation and required dlls to the
REM output folder.
REM
REM Expected parameters
REM    Root folder
REM    Output folder
REM    Architecture

if "%~1"=="" goto error_input
if "%~2"=="" goto error_output
if "%~3"=="" goto error_arch

echo Copying data files
echo.

REM Copy files
xcopy /F /Y "%~1/dists/engine-data/*.dat" %~2 > NUL 2>&1
xcopy /F /Y "%~1/dists/engine-data/*.tbl" %~2 > NUL 2>&1
xcopy /F /Y "%~1/dists/engine-data/*.cpt" %~2 > NUL 2>&1
xcopy /F /Y "%~1/dists/engine-data/README" %~2 > NUL 2>&1
xcopy /F /Y "%~1/gui/themes/*.zip" %~2 > NUL 2>&1
xcopy /F /Y "%~1/gui/themes/translations.dat" %~2 > NUL 2>&1
xcopy /F /Y "%SCUMMVM_LIBS%/lib/%~3/SDL.dll" %~2 > NUL 2>&1
goto done

:error_output
@echo Invalid root folder (%~1)!
goto done

:error_output
@echo Invalid output folder (%~2)!
goto done

:error_arch
@echo Invalid arch parameter (was: %~3, allowed: x86, x64)!
goto done

:done
exit /B0
