#!/bin/bash

set -e

if [ -e dependencies/.done ] ; then
	echo ">>> Reusing cached dependencies"
	exit 0;
fi

mkdir -p dependencies
prefix=$(pwd)/dependencies

echo ">>> Building libjpeg-turbo"
git clone --depth 1 -b pie-release https://android.googlesource.com/platform/external/libjpeg-turbo
pushd libjpeg-turbo
autoreconf -fiv
./configure --host=$CROSS_HOST --prefix=$prefix --with-sysroot=$ANDROID_NDK/sysroot && make -j4 install
popd

echo ">>> Building freetype"
git clone --depth 1 -b pie-release https://android.googlesource.com/platform/external/freetype
pushd freetype
sh autogen.sh
./configure --host=$CROSS_HOST --with-harfbuzz=no --with-bzip2=no --with-png=no --prefix=$prefix --with-sysroot=$ANDROID_NDK/sysroot && make -j4 install
popd
cp -p .travis/freetype-config dependencies/bin

touch dependencies/.done
