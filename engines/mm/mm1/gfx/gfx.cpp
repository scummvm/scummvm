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
#include "common/debug.h"
#include "graphics/paletteman.h"
#include "graphics/screen.h"
#include "mm/mm1/gfx/gfx.h"

namespace MM {
namespace MM1 {
namespace Gfx {

byte EGA_INDEXES[EGA_PALETTE_COUNT];

static const uint32 EGA_PALETTE[16] = {
	0x000000, 0x0000aa, 0x00aa00, 0x00aaaa,
	0xaa0000, 0xaa00aa, 0xaa5500, 0xaaaaaa,
	0x555555, 0x5555ff, 0x55ff55, 0x55ffff,
	0xff5555, 0xff55ff, 0xffff55, 0xffffff
};


void GFX::setEgaPalette() {
	byte pal[16 * 3];
	byte *pDest = pal;

	for (int i = 0; i < EGA_PALETTE_COUNT; ++i) {
		*pDest++ = (EGA_PALETTE[i] >> 16) & 0xff;
		*pDest++ = (EGA_PALETTE[i] >> 8) & 0xff;
		*pDest++ = EGA_PALETTE[i] & 0xff;
		EGA_INDEXES[i] = i;
	}

	g_system->getPaletteManager()->setPalette(pal, 0, 16);

	uint32 c = 0xffffffff;
	g_system->getPaletteManager()->setPalette((const byte *)&c, 255, 1);

	// Set the EGA palette indexes to be themselves
}

void GFX::findPalette(const byte palette[256 * 3]) {
	for (int col = 0; col < 16; ++col) {
		byte r = (EGA_PALETTE[col] >> 16) & 0xff;
		byte g = (EGA_PALETTE[col] >> 8) & 0xff;
		byte b = EGA_PALETTE[col] & 0xff;
		int closestDiff = 0x7fffffff;
		byte closest = 0;
		const byte *pal = &palette[0];

		for (int i = 0; i < 256; ++i, pal += 3) {
			int diff = ABS((int)r - (int)pal[0]) +
				ABS((int)g - (int)pal[1]) +
				ABS((int)b - (int)pal[2]);
			if (diff < closestDiff) {
				closestDiff = diff;
				closest = i;
			}
		}

		EGA_INDEXES[col] = closest;
	}
}

} // namespace Gfx
} // namespace MM1
} // namespace MM
