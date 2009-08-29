#!/bin/sh
DIR=`busybox dirname "$0"`
LIBDIR=/mmc/mmca1/.system/lib
export LD_PRELOAD=$LIBDIR/libz.so:$LIBDIR/libSDL-1.2.so.0:$LIBDIR/libvorbis.so.0:$LIBDIR/libvorbisfile.so.3:$LIBDIR/libogg.so.0:$LIBDIR/libmad.so.0:$LIBDIR/libutil.so.1:$LIBDIR/libtaskman.so
export SDL_QT_MODIFICATOR=1
export HOME=/$DIR
cd $DIR
rm /mmc/mmca1/.system/mySDL.cfg
cp $DIR/mySDL.cfg /mmc/mmca1/.system/mySDL.cfg
./scummvm --path=$DIR --gfx-mode=1x > $DIR/scummvm.log
