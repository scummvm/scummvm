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

#include "ags/lib/allegro/base.h"
#include "ags/lib/allegro/gfx.h"
#include "ags/lib/allegro/surface.h"

#ifndef AGS_LIB_ALLEGRO_DRAW_H
#define AGS_LIB_ALLEGRO_DRAW_H

namespace AGS3 {

#define DRAW_MODE_SOLID             0        /* flags for drawing_mode() */
#define DRAW_MODE_XOR               1
#define DRAW_MODE_COPY_PATTERN      2
#define DRAW_MODE_SOLID_PATTERN     3
#define DRAW_MODE_MASKED_PATTERN    4
#define DRAW_MODE_TRANS             5

AL_FUNC(void, drawing_mode, (int mode, BITMAP *pattern, int x_anchor, int y_anchor));
AL_FUNC(void, xor_mode, (int on));
AL_FUNC(void, solid_mode, (void));

typedef void (*DrawMethod)(BITMAP *, int, int, int);

extern void do_line(BITMAP *bmp, int x1, int y1, int x2, int y2, int d, DrawMethod proc);

} // namespace AGS3

#endif
