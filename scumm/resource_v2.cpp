/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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
#include "scumm.h"
#include "intern.h"
#include "player_v2.h"
#include "player_v1.h"
#include "resource.h"
#include "sound/mididrv.h"

void ScummEngine_v2::readClassicIndexFile() {
	int i;

	if (_gameId == GID_MANIAC) {
		if (!(_features & GF_AMIGA) && !(_features & GF_NES))
			_musicEngine = new Player_V1(this);

		_numGlobalObjects = 800;
		_numRooms = 55;
		_numCostumes = 35;
		_numScripts = 200;
		_numSounds = 100;
	} else if (_gameId == GID_ZAK) {
		if (!(_features & GF_AMIGA))
			_musicEngine = new Player_V2(this);

		_numGlobalObjects = 775;
		_numRooms = 61;
		_numCostumes = 37;
		_numScripts = 155;
		_numSounds = 120;
	}

	_fileHandle.seek(0, SEEK_SET);

	readMAXS();

	// Jamieson630: palManipulate variable initialization
	_palManipCounter = 0;
	_palManipPalette = 0; // Will allocate when needed
	_palManipIntermediatePal = 0; // Will allocate when needed

	_fileHandle.readUint16LE(); /* version magic number */
	for (i = 0; i != _numGlobalObjects; i++) {
		byte tmp = _fileHandle.readByte();
		_objectOwnerTable[i] = tmp & OF_OWNER_MASK;
		_objectStateTable[i] = tmp >> OF_STATE_SHL;
	}

	for (i = 0; i < _numRooms; i++) {
		res.roomno[rtRoom][i] = i;
	}
	_fileHandle.seek(_numRooms, SEEK_CUR);
	for (i = 0; i < _numRooms; i++) {
		res.roomoffs[rtRoom][i] = _fileHandle.readUint16LE();
		if (res.roomoffs[rtRoom][i] == 0xFFFF)
			res.roomoffs[rtRoom][i] = 0xFFFFFFFF;
	}

	for (i = 0; i < _numCostumes; i++) {
		res.roomno[rtCostume][i] = _fileHandle.readByte();
	}
	for (i = 0; i < _numCostumes; i++) {
		res.roomoffs[rtCostume][i] = _fileHandle.readUint16LE();
		if (res.roomoffs[rtCostume][i] == 0xFFFF)
			res.roomoffs[rtCostume][i] = 0xFFFFFFFF;
	}

	for (i = 0; i < _numScripts; i++) {
		res.roomno[rtScript][i] = _fileHandle.readByte();
	}
	for (i = 0; i < _numScripts; i++) {
		res.roomoffs[rtScript][i] = _fileHandle.readUint16LE();
		if (res.roomoffs[rtScript][i] == 0xFFFF)
			res.roomoffs[rtScript][i] = 0xFFFFFFFF;
	}

	for (i = 0; i < _numSounds; i++) {
		res.roomno[rtSound][i] = _fileHandle.readByte();
	}
	for (i = 0; i < _numSounds; i++) {
		res.roomoffs[rtSound][i] = _fileHandle.readUint16LE();
		if (res.roomoffs[rtSound][i] == 0xFFFF)
			res.roomoffs[rtSound][i] = 0xFFFFFFFF;
	}
}

void ScummEngine_v2::readEnhancedIndexFile() {

	if (!(_features & GF_AMIGA))
		_musicEngine = new Player_V2(this);

	_numGlobalObjects = _fileHandle.readUint16LE();
	_fileHandle.seek(_numGlobalObjects, SEEK_CUR); // Skip object flags
	_numRooms = _fileHandle.readByte();
	_fileHandle.seek(_numRooms * 3, SEEK_CUR);
	_numCostumes = _fileHandle.readByte();
	_fileHandle.seek(_numCostumes * 3, SEEK_CUR);
	_numScripts = _fileHandle.readByte();
	_fileHandle.seek(_numScripts * 3, SEEK_CUR);
	_numSounds = _fileHandle.readByte();

	_fileHandle.clearIOFailed();
	_fileHandle.seek(0, SEEK_SET);

	readMAXS();

	// Jamieson630: palManipulate variable initialization
	_palManipCounter = 0;
	_palManipPalette = 0; // Will allocate when needed
	_palManipIntermediatePal = 0; // Will allocate when needed

	_fileHandle.readUint16LE(); /* version magic number */
	int num = _fileHandle.readUint16LE();
	assert(num == _numGlobalObjects);
	for (int i = 0; i != num; i++) {
		byte tmp = _fileHandle.readByte();
		_objectOwnerTable[i] = tmp & OF_OWNER_MASK;
		_objectStateTable[i] = tmp >> OF_STATE_SHL;
	}
	readResTypeList(rtRoom, MKID('ROOM'), "room");
	readResTypeList(rtCostume, MKID('COST'), "costume");
	readResTypeList(rtScript, MKID('SCRP'), "script");
	readResTypeList(rtSound, MKID('SOUN'), "sound");
}

void ScummEngine_v2::readIndexFile() {
	int magic = 0;
	debug(9, "readIndexFile()");

	closeRoom();
	openRoom(0);

	magic = _fileHandle.readUint16LE();

	switch(magic) {
		case 0x0100:
			printf("Enhanced V2 game detected\n");
			readEnhancedIndexFile();			
			break;
		case 0x0A31:
			printf("Classic V1 game detected\n");
			_version = 1;
			readClassicIndexFile();
			break;
		case 0x4643:
			if (!(_features & GF_NES))
				error("Use maniacnes target");
			printf("NES V1 game detected\n");
			_version = 1;
			readClassicIndexFile();
			break;
		default:
			error("Unknown magic id (0x%X) - this version is unsupported", magic);
			break;
	}

	closeRoom();
}

void ScummEngine_v2::readMAXS() {
	// FIXME - I'm not sure for those values yet, they will have to be rechecked

	_numVariables = 800;				// 800
	_numBitVariables = 4096;			// 2048
	_numLocalObjects = 200;				// 200
	_numArray = 50;
	_numVerbs = 100;
	_numNewNames = 0;
	_objectRoomTable = NULL;
	_numCharsets = 9;					// 9
	_numInventory = 80;					// 80
	_numGlobalScripts = 200;
	_numFlObject = 50;

	_shadowPaletteSize = 256;

	_roomPalette = (byte *) calloc(_shadowPaletteSize, 1);		// FIXME - needs to be removed later
	_shadowPalette = (byte *) calloc(_shadowPaletteSize, 1);	// FIXME - needs to be removed later
	allocateArrays();
}

void ScummEngine_v2::loadCharset(int num) {
	// Stub, V2 font resources are hardcoded into the engine.
}
