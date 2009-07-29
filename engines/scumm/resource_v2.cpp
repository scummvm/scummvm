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



#include "scumm/file.h"
#include "scumm/scumm_v2.h"
#include "scumm/resource.h"

namespace Scumm {

void ScummEngine_v2::readClassicIndexFile() {
	int i;

	if (_game.id == GID_MANIAC) {
		if (_game.version == 0) {
			_numGlobalObjects = 256;
			_numRooms = 55;
			_numCostumes = 25;
			_numScripts = 160;
			_numSounds = 70;
		} else if (_game.platform == Common::kPlatformNES) {
			_numGlobalObjects = 775;
			_numRooms = 55;

			// costumes 25-36 are special. see v1MMNEScostTables[] in costume.cpp
			// costumes 37-76 are room graphics resources
			// costume 77 is a character set translation table
			// costume 78 is a preposition list
			// costume 79 is unused but allocated, so the total is a nice even number :)
			_numCostumes = 80;
			_numScripts = 200;
			_numSounds = 100;
		} else {
			_numGlobalObjects = 800;
			_numRooms = 55;
			_numCostumes = 35;
			_numScripts = 200;
			_numSounds = 100;
		}
	} else if (_game.id == GID_ZAK) {
		if (_game.platform == Common::kPlatformC64) {
			_numGlobalObjects = 775;
			_numRooms = 59;
			_numCostumes = 38;
			_numScripts = 155;
			_numSounds = 127;
		} else {
			_numGlobalObjects = 775;
			_numRooms = 61;
			_numCostumes = 37;
			_numScripts = 155;
			_numSounds = 120;
		}
	}

	_fileHandle->seek(0, SEEK_SET);

	readMAXS(0);
	allocateArrays();

	_fileHandle->readUint16LE(); /* version magic number */
	for (i = 0; i != _numGlobalObjects; i++) {
		byte tmp = _fileHandle->readByte();
		_objectOwnerTable[i] = tmp & OF_OWNER_MASK;
		_objectStateTable[i] = tmp >> OF_STATE_SHL;
	}

	for (i = 0; i < _numRooms; i++) {
		_res->roomno[rtRoom][i] = i;
	}
	_fileHandle->seek(_numRooms, SEEK_CUR);
	for (i = 0; i < _numRooms; i++) {
		_res->roomoffs[rtRoom][i] = _fileHandle->readUint16LE();
		if (_res->roomoffs[rtRoom][i] == 0xFFFF)
			_res->roomoffs[rtRoom][i] = (uint32)RES_INVALID_OFFSET;
	}

	for (i = 0; i < _numCostumes; i++) {
		_res->roomno[rtCostume][i] = _fileHandle->readByte();
	}
	for (i = 0; i < _numCostumes; i++) {
		_res->roomoffs[rtCostume][i] = _fileHandle->readUint16LE();
		if (_res->roomoffs[rtCostume][i] == 0xFFFF)
			_res->roomoffs[rtCostume][i] = (uint32)RES_INVALID_OFFSET;
	}

	for (i = 0; i < _numScripts; i++) {
		_res->roomno[rtScript][i] = _fileHandle->readByte();
	}
	for (i = 0; i < _numScripts; i++) {
		_res->roomoffs[rtScript][i] = _fileHandle->readUint16LE();
		if (_res->roomoffs[rtScript][i] == 0xFFFF)
			_res->roomoffs[rtScript][i] = (uint32)RES_INVALID_OFFSET;
	}

	for (i = 0; i < _numSounds; i++) {
		_res->roomno[rtSound][i] = _fileHandle->readByte();
	}
	for (i = 0; i < _numSounds; i++) {
		_res->roomoffs[rtSound][i] = _fileHandle->readUint16LE();
		if (_res->roomoffs[rtSound][i] == 0xFFFF)
			_res->roomoffs[rtSound][i] = (uint32)RES_INVALID_OFFSET;
	}
}

void ScummEngine_v2::readEnhancedIndexFile() {

	_numGlobalObjects = _fileHandle->readUint16LE();
	_fileHandle->seek(_numGlobalObjects, SEEK_CUR);
	_numRooms = _fileHandle->readByte();
	_fileHandle->seek(_numRooms * 3, SEEK_CUR);
	_numCostumes = _fileHandle->readByte();
	_fileHandle->seek(_numCostumes * 3, SEEK_CUR);
	_numScripts = _fileHandle->readByte();
	_fileHandle->seek(_numScripts * 3, SEEK_CUR);
	_numSounds = _fileHandle->readByte();

	_fileHandle->clearErr();
	_fileHandle->seek(0, SEEK_SET);

	readMAXS(0);
	allocateArrays();

	_fileHandle->readUint16LE(); /* version magic number */
	readGlobalObjects();
	readResTypeList(rtRoom);
	readResTypeList(rtCostume);
	readResTypeList(rtScript);
	readResTypeList(rtSound);
}

void ScummEngine_v2::readGlobalObjects() {
	int i;
	int num = _fileHandle->readUint16LE();
	assert(num == _numGlobalObjects);

	for (i = 0; i != num; i++) {
		byte tmp = _fileHandle->readByte();
		_objectOwnerTable[i] = tmp & OF_OWNER_MASK;
		_objectStateTable[i] = tmp >> OF_STATE_SHL;
	}
}

void ScummEngine_v2::readIndexFile() {
	int magic = 0;
	debug(9, "readIndexFile()");

	closeRoom();
	openRoom(0);

	magic = _fileHandle->readUint16LE();

	switch (magic) {
		case 0x0100:
			printf("Enhanced V2 game detected\n");
			assert(_game.version == 2);
			readEnhancedIndexFile();
			break;
		case 0x0A31:
			printf("Classic V1 game detected\n");
			assert(_game.version == 1);
			readClassicIndexFile();
			break;
		case 0x4643:
			if (!(_game.platform == Common::kPlatformNES))
				error("Use maniac target");
			printf("NES V1 game detected\n");
			assert(_game.version == 1);
			readClassicIndexFile();
			break;
		case 0x132:
			printf("C64 V1 game detected\n");
			if (_game.id == GID_MANIAC) {
				assert(_game.version == 0);
			} else {
				assert(_game.version == 1);
			}
			readClassicIndexFile();
			break;
		case 0x032:
			printf("Apple II V1 game detected\n");
			assert(_game.version == 0);
			readClassicIndexFile();
			break;
		default:
			error("Unknown magic id (0x%X) - this version is unsupported", magic);
			break;
	}

	closeRoom();
}

void ScummEngine_v2::loadCharset(int num) {
	// Stub, V2 font resources are hardcoded into the engine.
}

} // End of namespace Scumm
