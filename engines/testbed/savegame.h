#ifndef SAVEGAME_H
#define SAVEGAME_H

#include "testbed/testsuite.h"

namespace Testbed {

namespace SaveGametests {

// Helper functions for SaveGame tests
bool writeDataToFile(const char *fileName, const char *msg);
bool readAndVerifyData(const char *fileName, const char *expected);
// will contain function declarations for SaveGame tests
bool testSaveLoadState();
bool testRemovingSavefile();
bool testRenamingSavefile();
bool testListingSavefile();
bool testErrorMessages();
// add more here
}

class SaveGameTestSuite : public Testsuite {
public:
	/**
	 * The constructor for the SaveGameTestSuite
	 * For every test to be executed one must:
	 * 1) Create a function that would invoke the test
	 * 2) Add that test to list by executing addTest()
	 *
	 * @see addTest()
	 */
	SaveGameTestSuite();
	~SaveGameTestSuite(){}
	const char *getName() const;

};

}	// End of namespace Testbed

#endif
