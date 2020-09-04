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

#include "cine/cine.h"
#include "cine/bg.h"
#include "cine/bg_list.h"
#include "cine/various.h"
#include "cine/pal.h"

#include "common/config-manager.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/str.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "graphics/cursorman.h"
#include "graphics/primitives.h"

namespace Cine {

byte *collisionPage;
FWRenderer *renderer = NULL;

#define DEFAULT_MESSAGE_BG 1
#define DEFAULT_CMD_Y 185

// Constants related to kLowPalFormat
#define kLowPalBytesPerColor 2
#define kLowPalNumColors 16
#define kLowPalNumBytes ((kLowPalNumColors) * (kLowPalBytesPerColor))

/** Low resolution (9-bit) color format used in Cine's 16-color modes. */
#define kLowPalFormat Graphics::PixelFormat(kLowPalBytesPerColor, 3, 3, 3, 0, 8, 4, 0, 0)


// Constants related to kHighPalFormat
#define kHighPalBytesPerColor 3
#define kHighPalNumColors 256
#define kHighPalNumBytes ((kHighPalNumColors) * (kHighPalBytesPerColor))

/** High resolution (24-bit) color format used in Cine's 256-color modes. */
#define kHighPalFormat Graphics::PixelFormat(kHighPalBytesPerColor, 8, 8, 8, 0, 0, 8, 16, 0)

static const byte mouseCursorNormal[] = {
	0x00, 0x00, 0x40, 0x00, 0x60, 0x00, 0x70, 0x00,
	0x78, 0x00, 0x7C, 0x00, 0x7E, 0x00, 0x7F, 0x00,
	0x7F, 0x80, 0x7C, 0x00, 0x6C, 0x00, 0x46, 0x00,
	0x06, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00, 0x00,
	0xC0, 0x00, 0xE0, 0x00, 0xF0, 0x00, 0xF8, 0x00,
	0xFC, 0x00, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x80,
	0xFF, 0xC0, 0xFF, 0xC0, 0xFE, 0x00, 0xFF, 0x00,
	0xCF, 0x00, 0x07, 0x80, 0x07, 0x80, 0x03, 0x80
};

static const byte mouseCursorDisk[] = {
	0x7F, 0xFC, 0x9F, 0x12, 0x9F, 0x12, 0x9F, 0x12,
	0x9F, 0x12, 0x9F, 0xE2, 0x80, 0x02, 0x9F, 0xF2,
	0xA0, 0x0A, 0xA0, 0x0A, 0xA0, 0x0A, 0xA0, 0x0A,
	0xA0, 0x0A, 0xA0, 0x0A, 0x7F, 0xFC, 0x00, 0x00,
	0x7F, 0xFC, 0xFF, 0xFE, 0xFF, 0xFE, 0xFF, 0xFE,
	0xFF, 0xFE, 0xFF, 0xFE, 0xFF, 0xFE, 0xFF, 0xFE,
	0xFF, 0xFE, 0xFF, 0xFE, 0xFF, 0xFE, 0xFF, 0xFE,
	0xFF, 0xFE, 0xFF, 0xFE, 0x7F, 0xFC, 0x00, 0x00
};

static const byte mouseCursorCross[] = {
	0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
	0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x7C, 0x7C,
	0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
	0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x03, 0x80, 0x03, 0x80, 0x03, 0x80, 0x03, 0x80,
	0x03, 0x80, 0x03, 0x80, 0xFF, 0xFE, 0xFE, 0xFE,
	0xFF, 0xFE, 0x03, 0x80, 0x03, 0x80, 0x03, 0x80,
	0x03, 0x80, 0x03, 0x80, 0x03, 0x80, 0x00, 0x00
};

static const struct MouseCursor {
	int hotspotX;
	int hotspotY;
	const byte *bitmap;
} mouseCursors[] = {
	{ 1, 1, mouseCursorNormal },
	{ 0, 0, mouseCursorDisk },
	{ 7, 7, mouseCursorCross }
};

static const byte cursorPalette[] = {
	0, 0, 0, 0xff,
	0xff, 0xff, 0xff, 0xff
};

void plotPoint(int x, int y, int color, void *data) {
	byte *output = (byte *)data;
	if (x >= 0 && x < 320 && y >= 0 && y < 200) {
		output[y * 320 + x] = (byte)color;
	}
}

/**
 * Initialize renderer
 */
FWRenderer::FWRenderer() : _savedBackBuffers(), _background(NULL), _backupPal(), _cmd(""),
	_messageBg(DEFAULT_MESSAGE_BG), _cmdY(DEFAULT_CMD_Y), _backBuffer(new byte[_screenSize]),
	_activePal(), _changePal(0), _showCollisionPage(false), _fadeToBlackLastCalledMs(0) {

	assert(_backBuffer);

	memset(_backBuffer, 0, _screenSize);
	memset(_bgName, 0, sizeof(_bgName));
}

void FWRenderer::removeSavedBackBuffers() {
	for (int i = 0; i < ARRAYSIZE(_savedBackBuffers); i++) {
		if (_savedBackBuffers[i]) {
			delete[] _savedBackBuffers[i];
			_savedBackBuffers[i] = nullptr;
		}
	}
}

/**
 * Destroy renderer
 */
FWRenderer::~FWRenderer() {
	delete[] _background;
	delete[] _backBuffer;

	removeSavedBackBuffers();

	clearMenuStack();
}

bool FWRenderer::initialize() {
	_backupPal = _activePal = Palette(kLowPalFormat, kLowPalNumColors);
	return true;
}


/**
 * Reset renderer state
 */
void FWRenderer::clear() {
	delete[] _background;

	_background = NULL;
	_backupPal.clear();
	_activePal.clear();

	memset(_backBuffer, 0, _screenSize);
	removeSavedBackBuffers();

	_cmd = "";
	_cmdY = DEFAULT_CMD_Y;
	_messageBg = DEFAULT_MESSAGE_BG;
	_changePal = 0;
	_showCollisionPage = false;
}

const Cine::Palette& FWRenderer::getFadeInSourcePalette() {
	return _backupPal;
}

/**
 * Draw 1bpp sprite using selected color
 * @param obj Object info
 * @param fillColor Sprite color
 */
void FWRenderer::fillSprite(const ObjectStruct &obj, uint8 color) {
	const byte *data = g_cine->_animDataTable[obj.frame].data();
	int x, y, width, height;

	x = obj.x;
	y = obj.y;
	width = g_cine->_animDataTable[obj.frame]._realWidth;
	height = g_cine->_animDataTable[obj.frame]._height;

	gfxFillSprite(data, width, height, _backBuffer, x, y, color);
}

/**
 * Draw 1bpp sprite using selected color on background
 * @param obj Object info
 * @param fillColor Sprite color
 */
void FWRenderer::incrustMask(const BGIncrust &incrust, uint8 color) {
	const ObjectStruct &obj = g_cine->_objectTable[incrust.objIdx];
	const byte *data = g_cine->_animDataTable[obj.frame].data();
	int x, y, width, height;

	x = obj.x;
	y = obj.y;
	width = g_cine->_animDataTable[obj.frame]._realWidth;
	height = g_cine->_animDataTable[obj.frame]._height;

	gfxFillSprite(data, width, height, _background, x, y, color);
}

/**
 * Draw color sprite using with external mask
 * @param obj Object info
 * @param mask External mask
 */
void FWRenderer::drawMaskedSprite(const ObjectStruct &obj, const byte *mask) {
	const byte *data = g_cine->_animDataTable[obj.frame].data();
	int x, y, width, height;

	x = obj.x;
	y = obj.y;
	width = g_cine->_animDataTable[obj.frame]._realWidth;
	height = g_cine->_animDataTable[obj.frame]._height;

	assert(mask);

	drawSpriteRaw(data, mask, width, height, _backBuffer, x, y);
}

/**
 * Draw color sprite
 * @param obj Object info
 */
void FWRenderer::drawSprite(const ObjectStruct &obj) {
	const byte *mask = g_cine->_animDataTable[obj.frame].mask();
	drawMaskedSprite(obj, mask);
}

/**
 * Draw color sprite on background
 * @param obj Object info
 */
void FWRenderer::incrustSprite(const BGIncrust &incrust) {
	const ObjectStruct &obj = g_cine->_objectTable[incrust.objIdx];

	const byte *data = g_cine->_animDataTable[obj.frame].data();
	const byte *mask = g_cine->_animDataTable[obj.frame].mask();
	int x, y, width, height;

	x = obj.x;
	y = obj.y;
	width = g_cine->_animDataTable[obj.frame]._realWidth;
	height = g_cine->_animDataTable[obj.frame]._height;

	// There was an assert(mask) here before but it made savegame loading
	// in Future Wars sometimes fail the assertion (e.g. see bug #2055912).
	// Not drawing sprites that have no mask seems to work, but not sure
	// if this is really a correct way to fix this.
	if (mask) {
		drawSpriteRaw(data, mask, width, height, _background, x, y);
	} else { // mask == NULL
		warning("FWRenderer::incrustSprite: Skipping maskless sprite (frame=%d)", obj.frame);
	}
}

/**
 * Draw command box on screen
 */
void FWRenderer::drawCommand() {
	if (disableSystemMenu == 0) {
		int x = 10, y = _cmdY;

		drawPlainBox(x, y, 301, 11, 0);
		drawBorder(x - 1, y - 1, 302, 12, 2);

		x += 2;
		y += 2;

		for (unsigned int i = 0; i < _cmd.size(); i++) {
			x = drawChar(_cmd[i], x, y);
		}
	}
}

void FWRenderer::drawString(const char *string, byte param) {
	int width;
	byte minBrightnessColorIndex = 4;

	bool useEnsureContrast = true;
	if (useEnsureContrast && g_cine->getGameType() == Cine::GType_OS) {
		bool paletteChanged = _activePal.ensureContrast(minBrightnessColorIndex);
		if (paletteChanged) {
			clearBackBuffer();
			setPalette();
		}
	}

	// Both Future Wars and Operation Stealth 16 color PC versions do this
	int y = 80;
	if (param == 1) {
		y = 20;
	} else if (param == 2) {
		y = 140;
	}

	width = getStringWidth(string);

	if (width == 0) {
		return;
	}

	width = MIN<int>(width + 20, 300);

	drawMessage(string, (320 - width) / 2, y, width, minBrightnessColorIndex);

	blit();
}

/**
 * Draw message in a box
 * @param str Message to draw
 * @param x Top left message box corner coordinate
 * @param y Top left message box corner coordinate
 * @param width Message box width
 * @param color Message box background color (Or if negative draws only the text)
 * @note Negative colors are used in Operation Stealth's timed cutscenes
 * (e.g. when first meeting The Movement for the Liberation of Santa Paragua).
 */
void FWRenderer::drawMessage(const char *str, int x, int y, int width, int color) {
	int i, tx, ty, tw;
	int line = 0, words = 0, cw = 0;
	int space = 0, extraSpace = 0;

	if (color >= 0) {
		if (useTransparentDialogBoxes())
			drawTransparentBox(x, y, width, 4);
		else
			drawPlainBox(x, y, width, 4, color);
	}
	tx = x + 4;
	ty = str[0] ? y - 5 : y + 4;
	tw = width - 8;

	for (i = 0; str[i]; i++, line--) {
		// Fit line of text into textbox
		if (!line) {
			while (str[i] == ' ')
				i++;
			line = fitLine(str + i, tw, words, cw);

			if (str[i + line] != '\0' && str[i + line] != 0x7C && words) {
				space = (tw - cw) / words;
				extraSpace = (tw - cw) % words;
			} else {
				space = 5;
				extraSpace = 0;
			}

			ty += 9;
			if (color >= 0) {
				if (useTransparentDialogBoxes())
					drawTransparentBox(x, ty, width, 9);
				else
					drawPlainBox(x, ty, width, 9, color);
			}
			tx = x + 4;
		}

		// draw characters
		if (str[i] == ' ') {
			tx += space + extraSpace;

			if (extraSpace) {
				extraSpace = 0;
			}
		} else {
			tx = drawChar(str[i], tx, ty);
		}
	}

	ty += 9;
	if (color >= 0) {
		if (useTransparentDialogBoxes())
			drawTransparentBox(x, ty, width, 4);
		else
			drawPlainBox(x, ty, width, 4, color);
		drawDoubleBorder(x, y, width, ty - y + 4, (useTransparentDialogBoxes() ? transparentDialogBoxStartColor() : 0) + 2);
	}
}

/**
 * Draw rectangle on screen
 * @param x Top left corner coordinate
 * @param y Top left corner coordinate
 * @param width Rectangle width (Negative values draw the box horizontally flipped)
 * @param height Rectangle height (Negative values draw the box vertically flipped)
 * @param color Fill color
 * @note An on-screen rectangle's drawn width is always at least one.
 * @note An on-screen rectangle's drawn height is always at least one.
 */
void FWRenderer::drawPlainBox(int x, int y, int width, int height, byte color) {
	// Handle horizontally flipped boxes
	if (width < 0) {
		width = ABS(width);
		x -= width;
	}

	// Handle vertically flipped boxes
	if (height < 0) {
		height = ABS(height);
		y -= height;
	}

	// Clip the rectangle to screen dimensions
	Common::Rect boxRect(x, y, x + width, y + height);
	Common::Rect screenRect(320, 200);
	boxRect.clip(screenRect);

	byte *dest = _backBuffer + boxRect.top * 320 + boxRect.left;
	for (int i = 0; i < boxRect.height(); i++) {
		memset(dest + i * 320, color, boxRect.width());
	}
}

bool FWRenderer::useTransparentDialogBoxes() {
	return _activePal.colorCount() == 16 &&
		((g_cine->getPlatform() == Common::kPlatformAmiga) ||
		ConfMan.getBool("transparentdialogboxes"));
}

byte FWRenderer::transparentDialogBoxStartColor() {
	return 16;
}

void FWRenderer::drawTransparentBox(int x, int y, int width, int height) {
	byte startColor = transparentDialogBoxStartColor();

	// Handle horizontally flipped boxes
	if (width < 0) {
		width = ABS(width);
		x -= width;
	}

	// Handle vertically flipped boxes
	if (height < 0) {
		height = ABS(height);
		y -= height;
	}

	// Clip the rectangle to screen dimensions
	Common::Rect boxRect(x, y, x + width, y + height);
	Common::Rect screenRect(320, 200);
	boxRect.clip(screenRect);

	byte *dest = _backBuffer + boxRect.top * 320 + boxRect.left;
	const int lineAdd = 320 - boxRect.width();
	for (int i = 0; i < boxRect.height(); ++i) {
		for (int j = 0; j < boxRect.width(); ++j, ++dest) {
			if (*dest < startColor)
				*dest += startColor;
		}
		dest += lineAdd;
	}
}

/**
 * Draw empty rectangle
 * @param x Top left corner coordinate
 * @param y Top left corner coordinate
 * @param width Rectangle width
 * @param height Rectangle height
 * @param color Line color
 */
void FWRenderer::drawBorder(int x, int y, int width, int height, byte color) {
	drawLine(x, y, width, 1, color);
	drawLine(x, y + height, width, 1, color);
	drawLine(x, y, 1, height, color);
	drawLine(x + width, y, 1, height + 1, color);
}

/**
 * Draw empty 2 color rectangle (inner line color is black)
 * @param x Top left corner coordinate
 * @param y Top left corner coordinate
 * @param width Rectangle width
 * @param height Rectangle height
 * @param color Outter line color
 */
void FWRenderer::drawDoubleBorder(int x, int y, int width, int height, byte color) {
	drawBorder(x + 1, y + 1, width - 2, height - 2, 0);
	drawBorder(x, y, width, height, color);
}

/**
 * Draw text character on screen
 * @param character Character to draw
 * @param x Character coordinate
 * @param y Character coordinate
 */
int FWRenderer::drawChar(char character, int x, int y) {
	int width;

	if (character == ' ') {
		x += 5;
	} else if ((width = g_cine->_textHandler.fontParamTable[(unsigned char)character].characterWidth)) {
		int idx = g_cine->_textHandler.fontParamTable[(unsigned char)character].characterIdx;
		drawSpriteRaw(g_cine->_textHandler.textTable[idx][FONT_DATA], g_cine->_textHandler.textTable[idx][FONT_MASK], FONT_WIDTH, FONT_HEIGHT, _backBuffer, x, y);
		x += width + 1;
	}

	return x;
}

/**
 * Clears the character glyph to black
 * This function is called "undrawChar", because the original only applies
 * this drawing after the original glyph has been drawn.
 * Possible TODO: Find a better name.
 * @param character Character to undraw
 * @param x Character coordinate
 * @param y Character coordinate
 */
int FWRenderer::undrawChar(char character, int x, int y) {
	int width;

	if (character == ' ') {
		x += 5;
	} else if ((width = g_cine->_textHandler.fontParamTable[(unsigned char)character].characterWidth)) {
		int idx = g_cine->_textHandler.fontParamTable[(unsigned char)character].characterIdx;
		const byte *sprite = g_cine->_textHandler.textTable[idx][FONT_DATA];
		for (uint i = 0; i < FONT_HEIGHT; ++i) {
			byte *dst = _backBuffer + (y + i) * 320 + x;
			for (uint j = 0; j < FONT_WIDTH; ++j, ++dst) {
				// The original does this based on whether bit 1 of the pixel
				// is set. Since that's the only bit ever set in (FW) this
				// check should be fine.
				// TODO: Check how Operation Stealth Amiga works
				if (*sprite++) {
					*dst = 0;
				}
			}
		}
		x += width + 1;
	}

	return x;
}

int FWRenderer::getStringWidth(const char *str) {
	int padding = (g_cine->getGameType() == Cine::GType_OS) ? 2 : 1;
	const char *p = str;
	int width = 0;
	int maxWidth = 0;

	while (*p) {
		unsigned char currChar = (unsigned char)*p;
		if (currChar == '|') {
			maxWidth = MAX<int>(width, maxWidth);
			width = 0;
		} else if (currChar == ' ')
			width += 5;
		else
			width += g_cine->_textHandler.fontParamTable[currChar].characterWidth + padding;
		p++;
	}

	maxWidth = MAX<int>(width, maxWidth);

	return width;
}

/**
 * Draw Line
 * @param x Line end coordinate
 * @param y Line end coordinate
 * @param width Horizontal line length
 * @param height Vertical line length
 * @param color Line color
 * @note Either width or height must be equal to 1
 */
void FWRenderer::drawLine(int x, int y, int width, int height, byte color) {
	// this line is a special case of rectangle ;-)
	drawPlainBox(x, y, width, height, color);
}

/**
 * Hide invisible parts of the sprite
 * @param[in,out] mask Mask to be updated
 * @param it Overlay info from overlayList
 */
void FWRenderer::remaskSprite(byte *mask, Common::List<overlay>::iterator it) {
	AnimData &sprite = g_cine->_animDataTable[g_cine->_objectTable[it->objIdx].frame];
	int x, y, width, height, idx;
	int mx, my, mw, mh;

	x = g_cine->_objectTable[it->objIdx].x;
	y = g_cine->_objectTable[it->objIdx].y;
	width = sprite._realWidth;
	height = sprite._height;

	for (++it; it != g_cine->_overlayList.end(); ++it) {
		if (it->type != 5) {
			continue;
		}

		idx = ABS(g_cine->_objectTable[it->objIdx].frame);
		mx = g_cine->_objectTable[it->objIdx].x;
		my = g_cine->_objectTable[it->objIdx].y;
		mw = g_cine->_animDataTable[idx]._realWidth;
		mh = g_cine->_animDataTable[idx]._height;

		gfxUpdateSpriteMask(mask, x, y, width, height, g_cine->_animDataTable[idx].data(), mx, my, mw, mh);
	}
}

/**
 * Draw background to backbuffer
 */
void FWRenderer::drawBackground() {
	assert(_background);
	memcpy(_backBuffer, _background, _screenSize);
}

void FWRenderer::clearBackBuffer() {
	if (_backBuffer) {
		memset(_backBuffer, 0, _screenSize);
	}
}

/**
 * Draw one overlay
 * @param it Overlay info
 */
void FWRenderer::renderOverlay(const Common::List<overlay>::iterator &it) {
	int idx, len, width;
	ObjectStruct *obj;
	AnimData *sprite;
	byte *mask;

	switch (it->type) {
	// color sprite
	case 0:
		if (g_cine->_objectTable[it->objIdx].frame < 0) {
			return;
		}
		sprite = &g_cine->_animDataTable[g_cine->_objectTable[it->objIdx].frame];
		len = sprite->_realWidth * sprite->_height;
		mask = new byte[len];
		if (sprite->mask() != NULL) {
			memcpy(mask, sprite->mask(), len);
		} else {
			// This case happens in French Amiga Future Wars (Bug #10643) when
			// walking left from the scene with the open manhole cover. This
			// seems to work fine at least in this case.
			memset(mask, 0, len);
		}
		remaskSprite(mask, it);
		drawMaskedSprite(g_cine->_objectTable[it->objIdx], mask);
		delete[] mask;
		break;

	// game message
	case 2:
		if (it->objIdx >= g_cine->_messageTable.size()) {
			return;
		}

		_messageLen += g_cine->_messageTable[it->objIdx].size();
		drawMessage(g_cine->_messageTable[it->objIdx].c_str(), it->x, it->y, it->width, it->color);
		waitForPlayerClick = 1;
		break;

	// action failure message
	case 3:
		idx = it->objIdx * 4 + g_cine->_rnd.getRandomNumber(3);
		len = strlen(failureMessages[idx]);
		_messageLen += len;
		width = 6 * len + 20;
		width = width > 300 ? 300 : width;

		drawMessage(failureMessages[idx], (320 - width) / 2, 80, width, 4);
		waitForPlayerClick = 1;
		break;

	// bitmap
	case 4:
		assert(it->objIdx < NUM_MAX_OBJECT);
		obj = &g_cine->_objectTable[it->objIdx];

		if (obj->frame < 0) {
			return;
		}

		if (!g_cine->_animDataTable[obj->frame].data()) {
			return;
		}

		fillSprite(*obj);
		break;

	default:
		break;
	}
}

/**
 * Draw overlays
 */
void FWRenderer::drawOverlays() {
	// WORKAROUND: Show player behind stairs by moving him behind everything
	// in the scene right after leaving Dr. Why's control room.
	if (g_cine->getGameType() == Cine::GType_OS &&
		g_cine->_overlayList.size() >= 2 &&
		g_cine->_overlayList.back().objIdx == 1 &&
		g_cine->_objectTable.size() >= 2 &&
		g_cine->_objectTable[1].x == 231 &&
		g_cine->_objectTable[1].y >= 142 &&
		scumm_stricmp(renderer->getBgName(), "56VIDE.PI1") == 0) {
		Cine::overlay playerOverlay = g_cine->_overlayList.back();
		g_cine->_overlayList.pop_back();
		g_cine->_overlayList.push_front(playerOverlay);
	}

	Common::List<overlay>::iterator it;

	for (it = g_cine->_overlayList.begin(); it != g_cine->_overlayList.end(); ++it) {
		renderOverlay(it);
	}
}

/**
 * Draw another frame
 */
void FWRenderer::drawFrame(bool wait) {
	drawBackground();
	drawOverlays();

	if (!_cmd.empty()) {
		drawCommand();
	}

	// DIFFERENCE FROM DISASSEMBLY:
	// Waiting for g_cine->getTimerDelay() since last call to this function
	// from mainLoop() was in Future Wars and Operation Stealth disassembly here.
	// The wait did nothing else but simply wait for the waiting period to end.
	// It has been moved to manageEvents() function call in executePlayerInput()
	// to make better use of the waiting period. Now it is used to read mouse button
	// status and possibly update the command line while moving the mouse
	// (e.g. "EXAMINE DOOR" -> "EXAMINE BUTTON").

	if (reloadBgPalOnNextFlip) {
		_activePal = getFadeInSourcePalette();
		reloadBgPalOnNextFlip = 0;
		_changePal = 1; // From disassembly
	}

	if (_changePal) { // From disassembly
		setPalette();
		_changePal = 0; // From disassembly
	}

	const int menus = _menuStack.size();
	for (int i = 0; i < menus; ++i)
		_menuStack[i]->drawMenu(*this, (i == menus - 1));

	blit();

	if (gfxFadeInRequested) {
		fadeFromBlack();
		gfxFadeInRequested = 0;
	}
}

/**
 * Turn on or off the showing of the collision page.
 * If turned on the blitting routine shows the collision page instead of the back buffer.
 * @note Useful for debugging collision page related problems.
 */
void FWRenderer::showCollisionPage(bool state) {
	_showCollisionPage = state;
}

void FWRenderer::blitBackBuffer() {
	blit(false);
}

void FWRenderer::blit(bool useCollisionPage) {
	// Show the back buffer or the collision page. Normally the back
	// buffer but showing the collision page is useful for debugging.
	byte *source = (useCollisionPage ? collisionPage : _backBuffer);
	g_system->copyRectToScreen(source, 320, 0, 0, 320, 200);
	g_system->updateScreen();
}

/**
 * Update screen
 */
void FWRenderer::blit() {
	blit(_showCollisionPage);
}

bool FWRenderer::hasSavedBackBuffer(BackBufferSource source) {
	return source >= 0 && source < MAX_BACK_BUFFER_SOURCES && _savedBackBuffers[source];
}

void FWRenderer::saveBackBuffer(BackBufferSource source) {
	if (_backBuffer && source >= 0 && source < MAX_BACK_BUFFER_SOURCES) {
		if (!_savedBackBuffers[source]) {
			_savedBackBuffers[source] = new byte[_screenSize];
		}
		memcpy(_savedBackBuffers[source], _backBuffer, _screenSize);
	}
}

void FWRenderer::popSavedBackBuffer(BackBufferSource source) {
	restoreSavedBackBuffer(source);
	removeSavedBackBuffer(source);
}

void FWRenderer::restoreSavedBackBuffer(BackBufferSource source) {
	if (_backBuffer && hasSavedBackBuffer(source)) {
		memcpy(_backBuffer, _savedBackBuffers[source], _screenSize);
		blitBackBuffer();
	}
}

void FWRenderer::removeSavedBackBuffer(BackBufferSource source) {
	delete[] _savedBackBuffers[source];
	_savedBackBuffers[source] = nullptr;
}

/**
 * Set player command string
 * @param cmd New command string
 */
void FWRenderer::setCommand(Common::String cmd) {
	_cmd = cmd;
}

Common::String FWRenderer::getCommand() {
	return _cmd;
}

void FWRenderer::setBlackPalette(bool updateChangePal) {
	_activePal.fillWithBlack();
	if (updateChangePal) {
		_changePal = 1; // From disassembly when called from main loop's initialization section
	}
}

void FWRenderer::setPalette() {
	assert(_activePal.isValid() && !_activePal.empty());
	_activePal.setGlobalOSystemPalette();
}

int16 FWRenderer::addBackground(const char *bgName, uint16 bgIdx) {
	error("Future Wars renderer doesn't support multiple backgrounds");
}

/**
 * Load background into renderer
 * @param bg Raw background data
 * @todo Combine with OSRenderer's version of loadBg16
 */
void FWRenderer::loadBg16(const byte *bg, const char *name, unsigned int idx) {
	assert(idx == 0);

	if (!_background) {
		_background = new byte[_screenSize];
	}

	assert(_background);

	Common::strlcpy(_bgName, name, sizeof(_bgName));

	// Load the 16 color palette
	_backupPal.load(bg, kLowPalNumBytes, kLowPalFormat, kLowPalNumColors, CINE_BIG_ENDIAN);

	// Jump over the palette data to the background data
	bg += kLowPalNumBytes;

	gfxConvertSpriteToRaw(_background, bg, 160, 200);
}

/**
 * Placeholder for Operation Stealth implementation
 */
void FWRenderer::loadCt16(const byte *ct, const char *name) {
	error("Future Wars renderer doesn't support multiple backgrounds");
}

/**
 * Placeholder for Operation Stealth implementation
 */
void FWRenderer::loadBg256(const byte *bg, const char *name, unsigned int idx) {
	error("Future Wars renderer doesn't support multiple backgrounds");
}

/**
 * Placeholder for Operation Stealth implementation
 */
void FWRenderer::loadCt256(const byte *ct, const char *name) {
	error("Future Wars renderer doesn't support multiple backgrounds");
}

/**
 * Placeholder for Operation Stealth implementation
 */
void FWRenderer::selectBg(unsigned int idx) {
	error("Future Wars renderer doesn't support multiple backgrounds");
}

/**
 * Placeholder for Operation Stealth implementation
 */
void FWRenderer::selectScrollBg(unsigned int idx) {
	error("Future Wars renderer doesn't support multiple backgrounds");
}

/**
 * Placeholder for Operation Stealth implementation
 */
void FWRenderer::setScroll(unsigned int shift) {
	error("Future Wars renderer doesn't support multiple backgrounds");
}

/**
 * Future Wars has no scrolling backgrounds so scroll value is always zero.
 */
uint FWRenderer::getScroll() const {
	return 0;
}

/**
 * Placeholder for Operation Stealth implementation
 */
void FWRenderer::removeBg(unsigned int idx) {
	error("Future Wars renderer doesn't support multiple backgrounds");
}

void FWRenderer::saveBgNames(Common::OutSaveFile &fHandle) {
	fHandle.write(_bgName, 13);
}

const char *FWRenderer::getBgName(uint idx) const {
	assert(idx == 0);
	return _bgName;
}

/**
 * Restore active and backup palette from save
 * @param fHandle Savefile open for reading
 */
void FWRenderer::restorePalette(Common::SeekableReadStream &fHandle, int version) {
	byte buf[kLowPalNumBytes];

	// Load the active 16 color palette from file
	fHandle.read(buf, kLowPalNumBytes);
	_activePal.load(buf, sizeof(buf), kLowPalFormat, kLowPalNumColors, CINE_BIG_ENDIAN);

	// Load the backup 16 color palette from file
	fHandle.read(buf, kLowPalNumBytes);
	_backupPal.load(buf, sizeof(buf), kLowPalFormat, kLowPalNumColors, CINE_BIG_ENDIAN);

	_changePal = 1; // From disassembly
}

/**
 * Write active and backup palette to save
 * @param fHandle Savefile open for writing
 */
void FWRenderer::savePalette(Common::OutSaveFile &fHandle) {
	byte buf[kLowPalNumBytes];

	// Make sure the active palette has the correct format and color count
	assert(_activePal.colorFormat() == kLowPalFormat);
	assert(_activePal.colorCount() == kLowPalNumColors);

	// Make sure the backup palette has the correct format and color count
	assert(_backupPal.colorFormat() == kLowPalFormat);
	assert(_backupPal.colorCount() == kLowPalNumColors);

	// Write the active palette to the file
	_activePal.save(buf, sizeof(buf), CINE_BIG_ENDIAN);
	fHandle.write(buf, kLowPalNumBytes);

	// Write the backup palette to the file
	_backupPal.save(buf, sizeof(buf), CINE_BIG_ENDIAN);
	fHandle.write(buf, kLowPalNumBytes);
}

/**
 * Write active and backup palette to save
 * @param fHandle Savefile open for writing
 * @todo Add support for saving the palette in the 16 color version of Operation Stealth.
 *       Possibly combine with FWRenderer's savePalette-method?
 */
void OSRenderer::savePalette(Common::OutSaveFile &fHandle) {
	byte buf[kHighPalNumBytes];

	// We can have 16 color palette in many cases
	fHandle.writeUint16LE(_activePal.colorCount());

	// Write the active 256 color palette.
	_activePal.save(buf, sizeof(buf), CINE_LITTLE_ENDIAN);
	fHandle.write(buf, kHighPalNumBytes);

	// Write the backup 256 color palette.
	_backupPal.save(buf, sizeof(buf), CINE_LITTLE_ENDIAN);
	fHandle.write(buf, kHighPalNumBytes);
}

/**
 * Restore active and backup palette from save
 * @param fHandle Savefile open for reading
 */
void OSRenderer::restorePalette(Common::SeekableReadStream &fHandle, int version) {
	byte buf[kHighPalNumBytes];
	uint colorCount = (version > 0) ? fHandle.readUint16LE() : kHighPalNumColors;

	// Load the active color palette
	fHandle.read(buf, kHighPalNumBytes);

	if (colorCount == kHighPalNumColors) {
		// Load the active 256 color palette from file
		_activePal.load(buf, sizeof(buf), kHighPalFormat, kHighPalNumColors, CINE_LITTLE_ENDIAN);
	} else {
		// Load the active 16 color palette from file
		_activePal.load(buf, sizeof(buf), kLowPalFormat, kLowPalNumColors, CINE_LITTLE_ENDIAN);
	}

	// Load the backup color palette
	fHandle.read(buf, kHighPalNumBytes);

	if (colorCount == kHighPalNumColors) {
		_backupPal.load(buf, sizeof(buf), kHighPalFormat, kHighPalNumColors, CINE_LITTLE_ENDIAN);
	} else {
		_backupPal.load(buf, sizeof(buf), kLowPalFormat, kLowPalNumColors, CINE_LITTLE_ENDIAN);
	}

	_changePal = 1; // From disassembly
}

/**
 * Rotate active palette
 * @param a First color to rotate
 * @param b Last color to rotate
 * @param c Possibly rotation step, must be 0 or 1 at the moment
 */
void FWRenderer::rotatePalette(int firstIndex, int lastIndex, int mode) {
	if (mode == 1) {
		_activePal.rotateRight(firstIndex, lastIndex);
	} else if (mode == 2) {
		_activePal.rotateLeft(firstIndex, lastIndex);
	} else {
		_activePal = _backupPal;
	}
	setPalette();
}

void OSRenderer::rotatePalette(int firstIndex, int lastIndex, int mode) {
	if (mode == 1) {
		_activePal.rotateRight(firstIndex, lastIndex);
	} else if (mode == 2) {
		_activePal.rotateLeft(firstIndex, lastIndex);
	} else if (_currentBg > 0 && _currentBg < 8) {
		_activePal = _bgTable[_currentBg].pal;
	} else { // background indices 0 and 8 use backup palette
		_activePal = _backupPal;
	}
	setPalette();
}

/**
 * Copy part of backup palette to active palette and transform
 * @param first First color to transform
 * @param last Last color to transform
 * @param r Red channel transformation
 * @param g Green channel transformation
 * @param b Blue channel transformation
 */
void FWRenderer::transformPalette(int first, int last, int r, int g, int b) {
	if (!_activePal.isValid() || _activePal.empty()) {
		_activePal = Cine::Palette(kLowPalFormat, kLowPalNumColors);
	}

	_backupPal.saturatedAddColor(_activePal, first, last, r, g, b);
	_changePal = 1; // From disassembly
	gfxFadeOutCompleted = 0;
}

uint FWRenderer::fadeDelayMs() {
	// For PC wait for vertical retrace and wait for three timer interrupt ticks.
	// On PC vertical retrace is 70Hz (1000ms / 70 ~= 14.29ms) and
	// timer interrupt tick is set to (10923000ms / 1193180) ~= 9.15ms.
	// So 14.29ms + 3 * 9.15ms ~= 41.74ms ~= 42ms. That's the maximum to wait for PC.
	// Because the vertical retrace might come earlier the minimum to wait is
	// 0ms + 3 * 9.15ms (The wait for three timer ticks is absolute) = 27.45ms ~= 27ms.
	// So the wait on PC is something between 27ms and 42ms.
	// Probably something else on Amiga (Didn't they have 50Hz or 60Hz monitors?).
	return 42;
}

uint FWRenderer::fadeToBlackMinMs() {
	return 1000;
}

/**
 * Fade to black
 * \bug Operation Stealth sometimes seems to fade to black using
 * transformPalette resulting in double fadeout
 */
void FWRenderer::fadeToBlack() {
	assert(_activePal.isValid() && !_activePal.empty());

	bool skipFade = false;
	uint32 now = g_system->getMillis();

	// HACK: Try to cirmumvent double fade outs by throttling function call.
	if (hacksEnabled && _fadeToBlackLastCalledMs != 0 && (now - _fadeToBlackLastCalledMs) < fadeToBlackMinMs()) {
		skipFade = true;
		warning("Skipping fade to black (Time since last called = %d ms < throttling value of %d ms)",
			now - _fadeToBlackLastCalledMs, fadeToBlackMinMs());
	} else {
		_fadeToBlackLastCalledMs = now;
	}

	for (int i = (skipFade ? 7 : 0); i < 8; i++) {
		// Fade out the whole palette by 1/7th
		// (Operation Stealth used 36 / 252, which is 1 / 7. Future Wars used 1 / 7 directly).
		_activePal.saturatedAddNormalizedGray(_activePal, 0, _activePal.colorCount() - 1, -1, 7);

		setPalette();
		g_system->updateScreen();
		g_system->delayMillis(fadeDelayMs());
	}

	clearBackBuffer();
	forbidBgPalReload = gfxFadeOutCompleted = 1;

	// HACK: This is not present in disassembly
	// but this is an attempt to prevent flashing a
	// normally illuminated screen and then fading it in by
	// resetting possible pending background palette reload.
	if (hacksEnabled) {
		reloadBgPalOnNextFlip = 0;
	}
}

void FWRenderer::fadeFromBlack() {
	assert(_activePal.isValid() && !_activePal.empty());

	const Palette& sourcePalette = getFadeInSourcePalette();

	// Initialize active palette to source palette's format and size if they differ
	if (_activePal.colorFormat() != sourcePalette.colorFormat() || _activePal.colorCount() != sourcePalette.colorCount()) {
		_activePal = Cine::Palette(sourcePalette.colorFormat(), sourcePalette.colorCount());
	}

	for (int i = 7; i >= 0; i--) {
		sourcePalette.saturatedAddNormalizedGray(_activePal, 0, _activePal.colorCount() - 1, -i, 7);

		setPalette();
		g_system->updateScreen();
		g_system->delayMillis(fadeDelayMs());
	}

	forbidBgPalReload = gfxFadeOutCompleted = 0;
}

// Menu implementation

void FWRenderer::pushMenu(Menu *menu) {
	_menuStack.push(menu);
}

Menu *FWRenderer::popMenu() {
	if (_menuStack.empty())
		return 0;

	Menu *menu = _menuStack.top();
	_menuStack.pop();
	return menu;
}

void FWRenderer::clearMenuStack() {
	Menu *menu = 0;
	while ((menu = popMenu()) != 0)
		delete menu;
}

SelectionMenu::SelectionMenu(Common::Point p, int width, Common::StringArray elements)
	: Menu(kSelectionMenu), _pos(p), _width(width), _elements(elements), _selection(-1) {
}

void SelectionMenu::setSelection(int selection) {
	if (selection >= getElementCount() || selection < -1) {
		warning("Invalid selection %d", selection);
		selection = -1;
	}

	_selection = selection;
}

void SelectionMenu::drawMenu(FWRenderer &r, bool top) {
	const int height = getElementCount() * 9 + 10;
	int x = _pos.x;
	int y = _pos.y;

	if (x + _width > 319)
		x = 319 - _width;

	if (y + height > 199)
		y = 199 - height;

	byte doubleBorderColor = (r.useTransparentDialogBoxes() ? r.transparentDialogBoxStartColor() : 0) + 2;

	if (r.useTransparentDialogBoxes()) {
		r.drawTransparentBox(x, y, _width, height);
		r.drawDoubleBorder(x, y, _width, height, doubleBorderColor);
	} else {
		r.drawPlainBox(x, y, _width, height, r._messageBg);
		r.drawDoubleBorder(x, y, _width, height, doubleBorderColor);
	}

	int lineY = y + 4;

	const int elemCount = getElementCount();
	for (int i = 0; i < elemCount; ++i, lineY += 9) {
		int charX = x + 4;

		if (i == _selection) {
			int color = (r.useTransparentDialogBoxes() ? 2 : 0);

			if (!top) {
				color += (r.useTransparentDialogBoxes() ? r.transparentDialogBoxStartColor() : 0);
			}

			r.drawPlainBox(x + 2, lineY - 1, _width - 3, 9, color);
		}

		const int size = _elements[i].size();
		for (int j = 0; j < size; ++j) {
			if (r.useTransparentDialogBoxes() && i == _selection) {
				charX = r.undrawChar(_elements[i][j], charX, lineY);
			} else {
				charX = r.drawChar(_elements[i][j], charX, lineY);
			}
		}
	}
}

TextInputMenu::TextInputMenu(Common::Point p, int width, const char *info)
	: Menu(kTextInputMenu), _pos(p), _width(width), _info(info), _input(), _cursor(0) {
}

void TextInputMenu::setInput(const char *input, int cursor) {
	_input = input;
	_cursor = cursor;
}

void TextInputMenu::drawMenu(FWRenderer &r, bool top) {
	const int x = _pos.x;
	const int y = _pos.y;

	int i, tx, ty, tw;
	int line = 0, words = 0, cw = 0;
	int space = 0, extraSpace = 0;

	if (r.useTransparentDialogBoxes())
		r.drawTransparentBox(x, y, _width, 4);
	else
		r.drawPlainBox(x, y, _width, 4, r._messageBg);
	tx = x + 4;
	ty = _info[0] ? y - 5 : y + 4;
	tw = _width - 8;

	const int infoSize = _info.size();

	// input box info message
	for (i = 0; i < infoSize; i++, line--) {
		// fit line of text
		if (!line) {
			line = fitLine(_info.c_str() + i, tw, words, cw);

			if (i + line < infoSize && words) {
				space = (tw - cw) / words;
				extraSpace = (tw - cw) % words;
			} else {
				space = 5;
				extraSpace = 0;
			}

			ty += 9;
			if (r.useTransparentDialogBoxes())
				r.drawTransparentBox(x, ty, _width, 9);
			else
				r.drawPlainBox(x, ty, _width, 9, r._messageBg);
			tx = x + 4;
		}

		// draw characters
		if (_info[i] == ' ') {
			tx += space + extraSpace;

			if (extraSpace) {
				extraSpace = 0;
			}
		} else {
			tx = r.drawChar(_info[i], tx, ty);
		}
	}

	// input area background
	ty += 9;
	if (r.useTransparentDialogBoxes())
		r.drawTransparentBox(x, ty, _width, 9);
	else
		r.drawPlainBox(x, ty, _width, 9, r._messageBg);
	r.drawPlainBox(x + 16, ty - 1, _width - 32, 9, 0);
	tx = x + 20;

	// text in input area
	const int inputSize = _input.size();
	for (i = 0; i < inputSize; i++) {
		tx = r.drawChar(_input[i], tx, ty);

		if (_cursor == i + 2) {
			r.drawLine(tx, ty - 1, 1, 9, 2);
		}
	}

	if (_input.empty() || _cursor == 1) {
		r.drawLine(x + 20, ty - 1, 1, 9, 2);
	}

	ty += 9;
	if (r.useTransparentDialogBoxes())
		r.drawTransparentBox(x, ty, _width, 4);
	else
		r.drawPlainBox(x, ty, _width, 4, r._messageBg);
	r.drawDoubleBorder(x, y, _width, ty - y + 4, (r.useTransparentDialogBoxes() ? r.transparentDialogBoxStartColor() : 0) + 2);
}

// -------------------

/**
 * Initialize Operation Stealth renderer
 */
OSRenderer::OSRenderer() : FWRenderer(), _bgTable(), _currentBg(0), _scrollBg(0),
	_bgShift(0) {

	_bgTable.resize(9); // Resize the background table to its required size
}

/**
 * Destroy Operation Stealth renderer
 */
OSRenderer::~OSRenderer() {
	for (uint i = 0; i < _bgTable.size(); i++) {
		_bgTable[i].clear();
	}
}

bool OSRenderer::initialize() {
	_backupPal = _activePal = Palette(kHighPalFormat, kHighPalNumColors);
	return true;
}

/**
 * Reset Operation Stealth renderer state
 */
void OSRenderer::clear() {
	for (uint i = 0; i < _bgTable.size(); i++) {
		_bgTable[i].clear();
	}

	_currentBg = 0;
	_scrollBg = 0;
	_bgShift = 0;

	FWRenderer::clear();
}

/**
 * Draw 1bpp sprite using selected color on backgrounds
 * @param obj Object info
 * @param fillColor Sprite color
 */
void OSRenderer::incrustMask(const BGIncrust &incrust, uint8 color) {
	const ObjectStruct &obj = g_cine->_objectTable[incrust.objIdx];
	const byte *data = g_cine->_animDataTable[obj.frame].data();
	int x, y, width, height;

	x = incrust.x;
	y = incrust.y;
	width = g_cine->_animDataTable[obj.frame]._realWidth;
	height = g_cine->_animDataTable[obj.frame]._height;

	if (_bgTable[incrust.bgIdx].bg) {
		gfxFillSprite(data, width, height, _bgTable[incrust.bgIdx].bg, x, y, color);
	}
}

const Cine::Palette& OSRenderer::getFadeInSourcePalette() {
	assert(_currentBg <= 8);

	if (_currentBg == 0) {
		return _backupPal;
	} else {
		return _bgTable[_currentBg].pal;
	}
}

/**
 * Draw color sprite
 * @param obj Object info
 */
void OSRenderer::drawSprite(const ObjectStruct &obj) {
	const byte *data = g_cine->_animDataTable[obj.frame].data();
	int x, y, width, height, transColor;

	x = obj.x;
	y = obj.y;
	transColor = obj.part;
	width = g_cine->_animDataTable[obj.frame]._realWidth;
	height = g_cine->_animDataTable[obj.frame]._height;

	drawSpriteRaw2(data, transColor, width, height, _backBuffer, x, y);
}

/**
 * Draw color sprite
 * @param obj Object info
 */
void OSRenderer::incrustSprite(const BGIncrust &incrust) {
	const ObjectStruct &obj = g_cine->_objectTable[incrust.objIdx];
	const byte *data = g_cine->_animDataTable[incrust.frame].data();
	int x, y, width, height, transColor;

	x = incrust.x;
	y = incrust.y;
	transColor = obj.part;
	width = g_cine->_animDataTable[incrust.frame]._realWidth;
	height = g_cine->_animDataTable[incrust.frame]._height;

	if (_bgTable[incrust.bgIdx].bg) {
		// HACK: Fix transparency colors of shadings near walls
		// in labyrinth scene in Operation Stealth after loading a savegame
		// saved in the labyrinth.
		if (hacksEnabled && incrust.objIdx == 1 && incrust.frame < 16 && transColor == 5 &&
			scumm_stricmp(currentPrcName, "LABY.PRC") == 0) {
			transColor = 0;
		}

		drawSpriteRaw2(data, transColor, width, height, _bgTable[incrust.bgIdx].bg, x, y);
	}
}

/**
 * Draw text character on screen
 * @param character Character to draw
 * @param x Character coordinate
 * @param y Character coordinate
 */
int OSRenderer::drawChar(char character, int x, int y) {
	int width;

	if (character == ' ') {
		x += 5;
	} else if ((width = g_cine->_textHandler.fontParamTable[(unsigned char)character].characterWidth)) {
		int idx = g_cine->_textHandler.fontParamTable[(unsigned char)character].characterIdx;
		drawSpriteRaw2(g_cine->_textHandler.textTable[idx][FONT_DATA], 0, FONT_WIDTH, FONT_HEIGHT, _backBuffer, x, y);
		x += width + 1;
	}

	return x;
}

/**
 * Draw background to backbuffer
 */
void OSRenderer::drawBackground() {
	byte *main;

	main = _bgTable[_currentBg].bg;
	assert(main);

	if (!_bgShift) {
		memcpy(_backBuffer, main, _screenSize);
	} else {
		unsigned int rowShift = _bgShift % 200;
		byte *scroll = _bgTable[_scrollBg].bg;
		assert(scroll);

		if (!rowShift) {
			memcpy(_backBuffer, scroll, _screenSize);
		} else {
			int mainShift = rowShift * _screenWidth;
			int mainSize = _screenSize - mainShift;

			if (mainSize > 0) { // Just a precaution
				memcpy(_backBuffer, main + mainShift, mainSize);
			}
			if (mainShift > 0) { // Just a precaution
				memcpy(_backBuffer + mainSize, scroll, mainShift);
			}
		}
	}
}

/**
 * Draw one overlay
 * @param it Overlay info
 * @todo Add handling of type 22 overlays
 */
void OSRenderer::renderOverlay(const Common::List<overlay>::iterator &it) {
	int len, idx, width, height;
	ObjectStruct *obj;
	AnimData *sprite;
	byte color, transparentColor;
	bool useTopLeftForTransCol = false;

	switch (it->type) {
	// color sprite
	case 0:
		if (g_cine->_objectTable[it->objIdx].frame < 0) {
			break;
		}

		sprite = &g_cine->_animDataTable[g_cine->_objectTable[it->objIdx].frame];
		obj = &g_cine->_objectTable[it->objIdx];
		transparentColor = obj->part & 0x0F;

		// HACK: Correct transparency color from 6 to 0 for the first frame of sea animation
		// in 16 color DOS version of Operation Stealth in the flower shop scene
		// (The scene in which the player arrives immediately after leaving the airport).
		if (hacksEnabled && it->objIdx == 141 && obj->frame == 100 && obj->part == 6 && sprite->_bpp == 4 &&
			scumm_stricmp(currentPrcName, "AIRPORT.PRC") == 0 &&
			scumm_stricmp(renderer->getBgName(), "21.PI1") == 0) {
			useTopLeftForTransCol = true;
		}

		// HACK: Correct transparency color from 8 to 51 for the player's walking animation
		// in 256 color DOS version of Operation Stealth in the scene right after
		// leaving Dr. Why's control room.
		if (hacksEnabled && it->objIdx == 1 && obj->part == 8 && sprite->_bpp == 5 &&
			scumm_stricmp(currentPrcName, "ILE.PRC") == 0 &&
			scumm_stricmp(renderer->getBgName(), "56VIDE.PI1") == 0) {
			useTopLeftForTransCol = true;
		}

		// HACK: Correct transparency color from 1 to 3 for the player emerging from a manhole
		// in 256 color DOS version of Operation Stealth when entering the Dr. Why's island.
		if (hacksEnabled && it->objIdx == 43 && obj->frame >= 100 && obj->frame <= 102 &&
			obj->part == 1 && sprite->_bpp == 5 &&
			scumm_stricmp(currentPrcName, "SOUSMAR2.PRC") == 0 &&
			scumm_stricmp(renderer->getBgName(), "56.PI1") == 0) {
			useTopLeftForTransCol = true;
		}

		if (useTopLeftForTransCol) {
			// Use top left corner value for transparency
			transparentColor = sprite->getColor(0, 0);
		}

		drawSprite(&(*it), sprite->data(), sprite->_realWidth, sprite->_height, _backBuffer, g_cine->_objectTable[it->objIdx].x, g_cine->_objectTable[it->objIdx].y, transparentColor, sprite->_bpp);
		break;

	// game message
	case 2:
		if (it->objIdx >= g_cine->_messageTable.size()) {
			return;
		}

		_messageLen += g_cine->_messageTable[it->objIdx].size();
		drawMessage(g_cine->_messageTable[it->objIdx].c_str(), it->x, it->y, it->width, it->color);
		if (it->color >= 0) { // This test isn't in Future Wars's implementation
			waitForPlayerClick = 1;
		}
		break;

	// action failure message
	case 3:
		idx = it->objIdx * 4 + g_cine->_rnd.getRandomNumber(3);
		len = strlen(failureMessages[idx]);
		_messageLen += len;
		width = 6 * len + 20;
		width = width > 300 ? 300 : width;

		// The used color here differs from Future Wars
		drawMessage(failureMessages[idx], (320 - width) / 2, 80, width, _messageBg);
		waitForPlayerClick = 1;
		break;

	// bitmap
	case 4:
		if (g_cine->_objectTable[it->objIdx].frame >= 0) {
			FWRenderer::renderOverlay(it);
		}
		break;

	// masked background
	case 20:
		assert(it->objIdx < NUM_MAX_OBJECT);
		lastType20OverlayBgIdx = it->x; // A global variable updated here!
		obj = &g_cine->_objectTable[it->objIdx];
		sprite = &g_cine->_animDataTable[obj->frame];

		if (obj->frame < 0 || it->x < 0 || it->x > 8 || !_bgTable[it->x].bg || sprite->_bpp != 1) {
			break;
		}

		maskBgOverlay(it->x, _bgTable[it->x].bg, sprite->data(), sprite->_realWidth, sprite->_height, _backBuffer, obj->x, obj->y);
		break;

	// line drawing
	case 22:
		assert(it->objIdx < NUM_MAX_OBJECT);
		obj = &g_cine->_objectTable[it->objIdx];
		color = obj->part & 0x0F;
		width = obj->frame;
		height = obj->costume;
		// Using Bresenham's algorithm, looks good enough for visual purposes in Operation Stealth
		Graphics::drawLine(obj->x, obj->y, width, height, color, plotPoint, _backBuffer);
		break;

	// something else
	default:
		FWRenderer::renderOverlay(it);
		break;
	}
}

/**
 * Copy part of backup palette to active palette and transform
 * @param first First color to transform
 * @param last Last color to transform
 * @param r Red channel transformation
 * @param g Green channel transformation
 * @param b Blue channel transformation
 */
void OSRenderer::transformPalette(int first, int last, int r, int g, int b) {
	// Background indices 0 and 8 use backup palette
	const Cine::Palette& srcPal =
		(_currentBg > 0 && _currentBg < 8) ? _bgTable[_currentBg].pal : _backupPal;

	// Initialize active palette to current background's palette format and size if they differ
	if (_activePal.colorFormat() != srcPal.colorFormat() || _activePal.colorCount() != srcPal.colorCount()) {
		_activePal = Cine::Palette(srcPal.colorFormat(), srcPal.colorCount());
	}

	// If asked to change whole 16 color palette then
	// assume it means the whole palette regardless of size.
	// In Operation Stealth DOS 16 color and 256 color disassembly mapping was from 0-15 to 0-255.
	if (first == 0 && last == 15) {
		last = srcPal.colorCount() - 1;
	}

	srcPal.saturatedAddColor(_activePal, first, last, r, g, b, kLowPalFormat);
	_changePal = 1; // From disassembly
	gfxFadeOutCompleted = 0;
}

int16 OSRenderer::addBackground(const char *bgName, uint16 bgIdx) {
	byte *ptr, *dataPtr;

	int16 fileIdx = findFileInBundle(bgName);
	if (fileIdx < 0) {
		warning("OSRenderer::addBackground(\"%s\", %d): Could not find background in file bundle.", bgName, bgIdx);
		return -1;
	}
	checkDataDisk(-1);
	ptr = dataPtr = readBundleFile(fileIdx);

	uint16 bpp = READ_BE_UINT16(ptr); ptr += 2;

	if (!_bgTable[bgIdx].bg) {
		_bgTable[bgIdx].bg = new byte[_screenSize];
	}

	Common::strlcpy(_bgTable[bgIdx].name, bgName, sizeof(_bgTable[bgIdx].name));

	if (bpp == 8) {
		_bgTable[bgIdx].pal.load(ptr, kHighPalNumBytes, kHighPalFormat, kHighPalNumColors, CINE_LITTLE_ENDIAN);
		memcpy(_bgTable[bgIdx].bg, ptr + kHighPalNumBytes, _screenSize);
	} else {
		_bgTable[bgIdx].pal.load(ptr, kLowPalNumBytes, kLowPalFormat, kLowPalNumColors, CINE_BIG_ENDIAN);
		gfxConvertSpriteToRaw(_bgTable[bgIdx].bg, ptr + kLowPalNumBytes, 160, 200);
	}
	free(dataPtr);
	return 0;
}

/**
 * Load 16 color background into renderer
 * @param bg Raw background data
 * @param name Background filename
 * @param pos Background index
 * @todo Combine with FWRenderer's version of loadBg16
 */
void OSRenderer::loadBg16(const byte *bg, const char *name, unsigned int idx) {
	assert(idx < 9);

	if (!_bgTable[idx].bg) {
		_bgTable[idx].bg = new byte[_screenSize];
	}

	assert(_bgTable[idx].bg);

	Common::strlcpy(_bgTable[idx].name, name, sizeof(_bgTable[idx].name));

	// Load the 16 color palette
	_backupPal.load(bg, kLowPalNumBytes, kLowPalFormat, kLowPalNumColors, CINE_BIG_ENDIAN);

	// Jump over the palette data to the background data
	bg += kLowPalNumBytes;

	gfxConvertSpriteToRaw(_bgTable[idx].bg, bg, 160, 200);
}

/**
 * Load 16 color CT data as background into renderer
 * @param ct Raw CT data
 * @param name Background filename
 */
void OSRenderer::loadCt16(const byte *ct, const char *name) {
	assert(collisionPage);

	// Make the 9th background point directly to the collision page
	// and load the picture into it.
	setBackground8ToCollisionPage();
	_bgTable[kCollisionPageBgIdxAlias].pal.load(ct, kLowPalNumBytes, kLowPalFormat, kLowPalNumColors, CINE_BIG_ENDIAN);
	gfxConvertSpriteToRaw(_bgTable[kCollisionPageBgIdxAlias].bg, ct + kLowPalNumBytes, 160, 200);
}

void OSRenderer::setBackground8ToCollisionPage() {
	byte* oldBg = _bgTable[kCollisionPageBgIdxAlias].bg;
	if (oldBg && oldBg != collisionPage) {
		delete[] _bgTable[kCollisionPageBgIdxAlias].bg;
	}
	_bgTable[kCollisionPageBgIdxAlias].bg = collisionPage;
}

/**
 * Load 256 color background into renderer
 * @param bg Raw background data
 * @param name Background filename
 * @param pos Background index
 */
void OSRenderer::loadBg256(const byte *bg, const char *name, unsigned int idx) {
	assert(idx < 9);

	if (!_bgTable[idx].bg) {
		_bgTable[idx].bg = new byte[_screenSize];
	}

	assert(_bgTable[idx].bg);

	Common::strlcpy(_bgTable[idx].name, name, sizeof(_bgTable[idx].name));
	_backupPal.load(bg, kHighPalNumBytes, kHighPalFormat, kHighPalNumColors, CINE_LITTLE_ENDIAN);

	memcpy(_bgTable[idx].bg, bg + kHighPalNumBytes, _screenSize);
}

/**
 * Load 256 color CT data as background into renderer
 * @param ct Raw CT data
 * @param name Background filename
 */
void OSRenderer::loadCt256(const byte *ct, const char *name) {
	assert(collisionPage);

	// Make the 9th background point directly to the collision page
	// and load the picture into it.
	setBackground8ToCollisionPage();
	_bgTable[kCollisionPageBgIdxAlias].pal.load(ct, kHighPalNumBytes, kHighPalFormat, kHighPalNumColors, CINE_LITTLE_ENDIAN);
	memcpy(_bgTable[kCollisionPageBgIdxAlias].bg, ct + kHighPalNumBytes, _screenSize);
}

/**
 * Select active background and load its palette
 * @param idx Background index
 */
void OSRenderer::selectBg(unsigned int idx) {
	assert(idx < 9);

	if (idx <= 8 && _bgTable[idx].bg) {
		_currentBg = idx;
		if (!forbidBgPalReload) {
			reloadBgPalOnNextFlip = 1;
		}
	} else
		warning("OSRenderer::selectBg(%d) - attempt to select null background", idx);
}

/**
 * Select scroll background
 * @param idx Scroll background index
 */
void OSRenderer::selectScrollBg(unsigned int idx) {
	assert(idx < 9);

	if (idx <= 8 && _bgTable[idx].bg) {
		_scrollBg = idx;
	}
}

/**
 * Set background scroll
 * @param shift Background scroll in pixels
 */
void OSRenderer::setScroll(unsigned int shift) {
	_bgShift = shift % 400;
}

/**
 * Get background scroll
 * @return Background scroll in pixels
 */
uint OSRenderer::getScroll() const {
	return _bgShift;
}

/**
 * Unload background from renderer
 * @param idx Background to unload
 */
void OSRenderer::removeBg(unsigned int idx) {
	assert(idx > 0 && idx < 9);

	if (_currentBg == idx) {
		_currentBg = 0;
	}

	if (_scrollBg == idx) {
		_scrollBg = 0;
	}

	_bgTable[idx].clear();
}

void OSRenderer::saveBgNames(Common::OutSaveFile &fHandle) {
	for (int i = 0; i < 8; i++) {
		fHandle.write(_bgTable[i].name, 13);
	}
}

const char *OSRenderer::getBgName(uint idx) const {
	assert(idx < 9);
	return _bgTable[idx].name;
}

void setMouseCursor(int cursor) {
	static int currentMouseCursor = -1;
	assert(cursor >= 0 && cursor < 3);
	if (currentMouseCursor != cursor) {
		byte mouseCursor[16 * 16];
		const MouseCursor *mc = &mouseCursors[cursor];
		const byte *src = mc->bitmap;
		for (int i = 0; i < 32; ++i) {
			int offs = i * 8;
			for (byte mask = 0x80; mask != 0; mask >>= 1) {
				if (src[0] & mask) {
					mouseCursor[offs] = 1;
				} else if (src[32] & mask) {
					mouseCursor[offs] = 0;
				} else {
					mouseCursor[offs] = 0xFF;
				}
				++offs;
			}
			++src;
		}
		CursorMan.replaceCursor(mouseCursor, 16, 16, mc->hotspotX, mc->hotspotY, 0xFF);
		CursorMan.replaceCursorPalette(cursorPalette, 0, 2);
		currentMouseCursor = cursor;
	}
}

void gfxFillSprite(const byte *spritePtr, uint16 width, uint16 height, byte *page, int16 x, int16 y, uint8 fillColor) {
	int16 i, j;

	for (i = 0; i < height; i++) {
		byte *destPtr = page + x + y * 320;
		destPtr += i * 320;

		for (j = 0; j < width; j++) {
			if (x + j >= 0 && x + j < 320 && i + y >= 0 && i + y < 200 && !*spritePtr) {
				*destPtr = fillColor;
			}

			destPtr++;
			spritePtr++;
		}
	}
}

void gfxDrawMaskedSprite(const byte *spritePtr, const byte *maskPtr, uint16 width, uint16 height, byte *page, int16 x, int16 y) {
	int16 i, j;

	for (i = 0; i < height; i++) {
		byte *destPtr = page + x + y * 320;
		destPtr += i * 320;

		for (j = 0; j < width; j++) {
			if (x + j >= 0 && x + j < 320 && i + y >= 0 && i + y < 200 && *maskPtr == 0) {
				*destPtr = *spritePtr;
			}
			++destPtr;
			++spritePtr;
			++maskPtr;
		}
	}
}

void gfxUpdateSpriteMask(byte *destMask, int16 x, int16 y, int16 width, int16 height, const byte *srcMask, int16 xm, int16 ym, int16 maskWidth, int16 maskHeight) {
	int16 i, j, d, spritePitch, maskPitch;

	spritePitch = width;
	maskPitch = maskWidth;

	// crop update area to overlapping parts of masks
	if (y > ym) {
		d = y - ym;
		srcMask += d * maskPitch;
		maskHeight -= d;
	} else if (y < ym) {
		d = ym - y;
		destMask += d * spritePitch;
		height -= d;
	}

	if (x > xm) {
		d = x - xm;
		srcMask += d;
		maskWidth -= d;
	} else if (x < xm) {
		d = xm - x;
		destMask += d;
		width -= d;
	}

	// update mask
	for (j = 0; j < MIN(maskHeight, height); ++j) {
		for (i = 0; i < MIN(maskWidth, width); ++i) {
			destMask[i] |= srcMask[i] ^ 1;
		}
		destMask += spritePitch;
		srcMask += maskPitch;
	}
}

void gfxUpdateIncrustMask(byte *destMask, int16 x, int16 y, int16 width, int16 height, const byte *srcMask, int16 xm, int16 ym, int16 maskWidth, int16 maskHeight) {
	int16 i, j, d, spritePitch, maskPitch;

	spritePitch = width;
	maskPitch = maskWidth;

	// crop update area to overlapping parts of masks
	if (y > ym) {
		d = y - ym;
		srcMask += d * maskPitch;
		maskHeight -= d;
	} else if (y < ym) {
		d = ym - y > height ? height : ym - y;
		memset(destMask, 1, d * spritePitch);
		destMask += d * spritePitch;
		height -= d;
	}

	if (x > xm) {
		d = x - xm;
		xm = x;
		srcMask += d;
		maskWidth -= d;
	}

	d = xm - x;
	maskWidth += d;

	// update mask
	for (j = 0; j < MIN(maskHeight, height); ++j) {
		for (i = 0; i < width; ++i) {
			destMask[i] |= i < d || i >= maskWidth ? 1 : srcMask[i - d];
		}
		destMask += spritePitch;
		srcMask += maskPitch;
	}

	if (j < height) {
		memset(destMask, 1, (height - j) * spritePitch);
	}
}

void gfxDrawLine(int16 x1, int16 y1, int16 x2, int16 y2, byte color, byte *page) {
	if (x1 == x2) {
		if (y1 > y2) {
			SWAP(y1, y2);
		}
		while (y1 <= y2) {
			*(page + (y1 * 320 + x1)) = color;
			y1++;
		}
	} else {
		if (x1 > x2) {
			SWAP(x1, x2);
		}
		while (x1 <= x2) {
			*(page + (y1 * 320 + x1)) = color;
			x1++;
		}
	}

}

void gfxDrawPlainBoxRaw(int16 x1, int16 y1, int16 x2, int16 y2, byte color, byte *page) {
	int16 t;

	if (x1 > x2) {
		SWAP(x1, x2);
	}

	if (y1 > y2) {
		SWAP(y1, y2);
	}

	t = x1;
	while (y1 <= y2) {
		x1 = t;
		while (x1 <= x2) {
			*(page + y1 * 320 + x1) = color;
			x1++;
		}
		y1++;
	}
}

int16 gfxGetBit(int16 x, int16 y, const byte *ptr, int16 width) {
	const byte *ptrToData = (ptr) + y * width + x;

	if (x > width) {
		return 0;
	}

	if (*ptrToData) {
		return 0;
	}

	return 1;
}

void gfxResetRawPage(byte *pageRaw) {
	memset(pageRaw, 0, 320 * 200);
}

void gfxConvertSpriteToRaw(byte *dst, const byte *src, uint16 w, uint16 h) {
	// Output is 4 bits per pixel.
	// Pixels are in 16 pixel chunks (8 bytes of source per 16 pixels of output).
	// The source data is interleaved so that
	// 1st big-endian 16-bit value contains all bit position 0 values for 16 pixels,
	// 2nd big-endian 16-bit value contains all bit position 1 values for 16 pixels,
	// 3rd big-endian 16-bit value contains all bit position 2 values for 16 pixels,
	// 4th big-endian 16-bit value contains all bit position 3 values for 16 pixels.
	// 1st pixel's bits are in the 16th bits,
	// 2nd pixel's bits are in the 15th bits,
	// 3rd pixel's bits are in the 14th bits etc.
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w / 8; ++x) {
			for (int bit = 0; bit < 16; ++bit) {
				uint8 color = 0;
				for (int p = 0; p < 4; ++p) {
					if (READ_BE_UINT16(src + p * 2) & (1 << (15 - bit))) {
						color |= 1 << p;
					}
				}
				*dst++ = color;
			}
			src += 8;
		}
	}
}

void drawSpriteRaw(const byte *spritePtr, const byte *maskPtr, int16 width, int16 height, byte *page, int16 x, int16 y) {
	int16 i, j;

	// FIXME: Is it a bug if maskPtr == NULL?
	if (!maskPtr)
		warning("drawSpriteRaw: maskPtr == NULL");

	for (i = 0; i < height; i++) {
		byte *destPtr = page + x + y * 320;
		destPtr += i * 320;

		for (j = 0; j < width; j++) {
			if ((!maskPtr || !(*maskPtr)) && x + j >= 0 && x + j < 320 && i + y >= 0 && i + y < 200) {
				*(destPtr++) = *(spritePtr++);
			} else {
				destPtr++;
				spritePtr++;
			}

			if (maskPtr)
				maskPtr++;
		}
	}
}

void OSRenderer::drawSprite(overlay *overlayPtr, const byte *spritePtr, int16 width, int16 height, byte *page, int16 x, int16 y, byte transparentColor, byte bpp) {
	byte *pMask = NULL;

	// draw the mask based on next objects in the list
	Common::List<overlay>::iterator it;
	for (it = g_cine->_overlayList.begin(); it != g_cine->_overlayList.end(); ++it) {
		if (&(*it) == overlayPtr) {
			break;
		}
	}

	while (it != g_cine->_overlayList.end()) {
		overlay *pCurrentOverlay = &(*it);
		if ((pCurrentOverlay->type == 5) || ((pCurrentOverlay->type == 21) && (pCurrentOverlay->x == overlayPtr->objIdx))) {
			AnimData *sprite = &g_cine->_animDataTable[g_cine->_objectTable[it->objIdx].frame];

			if (pMask == NULL) {
				pMask = new byte[width * height];

				for (int i = 0; i < height; i++) {
					for (int j = 0; j < width; j++) {
						byte spriteColor = spritePtr[width * i + j];
						pMask[width * i + j] = spriteColor;
					}
				}
			}

			for (int i = 0; i < sprite->_realWidth; i++) {
				for (int j = 0; j < sprite->_height; j++) {
					int inMaskX = (g_cine->_objectTable[it->objIdx].x + i) - x;
					int inMaskY = (g_cine->_objectTable[it->objIdx].y + j) - y;

					if (inMaskX >= 0 && inMaskX < width) {
						if (inMaskY >= 0 && inMaskY < height) {
							if (sprite->_bpp == 1) {
								if (!sprite->getColor(i, j)) {
									pMask[inMaskY * width + inMaskX] = page[x + y * 320 + inMaskX + inMaskY * 320];
								}
							}
						}
					}
				}
			}
		}
		++it;
	}

	// now, draw with the mask we created
	if (pMask) {
		spritePtr = pMask;
	}

	// ignore transparent color in 1bpp
	if (bpp == 1) {
		transparentColor = 1;
	}

	{
		for (int i = 0; i < height; i++) {
			byte *destPtr = page + x + y * 320;
			destPtr += i * 320;

			for (int j = 0; j < width; j++) {
				byte color = *(spritePtr++);
				if ((transparentColor != color) && x + j >= 0 && x + j < 320 && i + y >= 0 && i + y < 200) {
					*(destPtr++) = color;
				} else {
					destPtr++;
				}
			}
		}
	}

	delete[] pMask;
}

void drawSpriteRaw2(const byte *spritePtr, byte transColor, int16 width, int16 height, byte *page, int16 x, int16 y) {
	int16 i, j;

	for (i = 0; i < height; i++) {
		byte *destPtr = page + x + y * 320;
		destPtr += i * 320;

		for (j = 0; j < width; j++) {
			if ((*spritePtr != transColor) && (x + j >= 0 && x + j < 320 && i + y >= 0 && i + y < 200)) {
				*destPtr = *spritePtr;
			}
			destPtr++;
			spritePtr++;
		}
	}
}

void maskBgOverlay(int targetBgIdx, const byte *bgPtr, const byte *maskPtr, int16 width, int16 height,
				   byte *page, int16 x, int16 y) {
	int16 i, j, tmpWidth, tmpHeight;
	Common::List<BGIncrust>::iterator it;
	const byte *backup = maskPtr;

	// background pass
	for (i = 0; i < height; i++) {
		byte *destPtr = page + x + y * 320;
		const byte *srcPtr = bgPtr + x + y * 320;
		destPtr += i * 320;
		srcPtr += i * 320;

		for (j = 0; j < width; j++) {
			if ((!maskPtr || !(*maskPtr)) && (x + j >= 0
					&& x + j < 320 && i + y >= 0 && i + y < 200)) {
				*destPtr = *srcPtr;
			}

			destPtr++;
			srcPtr++;

			if (maskPtr)
				maskPtr++;
		}
	}

	maskPtr = backup;

	// incrust pass
	for (it = g_cine->_bgIncrustList.begin(); it != g_cine->_bgIncrustList.end(); ++it) {
		// HACK: Remove drawing of red corners around doors in rat maze in Operation Stealth
		// by skipping drawing of possible collision table data to non-collision table page.
		if (hacksEnabled && it->bgIdx == kCollisionPageBgIdxAlias && targetBgIdx != kCollisionPageBgIdxAlias) {
			continue;
		}

		tmpWidth = g_cine->_animDataTable[it->frame]._realWidth;
		tmpHeight = g_cine->_animDataTable[it->frame]._height;
		byte *mask = (byte *)malloc(tmpWidth * tmpHeight);

		if (it->param == 0) {
			generateMask(g_cine->_animDataTable[it->frame].data(), mask, tmpWidth * tmpHeight, it->part);
			gfxUpdateIncrustMask(mask, it->x, it->y, tmpWidth, tmpHeight, maskPtr, x, y, width, height);
			gfxDrawMaskedSprite(g_cine->_animDataTable[it->frame].data(), mask, tmpWidth, tmpHeight, page, it->x, it->y);
		} else {
			memcpy(mask, g_cine->_animDataTable[it->frame].data(), tmpWidth * tmpHeight);
			gfxUpdateIncrustMask(mask, it->x, it->y, tmpWidth, tmpHeight, maskPtr, x, y, width, height);
			gfxFillSprite(mask, tmpWidth, tmpHeight, page, it->x, it->y);
		}

		free(mask);
	}
}

} // End of namespace Cine
