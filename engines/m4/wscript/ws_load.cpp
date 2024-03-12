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

#include "m4/wscript/ws_load.h"
#include "m4/wscript/ws_machine.h"
#include "m4/wscript/wst_regs.h"
#include "m4/core/errors.h"
#include "m4/core/imath.h"
#include "m4/graphics/graphics.h"
#include "m4/mem/mem.h"
#include "m4/vars.h"

namespace M4 {

#define CHUNK_MACH	0x4D414348	//'MACH'
#define CHUNK_SEQU	0x53455155	//'SEQU'
#define CHUNK_DATA	0x44415441	//'DATA'
#define CHUNK_CELS	0x43454C53	//'CELS'

#define CHUNK_NECS	0x4E454353	//INTEL 'SCEN'
#define CHUNK_HCAM	0x4843414D	//INTEL 'MACH'
#define CHUNK_UQES	0x55514553	//INTEL 'SEQU'
#define CHUNK_SLEC	0x534C4543	//INTEL 'CELS'
#define CHUNK_ATAD	0x41544144	//INTEL 'DATA'

#define MACH_NUM_STATES		0
#define MACH_OFFSETS		1

#define SEQU_NUM_VARS		0
#define SEQU_SEQU_START		1

#define DATA_REC_COUNT		0
#define DATA_REC_SIZE		1
#define DATA_REC_START		2

#define MAX_ASSET_HASH		255

/**
 * This tries to encapsulate an uint32 * that also does endian conversion automatically.
 * GetNextInt32 can't just return values directly, because in the case of some chunk
 * types, the underlying data being pointed to gets byte swapped.
 */
class IntPointer {
private:
	int32 *_ptr = nullptr;

public:
	void set(int32 *ptr) { _ptr = ptr; }
	void swap() { *_ptr = SWAP_INT32(*_ptr); }
	int32 *ptr() const { return _ptr; }
	int32 operator*() const { return READ_LE_INT32(_ptr); }
};

static int32 ProcessCELS(const char * /*assetName*/, char **parseAssetPtr, char * /*mainAssetPtr*/, char *endOfAssetBlock,
	int32 **dataOffset, int32 **palDataOffset, RGB8 *myPalette);
static void RestoreSSPaletteInfo(RGB8 *myPalette, int32 *palPtr);

static bool GetNextint32(char **assetPtr, char *endOfAssetBlock, IntPointer &returnVal) {
	// Check to see if we still have an int32 available
	if ((endOfAssetBlock - *assetPtr) < 4) {
		return false;
	}

	// Get the next int32
	returnVal.set((int32 *)*assetPtr);
	*assetPtr += 4;

	return true;
}

bool InitWSAssets() {
	int32 i;

	// Make sure this is only called once.
	if (_GWS(wsloaderInitialized)) {
		error_show(FL, 'WSSN');
	}

	// Allocate space for the tables used by the loader and the resource io MACHine tables
	if ((_GWS(globalMACHnames) = (char **)mem_alloc(sizeof(char *) * 256, "MACH resource table")) == nullptr) {
		return false;
	}
	if ((_GWS(globalMACHHandles) = (MemHandle *)mem_alloc(sizeof(Handle) * 256, "CELS Handles table")) == nullptr) {
		return false;
	}
	if ((_GWS(globalMACHoffsets) = (int32 *)mem_alloc(sizeof(int32 *) * 256, "MACH offsets table")) == nullptr) {
		return false;
	}
	for (i = 0; i < 256; i++) {
		_GWS(globalMACHnames)[i] = nullptr;
		_GWS(globalMACHHandles)[i] = nullptr;
		_GWS(globalMACHoffsets)[i] = -1;
	}

	// SEQUence tables
	if ((_GWS(globalSEQUnames) = (char **)mem_alloc(sizeof(char *) * 256, "SEQU resource table")) == nullptr) {
		return false;
	}
	if ((_GWS(globalSEQUHandles) = (MemHandle *)mem_alloc(sizeof(Handle) * 256, "CELS Handles table")) == nullptr) {
		return false;
	}
	if ((_GWS(globalSEQUoffsets) = (int32 *)mem_alloc(sizeof(int32 *) * 256, "SEQU offsets table")) == nullptr) {
		return false;
	}
	for (i = 0; i < 256; i++) {
		_GWS(globalSEQUnames)[i] = nullptr;
		_GWS(globalSEQUHandles)[i] = nullptr;
		_GWS(globalSEQUoffsets)[i] = -1;
	}

	// DATA tables
	if ((_GWS(globalDATAnames) = (char **)mem_alloc(sizeof(char *) * 256, "DATA resource table")) == nullptr) {
		return false;
	}
	if ((_GWS(globalDATAHandles) = (MemHandle *)mem_alloc(sizeof(Handle) * 256, "CELS Handles table")) == nullptr) {
		return false;
	}
	if ((_GWS(globalDATAoffsets) = (int32 *)mem_alloc(sizeof(int32 *) * 256, "DATA offsets table")) == nullptr) {
		return false;
	}
	for (i = 0; i < 256; i++) {
		_GWS(globalDATAnames)[i] = nullptr;
		_GWS(globalDATAHandles)[i] = nullptr;
		_GWS(globalDATAoffsets)[i] = -1;
	}

	// CELS tables
	if ((_GWS(globalCELSnames) = (char **)mem_alloc(sizeof(char *) * 256, "CELS resource table")) == nullptr) {
		return false;
	}
	if ((_GWS(globalCELSHandles) = (MemHandle *)mem_alloc(sizeof(MemHandle *) * 256, "CELS Handles table")) == nullptr) {
		return false;
	}
	if ((_GWS(globalCELSoffsets) = (int32 *)mem_alloc(sizeof(int32 *) * 256, "CELS offsets table")) == nullptr) {
		return false;
	}
	if ((_GWS(globalCELSPaloffsets) = (int32 *)mem_alloc(sizeof(int32 *) * 256, "CELS pal offsets table")) == nullptr) {
		return false;
	}
	for (i = 0; i < 256; i++) {
		_GWS(globalCELSnames)[i] = nullptr;
		_GWS(globalCELSHandles)[i] = nullptr;
		_GWS(globalCELSoffsets)[i] = -1;
		_GWS(globalCELSPaloffsets)[i] = -1;
	}

	// Set the global to indicate the loader is active
	_GWS(wsloaderInitialized) = true;

	return true;
}

bool ClearWSAssets(uint32 assetType, int32 minHash, int32 maxHash) {
	int32 i;
	assert(maxHash >= minHash);

	if (!_GWS(wsloaderInitialized)) {
		return false;
	}

	// Bounds checking
	if (minHash < 0)
		minHash = 0;
	if (maxHash > MAX_ASSET_HASH)
		maxHash = MAX_ASSET_HASH;

	switch (assetType) {
	case _WS_ASSET_MACH:
		// Clear the machines table for entries [minHash, maxHash]
		for (i = minHash; i <= maxHash; i++) {
			terminateMachinesByHash(i);
			if (_GWS(globalMACHnames)[i]) {
				rtoss(_GWS(globalMACHnames)[i]);
				mem_free(_GWS(globalMACHnames)[i]);
				_GWS(globalMACHnames)[i] = nullptr;
				_GWS(globalMACHHandles)[i] = nullptr;
				_GWS(globalMACHoffsets)[i] = -1;
			}
		}
		break;

	case _WS_ASSET_SEQU:
		// Clear the sequences table for entries [minHash, maxHash]
		for (i = minHash; i <= maxHash; i++) {
			if (_GWS(globalSEQUnames)[i]) {
				rtoss(_GWS(globalSEQUnames)[i]);
				mem_free(_GWS(globalSEQUnames)[i]);
				_GWS(globalSEQUnames)[i] = nullptr;
				_GWS(globalSEQUHandles)[i] = nullptr;
				_GWS(globalSEQUoffsets)[i] = -1;
			}
		}
		break;

	case _WS_ASSET_DATA:
		// Clear the data table for entries [minHash, maxHash]
		for (i = minHash; i <= maxHash; i++) {
			if (_GWS(globalDATAnames)[i]) {
				rtoss(_GWS(globalDATAnames)[i]);
				mem_free(_GWS(globalDATAnames)[i]);
				_GWS(globalDATAnames)[i] = nullptr;
				_GWS(globalDATAHandles)[i] = nullptr;
				_GWS(globalDATAoffsets)[i] = -1;
			}
		}
		break;

	case _WS_ASSET_CELS:
		// Clear the cels tables for entries [minHash, maxHash]
		for (i = minHash; i <= maxHash; i++) {
			if (_GWS(globalCELSnames)[i]) {
				rtoss(_GWS(globalCELSnames)[i]);
				mem_free(_GWS(globalCELSnames)[i]);
				_GWS(globalCELSnames)[i] = nullptr;
				_GWS(globalCELSHandles)[i] = nullptr;
				_GWS(globalCELSoffsets)[i] = -1;
				_GWS(globalCELSPaloffsets)[i] = -1;
			}
		}
		break;

	default:
		return false;
	}
	return true;
}

void ShutdownWSAssets(void) {
	if (!_GWS(wsloaderInitialized))
		return;

	// For each asset type, clear the entire table
	ClearWSAssets(_WS_ASSET_MACH, 0, MAX_ASSET_HASH);
	ClearWSAssets(_WS_ASSET_SEQU, 0, MAX_ASSET_HASH);
	ClearWSAssets(_WS_ASSET_CELS, 0, MAX_ASSET_HASH);
	ClearWSAssets(_WS_ASSET_DATA, 0, MAX_ASSET_HASH);

	// Deallocate all tables
	if (_GWS(globalMACHnames)) mem_free(_GWS(globalMACHnames));
	if (_GWS(globalSEQUnames)) mem_free(_GWS(globalSEQUnames));
	if (_GWS(globalDATAnames)) mem_free(_GWS(globalDATAnames));
	if (_GWS(globalCELSnames)) mem_free(_GWS(globalCELSnames));

	if (_GWS(globalMACHHandles)) mem_free(_GWS(globalMACHHandles));
	if (_GWS(globalMACHoffsets)) mem_free(_GWS(globalMACHoffsets));
	if (_GWS(globalSEQUHandles)) mem_free(_GWS(globalSEQUHandles));
	if (_GWS(globalSEQUoffsets)) mem_free(_GWS(globalSEQUoffsets));
	if (_GWS(globalDATAHandles)) mem_free(_GWS(globalDATAHandles));
	if (_GWS(globalDATAoffsets)) mem_free(_GWS(globalDATAoffsets));
	if (_GWS(globalCELSHandles)) mem_free(_GWS(globalCELSHandles));
	if (_GWS(globalCELSoffsets)) mem_free(_GWS(globalCELSoffsets));
	if (_GWS(globalCELSPaloffsets)) mem_free(_GWS(globalCELSPaloffsets));

	_GWS(wsloaderInitialized) = false;
}

bool LoadWSAssets(const char *wsAssetName, RGB8 *myPalette) {
	MemHandle workHandle;
	char *mainAssetPtr, *parseAssetPtr, *endOfAssetBlock;
	uint32 *tempPtr;
	IntPointer chunkType, chunkSize, chunkHash;
	bool finished, chunkSwap;
	int32 *celsPtr, *palPtr;
	int32 i;
	int32 assetSize;

	// Check that the loader has been initialized
	if (!_GWS(wsloaderInitialized)) {
		error_show(FL, 'WSLI');
	}

	// Use the resource io manager to read in the entire block
	if ((workHandle = rget(wsAssetName, &assetSize)) == nullptr) {
		error_show(FL, 'FNF!', "Asset Name: %s", wsAssetName);
	}

	// Lock the handle so we can step through the chunk
	HLock(workHandle);
	mainAssetPtr = (char *)(*workHandle);

	endOfAssetBlock = (char *)((intptr)mainAssetPtr + assetSize);

	parseAssetPtr = mainAssetPtr;

	// Set the finished flag
	finished = false;

	// Get the first chunkType
	if (!GetNextint32(&parseAssetPtr, endOfAssetBlock, chunkType)) {
		finished = true;
	}

	// Process each chunk according to type
	while (!finished) {
		// Read in the chunk size and hash number
		if (!GetNextint32(&parseAssetPtr, endOfAssetBlock, chunkSize)) {
			error_show(FL, 'WSLE', "Asset Name: %s", wsAssetName);
		}
		if (!GetNextint32(&parseAssetPtr, endOfAssetBlock, chunkHash)) {
			error_show(FL, 'WSLE', "Asset Name: %s", wsAssetName);
		}

		// Process the chunk according to type
		chunkSwap = false;
		switch (*chunkType) {
		// Chunk is a machine chunk
		case CHUNK_HCAM:
			// Byte swap the type, size and hash and continue through case CHUNK_MACH.
			chunkType.swap();
			chunkSize.swap();
			chunkHash.swap();
			chunkSwap = true;
			// Fall through

		case CHUNK_MACH:
			// Check the validity of the machine hash number, and clear it
			if (*chunkHash > MAX_ASSET_HASH) {
				error_show(FL, 'WSLA', "Asset Name: %s, MACH hash was: %d", wsAssetName, *chunkHash);
			}
			ClearWSAssets(_WS_ASSET_MACH, *chunkHash, *chunkHash);

			// Store the resource name, and the offset into the resource block
			_GWS(globalMACHnames)[*chunkHash] = mem_strdup(wsAssetName);
			_GWS(globalMACHHandles)[*chunkHash] = workHandle;
			_GWS(globalMACHoffsets)[*chunkHash] = parseAssetPtr - mainAssetPtr;

			// Check that the assetblocksize is big enough that the chunk body was read in...
			if ((endOfAssetBlock - parseAssetPtr) < (int)(*chunkSize - 12)) {
				error_show(FL, 'WSLE', "Asset Name: %s, MACH hash was: %d", wsAssetName, *chunkHash);
			}

			// Byteswap the entire machine if necessary
			if (chunkSwap) {
				tempPtr = (uint32 *)parseAssetPtr;
				for (i = 0; i < (*chunkSize - 12) >> 2; i++) {	//>>2 - chunkSize is bytes, not int32s
					*tempPtr = SWAP_INT32(*tempPtr);
					tempPtr++;
				}
			}

			// Update the assetPtr to the beginning of the next chunk
			parseAssetPtr += *chunkSize - 12;
			break;

		case CHUNK_UQES:
			// Chunk is a machine chunk
			// Byte swap the type, size and hash and continue through case CHUNK_SEQU.
			chunkType.swap();
			chunkSize.swap();
			chunkHash.swap();
			chunkSwap = true;
			// Fall through

		case CHUNK_SEQU:
			// Check the validity of the sequence hash number, and clear it
			if (*chunkHash > MAX_ASSET_HASH) {
				error_show(FL, 'WSLA', "Asset Name: %s, SEQU hash was: %d", wsAssetName, *chunkHash);
			}
			ClearWSAssets(_WS_ASSET_SEQU, *chunkHash, *chunkHash);

			// Store the resource name, and the offset into the resource block
			_GWS(globalSEQUnames)[*chunkHash] = mem_strdup(wsAssetName);
			_GWS(globalSEQUHandles)[*chunkHash] = workHandle;
			_GWS(globalSEQUoffsets)[*chunkHash] = (intptr)parseAssetPtr - (intptr)mainAssetPtr;

			// Check that the assetblocksize is big enough that the chunk body was read in...
			if ((endOfAssetBlock - parseAssetPtr) < (int)(*chunkSize - 12)) {
				error_show(FL, 'WSLE', "Asset Name: %s, SEQU hash was: %d", wsAssetName, *chunkHash);
			}

			// Byteswap the entire sequence if necessary
			if (chunkSwap) {
				tempPtr = (uint32 *)parseAssetPtr;
				for (i = 0; i < (*chunkSize - 12) >> 2; i++) {	//>>2 - chunkSize is bytes, not int32s
					*tempPtr = SWAP_INT32(*tempPtr);
					tempPtr++;
				}
			}

			// Update the assetPtr to the beginning of the next chunk
			parseAssetPtr += *chunkSize - 12;
			break;

		case CHUNK_ATAD:
			// Chunk is a data chunk
			// Byte swap the type, size and hash and continue through case CHUNK_DATA.
			chunkType.swap();
			chunkSize.swap();
			chunkHash.swap();
			chunkSwap = true;
			// Fall through

		case CHUNK_DATA:
			// Check the validity of the data block hash number, and clear it
			if (*chunkHash > MAX_ASSET_HASH) {
				error_show(FL, 'WSLA', "Asset Name: %s, DATA hash was: %d", wsAssetName, *chunkHash);
			}
			ClearWSAssets(_WS_ASSET_DATA, *chunkHash, *chunkHash);

			// Store the resource name, and the offset into the resource block
			_GWS(globalDATAnames)[*chunkHash] = mem_strdup(wsAssetName);
			_GWS(globalDATAHandles)[*chunkHash] = workHandle;
			_GWS(globalDATAoffsets)[*chunkHash] = (intptr)parseAssetPtr - (intptr)mainAssetPtr;

			// Check that the assetblocksize is big enough that the chunk body was read in...
			if ((endOfAssetBlock - parseAssetPtr) < (int)(*chunkSize - 12)) {
				error_show(FL, 'WSLE', "Asset Name: %s, DATA hash was: %d", wsAssetName, *chunkHash);
			}

			// Byteswap the entire data block if necessary
			if (chunkSwap) {
				tempPtr = (uint32 *)parseAssetPtr;
				for (i = 0; i < (*chunkSize - 12) >> 2; i++) {	//>>2 - chunkSize is bytes, not int32s
					*tempPtr = SWAP_INT32(*tempPtr);
					tempPtr++;
				}
			}

			// Update the assetPtr to the beginning of the next chunk
			parseAssetPtr += *chunkSize - 12;
			break;

		case CHUNK_SLEC:
			// Byte swap the type, size and hash and continue through case CHUNK_CELS.
			chunkType.swap();
			chunkSize.swap();
			chunkHash.swap();
			chunkSwap = true;
			// Fall through

		case CHUNK_CELS: {
			// Check the validity of the cels hash number, and clear it
			if (*chunkHash > MAX_ASSET_HASH) {
				error_show(FL, 'WSLA', "Asset Name: %s, CELS hash was: %d", wsAssetName, *chunkHash);
			}

			ClearWSAssets(_WS_ASSET_CELS, *chunkHash, *chunkHash);

			// Store the resource name
			_GWS(globalCELSnames)[*chunkHash] = mem_strdup(wsAssetName);

			// Process the SS from the stream file
			if (ProcessCELS(wsAssetName, &parseAssetPtr, mainAssetPtr, endOfAssetBlock, &celsPtr, &palPtr, myPalette) < 0) {
				error_show(FL, 'WSLP', "Asset Name: %s, CELS hash was: %d", wsAssetName, *chunkHash);
			}

			// At this point, celsPtr points to the beginning of the cels data, palPtr to the pal data
			// Store the Handle, and calculate the offsets
			_GWS(globalCELSHandles)[*chunkHash] = workHandle;
			if (celsPtr) {
				_GWS(globalCELSoffsets)[*chunkHash] = (intptr)celsPtr - (intptr)mainAssetPtr;
			} else {
				_GWS(globalCELSoffsets)[*chunkHash] = -1;
			}
			if (palPtr) {
				_GWS(globalCELSPaloffsets)[*chunkHash] = (intptr)palPtr - (intptr)mainAssetPtr;
			} else {
				_GWS(globalCELSPaloffsets)[*chunkHash] = -1;
			}
			break;
		}

		default:
			error_show(FL, 'WSLT', "Asset Name: %s, %d bytes into the file.", wsAssetName,
				(intptr)parseAssetPtr - 12 - (intptr)mainAssetPtr);
			break;
		}

		// Read the next chunkType, or signal we are finished
		if (!GetNextint32(&parseAssetPtr, endOfAssetBlock, chunkType)) {
			finished = true;
		}
	}

	// Unlock the handle now
	HUnLock(workHandle);
	return true;
}

M4sprite *CreateSprite(MemHandle resourceHandle, int32 handleOffset, int32 index, M4sprite *mySprite, bool *streamSeries) {
	uint32 *myCelSource, *data, *offsets, numCels;
	uint32 *celsPtr;

	// Parameter verification
	if ((!resourceHandle) || (!*resourceHandle)) {
		ws_LogErrorMsg(FL, "No sprite source in memory.");
		return nullptr;
	}

	if (!mySprite) {
		mySprite = (M4sprite *)mem_alloc(sizeof(M4sprite), "Sprite");
		if (!mySprite) {
			ws_LogErrorMsg(FL, "Out of memory - mem requested: %d.", sizeof(M4sprite));
			return nullptr;
		}
	}

	// Find the cels source  from the asset block
	HLock(resourceHandle);
	celsPtr = (uint32 *)((intptr)*resourceHandle + handleOffset);

	// Check that the index into the series requested is within a valid range
	numCels = FROM_LE_32(celsPtr[CELS_COUNT]);
	if (index >= (int)numCels) {
		ws_LogErrorMsg(FL, "CreateSprite: Sprite index out of range - max index: %d, requested index: %d", numCels - 1, index);
		return nullptr;
	}

	// Find the offset table, and the beginning of the data for all sprites
	offsets = &celsPtr[CELS_OFFSETS];
	data = &celsPtr[CELS_OFFSETS + numCels];

	// Find the sprite data for the specific sprite in the series
	myCelSource = (uint32 *)((intptr)data + FROM_LE_32(offsets[index]));

	// Set the stream boolean
	if (streamSeries) {
		if (myCelSource[CELS_STREAM])
			*streamSeries = true;
		else
			*streamSeries = false;
	}

	// Initialize the sprite struct and return it
	mySprite->next = mySprite->prev = nullptr;
	mySprite->sourceHandle = resourceHandle;
	mySprite->xOffset = FROM_LE_32(myCelSource[CELS_X]);
	mySprite->yOffset = FROM_LE_32(myCelSource[CELS_Y]);
	mySprite->w = FROM_LE_32(myCelSource[CELS_W]);
	mySprite->h = FROM_LE_32(myCelSource[CELS_H]);
	mySprite->encoding = (uint8)myCelSource[CELS_COMP];
	mySprite->data = (uint8 *)&myCelSource[CELS_DATA];

	if ((mySprite->w > 0) && (mySprite->h > 0)) {
		mySprite->sourceOffset = (int32)((intptr)mySprite->data - (intptr)*resourceHandle);
	} else {
		mySprite->sourceOffset = 0;
	}

	// This value MUST be set before sprite draws are called after the block has been locked
	mySprite->data = nullptr;

	// Unlock the handle
	HUnLock(resourceHandle);

	return mySprite;
}

int32 LoadSpriteSeries(const char *assetName, MemHandle *seriesHandle, int32 *celsOffset, int32 *palOffset, RGB8 *myPalette) {
	MemHandle workHandle;
	int32 celsSize, *celsPtr, *palPtr;
	char *mainAssetPtr, *endOfAssetBlock, *parseAssetPtr;
	int32 assetSize;

	// This loads a sprite series into the provided vars, rather than the WS tables.
	// The WS loader is not involved with this procedure.

	// Load in the sprite series
	if ((workHandle = rget(assetName, &assetSize)) == nullptr)
		error_show(FL, 'FNF!', "Sprite series: %s", assetName);

	HLock(workHandle);

	mainAssetPtr = (char *)*workHandle;
	endOfAssetBlock = (char *)((intptr)mainAssetPtr + assetSize);
	parseAssetPtr = mainAssetPtr;

	// Process the SS from the stream file
	if ((celsSize = ProcessCELS(assetName, &parseAssetPtr, mainAssetPtr, endOfAssetBlock, &celsPtr, &palPtr, myPalette)) < 0) {
		error_show(FL, 'WSLP', "series: %s", assetName);
	}

	// Store the handle and offsets
	*seriesHandle = workHandle;
	*celsOffset = (intptr)celsPtr - (intptr)mainAssetPtr;
	*palOffset = (intptr)palPtr - (intptr)mainAssetPtr;

	HUnLock(workHandle);

	return celsSize;
}

int32 LoadSpriteSeriesDirect(const char *assetName, MemHandle *seriesHandle, int32 *celsOffset, int32 *palOffset, RGB8 *myPalette) {
	MemHandle workHandle;
	Common::File f;
	int32 celsSize, *celsPtr, *palPtr;
	char *mainAssetPtr, *endOfAssetBlock, *parseAssetPtr;
	uint32 assetSize;

	// This loads a sprite series into the provided vars, rather than the WS tables.
	// The WS loader is not involved with this procedure.

	// First open the file
	if (!f.open(assetName))
		return -1;

	// Get the file size
	assetSize = f.size();

	// Create a handle big enough to hold the contents of the file
	if ((workHandle = NewHandle(assetSize, "ss file")) == nullptr)
		return -1;

	// Lock the handle and read the contents of the file intoit
	HLock(workHandle);
	mainAssetPtr = (char *)*workHandle;
	if (f.read(mainAssetPtr, assetSize) < assetSize)
		return -1;

	// Close the file
	f.close();

	// Set up some pointers
	endOfAssetBlock = (char *)((intptr)mainAssetPtr + assetSize);
	parseAssetPtr = mainAssetPtr;

	// Process the SS from the stream file
	if ((celsSize = ProcessCELS(assetName, &parseAssetPtr, mainAssetPtr, endOfAssetBlock, &celsPtr, &palPtr, myPalette)) < 0) {
		error_show(FL, 'WSLP', "series: %s", assetName);
	}

	// Store the handle and offsets
	*seriesHandle = workHandle;
	*celsOffset = (intptr)celsPtr - (intptr)mainAssetPtr;
	*palOffset = (intptr)palPtr - (intptr)mainAssetPtr;
	HUnLock(workHandle);

	return celsSize;
}

bool ws_GetSSMaxWH(MemHandle ssHandle, int32 ssOffset, int32 *maxW, int32 *maxH) {
	int32 *celsPtr;

	// Parameter verification
	if ((!ssHandle) || (!*ssHandle)) {
		ws_LogErrorMsg(FL, "nullptr Handle given.");
		return false;
	}

	// Lock the handle, and get the cels source
	HLock(ssHandle);
	celsPtr = (int32 *)((intptr)*ssHandle + ssOffset);

	// Return the values
	if (maxW) {
		*maxW = FROM_LE_32(celsPtr[CELS_SS_MAX_W]);
	}

	if (maxH) {
		*maxH = FROM_LE_32(celsPtr[CELS_SS_MAX_H]);
	}

	// unlock the handle
	HUnLock(ssHandle);

	return true;
}

int32 AddWSAssetCELS(const char *wsAssetName, int32 hash, RGB8 *myPalette) {
	MemHandle workHandle;
	char *parseAssetPtr, *mainAssetPtr, *endOfAssetBlock;
	int32 emptySlot, i, assetSize, *celsPtr, *palPtr;

	// Check that the loader has been initialized
	if (!_GWS(wsloaderInitialized)) {
		error_show(FL, 'WSLI', "Asset Name: %s", wsAssetName);
	}

	emptySlot = -1;

	// If hash is < 0, find the first available slot
	if (hash < 0) {
		// Search through the SS names table
		for (i = 0; i <= MAX_ASSET_HASH; i++) {
			// See if there is something loaded in this slot
			if (_GWS(globalCELSnames)[i]) {
				if (!strcmp(_GWS(globalCELSnames)[i], wsAssetName)) {
					break;
				}
			} else if (emptySlot < 0) {
				// Else we found an empty slot
				emptySlot = i;
			}
		}
	} else {
		// Else the SS must be stored in the given hash, replacing any previous contents.
		// Index checking
		if (hash > MAX_ASSET_HASH) {
			error_show(FL, 'WSLA', "Asset Name: %s, hash given was %d", wsAssetName, hash);
		}

		// Check to see if the SS is already loaded in the given hash slot
		if (_GWS(globalCELSnames)[hash] && (!strcmp(_GWS(globalCELSnames)[hash], wsAssetName))) {
			if (_GWS(globalCELSPaloffsets)[hash] >= 0) {
				// Get the pointer to the pal data
				workHandle = _GWS(globalCELSHandles)[hash];
				palPtr = (int32 *)((intptr)*workHandle + _GWS(globalCELSPaloffsets)[hash]);

				// Restore the palette and unlock the handle
				RestoreSSPaletteInfo(myPalette, palPtr);
				HUnLock(workHandle);
			}

			// Since the SS is already loaded, return the slot
			return hash;
		} else {
			// The series is not already loaded, set up values for the next if statement
			i = MAX_ASSET_HASH + 1;
			emptySlot = hash;
		}
	}

	// If we've searched the entire table and not found the series, but
	// we found an empty slot to load the SS into
	if ((i > MAX_ASSET_HASH) && (emptySlot >= 0)) {

		if ((workHandle = rget(wsAssetName, &assetSize)) == nullptr) {
			error_show(FL, 'FNF!', wsAssetName);
		}

		// Lock the handle so we can step through the chunk
		HLock(workHandle);
		mainAssetPtr = (char *)(*workHandle);

		parseAssetPtr = mainAssetPtr;
		endOfAssetBlock = (char *)((intptr)mainAssetPtr + assetSize);

		ClearWSAssets(_WS_ASSET_CELS, emptySlot, emptySlot);

		// Store the resource name
		_GWS(globalCELSnames)[emptySlot] = mem_strdup(wsAssetName);

		// Process the SS from the stream file
		if (ProcessCELS(wsAssetName, &parseAssetPtr, mainAssetPtr, endOfAssetBlock, &celsPtr, &palPtr, myPalette) < 0) {
			error_show(FL, 'WSLP', "Asset Name: %s", wsAssetName);
		}

		// At this point, celsPtr points to the beginning of the cels data, palPtr to the pal data
		// Store the Handle, and calculate the offsets
		_GWS(globalCELSHandles)[emptySlot] = workHandle;
		if (celsPtr) {
			_GWS(globalCELSoffsets)[emptySlot] = (intptr)celsPtr - (intptr)mainAssetPtr;
		} else {
			_GWS(globalCELSoffsets)[emptySlot] = -1;
		}
		if (palPtr) {
			_GWS(globalCELSPaloffsets)[emptySlot] = (intptr)palPtr - (intptr)mainAssetPtr;
		} else {
			_GWS(globalCELSPaloffsets)[emptySlot] = -1;
		}

		// Unlock the handle
		HUnLock(workHandle);

		return emptySlot;
	} else if (i < MAX_ASSET_HASH) {
		// Else if we found the SS already loaded
		if (_GWS(globalCELSPaloffsets)[i] >= 0) {
			// Get the pointer to the pal data
			workHandle = _GWS(globalCELSHandles)[i];
			HLock(workHandle);
			palPtr = (int32 *)((intptr)*workHandle + _GWS(globalCELSPaloffsets)[i]);

			// Restore the palette and unlock the handle
			RestoreSSPaletteInfo(myPalette, palPtr);
			HUnLock(workHandle);
		}

		// Return the hash number for the series
		return i;
	} else {
		// Else we searched the entire table, it was not already loaded, and there are no empty slots
		error_show(FL, 'WSLF', "Asset Name: %s", wsAssetName);
	}

	return -1;
}

static int32 ProcessCELS(const char * /*assetName*/, char **parseAssetPtr, char * /*mainAssetPtr*/, char *endOfAssetBlock,
		int32 **dataOffset, int32 **palDataOffset, RGB8 *myPalette) {
	IntPointer celsType, numColors, celsSize;
	int32 *tempPtr, *data, *dataPtr, *offsetPtr, *palData, i, j;
	IntPointer header, format;
	bool byteSwap;

	if (!_GWS(wsloaderInitialized))
		return -1;

	*dataOffset = nullptr;
	*palDataOffset = nullptr;

	// Get the header and the format
	if (!GetNextint32(parseAssetPtr, endOfAssetBlock, header)) {
		ws_LogErrorMsg(FL, "Unable to get the SS header");
		return -1;
	}
	if (!GetNextint32(parseAssetPtr, endOfAssetBlock, format)) {
		ws_LogErrorMsg(FL, "Unable to get the SS format");
		return -1;
	}

	// Make sure the file is tagged "M4SS" (or "SS4M")
	if (*header == HEAD_SS4M) {
		header.swap();
		format.swap();
	} else if (*header != HEAD_M4SS) {
		ws_LogErrorMsg(FL, "SS chunk is not a valid M4SS chunk.");
		return -1;
	}

	// Verify the format is recent. This is a version control
	if (*format < SS_FORMAT) {
		ws_LogErrorMsg(FL, "Format is antique and cannot be read - rebuild series.");
		return -1;
	}

	// Get the CELS chunk type - either PAL information, or the actual SS info
	if (!GetNextint32(parseAssetPtr, endOfAssetBlock, celsType)) {
		ws_LogErrorMsg(FL, "Unable to read the SS chunk type.");
		return -1;
	}

	byteSwap = false;
	// If the chunk is PAL info - celsType == CELS_LAP_ indicates the chunk needs to be byte-swapped.
	if ((*celsType == CELS__PAL) || (*celsType == CELS_LAP_)) {

		// Read the chunk size, and the number of palette colors, and byte-swap if necessary
		if (!GetNextint32(parseAssetPtr, endOfAssetBlock, celsSize)) {
			ws_LogErrorMsg(FL, "Unable to read the SS PAL chunk size.");
			return -1;
		}
		if (!GetNextint32(parseAssetPtr, endOfAssetBlock, numColors)) {
			ws_LogErrorMsg(FL, "Unable to read the SS PAL number of colors.");
			return -1;
		}

		if (*celsType == CELS_LAP_) {
			celsType.swap();
			celsSize.swap();
			numColors.swap();
			byteSwap = true;
		}

		// Verify that we actually got legitimate values
		if (((int32)(*celsSize) <= 0) || ((int32)(*numColors) <= 0)) {
			ws_LogErrorMsg(FL, "Pal info has been corrupted");
			return -1;
		}

		// The asset block offset for palData should begin with the number of colors
		*palDataOffset = numColors.ptr();
		palData = numColors.ptr();

		if (((intptr)endOfAssetBlock - (intptr)(*parseAssetPtr)) < ((int32)(*celsSize) - 8)) {
			ws_LogErrorMsg(FL, "Pal info is larger than asset block.");
			return -1;
		}

		// If the chunk is in the wrong format, byte-swap the entire chunk
		// Note: we do this because we want the data stored in nrgb format
		// The data is always read in low byte first, but we need it high byte first
		// regardless of the endianness of the machine.
		if (byteSwap) {
			tempPtr = numColors.ptr() + 1;
			for (i = 0; i < *numColors; i++) {
				*tempPtr = SWAP_INT32(*tempPtr);
				tempPtr++;
			}
		}

		*parseAssetPtr += *numColors << 2;

		// The palette info has been processed, now it can be stored
		if (myPalette) {
			tempPtr = (int32 *)(&palData[1]);
			for (i = 0; i < *numColors; i++) {
				j = (*tempPtr & 0xff000000) >> 24;
				myPalette[j].r = (*tempPtr & 0x00ff0000) >> 14;
				myPalette[j].g = (*tempPtr & 0x0000ff00) >> 6;
				myPalette[j].b = (*tempPtr & 0x000000ff) << 2;
				tempPtr++;
			}
		}

		byteSwap = false;
		// Pal chunk has been processed, get the next chunk type
		if (!GetNextint32(parseAssetPtr, endOfAssetBlock, celsType)) {
			ws_LogErrorMsg(FL, "Unable to read the SS chunk type.");
			return -1;
		}
	}

	// The chunk header must now be the SS information (possibly byte-swapped)
	if ((*celsType != CELS___SS) && (*celsType != CELS_SS__)) {
		ws_LogErrorMsg(FL, "SS chunk type is invalid.");
		return -1;
	}

	// Read in the chunk size
	if (!GetNextint32(parseAssetPtr, endOfAssetBlock, celsSize)) {
		ws_LogErrorMsg(FL, "Unable to read the SS chunk size.");
		return -1;
	}

	// Byteswap if necessary
	if (*celsType == CELS_SS__) {
		celsType.swap();
		celsSize.swap();
		byteSwap = true;
	}

	// The asset block offset for the cel should begin with the celsType
	*dataOffset = (int32 *)celsType.ptr();
	data = (int32 *)celsType.ptr();

	// Verify that we actually got legitimate values
	if ((int32)(*celsSize) <= 0) {
		ws_LogErrorMsg(FL, "SS info has been corrupted");
		return -1;
	}

	if (((intptr)endOfAssetBlock - (intptr)data) < (int32)*celsSize) {
		ws_LogErrorMsg(FL, "SS info is larger than asset block.");
		return -1;
	}

	// Check to see if we need to byte-swap the header information.
	if (byteSwap) {

		// The chunk header begins at (*data)[2]
		// byte-swap the entire chunk header
		tempPtr = &(data[2]);
		for (i = 0; i < SS_HEAD_SIZE - 2; i++) {
			*tempPtr = SWAP_INT32(*tempPtr);
			tempPtr++;
		}

		if ((int32)(data[CELS_COUNT]) <= 0) {
			ws_LogErrorMsg(FL, "SS info has been corrupted");
			return -1;
		}

		// The chunk header has been byteswapped, now we must byte-swap the table of
		// offsets into the chunk, which indicate where each individual sprite can be found.
		offsetPtr = &(data[CELS_OFFSETS]);
		tempPtr = offsetPtr;
		for (i = 0; i < data[CELS_COUNT]; i++) {
			*tempPtr = SWAP_INT32(*tempPtr);
			tempPtr++;
		}

		// dataPtr points to the beginning of the block which is a concatenation of
		// all the sprites.  Loop through and byteswap each individual sprite header.
		dataPtr = tempPtr;
		for (i = 0; i < data[CELS_COUNT]; i++) {

			// The beginning of sprite i is the dataPtr + the number of bytes in the offset table
			tempPtr = (int32 *)((intptr)dataPtr + offsetPtr[i]);

			// Byteswap the individual sprite's header
			for (j = 0; j < SS_INDV_HEAD; j++) {
				*tempPtr = SWAP_INT32(*tempPtr);
				tempPtr++;
			}
		}
	}

	// Return the size of the chunk containing the sprite series info
	return *celsSize;
}

static void RestoreSSPaletteInfo(RGB8 *myPalette, int32 *palPtr) {
	uint32 *tempPtr, i, j;

	// Parameter verification
	if ((!myPalette) || (!palPtr))
		return;

	// Set up a pointer that can step through the pal info for the SS, and restore each color
	if (myPalette) {
		tempPtr = (uint32 *)(&palPtr[1]);
		for (i = 0; i < FROM_LE_32(palPtr[0]); i++) {
			j = (*tempPtr & 0xff000000) >> 24;
			myPalette[j].r = (FROM_LE_32(*tempPtr) & 0x00ff0000) >> 14;
			myPalette[j].g = (FROM_LE_32(*tempPtr) & 0x0000ff00) >> 6;
			myPalette[j].b = (FROM_LE_32(*tempPtr) & 0x000000ff) << 2;
			tempPtr++;
		}
	}
}

M4sprite *GetWSAssetSprite(char *spriteName, uint32 hash, uint32 index, M4sprite *mySprite, bool *streamSeries) {
	MemHandle workHandle;
	int32 i;

	// Ensure wsloader has been initialized
	if (!_GWS(wsloaderInitialized)) {
		ws_LogErrorMsg(FL, "WS loader has not been initialized.");
		return nullptr;
	}

	// If spriteName is specified, we search by name, otherwise hash is assumed to be correct
	if (spriteName) {
		if (!_GWS(globalCELSnames)) return nullptr;
		for (i = 0; i <= MAX_ASSET_HASH; i++) {
			if (!strcmp(spriteName, _GWS(globalCELSnames)[i])) {
				break;
			}
		}
		hash = i;
	}

	// Check for valid index, and sprite loaded at that index
	if (hash > MAX_ASSET_HASH) {
		if (spriteName) {
			ws_LogErrorMsg(FL, "Sprite series is not in memory: %s.", spriteName);
		} else {
			ws_LogErrorMsg(FL, "Series number out of range: requested num: %d.", hash);
		}
	}

	// Get the resource handle
	workHandle = _GWS(globalCELSHandles)[hash];

	// Create the sprite
	mySprite = CreateSprite(workHandle, _GWS(globalCELSoffsets)[hash], index, mySprite, streamSeries);

	// Check the sprite
	if (!mySprite) {
		ws_LogErrorMsg(FL, "Series: %s, Hash: %d, index: %d", _GWS(globalCELSnames)[hash], hash, index);
	}

	return mySprite;
}


int32 LoadSpriteSeries(const char *assetName, Handle *seriesHandle, int32 *celsOffset, int32 *palOffset, RGB8 *myPalette) {
	MemHandle workHandle;
	int32 celsSize, *celsPtr, *palPtr;
	char *mainAssetPtr, *endOfAssetBlock, *parseAssetPtr;
	int32 assetSize;

	//This loads a sprite series into the provided vars, rather than the WS tables.
	//The WS loader is not involved with this procedure.

	// Load in the sprite series
	if ((workHandle = rget(assetName, &assetSize)) == nullptr)
		error_show(FL, 'FNF!', "Sprite series: %s", assetName);

	HLock(workHandle);

	mainAssetPtr = (char *)*workHandle;
	endOfAssetBlock = (char *)((intptr)mainAssetPtr + assetSize);
	parseAssetPtr = mainAssetPtr;

	// Process the SS from the stream file
	if ((celsSize = ProcessCELS(assetName, &parseAssetPtr, mainAssetPtr, endOfAssetBlock, &celsPtr, &palPtr, myPalette)) < 0) {
		error_show(FL, 'WSLP', "series: %s", assetName);
	}

	// Store the handle and offsets
	*seriesHandle = workHandle;
	*celsOffset = (intptr)celsPtr - (intptr)mainAssetPtr;
	*palOffset = (intptr)palPtr - (intptr)mainAssetPtr;

	HUnLock(workHandle);

	return celsSize;
}

int32 LoadSpriteSeriesDirect(const char *assetName, Handle *seriesHandle, int32 *celsOffset, int32 *palOffset, RGB8 *myPalette) {
	MemHandle workHandle;
	Common::File f;
	int32 celsSize, *celsPtr, *palPtr;
	char *mainAssetPtr, *endOfAssetBlock, *parseAssetPtr;
	uint32 assetSize;

	// This loads a sprite series into the provided vars, rather than the WS tables.
	// The WS loader is not involved with this procedure.

	// First open the file
	if (!f.open(assetName))
		return -1;

	// Get the size
	assetSize = f.size();

	// Create a handle big enough to hold the contents of the file
	if ((workHandle = NewHandle(assetSize, "ss file")) == nullptr) {
		f.close();
		return -1;
	}

	// Lock the handle and read the contents of the file intoit
	HLock(workHandle);
	mainAssetPtr = (char *)*workHandle;
	if (f.read(mainAssetPtr, assetSize) < assetSize) {
		f.close();
		return -1;
	}

	// Close the file
	f.close();

	// Set up some pointers
	endOfAssetBlock = (char *)((intptr)mainAssetPtr + assetSize);
	parseAssetPtr = mainAssetPtr;

	// Process the SS from the stream file
	if ((celsSize = ProcessCELS(assetName, &parseAssetPtr, mainAssetPtr, endOfAssetBlock, &celsPtr, &palPtr, myPalette)) < 0) {
		error_show(FL, 'WSLP', "series: %s", assetName);
	}

	// Store the handle and offsets
	*seriesHandle = workHandle;
	*celsOffset = (intptr)celsPtr - (intptr)mainAssetPtr;
	*palOffset = (intptr)palPtr - (intptr)mainAssetPtr;
	HUnLock(workHandle);

	return celsSize;
}

CCB *GetWSAssetCEL(uint32 hash, uint32 index, CCB *myCCB) {
	bool streamSeries;
	M4sprite *mySprite;

	// Ensure the WS loader has been initialized.
	if (!_GWS(wsloaderInitialized)) {
		ws_LogErrorMsg(FL, "WS loader has not been initialized.");
		return nullptr;
	}

	//If a memory location to store the CCB has not been provided...
	if (!myCCB) {
		// Create the CCB struct
		if ((myCCB = (CCB *)mem_alloc(sizeof(CCB), "CCB")) == nullptr) {
			ws_LogErrorMsg(FL, "Out of memory - mem requested: %d bytes", sizeof(CCB));
			return nullptr;
		}

		// Create the CCB current location and new location rectangles
		if ((myCCB->currLocation = (M4Rect *)mem_alloc(sizeof(M4Rect), "M4Rect")) == nullptr) {
			ws_LogErrorMsg(FL, "Out of memory - mem requested: %d bytes", sizeof(M4Rect));
			return nullptr;
		}
		myCCB->currLocation->x1 = 0;
		myCCB->currLocation->y1 = 0;
		myCCB->currLocation->x2 = 0;
		myCCB->currLocation->y2 = 0;
		if ((myCCB->newLocation = (M4Rect *)mem_alloc(sizeof(M4Rect), "M4Rect")) == nullptr) {
			ws_LogErrorMsg(FL, "Out of memory - mem requested: %d bytes", sizeof(M4Rect));
			return nullptr;
		}
		myCCB->maxArea = nullptr;
		myCCB->source = nullptr;
	}

	//The source for a CCB is a sprite.  create the sprite from the WS tables hash, index
	mySprite = myCCB->source;
	if ((mySprite = GetWSAssetSprite(nullptr, hash, index, mySprite, &streamSeries)) == nullptr) {
		// Term messages for whatever went wrong are printed from within GetWSAssetSprite()
		return nullptr;
	}
	myCCB->source = mySprite;
	if (streamSeries) {
		myCCB->flags |= CCB_STREAM;
	}

	//Initialize the CCB and return
	myCCB->newLocation->x1 = 0;
	myCCB->newLocation->y1 = 0;
	myCCB->newLocation->x2 = 0;
	myCCB->newLocation->y2 = 0;
	myCCB->scaleX = 0;
	myCCB->scaleY = 0;
	myCCB->seriesName = _GWS(globalCELSnames)[hash];
	return myCCB;
}

int32 GetWSAssetCELCount(uint32 hash) {
	uint32 *celsPtr;

	// Ensure the WS loader has been initialized.
	if (!_GWS(wsloaderInitialized)) {
		ws_LogErrorMsg(FL, "WS loader has not been initialized.");
		return -1;
	}

	// Verify the hash is valid, and a SS for that hash has been loaded
	if (hash > MAX_ASSET_HASH) {
		ws_LogErrorMsg(FL, "Series number out of range: requested num: %d", hash);
		return -1;
	}

	// Make sure the series is still in memory
	if ((!_GWS(globalCELSHandles)[hash]) || (!*_GWS(globalCELSHandles)[hash])) {
		ws_LogErrorMsg(FL, "Series not in memory series num: %d", hash);
		return -1;
	}

	// Find and return the number of sprites in the SS
	celsPtr = (uint32 *)((intptr)*(_GWS(globalCELSHandles)[hash]) + (uint32)(_GWS(globalCELSoffsets)[hash]));
	return celsPtr[CELS_COUNT];
}


int32 GetWSAssetCELFrameRate(uint32 hash) {
	uint32 *celsPtr;

	// Ensure the WS loader has been initialized.
	if (!_GWS(wsloaderInitialized)) {
		ws_LogErrorMsg(FL, "WS loader has not been initialized.");
		return -1;
	}

	// Verify the hash is valid, and a SS for that hash has been loaded
	if (hash > MAX_ASSET_HASH) {
		ws_LogErrorMsg(FL, "Series number out of range: requested num: %d", hash);
		return -1;
	}

	// Make sure the series is still in memory
	if ((!_GWS(globalCELSHandles)[hash]) || (!*_GWS(globalCELSHandles)[hash])) {
		ws_LogErrorMsg(FL, "Series not in memory series num: %d", hash);
		return -1;
	}

	// Find and return the frame rate for the SS
	celsPtr = (uint32 *)((intptr)*(_GWS(globalCELSHandles)[hash]) + (uint32)(_GWS(globalCELSoffsets)[hash]));
	return celsPtr[CELS_FRAME_RATE];
}


int32 GetWSAssetCELPixSpeed(uint32 hash) {
	uint32 *celsPtr;

	// Ensure the WS loader has been initialized.
	if (!_GWS(wsloaderInitialized)) {
		ws_LogErrorMsg(FL, "WS loader has not been initialized.");
		return -1;
	}

	// Verify the hash is valid, and a SS for that hash has been loaded
	if (hash > MAX_ASSET_HASH) {
		ws_LogErrorMsg(FL, "Series number out of range: requested num: %d", hash);
		return -1;
	}

	// Make sure the series is still in memory
	if ((!_GWS(globalCELSHandles)[hash]) || (!*_GWS(globalCELSHandles)[hash])) {
		ws_LogErrorMsg(FL, "Series not in memory series num: %d", hash);
		return -1;
	}

	// Find and return the pix speed for the SS
	celsPtr = (uint32 *)((intptr)*(_GWS(globalCELSHandles)[hash]) + (uint32)(_GWS(globalCELSoffsets)[hash]));
	return celsPtr[CELS_PIX_SPEED];
}

int32 ws_get_sprite_width(uint32 hash, int32 index) {
	uint32 *celsPtr, *offsets, *data, *myCelSource;
	int32 numCels;

	// Ensure the WS loader has been initialized.
	if (!_GWS(wsloaderInitialized)) {
		ws_LogErrorMsg(FL, "WS loader has not been initialized.");
		return -1;
	}

	// Verify the hash is valid, and a SS for that hash has been loaded
	if (hash > MAX_ASSET_HASH) {
		ws_LogErrorMsg(FL, "Series number out of range: requested num: %d", hash);
		return -1;
	}

	// Make sure the series is still in memory
	if ((!_GWS(globalCELSHandles)[hash]) || (!*_GWS(globalCELSHandles)[hash])) {
		ws_LogErrorMsg(FL, "Series not in memory series num: %d", hash);
		return -1;
	}

	// Find the source for the SS
	celsPtr = (uint32 *)((intptr)*(_GWS(globalCELSHandles)[hash]) + (uint32)(_GWS(globalCELSoffsets)[hash]));

	// Check that the index into the series requested is within a valid range
	numCels = celsPtr[CELS_COUNT];
	if (index >= numCels) {
		ws_LogErrorMsg(FL, "ws_get_sprite_width: Sprite index out of range - max index: %d, requested index: %d, hash: %d",
			numCels - 1, index, hash);
		return -1;
	}

	// Find the offset table in the SS header
	offsets = &celsPtr[CELS_OFFSETS];

	// Find the beginning of the data for all sprites in the SS
	data = &celsPtr[CELS_OFFSETS + numCels];

	// Find the sprite data for the specific sprite in the series
	myCelSource = (uint32 *)((intptr)data + offsets[index]);

	return myCelSource[CELS_W];
}

int32 ws_get_sprite_height(uint32 hash, int32 index) {
	uint32 *celsPtr, *offsets, *data, *myCelSource;
	int32 numCels;

	// Ensure the WS loader has been initialized.
	if (!_GWS(wsloaderInitialized)) {
		ws_LogErrorMsg(FL, "WS loader has not been initialized.");
		return -1;
	}

	// Verify the hash is valid, and a SS for that hash has been loaded
	if (hash > MAX_ASSET_HASH) {
		ws_LogErrorMsg(FL, "Series number out of range: requested num: %d", hash);
		return -1;
	}

	// Make sure the series is still in memory
	if ((!_GWS(globalCELSHandles)[hash]) || (!*_GWS(globalCELSHandles)[hash])) {
		ws_LogErrorMsg(FL, "Series not in memory series num: %d", hash);
		return -1;
	}

	// Find the source for the SS
	celsPtr = (uint32 *)((intptr)*(_GWS(globalCELSHandles)[hash]) + (uint32)(_GWS(globalCELSoffsets)[hash]));

	// Check that the index into the series requested is within a valid range
	numCels = celsPtr[CELS_COUNT];
	if (index >= numCels) {
		ws_LogErrorMsg(FL, "ws_get_sprite_height: Sprite index out of range - max index: %d, requested index: %d, hash: %d",
			numCels - 1, index, hash);
		return -1;
	}

	// Find the offset table in the SS header
	offsets = &celsPtr[CELS_OFFSETS];

	// Find the beginning of the data for all sprites in the SS
	data = &celsPtr[CELS_OFFSETS + numCels];

	// Find the sprite data for the specific sprite in the series
	myCelSource = (uint32 *)((intptr)data + offsets[index]);

	return myCelSource[CELS_H];
}

MemHandle ws_GetSEQU(uint32 hash, int32 *numLocalVars, int32 *offset) {
	uint32 *sequPtr;

	// Ensure the WS loader has been initialized.
	if (!_GWS(wsloaderInitialized)) {
		ws_LogErrorMsg(FL, "WS loader has not been initialized.");
		return nullptr;
	}

	// Verify the hash is valid, and a SEQU for that hash has been loaded
	if (hash > MAX_ASSET_HASH) {
		ws_LogErrorMsg(FL, "SEQU number out of range: requested num: %d", hash);
		return nullptr;
	}

	// Make sure the SEQU is still in memory
	if ((!_GWS(globalSEQUHandles)[hash]) || (!*_GWS(globalSEQUHandles)[hash])) {
		ws_LogErrorMsg(FL, "SEQU not in memory: sequence num: %d", hash);
		return nullptr;
	}

	// Find the sequence chunk
	sequPtr = (uint32 *)((intptr)*(_GWS(globalSEQUHandles)[hash]) + (uint32)(_GWS(globalSEQUoffsets)[hash]));

	// Return the offset into the resource chunk, and the number of local vars used by the sequence
	*offset = (intptr)(&sequPtr[SEQU_SEQU_START]) - (intptr)*(_GWS(globalSEQUHandles)[hash]);
	*numLocalVars = FROM_LE_32(sequPtr[SEQU_NUM_VARS]);

	// Return the resource handle
	return _GWS(globalSEQUHandles)[hash];
}

MemHandle ws_GetMACH(uint32 hash, int32 *numStates, int32 *stateTableOffset, int32 *machInstrOffset) {
	uint32 *machPtr;

	// Ensure the WS loader has been initialized.
	if (!_GWS(wsloaderInitialized)) {
		ws_LogErrorMsg(FL, "WS loader has not been initialized.");
		return nullptr;
	}

	// Verify the hash is valid, and a MACH for that hash has been loaded
	if (hash > MAX_ASSET_HASH) {
		ws_LogErrorMsg(FL, "MACH number out of range: requested num: %d", hash);
		return nullptr;
	}

	// Make sure the MACH is still in memory
	if ((!_GWS(globalMACHHandles)[hash]) || (!*_GWS(globalMACHHandles)[hash])) {
		ws_LogErrorMsg(FL, "MACH not in memory: machine num: %d", hash);
		return nullptr;
	}

	// Lock the handle
	HLock(_GWS(globalMACHHandles)[hash]);

	// Find the machine chunk
	machPtr = (uint32 *)((intptr)*(_GWS(globalMACHHandles)[hash]) + (uint32)(_GWS(globalMACHoffsets)[hash]));

	// Set the number of states, the state offset table, the start of the mach instructions
	*numStates = FROM_LE_32(machPtr[MACH_NUM_STATES]);
	*stateTableOffset = (intptr)(&machPtr[MACH_OFFSETS]) - (intptr)(*_GWS(globalMACHHandles)[hash]);
	*machInstrOffset = ((intptr)machPtr + ((*numStates + 1) << 2)) - (intptr)(*_GWS(globalMACHHandles)[hash]);

	// Unlock and return the handle
	HUnLock(_GWS(globalMACHHandles)[hash]);
	return _GWS(globalMACHHandles)[hash];
}

MemHandle ws_GetDATA(uint32 hash, uint32 index, int32 *rowOffset) {
	uint32 *dataPtr;

	// Ensure the WS loader has been initialized.
	if (!_GWS(wsloaderInitialized)) {
		ws_LogErrorMsg(FL, "WS loader has not been initialized.");
		return nullptr;
	}

	// Verify the hash is valid, and a SS for that hash has been loaded
	if (hash > MAX_ASSET_HASH) {
		ws_LogErrorMsg(FL, "DATA number out of range: requested num: %d", hash);
		return nullptr;
	}

	// Make sure the DATA is still in memory
	if ((!_GWS(globalDATAHandles)[hash]) || (!*_GWS(globalDATAHandles)[hash])) {
		ws_LogErrorMsg(FL, "DATA not in memory: data num: %d", hash);
		return nullptr;
	}

	// Find the data block chunk
	dataPtr = (uint32 *)((intptr)*(_GWS(globalDATAHandles)[hash]) + (uint32)(_GWS(globalDATAoffsets)[hash]));

	// Verify the row index of the data block is valid
	if (index > FROM_LE_32(dataPtr[DATA_REC_COUNT])) {
		term_message("File: %s, line: %d, ws_GetDATA() failed:", FL);
		term_message("Data block num: %d", hash);
		term_message("Data row out of range - max row index: %d, requested row index: %d",
			FROM_LE_32(dataPtr[DATA_REC_COUNT]), index);
		return nullptr;
	}

	*rowOffset = (int32)((intptr)(&dataPtr[DATA_REC_START]) +
		((index * FROM_LE_32(dataPtr[DATA_REC_SIZE])) << 2)
		- (intptr)(*_GWS(globalDATAHandles)[hash]));
	// Return the data handle
	return _GWS(globalDATAHandles)[hash];
}

int32 ws_GetDATACount(uint32 hash) {
	uint32 *dataPtr;

	// Ensure the WS loader has been initialized.
	if (!_GWS(wsloaderInitialized)) {
		ws_LogErrorMsg(FL, "WS loader has not been initialized.");
		return -1;
	}

	// Verify the hash is valid, and a SS for that hash has been loaded
	if (hash > MAX_ASSET_HASH) {
		ws_LogErrorMsg(FL, "DATA number out of range: requested num: %d", hash);
		return -1;
	}

	// Make sure the DATA is still in memory
	if ((!_GWS(globalDATAHandles)[hash]) || (!*_GWS(globalDATAHandles)[hash])) {
		ws_LogErrorMsg(FL, "DATA not in memory: data num: %d", hash);
		return -1;
	}

	// Find the data block chunk
	dataPtr = (uint32 *)((intptr)*(_GWS(globalDATAHandles)[hash]) + (uint32)(_GWS(globalDATAoffsets)[hash]));

	// Return the number of rows in the data block
	return FROM_LE_32(dataPtr[DATA_REC_COUNT]);
}

static int32 GetSSHeaderInfo(SysFile *sysFile, uint32 **data, RGB8 *myPalette) {
	uint32 celsType, celsSize, numColors, *myColors;
	uint32 *tempPtr, i, j, header, format;
	int32 numCels, dataOffset;
	bool byteSwap;
	void *handlebuffer;

	if (!sysFile) {
		ws_LogErrorMsg(FL, "nullptr FILE POINTER given.");
		return -1;
	}

	// Read in the series header and the format number
	handlebuffer = &header;
	if (!(*sysFile).read(&handlebuffer, 4)) {
		ws_LogErrorMsg(FL, "Unable to read series header.");
		return -1;
	}
	handlebuffer = &format;
	if (!(*sysFile).read(&handlebuffer, 4)) {
		ws_LogErrorMsg(FL, "Unable to read series format.");
		return -1;
	}

	// Make sure the header is "M4SS", and that the format is not antique
	if (header == HEAD_SS4M) {
		format = SWAP_INT32(format);
	} else if (header != HEAD_M4SS) {
		ws_LogErrorMsg(FL, "Series is not a valid M4SS series.");
		return -1;
	}
	if (format < SS_FORMAT) {
		ws_LogErrorMsg(FL, "Format is antique and cannot be read - rebuild series.");
		return -1;
	}

	// Read in the SS chunk type - either PAL or SS info
	handlebuffer = &celsType;
	if (!(*sysFile).read(&handlebuffer, 4)) {
		ws_LogErrorMsg(FL, "Unable to read series chunk type.");
		return -1;
	}

	if ((celsType == CELS__PAL) || (celsType == CELS_LAP_)) {
		// PAL info, read in the size of the PAL chunk
		handlebuffer = &celsSize;
		if (!(*sysFile).read(&handlebuffer, 4)) {
			ws_LogErrorMsg(FL, "Unable to read series chunk size.");
			return -1;
		}

		// Now read in the number of colors to be inserted into the PAL
		handlebuffer = &numColors;
		if (!(*sysFile).read(&handlebuffer, 4)) {
			ws_LogErrorMsg(FL, "Unable to read number of colors in PAL chunk.");
			return -1;
		}

		// Make sure the info is in the correct format (swap between Motorola and Intel formats)
		if (celsType == CELS_LAP_) {
			celsSize = SWAP_INT32(celsSize);
			numColors = SWAP_INT32(numColors);
			byteSwap = true;
		} else {
			byteSwap = false;
		}

		// If there is at least one color specified in this block
		if (numColors > 0) {
			if ((myColors = (uint32 *)mem_alloc(celsSize - 12, "ss pal info")) == nullptr) {
				ws_LogErrorMsg(FL, "Failed to mem_alloc() %d bytes.", celsSize - 12);
				return -1;
			}

			// Read in the color info into a temp buffer
			handlebuffer = myColors;
			if (!(*sysFile).read(&handlebuffer, numColors << 2)) {
				ws_LogErrorMsg(FL, "Failed to read in the PAL color info.");
				return -1;
			}

			// If the chunk is in the wrong format, byte-swap the entire chunk
			// note: we do this because we want the data stored in nrgb format
			// The data is always read in low byte first, but we need it high byte first
			// regardless of the endianness of the machine.
			if (byteSwap) {
				tempPtr = (uint32 *)&myColors[0];
				for (i = 0; i < numColors; i++) {
					*tempPtr = SWAP_INT32(*tempPtr);
					tempPtr++;
				}
			}

			// If we have a place to store the color info
			if (myPalette) {
				tempPtr = (uint32 *)(&myColors[0]);
				for (i = 0; i < numColors; i++) {
					j = (*tempPtr & 0xff000000) >> 24;
					myPalette[j].r = (*tempPtr & 0x00ff0000) >> 14;
					myPalette[j].g = (*tempPtr & 0x0000ff00) >> 6;
					myPalette[j].b = (*tempPtr & 0x000000ff) << 2;
					tempPtr++;
				}
			}

			// Turf the temp buffer
			mem_free((void *)myColors);
		}

		// Read in the next chunk type
		handlebuffer = &celsType;
		if (!(*sysFile).read(&handlebuffer, 4)) {
			ws_LogErrorMsg(FL, "Failed to read in series chunk type.");
			return -1;
		}
	}

	// Make sure the chunk type is Sprite Series info
	if ((celsType != CELS___SS) && (celsType != CELS_SS__)) {
		ws_LogErrorMsg(FL, "Series chunk type is not labelled as SS info.");
		return -1;
	}

	// Read in the size of the entire chunk
	handlebuffer = &celsSize;
	if (!(*sysFile).read(&handlebuffer, 4)) {
		ws_LogErrorMsg(FL, "Failed to read in series chunk size.");
		return -1;
	}

	// If the chunk is the wrong format, byte-swap (between motorola and intel formats)
	if (celsType == CELS_SS__) {
		celsSize = SWAP_INT32(celsSize);
	}

	// *data contains header + offsets, therefore, we must scan ahead
	// and find out how many cels are here...
	if (!(*sysFile).seek_ahead((CELS_COUNT - CELS_SRC_SIZE - 1) << 2)) {
		ws_LogErrorMsg(FL, "Failed to seek ahead in the stream.");
		return -1;
	}

	// Read how many sprites are in the series
	handlebuffer = &numCels;
	if (!(*sysFile).read(&handlebuffer, 4)) {
		ws_LogErrorMsg(FL, "Failed to read the number of sprites in the series.");
		return -1;
	}

	// Again, byte-swap if the chunk is in the wrong format
	if (celsType == CELS_SS__) {
		numCels = SWAP_INT32(numCels);
	}

	// Now, seek backwards to where we left off
	if (!(*sysFile).seek_ahead((CELS_SRC_SIZE - CELS_COUNT) * 4)) {
		ws_LogErrorMsg(FL, "Failed to seek backwards in the stream.");
		return -1;
	}

	// Allocate a block to hold both the series header, and the sprite offset table
	if ((*data = (uint32 *)mem_alloc((SS_HEAD_SIZE + numCels) * 4, "ss header")) == nullptr) {
		ws_LogErrorMsg(FL, "Failed to mem_alloc() %d bytes.", (SS_HEAD_SIZE + numCels) << 2);
		return -1;
	}

	// Read in the series header and the sprite offset table
	// Since we already read in celsType and celsSize, SS_HEAD_SIZE-2
	handlebuffer = &((*data)[2]);
	if (!(*sysFile).read(&handlebuffer, (SS_HEAD_SIZE + numCels - 2) << 2)) {
		ws_LogErrorMsg(FL, "Failed to read the series header and the sprite offset table.");
		return -1;
	}

	// Set the celsType and the celsSize
	(*data)[0] = celsType;
	(*data)[1] = celsSize;

	// If the chunk is in the wrong format, byte-swap the series header
	if (celsType == CELS_SS__) {
		tempPtr = &((*data)[2]);
		for (i = 0; i < (uint)(SS_HEAD_SIZE + numCels - 2); i++) {
			*tempPtr = SWAP_INT32(*tempPtr);
			tempPtr++;
		}
	}

	// Find out how far into the stream we are, and return that value
	dataOffset = (*sysFile).get_pos();
	return dataOffset;
}

bool ws_OpenSSstream(SysFile *sysFile, Anim8 *anim8) {
	CCB *myCCB;
	frac16 *myRegs;
	uint32 *celsPtr, *offsets;
	int32 ssDataOffset, i, numSprites;
	int32 obesest_frame = 0;
	uint32 maxFrameSize;

	// Verify the parameters
	if (!sysFile || !anim8 || !anim8->myCCB) {
		ws_LogErrorMsg(FL, "SysFile* streamFile invalid.");
		return false;
	}

	myCCB = anim8->myCCB;
	myRegs = anim8->myRegs;
	ssDataOffset = 0;

	// Read in the SS stream header
	if ((ssDataOffset = GetSSHeaderInfo(sysFile, &(myCCB->streamSSHeader), &_G(master_palette)[0])) <= 0) {
		return false;
	}

	// Automatically set some of the sequence registers
	celsPtr = myCCB->streamSSHeader;
	numSprites = celsPtr[CELS_COUNT];
	myRegs[IDX_CELS_INDEX] = -(1 << 16);	// First frame inc will make it 0
	myRegs[IDX_CELS_COUNT] = numSprites << 16;
	myRegs[IDX_CELS_FRAME_RATE] = celsPtr[CELS_FRAME_RATE] << 16;

	// Here we convert the offset table to become the actual size of the data for each sprite
	// This is so the stream can be optimized to always read in on sprite boundaries
	// Get the beginning of the offset table
	offsets = &celsPtr[CELS_OFFSETS];

	maxFrameSize = 0;
	// For all but the last frame, the frame size is the difference in offset values
	for (i = 0; i < numSprites - 1; i++) {
		offsets[i] = offsets[i + 1] - offsets[i];
		if (offsets[i] > maxFrameSize) {
			maxFrameSize = offsets[i];
			obesest_frame = i;
		}
	}

	// For the last sprite we take the entire chunk size - the chunk header - the offset for that sprite
	offsets[numSprites - 1] = celsPtr[CELS_SRC_SIZE] - ((SS_HEAD_SIZE + celsPtr[CELS_COUNT]) << 2) - offsets[numSprites - 1];
	if (offsets[numSprites - 1] > maxFrameSize) {
		maxFrameSize = offsets[numSprites - 1];
		obesest_frame = numSprites - 1;
	}

	// Calculate the maximum size a sprite could be
	maxFrameSize += SS_INDV_HEAD << 2;

	if (!myCCB->source) {
		myCCB->source = (M4sprite *)mem_alloc(sizeof(M4sprite), "Sprite");
		if (!myCCB->source) {
			ws_LogErrorMsg(FL, "Failed to mem_alloc() %d bytes.", sizeof(M4sprite));
			return false;
		}
	}

	term_message("Biggest frame was: %d, size: %d bytes (compressed)", obesest_frame, maxFrameSize);

	// Access the streamer to recognize the new client
	if ((myCCB->myStream = (void *)f_stream_Open(sysFile, ssDataOffset, maxFrameSize, maxFrameSize << 4, numSprites, (int32 *)offsets, 4, false)) == nullptr) {
		ws_LogErrorMsg(FL, "Failed to open a stream.");
		return false;
	}

	// Tag the CCB as being streamed
	myCCB->flags |= CCB_DISC_STREAM;
	myCCB->seriesName = nullptr;

	// Get the first frame
	if (!ws_GetNextSSstreamCel(anim8)) {
		ws_LogErrorMsg(FL, "Failed to get the first stream frame.");
		return false;
	}

	return true;
}

bool ws_GetNextSSstreamCel(Anim8 *anim8) {
	CCB *myCCB;
	M4sprite *mySprite;
	uint32 *celsPtr, *offsets, *myCelSource;
	uint32 frameNum;

	// Verify the parameters
	if (!anim8) {
		ws_LogErrorMsg(FL, "nullptr Anim8* given");
		return false;
	}

	myCCB = anim8->myCCB;
	if ((!anim8->myCCB) || (!myCCB->streamSSHeader) || (!myCCB->myStream)) {
		ws_LogErrorMsg(FL, "Invalid Anim8* given.");
		return false;
	}
	if (!(myCCB->flags & CCB_DISC_STREAM)) {
		ws_LogErrorMsg(FL, "Anim8* given has not previously opened a stream");
		return false;
	}

	// Find the SS source and the offset table into the source
	celsPtr = myCCB->streamSSHeader;
	offsets = &celsPtr[CELS_OFFSETS];

	// Automatically increment the sequence register
	anim8->myRegs[IDX_CELS_INDEX] += 0x10000;

	// Check whether the end of the SS has been streamed
	frameNum = anim8->myRegs[IDX_CELS_INDEX] >> 16;
	if (frameNum >= celsPtr[CELS_COUNT]) {
		ws_LogErrorMsg(FL, "No more frames available to stream");
		return false;
	}

	// Read the next sprite from the stream.  Note the offset table was converted to absolute size when the stream was opened.
	if (f_stream_Read((strmRequest *)myCCB->myStream, (uint8 **)(&myCCB->streamSpriteSource), offsets[frameNum]) < (int)offsets[frameNum]) {
		ws_LogErrorMsg(FL, "Unable to read the next stream frame");
		return false;
	}

	// Flag the CCB if the sprite series is a delta-streaming sprite series
	if (myCCB->streamSpriteSource[CELS_STREAM]) {
		myCCB->flags |= CCB_STREAM;
	}

	// Initialize the sprite structure
	myCelSource = myCCB->streamSpriteSource;
	mySprite = myCCB->source;

	mySprite->xOffset = (int32)convert_intel32((uint32)myCelSource[CELS_X]);
	mySprite->yOffset = (int32)convert_intel32((uint32)myCelSource[CELS_Y]);
	mySprite->w = (int32)convert_intel32((uint32)myCelSource[CELS_W]);
	mySprite->h = (int32)convert_intel32((uint32)myCelSource[CELS_H]);

	{
		uint32 temp = (uint8)myCelSource[CELS_COMP];
		mySprite->encoding = (uint8)convert_intel32(temp);
	}

	mySprite->data = (uint8 *)&myCelSource[CELS_DATA];

	// Initialize the CCB structure
	myCCB->newLocation->x1 = 0;
	myCCB->newLocation->y1 = 0;
	myCCB->newLocation->x2 = 0;
	myCCB->newLocation->y2 = 0;
	myCCB->scaleX = 0;
	myCCB->scaleY = 0;

	return true;
}

void ws_CloseSSstream(CCB *myCCB) {
	// Verify the parameters
	if ((!myCCB) || (!(myCCB->flags & CCB_DISC_STREAM))) {
		ws_LogErrorMsg(FL, "Invalid CCB* given.");
		return;
	}

	// Remove the CCB_DISC_STREAM flag
	myCCB->flags &= ~CCB_DISC_STREAM;

	// Free up the CCB pointers which store streaming information
	if (myCCB->streamSSHeader) {
		mem_free((char *)myCCB->streamSSHeader);
	}

	// Close the stream
	if (myCCB->myStream) {
		f_stream_Close((strmRequest *)myCCB->myStream);
		myCCB->myStream = nullptr;
	}
}

} // End of namespace M4
