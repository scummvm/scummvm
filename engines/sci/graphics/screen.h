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

#ifndef SCI_GRAPHICS_SCREEN_H
#define SCI_GRAPHICS_SCREEN_H

#include "sci/sci.h"
#include "sci/graphics/helpers.h"
#include "sci/graphics/view.h"

#include "graphics/font.h"
#include "graphics/sjis.h"
#include "graphics/korfont.h"
#include "graphics/pixelformat.h"

#include "common/rendermode.h"

namespace Sci {

enum {
	SCI_SCREEN_UPSCALEDMAXHEIGHT = 200,
	SCI_SCREEN_UPSCALEDMAXWIDTH  = 320
};

enum GfxScreenUpscaledMode {
	GFX_SCREEN_UPSCALED_DISABLED	= 0,
	GFX_SCREEN_UPSCALED_480x300     = 1,
	GFX_SCREEN_UPSCALED_640x400		= 2
};

enum GfxScreenMasks {
	GFX_SCREEN_MASK_VISUAL		= 1,
	GFX_SCREEN_MASK_PRIORITY	= 2,
	GFX_SCREEN_MASK_CONTROL		= 4,
	GFX_SCREEN_MASK_ALL			= GFX_SCREEN_MASK_VISUAL|GFX_SCREEN_MASK_PRIORITY|GFX_SCREEN_MASK_CONTROL
};

enum {
	DITHERED_BG_COLORS_SIZE = 256
};

class GfxDriver;

/**
 * Screen class, actually creates 3 (4) screens internally:
 * - visual/display (for the user),
 * - priority (contains priority information) and
 * - control (contains control information).
 * Handles all operations to it and copies parts of visual/display screen to
 * the actual screen, so the user can really see it.
 */
class GfxScreen {
public:
	GfxScreen(ResourceManager *resMan, Common::RenderMode renderMode);
	~GfxScreen();

	uint16 getWidth() { return _width; }
	uint16 getHeight() { return _height; }
	uint16 getScriptWidth() { return _scriptWidth; }
	uint16 getScriptHeight() { return _scriptHeight; }
	uint16 getDisplayWidth() { return _displayWidth; }
	uint16 getDisplayHeight() { return _displayHeight; }
	byte getColorWhite() { return _colorWhite; }
	byte getColorDefaultVectorData() { return _colorDefaultVectorData; }

	void clearForRestoreGame();
	void copyToScreen();
	void kernelSyncWithFramebuffer();
	void copyRectToScreen(const Common::Rect &rect);
	void copyHiResRectToScreen(const byte *srcBuffer, int pitch, int x, int y, int w, int h, const byte *colorMap);
	void copyRectToScreen(const Common::Rect &rect, int16 x, int16 y);

	// functions to manipulate a backup copy of the screen (for transitions)
	void bakCreateBackup();
	void bakCopyRectToScreen(const Common::Rect &rect, int16 x, int16 y);
	void bakDiscard();

	// video frame displaying
	void copyVideoFrameToScreen(const byte *buffer, int pitch, const Common::Rect &rect);

	// Vector drawing
private:
	void vectorPutLinePixel(int16 x, int16 y, byte drawMask, byte color, byte priority, byte control);
	void vectorPutLinePixel480x300(int16 x, int16 y, byte drawMask, byte color, byte priority, byte control);

public:
	void vectorAdjustLineCoordinates(int16 *left, int16 *top, int16 *right, int16 *bottom, byte drawMask, byte color, byte priority, byte control);
	byte vectorIsFillMatch(int16 x, int16 y, byte screenMask, byte checkForColor, byte checkForPriority, byte checkForControl, bool isEGA);

	byte getDrawingMask(byte color, byte prio, byte control);
	void drawLine(Common::Point startPoint, Common::Point endPoint, byte color, byte prio, byte control);
	void drawLine(int16 left, int16 top, int16 right, int16 bottom, byte color, byte prio, byte control) {
		drawLine(Common::Point(left, top), Common::Point(right, bottom), color, prio, control);
	}

	GfxScreenUpscaledMode getUpscaledHires() const {
		return _upscaledHires;
	}

	bool isUnditheringEnabled() const {
		return _unditheringEnabled;
	}
	void enableUndithering(bool flag);

	void putMacChar(const Graphics::Font *commonFont, int16 x, int16 y, uint16 chr, byte color);
	void putKanjiChar(Graphics::FontSJIS *commonFont, int16 x, int16 y, uint16 chr, byte color);
	void putHangulChar(Graphics::FontKorean *commonFont, int16 x, int16 y, uint16 chr, byte color);

	int bitsGetDataSize(Common::Rect rect, byte mask);
	void bitsSave(Common::Rect rect, byte mask, byte *memoryPtr);
	void bitsGetRect(const byte *memoryPtr, Common::Rect *destRect);
	void bitsRestore(const byte *memoryPtr);

	void scale2x(const SciSpan<const byte> &src, SciSpan<byte> &dst, int16 srcWidth, int16 srcHeight, byte bytesPerPixel = 1);

	void adjustToUpscaledCoordinates(int16 &y, int16 &x);
	void adjustBackUpscaledCoordinates(int16 &y, int16 &x);

	void dither(bool addToFlag);

	// Force a color combination as a dithered color
	void ditherForceDitheredColor(byte color);
	int16 *unditherGetDitheredBgColors();

	void debugShowMap(int mapNo);

	int _picNotValid; // possible values 0, 1 and 2
	int _picNotValidSci11; // another variable that is used by kPicNotValid in sci1.1

	int16 kernelPicNotValid(int16 newPicNotValid);
	void kernelShakeScreen(uint16 shakeCount, uint16 direction);

	void setFontIsUpscaled(bool isUpscaled) { _fontIsUpscaled = isUpscaled; }
	bool fontIsUpscaled() const { return _fontIsUpscaled; }

	void grabPalette(byte *buffer, uint start, uint num) const;
	void setPalette(const byte *buffer, uint start, uint num, bool update = true);

	byte getCurPaletteMapValue() const { return _curPaletteMapValue; }
	void setCurPaletteMapValue(byte val) { _curPaletteMapValue = val; }
	void setPaletteMods(const PaletteMod *mods, unsigned int count);
	bool paletteModsEnabled() const { return _paletteModsEnabled; }

	GfxDriver *gfxDriver() const { return _gfxDrv; }

private:
	uint16 _width;
	uint16 _height;
	uint _pixels;
	uint16 _scriptWidth;
	uint16 _scriptHeight;
	uint16 _displayWidth;
	uint16 _displayHeight;
	uint _displayPixels;

	byte _colorWhite;
	byte _colorDefaultVectorData;

	void bitsRestoreScreen(Common::Rect rect, const byte *&memoryPtr, byte *screen, uint16 screenWidth);
	void bitsRestoreDisplayScreen(Common::Rect rect, const byte *&memoryPtr, byte *screen);
	void bitsSaveScreen(Common::Rect rect, const byte *screen, uint16 screenWidth, byte *&memoryPtr);
	void bitsSaveDisplayScreen(Common::Rect rect, const byte *screen, byte *&memoryPtr);

	void setShakePos(uint16 shakeXOffset, uint16 shakeYOffset);

	/**
	 * If this flag is true, undithering is enabled, otherwise disabled.
	 */
	bool _unditheringEnabled;
	int16 _ditheredPicColors[DITHERED_BG_COLORS_SIZE];

	// These screens have the real resolution of the game engine (320x200 for
	// SCI0/SCI1/SCI11 PC games, other resolutions on Mac). SCI0 games will be
	// dithered in here at any time.
	byte *_visualScreen;
	byte *_priorityScreen;
	byte *_controlScreen;

	/**
	 * This screen is the one, where pixels are copied out of into the frame buffer.
	 * It may be 640x400 for japanese SCI1 games. SCI0 games may be undithered in here.
	 * Only read from this buffer for Save/ShowBits usage.
	 */
	byte *_displayScreen;
	Graphics::Surface _displayScreenSurface;

	/**
	 * Support for CGA and Hercules and other graphic modes that the original
	 * interpreters allowed. It also performs the rgb rendering if needed.
	 */
	GfxDriver *_gfxDrv;

	// For RGB per-view/pic palette mods
	byte *_paletteMapScreen;
	byte _curPaletteMapValue;
	PaletteMod _paletteMods[256];
	bool _paletteModsEnabled;

	byte *_backupScreen; // for bak* functions

	void displayRect(const Common::Rect &rect, int x, int y);

	ResourceManager *_resMan;

	/**
	 * Pointer to the currently active screen (changing only required for
	 * debug purposes, to show for example the priority screen).
	 */
	byte *_activeScreen;

	/**
	 * This variable defines, if upscaled hires is active and what upscaled mode
	 * is used.
	 */
	GfxScreenUpscaledMode _upscaledHires;

	/**
	 * This flag is part of a hack to properly remove hires graphics from the screen
	 * when mixed speech/text mode is enabled. Sometimes the portrait is drawn
	 * out of bounds of the window/view port rect, so GfxPaint16::bitsShow() will
	 * not be able to refresh that part of the screen.
	 * The mixed speech/text is a mode that wasn't part of the original game, so
	 * there wasn't any need for Sierra to fix it. The mode has been added to ScummVM
	 * via complex script patches. Another solution would be to update these patches
	 * with better y-coordinates (inside the port rect)...
	 */
	bool _activeHiresView;

	/**
	 * This buffer is used to draw a single hires font glyph.
	 */
	byte *_hiresGlyphBuffer;

	/**
	 * This here holds a translation for vertical+horizontal coordinates between native
	 * (visual) and actual (display) screen.
	 */
	int16 _upscaledHeightMapping[SCI_SCREEN_UPSCALEDMAXHEIGHT + 1];
	int16 _upscaledWidthMapping[SCI_SCREEN_UPSCALEDMAXWIDTH + 1];

	/**
	 * This defines whether or not the font we're drawing is already scaled
	 * to the screen size (and we therefore should not upscale it ourselves).
	 */
	bool _fontIsUpscaled;


	// pixel related code, in header so that it can be inlined for performance
public:
	void putPixel(int16 x, int16 y, byte drawMask, byte color, byte priority, byte control) {
		if (_upscaledHires == GFX_SCREEN_UPSCALED_480x300) {
			putPixel480x300(x, y, drawMask, color, priority, control);
			return;
		}

		// Set pixel for visual, priority and control map directly, those are not upscaled
		const int offset = y * _width + x;

		if (drawMask & GFX_SCREEN_MASK_VISUAL) {
			_visualScreen[offset] = color;
			if (_paletteMapScreen)
				_paletteMapScreen[offset] = _curPaletteMapValue;

			switch (_upscaledHires) {
			case GFX_SCREEN_UPSCALED_DISABLED:
				_displayScreen[offset] = color;
				break;

			case GFX_SCREEN_UPSCALED_640x400:
				putScaledPixelOnDisplay(x, y, color);
				break;
			default:
				break;
			}
		}
		if (drawMask & GFX_SCREEN_MASK_PRIORITY) {
			_priorityScreen[offset] = priority;
		}
		if (drawMask & GFX_SCREEN_MASK_CONTROL) {
			_controlScreen[offset] = control;
		}
	}

	void putPixel480x300(int16 x, int16 y, byte drawMask, byte color, byte priority, byte control) {
		const int offset = ((y * 3) / 2 * _width) + ((x * 3) / 2);

		// All maps are upscaled
		// TODO: figure out, what Sierra exactly did on Mac for these games
		if (drawMask & GFX_SCREEN_MASK_VISUAL) {
			putPixel480x300Worker(x, y, offset, _visualScreen, color);
			putPixel480x300Worker(x, y, offset, _displayScreen, color);
		}
		if (drawMask & GFX_SCREEN_MASK_PRIORITY) {
			putPixel480x300Worker(x, y, offset, _priorityScreen, priority);
		}
		if (drawMask & GFX_SCREEN_MASK_CONTROL) {
			putPixel480x300Worker(x, y, offset, _controlScreen, control);
		}
	}
	void putPixel480x300Worker(int16 x, int16 y, int offset, byte *screen, byte byteToSet) {
		screen[offset] = byteToSet;
		if (x & 1)
			screen[offset + 1] = byteToSet;
		if (y & 1)
			screen[offset + _width] = byteToSet;
		if ((x & 1) && (y & 1))
			screen[offset + _width + 1] = byteToSet;
	}

	// This is called from vector drawing to put a pixel at a certain location
	void vectorPutPixel(int16 x, int16 y, byte drawMask, byte color, byte priority, byte control) {
		switch (_upscaledHires) {
		case GFX_SCREEN_UPSCALED_640x400:
		// For regular upscaled modes forward to the regular putPixel
			putPixel(x, y, drawMask, color, priority, control);
			return;
			break;

		default:
			break;
		}

		// For non-upscaled mode and 480x300 Mac put pixels directly
		int offset = y * _width + x;

		if (drawMask & GFX_SCREEN_MASK_VISUAL) {
			_visualScreen[offset] = color;
			_displayScreen[offset] = color;
			if (_paletteMapScreen)
				_paletteMapScreen[offset] = _curPaletteMapValue;

		}
		if (drawMask & GFX_SCREEN_MASK_PRIORITY) {
			_priorityScreen[offset] = priority;
		}
		if (drawMask & GFX_SCREEN_MASK_CONTROL) {
			_controlScreen[offset] = control;
		}
	}

	/**
	 * This will just change a pixel directly on displayscreen. It is supposed to be
	 * only used on upscaled-Hires games where hires content needs to get drawn ONTO
	 * the upscaled display screen (like japanese fonts, hires portraits, etc.).
	 */
	void putPixelOnDisplay(int16 x, int16 y, byte color) {
		int offset = y * _displayWidth + x;
		_displayScreen[offset] = color;
	}

	// Upscales a pixel and puts it on display screen only
	void putScaledPixelOnDisplay(int16 x, int16 y, byte color) {
		int displayOffset = 0;

		switch (_upscaledHires) {
		case GFX_SCREEN_UPSCALED_640x400:
			displayOffset = (y * 2) * _displayWidth + x * 2; // straight 1 pixel -> 2 mapping

			_displayScreen[displayOffset] = color;
			_displayScreen[displayOffset + 1] = color;
			_displayScreen[displayOffset + _displayWidth] = color;
			_displayScreen[displayOffset + _displayWidth + 1] = color;
			break;

		default:
			break;
		}
	}

	/**
	 * This is used to put font pixels onto the screen - we adjust differently, so that we won't
	 *  do triple pixel lines in any case on upscaled hires. That way the font will not get distorted
	 *  Sierra SCI didn't do this
	 */
	void putFontPixel(int16 startingY, int16 x, int16 y, byte color) {
		int16 actualY = startingY + y;
		if (_fontIsUpscaled) {
			// Do not scale ourselves, but put it on the display directly
			putPixelOnDisplay(x, actualY, color);
		} else {
			if (_upscaledHires == GFX_SCREEN_UPSCALED_480x300) {
				putPixel480x300(x, actualY, GFX_SCREEN_MASK_VISUAL, color, 0, 0);
				return;
			}

			int offset = actualY * _width + x;

			_visualScreen[offset] = color;
			switch (_upscaledHires) {
			case GFX_SCREEN_UPSCALED_DISABLED:
				_displayScreen[offset] = color;
				break;
			case GFX_SCREEN_UPSCALED_640x400: {
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
				putScaledPixelOnDisplay(x, actualY, color);
				break;
			}
		}
	}

	byte getPixel(byte *screen, int16 x, int16 y) {
		switch (_upscaledHires) {
		case GFX_SCREEN_UPSCALED_480x300: {
			int offset = ((y * 3) / 2) * _width + ((y * 3) / 2);

			return screen[offset];
			break;
		}
		default:
			break;
		}
		return screen[y * _width + x];
	}

	byte getVisual(int16 x, int16 y) {
		return getPixel(_visualScreen, x, y);
	}
	byte getPriority(int16 x, int16 y) {
		return getPixel(_priorityScreen, x, y);
	}
	byte getControl(int16 x, int16 y) {
		return getPixel(_controlScreen, x, y);
	}

	// Vector related public code - in here, so that it can be inlined
	byte vectorGetPixel(byte *screen, int16 x, int16 y) {
		return screen[y * _width + x];
	}

	byte vectorGetVisual(int16 x, int16 y) {
		return vectorGetPixel(_visualScreen, x, y);
	}
	byte vectorGetPriority(int16 x, int16 y) {
		return vectorGetPixel(_priorityScreen, x, y);
	}
	byte vectorGetControl(int16 x, int16 y) {
		return vectorGetPixel(_controlScreen, x, y);
	}

	void vectorAdjustCoordinate(int16 *x, int16 *y) {
		switch (_upscaledHires) {
		case GFX_SCREEN_UPSCALED_480x300:
			*x = (*x * 3) / 2;
			*y = (*y * 3) / 2;
			break;
		default:
			break;
		}
	}

	bool hasActiveHiresView() const { return _activeHiresView; }
	void toggleActiveHiresView(bool toggle) { _activeHiresView = toggle; }
};

} // End of namespace Sci

#endif // SCI_GRAPHICS_SCREEN_H
