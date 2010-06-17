#ifndef TEMPLATE_H
#define TEMPLATE_H

#include "testbed/testsuite.h"

// This file can be used as template for header files of other newer testsuites.

namespace Testbed {

namespace XXXtests {

// Helper functions for XXX tests

// will contain function declarations for XXX tests
// add more here
}

class XXXTestSuite : public Testsuite {
public:
	/**
	 * The constructor for the XXXTestSuite
	 * For every test to be executed one must:
	 * 1) Create a function that would invoke the test
	 * 2) Add that test to list by executing addTest()
	 *
	 * @see addTest()
	 */
	XXXTestSuite();
	~XXXTestSuite(){}
	const char *getName() const;

};

}	// End of namespace Testbed

#endif
