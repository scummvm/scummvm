#!/bin/sh
GAMES=/mmc/mmca1/.Games
DIR=`busybox dirname "$0"`
LIBDIR=/mmc/mmca1/.system/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$LIBDIR
export SDL_QT_MODIFICATOR=1
export HOME=$GAMES
cd $DIR
rm /mmc/mmca1/.system/mySDL.cfg
cp $DIR/mySDL.cfg /mmc/mmca1/.system/mySDL.cfg
exec $DIR/scummvm --gfx-mode=1x > $DIR/scummvm.log
