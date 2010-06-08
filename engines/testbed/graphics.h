#ifndef GRAPHICS_H
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
	~GFXTestSuite(){}
	void execute();
	const char *getName();
	static void setCustomColor(uint r, uint g, uint b);

private:
	/**
	 * A Palette consists of 4 components RGBA.
	 * As of now we only take 3 colors
	 * 0 (R:0, G:0, B:0) Black (kColorBlack)
	 * 1 (R:255, G:255, B:255) White (kColorWhite)
	 * 2 (R:255, G:255, B:255) your customized color (by default white) (kColorCustom)
	 */
	static byte _palette[3 * 4];
};

}	// End of namespace Testbed

#endif
