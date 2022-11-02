#!/bin/sh

cd `dirname $0`

if [ ! -f $HOME/.scummvmrc ] ; then
	cp ./scummvmrc $HOME/.scummvmrc
fi

exec ./scummvm 2>&1 >/var/tmp/scummvm.log
