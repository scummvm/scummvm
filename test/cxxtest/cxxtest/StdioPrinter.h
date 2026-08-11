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

#ifndef __cxxtest__StdioPrinter_h__
#define __cxxtest__StdioPrinter_h__

//
// The StdioPrinter is an StdioFilePrinter which defaults to stdout.
// This should have been called StdOutPrinter or something, but the name
// has been historically used.
//

#include <cxxtest/StdioFilePrinter.h>

namespace CxxTest
{
class StdioPrinter : public StdioFilePrinter
{
public:
    StdioPrinter(FILE *o = stdout, const char *preLine = ":", const char *postLine = "") :
        StdioFilePrinter(o, preLine, postLine) {}
};
}

#endif // __cxxtest__StdioPrinter_h__
