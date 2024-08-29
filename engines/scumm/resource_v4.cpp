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

#include "common/md5.h"
#include "common/memstream.h"

#include "scumm/scumm_v4.h"
#include "scumm/file.h"
#include "scumm/resource.h"
#include "scumm/sound.h"
#include "scumm/util.h"

namespace Scumm {

extern const char *nameOfResType(ResType type);

int ScummEngine_v4::readResTypeList(ResType type) {
	uint num;

	debug(9, "readResTypeList(%s)", nameOfResType(type));

	num = _fileHandle->readUint16LE();

	if (num != _res->_types[type].size()) {
		error("Invalid number of %ss (%d) in directory", nameOfResType(type), num);
	}

	for (ResId idx = 0; idx < num; idx++) {
		_res->_types[type][idx]._roomno = _fileHandle->readByte();
		_res->_types[type][idx]._roomoffs = _fileHandle->readUint32LE();
	}

	// WORKAROUND: It seems that the French floppy EGA had its own Roland MT-32 patch, with
	// a DISK09.LEC file different from the one still distributed by LucasFilm Games/Disney
	// today. Unfortunately, if different patches existed back then, they appear to be lost.
	//
	// This allows using the official English Roland MT-32 patch along with any EGA version,
	// by adjusting the sound directory offsets to match the available patch.
	if (type == rtSound && _game.id == GID_MONKEY_EGA && _sound->_musicType == MDT_MIDI) {
		Common::File rolandPatchFile;
		if (rolandPatchFile.open("DISK09.LEC")) {
			Common::String md5 = Common::computeStreamMD5AsString(rolandPatchFile);
			if (md5 == "64ab9552f71dd3344767718eb01e5fd5") {
				uint32 patchOffsets[19] = {
					28957,	23427,	35913,	49919,	51918,
					53643,	55368,	57093,	58818,	62502,
					73,		66844,	71991,	83107,	91566,
					95614,	98650,	105020,	112519
				};
				for (ResId idx = 150; idx < 169; idx++) {
					_res->_types[type][idx]._roomno = 94;
					_res->_types[type][idx]._roomoffs = patchOffsets[idx - 150];
				}
			}
		}
	}

	return num;
}

void ScummEngine_v4::readIndexFile() {
	uint16 blocktype;
	uint32 itemsize;

	debug(9, "readIndexFile()");

	closeRoom();
	openRoom(0);

	while (true) {
		// Figure out the sizes of various resources
		itemsize = _fileHandle->readUint32LE();
		blocktype = _fileHandle->readUint16LE();
		if (_fileHandle->eos() || _fileHandle->err())
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

			// Indy3 FM-TOWNS has 32 extra bytes of unknown meaning
			if (_game.id == GID_INDY3 && _game.platform == Common::kPlatformFMTowns)
				itemsize += 32;
			break;
		default:
			break;
		}
		_fileHandle->seek(itemsize - 8, SEEK_CUR);
	}

	_fileHandle->seek(0, SEEK_SET);

	readMAXS(0);
	allocateArrays();

	while (true) {
		itemsize = _fileHandle->readUint32LE();

		if (_fileHandle->eos() || _fileHandle->err())
			break;

		blocktype = _fileHandle->readUint16LE();

		switch (blocktype) {

		case 0x4E52:	// 'NR'
			// Names of rooms. Maybe we should put them into a table, for use by the debugger?
			for (int room; (room = _fileHandle->readByte()); ) {
				char buf[10];
				_fileHandle->read(buf, 9);
				buf[9] = 0;
				for (int i = 0; i < 9; i++)
					buf[i] ^= 0xFF;
				debug(5, "Room %d: '%s'", room, buf);
			}
			break;

		case 0x5230:	// 'R0'
			readResTypeList(rtRoom);
			break;

		case 0x5330:	// 'S0'
			readResTypeList(rtScript);
			break;

		case 0x4E30:	// 'N0'
			readResTypeList(rtSound);
			break;

		case 0x4330:	// 'C0'
			readResTypeList(rtCostume);
			break;

		case 0x4F30:	// 'O0'
			readGlobalObjects();
			break;

		default:
			error("Bad ID %04X found in index file directory", blocktype);
		}
	}
	closeRoom();
}

void ScummEngine_v4::loadCharset(int no) {
	uint32 size;
	memset(_charsetData, 0, sizeof(_charsetData));

	assertRange(0, no, 4, "charset");
	closeRoom();

	Common::File file;
	char buf[20];
	byte *data;

	Common::sprintf_s(buf, "%03d.LFL", 900 + no);
	file.open(buf);

	if (file.isOpen() == false) {
		error("loadCharset(%d): Missing file charset: %s", no, buf);
	}

	size = file.readUint32LE() + 11;
	data = _res->createResource(rtCharset, no, size);
	file.read(data, size);

	// WORKAROUND: The French floppy EGA and VGA versions of Monkey Island 1
	// don't properly follow CP850 for the \x85 character in the 904.LFL font.
	// It should be the `à` letter, but it will print the `ç` letter (which is
	// already at \x87) instead, breaking at least the "Non!  Ce n'est pas tout
	// \x85 fait \x87a." line in the copy protection screen. The `à` character
	// does exist, but at the invalid \x86 position.  So we replace \x85 with
	// \x86 (and then \x86 with \x87 so that the whole charset resource keeps
	// the same size), but only when detecting the faulty 904.LFL file.
	if ((_game.id == GID_MONKEY_EGA || _game.id == GID_MONKEY_VGA) && no == 4 && size == 4857 && _language == Common::FR_FRA && enhancementEnabled(kEnhTextLocFixes)) {
		Common::MemoryReadStream stream(data, size);
		Common::String md5 = Common::computeStreamMD5AsString(stream);

		if (md5 == "f273c26bbcdfb9f87e42748c3e2729d8") {
			warning("Fixing the invalid content of the 904.LFL a-grave character");
			memmove(data + 4457,      data + 4457 + 37,      40); // replace \x85 with \x86
			memmove(data + 4457 + 40, data + 4457 + 37 + 40, 37); // replace \x86 with \x87
			WRITE_LE_UINT32(data + 557, READ_LE_UINT32(data + 557) + (40 - 37)); // adjust \x86 start offset
		}
	}
}

void ScummEngine_v4::readMAXS(int blockSize) {
	// FIXME - I'm not sure for those values yet, they will have to be rechecked

	_numVariables = 800;				// 800
	_numBitVariables = 4096;			// 2048
	_numLocalObjects = 200;				// 200
	_numArray = 50;
	_numVerbs = 100;
	_numNewNames = 50;
	_objectRoomTable = nullptr;
	_numCharsets = 9;					// 9
	_numInventory = 80;					// 80
	_numGlobalScripts = 200;
	_numFlObject = 50;

	_shadowPaletteSize = 256;

	_shadowPalette = (byte *) calloc(_shadowPaletteSize, 1);	// FIXME - needs to be removed later
}

void ScummEngine_v4::readGlobalObjects() {
	int i;
	int num = _fileHandle->readUint16LE();
	assert(num == _numGlobalObjects);

	uint32 bits;
	byte tmp;
	if (_game.id == GID_LOOM && _game.platform == Common::kPlatformPCEngine) {
		for (i = 0; i != num; i++) {
			bits = _fileHandle->readByte();
			bits |= _fileHandle->readByte() << 8;
			bits |= _fileHandle->readByte() << 16;
			_classData[i] = bits;
		}
		for (i = 0; i != num; i++) {
			tmp = _fileHandle->readByte();
			_objectOwnerTable[i] = tmp & OF_OWNER_MASK;
			_objectStateTable[i] = tmp >> OF_STATE_SHL;
		}
	} else {
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

	// FIXME: Indy3 FM-TOWNS has 32 extra bytes of unknown meaning
	if (_game.id == GID_INDY3 && _game.platform == Common::kPlatformFMTowns)
		_fileHandle->seek(32, SEEK_CUR);
}


} // End of namespace Scumm
