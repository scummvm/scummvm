#!/bin/bash -eux
# -e: Exit immediately if a command exits with a non-zero status.
# -u: Treat unset variables as an error when substituting.
# -x: Display expanded script commands

mkdir -p build-release030
cd build-release030

export ASFLAGS="-m68030"
export CXXFLAGS="-m68030 -DDISABLE_FANCY_THEMES"
export LDFLAGS="-m68030"

if [ ! -f config.log ]
then
../configure \
	--backend=atari \
	--host=m68k-atari-mint \
	--enable-release \
	--disable-mt32emu \
	--disable-lua \
	--disable-nuked-opl \
	--disable-16bit \
	--disable-highres \
	--disable-scalers \
	--disable-translation \
	--disable-eventrecorder \
	--disable-tts \
	--disable-bink \
	--opengl-mode=none \
	--enable-verbose-build \
	--enable-text-console \
	--disable-engine=hugo,director
fi

make -j 16
rm -rf dist-generic
make dist-generic

# remove unused files
rm -f dist-generic/scummvm/data/*.zip dist-generic/scummvm/data/{gui-icons,achievements,macgui,shaders}.dat

# readme.txt
cp ../backends/platform/atari/readme.txt dist-generic/scummvm
unix2dos dist-generic/scummvm/readme.txt
