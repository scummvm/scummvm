#!/bin/bash

echo Quick script to make building a distribution of the GP2X Wiz backend more consistent.

echo Collecting files.
mkdir "scummvm-wiz-`date '+%Y-%m-%d'`"
mkdir "scummvm-wiz-`date '+%Y-%m-%d'`/scummvm"
mkdir "scummvm-wiz-`date '+%Y-%m-%d'`/scummvm/saves"
mkdir "scummvm-wiz-`date '+%Y-%m-%d'`/scummvm/plugins"
mkdir "scummvm-wiz-`date '+%Y-%m-%d'`/scummvm/engine-data"

echo "Please put your save games in this dir" >> "scummvm-wiz-`date '+%Y-%m-%d'`/scummvm/saves/PUT_SAVES_IN_THIS_DIR"

cp ./scummvm.gpe ./scummvm-wiz-`date '+%Y-%m-%d'`/scummvm/
cp ./scummvm.png ./scummvm-wiz-`date '+%Y-%m-%d'`/scummvm/
cp ./README-GP2XWIZ ./scummvm-wiz-`date '+%Y-%m-%d'`/scummvm/
cp ./scummvm.ini ./scummvm-wiz-`date '+%Y-%m-%d'`/
cp ../../../../scummvm.wiz ./scummvm-wiz-`date '+%Y-%m-%d'`/scummvm/
cp ../../../../AUTHORS ./scummvm-wiz-`date '+%Y-%m-%d'`/scummvm/
cp ../../../../README ./scummvm-wiz-`date '+%Y-%m-%d'`/scummvm/
cp ../../../../COPYING ./scummvm-wiz-`date '+%Y-%m-%d'`/scummvm/
cp ../../../../COPYRIGHT ./scummvm-wiz-`date '+%Y-%m-%d'`/scummvm/
cp ../../../../NEWS ./scummvm-wiz-`date '+%Y-%m-%d'`/scummvm/
cp ../../../../gui/themes/scummmodern.zip ./scummvm-wiz-`date '+%Y-%m-%d'`/scummvm/
cp ../../../../backends/vkeybd/vkeybd.zip ./scummvm-wiz-`date '+%Y-%m-%d'`/scummvm/
cp ../../../../dists/pred.dic ./scummvm-wiz-`date '+%Y-%m-%d'`/scummvm/
cp ../../../../dists/engine-data/* ./scummvm-wiz-`date '+%Y-%m-%d'`/scummvm/engine-data
cp ../../../../plugins/* ./scummvm-wiz-`date '+%Y-%m-%d'`/scummvm/plugins

echo Making Stripped exe.
arm-open2x-linux-strip ./scummvm-wiz-`date '+%Y-%m-%d'`/scummvm/scummvm.wiz

echo Building ZIP bundle.
if [ -f /usr/bin/zip ]
	then
		rm ./"scummvm-wiz-`date '+%Y-%m-%d'`.zip"
		cd "scummvm-wiz-`date '+%Y-%m-%d'`"
		zip -r -9 "../scummvm-wiz-`date '+%Y-%m-%d'`.zip" *
		echo You should have a "scummvm-wiz-`date '+%Y-%m-%d'`.zip" for the GP2X Wiz backend ready to go.
		cd ..
		rm -R ./"scummvm-wiz-`date '+%Y-%m-%d'`"
	else
		echo - /usr/bin/zip not found, ZIP bundle not created.
		echo All included files can also be found in ./"scummvm-wiz-`date '+%Y-%m-%d'`"
		echo - Please use you preferred archive tool to bundle these files.
fi
