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
#include <cxxtest/OutputStream.h>

namespace CxxTest
{

inline void print_help(OutputStream &os, const char* name)
{
    os << name << " <suitename>" << endl;
    os << name << " <suitename> <testname>" << endl;
    os << name << " -h" << endl;
    os << name << " --help" << endl;
    os << name << " --help-tests" << endl;
    os << name << " -v             Enable tracing output." << endl;
}


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
    StdOStreamAdapter out(CXXTEST_STD(cout));
    StdOStreamAdapter err(CXXTEST_STD(cerr));
#else
    StdioFileAdapter out(stdout);
    StdioFileAdapter err(stderr);
#endif // _CXXTEST_HAVE_STD
//
// Print command-line syntax
//
    for (int i = 1; i < argc; i++)
    {
        if ((CXXTEST_STD(strcmp)(argv[i], "-h") == 0) || (CXXTEST_STD(strcmp)(argv[i], "--help") == 0))
        {
            print_help(err, argv[0]);
            return 0;
        }
        else if ((CXXTEST_STD(strcmp)(argv[1], "--help-tests") == 0))
        {
            out << "Suite/Test Names" << endl;
            out << "---------------------------------------------------------------------------" << endl;
            for (SuiteDescription *sd = RealWorldDescription().firstSuite(); sd; sd = sd->next())
                for (TestDescription *td = sd->firstTest(); td; td = td->next())
                {
                    out << td->suiteName() << " " << td->testName() << endl;
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
            err << "ERROR: unknown option '" << argv[1] << "'" << endl;
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
            err << "ERROR: unknown suite '" << argv[1] << "'" << endl;
            return -1;
        }
    }
    if ((argc == 3) && (argv[1][0] != '-'))
    {
        status = leaveOnly(argv[1], argv[2]);
        if (!status)
        {
            err << "ERROR: unknown test '" << argv[1] << "::" << argv[2] << "'" << endl;
            return -1;
        }
    }

    tmp.process_commandline(argc, argv);
    return tmp.run();
}

}
#endif
