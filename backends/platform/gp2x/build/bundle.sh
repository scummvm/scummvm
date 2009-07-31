#!/bin/bash

echo Quick script to make building a distribution of the GP2X port more consistent.

PATH=/opt/open2x/gcc-4.1.1-glibc-2.3.6/arm-open2x-linux/bin:$PATH
PATH=/opt/open2x/gcc-4.1.1-glibc-2.3.6/bin:$PATH
export CXX=arm-open2x-linux-g++
export CXXFLAGS=-march=armv4t
export CPPFLAGS=-I/opt/open2x/gcc-4.1.1-glibc-2.3.6/include
export LDFLAGS=-L/opt/open2x/gcc-4.1.1-glibc-2.3.6/lib

echo Collecting files.
mkdir "scummvm-gp2x-`date '+%Y-%m-%d'`"
mkdir "scummvm-gp2x-`date '+%Y-%m-%d'`/saves"
mkdir "scummvm-gp2x-`date '+%Y-%m-%d'`/plugins"
mkdir "scummvm-gp2x-`date '+%Y-%m-%d'`/engine-data"

echo "Please put your save games in this dir" >> "scummvm-gp2x-`date '+%Y-%m-%d'`/saves/PUT_SAVES_IN_THIS_DIR"

cp ./scummvm.gpe ./scummvm-gp2x-`date '+%Y-%m-%d'`/
cp ./scummvm.png ./scummvm-gp2x-`date '+%Y-%m-%d'`/
cp ./README-GP2X ./scummvm-gp2x-`date '+%Y-%m-%d'`/
cp ./mmuhack.o ./scummvm-gp2x-`date '+%Y-%m-%d'`/
cp ../../../../scummvm.gp2x ./scummvm-gp2x-`date '+%Y-%m-%d'`/
cp ../../../../AUTHORS ./scummvm-gp2x-`date '+%Y-%m-%d'`/
cp ../../../../README ./scummvm-gp2x-`date '+%Y-%m-%d'`/
cp ../../../../COPYING ./scummvm-gp2x-`date '+%Y-%m-%d'`/
cp ../../../../COPYRIGHT ./scummvm-gp2x-`date '+%Y-%m-%d'`/
cp ../../../../NEWS ./scummvm-gp2x-`date '+%Y-%m-%d'`/
cp ../../../../gui/themes/scummmodern.zip ./scummvm-gp2x-`date '+%Y-%m-%d'`/
cp ../../../../backends/vkeybd/packs/vkeybd_default.zip ./scummvm-gp2x-`date '+%Y-%m-%d'`/
cp ../../../../dists/pred.dic ./scummvm-gp2x-`date '+%Y-%m-%d'`/
cp ../../../../dists/engine-data/* ./scummvm-gp2x-`date '+%Y-%m-%d'`/engine-data
cp ../../../../plugins/* ./scummvm-gp2x-`date '+%Y-%m-%d'`/plugins

echo Making Stripped GPE.
arm-open2x-linux-strip ./scummvm-gp2x-`date '+%Y-%m-%d'`/scummvm.gp2x

echo Building ZIP bundle.
if [ -f /usr/bin/zip ]
	then
		rm ./"scummvm-gp2x-`date '+%Y-%m-%d'`.zip"
		cd "scummvm-gp2x-`date '+%Y-%m-%d'`"
		zip -r -9 "../scummvm-gp2x-`date '+%Y-%m-%d'`.zip" *
		echo You should have a "scummvm-gp2x-`date '+%Y-%m-%d'`.zip" for the GP2X port ready to go.
		cd ..
		rm -R ./"scummvm-gp2x-`date '+%Y-%m-%d'`"
	else
		echo - /usr/bin/zip not found, ZIP bundle not created.
		echo All included files can also be found in ./"scummvm-gp2x-`date '+%Y-%m-%d'`"
		echo - Please use you preferred archive tool to bundle these files.
fi
