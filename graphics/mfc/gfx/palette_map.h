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

#ifndef GRAPHICS_MFC_GFX_PALETTE_MAP_H
#define GRAPHICS_MFC_GFX_PALETTE_MAP_H

#include "graphics/palette.h"

namespace Graphics {
namespace MFC {
namespace Gfx {

/**
 * Simple wrapper over Graphics::PaletteLookup
 */
class PaletteMap {
private:
	uint32 *_map;
	size_t _srcPalCount;

public:
	PaletteMap(const Graphics::Palette &src,
		const Graphics::Palette &dest);
	~PaletteMap();

	void map(const byte *src, byte *dest, size_t len);
};

} // namespace Gfx
} // namespace MFC
} // namespace Graphics

#endif
