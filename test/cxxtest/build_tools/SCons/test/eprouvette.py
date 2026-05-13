#!/usr/bin/env python
# vim: fileencoding=utf-8
#-------------------------------------------------------------------------
# CxxTest: A lightweight C++ unit testing library.
# Copyright (c) 2008 Sandia Corporation.
# This software is distributed under the LGPL License v3
# For more information, see the COPYING file in the top CxxTest directory.
# Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
# the U.S. Government retains certain rights in this software.
#-------------------------------------------------------------------------

from __future__ import print_function
import os, sys
from os.path import isdir, isfile, islink, join
from optparse import OptionParser
from subprocess import check_call, CalledProcessError, PIPE

options = None
args    = []
available_types = set(['scons'])
tool_stdout = PIPE

def main():
    global options
    global args
    global tool_stdout
    """Parse the options and execute the program."""
    usage = \
    """Usage: %prog [options] [test1 [test2 [...]]]

    If you provide one or more tests, this will run the provided tests.
    Otherwise, it will look for tests in the current directory and run them all.
    """
    # option parsing
    parser = OptionParser(usage)

    parser.set_defaults(
            action='run',
            verbose=True)

    parser.add_option("-c", "--clean",
            action='store_const', const='clean', dest='action',
            help="deletes any generated files in the tests")
    parser.add_option("--run",
            action='store_const', const='run', dest='action',
            help="sets up the environment, compiles and runs the tests")
    parser.add_option("-v", "--verbose",
            action='store_true', dest='verbose',
            help="spew out more details")
    parser.add_option("-q", "--quiet",
            action='store_false', dest='verbose',
            help="spew out only success/failure of tests")
    parser.add_option("--target-dir",
            dest='target_dir', action='store', default='./',
            help='target directory to look for tests in. default: %default')
    parser.add_option("--debug",
            dest='debug', action='store_true', default=False,
            help='turn on debug output.')

    (options, args) = parser.parse_args()

    if options.debug or options.verbose:
        tool_stdout = None
    # gather the tests
    tests = []
    if len(args) == 0:
        tests = crawl_tests(options.target_dir)
    else:
        tests = args
    tests = purge_tests(tests)

    # run the tests
    if options.action == 'run':
        for t in tests:
            run_test(t)
    elif options.action == 'clean':
        for t in tests:
            clean_test(t)

def crawl_tests(target):
    """Gather the directories in the test directory."""
    files = os.listdir(target)
    return [f for f in files if isdir(f) and f[0] != '.']

def purge_tests(dirs):
    """Look at the test candidates and purge those that aren't from the list"""
    tests = []
    for t in dirs:
        if isfile(join(t, 'TestDef.py')):
            tests.append(t)
        else:
            warn("{0} is not a test (missing TestDef.py file).".format(t))
    return tests

def warn(msg):
    """A general warning function."""
    if options.verbose:
        print('[Warn]: ' + msg, file=sys.stderr)

def notice(msg):
    """A general print function."""
    if options.verbose:
        print(msg)

def debug(msg):
    """A debugging function"""
    if options.debug:
        print(msg)

def run_test(t):
    """Runs the test in directory t."""
    opts = read_opts(t)
    notice("-----------------------------------------------------")
    notice("running test '{0}':\n".format(t))
    readme = join(t, 'README')
    if isfile(readme):
        notice(open(readme).read())
        notice("")
    if opts['type'] not in available_types:
        warn('{0} is not a recognised test type in {1}'.format(opts['type'], t))
        return
    if not opts['expect_success']:
        warn("tests that fail intentionally are not yet supported.")
        return

    # set up the environment
    setup_env(t, opts)
    # run the test
    try:
        if opts['type'] == 'scons':
            run_scons(t, opts)
    except RuntimeError as e:
        print("Test {0} failed.".format(t))
        return

    if not options.verbose:
        print('.', end='')
        sys.stdout.flush()
    else:
        print("test '{0}' successful.".format(t))

def read_opts(t):
    """Read the test options and return them."""
    opts = {
            'expect_success' : True,
            'type'           : 'scons',
            'links'          : {}
            }
    f = open(join(t, "TestDef.py"))
    exec(f.read(), opts)
    return opts

def setup_env(t, opts):
    """Set up the environment for the test."""
    # symlinks
    links = opts['links']
    for link in links:
        frm = links[link]
        to  = join(t, link)
        debug("Symlinking {0} to {1}".format(frm, to))
        if islink(to):
            os.unlink(to)
        os.symlink(frm, to)

def teardown_env(t, opts):
    """Remove all files generated for the test."""
    links = opts['links']
    for link in links:
        to  = join(t, link)
        debug('removing link {0}'.format(to))
        os.unlink(to)

def clean_test(t):
    """Remove all generated files."""
    opts = read_opts(t)
    notice("cleaning test {0}".format(t))
    if opts['type'] == 'scons':
        setup_env(t, opts) # scons needs the environment links to work
        clean_scons(t, opts)
    teardown_env(t, opts)

def clean_scons(t, opts):
    """Make scons clean after itself."""
    cwd = os.getcwd()
    os.chdir(t)
    try:
        check_call(['scons', '--clean'], stdout=tool_stdout, stderr=None)
    except CalledProcessError as e:
        warn("SCons failed with error {0}".format(e.returncode))
    os.chdir(cwd)
    sconsign = join(t, '.sconsign.dblite')
    if isfile(sconsign):
        os.unlink(sconsign)

def run_scons(t, opts):
    """Run scons test."""
    cwd = os.getcwd()
    os.chdir(t)
    try:
        check_call(['scons', '--clean'], stdout=tool_stdout)
        check_call(['scons', '.'], stdout=tool_stdout)
        check_call(['scons', 'check'], stdout=tool_stdout)
    except CalledProcessError as e:
        os.chdir(cwd) # clean up
        raise e
    os.chdir(cwd)

if __name__ == "__main__":
    main()

if not options.verbose:
    print() # quiet doesn't output newlines.
