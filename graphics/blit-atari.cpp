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
#include "graphics/surface.h"

#include <cstdlib>	// calloc
#include <cstring>	// memcpy
#include <mint/cookie.h>
#include <mint/falcon.h>

#include "backends/graphics/atari/atari-graphics-superblitter.h"

namespace Graphics {

// hijack surface overrides here as well as these are tightly related
// to the blitting routine below
void Surface::create(int16 width, int16 height, const PixelFormat &f) {
	assert(width >= 0 && height >= 0);
	free();

	w = width;
	h = height;
	format = f;
	pitch = w * format.bytesPerPixel;

	if (width && height) {
		if (VgetMonitor() == MON_VGA && Getcookie(C_SupV, NULL) == C_FOUND)
			pixels = (void*)ct60_vmalloc(width * height * format.bytesPerPixel);
		else
			pixels = calloc(width * height, format.bytesPerPixel);
		assert(pixels);
	}
}

void Surface::free() {
	if (((uintptr)pixels & 0xFF000000) >= 0xA0000000)
		ct60_vmfree(pixels);
	else
		::free(pixels);

	pixels = 0;
	w = h = pitch = 0;
	format = PixelFormat();
}

// Function to blit a rect (version optimized for Atari Falcon with SuperVidel's SuperBlitter)
void copyBlit(byte *dst, const byte *src,
			   const uint dstPitch, const uint srcPitch,
			   const uint w, const uint h,
			   const uint bytesPerPixel) {
	if (dst == src)
		return;

	if (((uintptr)src & 0xFF000000) >= 0xA0000000 && ((uintptr)dst & 0xFF000000) >= 0xA0000000) {
		// while busy blitting...
		while (*SV_BLITTER_CONTROL & 1);

		*SV_BLITTER_SRC1           = (long)src;
		*SV_BLITTER_SRC2           = 0x00000000;
		*SV_BLITTER_DST            = (long)dst;
		*SV_BLITTER_COUNT          = w * bytesPerPixel - 1;
		*SV_BLITTER_SRC1_OFFSET    = srcPitch;
		*SV_BLITTER_SRC2_OFFSET    = 0x00000000;
		*SV_BLITTER_DST_OFFSET     = dstPitch;
		*SV_BLITTER_MASK_AND_LINES = h;
		*SV_BLITTER_CONTROL        = 0x01;

		// wait until we finish otherwise we may overwrite pixels written manually afterwards
		while (*SV_BLITTER_CONTROL & 1);
	} else if (dstPitch == srcPitch && ((w * bytesPerPixel) == dstPitch)) {
		memcpy(dst, src, dstPitch * h);
	} else {
		for (uint i = 0; i < h; ++i) {
			memcpy(dst, src, w * bytesPerPixel);
			dst += dstPitch;
			src += srcPitch;
		}
	}
}

} // End of namespace Graphics
