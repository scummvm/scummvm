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

#include <cstdlib>	// malloc
#include <cstring>	// memcpy, memset
#include <mint/cookie.h>
#include <mint/falcon.h>

#include "backends/graphics/atari/atari-graphics-superblitter.h"
#include "common/textconsole.h"	// error

static inline bool hasMove16() {
	long val;
	static bool hasMove16 = Getcookie(C__CPU, &val) == C_FOUND && val >= 40;
	return hasMove16;
}

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
	pitch = (w * format.bytesPerPixel + ALIGN - 1) & (-ALIGN);

	if (width && height) {
		if (VgetMonitor() == MON_VGA && Getcookie(C_SupV, NULL) == C_FOUND) {
			pixels = (void *)ct60_vmalloc(height * pitch);

			if (!pixels)
				error("Not enough SVRAM to allocate a surface");

			assert((uintptr)pixels >= 0xA0000000);
		} else {
			// align buffer to a 16-byte boundary for move16 or C2P conversion
			void *pixelsUnaligned = ::malloc(sizeof(uintptr) + (height * pitch) + ALIGN - 1);

			if (!pixelsUnaligned)
				error("Not enough memory to allocate a surface");

			pixels = (void *)(((uintptr)pixelsUnaligned + sizeof(uintptr) + ALIGN - 1) & (-ALIGN));

			// store the unaligned pointer for later free()
			*((uintptr *)pixels - 1) = (uintptr)pixelsUnaligned;
		}

		memset(pixels, 0, height * pitch);
	}
}

void Surface::free() {
	if (((uintptr)pixels & 0xFF000000) >= 0xA0000000)
		ct60_vmfree(pixels);
	else if (pixels)
		::free((void *)*((uintptr *)pixels - 1));

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
		if (hasMove16() && ((uintptr)src & (ALIGN - 1)) == 0 && ((uintptr)dst & (ALIGN - 1)) == 0) {
			__asm__ volatile(
			"	move.l	%2,d0\n"
			"	lsr.l	#4,d0\n"
			"	beq.b	3f\n"

			"	moveq	#0x0f,d1\n"
			"	and.l	d0,d1\n"
			"	neg.l	d1\n"
			"	lsr.l	#4,d0\n"
			"	jmp	(2f,pc,d1.l*4)\n"
			"1:\n"
			"	move16	(%0)+,(%1)+\n"
			"	move16	(%0)+,(%1)+\n"
			"	move16	(%0)+,(%1)+\n"
			"	move16	(%0)+,(%1)+\n"
			"	move16	(%0)+,(%1)+\n"
			"	move16	(%0)+,(%1)+\n"
			"	move16	(%0)+,(%1)+\n"
			"	move16	(%0)+,(%1)+\n"
			"	move16	(%0)+,(%1)+\n"
			"	move16	(%0)+,(%1)+\n"
			"	move16	(%0)+,(%1)+\n"
			"	move16	(%0)+,(%1)+\n"
			"	move16	(%0)+,(%1)+\n"
			"	move16	(%0)+,(%1)+\n"
			"	move16	(%0)+,(%1)+\n"
			"	move16	(%0)+,(%1)+\n"
			"2:\n"
			"	dbra	d0,1b\n"
			// handle also the unlikely case when 'dstPitch'
			// is not divisible by 16 but 'src' and 'dst' are
			"3:\n"
			"	moveq	#0x0f,d0\n"
			"	and.l	%2,d0\n"
			"	neg.l	d0\n"
			"	jmp	(4f,pc,d0.l*2)\n"
			// only 15x move.b as 16 would be handled above
			"	move.b	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"4:\n"
				: // outputs
				: "a"(src), "a"(dst), "g"(dstPitch * h) // inputs
				: "d0", "d1", "cc" AND_MEMORY
			);
		} else {
			memcpy(dst, src, dstPitch * h);
		}
	} else {
		if (hasMove16() && ((uintptr)src & (ALIGN - 1)) == 0 && ((uintptr)dst & (ALIGN - 1)) == 0) {
			__asm__ volatile(
			"0:\n"
			"	move.l	%2,d0\n"
			"	lsr.l	#4,d0\n"
			"	beq.b	3f\n"

			"	moveq	#0x0f,d1\n"
			"	and.l	d0,d1\n"
			"	neg.l	d1\n"
			"	lsr.l	#4,d0\n"
			"	jmp	(2f,pc,d1.l*4)\n"
			"1:\n"
			"	move16	(%0)+,(%1)+\n"
			"	move16	(%0)+,(%1)+\n"
			"	move16	(%0)+,(%1)+\n"
			"	move16	(%0)+,(%1)+\n"
			"	move16	(%0)+,(%1)+\n"
			"	move16	(%0)+,(%1)+\n"
			"	move16	(%0)+,(%1)+\n"
			"	move16	(%0)+,(%1)+\n"
			"	move16	(%0)+,(%1)+\n"
			"	move16	(%0)+,(%1)+\n"
			"	move16	(%0)+,(%1)+\n"
			"	move16	(%0)+,(%1)+\n"
			"	move16	(%0)+,(%1)+\n"
			"	move16	(%0)+,(%1)+\n"
			"	move16	(%0)+,(%1)+\n"
			"	move16	(%0)+,(%1)+\n"
			"2:\n"
			"	dbra	d0,1b\n"
			// handle (w * bytesPerPixel) % 16
			"3:\n"
			"	moveq	#0x0f,d0\n"
			"	and.l	%2,d0\n"
			"	neg.l	d0\n"
			"	jmp	(4f,pc,d0.l*2)\n"
			// only 15x move.b as 16 would be handled above
			"	move.b	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"4:\n"
			"	add.l	%4,%1\n"
			"	add.l	%5,%0\n"
			"	dbra	%3,0b\n"
				: // outputs
				: "a"(src), "a"(dst), "g"(w * bytesPerPixel), "d"(h - 1),
				  "g"(dstPitch - w * bytesPerPixel), "g"(srcPitch - w * bytesPerPixel) // inputs
				: "d0", "d1", "d2", "cc" AND_MEMORY
			);
		} else {
			for (uint i = 0; i < h; ++i) {
				memcpy(dst, src, w * bytesPerPixel);
				dst += dstPitch;
				src += srcPitch;
			}
		}
	}
}

} // End of namespace Graphics
