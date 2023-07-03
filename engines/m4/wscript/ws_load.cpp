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
#include "m4/core/errors.h"
#include "m4/core/imath.h"
#include "m4/graphics/graphics.h"
#include "m4/mem/mem.h"
#include "m4/vars.h"

namespace M4 {

static bool GetNextint32(char **assetPtr, char *endOfAssetBlock, uint32 **returnVal);
static int32 ProcessCELS(const char * /*assetName*/, char **parseAssetPtr, char * /*mainAssetPtr*/, char *endOfAssetBlock,
	int32 **dataOffset, int32 **palDataOffset, RGB8 *myPalette);
static void RestoreSSPaletteInfo(RGB8 *myPalette, int32 *palPtr);

bool InitWSAssets() {
	int32 i;

	// Make sure this is only called once.
	if (_G(wsloaderInitialized)) {
		error_show(FL, 'WSSN');
	}

	// Allocate space for the tables used by the loader and the resource io MACHine tables
	if ((_G(globalMACHnames) = (char **)mem_alloc(sizeof(char *) * 256, "MACH resource table")) == nullptr) {
		return false;
	}
	if ((_G(globalMACHHandles) = (Handle *)mem_alloc(sizeof(Handle) * 256, "CELS Handles table")) == nullptr) {
		return false;
	}
	if ((_G(globalMACHoffsets) = (int32 *)mem_alloc(sizeof(int32 *) * 256, "MACH offsets table")) == nullptr) {
		return false;
	}
	for (i = 0; i < 256; i++) {
		_G(globalMACHnames)[i] = nullptr;
		_G(globalMACHHandles)[i] = nullptr;
		_G(globalMACHoffsets)[i] = -1;
	}

	// SEQUence tables
	if ((_G(globalSEQUnames) = (char **)mem_alloc(sizeof(char *) * 256, "SEQU resource table")) == nullptr) {
		return false;
	}
	if ((_G(globalSEQUHandles) = (Handle *)mem_alloc(sizeof(Handle) * 256, "CELS Handles table")) == nullptr) {
		return false;
	}
	if ((_G(globalSEQUoffsets) = (int32 *)mem_alloc(sizeof(int32 *) * 256, "SEQU offsets table")) == nullptr) {
		return false;
	}
	for (i = 0; i < 256; i++) {
		_G(globalSEQUnames)[i] = nullptr;
		_G(globalSEQUHandles)[i] = nullptr;
		_G(globalSEQUoffsets)[i] = -1;
	}

	// DATA tables
	if ((_G(globalDATAnames) = (char **)mem_alloc(sizeof(char *) * 256, "DATA resource table")) == nullptr) {
		return false;
	}
	if ((_G(globalDATAHandles) = (Handle *)mem_alloc(sizeof(Handle) * 256, "CELS Handles table")) == nullptr) {
		return false;
	}
	if ((_G(globalDATAoffsets) = (int32 *)mem_alloc(sizeof(int32 *) * 256, "DATA offsets table")) == nullptr) {
		return false;
	}
	for (i = 0; i < 256; i++) {
		_G(globalDATAnames)[i] = nullptr;
		_G(globalDATAHandles)[i] = nullptr;
		_G(globalDATAoffsets)[i] = -1;
	}

	// CELS tables
	if ((_G(globalCELSnames) = (char **)mem_alloc(sizeof(char *) * 256, "CELS resource table")) == nullptr) {
		return false;
	}
	if ((_G(globalCELSHandles) = (Handle *)mem_alloc(sizeof(Handle) * 256, "CELS Handles table")) == nullptr) {
		return false;
	}
	if ((_G(globalCELSoffsets) = (int32 *)mem_alloc(sizeof(int32 *) * 256, "CELS offsets table")) == nullptr) {
		return false;
	}
	if ((_G(globalCELSPaloffsets) = (int32 *)mem_alloc(sizeof(int32 *) * 256, "CELS pal offsets table")) == nullptr) {
		return false;
	}
	for (i = 0; i < 256; i++) {
		_G(globalCELSnames)[i] = nullptr;
		_G(globalCELSHandles)[i] = nullptr;
		_G(globalCELSoffsets)[i] = -1;
		_G(globalCELSPaloffsets)[i] = -1;
	}

	//set the global to indicate the loader is active
	_G(wsloaderInitialized) = true;

	return true;
}

bool ClearWSAssets(uint32 assetType, int32 minHash, int32 maxHash) {
	int32 i;

	if (!_G(wsloaderInitialized)) {
		return false;
	}

	// Bounds checking
	if (minHash < 0)
		minHash = 0;
	if (maxHash > MAX_ASSET_HASH)
		maxHash = MAX_ASSET_HASH;

	switch (assetType) {
	case _WS_ASSET_MACH:
		//clear the machines table for entries [minHash, maxHash]
		for (i = minHash; i <= maxHash; i++) {
			TerminateMachinesByHash(i);
			if (_G(globalMACHnames)[i]) {
				rtoss(_G(globalMACHnames)[i]);
				mem_free(_G(globalMACHnames)[i]);
				_G(globalMACHnames)[i] = nullptr;
				_G(globalMACHHandles)[i] = nullptr;
				_G(globalMACHoffsets)[i] = -1;
			}
		}
		break;

	case _WS_ASSET_SEQU:
		// Clear the sequences table for entries [minHash, maxHash]
		for (i = minHash; i <= maxHash; i++) {
			if (_G(globalSEQUnames)[i]) {
				rtoss(_G(globalSEQUnames)[i]);
				mem_free(_G(globalSEQUnames)[i]);
				_G(globalSEQUnames)[i] = nullptr;
				_G(globalSEQUHandles)[i] = nullptr;
				_G(globalSEQUoffsets)[i] = -1;
			}
		}
		break;

	case _WS_ASSET_DATA:
		//clear the data table for entries [minHash, maxHash]
		for (i = minHash; i <= maxHash; i++) {
			if (_G(globalDATAnames)[i]) {
				rtoss(_G(globalDATAnames)[i]);
				mem_free(_G(globalDATAnames)[i]);
				_G(globalDATAnames)[i] = nullptr;
				_G(globalDATAHandles)[i] = nullptr;
				_G(globalDATAoffsets)[i] = -1;
			}
		}
		break;

	case _WS_ASSET_CELS:
		//clear the cels tables for entries [minHash, maxHash]
		for (i = minHash; i <= maxHash; i++) {
			if (_G(globalCELSnames)[i]) {
				rtoss(_G(globalCELSnames)[i]);
				mem_free(_G(globalCELSnames)[i]);
				_G(globalCELSnames)[i] = nullptr;
				_G(globalCELSHandles)[i] = nullptr;
				_G(globalCELSoffsets)[i] = -1;
				_G(globalCELSPaloffsets)[i] = -1;
			}
		}
		break;

	default:
		return false;
	}
	return true;
}

void ShutdownWSAssets(void) {
	if (!_G(wsloaderInitialized))
		return;

	// For each asset type, clear the entire table
	ClearWSAssets(_WS_ASSET_MACH, 0, MAX_ASSET_HASH);
	ClearWSAssets(_WS_ASSET_SEQU, 0, MAX_ASSET_HASH);
	ClearWSAssets(_WS_ASSET_CELS, 0, MAX_ASSET_HASH);
	ClearWSAssets(_WS_ASSET_DATA, 0, MAX_ASSET_HASH);

	// Deallocate all tables
	if (_G(globalMACHnames)) mem_free(_G(globalMACHnames));
	if (_G(globalSEQUnames)) mem_free(_G(globalSEQUnames));
	if (_G(globalDATAnames)) mem_free(_G(globalDATAnames));
	if (_G(globalCELSnames)) mem_free(_G(globalCELSnames));

	if (_G(globalMACHHandles)) mem_free(_G(globalMACHHandles));
	if (_G(globalMACHoffsets)) mem_free(_G(globalMACHoffsets));
	if (_G(globalSEQUHandles)) mem_free(_G(globalSEQUHandles));
	if (_G(globalSEQUoffsets)) mem_free(_G(globalSEQUoffsets));
	if (_G(globalDATAHandles)) mem_free(_G(globalDATAHandles));
	if (_G(globalDATAoffsets)) mem_free(_G(globalDATAoffsets));
	if (_G(globalCELSHandles)) mem_free(_G(globalCELSHandles));
	if (_G(globalCELSoffsets)) mem_free(_G(globalCELSoffsets));
	if (_G(globalCELSPaloffsets)) mem_free(_G(globalCELSPaloffsets));

	_G(wsloaderInitialized) = false;
}

bool LoadWSAssets(const char *wsAssetName, RGB8 *myPalette) {
	MemHandle workHandle;
	char *mainAssetPtr, *parseAssetPtr, *endOfAssetBlock;
	uint32 *tempPtr;
	uint32 *chunkType, *chunkSize, *chunkHash;
	bool finished, chunkSwap;
	int32 *celsPtr, *palPtr;
	uint32 i;
	int32 assetSize;

	// Check that the loader has been initialized
	if (!_G(wsloaderInitialized)) {
		error_show(FL, 'WSLI');
	}

	// Use the resource io manager to read in the entire block
	if ((workHandle = rget(wsAssetName, &assetSize)) == nullptr) {
		error_show(FL, 'FNF!', "Asset Name: %s", wsAssetName);
	}

	// Lock the handle so we can step through the chunk
	HLock(workHandle);
	mainAssetPtr = (char *)(*workHandle);

	endOfAssetBlock = (char *)((byte *)mainAssetPtr + (uint32)assetSize);

	parseAssetPtr = mainAssetPtr;

	// Set the finished flag
	finished = false;

	// Get the first chunkType
	if (!GetNextint32(&parseAssetPtr, endOfAssetBlock, &chunkType)) {
		finished = true;
	}

	// Process each chunk according to type 
	while (!finished) {
		// Read in the chunk size and hash number
		if (!GetNextint32(&parseAssetPtr, endOfAssetBlock, &chunkSize)) {
			error_show(FL, 'WSLE', "Asset Name: %s", wsAssetName);
		}
		if (!GetNextint32(&parseAssetPtr, endOfAssetBlock, &chunkHash)) {
			error_show(FL, 'WSLE', "Asset Name: %s", wsAssetName);
		}

		// Process the chunk according to type
		chunkSwap = false;
		switch (*chunkType) {
		// Chunk is a machine chunk
		case CHUNK_HCAM:
			// Byte swap the type, size and hash and continue through case CHUNK_MACH.
			*chunkType = SWAP_INT32(*chunkType);
			*chunkSize = SWAP_INT32(*chunkSize);
			*chunkHash = SWAP_INT32(*chunkHash);
			chunkSwap = true;
			// Fall through

		case CHUNK_MACH:
			// Check the validity of the machine hash number, and clear it
			if (*chunkHash > MAX_ASSET_HASH) {
				error_show(FL, 'WSLA', "Asset Name: %s, MACH hash was: %ld", wsAssetName, *chunkHash);
			}
			ClearWSAssets(_WS_ASSET_MACH, *chunkHash, *chunkHash);

			// Store the resource name, and the offset into the resource block
			_G(globalMACHnames)[*chunkHash] = mem_strdup(wsAssetName);
			_G(globalMACHHandles)[*chunkHash] = workHandle;
			_G(globalMACHoffsets)[*chunkHash] = parseAssetPtr - mainAssetPtr;

			// Check that the assetblocksize is big enough that the chunk body was read in...
			if ((endOfAssetBlock - parseAssetPtr) < (int)(*chunkSize - 12)) {
				error_show(FL, 'WSLE', "Asset Name: %s, MACH hash was: %ld", wsAssetName, *chunkHash);
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
			*chunkType = SWAP_INT32(*chunkType);
			*chunkSize = SWAP_INT32(*chunkSize);
			*chunkHash = SWAP_INT32(*chunkHash);
			chunkSwap = true;
			// Fall through

		case CHUNK_SEQU:
			// Check the validity of the sequence hash number, and clear it
			if (*chunkHash > MAX_ASSET_HASH) {
				error_show(FL, 'WSLA', "Asset Name: %s, SEQU hash was: %ld", wsAssetName, *chunkHash);
			}
			ClearWSAssets(_WS_ASSET_SEQU, *chunkHash, *chunkHash);

			// Store the resource name, and the offset into the resource block
			_G(globalSEQUnames)[*chunkHash] = mem_strdup(wsAssetName);
			_G(globalSEQUHandles)[*chunkHash] = workHandle;
			_G(globalSEQUoffsets)[*chunkHash] = (int32)parseAssetPtr - (int32)mainAssetPtr;

			// Check that the assetblocksize is big enough that the chunk body was read in...
			if ((endOfAssetBlock - parseAssetPtr) < (int)(*chunkSize - 12)) {
				error_show(FL, 'WSLE', "Asset Name: %s, SEQU hash was: %ld", wsAssetName, *chunkHash);
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
			*chunkType = SWAP_INT32(*chunkType);
			*chunkSize = SWAP_INT32(*chunkSize);
			*chunkHash = SWAP_INT32(*chunkHash);
			chunkSwap = true;
			// Fall through

		case CHUNK_DATA:
			// Check the validity of the data block hash number, and clear it
			if (*chunkHash > MAX_ASSET_HASH) {
				error_show(FL, 'WSLA', "Asset Name: %s, DATA hash was: %ld", wsAssetName, *chunkHash);
			}
			ClearWSAssets(_WS_ASSET_DATA, *chunkHash, *chunkHash);

			// Store the resource name, and the offset into the resource block
			_G(globalDATAnames)[*chunkHash] = mem_strdup(wsAssetName);
			_G(globalDATAHandles)[*chunkHash] = workHandle;
			_G(globalDATAoffsets)[*chunkHash] = (int32)parseAssetPtr - (int32)mainAssetPtr;

			// Check that the assetblocksize is big enough that the chunk body was read in...
			if ((endOfAssetBlock - parseAssetPtr) < (int)(*chunkSize - 12)) {
				error_show(FL, 'WSLE', "Asset Name: %s, DATA hash was: %ld", wsAssetName, *chunkHash);
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
			*chunkType = SWAP_INT32(*chunkType);
			*chunkSize = SWAP_INT32(*chunkSize);
			*chunkHash = SWAP_INT32(*chunkHash);
			chunkSwap = true;
			// Fall through

		case CHUNK_CELS:
			// Check the validity of the cels hash number, and clear it
			if (*chunkHash > MAX_ASSET_HASH) {
				error_show(FL, 'WSLA', "Asset Name: %s, CELS hash was: %ld", wsAssetName, *chunkHash);
			}
			ClearWSAssets(_WS_ASSET_CELS, *chunkHash, *chunkHash);

			// Store the resource name
			_G(globalCELSnames)[*chunkHash] = mem_strdup(wsAssetName);

			// Process the SS from the stream file
			if (ProcessCELS(wsAssetName, &parseAssetPtr, mainAssetPtr, endOfAssetBlock, &celsPtr, &palPtr, myPalette) < 0) {
				error_show(FL, 'WSLP', "Asset Name: %s, CELS hash was: %ld", wsAssetName, *chunkHash);
			}

			// At this point, celsPtr points to the beginning of the cels data, palPtr to the pal data
			// Store the Handle, and calculate the offsets
			_G(globalCELSHandles)[*chunkHash] = workHandle;
			if (celsPtr) {
				_G(globalCELSoffsets)[*chunkHash] = (int32)celsPtr - (int32)mainAssetPtr;
			} else {
				_G(globalCELSoffsets)[*chunkHash] = -1;
			}
			if (palPtr) {
				_G(globalCELSPaloffsets)[*chunkHash] = (int32)palPtr - (int32)mainAssetPtr;
			} else {
				_G(globalCELSPaloffsets)[*chunkHash] = -1;
			}
			break;

		default:
			error_show(FL, 'WSLT', "Asset Name: %s, %ld bytes into the file.", wsAssetName,
				(int32)parseAssetPtr - 12 - (int32)mainAssetPtr);
			break;
		}

		// Read the next chunkType, or signal we are finished
		if (!GetNextint32(&parseAssetPtr, endOfAssetBlock, &chunkType)) {
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
			ws_LogErrorMsg(FL, "Out of memory - mem requested: %ld.", sizeof(M4sprite));
			return nullptr;
		}
	}

	// Find the cels source  from the asset block
	HLock(resourceHandle);
	celsPtr = (uint32 *)((int32)*resourceHandle + handleOffset);

	// Check that the index into the series requested is within a valid range
	numCels = celsPtr[CELS_COUNT];
	if (index >= (int)numCels) {
		ws_LogErrorMsg(FL, "Sprite index out of range - max index: %ld, requested index: %ld", numCels - 1, index);
		return nullptr;
	}

	// Find the offset table, and the beginning of the data for all sprites
	offsets = &celsPtr[CELS_OFFSETS];
	data = &celsPtr[CELS_OFFSETS + numCels];

	// Find the sprite data for the specific sprite in the series
	myCelSource = (uint32 *)((uint32)data + offsets[index]);

	// Set the stream boolean
	if (streamSeries) {
		if (myCelSource[CELS_STREAM])
			*streamSeries = true;
		else
			*streamSeries = false;
	}

	// Initialize the sprite struct and return it
	mySprite->sourceHandle = resourceHandle;
	mySprite->xOffset = (int32)myCelSource[CELS_X];
	mySprite->yOffset = (int32)myCelSource[CELS_Y];
	mySprite->w = (int32)myCelSource[CELS_W];
	mySprite->h = (int32)myCelSource[CELS_H];
	mySprite->encoding = (uint8)myCelSource[CELS_COMP];
	mySprite->data = (uint8 *)&myCelSource[CELS_DATA];

	if ((mySprite->w > 0) && (mySprite->h > 0)) {
		mySprite->sourceOffset = (int32)((int32)(mySprite->data) - (int32)*resourceHandle);
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
	endOfAssetBlock = (char *)((uint32)mainAssetPtr + (uint32)assetSize);
	parseAssetPtr = mainAssetPtr;

	// Process the SS from the stream file
	if ((celsSize = ProcessCELS(assetName, &parseAssetPtr, mainAssetPtr, endOfAssetBlock, &celsPtr, &palPtr, myPalette)) < 0) {
		error_show(FL, 'WSLP', "series: %s", assetName);
	}

	// Store the handle and offsets
	*seriesHandle = workHandle;
	*celsOffset = (int32)celsPtr - (int32)mainAssetPtr;
	*palOffset = (int32)palPtr - (int32)mainAssetPtr;

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
	endOfAssetBlock = (char *)((uint32)mainAssetPtr + (uint32)assetSize);
	parseAssetPtr = mainAssetPtr;

	// Process the SS from the stream file
	if ((celsSize = ProcessCELS(assetName, &parseAssetPtr, mainAssetPtr, endOfAssetBlock, &celsPtr, &palPtr, myPalette)) < 0) {
		error_show(FL, 'WSLP', "series: %s", assetName);
	}

	// Store the handle and offsets
	*seriesHandle = workHandle;
	*celsOffset = (int32)celsPtr - (int32)mainAssetPtr;
	*palOffset = (int32)palPtr - (int32)mainAssetPtr;
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
	celsPtr = (int32 *)((int32)*ssHandle + ssOffset);

	// Return the values
	if (maxW) {
		*maxW = celsPtr[CELS_SS_MAX_W];
	}

	if (maxH) {
		*maxH = celsPtr[CELS_SS_MAX_H];
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
	if (!_G(wsloaderInitialized)) {
		error_show(FL, 'WSLI', "Asset Name: %s", wsAssetName);
	}

	emptySlot = -1;

	// If hash is < 0, find the first available slot
	if (hash < 0) {
		// Search through the SS names table
		for (i = 0; i <= MAX_ASSET_HASH; i++) {
			// See if there is something loaded in this slot
			if (_G(globalCELSnames)[i]) {
				if (!strcmp(_G(globalCELSnames)[i], wsAssetName)) {
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
			error_show(FL, 'WSLA', "Asset Name: %s, hash given was %ld", wsAssetName, hash);
		}

		// Check to see if the SS is already loaded in the given hash slot
		if (_G(globalCELSnames)[hash] && (!strcmp(_G(globalCELSnames)[hash], wsAssetName))) {
			if (_G(globalCELSPaloffsets)[hash] >= 0) {
				// Get the pointer to the pal data
#ifdef TODO
				workHandle = _G(globalCELSHandles)[hash];
				palPtr = (int32 *)((int32)*workHandle + _G(globalCELSPaloffsets)[hash]);
#else
				error("TODO: Figure out dereferencing");
#endif

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

		if ((workHandle = rget(wsAssetName, &assetSize)) == NULL) {
			error_show(FL, 'FNF!', (char *)wsAssetName);
		}

		// Lock the handle so we can step through the chunk
		HLock(workHandle);
		mainAssetPtr = (char *)(*workHandle);

		parseAssetPtr = mainAssetPtr;
		endOfAssetBlock = (char *)((uint32)mainAssetPtr + (uint32)assetSize);

		ClearWSAssets(_WS_ASSET_CELS, emptySlot, emptySlot);

		// Store the resource name
		_G(globalCELSnames)[emptySlot] = mem_strdup(wsAssetName);

		// Process the SS from the stream file
		if (ProcessCELS(wsAssetName, &parseAssetPtr, mainAssetPtr, endOfAssetBlock, &celsPtr, &palPtr, myPalette) < 0) {
			error_show(FL, 'WSLP', "Asset Name: %s", wsAssetName);
		}

		// At this point, celsPtr points to the beginning of the cels data, palPtr to the pal data
		// Store the Handle, and calculate the offsets
		_G(globalCELSHandles)[emptySlot] = workHandle;
		if (celsPtr) {
			_G(globalCELSoffsets)[emptySlot] = (int32)celsPtr - (int32)mainAssetPtr;
		} else {
			_G(globalCELSoffsets)[emptySlot] = -1;
		}
		if (palPtr) {
			_G(globalCELSPaloffsets)[emptySlot] = (int32)palPtr - (int32)mainAssetPtr;
		} else {
			_G(globalCELSPaloffsets)[emptySlot] = -1;
		}

		// Unlock the handle
		HUnLock(workHandle);

		return emptySlot;
	} else if (i < MAX_ASSET_HASH) {
		// Else if we found the SS already loaded
		if (_G(globalCELSPaloffsets)[i] >= 0) {
			// Get the pointer to the pal data
#ifdef TODO
			workHandle = _G(globalCELSHandles)[i];
			HLock(workHandle);
			palPtr = (int32 *)((int32)*workHandle + _G(globalCELSPaloffsets)[i]);
#else
			error("TODO: Figure out dereferencing");
#endif
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

static bool GetNextint32(char **assetPtr, char *endOfAssetBlock, uint32 **returnVal) {
	// Check to see if we still have an int32 available
	if ((endOfAssetBlock - *assetPtr) < 4) {
		return false;
	}

	// Get the next int32
	*returnVal = (uint32 *)*assetPtr;
	*assetPtr += 4;

	return true;
}

static int32 ProcessCELS(const char * /*assetName*/, char **parseAssetPtr, char * /*mainAssetPtr*/, char *endOfAssetBlock,
	int32 **dataOffset, int32 **palDataOffset, RGB8 *myPalette) {
	uint32 *celsType, *numColors, *palData;
	uint32 *tempPtr, *celsSize, *data, *dataPtr, *offsetPtr, i, j, *header, *format;
	bool	byteSwap;

	if (!_G(wsloaderInitialized))
		return -1;

	*dataOffset = nullptr;
	*palDataOffset = nullptr;

	// Get the header and the format
	if (!GetNextint32(parseAssetPtr, endOfAssetBlock, &header)) {
		ws_LogErrorMsg(FL, "Unable to get the SS header");
		return -1;
	}
	if (!GetNextint32(parseAssetPtr, endOfAssetBlock, &format)) {
		ws_LogErrorMsg(FL, "Unable to get the SS format");
		return -1;
	}

	// Make sure the file is tagged "M4SS" (or "SS4M")
	if (*header == HEAD_SS4M) {
		*header = SWAP_INT32(*header);
		*format = SWAP_INT32(*format);
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
	if (!GetNextint32(parseAssetPtr, endOfAssetBlock, &celsType)) {
		ws_LogErrorMsg(FL, "Unable to read the SS chunk type.");
		return -1;
	}

	byteSwap = false;
	// If the chunk is PAL info - celsType == CELS_LAP_ indicates the chunk needs to be byte-swapped.
	if ((*celsType == CELS__PAL) || (*celsType == CELS_LAP_)) {

		// Read the chunk size, and the number of palette colors, and byte-swap if necessary
		if (!GetNextint32(parseAssetPtr, endOfAssetBlock, &celsSize)) {
			ws_LogErrorMsg(FL, "Unable to read the SS PAL chunk size.");
			return -1;
		}
		if (!GetNextint32(parseAssetPtr, endOfAssetBlock, &numColors)) {
			ws_LogErrorMsg(FL, "Unable to read the SS PAL number of colors.");
			return -1;
		}

		if (*celsType == CELS_LAP_) {
			*celsType = SWAP_INT32(*celsType);
			*celsSize = SWAP_INT32(*celsSize);
			*numColors = SWAP_INT32(*numColors);
			byteSwap = true;
		}

		// Verify that we actually got legitimate values
		if (((int32)(*celsSize) <= 0) || ((int32)(*numColors) <= 0)) {
			ws_LogErrorMsg(FL, "Pal info has been corrupted");
			return -1;
		}

		// The asset block offset for palData should begin with the number of colors
		*palDataOffset = (int32 *)numColors;
		palData = (uint32 *)numColors;

		if (((int32)endOfAssetBlock - (int32)(*parseAssetPtr)) < ((int32)(*celsSize) - 8)) {
			ws_LogErrorMsg(FL, "Pal info is larger than asset block.");
			return -1;
		}

		// If the chunk is in the wrong format, byte-swap the entire chunk
		// Note: we do this because we want the data stored in nrgb format
		// The data is always read in low byte first, but we need it high byte first
		// regardless of the endianness of the machine.
		if (byteSwap) {
			tempPtr = (uint32 *)&numColors[1];
			for (i = 0; i < *numColors; i++) {
				*tempPtr = SWAP_INT32(*tempPtr);
				tempPtr++;
			}
		}

		*parseAssetPtr += *numColors << 2;

		// The palette info has been processed, now it can be stored 
		if (myPalette) {
			tempPtr = (uint32 *)(&palData[1]);
			for (i = 0; i < (uint32)*numColors; i++) {
				j = (*tempPtr & 0xff000000) >> 24;
				myPalette[j].r = (*tempPtr & 0x00ff0000) >> 14;
				myPalette[j].g = (*tempPtr & 0x0000ff00) >> 6;
				myPalette[j].b = (*tempPtr & 0x000000ff) << 2;
				tempPtr++;
			}
		}

		byteSwap = false;
		// Pal chunk has been processed, get the next chunk type
		if (!GetNextint32(parseAssetPtr, endOfAssetBlock, &celsType)) {
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
	if (!GetNextint32(parseAssetPtr, endOfAssetBlock, &celsSize)) {
		ws_LogErrorMsg(FL, "Unable to read the SS chunk size.");
		return -1;
	}

	// Byteswap if necessary
	if (*celsType == CELS_SS__) {
		*celsType = SWAP_INT32(*celsType);
		*celsSize = SWAP_INT32(*celsSize);
		byteSwap = true;
	}

	// The asset block offset for the cel should begin with the celsType
	*dataOffset = (int32 *)celsType;
	data = (uint32 *)celsType;

	// Verify that we actually got legitimate values
	if ((int32)(*celsSize) <= 0) {
		ws_LogErrorMsg(FL, "SS info has been corrupted");
		return -1;
	}

	if (((int32)endOfAssetBlock - (int32)data) < (int32)*celsSize) {
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
			tempPtr = (uint32 *)((uint32)dataPtr + offsetPtr[i]);

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
		for (i = 0; i < (uint32)palPtr[0]; i++) {
			j = (*tempPtr & 0xff000000) >> 24;
			myPalette[j].r = (*tempPtr & 0x00ff0000) >> 14;
			myPalette[j].g = (*tempPtr & 0x0000ff00) >> 6;
			myPalette[j].b = (*tempPtr & 0x000000ff) << 2;
			tempPtr++;
		}
	}
}

} // End of namespace M4
