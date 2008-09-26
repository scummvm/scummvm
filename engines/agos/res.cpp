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

// Resource file routines for Simon1/Simon2


#include "common/file.h"

#include "agos/agos.h"
#include "agos/intern.h"
#include "agos/sound.h"

#include "common/zlib.h"

using Common::File;

namespace AGOS {

uint16 AGOSEngine::to16Wrapper(uint value) {
	if (getGameType() == GType_FF || getGameType() == GType_PP)
		return TO_LE_16(value);
	else
		return TO_BE_16(value);
}

uint16 AGOSEngine::readUint16Wrapper(const void *src) {
	if (getGameType() == GType_FF || getGameType() == GType_PP)
		return READ_LE_UINT16(src);
	else
		return READ_BE_UINT16(src);
}

uint32 AGOSEngine::readUint32Wrapper(const void *src) {
	if (getGameType() == GType_FF || getGameType() == GType_PP)
		return READ_LE_UINT32(src);
	else
		return READ_BE_UINT32(src);
}

void AGOSEngine::decompressData(const char *srcName, byte *dst, uint32 offset, uint32 srcSize, uint32 dstSize) {
#ifdef USE_ZLIB
		File in;
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
	uint item_array_size, item_array_inited, stringtable_num;
	uint32 version;
	uint i;

	item_array_size = in->readUint32BE();
	version = in->readUint32BE();
	item_array_inited = in->readUint32BE();
	stringtable_num = in->readUint32BE();

	// First two items are predefined
	if (getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2) {
		item_array_size += 2;
		item_array_inited = item_array_size;
	} else {
		item_array_inited += 2;
		item_array_size += 2;
	}

	if (version != 0x80)
		error("allocGamePcVars: Not a runtime database");

	_itemArrayPtr = (Item **)calloc(item_array_size, sizeof(Item *));
	if (_itemArrayPtr == NULL)
		error("allocGamePcVars: Out of memory for Item array");

	_itemArraySize = item_array_size;
	_itemArrayInited = item_array_inited;

	for (i = 1; i < item_array_inited; i++) {
		_itemArrayPtr[i] = (Item *)allocateItem(sizeof(Item));
	}

	// The rest is cleared automatically by calloc
	allocateStringTable(stringtable_num + 10);
	_stringTabNum = stringtable_num;

	return item_array_inited;
}

void AGOSEngine::loadGamePcFile() {
	Common::File in;
	int fileSize;

	if (getFileName(GAME_BASEFILE) != NULL) {
		/* Read main gamexx file */
		in.open(getFileName(GAME_BASEFILE));
		if (in.isOpen() == false) {
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
		in.close();
	}

	if (getFileName(GAME_TBLFILE) != NULL) {
		/* Read list of TABLE resources */
		in.open(getFileName(GAME_TBLFILE));
		if (in.isOpen() == false) {
			error("loadGamePcFile: Can't load table resources file '%s'", getFileName(GAME_TBLFILE));
		}

		fileSize = in.size();

		_tblList = (byte *)malloc(fileSize);
		if (_tblList == NULL)
			error("loadGamePcFile: Out of memory for strip table list");
		in.read(_tblList, fileSize);
		in.close();

		/* Remember the current state */
		_subroutineListOrg = _subroutineList;
		_tablesHeapPtrOrg = _tablesHeapPtr;
		_tablesHeapCurPosOrg = _tablesHeapCurPos;
	}

	if (getFileName(GAME_STRFILE) != NULL) {
		/* Read list of TEXT resources */
		in.open(getFileName(GAME_STRFILE));
		if (in.isOpen() == false)
			error("loadGamePcFile: Can't load text resources file '%s'", getFileName(GAME_STRFILE));

		fileSize = in.size();
		_strippedTxtMem = (byte *)malloc(fileSize);
		if (_strippedTxtMem == NULL)
			error("loadGamePcFile: Out of memory for strip text list");
		in.read(_strippedTxtMem, fileSize);
		in.close();
	}

	if (getFileName(GAME_STATFILE) != NULL) {
		/* Read list of ROOM STATE resources */
		in.open(getFileName(GAME_STATFILE));
		if (in.isOpen() == false) {
			error("loadGamePcFile: Can't load state resources file '%s'", getFileName(GAME_STATFILE));
		}

		_numRoomStates = in.size() / 8;

		_roomStates = (RoomState *)calloc(_numRoomStates, sizeof(RoomState));
		if (_roomStates == NULL)
			error("loadGamePcFile: Out of memory for room state list");

		for (uint s = 0; s < _numRoomStates; s++) {
			uint16 num = in.readUint16BE() - (_itemArrayInited - 2);

			_roomStates[num].state = in.readUint16BE();
			_roomStates[num].classFlags = in.readUint16BE();
			_roomStates[num].roomExitStates = in.readUint16BE();
		}
		in.close();
	}

	if (getFileName(GAME_RMSLFILE) != NULL) {
		/* Read list of ROOM ITEMS resources */
		in.open(getFileName(GAME_RMSLFILE));
		if (in.isOpen() == false) {
			error("loadGamePcFile: Can't load room resources file '%s'", getFileName(GAME_RMSLFILE));
		}

		fileSize = in.size();

		_roomsList = (byte *)malloc(fileSize);
		if (_roomsList == NULL)
			error("loadGamePcFile: Out of memory for room items list");
		in.read(_roomsList, fileSize);
		in.close();
	}

	if (getFileName(GAME_XTBLFILE) != NULL) {
		/* Read list of XTABLE resources */
		in.open(getFileName(GAME_XTBLFILE));
		if (in.isOpen() == false) {
			error("loadGamePcFile: Can't load xtable resources file '%s'", getFileName(GAME_XTBLFILE));
		}

		fileSize = in.size();

		_xtblList = (byte *)malloc(fileSize);
		if (_xtblList == NULL)
			error("loadGamePcFile: Out of memory for strip xtable list");
		in.read(_xtblList, fileSize);
		in.close();

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
	if (_textMem == NULL)
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
		item->children = NULL;
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
		item->children = NULL;
	} else {
		item->adjective = in->readUint16BE();
		item->noun = in->readUint16BE();
		item->state = in->readUint16BE();
		item->next = (uint16)fileReadItemID(in);
		item->child = (uint16)fileReadItemID(in);
		item->parent = (uint16)fileReadItemID(in);
		in->readUint16BE();
		item->classFlags = in->readUint16BE();
		item->children = NULL;
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
	_gameFile = new File();
	_gameFile->open(getFileName(GAME_GMEFILE));

	if (!_gameFile->isOpen())
		error("openGameFile: Can't load game file '%s'", getFileName(GAME_GMEFILE));

	uint32 size = _gameFile->readUint32LE();

	_gameOffsetsPtr = (uint32 *)malloc(size);
	if (_gameOffsetsPtr == NULL)
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

void AGOSEngine::loadVGABeardFile(uint16 id) {
	uint32 offs, size;

	if (getFeatures() & GF_OLD_BUNDLE) {
		File in;
		char filename[15];
		if (id == 23)
			id = 112;
		else if (id == 328)
			id = 119;

		if (getPlatform() == Common::kPlatformAmiga) {
			if (getFeatures() & GF_TALKIE)
				sprintf(filename, "0%d.out", id);
			else
				sprintf(filename, "0%d.pkd", id);
		} else {
			sprintf(filename, "0%d.VGA", id);
		}

		in.open(filename);
		if (in.isOpen() == false)
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
		in.close();
	} else {
		offs = _gameOffsetsPtr[id];

		size = _gameOffsetsPtr[id + 1] - offs;
		readGameFile(_vgaBufferPointers[11].vgaFile2, offs, size);
	}
}

void AGOSEngine::loadVGAVideoFile(uint16 id, uint8 type) {
	File in;
	char filename[15];
	byte *dst;
	uint32 file, offs, srcSize, dstSize;
	uint extraBuffer = 0;

	if ((getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) &&
		id == 2 && type == 2) {
		// WORKAROUND: For the exta long strings in foreign languages
		// Allocate more space for text to cope with foreign languages that use
		// up more space than english. I hope 6400 bytes are enough. This number
		// is base on: 2 (lines) * 320 (screen width) * 10 (textheight) -- olki
		extraBuffer += 6400;
	}

	if (getFeatures() & GF_ZLIBCOMP) {
		loadOffsets(getFileName(GAME_GFXIDXFILE), id * 3 + type, file, offs, srcSize, dstSize);

		if (getPlatform() == Common::kPlatformAmiga)
			sprintf(filename, "GFX%d.VGA", file);
		else
			sprintf(filename, "graphics.vga");

		dst = allocBlock(dstSize + extraBuffer);
		decompressData(filename, dst, offs, srcSize, dstSize);
	} else if (getFeatures() & GF_OLD_BUNDLE) {
		if (getPlatform() == Common::kPlatformAcorn) {
			sprintf(filename, "%.3d%d.DAT", id, type);
		} else if (getPlatform() == Common::kPlatformAmiga || getPlatform() == Common::kPlatformAtariST) {
			if (getFeatures() & GF_TALKIE) {
				sprintf(filename, "%.3d%d.out", id, type);
			} else if (getGameType() == GType_ELVIRA1 && getFeatures() & GF_DEMO) {
				if (getPlatform() == Common::kPlatformAtariST)
					sprintf(filename, "%.2d%d.out", id, type);
				else if (id == 20)
					sprintf(filename, "D%d.out", type);
				else if (id == 26)
					sprintf(filename, "J%d.out", type);
				else if (id == 27)
					sprintf(filename, "K%d.out", type);
				else if (id == 33)
					sprintf(filename, "Q%d.out", type);
				else if (id == 34)
					sprintf(filename, "R%d.out", type);
				else
					sprintf(filename, "%.1d%d.out", id, type);
			} else if (getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2) {
				sprintf(filename, "%.2d%d.pkd", id, type);
			} else {
				sprintf(filename, "%.3d%d.pkd", id, type);
			}
		} else {
			if (getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2 || getGameType() == GType_WW) {
				sprintf(filename, "%.2d%d.VGA", id, type);
			} else {
				sprintf(filename, "%.3d%d.VGA", id, type);
			}
		}

		in.open(filename);
		if (in.isOpen() == false) {
			error("loadVGAVideoFile: Can't load %s", filename);
		}

		dstSize = srcSize = in.size();
		if (getFeatures() & GF_CRUNCHED) {
			byte *srcBuffer = (byte *)malloc(srcSize);
			if (in.read(srcBuffer, srcSize) != srcSize)
				error("loadVGAVideoFile: Read failed");

			dstSize = READ_BE_UINT32(srcBuffer + srcSize - 4);
			dst = allocBlock (dstSize + extraBuffer);
			decrunchFile(srcBuffer, dst, srcSize);
			free(srcBuffer);
		} else {
			dst = allocBlock(dstSize + extraBuffer);
			if (in.read(dst, dstSize) != dstSize)
				error("loadVGAVideoFile: Read failed");
		}
		in.close();
	} else {
		id = id * 2 + (type - 1);
		offs = _gameOffsetsPtr[id];

		dstSize = _gameOffsetsPtr[id + 1] - offs;
		dst = allocBlock(dstSize + extraBuffer);
		readGameFile(dst, offs, dstSize);
	}
}

} // End of namespace AGOS
