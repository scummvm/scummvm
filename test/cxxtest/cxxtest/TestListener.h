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

#ifndef __cxxtest__TestListener_h__
#define __cxxtest__TestListener_h__

//
// TestListener is the base class for all "listeners",
// i.e. classes that receive notifications of the
// testing process.
//
// The names of the parameters are in comments to avoid
// "unused parameter" warnings.
//

#include <cxxtest/Descriptions.h>

namespace CxxTest
{
class TestListener
{
public:
    TestListener() {}
    virtual ~TestListener() {}
    virtual void process_commandline(int& /*argc*/, char** /*argv*/) {}

    virtual void enterWorld(const WorldDescription & /*desc*/) {}
    virtual void enterSuite(const SuiteDescription & /*desc*/) {}
    virtual void enterTest(const TestDescription & /*desc*/) {}
    virtual void trace(const char * /*file*/, int /*line*/,
                       const char * /*expression*/) {}
    virtual void warning(const char * /*file*/, int /*line*/,
                         const char * /*expression*/) {}
    virtual void skippedTest(const char * /*file*/, int /*line*/,
                             const char * /*expression*/) {}
    virtual void failedTest(const char * /*file*/, int /*line*/,
                            const char * /*expression*/) {}
    virtual void failedAssert(const char * /*file*/, int /*line*/,
                              const char * /*expression*/) {}
    virtual void failedAssertEquals(const char * /*file*/, int /*line*/,
                                    const char * /*xStr*/, const char * /*yStr*/,
                                    const char * /*x*/, const char * /*y*/) {}
    virtual void failedAssertSameData(const char * /*file*/, int /*line*/,
                                      const char * /*xStr*/, const char * /*yStr*/,
                                      const char * /*sizeStr*/, const void * /*x*/,
                                      const void * /*y*/, unsigned /*size*/) {}
    virtual void failedAssertDelta(const char * /*file*/, int /*line*/,
                                   const char * /*xStr*/, const char * /*yStr*/,
                                   const char * /*dStr*/, const char * /*x*/,
                                   const char * /*y*/, const char * /*d*/) {}
    virtual void failedAssertDiffers(const char * /*file*/, int /*line*/,
                                     const char * /*xStr*/, const char * /*yStr*/,
                                     const char * /*value*/) {}
    virtual void failedAssertLessThan(const char * /*file*/, int /*line*/,
                                      const char * /*xStr*/, const char * /*yStr*/,
                                      const char * /*x*/, const char * /*y*/) {}
    virtual void failedAssertLessThanEquals(const char * /*file*/, int /*line*/,
                                            const char * /*xStr*/, const char * /*yStr*/,
                                            const char * /*x*/, const char * /*y*/) {}
    virtual void failedAssertPredicate(const char * /*file*/, int /*line*/,
                                       const char * /*predicate*/, const char * /*xStr*/, const char * /*x*/) {}
    virtual void failedAssertRelation(const char * /*file*/, int /*line*/,
                                      const char * /*relation*/, const char * /*xStr*/, const char * /*yStr*/,
                                      const char * /*x*/, const char * /*y*/) {}
    virtual void failedAssertThrows(const char * /*file*/, int /*line*/,
                                    const char * /*expression*/, const char * /*type*/,
                                    bool /*otherThrown*/) {}
    virtual void failedAssertThrowsNot(const char * /*file*/, int /*line*/,
                                       const char * /*expression*/) {}
    virtual void failedAssertSameFiles(const char* /*file*/, int /*line*/,
                                       const char* , const char*, const char*) {}
    virtual void leaveTest(const TestDescription & /*desc*/) {}
    virtual void leaveSuite(const SuiteDescription & /*desc*/) {}
    virtual void leaveWorld(const WorldDescription & /*desc*/) {}
};
}

#endif // __cxxtest__TestListener_h__
