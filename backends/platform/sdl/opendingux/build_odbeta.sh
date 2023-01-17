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
	libc=uclibc
        ;;

        lepus | rs90)
	libc=musl
        ;;

        *)
        echo "please provide a valid target for the build: gcw0, lepus or rs90"
        exit 1
        ;;
esac

TOOLCHAIN=/opt/$target-toolchain
SYSROOT=$TOOLCHAIN/mipsel-$target-linux-$libc

export PATH=$TOOLCHAIN/usr/bin:$SYSROOT/usr/include:$TOOLCHAIN/bin:$PATH
export CXX=mipsel-linux-g++

./configure --host=opendingux-$target --enable-release --disable-detection-full --default-dynamic --enable-plugins

make -j12 od-make-opk $dualopk

ls -lh scummvm_$target*.opk
