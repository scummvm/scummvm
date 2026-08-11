#-------------------------------------------------------------------------
# CxxTest: A lightweight C++ unit testing library.
# Copyright (c) 2008 Sandia Corporation.
# This software is distributed under the LGPL License v3
# For more information, see the COPYING file in the top CxxTest directory.
# Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
# the U.S. Government retains certain rights in this software.
#-------------------------------------------------------------------------

#
# TODO: add line number info
# TODO: add test function names
#



import sys
import re
from .cxxtest_misc import abort
from . import cxx_parser
import re

def cstr( str ):
    '''Convert a string to its C representation'''
    return '"' + re.sub('\\\\', '\\\\\\\\', str ) + '"'

def scanInputFiles(files, _options):
    '''Scan all input files for test suites'''
    suites=[]
    for file in files:
        try:
            print("Parsing file "+file, end=' ')
            sys.stdout.flush()
            parse_info = cxx_parser.parse_cpp(filename=file,optimize=1)
        except IOError as err:
            print(" error.")
            print(str(err))
            continue
        print("done.")
        sys.stdout.flush()
        #
        # WEH: see if it really makes sense to use parse information to
        # initialize this data.  I don't think so...
        #
        _options.haveStandardLibrary=1
        if not parse_info.noExceptionLogic:
            _options.haveExceptionHandling=1
        #
        keys = list(parse_info.index.keys())
        tpat = re.compile("[Tt][Ee][Ss][Tt]")
        for key in keys:
            if parse_info.index[key].scope_t == "class" and parse_info.is_baseclass(key,"CxxTest::TestSuite"):
                name=parse_info.index[key].name
                if key.startswith('::'):
                    fullname = key[2:]
                else:
                    fullname = key
                suite = {
                        'fullname'     : fullname,
                        'name'         : name,
                        'file'         : file,
                        'cfile'        : cstr(file),
                        'line'         : str(parse_info.index[key].lineno),
                        'generated'    : 0,
                        'object'       : 'suite_%s' % fullname.replace('::','_'),
                        'dobject'      : 'suiteDescription_%s' % fullname.replace('::','_'),
                        'tlist'        : 'Tests_%s' % fullname.replace('::','_'),
                        'tests'        : [],
                        'lines'        : [] }
                for fn in parse_info.get_functions(key,quiet=True):
                    tname = fn[0]
                    lineno = str(fn[1])
                    if tname.startswith('createSuite'):
                        # Indicate that we're using a dynamically generated test suite
                        suite['create'] = str(lineno) # (unknown line)
                    if tname.startswith('destroySuite'):
                        # Indicate that we're using a dynamically generated test suite
                        suite['destroy'] = str(lineno) # (unknown line)
                    if not tpat.match(tname):
                        # Skip non-test methods
                        continue
                    test = { 'name'   : tname,
                        'suite'  : suite,
                        'class'  : 'TestDescription_suite_%s_%s' % (suite['fullname'].replace('::','_'), tname),
                        'object' : 'testDescription_suite_%s_%s' % (suite['fullname'].replace('::','_'), tname),
                        'line'   : lineno,
                        }
                    suite['tests'].append(test)
                suites.append(suite)

    if not _options.root:
        ntests = 0
        for suite in suites:
            ntests += len(suite['tests'])
        if ntests == 0:
            abort( 'No tests defined' )
    #
    return [_options, suites]
