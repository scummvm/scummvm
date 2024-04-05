call ..\..\set.bat

if "%1"=="copyHeaders" goto copyHeaders

call ..\..\buildVcProj.bat XmlRpc
rd /S /Q .Release
rd /S /Q .Debug

:copyHeaders
md %XLibs%\XmlRpc
copy /b *.h %XLibs%\XmlRpc

