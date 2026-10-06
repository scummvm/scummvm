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

// move16 ignores the lowest 4 bits so only the offset within 16 bytes has to match
static inline bool haveSameAlignment(uintptr a, uintptr b) {
	return ((a ^ b) & (MALLOC_ALIGNMENT - 1)) == 0;
}
#endif

// writes are made long-aligned by copying a byte/word head first, reads from src may be misaligned
static inline void copyLong(byte *dst, const byte *src, uint w, uint h, uint dstSkip, uint srcSkip) {
	int loopCount = h - 1;
	__asm__ volatile(
	"0:\n"
	"	move.l	%3,%%d0\n"
	// copy the head up to the next 4-byte boundary of dst
	"	move.l	%1,%%d1\n"
	"	neg.l	%%d1\n"
	"	and.l	#3,%%d1\n"
	"	sub.l	%%d1,%%d0\n"
	"	lsr.l	#1,%%d1\n"
	"	bcc.b	1f\n"

	"	move.b	(%0)+,(%1)+\n"
	"1:\n"
	"	lsr.l	#1,%%d1\n"
	"	bcc.b	2f\n"

	"	move.w	(%0)+,(%1)+\n"
	"2:\n"
	// 256-byte blocks
	"	move.l	%%d0,%%d1\n"
	"	lsr.l	#8,%%d1\n"
	"	bra.w	4f\n"
	"3:\n"
	"	.rept	64\n"
	"	move.l	(%0)+,(%1)+\n"
	"	.endr\n"
	"4:\n"
	"	dbra	%%d1,3b\n"

	"	move.w	%%d0,%%d1\n"
	"	and.w	#0xff,%%d1\n"
	"	lsr.w	#2,%%d1\n"
	"	bra.b	6f\n"
	"5:\n"
	"	move.l	(%0)+,(%1)+\n"
	"6:\n"
	"	dbra	%%d1,5b\n"
	// copy the tail after the last long
	"	btst	#1,%%d0\n"
	"	beq.b	7f\n"

	"	move.w	(%0)+,(%1)+\n"
	"7:\n"
	"	btst	#0,%%d0\n"
	"	beq.b	8f\n"

	"	move.b	(%0)+,(%1)+\n"
	"8:\n"
	"	add.l	%4,%1\n"
	"	add.l	%5,%0\n"
	"	dbra	%2,0b\n"
		: "+a"(src), "+a"(dst), "+d"(loopCount) // outputs
		: "g"(w), "r"(dstSkip), "r"(srcSkip) // inputs
		: "d0", "d1", "cc" AND_MEMORY
	);
}

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
		if (hasMove16() && dstPitch * h >= 16 && haveSameAlignment((uintptr)src, (uintptr)dst)) {
			__asm__ volatile(
			"	move.l	%2,%%d0\n"
			// copy the head up to the next 16-byte boundary
			"	move.l	%1,%%d1\n"
			"	neg.l	%%d1\n"
			"	moveq	#0x0f,%%d2\n"
			"	and.l	%%d2,%%d1\n"
			"	beq.b	8f\n"

			"	sub.l	%%d1,%%d0\n"
			"	lsr.l	#1,%%d1\n"
			"	bcc.b	5f\n"

			"	move.b	(%0)+,(%1)+\n"
			"5:\n"
			"	lsr.l	#1,%%d1\n"
			"	bcc.b	6f\n"

			"	move.w	(%0)+,(%1)+\n"
			"6:\n"
			"	lsr.l	#1,%%d1\n"
			"	bcc.b	7f\n"

			"	move.l	(%0)+,(%1)+\n"
			"7:\n"
			"	lsr.l	#1,%%d1\n"
			"	bcc.b	8f\n"

			"	move.l	(%0)+,(%1)+\n"
			"	move.l	(%0)+,(%1)+\n"
			"8:\n"
			"	and.l	%%d0,%%d2\n"
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
			// handle the tail after the last 16-byte boundary
			"3:\n"
			"	lsl.l	#4,%%d2\n"
			"	jmp		(9f,%%pc,%%d2.l)\n"
			// 16-byte entries, one for each tail length
			"9:\n"
			// 0 bytes
			"	bra.w	4f\n"

			// 1 byte
			"	.org	9b+16\n"
			"	move.b	(%0)+,(%1)+\n"
			"	bra.w	4f\n"

			// 2 bytes
			"	.org	9b+32\n"
			"	move.w	(%0)+,(%1)+\n"
			"	bra.w	4f\n"

			// 3 bytes
			"	.org	9b+48\n"
			"	move.w	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	bra.w	4f\n"

			// 4 bytes
			"	.org	9b+64\n"
			"	move.l	(%0)+,(%1)+\n"
			"	bra.w	4f\n"

			// 5 bytes
			"	.org	9b+80\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	bra.w	4f\n"

			// 6 bytes
			"	.org	9b+96\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.w	(%0)+,(%1)+\n"
			"	bra.w	4f\n"

			// 7 bytes
			"	.org	9b+112\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.w	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	bra.w	4f\n"

			// 8 bytes
			"	.org	9b+128\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.l	(%0)+,(%1)+\n"
			"	bra.w	4f\n"

			// 9 bytes
			"	.org	9b+144\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	bra.w	4f\n"

			// 10 bytes
			"	.org	9b+160\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.w	(%0)+,(%1)+\n"
			"	bra.w	4f\n"

			// 11 bytes
			"	.org	9b+176\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.w	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	bra.w	4f\n"

			// 12 bytes
			"	.org	9b+192\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.l	(%0)+,(%1)+\n"
			"	bra.w	4f\n"

			// 13 bytes
			"	.org	9b+208\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	bra.w	4f\n"

			// 14 bytes
			"	.org	9b+224\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.w	(%0)+,(%1)+\n"
			"	bra.w	4f\n"

			// 15 bytes
			"	.org	9b+240\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.w	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"4:\n"
				: "+a"(src), "+a"(dst) // outputs
				: "g"(dstPitch * h) // inputs
				: "d0", "d1", "d2", "cc" AND_MEMORY
			);
			// WARNING: src and dst are modified by the asm code
		} else
#endif
		if (dstPitch * h >= 4) {
			copyLong(dst, src, dstPitch * h, 1, 0, 0);
		} else {
			memcpy(dst, src, dstPitch * h);
		}
	} else {
#ifdef USE_MOVE16
		if (hasMove16() && w >= 16 && haveSameAlignment((uintptr)src, (uintptr)dst) && haveSameAlignment(srcPitch, dstPitch)) {
			int loopCount = h - 1;
			__asm__ volatile(
			"0:\n"
			"	move.l	%3,%%d0\n"
			// copy the head up to the next 16-byte boundary
			"	move.l	%1,%%d1\n"
			"	neg.l	%%d1\n"
			"	moveq	#0x0f,%%d2\n"
			"	and.l	%%d2,%%d1\n"
			"	beq.b	8f\n"

			"	sub.l	%%d1,%%d0\n"
			"	lsr.l	#1,%%d1\n"
			"	bcc.b	5f\n"

			"	move.b	(%0)+,(%1)+\n"
			"5:\n"
			"	lsr.l	#1,%%d1\n"
			"	bcc.b	6f\n"

			"	move.w	(%0)+,(%1)+\n"
			"6:\n"
			"	lsr.l	#1,%%d1\n"
			"	bcc.b	7f\n"

			"	move.l	(%0)+,(%1)+\n"
			"7:\n"
			"	lsr.l	#1,%%d1\n"
			"	bcc.b	8f\n"

			"	move.l	(%0)+,(%1)+\n"
			"	move.l	(%0)+,(%1)+\n"
			"8:\n"
			"	and.l	%%d0,%%d2\n"
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
			// handle the tail after the last 16-byte boundary
			"3:\n"
			"	lsl.l	#5,%%d2\n"
			"	jmp		(9f,%%pc,%%d2.l)\n"
			// 32-byte entries, one for each tail length
			"9:\n"
			// 0 bytes
			"	add.l	%4,%1\n"
			"	add.l	%5,%0\n"
			"	dbra	%2,0b\n"
			"	bra.w	4f\n"

			// 1 byte
			"	.org	9b+32\n"
			"	move.b	(%0)+,(%1)+\n"
			"	add.l	%4,%1\n"
			"	add.l	%5,%0\n"
			"	dbra	%2,0b\n"
			"	bra.w	4f\n"

			// 2 bytes
			"	.org	9b+64\n"
			"	move.w	(%0)+,(%1)+\n"
			"	add.l	%4,%1\n"
			"	add.l	%5,%0\n"
			"	dbra	%2,0b\n"
			"	bra.w	4f\n"

			// 3 bytes
			"	.org	9b+96\n"
			"	move.w	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	add.l	%4,%1\n"
			"	add.l	%5,%0\n"
			"	dbra	%2,0b\n"
			"	bra.w	4f\n"

			// 4 bytes
			"	.org	9b+128\n"
			"	move.l	(%0)+,(%1)+\n"
			"	add.l	%4,%1\n"
			"	add.l	%5,%0\n"
			"	dbra	%2,0b\n"
			"	bra.w	4f\n"

			// 5 bytes
			"	.org	9b+160\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	add.l	%4,%1\n"
			"	add.l	%5,%0\n"
			"	dbra	%2,0b\n"
			"	bra.w	4f\n"

			// 6 bytes
			"	.org	9b+192\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.w	(%0)+,(%1)+\n"
			"	add.l	%4,%1\n"
			"	add.l	%5,%0\n"
			"	dbra	%2,0b\n"
			"	bra.w	4f\n"

			// 7 bytes
			"	.org	9b+224\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.w	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	add.l	%4,%1\n"
			"	add.l	%5,%0\n"
			"	dbra	%2,0b\n"
			"	bra.w	4f\n"

			// 8 bytes
			"	.org	9b+256\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.l	(%0)+,(%1)+\n"
			"	add.l	%4,%1\n"
			"	add.l	%5,%0\n"
			"	dbra	%2,0b\n"
			"	bra.w	4f\n"

			// 9 bytes
			"	.org	9b+288\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	add.l	%4,%1\n"
			"	add.l	%5,%0\n"
			"	dbra	%2,0b\n"
			"	bra.w	4f\n"

			// 10 bytes
			"	.org	9b+320\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.w	(%0)+,(%1)+\n"
			"	add.l	%4,%1\n"
			"	add.l	%5,%0\n"
			"	dbra	%2,0b\n"
			"	bra.w	4f\n"

			// 11 bytes
			"	.org	9b+352\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.w	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	add.l	%4,%1\n"
			"	add.l	%5,%0\n"
			"	dbra	%2,0b\n"
			"	bra.w	4f\n"

			// 12 bytes
			"	.org	9b+384\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.l	(%0)+,(%1)+\n"
			"	add.l	%4,%1\n"
			"	add.l	%5,%0\n"
			"	dbra	%2,0b\n"
			"	bra.w	4f\n"

			// 13 bytes
			"	.org	9b+416\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	add.l	%4,%1\n"
			"	add.l	%5,%0\n"
			"	dbra	%2,0b\n"
			"	bra.w	4f\n"

			// 14 bytes
			"	.org	9b+448\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.w	(%0)+,(%1)+\n"
			"	add.l	%4,%1\n"
			"	add.l	%5,%0\n"
			"	dbra	%2,0b\n"
			"	bra.w	4f\n"

			// 15 bytes
			"	.org	9b+480\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.l	(%0)+,(%1)+\n"
			"	move.w	(%0)+,(%1)+\n"
			"	move.b	(%0)+,(%1)+\n"
			"	add.l	%4,%1\n"
			"	add.l	%5,%0\n"
			"	dbra	%2,0b\n"
			"4:\n"
				: "+a"(src), "+a"(dst), "+d"(loopCount) // outputs
				: "g"(w * bytesPerPixel),
				  "r"(dstPitch - w * bytesPerPixel), "r"(srcPitch - w * bytesPerPixel) // inputs
				: "d0", "d1", "d2", "cc" AND_MEMORY
			);
			// WARNING: src and dst are modified by the asm code
		} else
#endif
		if (w * bytesPerPixel >= 4) {
			copyLong(dst, src, w * bytesPerPixel, h, dstPitch - w * bytesPerPixel, srcPitch - w * bytesPerPixel);
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
