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
void selectCursor(uint16 num) {
	cursor_x_shift = cursor_shifts[num][0];
	cursor_y_shift = cursor_shifts[num][1];
	cursor_shape = souri_data + num * CURSOR_WIDTH * CURSOR_HEIGHT * 2 / CGA_PIXELS_PER_BYTE;

	byte *src = cursor_shape;
	byte *dst = cursorImage;
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

	g_system->setMouseCursor(cursorImage, CURSOR_WIDTH, CURSOR_HEIGHT, cursor_x_shift, cursor_y_shift, 255);
	g_system->showMouse(true);
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
Restore pixels under cursor and update cursor sprite
*/
void updateUndrawCursor(byte *target) {
	/*TODO: does this call order makes any sense?*/
	updateCursor();
	undrawCursor(target);
}

} // End of namespace Chamber
