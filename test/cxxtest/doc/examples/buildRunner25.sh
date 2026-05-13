#!/bin/bash

. GetGlobals.sh
export PATH=$CXXTEST/bin:$PATH

# @main:
cxxtestgen --error-printer -o runner.cpp MyTestSuite12.h
# @:main

# @compile:
g++ -o runner -I$CXXTEST runner.cpp
# @:compile

./runner > buildRunner.log
# @run:
./runner
# @:run
\rm -f runner runner.cpp


cp MyTestSuite12.h runner2.cpp
g++ -o runner2 -I$CXXTEST runner2.cpp
./runner2
\rm -f runner2 runner2.cpp
