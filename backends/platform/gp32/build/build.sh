#!/bin/bash

echo Quick script to make building all the time less painful.

# Set the paths up here to support the build.

export PATH=/opt/devkitARM/bin:$PATH
export CXXFLAGS=-march=armv4t
export LDFLAGS=-static

echo Copy Makefile upto the root of the source tree.
cp -f Makefile ../../../../Makefile

cd ../../../..

echo Building ScummVM for GP2X.
make

echo Build for GP32 - complete - Please check build logs.
