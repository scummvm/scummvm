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

#ifndef __cxxtest__TestSuite_cpp__
#define __cxxtest__TestSuite_cpp__

#include <cxxtest/TestSuite.h>
#if defined(_CXXTEST_HAVE_STD)
#include <fstream>
#endif

namespace CxxTest
{
//
// TestSuite members
//
TestSuite::~TestSuite() {}
void TestSuite::setUp() {}
void TestSuite::tearDown() {}

//
// Test-aborting stuff
//
static bool currentAbortTestOnFail = false;

bool abortTestOnFail()
{
    return currentAbortTestOnFail;
}

void setAbortTestOnFail(bool value)
{
    currentAbortTestOnFail = value;
}

void doAbortTest()
{
#   if defined(_CXXTEST_HAVE_EH)
    if (currentAbortTestOnFail)
    {
        throw AbortTest();
    }
#   endif // _CXXTEST_HAVE_EH
}

//
// Max dump size
//
static unsigned currentMaxDumpSize = CXXTEST_MAX_DUMP_SIZE;

unsigned maxDumpSize()
{
    return currentMaxDumpSize;
}

void setMaxDumpSize(unsigned value)
{
    currentMaxDumpSize = value;
}

//
// Some non-template functions
//
void doTrace(const char *file, int line, const char *message)
{
    if (tracker().print_tracing)
    {
        tracker().trace(file, line, message);
    }
}

void doWarn(const char *file, int line, const char *message)
{
    tracker().warning(file, line, message);
}

#   if defined(_CXXTEST_HAVE_EH)
void doSkipTest(const char* file, int line, const char* message)
{
    tracker().skippedTest(file, line, message);
    throw SkipTest();
#   else
void doSkipTest(const char * file, int line, const char*)
{
    doWarn(file, line, "Test skipping is not supported without exception handling.");
#   endif
}

void doFailTest(const char * file, int line, const char * message)
{
    tracker().failedTest(file, line, message);
    TS_ABORT();
}

void doFailAssert(const char * file, int line,
                  const char * expression, const char * message)
{
    if (message)
    {
        tracker().failedTest(file, line, message);
    }
    tracker().failedAssert(file, line, expression);
    TS_ABORT();
}

bool sameData(const void * x, const void * y, unsigned size)
{
    if (size == 0)
    {
        return true;
    }

    if (x == y)
    {
        return true;
    }

    if (!x || !y)
    {
        return false;
    }

    const char *cx = (const char *)x;
    const char *cy = (const char *)y;
    while (size --)
    {
        if (*cx++ != *cy++)
        {
            return false;
        }
    }

    return true;
}

void doAssertSameData(const char * file, int line,
                      const char * xExpr, const void * x,
                      const char * yExpr, const void * y,
                      const char * sizeExpr, unsigned size,
                      const char * message)
{
    if (!sameData(x, y, size))
    {
        if (message)
        {
            tracker().failedTest(file, line, message);
        }
        tracker().failedAssertSameData(file, line, xExpr, yExpr, sizeExpr, x, y, size);
        TS_ABORT();
    }
}

#if defined(_CXXTEST_HAVE_STD)
bool sameFiles(const char * file1, const char * file2, std::ostringstream & explanation)
{
    std::string ppprev_line;
    std::string pprev_line;
    std::string prev_line;
    std::string curr_line;

    std::ifstream is1;
    is1.open(file1);
    std::ifstream is2;
    is2.open(file2);
    if (!is1)
    {
        explanation << "File '" << file1 << "' does not exist!";
        return false;
    }
    if (!is2)
    {
        explanation << "File '" << file2 << "' does not exist!";
        return false;
    }

    int nline = 1;
    char c1, c2;
    while (1)
    {
        is1.get(c1);
        is2.get(c2);
        if (!is1 && !is2) { return true; }
        if (!is1)
        {
            explanation << "File '" << file1 << "' ended before file '" << file2 << "' (line " << nline << ")";
            explanation << std::endl << "= " << ppprev_line << std::endl << "=  " << pprev_line << std::endl << "= " << prev_line << std::endl << "< " << curr_line;
            is1.get(c1);
            while (is1 && (c1 != '\n'))
            {
                explanation << c1;
                is1.get(c1);
            }
            explanation << std::endl;
            return false;
        }
        if (!is2)
        {
            explanation << "File '" << file2 << "' ended before file '" << file1 << "' (line " << nline << ")";
            explanation << std::endl << "= " << ppprev_line << std::endl << "=  " << pprev_line << std::endl << "= " << prev_line << std::endl << "> " << curr_line;
            is2.get(c2);
            while (is2 && (c2 != '\n'))
            {
                explanation << c2;
                is2.get(c2);
            }
            explanation << std::endl;
            return false;
        }
        if (c1 != c2)
        {
            explanation << "Files '" << file1 << "' and '" << file2 << "' differ at line " << nline;
            explanation << std::endl << "= " << ppprev_line << std::endl << "=  " << pprev_line << std::endl << "= " << prev_line;

            explanation << std::endl << "< " << curr_line;
            is2.get(c1);
            while (is1 && (c1 != '\n'))
            {
                explanation << c1;
                is2.get(c1);
            }
            explanation << std::endl;

            explanation << std::endl << "> " << curr_line;
            is2.get(c2);
            while (is2 && (c2 != '\n'))
            {
                explanation << c2;
                is2.get(c2);
            }
            explanation << std::endl;

            return false;
        }
        if (c1 == '\n')
        {
            ppprev_line = pprev_line;
            pprev_line = prev_line;
            prev_line = curr_line;
            curr_line = "";
            nline++;
        }
        else
        {
            curr_line += c1;
        }
    }
}
#endif

void doAssertSameFiles(const char * file, int line,
                       const char * file1, const char * file2,
                       const char * message)
{
#if defined(_CXXTEST_HAVE_STD)
    std::ostringstream explanation;
    if (!sameFiles(file1, file2, explanation))
    {
        if (message)
        {
            tracker().failedTest(file, line, message);
        }
        tracker().failedAssertSameFiles(file, line, file1, file2, explanation.str().c_str());
        TS_ABORT();
    }
#else
    tracker().failedAssertSameFiles(file, line, file1, file2, "This test is only supported when --have-std is enabled");
    TS_ABORT();
#endif
}

void doFailAssertThrows(const char * file, int line,
                        const char * expr, const char * type,
                        bool otherThrown,
                        const char * message,
                        const char * exception)
{
    if (exception)
    {
        tracker().failedTest(file, line, exception);
    }
    if (message)
    {
        tracker().failedTest(file, line, message);
    }

    tracker().failedAssertThrows(file, line, expr, type, otherThrown);
    TS_ABORT();
}

void doFailAssertThrowsNot(const char * file, int line,
                           const char * expression, const char * message,
                           const char * exception)
{
    if (exception)
    {
        tracker().failedTest(file, line, exception);
    }
    if (message)
    {
        tracker().failedTest(file, line, message);
    }

    tracker().failedAssertThrowsNot(file, line, expression);
    TS_ABORT();
}
}

#endif // __cxxtest__TestSuite_cpp__
