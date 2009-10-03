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
 * $URL$
 * $Id$
 *
 */

#include "sci/sci.h"
#include "sci/gui/gui_helpers.h"
#include "sci/gui/gui_memmgr.h"

namespace Sci {

static tagHandle *pHandles = 0;
static int16 nHandles = 0;
static byte *_heap = 0;
SCIHANDLE _firstfree = 0;
Common::HashMap<HEAPHANDLE, tagHeapInfo> heapInfo;
#define HEAP_BOTTOM 1000

//----------------------
void FreeMem(void) {
	// deleting handles
	if (pHandles) {
		for (uint16 i = 0; i < nHandles; i++)
			hunkDisposeHandle(i);
		delete[] pHandles;
		pHandles = 0;
	}
	// deleting heap
	if (_heap) {
		delete[] _heap;
		_heap = 0;
	}
}
//----------------------
bool InitMem(int16 max_handles) {
	// heap
	_heap = new byte[HEAP_SIZE];
	memset(_heap, 0x55, HEAP_SIZE);
	_firstfree = HEAP_START;
	heapSetBlockSize(_firstfree, HEAP_SIZE - 1 - HEAP_START);
	heapSetBlockNext(_firstfree, (SCIHANDLE)(HEAP_SIZE - 1));
	//hunk
	pHandles = new tagHandle[max_handles];
	if (pHandles) {
		nHandles = max_handles;
		memset(pHandles, 0, sizeof(tagHandle) * max_handles); // zerofy all
		return 1;
	}
	return 0;
}
//----------------------
SCIHANDLE hunkNeedHandle(uint16 size, uint32 resId) {
	SCIHANDLE newh = 0;
	// see if there is an empty handle available
	for (int16 i = 1; i < nHandles; i++)
		if (pHandles[i].ptr == 0) {
			newh = i;
			break;
		}
	// if unused handle was found - allocate memory and return it
	if (newh == nHandles)
		return 0;
	pHandles[newh].ptr = (byte*)malloc(size);
	assert(pHandles[newh].ptr);
	if (pHandles[newh].ptr) {
		pHandles[newh].size = size;
		pHandles[newh].resId = resId;
		debug(
				5,
				"   MemMgr: Requested %6db for res %08X, allocated handle %04X",
				size, resId, newh);
		return newh;
	}
	return 0;
}
//----------------------
void hunkDisposeHandle(SCIHANDLE handle) {
// 	if (handle > 0 && handle < nHandles && pHandles[handle].ptr) {
// 		debug(
// 				5,
// 				"   MemMgr: disposing handle 0x%04X, size=%8db, associated res %08X",
// 				handle, pHandles[handle].size, pHandles[handle].resId);
// 		free(pHandles[handle].ptr);
// 		pHandles[handle].ptr = 0;
// 		// deleting associated resource handler
// 		// Check that this don't fail on delete as ResGetLoaded could return 0;
// 		if (pHandles[handle].resId != 0xFFFFFFFF)
// 			delete g_sci->ResMgr.ResGetLoaded(pHandles[handle].resId);
// 	}
}
//----------------------
byte *hunk2Ptr(SCIHANDLE handle) {
	if (handle > 0 && handle < nHandles && pHandles[handle].ptr)
		return (byte *)pHandles[handle].ptr;
	return 0;
}
//----------------------
SCIHANDLE ptr2hunk(byte *ptr) {
	for (int i = 0; i < nHandles; i++) {
		if (ptr >= pHandles[i].ptr && ptr <= pHandles[i].ptr + pHandles[i].size)
			return i;
	}
return 0;
}
//----------------------
uint32 hunkHandleSize(SCIHANDLE handle) {
	if (handle > 0 && handle < nHandles && pHandles[handle].ptr)
		return pHandles[handle].size;
	return 0;
}
//----------------------
// int16 hunkResourceNum(SCIHANDLE handle) {
// 	if (handle > 0 && handle < nHandles && pHandles[handle].ptr)
// 		return RES_ID2NUM(pHandles[handle].resId);
// 	return -1;
// }

//----------------------
void hunkDump() {
	debug("\nMemMgr: Hunk dump:");
	uint32 size = 0;
	for (int i = 0; i < nHandles; i++)
		if (pHandles[i].ptr) {
			debug("  %04X, size=%8db, associated res %08X", i,
					pHandles[i].size, pHandles[i].resId);
			size += pHandles[i].size;
		}
	debug("Total memory allocated: %db", size);
	debug("End dump");
}

uint32 hunkUsed() {
	uint32 size = 0;
	for (int i = 0; i < nHandles; i++)
		if (pHandles[i].ptr)
			size += pHandles[i].size;
	return size;
}
//----------------------
// HEAP
//----------------------
//---------------------------------------------
// TODO : make sure that STRING, STACK and SCRIPT DATA is never allocated below HEAP_BOTTOM=1000
// otherwise it will cause problems with kernel string fuctions that assumes that anything below 1000 is res number
HEAPHANDLE heapNewPtr(uint16 size, kDataType type, const char *info) {
	if (size == 0)
		warning("Zero Heap Allocation Request!");

	HEAPHANDLE ptr, prev, next;
	ptr = prev = _firstfree;
	// 2 bytes block header header + block size must be odd
	size += 2 + (size & 1);
	// looking for an empty block to use
	uint16 blocksize;
	bool bBottomSafe = !(type == kDataString || type == kDataUnknown);

	while (ptr < HEAP_SIZE - 1) {
		blocksize = heapGetBlockSize(ptr);
		next = heapGetBlockNext(ptr);
		if (blocksize >= size) {
			if (bBottomSafe || (!bBottomSafe && ptr > HEAP_BOTTOM)) {
				if (blocksize <= size + 4) { // use all block
					size = blocksize;
					heapSetBlockNext(prev, next);
				} else { // split block in 2 blocks
					HEAPHANDLE newblock = ptr + size;
					heapSetBlockNext(prev, newblock);
					heapSetBlockSize(newblock, blocksize - size);
					heapSetBlockNext(newblock, next);
					next = newblock;
				}
				// setting allocated block
				heapSetBlockSize(ptr, size);
				// updating firstfree pointer
				if (ptr == _firstfree)
					_firstfree = next;
				setHeapInfo(ptr, type, info);
				return ptr;
			} //if (bBottomSafe || (!bBottomSafe && ptr>HEAP_BOTTOM))
			else { // !bottomsafe && ptr < HEAP_BOTTOM
				if (blocksize + ptr - HEAP_BOTTOM >= size) {
					// splitting the block into 3 parts
					// [2][ptr...999] [2][1002...1000+size-1] [2][1002+size...ptr+blocksize]
					//   free              returned              free
					heapSetBlockSize(ptr, HEAP_BOTTOM-ptr + 2);
					heapSetBlockSize(HEAP_BOTTOM+2, size);
					heapSetBlockSize(HEAP_BOTTOM+2 + size, blocksize - size
							- (HEAP_BOTTOM-ptr + 2));

					heapSetBlockNext(HEAP_BOTTOM+2 + size, next);
					heapSetBlockNext(ptr, HEAP_BOTTOM+2 + size);
					setHeapInfo(HEAP_BOTTOM+2, type, info);
					return HEAP_BOTTOM + 2;
				}
			}
		} // if (blocksize >= size)
		// block too small - moving to next one
		prev = ptr;
		ptr = next;
	}
	// allocation error - out of heap
	warning("Out of heap space");
	return 0;
}
//--------------------------------------------
void heapDisposePtr(HEAPHANDLE handle) {
	HEAPHANDLE prev, next;
	prev = next = _firstfree;
	// searching for prev and next free blocks (before & after deleted block)
	while (next < handle) {
		prev = next;
		next = heapGetBlockNext(prev);
	}
	// if deleted block is before 1st free space then it'll be 1st free space
	if (handle < _firstfree) {
		next = _firstfree;
		_firstfree = handle;
	} else
		heapSetBlockNext(prev, handle);

	heapSetBlockNext(handle, next);
	//Try to merge with previous
	if (prev + heapGetBlockSize(prev) == handle) {
		heapSetBlockSize(prev, heapGetBlockSize(prev)
				+ heapGetBlockSize(handle));
		heapSetBlockNext(prev, next);
		handle = prev;
	}
	//Try to merge with next
	if (handle + heapGetBlockSize(handle) == next && next != (HEAP_SIZE - 1)) {
		heapSetBlockSize(handle, heapGetBlockSize(handle) + heapGetBlockSize(
				next));
		heapSetBlockNext(handle, heapGetBlockNext(next));
	}
}
//-------------------------------------
uint16 heapFreeSize() {
	HEAPHANDLE free = _firstfree;
	uint16 szFree = 0;
	while (free < HEAP_SIZE - 1) {
		int size = heapGetBlockSize(free);
		free = heapGetBlockNext(free);
		szFree += size;
	}
	return szFree;
}
//-------------------------------------
uint16 heapLargestFreeSize() {
	HEAPHANDLE free = _firstfree;
	uint16 maxFree = 0;
	while (free < HEAP_SIZE - 1) {
		int size = heapGetBlockSize(free);
		free = heapGetBlockNext(free);
		if (size > maxFree)
			maxFree = size;
	}
	return maxFree ? maxFree - 2 : 0;
}
//-------------------------------------
void heapDump() {
	debug("\nMemMgr:Heap dump:");
	HEAPHANDLE ptr = HEAP_START;
	HEAPHANDLE free = _firstfree;

	uint16 szFree = 0, szUsed = 0;
	while (ptr < HEAP_SIZE - 1) {
		int size = heapGetBlockSize(ptr);
		if (ptr == free) {
			free = heapGetBlockNext(free);
			debug("  %04X %6u size:%6d FREE next=%04X", ptr, ptr,
					heapGetDataSize(ptr), free);
			szFree += size;
		} else {
			debug("  %04X %6u size:%6d USED", ptr, ptr, heapGetDataSize(ptr));
			szUsed += size;
		}
		ptr += size;
	}
	debug("Total %db used, %db free\nEnd Dump", szUsed, szFree);
}
//----------------------------
byte *heap2Ptr(HEAPHANDLE ptr) {
	return (ptr >= HEAP_START) ? (byte *)(_heap + ptr) : NULL;
}
//----------------------------
HEAPHANDLE ptr2heap(byte *ptr) {
	return ptr > _heap && ptr < (_heap + HEAP_SIZE) ? (HEAPHANDLE)(ptr - _heap)
			: 0;
}
//----------------------------
uint16 heapGetBlockSize(HEAPHANDLE ptr) {
	return READ_UINT16(_heap + ptr - 2);
}
//----------------------------
uint16 heapGetDataSize(HEAPHANDLE ptr) {
	return heapGetBlockSize(ptr) - 2;
}
//----------------------------
void heapFillPtr(HEAPHANDLE ptr, byte filler) {
	memset(heap2Ptr(ptr), filler, heapGetDataSize(ptr));
}
//----------------------------
void heapClearPtr(HEAPHANDLE ptr) {
	heapFillPtr(ptr, 0);
}
//----------------------------
void heapSetBlockSize(HEAPHANDLE ptr, uint16 nbytes) {
	WRITE_UINT16(_heap + ptr - 2, nbytes);
}
//----------------------------
HEAPHANDLE heapGetBlockNext(HEAPHANDLE ptr) {
	return READ_UINT16(_heap + ptr);
}
//----------------------------
void heapSetBlockNext(HEAPHANDLE ptr, SCIHANDLE next) {
	WRITE_UINT16(_heap + ptr, next);
}
//----------------------------
void heapDisposePtr(byte *ptr) {
	heapDisposePtr(ptr - _heap);
}
//---------------------------------------------
//
//
void setHeapInfo(HEAPHANDLE hnd, kDataType type, const char *szinfo) {
	tagHeapInfo info = { kDataUnknown, { 0 } };
	info.type = type;
	if (szinfo)
		strncpy(info.info, szinfo, 20);
	heapInfo.setVal(hnd, info);
}
//--------------------------------
bool saveMemState(Common::OutSaveFile *pFile) {
	pFile->writeString("MEMORY\n");
	// saving heap
	pFile->write(_heap, HEAP_SIZE);
	pFile->writeUint16LE(_firstfree);
	// TODO : process and write hunk handles
	//pFile->writeString("HUNK\n");
	return true;
}
//--------------------------------
bool restoreMemState(Common::InSaveFile *pFile) {
	if (pFile->readLine() != "MEMORY")
		return false;
	pFile->read(_heap, HEAP_SIZE);
	_firstfree = pFile->readUint16LE();

	return true;
}

//-------------------------------
}// end of namespace SCI
