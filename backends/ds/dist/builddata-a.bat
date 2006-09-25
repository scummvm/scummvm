@echo off

echo Please note:
echo .
echo 1) Build A of ScummVM DS supports only Lucasarts SCUMM games!
echo For a list of which games are supported under which build,
echo see http://scummvm.drunkencoders.com
echo .
echo 2) This build method requires your zip to be below 30Mb,
echo and uncompressed.
echo .

if not exist scummdata-a.zip goto nofile
copy /b scummvm-a.ds.gba+scummdata-a.zip scummvm-a-withdata.ds.gba
echo Build proccess successful!
echo Copy scummvm-a-withdata.ds.gba to your device.
pause
goto end

:nofile
echo Error: scummdata-a.zip not found!  
echo Place your games inside an uncompressed zip called scummdata-a.zip, 
echo one per folder, and try again.
echo .
echo Build process failed!
pause

:end
