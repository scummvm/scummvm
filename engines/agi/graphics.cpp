/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2003 Sarien Team
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

#include "common/stdafx.h"

#include "graphics/cursorman.h"

#include "agi/agi.h"
#include "agi/graphics.h"

namespace Agi {

#define DEV_X0(x) ((x) << 1)
#define DEV_X1(x) (((x) << 1) + 1)
#define DEV_Y(x) (x)

#ifndef MAX_INT
#  define MAX_INT (int)((unsigned)~0 >> 1)
#endif

#include "agi/font.h"

/**
 * 16 color RGB palette (plus 16 transparent colors).
 * This array contains the 6-bit RGB values of the EGA palette exported
 * to the console drivers.
 */
uint8 egaPalette[16 * 3] = {
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x2a,
	0x00, 0x2a, 0x00,
	0x00, 0x2a, 0x2a,
	0x2a, 0x00, 0x00,
	0x2a, 0x00, 0x2a,
	0x2a, 0x15, 0x00,
	0x2a, 0x2a, 0x2a,
	0x15, 0x15, 0x15,
	0x15, 0x15, 0x3f,
	0x15, 0x3f, 0x15,
	0x15, 0x3f, 0x3f,
	0x3f, 0x15, 0x15,
	0x3f, 0x15, 0x3f,
	0x3f, 0x3f, 0x15,
	0x3f, 0x3f, 0x3f
};

/**
 * 16 color amiga-ish palette.
 */
uint8 newPalette[16 * 3] = {
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x3f,
	0x00, 0x2A, 0x00,
	0x00, 0x2A, 0x2A,
	0x33, 0x00, 0x00,
	0x2f, 0x1c, 0x37,
	0x23, 0x14, 0x00,
	0x2f, 0x2f, 0x2f,
	0x15, 0x15, 0x15,
	0x00, 0x2f, 0x3f,
	0x00, 0x33, 0x15,
	0x15, 0x3F, 0x3F,
	0x3f, 0x27, 0x23,
	0x3f, 0x15, 0x3f,
	0x3b, 0x3b, 0x00,
	0x3F, 0x3F, 0x3F
};

static uint16 cgaMap[16] = {
	0x0000,			/*  0 - black */
	0x0d00,			/*  1 - blue */
	0x0b00,			/*  2 - green */
	0x0f00,			/*  3 - cyan */
	0x000b,			/*  4 - red */
	0x0b0d,			/*  5 - magenta */
	0x000d,			/*  6 - brown */
	0x0b0b,			/*  7 - gray */
	0x0d0d,			/*  8 - dark gray */
	0x0b0f,			/*  9 - light blue */
	0x0b0d,			/* 10 - light green */
	0x0f0d,			/* 11 - light cyan */
	0x0f0d,			/* 12 - light red */
	0x0f00,			/* 13 - light magenta */
	0x0f0b,			/* 14 - yellow */
	0x0f0f			/* 15 - white */
};

struct UpdateBlock {
	int x1, y1;
	int x2, y2;
};

static struct UpdateBlock update = {
	MAX_INT, MAX_INT, 0, 0
};

/*
 *  Layer 4:  640x480?  ==================  User display
 *                              ^
 *                              |  do_update(), put_block()
 *                              |
 *  Layer 3:  640x480?  ==================  Framebuffer
 *                              ^
 *                              |  flush_block(), put_pixels()
 *                              |
 *  Layer 2:  320x200   ==================  AGI engine screen (console), put_pixel()
 *                              |
 *  Layer 1:  160x168   ==================  AGI screen
 */

#define SHAKE_MAG 3

void GfxMgr::shakeStart() {
	int i;

	if ((_shakeH = (uint8 *)malloc(GFX_WIDTH * SHAKE_MAG)) == NULL)
		return;

	if ((_shakeV = (uint8 *)malloc(SHAKE_MAG * (GFX_HEIGHT - SHAKE_MAG))) == NULL) {
		free(_shakeH);
		return;
	}

	for (i = 0; i < GFX_HEIGHT - SHAKE_MAG; i++) {
		memcpy(_shakeV + i * SHAKE_MAG, _agiScreen + i * GFX_WIDTH, SHAKE_MAG);
	}

	for (i = 0; i < SHAKE_MAG; i++) {
		memcpy(_shakeH + i * GFX_WIDTH, _agiScreen + i * GFX_WIDTH, GFX_WIDTH);
	}
}

void GfxMgr::shakeScreen(int n) {
	int i;

	if (n == 0) {
		for (i = 0; i < (GFX_HEIGHT - SHAKE_MAG); i++) {
			memmove(&_agiScreen[GFX_WIDTH * i],
					&_agiScreen[GFX_WIDTH * (i + SHAKE_MAG) + SHAKE_MAG],
					GFX_WIDTH - SHAKE_MAG);
		}
	} else {
		for (i = GFX_HEIGHT - SHAKE_MAG - 1; i >= 0; i--) {
			memmove(&_agiScreen[GFX_WIDTH * (i + SHAKE_MAG) + SHAKE_MAG],
					&_agiScreen[GFX_WIDTH * i], GFX_WIDTH - SHAKE_MAG);
		}
	}
}

void GfxMgr::shakeEnd() {
	int i;

	for (i = 0; i < GFX_HEIGHT - SHAKE_MAG; i++) {
		memcpy(_agiScreen + i * GFX_WIDTH, _shakeV + i * SHAKE_MAG, SHAKE_MAG);
	}

	for (i = 0; i < SHAKE_MAG; i++) {
		memcpy(_agiScreen + i * GFX_WIDTH, _shakeH + i * GFX_WIDTH, GFX_WIDTH);
	}

	flushBlock(0, 0, GFX_WIDTH - 1, GFX_HEIGHT - 1);

	free(_shakeV);
	free(_shakeH);
}

void GfxMgr::putTextCharacter(int l, int x, int y, unsigned int c, int fg, int bg, bool checkerboard) {
	int x1, y1, xx, yy, cc;
	const uint8 *p;

	p = Agi::curFont + ((unsigned int)c * CHAR_LINES);
	for (y1 = 0; y1 < CHAR_LINES; y1++) {
		for (x1 = 0; x1 < CHAR_COLS; x1++) {
			xx = x + x1;
			yy = y + y1;
			cc = (*p & (1 << (7 - x1))) ? fg : bg;
			_agiScreen[xx + yy * GFX_WIDTH] = cc;
		}

		p++;
	}

	// Simple checkerboard effect to simulate "greyed out" text.
	// This is what Sierra's interpreter does for things like menu items
	// that aren't selectable (such as separators). -- dsymonds
	if (checkerboard) {
		for (yy = y; yy < y + CHAR_LINES; yy++)
			for (xx = x + (~yy & 1); xx < x + CHAR_COLS; xx += 2)
				_agiScreen[xx + yy * GFX_WIDTH] = 15;
	}

	/* FIXME: we don't want this when we're writing on the
	 *        console!
	 */
	flushBlock(x, y, x + CHAR_COLS - 1, y + CHAR_LINES - 1);
}

void GfxMgr::drawRectangle(int x1, int y1, int x2, int y2, int c) {
	int y, w, h;
	uint8 *p0;

	if (x1 >= GFX_WIDTH)
		x1 = GFX_WIDTH - 1;
	if (y1 >= GFX_HEIGHT)
		y1 = GFX_HEIGHT - 1;
	if (x2 >= GFX_WIDTH)
		x2 = GFX_WIDTH - 1;
	if (y2 >= GFX_HEIGHT)
		y2 = GFX_HEIGHT - 1;

	w = x2 - x1 + 1;
	h = y2 - y1 + 1;
	p0 = &_agiScreen[x1 + y1 * GFX_WIDTH];
	for (y = 0; y < h; y++) {
		memset(p0, c, w);
		p0 += GFX_WIDTH;
	}
}

void GfxMgr::drawFrame(int x1, int y1, int x2, int y2, int c1, int c2) {
	int y, w;
	uint8 *p0;

	/* top line */
	w = x2 - x1 + 1;
	p0 = &_agiScreen[x1 + y1 * GFX_WIDTH];
	memset(p0, c1, w);

	/* bottom line */
	p0 = &_agiScreen[x1 + y2 * GFX_WIDTH];
	memset(p0, c2, w);

	/* side lines */
	for (y = y1; y <= y2; y++) {
		_agiScreen[x1 + y * GFX_WIDTH] = c1;
		_agiScreen[x2 + y * GFX_WIDTH] = c2;
	}
}

void GfxMgr::drawBox(int x1, int y1, int x2, int y2, int colour1, int colour2, int m) {
	x1 += m;
	y1 += m;
	x2 -= m;
	y2 -= m;

	drawRectangle(x1, y1, x2, y2, colour1);
	drawFrame(x1 + 2, y1 + 2, x2 - 2, y2 - 2, colour2, colour2);
	flushBlock(x1, y1, x2, y2);
}

void GfxMgr::printCharacter(int x, int y, char c, int fg, int bg) {
	x *= CHAR_COLS;
	y *= CHAR_LINES;

	putTextCharacter(0, x, y, c, fg, bg);
	// redundant! already inside put_text_character!
	// flush_block (x, y, x + CHAR_COLS - 1, y + CHAR_LINES - 1);
}

/**
 * Draw button
 * @param x  x coordinate of the button
 * @param y  y coordinate of the button
 * @param a  set if the button has focus
 * @param p  set if the button is pressed
 */
void GfxMgr::drawButton(int x, int y, const char *s, int a, int p, int fgcolor, int bgcolor) {
	int len = strlen(s);
	int x1, y1, x2, y2;

	x1 = x - 3;
	y1 = y - 3;
	x2 = x + CHAR_COLS * len + 2;
	y2 = y + CHAR_LINES + 2;

	while (*s) {
		putTextCharacter(0, x + (!!p), y + (!!p), *s++, a ? bgcolor : fgcolor, a ? fgcolor : bgcolor);
		x += CHAR_COLS;
	}

	x1 -= 2;
	y1 -= 2;
	x2 += 2;
	y2 += 2;

	flushBlock(x1, y1, x2, y2);
}

int GfxMgr::testButton(int x, int y, const char *s) {
	int len = strlen(s);
	int x1, y1, x2, y2;

	x1 = x - 3;
	y1 = y - 3;
	x2 = x + CHAR_COLS * len + 2;
	y2 = y + CHAR_LINES + 2;

	if ((int)g_mouse.x >= x1 && (int)g_mouse.y >= y1 && (int)g_mouse.x <= x2 && (int)g_mouse.y <= y2)
		return true;

	return false;
}

void GfxMgr::putBlock(int x1, int y1, int x2, int y2) {
	gfxPutBlock(x1, y1, x2, y2);
}

void GfxMgr::putScreen() {
	putBlock(0, 0, GFX_WIDTH - 1, GFX_HEIGHT - 1);
}

void GfxMgr::pollTimer() {
	_vm->agiTimerLow();
}

int GfxMgr::getKey() {
	return _vm->agiGetKeypressLow();
}

int GfxMgr::keypress() {
	return _vm->agiIsKeypressLow();
}

/*
 * Public functions
 */

/**
 * Initialize the color palette
 * This function initializes the color palette using the specified 16-color
 * RGB palette and creates 16 extra palette entries with translucent colors
 * for the interpreter console.
 * @param p  A pointer to the 16-color RGB palette.
 */
void GfxMgr::initPalette(uint8 *p) {
	int i;

	for (i = 0; i < 48; i++) {
		_palette[i] = p[i];
		_palette[i + 48] = (p[i] + 0x30) >> 2;
	}
}

void GfxMgr::gfxSetPalette() {
	int i;
	byte pal[32 * 4];

	for (i = 0; i < 32; i++) {
		pal[i * 4 + 0] = _palette[i * 3 + 0] << 2;
		pal[i * 4 + 1] = _palette[i * 3 + 1] << 2;
		pal[i * 4 + 2] = _palette[i * 3 + 2] << 2;
		pal[i * 4 + 3] = 0;
	}
	g_system->setPalette(pal, 0, 32);
}

/* put a block onto the screen */
void GfxMgr::gfxPutBlock(int x1, int y1, int x2, int y2) {
	if (x1 >= GFX_WIDTH)
		x1 = GFX_WIDTH - 1;
	if (y1 >= GFX_HEIGHT)
		y1 = GFX_HEIGHT - 1;
	if (x2 >= GFX_WIDTH)
		x2 = GFX_WIDTH - 1;
	if (y2 >= GFX_HEIGHT)
		y2 = GFX_HEIGHT - 1;

	g_system->copyRectToScreen(_screen + y1 * 320 + x1, 320, x1, y1, x2 - x1 + 1, y2 - y1 + 1);
}

static const byte mouseCursorArrow[] = {
	// This is the same arrow cursor that was later used in early SCI games
	0x00, 0x00, 0x40, 0x00, 0x60, 0x00, 0x70, 0x00,
	0x78, 0x00, 0x7C, 0x00, 0x7E, 0x00, 0x7F, 0x00,
	0x7F, 0x80, 0x7F, 0xC0, 0x7C, 0x00, 0x46, 0x00,
	0x06, 0x00, 0x03, 0x00, 0x03, 0x00, 0x01, 0x80,
	0xC0, 0x00, 0xA0, 0x00, 0x90, 0x00, 0x88, 0x00,
	0x84, 0x00, 0x82, 0x00, 0x81, 0x00, 0x80, 0x80,
	0x80, 0x40, 0x80, 0x20, 0x82, 0x00, 0xA9, 0x00,
	0xC9, 0x00, 0x04, 0x80, 0x04, 0x80, 0x02, 0x40
};

/**
 * Initialize graphics device.
 *
 * @see deinit_video()
 */
int GfxMgr::initVideo() {
	if (_vm->_renderMode == Common::kRenderEGA)
		initPalette(egaPalette);
	else
		initPalette(newPalette);

	if ((_agiScreen = (uint8 *)calloc(GFX_WIDTH, GFX_HEIGHT)) == NULL)
		return errNotEnoughMemory;

	gfxSetPalette();

	byte mouseCursor[16 * 16];
	const byte *src = mouseCursorArrow;
	for (int i = 0; i < 32; ++i) {
		int offs = i * 8;
		for (byte mask = 0x80; mask != 0; mask >>= 1) {
			if (src[0] & mask) {
				mouseCursor[offs] = 2;
			} else if (src[32] & mask) {
				mouseCursor[offs] = 0;
			} else {
				mouseCursor[offs] = 0xFF;
			}
			++offs;
		}
		++src;
	}

	const byte cursorPalette[] = {
		0, 0, 0, 0,
		0, 0, 0, 0,
		255, 255, 255, 0
	};

	CursorMan.replaceCursorPalette(cursorPalette, 0, 3);
	CursorMan.replaceCursor(mouseCursor, 16, 16, 1, 1);

	return errOK;
}

/**
 * Deinitialize graphics device.
 *
 * @see init_video()
 */
int GfxMgr::deinitVideo() {
	free(_agiScreen);

	return errOK;
}

int GfxMgr::initMachine() {
	_screen = (unsigned char *)malloc(320 * 200);
	_vm->_clockCount = 0;

	return errOK;
}

int GfxMgr::deinitMachine() {
	free(_screen);

	return errOK;
}

/**
 * Write pixels on the output device.
 * This function writes a row of pixels on the output device. Only the
 * lower 4 bits of each pixel in the row will be used, making this
 * function suitable for use with rows from the AGI screen.
 * @param x x coordinate of the row start (AGI coord.)
 * @param y y coordinate of the row start (AGI coord.)
 * @param n number of pixels in the row
 * @param p pointer to the row start in the AGI screen
 */
void GfxMgr::putPixelsA(int x, int y, int n, uint8 *p) {
	if (_vm->_renderMode == Common::kRenderCGA) {
		for (x *= 2; n--; p++, x += 2) {
			register uint16 q = (cgaMap[(*p & 0xf0) >> 4] << 4) | cgaMap[*p & 0x0f];
			if (_vm->_debug.priority)
				q >>= 4;
			*(uint16 *)&_agiScreen[x + y * GFX_WIDTH] = q & 0x0f0f;
		}
	} else {
		for (x *= 2; n--; p++, x += 2) {
			register uint16 q = ((uint16) * p << 8) | *p;
			if (_vm->_debug.priority)
				q >>= 4;
			*(uint16 *)&_agiScreen[x + y * GFX_WIDTH] = q & 0x0f0f;
		}
	}
}

/**
 * Schedule blocks for blitting on the output device.
 * This function gets the coordinates of a block in the AGI screen and
 * schedule it to be updated in the output device.
 * @param x1 x coordinate of the upper left corner of the block (AGI coord.)
 * @param y1 y coordinate of the upper left corner of the block (AGI coord.)
 * @param x2 x coordinate of the lower right corner of the block (AGI coord.)
 * @param y2 y coordinate of the lower right corner of the block (AGI coord.)
 *
 * @see do_update()
 */
void GfxMgr::scheduleUpdate(int x1, int y1, int x2, int y2) {
	if (x1 < update.x1)
		update.x1 = x1;
	if (y1 < update.y1)
		update.y1 = y1;
	if (x2 > update.x2)
		update.x2 = x2;
	if (y2 > update.y2)
		update.y2 = y2;
}

/**
 * Update scheduled blocks on the output device.
 * This function exposes the blocks scheduled for updating to the output
 * device. Blocks can be scheduled at any point of the AGI cycle.
 *
 * @see schedule_update()
 */
void GfxMgr::doUpdate() {
	if (update.x1 <= update.x2 && update.y1 <= update.y2) {
		gfxPutBlock(update.x1, update.y1, update.x2, update.y2);
	}

	/* reset update block variables */
	update.x1 = MAX_INT;
	update.y1 = MAX_INT;
	update.x2 = 0;
	update.y2 = 0;
}

/**
 * Updates a block of the framebuffer with contents of the AGI engine screen.
 * This function updates a block in the output device with the contents of
 * the AGI engine screen, handling console transparency.
 * @param x1 x coordinate of the upper left corner of the block
 * @param y1 y coordinate of the upper left corner of the block
 * @param x2 x coordinate of the lower right corner of the block
 * @param y2 y coordinate of the lower right corner of the block
 *
 * @see flush_block_a()
 */
void GfxMgr::flushBlock(int x1, int y1, int x2, int y2) {
	int y, w;
	uint8 *p0;

	scheduleUpdate(x1, y1, x2, y2);

	p0 = &_agiScreen[x1 + y1 * GFX_WIDTH];
	w = x2 - x1 + 1;

	for (y = y1; y <= y2; y++) {
		memcpy(_screen + 320 * y + x1, p0, w);
		p0 += GFX_WIDTH;
	}
}

/**
 * Updates a block of the framebuffer receiving AGI picture coordinates.
 * @param x1 x AGI picture coordinate of the upper left corner of the block
 * @param y1 y AGI picture coordinate of the upper left corner of the block
 * @param x2 x AGI picture coordinate of the lower right corner of the block
 * @param y2 y AGI picture coordinate of the lower right corner of the block
 *
 * @see flush_block()
 */
void GfxMgr::flushBlockA(int x1, int y1, int x2, int y2) {
	//y1 += 8;
	//y2 += 8;
	flushBlock(DEV_X0(x1), DEV_Y(y1), DEV_X1(x2), DEV_Y(y2));
}

/**
 * Updates the framebuffer with contents of the AGI engine screen (console-aware).
 * This function updates the output device with the contents of the AGI
 * screen, handling console transparency.
 */
void GfxMgr::flushScreen() {
	flushBlock(0, 0, GFX_WIDTH - 1, GFX_HEIGHT - 1);
}

/**
 * Clear the output device screen (console-aware).
 * This function clears the output device screen and updates the
 * output device. Contents of the AGI screen are left untouched. This
 * function can be used to simulate a switch to a text mode screen in
 * a graphic-only device.
 * @param c  color to clear the screen
 */
void GfxMgr::clearScreen(int c) {
	memset(_agiScreen, c, GFX_WIDTH * GFX_HEIGHT);
	flushScreen();
}

/**
 * Save a block of the AGI engine screen
 */
void GfxMgr::saveBlock(int x1, int y1, int x2, int y2, uint8 *b) {
	uint8 *p0;
	int w, h;

	p0 = &_agiScreen[x1 + GFX_WIDTH * y1];
	w = x2 - x1 + 1;
	h = y2 - y1 + 1;
	while (h--) {
		memcpy(b, p0, w);
		b += w;
		p0 += GFX_WIDTH;
	}
}

/**
 * Restore a block of the AGI engine screen
 */
void GfxMgr::restoreBlock(int x1, int y1, int x2, int y2, uint8 *b) {
	uint8 *p0;
	int w, h;

	p0 = &_agiScreen[x1 + GFX_WIDTH * y1];
	w = x2 - x1 + 1;
	h = y2 - y1 + 1;
	while (h--) {
		memcpy(p0, b, w);
		b += w;
		p0 += GFX_WIDTH;
	}
	flushBlock(x1, y1, x2, y2);
}

} // End of namespace Agi
