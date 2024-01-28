#!/bin/bash

set -e

TOOLCHAIN=/opt/miyoomini-toolchain
SYSROOT=$TOOLCHAIN/arm-linux-gnueabihf

export PATH=$TOOLCHAIN/usr/bin:$SYSROOT/usr/include:$TOOLCHAIN/bin:$PATH
export CXX=arm-linux-gnueabihf-g++

./configure --host=miyoomini --enable-release --enable-plugins --default-dynamic --enable-engine=testbed --enable-ext-neon --with-freetype2-prefix=/opt/miyoomini-toolchain/arm-linux-gnueabihf/libc/usr/bin/

make -j5 all sd-root sd-zip
