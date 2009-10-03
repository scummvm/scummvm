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

#ifndef SCI_MEMMGR_H
#define SCI_MEMMGR_H

#include "common/savefile.h"
#include "common/endian.h"
#include "common/hashmap.h"

/*
 Memory management functions for SCI engine;
 All fuctions are simplified as there is no 16-bit limitations as in
 original SCI. Although it may cause problems.

 SCI Heap functionality.
 Heap size is limited to 65535 bytes to be consistent with SCI scripts
 Free block header is [wBlockSize][wNextFree]
 Used block header is [wBlockSize][data.....]
 ^-----returned HEAPHANDLE
 */

namespace Sci {

typedef uint16 SCIHANDLE;
typedef uint16 HEAPHANDLE;

enum {
	HEAP_SIZE	= 0x10000l,
	HEAP_START	= 2
};

struct tagHandle {
	byte *ptr; // data pointer
	uint32 size; // memory block size
	uint32 resId; // associated resource
};
//---------------------------------------------
// for debugger
enum kDataType {
	kDataUnknown = 0,
	kDataClass,
	kDataObject,
	kDataRect,
	kDataWindow,
	kDataPort,
	kDataString,
	kDataListHdr,
	kDataListNode,
	kDataCast,
	kDataPalette,
	kDataScript,
	kDataFile
};

struct tagHeapInfo {
	kDataType type;
	char info[21];
};

void setHeapInfo(HEAPHANDLE hnd, kDataType type, const char *info = 0);

bool InitMem(int16 max_handles);
void FreeMem(void);

// hunk functions
SCIHANDLE hunkNeedHandle(uint16 size, uint32 resId = 0xFFFFFFFF);
void hunkDisposeHandle(SCIHANDLE handle);

byte *hunk2Ptr(SCIHANDLE handle);
SCIHANDLE ptr2hunk(byte *ptr);
uint32 hunkHandleSize(SCIHANDLE handle);
int16 hunkResourceNum(SCIHANDLE handle);
void hunkDump();
uint32 hunkUsed();

// heap functions
HEAPHANDLE heapNewPtr(uint16 size, kDataType type/*=kDataUnknown*/,
		const char *info = 0);
void heapDisposePtr(HEAPHANDLE handle);
void heapDisposePtr(byte *ptr);
byte *heap2Ptr(HEAPHANDLE ptr);
HEAPHANDLE ptr2heap(byte *ptr);
uint16 heapGetBlockSize(HEAPHANDLE ptr);
uint16 heapGetDataSize(HEAPHANDLE ptr);
void heapFillPtr(HEAPHANDLE ptr, byte filler);
void heapClearPtr(HEAPHANDLE ptr);
void heapDump();
uint16 heapFreeSize();
uint16 heapLargestFreeSize();

void heapSetBlockSize(HEAPHANDLE ptr, uint16 nbytes);
HEAPHANDLE heapGetBlockNext(HEAPHANDLE ptr);
void heapSetBlockNext(HEAPHANDLE ptr, SCIHANDLE next);

bool saveMemState(Common::OutSaveFile *pFile);
bool restoreMemState(Common::InSaveFile *pFile);

} // end of namespace SCI

#endif
