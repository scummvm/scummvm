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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "cine/cine.h"
#include "cine/bg.h"
#include "cine/bg_list.h"
#include "cine/various.h"
#include "cine/pal.h"

#include "common/endian.h"
#include "common/system.h"

#include "graphics/cursorman.h"

namespace Cine {

byte *page3Raw;
FWRenderer *renderer = NULL;

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

/*! \brief Initialize renderer
 */
FWRenderer::FWRenderer() : _background(NULL), _palette(NULL), _cmd(""),
	_cmdY(0), _messageBg(0), _backBuffer(new byte[_screenSize]),
	_activeLowPal(NULL), _changePal(0) {

	assert(_backBuffer);

	memset(_backBuffer, 0, _screenSize);
	memset(_bgName, 0, sizeof (_bgName));
}

/* \brief Destroy renderer
 */
FWRenderer::~FWRenderer() {
	delete[] _background;
	delete[] _palette;
	delete[] _backBuffer;
	delete[] _activeLowPal;
}

/* \brief Reset renderer state
 */
void FWRenderer::clear() {
	delete[] _background;
	delete[] _palette;
	delete[] _activeLowPal;

	_background = NULL;
	_palette = NULL;
	_activeLowPal = NULL;

	memset(_backBuffer, 0, _screenSize);

	_cmd = "";
	_cmdY = 0;
	_messageBg = 0;
	_changePal = 0;
}

/*! \brief Draw 1bpp sprite using selected color
 * \param obj Object info
 * \param fillColor Sprite color
 */
void FWRenderer::fillSprite(const objectStruct &obj, uint8 color) {
	const byte *data = animDataTable[obj.frame].data();
	int x, y, width, height;

	x = obj.x;
	y = obj.y;
	width = animDataTable[obj.frame]._realWidth;
	height = animDataTable[obj.frame]._height;

	gfxFillSprite(data, width, height, _backBuffer, x, y, color);
}

/*! \brief Draw 1bpp sprite using selected color on background
 * \param obj Object info
 * \param fillColor Sprite color
 */
void FWRenderer::incrustMask(const objectStruct &obj, uint8 color) {
	const byte *data = animDataTable[obj.frame].data();
	int x, y, width, height;

	x = obj.x;
	y = obj.y;
	width = animDataTable[obj.frame]._realWidth;
	height = animDataTable[obj.frame]._height;

	gfxFillSprite(data, width, height, _background, x, y, color);
}

/*! \brief Draw color sprite using with external mask
 * \param obj Object info
 * \param mask External mask
 */
void FWRenderer::drawMaskedSprite(const objectStruct &obj, const byte *mask) {
	const byte *data = animDataTable[obj.frame].data();
	int x, y, width, height;

	x = obj.x;
	y = obj.y;
	width = animDataTable[obj.frame]._realWidth;
	height = animDataTable[obj.frame]._height;

	assert(mask);

	drawSpriteRaw(data, mask, width, height, _backBuffer, x, y);
}

/*! \brief Draw color sprite
 * \param obj Object info
 */
void FWRenderer::drawSprite(const objectStruct &obj) {
	const byte *mask = animDataTable[obj.frame].mask();
	drawMaskedSprite(obj, mask);
}

/*! \brief Draw color sprite on background
 * \param obj Object info
 */
void FWRenderer::incrustSprite(const objectStruct &obj) {
	const byte *data = animDataTable[obj.frame].data();
	const byte *mask = animDataTable[obj.frame].mask();
	int x, y, width, height;

	x = obj.x;
	y = obj.y;
	width = animDataTable[obj.frame]._realWidth;
	height = animDataTable[obj.frame]._height;

	assert(mask);

	drawSpriteRaw(data, mask, width, height, _background, x, y);
}

/*! \brief Draw command box on screen
 */
void FWRenderer::drawCommand() {
	unsigned int i;
	int x = 10, y = _cmdY;

	drawPlainBox(x, y, 301, 11, 0);
	drawBorder(x - 1, y - 1, 302, 12, 2);

	x += 2;
	y += 2;

	for (i = 0; i < _cmd.size(); i++) {
		x = drawChar(_cmd[i], x, y);
	}
}

/*! \brief Draw message in a box
 * \param str Message to draw
 * \param x Top left message box corner coordinate
 * \param y Top left message box corner coordinate
 * \param width Message box width
 * \param color Message box background color
 */
void FWRenderer::drawMessage(const char *str, int x, int y, int width, byte color) {
	int i, tx, ty, tw;
	int line = 0, words = 0, cw = 0;
	int space = 0, extraSpace = 0;

	drawPlainBox(x, y, width, 4, color);
	tx = x + 4;
	ty = str[0] ? y - 5 : y + 4;
	tw = width - 8;

	for (i = 0; str[i]; i++, line--) {
		// Fit line of text into textbox
		if (!line) {
			while (str[i] == ' ') i++;
			line = fitLine(str + i, tw, words, cw);

			if ( str[i + line] != '\0' && str[i + line] != 0x7C && words) {
				space = (tw - cw) / words;
				extraSpace = (tw - cw) % words;
			} else {
				space = 5;
				extraSpace = 0;
			}

			ty += 9;
			drawPlainBox(x, ty, width, 9, color);
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
	drawPlainBox(x, ty, width, 4, color);
	drawDoubleBorder(x, y, width, ty - y + 4, 2);
}

/*! \brief Draw rectangle on screen
 * \param x Top left corner coordinate
 * \param y Top left corner coordinate
 * \param width Rectangle width
 * \param height Rectangle height
 * \param color Fill color
 */
void FWRenderer::drawPlainBox(int x, int y, int width, int height, byte color) {
	int i;
	byte *dest = _backBuffer + y * 320 + x;

	if (width < 0) {
		x += width;
		width = -width;
	}

	if (height < 0) {
		y += height;
		height = -height;
	}

	for (i = 0; i < height; i++) {
		memset(dest + i * 320, color, width);
	}
}

/*! \brief Draw empty rectangle
 * \param x Top left corner coordinate
 * \param y Top left corner coordinate
 * \param width Rectangle width
 * \param height Rectangle height
 * \param color Line color
 */
void FWRenderer::drawBorder(int x, int y, int width, int height, byte color) {
	drawLine(x, y, width, 1, color);
	drawLine(x, y + height, width, 1, color);
	drawLine(x, y, 1, height, color);
	drawLine(x + width, y, 1, height + 1, color);
}

/*! \brief Draw empty 2 color rectangle (inner line color is black)
 * \param x Top left corner coordinate
 * \param y Top left corner coordinate
 * \param width Rectangle width
 * \param height Rectangle height
 * \param color Outter line color
 */
void FWRenderer::drawDoubleBorder(int x, int y, int width, int height, byte color) {
	drawBorder(x + 1, y + 1, width - 2, height - 2, 0);
	drawBorder(x, y, width, height, color);
}

/*! \brief Draw text character on screen
 * \param character Character to draw
 * \param x Character coordinate
 * \param y Character coordinate
 */
int FWRenderer::drawChar(char character, int x, int y) {
	int width, idx;

	if (character == ' ') {
		x += 5;
	} else if ((width = fontParamTable[(unsigned char)character].characterWidth)) {
		idx = fontParamTable[(unsigned char)character].characterIdx;
		drawSpriteRaw(g_cine->_textHandler.textTable[idx][0], g_cine->_textHandler.textTable[idx][1], 16, 8, _backBuffer, x, y);
		x += width + 1;
	}

	return x;
}

/*! \brief Draw Line
 * \param x Line end coordinate
 * \param y Line end coordinate
 * \param width Horizontal line length
 * \param height Vertical line length
 * \param color Line color
 * \note Either width or height must be equal to 1
 */
void FWRenderer::drawLine(int x, int y, int width, int height, byte color) {
	// this line is a special case of rectangle ;-)
	drawPlainBox(x, y, width, height, color);
}

/*! \brief Hide invisible parts of the sprite
 * \param[in,out] mask Mask to be updated
 * \param it Overlay info from overlayList
 */
void FWRenderer::remaskSprite(byte *mask, Common::List<overlay>::iterator it) {
	AnimData &sprite = animDataTable[objectTable[it->objIdx].frame];
	int x, y, width, height, idx;
	int mx, my, mw, mh;

	x = objectTable[it->objIdx].x;
	y = objectTable[it->objIdx].y;
	width = sprite._realWidth;
	height = sprite._height;

	for (++it; it != overlayList.end(); ++it) {
		if (it->type != 5) {
			continue;
		}

		idx = ABS(objectTable[it->objIdx].frame);
		mx = objectTable[it->objIdx].x;
		my = objectTable[it->objIdx].y;
		mw = animDataTable[idx]._realWidth;
		mh = animDataTable[idx]._height;

		gfxUpdateSpriteMask(mask, x, y, width, height, animDataTable[idx].data(), mx, my, mw, mh);
	}
}

/*! \brief Draw background to backbuffer
 */
void FWRenderer::drawBackground() {
	assert(_background);
	memcpy(_backBuffer, _background, _screenSize);
}

/*! \brief Draw one overlay
 * \param it Overlay info
 */
void FWRenderer::renderOverlay(const Common::List<overlay>::iterator &it) {
	int idx, len, width;
	objectStruct *obj;
	AnimData *sprite;
	byte *mask;

	switch (it->type) {
	// color sprite
	case 0:
		sprite = animDataTable + objectTable[it->objIdx].frame;
		len = sprite->_realWidth * sprite->_height;
		mask = new byte[len];
		memcpy(mask, sprite->mask(), len);
		remaskSprite(mask, it);
		drawMaskedSprite(objectTable[it->objIdx], mask);
		delete[] mask;
		break;

	// game message
	case 2:
		if (it->objIdx >= messageTable.size()) {
			return;
		}

		_messageLen += messageTable[it->objIdx].size();
		drawMessage(messageTable[it->objIdx].c_str(), it->x, it->y, it->width, it->color);
		break;

	// action failure message
	case 3:
		idx = it->objIdx * 4 + g_cine->_rnd.getRandomNumber(3);
		len = strlen(failureMessages[idx]);
		_messageLen += len;
		width = 6 * len + 20;
		width = width > 300 ? 300 : width;

		drawMessage(failureMessages[idx], (320 - width) / 2, 80, width, 4);
		break;

	// bitmap
	case 4:
		assert(it->objIdx < NUM_MAX_OBJECT);
		obj = objectTable + it->objIdx;

		if (obj->frame < 0) {
			return;
		}

		if (!animDataTable[obj->frame].data()) {
			return;
		}

		fillSprite(*obj);
		break;
	}
}

/*! \brief Draw overlays
 */
void FWRenderer::drawOverlays() {
	Common::List<overlay>::iterator it;

	for (it = overlayList.begin(); it != overlayList.end(); ++it) {
		renderOverlay(it);
	}
}

/*! \brief Draw another frame
 */
void FWRenderer::drawFrame() {
	drawBackground();
	drawOverlays();

	if (!_cmd.empty()) {
		drawCommand();
	}

	if (_changePal) {
		refreshPalette();
	}

	blit();
}

/*! \brief Update screen
 */
void FWRenderer::blit() {
	g_system->copyRectToScreen(_backBuffer, 320, 0, 0, 320, 200);
}

/*! \brief Set player command string
 * \param cmd New command string
 */
void FWRenderer::setCommand(const char *cmd) {
	_cmd = cmd;
}

/*! \brief Refresh current palette
 */
void FWRenderer::refreshPalette() {
	int i;
	byte pal[16*4];

	assert(_activeLowPal);

	for (i = 0; i < 16; i++) {
		// This seems to match the output from DOSbox.
		pal[i * 4 + 2] = ((_activeLowPal[i] & 0x00f) >> 0) * 32;
		pal[i * 4 + 1] = ((_activeLowPal[i] & 0x0f0) >> 4) * 32;
		pal[i * 4 + 0] = ((_activeLowPal[i] & 0xf00) >> 8) * 32;
		pal[i * 4 + 3] = 0;
	}

	g_system->setPalette(pal, 0, 16);
	_changePal = 0;
}

/*! \brief Load palette of current background
 */
void FWRenderer::reloadPalette() {
	assert(_palette);

	if (!_activeLowPal) {
		_activeLowPal = new uint16[_lowPalSize];
	}

	assert(_activeLowPal);

	memcpy(_activeLowPal, _palette, _lowPalSize * sizeof (uint16));
	_changePal = 1;
}

/*! \brief Load background into renderer
 * \param bg Raw background data
 */
void FWRenderer::loadBg16(const byte *bg, const char *name) {
	int i;

	if (!_background) {
		_background = new byte[_screenSize];
	}

	if (!_palette) {
		_palette = new uint16[_lowPalSize];
	}

	assert(_background && _palette);

	strcpy(_bgName, name);

	for (i = 0; i < _lowPalSize; i++, bg += 2) {
		_palette[i] = READ_BE_UINT16(bg);
	}

	gfxConvertSpriteToRaw(_background, bg, 160, 200);
}

/*! \brief Placeholder for Operation Stealth implementation
 */
void FWRenderer::loadBg16(const byte *bg, const char *name, unsigned int idx) {
	error("Future Wars renderer doesn't support multiple backgrounds");
}

/*! \brief Placeholder for Operation Stealth implementation
 */
void FWRenderer::loadCt16(const byte *ct, const char *name) {
	error("Future Wars renderer doesn't support multiple backgrounds");
}

/*! \brief Placeholder for Operation Stealth implementation
 */
void FWRenderer::loadBg256(const byte *bg, const char *name) {
	error("Future Wars renderer doesn't support 256 color mode");
}

/*! \brief Placeholder for Operation Stealth implementation
 */
void FWRenderer::loadBg256(const byte *bg, const char *name, unsigned int idx) {
	error("Future Wars renderer doesn't support multiple backgrounds");
}

/*! \brief Placeholder for Operation Stealth implementation
 */
void FWRenderer::loadCt256(const byte *ct, const char *name) {
	error("Future Wars renderer doesn't support multiple backgrounds");
}

/*! \brief Placeholder for Operation Stealth implementation
 */
void FWRenderer::selectBg(unsigned int idx) {
	error("Future Wars renderer doesn't support multiple backgrounds");
}

/*! \brief Placeholder for Operation Stealth implementation
 */
void FWRenderer::selectScrollBg(unsigned int idx) {
	error("Future Wars renderer doesn't support multiple backgrounds");
}

/*! \brief Placeholder for Operation Stealth implementation
 */
void FWRenderer::setScroll(unsigned int shift) {
	error("Future Wars renderer doesn't support multiple backgrounds");
}

/*! \brief Placeholder for Operation Stealth implementation
 */
void FWRenderer::removeBg(unsigned int idx) {
	error("Future Wars renderer doesn't support multiple backgrounds");
}

void FWRenderer::saveBg(Common::OutSaveFile &fHandle) {
	fHandle.write(_bgName, 13);
}

/*! \brief Restore active and backup palette from save
 * \param fHandle Savefile open for reading
 */
void FWRenderer::restorePalette(Common::InSaveFile &fHandle) {
	int i;

	if (!_palette) {
		_palette = new uint16[_lowPalSize];
	}

	if (!_activeLowPal) {
		_activeLowPal = new uint16[_lowPalSize];
	}

	assert(_palette && _activeLowPal);

	for (i = 0; i < _lowPalSize; i++) {
		_activeLowPal[i] = fHandle.readUint16BE();
	}

	for (i = 0; i < _lowPalSize; i++) {
		_palette[i] = fHandle.readUint16BE();
	}

	_changePal = 1;
}

/*! \brief Write active and backup palette to save
 * \param fHandle Savefile open for writing
 */
void FWRenderer::savePalette(Common::OutSaveFile &fHandle) {
	int i;

	assert(_palette && _activeLowPal);

	for (i = 0; i < _lowPalSize; i++) {
		fHandle.writeUint16BE(_activeLowPal[i]);
	}

	for (i = 0; i < _lowPalSize; i++) {
		fHandle.writeUint16BE(_palette[i]);
	}
}

/*! \brief Rotate active palette
 * \param a First color to rotate
 * \param b Last color to rotate
 * \param c Possibly rotation step, must be equal to 1 at the moment
 */
void FWRenderer::rotatePalette(int a, int b, int c) {
	palRotate(_activeLowPal, a, b, c);
	refreshPalette();
}

/*! \brief Copy part of backup palette to active palette and transform
 * \param first First color to transform
 * \param last Last color to transform
 * \param r Red channel transformation
 * \param g Green channel transformation
 * \param b Blue channel transformation
 */
void FWRenderer::transformPalette(int first, int last, int r, int g, int b) {
	if (!_activeLowPal) {
		_activeLowPal = new uint16[_lowPalSize];
		memset(_activeLowPal, 0, _lowPalSize * sizeof (uint16));
	}

	transformPaletteRange(_activeLowPal, _palette, first, last, r, g, b);
	refreshPalette();
}

/*! \brief Draw menu box, one item per line with possible highlight
 * \param items Menu items
 * \param height Item count
 * \param x Top left menu corner coordinate
 * \param y Top left menu corner coordinate
 * \param width Menu box width
 * \param selected Index of highlighted item (no highlight if less than 0)
 */
void FWRenderer::drawMenu(const CommandeType *items, unsigned int height, int x, int y, int width, int selected) {
	int tx, ty, th = height * 9 + 10;
	unsigned int i, j;

	if (x + width > 319) {
		x = 319 - width;
	}

	if (y + th > 199) {
		y = 199 - th;
	}

	drawPlainBox(x, y, width, 4, _messageBg);

	ty = y + 4;

	for (i = 0; i < height; i++, ty += 9) {
		drawPlainBox(x, ty, width, 9, (int)i == selected ? 0 : _messageBg);
		tx = x + 4;

		for (j = 0; items[i][j]; j++) {
			tx = drawChar(items[i][j], tx, ty);
		}
	}

	drawPlainBox(x, ty, width, 4, _messageBg);
	drawDoubleBorder(x, y, width, ty - y + 4, 2);
}

/*! \brief Draw text input box
 * \param info Input box message
 * \param input Text entered in the input area
 * \param cursor Cursor position in the input area
 * \param x Top left input box corner coordinate
 * \param y Top left input box corner coordinate
 * \param width Input box width
 */
void FWRenderer::drawInputBox(const char *info, const char *input, int cursor, int x, int y, int width) {
	int i, tx, ty, tw;
	int line = 0, words = 0, cw = 0;
	int space = 0, extraSpace = 0;

	drawPlainBox(x, y, width, 4, _messageBg);
	tx = x + 4;
	ty = info[0] ? y - 5 : y + 4;
	tw = width - 8;

	// input box info message
	for (i = 0; info[i]; i++, line--) {
		// fit line of text
		if (!line) {
			line = fitLine(info + i, tw, words, cw);

			if ( info[i + line] != '\0' && words) {
				space = (tw - cw) / words;
				extraSpace = (tw - cw) % words;
			} else {
				space = 5;
				extraSpace = 0;
			}

			ty += 9;
			drawPlainBox(x, ty, width, 9, _messageBg);
			tx = x + 4;
		}

		// draw characters
		if (info[i] == ' ') {
			tx += space + extraSpace;

			if (extraSpace) {
				extraSpace = 0;
			}
		} else {
			tx = drawChar(info[i], tx, ty);
		}
	}

	// input area background
	ty += 9;
	drawPlainBox(x, ty, width, 9, _messageBg);
	drawPlainBox(x + 16, ty - 1, width - 32, 9, 0);
	tx = x + 20;

	// text in input area
	for (i = 0; input[i]; i++) {
		tx = drawChar(input[i], tx, ty);

		if (cursor == i + 2) {
			drawLine(tx, ty - 1, 1, 9, 2);
		}
	}

	if (!input[0] || cursor == 1) {
		drawLine(x + 20, ty - 1, 1, 9, 2);
	}

	ty += 9;
	drawPlainBox(x, ty, width, 4, _messageBg);
	drawDoubleBorder(x, y, width, ty - y + 4, 2);
}

/*! \brief Fade to black
 */
void FWRenderer::fadeToBlack() {
	// FIXME: _activeLowPal is invalid when starting Operation Stealth
	// Adding this sanity check fixes a crash when the game
	// starts, but I'm not sure if this is the best place to check it
	if (!_activeLowPal) {
		warning("_activeLowPal is invalid");
		return;
	}

	assert(_activeLowPal);

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 16; j++) {
			_activeLowPal[j] = transformColor(_activeLowPal[j], -1, -1, -1);
		}

		refreshPalette();
		g_system->updateScreen();
		g_system->delayMillis(50);
	}
}

/*! \brief Initialize Operation Stealth renderer
 */
OSRenderer::OSRenderer() : _activeHiPal(NULL), _currentBg(0), _scrollBg(0),
	_bgShift(0) {

	int i;
	for (i = 0; i < 9; i++) {
		_bgTable[i].bg = NULL;
		_bgTable[i].lowPal = NULL;
		_bgTable[i].hiPal = NULL;
		memset(_bgTable[i].name, 0, sizeof (_bgTable[i].name));
	}
}

/*! \brief Destroy Operation Stealth renderer
 */
OSRenderer::~OSRenderer() {
	delete[] _activeHiPal;

	for (int i = 0; i < 9; i++) {
		delete[] _bgTable[i].bg;
		delete[] _bgTable[i].lowPal;
		delete[] _bgTable[i].hiPal;
	}
}

/*! \brief Reset Operation Stealth renderer state
 */
void OSRenderer::clear() {
	delete[] _activeHiPal;
	_activeHiPal = NULL;

	for (int i = 0; i < 9; i++) {
		delete[] _bgTable[i].bg;
		delete[] _bgTable[i].lowPal;
		delete[] _bgTable[i].hiPal;

		_bgTable[i].bg = NULL;
		_bgTable[i].lowPal = NULL;
		_bgTable[i].hiPal = NULL;
		memset(_bgTable[i].name, 0, sizeof (_bgTable[i].name));
	}

	_currentBg = 0;
	_scrollBg = 0;
	_bgShift = 0;

	FWRenderer::clear();
}

/*! \brief Draw 1bpp sprite using selected color on backgrounds
 * \param obj Object info
 * \param fillColor Sprite color
 */
void OSRenderer::incrustMask(const objectStruct &obj, uint8 color) {
	const byte *data = animDataTable[obj.frame].data();
	int x, y, width, height, i;

	x = obj.x;
	y = obj.y;
	width = animDataTable[obj.frame]._realWidth;
	height = animDataTable[obj.frame]._height;

	for (i = 0; i < 8; i++) {
		if (!_bgTable[i].bg) {
			continue;
		}

		gfxFillSprite(data, width, height, _bgTable[i].bg, x, y, color);
	}
}

/*! \brief Draw color sprite
 * \param obj Object info
 */
void OSRenderer::drawSprite(const objectStruct &obj) {
	const byte *data = animDataTable[obj.frame].data();
	int x, y, width, height, transColor;

	x = obj.x;
	y = obj.y;
	transColor = obj.part;
	width = animDataTable[obj.frame]._realWidth;
	height = animDataTable[obj.frame]._height;

	drawSpriteRaw2(data, transColor, width, height, _backBuffer, x, y);
}

/*! \brief Draw color sprite
 * \param obj Object info
 */
void OSRenderer::incrustSprite(const objectStruct &obj) {
	const byte *data = animDataTable[obj.frame].data();
	int x, y, width, height, transColor, i;

	x = obj.x;
	y = obj.y;
	transColor = obj.part;
	width = animDataTable[obj.frame]._realWidth;
	height = animDataTable[obj.frame]._height;

	for (i = 0; i < 8; i++) {
		if (!_bgTable[i].bg) {
			continue;
		}

		drawSpriteRaw2(data, transColor, width, height, _bgTable[i].bg, x, y);
	}
}

/*! \brief Draw text character on screen
 * \param character Character to draw
 * \param x Character coordinate
 * \param y Character coordinate
 */
int OSRenderer::drawChar(char character, int x, int y) {
	int width, idx;

	if (character == ' ') {
		x += 5;
	} else if ((width = fontParamTable[(unsigned char)character].characterWidth)) {
		idx = fontParamTable[(unsigned char)character].characterIdx;
		drawSpriteRaw2(g_cine->_textHandler.textTable[idx][0], 0, 16, 8, _backBuffer, x, y);
		x += width + 1;
	}

	return x;
}

/*! \brief Draw background to backbuffer
 */
void OSRenderer::drawBackground() {
	byte *main;

	main = _bgTable[_currentBg].bg;
	assert(main);

	if (!_bgShift) {
		memcpy(_backBuffer, main, _screenSize);
	} else {
		byte *scroll = _bgTable[_scrollBg].bg;
		int mainShift = _bgShift * _screenWidth;
		int mainSize = _screenSize - mainShift;

		assert(scroll);

		memcpy(_backBuffer, main + mainShift, mainSize);
		memcpy(_backBuffer + mainSize, scroll, mainShift);
	}
}

/*! \brief Draw one overlay
 * \param it Overlay info
 * \todo Add handling of type 22 overlays
 */
void OSRenderer::renderOverlay(const Common::List<overlay>::iterator &it) {
	int len;
	objectStruct *obj;
	AnimData *sprite;
	byte *mask;

	switch (it->type) {
	// color sprite
	case 0:
		if (objectTable[it->objIdx].frame < 0) {
			break;
		}
		sprite = animDataTable + objectTable[it->objIdx].frame;
		len = sprite->_realWidth * sprite->_height;
		mask = new byte[len];
		generateMask(sprite->data(), mask, len, objectTable[it->objIdx].part);
		remaskSprite(mask, it);
		drawMaskedSprite(objectTable[it->objIdx], mask);
		delete[] mask;
		break;

	// bitmap
	case 4:
		if (objectTable[it->objIdx].frame >= 0) {
			FWRenderer::renderOverlay(it);
		}
		break;

	// masked background
	case 20:
		assert(it->objIdx < NUM_MAX_OBJECT);
		obj = objectTable + it->objIdx;
		sprite = animDataTable + obj->frame;

		if (obj->frame < 0 || it->x > 8 || !_bgTable[it->x].bg || sprite->_bpp != 1) {
			break;
		}

		maskBgOverlay(_bgTable[it->x].bg, sprite->data(), sprite->_realWidth, sprite->_height, _backBuffer, obj->x, obj->y);
		break;

	// something else
	default:
		FWRenderer::renderOverlay(it);
		break;
	}
}

/*! \brief Refresh current palette
 */
void OSRenderer::refreshPalette() {
	if (!_activeHiPal) {
		FWRenderer::refreshPalette();
		return;
	}

	int i;
	byte pal[256*4];

	for (i = 0; i < 256; i++) {
		pal[i * 4 + 0] = _activeHiPal[i * 3 + 0];
		pal[i * 4 + 1] = _activeHiPal[i * 3 + 1];
		pal[i * 4 + 2] = _activeHiPal[i * 3 + 2];
		pal[i * 4 + 3] = 0;
	}

	g_system->setPalette(pal, 0, 256);
	_changePal = 0;
}

/*! \brief Load palette of current background
 */
void OSRenderer::reloadPalette() {
	// selected background in plane takeoff scene has swapped colors 12
	// and 14, shift background has it right
	palBg *bg = _bgShift ? &_bgTable[_scrollBg] : &_bgTable[_currentBg];

	assert(bg->lowPal || bg->hiPal);

	if (bg->lowPal) {
		if (!_activeLowPal) {
			_activeLowPal = new uint16[_lowPalSize];
		}

		assert(_activeLowPal);

		delete[] _activeHiPal;
		_activeHiPal = NULL;

		memcpy(_activeLowPal, bg->lowPal, _lowPalSize * sizeof (uint16));
	} else {
		if (!_activeHiPal) {
			_activeHiPal = new byte[_hiPalSize];
		}

		assert(_activeHiPal);

		delete[] _activeLowPal;
		_activeLowPal = NULL;

		memcpy(_activeHiPal, bg->hiPal, _hiPalSize);
	}
	_changePal = 1;
}

/*! \brief Rotate active palette
 * \param a First color to rotate
 * \param b Last color to rotate
 * \param c Possibly rotation step, must be equal to 1 at the moment
 */
void OSRenderer::rotatePalette(int a, int b, int c) {
	if (_activeLowPal) {
		FWRenderer::rotatePalette(a, b, c);
		return;
	}

	palRotate(_activeHiPal, a, b, c);
	refreshPalette();
}

/*! \brief Copy part of backup palette to active palette and transform
 * \param first First color to transform
 * \param last Last color to transform
 * \param r Red channel transformation
 * \param g Green channel transformation
 * \param b Blue channel transformation
 */
void OSRenderer::transformPalette(int first, int last, int r, int g, int b) {
	palBg *bg = _bgShift ? &_bgTable[_scrollBg] : &_bgTable[_currentBg];

	if (!bg->lowPal) {
		if (!_activeHiPal) {
			_activeHiPal = new byte[_hiPalSize];
			memset(_activeHiPal, 0, _hiPalSize);
		}

		delete[] _activeLowPal;
		_activeLowPal = NULL;

		transformPaletteRange(_activeHiPal, bg->hiPal, first, last, r, g, b);
	} else {
		if (!_activeLowPal) {
			_activeLowPal = new uint16[_lowPalSize];
			memset(_activeLowPal, 0, _lowPalSize * sizeof (uint16));
		}

		delete[] _activeHiPal;
		_activeHiPal = NULL;

		transformPaletteRange(_activeLowPal, bg->lowPal, first, last, r, g, b);
	}

	refreshPalette();
}

/*! \brief Load 16 color background into renderer
 * \param bg Raw background data
 * \param name Background filename
 */
void OSRenderer::loadBg16(const byte *bg, const char *name) {
	loadBg16(bg, name, 0);
}

/*! \brief Load 16 color background into renderer
 * \param bg Raw background data
 * \param name Background filename
 * \param pos Background index
 */
void OSRenderer::loadBg16(const byte *bg, const char *name, unsigned int idx) {
	int i;
	assert(idx < 9);

	if (!_bgTable[idx].bg) {
		_bgTable[idx].bg = new byte[_screenSize];
	}

	if (!_bgTable[idx].lowPal) {
		_bgTable[idx].lowPal = new uint16[_lowPalSize];
	}

	assert(_bgTable[idx].bg && _bgTable[idx].lowPal);

	delete[] _bgTable[idx].hiPal;
	_bgTable[idx].hiPal = NULL;

	strcpy(_bgTable[idx].name, name);

	for (i = 0; i < _lowPalSize; i++, bg += 2) {
		_bgTable[idx].lowPal[i] = READ_BE_UINT16(bg);
	}

	gfxConvertSpriteToRaw(_bgTable[idx].bg, bg, 160, 200);
}

/*! \brief Load 16 color CT data as background into renderer
 * \param ct Raw CT data
 * \param name Background filename
 */
void OSRenderer::loadCt16(const byte *ct, const char *name) {
	loadBg16(ct, name, 8);
}

/*! \brief Load 256 color background into renderer
 * \param bg Raw background data
 * \param name Background filename
 */
void OSRenderer::loadBg256(const byte *bg, const char *name) {
	loadBg256(bg, name, 0);
}

/*! \brief Load 256 color background into renderer
 * \param bg Raw background data
 * \param name Background filename
 * \param pos Background index
 */
void OSRenderer::loadBg256(const byte *bg, const char *name, unsigned int idx) {
	assert(idx < 9);

	if (!_bgTable[idx].bg) {
		_bgTable[idx].bg = new byte[_screenSize];
	}

	if (!_bgTable[idx].hiPal) {
		_bgTable[idx].hiPal = new byte[_hiPalSize];
	}

	assert(_bgTable[idx].bg && _bgTable[idx].hiPal);

	delete[] _bgTable[idx].lowPal;
	_bgTable[idx].lowPal = NULL;

	strcpy(_bgTable[idx].name, name);
	memcpy(_bgTable[idx].hiPal, bg, _hiPalSize);
	memcpy(_bgTable[idx].bg, bg + _hiPalSize, _screenSize);
}

/*! \brief Load 256 color CT data as background into renderer
 * \param ct Raw CT data
 * \param name Background filename
 */
void OSRenderer::loadCt256(const byte *ct, const char *name) {
	loadBg256(ct, name, 8);
}

/*! \brief Select active background and load its palette
 * \param idx Background index
 */
void OSRenderer::selectBg(unsigned int idx) {
	assert(idx < 9 && _bgTable[idx].bg);
	assert(_bgTable[idx].lowPal || _bgTable[idx].hiPal);

	_currentBg = idx;
	reloadPalette();
}

/*! \brief Select scroll background
 * \param idx Scroll background index
 */
void OSRenderer::selectScrollBg(unsigned int idx) {
	assert(idx < 9);

	if (_bgTable[idx].bg) {
		_scrollBg = idx;
	}
	reloadPalette();
}

/*! \brief Set background scroll
 * \param shift Background scroll in pixels
 */
void OSRenderer::setScroll(unsigned int shift) {
	assert(shift <= 200);

	_bgShift = shift;
}

/*! \brief Unload background from renderer
 * \param idx Background to unload
 */
void OSRenderer::removeBg(unsigned int idx) {
	assert(idx > 0 && idx < 9);

	if (_currentBg == idx) {
		_currentBg = 0;
	}

	if (_scrollBg == idx) {
		_scrollBg = 0;
	}

	delete[] _bgTable[idx].bg;
	delete[] _bgTable[idx].lowPal;
	delete[] _bgTable[idx].hiPal;
	_bgTable[idx].bg = NULL;
	_bgTable[idx].lowPal = NULL;
	_bgTable[idx].hiPal = NULL;
	memset(_bgTable[idx].name, 0, sizeof (_bgTable[idx].name));
}

/*! \brief Fade to black
 * \bug Operation Stealth sometimes seems to fade to black using
 * transformPalette resulting in double fadeout
 */
void OSRenderer::fadeToBlack() {
	if (!_activeHiPal) {
		FWRenderer::fadeToBlack();
		return;
	}

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < _hiPalSize; j++) {
			_activeHiPal[j] = CLIP(_activeHiPal[j] - 32, 0, 255);
		}

		refreshPalette();
		g_system->updateScreen();
		g_system->delayMillis(50);
	}
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
		CursorMan.replaceCursor(mouseCursor, 16, 16, mc->hotspotX, mc->hotspotY);
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

void maskBgOverlay(const byte *bgPtr, const byte *maskPtr, int16 width, int16 height,
				   byte *page, int16 x, int16 y) {
	int16 i, j, tmpWidth, tmpHeight;
	Common::List<BGIncrust>::iterator it;
	byte *mask;
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
			maskPtr++;
		}
	}

	maskPtr = backup;

	// incrust pass
	for (it = bgIncrustList.begin(); it != bgIncrustList.end(); ++it) {
		tmpWidth = animDataTable[it->frame]._realWidth;
		tmpHeight = animDataTable[it->frame]._height;
		mask = (byte*)malloc(tmpWidth * tmpHeight);

		if (it->param == 0) {
			generateMask(animDataTable[it->frame].data(), mask, tmpWidth * tmpHeight, it->part);
			gfxUpdateIncrustMask(mask, it->x, it->y, tmpWidth, tmpHeight, maskPtr, x, y, width, height);
			gfxDrawMaskedSprite(animDataTable[it->frame].data(), mask, tmpWidth, tmpHeight, page, it->x, it->y);
		} else {
			memcpy(mask, animDataTable[it->frame].data(), tmpWidth * tmpHeight);
			gfxUpdateIncrustMask(mask, it->x, it->y, tmpWidth, tmpHeight, maskPtr, x, y, width, height);
			gfxFillSprite(mask, tmpWidth, tmpHeight, page, it->x, it->y);
		}

		free(mask);
	}
}

} // End of namespace Cine
