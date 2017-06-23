#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

#ln -sf libtremor.a $LOCAL_PATH/../../../obj/local/$1/libvorbisidec.a
ln -sf libflac.a $LOCAL_PATH/../../../obj/local/$1/libFLAC.a
ln -sf libvorbis.a $LOCAL_PATH/../../../obj/local/$1/libvorbisfile.a
ln -sf libtheora.so $LOCAL_PATH/../../../obj/local/$1/libtheoradec.so
ln -sf libsdl_net.so $LOCAL_PATH/../../../obj/local/$1/libSDL_net.so
ln -sf libglshim.a $LOCAL_PATH/../../../obj/local/$1/libGL.a

mkdir -p scummvm/bin-$1

if [ \! -f scummvm/bin-$1/config.mk ] ; then
	../setEnvironment-$1.sh sh -c "cd scummvm/bin-$1 && env LIBS='-lflac -lvorbis -logg -lmad -lz -lgcc -ltheora -lpng -lfreetype -lfaad -lgnustl_static' ../configure --host=androidsdl-$1 --enable-zlib --enable-vorbis --enable-mad --enable-flac --enable-png --enable-theoradec --enable-sdlnet --enable-libcurl --enable-vkeybd --enable-release --enable-mt32emu --disable-readline --disable-nasm --disable-timidity --disable-fluidsynth --datadir=. "
fi
../setEnvironment-$1.sh make -C scummvm/bin-$1 && cp -f scummvm/bin-$1/scummvm libapplication-$1.so
