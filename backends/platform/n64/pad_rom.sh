#!/bin/bash

TARGET="scummvm"
BASESIZE=2097152

CARTSIZE=`ls -l $TARGET.v64 | cut -d" " -f5`

REMAINDER=`echo $CARTSIZE % $BASESIZE   | bc`
REMAINDER=`echo $BASESIZE - $REMAINDER  | bc`
CARTSIZE=`echo $CARTSIZE + $REMAINDER  | bc`

ucon64 -q --n64 --v64 --chk --padn=$CARTSIZE  $TARGET.v64

