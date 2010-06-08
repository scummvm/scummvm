#include "testbed/gfxtests.h"
#include "testbed/graphics.h"
#include "testbed/testsuite.h"

#include "graphics/fontman.h"
#include "graphics/surface.h"
#include "graphics/cursorman.h"

namespace Testbed {

bool testFullScreenMode() {

	Testsuite::displayMessage("Testing fullscreen mode. \n \
	If the feature is supported by the backend, you should expect to see a toggle between fullscreen and normal modes");

	Common::Point pt(0,100);
	Common::Rect rect = Testsuite::writeOnScreen("Testing fullscreen mode", pt);
	
	bool isFeaturePresent;
	bool isFeatureEnabled;

	isFeaturePresent = g_system->hasFeature(OSystem::kFeatureFullscreenMode);
	isFeatureEnabled = g_system->getFeatureState(OSystem::kFeatureFullscreenMode);
	g_system->delayMillis(1000);

	if (isFeaturePresent) {
		//Toggle

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

bool testAspectRatio() {
	Testsuite::displayMessage("Testing Aspect Ratio Correction. \n \
	With this feature enabled games running at 320x200 should be scaled upto 320x240 pixels");
	
	Common::Point pt(0,100);
	Common::Rect rect = Testsuite::writeOnScreen("Testing Aspect ratio correction", pt);
	
	bool isFeaturePresent;
	bool isFeatureEnabled;

	isFeaturePresent = g_system->hasFeature(OSystem::kFeatureAspectRatioCorrection);
	isFeatureEnabled = g_system->getFeatureState(OSystem::kFeatureAspectRatioCorrection);
	g_system->delayMillis(1000);

	if (isFeaturePresent) {
		//Toggle

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

bool testPalettizedCursors() {
	Testsuite::displayMessage("Testing Cursors. You should expect to see a red colored cursor.\n");
	
	Common::Point pt(0,100);
	Common::Rect rect = Testsuite::writeOnScreen("Testing Palettized Cursors", pt);
	
	bool isFeaturePresent;
	bool isFeatureEnabled;

	isFeaturePresent = g_system->hasFeature(OSystem::kFeatureCursorHasPalette);
	isFeatureEnabled = g_system->getFeatureState(OSystem::kFeatureCursorHasPalette);
	g_system->delayMillis(1000);

	if (isFeaturePresent) {
		byte palette[3 * 4]; // Black, white and yellow
		palette[0] = palette[1] = palette[2] = 0;
		palette[4] = palette[5] = palette[6] = 255;
		palette[8] = palette[9] = 255;
		palette[10] = 0;
		
		byte buffer[10 * 10];
		memset(buffer, 2, 10 * 10);
		
		CursorMan.pushCursorPalette(palette, 0, 3);
		CursorMan.pushCursor(buffer, 10, 10, 40, 40, 2, 1);
		CursorMan.showMouse(true);
		g_system->updateScreen();
	}
	else {
		Testsuite::displayMessage("feature not supported");
	}
	Testsuite::clearScreen(rect);
	return true;
}

bool testCopyRectToScreen() {
	Testsuite::displayMessage("Testing Blitting a Bitmap to screen. \n\
	You should expect to see a 20x40 yellow horizontal rectangle centred at the screen.");

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
