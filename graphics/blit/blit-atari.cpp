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
#include "backends/platform/atari/dlmalloc.h"

#include <cstdlib>	// malloc
#include <cstring>	// memcpy, memset
#include <mint/cookie.h>

#include "backends/graphics/atari/atari-graphics-superblitter.h"
#include "common/textconsole.h"	// error

// bits 26:0
#define SV_BLITTER_SRC1           ((volatile long*)0x80010058)
#define SV_BLITTER_SRC2           ((volatile long*)0x8001005C)
#define SV_BLITTER_DST            ((volatile long*)0x80010060)
// The amount of bytes that are to be copied in a horizontal line, minus 1
#define SV_BLITTER_COUNT          ((volatile long*)0x80010064)
// The amount of bytes that are to be added to the line start address after a line has been copied, in order to reach the next one
#define SV_BLITTER_SRC1_OFFSET    ((volatile long*)0x80010068)
#define SV_BLITTER_SRC2_OFFSET    ((volatile long*)0x8001006C)
#define SV_BLITTER_DST_OFFSET     ((volatile long*)0x80010070)
// bits 11:0 - The amount of horizontal lines to do
#define SV_BLITTER_MASK_AND_LINES ((volatile long*)0x80010074)
// bit    0 - busy / start
// bits 4:1 - blit mode
#define SV_BLITTER_CONTROL        ((volatile long*)0x80010078)
// bit 0 - empty (read only)
// bit 1 - full (read only)
// bits 31:0 - data (write only)
#define SV_BLITTER_FIFO           ((volatile long*)0x80010080)

#ifdef USE_SV_BLITTER
static bool isSuperBlitterLocked;

static void syncSuperBlitter() {
	// if externally locked, let the owner decide when to sync (unlock)
	if (isSuperBlitterLocked)
		return;

	// while FIFO not empty...
	if (superVidelFwVersion >= 9)
		while (!(*SV_BLITTER_FIFO & 1));
	// while busy blitting...
	while (*SV_BLITTER_CONTROL & 1);
}
#endif

void lockSuperBlitter() {
#ifdef USE_SV_BLITTER
	assert(!isSuperBlitterLocked);

	isSuperBlitterLocked = true;
#endif
}

void unlockSuperBlitter() {
#ifdef USE_SV_BLITTER
	assert(isSuperBlitterLocked);

	isSuperBlitterLocked = false;
	if (hasSuperVidel())
		syncSuperBlitter();
#endif
}

// see atari-graphics.cpp
extern bool g_unalignedPitch;
extern mspace g_mspace;

namespace Graphics {

constexpr size_t ALIGN = 16;	// 16 bytes

// hijack surface overrides here as well as these are tightly related
// to the blitting routine below
void Surface::create(int16 width, int16 height, const PixelFormat &f) {
	assert(width >= 0 && height >= 0);
	free();

	w = width;
	h = height;
	format = f;
	// align pitch to a 16-byte boundary for a possible C2P conversion
	pitch = g_unalignedPitch
		? w * format.bytesPerPixel
		: (w * format.bytesPerPixel + ALIGN - 1) & (-ALIGN);

	if (width && height) {
#ifdef USE_SV_BLITTER
		if (g_mspace) {
			pixels = mspace_calloc(g_mspace, height * pitch, f.bytesPerPixel);

			if (!pixels)
				error("Not enough memory to allocate a surface");
			else if (pixels <= (void *)0xA0000000)
				warning("SuperVidel surface allocated in regular memory");
		} else {
#else
		{
#endif
			pixels = ::calloc(height * pitch, f.bytesPerPixel);
			if (!pixels)
				error("Not enough memory to allocate a surface");
			else
				assert(((uintptr)pixels & (ALIGN - 1)) == 0);
		}
	}
}

void Surface::free() {
#ifdef USE_SV_BLITTER
	if (g_mspace)
		mspace_free(g_mspace, pixels);
	else
#endif
	if (pixels)
		::free(pixels);

	pixels = nullptr;
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

#ifdef USE_SV_BLITTER
	if (((uintptr)src & 0xFF000000) >= 0xA0000000 && ((uintptr)dst & 0xFF000000) >= 0xA0000000) {
		if (superVidelFwVersion >= 9) {
			*SV_BLITTER_FIFO = (long)src;				// SV_BLITTER_SRC1
			*SV_BLITTER_FIFO = 0x00000000;				// SV_BLITTER_SRC2
			*SV_BLITTER_FIFO = (long)dst;				// SV_BLITTER_DST
			*SV_BLITTER_FIFO = w * bytesPerPixel - 1;	// SV_BLITTER_COUNT
			*SV_BLITTER_FIFO = srcPitch;				// SV_BLITTER_SRC1_OFFSET
			*SV_BLITTER_FIFO = 0x00000000;				// SV_BLITTER_SRC2_OFFSET
			*SV_BLITTER_FIFO = dstPitch;				// SV_BLITTER_DST_OFFSET
			*SV_BLITTER_FIFO = h;						// SV_BLITTER_MASK_AND_LINES
			*SV_BLITTER_FIFO = 0x01;					// SV_BLITTER_CONTROL
		}  else {
			// make sure the blitter is idle
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
		}

		syncSuperBlitter();
	} else
#endif
	if (dstPitch == srcPitch && dstPitch == (w * bytesPerPixel)) {
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
