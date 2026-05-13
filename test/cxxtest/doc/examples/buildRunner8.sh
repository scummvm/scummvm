#!/bin/bash

. GetGlobals.sh
export PATH=$CXXTEST/bin:$PATH

# @main:
cxxtestgen --gui=X11Gui -o runner.cpp MyTestSuite2.h ../../sample/gui/GreenYellowRed.h
# @:main

# @compile:
/opt/local/bin/g++-mp-4.4 -o runner -I$CXXTEST runner.cpp -L/opt/local/lib -lX11
# @:compile

./runner
\rm -f runner runner.cpp
