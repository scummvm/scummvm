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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/backends/platform/psp/osys_psp.cpp $
 * $Id: osys_psp.cpp 46126 2009-11-24 14:18:46Z fingolfin $
 *
 */

#include "common/scummsys.h"
#include "common/singleton.h"
#include "common/list.h"
#include "backends/platform/psp/psppixelformat.h"
#include "backends/platform/psp/memory.h"

// Class Copier --------------------------------------------------------------------------
//#define __PSP_DEBUG_FUNCS__	/* For debugging the stack */
//#define __PSP_DEBUG_PRINT__

#include "backends/platform/psp/trace.h"

//#define TEST_MEMORY_COPY

extern "C" {

void *__wrap_memcpy(void *dst, void *src, size_t bytes) {
	PspMemory::fastCopy((byte *)dst, (byte *)src, bytes);
	return dst;
}

}

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
		copy32Aligned((uint32 *)dst, (uint32 *)src, bytes);
	}

#ifdef TEST_MEMORY_COPY
	testCopy(debugDst, debugSrc, debugBytes);
#endif		
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

// 
// used to swap red and blue
void PspMemory::swap(uint16 *dst16, const uint16 *src16, uint32 bytes, PSPPixelFormat &format) {
	DEBUG_ENTER_FUNC();

#ifdef TEST_MEMORY_COPY
	uint32 debugBytes = bytes;
	const uint16 *debugDst = dst16, *debugSrc = src16;
#endif
	
	// align the destination pointer first
	uint32 prefixDst = (((uint32)dst16) & 0x3);	// for swap, we can only have 2 or 0 as our prefix
	
	if (prefixDst) {
		bytes -= prefixDst;						// remember we assume bytes > 4
		*dst16++ = format.swapRedBlue16(*src16++);
		
		if (bytes < MIN_AMOUNT_FOR_COMPLEX_COPY) { // check if it's worthwhile to continue
			swap16(dst16, src16, bytes, format);

#ifdef TEST_MEMORY_COPY
			testSwap(debugDst, debugSrc, debugBytes, format);
#endif		
			return;
		}
	}
	
	// check the source pointer alignment now
	uint32 alignSrc = (((uint32)src16) & 0x3);
	
	if (alignSrc) {						// we'll need to realign our reads
		PSP_DEBUG_PRINT("misaligned copy of %u bytes from %p to %p\n", bytes, src16, dst16);
		swap32Misaligned((uint32 *)dst16, src16, bytes, format);
	} else {
		swap32Aligned((uint32 *)dst16, (const uint32 *)src16, bytes, format);
	}
	
#ifdef TEST_MEMORY_COPY
	testSwap(debugDst, debugSrc, debugBytes, format);
#endif		
	
}

void PspMemory::testSwap(const uint16 *debugDst, const uint16 *debugSrc, uint32 debugBytes, PSPPixelFormat &format) {
	
	bool mismatch = false;
	PSP_INFO_PRINT("testing fastSwap...");
	
	uint32 shorts = debugBytes >> 1;

	for (uint32 i = 0; i < shorts; i++) {
		if (debugDst[i] != format.swapRedBlue16(debugSrc[i])) {
			if (!mismatch) {
				PSP_INFO_PRINT("**** mismatch in swap! ****\n");
				PSP_INFO_PRINT("dst[%p], src[%p], bytes[%u]\n", debugDst, debugSrc, debugBytes);
				mismatch = true;
			}
			PSP_INFO_PRINT("[%d]%x!=%x ", i<<1, format.swapRedBlue16(debugSrc[i]), debugDst[i]);
		}
	}
	if (mismatch) {
		PSP_INFO_PRINT("\n");
	} else {
		PSP_INFO_PRINT("ok\n");
	}	
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
	byte *src = (byte *)src32;
	
	while (bytesLeft--) {
		*dst++ = *src++;
	}
}

void PspMemory::swap32Aligned(uint32 *dst32, const uint32 *src32, uint32 bytes, PSPPixelFormat &format) {
	DEBUG_ENTER_FUNC();
	int words4 = bytes >> 4;
	
	// try blocks of 4 words at a time
	while (words4--) {
		uint32 a, b, c, d;
		a = format.swapRedBlue32(src32[0]);
		b = format.swapRedBlue32(src32[1]);
		c = format.swapRedBlue32(src32[2]);
		d = format.swapRedBlue32(src32[3]);
		dst32[0] = a;
		dst32[1] = b;
		dst32[2] = c;
		dst32[3] = d;
		dst32 += 4;
		src32 += 4;
	}

	uint32 bytesLeft = bytes & 0xF;
	uint32 words = bytesLeft >> 2;
	
	// now just do words
	while (words--) {
		*dst32++ = format.swapRedBlue32(*src32++);
	}	

	bytesLeft = bytes & 0x3;
	
	if (bytesLeft) {	// for swap, can only be 1 short left
		*((uint16 *)dst32) = format.swapRedBlue16(*((uint16 *)src32));
	}
}


// More challenging -- need to shift
// Assume dst is aligned
void PspMemory::copy32Misaligned(uint32 *dst32, const byte *src, uint32 bytes, uint32 alignSrc) {
	PSP_DEBUG_PRINT("copy32Misaligned: dst32[%p], src[%p], bytes[%d], alignSrc[%d]\n", dst32, src, bytes, alignSrc);
	
	uint32 *src32 = (uint32 *)(((uint32)src) & 0xFFFFFFFC);	// remove misalignment
	uint32 offset;
	
	switch (alignSrc) {
	case 1:
		offset = misaligned32Detail(dst32, src32, bytes, alignSrc, 8, 24);
		break;
	case 2:
		offset = misaligned32Detail(dst32, src32, bytes, alignSrc, 16, 16);
		break;
	default: /* 3 */
		offset = misaligned32Detail(dst32, src32, bytes, alignSrc, 24, 8);
		break;
	}
	
	uint32 remainingBytes = bytes & 3;
	
	if (remainingBytes) {
		byte *dst = (byte *)dst32;
		src += offset;
		dst += offset;
		copy8(dst, src, remainingBytes);
	}	
}

// returns offset in dst
uint32 PspMemory::misaligned32Detail(uint32 *dst32, uint32 *src32, uint32 bytes, uint32 alignSrc, const uint32 shiftValue, const uint32 lastShiftValue) {
	uint32 *origDst32 = dst32;
	register uint32 dstWord, srcWord;
	
	PSP_DEBUG_PRINT("misaligned32Detail(): alignSrc[%d], dst32[%p], src32[%p], bytes[%d]\n", alignSrc, dst32, src32, bytes);
	
	// Try to do groups of 4 words
	uint32 words4 = bytes >> 4;
	
	srcWord = src32[0];

	while (words4--) {
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
	
	uint32 words = (bytes & 0xF) >> 2;
	
	// we read one word ahead of what we write
	// setup the first read
	if (words) {
		src32++;	// we already loaded the value, so just increment
		
		while (words--) {
			dstWord = srcWord >> shiftValue;
			srcWord = *src32++;
			dstWord |= srcWord << lastShiftValue;
			*dst32++ = dstWord;
		}
	}
	
	return (byte *)dst32 - (byte *)origDst32;
}

// More challenging -- need to shift
// We assume dst is aligned
void PspMemory::swap32Misaligned(uint32 *dst32, const uint16 *src16, uint32 bytes, PSPPixelFormat &format) {
	DEBUG_ENTER_FUNC();

	const uint32 shiftValue = 16;
	uint32 *src32 = (uint32 *)(((uint32)src16) & 0xFFFFFFFC);	// remove misalignment
	
	// Try to do groups of 4 words
	uint32 words4 = bytes >> 4;
	uint32 srcWord = src32[0];	// preload

	while (words4--) {
		uint32 dstWord = srcWord >> shiftValue;
		srcWord = src32[1];
		dstWord |= srcWord << shiftValue;
		dst32[0] = format.swapRedBlue32(dstWord);
		dstWord = srcWord >> shiftValue;
		srcWord = src32[2];
		dstWord |= srcWord << shiftValue;
		dst32[1] = format.swapRedBlue32(dstWord);
		dstWord = srcWord >> shiftValue;
		srcWord = src32[3];
		dstWord |= srcWord << shiftValue;
		dst32[2] = format.swapRedBlue32(dstWord);
		dstWord = srcWord >> shiftValue;
		srcWord = src32[4];
		dstWord |= srcWord << shiftValue;
		dst32[3] = format.swapRedBlue32(dstWord);
		src32 += 4;
		dst32 += 4;
	}
	
	uint32 words = (bytes & 0xF) >> 2;
	
	// we read one word ahead of what we write
	// setup the first read
	if (words) {
		//srcWord = *src32++;	// don't need this. already loaded
		src32++;	// we already have the value loaded in
		
		while (words--) {
			uint32 dstWord = srcWord >> shiftValue;
			srcWord = *src32++;
			dstWord |= srcWord << shiftValue;
			*dst32++ = format.swapRedBlue32(dstWord);
		}
	}
	
	uint32 bytesLeft = bytes & 3;
	
	if (bytesLeft) {	// for swap, can only be 1 short left
		*((uint16 *)dst32) = format.swapRedBlue16((uint16)(srcWord >> shiftValue));
	}
}

inline void PspMemory::copy16(uint16 *dst16, const uint16 *src16, uint32 bytes) {
	PSP_DEBUG_PRINT("copy16(): dst16[%p], src16[%p], bytes[%d]\n", dst16, src16, bytes);
	
	uint32 shorts = bytes >> 1;
	uint32 remainingBytes = bytes & 1;

	for (; shorts > 0 ; shorts--) {
		*dst16++ = *src16++;
	}
	if (remainingBytes)
		*(byte *)dst16 = *(byte *)src16;
}

// Class VramAllocator -----------------------------------

DECLARE_SINGLETON(VramAllocator)

//#define __PSP_DEBUG_FUNCS__	/* For debugging the stack */
//#define __PSP_DEBUG_PRINT__

#include "backends/platform/psp/trace.h"


void *VramAllocator::allocate(int32 size, bool smallAllocation /* = false */) {
	DEBUG_ENTER_FUNC();
	assert(size > 0);

	byte *lastAddress = smallAllocation ? (byte *)VRAM_SMALL_ADDRESS : (byte *)VRAM_START_ADDRESS;
	Common::List<Allocation>::iterator i;

	// Find a block that fits, starting from the beginning
	for (i = _allocList.begin(); i != _allocList.end(); ++i) {
		byte *currAddress = (*i).address;

		if (currAddress - lastAddress >= size) // We found a match
			break;

		if ((*i).getEnd() > lastAddress)
			lastAddress = (byte *)(*i).getEnd();
	}

	if (lastAddress + size > (byte *)VRAM_END_ADDRESS) {
		PSP_DEBUG_PRINT("No space for allocation of %d bytes. %d bytes already allocated.\n",
		                size, _bytesAllocated);
		return NULL;
	}

	_allocList.insert(i, Allocation(lastAddress, size));
	_bytesAllocated += size;

	PSP_DEBUG_PRINT("Allocated in VRAM, size %u at %p.\n", size, lastAddress);
	PSP_DEBUG_PRINT("Total allocated %u, remaining %u.\n", _bytesAllocated, (2 * 1024 * 1024) - _bytesAllocated);

	return lastAddress;
}

// Deallocate a block from VRAM
void VramAllocator::deallocate(void *address) {
	DEBUG_ENTER_FUNC();
	address = (byte *)CACHED(address);	// Make sure all addresses are the same

	Common::List<Allocation>::iterator i;

	// Find the Allocator to deallocate
	for (i = _allocList.begin(); i != _allocList.end(); ++i) {
		if ((*i).address == address) {
			_bytesAllocated -= (*i).size;
			_allocList.erase(i);
			PSP_DEBUG_PRINT("Deallocated address[%p], size[%u]\n", (*i).address, (*i).size);
			return;
		}
	}

	PSP_DEBUG_PRINT("Address[%p] not allocated.\n", address);
}
