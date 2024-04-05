call ..\set.bat

call ..\buildVcProj.bat XUtil

copy /b xutil.h %XLibs%
copy /b AutomaticLink.h %XLibs%
copy /b console.h %XLibs%
copy /b CRC\crc.h %XLibs%
copy /b sKey.h %XLibs%
copy /b mtsection.h %XLibs%
copy /b Unicode\*.h %XLibs%