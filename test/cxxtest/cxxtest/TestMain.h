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

#ifndef __CxxTestMain_h
#define __CxxTestMain_h

#include <cxxtest/TestTracker.h>
#include <cxxtest/Flags.h>
#include <cxxtest/StdValueTraits.h>

#if defined(_CXXTEST_HAVE_STD)
#ifdef _CXXTEST_OLD_STD
#   include <iostream.h>
#   include <string.h>
#else // !_CXXTEST_OLD_STD
#   include <iostream>
#   include <cstring>
#endif // _CXXTEST_OLD_STD
#endif

namespace CxxTest
{

#if defined(_CXXTEST_HAVE_STD)
inline void print_help(const char* name)
{
    CXXTEST_STD(cerr) << name << " <suitename>" << CXXTEST_STD(endl);
    CXXTEST_STD(cerr) << name << " <suitename> <testname>" << CXXTEST_STD(endl);
    CXXTEST_STD(cerr) << name << " -h" << CXXTEST_STD(endl);
    CXXTEST_STD(cerr) << name << " --help" << CXXTEST_STD(endl);
    CXXTEST_STD(cerr) << name << " --help-tests" << CXXTEST_STD(endl);
    CXXTEST_STD(cerr) << name << " -v             Enable tracing output." << CXXTEST_STD(endl);
}
#endif


template <class TesterT>
int Main(TesterT& tmp, int argc, char* argv[])
{
//
// Parse the command-line arguments. The default behavior is to run all tests
//
// This is a primitive parser, but I'm not sure what sort of portable
// parser should be used in cxxtest.
//

#if defined(_CXXTEST_HAVE_STD)
//
// Print command-line syntax
//
    for (int i = 1; i < argc; i++)
    {
        if ((CXXTEST_STD(strcmp)(argv[i], "-h") == 0) || (CXXTEST_STD(strcmp)(argv[i], "--help") == 0))
        {
            print_help(argv[0]);
            return 0;
        }
        else if ((CXXTEST_STD(strcmp)(argv[1], "--help-tests") == 0))
        {
            CXXTEST_STD(cout) << "Suite/Test Names" << CXXTEST_STD(endl);
            CXXTEST_STD(cout) << "---------------------------------------------------------------------------" << CXXTEST_STD(endl);
            for (SuiteDescription *sd = RealWorldDescription().firstSuite(); sd; sd = sd->next())
                for (TestDescription *td = sd->firstTest(); td; td = td->next())
                {
                    CXXTEST_STD(cout) << td->suiteName() << " " << td->testName() << CXXTEST_STD(endl);
                }
            return 0;
        }
    }

//
// Process command-line options here.
//
    while ((argc > 1) && (argv[1][0] == '-'))
    {
        if (CXXTEST_STD(strcmp)(argv[1], "-v") == 0)
        {
            tracker().print_tracing = true;
        }
        else
        {
            CXXTEST_STD(cerr) << "ERROR: unknown option '" << argv[1] << "'" << CXXTEST_STD(endl);
            return -1;
        }
        for (int i = 1; i < (argc - 1); i++)
        {
            argv[i] = argv[i + 1];
        }
        argc--;
    }

//
// Run experiments
//
    bool status = false;
    if ((argc == 2) && (argv[1][0] != '-'))
    {
        status = leaveOnly(argv[1]);
        if (!status)
        {
            CXXTEST_STD(cerr) << "ERROR: unknown suite '" << argv[1] << "'" << CXXTEST_STD(endl);
            return -1;
        }
    }
    if ((argc == 3) && (argv[1][0] != '-'))
    {
        status = leaveOnly(argv[1], argv[2]);
        if (!status)
        {
            CXXTEST_STD(cerr) << "ERROR: unknown test '" << argv[1] << "::" << argv[2] << "'" << CXXTEST_STD(endl);
            return -1;
        }
    }
#endif

    tmp.process_commandline(argc, argv);
    return tmp.run();
}

}
#endif
