#!/bin/bash

echo Quick script to make building a distribution of the GP32 port more consistent.

echo Collecting files.
mkdir "scummvm-gp32-`date '+%Y-%m-%d'`"
mkdir "scummvm-gp32-`date '+%Y-%m-%d'`/saves"

echo "Please put your save games in this dir" >> "scummvm-gp32-`date '+%Y-%m-%d'`/saves/PUT_SAVES_IN_THIS_DIR"

# No current port README.
#cp ./README-GP32.html ./scummvm-gp32-`date '+%Y-%m-%d'`/
#cp ./README-GP32 ./scummvm-gp32-`date '+%Y-%m-%d'`/

cp ../../../../scummvm.fxe ./scummvm-gp32-`date '+%Y-%m-%d'`/
cp ../../../../AUTHORS ./scummvm-gp32-`date '+%Y-%m-%d'`/
cp ../../../../README ./scummvm-gp32-`date '+%Y-%m-%d'`/
cp ../../../../COPYING ./scummvm-gp32-`date '+%Y-%m-%d'`/
cp ../../../../COPYRIGHT ./scummvm-gp32-`date '+%Y-%m-%d'`/
cp ../../../../NEWS ./scummvm-gp32-`date '+%Y-%m-%d'`/
cp ../../../../dists/pred.dic ./scummvm-gp32-`date '+%Y-%m-%d'`/

# GP32 does not support themes.
#cp ../../../../gui/themes/modern.ini ./scummvm-gp32-`date '+%Y-%m-%d'`/
#cp ../../../../gui/themes/modern.zip ./scummvm-gp32-`date '+%Y-%m-%d'`/


echo Building ZIP bundle.
if [ -f /usr/bin/zip ]
	then
		cd "scummvm-gp32-`date '+%Y-%m-%d'`"
		zip "../scummvm-gp32-`date '+%Y-%m-%d'`.zip" * -r -9
		echo You should have a "scummvm-gp32-`date '+%Y-%m-%d'`.zip" for the GP2X port ready to go.
		echo All included files can also be found in ./"scummvm-gp32-`date '+%Y-%m-%d'`"
	else  
		echo - /usr/bin/zip not found, ZIP bundle not created.
		echo All included files can also be found in ./"scummvm-gp32-`date '+%Y-%m-%d'`"
		echo - Please use you preferred archive tool to bundle these files.
fi
