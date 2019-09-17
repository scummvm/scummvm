#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`
ANDROIDSDL=../../../../androidsdl

#ln -sf libtremor.a $ANDROIDSDL/project/obj/local/$1/libvorbisidec.a
ln -sf libflac.a $ANDROIDSDL/project/obj/local/$1/libFLAC.a
ln -sf libvorbis.a $ANDROIDSDL/project/obj/local/$1/libvorbisfile.a
ln -sf libtheora.so $ANDROIDSDL/project/obj/local/$1/libtheoradec.so
ln -sf libsdl_net.so $ANDROIDSDL/project/obj/local/$1/libSDL_net.so
ln -sf libglshim.a $ANDROIDSDL/project/obj/local/$1/libGL.a

mkdir -p scummvm/bin-$1

if [ \! -f scummvm/bin-$1/config.mk ] ; then
	$ANDROIDSDL/project/jni/application/setEnvironment-$1.sh sh -c "cd scummvm/bin-$1 && env LIBS='-lflac -lvorbis -logg -lmad -lz -lgcc -ltheora -lpng -lfreetype -lfaad -lgnustl_static' ../configure --host=androidsdl-$1 --enable-zlib --enable-vorbis --enable-mad --enable-flac --enable-png --enable-theoradec --disable-sdlnet --disable-libcurl --disable-cloud --enable-vkeybd --enable-mt32emu --disable-readline --disable-nasm --disable-timidity --disable-fluidsynth --datadir=. "
fi
$ANDROIDSDL/project/jni/application/setEnvironment-$1.sh make -j4 -C scummvm/bin-$1
make -C scummvm/bin-$1 androidsdl
mv -f scummvm/bin-$1/scummvm*.z* AndroidData
cp -f scummvm/bin-$1/scummvm libapplication-$1.so
