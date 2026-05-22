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

#ifndef __cxxtest__StdioFilePrinter_h__
#define __cxxtest__StdioFilePrinter_h__

//
// The StdioFilePrinter is a simple TestListener that
// just prints "OK" if everything goes well, otherwise
// reports the error in the format of compiler messages.
// This class uses <stdio.h>, i.e. FILE * and fprintf().
//

#include <cxxtest/ErrorFormatter.h>
#include <stdio.h>

namespace CxxTest
{
class StdioFilePrinter : public ErrorFormatter
{
public:
    StdioFilePrinter(FILE *o, const char *preLine = ":", const char *postLine = "") :
        ErrorFormatter(new StdioFileAdapter(o), preLine, postLine) {}
    virtual ~StdioFilePrinter() { delete outputStream(); }
};
}

#endif // __cxxtest__StdioFilePrinter_h__
