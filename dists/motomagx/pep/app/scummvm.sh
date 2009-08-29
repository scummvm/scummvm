#!/bin/sh
GAMES=/mmc/mmca1/.Games
export DATA_DIR=$GAMES/.data/ScummVM
export LD_LIBRARY_PATH=$GAMES/.lib:$LD_LIBRARY_PATH
# uncomment next line if you wanna 270-degrees clockwise rotated screen 
#export SDL_QT_INVERT_ROTATION=1

GAME_PATH=`basename $0`
GAME_PATH=`echo $0 | sed -e 's/'$GAME_PATH'//g'`
cd "$GAME_PATH"

export HOME="$GAME_PATH"

export LD_PREADLOAD=/usr/lib/libtaskman.so

exec ./scummvm -g 1x
