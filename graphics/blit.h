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

#ifndef GRAPHICS_BLIT_H
#define GRAPHICS_BLIT_H

#include "graphics/pixelformat.h"
#include "graphics/transform_struct.h"
#include "graphics/blit/blit-nocode.h"

namespace Graphics {

/** Converting a palette for use with crossBlitMap(). */
inline static void convertPaletteToMap(uint32 *dst, const byte *src, uint colors, const Graphics::PixelFormat &format) {
	while (colors-- > 0) {
		*dst++ = format.RGBToColor(src[0], src[1], src[2]);
		src += 3;
	}
}

inline int BlendBlit::getScaleFactor(int srcSize, int dstSize) {
	return SCALE_THRESHOLD * srcSize / dstSize;
}

inline Graphics::PixelFormat Graphics::BlendBlit::getSupportedPixelFormat() {
	return PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
}

} // End of namespace Graphics

#endif // GRAPHICS_BLIT_H
