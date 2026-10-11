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

#ifndef ULTIMA3_GFX_PIXELS_H
#define ULTIMA3_GFX_PIXELS_H

#include "common/scummsys.h"

namespace Ultima {
namespace Ultima3 {
namespace Gfx {

/**
 * Expands pixels held two bits each, the first in the top bits of a byte,
 * into a pixel per byte
 */
inline void unpack2bpp(const byte *src, byte *dest, int count) {
	for (int i = 0; i < count; ++i)
		dest[i] = (src[i >> 2] >> (6 - (i & 3) * 2)) & 3;
}

/**
 * Expands pixels held four bits each, the first in the top half of a byte,
 * into a pixel per byte
 */
inline void unpack4bpp(const byte *src, byte *dest, int count) {
	for (int i = 0; i < count; ++i)
		dest[i] = (i & 1) ? (src[i >> 1] & 0x0F) : (src[i >> 1] >> 4);
}

} // namespace Gfx
} // namespace Ultima3
} // namespace Ultima

#endif
