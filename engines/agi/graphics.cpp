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
#include "common/file.h"
#include "common/textconsole.h"
#include "engines/util.h"

#include "graphics/cursorman.h"
#include "graphics/palette.h"

#include "agi/agi.h"
#include "agi/graphics.h"
#include "agi/mouse_cursor.h"
#include "agi/palette.h"
#include "agi/picture.h"
#include "agi/text.h"

namespace Agi {

#include "agi/font.h"

GfxMgr::GfxMgr(AgiBase *vm) : _vm(vm) {
	_agipalFileNum = 0;

	memset(&_paletteGfxMode, 0, sizeof(_paletteGfxMode));
	memset(&_paletteTextMode, 0, sizeof(_paletteTextMode));

	memset(&_mouseCursor, 0, sizeof(_mouseCursor));
	memset(&_mouseCursorBusy, 0, sizeof(_mouseCursorBusy));

	initPriorityTable();

	_renderStartOffsetY = 0;
}

/**
 * Initialize graphics device.
 *
 * @see deinit_video()
 */
int GfxMgr::initVideo() {
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
	case Common::kRenderAmiga:
		if (!ConfMan.getBool("altamigapalette")) {
			// Set the correct Amiga palette depending on AGI interpreter version
			if (_vm->getVersion() < 0x2936)
				initPalette(_paletteGfxMode, PALETTE_AMIGA_V1, 16, 4);
			else if (_vm->getVersion() == 0x2936)
				initPalette(_paletteGfxMode, PALETTE_AMIGA_V2, 16, 4);
			else if (_vm->getVersion() > 0x2936)
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
		error("initVideo: unsupported render mode");
		break;
	}

	// set up mouse cursors
	switch (_vm->_renderMode) {
	case Common::kRenderEGA:
	case Common::kRenderCGA:
	case Common::kRenderVGA:
		initMouseCursor(&_mouseCursor, MOUSECURSOR_SCI, 11, 16, 1, 1);
		initMouseCursor(&_mouseCursorBusy, MOUSECURSOR_SCI_BUSY, 15, 16, 7, 8);
		break;
	case Common::kRenderAmiga:
		initMouseCursor(&_mouseCursor, MOUSECURSOR_AMIGA, 8, 11, 1, 1);
		initMouseCursor(&_mouseCursorBusy, MOUSECURSOR_AMIGA_BUSY, 13, 16, 7, 8);
		break;
	case Common::kRenderApple2GS:
		// had no special busy mouse cursor
		initMouseCursor(&_mouseCursor, MOUSECURSOR_APPLE_II_GS, 9, 11, 1, 1);
		initMouseCursor(&_mouseCursorBusy, MOUSECURSOR_SCI_BUSY, 15, 16, 7, 8);
		break;
	case Common::kRenderAtariST:
		initMouseCursor(&_mouseCursor, MOUSECURSOR_ATARI_ST, 11, 16, 1, 1);
		initMouseCursor(&_mouseCursorBusy, MOUSECURSOR_SCI_BUSY, 15, 16, 7, 8);
		break;
	case Common::kRenderMacintosh:
		// It looks like Atari ST + Macintosh used the same standard mouse cursor
		// TODO: Verify by checking actual hardware
		initMouseCursor(&_mouseCursor, MOUSECURSOR_ATARI_ST, 11, 16, 1, 1);
		initMouseCursor(&_mouseCursorBusy, MOUSECURSOR_MACINTOSH_BUSY, 10, 14, 7, 8);
		break;
	default:
		error("initVideo: unsupported render mode");
		break;
	}

	_pixels = SCRIPT_WIDTH * SCRIPT_HEIGHT;
	_visualScreen = (byte *)calloc(_pixels, 1);
	_priorityScreen = (byte *)calloc(_pixels, 1);
	_activeScreen = _visualScreen;
	//_activeScreen = _priorityScreen;

	_displayPixels = DISPLAY_WIDTH * DISPLAY_HEIGHT;
	_displayScreen = (byte *)calloc(_displayPixels, 1);

	initGraphics(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_WIDTH > 320);

	setPalette(true); // set gfx-mode palette

	// set up mouse cursor palette
	CursorMan.replaceCursorPalette(MOUSECURSOR_PALETTE, 1, ARRAYSIZE(MOUSECURSOR_PALETTE) / 3);
	setMouseCursor();

	return errOK;
}

/**
 * Deinitialize graphics device.
 *
 * @see init_video()
 */
int GfxMgr::deinitVideo() {
	free(_displayScreen);
	free(_visualScreen);
	free(_priorityScreen);

	return errOK;
}

void GfxMgr::setRenderStartOffset(uint16 offsetY) {
	if (offsetY >= (DISPLAY_HEIGHT - SCRIPT_HEIGHT))
		error("invalid render start offset");

	_renderStartOffsetY = offsetY;
}
uint16 GfxMgr::getRenderStartOffsetY() {
	return _renderStartOffsetY;
}

void GfxMgr::debugShowMap(int mapNr) {
	switch (mapNr) {
	case 0:
		_activeScreen = _visualScreen;
		break;
	case 1:
		_activeScreen = _priorityScreen;
		break;
	default:
		break;
	}

	render_Block(0, 167, SCRIPT_WIDTH, SCRIPT_HEIGHT);
}

void GfxMgr::clear(byte color, byte priority) {
	memset(_visualScreen, color, _pixels);
	memset(_priorityScreen, priority, _pixels);
}

void GfxMgr::clearDisplay(byte color, bool copyToScreen) {
	memset(_displayScreen, color, _displayPixels);

	if (copyToScreen) {
		g_system->copyRectToScreen(_displayScreen, DISPLAY_WIDTH, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
	}
}

void GfxMgr::putPixel(int16 x, int16 y, byte drawMask, byte color, byte priority) {
	int offset = y * SCRIPT_WIDTH + x;

	if (drawMask & GFX_SCREEN_MASK_VISUAL) {
		_visualScreen[offset] = color;
	}
	if (drawMask & GFX_SCREEN_MASK_PRIORITY) {
		_priorityScreen[offset] = priority;
	}
}

void GfxMgr::putPixelOnDisplay(int16 x, int16 y, byte color) {
	int offset = y * DISPLAY_WIDTH + x;

	_displayScreen[offset] = color;
}

byte GfxMgr::getColor(int16 x, int16 y) {
	int offset = y * SCRIPT_WIDTH + x;

	return _visualScreen[offset];
}

byte GfxMgr::getPriority(int16 x, int16 y) {
	int offset = y * SCRIPT_WIDTH + x;

	return _priorityScreen[offset];
}

// used, when a control pixel is found
// will search downwards and compare priority in case any is found
bool GfxMgr::checkControlPixel(int16 x, int16 y, byte viewPriority) {
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

static byte CGA_MixtureColorTable[] = {
	0x00, 0x08, 0x04, 0x0C, 0x01, 0x09, 0x02, 0x05,
	0x0A, 0x0D, 0x06, 0x0E, 0x0B, 0x03, 0x07, 0x0F
};

byte GfxMgr::getCGAMixtureColor(byte color) {
	return CGA_MixtureColorTable[color & 0x0F];
}

// Attention: y-coordinate points to the LOWER left!
void GfxMgr::render_Block(int16 x, int16 y, int16 width, int16 height, bool copyToScreen) {
	if (!render_Clip(x, y, width, height))
		return;

	switch (_vm->_renderMode) {
	case Common::kRenderCGA:
		render_BlockCGA(x, y, width, height, copyToScreen);
		break;
	case Common::kRenderEGA:
	default:
		render_BlockEGA(x, y, width, height, copyToScreen);
		break;
	}

	if (copyToScreen) {
		int16 upperY = y - height + 1 + _renderStartOffsetY;
		g_system->copyRectToScreen(_displayScreen + upperY * DISPLAY_WIDTH + x * 2, DISPLAY_WIDTH, x * 2, upperY, width * 2, height);
	}
}

bool GfxMgr::render_Clip(int16 &x, int16 &y, int16 &width, int16 &height, int16 clipAgainstWidth, int16 clipAgainstHeight) {
	if ((x >= clipAgainstWidth) || ((x + width - 1) < 0) ||
	        (y < 0) || ((y - (height - 1)) >= clipAgainstHeight)) {
		return false;
	}

	if ((y - height + 1) < 0)
		height = y + 1;

	if (y >= clipAgainstHeight) {
		height -= y - (clipAgainstHeight - 1);
		y = clipAgainstHeight - 1;
	}

	if (x < 0) {
		width += x;
		x = 0;
	}

	if ((x + width - 1) >= clipAgainstWidth) {
		width = clipAgainstWidth - x;
	}
	return true;
}

void GfxMgr::render_BlockEGA(int16 x, int16 y, int16 width, int16 height, bool copyToScreen) {
	int offsetVisual = SCRIPT_WIDTH * y + x;
	int offsetDisplay = (DISPLAY_WIDTH * (y + _renderStartOffsetY)) + x * 2;
	int16 remainingWidth = width;
	int16 remainingHeight = height;
	byte curColor = 0;

	while (remainingHeight) {
		remainingWidth = width;
		while (remainingWidth) {
			curColor = _activeScreen[offsetVisual++];
			_displayScreen[offsetDisplay++] = curColor;
			_displayScreen[offsetDisplay++] = curColor;
			remainingWidth--;
		}
		offsetVisual -= SCRIPT_WIDTH + width;
		offsetDisplay -= DISPLAY_WIDTH + width * 2;

		remainingHeight--;
	}
}

void GfxMgr::render_BlockCGA(int16 x, int16 y, int16 width, int16 height, bool copyToScreen) {
	int offsetVisual = SCRIPT_WIDTH * y + x;
	int offsetDisplay = (DISPLAY_WIDTH * (y + _renderStartOffsetY)) + x * 2;
	int16 remainingWidth = width;
	int16 remainingHeight = height;
	byte curColor = 0;

	while (remainingHeight) {
		remainingWidth = width;
		while (remainingWidth) {
			curColor = _activeScreen[offsetVisual++];
			_displayScreen[offsetDisplay++] = curColor & 0x03; // we process CGA mixture
			_displayScreen[offsetDisplay++] = curColor >> 2;
			remainingWidth--;
		}
		offsetVisual -= SCRIPT_WIDTH + width;
		offsetDisplay -= DISPLAY_WIDTH + width * 2;

		remainingHeight--;
	}
}

void GfxMgr::transition_Amiga() {
	uint16 screenPos = 1;
	uint16 screenStepPos = 1;
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

			posY += _renderStartOffsetY; // adjust to only update the main area, not the status bar
			posX *= 2; // adjust for display screen

			screenStepPos = (screenStepPos * 2) + (_renderStartOffsetY * DISPLAY_WIDTH); // adjust here too for display screen
			for (int16 multiPixel = 0; multiPixel < 4; multiPixel++) {
				g_system->copyRectToScreen(_displayScreen + screenStepPos, DISPLAY_WIDTH, posX, posY, 2, 1);
				screenStepPos += (0x1A40 * 2); // 6720d
				posY += 42;
			}
			stepCount++;
			if (stepCount == 220) {
				// 30 times for the whole transition, so should take around 0.5 seconds
				g_system->updateScreen();
				g_system->delayMillis(16);
				stepCount = 0;
			}
		}
	} while (screenPos != 1);

	// Enable mouse again
	if ((_vm->_game.mouseEnabled) && (!_vm->_game.mouseHidden)) {
		CursorMan.showMouse(true);
	}

	g_system->updateScreen();
}

// This transition code was not reverse engineered, but created based on the Amiga transition code
// Atari ST definitely had a hi-res transition using the full resolution unlike the Amiga transition.
void GfxMgr::transition_AtariSt() {
	uint16 screenPos = 1;
	uint16 screenStepPos = 1;
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
			posY = screenStepPos / DISPLAY_WIDTH;
			posX = screenStepPos - (posY * DISPLAY_WIDTH);

			posY += _renderStartOffsetY; // adjust to only update the main area, not the status bar

			screenStepPos = screenStepPos + (_renderStartOffsetY * DISPLAY_WIDTH); // adjust here too for display screen
			for (int16 multiPixel = 0; multiPixel < 8; multiPixel++) {
				g_system->copyRectToScreen(_displayScreen + screenStepPos, DISPLAY_WIDTH, posX, posY, 1, 1);
				screenStepPos += 0x1a40; // 6720d
				posY += 21;
			}
			stepCount++;
			if (stepCount == 168) {
				// 40 times for the whole transition, so should take around 0.7 seconds
				// When using an Atari ST emulator, the transition seems to be even slower than this
				// TODO: should get checked on real hardware
				g_system->updateScreen();
				g_system->delayMillis(16);
				stepCount = 0;
			}
		}
	} while (screenPos != 1);

	// Enable mouse again
	if ((_vm->_game.mouseEnabled) && (!_vm->_game.mouseHidden)) {
		CursorMan.showMouse(true);
	}

	g_system->updateScreen();
}

// Attention: y coordinate is here supposed to be the upper one!
void GfxMgr::block_save(int16 x, int16 y, int16 width, int16 height, byte *bufferPtr) {
	int16 startOffset = y * SCRIPT_WIDTH + x;
	int16 offset = startOffset;
	int16 remainingHeight = height;
	byte *curBufferPtr = bufferPtr;

	//warning("block_save: %d, %d -> %d, %d", x, y, width, height);

	while (remainingHeight) {
		memcpy(curBufferPtr, _visualScreen + offset, width);
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

// Attention: y coordinate is here supposed to be the upper one!
void GfxMgr::block_restore(int16 x, int16 y, int16 width, int16 height, byte *bufferPtr) {
	int16 startOffset = y * SCRIPT_WIDTH + x;
	int16 offset = startOffset;
	int16 remainingHeight = height;
	byte *curBufferPtr = bufferPtr;

	//warning("block_restore: %d, %d -> %d, %d", x, y, width, height);

	while (remainingHeight) {
		memcpy(_visualScreen + offset, curBufferPtr, width);
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

// Attention: uses visual screen coordinates!
void GfxMgr::copyDisplayRectToScreen(int16 x, int16 y, int16 width, int16 height) {
	g_system->copyRectToScreen(_displayScreen + y * DISPLAY_WIDTH + x, DISPLAY_WIDTH, x, y, width, height);
}
void GfxMgr::copyDisplayToScreen() {
	g_system->copyRectToScreen(_displayScreen, DISPLAY_WIDTH, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
}

// coordinates are for visual screen, but are supposed to point somewhere inside the playscreen
// attention: Clipping is done here against 160x200 instead of 160x168
//            Original interpreter didn't do any clipping, we do it for security.
//            Clipping against the regular script width/height must not be done,
//            because at least during the intro one message box goes beyond playscreen
//            Going beyond 160x168 will result in messageboxes not getting fully removed
//            In KQ4's case, the scripts clear the screen that's why it works.
void GfxMgr::drawBox(int16 x, int16 y, int16 width, int16 height, byte backgroundColor, byte lineColor) {
	if (!render_Clip(x, y, width, height, SCRIPT_WIDTH, DISPLAY_HEIGHT - _renderStartOffsetY))
		return;

	// coordinate translation: visual-screen -> display-screen
	x = x * 2;
	y = y + _renderStartOffsetY; // drawDisplayRect paints anywhere on the whole screen, our coordinate is within playscreen
	width = width * 2; // width was given as visual width, we need display width

	// draw box background
	drawDisplayRect(x, y, width, height, backgroundColor);

	// draw lines
	switch (_vm->_renderMode) {
	case Common::kRenderApple2GS:
	case Common::kRenderAmiga:
		// Slightly different window frame, and actually using 1-pixel width, which is "hi-res"
		drawDisplayRect(x + 2, y - 2, width - 4, 1, lineColor);
		drawDisplayRect(x + width - 3, y - 2, 1, height - 4, lineColor);
		drawDisplayRect(x + 2, y - height + 3, width - 4, 1, lineColor);
		drawDisplayRect(x + 2, y - 2, 1, height - 4, lineColor);
		break;
	case Common::kRenderMacintosh:
		// 1 pixel between box and frame lines. Frame lines were black
		drawDisplayRect(x + 1, y - 1, width - 2, 1, 0);
		drawDisplayRect(x + width - 2, y - 1, 1, height - 2, 0);
		drawDisplayRect(x + 1, y - height + 2, width - 2, 1, 0);
		drawDisplayRect(x + 1, y - 1, 1, height - 2, 0);
		break;
	case Common::kRenderCGA:
	case Common::kRenderEGA:
	case Common::kRenderVGA:
	case Common::kRenderAtariST:
	default:
		drawDisplayRect(x + 2, y - 1, width - 4, 1, lineColor);
		drawDisplayRect(x + width - 4, y - 2, 2, height - 4, lineColor);
		drawDisplayRect(x + 2, y - height + 2, width - 4, 1, lineColor);
		drawDisplayRect(x + 2, y - 2, 2, height - 4, lineColor);
		break;
	}
}

// coordinates are directly for display screen
void GfxMgr::drawDisplayRect(int16 x, int16 y, int16 width, int16 height, byte color, bool copyToScreen) {
	switch (_vm->_renderMode) {
	case Common::kRenderCGA:
		drawDisplayRectCGA(x, y, width, height, color);
		break;
	case Common::kRenderEGA:
	default:
		drawDisplayRectEGA(x, y, width, height, color);
		break;
	}
	if (copyToScreen) {
		int16 upperY = y - height + 1;
		g_system->copyRectToScreen(_displayScreen + upperY * DISPLAY_WIDTH + x, DISPLAY_WIDTH, x, upperY, width, height);
	}
}

void GfxMgr::drawDisplayRectEGA(int16 x, int16 y, int16 width, int16 height, byte color) {
	int offsetDisplay = (DISPLAY_WIDTH * y) + x;
	int16 remainingHeight = height;

	while (remainingHeight) {
		memset(_displayScreen + offsetDisplay, color, width);

		offsetDisplay -= DISPLAY_WIDTH;
		remainingHeight--;
	}
}

void GfxMgr::drawDisplayRectCGA(int16 x, int16 y, int16 width, int16 height, byte color) {
	int offsetDisplay = (DISPLAY_WIDTH * y) + x;
	int16 remainingHeight = height;
	int16 remainingWidth = width;
	byte CGAMixtureColor = getCGAMixtureColor(color);
	byte *displayScreen = nullptr;

	// we should never get an uneven width
	assert((width & 1) == 0);

	while (remainingHeight) {
		remainingWidth = width;

		// set up pointer
		displayScreen = _displayScreen + offsetDisplay;

		while (remainingWidth) {
			*displayScreen++ = CGAMixtureColor & 0x03;
			*displayScreen++ = CGAMixtureColor >> 2;
			remainingWidth -= 2;
		}

		offsetDisplay -= DISPLAY_WIDTH;
		remainingHeight--;
	}
}

// row + column are text-coordinates
void GfxMgr::drawCharacter(int16 row, int16 column, byte character, byte foreground, byte background, bool disabledLook) {
	int16 x = column * FONT_DISPLAY_WIDTH;
	int16 y = row * FONT_DISPLAY_HEIGHT;
	byte  transformXOR = 0;
	byte  transformOR = 0;

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

// only meant for internal use (SystemUI)
void GfxMgr::drawStringOnDisplay(int16 x, int16 y, const char *text, byte foregroundColor, byte backgroundColor) {
	while (*text) {
		drawCharacterOnDisplay(x, y, *text, foregroundColor, backgroundColor);
		text++;
		x += FONT_DISPLAY_WIDTH;
	}
}

void GfxMgr::drawCharacterOnDisplay(int16 x, int16 y, const byte character, byte foreground, byte background, byte transformXOR, byte transformOR) {
	int16       curX, curY;
	const byte *fontData;
	byte        curByte = 0;
	uint16      curBit;

	// get font data of specified character
	fontData = _vm->getFontData() + character * FONT_BYTES_PER_CHARACTER;

	curBit = 0;
	for (curY = 0; curY < FONT_DISPLAY_HEIGHT; curY++) {
		for (curX = 0; curX < FONT_DISPLAY_WIDTH; curX++) {
			if (!curBit) {
				curByte = *fontData;
				// do transformations in case they are needed (invert/disabled look)
				curByte ^= transformXOR;
				curByte |= transformOR;
				fontData++;
				curBit  = 0x80;
			}
			if (curByte & curBit) {
				putPixelOnDisplay(x + curX, y + curY, foreground);
			} else {
				putPixelOnDisplay(x + curX, y + curY, background);
			}
			curBit = curBit >> 1;
		}
		if (transformOR)
			transformOR ^= 0xFF;
	}

	copyDisplayRectToScreen(x, y, FONT_DISPLAY_WIDTH, FONT_DISPLAY_HEIGHT);
}

#define SHAKE_VERTICAL_PIXELS 4
#define SHAKE_HORIZONTAL_PIXELS 8

// Sierra used some EGA port trickery to do it, we have to do it by copying pixels around
void GfxMgr::shakeScreen(int16 repeatCount) {
	int shakeNr, shakeCount;
	uint8 *blackSpace;

	if ((blackSpace = (uint8 *)calloc(SHAKE_HORIZONTAL_PIXELS * DISPLAY_WIDTH, 1)) == NULL)
		return;

	shakeCount = repeatCount * 8; // effectively 4 shakes per repeat

	// it's 4 pixels down and 8 pixels to the right
	// and it's also filling the remaining space with black
	for (shakeNr = 0; shakeNr < shakeCount; shakeNr++) {
		if (shakeNr & 1) {
			// move back
			copyDisplayToScreen();
		} else {
			g_system->copyRectToScreen(_displayScreen, DISPLAY_WIDTH, SHAKE_HORIZONTAL_PIXELS, SHAKE_VERTICAL_PIXELS, DISPLAY_WIDTH - SHAKE_HORIZONTAL_PIXELS, DISPLAY_HEIGHT - SHAKE_VERTICAL_PIXELS);
			// additionally fill the remaining space with black
			g_system->copyRectToScreen(blackSpace, DISPLAY_WIDTH, 0, 0, DISPLAY_WIDTH, SHAKE_VERTICAL_PIXELS);
			g_system->copyRectToScreen(blackSpace, SHAKE_HORIZONTAL_PIXELS, 0, 0, SHAKE_HORIZONTAL_PIXELS, DISPLAY_HEIGHT);
		}
		g_system->updateScreen();
		g_system->delayMillis(66); // Sierra waited for 4 V'Syncs, which is around 66 milliseconds
	}

	free(blackSpace);
}

void GfxMgr::updateScreen() {
	g_system->updateScreen();
}

void GfxMgr::initPriorityTable() {
	_priorityTableSet = false;

	createDefaultPriorityTable(_priorityTable);
}

void GfxMgr::createDefaultPriorityTable(uint8 *priorityTable) {
	int16 priority, step;
	int16 yPos = 0;

	for (priority = 1; priority < 15; priority++) {
		for (step = 0; step < 12; step++) {
			priorityTable[yPos++] = priority < 4 ? 4 : priority;
		}
	}
}

void GfxMgr::setPriorityTable(int16 priorityBase) {
	int16 x, priorityY, priority;

	_priorityTableSet = true;
	x = (SCRIPT_HEIGHT - priorityBase) * SCRIPT_HEIGHT / 10;

	for (priorityY = 0; priorityY < SCRIPT_HEIGHT; priorityY++) {
		priority = (priorityY - priorityBase) < 0 ? 4 : (priorityY - priorityBase) * SCRIPT_HEIGHT / x + 5;
		if (priority > 15)
			priority = 15;
		_priorityTable[priorityY] = priority;
	}
}

// used for saving
int16 GfxMgr::saveLoadGetPriority(int16 yPos) {
	assert(yPos < SCRIPT_HEIGHT);
	return _priorityTable[yPos];
}
bool GfxMgr::saveLoadWasPriorityTableModified() {
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
int16 GfxMgr::priorityToY(int16 priority) {
	int16 currentY;

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
	//  For more information see bug #1712585 (dwarf sprite priority)
	//
	// This glitch is definitely present in 2.425, 2.936 and 3.002.086.
	//
	// Priority bands were working properly in: 3.001.098 (Black Cauldron)
	uint16 agiVersion = _vm->getVersion();

	if (agiVersion <= 0x3086) {
		return 168; // Buggy behavior, see above
	}

	currentY = 167;
	while (_priorityTable[currentY] >= priority) {
		currentY--;
		if (currentY < 0) // Original AGI didn't do this, we abort in that case and return -1
			break;
	}
	return currentY;
}

int16 GfxMgr::priorityFromY(int16 yPos) {
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
		g_system->getPaletteManager()->setPalette(_paletteGfxMode, 0, 256);
	} else {
		g_system->getPaletteManager()->setPalette(_paletteTextMode, 0, 256);
	}
}

//Gets AGIPAL Data
void GfxMgr::setAGIPal(int p0) {
	//If 0 from savefile, do not use
	if (p0 == 0)
		return;

	char filename[15];
	sprintf(filename, "pal.%d", p0);

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

int GfxMgr::getAGIPalFileNum() {
	return _agipalFileNum;
}

void GfxMgr::initMouseCursor(MouseCursorData *mouseCursor, const byte *bitmapData, uint16 width, uint16 height, int hotspotX, int hotspotY) {
	mouseCursor->bitmapData = bitmapData;
	mouseCursor->width = width;
	mouseCursor->height = height;
	mouseCursor->hotspotX = hotspotX;
	mouseCursor->hotspotY = hotspotY;
}

void GfxMgr::setMouseCursor(bool busy) {
	MouseCursorData *mouseCursor = nullptr;

	if (!busy) {
		mouseCursor = &_mouseCursor;
	} else {
		mouseCursor = &_mouseCursorBusy;
	}

	if (mouseCursor) {
		CursorMan.replaceCursor(mouseCursor->bitmapData, mouseCursor->width, mouseCursor->height, mouseCursor->hotspotX, mouseCursor->hotspotY, 0);
	}
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
