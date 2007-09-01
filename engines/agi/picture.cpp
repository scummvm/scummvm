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

#include "common/stdafx.h"

#include "agi/agi.h"
#include "agi/graphics.h"


namespace Agi {

#define nextByte data[foffs++]

static uint8 *data;
static uint32 flen;
static uint32 foffs;

static uint8 patCode;
static uint8 patNum;
static uint8 priOn;
static uint8 scrOn;
static uint8 scrColour;
static uint8 priColour;

static uint8 circles[][15] = {	/* agi circle bitmaps */
	{0x80},
	/* {0xfc}, */
	{ 3 << 4 },	/* pattern data different from specs. fixes gold rush. does not seem to break any other v3 games */
	{0x5f, 0xf4},
	{0x66, 0xff, 0xf6, 0x60},
	{0x23, 0xbf, 0xff, 0xff, 0xee, 0x20},
	{0x31, 0xe7, 0x9e, 0xff, 0xff, 0xde, 0x79, 0xe3, 0x00},
	{0x38, 0xf9, 0xf3, 0xef, 0xff, 0xff, 0xff, 0xfe, 0xf9, 0xf3, 0xe3, 0x80},
	{0x18, 0x3c, 0x7e, 0x7e, 0x7e, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x7e, 0x7e, 0x3c, 0x18}
};

static uint8 splatterMap[32] = {	/* splatter brush bitmaps */
	0x20, 0x94, 0x02, 0x24, 0x90, 0x82, 0xa4, 0xa2,
	0x82, 0x09, 0x0a, 0x22, 0x12, 0x10, 0x42, 0x14,
	0x91, 0x4a, 0x91, 0x11, 0x08, 0x12, 0x25, 0x10,
	0x22, 0xa8, 0x14, 0x24, 0x00, 0x50, 0x24, 0x04
};

static uint8 splatterStart[128] = {	/* starting bit position */
	0x00, 0x18, 0x30, 0xc4, 0xdc, 0x65, 0xeb, 0x48,
	0x60, 0xbd, 0x89, 0x05, 0x0a, 0xf4, 0x7d, 0x7d,
	0x85, 0xb0, 0x8e, 0x95, 0x1f, 0x22, 0x0d, 0xdf,
	0x2a, 0x78, 0xd5, 0x73, 0x1c, 0xb4, 0x40, 0xa1,
	0xb9, 0x3c, 0xca, 0x58, 0x92, 0x34, 0xcc, 0xce,
	0xd7, 0x42, 0x90, 0x0f, 0x8b, 0x7f, 0x32, 0xed,
	0x5c, 0x9d, 0xc8, 0x99, 0xad, 0x4e, 0x56, 0xa6,
	0xf7, 0x68, 0xb7, 0x25, 0x82, 0x37, 0x3a, 0x51,
	0x69, 0x26, 0x38, 0x52, 0x9e, 0x9a, 0x4f, 0xa7,
	0x43, 0x10, 0x80, 0xee, 0x3d, 0x59, 0x35, 0xcf,
	0x79, 0x74, 0xb5, 0xa2, 0xb1, 0x96, 0x23, 0xe0,
	0xbe, 0x05, 0xf5, 0x6e, 0x19, 0xc5, 0x66, 0x49,
	0xf0, 0xd1, 0x54, 0xa9, 0x70, 0x4b, 0xa4, 0xe2,
	0xe6, 0xe5, 0xab, 0xe4, 0xd2, 0xaa, 0x4c, 0xe3,
	0x06, 0x6f, 0xc6, 0x4a, 0xa4, 0x75, 0x97, 0xe1
};

void PictureMgr::putVirtPixel(int x, int y) {
	uint8 *p;

	if (x < 0 || y < 0 || x >= _WIDTH || y >= _HEIGHT)
		return;

	p = &_vm->_game.sbuf16c[y * _WIDTH + x];

	if (priOn)
		*p = (priColour << 4) | (*p & 0x0f);
	if (scrOn)
		*p = scrColour | (*p & 0xf0);
}

/* For the flood fill routines */

/* MH2 needs stack size > 300 */
#define STACK_SIZE 512
static unsigned int stackPtr;
static uint16 stack[STACK_SIZE];

static INLINE void lpush(uint16 c) {
	assert(stackPtr < STACK_SIZE);

	stack[stackPtr] = c;
	stackPtr++;
}

static INLINE uint16 lpop() {
	if (stackPtr == 0)
		return 0xffff;

	stackPtr--;
	return stack[stackPtr];
}

/**
 * Draw an AGI line.
 * A line drawing routine sent by Joshua Neal, modified by Stuart George
 * (fixed >>2 to >>1 and some other bugs like x1 instead of y1, etc.)
 * @param x1  x coordinate of start point
 * @param y1  y coordinate of start point
 * @param x2  x coordinate of end point
 * @param y2  y coordinate of end point
 */
void PictureMgr::drawLine(int x1, int y1, int x2, int y2) {
	int i, x, y, deltaX, deltaY, stepX, stepY, errorX, errorY, detdelta;

	/* CM: Do clipping */
#define clip(x, y) if((x)>=(y)) (x)=(y)
	clip(x1, _WIDTH - 1);
	clip(x2, _WIDTH - 1);
	clip(y1, _HEIGHT - 1);
	clip(y2, _HEIGHT - 1);

	/* Vertical line */

	if (x1 == x2) {
		if (y1 > y2) {
			y = y1;
			y1 = y2;
			y2 = y;
		}

		for (; y1 <= y2; y1++)
			putVirtPixel(x1, y1);

		return;
	}

	/* Horizontal line */

	if (y1 == y2) {
		if (x1 > x2) {
			x = x1;
			x1 = x2;
			x2 = x;
		}
		for (; x1 <= x2; x1++)
			putVirtPixel(x1, y1);
		return;
	}

	y = y1;
	x = x1;

	stepY = 1;
	deltaY = y2 - y1;
	if (deltaY < 0) {
		stepY = -1;
		deltaY = -deltaY;
	}

	stepX = 1;
	deltaX = x2 - x1;
	if (deltaX < 0) {
		stepX = -1;
		deltaX = -deltaX;
	}

	if (deltaY > deltaX) {
		i = deltaY;
		detdelta = deltaY;
		errorX = deltaY / 2;
		errorY = 0;
	} else {
		i = deltaX;
		detdelta = deltaX;
		errorX = 0;
		errorY = deltaX / 2;
	}

	putVirtPixel(x, y);

	do {
		errorY += deltaY;
		if (errorY >= detdelta) {
			errorY -= detdelta;
			y += stepY;
		}

		errorX += deltaX;
		if (errorX >= detdelta) {
			errorX -= detdelta;
			x += stepX;
		}

		putVirtPixel(x, y);
		i--;
	} while (i > 0);
}

/**
 * Draw a relative AGI line.
 * Draws short lines relative to last position. (drawing action 0xF7)
 */
void PictureMgr::dynamicDrawLine() {
	int x1, y1, disp, dx, dy;

	x1 = nextByte;
	y1 = nextByte;

	putVirtPixel(x1, y1);

	for (;;) {
		if ((disp = nextByte) >= 0xe0)
			break;

		dx = ((disp & 0xf0) >> 4) & 0x0f;
		dy = (disp & 0x0f);

		if (dx & 0x08)
			dx = -(dx & 0x07);
		if (dy & 0x08)
			dy = -(dy & 0x07);

		drawLine(x1, y1, x1 + dx, y1 + dy);
		x1 += dx;
		y1 += dy;
	}
	foffs--;
}

/**************************************************************************
** absoluteLine
**
** Draws long lines to actual locations (cf. relative) (drawing action 0xF6)
**************************************************************************/
void PictureMgr::absoluteDrawLine() {
	int x1, y1, x2, y2;

	x1 = nextByte;
	y1 = nextByte;
	putVirtPixel(x1, y1);

	for (;;) {
		if ((x2 = nextByte) >= 0xe0)
			break;

		if ((y2 = nextByte) >= 0xe0)
			break;

		drawLine(x1, y1, x2, y2);
		x1 = x2;
		y1 = y2;
	}
	foffs--;
}

/**************************************************************************
** okToFill
**************************************************************************/
INLINE int PictureMgr::isOkFillHere(int x, int y) {
	uint8 p;

	if (x < 0 || x >= _WIDTH || y < 0 || y >= _HEIGHT)
		return false;

	if (!scrOn && !priOn)
		return false;

	p = _vm->_game.sbuf16c[y * _WIDTH + x];

	if (!priOn && scrOn && scrColour != 15)
		return (p & 0x0f) == 15;

	if (priOn && !scrOn && priColour != 4)
		return (p >> 4) == 4;

	return (scrOn && (p & 0x0f) == 15 && scrColour != 15);
}

/**************************************************************************
** agi_fill
**************************************************************************/
void PictureMgr::fillScanline(int x, int y) {
	unsigned int c;
	int newspanUp, newspanDown;

	if (!isOkFillHere(x, y))
		return;

	/* Scan for left border */
	for (c = x - 1; isOkFillHere(c, y); c--);

	newspanUp = newspanDown = 1;
	for (c++; isOkFillHere(c, y); c++) {
		putVirtPixel(c, y);
		if (isOkFillHere(c, y - 1)) {
			if (newspanUp) {
				lpush(c + 320 * (y - 1));
				newspanUp = 0;
			}
		} else {
			newspanUp = 1;
		}

		if (isOkFillHere(c, y + 1)) {
			if (newspanDown) {
				lpush(c + 320 * (y + 1));
				newspanDown = 0;
			}
		} else {
			newspanDown = 1;
		}
	}
}

void PictureMgr::agiFill(unsigned int x, unsigned int y) {
	lpush(x + 320 * y);

	for (;;) {
		uint16 c = lpop();

		/* Exit if stack is empty */
		if (c == 0xffff)
			break;

		x = c % 320;
		y = c / 320;

		fillScanline(x, y);
	}

	stackPtr = 0;
}

/**************************************************************************
** xCorner
**
** Draws an xCorner  (drawing action 0xF5)
**************************************************************************/
void PictureMgr::xCorner() {
	int x1, x2, y1, y2;

	x1 = nextByte;
	y1 = nextByte;
	putVirtPixel(x1, y1);

	for (;;) {
		x2 = nextByte;

		if (x2 >= 0xe0)
			break;

		drawLine(x1, y1, x2, y1);
		x1 = x2;
		y2 = nextByte;

		if (y2 >= 0xe0)
			break;

		drawLine(x1, y1, x1, y2);
		y1 = y2;
	}
	foffs--;
}

/**************************************************************************
** yCorner
**
** Draws an yCorner  (drawing action 0xF4)
**************************************************************************/
void PictureMgr::yCorner() {
	int x1, x2, y1, y2;

	x1 = nextByte;
	y1 = nextByte;
	putVirtPixel(x1, y1);

	for (;;) {
		y2 = nextByte;

		if (y2 >= 0xe0)
			break;

		drawLine(x1, y1, x1, y2);
		y1 = y2;
		x2 = nextByte;

		if (x2 >= 0xe0)
			break;

		drawLine(x1, y1, x2, y1);
		x1 = x2;
	}

	foffs--;
}

/**************************************************************************
** fill
**
** AGI flood fill.  (drawing action 0xF8)
**************************************************************************/
void PictureMgr::fill() {
	int x1, y1;

	while ((x1 = nextByte) < 0xe0 && (y1 = nextByte) < 0xe0)
		agiFill(x1, y1);

	foffs--;
}

/**************************************************************************
** plotPattern
**
** Draws pixels, circles, squares, or splatter brush patterns depending
** on the pattern code.
**************************************************************************/

int PictureMgr::plotPatternPoint(int x, int y, int bitpos) {
	if (patCode & 0x20) {
		if ((splatterMap[bitpos >> 3] >> (7 - (bitpos & 7))) & 1) {
			putVirtPixel(x, y);
		}
		bitpos++;
		if (bitpos == 0xff)
			bitpos = 0;
	} else
		putVirtPixel(x, y);

	return bitpos;
}

void PictureMgr::plotPattern(int x, int y) {
	int32 circlePos = 0;
	uint32 x1, y1, pensize, bitpos = splatterStart[patNum];

	pensize = (patCode & 7);

	if (x < (int)pensize)
		x = pensize - 1;
	if (y < (int)pensize)
		y = pensize;

	for (y1 = y - pensize; y1 <= y + pensize; y1++) {
		for (x1 = x - (pensize + 1) / 2; x1 <= x + pensize / 2; x1++) {
			if (patCode & 0x10) {	/* Square */
				bitpos = plotPatternPoint (x1, y1, bitpos);
			} else {	/* Circle */
				if ((circles[patCode & 7][circlePos >> 3] >> (7 - (circlePos & 7))) & 1) {
					bitpos = plotPatternPoint(x1, y1, bitpos);
				}
				circlePos++;
			}
		}
	}
}

/**************************************************************************
** plotBrush
**
** Plots points and various brush patterns.
**************************************************************************/
void PictureMgr::plotBrush() {
	int x1, y1;

	for (;;) {
		if (patCode & 0x20) {
			if ((patNum = nextByte) >= 0xe0)
				break;
			patNum = (patNum >> 1) & 0x7f;
		}

		if ((x1 = nextByte) >= 0xe0)
			break;

		if ((y1 = nextByte) >= 0xe0)
			break;

		plotPattern(x1, y1);
	}

	foffs--;
}

/**************************************************************************
** Draw AGI vC64 format picture
**************************************************************************/

void PictureMgr::drawPictureVC64() {
	uint8 act;
	int drawing;

	patCode = 0;
	patNum = 0;
	priOn = scrOn = false;
	scrColour = 0xf;
	priColour = 0x4;

	drawing = 1;

	debugC(8, kDebugLevelMain, "Drawing vC64 picture");
	for (drawing = 1; drawing && foffs < flen;) {
		act = nextByte;
		switch (act) {
		case 0xe0:	/* x-corner */
			xCorner();
			break;
		case 0xe1:	/* y-corner */
			yCorner();
			break;
		case 0xe2:	/* dynamic draw lines */
			dynamicDrawLine();
			break;
		case 0xe3:	/* absolute draw lines */
			absoluteDrawLine();
			break;
		case 0xe4:	/* fill */
			scrColour = nextByte;
			scrColour &= 0xF;	/* for v3 drawing diff */
			fill();
			break;
		case 0xe5:	/* enable screen drawing */
			scrOn = true;
			break;
		case 0xe6:	/* plot brush */
			patCode = nextByte;
			plotBrush();
			break;
		case 0xf0:	/* set colour on screen */
		case 0xf2:
		case 0xf3:
		case 0xf4:
		case 0xf5:
		case 0xf6:
		case 0xf7:
		case 0xf8:
		case 0xf9:
		case 0xfa:
		case 0xfb:
		case 0xfc:
		case 0xfd:
		case 0xfe:
			scrColour = act - 0xf0;
			break;
		default:
			warning("Unknown vC64 picture opcode (%x)", act);
		}
	}
}

/**************************************************************************
** Draw AGI v1 format picture
**************************************************************************/

void PictureMgr::drawPictureV1() {
	uint8 act;
	int drawing;

	patCode = 0;
	patNum = 0;
	priOn = scrOn = false;
	scrColour = 0xf;
	priColour = 0x4;

	drawing = 1;

	debugC(8, kDebugLevelMain, "Drawing v1 picture");
	for (drawing = 1; drawing && foffs < flen;) {
		act = nextByte;
		switch (act) {
		case 0xf1:	/* set colour on screen */
			scrColour = nextByte;
			scrColour &= 0xF;	/* for v3 drawing diff */
			scrOn = true;
			priOn = false;
			break;
		case 0xf3:	/* set colour on priority */
			scrColour = nextByte;
			scrColour &= 0xF;	/* for v3 drawing diff */
			scrOn = true;
			priColour = nextByte;
			priColour &= 0xf;	/* for v3 drawing diff */
			priOn = true;
			break;
		case 0xf4:	/* y-corner */
			yCorner();
			break;
		case 0xf5:	/* x-corner */
			xCorner();
			break;
		case 0xf6:	/* absolute draw lines */
			absoluteDrawLine();
			break;
		case 0xf7:	/* dynamic draw lines */
		case 0xfb:
			dynamicDrawLine();
			break;
		case 0xfa:
			scrOn = false;
			priOn = true;
			absoluteDrawLine();
			scrOn = true;
			priOn = false;
			break;
		case 0xfc:	/* fill */
			scrColour = nextByte;
			scrColour &= 0xF;
			priColour = nextByte;
			priColour &= 0xf;
			fill();
			break;
		case 0xFF:	/* end of pic data */
			drawing = 0;
			break;
		default:
			warning("Unknown v1 picture opcode (%x)", act);
		}
	}
}

/**************************************************************************
** Draw AGI v1.5 format picture
**************************************************************************/

void PictureMgr::drawPictureV15() {
	uint8 act;
	int drawing;

	patCode = 0;
	patNum = 0;
	priOn = scrOn = false;
	scrColour = 0xf;

	drawing = 1;

	debugC(8, kDebugLevelMain, "Drawing v1.5 picture");
	for (drawing = 1; drawing && foffs < flen;) {
		act = nextByte;
		switch (act) {
		case 0xf0:	
			break;
		case 0xf1:	/* set colour on screen */
			scrColour = nextByte;
			scrColour &= 0xF;
			break;
		case 0xf3:
			break;
		case 0xf8:	/* absolute draw lines */
		case 0xf9:
		case 0xfa:
		case 0xfb:
			absoluteDrawLine();
			break;
		case 0xfe:	/* fill */
			scrColour = nextByte;
			scrColour &= 0xF;
			scrOn = true;
			fill();
			break;
		case 0xFF:	/* end of pic data */
			drawing = 0;
			break;
		default:
			warning("Unknown v1.5 picture opcode (%x)", act);
		}
	}
}

/**************************************************************************
** Draw AGI v2 format picture
**************************************************************************/

void PictureMgr::drawPictureV2() {
	uint8 act;
	int drawing;

	patCode = 0;
	patNum = 0;
	priOn = scrOn = false;
	scrColour = 0xf;
	priColour = 0x4;

	drawing = 1;

	debugC(8, kDebugLevelMain, "Drawing v2 picture");
	for (drawing = 1; drawing && foffs < flen;) {
		act = nextByte;
		switch (act) {
		case 0xf0:	/* set colour on screen */
			scrColour = nextByte;
			scrColour &= 0xF;	/* for v3 drawing diff */
			scrOn = true;
			break;
		case 0xf1:	/* disable screen drawing */
			scrOn = false;
			break;
		case 0xf2:	/* set colour on priority */
			priColour = nextByte;
			priColour &= 0xf;	/* for v3 drawing diff */
			priOn = true;
			break;
		case 0xf3:	/* disable priority screen */
			priOn = false;
			break;
		case 0xf4:	/* y-corner */
			yCorner();
			break;
		case 0xf5:	/* x-corner */
			xCorner();
			break;
		case 0xf6:	/* absolute draw lines */
			absoluteDrawLine();
			break;
		case 0xf7:	/* dynamic draw lines */
			dynamicDrawLine();
			break;
		case 0xf8:	/* fill */
			fill();
			break;
		case 0xf9:	/* set pattern */
			patCode = nextByte;

			if (_vm->getGameType() == GType_PreAGI)
				plotBrush();
			break;
		case 0xfA:	/* plot brush */
			plotBrush();
			break;
		case 0xFF:	/* end of pic data */
			drawing = 0;
			break;
		default:
			warning("Unknown v2 picture opcode (%x)", act);
		}
	}
}

/*
 * Public functions
 */

/**
 * convert AGI v3 format picture to AGI v2 format
 */
uint8 *PictureMgr::convertV3Pic(uint8 *src, uint32 len) {
	uint8 d, old = 0, x, *in, *xdata, *out, mode = 0;
	uint32 i, ulen;

	xdata = (uint8 *)malloc(len + len / 2);

	out = xdata;
	in = src;

	for (i = ulen = 0; i < len; i++, ulen++) {
		d = *in++;

		*out++ = x = mode ? ((d & 0xF0) >> 4) + ((old & 0x0F) << 4) : d;

		if (x == 0xFF) {
			ulen++;
			break;
		}

		if (x == 0xf0 || x == 0xf2) {
			if (mode) {
				*out++ = d & 0x0F;
				ulen++;
			} else {
				d = *in++;
				*out++ = (d & 0xF0) >> 4;
				i++, ulen++;
			}

			mode = !mode;
		}

		old = d;
	}

	free(src);
	xdata = (uint8 *)realloc(xdata, ulen);

	return xdata;
}

/**
 * Decode an AGI picture resource.
 * This function decodes an AGI picture resource into the correct slot
 * and draws it on the AGI screen, optionally clearing the screen before
 * drawing.
 * @param n      AGI picture resource number
 * @param clear  clear AGI screen before drawing
 * @param agi256 load an AGI256 picture resource
 */
int PictureMgr::decodePicture(int n, int clear, bool agi256) {
	debugC(8, kDebugLevelResources, "(%d)", n);

	patCode = 0;
	patNum = 0;
	priOn = scrOn = false;
	scrColour = 0xF;
	priColour = 0x4;

	data = _vm->_game.pictures[n].rdata;
	flen = _vm->_game.dirPic[n].len;
	foffs = 0;

	if (clear && !agi256) // 256 color pictures should always fill the whole screen, so no clearing for them.
		memset(_vm->_game.sbuf16c, 0x4f, _WIDTH * _HEIGHT); // Clear 16 color AGI screen (Priority 4, color white).

	if (!agi256) {
		drawPictureV2(); // Draw 16 color picture.
	} else {
		const uint32 maxFlen = _WIDTH * _HEIGHT;
		memcpy(_vm->_game.sbuf256c, data, MIN(flen, maxFlen)); // Draw 256 color picture.

		if (flen < maxFlen) {
			warning("Undersized AGI256 picture resource %d, using it anyway. Filling rest with white.", n);
			memset(_vm->_game.sbuf256c + flen, 0x0f, maxFlen - flen); // Fill missing area with white.
		} else if (flen > maxFlen)
			warning("Oversized AGI256 picture resource %d, decoding only %ux%u part of it", n, _WIDTH, _HEIGHT);
	}

	if (clear)
		_vm->clearImageStack();
	_vm->recordImageStackCall(ADD_PIC, n, clear, agi256, 0, 0, 0, 0);

	return errOK;
}

/**
 * Unload an AGI picture resource.
 * This function unloads an AGI picture resource and deallocates
 * resource data.
 * @param n AGI picture resource number
 */
int PictureMgr::unloadPicture(int n) {
	/* remove visual buffer & priority buffer if they exist */
	if (_vm->_game.dirPic[n].flags & RES_LOADED) {
		free(_vm->_game.pictures[n].rdata);
		_vm->_game.dirPic[n].flags &= ~RES_LOADED;
	}

	return errOK;
}

/**
 * Show AGI picture.
 * This function copies a ``hidden'' AGI picture to the output device.
 */
void PictureMgr::showPic() {
	int i, y;
	int offset;

	debugC(8, kDebugLevelMain, "Show picture!");

	i = 0;
	offset = _vm->_game.lineMinPrint * CHAR_LINES;
	for (y = 0; y < _HEIGHT; y++) {
		_gfx->putPixelsA(0, y + offset, _WIDTH, &_vm->_game.sbuf16c[i]);
		i += _WIDTH;
	}

	_gfx->flushScreen();
}

} // End of namespace Agi
