#!/bin/bash

. GetGlobals.sh
export PATH=$CXXTEST/bin:$PATH

# @part:
cxxtestgen --part --error-printer -o MyTestSuite1.cpp MyTestSuite1.h
cxxtestgen --part --error-printer -o MyTestSuite2.cpp MyTestSuite2.h
# @:part

# @root:
cxxtestgen --root --error-printer -o runner.cpp
# @:root

# @compile:
g++ -o runner -I$CXXTEST runner.cpp MyTestSuite1.cpp MyTestSuite2.cpp
# @:compile

./runner -v

rm -f MyTestSuite1.cpp MyTestSuite2.cpp runner.cpp runner
