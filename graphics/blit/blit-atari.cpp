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

#include <mint/cookie.h>

#include "backends/graphics/atari/atari-supervidel.h"
#include "backends/platform/atari/dlmalloc.h"	// MALLOC_ALIGNMENT

static_assert(MALLOC_ALIGNMENT == 16, "MALLOC_ALIGNMENT must be == 16");

#ifdef USE_MOVE16
static inline bool hasMove16() {
	long val;
	static bool hasMove16 = Getcookie(C__CPU, &val) == C_FOUND && val >= 40;
	return hasMove16;
}

template<typename T>
constexpr bool isAligned(T val) {
	return (reinterpret_cast<uintptr>(val) & (MALLOC_ALIGNMENT - 1)) == 0;
}
#endif

namespace Graphics {

// Function to blit a rect with a transparent color key
void keyBlitLogicAtari(byte *dst, const byte *src, const uint w, const uint h,
					   const uint srcDelta, const uint dstDelta, const uint32 key) {
#ifdef USE_SV_BLITTER
	if (key == 0 && (uintptr)src >= 0xA0000000 && (uintptr)dst >= 0xA0000000) {
		if (g_superVidelFwVersion >= 9) {
			*SV_BLITTER_FIFO = (long)src;				// SV_BLITTER_SRC1
			*SV_BLITTER_FIFO = (long)(g_blitMask ? g_blitMask : src);	// SV_BLITTER_SRC2
			*SV_BLITTER_FIFO = (long)dst;				// SV_BLITTER_DST
			*SV_BLITTER_FIFO = w - 1;					// SV_BLITTER_COUNT
			*SV_BLITTER_FIFO = srcDelta + w;			// SV_BLITTER_SRC1_OFFSET
			*SV_BLITTER_FIFO = srcDelta + w;			// SV_BLITTER_SRC2_OFFSET
			*SV_BLITTER_FIFO = dstDelta + w;			// SV_BLITTER_DST_OFFSET
			*SV_BLITTER_FIFO = h;						// SV_BLITTER_MASK_AND_LINES
			*SV_BLITTER_FIFO = 0x03;					// SV_BLITTER_CONTROL
		}  else {
			// make sure the blitter is idle
			while (*SV_BLITTER_CONTROL & 1);

			*SV_BLITTER_SRC1           = (long)src;
			*SV_BLITTER_SRC2           = (long)(g_blitMask ? g_blitMask : src);
			*SV_BLITTER_DST            = (long)dst;
			*SV_BLITTER_COUNT          = w - 1;
			*SV_BLITTER_SRC1_OFFSET    = srcDelta + w;
			*SV_BLITTER_SRC2_OFFSET    = srcDelta + w;
			*SV_BLITTER_DST_OFFSET     = dstDelta + w;
			*SV_BLITTER_MASK_AND_LINES = h;
			*SV_BLITTER_CONTROL        = 0x03;
		}

		SyncSuperBlitter();
	} else
#endif
	{
		for (uint y = 0; y < h; ++y) {
			for (uint x = 0; x < w; ++x) {
				const uint32 color = *src++;
				if (color != key)
					*dst++ = color;
				else
					dst++;
			}

			src += srcDelta;
			dst += dstDelta;
		}
	}
}

// Function to blit a rect (version optimized for Atari Falcon with SuperVidel's SuperBlitter)
void copyBlit(byte *dst, const byte *src,
			   const uint dstPitch, const uint srcPitch,
			   const uint w, const uint h,
			   const uint bytesPerPixel) {
	if (dst == src)
		return;

#ifdef USE_SV_BLITTER
	if ((uintptr)src >= 0xA0000000 && (uintptr)dst >= 0xA0000000) {
		if (g_superVidelFwVersion >= 9) {
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

		SyncSuperBlitter();
	} else
#endif
	if (dstPitch == srcPitch && dstPitch == (w * bytesPerPixel)) {
#ifdef USE_MOVE16
		if (hasMove16() && isAligned(src) && isAligned(dst)) {
			__asm__ volatile(
			"	move.l	%2,%%d0\n"
			"	lsr.l	#4,%%d0\n"
			"	beq.b	3f\n"

			"	moveq	#0x0f,%%d1\n"
			"	and.l	%%d0,%%d1\n"
			"	neg.l	%%d1\n"
			"	lsr.l	#4,%%d0\n"
			"	jmp		(2f,%%pc,%%d1.l*4)\n"
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
			"	dbra	%%d0,1b\n"
			// handle also the case when 'dstPitch' is not
			// divisible by 16 but 'src' and 'dst' are
			"3:\n"
			"	moveq	#0x0f,%%d0\n"
			"	and.l	%2,%%d0\n"
			"	neg.l	%%d0\n"
			"	jmp		(4f,%%pc,%%d0.l*2)\n"
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
#else
		{
#endif
			memcpy(dst, src, dstPitch * h);
		}
	} else {
#ifdef USE_MOVE16
		if (hasMove16() && isAligned(src) && isAligned(dst) && isAligned(srcPitch) && isAligned(dstPitch)) {
			__asm__ volatile(
			"	move.l	%2,%%d0\n"

			"	moveq	#0x0f,%%d1\n"
			"	and.l	%%d0,%%d1\n"
			"	neg.l	%%d1\n"
			"	lea		(4f,%%pc,%%d1.l*2),%%a0\n"
			"	move.l	%%a0,%%a1\n"

			"	lsr.l	#4,%%d0\n"
			"	beq.b	3f\n"

			"	moveq	#0x0f,%%d1\n"
			"	and.l	%%d0,%%d1\n"
			"	neg.l	%%d1\n"
			"	lea		(2f,%%pc,%%d1.l*4),%%a0\n"
			"	lsr.l	#4,%%d0\n"
			"	move.l	%%d0,%%d1\n"
			"0:\n"
			"	move.l	%%d1,%%d0\n"
			"	jmp		(%%a0)\n"
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
			"	dbra	%%d0,1b\n"
			// handle (w * bytesPerPixel) % 16
			"3:\n"
			"	jmp		(%%a1)\n"
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
				: "d0", "d1", "a0", "a1", "cc" AND_MEMORY
			);
		} else {
#else
		{
#endif
			for (uint i = 0; i < h; ++i) {
				memcpy(dst, src, w * bytesPerPixel);
				dst += dstPitch;
				src += srcPitch;
			}
		}
	}
}

} // End of namespace Graphics
