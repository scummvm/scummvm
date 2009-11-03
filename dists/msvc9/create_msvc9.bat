@echo off
echo.
echo Automatic creation of the MSVC9 project files
echo.

if not exist create_msvc.exe goto no_tool

choice /m "Do you want to enable all engines? "
if errorlevel 2 goto normal_build
if errorlevel 1 goto all_engines

:no_tool
echo create_msvc.exe not found in the current folder.
echo You need to build it first and copy it in this
echo folder
goto done

:all_engines
echo Creating project files with all engines enabled
create_msvc ..\.. --enable-all-engines --msvc-version 9
goto done

:normal_build
echo Creating normal project files, with only the stable
echo engines enabled
create_msvc ..\.. --msvc-version 9
goto done

:done
pause
