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
 */

#include "common/util.h"
#include "common/system.h"
#include "common/timer.h"
#include "graphics/surface.h"
#include "engines/util.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/view.h"

namespace Sci {

GfxScreen::GfxScreen(ResourceManager *resMan) : _resMan(resMan) {

	// Scale the screen, if needed
	_upscaledHires = GFX_SCREEN_UPSCALED_DISABLED;
	
	// we default to scripts running at 320x200
	_scriptWidth = 320;
	_scriptHeight = 200;
	_width = 0;
	_height = 0;
	_displayWidth = 0;
	_displayHeight = 0;
	
	// King's Quest 6 and Gabriel Knight 1 have hires content, gk1/cd was able
	// to provide that under DOS as well, but as gk1/floppy does support
	// upscaled hires scriptswise, but doesn't actually have the hires content
	// we need to limit it to platform windows.
	if (g_sci->getPlatform() == Common::kPlatformWindows) {
		if (g_sci->getGameId() == GID_KQ6)
			_upscaledHires = GFX_SCREEN_UPSCALED_640x440;
#ifdef ENABLE_SCI32
		if (g_sci->getGameId() == GID_GK1)
			_upscaledHires = GFX_SCREEN_UPSCALED_640x480;
#endif
	}

	// Japanese versions of games use hi-res font on upscaled version of the game.
	if ((g_sci->getLanguage() == Common::JA_JPN) && (getSciVersion() <= SCI_VERSION_1_1))
		_upscaledHires = GFX_SCREEN_UPSCALED_640x400;
	
	// Macintosh SCI0 games used 480x300, while the scripts were running at 320x200
	if (g_sci->getPlatform() == Common::kPlatformMacintosh) {
		if (getSciVersion() <= SCI_VERSION_01) {
			_upscaledHires = GFX_SCREEN_UPSCALED_480x300;
			_width = 480;
			_height = 300; // regular visual, priority and control map are 480x300 (this is different than other upscaled SCI games)
		}
	
		// Some Mac SCI1/1.1 games only take up 190 rows and do not
		// have the menu bar.
		// TODO: Verify that LSL1 and LSL5 use height 190
		switch (g_sci->getGameId()) {
		case GID_FREDDYPHARKAS:
		case GID_KQ5:
		case GID_KQ6:
		case GID_LSL1:
		case GID_LSL5:
		case GID_SQ1:
			_scriptHeight = 190;
			break;
		default:
			break;
		}
	}

#ifdef ENABLE_SCI32
	// GK1 Mac uses a 640x480 resolution too
	if (g_sci->getPlatform() == Common::kPlatformMacintosh) {
		if (g_sci->getGameId() == GID_GK1)
			_upscaledHires = GFX_SCREEN_UPSCALED_640x480;
	}
#endif

	if (_resMan->detectHires()) {
		_scriptWidth = 640;
		_scriptHeight = 480;
	}

#ifdef ENABLE_SCI32
	// Phantasmagoria 1 effectively outputs 630x450
	//  Coordinate translation has to use this resolution as well
	if (g_sci->getGameId() == GID_PHANTASMAGORIA) {
		_width = 630;
		_height = 450;
	}
#endif

	// if not yet set, set those to script-width/height
	if (!_width)
		_width = _scriptWidth;
	if (!_height)
		_height = _scriptHeight;

	_pixels = _width * _height;

	switch (_upscaledHires) {
	case GFX_SCREEN_UPSCALED_480x300:
		// Space Quest 3, Hoyle 1+2 on MAC use this one
		_displayWidth = 480;
		_displayHeight = 300;
		for (int i = 0; i <= _scriptHeight; i++)
			_upscaledHeightMapping[i] = (i * 3) >> 1;
		for (int i = 0; i <= _scriptWidth; i++)
			_upscaledWidthMapping[i] = (i * 3) >> 1;
		break;
	case GFX_SCREEN_UPSCALED_640x400:
		// Police Quest 2 and Quest For Glory on PC9801 (Japanese)
		_displayWidth = 640;
		_displayHeight = 400;
		for (int i = 0; i <= _scriptHeight; i++)
			_upscaledHeightMapping[i] = i * 2;
		for (int i = 0; i <= _scriptWidth; i++)
			_upscaledWidthMapping[i] = i * 2;
		break;
	case GFX_SCREEN_UPSCALED_640x440:
		// used by King's Quest 6 on Windows
		_displayWidth = 640;
		_displayHeight = 440;
		for (int i = 0; i <= _scriptHeight; i++)
			_upscaledHeightMapping[i] = (i * 11) / 5;
		for (int i = 0; i <= _scriptWidth; i++)
			_upscaledWidthMapping[i] = i * 2;
		break;
	case GFX_SCREEN_UPSCALED_640x480:
		// Gabriel Knight 1 (VESA, Mac)
		_displayWidth = 640;
		_displayHeight = 480;
		for (int i = 0; i <= _scriptHeight; i++)
			_upscaledHeightMapping[i] = (i * 12) / 5;
		for (int i = 0; i <= _scriptWidth; i++)
			_upscaledWidthMapping[i] = i * 2;
		break;
	default:
		if (!_displayWidth)
			_displayWidth = _width;
		if (!_displayHeight)
			_displayHeight = _height;
		memset(&_upscaledHeightMapping, 0, sizeof(_upscaledHeightMapping) );
		memset(&_upscaledWidthMapping, 0, sizeof(_upscaledWidthMapping) );
		break;
	}

	_displayPixels = _displayWidth * _displayHeight;
	
	// Allocate visual, priority, control and display screen
	_visualScreen = (byte *)calloc(_pixels, 1);
	_priorityScreen = (byte *)calloc(_pixels, 1);
	_controlScreen = (byte *)calloc(_pixels, 1);
	_displayScreen = (byte *)calloc(_displayPixels, 1);

	memset(&_ditheredPicColors, 0, sizeof(_ditheredPicColors));

	// Sets display screen to be actually displayed
	_activeScreen = _displayScreen;

	_picNotValid = 0;
	_picNotValidSci11 = 0;
	_unditheringEnabled = true;
	_fontIsUpscaled = false;

	if (_resMan->getViewType() != kViewEga) {
		// It is not 100% accurate to set white to be 255 for Amiga 32-color
		// games. But 255 is defined as white in our SCI at all times, so it
		// doesn't matter.
		_colorWhite = 255;
		if (getSciVersion() >= SCI_VERSION_1_1)
			_colorDefaultVectorData = 255;
		else
			_colorDefaultVectorData = 0;
	} else {
		_colorWhite = 15;
		_colorDefaultVectorData = 0;
	}

	// Initialize the actual screen

	if (g_sci->hasMacIconBar()) {
		// For SCI1.1 Mac games with the custom icon bar, we need to expand the screen
		// to accommodate for the icon bar. Of course, both KQ6 and QFG1 VGA differ in size.
		// We add 2 to the height of the icon bar to add a buffer between the screen and the
		// icon bar (as did the original interpreter).
		if (g_sci->getGameId() == GID_KQ6)
			initGraphics(_displayWidth, _displayHeight + 26 + 2, _displayWidth > 320);
		else if (g_sci->getGameId() == GID_FREDDYPHARKAS)
			initGraphics(_displayWidth, _displayHeight + 28 + 2, _displayWidth > 320);
		else
			error("Unknown SCI1.1 Mac game");
	} else
		initGraphics(_displayWidth, _displayHeight, _displayWidth > 320);

	// Initialize code pointers
	_vectorAdjustCoordinatePtr = &GfxScreen::vectorAdjustCoordinateNOP;
	_vectorAdjustLineCoordinatesPtr = &GfxScreen::vectorAdjustLineCoordinatesNOP;
	_vectorIsFillMatchPtr = &GfxScreen::vectorIsFillMatchNormal;
	_vectorPutPixelPtr = &GfxScreen::putPixelNormal;
	_vectorPutLinePixelPtr = &GfxScreen::putPixel;
	_vectorGetPixelPtr = &GfxScreen::getPixelNormal;
	_putPixelPtr = &GfxScreen::putPixelNormal;
	_getPixelPtr = &GfxScreen::getPixelNormal;
	
	switch (_upscaledHires) {
	case GFX_SCREEN_UPSCALED_480x300:
		_vectorAdjustCoordinatePtr = &GfxScreen::vectorAdjustCoordinate480x300Mac;
		_vectorAdjustLineCoordinatesPtr = &GfxScreen::vectorAdjustLineCoordinates480x300Mac;
		// vectorPutPixel -> we already adjust coordinates for vector code, that's why we can set pixels directly
		// vectorGetPixel -> see vectorPutPixel
		_vectorPutLinePixelPtr = &GfxScreen::vectorPutLinePixel480x300Mac;
		_putPixelPtr = &GfxScreen::putPixelAllUpscaled;
		_getPixelPtr = &GfxScreen::getPixelUpscaled;
		break;
	case GFX_SCREEN_UPSCALED_640x400:
	case GFX_SCREEN_UPSCALED_640x440:
	case GFX_SCREEN_UPSCALED_640x480:
		_vectorPutPixelPtr = &GfxScreen::putPixelDisplayUpscaled;
		_putPixelPtr = &GfxScreen::putPixelDisplayUpscaled;
		break;
	case GFX_SCREEN_UPSCALED_DISABLED:
		break;
	}
}

GfxScreen::~GfxScreen() {
	free(_visualScreen);
	free(_priorityScreen);
	free(_controlScreen);
	free(_displayScreen);
}

// should not be used regularly; only meant for restore game
void GfxScreen::clearForRestoreGame() {
	// reset all screen data
	memset(_visualScreen, 0, _pixels);
	memset(_priorityScreen, 0, _pixels);
	memset(_controlScreen, 0, _pixels);
	memset(_displayScreen, 0, _displayPixels);
	memset(&_ditheredPicColors, 0, sizeof(_ditheredPicColors));
	_fontIsUpscaled = false;
	copyToScreen();
}

void GfxScreen::copyToScreen() {
	g_system->copyRectToScreen(_activeScreen, _displayWidth, 0, 0, _displayWidth, _displayHeight);
}

void GfxScreen::copyFromScreen(byte *buffer) {
	// TODO this ignores the pitch
	Graphics::Surface *screen = g_system->lockScreen();
	memcpy(buffer, screen->getPixels(), _displayPixels);
	g_system->unlockScreen();
}

void GfxScreen::kernelSyncWithFramebuffer() {
	// TODO this ignores the pitch
	Graphics::Surface *screen = g_system->lockScreen();
	memcpy(_displayScreen, screen->getPixels(), _displayPixels);
	g_system->unlockScreen();
}

void GfxScreen::copyRectToScreen(const Common::Rect &rect) {
	if (!_upscaledHires)  {
		g_system->copyRectToScreen(_activeScreen + rect.top * _displayWidth + rect.left, _displayWidth, rect.left, rect.top, rect.width(), rect.height());
	} else {
		int rectHeight = _upscaledHeightMapping[rect.bottom] - _upscaledHeightMapping[rect.top];
		int rectWidth  = _upscaledWidthMapping[rect.right] - _upscaledWidthMapping[rect.left];
		g_system->copyRectToScreen(_activeScreen + _upscaledHeightMapping[rect.top] * _displayWidth + _upscaledWidthMapping[rect.left], _displayWidth, _upscaledWidthMapping[rect.left], _upscaledHeightMapping[rect.top], rectWidth, rectHeight);
	}
}

/**
 * This copies a rect to screen w/o scaling adjustment and is only meant to be
 * used on hires graphics used in upscaled hires mode.
 */
void GfxScreen::copyDisplayRectToScreen(const Common::Rect &rect) {
	if (!_upscaledHires)
		error("copyDisplayRectToScreen: not in upscaled hires mode");
	g_system->copyRectToScreen(_activeScreen + rect.top * _displayWidth + rect.left, _displayWidth, rect.left, rect.top, rect.width(), rect.height());
}

void GfxScreen::copyRectToScreen(const Common::Rect &rect, int16 x, int16 y) {
	if (!_upscaledHires)  {
		g_system->copyRectToScreen(_activeScreen + rect.top * _displayWidth + rect.left, _displayWidth, x, y, rect.width(), rect.height());
	} else {
		int rectHeight = _upscaledHeightMapping[rect.bottom] - _upscaledHeightMapping[rect.top];
		int rectWidth  = _upscaledWidthMapping[rect.right] - _upscaledWidthMapping[rect.left];

		g_system->copyRectToScreen(_activeScreen + _upscaledHeightMapping[rect.top] * _displayWidth + _upscaledWidthMapping[rect.left], _displayWidth, _upscaledWidthMapping[x], _upscaledHeightMapping[y], rectWidth, rectHeight);
	}
}

byte GfxScreen::getDrawingMask(byte color, byte prio, byte control) {
	byte flag = 0;
	if (color != 255)
		flag |= GFX_SCREEN_MASK_VISUAL;
	if (prio != 255)
		flag |= GFX_SCREEN_MASK_PRIORITY;
	if (control != 255)
		flag |= GFX_SCREEN_MASK_CONTROL;
	return flag;
}

void GfxScreen::vectorAdjustCoordinateNOP(int16 *x, int16 *y) {
}

void GfxScreen::vectorAdjustCoordinate480x300Mac(int16 *x, int16 *y) {
	*x = _upscaledWidthMapping[*x];
	*y = _upscaledHeightMapping[*y];
}

void GfxScreen::vectorAdjustLineCoordinatesNOP(int16 *left, int16 *top, int16 *right, int16 *bottom, byte drawMask, byte color, byte priority, byte control) {
}

void GfxScreen::vectorAdjustLineCoordinates480x300Mac(int16 *left, int16 *top, int16 *right, int16 *bottom, byte drawMask, byte color, byte priority, byte control) {
	int16 displayLeft = _upscaledWidthMapping[*left];
	int16 displayRight = _upscaledWidthMapping[*right];
	int16 displayTop = _upscaledHeightMapping[*top];
	int16 displayBottom = _upscaledHeightMapping[*bottom];
	
	if (displayLeft < displayRight) {
		// one more pixel to the left, one more pixel to the right
		if (displayLeft > 0)
			vectorPutLinePixel(displayLeft - 1, displayTop, drawMask, color, priority, control);
		vectorPutLinePixel(displayRight + 1, displayBottom, drawMask, color, priority, control);
	} else if (displayLeft > displayRight) {
		if (displayRight > 0)
			vectorPutLinePixel(displayRight - 1, displayBottom, drawMask, color, priority, control);
		vectorPutLinePixel(displayLeft + 1, displayTop, drawMask, color, priority, control);
	}
	*left = displayLeft;
	*top = displayTop;
	*right = displayRight;
	*bottom = displayBottom;
}

byte GfxScreen::vectorIsFillMatchNormal(int16 x, int16 y, byte screenMask, byte checkForColor, byte checkForPriority, byte checkForControl, bool isEGA) {
	int offset = y * _width + x;
	byte match = 0;

	if (screenMask & GFX_SCREEN_MASK_VISUAL) {
		if (!isEGA) {
			if (*(_visualScreen + offset) == checkForColor)
				match |= GFX_SCREEN_MASK_VISUAL;
		} else {
			// In EGA games a pixel in the framebuffer is only 4 bits. We store
			// a full byte per pixel to allow undithering, but when comparing
			// pixels for flood-fill purposes, we should only compare the
			// visible color of a pixel.

			byte EGAcolor = *(_visualScreen + offset);
			if ((x ^ y) & 1)
				EGAcolor = (EGAcolor ^ (EGAcolor >> 4)) & 0x0F;
			else
				EGAcolor = EGAcolor & 0x0F;
			if (EGAcolor == checkForColor)
				match |= GFX_SCREEN_MASK_VISUAL;
		}
	}
	if ((screenMask & GFX_SCREEN_MASK_PRIORITY) && *(_priorityScreen + offset) == checkForPriority)
		match |= GFX_SCREEN_MASK_PRIORITY;
	if ((screenMask & GFX_SCREEN_MASK_CONTROL) && *(_controlScreen + offset) == checkForControl)
		match |= GFX_SCREEN_MASK_CONTROL;
	return match;
}

// Special 480x300 Mac putPixel for vector line drawing, also draws an additional pixel below the actual one
void GfxScreen::vectorPutLinePixel480x300Mac(int16 x, int16 y, byte drawMask, byte color, byte priority, byte control) {
	int offset = y * _width + x;
	
	if (drawMask & GFX_SCREEN_MASK_VISUAL) {
		_visualScreen[offset] = color;
		_visualScreen[offset + _width] = color;
		_displayScreen[offset] = color;
		// also set pixel below actual pixel
		_displayScreen[offset + _displayWidth] = color;
	}
	if (drawMask & GFX_SCREEN_MASK_PRIORITY) {
		_priorityScreen[offset] = priority;
		_priorityScreen[offset + _width] = priority;
	}
	if (drawMask & GFX_SCREEN_MASK_CONTROL) {
		_controlScreen[offset] = control;
		_controlScreen[offset + _width] = control;
	}
}

// Directly sets a pixel on various screens, display is not upscaled
void GfxScreen::putPixelNormal(int16 x, int16 y, byte drawMask, byte color, byte priority, byte control) {
	int offset = y * _width + x;

	if (drawMask & GFX_SCREEN_MASK_VISUAL) {
		_visualScreen[offset] = color;
		_displayScreen[offset] = color;
	}
	if (drawMask & GFX_SCREEN_MASK_PRIORITY)
		_priorityScreen[offset] = priority;
	if (drawMask & GFX_SCREEN_MASK_CONTROL)
		_controlScreen[offset] = control;
}

// Directly sets a pixel on various screens, display IS upscaled
void GfxScreen::putPixelDisplayUpscaled(int16 x, int16 y, byte drawMask, byte color, byte priority, byte control) {
	int offset = y * _width + x;

	if (drawMask & GFX_SCREEN_MASK_VISUAL) {
		_visualScreen[offset] = color;
		putScaledPixelOnScreen(_displayScreen, x, y, color);
	}
	if (drawMask & GFX_SCREEN_MASK_PRIORITY)
		_priorityScreen[offset] = priority;
	if (drawMask & GFX_SCREEN_MASK_CONTROL)
		_controlScreen[offset] = control;
}

// Directly sets a pixel on various screens, ALL screens ARE upscaled
void GfxScreen::putPixelAllUpscaled(int16 x, int16 y, byte drawMask, byte color, byte priority, byte control) {
	if (drawMask & GFX_SCREEN_MASK_VISUAL) {
		putScaledPixelOnScreen(_visualScreen, x, y, color);
		putScaledPixelOnScreen(_displayScreen, x, y, color);
	}
	if (drawMask & GFX_SCREEN_MASK_PRIORITY)
		putScaledPixelOnScreen(_priorityScreen, x, y, priority);
	if (drawMask & GFX_SCREEN_MASK_CONTROL)
		putScaledPixelOnScreen(_controlScreen, x, y, control);
}

/**
 * This is used to put font pixels onto the screen - we adjust differently, so that we won't
 *  do triple pixel lines in any case on upscaled hires. That way the font will not get distorted
 *  Sierra SCI didn't do this
 */
void GfxScreen::putFontPixel(int16 startingY, int16 x, int16 y, byte color) {
	int16 actualY = startingY + y;
	if (_fontIsUpscaled) {
		// Do not scale ourselves, but put it on the display directly
		putPixelOnDisplay(x, actualY, color);
	} else {
		int offset = actualY * _width + x;

		_visualScreen[offset] = color;
		switch (_upscaledHires) {
		case GFX_SCREEN_UPSCALED_DISABLED:
			_displayScreen[offset] = color;
			break;
		case GFX_SCREEN_UPSCALED_640x400:
		case GFX_SCREEN_UPSCALED_640x440:
		case GFX_SCREEN_UPSCALED_640x480: {
			// to 1-> 4 pixels upscaling for all of those, so that fonts won't look weird
			int displayOffset = (_upscaledHeightMapping[startingY] + y * 2) * _displayWidth + x * 2;
			_displayScreen[displayOffset] = color;
			_displayScreen[displayOffset + 1] = color;
			displayOffset += _displayWidth;
			_displayScreen[displayOffset] = color;
			_displayScreen[displayOffset + 1] = color;
			break;
		}
		default:
			putScaledPixelOnScreen(_displayScreen, x, actualY, color);
			break;
		}
	}
}

/**
 * This will just change a pixel directly on displayscreen. It is supposed to be
 * only used on upscaled-Hires games where hires content needs to get drawn ONTO
 * the upscaled display screen (like japanese fonts, hires portraits, etc.).
 */
void GfxScreen::putPixelOnDisplay(int16 x, int16 y, byte color) {
	int offset = y * _displayWidth + x;
	_displayScreen[offset] = color;
}

//void GfxScreen::putScaledPixelOnDisplay(int16 x, int16 y, byte color) {
//}

void GfxScreen::putScaledPixelOnScreen(byte *screen, int16 x, int16 y, byte data) {
	int displayOffset = _upscaledHeightMapping[y] * _displayWidth + _upscaledWidthMapping[x];
	int heightOffsetBreak = (_upscaledHeightMapping[y + 1] - _upscaledHeightMapping[y]) * _displayWidth;
	int heightOffset = 0;
	int widthOffsetBreak = _upscaledWidthMapping[x + 1] - _upscaledWidthMapping[x];
	do {
		int widthOffset = 0;
		do {
			screen[displayOffset + heightOffset + widthOffset] = data;
			widthOffset++;
		} while (widthOffset != widthOffsetBreak);
		heightOffset += _displayWidth;
	} while (heightOffset != heightOffsetBreak);
}

/**
 * Sierra's Bresenham line drawing.
 * WARNING: Do not replace this with Graphics::drawLine(), as this causes issues
 * with flood fill, due to small difference in the Bresenham logic.
 */
void GfxScreen::drawLine(Common::Point startPoint, Common::Point endPoint, byte color, byte priority, byte control) {
    int16 maxWidth = _width - 1;
    int16 maxHeight = _height - 1;
    // we need to clip values here, lsl3 room 620 background picture draws a line from 0, 199 t 320, 199
    //  otherwise we would get heap corruption.
	int16 left = CLIP<int16>(startPoint.x, 0, maxWidth);
	int16 top = CLIP<int16>(startPoint.y, 0, maxHeight);
	int16 right = CLIP<int16>(endPoint.x, 0, maxWidth);
	int16 bottom = CLIP<int16>(endPoint.y, 0, maxHeight);

	//set_drawing_flag
	byte drawMask = getDrawingMask(color, priority, control);

	vectorAdjustLineCoordinates(&left, &top, &right, &bottom, drawMask, color, priority, control);

	// horizontal line
	if (top == bottom) {
		if (right < left)
			SWAP(right, left);
		for (int i = left; i <= right; i++)
			vectorPutLinePixel(i, top, drawMask, color, priority, control);
		return;
	}
	// vertical line
	if (left == right) {
		if (top > bottom)
			SWAP(top, bottom);
		for (int i = top; i <= bottom; i++)
			vectorPutLinePixel(left, i, drawMask, color, priority, control);
		return;
	}
	// sloped line - draw with Bresenham algorithm
	int16 dy = bottom - top;
	int16 dx = right - left;
	int16 stepy = dy < 0 ? -1 : 1;
	int16 stepx = dx < 0 ? -1 : 1;
	dy = ABS(dy) << 1;
	dx = ABS(dx) << 1;

	// setting the 1st and last pixel
	vectorPutLinePixel(left, top, drawMask, color, priority, control);
	vectorPutLinePixel(right, bottom, drawMask, color, priority, control);
	// drawing the line
	if (dx > dy) { // going horizontal
		int fraction = dy - (dx >> 1);
		while (left != right) {
			if (fraction >= 0) {
				top += stepy;
				fraction -= dx;
			}
			left += stepx;
			fraction += dy;
			vectorPutLinePixel(left, top, drawMask, color, priority, control);
		}
	} else { // going vertical
		int fraction = dx - (dy >> 1);
		while (top != bottom) {
			if (fraction >= 0) {
				left += stepx;
				fraction -= dy;
			}
			top += stepy;
			fraction += dx;
			vectorPutLinePixel(left, top, drawMask, color, priority, control);
		}
	}
}

// We put hires kanji chars onto upscaled background, so we need to adjust
// coordinates. Caller gives use low-res ones.
void GfxScreen::putKanjiChar(Graphics::FontSJIS *commonFont, int16 x, int16 y, uint16 chr, byte color) {
	byte *displayPtr = _displayScreen + y * _displayWidth * 2 + x * 2;
	// we don't use outline, so color 0 is actually not used
	commonFont->drawChar(displayPtr, chr, _displayWidth, 1, color, 0, -1, -1);
}

byte GfxScreen::getPixelNormal(byte *screen, int16 x, int16 y) {
	return screen[y * _width + x];
}

byte GfxScreen::getPixelUpscaled(byte *screen, int16 x, int16 y) {
	int16 mappedX = _upscaledWidthMapping[x];
	int16 mappedY = _upscaledHeightMapping[y];
	return screen[mappedY * _width + mappedX];
}

int GfxScreen::bitsGetDataSize(Common::Rect rect, byte mask) {
	int byteCount = sizeof(rect) + sizeof(mask);
	int pixels = rect.width() * rect.height();
	if (mask & GFX_SCREEN_MASK_VISUAL) {
		byteCount += pixels; // _visualScreen
		if (!_upscaledHires) {
			byteCount += pixels; // _displayScreen
		} else {
			int rectHeight = _upscaledHeightMapping[rect.bottom] - _upscaledHeightMapping[rect.top];
			int rectWidth = _upscaledWidthMapping[rect.right] - _upscaledWidthMapping[rect.left];
			byteCount += rectHeight * rect.width() * rectWidth; // _displayScreen (upscaled hires)
		}
	}
	if (mask & GFX_SCREEN_MASK_PRIORITY) {
		byteCount += pixels; // _priorityScreen
	}
	if (mask & GFX_SCREEN_MASK_CONTROL) {
		byteCount += pixels; // _controlScreen
	}
	if (mask & GFX_SCREEN_MASK_DISPLAY) {
		if (!_upscaledHires)
			error("bitsGetDataSize() called w/o being in upscaled hires mode");
		byteCount += pixels; // _displayScreen (coordinates actually are given to us for hires displayScreen)
	}

	return byteCount;
}

void GfxScreen::bitsSave(Common::Rect rect, byte mask, byte *memoryPtr) {
	memcpy(memoryPtr, (void *)&rect, sizeof(rect)); memoryPtr += sizeof(rect);
	memcpy(memoryPtr, (void *)&mask, sizeof(mask)); memoryPtr += sizeof(mask);

	if (mask & GFX_SCREEN_MASK_VISUAL) {
		bitsSaveScreen(rect, _visualScreen, _width, memoryPtr);
		bitsSaveDisplayScreen(rect, memoryPtr);
	}
	if (mask & GFX_SCREEN_MASK_PRIORITY) {
		bitsSaveScreen(rect, _priorityScreen, _width, memoryPtr);
	}
	if (mask & GFX_SCREEN_MASK_CONTROL) {
		bitsSaveScreen(rect, _controlScreen, _width, memoryPtr);
	}
	if (mask & GFX_SCREEN_MASK_DISPLAY) {
		if (!_upscaledHires)
			error("bitsSave() called w/o being in upscaled hires mode");
		bitsSaveScreen(rect, _displayScreen, _displayWidth, memoryPtr);
	}
}

void GfxScreen::bitsSaveScreen(Common::Rect rect, byte *screen, uint16 screenWidth, byte *&memoryPtr) {
	int width = rect.width();
	int y;

	screen += (rect.top * screenWidth) + rect.left;

	for (y = rect.top; y < rect.bottom; y++) {
		memcpy(memoryPtr, (void *)screen, width); memoryPtr += width;
		screen += screenWidth;
	}
}

void GfxScreen::bitsSaveDisplayScreen(Common::Rect rect, byte *&memoryPtr) {
	byte *screen = _displayScreen;
	int width;
	int y;

	if (!_upscaledHires) {
		width = rect.width();
		screen += (rect.top * _displayWidth) + rect.left;
	} else {
		screen += (_upscaledHeightMapping[rect.top] * _displayWidth) + _upscaledWidthMapping[rect.left];
		width = _upscaledWidthMapping[rect.right] - _upscaledWidthMapping[rect.left];
		rect.top = _upscaledHeightMapping[rect.top];
		rect.bottom = _upscaledHeightMapping[rect.bottom];
	}

	for (y = rect.top; y < rect.bottom; y++) {
		memcpy(memoryPtr, (void *)screen, width); memoryPtr += width;
		screen += _displayWidth;
	}
}

void GfxScreen::bitsGetRect(byte *memoryPtr, Common::Rect *destRect) {
	memcpy((void *)destRect, memoryPtr, sizeof(Common::Rect));
}

void GfxScreen::bitsRestore(byte *memoryPtr) {
	Common::Rect rect;
	byte mask;

	memcpy((void *)&rect, memoryPtr, sizeof(rect)); memoryPtr += sizeof(rect);
	memcpy((void *)&mask, memoryPtr, sizeof(mask)); memoryPtr += sizeof(mask);

	if (mask & GFX_SCREEN_MASK_VISUAL) {
		bitsRestoreScreen(rect, memoryPtr, _visualScreen, _width);
		bitsRestoreDisplayScreen(rect, memoryPtr);
	}
	if (mask & GFX_SCREEN_MASK_PRIORITY) {
		bitsRestoreScreen(rect, memoryPtr, _priorityScreen, _width);
	}
	if (mask & GFX_SCREEN_MASK_CONTROL) {
		bitsRestoreScreen(rect, memoryPtr, _controlScreen, _width);
	}
	if (mask & GFX_SCREEN_MASK_DISPLAY) {
		if (!_upscaledHires)
			error("bitsRestore() called w/o being in upscaled hires mode");
		bitsRestoreScreen(rect, memoryPtr, _displayScreen, _displayWidth);
		// WORKAROUND - we are not sure what sierra is doing. If we don't do this here, portraits won't get fully removed
		//  from screen. Some lowres showBits() call is used for that and it's not covering the whole area
		//  We would need to find out inside the kq6 windows interpreter, but this here works already and seems not to have
		//  any side-effects. The whole hires is hacked into the interpreter, so maybe this is even right.
		copyDisplayRectToScreen(rect);
	}
}

void GfxScreen::bitsRestoreScreen(Common::Rect rect, byte *&memoryPtr, byte *screen, uint16 screenWidth) {
	int width = rect.width();
	int y;

	screen += (rect.top * screenWidth) + rect.left;

	for (y = rect.top; y < rect.bottom; y++) {
		memcpy((void *) screen, memoryPtr, width); memoryPtr += width;
		screen += screenWidth;
	}
}

void GfxScreen::bitsRestoreDisplayScreen(Common::Rect rect, byte *&memoryPtr) {
	byte *screen = _displayScreen;
	int width;
	int y;

	if (!_upscaledHires) {
		screen += (rect.top * _displayWidth) + rect.left;
		width = rect.width();
	} else {
		screen += (_upscaledHeightMapping[rect.top] * _displayWidth) + _upscaledWidthMapping[rect.left];
		width = _upscaledWidthMapping[rect.right] - _upscaledWidthMapping[rect.left];
		rect.top = _upscaledHeightMapping[rect.top];
		rect.bottom = _upscaledHeightMapping[rect.bottom];
	}

	for (y = rect.top; y < rect.bottom; y++) {
		memcpy((void *) screen, memoryPtr, width); memoryPtr += width;
		screen += _displayWidth;
	}
}

void GfxScreen::setVerticalShakePos(uint16 shakePos) {
	if (!_upscaledHires)
		g_system->setShakePos(shakePos);
	else
		g_system->setShakePos(_upscaledHeightMapping[shakePos]);
}

void GfxScreen::kernelShakeScreen(uint16 shakeCount, uint16 directions) {
	while (shakeCount--) {
		if (directions & SCI_SHAKE_DIRECTION_VERTICAL)
			setVerticalShakePos(10);
		// TODO: horizontal shakes
		g_system->updateScreen();
		g_sci->getEngineState()->wait(3);

		if (directions & SCI_SHAKE_DIRECTION_VERTICAL)
			setVerticalShakePos(0);

		g_system->updateScreen();
		g_sci->getEngineState()->wait(3);
	}
}

void GfxScreen::dither(bool addToFlag) {
	int y, x;
	byte color, ditheredColor;
	byte *visualPtr = _visualScreen;
	byte *displayPtr = _displayScreen;
	
	if (!_unditheringEnabled) {
		// Do dithering on visual and display-screen
		for (y = 0; y < _height; y++) {
			for (x = 0; x < _width; x++) {
				color = *visualPtr;
				if (color & 0xF0) {
					color ^= color << 4;
					color = ((x^y) & 1) ? color >> 4 : color & 0x0F;
					switch (_upscaledHires) {
					case GFX_SCREEN_UPSCALED_DISABLED:
					case GFX_SCREEN_UPSCALED_480x300:
						*displayPtr = color;
						break;
					default:
						putScaledPixelOnScreen(_displayScreen, x, y, color);
						break;
					}
					*visualPtr = color;
				}
				visualPtr++; displayPtr++;
			}
		}
	} else {
		if (!addToFlag)
			memset(&_ditheredPicColors, 0, sizeof(_ditheredPicColors));
		// Do dithering on visual screen and put decoded but undithered byte onto display-screen
		for (y = 0; y < _height; y++) {
			for (x = 0; x < _width; x++) {
				color = *visualPtr;
				if (color & 0xF0) {
					color ^= color << 4;
					// remember dither combination for cel-undithering
					_ditheredPicColors[color]++;
					// if decoded color wants do dither with black on left side, we turn it around
					//  otherwise the normal ega color would get used for display
					if (color & 0xF0) {
						ditheredColor = color;
					}	else {
						ditheredColor = color << 4;
					}
					switch (_upscaledHires) {
					case GFX_SCREEN_UPSCALED_DISABLED:
					case GFX_SCREEN_UPSCALED_480x300:
						*displayPtr = ditheredColor;
						break;
					default:
						putScaledPixelOnScreen(_displayScreen, x, y, ditheredColor);
						break;
					}
					color = ((x^y) & 1) ? color >> 4 : color & 0x0F;
					*visualPtr = color;
				}
				visualPtr++; displayPtr++;
			}
		}
	}
}

void GfxScreen::ditherForceDitheredColor(byte color) {
	_ditheredPicColors[color] = 256;
}

void GfxScreen::enableUndithering(bool flag) {
	_unditheringEnabled = flag;
}

int16 *GfxScreen::unditherGetDitheredBgColors() {
	if (_unditheringEnabled)
		return _ditheredPicColors;
	else
		return NULL;
}

void GfxScreen::debugShowMap(int mapNo) {
	// We cannot really support changing maps when display screen has a different resolution than visual screen
	if ((_width != _displayWidth) || (_height != _displayHeight))
		return;

	switch (mapNo) {
	case 0:
		_activeScreen = _visualScreen;
		break;
	case 1:
		_activeScreen = _priorityScreen;
		break;
	case 2:
		_activeScreen = _controlScreen;
		break;
	case 3:
		_activeScreen = _displayScreen;
		break;
	}
	copyToScreen();
}

void GfxScreen::scale2x(const byte *src, byte *dst, int16 srcWidth, int16 srcHeight, byte bytesPerPixel) {
	assert(bytesPerPixel == 1 || bytesPerPixel == 2);
	const int newWidth = srcWidth * 2;
	const int pitch = newWidth * bytesPerPixel;
	const byte *srcPtr = src;

	if (bytesPerPixel == 1) {
		for (int y = 0; y < srcHeight; y++) {
			for (int x = 0; x < srcWidth; x++) {
				const byte color = *srcPtr++;
				dst[0] = color;
				dst[1] = color;
				dst[newWidth] = color;
				dst[newWidth + 1] = color;
				dst += 2;
			}
			dst += newWidth;
		}
	} else if (bytesPerPixel == 2) {
		for (int y = 0; y < srcHeight; y++) {
			for (int x = 0; x < srcWidth; x++) {
				const byte color = *srcPtr++;
				const byte color2 = *srcPtr++;
				dst[0] = color;
				dst[1] = color2;
				dst[2] = color;
				dst[3] = color2;
				dst[pitch] = color;
				dst[pitch + 1] = color2;
				dst[pitch + 2] = color;
				dst[pitch + 3] = color2;
				dst += 4;
			}
			dst += pitch;
		}
	}
}

struct UpScaledAdjust {
	GfxScreenUpscaledMode gameHiresMode;
	Sci32ViewNativeResolution viewNativeRes;
	int numerator;
	int denominator;
};

static const UpScaledAdjust s_upscaledAdjustTable[] = {
	{ GFX_SCREEN_UPSCALED_640x480, SCI_VIEW_NATIVERES_640x400, 5, 6 }
};

void GfxScreen::adjustToUpscaledCoordinates(int16 &y, int16 &x, Sci32ViewNativeResolution viewNativeRes) {
	x = _upscaledWidthMapping[x];
	y = _upscaledHeightMapping[y];

	for (int i = 0; i < ARRAYSIZE(s_upscaledAdjustTable); i++) {
		if (s_upscaledAdjustTable[i].gameHiresMode == _upscaledHires &&
				s_upscaledAdjustTable[i].viewNativeRes == viewNativeRes) {
			y = (y * s_upscaledAdjustTable[i].numerator) / s_upscaledAdjustTable[i].denominator;
			break;
		}
	}
}

void GfxScreen::adjustBackUpscaledCoordinates(int16 &y, int16 &x, Sci32ViewNativeResolution viewNativeRes) {
	for (int i = 0; i < ARRAYSIZE(s_upscaledAdjustTable); i++) {
		if (s_upscaledAdjustTable[i].gameHiresMode == _upscaledHires &&
				s_upscaledAdjustTable[i].viewNativeRes == viewNativeRes) {
			y = (y * s_upscaledAdjustTable[i].denominator) / s_upscaledAdjustTable[i].numerator;
			break;
		}
	}

	switch (_upscaledHires) {
	case GFX_SCREEN_UPSCALED_480x300:
		x = (x * 4) / 6;
		y = (y * 4) / 6;
		break;
	case GFX_SCREEN_UPSCALED_640x400:
		x /= 2;
		y /= 2;
		break;
	case GFX_SCREEN_UPSCALED_640x440:
		x /= 2;
		y = (y * 5) / 11;
		break;
	case GFX_SCREEN_UPSCALED_640x480:
		x /= 2;
		y = (y * 5) / 12;
	default:
		break;
	}
}

int16 GfxScreen::kernelPicNotValid(int16 newPicNotValid) {
	int16 oldPicNotValid;

	if (getSciVersion() >= SCI_VERSION_1_1) {
		oldPicNotValid = _picNotValidSci11;

		if (newPicNotValid != -1)
			_picNotValidSci11 = newPicNotValid;
	} else {
		oldPicNotValid = _picNotValid;

		if (newPicNotValid != -1)
			_picNotValid = newPicNotValid;
	}

	return oldPicNotValid;
}

} // End of namespace Sci
