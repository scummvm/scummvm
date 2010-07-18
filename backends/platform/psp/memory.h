
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

#ifndef PSP_MEMORY_H
#define PSP_MEMORY_H

#include "backends/platform/psp/psppixelformat.h"
#include "common/list.h"

#define UNCACHED(x)		((byte *)(((uint32)(x)) | 0x40000000))	/* make an uncached access */
#define CACHED(x)		((byte *)(((uint32)(x)) & 0xBFFFFFFF))	/* make an uncached access into a cached one */

#define MIN_AMOUNT_FOR_COMPLEX_COPY  8
#define MIN_AMOUNT_FOR_MISALIGNED_COPY 8

//#define __PSP_DEBUG_PRINT__

#include "backends/platform/psp/trace.h"

/**
 *	Class that does memory copying and swapping if needed
 */
class PspMemory {
private:
	static void testCopy(const byte *debugDst, const byte *debugSrc, uint32 debugBytes);
	static void copy(byte *dst, const byte *src, uint32 bytes);
	static void swap(uint16 *dst16, const uint16 *src16, uint32 bytes, PSPPixelFormat &format);
	static void copy32Aligned(uint32 *dst32, const uint32 *src32, uint32 bytes);
	static void swap32Aligned(uint32 *dst32, const uint32 *src32, uint32 bytes, PSPPixelFormat &format);
	static void copy32Misaligned(uint32 *dst32, const byte *src, uint32 bytes, uint32 alignSrc);
	static uint32 misaligned32Detail(uint32 *dst32, uint32 *src32, uint32 bytes, uint32 alignSrc, const uint32 shiftValue, const uint32 lastShiftValue);
	static void swap32Misaligned(uint32 *dst32, const uint16 *src16, uint32 bytes, PSPPixelFormat &format);
	static void copy16(uint16 *dst, const uint16 *src, uint32 bytes);

	// For swapping, we know that we have multiples of 16 bits
	static void swap16(uint16 *dst16, const uint16 *src16, uint32 bytes, PSPPixelFormat &format) {
		PSP_DEBUG_PRINT("swap16 called with dst16[%p], src16[%p], bytes[%d]\n", dst16, src16, bytes);
		uint32 shorts = bytes >> 1;

		for (; shorts > 0 ; shorts--) {
			*dst16++ = format.swapRedBlue16(*src16++);
		}
	}
	
	static void copy8(byte *dst, const byte *src, uint32 bytes) {
		PSP_DEBUG_PRINT("copy8 called with dst[%p], src[%p], bytes[%d]\n", dst, src, bytes);
		while (bytes--) {
			*dst++ = *src++;
		}
	}

public:	
	// This is the interface to the outside world
	static void fastCopy(byte *dst, const byte *src, uint32 bytes) {
		if (bytes < MIN_AMOUNT_FOR_COMPLEX_COPY) {
			copy8(dst, src, bytes);
		} else {	// go to more powerful copy
			copy(dst, src, bytes);
		}
	}

	static void fastSwap(byte *dst, const byte *src, uint32 bytes, PSPPixelFormat &format) {
		if (bytes < MIN_AMOUNT_FOR_COMPLEX_COPY * 2) {
			swap16((uint16 *)dst, (uint16 *)src, bytes, format);
		} else {	// go to more powerful copy
			swap((uint16 *)dst, (uint16 *)src, bytes, format);
		}
	}
};

/**
 *	Class that allocates memory in the VRAM
 */
class VramAllocator : public Common::Singleton<VramAllocator> {
public:
	VramAllocator() : _bytesAllocated(0) {}
	void *allocate(int32 size, bool smallAllocation = false);	// smallAllocation e.g. palettes
	void deallocate(void *pointer);

	static inline bool isAddressInVram(void *address) {
		if ((uint32)(CACHED(address)) >= VRAM_START_ADDRESS && (uint32)(CACHED(address)) < VRAM_END_ADDRESS)
			return true;
		return false;
	}


private:
	/**
	 *	Used to allocate in VRAM
	 */
	struct Allocation {
		byte *address;
		uint32 size;
		void *getEnd() { return address + size; }
		Allocation(void *Address, uint32 Size) : address((byte *)Address), size(Size) {}
		Allocation() : address(0), size(0) {}
	};

	enum {
		VRAM_START_ADDRESS = 0x04000000,
		VRAM_END_ADDRESS   = 0x04200000,
		VRAM_SMALL_ADDRESS = VRAM_END_ADDRESS - (4 * 1024)	// 4K in the end for small allocations
	};
	Common::List <Allocation> _allocList;		// List of allocations
	uint32 _bytesAllocated;
};

#endif /* PSP_MEMORY_H */
