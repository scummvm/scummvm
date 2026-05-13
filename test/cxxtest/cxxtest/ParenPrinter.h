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

#ifndef __cxxtest__ParenPrinter_h__
#define __cxxtest__ParenPrinter_h__

//
// The ParenPrinter is identical to the ErrorPrinter, except it
// prints the line number in a format expected by some compilers
// (notably, MSVC).
//

#include <cxxtest/ErrorPrinter.h>

namespace CxxTest
{
class ParenPrinter : public ErrorPrinter
{
public:
    ParenPrinter(CXXTEST_STD(ostream) &o = CXXTEST_STD(cout)) : ErrorPrinter(o, "(", ")") {}
};
}

#endif // __cxxtest__ParenPrinter_h__
