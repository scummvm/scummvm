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

if [ ! -f ../backends/platform/atari/.patched ]
then
	cd .. && cat backends/platform/atari/patches/print_rate.patch | patch -p1 && cd -
	cd .. && cat backends/platform/atari/patches/tooltips.patch | patch -p1 && cd -
	touch ../backends/platform/atari/.patched
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

make -j$(getconf _NPROCESSORS_CONF) atarilitedist
