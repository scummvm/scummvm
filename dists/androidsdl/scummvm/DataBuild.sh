#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

rm AndroidData/scummvm*
make -C scummvm androidsdl
cp -f scummvm/scummvm*.z* AndroidData
rm scummvm/scummvm*.z*