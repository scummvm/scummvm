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
 * This file contains the handle based Memory Manager code
 */

#define BODGE

#include "common/file.h"
#include "common/memstream.h"
#include "common/textconsole.h"
#include "common/str.h"

#include "tinsel/actors.h"
#include "tinsel/background.h"
#include "tinsel/drives.h"
#include "tinsel/dw.h"
#include "tinsel/handle.h"
#include "tinsel/heapmem.h"			// heap memory manager
#include "tinsel/palette.h"
#include "tinsel/sched.h"
#include "tinsel/timers.h"	// for DwGetCurrentTime()
#include "tinsel/tinsel.h"
#include "tinsel/scene.h"
#include "tinsel/noir/lzss.h"

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
	fLoaded		= 0x20000000L,	///< set when file data has been loaded
	fUnknown	= 0x40000000L	///< v3 specific
};
#define FSIZE_MASK	((TinselVersion == 3) ? 0xFFFFFFFFL : 0x00FFFFFFL)	//!< mask to isolate the filesize
#define MEMFLAGS(x) ((TinselVersion == 3) ? x->flags2 : x->filesize)
#define MEMFLAGSET(x, mask) ((TinselVersion == 3) ? x->flags2 |= mask : x->filesize |= mask)

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
	bool t2Flag = TinselVersion >= 2;
	int RECORD_SIZE = t2Flag ? 24 : 20;

	int len;
	uint i;
	MEMHANDLE *pH;
	TinselFile f(TinselV1Mac || TinselV1Saturn);

	Common::String indexFileName = TinselV1PSX ? PSX_INDEX_FILENAME : INDEX_FILENAME;

	if (Common::File::exists("index_"))
		indexFileName = "index_";

	if (f.open(indexFileName)) {
		// get size of index file
		len = f.size();

		if (len > 0) {
			if ((len % RECORD_SIZE) != 0) {
				// index file is corrupt
				error(FILE_IS_CORRUPT, indexFileName.c_str());
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
				error(FILE_IS_CORRUPT, indexFileName.c_str());
			}

			// close the file
			f.close();
		} else {	// index file is corrupt
			error(FILE_IS_CORRUPT, indexFileName.c_str());
		}
	} else {	// cannot find the index file
		error(CANNOT_FIND_FILE, indexFileName.c_str());
	}

	// allocate memory nodes and load all permanent graphics
	for (i = 0, pH = _handleTable; i < _numHandles; i++, pH++) {
		if (MEMFLAGS(pH) & fPreload) {
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
		error(CANNOT_FIND_FILE, _szCdPlayFile.toString().c_str());
}

void Handle::LoadCDGraphData(MEMHANDLE *pH) {
	// read the data
	uint bytes;
	byte *addr;
	int	retries = 0;

	assert(!(pH->filesize & fCompressed));

	// Can't be preloaded
	assert(!(MEMFLAGS(pH) & fPreload));

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
	MEMFLAGSET(pH, fLoaded);

	// clear the loading flag
//	pH->filesize &= ~fLoading;

	if (bytes != ((_cdTopHandle - _cdBaseHandle) & OFFSETMASK))
		// file is corrupt
		error(FILE_READ_ERROR, "CD play file");
}

/**
 * Called immediately preceding a CDplay().
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

	if ((TinselVersion != 3) && MEMFLAGS(pH) & fCompressed) {
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

		if ((TinselVersion == 3) && MEMFLAGS(pH) & fCompressed) {
			bytes = decompressLZSS(f, addr);
		} else {
			bytes = f.read(addr, pH->filesize & FSIZE_MASK);
		}

		// close the file
		f.close();

		// discardable - unlock the memory
		MemoryUnlock(pH->_node);

		// set the loaded flag
		MEMFLAGSET(pH, fLoaded);

		if (bytes == int(pH->filesize & FSIZE_MASK)) {
			return;
		}

		// file is corrupt
		error(FILE_IS_CORRUPT, szFilename);
	}

	// cannot find file
	error(CANNOT_FIND_FILE, szFilename);
}

/**
 * Return a font specified by a SCNHANDLE
 * Handles endianess internally
 * @param offset			Handle and offset to data
 * @return FONT structure
*/
FONT *Handle::GetFont(SCNHANDLE offset) {
	byte *data = LockMem(offset);
	const bool isBE = TinselV1Mac || TinselV1Saturn;
	const uint32 size = ((TinselVersion == 3) ? 12 * 4 : 11 * 4) + 300 * 4;	// FONT struct size
	Common::MemoryReadStreamEndian *stream = new Common::MemoryReadStreamEndian(data, size, isBE);

	FONT *font = new FONT();
	font->xSpacing = stream->readSint32();
	font->ySpacing = stream->readSint32();
	font->xShadow = stream->readSint32();
	font->yShadow = stream->readSint32();
	font->spaceSize = stream->readSint32();
	font->baseColor = (TinselVersion == 3) ? stream->readSint32() : 0;
	font->fontInit.hObjImg = stream->readUint32();
	font->fontInit.objFlags = stream->readSint32();
	font->fontInit.objID = stream->readSint32();
	font->fontInit.objX = stream->readSint32();
	font->fontInit.objY = stream->readSint32();
	font->fontInit.objZ = stream->readSint32();
	for (int i = 0; i < 300; i++)
		font->fontDef[i] = stream->readUint32();

	delete stream;

	return font;
}

/**
 * Return a palette specified by a SCNHANDLE
 * Handles endianess internally
 * @param offset			Handle and offset to data
 * @return PALETTE structure
*/
PALETTE *Handle::GetPalette(SCNHANDLE offset) {
	byte *data = LockMem(offset);
	const bool isBE = TinselV1Mac || TinselV1Saturn;
	const uint32 size = 4 + 256 * 4;	// numColors + 256 COLORREF (max)
	Common::MemoryReadStreamEndian *stream = new Common::MemoryReadStreamEndian(data, size, isBE);

	PALETTE *pal = new PALETTE();

	pal->numColors = stream->readSint32();
	for (int32 i = 0; i < pal->numColors; i++) {
		pal->palRGB[i] = stream->readUint32();

		// get the RGB color model values
		pal->palette[i * 3] = (byte)(pal->palRGB[i] & 0xFF);
		pal->palette[i * 3 + 1] = (byte)((pal->palRGB[i] >> 8) & 0xFF);
		pal->palette[i * 3 + 2] = (byte)((pal->palRGB[i] >> 16) & 0xFF);
	}

	delete stream;

	return pal;
}

/**
 * Return an image specified by a SCNHANDLE
 * Handles endianess internally
 * @param offset			Handle and offset to data
 * @return IMAGE structure
*/
const IMAGE *Handle::GetImage(SCNHANDLE offset) {
	byte *data = LockMem(offset);
	const bool isBE = TinselV1Mac || TinselV1Saturn;
	const uint32 size = 16; // IMAGE struct size

	Common::MemoryReadStreamEndian *stream = new Common::MemoryReadStreamEndian(data, size, isBE);

	IMAGE *img = new IMAGE();

	img->imgWidth = stream->readSint16();
	img->imgHeight = stream->readUint16();
	img->anioffX = stream->readSint16();
	img->anioffY = stream->readSint16();
	img->hImgBits = stream->readUint32();

	if (TinselVersion != 3) {
		img->hImgPal = stream->readUint32();
	} else {
		img->isRLE = stream->readSint16();
		img->colorFlags = stream->readSint16();
	}

	delete stream;

	return img;
}

void Handle::SetImagePalette(SCNHANDLE offset, SCNHANDLE palHandle) {
	byte *img = LockMem(offset);
	WRITE_32(img + 12, palHandle); // hImgPal
}

SCNHANDLE Handle::GetFontImageHandle(SCNHANDLE offset) {
	FONT *font = GetFont(offset);
	SCNHANDLE handle = font->fontInit.hObjImg;
	delete font;

	return handle;
}

/**
 * Return an actor's data specified by a SCNHANDLE
 * Handles endianess internally
 * @param offset			Handle and offset to data
 * @param count				Data count
 * @return IMAGE structure
*/
const ACTORDATA *Handle::GetActorData(SCNHANDLE offset, uint32 count) {
	byte *data = LockMem(offset);
	const bool isBE = TinselV1Mac || TinselV1Saturn;
	const uint32 size = (TinselVersion >= 2) ? 20 : 12; // ACTORDATA struct size

	Common::MemoryReadStreamEndian *stream = new Common::MemoryReadStreamEndian(data, size * count, isBE);

	ACTORDATA *actorData = new ACTORDATA[count];

	for (uint32 i = 0; i < count; i++) {
		if (TinselVersion <= 1) {
			actorData[i].masking = stream->readSint32();
			actorData[i].hActorId = stream->readUint32();
			actorData[i].hActorCode = stream->readUint32();
		} else {
			actorData[i].hActorId = stream->readUint32();
			actorData[i].hTagText = stream->readUint32();
			actorData[i].tagPortionV = stream->readSint32();
			actorData[i].tagPortionH = stream->readSint32();
			actorData[i].hActorCode = stream->readUint32();
		}
	}

	delete stream;

	return actorData;
}

/**
 * Return a process specified by a SCNHANDLE
 * Handles endianess internally
 * @param offset			Handle and offset to data
 * @param count				Data count
 * @return PROCESS_STRUC structure
*/
const PROCESS_STRUC *Handle::GetProcessData(SCNHANDLE offset, uint32 count) {
	byte *data = LockMem(offset);
	const bool isBE = TinselV1Mac || TinselV1Saturn;
	const uint32 size = 8; // PROCESS_STRUC struct size

	Common::MemoryReadStreamEndian *stream = new Common::MemoryReadStreamEndian(data, size * count, isBE);

	PROCESS_STRUC *processData = new PROCESS_STRUC[count];

	for (uint32 i = 0; i < count; i++) {
		processData[i].processId = stream->readUint32();
		processData[i].hProcessCode = stream->readUint32();
	}

	delete stream;

	return processData;
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

	if (MEMFLAGS(pH) & fPreload) {
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
		assert(MEMFLAGS(pH) & fLoaded);

		offset -= _cdBaseHandle;
	} else {
		if (!MemoryDeref(pH->_node)) {
			// Data was discarded, we have to reload
			MemoryReAlloc(pH->_node, pH->filesize & FSIZE_MASK);

			if (TinselVersion >= 2) {
				SetCD(pH->flags2 & fAllCds);
				CdCD(Common::nullContext);
			}
			LoadFile(pH);
		}

		// make sure address is valid
		assert(MEMFLAGS(pH) & fLoaded);
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

	if ((MEMFLAGS(pH) & fPreload) == 0) {
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

	if ((MEMFLAGS(pH) & fPreload) == 0) {
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

	if (TinselVersion <= 1)
		return 1;

	return GetCD(pH->flags2 & fAllCds);
}

/**
  * Searches for a resource by name and returns the handle to it.
  *
  * @param fileName Name of the resource to search for
  */
SCNHANDLE Handle::FindLanguageSceneHandle(const char *fileName) {
	Common::String nameString{fileName};

	for (uint i = 0; i < _numHandles; ++i) {
		if (nameString == Common::String{_handleTable[i].szName}) {
			return i << SCNHANDLE_SHIFT;
		}
	}
	error("Can't find handle for language scene\n");
}

} // End of namespace Tinsel
