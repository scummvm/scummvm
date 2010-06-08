#include "testbed/graphics.h"
#include "testbed/gfxtests.h"

namespace Testbed {

byte GFXTestSuite::_palette[3 * 4] = {0, 0, 0, 0, 255, 255, 255, 0, 255, 255, 255, 0};

GFXTestSuite::GFXTestSuite() {
	// Initialize color palettes
	// Te fourth field is for alpha channel which is unused
	// Assuming 8bpp as of now
	g_system->setPalette(_palette, 0, 3);
	g_system->grabPalette(_palette, 0, 3);
	
	// Add tests here
	addTest("FullScreenMode", &testFullScreenMode);
	addTest("AspectRatio", &testAspectRatio);
	addTest("PalettizedCursors", &testPalettizedCursors);
	addTest("BlitBitmaps", &testCopyRectToScreen);
}

const char *GFXTestSuite::getName() {
	return "GFX";
}

void GFXTestSuite::setCustomColor(uint r, uint g, uint b) {
	_palette[8] = r; 
	_palette[9] = g;
	_palette[10] = b;
	g_system->setPalette(_palette, 0, 3);
	g_system->grabPalette(_palette, 0, 3);
}

void GFXTestSuite::execute() {
	for (Common::Array<Test*>::iterator i = _testsToExecute.begin(); i != _testsToExecute.end(); ++i) {
		printf("Executing Test:%s\n", ((*i)->featureName).c_str());
		// Invoke the test
		(*i)->driver();
		_numTestsExecuted++;
		// Verify result by Interacting with the tester.
		Common::String prompt("Was this similar to what you expected?");
		if (handleInteractiveInput(prompt)) {
			_numTestsPassed++;
		}
	}

	// Report Generation
	genReport();
}

}
