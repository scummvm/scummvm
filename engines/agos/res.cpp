/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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
#include "common/stdafx.h"

#include "common/file.h"

#include "agos/agos.h"
#include "agos/intern.h"
#include "agos/sound.h"


#ifdef USE_ZLIB
#include <zlib.h>
#endif

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
			int result = uncompress(dst, &decompressedSize, srcBuffer, srcSize);
			if (result != Z_OK)
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

int AGOSEngine::allocGamePcVars(File *in) {
	uint item_array_size, item_array_inited, stringtable_num;
	uint32 version;
	uint i;

	item_array_size = in->readUint32BE();
	version = in->readUint32BE();
	item_array_inited = in->readUint32BE();
	stringtable_num = in->readUint32BE();

	if (getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2) {
		item_array_size += 2;
		item_array_inited = item_array_size;
	} else {
		item_array_inited += 2;				// first two items are predefined
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
	int num_inited_objects;
	int i, file_size;

	/* read main gamepc file */
	in.open(getFileName(GAME_BASEFILE));
	if (in.isOpen() == false) {
		error("loadGamePcFile: Can't load gamepc file '%s'", getFileName(GAME_BASEFILE));
	}

	num_inited_objects = allocGamePcVars(&in);

	createPlayer();
	readGamePcText(&in);

	for (i = 2; i < num_inited_objects; i++) {
		readItemFromGamePc(&in, _itemArrayPtr[i]);
	}

	readSubroutineBlock(&in);
	in.close();

	if (getFileName(GAME_TBLFILE) != NULL) {
		/* Read list of TABLE resources */
		in.open(getFileName(GAME_TBLFILE));
		if (in.isOpen() == false) {
			error("loadGamePcFile: Can't load table resources file '%s'", getFileName(GAME_TBLFILE));
		}

		file_size = in.size();

		_tblList = (byte *)malloc(file_size);
		if (_tblList == NULL)
			error("loadGamePcFile: Out of memory for strip table list");
		in.read(_tblList, file_size);
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

		file_size = in.size();
		_strippedTxtMem = (byte *)malloc(file_size);
		if (_strippedTxtMem == NULL)
			error("loadGamePcFile: Out of memory for strip text list");
		in.read(_strippedTxtMem, file_size);
		in.close();
	}

	if (getFileName(GAME_RMSLFILE) != NULL) {
		/* Read list of ROOM ITEMS resources */
		in.open(getFileName(GAME_RMSLFILE));
		if (in.isOpen() == false) {
			error("loadGamePcFile: Can't load room resources file '%s'", getFileName(GAME_XTBLFILE));
		}

		file_size = in.size();

		_roomsList = (byte *)malloc(file_size);
		if (_roomsList == NULL)
			error("loadGamePcFile: Out of memory for room items list");
		in.read(_roomsList, file_size);
		in.close();
	}

	if (getFileName(GAME_XTBLFILE) != NULL) {
		/* Read list of XTABLE resources */
		in.open(getFileName(GAME_XTBLFILE));
		if (in.isOpen() == false) {
			error("loadGamePcFile: Can't load xtable resources file '%s'", getFileName(GAME_XTBLFILE));
		}

		file_size = in.size();

		_xtblList = (byte *)malloc(file_size);
		if (_xtblList == NULL)
			error("loadGamePcFile: Out of memory for strip xtable list");
		in.read(_xtblList, file_size);
		in.close();

		/* Remember the current state */
		_xsubroutineListOrg = _subroutineList;
		_xtablesHeapPtrOrg = _tablesHeapPtr;
		_xtablesHeapCurPosOrg = _tablesHeapCurPos;
	}
}

void AGOSEngine::readGamePcText(Common::File *in) {
	_textSize = in->readUint32BE();
	_textMem = (byte *)malloc(_textSize);
	if (_textMem == NULL)
		error("readGamePcText: Out of text memory");

	in->read(_textMem, _textSize);

	setupStringTable(_textMem, _stringTabNum);
}

void AGOSEngine::readItemFromGamePc(Common::File *in, Item *item) {
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

void AGOSEngine::readItemChildren(Common::File *in, Item *item, uint type) {
	if (type == 1) {
		if (getGameType() == GType_ELVIRA1) {
			SubRoom *subRoom = (SubRoom *)allocateChildBlock(item, 1, sizeof(SubRoom));
			subRoom->roomShort = in->readUint32BE();
			subRoom->roomLong = in->readUint32BE();
			subRoom->flags = in->readUint16BE();
		} else {
			uint fr1 = in->readUint16BE();
			uint fr2 = in->readUint16BE();
			uint i, size;
			uint j, k;
			SubRoom *subRoom;

			size = SubRoom_SIZE;
			for (i = 0, j = fr2; i != 6; i++, j >>= 2)
				if (j & 3)
					size += sizeof(subRoom->roomExit[0]);

			subRoom = (SubRoom *)allocateChildBlock(item, 1, size);
			subRoom->subroutine_id = fr1;
			subRoom->roomExitStates = fr2;

			for (i = k = 0, j = fr2; i != 6; i++, j >>= 2)
				if (j & 3)
					subRoom->roomExit[k++] = (uint16)fileReadItemID(in);
		}
	} else if (type == 2) {
		if (getGameType() == GType_ELVIRA1) {
			SubObject *subObject = (SubObject *)allocateChildBlock(item, 2, sizeof(SubObject));
			in->readUint32BE();
			in->readUint32BE();
			in->readUint32BE();
			subObject->objectName = in->readUint32BE();
			subObject->objectSize = in->readUint16BE();
			subObject->objectWeight = in->readUint16BE();
			subObject->objectFlags = in->readUint16BE();
		} else {
			uint32 fr = in->readUint32BE();
			uint i, k, size;
			SubObject *subObject;

			size = SubObject_SIZE;
			for (i = 0; i != 16; i++)
				if (fr & (1 << i))
					size += sizeof(subObject->objectFlagValue[0]);

			subObject = (SubObject *)allocateChildBlock(item, 2, size);
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
		}
	} else if (type == 4) {
		if (getGameType() == GType_ELVIRA2) {
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
				size += sizeof(subSuperRoom->roomExit[0]);

			subSuperRoom = (SubSuperRoom *)allocateChildBlock(item, 4, size);
			subSuperRoom->subroutine_id = id;
			subSuperRoom->roomX = x;
			subSuperRoom->roomY = y;
			subSuperRoom->roomZ = z;
			subSuperRoom->roomExitStates = (uint16 *)calloc(j, sizeof(uint16));

			for (i = k = 0; i != j; i++)
					subSuperRoom->roomExit[k++] = in->readUint16BE();
		} else if (getGameType() == GType_ELVIRA1) {
			SubGenExit *genExit = (SubGenExit *)allocateChildBlock(item, 4, sizeof(SubGenExit));
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
		}
	} else if (type == 7) {
		SubContainer *container = (SubContainer *)allocateChildBlock(item, 7, sizeof(SubContainer));
		container->volume = in->readUint16BE();
		container->flags = in->readUint16BE();
	} else if (type == 8) {
		SubUserChain *chain = (SubUserChain *)allocateChildBlock(item, 8, sizeof(SubUserChain));
		chain->chChained = (uint16)fileReadItemID(in);
	} else if (type == 9) {
		setUserFlag(item, 0, in->readUint16BE());
		setUserFlag(item, 1, in->readUint16BE());
		setUserFlag(item, 2, in->readUint16BE());
		setUserFlag(item, 3, in->readUint16BE());
		if (getGameType() == GType_ELVIRA1) {
			setUserFlag(item, 4, in->readUint16BE());
			setUserFlag(item, 5, in->readUint16BE());
			setUserFlag(item, 6, in->readUint16BE());
			setUserFlag(item, 7, in->readUint16BE());
			SubUserFlag *subUserFlag = (SubUserFlag *) findChildOfType(item, 9);
			subUserFlag->userItems[0] = (uint16)fileReadItemID(in); 
			fileReadItemID(in);
			fileReadItemID(in);
			fileReadItemID(in);
		}
	} else if (type == 255) {
		SubUserInherit *inherit = (SubUserInherit *)allocateChildBlock(item, 255, sizeof(SubUserInherit));
		inherit->inMaster = (uint16)fileReadItemID(in);
	} else {
		error("readItemChildren: invalid type %d", type);
	}
}

uint fileReadItemID(Common::File *in) {
	uint32 val = in->readUint32BE();
	if (val == 0xFFFFFFFF)
		return 0;
	return val + 2;
}

void AGOSEngine::openGameFile() {
	if (getFileName(GAME_GMEFILE) != NULL) {
		_gameFile = new File();
		_gameFile->open(getFileName(GAME_GMEFILE));

		if (_gameFile->isOpen() == false)
			error("openGameFile: Can't load game file '%s'", getFileName(GAME_GMEFILE));

		uint32 size = _gameFile->readUint32LE();

		_gameOffsetsPtr = (uint32 *)malloc(size);
		if (_gameOffsetsPtr == NULL)
			error("openGameFile: Out of memory, game offsets");

		readGameFile(_gameOffsetsPtr, 0, size);
#if defined(SCUMM_BIG_ENDIAN)
		for (uint r = 0; r < size / 4; r++)
			_gameOffsetsPtr[r] = FROM_LE_32(_gameOffsetsPtr[r]);
#endif
	}
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

void AGOSEngine::loadSimonVGAFile(uint id) {
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

bool AGOSEngine::loadVGAFile(uint id, uint type) {
	File in;
	char filename[15];
	byte *dst = NULL;
	uint32 file, offs, srcSize, dstSize;
	uint extraBuffer = 0;

	if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
		// !!! HACK !!!
		// Allocate more space for text to cope with foreign languages that use
		// up more space than english. I hope 6400 bytes are enough. This number
		// is base on: 2 (lines) * 320 (screen width) * 10 (textheight) -- olki
		extraBuffer = (id == 5 ? 6400 : 0);
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
		if (getPlatform() == Common::kPlatformAmiga || getPlatform() == Common::kPlatformAtariST) {
			if (getFeatures() & GF_TALKIE) {
				sprintf(filename, "%.3d%d.out", id, type);
			} else if (getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2) {
				if (getGameId() == GID_ELVIRA1DEMO) {
					if (id == 20)
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
				} else {
					sprintf(filename, "%.2d%d.pkd", id, type);
				}
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
			// Sound VGA files don't always exist
			if (type == 3) {
				return false;
			} else {
				error("loadVGAFile: Can't load %s", filename);
			}
		}

		dstSize = srcSize = in.size();
		if (getFeatures() & GF_CRUNCHED) {
			byte *srcBuffer = (byte *)malloc(srcSize);
			if (in.read(srcBuffer, srcSize) != srcSize)
			error("loadVGAFile: Read failed");

			dstSize = READ_BE_UINT32(srcBuffer + srcSize - 4);
			if (type == 2 && dstSize != 64800) {
				dst = (byte *)malloc(dstSize);
				decrunchFile(srcBuffer, dst, srcSize);
				convertAmiga(dst, dstSize);
				free(dst);
			} else {
				dst = allocBlock (dstSize + extraBuffer);
				decrunchFile(srcBuffer, dst, srcSize);
			}
			free(srcBuffer);
		} else {
			if (getGameId() == GID_SIMON1CD32 && type == 2) {
				dst = (byte *)malloc(dstSize);
				if (in.read(dst, dstSize) != dstSize)
					error("loadVGAFile: Read failed");
				convertAmiga(dst, dstSize);
				free(dst);
			} else {
				dst = allocBlock(dstSize + extraBuffer);
				if (in.read(dst, dstSize) != dstSize)
					error("loadVGAFile: Read failed");
			}
		}
		in.close();
	} else {
		id = id * 2 + (type - 1);
		offs = _gameOffsetsPtr[id];

		dstSize = _gameOffsetsPtr[id + 1] - offs;
		dst = allocBlock(dstSize + extraBuffer);
		readGameFile(dst, offs, dstSize);
	}

	return true;
}

static const char *dimpSoundList[32] = {
	"Beep",
	"Birth",
	"Boiling",
	"Burp",
	"Cough",
	"Die1",
	"Die2",
	"Fart",
	"Inject",
	"Killchik",
	"Puke",
	"Lights",
	"Shock",
	"Snore",
	"Snotty",
	"Whip",
	"Whistle",
	"Work1",
	"Work2",
	"Yawn",
	"And0w",
	"And0x",
	"And0y",
	"And0z",
	"And10",
	"And11",
	"And12",
	"And13",
	"And14",
	"And15",
	"And16",
	"And17",
};


void AGOSEngine::loadSound(uint sound, int pan, int vol, uint type) {
	byte *dst;

	if (getGameId() == GID_DIMP) {
		File in;
		char filename[15];

		assert(sound >= 1 && sound <= 32);
		sprintf(filename, "%s.wav", dimpSoundList[sound - 1]);

		in.open(filename);
		if (in.isOpen() == false)
			error("loadSound: Can't load %s", filename);

		uint32 dstSize = in.size();
		dst = (byte *)malloc(dstSize);
		if (in.read(dst, dstSize) != dstSize)
			error("loadSound: Read failed");
		in.close();
	} else if (getFeatures() & GF_ZLIBCOMP) {
		char filename[15];

		uint32 file, offset, srcSize, dstSize;
		if (getPlatform() == Common::kPlatformAmiga) {
			loadOffsets((const char*)"sfxindex.dat", _zoneNumber * 22 + sound, file, offset, srcSize, dstSize);
		} else {
			loadOffsets((const char*)"effects.wav", _zoneNumber * 22 + sound, file, offset, srcSize, dstSize);
		}

		if (getPlatform() == Common::kPlatformAmiga)
			sprintf(filename, "sfx%d.wav", file);
		else
			sprintf(filename, "effects.wav");

		dst = (byte *)malloc(dstSize);
		decompressData(filename, dst, offset, srcSize, dstSize);
	} else {
		if (!_curSfxFile)
			error("loadSound: Can't load sound data file '%d3.VGA'", _zoneNumber);

		dst = _curSfxFile + READ_LE_UINT32(_curSfxFile + sound * 4);
	}

	if (type == 3)
		_sound->playSfx5Data(dst, sound, pan, vol);
	else if (type == 2)
		_sound->playAmbientData(dst, sound, pan, vol);
	else
		_sound->playSfxData(dst, sound, pan, vol);
}

void AGOSEngine::loadVoice(uint speechId) {
	if (getGameType() == GType_PP && speechId == 99) {
		_sound->stopVoice();
		return;
	}

	if (getFeatures() & GF_ZLIBCOMP) {
		char filename[15];

		uint32 file, offset, srcSize, dstSize;
		if (getPlatform() == Common::kPlatformAmiga) {
			loadOffsets((const char*)"spindex.dat", speechId, file, offset, srcSize, dstSize);
		} else {
			loadOffsets((const char*)"speech.wav", speechId, file, offset, srcSize, dstSize);
		}

		// Voice segment doesn't exist
		if (offset == 0xFFFFFFFF && srcSize == 0xFFFFFFFF && dstSize == 0xFFFFFFFF) {
			debug(0, "loadVoice: speechId %d removed", speechId);
			return;
		}

		if (getPlatform() == Common::kPlatformAmiga)
			sprintf(filename, "sp%d.wav", file);
		else
			sprintf(filename, "speech.wav");

		byte *dst = (byte *)malloc(dstSize);
		decompressData(filename, dst, offset, srcSize, dstSize);
		_sound->playVoiceData(dst, speechId);
	} else {
		_sound->playVoice(speechId);
	}
}

} // End of namespace AGOS
