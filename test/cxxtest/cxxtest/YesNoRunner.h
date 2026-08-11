/*
-------------------------------------------------------------------------
 CxxTest: A lightweight C++ unit testing library.
 Copyright (c) 2008 Sandia Corporation.
 This software is distributed under the LGPL License v3
 For more information, see the COPYING file in the top CxxTest directory.
 Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------
*/

#ifndef __cxxtest__YesNoRunner_h__
#define __cxxtest__YesNoRunner_h__

//
// The YesNoRunner is a simple TestListener that
// just returns true iff all tests passed.
//

#include <cxxtest/TestRunner.h>
#include <cxxtest/TestListener.h>

namespace CxxTest
{
class YesNoRunner : public TestListener
{
public:
    YesNoRunner()
    {
    }

    int run()
    {
        TestRunner::runAllTests(*this);
        return tracker().failedTests();
    }
};
}

#endif // __cxxtest__YesNoRunner_h__
