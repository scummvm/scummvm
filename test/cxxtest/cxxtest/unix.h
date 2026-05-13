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

#ifndef UNIX_ERROR_PRINTER_H_N4C6JUX4
#define UNIX_ERROR_PRINTER_H_N4C6JUX4

#ifndef _CXXTEST_HAVE_STD
#   define _CXXTEST_HAVE_STD
#endif // _CXXTEST_HAVE_STD

#include <cxxtest/Flags.h>
#include <cxxtest/TestRunner.h>
#include <cxxtest/TestListener.h>
#include <cxxtest/TestTracker.h>
#include <cxxtest/ValueTraits.h>
#include <cxxtest/StdValueTraits.h>
#include <cxxtest/ErrorFormatter.h> // CxxTest::OutputStream

namespace CxxTest
{
class UNIXErrorFormatter : public TestListener
{
public:
    UNIXErrorFormatter(OutputStream *o, const char *preLine = ":", const char *postLine = "") :
        _reported(false),
        _o(o),
        _preLine(preLine),
        _postLine(postLine)
    {
    }

    int run()
    {
        TestRunner::runAllTests(*this);
        return tracker().failedTests();
    }

    static void totalTests(OutputStream &o)
    {
        char s[WorldDescription::MAX_STRLEN_TOTAL_TESTS];
        const WorldDescription &wd = tracker().world();
        o << wd.strTotalTests(s) << (wd.numTotalTests() == 1 ? " test" : " tests");
    }

    void enterTest(const TestDescription &)
    {
        _reported = false;
    }

    void leaveWorld(const WorldDescription &desc)
    {
        if (tracker().failedTests())
        {
            (*_o) << "Failed " << tracker().failedTests() << " of " << totalTests << endl;
            unsigned numPassed = desc.numTotalTests() - tracker().failedTests() - tracer().skippedTests();
            unsigned numTotal = desc.numTotalTests() - tracker().skippedTests();
            if (numTotal == 0)
            {
                (*_o) << "Success rate: 100%" << endl;
            }
            else
            {
                (*_o) << "Success rate: " << (unsigned)(numPassed * 100.0 / numTotal) << "%" << endl;
            }
        }
    }

    void trace(const char *file, int line, const char *expression)
    {
        stop(file, line) << "Trace: " <<
                         expression << endl;
    }

    void warning(const char *file, int line, const char *expression)
    {
        stop(file, line) << "Warning: " <<
                         expression << endl;
    }

    void skippedTest(const char *file, int line, const char *expression)
    {
        stop(file, line) << "Warning: Test skipped: " <<
                         expression << endl;
    }

    void failedTest(const char *file, int line, const char *expression)
    {
        stop(file, line) << "Error: Test failed: " <<
                         expression << endl;
    }

    void failedAssert(const char *file, int line, const char *expression)
    {
        stop(file, line) << "Error: Assertion failed: " <<
                         expression << endl;
    }

    void failedAssertEquals(const char *file, int line,
                            const char *xStr, const char *yStr,
                            const char *x, const char *y)
    {
        stop(file, line) << "Error: Expected (" <<
                         xStr << " == " << yStr << "), found (" <<
                         x << " != " << y << ")" << endl;
    }

    void failedAssertSameData(const char *file, int line,
                              const char *xStr, const char *yStr,
                              const char *sizeStr, const void *x,
                              const void *y, unsigned size)
    {
        stop(file, line) << "Error: Expected " << sizeStr << " (" << size << ") bytes to be equal at (" <<
                         xStr << ") and (" << yStr << "), found:" << endl;
        dump(x, size);
        (*_o) << "     differs from" << endl;
        dump(y, size);
    }

    void failedAssertSameFiles(const char* file, int line,
                               const char*, const char*,
                               const char* explanation
                              )
    {
        stop(file, line) << "Error: " << explanation << endl;
    }

    void failedAssertDelta(const char *file, int line,
                           const char *xStr, const char *yStr, const char *dStr,
                           const char *x, const char *y, const char *d)
    {
        stop(file, line) << "Error: Expected (" <<
                         xStr << " == " << yStr << ") up to " << dStr << " (" << d << "), found (" <<
                         x << " != " << y << ")" << endl;
    }

    void failedAssertDiffers(const char *file, int line,
                             const char *xStr, const char *yStr,
                             const char *value)
    {
        stop(file, line) << "Error: Expected (" <<
                         xStr << " != " << yStr << "), found (" <<
                         value << ")" << endl;
    }

    void failedAssertLessThan(const char *file, int line,
                              const char *xStr, const char *yStr,
                              const char *x, const char *y)
    {
        stop(file, line) << "Error: Expected (" <<
                         xStr << " < " << yStr << "), found (" <<
                         x << " >= " << y << ")" << endl;
    }

    void failedAssertLessThanEquals(const char *file, int line,
                                    const char *xStr, const char *yStr,
                                    const char *x, const char *y)
    {
        stop(file, line) << "Error: Expected (" <<
                         xStr << " <= " << yStr << "), found (" <<
                         x << " > " << y << ")" << endl;
    }

    void failedAssertRelation(const char *file, int line,
                              const char *relation, const char *xStr, const char *yStr,
                              const char *x, const char *y)
    {
        stop(file, line) << "Error: Expected " << relation << "( " <<
                         xStr << ", " << yStr << " ), found !" << relation << "( " << x << ", " << y << " )" << endl;
    }

    void failedAssertPredicate(const char *file, int line,
                               const char *predicate, const char *xStr, const char *x)
    {
        stop(file, line) << "Error: Expected " << predicate << "( " <<
                         xStr << " ), found !" << predicate << "( " << x << " )" << endl;
    }

    void failedAssertThrows(const char *file, int line,
                            const char *expression, const char *type,
                            bool otherThrown)
    {
        stop(file, line) << "Error: Expected (" << expression << ") to throw (" <<
                         type << ") but it " << (otherThrown ? "threw something else" : "didn't throw") <<
                         endl;
    }

    void failedAssertThrowsNot(const char *file, int line, const char *expression)
    {
        stop(file, line) << "Error: Expected (" << expression << ") not to throw, but it did" <<
                         endl;
    }

protected:
    OutputStream *outputStream() const
    {
        return _o;
    }

private:
    UNIXErrorFormatter(const UNIXErrorFormatter &);
    UNIXErrorFormatter &operator=(const UNIXErrorFormatter &);

    OutputStream &stop(const char *file, int line)
    {
        reportTest();
        return (*_o) << file << _preLine << line << _postLine << ": ";
    }

    void reportTest(void)
    {
        if (_reported)
        {
            return;
        }
        (*_o) << tracker().suite().file() << ": In " << tracker().suite().suiteName() << "::" << tracker().test().testName() << ":" << endl;
        _reported = true;
    }

    void dump(const void *buffer, unsigned size)
    {
        if (!buffer)
        {
            dumpNull();
        }
        else
        {
            dumpBuffer(buffer, size);
        }
    }

    void dumpNull()
    {
        (*_o) << "   (null)" << endl;
    }

    void dumpBuffer(const void *buffer, unsigned size)
    {
        unsigned dumpSize = size;
        if (maxDumpSize() && dumpSize > maxDumpSize())
        {
            dumpSize = maxDumpSize();
        }

        const unsigned char *p = (const unsigned char *)buffer;
        (*_o) << "   { ";
        for (unsigned i = 0; i < dumpSize; ++ i)
        {
            (*_o) << byteToHex(*p++) << " ";
        }
        if (dumpSize < size)
        {
            (*_o) << "... ";
        }
        (*_o) << "}" << endl;
    }

    static void endl(OutputStream &o)
    {
        OutputStream::endl(o);
    }

    bool _reported;
    OutputStream *_o;
    const char *_preLine;
    const char *_postLine;
};

// ========================================================================================
// = Actual runner is a subclass only because that is how the original ErrorPrinter works =
// ========================================================================================

class unix : public UNIXErrorFormatter
{
public:
    unix(CXXTEST_STD(ostream) &o = CXXTEST_STD(cerr), const char *preLine = ":", const char *postLine = "") :
        UNIXErrorFormatter(new Adapter(o), preLine, postLine) {}
    virtual ~unix() { delete outputStream(); }

private:
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

#endif /* end of include guard: UNIX_ERROR_PRINTER_H_N4C6JUX4 */

// Copyright 2008 Sandia Corporation. Under the terms of Contract
// DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
// retains certain rights in this software.
