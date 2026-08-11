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

#ifndef __cxxtest_StdHeaders_h__
#define __cxxtest_StdHeaders_h__

//
// This file basically #includes the STL headers.
// It exists to support warning level 4 in Visual C++
//

#ifdef _MSC_VER
#   pragma warning( push, 1 )
#endif // _MSC_VER

#include <complex>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#ifdef _MSC_VER
#   pragma warning( pop )
#endif // _MSC_VER

#endif // __cxxtest_StdHeaders_h__
