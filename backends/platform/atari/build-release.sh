#!/bin/bash -eux
# -e: Exit immediately if a command exits with a non-zero status.
# -u: Treat unset variables as an error when substituting.
# -x: Display expanded script commands

mkdir -p build-release
cd build-release

export ASFLAGS="-m68020-60"
export CXXFLAGS="-m68020-60 -DUSE_MOVE16 -DUSE_SUPERVIDEL -DUSE_SV_BLITTER"
export LDFLAGS="-m68020-60"

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
	--disable-scalers \
	--disable-translation \
	--disable-eventrecorder \
	--disable-tts \
	--disable-bink \
	--opengl-mode=none \
	--enable-verbose-build \
	--enable-text-console \
	--disable-engine=director
fi

make -j 16
rm -rf dist-generic
make dist-generic

# remove unused files; absent gui-icons.dat massively speeds up startup time (used for the grid mode)
rm -f dist-generic/scummvm/data/{gui-icons,achievements,macgui,shaders}.dat

# move themes into 'themes' folder (with compression level zero for faster depacking)
mkdir -p dist-generic/scummvm/themes
cd dist-generic/scummvm/themes
(
for f in ../data/*.zip
do
	unzip -d tmp "$f" && cd tmp && zip -0 ../$(basename "$f") * && cd .. && rm -r tmp && rm "$f"
done
)
cd -

# readme.txt
cp ../backends/platform/atari/readme.txt dist-generic/scummvm
unix2dos dist-generic/scummvm/readme.txt
