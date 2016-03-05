#!/bin/bash

export PATH=/opt/gcw0-toolchain/usr/bin:$PATH

# Disable high resolution engines since we have 320x240 hardware
./configure --host=gcw0 --enable-plugins --default-dynamic --enable-release  --disable-engine=he,mohawk,neverhood,sword1,sword2,sword25,toltecs,wintermute,zvision --disable-mt32emu --disable-hq-scalers && make -j6 gcw-opk && ls -l scummvm.opk
