#!/bin/bash -eux
# -e: Exit immediately if a command exits with a non-zero status.
# -u: Treat unset variables as an error when substituting.
# -x: Display expanded script commands

mkdir -p build-release
cd build-release

PLATFORM=m68k-atari-mintelf
FASTCALL=false

export ASFLAGS="-m68020-60"
export CXXFLAGS="-m68020-60 -DUSE_MOVE16 -DUSE_SUPERVIDEL -DUSE_SV_BLITTER"
export LDFLAGS="-m68020-60"
export PKG_CONFIG_LIBDIR="$(${PLATFORM}-gcc -print-sysroot)/usr/lib/m68020-60/pkgconfig"

if $FASTCALL
then
	ASFLAGS="$ASFLAGS -mfastcall"
	CXXFLAGS="$CXXFLAGS -mfastcall"
	LDFLAGS="$LDFLAGS -mfastcall"
fi

if [ ! -f config.log ]
then
../configure \
	--backend=atari \
	--host=${PLATFORM} \
	--enable-release \
	--disable-png \
	--disable-enet \
	--disable-mt32emu \
	--disable-lua \
	--disable-nuked-opl \
	--disable-16bit \
	--disable-scalers \
	--disable-aspect \
	--disable-translation \
	--disable-eventrecorder \
	--disable-tts \
	--opengl-mode=none \
	--enable-verbose-build \
	--enable-text-console \
	--disable-engine=hugo,director,cine,ultima \
	--disable-detection-full
fi

make -j$(getconf _NPROCESSORS_CONF)
rm -rf dist-generic
make dist-generic

# create symbol file and strip
${PLATFORM}-nm -C dist-generic/scummvm/scummvm.ttp | grep -vF ' .L' | grep ' [TtWV] ' | ${PLATFORM}-c++filt | sort -u > dist-generic/scummvm/scummvm.sym
${PLATFORM}-strip -s dist-generic/scummvm/scummvm.ttp

# remove unused files; absent gui-icons.dat massively speeds up startup time (used for the grid mode)
rm dist-generic/scummvm/data/{achievements,encoding,gui-icons,macgui,shaders}.dat

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

cd dist-generic
mv scummvm scummvm-2.9.0-atari-full
zip -r -9 scummvm-2.9.0-atari-full.zip scummvm-2.9.0-atari-full
cd -

mv dist-generic/scummvm-2.9.0-atari-full.zip ..
