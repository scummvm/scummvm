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

#include "common/system.h"

#include "chamber/chamber.h"
#include "chamber/common.h"
#include "chamber/cursor.h"
#include "chamber/resdata.h"
#include "chamber/cga.h"
#include "chamber/ega.h"
#include "chamber/amiga.h"
#include "chamber/renderer.h"
#include "graphics/cursorman.h"
#include "graphics/palette.h"


namespace Chamber {


byte cursor_color = 0;

byte *cursor_shape = NULL;
byte cursor_anim_ticks;
byte cursor_anim_phase;

/*cursors hotspot offsets*/
uint16 cursor_shifts[CURSOR_MAX][2] = {
	{ 0, 0 },
	{ 7, 7 },
	{ 7, 7 },
	{ 0, 0 },
	{ 7, 7 },
	{ 0, 15 },
	{ 7, 7 },
	{ 7, 7 },
	{ 7, 7 }
};

uint16 cursor_x_shift;
byte cursor_y_shift;

uint16 cursor_x;
byte cursor_y;
byte cursor_backup[CURSOR_WIDTH_SPR * CURSOR_HEIGHT / CGA_BITS_PER_PIXEL];
uint16 last_cursor_draw_ofs = 0;
uint16 cursor_draw_ofs;
byte cursorImage[CURSOR_WIDTH * CURSOR_HEIGHT];

/*
Select cursor shape and its hotspot
*/
void CGARenderer::selectCursor(uint16 num) {
	cursor_x_shift = cursor_shifts[num][0];
	cursor_y_shift = cursor_shifts[num][1];

	byte *dst = cursorImage;

	cursor_shape = souri_data + num * CURSOR_WIDTH * CURSOR_HEIGHT * 2 / 4;
	byte *src = cursor_shape;
	for (int16 y = 0; y < CURSOR_HEIGHT; y++) {
		for (int16 x = 0; x < CURSOR_HEIGHT / 4; x++) {
			byte colors = *src;
			byte masks = *(src++ + CURSOR_HEIGHT * CURSOR_WIDTH / 4);

			for (int16 c = 0; c < 4; c++) {
				byte color = (colors & 0xC0) >> 6;
				byte mask = (masks & 0xC0) >> 6;
				colors <<= 2;
				masks <<= 2;

				if (!mask)
					*dst++ = color;
				else {
					*dst++ = 255;
				}
			}
		}
	}

	CursorMan.replaceCursor(cursorImage, CURSOR_WIDTH, CURSOR_HEIGHT, cursor_x_shift, cursor_y_shift, 255);
	CursorMan.showMouse(true);
}

void EGARenderer::selectCursor(uint16 num) {
	cursor_x_shift = cursor_shifts[num][0];
	cursor_y_shift = cursor_shifts[num][1];

	byte *dst = cursorImage;

	/*EGA SOURI.EGA: 64 bytes/cursor, 4 bytes per row.
	  Each row = two little-endian 16-bit planes:
	    planeA (bytes 0,1): black mask  (bit set = black pixel)
	    planeB (bytes 2,3): white mask  (bit set = white pixel)
	  A=0,B=0 -> transparent; A=1,B=0 -> black; B=1 -> white.*/
	cursor_shape = souri_data + num * (CURSOR_WIDTH * CURSOR_HEIGHT / 4);
	byte *src = cursor_shape;
	for (int16 y = 0; y < CURSOR_HEIGHT; y++) {
		uint16 planeA = (uint16)src[0] | ((uint16)src[1] << 8);
		uint16 planeB = (uint16)src[2] | ((uint16)src[3] << 8);
		src += 4;
		for (int16 x = 0; x < CURSOR_WIDTH; x++) {
			byte bitA = (planeA >> (CURSOR_WIDTH - 1 - x)) & 1;
			byte bitB = (planeB >> (CURSOR_WIDTH - 1 - x)) & 1;
			if (!bitA && !bitB)
				*dst++ = 255; /*transparent*/
			else if (bitB)
				*dst++ = (cursor_color == 0xAA) ? 14 : 15;  /*yellow on hotspot, white otherwise*/
			else
				*dst++ = 0;   /*black*/
		}
	}

	CursorMan.replaceCursor(cursorImage, CURSOR_WIDTH, CURSOR_HEIGHT, cursor_x_shift, cursor_y_shift, 255);
	// TODO: Replace use of cursor palettes
	CursorMan.replaceCursorPalette(Graphics::Palette::createEGAPalette().data(), 0, 16);
	CursorMan.showMouse(true);
}

void AmigaRenderer::selectCursor(uint16 num) {
	cursor_x_shift = cursor_shifts[num][0];
	cursor_y_shift = cursor_shifts[num][1];

	byte *dst = cursorImage;

	// 72-byte hardware sprite: skip the 4-byte header, then 16 rows of two BE words
	// plane0 is the outline, plane1 the body
	const int kAmigaCursorStride = 72;
	const int kAmigaCursorHeader = 4;
	cursor_shape = souri_data + num * kAmigaCursorStride + kAmigaCursorHeader;
	byte *src = cursor_shape;
	// white reticle, red on a hotspot; the rest use yellow or white
	bool reticle = (num == CURSOR_TARGET || num == CURSOR_CROSSHAIR);
	byte mainColor = 0;                                                      /*plane0 only: black edges*/
	byte bodyColor = reticle ? ((cursor_color == 0xAA) ? 12 : 15)
	                         : ((cursor_color == 0xAA) ? 14 : 15);           /*plane0+plane1: interior*/
	for (int16 y = 0; y < CURSOR_HEIGHT; y++) {
		uint16 plane0 = ((uint16)src[0] << 8) | (uint16)src[1];
		uint16 plane1 = ((uint16)src[2] << 8) | (uint16)src[3];
		src += 4;
		for (int16 x = 0; x < CURSOR_WIDTH; x++) {
			byte bit0 = (plane0 >> (CURSOR_WIDTH - 1 - x)) & 1;
			byte bit1 = (plane1 >> (CURSOR_WIDTH - 1 - x)) & 1;
			if (!bit0 && !bit1)
				*dst++ = 255; /*transparent*/
			else if (bit1)
				*dst++ = bodyColor;
			else
				*dst++ = mainColor;
		}
	}

	CursorMan.replaceCursor(cursorImage, CURSOR_WIDTH, CURSOR_HEIGHT, cursor_x_shift, cursor_y_shift, 255);
	// Fixed palette so the cursor stays visible: 0 black, 12 red, 14 yellow, 15 white
	static const byte cursorPal[16 * 3] = {
		0, 0, 0,        0, 0, 0,  0, 0, 0,  0, 0, 0,
		0, 0, 0,        0, 0, 0,  0, 0, 0,  0, 0, 0,
		0, 0, 0,        0, 0, 0,  0, 0, 0,  0, 0, 0,
		238, 17, 68,    0, 0, 0,  255, 255, 0, 255, 255, 255
	};
	CursorMan.replaceCursorPalette(cursorPal, 0, 16);
	CursorMan.showMouse(true);
}

/*
Build cursor sprite for its current pixel-grained position
*/
void updateCursor(void) {
}

/*
Draw cursor sprite and backup background pixels
*/
void drawCursor(byte *target) {
	g_system->updateScreen();
}

/*
Restore background pixels under cursor
*/
void undrawCursor(byte *target) {
}

/*
Hide the system mouse pointer (used during non-interactive room transitions)
*/
void hideMouseCursor(void) {
	CursorMan.showMouse(false);
}

/*
Restore pixels under cursor and update cursor sprite
*/
void updateUndrawCursor(byte *target) {
	/*TODO: does this call order makes any sense?*/
	updateCursor();
	undrawCursor(target);
}

} // End of namespace Chamber
