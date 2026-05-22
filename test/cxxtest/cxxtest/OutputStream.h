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

#ifndef __cxxtest__OutputStream_h__
#define __cxxtest__OutputStream_h__

//
//  Since we cannot rely on the standard
// iostreams, this header defines a base class
// analogout to std::ostream.
//

#ifdef _CXXTEST_HAVE_STD

#include <sstream>
#ifdef _CXXTEST_OLD_STD
#   include <iostream.h>
#else // !_CXXTEST_OLD_STD
#   include <iostream>
#endif // _CXXTEST_OLD_STD

#endif // _CXXTEST_HAVE_STD

#include <stdio.h>

namespace CxxTest
{
class OutputStream
{
public:
    virtual ~OutputStream() {}
    virtual void flush() {}
    virtual OutputStream &operator<<(unsigned /*number*/) { return *this; }
    virtual OutputStream &operator<<(const char * /*string*/) { return *this; }

    typedef void (*Manipulator)(OutputStream &);

    virtual OutputStream &operator<<(Manipulator m) { m(*this); return *this; }
    static void endl(OutputStream &o) { (o << "\n").flush(); }
};

// Support use in CxxTest namespace
void endl(OutputStream &o)
{
    OutputStream::endl(o);
}

#ifdef _CXXTEST_HAVE_STD

class StdOStreamAdapter : public OutputStream
{
    CXXTEST_STD(ostream) &_o;
public:
    StdOStreamAdapter(CXXTEST_STD(ostream) &o) : _o(o) {}
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

#endif // _CXXTEST_HAVE_STD

class StdioFileAdapter : public OutputStream
{
    StdioFileAdapter(const StdioFileAdapter &);
    StdioFileAdapter &operator=(const StdioFileAdapter &);

    FILE *_o;

public:
    StdioFileAdapter(FILE *o) : _o(o) {}
    void flush() { fflush(_o); }
    OutputStream &operator<<(unsigned i) { fprintf(_o, "%u", i); return *this; }
    OutputStream &operator<<(const char *s) { fputs(s, _o); return *this; }
    OutputStream &operator<<(Manipulator m) { return OutputStream::operator<<(m); }
};

}

#endif // __cxxtest__OutputStream_h__
