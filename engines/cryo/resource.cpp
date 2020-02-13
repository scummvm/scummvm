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
 */

#include "cryo/defs.h"
#include "cryo/cryo.h"
#include "cryo/platdefs.h"
#include "cryo/cryolib.h"
#include "cryo/eden.h"
#include "cryo/sound.h"

namespace Cryo {

#define CRYO_DAT_VER 1	// 32-bit integer

// Original name: prechargephrases
void EdenGame::preloadDialogs(int16 vid) {
	perso_t *perso = &_persons[PER_MORKUS];
	if (vid == 170)
		perso = &_persons[PER_UNKN_156];
	_globals->_characterPtr = perso;
	_globals->_dialogType = DialogType::dtInspect;
	int num = (perso->_id << 3) | _globals->_dialogType;
	Dialog *dial = (Dialog *)getElem(_gameDialogs, num);
	dialoscansvmas(dial);
}

////// datfile.c
void EdenGame::verifh(byte *ptr) {
	byte sum = 0;
	byte *head = ptr;

	for (int8 i = 0; i < 6; i++)
		sum += *head++;

	if (sum != 0xAB)
		return;

	debug("* Begin unpacking resource");
	head -= 6;
	uint16 h0 = READ_LE_UINT16(head);
	// 3 = 2 bytes for the uint16 and 1 byte for an unused char
	head += 3;
	uint16 h3 = READ_LE_UINT16(head);
	head += 2;
	byte *data = h0 + head + 26;
	h3 -= 6;
	head += h3;
	for (; h3; h3--)
		*data-- = *head--;
	head = data + 1;
	data = ptr;
	expandHSQ(head, data);
}

void EdenGame::openbigfile() {
	_bigfile.open("EDEN.DAT");

	char buf[16];
	int count = _bigfile.readUint16LE();
	_bigfileHeader = new PakHeaderNode(count);
	for (int j = 0; j < count; j++) {
		for (int k = 0; k < 16; k++)
			buf[k] = _bigfile.readByte();
		_bigfileHeader->_files[j]._name = Common::String(buf);
		_bigfileHeader->_files[j]._size = _bigfile.readUint32LE();
		_bigfileHeader->_files[j]._offs = _bigfile.readUint32LE();
		_bigfileHeader->_files[j]._flag = _bigfile.readByte();
	}

	_vm->_video->resetInternals();
	_vm->_video->setFile(&_bigfile);
}

void EdenGame::closebigfile() {
	_bigfile.close();
}

int EdenGame::loadmusicfile(int16 num) {
	PakHeaderItem *file = &_bigfileHeader->_files[num + 435];
	int32 size = file->_size;
	int32 offs = file->_offs;
	_bigfile.seek(offs, SEEK_SET);
	uint32 numread = size;
	if (numread > kMaxMusicSize)
		error("Music file %s is too big", file->_name.c_str());
	_bigfile.read(_musicBuf, numread);
	return size;
}

void EdenGame::loadRawFile(uint16 num, byte *buffer) {
	if (_vm->getPlatform() == Common::kPlatformDOS) {
		if ((_vm->isDemo() && num > 2204) || num > 2472)
			error("Trying to read invalid game resource");
	}

	assert(num < _bigfileHeader->_count);
	PakHeaderItem *file = &_bigfileHeader->_files[num];
	int32 size = file->_size;
	int32 offs = file->_offs;

	_bigfile.seek(offs, SEEK_SET);
	_bigfile.read(buffer, size);
}

void EdenGame::loadIconFile(uint16 num, Icon *buffer) {
	if (_vm->getPlatform() == Common::kPlatformDOS) {
		if ((_vm->isDemo() && num > 2204) || num > 2472)
			error("Trying to read invalid game resource");
	}

	assert(num < _bigfileHeader->_count);
	PakHeaderItem *file = &_bigfileHeader->_files[num];
	int32 size = file->_size;
	int32 offs = file->_offs;
	debug("* Loading icon - Resource %d (%s) at 0x%X, %d bytes", num, file->_name.c_str(), offs, size);
	_bigfile.seek(offs, SEEK_SET);

	int count = size / 18;	// sizeof(Icon)
	for (int i = 0; i < count; i++) {
		if (_vm->getPlatform() == Common::kPlatformMacintosh) {
			buffer[i].sx = _bigfile.readSint16BE();
			buffer[i].sy = _bigfile.readSint16BE();
			buffer[i].ex = _bigfile.readSint16BE();
			buffer[i].ey = _bigfile.readSint16BE();
			buffer[i]._cursorId = _bigfile.readUint16BE();
			buffer[i]._actionId = _bigfile.readUint32BE();
			buffer[i]._objectId = _bigfile.readUint32BE();
		}
		else {
			buffer[i].sx = _bigfile.readSint16LE();
			buffer[i].sy = _bigfile.readSint16LE();
			buffer[i].ex = _bigfile.readSint16LE();
			buffer[i].ey = _bigfile.readSint16LE();
			buffer[i]._cursorId = _bigfile.readUint16LE();
			buffer[i]._actionId = _bigfile.readUint32LE();
			buffer[i]._objectId = _bigfile.readUint32LE();
		}
	}
}

void EdenGame::loadRoomFile(uint16 num, Room *buffer) {
	if (_vm->getPlatform() == Common::kPlatformDOS) {
		if ((_vm->isDemo() && num > 2204) || num > 2472)
			error("Trying to read invalid game resource");
	}

	assert(num < _bigfileHeader->_count);
	PakHeaderItem *file = &_bigfileHeader->_files[num];
	int32 size = file->_size;
	int32 offs = file->_offs;
	debug("* Loading room - Resource %d (%s) at 0x%X, %d bytes", num, file->_name.c_str(), offs, size);
	_bigfile.seek(offs, SEEK_SET);

	int count = size / 14;	// sizeof(Room)
	for (int i = 0; i < count; i++) {
		buffer[i]._id = _bigfile.readByte();
		for (int j = 0; j < 4; j++)
			buffer[i]._exits[j] = _bigfile.readByte();
		buffer[i]._flags = _bigfile.readByte();
		if (_vm->getPlatform() == Common::kPlatformMacintosh) {
			buffer[i]._bank = _bigfile.readUint16BE();
			buffer[i]._party = _bigfile.readUint16BE();
		}
		else {
			buffer[i]._bank = _bigfile.readUint16LE();
			buffer[i]._party = _bigfile.readUint16LE();
		}
		buffer[i]._level = _bigfile.readByte();
		buffer[i]._video = _bigfile.readByte();
		buffer[i]._location = _bigfile.readByte();
		buffer[i]._backgroundBankNum = _bigfile.readByte();
	}
}

// Original name: shnmfl
void EdenGame::loadHnm(uint16 num) {
	unsigned int resNum = num - 1 + 485;
	assert(resNum < _bigfileHeader->_count);
	PakHeaderItem *file = &_bigfileHeader->_files[resNum];
	int size = file->_size;
	int offs = file->_offs;
	debug("* Loading movie %d (%s) at 0x%X, %d bytes", num, file->_name.c_str(), (uint)offs, size);
	_vm->_video->_file->seek(offs, SEEK_SET);
}

// Original name: ssndfl
int EdenGame::loadSound(uint16 num) {
	unsigned int resNum = num - 1 + ((_vm->getPlatform() == Common::kPlatformDOS && _vm->isDemo()) ? 656 : 661);
	assert(resNum < _bigfileHeader->_count);
	PakHeaderItem *file = &_bigfileHeader->_files[resNum];
	int32 size = file->_size;
	int32 offs = file->_offs;
	debug("* Loading sound %d (%s) at 0x%X, %d bytes", num, file->_name.c_str(), (uint)offs, size);
	if (_soundAllocated) {
		free(_voiceSamplesBuffer);
		_voiceSamplesBuffer = nullptr;
		_soundAllocated = false; //TODO: bug??? no alloc
	}
	else {
		_voiceSamplesBuffer = (byte *)malloc(size);
		_soundAllocated = true;
	}

	_bigfile.seek(offs, SEEK_SET);
	//For PC loaded data is a VOC file, on Mac version this is a raw samples
	if (_vm->getPlatform() == Common::kPlatformMacintosh)
		_bigfile.read(_voiceSamplesBuffer, size);
	else {
		// VOC files also include extra information for lipsync
		// 1. Standard VOC header
		_bigfile.read(_voiceSamplesBuffer, 0x1A);

		// 2. Lipsync?
		unsigned char chunkType = _bigfile.readByte();

		uint32 val = 0;
		_bigfile.read(&val, 3);
		unsigned int chunkLen = FROM_LE_32(val);

		if (chunkType == 5) {
			_bigfile.read(_gameLipsync + 7260, chunkLen);
			chunkType = _bigfile.readByte();
			_bigfile.read(&val, 3);
			chunkLen = FROM_LE_32(val);
		}

		// 3. Normal sound data
		if (chunkType == 1) {
			_bigfile.readUint16LE();
			size = chunkLen - 2;
			_bigfile.read(_voiceSamplesBuffer, size);
		}
	}

	return size;
}

void EdenGame::convertMacToPC() {
	// Convert all mac (big-endian) resources to native format
	// Array of longs
	int *p = (int *)_gameLipsync;
	for (int i = 0; i < 7240 / 4; i++)
		p[i] = FROM_BE_32(p[i]);
}

void EdenGame::loadpermfiles() {
	Common::File f;
	const int kNumIcons = 136;
	const int kNumRooms = 424;
	const int kNumFollowers = 15;
	const int kNumLabyrinthPath = 70;
	const int kNumDinoSpeedForCitaLevel = 16;
	const int kNumTabletView = 12;
	const int kNumPersoRoomBankTable = 84;
	const int kNumGotos = 130;
	const int kNumObjects = 42;
	const int kNumObjectLocations = 45;
	const int kNumPersons = 58;
	const int kNumCitadel = 7;
	const int kNumCharacterRects = 19;
	const int kNumCharacters = 20;
	const int kNumAreas = 12;
	// tab_2CEF0
	// tab_2CF70
	const int kNumActionCursors = 299;

	const int expectedDataSize =
		kNumIcons * 18 +		// sizeof(Icon)
		kNumRooms * 14 +		// sizeof(Room)
		kNumFollowers * 16 +	// sizeof(Follower)
		kNumLabyrinthPath +
		kNumDinoSpeedForCitaLevel +
		kNumTabletView +
		kNumPersoRoomBankTable +
		kNumGotos * 5 +			// sizeof(Goto)
		kNumObjects * 12 +		// sizeof(object_t)
		kNumObjectLocations * 2 +
		kNumPersons * 18 +		// sizeof(perso_t)
		kNumCitadel * 34 +		// sizeof(Citadel)
		kNumCharacterRects * 8 +
		kNumCharacters * 5 +
		kNumAreas * 10 +		// (sizeof(Area) - 4)
		64 * 2 +
		64 * 2 +
		kNumActionCursors +
		12 +
		3 * 6 * 2 * 3 * 2;

	if (f.open("cryo.dat")) {
		const int dataSize = f.size() - 8 - 4;	// CRYODATA + version
		char headerId[9];

		f.read(headerId, 8);
		headerId[8] = '\0';
		if (strcmp(headerId, "CRYODATA"))
			error("Invalid cryo.dat aux data file");

		if (f.readUint32LE() != CRYO_DAT_VER)
			error("Incorrect data version for cryo.dat");

		if (dataSize != expectedDataSize)
			error("Mismatching data in cryo.dat aux data file (got %d, expected %d)", dataSize, expectedDataSize);
	}
	else
		error("Can not load cryo.dat");

	switch (_vm->getPlatform()) {
	case Common::kPlatformDOS:
		// Since PC version stores hotspots and rooms info in the executable, load them from premade resource file
		for (int i = 0; i < kNumIcons; i++) {
			_gameIcons[i].sx = f.readSint16LE();
			_gameIcons[i].sy = f.readSint16LE();
			_gameIcons[i].ex = f.readSint16LE();
			_gameIcons[i].ey = f.readSint16LE();
			_gameIcons[i]._cursorId = f.readUint16LE();
			_gameIcons[i]._actionId = f.readUint32LE();
			_gameIcons[i]._objectId = f.readUint32LE();
		}

		for (int i = 0; i < kNumRooms; i++) {
			_gameRooms[i]._id = f.readByte();
			for (int j = 0; j < 4; j++)
				_gameRooms[i]._exits[j] = f.readByte();
			_gameRooms[i]._flags = f.readByte();
			_gameRooms[i]._bank = f.readUint16LE();
			_gameRooms[i]._party = f.readUint16LE();
			_gameRooms[i]._level = f.readByte();
			_gameRooms[i]._video = f.readByte();
			_gameRooms[i]._location = f.readByte();
			_gameRooms[i]._backgroundBankNum = f.readByte();
		}
		break;
	case Common::kPlatformMacintosh:
		loadIconFile(2498, _gameIcons);
		loadRoomFile(2497, _gameRooms);
		loadRawFile(2486, _gameLipsync);
		convertMacToPC();

		// Skip the icons and rooms of the DOS version
		f.skip(kNumIcons * 14 + kNumRooms * 11);
		break;
	default:
		error("Unsupported platform");
	}

	// Read the common static data

	for (int i = 0; i < kNumFollowers; i++) {
		_followerList[i]._id = f.readSByte();
		_followerList[i]._spriteNum = f.readSByte();
		_followerList[i].sx = f.readSint16LE();
		_followerList[i].sy = f.readSint16LE();
		_followerList[i].ex = f.readSint16LE();
		_followerList[i].ey = f.readSint16LE();
		_followerList[i]._spriteBank = f.readSint16LE();
		_followerList[i].ff_C = f.readSint16LE();
		_followerList[i].ff_E = f.readSint16LE();
	}

	f.read(_labyrinthPath, kNumLabyrinthPath);
	f.read(_dinoSpeedForCitadelLevel, kNumDinoSpeedForCitaLevel);
	f.read(_tabletView, kNumTabletView);
	f.read(_personRoomBankTable, kNumPersoRoomBankTable);
	f.read(_gotos, kNumGotos * 5);	// sizeof(Goto)

	for (int i = 0; i < kNumObjects; i++) {
		_objects[i]._id = f.readByte();
		_objects[i]._flags = f.readByte();
		_objects[i]._locations = f.readUint32LE();
		_objects[i]._itemMask = f.readUint16LE();
		_objects[i]._powerMask = f.readUint16LE();
		_objects[i]._count = f.readSint16LE();
	}

	for (int i = 0; i < kNumObjectLocations; i++) {
		_objectLocations[i] = f.readUint16LE();
	}

	for (int i = 0; i < kNumPersons; i++) {
		_persons[i]._roomNum = f.readUint16LE();
		_persons[i]._actionId = f.readUint16LE();
		_persons[i]._partyMask = f.readUint16LE();
		_persons[i]._id = f.readByte();
		_persons[i]._flags = f.readByte();
		_persons[i]._roomBankId = f.readByte();
		_persons[i]._spriteBank = f.readByte();
		_persons[i]._items = f.readUint16LE();
		_persons[i]._powers = f.readUint16LE();
		_persons[i]._targetLoc = f.readByte();
		_persons[i]._lastLoc = f.readByte();
		_persons[i]._speed = f.readByte();
		_persons[i]._steps = f.readByte();
	}

	for (int i = 0; i < kNumCitadel; i++) {
		_citadelList[i]._id = f.readSint16LE();
		for (int j = 0; j < 8; j++)
			_citadelList[i]._bank[j] = f.readSint16LE();
		for (int j = 0; j < 8; j++)
			_citadelList[i]._video[j] = f.readSint16LE();
	}

	for (int i = 0; i < kNumCharacterRects; i++) {
		_characterRects[i].left = f.readSint16LE();
		_characterRects[i].top = f.readSint16LE();
		_characterRects[i].right = f.readSint16LE();
		_characterRects[i].bottom = f.readSint16LE();
	}

	f.read(_characterArray, kNumCharacters * 5);

	for (int i = 0; i < kNumAreas; i++) {
		_areasTable[i]._num = f.readByte();
		_areasTable[i]._type = f.readByte();
		_areasTable[i]._flags = f.readUint16LE();
		_areasTable[i]._firstRoomIdx = f.readUint16LE();
		_areasTable[i]._citadelLevel = f.readByte();
		_areasTable[i]._placeNum = f.readByte();
		_areasTable[i]._citadelRoomPtr = nullptr;
		_areasTable[i]._visitCount = f.readSint16LE();
	}

	for (int i = 0; i < 64; i++) {
		tab_2CEF0[i] = f.readSint16LE();
	}

	for (int i = 0; i < 64; i++) {
		tab_2CF70[i] = f.readSint16LE();
	}

	f.read(_actionCursors, kNumActionCursors);
	f.read(_mapMode, 12);
	f.read(_cubeTextureCoords, 3 * 6 * 2 * 3 * 2);

	f.close();

	loadRawFile(0, _mainBankBuf);
	loadRawFile(402, _gameFont);
	loadRawFile(404, _gameDialogs);
	loadRawFile(403, _gameConditions);
}

bool EdenGame::ReadDataSyncVOC(unsigned int num) {
	unsigned int resNum = num - 1 + ((_vm->getPlatform() == Common::kPlatformDOS && _vm->isDemo()) ? 656 : 661);
	unsigned char vocHeader[0x1A];
	int filePos = 0;
	loadpartoffile(resNum, vocHeader, filePos, sizeof(vocHeader));
	filePos += sizeof(vocHeader);
	unsigned char chunkType = 0;
	loadpartoffile(resNum, &chunkType, sizeof(vocHeader), 1);
	filePos++;
	if (chunkType == 5) {
		uint32 chunkLen = 0;
		loadpartoffile(resNum, &chunkLen, filePos, 3);
		filePos += 3;
		chunkLen = FROM_LE_32(chunkLen);
		loadpartoffile(resNum, _gameLipsync + 7260, filePos, chunkLen);
		return true;
	}
	return false;
}

bool EdenGame::ReadDataSync(uint16 num) {
	if (_vm->getPlatform() == Common::kPlatformMacintosh) {
		long pos = READ_LE_UINT32(_gameLipsync + num * 4);
		if (pos != -1) {
			long len = 1024;
			loadpartoffile(1936, _gameLipsync + 7260, pos, len);
			return true;
		}
	}
	else
		return ReadDataSyncVOC(num + 1);	//TODO: remove -1 in caller
	return false;
}

void EdenGame::loadpartoffile(uint16 num, void *buffer, int32 pos, int32 len) {
	assert(num < _bigfileHeader->_count);
	PakHeaderItem *file = &_bigfileHeader->_files[num];
	int32 offs = READ_LE_UINT32(&file->_offs);
	debug("* Loading partial resource %d (%s) at 0x%X(+0x%X), %d bytes", num, file->_name.c_str(), offs, pos, len);
	_bigfile.seek(offs + pos, SEEK_SET);
	_bigfile.read(buffer, len);
}

void EdenGame::expandHSQ(byte *input, byte *output) {
	byte *src = input;
	byte *dst = output;
	byte *ptr;
	uint16 bit;        // bit
	uint16 queue = 0;  // queue
	uint16 len = 0;
	int16  ofs;
#define GetBit										\
bit = queue & 1;								\
queue >>= 1;									\
if (!queue) {									\
	queue = (src[1] << 8) | src[0]; src += 2;	\
	bit = queue & 1;							\
	queue = (queue >> 1) | 0x8000;				\
	}

	for (;;) {
		GetBit;
		if (bit)
			*dst++ = *src++;
		else {
			len = 0;
			GetBit;
			if (!bit) {
				GetBit;
				len = (len << 1) | bit;
				GetBit;
				len = (len << 1) | bit;
				ofs = 0xFF00 | *src++;      //TODO: -256
			}
			else {
				ofs = (src[1] << 8) | src[0];
				src += 2;
				len = ofs & 7;
				ofs = (ofs >> 3) | 0xE000;
				if (!len) {
					len = *src++;
					if (!len)
						break;
				}
			}
			ptr = dst + ofs;
			len += 2;
			while (len--)
				*dst++ = *ptr++;
		}
	}
}

}   // namespace Cryo
