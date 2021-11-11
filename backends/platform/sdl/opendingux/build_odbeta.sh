#!/bin/bash

CONFIG="./configure --host=opendingux-$target --enable-release --disable-detection-full"

case $target in

        gcw0)
	target2=$target
        ;;

        lepus)
	target2=$target
        ;;

        rg99)
	target2=rs90
        ;;
        
        *)
        echo "please provide a valid target for the build: gcw0, lepus or rg99"
        exit 0
        ;;
esac

TOOLCHAIN=/opt/$target2-toolchain

if [ $target == "gcw0" ]; then
	SYSROOT=$TOOLCHAIN/mipsel-$target2-linux-uclibc
else
	SYSROOT=$TOOLCHAIN/mipsel-$target2-linux-musl
fi

export PATH=$TOOLCHAIN/usr/bin:$SYSROOT/usr/include:$TOOLCHAIN/bin:$PATH
export CXX=mipsel-linux-g++
export CXXFLAGS="-funsigned-char" # workaround for a scummvm tolower() bug when adding games

$CONFIG && make -j12 od-make-opk && ls -lh scummvm_$target*.opk
