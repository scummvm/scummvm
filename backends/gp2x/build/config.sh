#!/bin/bash

echo Quick script to make running configure all the time less painful 
echo and let all the build work be done from the backend/build folder.

# Set the paths up here to generate the config.

PATH=/tools/open2x_gcc/gcc-4.0.2-glibc-2.3.5/arm-open2x-linux/arm-open2x-linux/bin:$PATH
PATH=/tools/open2x_gcc/gcc-4.0.2-glibc-2.3.5/arm-open2x-linux/bin:$PATH

# Export the tool names for cross-compiling
export CXX=arm-open2x-linux-g++
export CXXFLAGS=-march=armv4t
export CPPFLAGS=-I/tools/open2x_gcc/gcc-4.0.2-glibc-2.3.5/arm-open2x-linux/include
export LDFLAGS=-L/tools/open2x_gcc/gcc-4.0.2-glibc-2.3.5/arm-open2x-linux/lib 
export DEFINES=-DNDEBUG 

# Edit the configure line to suit.
cd ../../..
./configure --backend=gp2x --disable-mt32emu --host=gp2x --disable-mpeg2 --disable-flac  --disable-nasm --disable-hq-scalers --with-sdl-prefix=/tools/open2x_gcc/gcc-4.0.2-glibc-2.3.5/arm-open2x-linux/bin --enable-tremor --with-tremor-prefix=/tools/open2x_gcc/gcc-4.0.2-glibc-2.3.5/arm-open2x-linux --enable-zlib --with-zlib-prefix=/tools/open2x_gcc/gcc-4.0.2-glibc-2.3.5/arm-open2x-linux --enable-mad --with-mad-prefix=/tools/open2x_gcc/gcc-4.0.2-glibc-2.3.5/arm-open2x-linux

echo Generating config for GP2X complete. Check for errors.