#!/bin/bash

export PATH=/opt/toolchains/psp/bin:$PATH
export PSPDEV=/opt/toolchains/psp
CXXFLAGS="-isystem /opt/toolchains/psp/include"
export CXXFLAGS
export LDFLAGS=-L/opt/toolchains/psp/lib
make distclean
./configure --host=psp --disable-debug --enable-plugins --default-dynamic --enable-release
make -j4
VERSION=`make print-distversion`
DISTS=`make print-dists`
mkdir scummvm-$VERSION
cp -r $DISTS EBOOT.PBP plugins scummvm-$VERSION
mkdir scummvm-$VERSION/kbd
cp -r backends/platform/psp/kbd/*.png scummvm-$VERSION/kbd
zip -r9 scummvm-$VERSION-psp.zip scummvm-$VERSION

