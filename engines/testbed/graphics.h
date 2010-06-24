#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "testbed/testsuite.h"

namespace Testbed {

namespace GFXtests {

// Helper functions for GFX tests
void drawEllipse(int x, int y, int a, int b);
void setupMouseLoop(bool disableCursorPalette = false, const char *gfxModeName = "", int cursorTargetScale = 1);
void initMousePalette();
void mouseMovements();
void drawCursor(bool cursorPaletteDisabled = false, const char *gfxModeName = "", int cursorTargetScale = 1);

// will contain function declarations for GFX tests
bool fullScreenMode(); 
bool aspectRatio();
bool palettizedCursors();
bool copyRectToScreen();
bool iconifyWindow();
bool scaledCursors();
bool shakingEffect();
bool focusRectangle();
bool overlayGraphics();
bool paletteRotation();
bool pixelFormats();
// add more here
}

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
	const char *getName() const;
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
