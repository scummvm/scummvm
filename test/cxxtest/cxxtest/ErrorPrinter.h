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

#ifndef __cxxtest__ErrorPrinter_h__
#define __cxxtest__ErrorPrinter_h__

//
// The ErrorPrinter is a simple TestListener that
// just prints "OK" if everything goes well, otherwise
// reports the error in the format of compiler messages.
// The ErrorPrinter uses std::cout or stdout
//

#include <cxxtest/ErrorFormatter.h>

namespace CxxTest
{
class ErrorPrinter : public ErrorFormatter
{
public:
#ifdef _CXXTEST_HAVE_STD
    ErrorPrinter(CXXTEST_STD(ostream) &o = CXXTEST_STD(cout), const char *preLine = ":", const char *postLine = "",
                 const char *errorString = "Error",
                 const char *warningString = "Warning") :
        ErrorFormatter(new StdOStreamAdapter(o), preLine, postLine, errorString, warningString) {}
#else
    ErrorPrinter(FILE *o = stdout, const char *preLine = ":", const char *postLine = "",
                 const char *errorString = "Error",
                 const char *warningString = "Warning") :
        ErrorFormatter(new StdioFileAdapter(o), preLine, postLine, errorString, warningString) {}
#endif // _CXXTEST_HAVE_STD

    virtual ~ErrorPrinter() { delete outputStream(); }
};
}

#endif // __cxxtest__ErrorPrinter_h__
