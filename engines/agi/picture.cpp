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

PictureMgr::PictureMgr(AgiBase *agi, GfxMgr *gfx) {
	_vm = agi;
	_gfx = gfx;
	
	_data = NULL;
	_flen = _foffs = 0;

	_patCode = _patNum = _priOn = _scrOn = _scrColor = _priColor = 0;
	_xOffset = _yOffset = 0;

	_pictureType = AGIPIC_V2;
	_minCommand = 0xf0;
	_flags = 0;
}

void PictureMgr::putVirtPixel(int x, int y) {
	uint8 *p;

	x += _xOffset;
	y += _yOffset;

	if (x < 0 || y < 0 || x >= _width || y >= _height)
		return;

	p = &_vm->_game.sbuf16c[y * _width + x];

	if (_priOn)
		*p = (_priColor << 4) | (*p & 0x0f);
	if (_scrOn)
		*p = _scrColor | (*p & 0xf0);
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
	clip(x1, _width - 1);
	clip(x2, _width - 1);
	clip(y1, _height - 1);
	clip(y2, _height - 1);

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

	x1 = nextByte();
	y1 = nextByte();

	putVirtPixel(x1, y1);

	for (;;) {
		if ((disp = nextByte()) >= _minCommand)
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
	_foffs--;
}

/**************************************************************************
** absoluteLine
**
** Draws long lines to actual locations (cf. relative) (drawing action 0xF6)
**************************************************************************/
void PictureMgr::absoluteDrawLine() {
	int x1, y1, x2, y2;

	x1 = nextByte();
	y1 = nextByte();
	putVirtPixel(x1, y1);

	for (;;) {
		if ((x2 = nextByte()) >= _minCommand)
			break;

		if ((y2 = nextByte()) >= _minCommand)
			break;

		drawLine(x1, y1, x2, y2);
		x1 = x2;
		y1 = y2;
	}
	_foffs--;
}

/**************************************************************************
** okToFill
**************************************************************************/
INLINE int PictureMgr::isOkFillHere(int x, int y) {
	uint8 p;

	if (x < 0 || x >= _width || y < 0 || y >= _height)
		return false;

	if (!_scrOn && !_priOn)
		return false;

	p = _vm->_game.sbuf16c[y * _width + x];

	if (!_priOn && _scrOn && _scrColor != 15)
		return (p & 0x0f) == 15;

	if (_priOn && !_scrOn && _priColor != 4)
		return (p >> 4) == 4;

	return (_scrOn && (p & 0x0f) == 15 && _scrColor != 15);
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

	x1 = nextByte();
	y1 = nextByte();
	putVirtPixel(x1, y1);

	for (;;) {
		x2 = nextByte();

		if (x2 >= _minCommand)
			break;

		drawLine(x1, y1, x2, y1);
		x1 = x2;
		y2 = nextByte();

		if (y2 >= _minCommand)
			break;

		drawLine(x1, y1, x1, y2);
		y1 = y2;
	}
	_foffs--;
}

/**************************************************************************
** yCorner
**
** Draws an yCorner  (drawing action 0xF4)
**************************************************************************/
void PictureMgr::yCorner() {
	int x1, x2, y1, y2;

	x1 = nextByte();
	y1 = nextByte();
	putVirtPixel(x1, y1);

	for (;;) {
		y2 = nextByte();

		if (y2 >= _minCommand)
			break;

		drawLine(x1, y1, x1, y2);
		y1 = y2;
		x2 = nextByte();

		if (x2 >= _minCommand)
			break;

		drawLine(x1, y1, x2, y1);
		x1 = x2;
	}

	_foffs--;
}

/**************************************************************************
** fill
**
** AGI flood fill.  (drawing action 0xF8)
**************************************************************************/
void PictureMgr::fill() {
	int x1, y1;

	while ((x1 = nextByte()) < _minCommand && (y1 = nextByte()) < _minCommand)
		agiFill(x1, y1);

	_foffs--;
}

/**************************************************************************
** plotPattern
**
** Draws pixels, circles, squares, or splatter brush patterns depending
** on the pattern code.
**************************************************************************/

void PictureMgr::plotPattern(int x, int y) {
	static uint16 binary_list[] = {0x8000, 0x4000, 0x2000, 0x1000, 0x800, 0x400, 0x200, 0x100, 
		0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1};

	static uint8 circle_list[] = {0, 1, 4, 9, 16, 25, 37, 50};

	static uint16 circle_data[] =
		{0x8000, 
		0xE000, 0xE000, 0xE000, 
		0x7000, 0xF800, 0x0F800, 0x0F800, 0x7000, 
		0x3800, 0x7C00, 0x0FE00, 0x0FE00, 0x0FE00, 0x7C00, 0x3800, 
		0x1C00, 0x7F00, 0x0FF80, 0x0FF80, 0x0FF80, 0x0FF80, 0x0FF80, 0x7F00, 0x1C00,
		0x0E00, 0x3F80, 0x7FC0, 0x7FC0, 0x0FFE0, 0x0FFE0, 0x0FFE0, 0x7FC0, 0x7FC0, 0x3F80, 0x1F00, 0x0E00,
		0x0F80, 0x3FE0, 0x7FF0, 0x7FF0, 0x0FFF8, 0x0FFF8, 0x0FFF8, 0x0FFF8, 0x0FFF8, 0x7FF0, 0x7FF0, 0x3FE0, 0x0F80,
		0x07C0, 0x1FF0, 0x3FF8, 0x7FFC, 0x7FFC, 0x0FFFE, 0x0FFFE, 0x0FFFE, 0x0FFFE, 0x0FFFE, 0x7FFC, 0x7FFC, 0x3FF8, 0x1FF0, 0x07C0};

	uint16 circle_word;
	uint16 *circle_ptr;
	uint16 counter;
	uint16 pen_width = 0;
	int pen_final_x = 0;
	int pen_final_y = 0;
	
	uint8 t = 0;
	uint8 temp8;
	uint16 temp16;

	int	pen_x = x;
	int	pen_y = y;
	uint16	texture_num = 0;
	uint16	pen_size = (_patCode & 0x07);

	circle_ptr = &circle_data[circle_list[pen_size]];
	
	// setup the X position
	// = pen_x - pen.size/2

	pen_x = (pen_x * 2) - pen_size;
	if (pen_x < 0) pen_x = 0;

	temp16 = 320 - (2 * pen_size);
	if (pen_x >= temp16)
		pen_x = temp16;
		
	pen_x /= 2;
	pen_final_x = pen_x;	// original starting point?? -> used in plotrelated

	// Setup the Y Position
	// = pen_y - pen.size
	pen_y = pen_y - pen_size;
	if (pen_y < 0) pen_y = 0;

	temp16 = 167 - (2 * pen_size);
	if (pen_y >= temp16)
		pen_y = temp16;
		
	pen_final_y = pen_y;	// used in plotrelated

	t = (uint8)(texture_num | 0x01);		// even
	
	// new purpose for temp16
	
	temp16 =( pen_size<<1) +1;	// pen size
	pen_final_y += temp16;					// the last row of this shape
	temp16 = temp16 << 1;
	pen_width = temp16;					// width of shape?

	bool circleCond;
	int counterStep;
	int ditherCond;

	if (_flags == kPicFCircle)
		_patCode |= 0x10;

	if (_vm->getGameType() == GType_PreAGI) {
		circleCond = ((_patCode & 0x10) == 0);
		counterStep = 3;
		ditherCond = 0x03;
	} else {
		circleCond = ((_patCode & 0x10) != 0);
		counterStep = 4;
		ditherCond = 0x02;
	}

	for (; pen_y < pen_final_y; pen_y++) {
		circle_word = *circle_ptr++;
		
		for (counter = 0; counter <= pen_width; counter += counterStep) {
			if (circleCond || ((binary_list[counter>>1] & circle_word) != 0)) {
				temp8 = t % 2;
				t = t >> 1;
				if (temp8 != 0) 
					t = t ^ 0xB8;

				// == box plot, != circle plot
				if ((_patCode & 0x20) == 0 || (t & 0x03) == ditherCond)
					putVirtPixel(pen_x, pen_y);
			}
			pen_x++;
		}

		pen_x = pen_final_x;
	}

	return;
}

/**************************************************************************
** plotBrush
**
** Plots points and various brush patterns.
**************************************************************************/
void PictureMgr::plotBrush() {
	int x1, y1;

	for (;;) {
		if (_patCode & 0x20) {
			if ((_patNum = nextByte()) >= _minCommand)
				break;
			_patNum = (_patNum >> 1) & 0x7f;
		}

		if ((x1 = nextByte()) >= _minCommand)
			break;

		if ((y1 = nextByte()) >= _minCommand)
			break;

		plotPattern(x1, y1);
	}

	_foffs--;
}

/**************************************************************************
** Draw AGI picture
**************************************************************************/

void PictureMgr::drawPicture() {
	uint8 act;
	int drawing;
	int storedXOffset = 0, storedYOffset = 0;

	_patCode = 0;
	_patNum = 0;
	_priOn = _scrOn = false;
	_scrColor = 0xf;
	_priColor = 0x4;

	drawing = 1;

	debugC(8, kDebugLevelMain, "Drawing v2 picture");
	for (drawing = 1; drawing && _foffs < _flen;) {
		act = nextByte();

		if (_pictureType == AGIPIC_C64 && act >= 0xf0 && act <= 0xfe) {
			_scrColor = act - 0xf0;
			continue;
		}

		switch (act) {
		case 0xe0:	// x-corner (C64)
			xCorner();
			break;
		case 0xe1:	// y-corner (C64)
			yCorner();
			break;
		case 0xe2:	// dynamic draw lines (C64)
			dynamicDrawLine();
			break;
		case 0xe3:	// absolute draw lines (C64)
			absoluteDrawLine();
			break;
		case 0xe4:	// fill (C64)
			_scrColor = nextByte();
			_scrColor &= 0xF;	/* for v3 drawing diff */
			fill();
			break;
		case 0xe5:	// enable screen drawing (C64)
			_scrOn = true;
			break;
		case 0xe6:	// plot brush (C64)
			_patCode = nextByte();
			plotBrush();
			break;
		case 0xf0:	// set colour on screen (AGI pic v2)
			_scrColor = nextByte();
			_scrColor &= 0xF;	// for v3 drawing diff
			_scrOn = true;
			break;
		case 0xf1:
			if (_pictureType == AGIPIC_V1) {
				_scrColor = nextByte();
				_scrColor &= 0xF;	// for v3 drawing diff
				_scrOn = true;
				_priOn = false;
			} else if (_pictureType == AGIPIC_V15) {	// set colour on screen
				_scrColor = nextByte();
				_scrColor &= 0xF;
			} else if (_pictureType == AGIPIC_V2) {	// disable screen drawing
				_scrOn = false;
			}
			break;
		case 0xf2:	// set colour on priority (AGI pic v2)
			_priColor = nextByte();
			_priColor &= 0xf;	// for v3 drawing diff
			_priOn = true;
			break;
		case 0xf3:
			if (_pictureType == AGIPIC_V1) {
				_scrColor = nextByte();
				_scrColor &= 0xF;	// for v3 drawing diff
				_scrOn = true;
				_priColor = nextByte();
				_priColor &= 0xf;	// for v3 drawing diff
				_priOn = true;
			}

			// Empty in AGI pic V1.5

			if (_pictureType == AGIPIC_V2)	// disable priority screen
				_priOn = false;
			break;
		case 0xf4:	// y-corner
			yCorner();
			break;
		case 0xf5:	// x-corner
			xCorner();
			break;
		case 0xf6:	// absolute draw lines
			absoluteDrawLine();
			break;
		case 0xf7:	// dynamic draw lines
			dynamicDrawLine();
			break;
		case 0xf8:	// fill
			if (_pictureType == AGIPIC_V15) {
				absoluteDrawLine();
			} else if (_pictureType == AGIPIC_V2) {
				fill();
			}
			break;
		case 0xf9:	// set pattern
			if (_pictureType == AGIPIC_V15) {
				absoluteDrawLine();
			} else if (_pictureType == AGIPIC_V2) {
				_patCode = nextByte();

				if (_vm->getGameType() == GType_PreAGI)
					plotBrush();
			}
			break;
		case 0xfa:	// plot brush
			if (_pictureType == AGIPIC_V1) {
				_scrOn = false;
				_priOn = true;
				absoluteDrawLine();
				_scrOn = true;
				_priOn = false;
			} else if (_pictureType == AGIPIC_V15) {
				absoluteDrawLine();
			} else if (_pictureType == AGIPIC_V2) {
				plotBrush();
			}
			break;
		case 0xfb:
			if (_pictureType == AGIPIC_V1) {
				dynamicDrawLine();
			} else if (_pictureType == AGIPIC_V15) {
				absoluteDrawLine();
			}
			break;
		case 0xfc:	// fill (AGI pic v1)
			_scrColor = nextByte();
			_scrColor &= 0xF;
			_priColor = nextByte();
			_priColor &= 0xf;
			fill();
			break;
		case 0xfe:	// fill (AGI pic v1.5)
			_scrColor = nextByte();
			_scrColor &= 0xF;
			_scrOn = true;
			fill();
			break;
		case 0xff:	// end of pic data
			drawing = 0;
			break;
		default:
			warning("Unknown v2 picture opcode (%x)", act);
		}
		if (_flags == kPicFStep && _vm->getGameType() == GType_PreAGI) {
			// FIXME: This is used by Mickey for the crystal animation, but
			// currently it's very very very slow
			/*
			storedXOffset = _xOffset;
			storedYOffset = _yOffset;
			// FIXME: picture coordinates are correct for Mickey only
			showPic(10, 0, _width, _height);
			_gfx->doUpdate();
			g_system->updateScreen();
			_xOffset = storedXOffset;
			_yOffset = storedYOffset;
			g_system->delayMillis(25);
			*/
		}
	}
}

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
int PictureMgr::decodePicture(int n, int clear, bool agi256, int pic_width, int pic_height) {
	debugC(8, kDebugLevelResources, "(%d)", n);

	_patCode = 0;
	_patNum = 0;
	_priOn = _scrOn = false;
	_scrColor = 0xF;
	_priColor = 0x4;

	_data = _vm->_game.pictures[n].rdata;
	_flen = _vm->_game.dirPic[n].len;
	_foffs = 0;

	_width = pic_width;
	_height = pic_height;

	if (clear && !agi256) // 256 color pictures should always fill the whole screen, so no clearing for them.
		memset(_vm->_game.sbuf16c, 0x4f, _width * _height); // Clear 16 color AGI screen (Priority 4, color white).

	if (!agi256) {
		drawPicture(); // Draw 16 color picture.
	} else {
		const uint32 maxFlen = _width * _height;
		memcpy(_vm->_game.sbuf256c, _data, MIN(_flen, maxFlen)); // Draw 256 color picture.

		if (_flen < maxFlen) {
			warning("Undersized AGI256 picture resource %d, using it anyway. Filling rest with white.", n);
			memset(_vm->_game.sbuf256c + _flen, 0x0f, maxFlen - _flen); // Fill missing area with white.
		} else if (_flen > maxFlen)
			warning("Oversized AGI256 picture resource %d, decoding only %ux%u part of it", n, _width, _height);
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
void PictureMgr::showPic(int x, int y, int pic_width, int pic_height) {
	int i, y1;
	int offset;
	_width = pic_width;
	_height = pic_height;

	debugC(8, kDebugLevelMain, "Show picture!");

	i = 0;
	offset = _vm->_game.lineMinPrint * CHAR_LINES;
	for (y1 = y; y1 < y + _height; y1++) {
		_gfx->putPixelsA(x, y1 + offset, _width, &_vm->_game.sbuf16c[i]);
		i += _width;
	}

	_gfx->flushScreen();
}

// preagi needed functions (for plotPattern)
void PictureMgr::setPattern(uint8 code, uint8 num) {
	_patCode = code;
	_patNum = num;
}

void PictureMgr::setPictureType(int type) {
	_pictureType = type;

	if (type == AGIPIC_C64)
		_minCommand = 0xe0;
	else
		_minCommand = 0xf0;
}

void PictureMgr::setPictureData(uint8 *data, int len) {
	_data = data;
	_flen = len;
	_foffs = 0;
	_flags = 0;
}

} // End of namespace Agi
