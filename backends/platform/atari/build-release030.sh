#!/bin/bash -eux
# -e: Exit immediately if a command exits with a non-zero status.
# -u: Treat unset variables as an error when substituting.
# -x: Display expanded script commands

mkdir -p build-release030
cd build-release030

PLATFORM=m68k-atari-mintelf
FASTCALL=false

export ASFLAGS="-m68030"
export CXXFLAGS="-m68030 -DDISABLE_FANCY_THEMES"
export LDFLAGS="-m68030"
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
	--disable-highres \
	--disable-bink \
	--enable-verbose-build \
	--disable-engine=hugo,director,cine,ultima
fi

make -j$(getconf _NPROCESSORS_CONF)
rm -rf dist-generic
make dist-generic

# create symbol file and strip
rm dist-generic/scummvm/scummvm.prg
cp -a scummvm.prg dist-generic/scummvm/scummvm.prg
${PLATFORM}-nm -C dist-generic/scummvm/scummvm.prg | grep -vF ' .L' | grep ' [TtWV] ' | ${PLATFORM}-c++filt | sort -u > dist-generic/scummvm/scummvm.sym
${PLATFORM}-strip -s dist-generic/scummvm/scummvm.prg

# remove unused files
rm dist-generic/scummvm/data/*.zip dist-generic/scummvm/data/{achievements,classicmacfonts,encoding,gui-icons,macgui,shaders}.dat

# rename remaining files still not fitting into the 8+3 limit (this has to be supported by the backend, too)
mv dist-generic/scummvm/data/supernova.dat dist-generic/scummvm/data/supernov.dat
mv dist-generic/scummvm/data/teenagent.dat dist-generic/scummvm/data/teenagen.dat

# readme.txt
cp ../backends/platform/atari/readme.txt dist-generic/scummvm
unix2dos dist-generic/scummvm/readme.txt

cd dist-generic
mv scummvm scummvm-3.1.0git-atari-lite
zip -r -9 scummvm-3.1.0git-atari-lite.zip scummvm-3.1.0git-atari-lite
cd -

mv dist-generic/scummvm-3.1.0git-atari-lite.zip ..
