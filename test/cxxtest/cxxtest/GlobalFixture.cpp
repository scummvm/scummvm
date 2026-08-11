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

#ifndef __cxxtest__GlobalFixture_cpp__
#define __cxxtest__GlobalFixture_cpp__

#include <cxxtest/GlobalFixture.h>

namespace CxxTest
{
bool GlobalFixture::setUpWorld() { return true; }
bool GlobalFixture::tearDownWorld() { return true; }
bool GlobalFixture::setUp() { return true; }
bool GlobalFixture::tearDown() { return true; }

GlobalFixture::GlobalFixture() { attach(_list); }
GlobalFixture::~GlobalFixture() { detach(_list); }

GlobalFixture *GlobalFixture::firstGlobalFixture() { return (GlobalFixture *)_list.head(); }
GlobalFixture *GlobalFixture::lastGlobalFixture() { return (GlobalFixture *)_list.tail(); }
GlobalFixture *GlobalFixture::nextGlobalFixture() { return (GlobalFixture *)next(); }
GlobalFixture *GlobalFixture::prevGlobalFixture() { return (GlobalFixture *)prev(); }
}

#endif // __cxxtest__GlobalFixture_cpp__
