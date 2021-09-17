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

#include "chamber/chamber.h"
#include "chamber/common.h"
#include "chamber/cursor.h"
#include "chamber/resdata.h"
#include "chamber/cga.h"


namespace Chamber {


unsigned char cursor_color = 0;

unsigned char *cursor_shape = NULL;
unsigned char cursor_anim_ticks;
unsigned char cursor_anim_phase;

/*cursors hotspot offsets*/
unsigned int cursor_shifts[CURSOR_MAX][2] = {
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

unsigned int cursor_x_shift;
unsigned char cursor_y_shift;

unsigned int cursor_x;
unsigned char cursor_y;
unsigned char cursor_backup[CURSOR_WIDTH_SPR * CURSOR_HEIGHT / CGA_BITS_PER_PIXEL];
unsigned int last_cursor_draw_ofs = 0;
unsigned int cursor_draw_ofs;

/*
Select cursor shape and its hotspot
*/
void SelectCursor(unsigned int num) {
	cursor_x_shift = cursor_shifts[num][0];
	cursor_y_shift = cursor_shifts[num][1];
	cursor_shape = souri_data + num * CURSOR_WIDTH * CURSOR_HEIGHT * 2 / CGA_PIXELS_PER_BYTE;
}

/*
Build cursor sprite for its current pixel-grained position
*/
void UpdateCursor(void) {
	if (!cursor_shape)
		return;

	unsigned char *cursor, *sprite, *spr;
	unsigned char cursor_bit_shift;
	unsigned int x, y;
	x = cursor_x - cursor_x_shift;
	if ((signed int)x < 0) x = 0;
	y = cursor_y - cursor_y_shift;
	if ((signed int)y < 0) y = 0;

	cursor_bit_shift = (x % 4) * 2;
	cursor_draw_ofs = CGA_CalcXY_p(x / 4, y);

	cursor = cursor_shape;
	sprite = sprit_load_buffer;

	if (cursor_bit_shift == 0) {
		/*pixels*/
		spr = sprite;
		for (y = 0; y < CURSOR_HEIGHT; y++) {
			for (x = 0; x < CURSOR_WIDTH / 4; x++) {
				unsigned char p = *cursor++;
				spr[x * 2] = p;
			}
			spr[x * 2] = 0;
			spr += 5 * 2;
		}

		/*mask*/
		spr = sprite + 1;
		for (y = 0; y < CURSOR_HEIGHT; y++) {
			for (x = 0; x < CURSOR_WIDTH / 4; x++) {
				unsigned char p = *cursor++;
				spr[x * 2] = p;
			}
			spr[x * 2] = 0xFF;
			spr += 5 * 2;
		}
	} else {
		spr = sprite;
		for (y = 0; y < CURSOR_HEIGHT; y++) {
			unsigned char i;
			unsigned char p0 = *cursor++;
			unsigned char p1 = *cursor++;
			unsigned char p2 = *cursor++;
			unsigned char p3 = *cursor++;
			unsigned char p4 = 0;
			for (i = 0; i < cursor_bit_shift; i++) {
				p4 = (p4 >> 1) | (p3 << 7);
				p3 = (p3 >> 1) | (p2 << 7);
				p2 = (p2 >> 1) | (p1 << 7);
				p1 = (p1 >> 1) | (p0 << 7);
				p0 = (p0 >> 1) | (0 << 7);
			}
			spr[0] = p0;
			spr[2] = p1;
			spr[4] = p2;
			spr[6] = p3;
			spr[8] = p4;

			spr += 5 * 2;
		}

		spr = sprite + 1;
		for (y = 0; y < CURSOR_HEIGHT; y++) {
			unsigned char i;
			unsigned char p0 = *cursor++;
			unsigned char p1 = *cursor++;
			unsigned char p2 = *cursor++;
			unsigned char p3 = *cursor++;
			unsigned char p4 = 0xFF;
			for (i = 0; i < cursor_bit_shift; i++) {
				p4 = (p4 >> 1) | (p3 << 7);
				p3 = (p3 >> 1) | (p2 << 7);
				p2 = (p2 >> 1) | (p1 << 7);
				p1 = (p1 >> 1) | (p0 << 7);
				p0 = (p0 >> 1) | (1 << 7);
			}
			spr[0] = p0;
			spr[2] = p1;
			spr[4] = p2;
			spr[6] = p3;
			spr[8] = p4;
			spr += 5 * 2;
		}
	}
}

/*
Draw cursor sprite and backup background pixels
*/
void DrawCursor(unsigned char *target) {
	last_cursor_draw_ofs = cursor_draw_ofs;
	CGA_BlitSpriteBak(sprit_load_buffer, CURSOR_WIDTH_SPR / 4, CURSOR_WIDTH_SPR / 4, CURSOR_HEIGHT, target, cursor_draw_ofs, cursor_backup, cursor_color);
}

/*
Restore background pixels under cursor
*/
void UndrawCursor(unsigned char *target) {
	CGA_Blit(cursor_backup, CURSOR_WIDTH_SPR / 4, CURSOR_WIDTH_SPR / 4, CURSOR_HEIGHT, target, last_cursor_draw_ofs);
}

/*
Restore pixels under cursor and update cursor sprite
*/
void UpdateUndrawCursor(unsigned char *target) {
	/*TODO: does this call order makes any sense?*/
	UpdateCursor();
	UndrawCursor(target);
}

} // End of namespace Chamber
