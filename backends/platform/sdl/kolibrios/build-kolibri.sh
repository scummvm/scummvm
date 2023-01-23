#!/bin/bash

set -e

export KOS32_SDK_DIR=$HOME/sdk
export KOS32_AUTOBUILD=$HOME/autobuild

# Use plugins for both engines and detection as KolibriOS has a limit per executable module
./configure --host=kos32 --enable-release --enable-plugins --default-dynamic --enable-detection-dynamic --enable-engine=testbed

make -j5 all zip-root scummvm-zip
