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
 * This file contains the handle based Memory Manager code
 */

#define BODGE

#include "common/file.h"

#include "tinsel/dw.h"
#include "tinsel/scn.h"			// name of "index" file
#include "tinsel/handle.h"
#include "tinsel/heapmem.h"			// heap memory manager


// these are included only so the relevant structs can be used in convertLEStructToNative()
#include "tinsel/anim.h"
#include "tinsel/multiobj.h"
#include "tinsel/film.h"
#include "tinsel/object.h"
#include "tinsel/palette.h"
#include "tinsel/text.h"
#include "tinsel/scene.h"

namespace Tinsel {

//----------------- EXTERNAL GLOBAL DATA --------------------

#ifdef DEBUG
bool bLockedScene = 0;
#endif


//----------------- LOCAL DEFINES --------------------

struct MEMHANDLE {
	char szName[12];	//!< 00 - file name of graphics file
	int32 filesize;		//!< 12 - file size and flags
	MEM_NODE *pNode;	//!< 16 - memory node for the graphics
};


/** memory allocation flags - stored in the top bits of the filesize field */
enum {
	fPreload	= 0x01000000L,	//!< preload memory
	fDiscard	= 0x02000000L,	//!< discard memory
	fSound		= 0x04000000L,	//!< sound data
	fGraphic	= 0x08000000L,	//!< graphic data
	fCompressed	= 0x10000000L,	//!< compressed data
	fLoaded		= 0x20000000L	//!< set when file data has been loaded
};
#define	FSIZE_MASK	0x00FFFFFFL	//!< mask to isolate the filesize
#define	MALLOC_MASK	0xFF000000L	//!< mask to isolate the memory allocation flags
#define	OFFSETMASK	0x007fffffL	//!< get offset of address
//#define	HANDLEMASK		0xFF800000L	//!< get handle of address

//----------------- LOCAL GLOBAL DATA --------------------

// handle table gets loaded from index file at runtime
static MEMHANDLE *handleTable = 0;

// number of handles in the handle table
static uint numHandles = 0;


//----------------- FORWARD REFERENCES --------------------

static void LoadFile(MEMHANDLE *pH, bool bWarn);	// load a memory block as a file


/**
 * Loads the graphics handle table index file and preloads all the
 * permanent graphics etc.
 */
void SetupHandleTable(void) {
	enum { RECORD_SIZE = 20 };

	int len;
	uint i;
	MEMHANDLE *pH;
	Common::File f;

	if (f.open(INDEX_FILENAME)) {
		// get size of index file
		len = f.size();

		if (len > 0) {
			if ((len % RECORD_SIZE) != 0) {
				// index file is corrupt
				error("File %s is corrupt", INDEX_FILENAME);
			}

			// calc number of handles
			numHandles = len / RECORD_SIZE;

			// allocate memory for the index file
			handleTable = (MEMHANDLE *)calloc(numHandles, sizeof(struct MEMHANDLE));

			// make sure memory allocated
			assert(handleTable);

			// load data
			for (i = 0; i < numHandles; i++) {
				f.read(handleTable[i].szName, 12);
				handleTable[i].filesize = f.readUint32LE();
				// The pointer should always be NULL. We don't
				// need to read that from the file.
				handleTable[i].pNode = NULL;
				f.seek(4, SEEK_CUR);
			}

			if (f.ioFailed()) {
				// index file is corrupt
				error("File %s is corrupt", INDEX_FILENAME);
			}

			// close the file
			f.close();
		} else {	// index file is corrupt
			error("File %s is corrupt", INDEX_FILENAME);
		}
	} else {	// cannot find the index file
		error("Cannot find file %s", INDEX_FILENAME);
	}

	// allocate memory nodes and load all permanent graphics
	for (i = 0, pH = handleTable; i < numHandles; i++, pH++) {
		if (pH->filesize & fPreload) {
			// allocate a fixed memory node for permanent files
			pH->pNode = MemoryAlloc(DWM_FIXED, pH->filesize & FSIZE_MASK);

			// make sure memory allocated
			assert(pH->pNode);

			// load the data
			LoadFile(pH, true);
		}
#ifdef BODGE
		else if ((pH->filesize & FSIZE_MASK) == 8) {
			pH->pNode = NULL;
		}
#endif
		else {
			// allocate a discarded memory node for other files
			pH->pNode = MemoryAlloc(
				DWM_MOVEABLE | DWM_DISCARDABLE | DWM_NOALLOC,
				pH->filesize & FSIZE_MASK);

			// make sure memory allocated
			assert(pH->pNode);
		}
	}
}

void FreeHandleTable(void) {
	if (handleTable) {
		free(handleTable);
		handleTable = NULL;
	}
}

/**
 * Loads a memory block as a file.
 * @param pH			Memory block pointer
 * @param bWarn			If set, treat warnings as errors
 */
void LoadFile(MEMHANDLE *pH, bool bWarn) {
	Common::File f;
	char szFilename[sizeof(pH->szName) + 1];

	if (pH->filesize & fCompressed) {
		error("Compression handling has been removed!");
	}

	// extract and zero terminate the filename
	strncpy(szFilename, pH->szName, sizeof(pH->szName));
	szFilename[sizeof(pH->szName)] = 0;

	if (f.open(szFilename)) {
		// read the data
		int bytes;
		uint8 *addr;

		if (pH->filesize & fPreload)
			// preload - no need to lock the memory
			addr = (uint8 *)pH->pNode;
		else {
			// discardable - lock the memory
			addr = (uint8 *)MemoryLock(pH->pNode);
		}
#ifdef DEBUG
		if (addr == NULL) {
			if (pH->filesize & fPreload)
				// preload - no need to lock the memory
				addr = (uint8 *)pH->pNode;
			else {
				// discardable - lock the memory
				addr = (uint8 *)MemoryLock(pH->pNode);
			}
		}
#endif

		// make sure address is valid
		assert(addr);

		bytes = f.read(addr, pH->filesize & FSIZE_MASK);

		// close the file
		f.close();

		if ((pH->filesize & fPreload) == 0) {
			// discardable - unlock the memory
			MemoryUnlock(pH->pNode);
		}

		// set the loaded flag
		pH->filesize |= fLoaded;

		if (bytes == (pH->filesize & FSIZE_MASK)) {
			return;
		}

		if (bWarn)
			// file is corrupt
			error("File %s is corrupt", szFilename);
	}

	if (bWarn)
		// cannot find file
		error("Cannot find file %s", szFilename);
}

/**
 * Returns the address of a image, given its memory handle.
 * @param offset			Handle and offset to data
 */
uint8 *LockMem(SCNHANDLE offset) {
	uint32 handle = offset >> SCNHANDLE_SHIFT;	// calc memory handle to use
	MEMHANDLE *pH;			// points to table entry

	// range check the memory handle
	assert(handle < numHandles);

	pH = handleTable + handle;

	if (pH->filesize & fPreload) {
		// permanent files are already loaded
		return (uint8 *)pH->pNode + (offset & OFFSETMASK);
	} else {
		if (pH->pNode->pBaseAddr && (pH->filesize & fLoaded))
			// already allocated and loaded
			return pH->pNode->pBaseAddr + (offset & OFFSETMASK);
		
		if (pH->pNode->pBaseAddr == NULL)
			// must have been discarded - reallocate the memory
			MemoryReAlloc(pH->pNode, pH->filesize & FSIZE_MASK,
				DWM_MOVEABLE | DWM_DISCARDABLE);

		if (pH->pNode->pBaseAddr == NULL)
			error("Out of memory");

		LoadFile(pH, true);

		// make sure address is valid
		assert(pH->pNode->pBaseAddr);

		return pH->pNode->pBaseAddr + (offset & OFFSETMASK);
	}
}

/**
 * Called to make the current scene non-discardable.
 * @param offset			Handle and offset to data
 */
void LockScene(SCNHANDLE offset) {

	uint32 handle = offset >> SCNHANDLE_SHIFT;	// calc memory handle to use
	MEMHANDLE *pH;					// points to table entry

#ifdef DEBUG
	assert(!bLockedScene); // Trying to lock more than one scene
#endif

	// range check the memory handle
	assert(handle < numHandles);

	pH = handleTable + handle;

	// compact the heap to avoid fragmentation while scene is non-discardable
	HeapCompact(MAX_INT, false);

	if ((pH->filesize & fPreload) == 0) {
		// change the flags for the node
		MemoryReAlloc(pH->pNode, pH->filesize & FSIZE_MASK, DWM_MOVEABLE);
#ifdef DEBUG
		bLockedScene = true;
#endif
	}
}

/**
 * Called to make the current scene discardable again.
 * @param offset			Handle and offset to data
 */
void UnlockScene(SCNHANDLE offset) {

	uint32 handle = offset >> SCNHANDLE_SHIFT;	// calc memory handle to use
	MEMHANDLE *pH;					// points to table entry

	// range check the memory handle
	assert(handle < numHandles);

	pH = handleTable + handle;

	if ((pH->filesize & fPreload) == 0) {
		// change the flags for the node
		MemoryReAlloc(pH->pNode, pH->filesize & FSIZE_MASK, DWM_MOVEABLE | DWM_DISCARDABLE);
#ifdef DEBUG
		bLockedScene = false;
#endif
	}
}

/*----------------------------------------------------------------------*/

#ifdef BODGE

/**
 * Validates that a specified handle pointer is valid
 * @param offset			Handle and offset to data
 */
bool ValidHandle(SCNHANDLE offset) {
	uint32 handle = offset >> SCNHANDLE_SHIFT;	// calc memory handle to use
	MEMHANDLE *pH;					// points to table entry

	// range check the memory handle
	assert(handle < numHandles);

	pH = handleTable + handle;

	return (pH->filesize & FSIZE_MASK) != 8;
}
#endif

} // end of namespace Tinsel
