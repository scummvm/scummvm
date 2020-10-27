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

#include "common/config-manager.h"
#include "common/util.h"
#include "common/system.h"
#include "common/timer.h"
#include "graphics/surface.h"
#include "graphics/palette.h"
#include "graphics/cursorman.h"
#include "engines/util.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/view.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/scifx.h"

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

	_curPaletteMapValue = 0;
	_paletteModsEnabled = false;

	// King's Quest 6 and Gabriel Knight 1 have hires content, gk1/cd was able
	// to provide that under DOS as well, but as gk1/floppy does support
	// upscaled hires scriptswise, but doesn't actually have the hires content
	// we need to limit it to platform windows.
	if ((g_sci->getPlatform() == Common::kPlatformWindows) || (g_sci->forceHiresGraphics())) {
		if (g_sci->getGameId() == GID_KQ6)
			_upscaledHires = GFX_SCREEN_UPSCALED_640x440;
	}

	// Korean versions of games use hi-res font on upscaled version of the game.
	if ((g_sci->getLanguage() == Common::KO_KOR) && (getSciVersion() <= SCI_VERSION_1_1))
		_upscaledHires = GFX_SCREEN_UPSCALED_640x400;
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

	// Set up palette mods if requested
	if (ConfMan.getBool("palette_mods")) {
		setupCustomPaletteMods(this);
		ConfMan.setBool("rgb_rendering", true);
	}

	// Initialize the actual screen
	Graphics::PixelFormat format8 = Graphics::PixelFormat::createFormatCLUT8();
	const Graphics::PixelFormat *format = &format8;
	if (ConfMan.getBool("rgb_rendering"))
		format = 0; // Backend's preferred mode; RGB if available

	if (g_sci->hasMacIconBar()) {
		// For SCI1.1 Mac games with the custom icon bar, we need to expand the screen
		// to accommodate for the icon bar. Of course, both KQ6 and QFG1 VGA differ in size.
		// We add 2 to the height of the icon bar to add a buffer between the screen and the
		// icon bar (as did the original interpreter).
		if (g_sci->getGameId() == GID_KQ6)
			initGraphics(_displayWidth, _displayHeight + 26 + 2, format);
		else if (g_sci->getGameId() == GID_FREDDYPHARKAS)
			initGraphics(_displayWidth, _displayHeight + 28 + 2, format);
		else
			error("Unknown SCI1.1 Mac game");
	} else
		initGraphics(_displayWidth, _displayHeight, format);


	_format = g_system->getScreenFormat();

	// If necessary, allocate buffers for RGB mode
	if (_format.bytesPerPixel != 1) {
		_displayedScreen = (byte *)calloc(_displayPixels, 1);
		_rgbScreen = (byte *)calloc(_format.bytesPerPixel*_displayPixels, 1);
		_palette = new byte[3*256];

		if (_paletteModsEnabled)
			_paletteMapScreen = (byte *)calloc(_displayPixels, 1);
		else
			_paletteMapScreen = 0;
	} else {
		_displayedScreen = 0;
		_palette = 0;
		_rgbScreen = 0;
		_paletteMapScreen = 0;
	}
	_backupScreen = 0;
}

GfxScreen::~GfxScreen() {
	free(_visualScreen);
	free(_priorityScreen);
	free(_controlScreen);
	free(_displayScreen);

	free(_paletteMapScreen);
	free(_displayedScreen);
	free(_rgbScreen);
	delete[] _palette;
	delete[] _backupScreen;
}

void GfxScreen::convertToRGB(const Common::Rect &rect) {
	assert(_format.bytesPerPixel != 1);

	for (int y = rect.top; y < rect.bottom; ++y) {

		const byte *in = _displayedScreen + y * _displayWidth + rect.left;
		byte *out = _rgbScreen + (y * _displayWidth + rect.left) * _format.bytesPerPixel;

		// TODO: Reduce code duplication here

		if (_format.bytesPerPixel == 2) {

			if (_paletteMapScreen) {
				const byte *mod = _paletteMapScreen + y * _displayWidth + rect.left;
				for (int x = 0; x < rect.width(); ++x) {
					byte i = *in;
					byte r = _palette[3*i + 0];
					byte g = _palette[3*i + 1];
					byte b = _palette[3*i + 2];

					if (*mod) {
						r = MIN(r * (128 + _paletteMods[*mod].r) / 128, 255);
						g = MIN(g * (128 + _paletteMods[*mod].g) / 128, 255);
						b = MIN(b * (128 + _paletteMods[*mod].b) / 128, 255);
					}

					uint16 c = (uint16)_format.RGBToColor(r, g, b);
					WRITE_UINT16(out, c);
					in += 1;
					out += 2;
					mod += 1;
				}
			} else {
				for (int x = 0; x < rect.width(); ++x) {
					byte i = *in;
					byte r = _palette[3*i + 0];
					byte g = _palette[3*i + 1];
					byte b = _palette[3*i + 2];
					uint16 c = (uint16)_format.RGBToColor(r, g, b);
					WRITE_UINT16(out, c);
					in += 1;
					out += 2;
				}
			}

		} else {
			assert(_format.bytesPerPixel == 4);

			if (_paletteMapScreen) {
				const byte *mod = _paletteMapScreen + y * _displayWidth + rect.left;
				for (int x = 0; x < rect.width(); ++x) {
					byte i = *in;
					byte r = _palette[3*i + 0];
					byte g = _palette[3*i + 1];
					byte b = _palette[3*i + 2];

					if (*mod) {
						r = MIN(r * (128 + _paletteMods[*mod].r) / 128, 255);
						g = MIN(g * (128 + _paletteMods[*mod].g) / 128, 255);
						b = MIN(b * (128 + _paletteMods[*mod].b) / 128, 255);
					}

					uint32 c = _format.RGBToColor(r, g, b);
					WRITE_UINT32(out, c);
					in += 1;
					out += 4;
					mod += 1;
				}
			} else {
				for (int x = 0; x < rect.width(); ++x) {
					byte i = *in;
					byte r = _palette[3*i + 0];
					byte g = _palette[3*i + 1];
					byte b = _palette[3*i + 2];
					uint32 c = _format.RGBToColor(r, g, b);
					WRITE_UINT32(out, c);
					in += 1;
					out += 4;
				}
			}
		}
	}
}

void GfxScreen::displayRectRGB(const Common::Rect &rect, int x, int y) {
	// Display rect from _activeScreen to screen location x, y.
	// Clipping is assumed to be done already.

	Common::Rect targetRect;
	targetRect.left = x;
	targetRect.setWidth(rect.width());
	targetRect.top = y;
	targetRect.setHeight(rect.height());

	// 1. Update _displayedScreen
	for (int i = 0; i < rect.height(); ++i) {
		int offset = (rect.top + i) * _displayWidth + rect.left;
		int targetOffset = (targetRect.top + i) * _displayWidth + targetRect.left;
		memcpy(_displayedScreen + targetOffset, _activeScreen + offset, rect.width());
	}

	// 2. Convert to RGB
	convertToRGB(targetRect);

	// 3. Copy to screen
	g_system->copyRectToScreen(_rgbScreen + (targetRect.top * _displayWidth + targetRect.left) * _format.bytesPerPixel, _displayWidth * _format.bytesPerPixel, targetRect.left, targetRect.top, targetRect.width(), targetRect.height());
}

void GfxScreen::displayRect(const Common::Rect &rect, int x, int y) {
	// Display rect from _activeScreen to screen location x, y.
	// Clipping is assumed to be done already.

	if (_format.bytesPerPixel == 1) {
		g_system->copyRectToScreen(_activeScreen + rect.top * _displayWidth + rect.left, _displayWidth, x, y, rect.width(), rect.height());
	} else {
		displayRectRGB(rect, x, y);
	}
}


// should not be used regularly; only meant for restore game
void GfxScreen::clearForRestoreGame() {
	// reset all screen data
	memset(_visualScreen, 0, _pixels);
	memset(_priorityScreen, 0, _pixels);
	memset(_controlScreen, 0, _pixels);
	memset(_displayScreen, 0, _displayPixels);
	if (_displayedScreen) {
		memset(_displayedScreen, 0, _displayPixels);
		memset(_rgbScreen, 0, _format.bytesPerPixel*_displayPixels);
		if (_paletteMapScreen)
			memset(_paletteMapScreen, 0, _displayPixels);
	}
	memset(&_ditheredPicColors, 0, sizeof(_ditheredPicColors));
	_fontIsUpscaled = false;
	copyToScreen();
}

void GfxScreen::copyToScreen() {
	Common::Rect r(0, 0, _displayWidth, _displayHeight);
	displayRect(r, 0, 0);
}

void GfxScreen::copyVideoFrameToScreen(const byte *buffer, int pitch, const Common::Rect &rect, bool is8bit) {
	if (_format.bytesPerPixel == 1 || !is8bit) {
		g_system->copyRectToScreen(buffer, pitch, rect.left, rect.top, rect.width(), rect.height());
	} else {
		for (int i = 0; i < rect.height(); ++i) {
			int offset = i * pitch;
			int targetOffset = (rect.top + i) * _displayWidth + rect.left;
			memcpy(_displayedScreen + targetOffset, buffer + offset, rect.width());
		}
		convertToRGB(rect);
		g_system->copyRectToScreen(_rgbScreen + (rect.top * _displayWidth + rect.left) * _format.bytesPerPixel, _displayWidth * _format.bytesPerPixel, rect.left, rect.top, rect.width(), rect.height());
	}
}

void GfxScreen::kernelSyncWithFramebuffer() {
	if (_format.bytesPerPixel == 1) {
		Graphics::Surface *screen = g_system->lockScreen();
		const byte *pix = (const byte *)screen->getPixels();
		for (int y = 0; y < _displayHeight; ++y)
			memcpy(_displayScreen + y * _displayWidth, pix + y * screen->pitch, _displayWidth);
		g_system->unlockScreen();
	} else {
		memcpy(_displayScreen, _displayedScreen, _displayPixels);
	}
}

void GfxScreen::copyRectToScreen(const Common::Rect &rect) {
	if (!_upscaledHires)  {
		displayRect(rect, rect.left, rect.top);
	} else {
		int rectHeight = _upscaledHeightMapping[rect.bottom] - _upscaledHeightMapping[rect.top];
		int rectWidth  = _upscaledWidthMapping[rect.right] - _upscaledWidthMapping[rect.left];

		Common::Rect r;
		r.left =  _upscaledWidthMapping[rect.left];
		r.top = _upscaledHeightMapping[rect.top];
		r.setWidth(rectWidth);
		r.setHeight(rectHeight);
		displayRect(r, r.left, r.top);
	}
}

/**
 * This copies a rect to screen w/o scaling adjustment and is only meant to be
 * used on hires graphics used in upscaled hires mode.
 */
void GfxScreen::copyDisplayRectToScreen(const Common::Rect &rect) {
	if (!_upscaledHires)
		error("copyDisplayRectToScreen: not in upscaled hires mode");

	displayRect(rect, rect.left, rect.top);
}

void GfxScreen::copyRectToScreen(const Common::Rect &rect, int16 x, int16 y) {
	if (!_upscaledHires)  {
		displayRect(rect, x, y);
	} else {
		int rectHeight = _upscaledHeightMapping[rect.bottom] - _upscaledHeightMapping[rect.top];
		int rectWidth  = _upscaledWidthMapping[rect.right] - _upscaledWidthMapping[rect.left];

		Common::Rect r;
		r.left =  _upscaledWidthMapping[rect.left];
		r.top = _upscaledHeightMapping[rect.top];
		r.setWidth(rectWidth);
		r.setHeight(rectHeight);
		displayRect(r, _upscaledWidthMapping[x], _upscaledHeightMapping[y]);
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

void GfxScreen::vectorAdjustLineCoordinates(int16 *left, int16 *top, int16 *right, int16 *bottom, byte drawMask, byte color, byte priority, byte control) {
	switch (_upscaledHires) {
	case GFX_SCREEN_UPSCALED_480x300: {
		int16 displayLeft = (*left * 3) / 2;
		int16 displayRight = (*right * 3) / 2;
		int16 displayTop = (*top * 3) / 2;
		int16 displayBottom = (*bottom * 3) / 2;

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
		break;
	}
	default:
		break;
	}
}

// This is called from vector drawing to put a pixel at a certain location
void GfxScreen::vectorPutLinePixel(int16 x, int16 y, byte drawMask, byte color, byte priority, byte control) {
	if (_upscaledHires == GFX_SCREEN_UPSCALED_480x300) {
		vectorPutLinePixel480x300(x, y, drawMask, color, priority, control);
		return;
	}

	// For anything else forward to the regular putPixel
	putPixel(x, y, drawMask, color, priority, control);
}

// Special 480x300 Mac putPixel for vector line drawing, also draws an additional pixel below the actual one
void GfxScreen::vectorPutLinePixel480x300(int16 x, int16 y, byte drawMask, byte color, byte priority, byte control) {
	int offset = y * _width + x;

	if (drawMask & GFX_SCREEN_MASK_VISUAL) {
		// also set pixel below actual pixel
		_visualScreen[offset] = color;
		_visualScreen[offset + _width] = color;
		_displayScreen[offset] = color;
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

byte GfxScreen::vectorIsFillMatch(int16 x, int16 y, byte screenMask, byte checkForColor, byte checkForPriority, byte checkForControl, bool isEGA) {
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

// We put hires hangul chars onto upscaled background, so we need to adjust
// coordinates. Caller gives use low-res ones.
void GfxScreen::putHangulChar(Graphics::FontKorean *commonFont, int16 x, int16 y, uint16 chr, byte color) {
	byte *displayPtr = _displayScreen + y * _displayWidth * 2 + x * 2;
	// we don't use outline, so color 0 is actually not used
	commonFont->drawChar(displayPtr, chr, _displayWidth, 1, color, 0, -1, -1);
}

// We put hires kanji chars onto upscaled background, so we need to adjust
// coordinates. Caller gives use low-res ones.
void GfxScreen::putKanjiChar(Graphics::FontSJIS *commonFont, int16 x, int16 y, uint16 chr, byte color) {
	byte *displayPtr = _displayScreen + y * _displayWidth * 2 + x * 2;
	// we don't use outline, so color 0 is actually not used
	commonFont->drawChar(displayPtr, chr, _displayWidth, 1, color, 0, -1, -1);
}

int GfxScreen::bitsGetDataSize(Common::Rect rect, byte mask) {
	int byteCount = sizeof(rect) + sizeof(mask);
	int pixels = rect.width() * rect.height();
	if (mask & GFX_SCREEN_MASK_VISUAL) {
		byteCount += pixels; // _visualScreen
		if (!_upscaledHires) {
			byteCount += pixels; // _displayScreen
			if (_paletteMapScreen)
				byteCount += pixels; // _paletteMapScreen
		} else {
			int rectHeight = _upscaledHeightMapping[rect.bottom] - _upscaledHeightMapping[rect.top];
			int rectWidth = _upscaledWidthMapping[rect.right] - _upscaledWidthMapping[rect.left];
			byteCount += rectHeight * rectWidth; // _displayScreen (upscaled hires)
			if (_paletteMapScreen)
				byteCount += rectHeight * rectWidth; // _paletteMapScreen (upscaled hires)
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
		if (_paletteMapScreen)
			byteCount += pixels; // _paletteMapScreen
	}
	return byteCount;
}

void GfxScreen::bitsSave(Common::Rect rect, byte mask, byte *memoryPtr) {
	memcpy(memoryPtr, (void *)&rect, sizeof(rect)); memoryPtr += sizeof(rect);
	memcpy(memoryPtr, (void *)&mask, sizeof(mask)); memoryPtr += sizeof(mask);

	if (mask & GFX_SCREEN_MASK_VISUAL) {
		bitsSaveScreen(rect, _visualScreen, _width, memoryPtr);
		bitsSaveDisplayScreen(rect, _displayScreen, memoryPtr);
		if (_paletteMapScreen)
			bitsSaveDisplayScreen(rect, _paletteMapScreen, memoryPtr);
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
		if (_paletteMapScreen)
			bitsSaveScreen(rect, _paletteMapScreen, _displayWidth, memoryPtr);
	}
}

void GfxScreen::bitsSaveScreen(Common::Rect rect, const byte *screen, uint16 screenWidth, byte *&memoryPtr) {
	int width = rect.width();
	int y;

	screen += (rect.top * screenWidth) + rect.left;

	for (y = rect.top; y < rect.bottom; y++) {
		memcpy(memoryPtr, screen, width); memoryPtr += width;
		screen += screenWidth;
	}
}

void GfxScreen::bitsSaveDisplayScreen(Common::Rect rect, const byte *screen, byte *&memoryPtr) {
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
		memcpy(memoryPtr, screen, width); memoryPtr += width;
		screen += _displayWidth;
	}
}

void GfxScreen::bitsGetRect(const byte *memoryPtr, Common::Rect *destRect) {
	memcpy(destRect, memoryPtr, sizeof(Common::Rect));
}

void GfxScreen::bitsRestore(const byte *memoryPtr) {
	Common::Rect rect;
	byte mask;

	memcpy((void *)&rect, memoryPtr, sizeof(rect)); memoryPtr += sizeof(rect);
	memcpy((void *)&mask, memoryPtr, sizeof(mask)); memoryPtr += sizeof(mask);

	if (mask & GFX_SCREEN_MASK_VISUAL) {
		bitsRestoreScreen(rect, memoryPtr, _visualScreen, _width);
		bitsRestoreDisplayScreen(rect, memoryPtr, _displayScreen);
		if (_paletteMapScreen)
			bitsRestoreDisplayScreen(rect, memoryPtr, _paletteMapScreen);
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
		if (_paletteMapScreen)
			bitsRestoreScreen(rect, memoryPtr, _paletteMapScreen, _displayWidth);

		// WORKAROUND - we are not sure what sierra is doing. If we don't do this here, portraits won't get fully removed
		//  from screen. Some lowres showBits() call is used for that and it's not covering the whole area
		//  We would need to find out inside the kq6 windows interpreter, but this here works already and seems not to have
		//  any side-effects. The whole hires is hacked into the interpreter, so maybe this is even right.
		copyDisplayRectToScreen(rect);
	}
}

void GfxScreen::bitsRestoreScreen(Common::Rect rect, const byte *&memoryPtr, byte *screen, uint16 screenWidth) {
	int width = rect.width();
	int y;

	screen += (rect.top * screenWidth) + rect.left;

	for (y = rect.top; y < rect.bottom; y++) {
		memcpy((void *) screen, memoryPtr, width); memoryPtr += width;
		screen += screenWidth;
	}
}

void GfxScreen::bitsRestoreDisplayScreen(Common::Rect rect, const byte *&memoryPtr, byte *screen) {
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

void GfxScreen::setShakePos(uint16 shakeXOffset, uint16 shakeYOffset) {
	if (!_upscaledHires)
		g_system->setShakePos(shakeXOffset, shakeYOffset);
	else
		g_system->setShakePos(_upscaledWidthMapping[shakeXOffset], _upscaledHeightMapping[shakeYOffset]);
}

void GfxScreen::kernelShakeScreen(uint16 shakeCount, uint16 directions) {
	while (shakeCount--) {

		uint16 shakeXOffset = 0;
		if (directions & kShakeHorizontal) {
			shakeXOffset = 10;
		}

		uint16 shakeYOffset = 0;
		if (directions & kShakeVertical) {
			shakeYOffset = 10;
		}

		setShakePos(shakeXOffset, shakeYOffset);

		g_system->updateScreen();
		g_sci->getEngineState()->sleep(3);

		setShakePos(0, 0);

		g_system->updateScreen();
		g_sci->getEngineState()->sleep(3);
	}
}

void GfxScreen::dither(bool addToFlag) {
	int y, x;
	byte color, ditheredColor;
	byte *visualPtr = _visualScreen;
	byte *displayPtr = _displayScreen;
	byte *paletteMapPtr = _paletteMapScreen;

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
						if (_paletteMapScreen)
							*paletteMapPtr = _curPaletteMapValue;
						break;
					default:
						putScaledPixelOnDisplay(x, y, color);
						break;
					}
					*visualPtr = color;
				}
				visualPtr++; displayPtr++; paletteMapPtr++;
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
						if (_paletteMapScreen)
							*paletteMapPtr = _curPaletteMapValue;
						break;
					default:
						putScaledPixelOnDisplay(x, y, ditheredColor);
						break;
					}
					color = ((x^y) & 1) ? color >> 4 : color & 0x0F;
					*visualPtr = color;
				}
				visualPtr++; displayPtr++; paletteMapPtr++;
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
	default:
		break;
	}
	copyToScreen();
}

void GfxScreen::scale2x(const SciSpan<const byte> &src, SciSpan<byte> &dst, int16 srcWidth, int16 srcHeight, byte bytesPerPixel) {
	assert(bytesPerPixel == 1 || bytesPerPixel == 2);
	const int newWidth = srcWidth * 2;
	const int pitch = newWidth * bytesPerPixel;
	const byte *srcPtr = src.getUnsafeDataAt(0, srcWidth * srcHeight * bytesPerPixel);
	byte *dstPtr = dst.getUnsafeDataAt(0, srcWidth * srcHeight * bytesPerPixel);

	if (bytesPerPixel == 1) {
		for (int y = 0; y < srcHeight; y++) {
			for (int x = 0; x < srcWidth; x++) {
				const byte color = *srcPtr++;
				dstPtr[0] = color;
				dstPtr[1] = color;
				dstPtr[newWidth] = color;
				dstPtr[newWidth + 1] = color;
				dstPtr += 2;
			}
			dstPtr += newWidth;
		}
	} else if (bytesPerPixel == 2) {
		for (int y = 0; y < srcHeight; y++) {
			for (int x = 0; x < srcWidth; x++) {
				const byte color = *srcPtr++;
				const byte color2 = *srcPtr++;
				dstPtr[0] = color;
				dstPtr[1] = color2;
				dstPtr[2] = color;
				dstPtr[3] = color2;
				dstPtr[pitch] = color;
				dstPtr[pitch + 1] = color2;
				dstPtr[pitch + 2] = color;
				dstPtr[pitch + 3] = color2;
				dstPtr += 4;
			}
			dstPtr += pitch;
		}
	}
}

struct UpScaledAdjust {
	GfxScreenUpscaledMode gameHiresMode;
	int numerator;
	int denominator;
};

void GfxScreen::adjustToUpscaledCoordinates(int16 &y, int16 &x) {
	x = _upscaledWidthMapping[x];
	y = _upscaledHeightMapping[y];
}

void GfxScreen::adjustBackUpscaledCoordinates(int16 &y, int16 &x) {
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


void GfxScreen::grabPalette(byte *buffer, uint start, uint num) const {
	assert(start + num <= 256);
	if (_format.bytesPerPixel == 1) {
		g_system->getPaletteManager()->grabPalette(buffer, start, num);
	} else {
		memcpy(buffer, _palette + 3*start, 3*num);
	}
}

void GfxScreen::setPalette(const byte *buffer, uint start, uint num, bool update) {
	assert(start + num <= 256);
	if (_format.bytesPerPixel == 1) {
		g_system->getPaletteManager()->setPalette(buffer, start, num);
	} else {
		memcpy(_palette + 3*start, buffer, 3*num);
		if (update) {
			// directly paint from _displayedScreen, not from _activeScreen
			Common::Rect r(0, 0, _displayWidth, _displayHeight);
			convertToRGB(r);
			g_system->copyRectToScreen(_rgbScreen, _displayWidth * _format.bytesPerPixel, 0, 0, _displayWidth, _displayHeight);
		}
		// CHECKME: Inside or outside the if (update)?
		// (The !update case only happens inside transitions.)
		CursorMan.replaceCursorPalette(_palette, 0, 256);
	}
}


void GfxScreen::bakCreateBackup() {
	assert(!_backupScreen);
	_backupScreen = new byte[_format.bytesPerPixel * _displayPixels];
	if (_format.bytesPerPixel == 1) {
		Graphics::Surface *screen = g_system->lockScreen();
		memcpy(_backupScreen, screen->getPixels(), _displayPixels);
		g_system->unlockScreen();
	} else {
		memcpy(_backupScreen, _rgbScreen, _format.bytesPerPixel * _displayPixels);
	}
}

void GfxScreen::bakDiscard() {
	assert(_backupScreen);
	delete[] _backupScreen;
	_backupScreen = nullptr;
}

void GfxScreen::bakCopyRectToScreen(const Common::Rect &rect, int16 x, int16 y) {
	assert(_backupScreen);
	const byte *ptr = _backupScreen;
	ptr += _format.bytesPerPixel * (rect.left + rect.top * _displayWidth);
	g_system->copyRectToScreen(ptr, _format.bytesPerPixel * _displayWidth, x, y, rect.width(), rect.height());
}

void GfxScreen::setPaletteMods(const PaletteMod *mods, unsigned int count) {
	assert(count < 256);
	for (unsigned int i = 0; i < count; ++i)
		_paletteMods[i] = mods[i];

	_paletteModsEnabled = true;
}



} // End of namespace Sci
