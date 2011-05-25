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
 * This file contains the handle based Memory Manager code
 */

#define BODGE

#include "common/file.h"
#include "common/textconsole.h"

#include "tinsel/drives.h"
#include "tinsel/dw.h"
#include "tinsel/handle.h"
#include "tinsel/heapmem.h"			// heap memory manager
#include "tinsel/timers.h"	// for DwGetCurrentTime()
#include "tinsel/tinsel.h"
#include "tinsel/scene.h"

namespace Tinsel {

//----------------- EXTERNAL GLOBAL DATA --------------------

#ifdef DEBUG
static uint32 s_lockedScene = 0;
#endif


//----------------- LOCAL DEFINES --------------------

struct MEMHANDLE {
	char szName[12];	///< file name of graphics file
	int32 filesize;		///< file size and flags
	MEM_NODE *_node;	///< memory node for the graphics
	uint32 flags2;
};


/** memory allocation flags - stored in the top bits of the filesize field */
enum {
	fPreload	= 0x01000000L,	///< preload memory
	fDiscard	= 0x02000000L,	///< discard memory
	fSound		= 0x04000000L,	///< sound data
	fGraphic	= 0x08000000L,	///< graphic data
	fCompressed	= 0x10000000L,	///< compressed data
	fLoaded		= 0x20000000L	///< set when file data has been loaded
};
#define	FSIZE_MASK	0x00FFFFFFL	///< mask to isolate the filesize

//----------------- LOCAL GLOBAL DATA --------------------

// FIXME: Avoid non-const global vars

// handle table gets loaded from index file at runtime
static MEMHANDLE *handleTable = 0;

// number of handles in the handle table
static uint numHandles = 0;

static uint32 cdPlayHandle = (uint32)-1;

static SCNHANDLE cdBaseHandle = 0, cdTopHandle = 0;
static Common::File *cdGraphStream = 0;

static char szCdPlayFile[100];

//----------------- FORWARD REFERENCES --------------------

static void LoadFile(MEMHANDLE *pH);	// load a memory block as a file


/**
 * Loads the graphics handle table index file and preloads all the
 * permanent graphics etc.
 */
void SetupHandleTable() {
	bool t2Flag = (TinselVersion == TINSEL_V2);
	int RECORD_SIZE = t2Flag ? 24 : 20;

	int len;
	uint i;
	MEMHANDLE *pH;
	TinselFile f;

	if (f.open(TinselV1PSX? PSX_INDEX_FILENAME : INDEX_FILENAME)) {
		// get size of index file
		len = f.size();

		if (len > 0) {
			if ((len % RECORD_SIZE) != 0) {
				// index file is corrupt
				error(FILE_IS_CORRUPT, TinselV1PSX? PSX_INDEX_FILENAME : INDEX_FILENAME);
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
				handleTable[i].filesize = f.readUint32();
				// The pointer should always be NULL. We don't
				// need to read that from the file.
				handleTable[i]._node = NULL;
				f.seek(4, SEEK_CUR);
				// For Discworld 2, read in the flags2 field
				handleTable[i].flags2 = t2Flag ? f.readUint32() : 0;
			}

			if (f.eos() || f.err()) {
				// index file is corrupt
				error(FILE_IS_CORRUPT, (TinselV1PSX? PSX_INDEX_FILENAME : INDEX_FILENAME));
			}

			// close the file
			f.close();
		} else {	// index file is corrupt
			error(FILE_IS_CORRUPT, (TinselV1PSX? PSX_INDEX_FILENAME : INDEX_FILENAME));
		}
	} else {	// cannot find the index file
		error(CANNOT_FIND_FILE, (TinselV1PSX? PSX_INDEX_FILENAME : INDEX_FILENAME));
	}

	// allocate memory nodes and load all permanent graphics
	for (i = 0, pH = handleTable; i < numHandles; i++, pH++) {
		if (pH->filesize & fPreload) {
			// allocate a fixed memory node for permanent files
			pH->_node = MemoryAllocFixed((pH->filesize & FSIZE_MASK));

			// make sure memory allocated
			assert(pH->_node);

			// load the data
			LoadFile(pH);
		}
#ifdef BODGE
		else if ((pH->filesize & FSIZE_MASK) == 8) {
			pH->_node = NULL;
		}
#endif
		else {
			// allocate a discarded memory node for other files
			pH->_node = MemoryNoAlloc();

			// make sure memory allocated
			assert(pH->_node);
		}
	}
}

void FreeHandleTable() {
	free(handleTable);
	handleTable = NULL;

	delete cdGraphStream;
	cdGraphStream = NULL;
}

/**
 * Loads a memory block as a file.
 */
void OpenCDGraphFile() {
	delete cdGraphStream;

	// As the theory goes, the right CD will be in there!

	cdGraphStream = new Common::File;
	if (!cdGraphStream->open(szCdPlayFile))
		error(CANNOT_FIND_FILE, szCdPlayFile);
}

void LoadCDGraphData(MEMHANDLE *pH) {
	// read the data
	uint bytes;
	byte *addr;
	int	retries = 0;

	assert(!(pH->filesize & fCompressed));

	// Can't be preloaded
	assert(!(pH->filesize & fPreload));

	// discardable - lock the memory
	addr = (byte *)MemoryLock(pH->_node);

	// make sure address is valid
	assert(addr);

	// Move to correct place in file and load the required data
	assert(cdGraphStream);
	cdGraphStream->seek(cdBaseHandle & OFFSETMASK, SEEK_SET);
	bytes = cdGraphStream->read(addr, (cdTopHandle - cdBaseHandle) & OFFSETMASK);

	// New code to try and handle CD read failures 24/2/97
	while (bytes != ((cdTopHandle - cdBaseHandle) & OFFSETMASK) && retries++ < MAX_READ_RETRIES)	{
		// Try again
		cdGraphStream->seek(cdBaseHandle & OFFSETMASK, SEEK_SET);
		bytes = cdGraphStream->read(addr, (cdTopHandle - cdBaseHandle) & OFFSETMASK);
	}

	// discardable - unlock the memory
	MemoryUnlock(pH->_node);

	// set the loaded flag
	pH->filesize |= fLoaded;

	// clear the loading flag
//	pH->filesize &= ~fLoading;

	if (bytes != ((cdTopHandle - cdBaseHandle) & OFFSETMASK))
		// file is corrupt
		error(FILE_READ_ERROR, "CD play file");
}

/**
 * Called immediatly preceding a CDplay().
 * Prepares the ground so that when LockMem() is called, the
 * appropriate section of the extra scene file is loaded.
 * @param start			Handle of start of range
 * @param next			Handle of end of range + 1
 */
void LoadExtraGraphData(SCNHANDLE start, SCNHANDLE next) {
	OpenCDGraphFile();

	MemoryDiscard((handleTable + cdPlayHandle)->_node); // Free it

	// It must always be the same
	assert(cdPlayHandle == (start >> SCNHANDLE_SHIFT));
	assert(cdPlayHandle == (next >> SCNHANDLE_SHIFT));

	cdBaseHandle = start;
	cdTopHandle = next;
}

void SetCdPlaySceneDetails(int fileNum, const char *fileName) {
	strcpy(szCdPlayFile, fileName);
}

void SetCdPlayHandle(int fileNum) {
	cdPlayHandle = fileNum;
}


/**
 * Loads a memory block as a file.
 * @param pH			Memory block pointer
 */
void LoadFile(MEMHANDLE *pH) {
	Common::File f;
	char szFilename[sizeof(pH->szName) + 1];

	if (pH->filesize & fCompressed) {
		error("Compression handling has been removed");
	}

	// extract and zero terminate the filename
	memcpy(szFilename, pH->szName, sizeof(pH->szName));
	szFilename[sizeof(pH->szName)] = 0;

	if (f.open(szFilename)) {
		// read the data
		int bytes;
		uint8 *addr;

		// lock the memory
		addr = (uint8 *)MemoryLock(pH->_node);

		// make sure address is valid
		assert(addr);

		bytes = f.read(addr, pH->filesize & FSIZE_MASK);

		// close the file
		f.close();

		// discardable - unlock the memory
		MemoryUnlock(pH->_node);

		// set the loaded flag
		pH->filesize |= fLoaded;

		if (bytes == (pH->filesize & FSIZE_MASK)) {
			return;
		}

		// file is corrupt
		error(FILE_IS_CORRUPT, szFilename);
	}

	// cannot find file
	error(CANNOT_FIND_FILE, szFilename);
}

/**
 * Compute and return the address specified by a SCNHANDLE.
 * @param offset			Handle and offset to data
 */
byte *LockMem(SCNHANDLE offset) {
	uint32 handle = offset >> SCNHANDLE_SHIFT;	// calc memory handle to use
	MEMHANDLE *pH;			// points to table entry

	// range check the memory handle
	assert(handle < numHandles);

#ifdef DEBUG
	if (handle != s_lockedScene)
		warning("  Calling LockMem(0x%x), handle %d differs from active scene %d", offset, handle, s_lockedScene);
#endif

	pH = handleTable + handle;

	if (pH->filesize & fPreload) {
		// permanent files are already loaded, nothing to be done
	} else if (handle == cdPlayHandle) {
		// Must be in currently loaded/loadable range
		if (offset < cdBaseHandle || offset >= cdTopHandle)
			error("Overlapping (in time) CD-plays");

		// May have been discarded, if so, we have to reload
		if (!MemoryDeref(pH->_node)) {
			// Data was discarded, we have to reload
			MemoryReAlloc(pH->_node, cdTopHandle - cdBaseHandle);

			LoadCDGraphData(pH);

			// update the LRU time (new in this file)
			MemoryTouch(pH->_node);
		}

		// make sure address is valid
		assert(pH->filesize & fLoaded);

		offset -= cdBaseHandle;
	} else {
		if (!MemoryDeref(pH->_node)) {
			// Data was discarded, we have to reload
			MemoryReAlloc(pH->_node, pH->filesize & FSIZE_MASK);

			if (TinselV2) {
				SetCD(pH->flags2 & fAllCds);
				CdCD(nullContext);
			}
			LoadFile(pH);
		}

		// make sure address is valid
		assert(pH->filesize & fLoaded);
	}

	return MemoryDeref(pH->_node) + (offset & OFFSETMASK);
}

/**
 * Called to lock the current scene and make it non-discardable.
 * @param offset			Handle and offset to data
 */
void LockScene(SCNHANDLE offset) {

	uint32 handle = offset >> SCNHANDLE_SHIFT;	// calc memory handle to use
	MEMHANDLE *pH;					// points to table entry

#ifdef DEBUG
	assert(0 == s_lockedScene); // Trying to lock more than one scene
#endif

	// range check the memory handle
	assert(handle < numHandles);

	pH = handleTable + handle;

	if ((pH->filesize & fPreload) == 0) {
		// Ensure the scene handle is allocated.
		MemoryReAlloc(pH->_node, pH->filesize & FSIZE_MASK);

		// Now lock it to make sure it stays allocated and in a fixed position.
		MemoryLock(pH->_node);

#ifdef DEBUG
		s_lockedScene = handle;
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
		// unlock the scene data
		MemoryUnlock(pH->_node);

#ifdef DEBUG
		s_lockedScene = 0;
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

/**
 * TouchMem
 * @param offset			Handle and offset to data
 */
void TouchMem(SCNHANDLE offset) {
	MEMHANDLE *pH;					// points to table entry
	uint32 handle = offset >> SCNHANDLE_SHIFT;	// calc memory handle to use

	if (offset != 0) {
		pH = handleTable + handle;

		// update the LRU time whether its loaded or not!
		if (pH->_node)
			MemoryTouch(pH->_node);
	}
}

/**
 * Returns true if the given handle is into the cd graph data
 * @param offset			Handle and offset to data
 */
bool IsCdPlayHandle(SCNHANDLE offset) {
	uint32 handle = offset >> SCNHANDLE_SHIFT;	// calc memory handle to use

	// range check the memory handle
	assert(handle < numHandles);

	return (handle == cdPlayHandle);
}

/**
 * Returns the CD for a given scene handle
 */
int CdNumber(SCNHANDLE offset) {
	uint handle = offset >> SCNHANDLE_SHIFT;	// calc memory handle to use

	// range check the memory handle
	assert(handle < numHandles);

	MEMHANDLE *pH = handleTable + handle;

	if (!TinselV2)
		return 1;

	return GetCD(pH->flags2 & fAllCds);
}

} // End of namespace Tinsel
