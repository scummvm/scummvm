#-------------------------------------------------------------------------
# CxxTest: A lightweight C++ unit testing library.
# Copyright (c) 2008 Sandia Corporation.
# This software is distributed under the LGPL License v3
# For more information, see the COPYING file in the top CxxTest directory.
# Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
# the U.S. Government retains certain rights in this software.
#-------------------------------------------------------------------------

"""cxxtest: A Python package that supports the CxxTest test framework for C/C++.

.. _CxxTest: http://cxxtest.com/

CxxTest is a unit testing framework for C++ that is similar in
spirit to JUnit, CppUnit, and xUnit. CxxTest is easy to use because
it does not require precompiling a CxxTest testing library, it
employs no advanced features of C++ (e.g. RTTI) and it supports a
very flexible form of test discovery.

The cxxtest Python package includes capabilities for parsing C/C++ source files and generating
CxxTest drivers.
"""

from cxxtest.__release__ import __version__, __date__
__date__
__version__

__maintainer__ = "William E. Hart"
__maintainer_email__ = "whart222@gmail.com"
__license__ = "LGPL"
__url__ = "http://cxxtest.com"

from cxxtest.cxxtestgen import *
