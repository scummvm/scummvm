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
#include "got/gfx/palette.h"
#include "got/utils/file.h"
#include "got/vars.h"

namespace Got {

void load_palette() {
	byte buff[256 * 3];

	if (res_read("PALETTE", buff) < 0)
		error("Cannot Read PALETTE");

	g_system->getPaletteManager()->setPalette(buff, 0, 256);

	set_screen_pal();
}

void set_screen_pal() {
	byte pal[3];

	xgetpal(pal, 1, _G(scrn).pal_colors[0]);
	xsetpal(251, pal[0], pal[1], pal[2]);
	xgetpal(pal, 1, _G(scrn).pal_colors[1]);
	xsetpal(252, pal[0], pal[1], pal[2]);
	xgetpal(pal, 1, _G(scrn).pal_colors[2]);
	xsetpal(253, pal[0], pal[1], pal[2]);
}

void xsetpal(byte color, byte R, byte G, byte B) {
	byte rgb[3] = { R, G, B };
	g_system->getPaletteManager()->setPalette(rgb, color, 1);
}

void xgetpal(byte *pal, int num_colrs, int start_index) {
	g_system->getPaletteManager()->grabPalette(pal, start_index, num_colrs);
}

} // namespace Got
