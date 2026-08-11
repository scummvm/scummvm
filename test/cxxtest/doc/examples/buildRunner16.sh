#!/bin/bash

. GetGlobals.sh
export PATH=$CXXTEST/bin:$PATH

# @main:
cxxtestgen --error-printer -o runner.cpp MockTestSuite.h
# @:main

# @compile:
g++ -o runner -I. -I$CXXTEST runner.cpp time_mock.cpp rand_example.cpp
# @:compile

./runner
\rm -f runner runner.cpp
