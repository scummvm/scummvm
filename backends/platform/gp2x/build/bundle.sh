#!/bin/bash

echo Quick script to make building a distribution of the GP2X port more consistent.

PATH=/tools/open2x_gcc/gcc-4.0.2-glibc-2.3.5/arm-open2x-linux/arm-open2x-linux/bin:$PATH
PATH=/tools/open2x_gcc/gcc-4.0.2-glibc-2.3.5/arm-open2x-linux/bin:$PATH
export CXX=arm-open2x-linux-g++
export CXXFLAGS=-march=armv4t
export CPPFLAGS=-I/tools/open2x_gcc/gcc-4.0.2-glibc-2.3.5/arm-open2x-linux/include
export LDFLAGS=-L/tools/open2x_gcc/gcc-4.0.2-glibc-2.3.5/arm-open2x-linux/lib 

echo Collecting files.
mkdir "scummvm-gp2x-`date '+%Y-%m-%d'`"

cp ./scummvm.png ./scummvm-gp2x-`date '+%Y-%m-%d'`/
cp ./README-GP2X.html ./scummvm-gp2x-`date '+%Y-%m-%d'`/
cp ./README-GP2X ./scummvm-gp2x-`date '+%Y-%m-%d'`/
cp ../../../../scummvm.gpe ./scummvm-gp2x-`date '+%Y-%m-%d'`/
cp ../../../../AUTHORS ./scummvm-gp2x-`date '+%Y-%m-%d'`/
cp ../../../../README ./scummvm-gp2x-`date '+%Y-%m-%d'`/
cp ../../../../COPYING ./scummvm-gp2x-`date '+%Y-%m-%d'`/
cp ../../../../gui/themes/modern.ini ./scummvm-gp2x-`date '+%Y-%m-%d'`/
cp ../../../../gui/themes/modern.zip ./scummvm-gp2x-`date '+%Y-%m-%d'`/


echo Making Stripped GPE.
arm-open2x-linux-strip ./scummvm-gp2x-`date '+%Y-%m-%d'`/scummvm.gpe

echo Building ZIP bundle.
echo You should have a "scummvm-gp2x-`date '+%Y-%m-%d'`.zip" for the GP2X port ready to go.