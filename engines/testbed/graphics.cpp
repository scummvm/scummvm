#include "common/events.h"

#include "testbed/graphics.h"
#include "testbed/testsuite.h"

#include "graphics/cursorman.h"
#include "graphics/fontman.h"
#include "graphics/surface.h"

namespace Testbed {

byte GFXTestSuite::_palette[3 * 4] = {0, 0, 0, 0, 255, 255, 255, 0, 255, 255, 255, 0};

GFXTestSuite::GFXTestSuite() {
	// Initialize color palettes
	// The fourth field is for alpha channel which is unused
	// Assuming 8bpp as of now
	g_system->setPalette(_palette, 0, 3);
	g_system->grabPalette(_palette, 0, 3);
	
	// Add tests here
	// addTest("FullScreenMode", &GFXtests::fullScreenMode);
	addTest("AspectRatio", &GFXtests::aspectRatio);
	// addTest("PalettizedCursors", &GFXtests::palettizedCursors);
	// addTest("BlitBitmaps", &GFXtests::copyRectToScreen);
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

// GFXtests go here

void drawEllipse(int cx, int cy, int a, int b) {
	byte buffer[100][100] = {0};
	int shift = 25;
	// Assume a rectangle of 50x50
	// horizontal axis = y-axis
	// vertical axis = x-axis
	// The centre is (shift, shift). As of now assume it to be (0, 0)
	// and when done shift it to (shift, shift)
	float theta;
	int x, y, x1, y1;

	for (theta = 0; theta <= PI / 2; theta += PI / 90  ) {
		x = (int)(b * sin(theta) + 0.5);
		y = (int)(a * cos(theta) + 0.5);
		
		// This gives us four points
		
		x1 = x + shift;
		y1 = y + shift;
		
		buffer[x1][y1] = 1;

		x1 = (-1) * x + shift;
		y1 = y + shift;
		
		buffer[x1][y1] = 1;
		
		x1 = x + shift;
		y1 = (-1) * y + shift;
		
		buffer[x1][y1] = 1;

		x1 = (-1) * x + shift;
		y1 = (-1) * y + shift;
		
		buffer[x1][y1] = 1;
	}

	g_system->copyRectToScreen(&buffer[0][0], 100, cx, cy, 100, 100);
	g_system->updateScreen();
}

bool GFXtests::fullScreenMode() {

	Testsuite::displayMessage("Testing fullscreen mode.\n"
	"If the feature is supported by the backend, you should expect to see a toggle between fullscreen and normal modes");

	Common::Point pt(0, 100);
	Common::Rect rect = Testsuite::writeOnScreen("Testing fullscreen mode", pt);
	
	bool isFeaturePresent;
	bool isFeatureEnabled;

	isFeaturePresent = g_system->hasFeature(OSystem::kFeatureFullscreenMode);
	isFeatureEnabled = g_system->getFeatureState(OSystem::kFeatureFullscreenMode);
	g_system->delayMillis(1000);

	if (isFeaturePresent) {
		// Toggle

		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureFullscreenMode, !isFeatureEnabled);
		g_system->endGFXTransaction();

		g_system->delayMillis(1000);
		
		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureFullscreenMode, isFeatureEnabled);
		g_system->endGFXTransaction();
	}
	else {
		Testsuite::displayMessage("feature not supported");
	}

	Testsuite::clearScreen(rect);
	return true;
}

bool GFXtests::aspectRatio() {
	Testsuite::displayMessage("Testing Aspect Ratio Correction.\n"
	"With this feature enabled games running at 320x200 should be scaled upto 320x240 pixels");

	// Draw an ellipse
	drawEllipse(25, 25, 24, 20);
	
	Common::Point pt(0, 100);
	Common::Rect rect = Testsuite::writeOnScreen("Testing Aspect ratio correction", pt);
	
	bool isFeaturePresent;
	bool isFeatureEnabled;

	isFeaturePresent = g_system->hasFeature(OSystem::kFeatureAspectRatioCorrection);
	isFeatureEnabled = g_system->getFeatureState(OSystem::kFeatureAspectRatioCorrection);
	g_system->delayMillis(1000);

	if (isFeaturePresent) {
		// Toggle

		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureAspectRatioCorrection, !isFeatureEnabled);
		g_system->endGFXTransaction();

		g_system->delayMillis(1000);
		
		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureAspectRatioCorrection, isFeatureEnabled);
		g_system->endGFXTransaction();
	}
	else {
		Testsuite::displayMessage("feature not supported");
	}

	Testsuite::clearScreen(rect);
	return true;
}

bool GFXtests::palettizedCursors() {
	Testsuite::displayMessage("Testing Cursors. You should expect to see a yellow colored square cursor.\n"
	"You should be able to move it. The test finishes when the mouse(L/R) is clicked");
	
	Common::Point pt(0, 100);
	Common::Rect rect = Testsuite::writeOnScreen("Testing Palettized Cursors", pt);
	
	bool isFeaturePresent;
	bool isFeatureEnabled;

	isFeaturePresent = g_system->hasFeature(OSystem::kFeatureCursorHasPalette);
	isFeatureEnabled = g_system->getFeatureState(OSystem::kFeatureCursorHasPalette);

	if (isFeaturePresent) {
		byte palette[3 * 4]; // Black, white and yellow
		palette[0] = palette[1] = palette[2] = 0;
		palette[4] = palette[5] = palette[6] = 255;
		palette[8] = palette[9] = 255;
		palette[10] = 0;
		
		byte buffer[10 * 10];
		memset(buffer, 2, 10 * 10);
		
		CursorMan.pushCursorPalette(palette, 0, 3);
		CursorMan.pushCursor(buffer, 10, 10, 45, 45, 1);
		CursorMan.showMouse(true);

		Common::EventManager *eventMan = g_system->getEventManager();
		Common::Event event;

		bool quitLoop = false;
		uint32 lastRedraw = 0;
		const uint32 waitTime = 1000 / 45;

		while (!quitLoop) {
			while (eventMan->pollEvent(event)) {
	
				 if (lastRedraw + waitTime < g_system->getMillis()) {
        	        g_system->updateScreen();
            	    lastRedraw = g_system->getMillis();
           		 }

				switch (event.type) {
				case Common::EVENT_MOUSEMOVE:
					printf("Mouse Move\n");
					break;
				case Common::EVENT_LBUTTONDOWN:
				case Common::EVENT_RBUTTONDOWN:
					Testsuite::clearScreen(rect);
					Testsuite::writeOnScreen("Mouse Clicked", pt);
					printf("Mouse Clicked\n");
					g_system->delayMillis(1000);
					quitLoop = true;
					CursorMan.popCursorPalette();
					CursorMan.popCursor();
					Testsuite::clearScreen(rect);
					Testsuite::writeOnScreen("TestFinished", pt);
					g_system->delayMillis(1000);
					break;
				default:	
					;// Ignore any other event

				}
			}
		}
	} else {
		Testsuite::displayMessage("feature not supported");
	}
	Testsuite::clearScreen(rect);
	return true;
}

bool GFXtests::mouseMovements() {
	return true;
}

bool GFXtests::copyRectToScreen() {
	Testsuite::displayMessage("Testing Blitting a Bitmap to screen.\n"
	"You should expect to see a 20x40 yellow horizontal rectangle centred at the screen.");

	GFXTestSuite::setCustomColor(255, 255, 0);
	byte buffer[20 * 40];
	memset(buffer, 2, 20 * 40);

	uint x = g_system->getWidth() / 2 - 20;
	uint y = g_system->getHeight() / 2 - 10;

	g_system->copyRectToScreen(buffer, 40, x, y, 40, 20);
	g_system->updateScreen();
	g_system->delayMillis(1000);

	Common::Rect rect(x, y, x+40, y+20);
	Testsuite::clearScreen(rect);

	return true;

}

}
