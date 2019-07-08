#!/bin/bash

set -e

if [ -e dependencies/.done ] ; then
	echo ">>> Reusing cached dependencies"
	exit 0;
fi

dot_travis=$(dirname "$0")
. "$dot_travis"/android-set-environment

prefix=$(pwd)/dependencies/
mkdir -p $prefix

echo ">>> Building libjpeg-turbo for $CROSS_HOST"
git clone --depth 1 -b pie-release https://android.googlesource.com/platform/external/libjpeg-turbo
pushd libjpeg-turbo
autoreconf -fiv
./configure --host=$CROSS_HOST --prefix=$prefix --without-turbojpeg --disable-shared && make -j4 install || false
popd

echo ">>> Building freetype for $CROSS_HOST"
git clone --depth 1 -b pie-release https://android.googlesource.com/platform/external/freetype
pushd freetype
sh autogen.sh
./configure --host=$CROSS_HOST --with-harfbuzz=no --with-bzip2=no --with-png=no --prefix=$prefix --disable-shared && make -j4 install || false
popd
cp -p .travis/android-freetype-config $prefix/bin/freetype-config

touch dependencies/.done
