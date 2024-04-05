call ..\..\set.bat

if "%1"=="copyHeaders" goto copyHeaders

call ..\..\buildVcProj.bat XMath

:copyHeaders
del %XLibs%\XMath\*.* /q 
rd %XLibs%\XMath\ /q 

xcopy *.h %XLibs%\XMath\ /S 
del %XLibs%\XMath\stdafx.h /q 

