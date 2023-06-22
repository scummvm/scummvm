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
#include "m4/graphics/graphics.h"
#include "m4/globals.h"

namespace M4 {

bool InitWSAssets(void) {
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

M4sprite *CreateSprite(MemHandle resourceHandle, int32 handleOffset, int32 index, M4sprite *mySprite, bool *streamSeries) {
	uint32 *myCelSource, *data, *offsets, numCels;
	uint32 *celsPtr;

	// Parameter verification
	if ((!resourceHandle) || (!*resourceHandle)) {
		ws_LogErrorMsg(FL, "No sprite source in memory.");
		return NULL;
	}

	if (!mySprite) {
		mySprite = (M4sprite *)mem_alloc(sizeof(M4sprite), "Sprite");
		if (!mySprite) {
			ws_LogErrorMsg(FL, "Out of memory - mem requested: %ld.", sizeof(M4sprite));
			return NULL;
		}
	}

	// Find the cels source  from the asset block
	HLock(resourceHandle);
	celsPtr = (uint32 *)((int32)*resourceHandle + handleOffset);

	// Check that the index into the series requested is within a valid range
	numCels = celsPtr[CELS_COUNT];
	if (index >= (int)numCels) {
		ws_LogErrorMsg(FL, "Sprite index out of range - max index: %ld, requested index: %ld", numCels - 1, index);
		return NULL;
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
	mySprite->data = NULL;

	// Unlock the handle
	HUnLock(resourceHandle);

	return mySprite;
}

} // End of namespace M4
