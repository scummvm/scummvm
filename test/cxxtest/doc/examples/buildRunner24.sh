#!/bin/bash

. GetGlobals.sh
export PATH=$CXXTEST/bin:$PATH

# @main:
cxxtestgen --fog --error-printer -o runner.cpp Namespace2.h
# @:main

# @compile:
g++ -o runner -I$CXXTEST -I. runner.cpp
# @:compile

./runner
\rm -f runner runner.cpp

