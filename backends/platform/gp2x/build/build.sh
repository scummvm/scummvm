#!/bin/bash

echo Quick script to make building all the time less painful.

# Set the paths up here to support the build.

export PATH=/tools/open2x_gcc/gcc-4.0.2-glibc-2.3.5/arm-open2x-linux/arm-open2x-linux/bin:$PATH
export PATH=/tools/open2x_gcc/gcc-4.0.2-glibc-2.3.5/arm-open2x-linux/bin:$PATH
export CXX=arm-open2x-linux-g++
export CC=arm-open2x-linux-gcc
export CXXFLAGS=-march=armv4t
export LDFLAGS=-static

cd ../../../..

echo Building ScummVM for GP2X.
make

echo Build for GP2X - SDL - complete - Please check build logs.
