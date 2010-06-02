#ifdef GRAPHICS_H
#define GRAPHICS_H

#include "testbed/testsuite.h"

namespace Testbed {

class GFXTestSuite : public Testsuite {
public:
	/**
	 * The constructor for the GFXTestSuite
	 * For every test to be executed one must:
	 * 1) Create a function that would invoke the test
	 * 2) Add that test to list by executing addTest()
	 *
	 * @see addTest()
	 */
	GFXTestSuite();
	~GFXTestSuite() {};
}

}	// End of namespace Testbed

#endif
