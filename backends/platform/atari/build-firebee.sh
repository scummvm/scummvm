#!/bin/bash -eux
# -e: Exit immediately if a command exits with a non-zero status.
# -u: Treat unset variables as an error when substituting.
# -x: Display expanded script commands

mkdir -p build-firebee
cd build-firebee

PLATFORM=m68k-atari-mintelf
FASTCALL=false
export CXXFLAGS="-mcpu=5475"
export LDFLAGS="-mcpu=5475"
#export CXXFLAGS="-m68020-60"
#export LDFLAGS="-m68020-60"

CPU_DIR=$(${PLATFORM}-gcc ${CXXFLAGS} -print-multi-directory)

export PKG_CONFIG_LIBDIR="$(${PLATFORM}-gcc -print-sysroot)/usr/lib/${CPU_DIR}/pkgconfig"

if $FASTCALL
then
	CXXFLAGS="$CXXFLAGS -mfastcall"
	LDFLAGS="$LDFLAGS -mfastcall"
fi

if [ -f ../backends/platform/atari/.patched ]
then
	echo "FireBee SDL target shouldn't contain any ATARI patches!"
	exit 1
fi


if [ ! -f config.log ]
then
../configure \
	--backend=sdl \
	--host=${PLATFORM} \
	--with-sdl-prefix="$(${PLATFORM}-gcc -print-sysroot)/usr/bin/${CPU_DIR}" \
	--with-freetype2-prefix="$(${PLATFORM}-gcc -print-sysroot)/usr/bin/${CPU_DIR}" \
	--with-mikmod-prefix="$(${PLATFORM}-gcc -print-sysroot)/usr/bin/${CPU_DIR}" \
	--enable-release \
	--enable-verbose-build
fi

make -j$(getconf _NPROCESSORS_CONF) fbdist
