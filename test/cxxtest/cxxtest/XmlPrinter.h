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
#   define _CXXTEST_HAVE_STD
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
        XmlFormatter(new Adapter(o), new Adapter(ostr), &ostr) {}

    virtual ~XmlPrinter()
    {
        delete outputStream();
        delete outputFileStream();
    }

private:

    std::ostringstream ostr;

    class Adapter : public OutputStream
    {
        CXXTEST_STD(ostream) &_o;
    public:
        Adapter(CXXTEST_STD(ostream) &o) : _o(o) {}
        void flush() { _o.flush(); }
        OutputStream &operator<<(const char *s) { _o << s; return *this; }
        OutputStream &operator<<(Manipulator m) { return OutputStream::operator<<(m); }
        OutputStream &operator<<(unsigned i)
        {
            char s[1 + 3 * sizeof(unsigned)];
            numberToString(i, s);
            _o << s;
            return *this;
        }
    };
};
}

#endif // __cxxtest__XmlPrinter_h__
