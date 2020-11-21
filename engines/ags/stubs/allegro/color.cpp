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

#include "ags/stubs/allegro/color.h"
#include "common/textconsole.h"
#include "common/system.h"
#include "graphics/palette.h"

namespace AGS3 {

int _rgb_r_shift_15 = 0;
int _rgb_g_shift_15 = 0;
int _rgb_b_shift_15 = 0;
int _rgb_r_shift_16 = 0;
int _rgb_g_shift_16 = 0;
int _rgb_b_shift_16 = 0;
int _rgb_r_shift_24 = 0;
int _rgb_g_shift_24 = 0;
int _rgb_b_shift_24 = 0;
int _rgb_r_shift_32 = 0;
int _rgb_g_shift_32 = 0;
int _rgb_b_shift_32 = 0;
int _rgb_a_shift_32 = 0;

int bestfit_color(const PALETTE pal, int r, int g, int b) {
	error("TODO: bestfit_color");
}

void set_color(int idx, const RGB *p) {
	g_system->getPaletteManager()->setPalette((const byte *)p, idx, 1);
}

void set_palette(const PALETTE p) {

}

void set_palette_range(const PALETTE p, int from, int to, int retracesync) {
	byte palette[256 * 3];
	byte *destP = palette;
	for (int i = 0; i < 256; ++i, destP += 3) {
		destP[0] = p->r;
		destP[1] = p->g;
		destP[2] = p->b;
	}

	g_system->getPaletteManager()->setPalette(&palette[from], from, to - from + 1);
}

} // namespace AGS3
