#!/bin/bash
export VARIANT=gcw0
export PATH=/opt/gcw0-toolchain/usr/bin:/opt/gcw0-toolchain/mipsel-gcw0-linux-uclibc/sysroot/usr/include:/opt/gcw0-toolchain/bin:$PATH

./configure --host=opendingux --disable-fluidsynth --disable-all-engines --enable-engine=scumm,scumm-7-8,he,mohawk,cstime,myst,mystme,riven,sky,tinsel,agi,queen,neverhood,touche,bladerunner,titanic --disable-alsa --disable-vorbis --disable-mt32emu --disable-nuked-opl --disable-cloud --disable-eventrecorder --disable-tts --enable-release --enable-plugins --default-dynamic && make -j12 od-make-opk && ls -l scummvm_gcw0.opk
