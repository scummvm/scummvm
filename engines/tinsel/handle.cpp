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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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
#include "tinsel/scn.h"		// for the DW1 Mac resource handler
#include "tinsel/timers.h"	// for DwGetCurrentTime()
#include "tinsel/tinsel.h"
#include "tinsel/scene.h"

namespace Tinsel {

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

Handle::Handle() : _handleTable(0), _numHandles(0), _cdPlayHandle((uint32)-1), _cdBaseHandle(0), _cdTopHandle(0), _cdGraphStream(nullptr) {
}

Handle::~Handle() {
	free(_handleTable);
	_handleTable = nullptr;

	delete _cdGraphStream;
	_cdGraphStream = nullptr;
}

/**
 * Loads the graphics handle table index file and preloads all the
 * permanent graphics etc.
 */
void Handle::SetupHandleTable() {
	bool t2Flag = TinselV2;
	int RECORD_SIZE = t2Flag ? 24 : 20;

	int len;
	uint i;
	MEMHANDLE *pH;
	TinselFile f;

	const char *indexFileName = TinselV1PSX ? PSX_INDEX_FILENAME : INDEX_FILENAME;

	if (f.open(indexFileName)) {
		// get size of index file
		len = f.size();

		if (len > 0) {
			if ((len % RECORD_SIZE) != 0) {
				// index file is corrupt
				error(FILE_IS_CORRUPT, indexFileName);
			}

			// calc number of handles
			_numHandles = len / RECORD_SIZE;

			// allocate memory for the index file
			_handleTable = (MEMHANDLE *)calloc(_numHandles, sizeof(struct MEMHANDLE));

			// make sure memory allocated
			assert(_handleTable);

			// load data
			for (i = 0; i < _numHandles; i++) {
				f.read(_handleTable[i].szName, 12);
				_handleTable[i].filesize = f.readUint32();
				// The pointer should always be NULL. We don't
				// need to read that from the file.
				_handleTable[i]._node= nullptr;
				f.seek(4, SEEK_CUR);
				// For Discworld 2, read in the flags2 field
				_handleTable[i].flags2 = t2Flag ? f.readUint32() : 0;
			}

			if (f.eos() || f.err()) {
				// index file is corrupt
				error(FILE_IS_CORRUPT, indexFileName);
			}

			// close the file
			f.close();
		} else {	// index file is corrupt
			error(FILE_IS_CORRUPT, indexFileName);
		}
	} else {	// cannot find the index file
		error(CANNOT_FIND_FILE, indexFileName);
	}

	// allocate memory nodes and load all permanent graphics
	for (i = 0, pH = _handleTable; i < _numHandles; i++, pH++) {
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
			pH->_node= nullptr;
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

/**
 * Loads a memory block as a file.
 */
void Handle::OpenCDGraphFile() {
	delete _cdGraphStream;

	// As the theory goes, the right CD will be in there!

	_cdGraphStream = new Common::File;
	if (!_cdGraphStream->open(_szCdPlayFile))
		error(CANNOT_FIND_FILE, _szCdPlayFile.c_str());
}

void Handle::LoadCDGraphData(MEMHANDLE *pH) {
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
	assert(_cdGraphStream);
	_cdGraphStream->seek(_cdBaseHandle & OFFSETMASK, SEEK_SET);
	bytes = _cdGraphStream->read(addr, (_cdTopHandle - _cdBaseHandle) & OFFSETMASK);

	// New code to try and handle CD read failures 24/2/97
	while (bytes != ((_cdTopHandle - _cdBaseHandle) & OFFSETMASK) && retries++ < MAX_READ_RETRIES)	{
		// Try again
		_cdGraphStream->seek(_cdBaseHandle & OFFSETMASK, SEEK_SET);
		bytes = _cdGraphStream->read(addr, (_cdTopHandle - _cdBaseHandle) & OFFSETMASK);
	}

	// discardable - unlock the memory
	MemoryUnlock(pH->_node);

	// set the loaded flag
	pH->filesize |= fLoaded;

	// clear the loading flag
//	pH->filesize &= ~fLoading;

	if (bytes != ((_cdTopHandle - _cdBaseHandle) & OFFSETMASK))
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
void Handle::LoadExtraGraphData(SCNHANDLE start, SCNHANDLE next) {
	OpenCDGraphFile();

	MemoryDiscard((_handleTable + _cdPlayHandle)->_node); // Free it

	// It must always be the same
	assert(_cdPlayHandle == (start >> SCNHANDLE_SHIFT));
	assert(_cdPlayHandle == (next >> SCNHANDLE_SHIFT));

	_cdBaseHandle = start;
	_cdTopHandle = next;
}

void Handle::SetCdPlaySceneDetails(const char *fileName) {
	_szCdPlayFile = fileName;
}

void Handle::SetCdPlayHandle(int fileNum) {
	_cdPlayHandle = fileNum;
}

/**
 * Loads a memory block as a file.
 * @param pH			Memory block pointer
 */
void Handle::LoadFile(MEMHANDLE *pH) {
	Common::File f;
	char szFilename[sizeof(pH->szName) + 1];

	// extract and zero terminate the filename
	memcpy(szFilename, pH->szName, sizeof(pH->szName));
	szFilename[sizeof(pH->szName)] = 0;

	if (pH->filesize & fCompressed) {
		error("Compression handling has been removed - %s", szFilename);
	}

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
byte *Handle::LockMem(SCNHANDLE offset) {
	uint32 handle = offset >> SCNHANDLE_SHIFT;	// calc memory handle to use
	//debug("Locking offset of type %d (%x), offset %d, handle %d", (offset & HANDLEMASK) >> SCNHANDLE_SHIFT, (offset & HANDLEMASK) >> SCNHANDLE_SHIFT, offset & OFFSETMASK, handle);
	MEMHANDLE *pH;			// points to table entry

	// range check the memory handle
	assert(handle < _numHandles);

	pH = _handleTable + handle;

	if (pH->filesize & fPreload) {
		// permanent files are already loaded, nothing to be done
	} else if (handle == _cdPlayHandle) {
		// Must be in currently loaded/loadable range
		if (offset < _cdBaseHandle || offset >= _cdTopHandle)
			error("Overlapping (in time) CD-plays");

		// May have been discarded, if so, we have to reload
		if (!MemoryDeref(pH->_node)) {
			// Data was discarded, we have to reload
			MemoryReAlloc(pH->_node, _cdTopHandle - _cdBaseHandle);

			LoadCDGraphData(pH);

			// update the LRU time (new in this file)
			MemoryTouch(pH->_node);
		}

		// make sure address is valid
		assert(pH->filesize & fLoaded);

		offset -= _cdBaseHandle;
	} else {
		if (!MemoryDeref(pH->_node)) {
			// Data was discarded, we have to reload
			MemoryReAlloc(pH->_node, pH->filesize & FSIZE_MASK);

			if (TinselV2) {
				SetCD(pH->flags2 & fAllCds);
				CdCD(Common::nullContext);
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
void Handle::LockScene(SCNHANDLE offset) {

	uint32 handle = offset >> SCNHANDLE_SHIFT;	// calc memory handle to use
	MEMHANDLE *pH;					// points to table entry

	// range check the memory handle
	assert(handle < _numHandles);

	pH = _handleTable + handle;

	if ((pH->filesize & fPreload) == 0) {
		// Ensure the scene handle is allocated.
		MemoryReAlloc(pH->_node, pH->filesize & FSIZE_MASK);

		// Now lock it to make sure it stays allocated and in a fixed position.
		MemoryLock(pH->_node);
	}
}

/**
 * Called to make the current scene discardable again.
 * @param offset			Handle and offset to data
 */
void Handle::UnlockScene(SCNHANDLE offset) {

	uint32 handle = offset >> SCNHANDLE_SHIFT;	// calc memory handle to use
	MEMHANDLE *pH;					// points to table entry

	// range check the memory handle
	assert(handle < _numHandles);

	pH = _handleTable + handle;

	if ((pH->filesize & fPreload) == 0) {
		// unlock the scene data
		MemoryUnlock(pH->_node);
	}
}

/*----------------------------------------------------------------------*/

#ifdef BODGE

/**
 * Validates that a specified handle pointer is valid
 * @param offset			Handle and offset to data
 */
bool Handle::ValidHandle(SCNHANDLE offset) {
	uint32 handle = offset >> SCNHANDLE_SHIFT;	// calc memory handle to use
	MEMHANDLE *pH;					// points to table entry

	// range check the memory handle
	assert(handle < _numHandles);

	pH = _handleTable + handle;

	return (pH->filesize & FSIZE_MASK) != 8;
}
#endif

/**
 * TouchMem
 * @param offset			Handle and offset to data
 */
void Handle::TouchMem(SCNHANDLE offset) {
	MEMHANDLE *pH;					// points to table entry
	uint32 handle = offset >> SCNHANDLE_SHIFT;	// calc memory handle to use

	if (offset != 0) {
		pH = _handleTable + handle;

		// update the LRU time whether its loaded or not!
		if (pH->_node)
			MemoryTouch(pH->_node);
	}
}

/**
 * Returns true if the given handle is into the cd graph data
 * @param offset			Handle and offset to data
 */
bool Handle::IsCdPlayHandle(SCNHANDLE offset) {
	uint32 handle = offset >> SCNHANDLE_SHIFT;	// calc memory handle to use

	// range check the memory handle
	assert(handle < _numHandles);

	return (handle == _cdPlayHandle);
}

/**
 * Returns the CD for a given scene handle
 */
int Handle::CdNumber(SCNHANDLE offset) {
	uint handle = offset >> SCNHANDLE_SHIFT;	// calc memory handle to use

	// range check the memory handle
	assert(handle < _numHandles);

	MEMHANDLE *pH = _handleTable + handle;

	if (!TinselV2)
		return 1;

	return GetCD(pH->flags2 & fAllCds);
}

} // End of namespace Tinsel
