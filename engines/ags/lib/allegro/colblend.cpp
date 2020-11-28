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

#include "ags/lib/allegro/colblend.h"

namespace AGS3 {

unsigned long _blender_black(unsigned long x, unsigned long y, unsigned long n) {
	return 0;
}

unsigned long _blender_trans15(unsigned long x, unsigned long y, unsigned long n) {
	unsigned long result;

	if (n)
		n = (n + 1) / 8;

	x = ((x & 0xFFFF) | (x << 16)) & 0x3E07C1F;
	y = ((y & 0xFFFF) | (y << 16)) & 0x3E07C1F;

	result = ((x - y) * n / 32 + y) & 0x3E07C1F;

	return ((result & 0xFFFF) | (result >> 16));
}

unsigned long _blender_trans16(unsigned long x, unsigned long y, unsigned long n) {
	unsigned long result;

	if (n)
		n = (n + 1) / 8;

	x = ((x & 0xFFFF) | (x << 16)) & 0x7E0F81F;
	y = ((y & 0xFFFF) | (y << 16)) & 0x7E0F81F;

	result = ((x - y) * n / 32 + y) & 0x7E0F81F;

	return ((result & 0xFFFF) | (result >> 16));
}

unsigned long _blender_alpha15(unsigned long x, unsigned long y, unsigned long n) {
	unsigned long result;

	n = geta32(x);

	if (n)
		n = (n + 1) / 8;

	x = makecol15(getr32(x), getg32(x), getb32(x));

	x = (x | (x << 16)) & 0x3E07C1F;
	y = ((y & 0xFFFF) | (y << 16)) & 0x3E07C1F;

	result = ((x - y) * n / 32 + y) & 0x3E07C1F;

	return ((result & 0xFFFF) | (result >> 16));
}

unsigned long _blender_alpha16(unsigned long x, unsigned long y, unsigned long n) {
	unsigned long result;

	n = geta32(x);

	if (n)
		n = (n + 1) / 8;

	x = makecol16(getr32(x), getg32(x), getb32(x));

	x = (x | (x << 16)) & 0x7E0F81F;
	y = ((y & 0xFFFF) | (y << 16)) & 0x7E0F81F;

	result = ((x - y) * n / 32 + y) & 0x7E0F81F;

	return ((result & 0xFFFF) | (result >> 16));
}

unsigned long _blender_alpha24(unsigned long x, unsigned long y, unsigned long n) {
	unsigned long xx = makecol24(getr32(x), getg32(x), getb32(x));
	unsigned long res, g;

	n = geta32(x);

	if (n)
		n++;

	res = ((xx & 0xFF00FF) - (y & 0xFF00FF)) * n / 256 + y;
	y &= 0xFF00;
	xx &= 0xFF00;
	g = (xx - y) * n / 256 + y;

	res &= 0xFF00FF;
	g &= 0xFF00;

	return res | g;
}

} // namespace AGS3
