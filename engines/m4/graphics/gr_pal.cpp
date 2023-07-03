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
#include "graphics/palette.h"
#include "common/textconsole.h"
#include "m4/graphics/gr_pal.h"
#include "m4/vars.h"

namespace M4 {

static const uint8 EGAcolors[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

constexpr uint8 gr_pal_get_ega_color(uint8 myColor) {
	return EGAcolors[myColor];
}


uint8 *gr_color_createInverseTable(RGB8 *pal, uint8 bitDepth, int begin_color, int end_color) {
	error("TODO: gr_color_createInverseTable");
}

void gr_color_create_ipl5(uint8 *inverseColorTable, char *fname, int room_num) {
	error("TODO: gr_color_create_ipl5");
}

uint8 *gr_color_load_ipl5(const char *filename, uint8 *inverseColors) {
	error("TODO: gr_color_load_ipl5");
}

void gr_color_set(int32 c) {
	_G(color) = c;
}

byte gr_color_get_current() {
	return _G(color);
}

void gr_pal_clear(RGB8 *palette) {
	int i;
	RGB8 x = { 0, 0, 0 };

	for (i = 0; i < 256; i++) {
		palette[i].r = 0;
		palette[i].g = 0;
		palette[i].b = 0;
	}

	gr_pal_set(palette);
}

void gr_pal_set(RGB8 *pal) {
	gr_pal_set_range(pal, 0, 256);
}

void gr_pal_set_range(RGB8 *pal, int first_color, int num_colors) {
	g_system->getPaletteManager()->setPalette((const byte *)pal, first_color, num_colors);
}

void  gr_pal_set_entry(int32 index, RGB8 *entry) {
	g_system->getPaletteManager()->setPalette((const byte *)entry, index, 1);
}

} // namespace M4
