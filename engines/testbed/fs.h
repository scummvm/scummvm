#ifndef FS_H
#define FS_H

#include "testbed/testsuite.h"

namespace Testbed {

namespace FStests {

// Helper functions for FS tests

// will contain function declarations for FS tests
bool testOpenFile();
// add more here
}

class FSTestSuite : public Testsuite {
public:
	/**
	 * The constructor for the FSTestSuite
	 * For every test to be executed one must:
	 * 1) Create a function that would invoke the test
	 * 2) Add that test to list by executing addTest()
	 *
	 * @see addTest()
	 */
	FSTestSuite();
	~FSTestSuite(){}
	const char *getName() const;

};

}	// End of namespace Testbed

#endif
