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

void Copier::copy(byte *dst, const byte *src, uint32 bytes, PSPPixelFormat *format /* = NULL */) {
	DEBUG_ENTER_FUNC();
	
	uint32 prefixDst = (((uint32)dst) & 0x3);
	prefixDst = prefixDst ? 4 - prefixDst : 0;	// prefix only if we have address % 4 != 0
	uint32 prefixSrc = (((uint32)src) & 0x3);
	prefixSrc = prefixSrc ? 4 - prefixSrc : 0;  // prefix only if we have address % 4 != 0
	uint32 *dst32, *src32;
	bool swapRB = format ? format->swapRB : false;	// take swap value from pixelformat if it's given
#ifdef __PSP_DEBUG_PRINT__
	uint32 debugBytes = bytes;
	const byte *debugDst = dst, *debugSrc = src;
#endif	
	uint32 words, remainingBytes;
	
	//PSP_DEBUG_PRINT("dst[%p], src[%p], bytes[%d], swap[%s], prefixDst[%u], prefixSrc[%u]\n", dst, src, bytes, swapRB ? "true" : "false", prefixDst, prefixSrc);
	
	if (prefixDst || prefixSrc) { 			// we're not aligned to word boundaries
		if (prefixDst != prefixSrc) {		// worst case: we can never be aligned. this mode is highly inefficient. try to get engines not to use this mode too much
			PSP_DEBUG_PRINT("misaligned copy of %u bytes from %p to %p\n", bytes, src, dst);
			if ((prefixDst & 1) || (prefixSrc & 1))
				copy8(dst, src, bytes);	// no swap is possible on 8 bit
			else
				copy16((uint16 *)dst, (uint16 *)src, bytes, format);
				
			goto test;	
		}
		
		// Do the prefix: the part to get us aligned
		if (prefixDst & 1) { 			// byte
			copy8(dst, src, prefixDst);	// no swap available
		} else {						// short
			copy16((uint16 *)dst, (uint16 *)src, prefixDst, format);
		}
		if (bytes > prefixDst)	// check that we can afford to subtract from bytes
			bytes -= prefixDst;
		else {
			DEBUG_EXIT_FUNC();	
			return;
		}
		dst32 = (uint32 *)(dst + prefixDst);
		src32 = (uint32 *)(src + prefixSrc);
	} else { // We're aligned to word boundaries
		dst32 = (uint32 *)dst;
		src32 = (uint32 *)src;
	}	
	
	words = bytes >> 2;
	remainingBytes = bytes & 0x3;
	
	if (swapRB) {	// need to swap
		for (; words > 0; words--) {
			*dst32 = format->swapRedBlue32(*src32);
			dst32++;
			src32++;
		}
	} else { // no swapping
		for (; words > 0; words--) {
			*dst32 = *src32;
			dst32++;
			src32++;
		}
	}

	// Do any remaining bytes
	if (remainingBytes) {
		if (remainingBytes & 1)	// we have bytes left
			copy8((byte *)dst32, (byte *)src32, remainingBytes);
		else // 16bits left
			copy16((uint16*)dst32, (uint16 *)src32, remainingBytes, format);
	}

test:	
	// debug
#ifdef __PSP_DEBUG_PRINT__
	bool mismatch = false;

	for (uint32 i=0; i<debugBytes; i++) {
		if (debugDst[i] != debugSrc[i]) { 
			if (mismatch == false) {
				PSP_DEBUG_PRINT_SAMELN("mismatch in copy:\n");
				PSP_DEBUG_PRINT("dst[%p], src[%p], bytes[%u], swap[%s], prefixDst[%u], prefixSrc[%u]\n", debugDst, debugSrc, debugBytes, swapRB ? "true" : "false", prefixDst, prefixSrc);
				mismatch = true;
			}
			PSP_DEBUG_PRINT_SAMELN("%x!=%x ", debugSrc[i], debugDst[i]);
		}
	}
	if (mismatch) 
		PSP_DEBUG_PRINT("\n");
#endif

	DEBUG_EXIT_FUNC();
}

inline void Copier::copy8(byte *dst, const byte *src, uint32 bytes) {
	for (; bytes > 0; bytes--) {
		*dst = *src;
		dst++;
		src++;
	}	
}

inline void Copier::copy16(uint16 *dst, const uint16 *src, uint32 bytes, PSPPixelFormat *format /* = NULL */) {
	uint32 shorts = bytes >> 1;
	uint32 remainingBytes = bytes & 1;
	bool swapRB = format ? format->swapRB : false;

	if (swapRB) {
		for (; shorts > 0 ; shorts--) {
			*dst = format->swapRedBlue16(*src);
			dst++;
			src++;
		}	
	} else {
		for (; shorts > 0 ; shorts--) {
			*dst = *src;
			dst++;
			src++;
		}
	}
	if (remainingBytes)
		*(byte *)dst = *(byte *)src;
}
 
 
// Class VramAllocator -----------------------------------

DECLARE_SINGLETON(VramAllocator)

//#define __PSP_DEBUG_FUNCS__	/* For debugging the stack */
//#define __PSP_DEBUG_PRINT__

#include "backends/platform/psp/trace.h"


void *VramAllocator::allocate(int32 size, bool smallAllocation /* = false */) {
	DEBUG_ENTER_FUNC();
	assert (size > 0);
	
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
	
	DEBUG_EXIT_FUNC();
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
			DEBUG_EXIT_FUNC();
			return;
		}
	}	
	
	PSP_DEBUG_PRINT("Address[%p] not allocated.\n", address);
	DEBUG_EXIT_FUNC();
}		
