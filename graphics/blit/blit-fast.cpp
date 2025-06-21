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

#include "graphics/blit.h"
#include "graphics/pixelformat.h"
#include "common/endian.h"

namespace Graphics {

namespace {

template<bool bswap, int rotate>
static void swapBlit(byte *dst, const byte *src,
                     const uint dstPitch, const uint srcPitch,
                     const uint w, const uint h) {
	// Faster, but larger, to provide optimized handling for each case.
	const uint srcDelta = (srcPitch - w * sizeof(uint32));
	const uint dstDelta = (dstPitch - w * sizeof(uint32));

	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {
			uint32 col = *(const uint32 *)src;

			if (bswap)
				col = SWAP_BYTES_32(col);
			if (rotate != 0)
				col = ROTATE_RIGHT_32(col, rotate);

			*(uint32 *)dst = col;

			src += sizeof(uint32);
			dst += sizeof(uint32);
		}
		src += srcDelta;
		dst += dstDelta;
	}
}

} // End of anonymous namespace

// TODO: Add fast 24<->32bpp conversion
// TODO: Add fast 16<->16bpp conversion
struct FastBlitLookup {
	FastBlitFunc func;
	Graphics::PixelFormat srcFmt, dstFmt;
};

static const FastBlitLookup fastBlitFuncs_4to4[] = {
	// 32-bit byteswap
	{ swapBlit<true,   0>, Graphics::PixelFormat(4, 8, 8, 8, 8,  0,  8, 16, 24), Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16,  8,  0) }, // ABGR8888 -> RGBA8888
	{ swapBlit<true,   0>, Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16,  8,  0), Graphics::PixelFormat(4, 8, 8, 8, 8,  0,  8, 16, 24) }, // RGBA8888 -> ABGR8888
	{ swapBlit<true,   0>, Graphics::PixelFormat(4, 8, 8, 8, 8, 16,  8,  0, 24), Graphics::PixelFormat(4, 8, 8, 8, 8,  8, 16, 24,  0) }, // ARGB8888 -> BGRA8888
	{ swapBlit<true,   0>, Graphics::PixelFormat(4, 8, 8, 8, 8,  8, 16, 24,  0), Graphics::PixelFormat(4, 8, 8, 8, 8, 16,  8,  0, 24) }, // BGRA8888 -> ARGB8888

	// 32-bit rotate right
	{ swapBlit<false,  8>, Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16,  8,  0), Graphics::PixelFormat(4, 8, 8, 8, 8, 16,  8,  0, 24) }, // RGBA8888 -> ARGB8888
	{ swapBlit<false,  8>, Graphics::PixelFormat(4, 8, 8, 8, 8,  8, 16, 24,  0), Graphics::PixelFormat(4, 8, 8, 8, 8,  0,  8, 16, 24) }, // BGRA8888 -> ABGR8888

	// 32-bit rotate left
	{ swapBlit<false, 24>, Graphics::PixelFormat(4, 8, 8, 8, 8,  0,  8, 16, 24), Graphics::PixelFormat(4, 8, 8, 8, 8,  8, 16, 24,  0) }, // ABGR8888 -> BGRA8888
	{ swapBlit<false, 24>, Graphics::PixelFormat(4, 8, 8, 8, 8, 16,  8,  0, 24), Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16,  8,  0) }, // ARGB8888 -> RGBA8888

	// 32-bit byteswap and rotate right
	{ swapBlit<true,   8>, Graphics::PixelFormat(4, 8, 8, 8, 8,  0,  8, 16, 24), Graphics::PixelFormat(4, 8, 8, 8, 8, 16,  8,  0, 24) }, // ABGR8888 -> ARGB8888
	{ swapBlit<true,   8>, Graphics::PixelFormat(4, 8, 8, 8, 8, 16,  8,  0, 24), Graphics::PixelFormat(4, 8, 8, 8, 8,  0,  8, 16, 24) }, // ARGB8888 -> ABGR8888

	// 32-bit byteswap and rotate left
	{ swapBlit<true,  24>, Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16,  8,  0), Graphics::PixelFormat(4, 8, 8, 8, 8,  8, 16, 24,  0) }, // RGBA8888 -> BGRA8888
	{ swapBlit<true,  24>, Graphics::PixelFormat(4, 8, 8, 8, 8,  8, 16, 24,  0), Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16,  8,  0) }  // BGRA8888 -> RGBA8888
};

FastBlitFunc getFastBlitFunc(const PixelFormat &dstFmt, const PixelFormat &srcFmt) {
	const uint dstBpp = dstFmt.bytesPerPixel;
	const uint srcBpp = srcFmt.bytesPerPixel;
	const FastBlitLookup *table = nullptr;
	size_t length = 0;

	if (srcBpp == 4 && dstBpp == 4) {
		table = fastBlitFuncs_4to4;
		length = ARRAYSIZE(fastBlitFuncs_4to4);
	} else {
		return nullptr;
	}

	for (size_t i = 0; i < length; i++) {
		if (srcFmt != table[i].srcFmt)
			continue;
		if (dstFmt != table[i].dstFmt)
			continue;

		return table[i].func;
	}
	return nullptr;
}

} // End of namespace Graphics
