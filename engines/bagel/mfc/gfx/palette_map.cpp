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

#include "bagel/mfc/gfx/palette_map.h"

namespace Bagel {
namespace MFC {
namespace Gfx {

PaletteMap::PaletteMap(const Graphics::Palette &src,
		const Graphics::Palette &dest) : _srcPalCount(src.size()) {
	// Set up lookup map
	Graphics::PaletteLookup lookup(dest.data(), dest.size());
	_map = lookup.createMap(src.data(), src.size());
}

PaletteMap::~PaletteMap() {
	delete[] _map;
}

void PaletteMap::map(const byte *src, byte *dest, size_t len) {
	if (_map) {
		for (; len > 0; --len, ++src, ++dest)
			*dest = _map[*src];
	} else {
		// Identical palettes, so pixels can just be copied
		Common::copy(src, src + len, dest);
	}
}

} // namespace Gfx
} // namespace MFC
} // namespace Bagel
