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

#include "common/scummsys.h"
#include "common/singleton.h"
#include "backends/platform/psp/psppixelformat.h"
#include "backends/platform/psp/memory.h"

// Class Copier --------------------------------------------------------------------------
//#define __PSP_DEBUG_FUNCS__	/* For debugging the stack */
//#define __PSP_DEBUG_PRINT__

#include "backends/platform/psp/trace.h"

//#define TEST_MEMORY_COPY

void PspMemory::copy(byte *dst, const byte *src, uint32 bytes) {
	DEBUG_ENTER_FUNC();

#ifdef TEST_MEMORY_COPY
	uint32 debugBytes = bytes;
	const byte *debugDst = dst, *debugSrc = src;
#endif

	PSP_DEBUG_PRINT("copy(): dst[%p], src[%p], bytes[%d]\n", dst, src, bytes);

	// align the destination pointer first
	uint32 prefixDst = (((uint32)dst) & 0x3);

	if (prefixDst) {
		prefixDst = 4 - prefixDst;				// prefix only if we have address % 4 != 0
		PSP_DEBUG_PRINT("prefixDst[%d]\n", prefixDst);

		bytes -= prefixDst;						// remember we assume bytes >= 4

		if (bytes < MIN_AMOUNT_FOR_COMPLEX_COPY) {	// check if it's worthwhile to continue
			copy8(dst, src, bytes + prefixDst);
#ifdef TEST_MEMORY_COPY
			testCopy(debugDst, debugSrc, debugBytes);
#endif
			return;
		}

		while (prefixDst--) {
			*dst++ = *src++;
		}
	}

	// check the source pointer alignment now
	uint32 alignSrc = (((uint32)src) & 0x3);

	if (alignSrc) {						// we'll need to realign our reads
		copy32Misaligned((uint32 *)dst, src, bytes, alignSrc);
	} else {
		copy32Aligned((uint32 *)dst, (const uint32 *)src, bytes);
	}

#ifdef TEST_MEMORY_COPY
	testCopy(debugDst, debugSrc, debugBytes);
#endif
}

void PspMemory::copy32Aligned(uint32 *dst32, const uint32 *src32, uint32 bytes) {
	PSP_DEBUG_PRINT("copy32Aligned(): dst32[%p], src32[%p], bytes[%d]\n", dst32, src32, bytes);

	int words8 = bytes >> 5;

	// try blocks of 8 words at a time
	if (words8) {
		while (words8--) {
			uint32 a, b, c, d;
			a = src32[0];
			b = src32[1];
			c = src32[2];
			d = src32[3];
			dst32[0] = a;
			dst32[1] = b;
			dst32[2] = c;
			dst32[3] = d;
			a = src32[4];
			b = src32[5];
			c = src32[6];
			d = src32[7];
			dst32[4] = a;
			dst32[5] = b;
			dst32[6] = c;
			dst32[7] = d;
			dst32 += 8;
			src32 += 8;
		}
	}

	int words4 = (bytes & 0x1F) >> 4;

	// try blocks of 4 words at a time
	if (words4) {
		uint32 a, b, c, d;
		a = src32[0];
		b = src32[1];
		c = src32[2];
		d = src32[3];
		dst32[0] = a;
		dst32[1] = b;
		dst32[2] = c;
		dst32[3] = d;
		dst32 += 4;
		src32 += 4;
	}

	int bytesLeft = (bytes & 0xF);	// only look at bytes left after we did the above
	int wordsLeft = bytesLeft >> 2;

	// now just do single words
	while (wordsLeft) {
		*dst32++ = *src32++;
		wordsLeft--;
	}

	bytesLeft = bytes & 0x3;	// get remaining bytes

	PSP_DEBUG_PRINT("bytesLeft[%d]\n", bytesLeft);

	byte *dst = (byte *)dst32;
	const byte *src = (const byte *)src32;

	while (bytesLeft--) {
		*dst++ = *src++;
	}
}

// More challenging -- need to shift
// Assume dst is aligned
void PspMemory::copy32Misaligned(uint32 *dst32, const byte *src, uint32 bytes, uint32 alignSrc) {
	PSP_DEBUG_PRINT("copy32Misaligned: dst32[%p], src[%p], bytes[%d], alignSrc[%d]\n", dst32, src, bytes, alignSrc);

	uint32 *src32 = (uint32 *)(((uint32)src) & 0xFFFFFFFC);	// remove misalignment
	uint32 shiftValue, lastShiftValue;

	switch (alignSrc) {
	case 1:
		shiftValue = 8;
		lastShiftValue = 24;
		break;
	case 2:
		shiftValue = 16;
		lastShiftValue = 16;
		break;
	default: /* 3 */
		shiftValue = 24;
		lastShiftValue = 8;
		break;
	}

	uint32 dstWord, srcWord;

	// Try to do groups of 4 words
	uint32 words4 = bytes >> 4;

	srcWord = *src32;		// preload 1st word so we read ahead

	for (; words4; words4--) {
		dstWord = srcWord >> shiftValue;
		srcWord = src32[1];
		dstWord |= srcWord << lastShiftValue;
		dst32[0] = dstWord;
		dstWord = srcWord >> shiftValue;
		srcWord = src32[2];
		dstWord |= srcWord << lastShiftValue;
		dst32[1] = dstWord;
		dstWord = srcWord >> shiftValue;
		srcWord = src32[3];
		dstWord |= srcWord << lastShiftValue;
		dst32[2] = dstWord;
		dstWord = srcWord >> shiftValue;
		srcWord = src32[4];
		dstWord |= srcWord << lastShiftValue;
		dst32[3] = dstWord;
		src32 += 4;
		dst32 += 4;
	}

	uint32 words = (bytes & 0xF) >> 2;	// now get remaining words

	// we read one word ahead of what we write
	// setup the first read

	for (; words ;words--) {
		dstWord = srcWord >> shiftValue;
		srcWord = src32[1];				// we still go one ahead
		src32++;
		dstWord |= srcWord << lastShiftValue;
		*dst32++ = dstWord;
	}

	uint32 bytesLeft = bytes & 3;	// and remaining bytes

	if (bytesLeft) {
		byte *dst8 = (byte *)dst32;
		byte *src8 = ((byte *)src32) + ((uint32)src & 0x3);	// get exact location we should be at

		for(; bytesLeft; bytesLeft--) {
			*dst8++ = *src8++;
		}
	}
}

void PspMemory::testCopy(const byte *debugDst, const byte *debugSrc, uint32 debugBytes) {

	bool mismatch = false;
	PSP_INFO_PRINT("testing fastCopy...");

	for (uint32 i = 0; i < debugBytes; i++) {
		if (debugDst[i] != debugSrc[i]) {
			if (!mismatch) {
				PSP_INFO_PRINT("**** mismatch in copy! ****\n");
				PSP_INFO_PRINT("dst[%p], src[%p], bytes[%u]\n", debugDst, debugSrc, debugBytes);
				mismatch = true;
			}
			PSP_INFO_PRINT("[%d]%x!=%x ", i, debugSrc[i], debugDst[i]);
		}
	}
	if (mismatch) {
		PSP_INFO_PRINT("\n");
	} else {
		PSP_INFO_PRINT("ok\n");
	}
}

