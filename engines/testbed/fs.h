#ifndef FS_H
#define FS_H

#include "testbed/testsuite.h"

namespace Testbed {

namespace FStests {

// Note: These tests require a game-data directory
// So would work if game-path is set in the launcher or invoked as ./scummvm --path="path-to-testbed-data" testbed 
// from commandline

// Helper functions for FS tests

// will contain function declarations for FS tests
bool testReadFile();
bool testWriteFile();
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
