/* ScummVM - Graphic Adventure AweEngine
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

#include "common/textconsole.h"
#include "awe/bitmap.h"

namespace Awe {

static void clut(const uint8 *src, const uint8 *pal, int pitch, int w, int h, int bpp, bool flipY, int colorKey, uint8 *dst) {
	int dstPitch = bpp * w;
	if (flipY) {
		dst += (h - 1) * bpp * w;
		dstPitch = -bpp * w;
	}
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			const int color = src[x];
			const int b = pal[color * 4];
			const int g = pal[color * 4 + 1];
			const int r = pal[color * 4 + 2];
			dst[x * bpp] = r;
			dst[x * bpp + 1] = g;
			dst[x * bpp + 2] = b;
			if (bpp == 4) {
				dst[x * bpp + 3] = (color == 0 || (colorKey == ((r << 16) | (g << 8) | b))) ? 0 : 255;
			}
		}
		src += w;
		dst += dstPitch;
	}
}

uint8 *decode_bitmap(const uint8 *src, bool alpha, int colorKey, int *w, int *h) {
	if (memcmp(src, "BM", 2) != 0) {
		return nullptr;
	}
	const uint32 imageOffset = READ_LE_UINT32(src + 0xA);
	const int width = READ_LE_UINT32(src + 0x12);
	const int height = READ_LE_UINT32(src + 0x16);
	const int depth = READ_LE_UINT16(src + 0x1C);
	const int compression = READ_LE_UINT32(src + 0x1E);
	if ((depth != 8 && depth != 32) || compression != 0) {
		warning("Unhandled bitmap depth %d compression %d", depth, compression);
		return nullptr;
	}
	const int bpp = (!alpha && colorKey < 0) ? 3 : 4;
	uint8 *dst = (uint8 *)malloc(width * height * bpp);
	if (!dst) {
		warning("Failed to allocate bitmap buffer, width %d height %d bpp %d", width, height, bpp);
		return nullptr;
	}
	if (depth == 8) {
		const uint8 *palette = src + 14 /* BITMAPFILEHEADER */ + 40 /* BITMAPINFOHEADER */;
		const bool flipY = true;
		clut(src + imageOffset, palette, (width + 3) & ~3, width, height, bpp, flipY, colorKey, dst);
	} else {
		assert(depth == 32 && bpp == 3);
		const uint8 *p = src + imageOffset;
		for (int y = height - 1; y >= 0; --y) {
			uint8 *q = dst + y * width * bpp;
			for (int x = 0; x < width; ++x) {
				const uint32 color = READ_LE_UINT32(p); p += 4;
				*q++ = (color >> 16) & 255;
				*q++ = (color >> 8) & 255;
				*q++ = color & 255;
			}
		}
	}
	*w = width;
	*h = height;
	return dst;
}

} // namespace Awe
