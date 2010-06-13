#include "common/events.h"

#include "testbed/graphics.h"
#include "testbed/testsuite.h"

#include "graphics/cursorman.h"
#include "graphics/fontman.h"
#include "graphics/surface.h"
#include "graphics/VectorRendererSpec.h"

namespace Testbed {

byte GFXTestSuite::_palette[3 * 4] = {0, 0, 0, 0, 255, 255, 255, 0, 255, 255, 255, 0};

GFXTestSuite::GFXTestSuite() {
	// Initialize color palettes
	// The fourth field is for alpha channel which is unused
	// Assuming 8bpp as of now
	g_system->setPalette(_palette, 0, 3);
	g_system->grabPalette(_palette, 0, 3);
	
	// Add tests here
	// TODO: Can do it without transactions?
	addTest("FullScreenMode", &GFXtests::fullScreenMode);
	addTest("AspectRatio", &GFXtests::aspectRatio);
	addTest("PalettizedCursors", &GFXtests::palettizedCursors);
	addTest("BlitBitmaps", &GFXtests::copyRectToScreen);
	// TODO: doesn't returns back to normal states
	addTest("IconifyingWindow", &GFXtests::iconifyWindow);
	// TODO: need to fix it
	// addTest("ScaledCursors", &GFXtests::scaledCursors);
	addTest("shakingEffect", &GFXtests::shakingEffect);
	// TODO: unable to notice any change, make it noticable
	addTest("focusRectangle", &GFXtests::focusRectangle);
	// TODO: unable to notice any change, make it noticable
	addTest("Overlays", &GFXtests::overlayGraphics);
}

const char *GFXTestSuite::getName() const {
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

// Helper functions used by GFX tests

void GFXtests::drawCursor(const char *gfxModeName, int cursorTargetScale) {
		
		byte palette[3 * 4]; // Black, white and yellow
		palette[0] = palette[1] = palette[2] = 0;
		palette[4] = palette[5] = palette[6] = 255;
		palette[8] = palette[9] = 255;
		palette[10] = 0;
		
		byte buffer[10][10];
		memset(&buffer[0][0], 2, 10 * 10);

		// Mark the hotspot
		for (int i = 0; i < 10; i++) {
			buffer[i][i] = 0;
			buffer[9 - i][i] = 0;
		}
	
		CursorMan.pushCursorPalette(palette, 0, 3);
		CursorMan.pushCursor(&buffer[0][0], 10, 10, 5, 5, cursorTargetScale);
		CursorMan.showMouse(true);
		g_system->updateScreen();

		if (gfxModeName) {
			Common::Point pt(0, 100);
			char scaleFactor[10];
			snprintf(scaleFactor, 10, "%dx", cursorTargetScale);
			Common::String info = "GFX Mode:";
			info = info + gfxModeName  + " Cursor scaled by:" + scaleFactor;
			Testsuite::clearScreen();
			Testsuite::writeOnScreen(info, pt);
		}

}


/**
 * Used by aspectRatio()
 */

void GFXtests::drawEllipse(int cx, int cy, int a, int b) {	
	
	// Take a buffer of screen size

	byte buffer[200][320] = {{0}};
	float theta;
	int x, y, x1, y1;

	// Illuminate the center
	buffer[cx][cy] = 1;
	
	// Illuminate the points lying on ellipse

	for (theta = 0; theta <= PI / 2; theta += PI / 360  ) {
		x = (int)(b * sin(theta) + 0.5);
		y = (int)(a * cos(theta) + 0.5);
		
		// This gives us four points
		
		x1 = x + cx;
		y1 = y + cy;
		
		buffer[x1][y1] = 1;

		x1 = (-1) * x + cx;
		y1 = y + cy;
		
		buffer[x1][y1] = 1;
		
		x1 = x + cx;
		y1 = (-1) * y + cy;
		
		buffer[x1][y1] = 1;

		x1 = (-1) * x + cx;
		y1 = (-1) * y + cy;
		
		buffer[x1][y1] = 1;
	}

	g_system->copyRectToScreen(&buffer[0][0], 320, 0, 0, 320, 200);
	g_system->updateScreen();
}

// GFXtests go here

/**
 * Tests the fullscreen mode by: toggling between fullscreen and windowed mode
 */

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
	} else {
		Testsuite::displayMessage("feature not supported");
	}

	Testsuite::clearScreen(rect);
	return true;
}

/**
 * Tests the aspect ratio correction by: drawing an ellipse, when corrected the ellipse should render to a circle
 */

bool GFXtests::aspectRatio() {
	Testsuite::displayMessage("Testing Aspect Ratio Correction.\n"
	"With this feature enabled games running at 320x200 should be scaled upto 320x240 pixels");

	// Draw an ellipse on the screen
	
	drawEllipse(100, 160, 72, 60);
	
	Common::Point pt(0, 180);
	Testsuite::writeOnScreen("when corrected, it should be a circle!", pt);
	
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
	} else {
		Testsuite::displayMessage("feature not supported");
	}

	g_system->delayMillis(500);
	Testsuite::clearScreen();
	return true;
}

/**
 * Tests Palettized cursors.
 * Method: Create a yellow colored cursor, should be able to move it. Once you click test terminates
 */

bool GFXtests::palettizedCursors() {
	Testsuite::displayMessage("Testing Cursors. You should expect to see a yellow colored square cursor.\n"
	"You should be able to move it. The test finishes when the mouse(L/R) is clicked");
	
	Common::Point pt(0, 100);
	
	bool isFeaturePresent;
	bool isFeatureEnabled;

	isFeaturePresent = g_system->hasFeature(OSystem::kFeatureCursorHasPalette);
	isFeatureEnabled = g_system->getFeatureState(OSystem::kFeatureCursorHasPalette);

	if (isFeaturePresent) {
		GFXtests::drawCursor();
		
		Testsuite::writeOnScreen("Testing Palettized Cursors", pt);
		
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
					Testsuite::clearScreen();
					Testsuite::writeOnScreen("Mouse Clicked", pt);
					printf("Mouse Clicked\n");
					g_system->delayMillis(1000);
					quitLoop = true;
					Testsuite::clearScreen();
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
	Testsuite::clearScreen();

	// Testing Mouse Movements now!
	
	Testsuite::writeOnScreen("Moving mouse automatically from (0, 0) to (100, 100)", pt);
	g_system->warpMouse(0, 0);
	g_system->updateScreen();
	g_system->delayMillis(1000);

	for (int i = 0; i <= 100; i++) {
		g_system->delayMillis(20);
		g_system->warpMouse(i, i);
		g_system->updateScreen();
	}
	
	Testsuite::clearScreen();
	Testsuite::writeOnScreen("Mouse Moved to (100, 100)", pt);
	g_system->delayMillis(1500);
	// Popping cursor
	CursorMan.popCursorPalette();
	CursorMan.popCursor();

	Testsuite::clearScreen();
	return true;
}


/**
 * This basically blits the screen by the contents of its buffer.
 *
 */
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

/**
 * Testing feature : Iconifying window
 * It is expected the screen minimizes when this feature is enabled 
 */
bool GFXtests::iconifyWindow() {
	
	Testsuite::displayMessage("Testing Iconify Window mode.\n" 
	"If the feature is supported by the backend, you should expect to see a toggle between minimized and normal states");

	Common::Point pt(0, 100);
	Common::Rect rect = Testsuite::writeOnScreen("Testing Iconifying window", pt);
	
	bool isFeaturePresent;
	bool isFeatureEnabled;

	isFeaturePresent = g_system->hasFeature(OSystem::kFeatureIconifyWindow);
	isFeatureEnabled = g_system->getFeatureState(OSystem::kFeatureIconifyWindow);
	g_system->delayMillis(1000);

	if (isFeaturePresent) {
		// Toggle

		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureIconifyWindow, !isFeatureEnabled);
		g_system->endGFXTransaction();

		g_system->delayMillis(1000);
		
		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureIconifyWindow, isFeatureEnabled);
		g_system->endGFXTransaction();
	} else {
		Testsuite::displayMessage("feature not supported");
	}

	Testsuite::clearScreen(rect);
	return true;
}

/**
 * Testing feature: Scaled cursors
 */

bool GFXtests::scaledCursors() {

	// TODO : Understand and fix the problem relating scaled cursors
	
	const OSystem::GraphicsMode *gfxMode = g_system->getSupportedGraphicsModes();
	
	while (gfxMode->name) {
		// for every graphics mode display cursors for cursorTargetScale 1, 2 and 3
		// Switch Graphics mode
		g_system->warpMouse(80, 160);

		//if (g_system->setGraphicsMode(gfxMode->id)) {
		if (1) {
			drawCursor(gfxMode->name, 1);
			g_system->delayMillis(5000);
			drawCursor(gfxMode->name, 2);
			g_system->delayMillis(5000);
			drawCursor(gfxMode->name, 3);
			g_system->delayMillis(5000);
		} else {
			printf("Switching to graphics mode %s failed\n", gfxMode->name);
		}
		CursorMan.popAllCursors();
		gfxMode++;
	}

	return true;
}

bool GFXtests::shakingEffect() {
	Common::Point pt(0, 100);
	Testsuite::writeOnScreen("Does this shakes!!?", pt);
	int times = 25;
	while (times--) {
		g_system->setShakePos(10);
		g_system->updateScreen();
		g_system->setShakePos(0);
		g_system->updateScreen();
	}
	g_system->delayMillis(500);
	Testsuite::clearScreen();
	return true;
}

bool GFXtests::focusRectangle() {
	Testsuite::clearScreen();

	const Graphics::Font &font(*FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont));

	Graphics::Surface *screen = g_system->lockScreen();
	int screenHeight = g_system->getHeight();
	int screenWidth = g_system->getWidth();

	int height = font.getFontHeight();
	int width = screenWidth / 2;

	Common::Rect rectLeft(0, 0, width, height * 2);
	screen->fillRect(rectLeft, kColorWhite);
	font.drawString(screen, "Focus 1", rectLeft.left, rectLeft.top, width, kColorBlack, Graphics::kTextAlignLeft);

	Common::Rect rectRight(screenWidth - width, screenHeight - height * 2 , screenWidth, screenHeight);
	screen->fillRect(rectRight, kColorWhite);
	font.drawString(screen, "Focus 2", rectRight.left, rectRight.top, width, kColorBlack, Graphics::kTextAlignRight);
	g_system->unlockScreen();
	g_system->updateScreen();

	g_system->setFocusRectangle(rectLeft);
	g_system->updateScreen();
	
	g_system->delayMillis(1000);

	g_system->setFocusRectangle(rectRight);
	g_system->updateScreen();

	return true;
}

bool GFXtests::overlayGraphics() {
	Graphics::PixelFormat pf = g_system->getOverlayFormat();
	
	GFXTestSuite::setCustomColor(255, 255, 0);
	OverlayColor buffer[20 * 40];
	memset(buffer, 2, 20 * 40);

	int x = g_system->getWidth() / 2 - 20;
	int y = g_system->getHeight() / 2 - 10;

	g_system->copyRectToOverlay(buffer, 40, x, y, 40, 20);
	g_system->showOverlay();
	g_system->updateScreen();
	g_system->delayMillis(1000);
	g_system->hideOverlay();

	return true;
}

}
