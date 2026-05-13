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

#ifndef __CXXTEST__XUNIT_PRINTER_H
#define __CXXTEST__XUNIT_PRINTER_H

//
// XUnitPrinter combines an ErrorPrinter with an XML formatter.
//

#include <cxxtest/TeeListener.h>
#include <cxxtest/ErrorPrinter.h>
#include <cxxtest/XmlPrinter.h>

namespace CxxTest
{
class XUnitPrinter : public TeeListener
{
public:

    XmlPrinter xml_printer;
    ErrorPrinter error_printer;

    XUnitPrinter(CXXTEST_STD(ostream) &o = CXXTEST_STD(cout))
        : xml_printer(o)
    {
        setFirst(error_printer);
        setSecond(xml_printer);
    }

    int run()
    {
        TestRunner::runAllTests(*this);
        return tracker().failedTests();
    }
};
}

#endif //__CXXTEST__XUNIT_PRINTER_H
