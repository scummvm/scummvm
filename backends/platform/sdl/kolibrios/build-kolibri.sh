#!/bin/bash

set -e

export KOS32_SDK_DIR=$HOME/kolibrios/contrib/sdk
export KOS32_AUTOBUILD=$HOME/autobuild

# Use plugins for both engines and detection as KolibriOS has a limit per executable module
./configure --host=kos32 --enable-release --enable-plugins --default-dynamic --enable-detection-dynamic --with-vorbis-prefix=$KOS32_SDK_DIR/sources/libvorbis-1.3.7  --with-ogg-prefix=$KOS32_SDK_DIR/sources/libogg-1.3.5 --enable-engine=testbed

make -j12 all zip-root scummvm-zip
