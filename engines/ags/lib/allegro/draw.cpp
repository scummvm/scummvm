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

#include "ags/lib/allegro/draw.h"

namespace AGS3 {

void drawing_mode(int mode, BITMAP *pattern, int x_anchor, int y_anchor) {
	// TODO: Drawing mode
}

void xor_mode(int on) {
	drawing_mode(on ? DRAW_MODE_XOR : DRAW_MODE_SOLID, NULL, 0, 0);
}

void solid_mode(void) {
	drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
}

void do_line(BITMAP *bmp, int x1, int y1, int x2, int y2, int d, DrawMethod proc) {
	int dx = x2 - x1;
	int dy = y2 - y1;
	int i1, i2;
	int x, y;
	int dd;

	/* worker macro */
#define DO_LINE(pri_sign, pri_c, pri_cond, sec_sign, sec_c, sec_cond)     \
	{                                                                         \
		if (d##pri_c == 0) {                                                  \
			proc(bmp, x1, y1, d);                                             \
			return;                                                           \
		}                                                                     \
		\
		i1 = 2 * d##sec_c;                                                    \
		dd = i1 - (sec_sign (pri_sign d##pri_c));                             \
		i2 = dd - (sec_sign (pri_sign d##pri_c));                             \
		\
		x = x1;                                                               \
		y = y1;                                                               \
		\
		while (pri_c pri_cond pri_c##2) {                                     \
			proc(bmp, x, y, d);                                               \
			\
			if (dd sec_cond 0) {                                              \
				sec_c = sec_c sec_sign 1;                                     \
				dd += i2;                                                     \
			}                                                                 \
			else                                                              \
				dd += i1;                                                     \
			\
			pri_c = pri_c pri_sign 1;                                         \
		}                                                                     \
	}

	if (dx >= 0) {
		if (dy >= 0) {
			if (dx >= dy) {
				/* (x1 <= x2) && (y1 <= y2) && (dx >= dy) */
				DO_LINE(+, x, <= , +, y, >=);
			} else {
				/* (x1 <= x2) && (y1 <= y2) && (dx < dy) */
				DO_LINE(+, y, <= , +, x, >=);
			}
		} else {
			if (dx >= -dy) {
				/* (x1 <= x2) && (y1 > y2) && (dx >= dy) */
				DO_LINE(+, x, <= , -, y, <=);
			} else {
				/* (x1 <= x2) && (y1 > y2) && (dx < dy) */
				DO_LINE(-, y, >= , +, x, >=);
			}
		}
	} else {
		if (dy >= 0) {
			if (-dx >= dy) {
				/* (x1 > x2) && (y1 <= y2) && (dx >= dy) */
				DO_LINE(-, x, >= , +, y, >=);
			} else {
				/* (x1 > x2) && (y1 <= y2) && (dx < dy) */
				DO_LINE(+, y, <= , -, x, <=);
			}
		} else {
			if (-dx >= -dy) {
				/* (x1 > x2) && (y1 > y2) && (dx >= dy) */
				DO_LINE(-, x, >= , -, y, <=);
			} else {
				/* (x1 > x2) && (y1 > y2) && (dx < dy) */
				DO_LINE(-, y, >= , -, x, <=);
			}
		}
	}

#undef DO_LINE
}

} // namespace AGS3
