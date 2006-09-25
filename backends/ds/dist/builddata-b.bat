@echo off

echo Please note:
echo .
echo 1) Build B of ScummVM DS supports only Beneath a Steel Sky
echo and Flight of the Amazon Queen!
echo For a list of which games are supported under which build,
echo see http://scummvm.drunkencoders.com
echo .
echo 2) This build method requires your zip to be below 30Mb,
echo and uncompressed.
echo .

if not exist scummdata-b.zip goto nofile
copy /b scummvm-b.ds.gba+scummdata-b.zip scummvm-b-withdata.ds.gba
echo Build proccess successful!
echo Copy scummvm-b-withdata.ds.gba to your device.
pause
goto end

:nofile
echo Error: scummdata-b.zip not found!  
echo Place your games inside an uncompressed zip called scummdata-b.zip, 
echo one per folder, and try again.
echo .
echo Build process failed!
pause

:end
