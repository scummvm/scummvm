@ECHO OFF

SET target=%2

if "%1"=="clean" goto clean
if "%1"=="test" goto test
if "%1"=="fast" goto fast
if "%1"=="--help" goto help
if "%1"=="-h" goto help

SET platform=armi
if "%1"=="" goto build
SET platform=%1

:build
echo ---------------------------------------------------
@ECHO ON
call abld build %platform% urel %target%
goto End

:fast
echo ---------------------------------------------------
REM @ECHO ON
call abld library  %platform% urel %target%
REM call abld resource %platform% urel %target%
call abld target   %platform% urel %target%
goto End

:test
echo ---------------------------------------------------
@ECHO ON
call abld test build armi urel %target%
goto End

:clean
abld clean %target%
goto End

:help
echo Usage: make clean
echo        make [armi|wins] [target] 
goto End

:End
echo ---------------------------------------------------

