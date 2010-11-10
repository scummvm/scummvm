@echo off
echo.
echo Automatic creation of the Code::Blocks project files
echo.

if not exist create_project.exe goto no_tool

:question
echo.
set /p batchanswer="Enable (S)table engines only, or (A)ll engines? (S/A)"
if "%batchanswer%"=="s" goto stable
if "%batchanswer%"=="S" goto stable
if "%batchanswer%"=="a" goto all
if "%batchanswer%"=="A" goto all
goto question

:no_tool
echo create_project.exe not found in the current folder.
echo You need to build it first and copy it in this
echo folder
goto done

:all
echo Creating project files with all engines enabled (stable and unstable)
create_project ..\.. --enable-all-engines --codeblocks
goto done

:stable
echo Creating normal project files, with only the stable engines enabled
create_project ..\.. --codeblocks
goto done

:done
pause