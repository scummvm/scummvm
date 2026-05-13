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

#ifndef __cxxtest__GlobalFixture_h__
#define __cxxtest__GlobalFixture_h__

#include <cxxtest/LinkedList.h>

namespace CxxTest
{
class GlobalFixture : public Link
{
public:
    virtual bool setUpWorld();
    virtual bool tearDownWorld();
    virtual bool setUp();
    virtual bool tearDown();

    GlobalFixture();
    virtual ~GlobalFixture();

    static GlobalFixture *firstGlobalFixture();
    static GlobalFixture *lastGlobalFixture();
    GlobalFixture *nextGlobalFixture();
    GlobalFixture *prevGlobalFixture();

private:
    static List _list;
};
}

#endif // __cxxtest__GlobalFixture_h__
