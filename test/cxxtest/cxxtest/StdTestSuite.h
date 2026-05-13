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

#ifndef __cxxtest__StdTestSuite_h__
#define __cxxtest__StdTestSuite_h__

//
// This provides explicit partial specializations for STL-based
// TestSuite comparison functions
//

namespace CxxTest
{

#ifdef _CXXTEST_PARTIAL_TEMPLATE_SPECIALIZATION

template<class X, class Y, class D>
struct delta<std::vector<X>, std::vector<Y>, D>
{
    static bool test(std::vector<X> x, std::vector<Y> y, D d)
    {
        if (x.size() != y.size())
        {
            return false;
        }
        for (size_t i = 0; i < x.size(); ++i)
            if (! delta<X, Y, D>::test(x[i], y[i], d))
            {
                return false;
            }
        return true;
    }
};

template<class X, class Y, class D>
struct delta<std::list<X>, std::list<Y>, D>
{
    static bool test(std::list<X> x, std::list<Y> y, D d)
    {
        typename std::list<X>::const_iterator x_it = x.begin();
        typename std::list<Y>::const_iterator y_it = y.begin();
        for (; x_it != x.end(); ++x_it, ++y_it)
        {
            if (y_it == y.end())
            {
                return false;
            }
            if (! delta<X, Y, D>::test(*x_it, *y_it, d))
            {
                return false;
            }
        }
        return y_it == y.end();
    }
};

#endif

} // namespace CxxTest

#endif // __cxxtest__StdTestSuite_h__
