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

#ifndef __cxxtest__XmlPrinter_h__
#define __cxxtest__XmlPrinter_h__

//
// The XmlPrinter is a simple TestListener that
// prints JUnit style xml to the output stream
//


#include <cxxtest/Flags.h>

#ifndef _CXXTEST_HAVE_STD
#   error "XmlPrinter can't be used without std"
#endif // _CXXTEST_HAVE_STD

#include <cxxtest/XmlFormatter.h>
#include <cxxtest/StdValueTraits.h>

#include <sstream>
#ifdef _CXXTEST_OLD_STD
#   include <iostream.h>
#else // !_CXXTEST_OLD_STD
#   include <iostream>
#endif // _CXXTEST_OLD_STD

namespace CxxTest
{
class XmlPrinter : public XmlFormatter
{
public:
    XmlPrinter(CXXTEST_STD(ostream) &o = CXXTEST_STD(cout), const char* /*preLine*/ = ":", const char* /*postLine*/ = "") :
        XmlFormatter(new StdOStreamAdapter(o), new StdOStreamAdapter(ostr), &ostr) {}

    virtual ~XmlPrinter()
    {
        delete outputStream();
        delete outputFileStream();
    }

private:

    std::ostringstream ostr;
};
}

#endif // __cxxtest__XmlPrinter_h__
