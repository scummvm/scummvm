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
 * $URL$
 * $Id$
 */

#include "graphics/conversion.h"

namespace Graphics {

// TODO: YUV to RGB conversion function

// Function to blit a rect from one color format to another
bool crossBlit(byte *dst, const byte *src, int dstpitch, int srcpitch,
						int w, int h, const Graphics::PixelFormat &dstFmt, const Graphics::PixelFormat &srcFmt) {
	// Error out if conversion is impossible
	if ((srcFmt.bytesPerPixel == 1) || (dstFmt.bytesPerPixel == 1)
			 || (!srcFmt.bytesPerPixel) || (!dstFmt.bytesPerPixel)
			 || (srcFmt.bytesPerPixel > dstFmt.bytesPerPixel))
		return false;

	// Don't perform unnecessary conversion
	if (srcFmt == dstFmt) {
		if (dst == src) 
			return true;
		if (dstpitch == srcpitch && ((w * dstFmt.bytesPerPixel) == dstpitch)) {
			memcpy(dst,src,dstpitch * h);
			return true;
		} else {
			for (int i = 0; i < h; i++) {
				memcpy(dst,src,w * dstFmt.bytesPerPixel);
				dst += dstpitch;
				src += srcpitch;
			}
			return true;
		}
	}

	// Faster, but larger, to provide optimized handling for each case.
	int srcDelta, dstDelta;
	srcDelta = (srcpitch - w * srcFmt.bytesPerPixel);
	dstDelta = (dstpitch - w * dstFmt.bytesPerPixel);

	// TODO: optimized cases for dstDelta of 0
	uint8 r, g, b, a;
	if (dstFmt.bytesPerPixel == 2) {
		uint16 color;
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++, src += 2, dst += 2) {
				color = *(const uint16 *)src;
				srcFmt.colorToARGB(color, a, r, g, b);
				color = dstFmt.ARGBToColor(a, r, g, b);
				*(uint16 *)dst = color;
			}
			src += srcDelta;
			dst += dstDelta;
		}
	} else if (dstFmt.bytesPerPixel == 3) {
		uint32 color;
		uint8 *col = (uint8 *) &color;
#ifdef SCUMM_BIG_ENDIAN
		col++;
#endif
		if (srcFmt.bytesPerPixel == 2) {
			for (int y = 0; y < h; y++) {
				for (int x = 0; x < w; x++, src += 2, dst += 3) {
					color = *(const uint16 *)src;
					srcFmt.colorToARGB(color, a, r, g, b);
					color = dstFmt.ARGBToColor(a, r, g, b);
					memcpy(dst, col, 3);
				}
				src += srcDelta;
				dst += dstDelta;
			}
		} else {
			for (int y = 0; y < h; y++) {
				for (int x = 0; x < w; x++, src += 3, dst += 3) {
					memcpy(col, src, 3);
					srcFmt.colorToARGB(color, a, r, g, b);
					color = dstFmt.ARGBToColor(a, r, g, b);
					memcpy(dst, col, 3);
				}
				src += srcDelta;
				dst += dstDelta;
			}
		}
	} else if (dstFmt.bytesPerPixel == 4) {
		uint32 color;
		if (srcFmt.bytesPerPixel == 2) {
			for (int y = 0; y < h; y++) {
				for (int x = 0; x < w; x++, src += 2, dst += 4) {
					color = *(uint16 *) src;
					srcFmt.colorToARGB(color, a, r, g, b);
					color = dstFmt.ARGBToColor(a, r, g, b);
					*(uint32 *)dst = color;
				}
				src += srcDelta;
				dst += dstDelta;
			}
		} else if (srcFmt.bytesPerPixel == 3) {
			uint8 *col = (uint8 *)&color;
#ifdef SCUMM_BIG_ENDIAN
			col++;
#endif
			for (int y = 0; y < h; y++) {
				for (int x = 0; x < w; x++, src += 2, dst += 4) {
					memcpy(col, src, 3);
					srcFmt.colorToARGB(color, a, r, g, b);
					color = dstFmt.ARGBToColor(a, r, g, b);
					*(uint32 *)dst = color;
				}
				src += srcDelta;
				dst += dstDelta;
			}
		} else {
			for (int y = 0; y < h; y++) {
				for (int x = 0; x < w; x++, src += 4, dst += 4) {
					color = *(uint32 *) src;
					srcFmt.colorToARGB(color, a, r, g, b);
					color = dstFmt.ARGBToColor(a, r, g, b);
					*(uint32 *)dst = color;
				}
				src += srcDelta;
				dst += dstDelta;
			}
		}
	} else {
		return false;
	}
	return true;
}

} // End of namespace Graphics
