#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`
# default build mode is release
build_release=true

if [ "$#" -eq "1" ]; then
	if [ "$1" = "debug" ]; then
		build_release=false
		echo "Preparing for a debug build..."
	elif [ "$1" = "release" ]; then
		echo "Preparing for a release build..."
	else
		echo "Usage: $0 [debug|release]"
		echo "  debug:    build debug package"
		echo "  release:  build release package (default)"
		exit 0
	fi
else
	echo "Preparing for a release (default) build..."
fi
sleep 1

if $build_release ; then
	if [ -x scummvm/AndroidBuildRelease.sh ] && \
	     { cp scummvm/AndroidBuildRelease.sh scummvm/AndroidBuild.sh ; } ; then
		echo "AndroidBuild.sh created successfully"
	else
		echo "Error: Required script AndroidBuildRelease.sh could not be copied to AndroidBuild.sh"
		exit 0
	fi
else
	if [ -x scummvm/AndroidBuildDebug.sh ] && \
	     { cp scummvm/AndroidBuildDebug.sh scummvm/AndroidBuild.sh ; } ; then
		echo "AndroidBuild.sh created successfully"
	else
		echo "Error: Required script AndroidBuildDebug.sh could not be copied to AndroidBuild.sh"
		exit 0
	fi
fi

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

if $build_release ; then
	./build.sh release scummvm
else
	./build.sh debug scummvm
fi

# the androidsdl build.sh script ensures that the output file is named "app-release" even if we are in debug mode
mv project/app/build/outputs/apk/app-release.apk ../scummvm/dists/androidsdl/scummvm-debug.apk
cd ../scummvm/dists/androidsdl
rm scummvm/AndroidBuild.sh
