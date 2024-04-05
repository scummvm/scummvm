call ..\..\set.bat

if "%1"=="copyHeaders" goto copyHeaders

call ..\..\buildVcProj.bat Serialization


:: copying headers
:copyHeaders

del %XLibs%\Serialization\*.* /q /s
rd %XLibs%\Serialization\ /q /s

xcopy *.h %XLibs%\Serialization\  


del %XLibs%\FileUtils\*.* /q /s
rd %XLibs%\FileUtils\ /q /s

xcopy ..\FileUtils\*.h %XLibs%\FileUtils\  


del %XLibs%\XTL\*.* /q /s
rd %XLibs%\XTL\ /q /s

xcopy ..\XTL\*.h %XLibs%\XTL\  
