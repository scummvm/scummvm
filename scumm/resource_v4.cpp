/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "scumm/scumm.h"
#include "scumm/intern.h"
#include "scumm/resource.h"

namespace Scumm {

void ScummEngine_v4::readIndexFile() {
	uint16 blocktype;
	uint32 itemsize;
	int numblock = 0;

	debug(9, "readIndexFile()");

	closeRoom();
	openRoom(0);

	while (!_fileHandle->eof()) {
		itemsize = _fileHandle->readUint32LE();
		blocktype = _fileHandle->readUint16LE();
		if (_fileHandle->ioFailed())
			break;

		switch (blocktype) {
		case 0x4E52:	// 'NR'
			_fileHandle->readUint16LE();
			break;
		case 0x5230:	// 'R0'
			_numRooms = _fileHandle->readUint16LE();
			break;
		case 0x5330:	// 'S0'
			_numScripts = _fileHandle->readUint16LE();
			break;
		case 0x4E30:	// 'N0'
			_numSounds = _fileHandle->readUint16LE();
			break;
		case 0x4330:	// 'C0'
			_numCostumes = _fileHandle->readUint16LE();
			break;
		case 0x4F30:	// 'O0'
			_numGlobalObjects = _fileHandle->readUint16LE();
			break;
		}
		_fileHandle->seek(itemsize - 8, SEEK_CUR);
	}

	_fileHandle->clearIOFailed();
	_fileHandle->seek(0, SEEK_SET);

	readMAXS(0);

	// Jamieson630: palManipulate variable initialization
	_palManipCounter = 0;
	_palManipPalette = 0; // Will allocate when needed
	_palManipIntermediatePal = 0; // Will allocate when needed

	while (1) {
		itemsize = _fileHandle->readUint32LE();

		if (_fileHandle->ioFailed())
			break;

		blocktype = _fileHandle->readUint16LE();

		numblock++;

		switch (blocktype) {

		case 0x4E52:	// 'NR'
			_fileHandle->seek(itemsize - 6, SEEK_CUR);
			break;

		case 0x5230:	// 'R0'
			readResTypeList(rtRoom, MKID('ROOM'), "room");
			break;

		case 0x5330:	// 'S0'
			readResTypeList(rtScript, MKID('SCRP'), "script");
			break;

		case 0x4E30:	// 'N0'
			readResTypeList(rtSound, MKID('SOUN'), "sound");
			break;

		case 0x4330:	// 'C0'
			readResTypeList(rtCostume, MKID('COST'), "costume");
			break;

		case 0x4F30:	// 'O0'
			readGlobalObjects();
			break;

		default:
			// FIXME: this is a little hack because Indy3 FM-TOWNS has
			// 32 extra bytes of unknown meaning appended to 00.LFL
			if (!(_gameId == GID_INDY3 && _features & GF_FMTOWNS))
				error("Bad ID %c%c found in directory!", blocktype & 0xFF, blocktype >> 8);
			return;
		}
	}
	closeRoom();
}

void ScummEngine_v4::loadCharset(int no) {
	uint32 size;
	memset(_charsetData, 0, sizeof(_charsetData));

	checkRange(4, 0, no, "Loading illegal charset %d");
	closeRoom();

	File file;
	char buf[20];

	sprintf(buf, "%03d.LFL", 900 + no);
	file.open(buf);

	if (file.isOpen() == false) {
		error("loadCharset(%d): Missing file charset: %s", no, buf);
	}

	size = file.readUint32LE() + 11;
	file.read(createResource(rtCharset, no, size), size);
}

void ScummEngine_v4::readMAXS(int blockSize) {
	// FIXME - I'm not sure for those values yet, they will have to be rechecked

	_numVariables = 800;				// 800
	_numBitVariables = 4096;			// 2048
	_numLocalObjects = 200;				// 200
	_numArray = 50;
	_numVerbs = 100;
	_numNewNames = 50;
	_objectRoomTable = NULL;
	_numCharsets = 9;					// 9
	_numInventory = 80;					// 80
	_numGlobalScripts = 200;
	_numFlObject = 50;

	_shadowPaletteSize = 256;

	_shadowPalette = (byte *) calloc(_shadowPaletteSize, 1);	// FIXME - needs to be removed later
	allocateArrays();
}

void ScummEngine_v4::readGlobalObjects() {
	int i;
	int num = _fileHandle->readUint16LE();
	assert(num == _numGlobalObjects);

	uint32 bits;
	byte tmp;
	for (i = 0; i != num; i++) {
		bits = _fileHandle->readByte();
		bits |= _fileHandle->readByte() << 8;
		bits |= _fileHandle->readByte() << 16;
		_classData[i] = bits;
		tmp = _fileHandle->readByte();
		_objectOwnerTable[i] = tmp & OF_OWNER_MASK;
		_objectStateTable[i] = tmp >> OF_STATE_SHL;
	}
}


void ScummEngine_v8::readGlobalObjects() {
	int i;
	int num = _fileHandle->readUint32LE();
	assert(num == _numGlobalObjects);

	char buffer[40];
	for (i = 0; i < num; i++) {
		_fileHandle->read(buffer, 40);
		if (buffer[0]) {
			// Add to object name-to-id map
			_objectIDMap[buffer] = i;
		}
		_objectStateTable[i] = _fileHandle->readByte();
		_objectRoomTable[i] = _fileHandle->readByte();
		_classData[i] = _fileHandle->readUint32LE();
	}
	memset(_objectOwnerTable, 0xFF, num);
}

void ScummEngine_v7::readGlobalObjects() {
	int i;
	int num = _fileHandle->readUint16LE();
	assert(num == _numGlobalObjects);

	_fileHandle->read(_objectStateTable, num);
	_fileHandle->read(_objectRoomTable, num);
	memset(_objectOwnerTable, 0xFF, num);

	_fileHandle->read(_classData, num * sizeof(uint32));

#if defined(SCUMM_BIG_ENDIAN)
	// Correct the endianess if necessary
	for (i = 0; i != num; i++)
		_classData[i] = FROM_LE_32(_classData[i]);
#endif
}

void ScummEngine_v8::readMAXS(int blockSize) {
	debug(9, "readMAXS: MAXS has blocksize %d", blockSize);

	_fileHandle->seek(50 + 50, SEEK_CUR);            // 176 - 8
	_numVariables = _fileHandle->readUint32LE();     // 1500
	_numBitVariables = _fileHandle->readUint32LE();  // 2048
	_fileHandle->readUint32LE();                     // 40
	_numScripts = _fileHandle->readUint32LE();       // 458
	_numSounds = _fileHandle->readUint32LE();        // 789
	_numCharsets = _fileHandle->readUint32LE();      // 1
	_numCostumes = _fileHandle->readUint32LE();      // 446
	_numRooms = _fileHandle->readUint32LE();         // 95
	_fileHandle->readUint32LE();                     // 80
	_numGlobalObjects = _fileHandle->readUint32LE(); // 1401
	_fileHandle->readUint32LE();                     // 60
	_numLocalObjects = _fileHandle->readUint32LE();  // 200
	_numNewNames = _fileHandle->readUint32LE();      // 100
	_numFlObject = _fileHandle->readUint32LE();      // 128
	_numInventory = _fileHandle->readUint32LE();     // 80
	_numArray = _fileHandle->readUint32LE();         // 200
	_numVerbs = _fileHandle->readUint32LE();         // 50

	_objectRoomTable = (byte *)calloc(_numGlobalObjects, 1);
	_numGlobalScripts = 2000;

	_shadowPaletteSize = NUM_SHADOW_PALETTE * 256;
	_shadowPalette = (byte *)calloc(_shadowPaletteSize, 1);

	allocateArrays();
	_dynamicRoomOffsets = true;
}

void ScummEngine_v7::readMAXS(int blockSize) {
	debug(9, "readMAXS: MAXS has blocksize %d", blockSize);

	_fileHandle->seek(50 + 50, SEEK_CUR);
	_numVariables = _fileHandle->readUint16LE();
	_numBitVariables = _fileHandle->readUint16LE();
	_fileHandle->readUint16LE();                      // 40 in FT; 16 in Dig
	_numGlobalObjects = _fileHandle->readUint16LE();
	_numLocalObjects = _fileHandle->readUint16LE();
	_numNewNames = _fileHandle->readUint16LE();
	_numVerbs = _fileHandle->readUint16LE();
	_numFlObject = _fileHandle->readUint16LE();
	_numInventory = _fileHandle->readUint16LE();
	_numArray = _fileHandle->readUint16LE();
	_numRooms = _fileHandle->readUint16LE();
	_numScripts = _fileHandle->readUint16LE();
	_numSounds = _fileHandle->readUint16LE();
	_numCharsets = _fileHandle->readUint16LE();
	_numCostumes = _fileHandle->readUint16LE();

	_objectRoomTable = (byte *)calloc(_numGlobalObjects, 1);

	if ((_gameId == GID_FT) && (_features & GF_DEMO) && 
		(_features & GF_PC))
		_numGlobalScripts = 300;
	else
		_numGlobalScripts = 2000;

	_shadowPaletteSize = NUM_SHADOW_PALETTE * 256;
	_shadowPalette = (byte *)calloc(_shadowPaletteSize, 1);

	allocateArrays();
	_dynamicRoomOffsets = true;
}

void ScummEngine_v6::readMAXS(int blockSize) {
	debug(9, "readMAXS: MAXS has blocksize %d", blockSize);

	if (_heversion >= 70 && (blockSize == 44 + 8)) { // C++ based engine
		_numVariables = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_numRoomVariables = _fileHandle->readUint16LE();
		_numLocalObjects = _fileHandle->readUint16LE();
		_numArray = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE(); // unknown
		_fileHandle->readUint16LE(); // unknown
		_numFlObject = _fileHandle->readUint16LE();
		_numInventory = _fileHandle->readUint16LE();
		_numRooms = _fileHandle->readUint16LE();
		_numScripts = _fileHandle->readUint16LE();
		_numSounds = _fileHandle->readUint16LE();
		_numCharsets = _fileHandle->readUint16LE();
		_numCostumes = _fileHandle->readUint16LE();
		_numGlobalObjects = _fileHandle->readUint16LE();
		_numImages = _fileHandle->readUint16LE();
		_numSprites = _fileHandle->readUint16LE();
		_numLocalScripts = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE(); // heap related
		_numPalettes = _fileHandle->readUint16LE();
		_numUnk = _fileHandle->readUint16LE();
		_numTalkies = _fileHandle->readUint16LE();
		_numNewNames = 10;

		_objectRoomTable = (byte *)calloc(_numGlobalObjects, 1);
		_numGlobalScripts = 2048;

	} else if (_heversion >= 70 && (blockSize == 38 + 8)) { // Scummsys.9x
		_numVariables = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_numRoomVariables = _fileHandle->readUint16LE();
		_numLocalObjects = _fileHandle->readUint16LE();
		_numArray = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE(); // unknown
		_fileHandle->readUint16LE(); // unknown
		_numFlObject = _fileHandle->readUint16LE();
		_numInventory = _fileHandle->readUint16LE();
		_numRooms = _fileHandle->readUint16LE();
		_numScripts = _fileHandle->readUint16LE();
		_numSounds = _fileHandle->readUint16LE();
		_numCharsets = _fileHandle->readUint16LE();
		_numCostumes = _fileHandle->readUint16LE();
		_numGlobalObjects = _fileHandle->readUint16LE();
		_numImages = _fileHandle->readUint16LE();
		_numSprites = _fileHandle->readUint16LE();
		_numLocalScripts = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE(); // heap releated
		_numNewNames = 10;

		_objectRoomTable = (byte *)calloc(_numGlobalObjects, 1);
		if (_gameId == GID_FREDDI4)
			_numGlobalScripts = 2048;
		else
			_numGlobalScripts = 200;

	} else if (_heversion >= 70 && blockSize > 38) { // sputm7.2
		if (blockSize != 32 + 8)
				error("MAXS block of size %d not supported, please report", blockSize);
		_numVariables = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_numBitVariables = _numRoomVariables = _fileHandle->readUint16LE();
		_numLocalObjects = _fileHandle->readUint16LE();
		_numArray = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_numVerbs = _fileHandle->readUint16LE();
		_numFlObject = _fileHandle->readUint16LE();
		_numInventory = _fileHandle->readUint16LE();
		_numRooms = _fileHandle->readUint16LE();
		_numScripts = _fileHandle->readUint16LE();
		_numSounds = _fileHandle->readUint16LE();
		_numCharsets = _fileHandle->readUint16LE();
		_numCostumes = _fileHandle->readUint16LE();
		_numGlobalObjects = _fileHandle->readUint16LE();
		_numImages = _fileHandle->readUint16LE();
		_numNewNames = 10;

		_objectRoomTable = (byte *)calloc(_numGlobalObjects, 1);
		_numGlobalScripts = 200;

	} else if (_version == 6) {
		if (blockSize != 30 + 8)
			error("MAXS block of size %d not supported", blockSize);
		_numVariables = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE();                      // 16 in Sam/DOTT
		_numBitVariables = _fileHandle->readUint16LE();
		_numLocalObjects = _fileHandle->readUint16LE();
		_numArray = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE();                      // 0 in Sam/DOTT
		_numVerbs = _fileHandle->readUint16LE();
		_numFlObject = _fileHandle->readUint16LE();
		_numInventory = _fileHandle->readUint16LE();
		_numRooms = _fileHandle->readUint16LE();
		_numScripts = _fileHandle->readUint16LE();
		_numSounds = _fileHandle->readUint16LE();
		_numCharsets = _fileHandle->readUint16LE();
		_numCostumes = _fileHandle->readUint16LE();
		_numGlobalObjects = _fileHandle->readUint16LE();
		_numNewNames = 50;

		_objectRoomTable = NULL;
		_numGlobalScripts = 200;

		_shadowPaletteSize = 256;

		if (_heversion >= 70) {
			_objectRoomTable = (byte *)calloc(_numGlobalObjects, 1);
		}
	}

	if (_shadowPaletteSize)
		_shadowPalette = (byte *)calloc(_shadowPaletteSize, 1);

	allocateArrays();
	_dynamicRoomOffsets = true;
}

} // End of namespace Scumm
