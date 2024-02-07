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
#include "graphics/paletteman.h"
#include "common/textconsole.h"
#include "m4/graphics/gr_pal.h"
#include "m4/core/errors.h"
#include "m4/vars.h"

namespace M4 {

byte EGAcolors[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

uint8 gr_pal_get_ega_color(uint8 myColor) {
	return EGAcolors[myColor];
}


uint8 *gr_color_createInverseTable(RGB8 *pal, uint8 bitDepth, int begin_color, int end_color) {
	error("gr_color_createInverseTable is not implemented in ScummVM");
}

void gr_color_create_ipl5(uint8 *inverseColorTable, char *fname, int room_num) {
	error("gr_color_create_ipl5 is not implemented in ScummVM");
}

uint8 *gr_color_load_ipl5(const char *filename, uint8 *inverseColors) {
	error("gr_color_load_ipl5 is not implemented in ScummVM");
}

void gr_color_set(int32 c) {
	_G(color) = c;
}

byte gr_color_get_current() {
	return _G(color);
}

void gr_pal_clear(RGB8 *palette) {
	int i;

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

void gr_pal_set_RGB8(RGB8 *entry, int r, int g, int b) {
	entry->r = (byte)r;
	entry->g = (byte)g;
	entry->b = (byte)b;
}

void gr_pal_set_range(RGB8 *pal, int first_color, int num_colors) {
	g_system->getPaletteManager()->setPalette((const byte *)pal + first_color * 3,
		first_color, num_colors);
}

void gr_pal_set_range(int first_color, int num_colors) {
	gr_pal_set_range(_G(master_palette), first_color, num_colors);
}

void  gr_pal_set_entry(int32 index, RGB8 *entry) {
	g_system->getPaletteManager()->setPalette((const byte *)entry, index, 1);
}

void gr_pal_clear_range(RGB8 *palette, int first_color, int last_color) {
	for (int i = first_color; i <= last_color; i++) {
		palette[i].r = 0;
		palette[i].g = 0;
		palette[i].b = 0;
	}

	gr_pal_set_range(palette, first_color, last_color - first_color);
}

uint8 gr_pal_find_best_match(RGB8 *pal, uint8 r, uint8 g, uint8 b) {
	int i, index = 0, Rdiff, Gdiff, Bdiff;
	uint32 minDist = 0x7fffffff;

	for (i = 0; i < 256; ++i) {
		Rdiff = r - pal[i].r;
		Gdiff = g - pal[i].g;
		Bdiff = b - pal[i].b;
		if (Rdiff * Rdiff + Gdiff * Gdiff + Bdiff * Bdiff < (int)minDist) {
			minDist = Rdiff * Rdiff + Gdiff * Gdiff + Bdiff * Bdiff;
			index = i;
		}
	}

	return (uint8)index;
}

void gr_pal_interface(RGB8 *fixpal) {
	if (_GI().set_interface_palette(fixpal))
		return;

	// Low intesity
	gr_pal_set_RGB8(&fixpal[0], 0, 0, 0);		// r0 g0 b0		black
	gr_pal_set_RGB8(&fixpal[1], 0, 0, 168);		// r0 g0 b2		blue
	gr_pal_set_RGB8(&fixpal[2], 0, 168, 0);		// r0 g2 b0		green
	gr_pal_set_RGB8(&fixpal[3], 0, 168, 168);	// r0 g2 b2		cyan

	gr_pal_set_RGB8(&fixpal[4], 168, 0, 0);		// r2 g0 b0		red
	gr_pal_set_RGB8(&fixpal[5], 168, 0, 168);	// r2 g0 b2		violet
	gr_pal_set_RGB8(&fixpal[6], 168, 92, 0);	// r2 g1 b0		brown
	gr_pal_set_RGB8(&fixpal[7], 168, 168, 168);	// r2 g2 b2		light grey

	// high intensity
	gr_pal_set_RGB8(&fixpal[8], 92, 92, 92);	// r1 g1 b1		dark grey
	gr_pal_set_RGB8(&fixpal[9], 92, 92, 255);	// r1 g1 b2		light blue
	gr_pal_set_RGB8(&fixpal[10], 92, 255, 92);	// r1 g2 b1		light green
	gr_pal_set_RGB8(&fixpal[11], 92, 255, 255); // r1 g2 b2		light cyan

	gr_pal_set_RGB8(&fixpal[12], 255, 92, 92);	// r2 g1 b1		light red
	gr_pal_set_RGB8(&fixpal[13], 255, 92, 255);	// r2 g1 b2		pink
	gr_pal_set_RGB8(&fixpal[14], 255, 255, 23);	// r2 g2 b1		yellow
	gr_pal_set_RGB8(&fixpal[15], 255, 255, 255);// r1 g1 b1		white
}

void gr_pal_reset_ega_colors(RGB8 *pal) {
	EGAcolors[0] = gr_pal_find_best_match(pal, 0, 0, 0);		//__BLACK
	EGAcolors[1] = gr_pal_find_best_match(pal, 0, 0, 255);		//__BLUE
	EGAcolors[2] = gr_pal_find_best_match(pal, 0, 255, 0);		//__GREEN
	EGAcolors[3] = gr_pal_find_best_match(pal, 0, 255, 255);	//__CYAN
	EGAcolors[4] = gr_pal_find_best_match(pal, 255, 0, 0);		//__RED
	EGAcolors[5] = gr_pal_find_best_match(pal, 255, 0, 255);	//__VIOLET
	EGAcolors[6] = gr_pal_find_best_match(pal, 168, 84, 84);	//__BROWN
	EGAcolors[7] = gr_pal_find_best_match(pal, 168, 168, 168);	//__LTGRAY
	EGAcolors[8] = gr_pal_find_best_match(pal, 84, 84, 84);		//__DKGRAY
	EGAcolors[9] = gr_pal_find_best_match(pal, 0, 0, 127);		//__LTBLUE
	EGAcolors[10] = gr_pal_find_best_match(pal, 0, 127, 0);		//__LTGREEN
	EGAcolors[11] = gr_pal_find_best_match(pal, 0, 127, 127);	//__LTCYAN
	EGAcolors[12] = gr_pal_find_best_match(pal, 84, 0, 0);		//__LTRED
	EGAcolors[13] = gr_pal_find_best_match(pal, 84, 0, 0);		//__PINK
	EGAcolors[14] = gr_pal_find_best_match(pal, 0, 84, 84);		//__YELLOW
	EGAcolors[15] = gr_pal_find_best_match(pal, 255, 255, 255);	//__WHITE
}

void gr_backup_palette() {
	Common::copy(_G(master_palette), _G(master_palette) + 256, _G(backup_palette));
}

void gr_restore_palette() {
	Common::copy(_G(backup_palette), _G(backup_palette) + 256, _G(master_palette));
}

void pal_mirror_colours(int first_color, int last_color, RGB8 *pal) {
	if (first_color < 0 || last_color > 255 || first_color > last_color)
		error_show(FL, 'Burg', "pal_mirror_colours index error");

	int num_colors = last_color - first_color + 1;
	for (int index = 0; index < num_colors; ++index) {
		RGB8 *destP = pal + (last_color + num_colors - index);
		RGB8 *srcP = pal + (first_color + index);
		*destP = *srcP;
	}
}

void pal_mirror_colours(int first_color, int last_color) {
	pal_mirror_colours(first_color, last_color, _G(master_palette));
}

} // namespace M4
