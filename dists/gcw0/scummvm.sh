#!/bin/sh

cd `dirname $0`

if [ ! -f $HOME/.scummvmrc ] ; then
	cp /mnt/ScummVM/scummvmrc $HOME/.scummvmrc
fi

exec ./scummvm
