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
#include "graphics/palette.h"
#include "graphics/paletteman.h"
#include "graphics/screen.h"
#include "mm/mm1/gfx/gfx.h"

namespace MM {
namespace MM1 {
namespace Gfx {

byte EGA_INDEXES[EGA_PALETTE_COUNT];

void GFX::setEgaPalette() {
	Graphics::Palette ega = Graphics::Palette::createEGAPalette();
	g_system->getPaletteManager()->setPalette(ega);

	uint32 c = 0xffffffff;
	g_system->getPaletteManager()->setPalette((const byte *)&c, 255, 1);

	// Set the EGA palette indexes to be themselves
	for (int i = 0; i < EGA_PALETTE_COUNT; ++i)
		EGA_INDEXES[i] = i;
}

void GFX::findPalette(const byte palette[256 * 3]) {
	Graphics::Palette ega = Graphics::Palette::createEGAPalette();
	const byte *data = ega.data();

	for (int col = 0; col < 16; ++col) {
		byte r = data[col * 3 + 2];
		byte g = data[col * 3 + 1];
		byte b = data[col * 3 + 0];
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
