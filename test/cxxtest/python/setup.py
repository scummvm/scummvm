#-------------------------------------------------------------------------
# CxxTest: A lightweight C++ unit testing library.
# Copyright (c) 2008 Sandia Corporation.
# This software is distributed under the LGPL License v3
# For more information, see the COPYING file in the top CxxTest directory.
# Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
# the U.S. Government retains certain rights in this software.
#-------------------------------------------------------------------------

"""
Script to generate the installer for cxxtest.
"""

classifiers = """\
Development Status :: 4 - Beta
Intended Audience :: End Users/Desktop
License :: OSI Approved :: LGPL License
Natural Language :: English
Operating System :: Microsoft :: Windows
Operating System :: Unix
Programming Language :: Python
Topic :: Software Development :: Libraries :: Python Modules
"""

import os
import sys
from os.path import realpath, dirname
if sys.version_info >= (3,0):
    sys.path.insert(0, dirname(realpath(__file__))+os.sep+'python3')
    os.chdir('python3')

import cxxtest

try:
    from setuptools import setup
except ImportError:
    from distutils.core import setup

doclines = cxxtest.__doc__.split("\n")

setup(name="cxxtest",
      version=cxxtest.__version__,
      maintainer=cxxtest.__maintainer__,
      maintainer_email=cxxtest.__maintainer_email__,
      url = cxxtest.__url__,
      license = cxxtest.__license__,
      platforms = ["any"],
      description = doclines[0],
      classifiers = filter(None, classifiers.split("\n")),
      long_description = "\n".join(doclines[2:]),
      packages=['cxxtest'],
      keywords=['utility'],
      scripts=['scripts/cxxtestgen']
      #
      # The entry_points option is not supported by distutils.core
      #
      #entry_points="""
        #[console_scripts]
        #cxxtestgen = cxxtest.cxxtestgen:main
      #"""
      )
