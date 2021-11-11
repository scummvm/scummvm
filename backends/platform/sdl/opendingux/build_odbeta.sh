#!/bin/bash

set -e

target=$1
if [ "$2" = "dualopk" ]; then
	dualopk="dualopk=yes"
else
	dualopk=
fi

case $target in

        gcw0)
	target2=$target
	libc=uclibc
        ;;

        lepus)
	target2=$target
	libc=musl
        ;;

        rg99)
	target2=rs90
	libc=musl
        ;;
        
        *)
        echo "please provide a valid target for the build: gcw0, lepus or rg99"
        exit 1
        ;;
esac

TOOLCHAIN=/opt/$target2-toolchain
SYSROOT=$TOOLCHAIN/mipsel-$target2-linux-$libc

export PATH=$TOOLCHAIN/usr/bin:$SYSROOT/usr/include:$TOOLCHAIN/bin:$PATH
export CXX=mipsel-linux-g++
export CXXFLAGS="-funsigned-char" # workaround for a scummvm tolower() bug when adding games

./configure --host=opendingux-$target --enable-release --disable-detection-full

make -j12 od-make-opk $dualopk

ls -lh scummvm_$target*.opk
