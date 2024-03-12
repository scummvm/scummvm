/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/events.h"
#include "common/list.h"
#include "common/random.h"

#include "engines/engine.h"

#include "testbed/graphics.h"
#include "testbed/testsuite.h"

#include "graphics/cursorman.h"
#include "graphics/fontman.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"
#include "graphics/VectorRendererSpec.h"

#include "graphics/fonts/bdf.h"

namespace Testbed {

byte GFXTestSuite::_palette[256 * 3] = {0, 0, 0, 255, 255, 255, 255, 255, 255};

GFXTestSuite::GFXTestSuite() {
	// Add tests here

	// Pixel Formats
	addTest("pixelFormatsSupported", &GFXtests::pixelFormatsSupported);
	addTest("pixelFormatsRequired", &GFXtests::pixelFormatsRequired);

	// Blitting buffer on screen
	addTest("BlitBitmaps", &GFXtests::copyRectToScreen);

	// GFX Transcations
	addTest("FullScreenMode", &GFXtests::fullScreenMode);
	addTest("FilteringMode", &GFXtests::filteringMode);
	addTest("AspectRatio", &GFXtests::aspectRatio);
	addTest("IconifyingWindow", &GFXtests::iconifyWindow);

	// Mouse Layer tests (Palettes and movements)
	addTest("PalettizedCursors", &GFXtests::palettizedCursors);
	addTest("AlphaCursors", &GFXtests::alphaCursors);
	addTest("MaskedCursors", &GFXtests::maskedCursors);
	addTest("MouseMovements", &GFXtests::mouseMovements);
	// FIXME: Scaled cursor crash with odd dimmensions
	addTest("ScaledCursors", &GFXtests::scaledCursors);

	// Effects
	addTest("shakingEffect", &GFXtests::shakingEffect);
	// addTest("focusRectangle", &GFXtests::focusRectangle);

	// Overlay
	addTest("Overlays", &GFXtests::overlayGraphics);

	// Specific Tests:
	addTest("PaletteRotation", &GFXtests::paletteRotation);
	addTest("cursorTrailsInGUI", &GFXtests::cursorTrails);
}

void GFXTestSuite::prepare() {
	// Initialize color palettes
	// The fourth field is for alpha channel which is unused
	// Assuming 8bpp as of now
	g_system->getPaletteManager()->setPalette(_palette, 0, 3);

	// Init Mouse Palette (White-black-yellow)
	GFXtests::initMousePalette();
	GFXtests::initMouseCursor();
}

void GFXTestSuite::setCustomColor(uint r, uint g, uint b) {
	_palette[6] = r;
	_palette[7] = g;
	_palette[8] = b;

	// Set colorNum kColorSpecial with a special color.
	int absIndx = kColorSpecial * 3;
	_palette[absIndx + 1] = 173;
	_palette[absIndx + 2] = 255;
	_palette[absIndx + 3] = 47;
	g_system->getPaletteManager()->setPalette(_palette, 0, 256);
}

// Helper functions used by GFX tests

void GFXtests::initMousePalette() {
	byte palette[3 * 3]; // Black, white and yellow

	palette[0] = palette[1] = palette[2] = 0;
	palette[3] = palette[4] = palette[5] = 255;
	palette[6] = palette[7] = 255;
	palette[8] = 0;

	CursorMan.replaceCursorPalette(palette, 0, 3);
}

static const byte MOUSECURSOR_SCI[] = {
	1,1,0,0,0,0,0,0,0,0,0,
	1,2,1,0,0,0,0,0,0,0,0,
	1,2,2,1,0,0,0,0,0,0,0,
	1,2,2,2,1,0,0,0,0,0,0,
	1,2,2,2,2,1,0,0,0,0,0,
	1,2,2,2,2,2,1,0,0,0,0,
	1,2,2,2,2,2,2,1,0,0,0,
	1,2,2,2,2,2,2,2,1,0,0,
	1,2,2,2,2,2,2,2,2,1,0,
	1,2,2,2,2,2,2,2,2,2,1,
	1,2,2,2,2,2,1,0,0,0,0,
	1,2,1,0,1,2,2,1,0,0,0,
	1,1,0,0,1,2,2,1,0,0,0,
	0,0,0,0,0,1,2,2,1,0,0,
	0,0,0,0,0,1,2,2,1,0,0,
	0,0,0,0,0,0,1,2,2,1,0
};

void GFXtests::initMouseCursor() {
	CursorMan.replaceCursor(MOUSECURSOR_SCI, 11, 16, 0, 0, 0);
}

Common::Rect GFXtests::computeSize(const Common::Rect &cursorRect, int scalingFactor, int cursorTargetScale) {
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

void GFXtests::HSVtoRGB(int &rComp, int &gComp, int &bComp, int hue, int sat, int val) {
	float r = rComp;
	float g = gComp;
	float b = bComp;

	float h = hue * (360 / 256.0); // All colors are tried
	float s = sat;
	float v = val;

	int i;
	float f, p, q, t;

	if (s == 0) {
		rComp = gComp = bComp = (int)(v * 255);
		return;
	}

	h /= 60;
	i = (int)h;
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

	rComp = (int)(r * 255);
	gComp = (int)(g * 255);
	bComp = (int)(b * 255);
}

Common::Rect GFXtests::drawCursor(bool cursorPaletteDisabled, int cursorTargetScale) {
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
	byte tColor[3] = {0};
	// save first color in it.
	memcpy(tColor, &palette[0], 3 * sizeof(byte));

	// Move each color upward by 1
	for (int i = 0; i < size - 1; i++) {
		memcpy(&palette[i * 3], &palette[(i + 1) * 3], 3 * sizeof(byte));
	}
	// Assign last color to tcolor
	memcpy(&palette[(size - 1) * 3], tColor, 3 * sizeof(byte));
}

/**
 * Sets up mouse loop, exits when user clicks any of the mouse button
 */
void GFXtests::setupMouseLoop(bool disableCursorPalette, const char *gfxModeName, int cursorTargetScale) {
	bool isFeaturePresent;
	isFeaturePresent = g_system->hasFeature(OSystem::kFeatureCursorPalette);
	Common::Rect cursorRect;

	if (isFeaturePresent) {

		cursorRect = GFXtests::drawCursor(disableCursorPalette, cursorTargetScale);

		Common::EventManager *eventMan = g_system->getEventManager();
		Common::Event event;
		Common::Point pt(0, 100);

		bool quitLoop = false;
		uint32 lastRedraw = 0;
		const uint32 waitTime = 1000 / 45;

		Testsuite::clearScreen();
		Common::String info = disableCursorPalette ? "Using Game Palette" : "Using cursor palette";
		info += " to render the cursor, Click to finish";

		Common::String gfxScalarMode(gfxModeName);

		if (!gfxScalarMode.equals("")) {
			info = "The cursor size (yellow) should match the red rectangle.";
		}

		Testsuite::writeOnScreen(info, pt);

		info = "GFX Mode";
		info += gfxModeName;
		info += " ";

		char cScale = cursorTargetScale + '0';
		info += "Cursor scale: ";
		info += cScale;

		Common::Rect estimatedCursorRect;

		if (!gfxScalarMode.equals("")) {

			if (gfxScalarMode.contains("1x")) {
				estimatedCursorRect = computeSize(cursorRect, 1, cursorTargetScale);
			} else if (gfxScalarMode.contains("2x")) {
				estimatedCursorRect = computeSize(cursorRect, 2, cursorTargetScale);
			} else if (gfxScalarMode.contains("3x")) {
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

			GFXTestSuite::setCustomColor(255, 0, 0);
			g_system->fillScreen(estimatedCursorRect, 2);
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

/**
 * Used by aspectRatio()
 */
void GFXtests::drawEllipse(int cx, int cy, int a, int b) {

	// Take a buffer of screen size
	int width = g_system->getWidth();
	int height = Testsuite::getDisplayRegionCoordinates().y;
	byte *buffer = new byte[height * width]();
	double theta;
	int x, y, x1, y1;
	// Illuminate the center
	buffer[cx * width + cy] = 1;

	// Illuminate the points lying on ellipse

	for (theta = 0; theta <= M_PI / 2; theta += M_PI / 360) {
		x = (int)(b * sin(theta) + 0.5);
		y = (int)(a * cos(theta) + 0.5);

		// This gives us four points

		x1 = x + cx;
		y1 = y + cy;

		buffer[x1 * width + y1] = 1;

		x1 = (-1) * x + cx;
		y1 = y + cy;

		buffer[x1 * width + y1] = 1;

		x1 = x + cx;
		y1 = (-1) * y + cy;

		buffer[x1 * width + y1] = 1;

		x1 = (-1) * x + cx;
		y1 = (-1) * y + cy;

		buffer[x1 * width + y1] = 1;
	}

	g_system->copyRectToScreen(buffer, width, 0, 0, width, height);
	g_system->updateScreen();
	delete[] buffer;
}

// GFXtests go here

/**
 * Tests the fullscreen mode by: toggling between fullscreen and windowed mode
 */
TestExitStatus GFXtests::fullScreenMode() {
	Testsuite::clearScreen();
	Common::String info = "Fullscreen test. Here you should expect a toggle between windowed and fullscreen states depending "
	"upon your initial state.";

	Common::Point pt(0, 100);
	Testsuite::writeOnScreen("Testing fullscreen mode", pt);

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : FullScreenMode\n");
		return kTestSkipped;
	}

	bool isFeaturePresent;
	bool isFeatureEnabled;
	TestExitStatus passed = kTestPassed;
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
			passed = kTestFailed;
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
			passed = kTestFailed;
			Testsuite::logDetailedPrintf("g_system->setFeatureState() failed\n");
		}

		g_system->beginGFXTransaction();
			g_system->setFeatureState(OSystem::kFeatureFullscreenMode, !isFeatureEnabled);
		g_system->endGFXTransaction();

		g_system->delayMillis(1000);

		prompt = "This should be your initial state. Is it?";

		if (!Testsuite::handleInteractiveInput(prompt, "Yes, it is", "Nopes", kOptionLeft)) {
			// User selected incorrect mode
			Testsuite::logDetailedPrintf("switching back to initial state failed\n");
			passed = kTestFailed;
		}

	} else {
		Testsuite::displayMessage("feature not supported");
	}

	return passed;
}

/**
 * Tests the filtering mode by: toggling between filtered and non-filtered modes.
 */
TestExitStatus GFXtests::filteringMode() {
	Testsuite::clearScreen();
	Common::String info = "Filtering test. Here you should expect a toggle between filtered and non-filtered states depending "
	"upon your initial state.";

	Common::Point pt(0, 100);
	Testsuite::writeOnScreen("Testing filtering mode", pt);

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : FilteringMode\n");
		return kTestSkipped;
	}

	bool isFeaturePresent;
	bool isFeatureEnabled;
	TestExitStatus passed = kTestPassed;
	Common::String prompt;
	OptionSelected shouldSelect;

	isFeaturePresent = g_system->hasFeature(OSystem::kFeatureFilteringMode);

	if (isFeaturePresent) {
		// Toggle
		isFeatureEnabled = g_system->getFeatureState(OSystem::kFeatureFilteringMode);
		shouldSelect = isFeatureEnabled ? kOptionLeft : kOptionRight;

		// Do test in fullscreen if possible as filtering may have no effect in windowed mode
		bool fullScreenToggled = false;
		if (g_system->hasFeature(OSystem::kFeatureFullscreenMode) && !g_system->getFeatureState(OSystem::kFeatureFullscreenMode)) {
			fullScreenToggled = true;
			g_system->beginGFXTransaction();
				g_system->setFeatureState(OSystem::kFeatureFullscreenMode, true);
			g_system->endGFXTransaction();
		}

		g_system->delayMillis(1000);

		if (isFeatureEnabled) {
			Testsuite::logDetailedPrintf("Current Mode is Filtered\n");
		} else {
			Testsuite::logDetailedPrintf("Current Mode is Unfiltered\n");
		}

		prompt = " Which mode do you see currently ?  ";

		if (!Testsuite::handleInteractiveInput(prompt, "Filtered", "Unfiltered", shouldSelect)) {
			// User selected incorrect current state
			passed = kTestFailed;
			Testsuite::logDetailedPrintf("g_system->getFeatureState() failed\n");
		}

		g_system->beginGFXTransaction();
			g_system->setFeatureState(OSystem::kFeatureFilteringMode, !isFeatureEnabled);
		g_system->endGFXTransaction();

		// Current state should be now !isFeatureEnabled
		isFeatureEnabled = g_system->getFeatureState(OSystem::kFeatureFilteringMode);
		shouldSelect = isFeatureEnabled ? kOptionLeft : kOptionRight;

		g_system->delayMillis(1000);

		prompt = "  Which mode do you see now ?   ";

		if (!Testsuite::handleInteractiveInput(prompt, "Filtered", "Unfiltered", shouldSelect)) {
			// User selected incorrect mode
			passed = kTestFailed;
			Testsuite::logDetailedPrintf("g_system->setFeatureState() failed\n");
		}

		g_system->beginGFXTransaction();
			g_system->setFeatureState(OSystem::kFeatureFilteringMode, !isFeatureEnabled);
		g_system->endGFXTransaction();

		g_system->delayMillis(1000);

		prompt = "This should be your initial state. Is it?";

		if (!Testsuite::handleInteractiveInput(prompt, "Yes, it is", "Nopes", kOptionLeft)) {
			// User selected incorrect mode
			Testsuite::logDetailedPrintf("switching back to initial state failed\n");
			passed = kTestFailed;
		}

		// Restore fullscreen state
		if (fullScreenToggled) {
			g_system->beginGFXTransaction();
				g_system->setFeatureState(OSystem::kFeatureFullscreenMode, false);
			g_system->endGFXTransaction();
		}

	} else {
		Testsuite::displayMessage("feature not supported");
	}

	return passed;
}

/**
 * Tests the aspect ratio correction by: drawing an ellipse, when corrected the ellipse should render to a circle
 */
TestExitStatus GFXtests::aspectRatio() {

	Testsuite::clearScreen();
	Common::String info = "Aspect Ratio Correction test. If aspect ratio correction is enabled you should expect a circle on screen,"
	" an ellipse otherwise.";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : Aspect Ratio\n");
		return kTestSkipped;
	}
	// Draw an ellipse on the screen
	drawEllipse(80, 160, 72, 60);

	bool isFeaturePresent;
	bool isFeatureEnabled;
	TestExitStatus passed = kTestPassed;
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
			passed = kTestFailed;
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
			passed = kTestFailed;
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
		passed = kTestFailed;
	}

	return passed;
}

/**
 * Tests Palettized cursors.
 * Method: Create a yellow colored cursor, should be able to move it. Once you click test terminates
 */
TestExitStatus GFXtests::palettizedCursors() {

	Testsuite::clearScreen();
	Common::String info = "Palettized Cursors test.\n "
		"Here you should expect to see a yellow mouse cursor rendered with mouse graphics.\n"
		"You would be able to move the cursor. Later we use game graphics to render the cursor.\n"
		"For cursor palette it should be yellow and will be red if rendered by the game palette.\n"
		"The test finishes when mouse (L/R) is clicked.";


	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : Palettized Cursors\n");
		return kTestSkipped;
	}

	TestExitStatus passed = kTestPassed;

	// Testing with cursor Palette
	setupMouseLoop();

	if (Testsuite::handleInteractiveInput("Which color did the cursor appeared to you?", "Yellow", "Any other", kOptionRight)) {
		Testsuite::logDetailedPrintf("Couldn't use cursor palette for rendering cursor\n");
		passed = kTestFailed;
	}

	// Testing with game Palette
	GFXTestSuite::setCustomColor(255, 0, 0);
	setupMouseLoop(true);

	if (Testsuite::handleInteractiveInput("Which color did the cursor appeared to you?", "Red", "Any other", kOptionRight)) {
		Testsuite::logDetailedPrintf("Couldn't use Game palette for rendering cursor\n");
		passed = kTestFailed;
	}

	if (!Testsuite::handleInteractiveInput("     Did test run as was described?     ")) {
		passed = kTestFailed;
	}

	// re-enable cursor palette
	CursorMan.disableCursorPalette(false);
	// Done with cursors, make them invisible, any other test the could simply make it visible
	CursorMan.showMouse(false);
	return passed;
}

/**
 * Tests Alpha cursors.
 * Method: Create a purple colored cursor with alpha transparency, should be able to move it. Once you click test terminates
 */
TestExitStatus GFXtests::alphaCursors() {

	Testsuite::clearScreen();
	Common::String info = "Alpha Cursors test.\n "
		"Here you should expect to see a purple mouse cursor rendered with mouse graphics.\n"
		"You would be able to move the cursor. The cursor should be round, and the background should be visible underneath it.\n"
		"The test finishes when mouse (L/R) is clicked.";


	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : Alpha Cursors\n");
		return kTestSkipped;
	}

	TestExitStatus passed = kTestPassed;
	bool isFeaturePresent = g_system->hasFeature(OSystem::kFeatureCursorAlpha);

	if (isFeaturePresent) {
		const uint32 cursorData[] = {
			0xFF00FF00, 0xFF00FF00, 0xFF00FF00, 0xFF00FF3F, 0xFF00FF00, 0xFF00FF00, 0xFF00FF00,
			0xFF00FF00, 0xFF00FF3F, 0xFF00FF5F, 0xFF00FF7F, 0xFF00FF5F, 0xFF00FF3F, 0xFF00FF00,
			0xFF00FF00, 0xFF00FF5F, 0xFF00FF7F, 0xFF00FF9F, 0xFF00FF7F, 0xFF00FF5F, 0xFF00FF00,
			0xFF00FF3F, 0xFF00FF7F, 0xFF00FF9F, 0xFF00FFBF, 0xFF00FF9F, 0xFF00FF7F, 0xFF00FF3F,
			0xFF00FF00, 0xFF00FF5F, 0xFF00FF7F, 0xFF00FF9F, 0xFF00FF7F, 0xFF00FF5F, 0xFF00FF00,
			0xFF00FF00, 0xFF00FF3F, 0xFF00FF5F, 0xFF00FF7F, 0xFF00FF5F, 0xFF00FF3F, 0xFF00FF00,
			0xFF00FF00, 0xFF00FF00, 0xFF00FF00, 0xFF00FF3F, 0xFF00FF00, 0xFF00FF00, 0xFF00FF00
		};

		Graphics::PixelFormat format(4, 8, 8, 8, 8, 24, 16, 8, 0);
		CursorMan.replaceCursor(cursorData, 7, 7, 3, 3, 0, false, &format);
		CursorMan.showMouse(true);

		bool waitingForClick = true;
		while (waitingForClick) {
			Common::Event event;
			while (g_system->getEventManager()->pollEvent(event)) {
				if (event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_RBUTTONDOWN) {
					waitingForClick = false;
				}
			}

			g_system->delayMillis(10);
			g_system->updateScreen();
		}

		if (Testsuite::handleInteractiveInput("Which color did the cursor appear to you?", "Purple", "Any other", kOptionRight)) {
			Testsuite::logDetailedPrintf("Couldn't use alpha transparency for rendering cursor\n");
			passed = kTestFailed;
		}

		if (Testsuite::handleInteractiveInput("Which shape did the cursor appear to you?", "Round", "Any other", kOptionRight)) {
			Testsuite::logDetailedPrintf("Couldn't use alpha transparency for rendering cursor\n");
			passed = kTestFailed;
		}

		if (!Testsuite::handleInteractiveInput("     Did test run as was described?     ")) {
			passed = kTestFailed;
		}

		// Done with cursors, make them invisible, any other test the could simply make it visible
		CursorMan.showMouse(false);
	} else {
		Testsuite::displayMessage("feature not supported");
	}

	return passed;
}

/**
 * Tests Masked cursors.
 * Method: Create a cursor with a transparent, mask, inverted, and color-inverted areas, it should behave properly over colored areas. Once you click test terminates
 */
TestExitStatus GFXtests::maskedCursors() {

	Testsuite::clearScreen();
	Common::String info = "Masked cursors test.  If masked cursors are enabled, you should see a cursor with 4 sections:\n"
						  "T for transparent, O for opaque, I for inverted, C for colorized inverted.\n"
						  "If the I or C letters are absent, then that type of masking is unsupported";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : Masked Cursors\n");
		return kTestSkipped;
	}

	TestExitStatus passed = kTestPassed;
	bool isFeaturePresent = g_system->hasFeature(OSystem::kFeatureCursorMask);
	bool haveCursorPalettes = g_system->hasFeature(OSystem::kFeatureCursorPalette);
	bool haveCursorAlpha = g_system->hasFeature(OSystem::kFeatureCursorAlpha);

	g_system->delayMillis(1000);

	if (isFeaturePresent) {
		const byte cursorLeftColData[] = {
			1, 1, 1,
			0, 1, 0,
			0, 1, 0,
			0, 0, 0,

			0, 1, 0,
			1, 0, 1,
			0, 1, 0,
			0, 0, 0,

			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 0, 0,

			0, 1, 1,
			1, 0, 0,
			0, 1, 1,
			0, 0, 0,
		};

		byte cursorData[16 * 16];
		byte maskData[16 * 16];

		for (int y = 0; y < 16; y++) {
			for (int x = 0; x < 16; x++) {
				// Fill cursor and mask with white transparent
				cursorData[y * 16 + x] = 3;
				maskData[y * 16 + x] = kCursorMaskTransparent;
			}
		}

		for (int y = 12; y < 16; y++) {
			for (int x = 5; x < 16; x++) {
				// Fill color mask part with red
				cursorData[y * 16 + x] = 2;
			}
		}

		// Fill T O I C graphic
		for (int y = 0; y < 16; y++)
			for (int x = 0; x < 3; x++)
				if (cursorLeftColData[y * 3 + x])
					maskData[y * 16 + x + 1] = kCursorMaskOpaque;

		// Fill middle column
		for (int y = 0; y < 16; y++) {
			for (int x = 0; x < 5; x++) {
				maskData[y * 16 + x + 5] = kCursorMaskOpaque;
			}
		}

		bool haveInverted = g_system->hasFeature(OSystem::kFeatureCursorMaskInvert);
		bool haveColorXorBlend = g_system->hasFeature(OSystem::kFeatureCursorMaskPaletteXorColorXnor);

		// Fill middle column
		for (int y = 0; y < 16; y++) {
			for (int x = 0; x < 4; x++) {
				maskData[y * 16 + x + 5] = kCursorMaskOpaque;
			}
		}

		for (int x = 0; x < 5; x++) {
			for (int y = 0; y < 4; y++) {
				maskData[(y + 0) * 16 + x + 11] = kCursorMaskTransparent;
				maskData[(y + 4) * 16 + x + 11] = kCursorMaskOpaque;
				maskData[(y + 8) * 16 + x + 11] = kCursorMaskInvert;
				maskData[(y + 12) * 16 + x + 11] = kCursorMaskPaletteXorColorXnor;
			}
		}

		// Mask out unsupported types
		if (!haveInverted) {
			for (int y = 8; y < 12; y++)
				for (int x = 0; x < 16; x++)
					maskData[y * 16 + x] = kCursorMaskTransparent;
		}

		if (!haveColorXorBlend) {
			for (int y = 12; y < 16; y++)
				for (int x = 0; x < 16; x++)
					maskData[y * 16 + x] = kCursorMaskTransparent;
		}

		byte oldPalette[256 * 3];
		g_system->getPaletteManager()->grabPalette(oldPalette, 0, 256);

		byte newPalette[] = {0, 0, 0, 255, 255, 255, 255, 0, 0, 255, 255, 255};

		g_system->getPaletteManager()->setPalette(newPalette, 0, 4);

		if (haveCursorPalettes)
			g_system->setCursorPalette(newPalette, 0, 4);

		CursorMan.replaceCursor(cursorData, 16, 16, 1, 1, 0, false, nullptr, maskData);
		CursorMan.showMouse(true);

		bool waitingForClick = true;
		while (waitingForClick) {
			Common::Event event;
			while (g_system->getEventManager()->pollEvent(event)) {
				if (event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_RBUTTONDOWN) {
					waitingForClick = false;
				}
			}

			g_system->delayMillis(10);
			g_system->updateScreen();
		}

		g_system->getPaletteManager()->setPalette(oldPalette, 0, 256);

		if (!Testsuite::handleInteractiveInput("Was the part of the cursor to the right of the 'T' transparent?", "Yes", "No", kOptionLeft)) {
			return kTestFailed;
		}

		if (!Testsuite::handleInteractiveInput("Was the part of the cursor to the right of the 'O' opaque?", "Yes", "No", kOptionLeft)) {
			return kTestFailed;
		}

		if (haveInverted) {
			if (!Testsuite::handleInteractiveInput("Was the part of the cursor to the right of the 'I' inverted?", "Yes", "No", kOptionLeft)) {
				return kTestFailed;
			}
		}

		if (haveColorXorBlend) {
			if (!Testsuite::handleInteractiveInput("Was the part of the cursor to the right of the 'C' inverted according to the color to the left of it?", "Yes", "No", kOptionLeft)) {
				return kTestFailed;
			}
		}

		Common::String rgbInfo = "Try again with a cursor using RGB data?";

		if (!Testsuite::handleInteractiveInput(rgbInfo, "OK", "Skip", kOptionRight)) {
			if (haveCursorAlpha) {
				g_system->delayMillis(500);

				Graphics::PixelFormat rgbaFormat = Graphics::createPixelFormat<8888>();

				uint32 rgbaCursorData[16 * 16];
				for (uint i = 0; i < 16 * 16; i++) {
					byte colorByte = cursorData[i];
					byte r = newPalette[colorByte * 3 + 0];
					byte g = newPalette[colorByte * 3 + 1];
					byte b = newPalette[colorByte * 3 + 2];

					rgbaCursorData[i] = rgbaFormat.ARGBToColor(255, r, g, b);
				}

				CursorMan.replaceCursor(rgbaCursorData, 16, 16, 1, 1, 0, false, &rgbaFormat, maskData);

				waitingForClick = true;
				while (waitingForClick) {
					Common::Event event;
					while (g_system->getEventManager()->pollEvent(event)) {
						if (event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_RBUTTONDOWN) {
							waitingForClick = false;
						}
					}

					g_system->delayMillis(10);
					g_system->updateScreen();
				}

				if (!Testsuite::handleInteractiveInput("Was the part of the cursor to the right of the 'T' transparent?", "Yes", "No", kOptionLeft)) {
					return kTestFailed;
				}

				if (!Testsuite::handleInteractiveInput("Was the part of the cursor to the right of the 'O' opaque?", "Yes", "No", kOptionLeft)) {
					return kTestFailed;
				}

				if (haveInverted) {
					if (!Testsuite::handleInteractiveInput("Was the part of the cursor to the right of the 'I' inverted?", "Yes", "No", kOptionLeft)) {
						return kTestFailed;
					}
				}

				if (haveColorXorBlend) {
					if (!Testsuite::handleInteractiveInput("Was the part of the cursor to the right of the 'C' inverted according to the color to the left of it?", "Yes", "No", kOptionLeft)) {
						return kTestFailed;
					}
				}
			} else {
				Testsuite::displayMessage("feature not supported");
			}
		}

		CursorMan.showMouse(false);
	} else {
		Testsuite::displayMessage("feature not supported");
	}

	g_system->delayMillis(500);

	return passed;
}

/**
 * Tests automated mouse movements. "Warp" functionality provided by the backend.
 */

TestExitStatus GFXtests::mouseMovements() {
	Testsuite::clearScreen();
	// Ensure that the cursor is visible
	CursorMan.showMouse(true);

	Common::String info = "Testing Automated Mouse movements.\n"
						"You should expect cursor hotspot(top-left corner) to automatically move from (0, 0) to (100, 100).\n"
						"There we have a rectangle drawn, finally the cursor would lie centered in that rectangle.";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : Mouse Movements\n");
		return kTestSkipped;
	}

	// Draw Rectangle
	// Ensure that 2 represents red in current palette
	GFXTestSuite::setCustomColor(255, 0, 0);
	g_system->fillScreen(Common::Rect::center(106, 106, 14, 14), 2);

	// Testing Mouse Movements now!
	Common::Point pt(0, 10);
	Testsuite::writeOnScreen("Moving mouse hotspot automatically from (0, 0) to (100, 100)", pt);
	g_system->warpMouse(0, 0);
	g_system->updateScreen();
	g_system->delayMillis(1000);

	Common::Event event;

	for (int i = 0; i <= 100; i++) {
		g_system->delayMillis(20);
		g_system->warpMouse(i, i);
		g_system->getEventManager()->pollEvent(event);
		g_system->updateScreen();
	}

	Testsuite::writeOnScreen("Mouse hotspot Moved to (100, 100)", pt);
	g_system->delayMillis(1500);
	CursorMan.showMouse(false);

	if (Testsuite::handleInteractiveInput("Was the cursor centered in the rectangle at (100, 100)?", "Yes", "No", kOptionRight)) {
		return kTestFailed;
	}

	return kTestPassed;
}



/**
 * This basically blits the screen by the contents of its buffer.
 *
 */
TestExitStatus GFXtests::copyRectToScreen() {

	Testsuite::clearScreen();
	Common::String info = "Testing Blitting a Bitmap to screen.\n"
		"You should expect to see a 20x40 yellow horizontal rectangle centered at the screen.";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : Blitting Bitmap\n");
		return kTestSkipped;
	}

	GFXTestSuite::setCustomColor(255, 255, 0);
	byte buffer[20 * 40];
	memset(buffer, 2, 20 * 40);

	uint x = g_system->getWidth() / 2 - 20;
	uint y = g_system->getHeight() / 2 - 10;

	g_system->copyRectToScreen(buffer, 40, x, y, 40, 20);

	x = 10;
	y = 10;

	Graphics::Surface *screen = g_system->lockScreen();

	const char *text = "d";
	const Graphics::BdfFont *origFont = (const Graphics::BdfFont *)FontMan.getFontByName("helvB12.bdf");

	for (int i = origFont->getFontHeight(); i <= 20; i++) {

		//const Graphics::BdfFont *font = Graphics::BdfFont::scaleFont(origFont, i);
		const Graphics::BdfFont *font = origFont;
		int width = font->getStringWidth(text);

		Common::Rect bbox = font->getBoundingBox(text, x, y, g_system->getWidth());
		screen->frameRect(bbox, 15);

		font->drawString(screen, text, x, y, width, kColorCustom);

		x += width + 1;
	}

	g_system->unlockScreen();

	g_system->updateScreen();
	g_system->delayMillis(1000);

	if (Testsuite::handleInteractiveInput("      Did you see yellow rectangle ?       ", "Yes", "No", kOptionRight)) {
		return kTestFailed;
	}

	return kTestPassed;
}

/**
 * Testing feature : Iconifying window
 * It is expected the screen minimizes when this feature is enabled
 */
TestExitStatus GFXtests::iconifyWindow() {

	Testsuite::clearScreen();
	Common::String info = "Testing Iconify Window mode.\n If the feature is supported by the backend, "
		"you should expect the window to be minimized.\n However you would manually need to de-iconify.";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : Iconifying window\n");
		return kTestSkipped;
	}

	Common::Point pt(0, 100);
	Testsuite::writeOnScreen("Testing Iconifying window", pt);

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

	if (Testsuite::handleInteractiveInput("  Did you see the window minimized?  ", "Yes", "No", kOptionRight)) {
		return kTestFailed;
	}

	return kTestPassed;
}

/**
 * Testing feature: Scaled cursors
 */
TestExitStatus GFXtests::scaledCursors() {

	Testsuite::clearScreen();
	Common::String info = "Testing : Scaled cursors\n"
		"Here every graphics mode is tried with a cursorTargetScale of 1, 2 and 3.\n"
		"The expected cursor size is drawn as a rectangle.\n The cursor should approximately match that rectangle.\n"
		"This may take time, You may skip the later scalers and just examine the first three i.e 1x, 2x and 3x";

	bool isAspectRatioCorrected = g_system->getFeatureState(OSystem::kFeatureAspectRatioCorrection);

	if (isAspectRatioCorrected) {
		info += "\nDisabling Aspect ratio correction, for letting cusors match exactly, will be restored after this test.";
	}

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : Scaled Cursors\n");
		return kTestSkipped;
	}

	int maxLimit = 1000;
	if (!Testsuite::handleInteractiveInput("Do You want to restrict scalers to 1x, 2x and 3x only?", "Yes", "No", kOptionRight)) {
		maxLimit = 3;
	}


	if (isAspectRatioCorrected) {
		g_system->beginGFXTransaction();
			g_system->setFeatureState(OSystem::kFeatureAspectRatioCorrection, false);
		g_system->endGFXTransaction();
	}

	const int currGFXMode = g_system->getGraphicsMode();
	const OSystem::GraphicsMode *gfxMode = g_system->getSupportedGraphicsModes();

	while (gfxMode->name && maxLimit > 0) {
		// for every graphics mode display cursors for cursorTargetScale 1, 2 and 3
		// Switch Graphics mode
		// FIXME: Crashes with "3x" mode now.:

		info = Common::String::format("Testing : Scaled cursors with GFX Mode %s\n", gfxMode->name);
		if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
			Testsuite::logPrintf("\tInfo! Skipping sub-test : Scaled Cursors :: GFX Mode %s\n", gfxMode->name);
			gfxMode++;
			maxLimit--;
			continue;
		}
		if (Engine::shouldQuit()) {
			// Explicit exit requested
			Testsuite::logPrintf("Info! Explicit exit requested during scaling test, this test may be incomplete\n");
			return kTestSkipped;
		}

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
			return kTestFailed;
		}
		gfxMode++;
		maxLimit--;

		info = "Did the expected cursor size and the actual cursor size matched?";
		if (Testsuite::handleInteractiveInput(info, "Yes", "No", kOptionRight)) {
			Testsuite::logPrintf("\tInfo! Failed sub-test : Scaled Cursors :: GFX Mode %s\n", gfxMode->name);
		}

		if (Engine::shouldQuit()) {
			// Explicit exit requested
			Testsuite::logPrintf("Info! Explicit exit requested during scaling test, this test may be incomplete\n");
			return kTestSkipped;
		}

	}

	// Restore Original State
	g_system->beginGFXTransaction();

		bool isGFXModeSet = g_system->setGraphicsMode(currGFXMode);
		g_system->initSize(320, 200);

		if (isAspectRatioCorrected) {
			g_system->setFeatureState(OSystem::kFeatureAspectRatioCorrection, true);
		}

	OSystem::TransactionError gfxError = g_system->endGFXTransaction();

	if (gfxError != OSystem::kTransactionSuccess || !isGFXModeSet) {
		Testsuite::logDetailedPrintf("Switcing to initial state failed\n");
		return kTestFailed;
	}

	// Done with cursors, Make them invisible, any other test may enable and use it.
	CursorMan.showMouse(false);
	return kTestPassed;
}

TestExitStatus GFXtests::shakingEffect() {

	Testsuite::clearScreen();
	Common::String info = "Shaking test. You should expect the graphics(text/bars etc) drawn on the screen to shake!";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : Shaking Effect\n");
		return kTestSkipped;
	}

	// test vertical, horizontal, and diagonal
	Common::Point pt(0, 100);
	for (int i = 0; i < 3; ++i) {
		Common::String direction;
		int shakeXOffset;
		int shakeYOffset;
		switch (i) {
		case 0:
			direction = "vertical";
			shakeXOffset = 0;
			shakeYOffset = 10;
			break;
		case 1:
			direction = "horizontal";
			shakeXOffset = 10;
			shakeYOffset = 0;
			break;
		default:
			direction = "diagonal";
			shakeXOffset = 10;
			shakeYOffset = 10;
			break;
		}

		Testsuite::writeOnScreen(Common::String::format("If Shaking Effect works, this should shake %s", direction.c_str()), pt);
		int times = 15;
		while (times--) {
			g_system->setShakePos(shakeXOffset, shakeYOffset);
			g_system->delayMillis(50);
			g_system->updateScreen();
			g_system->setShakePos(0, 0);
			g_system->delayMillis(50);
			g_system->updateScreen();
			shakeXOffset = -shakeXOffset;
			shakeYOffset = -shakeYOffset;
		}
		g_system->delayMillis(1500);

		if (Testsuite::handleInteractiveInput("Did the Shaking test work as you were expecting?", "Yes", "No", kOptionRight)) {
			Testsuite::logDetailedPrintf("Shaking Effect didn't work");
			return kTestFailed;
		}
	}

	return kTestPassed;
}

TestExitStatus GFXtests::focusRectangle() {

	Testsuite::clearScreen();
	Common::String info = "Testing : Setting and hiding Focus \n"
		"If this feature is implemented, the focus should be toggled between the two rectangles on the corners";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : focus Rectangle\n");
		return kTestSkipped;
	}

	const Graphics::Font &font(*FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont));

	int screenHeight = g_system->getHeight();
	int screenWidth = g_system->getWidth();

	int height = font.getFontHeight();
	int width = screenWidth / 2;

	Common::Rect rectLeft(0, 0, width, height * 2);
	g_system->fillScreen(rectLeft, kColorWhite);

	Common::Rect rectRight(screenWidth - width, screenHeight - height * 2 , screenWidth, screenHeight);
	g_system->fillScreen(rectRight, kColorWhite);

	Graphics::Surface *screen = g_system->lockScreen();
	font.drawString(screen, "Focus 1", rectLeft.left, rectLeft.top, width, kColorBlack, Graphics::kTextAlignLeft);
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

	return kTestPassed;
}

TestExitStatus GFXtests::overlayGraphics() {
	Testsuite::clearScreen();
	Common::String info = "Overlay Graphics. You should expect to see a green colored rectangle on the screen";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : Overlay Graphics\n");
		return kTestSkipped;
	}

	int16 w = g_system->getOverlayWidth();
	int16 h = g_system->getOverlayHeight();
	Graphics::PixelFormat pf = g_system->getOverlayFormat();

	byte *buffer = new byte[50 * 100 * pf.bytesPerPixel];
	const uint32 value = pf.RGBToColor(0, 255, 0);

	if (pf.bytesPerPixel == 2) {
		uint16 *dst = (uint16 *)buffer;
		for (int i = 50 * 100; i > 0; --i) {
			*dst++ = value;
		}
	} else if (pf.bytesPerPixel == 4) {
		uint32 *dst = (uint32 *)buffer;
		for (int i = 50 * 100; i > 0; --i) {
			*dst++ = value;
		}
	} else {
		error("GFXtests::overlayGraphics: Unsupported color depth: %d", pf.bytesPerPixel);
	}

	g_system->showOverlay();
	g_system->copyRectToOverlay(buffer, 100 * pf.bytesPerPixel, (w - 100) / 2, (h - 50) / 2, 100, 50);
	g_system->updateScreen();

	delete[] buffer;

	g_system->delayMillis(1000);

	g_system->hideOverlay();
	g_system->updateScreen();

	if (Testsuite::handleInteractiveInput("Did you see a green overlayed rectangle?", "Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Overlay Rectangle feature doesn't works\n");
		return kTestFailed;
	}

	return kTestPassed;
}

TestExitStatus GFXtests::paletteRotation() {

	Common::String info = "Palette rotation. Here we draw a full 256 colored rainbow and then rotate it.\n"
						"Note that the screen graphics change without having to draw anything.\n"
						"The palette should appear to rotate, as a result, the background will change its color too.\n"
						"Click the mouse button to exit.";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : palette Rotation\n");
		return kTestSkipped;
	}

	Testsuite::clearEntireScreen();

	// Use 256 colors
	byte palette[256 * 3] = {0};

	int r, g, b;
	r = g = b = 0;
	int colIndx;

	for (int i = 0; i < 256; i++) {
		HSVtoRGB(r, g, b, i, 1, 1);
		colIndx = i * 3;
		palette[colIndx] = r;
		palette[colIndx + 1] = g;
		palette[colIndx + 2] = b;
	}

	// Initialize this palette.
	g_system->getPaletteManager()->setPalette(palette, 0, 256);

	// Draw 256 Rectangles, each 1 pixel wide and 10 pixels long
	// one for 0-255 color range other for 0-127-255 range
	byte buffer[256 * 30] = {0};

	for (int i = 0; i < 30; i++) {
		for (int j = 0; j < 256; j++) {
			if (i < 10) {
				buffer[i * 256 + j] = j + 2;
			} else if (i < 20) {
				buffer[i * 256 + j] = 0;
			} else {
				buffer[i * 256 + j] = ((j + 127) % 256) + 2;
			}
		}
	}

	g_system->copyRectToScreen(buffer, 256, 22, 50, 256, 30);

	// Show mouse
	CursorMan.showMouse(true);
	g_system->updateScreen();


	bool toRotate = true;
	Common::Event event;

	while (toRotate) {
		while (g_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_RBUTTONDOWN) {
				toRotate = false;
			}
		}

		rotatePalette(palette, 256);

		g_system->delayMillis(10);
		g_system->getPaletteManager()->setPalette(palette, 0, 256);
		g_system->updateScreen();
	}

	CursorMan.showMouse(false);
	// Reset initial palettes
	GFXTestSuite::setCustomColor(255, 0, 0);
	Testsuite::clearScreen();

	if (Testsuite::handleInteractiveInput("Did you see a rotation in colors of rectangles displayed on screen?", "Yes", "No", kOptionRight)) {
		return kTestFailed;
	}

	return kTestPassed;
}

TestExitStatus GFXtests::cursorTrails() {
	Common::String info = "With some shake offset the cursor was known to leave trails in the GUI\n"
						"Here we set some offset and ask user to check for mouse trails, \n"
						"the test is passed when there are no trails";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : Cursor Trails\n");
		return kTestSkipped;
	}
	TestExitStatus passed = kTestFailed;
	g_system->setShakePos(25, 25);
	g_system->updateScreen();
	if (Testsuite::handleInteractiveInput("Does the cursor leaves trails while moving?", "Yes", "No", kOptionRight)) {
		passed = kTestPassed;
	}
	g_system->setShakePos(0, 0);
	g_system->updateScreen();
	return passed;
}

TestExitStatus GFXtests::pixelFormatsSupported() {
	Testsuite::clearScreen();
	Common::String info = "Testing pixel formats. Here we iterate over all the supported pixel formats and display some colors using them\n"
		"This may take long, especially if the backend supports many pixel formats";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : Supported Pixel Formats\n");
		return kTestSkipped;
	}

	Common::List<Graphics::PixelFormat> list = g_system->getSupportedFormats();
	return GFXtests::pixelFormats(list);
}

TestExitStatus GFXtests::pixelFormatsRequired() {
	Testsuite::clearScreen();
	Common::String info = "Testing pixel formats. Here we iterate over some pixel formats directly required by some engines and display some colors using them\n"
		"This may fail, especially if the backend does not support many pixel formats";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : Required Pixel Formats\n");
		return kTestSkipped;
	}

	Common::List<Graphics::PixelFormat> list;
	list.push_back(Graphics::PixelFormat(2, 5, 6, 5, 0, 11,  5,  0,  0)); // BBDoU, Frotz, HDB, Hopkins, Nuvie, Petka, Riven, Sherlock (3DO), Titanic, Tony, Ultima 4, Ultima 8, ZVision
	list.push_back(Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16,  8,  0)); // Full Pipe, Gnap (little endian), Griffon, Groovie 2, SCI32 (HQ videos), Sludge, Sword25, Ultima 8, Wintermute
	list.push_back(Graphics::PixelFormat(4, 8, 8, 8, 8,  0,  8, 16, 24)); // Gnap (big endian)
	list.push_back(Graphics::PixelFormat(2, 5, 5, 5, 0, 10,  5,  0,  0)); // SCUMM HE99+, Last Express
	list.push_back(Graphics::PixelFormat(2, 5, 5, 5, 1, 10,  5,  0, 15)); // Dragons
	// list.push_back(Graphics::PixelFormat::createFormatCLUT8());

	return GFXtests::pixelFormats(list);
}

struct PixelFormatComparator {
	bool operator()(const Graphics::PixelFormat &l, const Graphics::PixelFormat &r) {
		return l.aLoss != r.aLoss ? l.aLoss < r.aLoss:
		       l.rLoss != r.rLoss ? l.rLoss < r.rLoss:
		       l.gLoss != r.gLoss ? l.gLoss < r.gLoss:
		       l.bLoss != r.bLoss ? l.bLoss < r.bLoss:
		                            l.toString() < r.toString();
	}
};

TestExitStatus GFXtests::pixelFormats(Common::List<Graphics::PixelFormat> &pfList) {
	int numFormatsTested = 0;
	int numPassed = 0;
	int numFailed = 0;

	Common::sort(pfList.begin(), pfList.end(), PixelFormatComparator());
	Testsuite::logDetailedPrintf("Testing Pixel Formats. Size of list : %d\n", pfList.size());

	bool seenTutorials[9] = {};

	for (Common::List<Graphics::PixelFormat>::const_iterator iter = pfList.begin(); iter != pfList.end(); iter++) {
		numFormatsTested++;

		Testsuite::logPrintf("Info! Testing Pixel Format: %s, %d of %d\n", iter->toString().c_str(), numFormatsTested, pfList.size());
		if (iter->bytesPerPixel == 1) {
			// Palettes already tested
			continue;
		} else if (iter->bytesPerPixel != 2 && iter->bytesPerPixel != 4) {
			Testsuite::logDetailedPrintf("bytesPerPixel must be 1, 2, or 4\n");
			continue;
		}

		if (!seenTutorials[iter->aLoss]) {
			showPixelFormat(Graphics::PixelFormat::createFormatCLUT8(), iter->aLoss);

			Common::Point pt(0, 170);
			Testsuite::writeOnScreen("Example displayed with Pixel Format CLUT8", pt, false);

			Common::String tutorial;
			tutorial = Common::String::format("Testing a group of Pixel Formats with %d-bit alpha channel.\nPlease, memorize the pattern displayed in the frame above.", 8 - iter->aLoss);
			if (iter->aLoss < 7) {
				tutorial += "\nIt should contain horizontal and vertical gradients for several different colors.";
			} else if (iter->aLoss == 7) {
				tutorial += "\nTop half of the frame should be empty, containing only a cross.";
				tutorial += "\nBottom half of the frame should contain *only horizontal* gradients for several different colors.";
			} else {
				tutorial += "\nIt should contain *only horizontal* gradients for several different colors.";
			}
			tutorial += "\nWe are going to display the same pattern in other Pixel Formats.";

			Testsuite::displayMessage(tutorial);
			seenTutorials[iter->aLoss] = true;
		}

		// Draw some nice gradients
		showPixelFormat(*iter, iter->aLoss);

		Common::Point pt(0, 170);
		Common::String msg;
		msg = Common::String::format("Testing Pixel Format %s, %d of %d", iter->toString().c_str(), numFormatsTested, pfList.size());
		Testsuite::writeOnScreen(msg, pt, true);

		g_system->delayMillis(500);

		if (Testsuite::handleInteractiveInput("Were you able to notice the colored gradients inside a white frame on the screen for this format?\nDid they match the pattern that was displayed before?", "Yes", "No", kOptionLeft)) {
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
		return kTestFailed;
	}

	return kTestPassed;
}

void GFXtests::showPixelFormat(const Graphics::PixelFormat &pf, uint aLoss) {

	// Those constants can be configured

	// Grid position and sizes
	const uint xOffset = 3;
	const uint yOffset = 10;
	const uint xStep = 5;
	const uint yStep = 5;

	// Base colors to modify     [R][Y][G][C][B][M][W]
	const uint nColors = 7;
	const uint colorR[nColors] = {1, 1, 0, 0, 0, 1, 1};
	const uint colorG[nColors] = {0, 1, 1, 1, 0, 0, 1};
	const uint colorB[nColors] = {0, 0, 0, 1, 1, 1, 1};

	// Number of levels in gradient
	// It is applied both to alpha levels and brightness levels,
	// e.g. for nLevels = 3 and red color we will test those #RRGGBBAA colors:
	//   #00000000 #80000000 #FF000000
	//   #00000080 #80000080 #FF000080
	//   #000000FF #800000FF #FF0000FF
	const uint nLevels = 9;

	// UI palette
	const uint nStdColors = 2;
	byte stdPalette[nStdColors * 3] = {0, 0, 0, 255, 255, 0};


	// Those constants are calculated

	const uint nTones = nLevels * (nLevels - 1) / 2;
	const uint paletteSize = nStdColors + nColors * nTones;
	STATIC_ASSERT(paletteSize < 256, cant_fit_all_the_tones_into_CLUT8_palettes);

	uint level[nLevels];
	for (uint i = 0; i < nLevels - 1; i++) {
		level[i] = i * 256 / (nLevels - 1);
	}
	level[nLevels - 1] = 255;


	// Init screen and working with dstSurface

	g_system->beginGFXTransaction();
		g_system->initSize(320, 200, &pf);
	OSystem::TransactionError gfxError = g_system->endGFXTransaction();
	if (gfxError) {
		Testsuite::logPrintf("WARNING! Pixel Format %s is unsupported\n", pf.toString().c_str());
		return;
	}
	Testsuite::clearScreen(true);

	Graphics::Surface *screen = g_system->lockScreen();
	Graphics::ManagedSurface dstSurface(screen->w, screen->h, screen->format);
	dstSurface.blitFrom(*screen);
	g_system->unlockScreen();


	// Init palette, if we are demonstating a CLUT8 preview
	// There are nTones different combinations of alpha and brightness levels for each color

	if (pf.bytesPerPixel == 1) {
		byte palette[paletteSize * 3] = {0};
		memcpy(palette, stdPalette, nStdColors * 3);

		level[nLevels - 1] = 256;
		for (uint c = 0; c < nColors; c++) {
			uint idx = 3 * (nStdColors + c * nTones);
			for (uint alpha = 1; alpha < nLevels; alpha++) {
				for (uint brightness = alpha; brightness < nLevels; brightness++) {
					uint value = level[alpha] * level[brightness] / 256;
					if (value == 256) {
					    value = 255;
					}

					palette[idx++] = colorR[c] * value;
					palette[idx++] = colorG[c] * value;
					palette[idx++] = colorB[c] * value;
				}
			}
		}
		level[nLevels - 1] = 255;

		g_system->getPaletteManager()->setPalette(palette, 0, paletteSize);
	}


	// Display the color gradients

	for (uint c = 0; c < nColors; c++) {
		for (uint alpha = 0; alpha < nLevels; alpha++) {
			for (uint brightness = 0; brightness < nLevels; brightness++) {
				uint x = xOffset + (nLevels * c + brightness) * xStep;
				uint y = yOffset + alpha * yStep;

				if (pf.bytesPerPixel != 1) {
					uint a = level[alpha];
					uint r = colorR[c] * level[brightness];
					uint g = colorG[c] * level[brightness];
					uint b = colorB[c] * level[brightness];
					uint color = pf.ARGBToColor(a, r, g, b);

					// blit transparent surface with given color
					// this cannot be done with drawing methods, since they ignore alpha
					Graphics::ManagedSurface tmp(xStep, yStep, pf);
					tmp.clear(color);
					dstSurface.blitFrom(tmp, Common::Point(x, y));
				} else {
					int a = 0;
					if (aLoss == 8 && brightness) {
						a = nLevels - 1;
					} else if (aLoss == 7 && brightness && level[alpha] >= 128) {
						a = nLevels - 1;
					} else if (aLoss < 7 && brightness && alpha) {
						a = alpha;
					}
					int b = brightness;

					// draw colored rect with pre-calculated tone similar to the color after blit
					if (a) {
						uint tone = (2 * nLevels - MIN(a, b)) * (MIN(a, b) - 1) / 2 + ABS(a - b);
						uint color = nStdColors + c * nTones + tone;
						for (uint dy = 0; dy < yStep; dy++) {
							dstSurface.hLine(x, y + dy, x + xStep - 1, color);
						}
					}
				}
			}
		}
	}


	// Display a frame around the gradients

	{
		const uint white = pf.bytesPerPixel == 1 ? 1 : pf.RGBToColor(255, 255, 255);

		uint x1 = xOffset - 2;
		uint y1 = yOffset - 2;
		uint x2 = xOffset + 2 + nLevels * xStep * nColors;
		uint y2 = yOffset + 2 + nLevels * yStep;
		dstSurface.frameRect(Common::Rect(x1, y1, x2, y2), white);

		// cross out the empty area for 1-bit alpha patterns

		if (aLoss == 7) {
			uint dy = yStep * int(nLevels / 2);
			y2 = yOffset + dy - 2;
			dstSurface.drawLine(x1, y2, x2 - 1, y2, white);

			x1 = (x1 + x2 - dy) / 2;
			x2 = x1 + dy;
			dstSurface.drawLine(x1, y1, x2, y2, white);
			dstSurface.drawLine(x1, y2, x2, y1, white);
		}
	}


	// End working with dstSurface

	g_system->copyRectToScreen(dstSurface.getPixels(), dstSurface.pitch, 0, 0,
	                           dstSurface.w, dstSurface.h);
	g_system->updateScreen();
}

} // End of namespace Testbed
