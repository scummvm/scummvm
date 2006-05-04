/* ScummVM - Scumm Interpreter
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

#include "simon/simon.h"
#include "simon/intern.h"
#include "simon/sound.h"


#ifdef USE_ZLIB
#include <zlib.h>
#endif

using Common::File;

namespace Simon {

// Script opcodes to load into memory
static const char *const opcode_arg_table_simon1win[256] = {
	" ", "I ", "I ", "I ", "I ", "I ", "I ", "II ", "II ", "II ", "II ", "B ", "B ", "BN ", "BN ",
	"BN ", "BN ", "BB ", "BB ", "BB ", "BB ", "II ", "II ", "N ", "I ", "I ", "I ", "IN ", "IB ",
	"II ", "I ", "I ", "II ", "II ", "IBB ", "BIB ", "BB ", "B ", "BI ", "IB ", "B ", "B ", "BN ",
	"BN ", "BN ", "BB ", "BB ", "BN ", "BN ", "BB ", "BB ", "BN ", "BB ", "BN ", "B ", "I ", "IB ",
	"IB ", "II ", "I ", "I ", "IN ", "B ", "T ", "T ", "NNNNNB ", "BT ", "BTS ", "T ", " ", "B ",
	"N ", "IBN ", "I ", "I ", "I ", "NN ", " ", " ", "IT ", "II ", "I ", "B ", " ", "IB ", "IBB ",
	"IIB ", "T ", " ", " ", "IB ", "IB ", "IB ", "B ", "BB ", "IBB ", "NB ", "N ", "NBNNN ", "N ",
	" ", "BNNNNNN ", "B ", " ", "B ", "B ", "BB ", "NNNNNIN ", "N ", "N ", "N ", "NNN ", "NBNN ",
	"IBNN ", "IB ", "IB ", "IB ", "IB ", "N ", "N ", "N ", "BI ", " ", " ", "N ", "I ", "IBB ",
	"NN ", "N ", "N ", "Ban ", "BB ", " ", " ", " ", " ", "IB ", "B ", " ", "II ", " ", "BI ", "N ",
	"I ", "IB ", "IB ", "IB ", "IB ", "IB ", "IB ", "IB ", "BI ", "BB ", "B ", "B ", "B ", "B ",
	"IBB ", "IBN ", "IB ", "B ", "BNBN ", "BBTS ", "N ", " ", "Ian ", "B ", "B ", "B ", "B ", "T ",
	"T ", "B ", " ", "I ", " ", " ", "BBI ", "NNBB ", "BBB ", " ", " ", " ", " ", "N ", "N ", " ",
	" ",
};

static const char *const opcode_arg_table_simon1dos[256] = {
	" ", "I ", "I ", "I ", "I ", "I ", "I ", "II ", "II ", "II ", "II ", "B ", "B ", "BN ", "BN ",
	"BN ", "BN ", "BB ", "BB ", "BB ", "BB ", "II ", "II ", "N ", "I ", "I ", "I ", "IN ", "IB ",
	"II ", "I ", "I ", "II ", "II ", "IBB ", "BIB ", "BB ", "B ", "BI ", "IB ", "B ", "B ", "BN ",
	"BN ", "BN ", "BB ", "BB ", "BN ", "BN ", "BB ", "BB ", "BN ", "BB ", "BN ", "B ", "I ", "IB ",
	"IB ", "II ", "I ", "I ", "IN ", "B ", "T ", "T ", "NNNNNB ", "BT ", "BT ", "T ", " ", "B ",
	"N ", "IBN ", "I ", "I ", "I ", "NN ", " ", " ", "IT ", "II ", "I ", "B ", " ", "IB ", "IBB ",
	"IIB ", "T ", " ", " ", "IB ", "IB ", "IB ", "B ", "BB ", "IBB ", "NB ", "N ", "NBNNN ", "N ",
	" ", "BNNNNNN ", "B ", " ", "B ", "B ", "BB ", "NNNNNIN ", "N ", "N ", "N ", "NNN ", "NBNN ",
	"IBNN ", "IB ", "IB ", "IB ", "IB ", "N ", "N ", "N ", "BI ", " ", " ", "N ", "I ", "IBB ",
	"NN ", "N ", "N ", "Ban ", "BB ", " ", " ", " ", " ", "IB ", "B ", " ", "II ", " ", "BI ", "N ",
	"I ", "IB ", "IB ", "IB ", "IB ", "IB ", "IB ", "IB ", "BI ", "BB ", "B ", "B ", "B ", "B ",
	"IBB ", "IBN ", "IB ", "B ", "BNBN ", "BBT ", "N ", " ", "Ian ", "B ", "B ", "B ", "B ", "T ",
	"T ", "B ", " ", "I ", " ", " ", "BBI ", "NNBB ", "BBB ", " ", " ", " ", " ", "N ", "N ", " ",
	" ",
};

static const char *const opcode_arg_table_simon2win[256] = {
	" ", "I ", "I ", "I ", "I ", "I ", "I ", "II ", "II ", "II ", "II ", "B ", "B ", "BN ", "BN ",
	"BN ", "BN ", "BB ", "BB ", "BB ", "BB ", "II ", "II ", "N ", "I ", "I ", "I ", "IN ", "IB ",
	"II ", "I ", "I ", "II ", "II ", "IBB ", "BIB ", "BB ", "B ", "BI ", "IB ", "B ", "B ", "BN ",
	"BN ", "BN ", "BB ", "BB ", "BN ", "BN ", "BB ", "BB ", "BN ", "BB ", "BN ", "B ", "I ", "IB ",
	"IB ", "II ", "I ", "I ", "IN ", "B ", "T ", "T ", "NNNNNB ", "BT ", "BTS ", "T ", " ", "B ",
	"N ", "IBN ", "I ", "I ", "I ", "NN ", " ", " ", "IT ", "II ", "I ", "B ", " ", "IB ", "IBB ",
	"IIB ", "T ", " ", " ", "IB ", "IB ", "IB ", "B ", "BB ", "IBB ", "NB ", "N ", "NNBNNN ", "NN ",
	" ", "BNNNNNN ", "B ", " ", "B ", "B ", "BB ", "NNNNNIN ", "N ", "N ", "N ", "NNN ", "NBNN ",
	"IBNN ", "IB ", "IB ", "IB ", "IB ", "N ", "N ", "N ", "BI ", " ", " ", "N ", "I ", "IBB ",
	"NNB ", "N ", "N ", "Ban ", "BB ", " ", " ", " ", " ", "IB ", "B ", " ", "II ", " ", "BI ",
	"N ", "I ", "IB ", "IB ", "IB ", "IB ", "IB ", "IB ", "IB ", "BI ", "BB ", "B ", "B ", "B ",
	"B ", "IBB ", "IBN ", "IB ", "B ", "BNBN ", "BBTS ", "N ", " ", "Ian ", "B ", "B ", "B ", "B ",
	"T ", "T ", "B ", " ", "I ", " ", " ", "BBI ", "NNBB ", "BBB ", " ", " ", " ", " ", "N ", "N ",
	" ", " ", "BT ", " ", "B "
};

static const char *const opcode_arg_table_simon2dos[256] = {
	" ", "I ", "I ", "I ", "I ", "I ", "I ", "II ", "II ", "II ", "II ", "B ", "B ", "BN ", "BN ",
	"BN ", "BN ", "BB ", "BB ", "BB ", "BB ", "II ", "II ", "N ", "I ", "I ", "I ", "IN ", "IB ",
	"II ", "I ", "I ", "II ", "II ", "IBB ", "BIB ", "BB ", "B ", "BI ", "IB ", "B ", "B ", "BN ",
	"BN ", "BN ", "BB ", "BB ", "BN ", "BN ", "BB ", "BB ", "BN ", "BB ", "BN ", "B ", "I ", "IB ",
	"IB ", "II ", "I ", "I ", "IN ", "B ", "T ", "T ", "NNNNNB ", "BT ", "BT ", "T ", " ", "B ",
	"N ", "IBN ", "I ", "I ", "I ", "NN ", " ", " ", "IT ", "II ", "I ", "B ", " ", "IB ", "IBB ",
	"IIB ", "T ", " ", " ", "IB ", "IB ", "IB ", "B ", "BB ", "IBB ", "NB ", "N ", "NNBNNN ", "NN ",
	" ", "BNNNNNN ", "B ", " ", "B ", "B ", "BB ", "NNNNNIN ", "N ", "N ", "N ", "NNN ", "NBNN ",
	"IBNN ", "IB ", "IB ", "IB ", "IB ", "N ", "N ", "N ", "BI ", " ", " ", "N ", "I ", "IBB ",
	"NNB ", "N ", "N ", "Ban ", "BB ", " ", " ", " ", " ", "IB ", "B ", " ", "II ", " ", "BI ",
	"N ", "I ", "IB ", "IB ", "IB ", "IB ", "IB ", "IB ", "IB ", "BI ", "BB ", "B ", "B ", "B ",
	"B ", "IBB ", "IBN ", "IB ", "B ", "BNBN ", "BBT ", "N ", " ", "Ian ", "B ", "B ", "B ", "B ",
	"T ", "T ", "B ", " ", "I ", " ", " ", "BBI ", "NNBB ", "BBB ", " ", " ", " ", " ", "N ", "N ",
	" ", " ", "BT ", " ", "B "
};

static const char *const opcode_arg_table_feeblefiles[256] = {
	" ", "I ", "I ", "I ", "I ", "I ", "I ", "II ", "II ", "II ", "II ", "B ", "B ", "BN ", "BN ",
	"BN ", "BN ", "BB ", "BB ", "BB ", "BB ", "II ", "II ", "N ", "I ", "I ", "I ", "IN ", "IB ",
	"II ", "I ", "I ", "II ", "II ", "IBB ", "BIB ", "BB ", "B ", "BI ", "IB ", "B ", "B ", "BN ",
	"BN ", "BN ", "BB ", "BB ", "BN ", "BN ", "BB ", "BB ", "BN ", "BB ", "BN ", "B ", "I ", "IB ",
	"IB ", "II ", "I ", "I ", "IN ", "B ", "T ", "T ", "NNNNNB ", "BT ", "BTS ", "T ", " ", "B ",
	"N ", "IBN ", "I ", "I ", "I ", "NN ", " ", " ", "IT ", "II ", "I ", "B ", " ", "IB ", "IBB ",
	"IIB ", "T ", " ", " ", "IB ", "IB ", "IB ", "B ", "BB ", "IBB ", "NB ", "N ", "NNBNNN ", "NN ",
	" ", "BNNNNNN ", "B ", " ", "B ", "B ", "BB ", "NNNNNIN ", "N ", "N ", "N ", "NNN ", "NBNN ",
	"IBNN ", "IB ", "IB ", "IB ", "IB ", "N ", "N ", "N ", "BI ", " ", " ", "N ", "I ", "IBB ",
	"NNB ", "N ", "N ", "Ban ", " ", " ", " ", " ", " ", "IB ", "B ", " ", "II ", " ", "BI ",
	"N ", "I ", "IB ", "IB ", "IB ", "IB ", "IB ", "IB ", "IB ", "BI ", "BB ", "B ", "B ", "B ",
	"B ", "IBB ", "IBN ", "IB ", "B ", "BNNN ", "BBTS ", "N ", " ", "Ian ", "B ", "B ", "B ", "B ",
	"T ", "N ", " ", " ", "I ", " ", " ", "BBI ", "NNBB ", "BBB ", " ", " ", "T ", " ", "N ", "N ",
	" ", " ", "BT ", " ", "B ", " ", "BBBB ", " ", " ", "BBBB ", "B ", "B ", "B ", "B "
};

uint16 SimonEngine::to16Wrapper(uint value) {
	if (getGameType() == GType_FF)
		return TO_LE_16(value);
	else
		return TO_BE_16(value);
}

uint16 SimonEngine::readUint16Wrapper(const void *src) {
	if (getGameType() == GType_FF)
		return READ_LE_UINT16(src);
	else
		return READ_BE_UINT16(src);
}

uint32 SimonEngine::readUint32Wrapper(const void *src) {
	if (getGameType() == GType_FF)
		return READ_LE_UINT32(src);
	else
		return READ_BE_UINT32(src);
}

void SimonEngine::decompressData(const char *srcName, byte *dst, uint32 offset, uint32 srcSize, uint32 dstSize) {
#ifdef USE_ZLIB
		File in;
		in.open(srcName);
		if (in.isOpen() == false)
			error("decompressData: can't open %s", srcName);

		in.seek(offset, SEEK_SET);
		if (srcSize != dstSize) {
			byte *srcBuffer = (byte *)malloc(srcSize);

			if (in.read(srcBuffer, srcSize) != srcSize)
				error("decompressData: read failed");

			unsigned long decompressedSize = dstSize;
			int result = uncompress(dst, &decompressedSize, srcBuffer, srcSize);
			if (result != Z_OK)
				error("decompressData() Zlib uncompress error");
			free(srcBuffer);
		} else {
			if (in.read(dst, dstSize) != dstSize)
				error("decompressData: read failed");
		}
		in.close();
#else
	error("Zlib support is required for Amiga and Macintosh versions");
#endif
}

void SimonEngine::loadOffsets(const char *filename, int number, uint32 &file, uint32 &offset, uint32 &srcSize, uint32 &dstSize) {
	Common::File in;

	int offsSize = (getPlatform() == Common::kPlatformAmiga) ? 16 : 12;

	/* read offsets from index */
	in.open(filename);
	if (in.isOpen() == false) {
		error("Can't open index file '%s'", filename);
	}

	in.seek(number * offsSize, SEEK_SET);
	offset = in.readUint32LE();
	dstSize = in.readUint32LE();
	srcSize = in.readUint32LE();
	file = in.readUint32LE();
	in.close();
}

int SimonEngine::allocGamePcVars(File *in) {
	uint item_array_size, item_array_inited, stringtable_num;
	uint32 version;
	uint i;

	item_array_size = in->readUint32BE();
	version = in->readUint32BE();
	item_array_inited = in->readUint32BE();
	stringtable_num = in->readUint32BE();

	item_array_inited += 2;				// first two items are predefined
	item_array_size += 2;

	if (version != 0x80)
		error("Not a runtime database");

	_itemArrayPtr = (Item **)calloc(item_array_size, sizeof(Item *));
	if (_itemArrayPtr == NULL)
		error("Out of memory for Item array");

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

void SimonEngine::loadGamePcFile() {
	Common::File in;
	int num_inited_objects;
	int i, file_size;

	/* read main gamepc file */
	in.open(getFileName(GAME_BASEFILE));
	if (in.isOpen() == false) {
		error("Can't open gamepc file '%s'", getFileName(GAME_BASEFILE));
	}

	num_inited_objects = allocGamePcVars(&in);

	createPlayer();
	readGamePcText(&in);

	for (i = 2; i < num_inited_objects; i++) {
		readItemFromGamePc(&in, _itemArrayPtr[i]);
	}

	readSubroutineBlock(&in);

	in.close();

	/* Read list of TABLE resources */
	in.open(getFileName(GAME_TBLFILE));
	if (in.isOpen() == false) {
		error("Can't open table resources file '%s'", getFileName(GAME_TBLFILE));
	}

	file_size = in.size();

	_tblList = (byte *)malloc(file_size);
	if (_tblList == NULL)
		error("Out of memory for strip table list");
	in.read(_tblList, file_size);
	in.close();

	/* Remember the current state */
	_subroutineListOrg = _subroutineList;
	_tablesHeapPtrOrg = _tablesHeapPtr;
	_tablesHeapCurPosOrg = _tablesHeapCurPos;

	if (getGameType() == GType_FF)
		return;

	/* Read list of TEXT resources */
	in.open(getFileName(GAME_STRFILE));
	if (in.isOpen() == false)
		error("Can't open text resources file '%s'", getFileName(GAME_STRFILE));

	file_size = in.size();
	_strippedTxtMem = (byte *)malloc(file_size);
	if (_strippedTxtMem == NULL)
		error("Out of memory for strip text list");
	in.read(_strippedTxtMem, file_size);
	in.close();
}

void SimonEngine::readGamePcText(Common::File *in) {
	_textSize = in->readUint32BE();
	_textMem = (byte *)malloc(_textSize);
	if (_textMem == NULL)
		error("Out of text memory");

	in->read(_textMem, _textSize);

	setupStringTable(_textMem, _stringTabNum);
}

void SimonEngine::readItemFromGamePc(Common::File *in, Item *item) {
	uint32 type;

	item->adjective = in->readUint16BE();
	item->noun = in->readUint16BE();
	item->state = in->readUint16BE();
	item->sibling = (uint16)fileReadItemID(in);
	item->child = (uint16)fileReadItemID(in);
	item->parent = (uint16)fileReadItemID(in);
	in->readUint16BE();
	item->classFlags = in->readUint16BE();
	item->children = NULL;

	type = in->readUint32BE();
	while (type) {
		type = in->readUint16BE();
		if (type != 0)
			readItemChildren(in, item, type);
	}
}

void SimonEngine::readItemChildren(Common::File *in, Item *item, uint type) {
	if (type == 1) {
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
	} else if (type == 2) {
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

		subObject->objectName = (uint16)in->readUint32BE();
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

byte *SimonEngine::readSingleOpcode(Common::File *in, byte *ptr) {
	int i, l;
	const char *string_ptr;
	uint val;

	const char *const *table;

	if (getGameType() == GType_FF) {
		table = opcode_arg_table_feeblefiles;
	} else if ((getGameType() == GType_SIMON2) && (getFeatures() & GF_TALKIE))
		table = opcode_arg_table_simon2win;
	else if (getGameType() == GType_SIMON2)
		table = opcode_arg_table_simon2dos;
	else if (getFeatures() & GF_TALKIE)
		table = opcode_arg_table_simon1win;
	else
		table = opcode_arg_table_simon1dos;

	i = 0;

	string_ptr = table[*ptr++];
	if (!string_ptr)
		error("Unable to locate opcode table. Perhaps you are using the wrong game target?");

	for (;;) {
		if (string_ptr[i] == ' ')
			return ptr;

		l = string_ptr[i++];
		switch (l) {
		case 'N':
		case 'S':
		case 'a':
		case 'n':
		case 'p':
		case 'v':
			val = in->readUint16BE();
			*ptr++ = val >> 8;
			*ptr++ = val & 255;
			break;

		case 'B':
			*ptr++ = in->readByte();
			if (ptr[-1] == 0xFF) {
				*ptr++ = in->readByte();
			}
			break;

		case 'I':
			val = in->readUint16BE();
			switch (val) {
			case 1:
				val = 0xFFFF;
				break;
			case 3:
				val = 0xFFFD;
				break;
			case 5:
				val = 0xFFFB;
				break;
			case 7:
				val = 0xFFF9;
				break;
			case 9:
				val = 0xFFF7;
				break;
			default:
				val = fileReadItemID(in);;
			}
			*ptr++ = val >> 8;
			*ptr++ = val & 255;
			break;

		case 'T':
			val = in->readUint16BE();
			switch (val) {
			case 0:
				val = 0xFFFF;
				break;
			case 3:
				val = 0xFFFD;
				break;
			default:
				val = (uint16)in->readUint32BE();
				break;
			}
			*ptr++ = val >> 8;
			*ptr++ = val & 255;
			break;
		default:
			error("Bad cmd table entry %c", l);
		}
	}
}

void SimonEngine::openGameFile() {
	if (!(getFeatures() & GF_OLD_BUNDLE)) {
		_gameFile = new File();
		_gameFile->open(getFileName(GAME_GMEFILE));

		if (_gameFile->isOpen() == false)
			error("Can't open game file '%s'", getFileName(GAME_GMEFILE));

		uint32 size = _gameFile->readUint32LE();

		_gameOffsetsPtr = (uint32 *)malloc(size);
		if (_gameOffsetsPtr == NULL)
			error("out of memory, game offsets");

		readGameFile(_gameOffsetsPtr, 0, size);
#if defined(SCUMM_BIG_ENDIAN)
		for (uint r = 0; r < size / 4; r++)
			_gameOffsetsPtr[r] = FROM_LE_32(_gameOffsetsPtr[r]);
#endif
	}
}

void SimonEngine::readGameFile(void *dst, uint32 offs, uint32 size) {
	_gameFile->seek(offs, SEEK_SET);
	if (_gameFile->read(dst, size) != size)
		error("readGameFile(%d,%d) read failed", offs, size);
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

static bool decrunchFile(byte *src, byte *dst, uint32 size) {
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

void SimonEngine::loadSimonVGAFile(uint vga_id) {
	uint32 offs, size;

	if (getFeatures() & GF_OLD_BUNDLE) {
		File in;
		char filename[15];
		if (vga_id == 23)
			vga_id = 112;
		if (vga_id == 328)
			vga_id = 119;

		if (getPlatform() == Common::kPlatformAmiga) {
			if (getFeatures() & GF_TALKIE)
				sprintf(filename, "0%d.out", vga_id);
			else 
				sprintf(filename, "0%d.pkd", vga_id);
		} else {
			sprintf(filename, "0%d.VGA", vga_id);
		}

		in.open(filename);
		if (in.isOpen() == false)
			error("loadSimonVGAFile: can't open %s", filename);

		size = in.size();
		if (getFeatures() & GF_CRUNCHED) {
			byte *srcBuffer = (byte *)malloc(size);
			if (in.read(srcBuffer, size) != size)
				error("loadSimonVGAFile: read failed");
			decrunchFile(srcBuffer, _vgaBufferPointers[11].vgaFile2, size);
			free(srcBuffer);
		} else {
			if (in.read(_vgaBufferPointers[11].vgaFile2, size) != size)
				error("loadSimonVGAFile: read failed");
		}
		in.close();
	} else {
		offs = _gameOffsetsPtr[vga_id];

		size = _gameOffsetsPtr[vga_id + 1] - offs;
		readGameFile(_vgaBufferPointers[11].vgaFile2, offs, size);
	}
}

byte *SimonEngine::loadVGAFile(uint id, uint type, uint32 &dstSize) {
	File in;
	char filename[15];
	byte *dst = NULL;
	uint32 file, offs, srcSize;
	uint extraBuffer = 0;

	if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
		// !!! HACK !!!
		// Allocate more space for text to cope with foreign languages that use
		// up more space than english. I hope 6400 bytes are enough. This number
		// is base on: 2 (lines) * 320 (screen width) * 10 (textheight) -- olki
		extraBuffer = (id == 5 ? 6400 : 0);
	}

	if (getFeatures() & GF_ZLIBCOMP) {
		if (getPlatform() == Common::kPlatformAmiga) {
			loadOffsets((const char*)"gfxindex.dat", id / 2 * 3 + type, file, offs, srcSize, dstSize);
		} else {
			loadOffsets((const char*)"graphics.vga", id / 2 * 3 + type, file, offs, srcSize, dstSize);
		}

		if (getPlatform() == Common::kPlatformAmiga)
			sprintf(filename, "GFX%d.VGA", file);
		else
			sprintf(filename, "graphics.vga");

		dst = allocBlock(dstSize + extraBuffer);
		decompressData(filename, dst, offs, srcSize, dstSize);
	} else if (getFeatures() & GF_OLD_BUNDLE) {
		if (getPlatform() == Common::kPlatformAmiga) {
			if (getFeatures() & GF_TALKIE)
				sprintf(filename, "%.3d%d.out", id / 2, type);
			else 
				sprintf(filename, "%.3d%d.pkd", id / 2, type);
		} else {
			sprintf(filename, "%.3d%d.VGA", id / 2, type);
		}

		in.open(filename);
		if (in.isOpen() == false) {
			if (type == 3) 
				return NULL;
			else
				error("loadVGAFile: can't open %s", filename);
		}

		dstSize = srcSize = in.size();
		if (getFeatures() & GF_CRUNCHED) {
			byte *srcBuffer = (byte *)malloc(srcSize);
			if (in.read(srcBuffer, srcSize) != srcSize)
				error("loadVGAFile: read failed");

			dstSize = READ_BE_UINT32(srcBuffer + srcSize - 4);
			dst = allocBlock (dstSize + extraBuffer);
			decrunchFile(srcBuffer, dst, srcSize);
			free(srcBuffer);
		} else {
			dst = allocBlock(dstSize + extraBuffer);
			if (in.read(dst, dstSize) != dstSize)
				error("loadVGAFile: read failed");
		}
		in.close();
	} else {
		offs = _gameOffsetsPtr[id];

		dstSize = _gameOffsetsPtr[id + 1] - offs;
		dst = allocBlock(dstSize + extraBuffer);
		readGameFile(dst, offs, dstSize);
	}

	dstSize += extraBuffer;
	return dst;
}

void SimonEngine::loadSound(uint sound, int pan, int vol, uint type) {
	byte *dst;

	if (getFeatures() & GF_ZLIBCOMP) {
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
		dst = _curSfxFile + READ_LE_UINT32(_curSfxFile + sound * 4);
	}

	if (type == 3)
		_sound->playSfx5Data(dst, sound, pan, vol);
	else if (type == 2)
		_sound->playAmbientData(dst, sound, pan, vol);
	else
		_sound->playSfxData(dst, sound, pan, vol);
}

void SimonEngine::loadVoice(uint speechId) {
	if (getFeatures() & GF_ZLIBCOMP) {
		char filename[15];

		uint32 file, offset, srcSize, dstSize;
		if (getPlatform() == Common::kPlatformAmiga) {
			loadOffsets((const char*)"spindex.dat", speechId, file, offset, srcSize, dstSize);
		} else {
			loadOffsets((const char*)"speech.wav", speechId, file, offset, srcSize, dstSize);
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

} // End of namespace Simon
