/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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

void ScummEngine_v3::readIndexFile() {
	if (_features & GF_OLD_BUNDLE) {
		int magic = 0;
		debug(9, "readIndexFile()");

		closeRoom();
		openRoom(0);

		magic = _fileHandle.readUint16LE();
		if (magic != 0x0100)
			warning("The magic id doesn't match (0x%X)", magic);

		_numGlobalObjects = _fileHandle.readUint16LE();
		_fileHandle.seek(_numGlobalObjects * 4, SEEK_CUR);
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
		readGlobalObjects();
		readResTypeList(rtRoom, MKID('ROOM'), "room");
		readResTypeList(rtCostume, MKID('COST'), "costume");
		readResTypeList(rtScript, MKID('SCRP'), "script");
		readResTypeList(rtSound, MKID('SOUN'), "sound");

		closeRoom();
	} else {
		uint16 blocktype;
		uint32 itemsize;
		int numblock = 0;

		debug(9, "readIndexFile()");

		closeRoom();
		openRoom(0);

		while (!_fileHandle.eof()) {
			itemsize = _fileHandle.readUint32LE();
			blocktype = _fileHandle.readUint16LE();
			if (_fileHandle.ioFailed())
				break;

			switch (blocktype) {
			case 0x4E52:	// 'NR'
				_fileHandle.readUint16LE();
				break;
			case 0x5230:	// 'R0'
				_numRooms = _fileHandle.readUint16LE();
				break;
			case 0x5330:	// 'S0'
				_numScripts = _fileHandle.readUint16LE();
				break;
			case 0x4E30:	// 'N0'
				_numSounds = _fileHandle.readUint16LE();
				break;
			case 0x4330:	// 'C0'
				_numCostumes = _fileHandle.readUint16LE();
				break;
			case 0x4F30:	// 'O0'
				_numGlobalObjects = _fileHandle.readUint16LE();
				break;
			}
			_fileHandle.seek(itemsize - 8, SEEK_CUR);
		}

		_fileHandle.clearIOFailed();
		_fileHandle.seek(0, SEEK_SET);

		readMAXS();

		// Jamieson630: palManipulate variable initialization
		_palManipCounter = 0;
		_palManipPalette = 0; // Will allocate when needed
		_palManipIntermediatePal = 0; // Will allocate when needed

		while (1) {
			itemsize = _fileHandle.readUint32LE();

			if (_fileHandle.ioFailed())
				break;

			blocktype = _fileHandle.readUint16LE();

			numblock++;

			switch (blocktype) {

			case 0x4E52:	// 'NR'
				_fileHandle.seek(itemsize - 6, SEEK_CUR);
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
				// FIXME: this is a little hack because Indy3 FM Towns has
				// 32 extra bytes of unknown meaning appended to 00.LFL
				if (!(_gameId == GID_INDY3 && _features & GF_FMTOWNS))
					error("Bad ID %c%c found in directory!", blocktype & 0xFF, blocktype >> 8);
				return;
			}
		}
		closeRoom();
	}
}

void ScummEngine_v3::loadCharset(int no) {
	uint32 size;
	memset(_charsetData, 0, sizeof(_charsetData));

	if (_gameId == GID_ZAK256)
		checkRange(2, 0, no, "Loading illegal charset %d");
	else
		checkRange(4, 0, no, "Loading illegal charset %d");
	closeRoom();

	File file;
	char buf[20];

	sprintf(buf, "%02d.LFL", 99 - no);
	
	file.open(buf, getGameDataPath());
		
	if (file.isOpen() == false) {
		error("loadCharset(%d): Missing file charset: %s", no, buf);
	}
		
	size = file.readUint16LE();
	file.read(createResource(rtCharset, no, size), size);
		
	file.close();
}

void ScummEngine_v3::readMAXS() {
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

void ScummEngine_v3::readGlobalObjects() {
	int num = _fileHandle.readUint16LE();
	assert(num == _numGlobalObjects);
	for (int i = 0; i != num; i++) {
		uint32 bits = _fileHandle.readByte();
		byte tmp;
		bits |= _fileHandle.readByte() << 8;
		bits |= _fileHandle.readByte() << 16;
		_classData[i] = bits;
		tmp = _fileHandle.readByte();
		_objectOwnerTable[i] = tmp & OF_OWNER_MASK;
		_objectStateTable[i] = tmp >> OF_STATE_SHL;
	}
}

} // End of namespace Scumm
