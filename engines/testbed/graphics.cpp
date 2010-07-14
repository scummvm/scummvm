/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#include "common/events.h"
#include "common/list.h"
#include "common/random.h"

#include "engines/engine.h"

#include "testbed/graphics.h"
#include "testbed/testsuite.h"

#include "graphics/cursorman.h"
#include "graphics/fontman.h"
#include "graphics/surface.h"
#include "graphics/VectorRendererSpec.h"

namespace Testbed {

byte GFXTestSuite::_palette[256 * 4] = {0, 0, 0, 0, 255, 255, 255, 0, 255, 255, 255, 0};

GFXTestSuite::GFXTestSuite() {
	// Initialize color palettes
	// The fourth field is for alpha channel which is unused
	// Assuming 8bpp as of now
	g_system->setPalette(_palette, 0, 3);

	// Init Mouse Palette (White-black-yellow)
	GFXtests::initMousePalette();

	// Add tests here

	// Blitting buffer on screen
	addTest("BlitBitmaps", &GFXtests::copyRectToScreen);

	// GFX Transcations
	addTest("FullScreenMode", &GFXtests::fullScreenMode);
	addTest("AspectRatio", &GFXtests::aspectRatio);
	addTest("IconifyingWindow", &GFXtests::iconifyWindow);

	// Mouse Layer tests (Palettes and movements)
	addTest("PalettizedCursors", &GFXtests::palettizedCursors);
	// FIXME: Scaled cursor crsh with odd dimmensions
	addTest("ScaledCursors", &GFXtests::scaledCursors);

	// Effects
	addTest("shakingEffect", &GFXtests::shakingEffect);
	addTest("focusRectangle", &GFXtests::focusRectangle);

	// Overlay
	addTest("Overlays", &GFXtests::overlayGraphics);

	// Specific Tests:
	addTest("Palette Rotation", &GFXtests::paletteRotation);
	//addTest("Pixel Formats", &GFXtests::pixelFormats);

}

const char *GFXTestSuite::getName() const {
	return "GFX";
}

void GFXTestSuite::setCustomColor(uint r, uint g, uint b) {
	_palette[8] = r;
	_palette[9] = g;
	_palette[10] = b;
	g_system->setPalette(_palette, 0, 256);
}

// Helper functions used by GFX tests

void GFXtests::initMousePalette() {
	byte palette[3 * 4]; // Black, white and yellow

	palette[0] = palette[1] = palette[2] = 0;
	palette[4] = palette[5] = palette[6] = 255;
	palette[8] = palette[9] = 255;
	palette[10] = 0;

	CursorMan.replaceCursorPalette(palette, 0, 3);

}

Common::Rect GFXtests::computeSize(Common::Rect &cursorRect, int scalingFactor, int cursorTargetScale) {
	if (cursorTargetScale == 1 || scalingFactor == 1) {
		// Game data and cursor would be scaled equally.
		// so dimensions would be same.
		return Common::Rect(cursorRect.width(), cursorRect.height());
	}

	if (scalingFactor == 2) {
		// Game data is scaled by 2, cursor is said to be scaled by 2 or 3. so it wud not be scaled any further
		// So a w/2 x h/2 rectangle when scaled would match the cursor
		return Common::Rect(cursorRect.width() / 2, cursorRect.height() / 2);
	}

	if (scalingFactor == 3) {
		// Cursor traget scale is 2 or 3.
		return Common::Rect((cursorRect.width() / cursorTargetScale), (cursorRect.height() / cursorTargetScale));
	} else {
		Testsuite::logPrintf("Unsupported scaler %dx\n", scalingFactor);
		return Common::Rect();
	}
}

void GFXtests::HSVtoRGB(int& rComp, int& gComp, int& bComp, int hue, int sat, int val) {

	float r = rComp;
	float g = gComp;
	float b = bComp;

	float h = hue * (360 / 254.0); // two colors are left for bg and fg
	float s = sat;
	float v = val;

	int i;
	float f, p, q, t;

	if (s == 0) {
		r = g = b = v * 255;
		return;
	}

	h /= 60;
	i = (int) h;
	f = h - i;
	p = v * (1 - s);
	q = v * (1 - s * f);
	t = v * (1 - s * (1 - f));

	switch (i) {
		case 0:
			r = v;
			g = t;
			b = p;
			break;
		case 1:
			r = q;
			g = v;
			b = p;
			break;
		case 2:
			r = p;
			g = v;
			b = t;
			break;
		case 3:
			r = p;
			g = q;
			b = v;
			break;
		case 4:
			r = t;
			g = p;
			b = v;
			break;
		default:
			r = v;
			g = p;
			b = q;
			break;
	}

	rComp = r * 255;
	gComp = g * 255;
	bComp = b * 255;
}

Common::Rect GFXtests::drawCursor(bool cursorPaletteDisabled, const char *gfxModeName, int cursorTargetScale) {

	// Buffer initialized with yellow color
	byte buffer[500];
	memset(buffer, 2, sizeof(buffer));

	int cursorWidth = 12;
	int cursorHeight = 12;

	/* Disable HotSpot highlighting as of now

	// Paint the cursor with yellow, except the hotspot
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			if (i != j && i != 15 - j) {
				buffer[i * 16 + j] = 2;
			}
		}
	}

	*/

	// Uncommenting the next line and commenting the line after that would reproduce the crash
	// CursorMan.replaceCursor(buffer, 11, 11, 0, 0, 255, cursorTargetScale);
	CursorMan.replaceCursor(buffer, 12, 12, 0, 0, 255, cursorTargetScale);
	CursorMan.showMouse(true);

	if (cursorPaletteDisabled) {
		CursorMan.disableCursorPalette(true);
	} else {
		initMousePalette();
		CursorMan.disableCursorPalette(false);
	}

	g_system->updateScreen();
	return Common::Rect(0, 0, cursorWidth, cursorHeight);
}

void rotatePalette(byte *palette, int size) {
	// Rotate the colors starting from address palette "size" times

	// take a temporary palette color
	byte tColor[4] = {0};
	// save first color in it.
	memcpy(tColor, &palette[0], 4 * sizeof(byte));

	// Move each color upward by 1
	for (int i = 0; i < size - 1; i++) {
		memcpy(&palette[i * 4], &palette[(i + 1) * 4], 4 * sizeof(byte));
	}
	// Assign last color to tcolor
	memcpy(&palette[(size -1) * 4], tColor, 4 * sizeof(byte));
}

/**
 * Sets up mouse loop, exits when user clicks any of the mouse button
 */
void GFXtests::setupMouseLoop(bool disableCursorPalette, const char *gfxModeName, int cursorTargetScale) {

	bool isFeaturePresent;
	isFeaturePresent = g_system->hasFeature(OSystem::kFeatureCursorHasPalette);
	Common::Rect cursorRect;

	if (isFeaturePresent) {

		cursorRect = GFXtests::drawCursor(disableCursorPalette, gfxModeName, cursorTargetScale);

		Common::EventManager *eventMan = g_system->getEventManager();
		Common::Event event;
		Common::Point pt(0, 100);

		bool quitLoop = false;
		uint32 lastRedraw = 0;
		const uint32 waitTime = 1000 / 45;

		Testsuite::clearScreen();
		Common::String info = disableCursorPalette ? "Using Game Palette" : "Using cursor palette";
		info += " to render the cursor, Click to finish";

		Testsuite::writeOnScreen(info, pt);

		info = "GFX Mode";
		info += gfxModeName;
		info += " ";

		char cScale = cursorTargetScale + '0';
		info += "Cursor scale: ";
		info += cScale;

		Common::String gfxScalarMode(gfxModeName);
		Common::Rect estimatedCursorRect;

		if (!gfxScalarMode.equals("")) {

			if (gfxScalarMode.contains("1x")) {
				estimatedCursorRect = computeSize(cursorRect, 1, cursorTargetScale);
			} else if (gfxScalarMode.contains("2x")) {
				estimatedCursorRect = computeSize(cursorRect, 2, cursorTargetScale);
			} else if (gfxScalarMode.contains("3x")){
				estimatedCursorRect = computeSize(cursorRect, 3, cursorTargetScale);
			} else {
				// If unable to detect scaler, default to 2
				Testsuite::writeOnScreen("Unable to detect scaling factor, assuming 2x", Common::Point(0, 5));
				estimatedCursorRect = computeSize(cursorRect, 2, cursorTargetScale);
			}
			Testsuite::writeOnScreen(info, Common::Point(0, 120));

			// Move cursor to (20, 20)
			g_system->warpMouse(20, 20);
			// Move estimated rect to (20, 20)
			estimatedCursorRect.moveTo(20, 20);

			Graphics::Surface *screen = g_system->lockScreen();
			GFXTestSuite::setCustomColor(255, 0, 0);
			screen->fillRect(estimatedCursorRect, 2);
			g_system->unlockScreen();
			g_system->updateScreen();
		}

		while (!quitLoop) {
			while (eventMan->pollEvent(event)) {
				if (Engine::shouldQuit()) {
					// Quit directly
					return;
				}
				if (lastRedraw + waitTime < g_system->getMillis()) {
					g_system->updateScreen();
					lastRedraw = g_system->getMillis();
				}

				switch (event.type) {
				case Common::EVENT_MOUSEMOVE:
					break;
				case Common::EVENT_LBUTTONDOWN:
				case Common::EVENT_RBUTTONDOWN:
					quitLoop = true;
					Testsuite::clearScreen();
					Testsuite::writeOnScreen("Mouse clicked", pt);
					g_system->delayMillis(1000);
					break;
				default:
					break;// Ignore handling any other event

				}
			}
		}
	} else {
		Testsuite::displayMessage("feature not supported");
	}
}

void GFXtests::mouseMovements() {
	// Testing Mouse Movements now!
	Common::Point pt(0, 100);
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
	g_system->delayMillis(1000);
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

	Common::Point pt(0, 100);
	Common::Rect rect = Testsuite::writeOnScreen("Testing fullscreen mode", pt);

	bool isFeaturePresent;
	bool isFeatureEnabled;
	bool passed = true;
	Common::String prompt;
	OptionSelected shouldSelect;

	isFeaturePresent = g_system->hasFeature(OSystem::kFeatureFullscreenMode);

	if (isFeaturePresent) {
		// Toggle
		isFeatureEnabled = g_system->getFeatureState(OSystem::kFeatureFullscreenMode);
		shouldSelect = isFeatureEnabled ? kOptionLeft : kOptionRight;

		g_system->delayMillis(1000);

		if (isFeatureEnabled) {
			Testsuite::logDetailedPrintf("Current Mode is Fullsecreen\n");
		} else {
			Testsuite::logDetailedPrintf("Current Mode is Windowed\n");
		}

		prompt = " Which mode do you see currently ?  ";

		if (!Testsuite::handleInteractiveInput(prompt, "Fullscreen", "Windowed", shouldSelect)) {
			// User selected incorrect current state
			passed = false;
			Testsuite::logDetailedPrintf("g_system->getFeatureState() failed\n");
		}

		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureFullscreenMode, !isFeatureEnabled);
		g_system->endGFXTransaction();

		// Current state should be now !isFeatureEnabled
		isFeatureEnabled = g_system->getFeatureState(OSystem::kFeatureFullscreenMode);
		shouldSelect = isFeatureEnabled ? kOptionLeft : kOptionRight;

		g_system->delayMillis(1000);

		prompt = "  Which screen mode do you see now ?   ";

		if (!Testsuite::handleInteractiveInput(prompt, "Fullscreen", "Windowed", shouldSelect)) {
			// User selected incorrect mode
			passed = false;
			Testsuite::logDetailedPrintf("g_system->setFeatureState() failed\n");
		}

		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureFullscreenMode, !isFeatureEnabled);
		g_system->endGFXTransaction();

		g_system->delayMillis(1000);

		prompt = "This should be your initial state. Is it?";

		if (!Testsuite::handleInteractiveInput(prompt, "Yes, it is", "Nopes", shouldSelect)) {
			// User selected incorrect mode
			Testsuite::logDetailedPrintf("switching back to initial state failed\n");
			passed = false;
		}

	} else {
		Testsuite::displayMessage("feature not supported");
	}

	Testsuite::clearScreen();
	return passed;
}

/**
 * Tests the aspect ratio correction by: drawing an ellipse, when corrected the ellipse should render to a circle
 */

bool GFXtests::aspectRatio() {
	// Draw an ellipse on the screen

	drawEllipse(100, 160, 72, 60);

	Common::Point pt(0, 180);
	Testsuite::writeOnScreen("Testing Aspect Ratio Correction!", pt);

	bool isFeaturePresent;
	bool isFeatureEnabled;
	bool passed;
	Common::String prompt;
	OptionSelected shouldSelect;

	isFeaturePresent = g_system->hasFeature(OSystem::kFeatureAspectRatioCorrection);
	isFeatureEnabled = g_system->getFeatureState(OSystem::kFeatureAspectRatioCorrection);
	g_system->delayMillis(1000);

	if (isFeaturePresent) {
		// Toggle
		shouldSelect = isFeatureEnabled ? kOptionLeft : kOptionRight;
		prompt = " What does the curve on screen appears to you ?";
		if (!Testsuite::handleInteractiveInput(prompt, "Circle", "Ellipse", shouldSelect)) {
			// User selected incorrect option
			passed = false;
			Testsuite::logDetailedPrintf("Aspect Ratio Correction failed\n");
		}

		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureAspectRatioCorrection, !isFeatureEnabled);
		g_system->endGFXTransaction();

		g_system->delayMillis(1000);

		shouldSelect = !isFeatureEnabled ? kOptionLeft : kOptionRight;
		prompt = " What does the curve on screen appears to you ?";
		if (!Testsuite::handleInteractiveInput(prompt, "Circle", "Ellipse", shouldSelect)) {
			// User selected incorrect option
			passed = false;
			Testsuite::logDetailedPrintf("Aspect Ratio Correction failed\n");
		}

		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureAspectRatioCorrection, isFeatureEnabled);
		g_system->endGFXTransaction();
	} else {
		Testsuite::displayMessage("feature not supported");
	}

	g_system->delayMillis(500);

	if (Testsuite::handleInteractiveInput("This should definetely be your initial state?", "Yes, it is", "Nopes", kOptionRight)) {
		// User selected incorrect mode
		Testsuite::logDetailedPrintf("Switching back to initial state failed\n");
		passed = false;
	}

	Testsuite::clearScreen();
	return passed;
}

/**
 * Tests Palettized cursors.
 * Method: Create a yellow colored cursor, should be able to move it. Once you click test terminates
 */

bool GFXtests::palettizedCursors() {


	bool passed = true;

	Testsuite::displayMessage("Testing Cursors. You should expect to see a yellow colored square cursor.\n"
	"You should be able to move it. The test finishes when the mouse(L/R) is clicked");

	// Testing with cursor Palette
	setupMouseLoop();
	// Test Automated Mouse movements (warp)
	mouseMovements();

	if (Testsuite::handleInteractiveInput("Which color did the cursor appeared to you?", "Yellow", "Any other", kOptionRight)) {
		Testsuite::logDetailedPrintf("Couldn't use cursor palette for rendering cursor\n");
		passed = false;
	}

	// Testing with game Palette
	GFXTestSuite::setCustomColor(255, 0, 0);
	setupMouseLoop(true);

	if (Testsuite::handleInteractiveInput("Which color did the cursor appeared to you?", "Red", "Any other", kOptionRight)) {
		Testsuite::logDetailedPrintf("Couldn't use Game palette for rendering cursor\n");
		passed = false;
	}

	if (!Testsuite::handleInteractiveInput("Did Cursor tests went as you were expecting?")) {
		passed = false;
	}

	Testsuite::clearScreen();
	// Done with cursors, make them invisible, any other test the could simply make it visible
	CursorMan.showMouse(false);
	return passed;
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
	Testsuite::clearScreen();

	if (Testsuite::handleInteractiveInput("Did the test worked as you were expecting?", "Yes", "No", kOptionRight)) {
		return false;
	}

	return true;

}

/**
 * Testing feature : Iconifying window
 * It is expected the screen minimizes when this feature is enabled
 */
bool GFXtests::iconifyWindow() {

	Testsuite::displayMessage("Testing Iconify Window mode.\n If the feature is supported by the backend,"
	"you should expect the window to be minimized. However you would manually need to de-iconify.");

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

	if (Testsuite::handleInteractiveInput("Did the test worked as you were expecting?", "Yes", "No", kOptionRight)) {
		return false;
	}

	Testsuite::clearScreen();
	return true;
}

/**
 * Testing feature: Scaled cursors
 */

bool GFXtests::scaledCursors() {

	Testsuite::displayMessage("Testing : Scaled cursors\n"
	"Here every graphics mode is tried with a cursorTargetScale of 1,2 and 3.\n"
	"The expected cursor size is drawn as a rectangle, the cursor should entirely cover that rectangle.\n"
	"This may take time, You may skip the later scalers and just examine the first three i.e 1x,2x and 3x");

	int maxLimit = 1000;
	if (!Testsuite::handleInteractiveInput("Do you want to restrict scalers to 1x, 2x and 3x only?", "Yes", "No", kOptionRight)) {
		maxLimit = 3;
	}

	const int currGFXMode = g_system->getGraphicsMode();
	const OSystem::GraphicsMode *gfxMode = g_system->getSupportedGraphicsModes();

	while (gfxMode->name && maxLimit > 0) {
		// for every graphics mode display cursors for cursorTargetScale 1, 2 and 3
		// Switch Graphics mode
		// FIXME: Crashes with "3x" mode now.:
		g_system->beginGFXTransaction();

		bool isGFXModeSet = g_system->setGraphicsMode(gfxMode->id);
		g_system->initSize(320, 200);

		OSystem::TransactionError gfxError = g_system->endGFXTransaction();

		if (gfxError == OSystem::kTransactionSuccess && isGFXModeSet) {

			setupMouseLoop(false, gfxMode->name, 1);
			Testsuite::clearScreen();

			setupMouseLoop(false, gfxMode->name, 2);
			Testsuite::clearScreen();

			setupMouseLoop(false, gfxMode->name, 3);
			Testsuite::clearScreen();

		} else {
			Testsuite::logDetailedPrintf("Switching to graphics mode %s failed\n", gfxMode->name);
			return false;
		}
		gfxMode++;
		maxLimit--;
	}

	// Restore Original State
	g_system->beginGFXTransaction();
	bool isGFXModeSet = g_system->setGraphicsMode(currGFXMode);
	g_system->initSize(320, 200);
	OSystem::TransactionError gfxError = g_system->endGFXTransaction();

	if (gfxError != OSystem::kTransactionSuccess || !isGFXModeSet) {
		Testsuite::logDetailedPrintf("Switcing to initial state failed\n");
		return false;
	}

	// Done with cursors, Make them invisible, any other test may enable and use it.
	CursorMan.showMouse(false);
	Testsuite::clearScreen();
	return true;
}

bool GFXtests::shakingEffect() {
	Common::Point pt(0, 100);
	Testsuite::writeOnScreen("If Shaking effect works,this should shake!", pt);
	int times = 35;
	while (times--) {
		g_system->setShakePos(10);
		g_system->updateScreen();
		g_system->setShakePos(0);
		g_system->updateScreen();
	}
	g_system->delayMillis(1500);

	if (Testsuite::handleInteractiveInput("Did the test worked as you were expecting?", "Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Shaking Effect didn't worked");
		return false;
	}
	Testsuite::clearScreen();
	return true;
}

bool GFXtests::focusRectangle() {

	Testsuite::displayMessage("Testing : Setting and hiding Focus \n"
	"If this feature is implemented, the focus should be toggled between the two rectangles on the corners");

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

	g_system->clearFocusRectangle();

	g_system->setFocusRectangle(rectLeft);
	g_system->updateScreen();

	g_system->delayMillis(1000);

	g_system->setFocusRectangle(rectRight);
	g_system->updateScreen();

	g_system->clearFocusRectangle();

	if (Testsuite::handleInteractiveInput("Did you noticed a variation in focus?", "Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Focus Rectangle feature doesn't works. Check platform.\n");
	}

	Testsuite::clearScreen();
	return true;
}

bool GFXtests::overlayGraphics() {

	Graphics::PixelFormat pf = g_system->getOverlayFormat();

	OverlayColor buffer[50 * 100];
	OverlayColor value = pf.RGBToColor(0, 255, 0);

	for (int i = 0; i < 50 * 100; i++) {
		buffer[i] = value;
	}

	g_system->showOverlay();
	g_system->copyRectToOverlay(buffer, 100, 270, 175, 100, 50);
	g_system->updateScreen();

	g_system->delayMillis(1000);

	g_system->hideOverlay();
	g_system->updateScreen();

	if (Testsuite::handleInteractiveInput("Did you see a green overlayed rectangle?", "Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Overlay Rectangle feature doesn't works\n");
		return false;
	}

	Testsuite::clearScreen();
	return true;
}

bool GFXtests::paletteRotation() {
	Common::Point pt(0, 10);
	Testsuite::writeOnScreen("Rotating palettes, palettes rotate towards left, click to stop!", pt);

	// Use 256 colors
	byte palette[256 * 4] = {0, 0, 0, 0,
							 255, 255, 255, 0};

	int r, g, b;
	int colIndx;

	for (int i = 2; i < 256; i++) {
		HSVtoRGB(r, g, b, i - 2, 1, 1);
		colIndx = i * 4;
		palette[colIndx] = r;
		palette[colIndx + 1] = g;
		palette[colIndx + 2] = b;
	}

	// Initialize this palette.
	g_system->setPalette(palette, 0, 256);

	// Draw 254 Rectangles, each 1 pixel wide and 10 pixels long
	// one for 0-255 color range other for 0-127-255 range
	byte buffer[254 * 30] = {0};

	for (int i = 0; i < 30; i++) {
		for (int j = 0; j < 254; j++) {
			if (i < 10) {
				buffer[i * 254 + j] = j + 2;
			} else if (i < 20) {
				buffer[i * 254 + j] = 0;
			} else {
				buffer[i * 254 + j] = ((j + 127) % 254) + 2;
			}
		}
	}

	g_system->copyRectToScreen(buffer, 254, 22, 50, 254, 30);
	g_system->updateScreen();
	g_system->delayMillis(1000);


	bool toRotate = true;
	Common::Event event;
	CursorMan.showMouse(true);

	while (toRotate) {
		while (g_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_RBUTTONDOWN) {
				toRotate = false;
			}
		}

		/*for (int i = 2; i < 256; i++) {
			debug("Palette: (%d %d %d) #", palette[i*4], palette[i*4+1], palette[i*4+2]);
		}*/

		rotatePalette(&palette[8], 254);

		/*for (int i = 2; i < 256; i++) {
			debug("Palette: (%d %d %d) #", palette[i*4], palette[i*4+1], palette[i*4+2]);
		}*/

		g_system->delayMillis(10);
		g_system->setPalette(palette, 0, 256);
		g_system->updateScreen();
	}

	CursorMan.showMouse(false);
	// Reset initial palettes
	GFXTestSuite::setCustomColor(255, 0, 0);
	Testsuite::clearScreen();

	if(Testsuite::handleInteractiveInput("Did you saw a rotation in colors of rectangles displayed on screen?", "Yes", "No", kOptionRight)) {
		return false;
	}

	Testsuite::clearScreen();
	return true;
}

bool GFXtests::pixelFormats() {
	Common::List<Graphics::PixelFormat> pfList = g_system->getSupportedFormats();
	Common::List<Graphics::PixelFormat>::const_iterator iter = pfList.begin();

	int numFormatsTested = 0;
	int numPassed = 0;
	bool numFailed = 0;

	Testsuite::logDetailedPrintf("Testing Pixel Formats. Size of list : %d\n", pfList.size());

	for (iter = pfList.begin(); iter != pfList.end(); iter++) {

		numFormatsTested++;
		if (iter->bytesPerPixel == 1) {
			// Palettes already tested
			continue;
		} else if (iter->bytesPerPixel > 2) {
			Testsuite::logDetailedPrintf("Can't test pixels with bpp > 2\n");
			continue;
		}

		// Switch to that pixel Format
		g_system->beginGFXTransaction();
		g_system->initSize(320, 200, &(*iter));
		g_system->endGFXTransaction();
		Testsuite::clearScreen(true);


		// Draw some nice gradients
		// Pick up some colors
		uint colors[6];

		colors[0] = iter->RGBToColor(255, 255, 255);
		colors[1] = iter->RGBToColor(135, 48, 21);
		colors[2] = iter->RGBToColor(205, 190, 87);
		colors[3] = iter->RGBToColor(0, 32, 64);
		colors[4] = iter->RGBToColor(181, 126, 145);
		colors[5] = iter->RGBToColor(47, 78, 36);

		Common::Point pt(0, 170);
		char msg[100];
		// XXX: Can use snprintf?
		snprintf(msg, sizeof(msg), "Testing Pixel Formats, %d of %d", numFormatsTested, pfList.size());
		Testsuite::writeOnScreen(msg, pt, true);

		// CopyRectToScreen could have been used, but that may involve writing code which
		// already resides in graphics/surface.h
		// So using Graphics::Surface

		Graphics::Surface *screen = g_system->lockScreen();

		// Draw 6 rectangles centred at (50, 160), piled over one another
		// each with color in colors[]
		for (int i = 0; i < 6; i++) {
			screen->fillRect(Common::Rect::center(160, 20 + i * 10, 100, 10), colors[i]);
		}

		g_system->unlockScreen();
		g_system->updateScreen();
		g_system->delayMillis(500);

		if(Testsuite::handleInteractiveInput("Were you able to notice the colored rectangles on the screen for this format?", "Yes", "No", kOptionLeft)) {
			numPassed++;
		} else {
			numFailed++;
			Testsuite::logDetailedPrintf("Testing pixel format failed for format #%d on the list\n", numFormatsTested);
		}
	}

	// Revert back to 8bpp
	g_system->beginGFXTransaction();
	g_system->initSize(320, 200);
	g_system->endGFXTransaction();
	GFXTestSuite::setCustomColor(255, 0, 0);
	initMousePalette();
	Testsuite::clearScreen();


	if (numFailed) {
		Testsuite::logDetailedPrintf("Pixel Format test: Failed : %d, Passed : %d, Ignored %d\n",numFailed, numPassed, numFormatsTested - (numPassed + numFailed));
		return false;
	}

	return true;
}

}
