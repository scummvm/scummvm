/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2001 Sarien Team
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
#include "agi/savegame.h"

namespace Agi {

#define next_byte data[foffs++]

static uint8 *data;
static uint32 flen;
static uint32 foffs;

static uint8 pat_code;
static uint8 pat_num;
static uint8 pri_on;
static uint8 scr_on;
static uint8 scr_colour;
static uint8 pri_colour;

static uint8 circles[][15] = {	/* agi circle bitmaps */
	{0x80},
	{0xfc},
	{0x5f, 0xf4},
	{0x66, 0xff, 0xf6, 0x60},
	{0x23, 0xbf, 0xff, 0xff, 0xee, 0x20},
	{0x31, 0xe7, 0x9e, 0xff, 0xff, 0xde, 0x79, 0xe3, 0x00},
	{0x38, 0xf9, 0xf3, 0xef, 0xff, 0xff, 0xff, 0xfe, 0xf9, 0xf3, 0xe3, 0x80},
	{0x18, 0x3c, 0x7e, 0x7e, 0x7e, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x7e, 0x7e, 0x3c, 0x18}
};

static uint8 splatter_map[32] = {	/* splatter brush bitmaps */
	0x20, 0x94, 0x02, 0x24, 0x90, 0x82, 0xa4, 0xa2,
	0x82, 0x09, 0x0a, 0x22, 0x12, 0x10, 0x42, 0x14,
	0x91, 0x4a, 0x91, 0x11, 0x08, 0x12, 0x25, 0x10,
	0x22, 0xa8, 0x14, 0x24, 0x00, 0x50, 0x24, 0x04
};

static uint8 splatter_start[128] = {	/* starting bit position */
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

static void fix_pixel_bothsides(int x, int y);

static void put_virt_pixel(int x, int y, int res) {
	uint8 *p;
	int width = _WIDTH * res;

	if (x < 0 || y < 0 || x >= width || y >= _HEIGHT)
		return;

	p =
#ifdef USE_HIRES
	    res > 1 ? &game.hires[y * width + x] :
#endif
	    &game.sbuf[y * width + x];

	if (pri_on)
		*p = (pri_colour << 4) | (*p & 0x0f);
	if (scr_on)
		*p = scr_colour | (*p & 0xf0);
}

/* For the flood fill routines */

/* MH2 needs stack size > 300 */
#define STACK_SIZE 512
static unsigned int stack_ptr;
static uint16 stack[STACK_SIZE];

static INLINE void _PUSH(uint16 c) {
	assert(stack_ptr < STACK_SIZE);

	stack[stack_ptr] = c;
	stack_ptr++;
}

static INLINE uint16 _POP() {
	if (stack_ptr == 0)
		return 0xffff;

	stack_ptr--;
	return stack[stack_ptr];
}

/**
 * Draw an AGI line.
 * A line drawing routine sent by Joshua Neal, modified by Stuart George
 * (fixed >>2 to >>1 and some other bugs like x1 instead of y1, etc.)
 * @param x1  x coordinate of start point
 * @param y1  y coordinate of start point
 * @param x2  x coordinate of end point
 * @param y2  y coordinate of end point
 * @param res horizontal resolution multiplier
 */
static void draw_line(int x1, int y1, int x2, int y2, int res) {
	int i, x, y, deltaX, deltaY, stepX, stepY, errorX, errorY, detdelta;
	int width = _WIDTH * res;

	/* CM: Do clipping */
#define clip(x, y) if((x)>=(y)) (x)=(y)
	clip(x1, width - 1);
	clip(x2, width - 1);
	clip(y1, _HEIGHT - 1);
	clip(y2, _HEIGHT - 1);

	/* Vertical line */

	if (x1 == x2) {
		if (y1 > y2) {
			y = y1;
			y1 = y2;
			y2 = y;
		}

		for (; y1 <= y2; y1++) {
			put_virt_pixel(x1, y1, res);
#ifdef USE_HIRES
			if (res > 1)
				fix_pixel_bothsides(x1, y1);
#endif
		}

		return;
	}

	/* Horizontal line */

	if (y1 == y2) {
		if (x1 > x2) {
			x = x1;
			x1 = x2;
			x2 = x;
		}
#ifdef USE_HIRES
		if (res > 1)
			fix_pixel_bothsides(x1, y1);
#endif

		for (; x1 <= x2; x1++)
			put_virt_pixel(x1, y1, res);

#ifdef USE_HIRES
		if (res > 1) {
			put_virt_pixel(x1, y1, res);
			fix_pixel_bothsides(x1, y1);
		}
#endif

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

	put_virt_pixel(x, y, res);
#ifdef USE_HIRES
	if (res > 1)
		fix_pixel_bothsides(x, y);
#endif

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

		put_virt_pixel(x, y, res);
#ifdef USE_HIRES
		if (res > 1)
			fix_pixel_bothsides(x, y);
#endif
		i--;
	} while (i > 0);

#ifdef USE_HIRES
	if (res > 1) {
		put_virt_pixel(x, y, res);
		fix_pixel_bothsides(x, y);
	}
#endif
}

/**
 * Draw a relative AGI line.
 * Draws short lines relative to last position. (drawing action 0xF7)
 * @param res  horizontal resolution multiplier
 */
static void dynamic_draw_line(int res) {
	int x1, y1, disp, dx, dy;

	x1 = next_byte * res;
	y1 = next_byte;

	put_virt_pixel(x1, y1, res);

	while (42) {
		if ((disp = next_byte) >= 0xf0)
			break;

		dx = ((disp & 0xf0) >> 4) & 0x0f;
		dy = (disp & 0x0f);

		if (dx & 0x08)
			dx = -(dx & 0x07);
		if (dy & 0x08)
			dy = -(dy & 0x07);

		dx *= res;

		draw_line(x1, y1, x1 + dx, y1 + dy, res);
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
static void absolute_draw_line(int res) {
	int x1, y1, x2, y2;

	x1 = next_byte * res;
	y1 = next_byte;
	put_virt_pixel(x1, y1, res);

	while (42) {
		if ((x2 = next_byte) >= 0xf0)
			break;

		if ((y2 = next_byte) >= 0xf0)
			break;

		x2 *= res;

		draw_line(x1, y1, x2, y2, res);
		x1 = x2;
		y1 = y2;
	}
	foffs--;
}

/**************************************************************************
** okToFill
**************************************************************************/
static INLINE int is_ok_fill_here(int x, int y) {
	uint8 p;

	if (x < 0 || x >= _WIDTH || y < 0 || y >= _HEIGHT)
		return false;

	if (!scr_on && !pri_on)
		return false;

	p = game.sbuf[y * _WIDTH + x];

	if (!pri_on && scr_on && scr_colour != 15)
		return (p & 0x0f) == 15;

	if (pri_on && !scr_on && pri_colour != 4)
		return (p >> 4) == 4;

	return (scr_on && (p & 0x0f) == 15 && scr_colour != 15);
}

/**************************************************************************
** agi_fill
**************************************************************************/
static void fill_scanline(int x, int y) {
	unsigned int c;
	int newspan_up, newspan_down;

	if (!is_ok_fill_here(x, y))
		return;

	/* Scan for left border */
	for (c = x - 1; is_ok_fill_here(c, y); c--);

	newspan_up = newspan_down = 1;
	for (c++; is_ok_fill_here(c, y); c++) {
		put_virt_pixel(c, y, 1);
		if (is_ok_fill_here(c, y - 1)) {
			if (newspan_up) {
				_PUSH(c + 320 * (y - 1));
				newspan_up = 0;
			}
		} else {
			newspan_up = 1;
		}

		if (is_ok_fill_here(c, y + 1)) {
			if (newspan_down) {
				_PUSH(c + 320 * (y + 1));
				newspan_down = 0;
			}
		} else {
			newspan_down = 1;
		}
	}
}

static void agi_fill(unsigned int x, unsigned int y) {
	_PUSH(x + 320 * y);

	while (42) {
		uint16 c = _POP();

		/* Exit if stack is empty */
		if (c == 0xffff)
			break;

		x = c % 320;
		y = c / 320;

		fill_scanline(x, y);
	}

	stack_ptr = 0;
}

/**************************************************************************
** xCorner
**
** Draws an xCorner  (drawing action 0xF5)
**************************************************************************/
static void x_corner(int res) {
	int x1, x2, y1, y2;

	x1 = next_byte * res;
	y1 = next_byte;
	put_virt_pixel(x1, y1, res);

	while (42) {
		x2 = next_byte;

		if (x2 >= 0xf0)
			break;

		x2 *= res;

		draw_line(x1, y1, x2, y1, res);
		x1 = x2;
		y2 = next_byte;

		if (y2 >= 0xf0)
			break;

		draw_line(x1, y1, x1, y2, res);
		y1 = y2;
	}
	foffs--;
}

/**************************************************************************
** yCorner
**
** Draws an yCorner  (drawing action 0xF4)
**************************************************************************/
static void y_corner(int res) {
	int x1, x2, y1, y2;

	x1 = next_byte * res;
	y1 = next_byte;
	put_virt_pixel(x1, y1, res);

	while (42) {
		y2 = next_byte;

		if (y2 >= 0xF0)
			break;

		draw_line(x1, y1, x1, y2, res);
		y1 = y2;
		x2 = next_byte;

		if (x2 >= 0xf0)
			break;

		x2 *= res;

		draw_line(x1, y1, x2, y1, res);
		x1 = x2;
	}

	foffs--;
}

/**************************************************************************
** fill
**
** AGI flood fill.  (drawing action 0xF8)
**************************************************************************/
static void fill() {
	int x1, y1;

	while ((x1 = next_byte) < 0xF0 && (y1 = next_byte) < 0xf0)
		agi_fill(x1, y1);

	foffs--;
}

/**************************************************************************
** plotPattern
**
** Draws pixels, circles, squares, or splatter brush patterns depending
** on the pattern code.
**************************************************************************/

static int plot_pattern_point(int x, int y, int bitpos, int res) {
	if (pat_code & 0x20) {
		if ((splatter_map[bitpos >> 3] >> (7 - (bitpos & 7))) & 1) {
#ifdef USE_HIRES
			if (res > 1) {
				/* extra randomness in hi-res brush fill
				 */
				if (rnd->getRandomNumber(3))
					put_virt_pixel(x * 2, y, 2);
				if (!rnd->getRandomNumber(3))
					put_virt_pixel(x * 2 + 1, y, 2);
			} else
#endif
			{
				put_virt_pixel(x, y, 1);
			}
		}
		bitpos++;
		if (bitpos == 0xff)
			bitpos = 0;
	} else {
#ifdef USE_HIRES
		if (res > 1) {
			/* double width pixels make MUMG and others
			 * look nicer
			 */
			put_virt_pixel(x * 2, y, 2);
			put_virt_pixel(x * 2 + 1, y, 2);
		} else
#endif
		{
			put_virt_pixel(x, y, 1);
		}
	}

	return bitpos;
}

static void plot_pattern(int x, int y, int res) {
	int32 circlePos = 0;
	uint32 x1, y1, pensize, bitpos = splatter_start[pat_num];

	pensize = (pat_code & 7);

	if (x < (int)pensize)
		x = pensize - 1;
	if (y < (int)pensize)
		y = pensize;

	for (y1 = y - pensize; y1 <= y + pensize; y1++) {
		for (x1 = x - (pensize + 1) / 2; x1 <= x + pensize / 2; x1++) {
			if (pat_code & 0x10) {	/* Square */
				bitpos = plot_pattern_point (x1, y1, bitpos, res);
			} else {	/* Circle */
				if ((circles[pat_code & 7][circlePos >> 3] >> (7 - (circlePos & 7))) & 1) {
					bitpos = plot_pattern_point(x1, y1, bitpos, res);
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
static void plot_brush(int res) {
	int x1, y1;

	while (42) {
		if (pat_code & 0x20) {
			if ((pat_num = next_byte) >= 0xF0)
				break;
			pat_num = (pat_num >> 1) & 0x7f;
		}

		if ((x1 = next_byte) >= 0xf0)
			break;

		if ((y1 = next_byte) >= 0xf0)
			break;

		plot_pattern(x1, y1, res);
	}

	foffs--;
}

#ifdef USE_HIRES

static void fix_pixel_bothsides(int x, int y) {
	uint8 *p, *s;

	if (x >= (_WIDTH * 2) - 2)
		return;

	/* Sometimes a solid color area in the lo-res pic is made
	 * with lines, and we want to keep this  effect in the
	 * hi-res pic.
	 */
	p = &game.hires[y * (_WIDTH * 2) + x];
	if ((*(p - 2) & 0x0f) == scr_colour)
		put_virt_pixel(x - 1, y, 2);
	if ((*(p + 2) & 0x0f) == scr_colour)
		put_virt_pixel(x + 1, y, 2);

	/* If two lines are contiguous in the lo-res pic, make them
	 * contiguous in the hi-res pic. This condition is needed
	 * in some scenes like in front of Lefty's in LSL1, to draw
	 * the pole. Note: it adds artifacts in some cases.
	 */
	s = &game.sbuf[y * _WIDTH + x / 2];
	if ((*(p - 1) & 0x0f) != (*(s - 1) & 0x0f))
		put_virt_pixel(x - 1, y, 2);
}

/**************************************************************************
** okToFill
**************************************************************************/
static INLINE int hires_fill_here(int x, int y) {
	uint8 *p, *s;

	if (x < 0 || x >= _WIDTH || y < 0 || y >= _HEIGHT)
		return false;

	if (!scr_on && !pri_on)
		return false;

	p = &game.hires[(int32) y * (_WIDTH * 2) + x * 2];
	s = &game.sbuf[y * _WIDTH + x];

	if (scr_on) {
		if (scr_colour == 0x0f)
			return false;
		if ((*p & 0x0f) != 0x0f || (*(p + 1) & 0x0f) != 0x0f)
			return false;
		if ((*s & 0x0f) != scr_colour)
			return false;
	}

	if (pri_on) {
		if (pri_colour == 0x04)
			return false;
		if ((*p >> 4) != 0x04 || (*(p + 1) >> 4) != 0x04)
			return false;
		if ((*s >> 4) != pri_colour)
			return false;
	}

	return true;
}

static void fix_pixel_left(int x, int y) {
	uint8 *p;

	if (!scr_on)
		return;

	p = &game.hires[y * (_WIDTH * 2) + x * 2 + 1];
	if ((*p & 0x0f) == 0x0f)
		put_virt_pixel(2 * x + 1, y, 2);
	else if ((*p & 0x0f) == (*(p - 1) & 0x0f))
		put_virt_pixel(2 * x + 1, y, 2);
}

static void fix_pixel_right(int x, int y) {
	int idx = y * (_WIDTH * 2) + x * 2;

	if (idx >= 160 * 168)
		return;

	if (scr_on && (game.hires[idx] & 0x0f) == 0x0f)
		put_virt_pixel(2 * x, y, 2);
}

static void fix_pixel_here(int x, int y) {
	uint8 p;

	p = game.hires[y * (_WIDTH * 2) + x * 2 + 1];
	if (scr_on && (p & 0x0f) == 0x0f)
		put_virt_pixel(2 * x + 1, y, 2);
}

/**************************************************************************
** agiFill
**************************************************************************/
static void hires_fill_scanline(int x, int y) {
	unsigned int c;
	int newspan_up, newspan_down;

	if (!hires_fill_here(x, y))
		return;

	/* Scan for left border */
	for (c = x - 1; c > 0 && hires_fill_here(c, y); c--);
	fix_pixel_left(c, y);

	newspan_up = newspan_down = 1;
	for (c++; hires_fill_here(c, y); c++) {
		put_virt_pixel(c * 2, y, 2);
		fix_pixel_here(c, y);

		if (hires_fill_here(c, y - 1)) {
			if (newspan_up) {
				_PUSH(c + 320 * (y - 1));
				newspan_up = 0;
			}
		} else {
			newspan_up = 1;
		}

		if (hires_fill_here(c, y + 1)) {
			if (newspan_down) {
				_PUSH(c + 320 * (y + 1));
				newspan_down = 0;
			}
		} else {
			newspan_down = 1;
		}
	}

	fix_pixel_right(c, y);
}

static void _hires_fill(unsigned int x, unsigned int y) {
	_PUSH(x + 320 * y);

	while (42) {
		uint16 c = _POP();

		/* Exit if stack is empty */
		if (c == 0xffff)
			break;

		x = c % 320;
		y = c / 320;

		hires_fill_scanline(x, y);
	}

	stack_ptr = 0;
}

/**************************************************************************
** fill
**
** AGI flood fill.  (drawing action 0xF8)
**************************************************************************/
static void hires_fill() {
	int x1, y1;

	while ((x1 = next_byte) < 0xf0 && (y1 = next_byte) < 0xf0) {
		_hires_fill(x1, y1);
	}

	foffs--;
}

/**
 * Show AGI picture.
 * This function copies a ``hidden'' AGI picture to the output device.
 */
void show_hires_pic() {
	int y, offset;
	int32 i;

	i = 0;
	offset = game.line_min_print * CHAR_LINES;
	for (y = 0; y < _HEIGHT; y++) {
		put_pixels_hires(0, y + offset, _WIDTH * 2, &game.hires[i]);
		i += _WIDTH * 2;
	}

	flush_screen();
}

void fix_hires_picture() {
	uint8 *p, *b;
	int i;

	p = game.hires;
	b = game.sbuf;

	for (i = 0; p < &game.hires[_WIDTH * _HEIGHT * 2] - 1; p++, i++) {
		if ((*p & 0x0f) == 0x0f && (*b & 0x0f) != 0x0f) {
			if ((*(p + 1) & 0x0f) != 0x0f)
				*p = *(p + 1);
			else
				*p = *b;
		}
		if ((*p >> 4) == 4 && (*b >> 4) != 4 && (*(b + 1) >> 4) != 4) {
			*p = (*p & 0x0f) | (*b & 0xf0);
		}
		b += (i & 1);
	}
}

#endif				/* USE_HIRES */

static void draw_picture() {
	uint8 act;
	int drawing;
#ifdef USE_HIRES
	int save_foffs;
#endif

	pat_code = 0;
	pat_num = 0;
	pri_on = scr_on = false;
	scr_colour = 0xf;
	pri_colour = 0x4;

	drawing = 1;

	debugC(8, kDebugLevelMain, "Drawing picture");
	for (drawing = 1; drawing && foffs < flen;) {

#ifdef USE_HIRES
		save_foffs = foffs;
#endif

		act = next_byte;
		switch (act) {
		case 0xf0:	/* set colour on screen */
			scr_colour = next_byte;
			scr_colour &= 0xF;	/* for v3 drawing diff */
			scr_on = true;
			break;
		case 0xf1:	/* disable screen drawing */
			scr_on = false;
			break;
		case 0xf2:	/* set colour on priority */
			pri_colour = next_byte;
			pri_colour &= 0xf;	/* for v3 drawing diff */
			pri_on = true;
			break;
		case 0xf3:	/* disable priority screen */
			pri_on = false;
			break;
		case 0xf4:	/* y-corner */
			y_corner(1);
			break;
		case 0xf5:	/* x-corner */
			x_corner(1);
			break;
		case 0xf6:	/* absolute draw lines */
			absolute_draw_line(1);
			break;
		case 0xf7:	/* dynamic draw lines */
			dynamic_draw_line(1);
			break;
		case 0xf8:	/* fill */
			fill();
			break;
		case 0xf9:	/* set pattern */
			pat_code = next_byte;
			break;
		case 0xfA:	/* plot brush */
			plot_brush(1);
			break;
		case 0xFF:	/* end of pic data */
		default:
			drawing = 0;
			break;
		}

#ifdef USE_HIRES
		foffs = save_foffs;

		act = next_byte;
		switch (act) {
		case 0xf0:	/* set colour on screen */
			scr_colour = next_byte;
			scr_colour &= 0xF;	/* for v3 drawing diff */
			scr_on = true;
			break;
		case 0xf1:	/* disable screen drawing */
			scr_on = false;
			break;
		case 0xf2:	/* set colour on priority */
			pri_colour = next_byte;
			pri_colour &= 0xf;	/* for v3 drawing diff */
			pri_on = true;
			break;
		case 0xf3:	/* disable priority screen */
			pri_on = false;
			break;
		case 0xf4:	/* y-corner */
			y_corner(2);
			break;
		case 0xf5:	/* x-corner */
			x_corner(2);
			break;
		case 0xf6:	/* absolute draw lines */
			absolute_draw_line(2);
			break;
		case 0xf7:	/* dynamic draw lines */
			dynamic_draw_line(2);
			break;
		case 0xf8:	/* fill */
			hires_fill();
			break;
		case 0xf9:	/* set pattern */
			pat_code = next_byte;
			break;
		case 0xfA:	/* plot brush */
			plot_brush(2);
			break;
		case 0xFF:	/* end of pic data */
		default:
			drawing = 0;
			break;
		}
#endif
	}
}

/*
 * Public functions
 */

/**
 *
 */
uint8 *convert_v3_pic(uint8 *data, uint32 len) {
	uint8 d, old = 0, x, *in, *xdata, *out, mode = 0;
	uint32 i, ulen;

	xdata = (uint8 *) malloc(len + len / 2);

	out = xdata;
	in = data;

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

	free(data);
	xdata = (uint8 *)realloc(xdata, ulen);

	return xdata;
}

/**
 * Decode an AGI picture resource.
 * This function decodes an AGI picture resource into the correct slot
 * and draws it on the AGI screen, optionally cleaning the screen before
 * drawing.
 * @param n      AGI picture resource number
 * @param clear  clear AGI screen before drawing
 */
int decode_picture(int n, int clear) {
	debugC(8, kDebugLevelResources, "(%d)", n);

	pat_code = 0;
	pat_num = 0;
	pri_on = scr_on = false;
	scr_colour = 0xF;
	pri_colour = 0x4;

	data = game.pictures[n].rdata;
	flen = game.dir_pic[n].len;
	foffs = 0;

	if (clear) {
		memset(game.sbuf, 0x4f, _WIDTH * _HEIGHT);
#ifdef USE_HIRES
		memset(game.hires, 0x4f, _WIDTH * 2 * _HEIGHT);
#endif
	}

	draw_picture();

#ifdef USE_HIRES
	fix_hires_picture();
#endif

	if (clear)
		clear_image_stack();
	record_image_stack_call(ADD_PIC, n, clear, 0, 0, 0, 0, 0);

	return err_OK;
}

/**
 * Unload an AGI picture resource.
 * This function unloads an AGI picture resource and deallocates
 * resource data.
 * @param n AGI picture resource number
 */
int unload_picture(int n) {
	/* remove visual buffer & priority buffer if they exist */
	if (game.dir_pic[n].flags & RES_LOADED) {
		free(game.pictures[n].rdata);
		game.dir_pic[n].flags &= ~RES_LOADED;
	}

	return err_OK;
}

/**
 * Show AGI picture.
 * This function copies a ``hidden'' AGI picture to the output device.
 */
void show_pic() {
	int i, y;
	int offset;

	debugC(8, kDebugLevelMain, "Show picture!");
#ifdef USE_HIRES
	if (opt.hires) {
		show_hires_pic();
		return;
	}
#endif

	i = 0;
	offset = game.line_min_print * CHAR_LINES;
	for (y = 0; y < _HEIGHT; y++) {
		put_pixels_a(0, y + offset, _WIDTH, &game.sbuf[i]);
		i += _WIDTH;
	}

	flush_screen();
}

}                             // End of namespace Agi
