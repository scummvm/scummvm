#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

if [ \! -d ../../../androidsdl ] ; then
   cd ../../..
   git clone git://github.com/pelya/commandergenius androidsdl
   cd androidsdl
   git submodule update --init project/jni/iconv/src
   git checkout d378ee692f2e380a0ab0635c1df2eb6941b5bf58
   cd project/jni/iconv/src
# checkout a specific version of iconv that allows building with the specific version (d378ee692f2e380a0ab0635c1df2eb6941b5bf58) of androidsdl 
   git checkout 07bead221ed4fa61cc8d880db3d9a5e704866097
   cd ../../../../../scummvm/dists/androidsdl
fi

if [ \! -d scummvm/scummvm ] ; then
   ln -s ../../../../scummvm scummvm
fi

if [ \! -d ../../../androidsdl/project/jni/application/scummvm ] ; then
   ln -s ../../../../scummvm/dists/androidsdl/scummvm ../../../androidsdl/project/jni/application
fi

cd ../../../androidsdl
./build.sh scummvm

mv project/app/build/outputs/apk/app-release.apk ../scummvm/dists/androidsdl/scummvm-debug.apk
cd ../scummvm/dists/androidsdl
