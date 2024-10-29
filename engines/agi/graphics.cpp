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

#include "common/config-manager.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "engines/util.h"

#include "graphics/cursorman.h"
#include "graphics/paletteman.h"

#include "agi/agi.h"
#include "agi/graphics.h"
#include "agi/mouse_cursor.h"
#include "agi/palette.h"
#include "agi/picture.h"
#include "agi/text.h"

namespace Agi {

#include "agi/font.h"

GfxMgr::GfxMgr(AgiBase *vm, GfxFont *font) : _vm(vm), _font(font) {
	memset(&_paletteGfxMode, 0, sizeof(_paletteGfxMode));
	memset(&_paletteTextMode, 0, sizeof(_paletteTextMode));

	memset(&_agipalPalette, 0, sizeof(_agipalPalette));
	_agipalFileNum = 0;

	memset(&_mouseCursor, 0, sizeof(_mouseCursor));
	memset(&_mouseCursorBusy, 0, sizeof(_mouseCursorBusy));

	initPriorityTable();

	_renderStartVisualOffsetY = 0;
	_renderStartDisplayOffsetY = 0;

	_upscaledHires = DISPLAY_UPSCALED_DISABLED;
	_displayScreenWidth = DISPLAY_DEFAULT_WIDTH;
	_displayScreenHeight = DISPLAY_DEFAULT_HEIGHT;
	_displayFontWidth = 8;
	_displayFontHeight = 8;

	_displayWidthMulAdjust = 0; // visualPos * (2+0) = displayPos
	_displayHeightMulAdjust = 0; // visualPos * (1+0) = displayPos

	_pixels = 0;
	_displayPixels = 0;

	_activeScreen = nullptr;
	_gameScreen = nullptr;
	_priorityScreen = nullptr;
	_displayScreen = nullptr;
}

/**
 * Initialize graphics device.
 */
void GfxMgr::initVideo() {
	bool forceHires = false;

	// Set up palettes
	initPalette(_paletteTextMode, PALETTE_EGA);

	switch (_vm->_renderMode) {
	case Common::kRenderEGA:
		initPalette(_paletteGfxMode, PALETTE_EGA);
		break;
	case Common::kRenderCGA:
		initPalette(_paletteGfxMode, PALETTE_CGA, 4, 8);
		break;
	case Common::kRenderVGA:
		initPalette(_paletteGfxMode, PALETTE_VGA, 256, 8);
		break;
	case Common::kRenderHercG:
		initPalette(_paletteGfxMode, PALETTE_HERCULES_GREEN, 2, 8);
		forceHires = true;
		break;
	case Common::kRenderHercA:
		initPalette(_paletteGfxMode, PALETTE_HERCULES_AMBER, 2, 8);
		forceHires = true;
		break;
	case Common::kRenderAmiga:
		if (!ConfMan.getBool("altamigapalette")) {
			// Set the correct Amiga palette depending on AGI interpreter version
			if (_vm->getVersion() < 0x2936)
				initPalette(_paletteGfxMode, PALETTE_AMIGA_V1, 16, 4);
			else if (_vm->getVersion() == 0x2936)
				initPalette(_paletteGfxMode, PALETTE_AMIGA_V2, 16, 4);
			else
				initPalette(_paletteGfxMode, PALETTE_AMIGA_V3, 16, 4);
		} else {
			// Set the old common alternative Amiga palette
			initPalette(_paletteGfxMode, PALETTE_AMIGA_ALT);
		}
		break;
	case Common::kRenderApple2GS:
		switch (_vm->getGameID()) {
		case GID_SQ1:
			// Special one, only used for Space Quest 1 on Apple IIgs. Is the same as Amiga v1 palette
			initPalette(_paletteGfxMode, PALETTE_APPLE_II_GS_SQ1, 16, 4);
			break;
		default:
			// Regular "standard" Apple IIgs palette, used by everything else
			initPalette(_paletteGfxMode, PALETTE_APPLE_II_GS, 16, 4);
			break;
		}
		break;
	case Common::kRenderAtariST:
		initPalette(_paletteGfxMode, PALETTE_ATARI_ST, 16, 3);
		break;
	case Common::kRenderMacintosh:
		switch (_vm->getGameID()) {
		case GID_KQ3:
		case GID_PQ1:
			initPaletteCLUT(_paletteGfxMode, PALETTE_MACINTOSH_CLUT, 16);
			break;
		case GID_GOLDRUSH:
			// We use the common KQ3/PQ1 palette at the moment.
			// It seems the Gold Rush palette, that came with the game is quite ugly.
			initPaletteCLUT(_paletteGfxMode, PALETTE_MACINTOSH_CLUT, 16);
			break;
		case GID_SQ2:
			initPaletteCLUT(_paletteGfxMode, PALETTE_MACINTOSH_CLUT3, 16);
			break;
		default:
			initPaletteCLUT(_paletteGfxMode, PALETTE_MACINTOSH_CLUT3, 16);
			break;
		}
		break;
	default:
		error("initVideo: unsupported render mode: %d", _vm->_renderMode);
		break;
	}

	if (_font->isFontHires() || forceHires) {
		// Upscaling enable
		_upscaledHires = DISPLAY_UPSCALED_640x400;
		_displayScreenWidth = 640;
		_displayScreenHeight = 400;
		_displayFontWidth = 16;
		_displayFontHeight = 16;

		_displayWidthMulAdjust = 2;
		_displayHeightMulAdjust = 1;
	}

	// set up mouse cursors
	switch (_vm->_renderMode) {
	case Common::kRenderEGA:
	case Common::kRenderCGA:
	case Common::kRenderVGA:
	case Common::kRenderHercG:
	case Common::kRenderHercA:
		initMouseCursor(&_mouseCursor, MOUSECURSOR_SCI, 11, 16, 0, 0);
		initMouseCursor(&_mouseCursorBusy, MOUSECURSOR_SCI_BUSY, 15, 16, 7, 8);
		break;
	case Common::kRenderAmiga:
		initMouseCursor(&_mouseCursor, MOUSECURSOR_AMIGA, 8, 11, 0, 0);
		initMouseCursor(&_mouseCursorBusy, MOUSECURSOR_AMIGA_BUSY, 13, 16, 7, 8);
		break;
	case Common::kRenderApple2GS:
		// had no special busy mouse cursor
		initMouseCursor(&_mouseCursor, MOUSECURSOR_APPLE_II_GS, 9, 11, 0, 0);
		initMouseCursor(&_mouseCursorBusy, MOUSECURSOR_SCI_BUSY, 15, 16, 7, 8);
		break;
	case Common::kRenderAtariST:
		initMouseCursor(&_mouseCursor, MOUSECURSOR_ATARI_ST, 11, 16, 0, 0);
		initMouseCursor(&_mouseCursorBusy, MOUSECURSOR_SCI_BUSY, 15, 16, 7, 8);
		break;
	case Common::kRenderMacintosh:
		// It looks like Atari ST + Macintosh used the same standard mouse cursor
		// TODO: Verify by checking actual hardware
		initMouseCursor(&_mouseCursor, MOUSECURSOR_ATARI_ST, 11, 16, 0, 0);
		initMouseCursor(&_mouseCursorBusy, MOUSECURSOR_MACINTOSH_BUSY, 10, 14, 7, 8);
		break;
	default:
		error("initVideo: unsupported render mode: %d", _vm->_renderMode);
		break;
	}

	_pixels = SCRIPT_WIDTH * SCRIPT_HEIGHT;
	_gameScreen = (byte *)calloc(_pixels, 1);
	_priorityScreen = (byte *)calloc(_pixels, 1);
	_activeScreen = _gameScreen;
	//_activeScreen = _priorityScreen;

	_displayPixels = _displayScreenWidth * _displayScreenHeight;
	_displayScreen = (byte *)calloc(_displayPixels, 1);

	initGraphics(_displayScreenWidth, _displayScreenHeight);

	setPalette(true); // set gfx-mode palette

	// set up mouse cursor palette
	CursorMan.replaceCursorPalette(MOUSECURSOR_PALETTE, 1, ARRAYSIZE(MOUSECURSOR_PALETTE) / 3);
	setMouseCursor();
}

/**
 * Deinitialize graphics device.
 */
void GfxMgr::deinitVideo() {
	// Free mouse cursors in case they were allocated
	free(_mouseCursor.bitmapDataAllocated);
	free(_mouseCursorBusy.bitmapDataAllocated);

	free(_displayScreen);
	free(_gameScreen);
	free(_priorityScreen);
}

void GfxMgr::setRenderStartOffset(uint16 offsetY) {
	if (offsetY >= (VISUAL_HEIGHT - SCRIPT_HEIGHT))
		error("invalid render start offset");

	_renderStartVisualOffsetY = offsetY;
	_renderStartDisplayOffsetY = offsetY * (1 + _displayHeightMulAdjust);
}

uint16 GfxMgr::getRenderStartDisplayOffsetY() const {
	return _renderStartDisplayOffsetY;
}

// Translates a game screen coordinate to a display screen coordinate
// Game screen to 320x200 -> x * 2, y + renderStart
// Game screen to 640x400 -> x * 4, (y * 2) + renderStart
void GfxMgr::translateGamePosToDisplayScreen(int16 &x, int16 &y) const {
	x = x * (2 + _displayWidthMulAdjust);
	y = y * (1 + _displayHeightMulAdjust) + _renderStartDisplayOffsetY;
}

// Translates a visual coordinate to a display screen coordinate
// Visual to 320x200 -> x * 2, y
// Visual to 640x400 -> x * 4, y * 2
void GfxMgr::translateVisualPosToDisplayScreen(int16 &x, int16 &y) const {
	x = x * (2 + _displayWidthMulAdjust);
	y = y * (1 + _displayHeightMulAdjust);
}

// Translates a display screen coordinate to a game screen coordinate
// Display screen to 320x200 -> x / 2, y - renderStart
// Display screen to 640x400 -> x / 4, (y / 2) - renderStart
void GfxMgr::translateDisplayPosToGameScreen(int16 &x, int16 &y) const {
	y -= _renderStartDisplayOffsetY; // remove status bar line
	x = x / (2 + _displayWidthMulAdjust);
	y = y / (1 + _displayHeightMulAdjust);
	if (y < 0)
		y = 0;
	if (y >= SCRIPT_HEIGHT)
		y = SCRIPT_HEIGHT + 1; // 1 beyond
}

// Translates dimension from visual screen to display screen
void GfxMgr::translateVisualDimensionToDisplayScreen(int16 &width, int16 &height) const {
	width = width * (2 + _displayWidthMulAdjust);
	height = height * (1 + _displayHeightMulAdjust);
}

// Translates dimension from display screen to visual screen
void GfxMgr::translateDisplayDimensionToVisualScreen(int16 &width, int16 &height) const {
	width = width / (2 + _displayWidthMulAdjust);
	height = height / (1 + _displayHeightMulAdjust);
}

// Translates a rect from game screen to display screen
void GfxMgr::translateGameRectToDisplayScreen(int16 &x, int16 &y, int16 &width, int16 &height) const {
	translateGamePosToDisplayScreen(x, y);
	translateVisualDimensionToDisplayScreen(width, height);
}

// Translates a rect from visual screen to display screen
void GfxMgr::translateVisualRectToDisplayScreen(int16 &x, int16 &y, int16 &width, int16 &height) const {
	translateVisualPosToDisplayScreen(x, y);
	translateVisualDimensionToDisplayScreen(width, height);
}

uint32 GfxMgr::getDisplayOffsetToGameScreenPos(int16 x, int16 y) const {
	translateGamePosToDisplayScreen(x, y);
	return (y * _displayScreenWidth) + x;
}

uint32 GfxMgr::getDisplayOffsetToVisualScreenPos(int16 x, int16 y) const {
	translateVisualPosToDisplayScreen(x, y);
	return (y * _displayScreenWidth) + x;
}

// Attention: uses display screen coordinates!
void GfxMgr::copyDisplayRectToScreen(int16 x, int16 y, int16 width, int16 height) {
	// Clamp to sane values to prevent off screen blits causing exceptions in backend
	// FIXME: Add warnings / debug of clamping?
	width = CLIP<int16>(width, 0, _displayScreenWidth);
	height = CLIP<int16>(height, 0, _displayScreenHeight);
	x = CLIP<int16>(x, 0, _displayScreenWidth-width);
	y = CLIP<int16>(y, 0, _displayScreenHeight-height);

	_vm->_system->copyRectToScreen(_displayScreen + y * _displayScreenWidth + x, _displayScreenWidth, x, y, width, height);
}

void GfxMgr::copyDisplayRectToScreen(int16 x, int16 adjX, int16 y, int16 adjY, int16 width, int16 adjWidth, int16 height, int16 adjHeight) {
	switch (_upscaledHires) {
	case DISPLAY_UPSCALED_DISABLED:
		break;
	case DISPLAY_UPSCALED_640x400:
		adjX *= 2; adjY *= 2;
		adjWidth *= 2; adjHeight *= 2;
		break;
	default:
		assert(0);
		break;
	}
	x += adjX; y += adjY;
	width += adjWidth; height += adjHeight;
	_vm->_system->copyRectToScreen(_displayScreen + y * _displayScreenWidth + x, _displayScreenWidth, x, y, width, height);
}

void GfxMgr::copyDisplayRectToScreenUsingGamePos(int16 x, int16 y, int16 width, int16 height) {
	translateGameRectToDisplayScreen(x, y, width, height);
	_vm->_system->copyRectToScreen(_displayScreen + (y * _displayScreenWidth) + x, _displayScreenWidth, x, y, width, height);
}

void GfxMgr::copyDisplayRectToScreenUsingVisualPos(int16 x, int16 y, int16 width, int16 height) {
	translateVisualRectToDisplayScreen(x, y, width, height);
	_vm->_system->copyRectToScreen(_displayScreen + (y * _displayScreenWidth) + x, _displayScreenWidth, x, y, width, height);
}

void GfxMgr::copyDisplayToScreen() {
	_vm->_system->copyRectToScreen(_displayScreen, _displayScreenWidth, 0, 0, _displayScreenWidth, _displayScreenHeight);
}

void GfxMgr::translateFontPosToDisplayScreen(int16 &x, int16 &y) const {
	x *= _displayFontWidth;
	y *= _displayFontHeight;
}

void GfxMgr::translateDisplayPosToFontScreen(int16 &x, int16 &y) const {
	x /= _displayFontWidth;
	y /= _displayFontHeight;
}

void GfxMgr::translateFontDimensionToDisplayScreen(int16 &width, int16 &height) const {
	width *= _displayFontWidth;
	height *= _displayFontHeight;
}

void GfxMgr::translateFontRectToDisplayScreen(int16 &x, int16 &y, int16 &width, int16 &height) const {
	translateFontPosToDisplayScreen(x, y);
	translateFontDimensionToDisplayScreen(width, height);
}

Common::Rect GfxMgr::getFontRectForDisplayScreen(int16 column, int16 row, int16 width, int16 height) const {
	Common::Rect displayRect(width * _displayFontWidth, height * _displayFontHeight);
	displayRect.moveTo(column * _displayFontWidth, row * _displayFontHeight);
	return displayRect;
}

void GfxMgr::debugShowMap(int mapNr) {
	switch (mapNr) {
	case 0:
		_activeScreen = _gameScreen;
		break;
	case 1:
		_activeScreen = _priorityScreen;
		break;
	default:
		break;
	}

	render_Block(0, 0, SCRIPT_WIDTH, SCRIPT_HEIGHT);
}

/**
 * Clears the game and priority screens
 */
void GfxMgr::clear(byte color, byte priority) {
	memset(_gameScreen, color, _pixels);
	memset(_priorityScreen, priority, _pixels);
}

/**
 * Clears the display screen and copies it to screen
 */
void GfxMgr::clearDisplay(byte color, bool copyToScreen) {
	memset(_displayScreen, color, _displayPixels);

	if (copyToScreen) {
		copyDisplayToScreen();
	}
}

/**
 * Puts a pixel on the game and/or priority screens
 */
void GfxMgr::putPixel(int16 x, int16 y, byte drawMask, byte color, byte priority) {
	int offset = y * SCRIPT_WIDTH + x;

	if (drawMask & GFX_SCREEN_MASK_VISUAL) {
		_gameScreen[offset] = color;
	}
	if (drawMask & GFX_SCREEN_MASK_PRIORITY) {
		_priorityScreen[offset] = priority;
	}
}

/**
 * Puts a pixel on the display screen.
 * If upscaling is enabled then the pixel and coordinates are upscaled.
 */
void GfxMgr::putPixelOnDisplay(int16 x, int16 y, byte color) {
	uint32 offset = 0;

	switch (_upscaledHires) {
	case DISPLAY_UPSCALED_DISABLED:
		offset = y * _displayScreenWidth + x;

		_displayScreen[offset] = color;
		break;
	case DISPLAY_UPSCALED_640x400:
		offset = (y * _displayScreenWidth) + x;

		_displayScreen[offset + 0] = color;
		_displayScreen[offset + 1] = color;
		_displayScreen[offset + _displayScreenWidth + 0] = color;
		_displayScreen[offset + _displayScreenWidth + 1] = color;
		break;
	default:
		break;
	}
}

/**
 * Puts a pixel on the display screen.
 * If upscaling is enabled then the pixel and coordinates are upscaled.
 */
void GfxMgr::putPixelOnDisplay(int16 x, int16 adjX, int16 y, int16 adjY, byte color) {
	switch (_upscaledHires) {
	case DISPLAY_UPSCALED_DISABLED:
		break;
	case DISPLAY_UPSCALED_640x400:
		adjX *= 2; adjY *= 2;
		break;
	default:
		assert(0);
		break;
	}
	x += adjX;
	y += adjY;
	putPixelOnDisplay(x, y, color);
}

/**
 * Puts a font pixel on the display screen.
 * If upscaling is enabled then isHires determines if the pixel and coordinates
 * are to be used as is or upscaled.
 */
void GfxMgr::putFontPixelOnDisplay(int16 baseX, int16 baseY, int16 addX, int16 addY, byte color, bool isHires) {
	uint32 offset = 0;

	switch (_upscaledHires) {
	case DISPLAY_UPSCALED_DISABLED:
		offset = ((baseY + addY) * _displayScreenWidth) + (baseX + addX);
		_displayScreen[offset] = color;
		break;
	case DISPLAY_UPSCALED_640x400:
		if (isHires) {
			offset = ((baseY + addY) * _displayScreenWidth) + (baseX + addX);
			_displayScreen[offset] = color;
		} else {
			offset = ((baseY + addY * 2) * _displayScreenWidth) + (baseX + addX * 2);
			_displayScreen[offset + 0] = color;
			_displayScreen[offset + 1] = color;
			_displayScreen[offset + _displayScreenWidth + 0] = color;
			_displayScreen[offset + _displayScreenWidth + 1] = color;
		}
		break;
	default:
		break;
	}
}

/**
 * Returns a color from the game screen
 */
byte GfxMgr::getColor(int16 x, int16 y) const {
	int offset = y * SCRIPT_WIDTH + x;

	return _gameScreen[offset];
}

/**
 * Returns a priority from the priority screen
 */
byte GfxMgr::getPriority(int16 x, int16 y) const {
	int offset = y * SCRIPT_WIDTH + x;

	return _priorityScreen[offset];
}

// used, when a control pixel is found
// will search downwards and compare priority in case any is found
bool GfxMgr::checkControlPixel(int16 x, int16 y, byte viewPriority) const {
	int offset = y * SCRIPT_WIDTH + x;
	byte curPriority;

	while (1) {
		y++;
		offset += SCRIPT_WIDTH;
		if (y >= SCRIPT_HEIGHT) {
			// end of screen, nothing but control pixels found
			return true; // draw view pixel
		}
		curPriority = _priorityScreen[offset];
		if (curPriority > 2) // valid priority found?
			break;
	}
	if (curPriority <= viewPriority)
		return true; // view priority is higher, draw
	return false; // view priority is lower, don't draw
}

static const byte CGA_MixtureColorTable[] = {
	0x00, 0x08, 0x04, 0x0C, 0x01, 0x09, 0x02, 0x05,
	0x0A, 0x0D, 0x06, 0x0E, 0x0B, 0x03, 0x07, 0x0F
};

byte GfxMgr::getCGAMixtureColor(byte color) const {
	return CGA_MixtureColorTable[color & 0x0F];
}

/**
 * Renders a block of the game screen on to the display screen.
 * Optionally copies the display block to screen immediately.
 */
// Attention: in our implementation, y-coordinate is upper left.
// Sierra passed the lower left instead. We changed it to make upscaling easier.
void GfxMgr::render_Block(int16 x, int16 y, int16 width, int16 height, bool copyToScreen) {
	if (!render_Clip(x, y, width, height, 0, SCRIPT_WIDTH, SCRIPT_HEIGHT)) {
		warning("render_Block ignored by clipping. x: %d, y: %d, w: %d, h: %d", x, y, width, height);
		return;
	}

	switch (_vm->_renderMode) {
	case Common::kRenderHercG:
	case Common::kRenderHercA:
		render_BlockHercules(x, y, width, height);
		break;
	case Common::kRenderCGA:
		render_BlockCGA(x, y, width, height);
		break;
	case Common::kRenderEGA:
	default:
		render_BlockEGA(x, y, width, height);
		break;
	}

	if (copyToScreen) {
		copyDisplayRectToScreenUsingGamePos(x, y, width, height);
	}
}

// FIXME: This function needs clarification and cleanup. Half of the code is
// logically dead or disabled. Is the purpose to adjust out-of-bounds coordinates
// so that they fit within boundaries, or is it to identify out-of-bounds
// coordinates so that the entire drawing operation can be rejected?
bool GfxMgr::render_Clip(int16 &x, int16 &y, int16 &width, int16 &height, const int16 minY, const int16 clipAgainstWidth, const int16 clipAgainstHeight) {
	if ((x >= clipAgainstWidth) || ((x + width - 1) < 0) ||
	        (y < minY) || ((y + (height - 1)) >= clipAgainstHeight)) {
		return false;
	}

	// FIXME: this check is always false, see above
	if (y < minY) {
		height += y;
		y = minY;
	}

	// FIXME: this check is always false, see above
	if ((y + height - 1) >= clipAgainstHeight) {
		height = clipAgainstHeight - y;
	}

	// FIXME: why is this disabled?
#if 0
	if ((y - height + 1) < 0)
		height = y + 1;

	if (y >= clipAgainstHeight) {
		height -= y - (clipAgainstHeight - 1);
		y = clipAgainstHeight - 1;
	}
#endif

	if (x < 0) {
		width += x;
		x = 0;
	}

	if ((x + width - 1) >= clipAgainstWidth) {
		width = clipAgainstWidth - x;
	}
	return true;
}

void GfxMgr::render_BlockEGA(int16 x, int16 y, int16 width, int16 height) {
	uint32 offsetVisual = SCRIPT_WIDTH * y + x;
	uint32 offsetDisplay = getDisplayOffsetToGameScreenPos(x, y);
	int16 remainingHeight = height;
	byte curColor = 0;
	int16 displayWidth = width * (2 + _displayWidthMulAdjust);

	while (remainingHeight) {
		int16 remainingWidth = width;

		switch (_upscaledHires) {
		case DISPLAY_UPSCALED_DISABLED:
			while (remainingWidth) {
				curColor = _activeScreen[offsetVisual++];
				_displayScreen[offsetDisplay++] = curColor;
				_displayScreen[offsetDisplay++] = curColor;
				remainingWidth--;
			}
			break;
		case DISPLAY_UPSCALED_640x400:
			while (remainingWidth) {
				curColor = _activeScreen[offsetVisual++];
				memset(&_displayScreen[offsetDisplay], curColor, 4);
				memset(&_displayScreen[offsetDisplay + _displayScreenWidth], curColor, 4);
				offsetDisplay += 4;
				remainingWidth--;
			}
			break;
		default:
			assert(0);
			break;
		}

		offsetVisual += SCRIPT_WIDTH - width;
		offsetDisplay += _displayScreenWidth - displayWidth;

		switch (_upscaledHires) {
		case DISPLAY_UPSCALED_640x400:
			offsetDisplay += _displayScreenWidth;
			break;
		default:
			break;
		}

		remainingHeight--;
	}
}

void GfxMgr::render_BlockCGA(int16 x, int16 y, int16 width, int16 height) {
	uint32 offsetVisual = SCRIPT_WIDTH * y + x;
	uint32 offsetDisplay = getDisplayOffsetToGameScreenPos(x, y);
	int16 remainingHeight = height;
	byte curColor = 0;
	int16 displayWidth = width * (2 + _displayWidthMulAdjust);

	while (remainingHeight) {
		int16 remainingWidth = width;

		switch (_upscaledHires) {
		case DISPLAY_UPSCALED_DISABLED:
			while (remainingWidth) {
				curColor = _activeScreen[offsetVisual++];
				_displayScreen[offsetDisplay++] = curColor & 0x03; // we process CGA mixture
				_displayScreen[offsetDisplay++] = curColor >> 2;
				remainingWidth--;
			}
			break;
		case DISPLAY_UPSCALED_640x400:
			while (remainingWidth) {
				curColor = _activeScreen[offsetVisual++];
				_displayScreen[offsetDisplay + 0] = curColor & 0x03; // we process CGA mixture
				_displayScreen[offsetDisplay + 1] = curColor >> 2;
				_displayScreen[offsetDisplay + 2] = curColor & 0x03;
				_displayScreen[offsetDisplay + 3] = curColor >> 2;
				_displayScreen[offsetDisplay + _displayScreenWidth + 0] = curColor & 0x03;
				_displayScreen[offsetDisplay + _displayScreenWidth + 1] = curColor >> 2;
				_displayScreen[offsetDisplay + _displayScreenWidth + 2] = curColor & 0x03;
				_displayScreen[offsetDisplay + _displayScreenWidth + 3] = curColor >> 2;
				offsetDisplay += 4;
				remainingWidth--;
			}
			break;
		default:
			assert(0);
			break;
		}

		offsetVisual += SCRIPT_WIDTH - width;
		offsetDisplay += _displayScreenWidth - displayWidth;

		switch (_upscaledHires) {
		case DISPLAY_UPSCALED_640x400:
			offsetDisplay += _displayScreenWidth;
			break;
		default:
			break;
		}

		remainingHeight--;
	}
}

static const uint8 herculesColorMapping[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x88, 0x00, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00,
	0x80, 0x10, 0x02, 0x20, 0x01, 0x08, 0x40, 0x04,
	0xAA, 0x00, 0xAA, 0x00, 0xAA, 0x00, 0xAA, 0x00,
	0x22, 0x88, 0x22, 0x88, 0x22, 0x88, 0x22, 0x88,
	0x88, 0x00, 0x88, 0x00, 0x88, 0x00, 0x88, 0x00,
	0x11, 0x22, 0x44, 0x88, 0x11, 0x22, 0x44, 0x88,
	0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,
	0x22, 0x00, 0x88, 0x00, 0x22, 0x00, 0x88, 0x00,
	0xD7, 0xFF, 0x7D, 0xFF, 0xD7, 0xFF, 0x7D, 0xFF,
	0xDD, 0x55, 0x77, 0xAA, 0xDD, 0x55, 0x77, 0xAA,
	0x7F, 0xEF, 0xFD, 0xDF, 0xFE, 0xF7, 0xBF, 0xFB,
	0xAA, 0xFF, 0xAA, 0xFF, 0xAA, 0xFF, 0xAA, 0xFF,
	0x77, 0xBB, 0xDD, 0xEE, 0x77, 0xBB, 0xDD, 0xEE,
	0x77, 0xFF, 0xFF, 0xFF, 0xDD, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

// Sierra actually seems to have rendered the whole screen all the time
void GfxMgr::render_BlockHercules(int16 x, int16 y, int16 width, int16 height) {
	uint32 offsetVisual = SCRIPT_WIDTH * y + x;
	uint32 offsetDisplay = getDisplayOffsetToGameScreenPos(x, y);
	int16 remainingHeight = height;
	byte curColor = 0;
	int16 displayWidth = width * (2 + _displayWidthMulAdjust);

	assert(_upscaledHires == DISPLAY_UPSCALED_640x400);

	uint16 lookupOffset1 = (y * 2 & 0x07);
	uint16 lookupOffset2 = 0;
	bool   getUpperNibble = false;
	byte   herculesColors1 = 0;
	byte   herculesColors2 = 0;

	while (remainingHeight) {
		int16 remainingWidth = width;

		lookupOffset1 = (lookupOffset1 + 0) & 0x07;
		lookupOffset2 = (lookupOffset1 + 1) & 0x07;

		getUpperNibble = (x & 1) ? false : true;
		while (remainingWidth) {
			curColor = _activeScreen[offsetVisual++] & 0x0F;

			if (getUpperNibble) {
				herculesColors1 = herculesColorMapping[curColor * 8 + lookupOffset1] & 0x0F;
				herculesColors2 = herculesColorMapping[curColor * 8 + lookupOffset2] & 0x0F;
			} else {
				herculesColors1 = herculesColorMapping[curColor * 8 + lookupOffset1] >> 4;
				herculesColors2 = herculesColorMapping[curColor * 8 + lookupOffset2] >> 4;
			}
			getUpperNibble ^= true;

			_displayScreen[offsetDisplay + 0] = (herculesColors1 & 0x08) ? 1 : 0;
			_displayScreen[offsetDisplay + 1] = (herculesColors1 & 0x04) ? 1 : 0;
			_displayScreen[offsetDisplay + 2] = (herculesColors1 & 0x02) ? 1 : 0;
			_displayScreen[offsetDisplay + 3] = (herculesColors1 & 0x01) ? 1 : 0;

			_displayScreen[offsetDisplay + _displayScreenWidth + 0] = (herculesColors2 & 0x08) ? 1 : 0;
			_displayScreen[offsetDisplay + _displayScreenWidth + 1] = (herculesColors2 & 0x04) ? 1 : 0;
			_displayScreen[offsetDisplay + _displayScreenWidth + 2] = (herculesColors2 & 0x02) ? 1 : 0;
			_displayScreen[offsetDisplay + _displayScreenWidth + 3] = (herculesColors2 & 0x01) ? 1 : 0;

			offsetDisplay += 4;
			remainingWidth--;
		}

		lookupOffset1 += 2;

		offsetVisual += SCRIPT_WIDTH - width;
		offsetDisplay += _displayScreenWidth - displayWidth;
		offsetDisplay += _displayScreenWidth;

		remainingHeight--;
	}
}

// Table used for at least Manhunter 2, it renders 2 lines -> 3 lines instead of 4
// Manhunter 1 is shipped with a broken Hercules font
// King's Quest 4 aborts right at the start, when Hercules rendering is active
#if 0
static const uint8 herculesCoordinateOffset[] = {
	0x00, 0x01, 0x03, 0x04, 0x06, 0x07, 0x01, 0x02,
	0x04, 0x05, 0x07, 0x00, 0x02, 0x03, 0x05, 0x06
};

static const uint8 herculesColorMapping[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x40, 0x00, 0x02, 0x00, 0x40, 0x00, 0x08, 0x00,
	0x80, 0x10, 0x02, 0x20, 0x01, 0x08, 0x40, 0x04,	0xAA, 0x00, 0xAA, 0x00, 0xAA, 0x00, 0xAA, 0x00,
	0x22, 0x88, 0x22, 0x88, 0x22, 0x88, 0x22, 0x88,	0x88, 0x00, 0x88, 0x00, 0x88, 0x00, 0x88, 0x00,
	0x11, 0x22, 0x44, 0x88, 0x11, 0x22, 0x44, 0x88,	0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,
	0x22, 0x00, 0x88, 0x00, 0x22, 0x00, 0x88, 0x00,	0xD7, 0xFF, 0x7D, 0xFF, 0xD7, 0xFF, 0x7D, 0xFF,
	0xDD, 0x55, 0x77, 0xAA, 0xDD, 0x55, 0x77, 0xAA,	0x7F, 0xEF, 0xFD, 0xDF, 0xFE, 0xF7, 0xBF, 0xFB,
	0xAA, 0xFF, 0xAA, 0xFF, 0xAA, 0xFF, 0xAA, 0xFF,	0x77, 0xBB, 0xDD, 0xEE, 0x77, 0xBB, 0xDD, 0xEE,
	0x7F, 0xEF, 0xFB, 0xBF, 0xEF, 0xFE, 0xBF, 0xFD,	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
#endif

void GfxMgr::transition_Amiga() {
	uint16 screenPos = 1;
	uint32 screenStepPos = 1;
	int16  posY = 0, posX = 0;
	int16  stepCount = 0;

	// disable mouse while transition is taking place
	if ((_vm->_game.mouseEnabled) && (!_vm->_game.mouseHidden)) {
		CursorMan.showMouse(false);
	}

	do {
		if (screenPos & 1) {
			screenPos = screenPos >> 1;
			screenPos = screenPos ^ 0x3500; // 13568d
		} else {
			screenPos = screenPos >> 1;
		}

		if ((screenPos < 13440) && (screenPos & 1)) {
			screenStepPos = screenPos >> 1;
			posY = screenStepPos / SCRIPT_WIDTH;
			posX = screenStepPos - (posY * SCRIPT_WIDTH);

			// Adjust to only update the game screen, not the status bar
			translateGamePosToDisplayScreen(posX, posY);

			switch (_upscaledHires) {
			case DISPLAY_UPSCALED_DISABLED:
				for (int16 multiPixel = 0; multiPixel < 4; multiPixel++) {
					screenStepPos = (posY * _displayScreenWidth) + posX;
					_vm->_system->copyRectToScreen(_displayScreen + screenStepPos, _displayScreenWidth, posX, posY, 2, 1);
					posY += 42;
				}
				break;
			case DISPLAY_UPSCALED_640x400:
				for (int16 multiPixel = 0; multiPixel < 4; multiPixel++) {
					screenStepPos = (posY * _displayScreenWidth) + posX;
					_vm->_system->copyRectToScreen(_displayScreen + screenStepPos, _displayScreenWidth, posX, posY, 4, 2);
					posY += 42 * 2;
				}
				break;
			default:
				assert(0);
				break;
			}

			stepCount++;
			if (stepCount == 220) {
				// 30 times for the whole transition, so should take around 0.5 seconds
				_vm->_system->updateScreen();
				_vm->_system->delayMillis(16);
				stepCount = 0;
			}
		}
	} while (screenPos != 1);

	// Enable mouse again
	if ((_vm->_game.mouseEnabled) && (!_vm->_game.mouseHidden)) {
		CursorMan.showMouse(true);
	}

	_vm->_system->updateScreen();
}

// This transition code was not reverse engineered, but created based on the Amiga transition code
// Atari ST definitely had a hi-res transition using the full resolution unlike the Amiga transition.
void GfxMgr::transition_AtariSt() {
	uint16 screenPos = 1;
	uint32 screenStepPos = 1;
	int16  posY = 0, posX = 0;
	int16  stepCount = 0;

	// disable mouse while transition is taking place
	if ((_vm->_game.mouseEnabled) && (!_vm->_game.mouseHidden)) {
		CursorMan.showMouse(false);
	}

	do {
		if (screenPos & 1) {
			screenPos = screenPos >> 1;
			screenPos = screenPos ^ 0x3500; // 13568d
		} else {
			screenPos = screenPos >> 1;
		}

		if ((screenPos < 13440) && (screenPos & 1)) {
			screenStepPos = screenPos >> 1;
			posY = screenStepPos / DISPLAY_DEFAULT_WIDTH;
			posX = screenStepPos - (posY * DISPLAY_DEFAULT_WIDTH);

			switch (_upscaledHires) {
			case DISPLAY_UPSCALED_DISABLED:
				posY += _renderStartDisplayOffsetY; // adjust to only update the main area, not the status bar
				for (int16 multiPixel = 0; multiPixel < 8; multiPixel++) {
					screenStepPos = (posY * _displayScreenWidth) + posX;
					_vm->_system->copyRectToScreen(_displayScreen + screenStepPos, _displayScreenWidth, posX, posY, 1, 1);
					posY += 21;
				}
				break;
			case DISPLAY_UPSCALED_640x400:
				posX *= 2; posY *= 2;
				posY += _renderStartDisplayOffsetY; // adjust to only update the main area, not the status bar
				for (int16 multiPixel = 0; multiPixel < 8; multiPixel++) {
					screenStepPos = (posY * _displayScreenWidth) + posX;
					_vm->_system->copyRectToScreen(_displayScreen + screenStepPos, _displayScreenWidth, posX, posY, 2, 2);
					posY += 21 * 2;
				}
				break;
			default:
				break;
			}

			stepCount++;
			if (stepCount == 168) {
				// 40 times for the whole transition, so should take around 0.7 seconds
				// When using an Atari ST emulator, the transition seems to be even slower than this
				// TODO: should get checked on real hardware
				_vm->_system->updateScreen();
				_vm->_system->delayMillis(16);
				stepCount = 0;
			}
		}
	} while (screenPos != 1);

	// Enable mouse again
	if ((_vm->_game.mouseEnabled) && (!_vm->_game.mouseHidden)) {
		CursorMan.showMouse(true);
	}

	_vm->_system->updateScreen();
}

/**
 * Copies a block of the game and priority screens to a buffer
 */
// Attention: y coordinate is here supposed to be the upper one!
void GfxMgr::block_save(int16 x, int16 y, int16 width, int16 height, byte *bufferPtr) const {
	int16 startOffset = y * SCRIPT_WIDTH + x;
	int16 offset = startOffset;
	int16 remainingHeight = height;
	byte *curBufferPtr = bufferPtr;

	//warning("block_save: %d, %d -> %d, %d", x, y, width, height);

	while (remainingHeight) {
		memcpy(curBufferPtr, _gameScreen + offset, width);
		offset += SCRIPT_WIDTH;
		curBufferPtr += width;
		remainingHeight--;
	}

	remainingHeight = height;
	offset = startOffset;
	while (remainingHeight) {
		memcpy(curBufferPtr, _priorityScreen + offset, width);
		offset += SCRIPT_WIDTH;
		curBufferPtr += width;
		remainingHeight--;
	}
}

/**
 * Copies a buffer filled by block_save back to the game and priority screens
 */
// Attention: y coordinate is here supposed to be the upper one!
void GfxMgr::block_restore(int16 x, int16 y, int16 width, int16 height, byte *bufferPtr) {
	int16 startOffset = y * SCRIPT_WIDTH + x;
	int16 offset = startOffset;
	int16 remainingHeight = height;
	byte *curBufferPtr = bufferPtr;

	//warning("block_restore: %d, %d -> %d, %d", x, y, width, height);

	while (remainingHeight) {
		memcpy(_gameScreen + offset, curBufferPtr, width);
		offset += SCRIPT_WIDTH;
		curBufferPtr += width;
		remainingHeight--;
	}

	remainingHeight = height;
	offset = startOffset;
	while (remainingHeight) {
		memcpy(_priorityScreen + offset, curBufferPtr, width);
		offset += SCRIPT_WIDTH;
		curBufferPtr += width;
		remainingHeight--;
	}
}

/**
 * Draw a box with a border on the display screen.
 * Currently only used when drawing a message box or an expanded menu.
 * Coordinates are for the visual screen instead of the game screen, because
 * while boxes are generally within the game area, there are exceptions:
 * - KQ4 intro displays a message box that extends below the game area.
 *   This would normally result in the message box not being fully removed,
 *   but the script clears the screen afterwards so it works.
 * - MMMG nursery rhyme message boxes appear over the menu bar.
 *   The scripts pass a y-coordinate of zero to print.at(). Bug #13820
 * The original interpreter didn't do any clipping; we clip against the visual
 * screen to prevent out of bounds writes while allowing boxes to be drawn outside
 * of the game area. The visual screen is 160x200 normally, 140x192 for Apple II.
 * The x, y parameters are the upper left of the box in our implementation.
 * Sierra passed the lower left. We change that to make upscaling easier.
 */
void GfxMgr::drawBox(int16 x, int16 y, int16 width, int16 height, byte backgroundColor, byte lineColor) {
	const int16 minY = 0 - _renderStartDisplayOffsetY;
	if (!render_Clip(x, y, width, height, minY, VISUAL_WIDTH, VISUAL_HEIGHT - _renderStartVisualOffsetY)) {
		warning("drawBox ignored by clipping. x: %d, y: %d, w: %d, h: %d", x, y, width, height);
		return;
	}

	// coordinate translation: visual-screen -> display-screen
	translateVisualRectToDisplayScreen(x, y, width, height);

	y = y + _renderStartDisplayOffsetY;	// drawDisplayRect paints anywhere on the whole screen, our coordinate is within playscreen

	// draw box background
	drawDisplayRect(x, y, width, height, backgroundColor);

	// draw lines
	switch (_vm->_renderMode) {
	case Common::kRenderApple2GS:
	case Common::kRenderAmiga:
		// Slightly different window frame, and actually using 1-pixel width, which is "hi-res"
		drawDisplayRect(x, +2, y, +2, width, -4, 0, 1, lineColor);
		drawDisplayRect(x + width, -3, y, +2, 0, 1, height, -4, lineColor);
		drawDisplayRect(x, +2, y + height, -3, width, -4, 0, 1, lineColor);
		drawDisplayRect(x, +2, y, +2, 0, 1, height, -4, lineColor);
		break;
	case Common::kRenderMacintosh:
		// 1 pixel between box and frame lines. Frame lines were black
		drawDisplayRect(x, +1, y, +1, width, -2, 0, 1, 0);
		drawDisplayRect(x + width, -2, y, +1, 0, 1, height, -2, 0);
		drawDisplayRect(x, +1, y + height, -2, width, -2, 0, 1, 0);
		drawDisplayRect(x, +1, y, +1, 0, 1, height, -2, 0);
		break;
	case Common::kRenderHercA:
	case Common::kRenderHercG:
		lineColor = 0; // change linecolor to black
		// fall through
	case Common::kRenderCGA:
	case Common::kRenderEGA:
	case Common::kRenderVGA:
	case Common::kRenderAtariST:
	default:
		drawDisplayRect(x, +2, y, +1, width, -4, 0, 1, lineColor);
		drawDisplayRect(x + width, -4, y, +2, 0, 2, height, -4, lineColor);
		drawDisplayRect(x, +2, y + height, -2, width, -4, 0, 1, lineColor);
		drawDisplayRect(x, +2, y, +2, 0, 2, height, -4, lineColor);
		break;
	}
}

/**
 * Draw a rectangle to the display screen
 */
void GfxMgr::drawDisplayRect(int16 x, int16 y, int16 width, int16 height, byte color, bool copyToScreen) {
	switch (_vm->_renderMode) {
	case Common::kRenderCGA:
		drawDisplayRectCGA(x, y, width, height, color);
		break;
	case Common::kRenderHercG:
	case Common::kRenderHercA:
		if (color)
			color = 1; // change any color except black to green/amber
		// fall through
	case Common::kRenderEGA:
	default:
		drawDisplayRectEGA(x, y, width, height, color);
		break;
	}
	if (copyToScreen) {
		copyDisplayRectToScreen(x, y, width, height);
	}
}

void GfxMgr::drawDisplayRect(int16 x, int16 adjX, int16 y, int16 adjY, int16 width, int16 adjWidth, int16 height, int16 adjHeight, byte color, bool copyToScreen) {
	switch (_upscaledHires) {
	case DISPLAY_UPSCALED_DISABLED:
		x += adjX; y += adjY;
		width += adjWidth; height += adjHeight;
		break;
	case DISPLAY_UPSCALED_640x400:
		x += adjX * 2; y += adjY * 2;
		width += adjWidth * 2; height += adjHeight * 2;
		break;
	default:
		assert(0);
		break;
	}
	drawDisplayRect(x, y, width, height, color, copyToScreen);
}

void GfxMgr::drawDisplayRectEGA(int16 x, int16 y, int16 width, int16 height, byte color) {
	uint32 offsetDisplay = (y * _displayScreenWidth) + x;
	int16 remainingHeight = height;

	while (remainingHeight) {
		memset(_displayScreen + offsetDisplay, color, width);

		offsetDisplay += _displayScreenWidth;
		remainingHeight--;
	}
}

void GfxMgr::drawDisplayRectCGA(int16 x, int16 y, int16 width, int16 height, byte color) {
	uint32 offsetDisplay = (y * _displayScreenWidth) + x;
	int16 remainingHeight = height;
	byte CGAMixtureColor = getCGAMixtureColor(color);
	byte *displayScreen = nullptr;

	// we should never get an uneven width
	assert((width & 1) == 0);

	while (remainingHeight) {
		int16 remainingWidth = width;

		// set up pointer
		displayScreen = _displayScreen + offsetDisplay;

		while (remainingWidth) {
			*displayScreen++ = CGAMixtureColor & 0x03;
			*displayScreen++ = CGAMixtureColor >> 2;
			remainingWidth -= 2;
		}

		offsetDisplay += _displayScreenWidth;
		remainingHeight--;
	}
}

/**
 * Draw a character to the display screen using text row and column coordinates
 */
void GfxMgr::drawCharacter(int16 row, int16 column, byte character, byte foreground, byte background, bool disabledLook) {
	int16 x = column;
	int16 y = row;
	byte  transformXOR = 0;
	byte  transformOR = 0;

	translateFontPosToDisplayScreen(x, y);

	// Now figure out, if special handling needs to be done
	if (_vm->_game.gfxMode) {
		if (background & 0x08) {
			// invert enabled
			background &= 0x07; // remove invert bit
			transformXOR = 0xFF;
		}
		if (disabledLook) {
			transformOR = 0x55;
		}
	}

	drawCharacterOnDisplay(x, y, character, foreground, background, transformXOR, transformOR);
}

/**
 * Draw a string to the display screen using display coordinates.
 * For internal use by SystemUI.
 */
void GfxMgr::drawStringOnDisplay(int16 x, int16 y, const char *text, byte foregroundColor, byte backgroundColor) {
	while (*text) {
		drawCharacterOnDisplay(x, y, *text, foregroundColor, backgroundColor);
		text++;
		x += _displayFontWidth;
	}
}

/**
 * Draw a string to the display screen using display coordinates.
 * For internal use by SystemUI.
 */
void GfxMgr::drawStringOnDisplay(int16 x, int16 adjX, int16 y, int16 adjY, const char *text, byte foregroundColor, byte backgroundColor) {
	switch (_upscaledHires) {
	case DISPLAY_UPSCALED_DISABLED:
		x += adjX;
		y += adjY;
		break;
	case DISPLAY_UPSCALED_640x400:
		x += adjX * 2;
		y += adjY * 2;
		break;
	default:
		assert(0);
		break;
	}
	drawStringOnDisplay(x, y, text, foregroundColor, backgroundColor);
}

/**
 * Draw a character to the display screen using text row and column coordinates
 */
void GfxMgr::drawCharacterOnDisplay(int16 x, int16 y, const byte character, byte foreground, byte background, byte transformXOR, byte transformOR) {
	int16       curX, curY;
	const byte *fontData;
	bool        fontIsHires = _font->isFontHires();
	int16       fontHeight = fontIsHires ? 16 : FONT_DISPLAY_HEIGHT;
	int16       fontWidth = fontIsHires ? 16 : FONT_DISPLAY_WIDTH;
	int16       fontBytesPerCharacter = fontIsHires ? 32 : FONT_BYTES_PER_CHARACTER;
	byte        curByte = 0;
	uint16      curBit;

	// get font data of specified character
	fontData = _font->getFontData() + character * fontBytesPerCharacter;

	curBit = 0;
	for (curY = 0; curY < fontHeight; curY++) {
		for (curX = 0; curX < fontWidth; curX++) {
			if (!curBit) {
				curByte = *fontData;
				// do transformations in case they are needed (invert/disabled look)
				curByte ^= transformXOR;
				curByte |= transformOR;
				fontData++;
				curBit  = 0x80;
			}
			if (curByte & curBit) {
				putFontPixelOnDisplay(x, y, curX, curY, foreground, fontIsHires);
			} else {
				putFontPixelOnDisplay(x, y, curX, curY, background, fontIsHires);
			}
			curBit = curBit >> 1;
		}
		if (transformOR)
			transformOR ^= 0xFF;
	}

	copyDisplayRectToScreen(x, y, _displayFontWidth, _displayFontHeight);
}

#define SHAKE_VERTICAL_PIXELS 4
#define SHAKE_HORIZONTAL_PIXELS 4

// Sierra used some EGA port trickery to do it, we let the backend take care of it.
//
// Shaking locations:
// - Fanmade "Enclosure" right during the intro
// - Space Quest 2 almost right at the start when getting captured (after walking into the space ship)
void GfxMgr::shakeScreen(int16 repeatCount) {
	int16 shakeHorizontalPixels = SHAKE_HORIZONTAL_PIXELS * (2 + _displayWidthMulAdjust);
	int16 shakeVerticalPixels = SHAKE_VERTICAL_PIXELS * (1 + _displayHeightMulAdjust);

	int shakeCount = repeatCount * 8; // effectively 4 shakes per repeat

	// it's 4 pixels down and 8 pixels to the right
	// and it's also filling the remaining space with black
	for (int shakeNr = 0; shakeNr < shakeCount; shakeNr++) {
		if (shakeNr & 1) {
			// move back
			_vm->_system->setShakePos(0, 0);
		} else {
			_vm->_system->setShakePos(shakeHorizontalPixels, shakeVerticalPixels);
		}
		_vm->_system->updateScreen();
		_vm->_system->delayMillis(66); // Sierra waited for 4 V'Syncs, which is around 66 milliseconds
	}
}

void GfxMgr::updateScreen() {
	_vm->_system->updateScreen();
}

void GfxMgr::initPriorityTable() {
	_priorityTableSet = false;

	createDefaultPriorityTable(_priorityTable);
}

void GfxMgr::createDefaultPriorityTable(uint8 *priorityTable) {
	int16 yPos = 0;

	for (int16 priority = 1; priority < 15; priority++) {
		for (int16 step = 0; step < 12; step++) {
			priorityTable[yPos++] = priority < 4 ? 4 : priority;
		}
	}
}

void GfxMgr::setPriorityTable(int16 priorityBase) {
	_priorityTableSet = true;
	int16 x = (SCRIPT_HEIGHT - priorityBase) * SCRIPT_HEIGHT / 10;

	for (int16 priorityY = 0; priorityY < SCRIPT_HEIGHT; priorityY++) {
		int16 priority = (priorityY - priorityBase) < 0 ? 4 : (priorityY - priorityBase) * SCRIPT_HEIGHT / x + 5;
		if (priority > 15)
			priority = 15;
		_priorityTable[priorityY] = priority;
	}
}

// used for saving
int16 GfxMgr::saveLoadGetPriority(int16 yPos) const {
	assert(yPos < SCRIPT_HEIGHT);
	return _priorityTable[yPos];
}

bool GfxMgr::saveLoadWasPriorityTableModified() const {
	return _priorityTableSet;
}

// used for restoring
void GfxMgr::saveLoadSetPriority(int16 yPos, int16 priority) {
	assert(yPos < SCRIPT_HEIGHT);
	_priorityTable[yPos] = priority;
}

void GfxMgr::saveLoadSetPriorityTableModifiedBool(bool wasModified) {
	_priorityTableSet = wasModified;
}

void GfxMgr::saveLoadFigureOutPriorityTableModifiedBool() {
	uint8 defaultPriorityTable[SCRIPT_HEIGHT]; /**< priority table */

	createDefaultPriorityTable(defaultPriorityTable);

	if (memcmp(defaultPriorityTable, _priorityTable, sizeof(_priorityTable)) == 0) {
		// Match, it is the default table, so reset the flag
		_priorityTableSet = false;
	} else {
		_priorityTableSet = true;
	}
}

/**
 * Convert sprite priority to y value.
 */
int16 GfxMgr::priorityToY(int16 priority) const {
	if (!_priorityTableSet) {
		// priority table wasn't set by scripts? calculate directly
		return (priority - 5) * 12 + 48;
	}

	// Dynamic priority bands were introduced in 2.425, but removed again until 2.936 (effectively last version of AGI2)
	// They are available from 2.936 onwards.
	// It seems there was a glitch, that caused priority bands to not get calculated properly.
	// It was caused by this function starting with Y = 168 instead of 167, which meant it always
	// returned with 168 as result.
	// This glitch is required in King's Quest 4 2.0, otherwise in room 54 ego will get drawn over
	//  the last dwarf, that enters the house.
	//  Dwarf is screen object 13 (view 152), gets fixed priority of 8, which would normally
	//  result in a Y of 101. Ego is priority (non-fixed) 8, which would mean that dwarf is
	//  drawn first, followed by ego, which would then draw ego over the dwarf.
	//  For more information see bug #3182 (dwarf sprite priority)
	//
	// This glitch is definitely present in 2.425, 2.936 and 3.002.086.
	//
	// Priority bands were working properly in: 3.001.098 (Black Cauldron)
	uint16 agiVersion = _vm->getVersion();

	if (agiVersion <= 0x3086) {
		return 168; // Buggy behavior, see above
	}

	int16 currentY = 167;
	while (_priorityTable[currentY] >= priority) {
		currentY--;
		if (currentY < 0) // Original AGI didn't do this, we abort in that case and return -1
			break;
	}
	return currentY;
}

int16 GfxMgr::priorityFromY(int16 yPos) const {
	assert(yPos < SCRIPT_HEIGHT);
	return _priorityTable[yPos];
}


/**
 * Initialize the color palette
 * This function initializes the color palette using the specified
 * RGB palette.
 * @param p           A pointer to the source RGB palette.
 * @param colorCount  Count of colors in the source palette.
 * @param fromBits    Bits per source color component.
 * @param toBits      Bits per destination color component.
 */
void GfxMgr::initPalette(uint8 *destPalette, const uint8 *paletteData, uint colorCount, uint fromBits, uint toBits) {
	const uint srcMax  = (1 << fromBits) - 1;
	const uint destMax = (1 << toBits) - 1;
	for (uint colorNr = 0; colorNr < colorCount; colorNr++) {
		for (uint componentNr = 0; componentNr < 3; componentNr++) { // Convert RGB components
			destPalette[colorNr * 3 + componentNr] = (paletteData[colorNr * 3 + componentNr] * destMax) / srcMax;
		}
	}
}

// Converts CLUT data to a palette, that we can use
void GfxMgr::initPaletteCLUT(uint8 *destPalette, const uint16 *paletteCLUTData, uint colorCount) {
	for (uint colorNr = 0; colorNr < colorCount; colorNr++) {
		for (uint componentNr = 0; componentNr < 3; componentNr++) { // RGB component
			byte component = (paletteCLUTData[colorNr * 3 + componentNr] >> 8);
			// Adjust gamma (1.8 to 2.2)
			component = (byte)(255 * pow(component / 255.0f, 0.8181f));
			destPalette[colorNr * 3 + componentNr] = component;
		}
	}
}

void GfxMgr::setPalette(bool gfxModePalette) {
	if (gfxModePalette) {
		_vm->_system->getPaletteManager()->setPalette(_paletteGfxMode, 0, 256);
	} else {
		_vm->_system->getPaletteManager()->setPalette(_paletteTextMode, 0, 256);
	}
}

//Gets AGIPAL Data
void GfxMgr::setAGIPal(int p0) {
	//If 0 from savefile, do not use
	if (p0 == 0)
		return;

	char filename[15];
	Common::sprintf_s(filename, "pal.%d", p0);

	Common::File agipal;
	if (!agipal.open(filename)) {
		warning("Couldn't open AGIPAL palette file '%s'. Not changing palette", filename);
		return; // Needed at least by Naturette 3 which uses AGIPAL but provides no palette files
	}

	//Chunk0 holds colors 0-7
	agipal.read(&_agipalPalette[0], 24);

	//Chunk1 is the same as the chunk0

	//Chunk2 chunk holds colors 8-15
	agipal.seek(24, SEEK_CUR);
	agipal.read(&_agipalPalette[24], 24);

	//Chunk3 is the same as the chunk2

	//Chunks4-7 are duplicates of chunks0-3

	if (agipal.eos() || agipal.err()) {
		warning("Couldn't read AGIPAL palette from '%s'. Not changing palette", filename);
		return;
	}

	// Use only the lowest 6 bits of each color component (Red, Green and Blue)
	// because VGA used only 6 bits per color component (i.e. VGA had 18-bit colors).
	// This should now be identical to the original AGIPAL-hack's behavior.
	bool validVgaPalette = true;
	for (int i = 0; i < 16 * 3; i++) {
		if (_agipalPalette[i] >= (1 << 6)) {
			_agipalPalette[i] &= 0x3F; // Leave only the lowest 6 bits of each color component
			validVgaPalette = false;
		}
	}

	if (!validVgaPalette)
		warning("Invalid AGIPAL palette (Over 6 bits per color component) in '%s'. Using only the lowest 6 bits per color component", filename);

	_agipalFileNum = p0;

	initPalette(_paletteGfxMode, _agipalPalette);
	setPalette(true); // set gfx-mode palette

	debug(1, "Using AGIPAL palette from '%s'", filename);
}

int GfxMgr::getAGIPalFileNum() const {
	return _agipalFileNum;
}

void GfxMgr::initMouseCursor(MouseCursorData *mouseCursor, const byte *bitmapData, uint16 width, uint16 height, int hotspotX, int hotspotY) {
	switch (_upscaledHires) {
	case DISPLAY_UPSCALED_DISABLED:
		mouseCursor->bitmapData = bitmapData;
		break;
	case DISPLAY_UPSCALED_640x400: {
		mouseCursor->bitmapDataAllocated = (byte *)malloc(width * height * 4);
		mouseCursor->bitmapData = mouseCursor->bitmapDataAllocated;

		// Upscale mouse cursor
		byte *upscaledData = mouseCursor->bitmapDataAllocated;

		for (uint16 y = 0; y < height; y++) {
			for (uint16 x = 0; x < width; x++) {
				byte curColor = *bitmapData++;
				upscaledData[x * 2 + 0] = curColor;
				upscaledData[x * 2 + 1] = curColor;
				upscaledData[x * 2 + (width * 2) + 0] = curColor;
				upscaledData[x * 2 + (width * 2) + 1] = curColor;
			}
			upscaledData += width * 2 * 2;
		}

		width *= 2;
		height *= 2;
		hotspotX *= 2;
		hotspotY *= 2;
		break;
	}
	default:
		assert(0);
		break;
	}
	mouseCursor->width = width;
	mouseCursor->height = height;
	mouseCursor->hotspotX = hotspotX;
	mouseCursor->hotspotY = hotspotY;
}

void GfxMgr::setMouseCursor(bool busy) {
	MouseCursorData &mouseCursor = busy ? _mouseCursorBusy : _mouseCursor;

	CursorMan.replaceCursor(mouseCursor.bitmapData, mouseCursor.width, mouseCursor.height, mouseCursor.hotspotX, mouseCursor.hotspotY, 0);
}

#if 0
void GfxMgr::setCursor(bool amigaStyleCursor, bool busy) {
	if (busy) {
		CursorMan.replaceCursorPalette(MOUSECURSOR_AMIGA_PALETTE, 1, ARRAYSIZE(MOUSECURSOR_AMIGA_PALETTE) / 3);
		CursorMan.replaceCursor(MOUSECURSOR_AMIGA_BUSY, 13, 16, 7, 8, 0);
		return;
	}

	if (!amigaStyleCursor) {
		CursorMan.replaceCursorPalette(sciMouseCursorPalette, 1, ARRAYSIZE(sciMouseCursorPalette) / 3);
		CursorMan.replaceCursor(sciMouseCursor, 11, 16, 1, 1, 0);
	} else { // amigaStyleCursor
		CursorMan.replaceCursorPalette(amigaMouseCursorPalette, 1, ARRAYSIZE(amigaMouseCursorPalette) / 3);
		CursorMan.replaceCursor(amigaMouseCursor, 8, 11, 1, 1, 0);
	}
}

void GfxMgr::setCursorPalette(bool amigaStyleCursor) {
	if (!amigaStyleCursor) {
		if (_currentCursorPalette != 1) {
			CursorMan.replaceCursorPalette(sciMouseCursorPalette, 1, ARRAYSIZE(sciMouseCursorPalette) / 3);
			_currentCursorPalette = 1;
		}
	} else { // amigaStyleCursor
		if (_currentCursorPalette != 2) {
			CursorMan.replaceCursorPalette(amigaMouseCursorPalette, 1, ARRAYSIZE(amigaMouseCursorPalette) / 3);
			_currentCursorPalette = 2;
		}
	}
}
#endif

} // End of namespace Agi
