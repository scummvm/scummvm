#!/bin/bash
export VARIANT=lepus
export PATH=/opt/lepus-toolchain/usr/bin:/opt/lepus-toolchain/mipsel-lepus-linux-musl/sysroot/usr/include:/opt/lepus-toolchain/bin:$PATH

./configure --host=opendingux --disable-fluidsynth --disable-highres --disable-all-engines --enable-engine=scumm,scumm-7-8,sky,tinsel,agi,queen --disable-alsa --disable-vorbis --disable-mt32emu --disable-nuked-opl --disable-cloud --disable-eventrecorder --disable-tts --enable-release --enable-plugins --default-dynamic && make -j12 od-make-opk && ls -l scummvm_lepus.opk
