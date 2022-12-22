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

// Resource file routines for Simon1/Simon2


#include "common/archive.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "agos/agos.h"
#include "agos/intern.h"

#include "common/compression/zlib.h"

namespace AGOS {

#ifdef ENABLE_AGOS2
uint16 AGOSEngine_Feeble::to16Wrapper(uint value) {
	return TO_LE_16(value);
}

uint16 AGOSEngine_Feeble::readUint16Wrapper(const void *src) {
	return READ_LE_UINT16(src);
}

uint32 AGOSEngine_Feeble::readUint32Wrapper(const void *src) {
	return READ_LE_UINT32(src);
}
#endif

uint16 AGOSEngine::to16Wrapper(uint value) {
	return TO_BE_16(value);
}

uint16 AGOSEngine::readUint16Wrapper(const void *src) {
	return READ_BE_UINT16(src);
}

uint32 AGOSEngine::readUint32Wrapper(const void *src) {
	return READ_BE_UINT32(src);
}

void AGOSEngine::decompressData(const char *srcName, byte *dst, uint32 offset, uint32 srcSize, uint32 dstSize) {
#ifdef USE_ZLIB
		Common::File in;
		in.open(srcName);
		if (in.isOpen() == false)
			error("decompressData: Can't load %s", srcName);

		in.seek(offset, SEEK_SET);
		if (srcSize != dstSize) {
			byte *srcBuffer = (byte *)malloc(srcSize);

			if (in.read(srcBuffer, srcSize) != srcSize)
				error("decompressData: Read failed");

			unsigned long decompressedSize = dstSize;
			if (!Common::uncompress(dst, &decompressedSize, srcBuffer, srcSize))
				error("decompressData: Zlib uncompress error");
			free(srcBuffer);
		} else {
			if (in.read(dst, dstSize) != dstSize)
				error("decompressData: Read failed");
		}
		in.close();
#else
	error("Zlib support is required for Amiga and Macintosh versions");
#endif
}

void AGOSEngine::loadOffsets(const char *filename, int number, uint32 &file, uint32 &offset, uint32 &srcSize, uint32 &dstSize) {
	Common::File in;

	int offsSize = (getPlatform() == Common::kPlatformAmiga) ? 16 : 12;

	/* read offsets from index */
	in.open(filename);
	if (in.isOpen() == false) {
		error("loadOffsets: Can't load index file '%s'", filename);
	}

	in.seek(number * offsSize, SEEK_SET);
	offset = in.readUint32LE();
	dstSize = in.readUint32LE();
	srcSize = in.readUint32LE();
	file = in.readUint32LE();
	in.close();
}

int AGOSEngine::allocGamePcVars(Common::SeekableReadStream *in) {
	uint32 itemArraySize, itemArrayInited, stringTableNum;
	uint32 version;
	uint32 i;

	itemArraySize = in->readUint32BE();
	version = in->readUint32BE();
	itemArrayInited = in->readUint32BE();
	stringTableNum = in->readUint32BE();

	// First two items are predefined
	if (getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2) {
		itemArraySize += 2;
		itemArrayInited = itemArraySize;
	} else {
		itemArrayInited += 2;
		itemArraySize += 2;
	}

	if (version != 0x80)
		error("allocGamePcVars: Not a runtime database");

	_itemArrayPtr = (Item **)calloc(itemArraySize, sizeof(Item *));
	if (_itemArrayPtr == nullptr)
		error("allocGamePcVars: Out of memory for Item array");

	_itemArraySize = itemArraySize;
	_itemArrayInited = itemArrayInited;

	for (i = 1; i < itemArrayInited; i++) {
		_itemArrayPtr[i] = (Item *)allocateItem(sizeof(Item));
	}

	// The rest is cleared automatically by calloc
	allocateStringTable(stringTableNum + 10);
	_stringTabNum = stringTableNum;

	return itemArrayInited;
}

void AGOSEngine_PN::loadGamePcFile() {
	if (getFileName(GAME_BASEFILE) != nullptr) {
		Common::File in;
		// Read dataBase
		if (!in.open(getFileName(GAME_BASEFILE))) {
			error("loadGamePcFile: Can't load database file '%s'", getFileName(GAME_BASEFILE));
		}

		_dataBaseSize = in.size();
		_dataBase = (byte *)malloc(_dataBaseSize);
		if (_dataBase == nullptr)
			error("loadGamePcFile: Out of memory for dataBase");
		in.read(_dataBase, _dataBaseSize);

		if (_dataBase[31] != 0)
			error("Later version of system requested");
	}

	if (getFileName(GAME_TEXTFILE) != nullptr) {
		Common::File in;
		// Read textBase
		if (!in.open(getFileName(GAME_TEXTFILE))) {
			error("loadGamePcFile: Can't load textbase file '%s'", getFileName(GAME_TEXTFILE));
		}

		_textBaseSize = in.size();
		_textBase = (byte *)malloc(_textBaseSize);
		if (_textBase == nullptr)
			error("loadGamePcFile: Out of memory for textBase");
		in.read(_textBase, _textBaseSize);

		if (_textBase[getlong(30L)] != 128)
			error("Unknown compression format");
	}
}

void AGOSEngine::loadGamePcFile() {
	int fileSize;

	if (getFileName(GAME_BASEFILE) != nullptr) {
		/* Read main gamexx file */
		Common::File in;
		if (!in.open(getFileName(GAME_BASEFILE))) {
			error("loadGamePcFile: Can't load gamexx file '%s'", getFileName(GAME_BASEFILE));
		}

		if (getFeatures() & GF_CRUNCHED_GAMEPC) {
			uint srcSize = in.size();
			byte *srcBuf = (byte *)malloc(srcSize);
			in.read(srcBuf, srcSize);

			uint dstSize = READ_BE_UINT32(srcBuf + srcSize - 4);
			byte *dstBuf = (byte *)malloc(dstSize);
			decrunchFile(srcBuf, dstBuf, srcSize);
			free(srcBuf);

			Common::MemoryReadStream stream(dstBuf, dstSize);
			readGamePcFile(&stream);
			free(dstBuf);
		} else {
			readGamePcFile(&in);
		}
	}

	if (getFileName(GAME_TBLFILE) != nullptr) {
		/* Read list of TABLE resources */
		Common::File in;
		if (!in.open(getFileName(GAME_TBLFILE))) {
			error("loadGamePcFile: Can't load table resources file '%s'", getFileName(GAME_TBLFILE));
		}

		fileSize = in.size();

		_tblList = (byte *)malloc(fileSize);
		if (_tblList == nullptr)
			error("loadGamePcFile: Out of memory for strip table list");
		in.read(_tblList, fileSize);

		/* Remember the current state */
		_subroutineListOrg = _subroutineList;
		_tablesHeapPtrOrg = _tablesHeapPtr;
		_tablesHeapCurPosOrg = _tablesHeapCurPos;
	}

	if (getFileName(GAME_STRFILE) != nullptr) {
		/* Read list of TEXT resources */
		Common::File in;
		if (!in.open(getFileName(GAME_STRFILE)))
			error("loadGamePcFile: Can't load text resources file '%s'", getFileName(GAME_STRFILE));

		fileSize = in.size();
		_strippedTxtMem = (byte *)malloc(fileSize);
		if (_strippedTxtMem == nullptr)
			error("loadGamePcFile: Out of memory for strip text list");
		in.read(_strippedTxtMem, fileSize);
	}

	if (getFileName(GAME_STATFILE) != nullptr) {
		/* Read list of ROOM STATE resources */
		Common::File in;
		if (!in.open(getFileName(GAME_STATFILE))) {
			error("loadGamePcFile: Can't load state resources file '%s'", getFileName(GAME_STATFILE));
		}

		_numRoomStates = in.size() / 8;

		_roomStates = (RoomState *)calloc(_numRoomStates, sizeof(RoomState));
		if (_roomStates == nullptr)
			error("loadGamePcFile: Out of memory for room state list");

		for (uint s = 0; s < _numRoomStates; s++) {
			uint16 num = in.readUint16BE() - (_itemArrayInited - 2);

			_roomStates[num].state = in.readUint16BE();
			_roomStates[num].classFlags = in.readUint16BE();
			_roomStates[num].roomExitStates = in.readUint16BE();
		}
	}

	if (getFileName(GAME_RMSLFILE) != nullptr) {
		/* Read list of ROOM ITEMS resources */
		Common::File in;
		if (!in.open(getFileName(GAME_RMSLFILE))) {
			error("loadGamePcFile: Can't load room resources file '%s'", getFileName(GAME_RMSLFILE));
		}

		fileSize = in.size();

		_roomsList = (byte *)malloc(fileSize);
		if (_roomsList == nullptr)
			error("loadGamePcFile: Out of memory for room items list");
		in.read(_roomsList, fileSize);
	}

	if (getFileName(GAME_XTBLFILE) != nullptr) {
		/* Read list of XTABLE resources */
		Common::File in;
		if (!in.open(getFileName(GAME_XTBLFILE))) {
			error("loadGamePcFile: Can't load xtable resources file '%s'", getFileName(GAME_XTBLFILE));
		}

		fileSize = in.size();

		_xtblList = (byte *)malloc(fileSize);
		if (_xtblList == nullptr)
			error("loadGamePcFile: Out of memory for strip xtable list");
		in.read(_xtblList, fileSize);

		/* Remember the current state */
		_xsubroutineListOrg = _subroutineList;
		_xtablesHeapPtrOrg = _tablesHeapPtr;
		_xtablesHeapCurPosOrg = _tablesHeapCurPos;
	}
}

void AGOSEngine::readGamePcFile(Common::SeekableReadStream *in) {
	int num_inited_objects;
	int i;

	num_inited_objects = allocGamePcVars(in);

	createPlayer();
	readGamePcText(in);

	for (i = 2; i < num_inited_objects; i++) {
		readItemFromGamePc(in, _itemArrayPtr[i]);
	}

	readSubroutineBlock(in);
}

void AGOSEngine::readGamePcText(Common::SeekableReadStream *in) {
	_textSize = in->readUint32BE();
	_textMem = (byte *)malloc(_textSize);
	if (_textMem == nullptr)
		error("readGamePcText: Out of text memory");

	in->read(_textMem, _textSize);

	setupStringTable(_textMem, _stringTabNum);
}

void AGOSEngine::readItemFromGamePc(Common::SeekableReadStream *in, Item *item) {
	uint32 type;

	if (getGameType() == GType_ELVIRA1) {
		item->itemName = (uint16)in->readUint32BE();
		item->adjective = in->readUint16BE();
		item->noun = in->readUint16BE();
		item->state = in->readUint16BE();
		in->readUint16BE();
		item->next = (uint16)fileReadItemID(in);
		item->child = (uint16)fileReadItemID(in);
		item->parent = (uint16)fileReadItemID(in);
		in->readUint16BE();
		in->readUint16BE();
		in->readUint16BE();
		item->classFlags = in->readUint16BE();
		item->children = nullptr;
	} else if (getGameType() == GType_ELVIRA2) {
		item->itemName = (uint16)in->readUint32BE();
		item->adjective = in->readUint16BE();
		item->noun = in->readUint16BE();
		item->state = in->readUint16BE();
		item->next = (uint16)fileReadItemID(in);
		item->child = (uint16)fileReadItemID(in);
		item->parent = (uint16)fileReadItemID(in);
		in->readUint16BE();
		item->classFlags = in->readUint16BE();
		item->children = nullptr;
	} else {
		item->adjective = in->readUint16BE();
		item->noun = in->readUint16BE();
		item->state = in->readUint16BE();
		item->next = (uint16)fileReadItemID(in);
		item->child = (uint16)fileReadItemID(in);
		item->parent = (uint16)fileReadItemID(in);
		in->readUint16BE();
		item->classFlags = in->readUint16BE();
		item->children = nullptr;
	}


	type = in->readUint32BE();
	while (type) {
		type = in->readUint16BE();
		if (type != 0)
			readItemChildren(in, item, type);
	}
}

void AGOSEngine::readItemChildren(Common::SeekableReadStream *in, Item *item, uint type) {
	if (type == kRoomType) {
		SubRoom *subRoom = (SubRoom *)allocateChildBlock(item, kRoomType, sizeof(SubRoom));
		subRoom->roomShort = in->readUint32BE();
		subRoom->roomLong = in->readUint32BE();
		subRoom->flags = in->readUint16BE();
	} else if (type == kObjectType) {
		SubObject *subObject = (SubObject *)allocateChildBlock(item, kObjectType, sizeof(SubObject));
		in->readUint32BE();
		in->readUint32BE();
		in->readUint32BE();
		subObject->objectName = in->readUint32BE();
		subObject->objectSize = in->readUint16BE();
		subObject->objectWeight = in->readUint16BE();
		subObject->objectFlags = in->readUint16BE();
	} else if (type == kGenExitType) {
		SubGenExit *genExit = (SubGenExit *)allocateChildBlock(item, kGenExitType, sizeof(SubGenExit));
		genExit->dest[0] = (uint16)fileReadItemID(in);
		genExit->dest[1] = (uint16)fileReadItemID(in);
		genExit->dest[2] = (uint16)fileReadItemID(in);
		genExit->dest[3] = (uint16)fileReadItemID(in);
		genExit->dest[4] = (uint16)fileReadItemID(in);
		genExit->dest[5] = (uint16)fileReadItemID(in);
		fileReadItemID(in);
		fileReadItemID(in);
		fileReadItemID(in);
		fileReadItemID(in);
		fileReadItemID(in);
		fileReadItemID(in);
	} else if (type == kContainerType) {
		SubContainer *container = (SubContainer *)allocateChildBlock(item, kContainerType, sizeof(SubContainer));
		container->volume = in->readUint16BE();
		container->flags = in->readUint16BE();
	} else if (type == kChainType) {
		SubChain *chain = (SubChain *)allocateChildBlock(item, kChainType, sizeof(SubChain));
		chain->chChained = (uint16)fileReadItemID(in);
	} else if (type == kUserFlagType) {
		setUserFlag(item, 0, in->readUint16BE());
		setUserFlag(item, 1, in->readUint16BE());
		setUserFlag(item, 2, in->readUint16BE());
		setUserFlag(item, 3, in->readUint16BE());
		setUserFlag(item, 4, in->readUint16BE());
		setUserFlag(item, 5, in->readUint16BE());
		setUserFlag(item, 6, in->readUint16BE());
		setUserFlag(item, 7, in->readUint16BE());
		SubUserFlag *subUserFlag = (SubUserFlag *)findChildOfType(item, kUserFlagType);
		subUserFlag->userItems[0] = (uint16)fileReadItemID(in);
		fileReadItemID(in);
		fileReadItemID(in);
		fileReadItemID(in);
	} else if (type == kInheritType) {
		SubInherit *inherit = (SubInherit *)allocateChildBlock(item, kInheritType, sizeof(SubInherit));
		inherit->inMaster = (uint16)fileReadItemID(in);
	} else {
		error("readItemChildren: invalid type %d", type);
	}
}

void AGOSEngine_Elvira2::readItemChildren(Common::SeekableReadStream *in, Item *item, uint type) {
	if (type == kRoomType) {
		uint fr1 = in->readUint16BE();
		uint fr2 = in->readUint16BE();
		uint i, size;
		uint j, k;
		SubRoom *subRoom;

		size = SubRoom_SIZE;
		for (i = 0, j = fr2; i != 6; i++, j >>= 2)
			if (j & 3)
				size += sizeof(subRoom->roomExit[0]);

		subRoom = (SubRoom *)allocateChildBlock(item, kRoomType, size);
		subRoom->subroutine_id = fr1;
		subRoom->roomExitStates = fr2;

		for (i = k = 0, j = fr2; i != 6; i++, j >>= 2)
			if (j & 3)
				subRoom->roomExit[k++] = (uint16)fileReadItemID(in);
	} else if (type == kObjectType) {
		uint32 fr = in->readUint32BE();
		uint i, k, size;
		SubObject *subObject;

		size = SubObject_SIZE;
		for (i = 0; i != 16; i++)
			if (fr & (1 << i))
				size += sizeof(subObject->objectFlagValue[0]);

		subObject = (SubObject *)allocateChildBlock(item, kObjectType, size);
		subObject->objectFlags = fr;

		k = 0;
		if (fr & 1) {
			subObject->objectFlagValue[k++] = (uint16)in->readUint32BE();
		}
		for (i = 1; i != 16; i++)
			if (fr & (1 << i))
				subObject->objectFlagValue[k++] = in->readUint16BE();

		if (getGameType() != GType_ELVIRA2)
			subObject->objectName = (uint16)in->readUint32BE();
	} else if (type == kSuperRoomType) {
		assert(getGameType() == GType_ELVIRA2);

		uint i, j, k, size;
		uint id, x, y, z;
		SubSuperRoom *subSuperRoom;

		id = in->readUint16BE();
		x = in->readUint16BE();
		y = in->readUint16BE();
		z = in->readUint16BE();

		j = x * y * z;
		size = SubSuperRoom_SIZE;
		for (i = 0; i != j; i++)
			size += sizeof(subSuperRoom->roomExitStates[0]);

		subSuperRoom = (SubSuperRoom *)allocateChildBlock(item, kSuperRoomType, size);
		subSuperRoom->subroutine_id = id;
		subSuperRoom->roomX = x;
		subSuperRoom->roomY = y;
		subSuperRoom->roomZ = z;

		for (i = k = 0; i != j; i++)
			subSuperRoom->roomExitStates[k++] = in->readUint16BE();
	} else if (type == kContainerType) {
		SubContainer *container = (SubContainer *)allocateChildBlock(item, kContainerType, sizeof(SubContainer));
		container->volume = in->readUint16BE();
		container->flags = in->readUint16BE();
	} else if (type == kChainType) {
		SubChain *chain = (SubChain *)allocateChildBlock(item, kChainType, sizeof(SubChain));
		chain->chChained = (uint16)fileReadItemID(in);
	} else if (type == kUserFlagType) {
		setUserFlag(item, 0, in->readUint16BE());
		setUserFlag(item, 1, in->readUint16BE());
		setUserFlag(item, 2, in->readUint16BE());
		setUserFlag(item, 3, in->readUint16BE());
	} else if (type == kInheritType) {
		SubInherit *inherit = (SubInherit *)allocateChildBlock(item, kInheritType, sizeof(SubInherit));
		inherit->inMaster = (uint16)fileReadItemID(in);
	} else {
		error("readItemChildren: invalid type %d", type);
	}
}

uint fileReadItemID(Common::SeekableReadStream *in) {
	uint32 val = in->readUint32BE();
	if (val == 0xFFFFFFFF)
		return 0;
	return val + 2;
}

void AGOSEngine::openGameFile() {
	_gameFile = new Common::File();
	_gameFile->open(getFileName(GAME_GMEFILE));

	if (!_gameFile->isOpen())
		error("openGameFile: Can't load game file '%s'", getFileName(GAME_GMEFILE));

	uint32 size = _gameFile->readUint32LE();

	_gameOffsetsPtr = (uint32 *)malloc(size);
	if (_gameOffsetsPtr == nullptr)
		error("openGameFile: Out of memory, game offsets");

	_gameFile->seek(0, SEEK_SET);

	for (uint r = 0; r < size / 4; r++)
		_gameOffsetsPtr[r] = _gameFile->readUint32LE();
}

void AGOSEngine::readGameFile(void *dst, uint32 offs, uint32 size) {
	_gameFile->seek(offs, SEEK_SET);
	if (_gameFile->read(dst, size) != size)
		error("readGameFile: Read failed (%d,%d)", offs, size);
}

// Thanks to Stuart Caie for providing the original
// C conversion upon which this decruncher is based.

#define SD_GETBIT(var) do {     \
	if (!bits--) {              \
		s -= 4;                 \
		if (s < src)            \
			return false;       \
		bb = READ_BE_UINT32(s); \
		bits = 31;              \
	}                           \
	(var) = bb & 1;             \
	bb >>= 1;                   \
}while (0)

#define SD_GETBITS(var, nbits) do { \
	bc = (nbits);                   \
	(var) = 0;                      \
	while (bc--) {                   \
		(var) <<= 1;                \
		SD_GETBIT(bit);             \
		(var) |= bit;               \
	}                               \
}while (0)

#define SD_TYPE_LITERAL (0)
#define SD_TYPE_MATCH   (1)

bool AGOSEngine::decrunchFile(byte *src, byte *dst, uint32 size) {
	byte *s = src + size - 4;
	uint32 destlen = READ_BE_UINT32 (s);
	uint32 bb, x, y;
	byte *d = dst + destlen;
	byte bc, bit, bits, type;

	// Initialize bit buffer.
	s -= 4;
	bb = x = READ_BE_UINT32 (s);
	bits = 0;
	do {
		x >>= 1;
		bits++;
	} while (x);
	bits--;

	while (d > dst) {
		SD_GETBIT(x);
		if (x) {
			SD_GETBITS(x, 2);
			switch (x) {
			case 0:
				type = SD_TYPE_MATCH;
				x = 9;
				y = 2;
				break;

			case 1:
				type = SD_TYPE_MATCH;
				x = 10;
				y = 3;
				break;

			case 2:
				type = SD_TYPE_MATCH;
				x = 12;
				SD_GETBITS(y, 8);
				break;

			default:
				type = SD_TYPE_LITERAL;
				x = 8;
				y = 8;
			}
		} else {
			SD_GETBIT(x);
			if (x) {
				type = SD_TYPE_MATCH;
				x = 8;
				y = 1;
			} else {
				type = SD_TYPE_LITERAL;
				x = 3;
				y = 0;
			}
		}

		if (type == SD_TYPE_LITERAL) {
			SD_GETBITS(x, x);
			y += x;
			if ((int)(y + 1) > (d - dst))
				return false; // Overflow?
			do {
				SD_GETBITS(x, 8);
				*--d = x;
			} while (y-- > 0);
		} else {
			if ((int)(y + 1) > (d - dst))
				return false; // Overflow?
			SD_GETBITS(x, x);
			if ((d + x) > (dst + destlen))
				return false; // Offset overflow?
			do {
				d--;
				*d = d[x];
			} while (y-- > 0);
		}
	}

	// Successful decrunch.
	return true;
}

#undef SD_GETBIT
#undef SD_GETBITS
#undef SD_TYPE_LITERAL
#undef SD_TYPE_MATCH

static bool getBit(Common::Stack<uint32> &dataList, uint32 &srcVal) {
	bool result = srcVal & 1;
	srcVal >>= 1;
	if (srcVal == 0) {
		srcVal = dataList.pop();

		result = srcVal & 1;
		srcVal = (srcVal >> 1) | 0x80000000L;
	}

	return result;
}

static uint32 copyBits(Common::Stack<uint32> &dataList, uint32 &srcVal, int numBits) {
	uint32 destVal = 0;

	for (int i = 0; i < numBits; ++i) {
		bool f = getBit(dataList, srcVal);
		destVal = (destVal << 1) | (f ? 1 : 0);
	}

	return destVal;
}

static void transferLoop(uint8 *dataOut, int &outIndex, uint32 destVal, int max) {
	assert(outIndex > max - 1);
	byte *pDest = dataOut + outIndex;

	 for (int i = 0; (i <= max) && (outIndex > 0); ++i) {
		pDest = dataOut + --outIndex;
		*pDest = pDest[destVal];
	 }
}

void AGOSEngine::decompressPN(Common::Stack<uint32> &dataList, uint8 *&dataOut, int &dataOutSize) {
	// Set up the output data area
	dataOutSize = dataList.pop();
	dataOut = new uint8[dataOutSize];
	int outIndex = dataOutSize;

	// Decompression routine
	uint32 srcVal = dataList.pop();
	uint32 destVal;

	while (outIndex > 0) {
		uint32 numBits = 0;
		int count = 0;

		if (getBit(dataList, srcVal)) {
			destVal = copyBits(dataList, srcVal, 2);

			if (destVal < 2) {
				count = destVal + 2;
				destVal = copyBits(dataList, srcVal, destVal + 9);
				transferLoop(dataOut, outIndex, destVal, count);
				continue;
			} else if (destVal != 3) {
				count = copyBits(dataList, srcVal, 8);
				destVal = copyBits(dataList, srcVal, 8);
				transferLoop(dataOut, outIndex, destVal, count);
				continue;
			} else {
				numBits = 8;
				count = 8;
			}
		} else if (getBit(dataList, srcVal)) {
			destVal = copyBits(dataList, srcVal, 8);
			transferLoop(dataOut, outIndex, destVal, 1);
			continue;
		} else {
			numBits = 3;
			count = 0;
		}

		destVal = copyBits(dataList, srcVal, numBits);
		count += destVal;

		// Loop through extracting specified number of bytes
		for (int i = 0; i <= count; ++i) {
			// Shift 8 bits from the source to the destination
			for (int bitCtr = 0; bitCtr < 8; ++bitCtr) {
				bool flag = getBit(dataList, srcVal);
				destVal = (destVal << 1) | (flag ? 1 : 0);
			}

			dataOut[--outIndex] = destVal & 0xff;
		}
	}
}

void AGOSEngine::loadVGABeardFile(uint16 id) {
	uint32 offs, size;

	if (getFeatures() & GF_OLD_BUNDLE) {
		Common::File in;
		char filename[15];
		if (id == 23)
			id = 112;
		else if (id == 328)
			id = 119;

		if (getPlatform() == Common::kPlatformAmiga) {
			if (getFeatures() & GF_TALKIE)
				Common::sprintf_s(filename, "0%d.out", id);
			else
				Common::sprintf_s(filename, "0%d.pkd", id);
		} else if (getPlatform() == Common::kPlatformAcorn) {
			Common::sprintf_s(filename, "%.2d/0%d", id / 100, id);
		} else {
			Common::sprintf_s(filename, "0%d.VGA", id);
		}

		if (!in.open(filename))
			error("loadSimonVGAFile: Can't load %s", filename);

		size = in.size();
		if (getFeatures() & GF_CRUNCHED) {
			byte *srcBuffer = (byte *)malloc(size);
			if (in.read(srcBuffer, size) != size)
				error("loadSimonVGAFile: Read failed");
			decrunchFile(srcBuffer, _vgaBufferPointers[11].vgaFile2, size);
			free(srcBuffer);
		} else {
			if (in.read(_vgaBufferPointers[11].vgaFile2, size) != size)
				error("loadSimonVGAFile: Read failed");
		}
	} else {
		offs = _gameOffsetsPtr[id];
		size = _gameOffsetsPtr[id + 1] - offs;
		readGameFile(_vgaBufferPointers[11].vgaFile2, offs, size);
	}
}

uint8 safeReadByte(const uint8 *&src, const uint8 *end) {
	if (src < end)
		return (*src++);
	error("decodePak98(): invalid data");
	return 0;
}

uint16 safeReadWord(const uint8 *&src, const uint8 *end) {
	uint8 lo = safeReadByte(src, end);
	return (safeReadByte(src, end) << 8) | lo;
}

#define S_NEXTBYTE safeReadByte(src, endSrc)
#define S_NEXTWORD safeReadWord(src, endSrc)

void decodePak98(uint8 *dst, uint32 outSize, const uint8 *src, uint32 inSize) {
	const uint8 *end = dst + outSize;
	const uint8 *endSrc = src + inSize;
	uint8 state = 0x80;
	uint8 flg = S_NEXTBYTE;

	for (uint32 srcBytesLeft = inSize - 1; srcBytesLeft; ) {
		if (state & flg) {
			if (dst < end)
				*dst++ = S_NEXTBYTE;
			--srcBytesLeft;
		} else {
			srcBytesLeft -= 2;
			uint16 cmd2 = S_NEXTWORD;
			uint8 cmd3 = cmd2 & 0x0F;
			cmd2 >>= 4;

			if (cmd2 == 0) {
				uint16 count = cmd3 + 4;
				--srcBytesLeft;
				if (cmd3 == 0x0F) {
					count = S_NEXTWORD;
					srcBytesLeft -= 2;
				} else if (cmd3 == 0x0E) {
					count = 18 + (S_NEXTBYTE);
					--srcBytesLeft;
				}

				uint8 destVal = S_NEXTBYTE;
				while (count-- && dst < end)
					*dst++ = destVal;

			} else if (cmd2 == 1) {
				uint16 count = cmd3 + 3;
				if (cmd3 == 0x0F) {
					count = S_NEXTWORD;
					srcBytesLeft -= 2;
				} else if (cmd3 == 0x0E) {
					count = 17 + (S_NEXTBYTE);
					--srcBytesLeft;
				}

				dst += count;

			} else if (cmd2 == 2) {
				uint16 count = cmd3 + 16;
				if (cmd3 == 0x0F) {
					count = S_NEXTWORD;
					srcBytesLeft -= 2;
				} else if (cmd3 == 0x0E) {
					count = 30 + (S_NEXTBYTE);
					--srcBytesLeft;
				}

				srcBytesLeft -= count;
				while (count-- && dst < end)
					*dst++ = S_NEXTBYTE;

			} else {
				uint16 count = cmd3 + 3;
				if (cmd3 == 0x0F) {
					count = 18 + (S_NEXTBYTE);
					--srcBytesLeft;
				}

				const uint8 *src2 = dst - cmd2;
				while (count-- && dst < end)
					*dst++ = *src2++;
			}
		}

		if (!(state >>= 1)) {
			state = 0x80;
			if (srcBytesLeft) {
				flg = S_NEXTBYTE;
				srcBytesLeft--;
			}
		}
	}
}

#undef S_NEXTBYTE
#undef S_NEXTWORD

void AGOSEngine::loadVGAVideoFile(uint16 id, uint8 type, bool useError) {
	Common::File in;
	char filename[15];
	byte *dst;
	uint32 file, offs, srcSize, dstSize;
	uint extraBuffer = 0;

	if ((getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) &&
		id == 2 && type == 2) {
		// WORKAROUND: For the extra long strings in foreign languages
		// Allocate more space for text to cope with foreign languages that use
		// up more space than English. I hope 6400 bytes are enough. This number
		// is base on: 2 (lines) * 320 (screen width) * 10 (textheight) -- olki
		extraBuffer += 6400;
	}

	if (getFeatures() & GF_ZLIBCOMP) {
		loadOffsets(getFileName(GAME_GFXIDXFILE), id * 3 + type, file, offs, srcSize, dstSize);

		if (getPlatform() == Common::kPlatformAmiga)
			Common::sprintf_s(filename, "GFX%d.VGA", file);
		else
			Common::sprintf_s(filename, "graphics.vga");

		dst = allocBlock(dstSize + extraBuffer);
		decompressData(filename, dst, offs, srcSize, dstSize);
	} else if (getFeatures() & GF_OLD_BUNDLE) {
		if (getPlatform() == Common::kPlatformAcorn) {
			Common::sprintf_s(filename, "%.2d/%.3d%d", id / 10, id, type);
		} else if (getPlatform() == Common::kPlatformAmiga || getPlatform() == Common::kPlatformAtariST) {
			if (getFeatures() & GF_TALKIE) {
				Common::sprintf_s(filename, "%.3d%d.out", id, type);
			} else if (getGameType() == GType_ELVIRA1 && getFeatures() & GF_DEMO) {
				if (getPlatform() == Common::kPlatformAtariST)
					Common::sprintf_s(filename, "%.2d%d.out", id, type);
				else
					Common::sprintf_s(filename, "%c%d.out", 48 + id, type);
			} else if (getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2) {
				Common::sprintf_s(filename, "%.2d%d.pkd", id, type);
			} else if (getGameType() == GType_PN) {
				Common::sprintf_s(filename, "%c%d.in", id + 48, type);
			} else {
				Common::sprintf_s(filename, "%.3d%d.pkd", id, type);
			}
		} else {
			if (getGameType() == GType_ELVIRA1 && getPlatform() == Common::kPlatformPC98) {
				Common::sprintf_s(filename, "%.2d.GR2", id);
			} else if (getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2 || getGameType() == GType_WW) {
				Common::sprintf_s(filename, "%.2d%d.VGA", id, type);
			} else if (getGameType() == GType_PN) {
				Common::sprintf_s(filename, "%c%d.out", id + 48, type);
			} else {
				Common::sprintf_s(filename, "%.3d%d.VGA", id, type);
			}
		}

		if (!in.open(filename)) {
			if (useError)
				error("loadVGAVideoFile: Can't load %s", filename);

			_block = _blockEnd = nullptr;
			return;
		}

		dstSize = srcSize = in.size();
		if (getGameType() == GType_PN && getPlatform() == Common::kPlatformDOS && id == 17 && type == 2) {
			// The A2.out file isn't compressed in PC version of Personal Nightmare
			dst = allocBlock(dstSize + extraBuffer);
			if (in.read(dst, dstSize) != dstSize)
				error("loadVGAVideoFile: Read failed");
		} else if (getGameType() == GType_PN && (getFeatures() & GF_CRUNCHED)) {
			Common::Stack<uint32> data;
			byte *dataOut = nullptr;
			int dataOutSize = 0;

			for (uint i = 0; i < srcSize / 4; ++i) {
				uint32 dataVal = in.readUint32BE();
				// Correct incorrect byte, in corrupt 72.out file, included in some PC versions.
				if (dataVal == 168042714)
					data.push(168050906);
				else
					data.push(dataVal);
			}

			decompressPN(data, dataOut, dataOutSize);
			dst = allocBlock (dataOutSize + extraBuffer);
			memcpy(dst, dataOut, dataOutSize);
			delete[] dataOut;
		} else if (getFeatures() & GF_CRUNCHED) {
			byte *srcBuffer = (byte *)malloc(srcSize);
			if (in.read(srcBuffer, srcSize) != srcSize)
				error("loadVGAVideoFile: Read failed");

			dstSize = READ_BE_UINT32(srcBuffer + srcSize - 4);
			dst = allocBlock (dstSize + extraBuffer);
			decrunchFile(srcBuffer, dst, srcSize);
			free(srcBuffer);
		} else if (getPlatform() == Common::kPlatformPC98) {
			bool compressed = (in.readUint16LE() == 1);
			srcSize = in.readUint32LE();
			if (type == 1) {
				if (compressed)
					srcSize = in.readUint32LE() + 2;
				in.seek(srcSize, SEEK_CUR);
				compressed = (in.readUint16LE() == 1);
				srcSize = in.readUint32LE();
			}

			if (compressed) {
				dstSize = srcSize;
				srcSize = in.readUint32LE();
				uint16 fill = in.readUint16LE();
				dst = allocBlock(dstSize);

				Common::fill<uint16*, uint16>((uint16*)dst, (uint16*)(dst + (dstSize & ~1)), TO_LE_16(fill));
				if (dstSize & 1)
					*(dst + dstSize - 1) = fill & 0xff;

				if (srcSize) {
					uint8 *srcBuffer = new uint8[srcSize];
					if (in.read(srcBuffer, srcSize) != srcSize)
						error("loadVGAVideoFile: Read failed");
					decodePak98(dst, dstSize, srcBuffer, srcSize);
					delete[] srcBuffer;
				}
			} else {
				dstSize = srcSize;
				dst = allocBlock(dstSize + extraBuffer);
				if (in.read(dst, dstSize) != dstSize)
					error("loadVGAVideoFile: Read failed");
			}
		} else {
			dst = allocBlock(dstSize + extraBuffer);
			if (in.read(dst, dstSize) != dstSize)
				error("loadVGAVideoFile: Read failed");
		}
	} else {
		id = id * 2 + (type - 1);
		offs = _gameOffsetsPtr[id];
		dstSize = _gameOffsetsPtr[id + 1] - offs;

		if (!dstSize) {
			if (useError)
				error("loadVGAVideoFile: Can't load id %d type %d", id, type);

			_block = _blockEnd = nullptr;
			return;
		}

		dst = allocBlock(dstSize + extraBuffer);
		readGameFile(dst, offs, dstSize);
	}
}

Common::SeekableReadStream *AGOSEngine::createPak98FileStream(const char *filename) {
	Common::File in;
	if (!in.open(filename))
		return nullptr;

	/*uint16 cmpType = */in.readUint16LE();
	uint32 outSize = in.readUint32LE();
	uint32 inSize = in.readUint32LE();
	uint16 fill = in.readUint16LE();

	uint8 *decBuffer = (uint8*)malloc(outSize);
	Common::fill<uint16*, uint16>((uint16*)decBuffer, (uint16*)(decBuffer + (outSize & ~1)), TO_LE_16(fill));
	if (outSize & 1)
		*(decBuffer + outSize - 1) = fill & 0xff;

	if (inSize) {
		uint8 *tempBuffer = new uint8[inSize];
		in.read(tempBuffer, inSize);
		decodePak98(decBuffer, outSize, tempBuffer, inSize);
		delete[] tempBuffer;
	}

	return new Common::MemoryReadStream(decBuffer, outSize, DisposeAfterUse::YES);
}

void AGOSEngine::convertPC98Image(VC10_state &state) {
	if (state.flags & (kDFCompressedFlip | kDFCompressed)) {
		const byte *src = state.srcPtr;
		uint32 outSize = READ_LE_UINT32(src + 2);
		assert(outSize >= (uint32)((state.width << 3) * state.height));
		uint32 inSize = READ_LE_UINT32(src + 6);
		uint16 fill = READ_LE_UINT16(src + 10);
		delete[] _pak98Buf;
		byte *decBuffer = new uint8[outSize];
		Common::fill<uint16*, uint16>((uint16*)decBuffer, (uint16*)(decBuffer + (outSize & ~1)), TO_LE_16(fill));
		if (outSize & 1)
			*(decBuffer + outSize - 1) = fill & 0xff;
		if (inSize)
			decodePak98(decBuffer, outSize, src + 12, inSize);
		_pak98Buf = state.srcPtr = decBuffer;
		_paletteModNext = 0;
	}

	// The PC-98 images are in a planar format, but slightly different from the Amiga format. It does
	// not make much sense to set the GF_PLANAR flag, since the Amiga code can't be used anyway.
	free(_planarBuf);
	uint16 planeLW = state.width << 1;
	uint16 planePitch = planeLW * 3;

	_planarBuf = (byte*)malloc((state.width << 3) * state.height);

	const byte *src[4];
	memset(src, 0, sizeof(src));
	for (int i = 0; i < 4; ++i)
		src[i] = state.srcPtr + i * planeLW;
	byte *dst = _planarBuf;

	for (int y = 0; y < state.height; ++y) {
		for (int x = 0; x < planeLW; ++x) {
			for (int i = 0; i <= 6; i += 2) {
				byte col = 0;
				for (int ii = 0; ii < 4; ++ii) {
					col |= ((*src[ii] >> (7 - i)) & 1) << (ii + 4);
					col |= ((*src[ii] >> (6 - i)) & 1) << ii;
				}
				*dst++ = col;
			}
			for (int ii = 0; ii < 4; ++ii)
				++src[ii];
		}
		for (int ii = 0; ii < 4; ++ii)
			src[ii] += planePitch;
	}

	state.srcPtr = _planarBuf;
	if (state.flags & kDFCompressedFlip)
		state.flags |= kDFFlip;
	state.flags &= ~(kDFCompressedFlip | kDFCompressed);
}

} // End of namespace AGOS
