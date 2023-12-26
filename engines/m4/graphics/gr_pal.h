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

#ifndef M4_GRAPHICS_GR_PAL_H
#define M4_GRAPHICS_GR_PAL_H

#include "m4/m4_types.h"

namespace M4 {

constexpr int MIN_PAL_ENTRY = 1;
constexpr int MAX_PAL_ENTRY = 255;

#define __BLACK	    (gr_pal_get_ega_color(0))
#define __BLUE		(gr_pal_get_ega_color(1))
#define __GREEN		(gr_pal_get_ega_color(2))
#define __CYAN		(gr_pal_get_ega_color(3))
#define __RED		(gr_pal_get_ega_color(4))
#define __VIOLET	(gr_pal_get_ega_color(5))
#define __BROWN		(gr_pal_get_ega_color(6))
#define __LTGRAY	(gr_pal_get_ega_color(7))
#define __DKGRAY	(gr_pal_get_ega_color(8))
#define __LTBLUE	(gr_pal_get_ega_color(9))
#define __LTGREEN	(gr_pal_get_ega_color(10))
#define __LTCYAN	(gr_pal_get_ega_color(11))
#define __LTRED		(gr_pal_get_ega_color(12))
#define __PINK		(gr_pal_get_ega_color(13))
#define __YELLOW	(gr_pal_get_ega_color(14))
#define __WHITE	    (gr_pal_get_ega_color(15))

uint8 gr_pal_get_ega_color(uint8 myColor);

void gr_color_create_ipl5(uint8 *inverseColorTable, char *fname, int room_num);
uint8 *gr_color_load_ipl5(const char *filename, uint8 *inverseColors);
void gr_color_set(int32 c);
byte gr_color_get_current();

void gr_pal_set_range(RGB8 *pal, int first_color, int num_colors);
void gr_pal_set_range(int first_color, int num_colors);
void gr_pal_set(RGB8 *pal);
void gr_pal_set_RGB8(RGB8 *entry, int r, int g, int b);
void gr_pal_set_entry(int32 index, RGB8 *entry);
void gr_pal_clear(RGB8 *palette);
void gr_pal_clear_range(RGB8 *palette, int first_color, int last_color);
uint8 gr_pal_find_best_match(RGB8 *pal, uint8 r, uint8 g, uint8 b);
void gr_pal_interface(RGB8 *fixpal);
void gr_pal_reset_ega_colors(RGB8 *pal);
void gr_backup_palette();
void gr_restore_palette();

void pal_mirror_colours(int first_color, int last_color, RGB8 *pal);
void pal_mirror_colours(int first_color, int last_color);

} // namespace M4

#endif
