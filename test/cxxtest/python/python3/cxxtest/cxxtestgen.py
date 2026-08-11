#-------------------------------------------------------------------------
# CxxTest: A lightweight C++ unit testing library.
# Copyright (c) 2008 Sandia Corporation.
# This software is distributed under the LGPL License v3
# For more information, see the COPYING file in the top CxxTest directory.
# Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
# the U.S. Government retains certain rights in this software.
#-------------------------------------------------------------------------

# vim: fileencoding=utf-8


# the above import important for forward-compatibility with python3,
# which is already the default in archlinux!

__all__ = ['main', 'create_manpage']

from . import __release__
import os
import sys
import re
import glob
from optparse import OptionParser
from . import cxxtest_parser
from string import Template

try:
    from . import cxxtest_fog
    imported_fog=True
except ImportError:
    imported_fog=False

from .cxxtest_misc import abort

try:
    from os.path import relpath
except ImportError:
    from .cxxtest_misc import relpath

# Global data is initialized by main()
options = []
suites = []
wrotePreamble = 0
wroteWorld = 0
lastIncluded = ''


def main(args=sys.argv, catch=False):
    '''The main program'''
    #
    # Reset global state
    #
    global wrotePreamble
    wrotePreamble=0
    global wroteWorld
    wroteWorld=0
    global lastIncluded
    lastIncluded = ''
    global suites
    suites = []
    global options
    options = []
    #
    try:
        files = parseCommandline(args)
        if imported_fog and options.fog:
            [options,suites] = cxxtest_fog.scanInputFiles( files, options )
        else:
            [options,suites] = cxxtest_parser.scanInputFiles( files, options )
        writeOutput()
    except SystemExit:
        if not catch:
            raise

def create_parser(asciidoc=False):
    parser = OptionParser("cxxtestgen [options] [<filename> ...]")
    if asciidoc:
        parser.description="The cxxtestgen command processes C++ header files to perform test discovery, and then it creates files for the CxxTest test runner."
    else:
        parser.description="The 'cxxtestgen' command processes C++ header files to perform test discovery, and then it creates files for the 'CxxTest' test runner."

    parser.add_option("--version",
                      action="store_true", dest="version", default=False,
                      help="Write the CxxTest version.")
    parser.add_option("-o", "--output",
                      dest="outputFileName", default=None, metavar="NAME",
                      help="Write output to file NAME.")
    parser.add_option("-w","--world", dest="world", default="cxxtest",
                      help="The label of the tests, used to name the XML results.")
    parser.add_option("", "--include", action="append",
                      dest="headers", default=[], metavar="HEADER",
                      help="Include file HEADER in the test runner before other headers.")
    parser.add_option("", "--abort-on-fail",
                      action="store_true", dest="abortOnFail", default=False,
                      help="Abort tests on failed asserts (like xUnit).")
    parser.add_option("", "--main",
                      action="store", dest="main", default="main",
                      help="Specify an alternative name for the main() function.")
    parser.add_option("", "--headers",
                      action="store", dest="header_filename", default=None,
                      help="Specify a filename that contains a list of header files that are processed to generate a test runner.")
    parser.add_option("", "--runner",
                      dest="runner", default="", metavar="CLASS",
                      help="Create a test runner that processes test events using the class CxxTest::CLASS.")
    parser.add_option("", "--gui",
                      dest="gui", metavar="CLASS",
                      help="Create a GUI test runner that processes test events using the class CxxTest::CLASS. (deprecated)")
    parser.add_option("", "--error-printer",
                      action="store_true", dest="error_printer", default=False,
                      help="Create a test runner using the ErrorPrinter class, and allow the use of the standard library.")
    parser.add_option("", "--xunit-printer",
                      action="store_true", dest="xunit_printer", default=False,
                      help="Create a test runner using the XUnitPrinter class.")
    parser.add_option("", "--xunit-file",  dest="xunit_file", default="",
                      help="The file to which the XML summary is written for test runners using the XUnitPrinter class.  The default XML filename is TEST-<world>.xml, where <world> is the value of the --world option.  (default: cxxtest)")
    parser.add_option("", "--have-std",
                      action="store_true", dest="haveStandardLibrary", default=False,
                      help="Use the standard library (even if not found in tests).")
    parser.add_option("", "--no-std",
                      action="store_true", dest="noStandardLibrary", default=False,
                      help="Do not use standard library (even if found in tests).")
    parser.add_option("", "--have-eh",
                      action="store_true", dest="haveExceptionHandling", default=False,
                      help="Use exception handling (even if not found in tests).")
    parser.add_option("", "--no-eh",
                      action="store_true", dest="noExceptionHandling", default=False,
                      help="Do not use exception handling (even if found in tests).")
    parser.add_option("", "--longlong",
                      dest="longlong", default=None, metavar="TYPE",
                      help="Use TYPE as for long long integers.  (default: not supported)")
    parser.add_option("", "--no-static-init",
                      action="store_true", dest="noStaticInit", default=False,
                      help="Do not rely on static initialization in the test runner.")
    parser.add_option("", "--template",
                      dest="templateFileName", default=None, metavar="TEMPLATE",
                      help="Generate the test runner using file TEMPLATE to define a template.")
    parser.add_option("", "--root",
                      action="store_true", dest="root", default=False,
                      help="Write the main() function and global data for a test runner.")
    parser.add_option("", "--part",
                      action="store_true", dest="part", default=False,
                      help="Write the tester classes for a test runner.")
    #parser.add_option("", "--factor",
                      #action="store_true", dest="factor", default=False,
                      #help="Declare the _CXXTEST_FACTOR macro.  (deprecated)")
    if imported_fog:
        fog_help = "Use new FOG C++ parser"
    else:
        fog_help = "Use new FOG C++ parser (disabled)"
    parser.add_option("-f", "--fog-parser",
                        action="store_true",
                        dest="fog",
                        default=False,
                        help=fog_help
                        )
    return parser

def parseCommandline(args):
    '''Analyze command line arguments'''
    global imported_fog
    global options

    parser = create_parser()
    (options, args) = parser.parse_args(args=args)
    if not options.header_filename is None:
        if not os.path.exists(options.header_filename):
            abort( "ERROR: the file '%s' does not exist!" % options.header_filename )
        INPUT = open(options.header_filename)
        headers = [line.strip() for line in INPUT]
        args.extend( headers )
        INPUT.close()

    if options.fog and not imported_fog:
        abort( "Cannot use the FOG parser.  Check that the 'ply' package is installed.  The 'ordereddict' package is also required if running Python 2.6")

    if options.version:
      printVersion()

    # the cxxtest builder relies on this behaviour! don't remove
    if options.runner == 'none':
        options.runner = None

    if options.xunit_printer or options.runner == "XUnitPrinter":
        options.xunit_printer=True
        options.runner="XUnitPrinter"
        if len(args) > 1:
            if options.xunit_file == "":
                if options.world == "":
                    options.world = "cxxtest"
                options.xunit_file="TEST-"+options.world+".xml"
        elif options.xunit_file == "":
            if options.world == "":
                options.world = "cxxtest"
            options.xunit_file="TEST-"+options.world+".xml"

    if options.error_printer:
      options.runner= "ErrorPrinter"
      options.haveStandardLibrary = True

    if options.noStaticInit and (options.root or options.part):
        abort( '--no-static-init cannot be used with --root/--part' )

    if options.gui and not options.runner:
        options.runner = 'StdioPrinter'

    files = setFiles(args[1:])
    if len(files) == 0 and not options.root:
        sys.stderr.write(parser.error("No input files found"))

    return files


def printVersion():
    '''Print CxxTest version and exit'''
    sys.stdout.write( "This is CxxTest version %s.\n" % __release__.__version__ )
    sys.exit(0)

def setFiles(patterns ):
    '''Set input files specified on command line'''
    files = expandWildcards( patterns )
    return files

def expandWildcards( patterns ):
    '''Expand all wildcards in an array (glob)'''
    fileNames = []
    for pathName in patterns:
        patternFiles = glob.glob( pathName )
        for fileName in patternFiles:
            fileNames.append( fixBackslashes( fileName ) )
    return fileNames

def fixBackslashes( fileName ):
    '''Convert backslashes to slashes in file name'''
    return re.sub( r'\\', '/', fileName )


def writeOutput():
    '''Create output file'''
    if options.templateFileName:
        writeTemplateOutput()
    else:
        writeSimpleOutput()

def writeSimpleOutput():
    '''Create output not based on template'''
    output = startOutputFile()
    writePreamble( output )
    if options.root or not options.part:
        writeMain( output )

    if len(suites) > 0:
        output.write("bool "+suites[0]['object']+"_init = false;\n")

    writeWorld( output )
    output.close()

include_re = re.compile( r"\s*\#\s*include\s+<cxxtest/" )
preamble_re = re.compile( r"^\s*<CxxTest\s+preamble>\s*$" )
world_re = re.compile( r"^\s*<CxxTest\s+world>\s*$" )
def writeTemplateOutput():
    '''Create output based on template file'''
    template = open(options.templateFileName)
    output = startOutputFile()
    while 1:
        line = template.readline()
        if not line:
            break;
        if include_re.search( line ):
            writePreamble( output )
            output.write( line )
        elif preamble_re.search( line ):
            writePreamble( output )
        elif world_re.search( line ):
            if len(suites) > 0:
                output.write("bool "+suites[0]['object']+"_init = false;\n")
            writeWorld( output )
        else:
            output.write( line )
    template.close()
    output.close()

def startOutputFile():
    '''Create output file and write header'''
    if options.outputFileName is not None:
        output = open( options.outputFileName, 'w' )
    else:
        output = sys.stdout
    output.write( "/* Generated file, do not edit */\n\n" )
    return output

def writePreamble( output ):
    '''Write the CxxTest header (#includes and #defines)'''
    global wrotePreamble
    if wrotePreamble: return
    output.write( "#ifndef CXXTEST_RUNNING\n" )
    output.write( "#define CXXTEST_RUNNING\n" )
    output.write( "#endif\n" )
    output.write( "\n" )
    if options.xunit_printer:
        output.write( "#include <fstream>\n" )
    if options.haveStandardLibrary:
        output.write( "#define _CXXTEST_HAVE_STD\n" )
    if options.haveExceptionHandling:
        output.write( "#define _CXXTEST_HAVE_EH\n" )
    if options.abortOnFail:
        output.write( "#define _CXXTEST_ABORT_TEST_ON_FAIL\n" )
    if options.longlong:
        output.write( "#define _CXXTEST_LONGLONG %s\n" % options.longlong )
    #if options.factor:
        #output.write( "#define _CXXTEST_FACTOR\n" )
    for header in options.headers:
        output.write( "#include \"%s\"\n" % header )
    output.write( "#include <cxxtest/TestListener.h>\n" )
    output.write( "#include <cxxtest/TestTracker.h>\n" )
    output.write( "#include <cxxtest/TestRunner.h>\n" )
    output.write( "#include <cxxtest/RealDescriptions.h>\n" )
    output.write( "#include <cxxtest/TestMain.h>\n" )
    if options.runner:
        output.write( "#include <cxxtest/%s.h>\n" % options.runner )
    if options.gui:
        output.write( "#include <cxxtest/%s.h>\n" % options.gui )
    output.write( "\n" )
    wrotePreamble = 1

def writeMain( output ):
    '''Write the main() function for the test runner'''
    if not (options.gui or options.runner):
       return
    output.write( 'int %s( int argc, char *argv[] ) {\n' % options.main )
    output.write( ' int status;\n' )
    if options.noStaticInit:
        output.write( ' CxxTest::initialize();\n' )
    if options.gui:
        tester_t = "CxxTest::GuiTuiRunner<CxxTest::%s, CxxTest::%s> " % (options.gui, options.runner)
    else:
        tester_t = "CxxTest::%s" % (options.runner)
    if options.xunit_printer:
       output.write( '    std::ofstream ofstr("%s");\n' % options.xunit_file )
       output.write( '    %s tmp(ofstr);\n' % tester_t )
    else:
       output.write( '    %s tmp;\n' % tester_t )
    output.write( '    CxxTest::RealWorldDescription::_worldName = "%s";\n' % options.world )
    output.write( '    status = CxxTest::Main< %s >( tmp, argc, argv );\n' % tester_t )
    output.write( '    return status;\n')
    output.write( '}\n' )


def writeWorld( output ):
    '''Write the world definitions'''
    global wroteWorld
    if wroteWorld: return
    writePreamble( output )
    writeSuites( output )
    if options.root or not options.part:
        writeRoot( output )
        writeWorldDescr( output )
    if options.noStaticInit:
        writeInitialize( output )
    wroteWorld = 1

def writeSuites(output):
    '''Write all TestDescriptions and SuiteDescriptions'''
    for suite in suites:
        writeInclude( output, suite['file'] )
        if isGenerated(suite):
            generateSuite( output, suite )
        if not options.noStaticInit:
            if isDynamic(suite):
                writeSuitePointer( output, suite )
            else:
                writeSuiteObject( output, suite )
            writeTestList( output, suite )
            writeSuiteDescription( output, suite )
        writeTestDescriptions( output, suite )

def isGenerated(suite):
    '''Checks whether a suite class should be created'''
    return suite['generated']

def isDynamic(suite):
    '''Checks whether a suite is dynamic'''
    return 'create' in suite

def writeInclude(output, file):
    '''Add #include "file" statement'''
    global lastIncluded
    if options.outputFileName:
        dirname = os.path.split(options.outputFileName)[0]
        tfile = relpath(file, dirname)
        if os.path.exists(tfile):
            if tfile == lastIncluded: return
            output.writelines( [ '#include "', tfile, '"\n\n' ] )
            lastIncluded = tfile
            return
    #
    # Use an absolute path if the relative path failed
    #
    tfile = os.path.abspath(file)
    if os.path.exists(tfile):
        if tfile == lastIncluded: return
        output.writelines( [ '#include "', tfile, '"\n\n' ] )
        lastIncluded = tfile
        return

def generateSuite( output, suite ):
    '''Write a suite declared with CXXTEST_SUITE()'''
    output.write( 'class %s : public CxxTest::TestSuite {\n' % suite['fullname'] )
    output.write( 'public:\n' )
    for line in suite['lines']:
        output.write(line)
    output.write( '};\n\n' )

def writeSuitePointer( output, suite ):
    '''Create static suite pointer object for dynamic suites'''
    if options.noStaticInit:
        output.write( 'static %s* %s;\n\n' % (suite['fullname'], suite['object']) )
    else:
        output.write( 'static %s* %s = 0;\n\n' % (suite['fullname'], suite['object']) )

def writeSuiteObject( output, suite ):
    '''Create static suite object for non-dynamic suites'''
    output.writelines( [ "static ", suite['fullname'], " ", suite['object'], ";\n\n" ] )

def writeTestList( output, suite ):
    '''Write the head of the test linked list for a suite'''
    if options.noStaticInit:
        output.write( 'static CxxTest::List %s;\n' % suite['tlist'] )
    else:
        output.write( 'static CxxTest::List %s = { 0, 0 };\n' % suite['tlist'] )

def writeWorldDescr( output ):
    '''Write the static name of the world name'''
    if options.noStaticInit:
        output.write( 'const char* CxxTest::RealWorldDescription::_worldName;\n' )
    else:
        output.write( 'const char* CxxTest::RealWorldDescription::_worldName = "cxxtest";\n' )

def writeTestDescriptions( output, suite ):
    '''Write all test descriptions for a suite'''
    for test in suite['tests']:
        writeTestDescription( output, suite, test )

def writeTestDescription( output, suite, test ):
    '''Write test description object'''
    if not options.noStaticInit:
        output.write( 'static class %s : public CxxTest::RealTestDescription {\n' % test['class'] )
    else:
        output.write( 'class %s : public CxxTest::RealTestDescription {\n' % test['class'] )
    #
    output.write( 'public:\n' )
    if not options.noStaticInit:
        output.write( ' %s() : CxxTest::RealTestDescription( %s, %s, %s, "%s" ) {}\n' %
                      (test['class'], suite['tlist'], suite['dobject'], test['line'], test['name']) )
    else:
        if isDynamic(suite):
            output.write( ' %s(%s* _%s) : %s(_%s) { }\n' %
                      (test['class'], suite['fullname'], suite['object'], suite['object'], suite['object']) )
            output.write( ' %s* %s;\n' % (suite['fullname'], suite['object']) )
        else:
            output.write( ' %s(%s& _%s) : %s(_%s) { }\n' %
                      (test['class'], suite['fullname'], suite['object'], suite['object'], suite['object']) )
            output.write( ' %s& %s;\n' % (suite['fullname'], suite['object']) )
    output.write( ' void runTest() { %s }\n' % runBody( suite, test ) )
    #
    if not options.noStaticInit:
        output.write( '} %s;\n\n' % test['object'] )
    else:
        output.write( '};\n\n' )

def runBody( suite, test ):
    '''Body of TestDescription::run()'''
    if isDynamic(suite): return dynamicRun( suite, test )
    else: return staticRun( suite, test )

def dynamicRun( suite, test ):
    '''Body of TestDescription::run() for test in a dynamic suite'''
    return 'if ( ' + suite['object'] + ' ) ' + suite['object'] + '->' + test['name'] + '();'

def staticRun( suite, test ):
    '''Body of TestDescription::run() for test in a non-dynamic suite'''
    return suite['object'] + '.' + test['name'] + '();'

def writeSuiteDescription( output, suite ):
    '''Write SuiteDescription object'''
    if isDynamic( suite ):
        writeDynamicDescription( output, suite )
    else:
        writeStaticDescription( output, suite )

def writeDynamicDescription( output, suite ):
    '''Write SuiteDescription for a dynamic suite'''
    output.write( 'CxxTest::DynamicSuiteDescription< %s > %s' % (suite['fullname'], suite['dobject']) )
    if not options.noStaticInit:
        output.write( '( %s, %s, "%s", %s, %s, %s, %s )' %
                      (suite['cfile'], suite['line'], suite['fullname'], suite['tlist'],
                       suite['object'], suite['create'], suite['destroy']) )
    output.write( ';\n\n' )

def writeStaticDescription( output, suite ):
    '''Write SuiteDescription for a static suite'''
    output.write( 'CxxTest::StaticSuiteDescription %s' % suite['dobject'] )
    if not options.noStaticInit:
        output.write( '( %s, %s, "%s", %s, %s )' %
                      (suite['cfile'], suite['line'], suite['fullname'], suite['object'], suite['tlist']) )
    output.write( ';\n\n' )

def writeRoot(output):
    '''Write static members of CxxTest classes'''
    output.write( '#include <cxxtest/Root.cpp>\n' )

def writeInitialize(output):
    '''Write CxxTest::initialize(), which replaces static initialization'''
    output.write( 'namespace CxxTest {\n' )
    output.write( ' void initialize()\n' )
    output.write( ' {\n' )
    for suite in suites:
        #print "HERE", suite
        writeTestList( output, suite )
        output.write( '  %s.initialize();\n' % suite['tlist'] )
        #writeSuiteObject( output, suite )
        if isDynamic(suite):
            writeSuitePointer( output, suite )
            output.write( '  %s = 0;\n' % suite['object'])
        else:
            writeSuiteObject( output, suite )
        output.write( ' static ')
        writeSuiteDescription( output, suite )
        if isDynamic(suite):
            #output.write( '  %s = %s.suite();\n' % (suite['object'],suite['dobject']) )
            output.write( '  %s.initialize( %s, %s, "%s", %s, %s, %s, %s );\n' %
                          (suite['dobject'], suite['cfile'], suite['line'], suite['fullname'],
                           suite['tlist'], suite['object'], suite['create'], suite['destroy']) )
            output.write( '  %s.setUp();\n' % suite['dobject'])
        else:
            output.write( '  %s.initialize( %s, %s, "%s", %s, %s );\n' %
                          (suite['dobject'], suite['cfile'], suite['line'], suite['fullname'],
                           suite['object'], suite['tlist']) )

        for test in suite['tests']:
            output.write( '  static %s %s(%s);\n' %
                          (test['class'], test['object'], suite['object']) )
            output.write( '  %s.initialize( %s, %s, %s, "%s" );\n' %
                          (test['object'], suite['tlist'], suite['dobject'], test['line'], test['name']) )

    output.write( ' }\n' )
    output.write( '}\n' )

man_template=Template("""CXXTESTGEN(1)
=============
:doctype: manpage


NAME
----
cxxtestgen - performs test discovery to create a CxxTest test runner


SYNOPSIS
--------
${usage}


DESCRIPTION
-----------
${description}


OPTIONS
-------
${options}


EXIT STATUS
-----------
*0*::
   Success

*1*::
   Failure (syntax or usage error; configuration error; document
   processing failure; unexpected error).


BUGS
----
See the CxxTest Home Page for the link to the CxxTest ticket repository.


AUTHOR
------
CxxTest was originally written by Erez Volk. Many people have
contributed to it.


RESOURCES
---------
Home page: <http://cxxtest.com/>

CxxTest User Guide: <http://cxxtest.com/cxxtest/doc/guide.html>



COPYING
-------
Copyright (c) 2008 Sandia Corporation.  This software is distributed
under the Lesser GNU General Public License (LGPL) v3
""")

def create_manpage():
    """Write ASCIIDOC manpage file"""
    parser = create_parser(asciidoc=True)
    #
    usage = parser.usage
    description = parser.description
    options=""
    for opt in parser.option_list:
        opts = opt._short_opts + opt._long_opts
        optstr = '*' + ', '.join(opts) + '*'
        if not opt.metavar is None:
            optstr += "='%s'" % opt.metavar
        optstr += '::\n'
        options += optstr
        #
        options += opt.help
        options += '\n\n'
    #
    OUTPUT = open('cxxtestgen.1.txt','w')
    OUTPUT.write( man_template.substitute(usage=usage, description=description, options=options) )
    OUTPUT.close()
