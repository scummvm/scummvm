#!/bin/bash

. GetGlobals.sh
export PATH=$CXXTEST/bin:$PATH

# @main:
cxxtestgen -f --error-printer -o runner.cpp MyTestSuite1.h MyTestSuite2.h MyTestSuite4.h
# @:main

# @compile:
g++ -o runner -I$CXXTEST runner.cpp
# @:compile

# @help:
./runner --help
# @:help
./runner --help &> runner13.help.txt

# @helpTests:
./runner --help-tests
# @:helpTests
./runner --help-tests &> runner13.helpTests.txt

# @MyTestSuite2:
./runner MyTestSuite2
# @:MyTestSuite2
./runner MyTestSuite2 &> runner13.MyTestSuite2.txt

# @testMultiplication:
./runner MyTestSuite2 testMultiplication
# @:testMultiplication
./runner MyTestSuite2 testMultiplication &> runner13.testMultiplication.txt

# @testMultiplicationVerbose:
./runner -v MyTestSuite2 testMultiplication
# @:testMultiplicationVerbose
./runner -v MyTestSuite2 testMultiplication &> runner13.testMultiplicationVerbose.txt

\rm -f runner runner.cpp

