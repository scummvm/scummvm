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

void SimonEngine::loadGamePcFile(const char *filename) {
	Common::File in;
	int num_inited_objects;
	int i, file_size;

	/* read main gamepc file */
	in.open(filename);
	if (in.isOpen() == false) {
		char *filename2;
		filename2 = (char *)malloc(strlen(filename) + 2);
		strcpy(filename2, filename);
		strcat(filename2, ".");
		in.open(filename2);
		free(filename2);
		if (in.isOpen() == false)
			error("Can't open gamepc file '%s' or '%s.'", gss->gamepc_filename, gss->gamepc_filename);
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
	in.open("TBLLIST");
	if (in.isOpen() == false) {
		in.open("TBLLIST.");
		if (in.isOpen() == false)
			error("Can't open table resources file 'TBLLIST' or 'TBLLIST.'");
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
	if (getPlatform() == Common::kPlatformAcorn)
		in.open("STRIPPED");
	else
		in.open("STRIPPED.TXT");
	if (in.isOpen() == false)
		error("Can't open text resources file 'STRIPPED.TXT'");

	file_size = in.size();
	_strippedTxtMem = (byte *)malloc(file_size);
	if (_strippedTxtMem == NULL)
		error("Out of memory for strip text list");
	in.read(_strippedTxtMem, file_size);
	in.close();
}

void SimonEngine::readGamePcText(Common::File *in) {
	uint text_size;
	byte *text_mem;

	_textSize = text_size = in->readUint32BE();
	text_mem = (byte *)malloc(text_size);
	if (text_mem == NULL)
		error("Out of text memory");

	in->read(text_mem, text_size);

	setupStringTable(text_mem, _stringTabNum);
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

void SimonEngine::read_vga_from_datfile_1(uint vga_id) {
	if (getFeatures() & GF_OLD_BUNDLE) {
		File in;
		char filename[15];
		uint32 size;
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
			error("read_vga_from_datfile_1: can't open %s", filename);
		size = in.size();

		if (getFeatures() & GF_CRUNCHED) {
			byte *buffer = new byte[size];
			if (in.read(buffer, size) != size)
				error("read_vga_from_datfile_1: read failed");
			decrunchFile(buffer, _vgaBufferPointers[11].vgaFile2, size);
			delete [] buffer;
		} else {
			if (in.read(_vgaBufferPointers[11].vgaFile2, size) != size)
				error("read_vga_from_datfile_1: read failed");
		}
		in.close();
	} else {
		uint32 offs_a = _gameOffsetsPtr[vga_id];
		uint32 size = _gameOffsetsPtr[vga_id + 1] - offs_a;

		resfile_read(_vgaBufferPointers[11].vgaFile2, offs_a, size);
	}
}

byte *SimonEngine::read_vga_from_datfile_2(uint id, uint type) {
	File in;
	char filename[15];
	byte *dst = NULL;

	// !!! HACK !!!
	// allocate more space for text to cope with foreign languages that use
	// up more space than english. I hope 6400 bytes are enough. This number
	// is base on: 2 (lines) * 320 (screen width) * 10 (textheight) -- olki
	int extraBuffer = (id == 5 ? 6400 : 0);

	if (getFeatures() & GF_ZLIBCOMP) {
		uint32 file, offset, srcSize, dstSize;
		if (getPlatform() == Common::kPlatformAmiga) {
			loadOffsets((const char*)"gfxindex.dat", id / 2 * 3 + type, file, offset, srcSize, dstSize);
		} else {
			loadOffsets((const char*)"graphics.vga", id / 2 * 3 + type, file, offset, srcSize, dstSize);
		}

		if (getPlatform() == Common::kPlatformAmiga)
			sprintf(filename, "GFX%d.VGA", file);
		else
			sprintf(filename, "graphics.vga");

		dst = allocBlock(dstSize);
		decompressData(filename, dst, offset, srcSize, dstSize);
		return dst;
	} else if (getFeatures() & GF_OLD_BUNDLE) {
		uint32 size;
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
				error("read_vga_from_datfile_2: can't open %s", filename);
		}
		size = in.size();

		if (getFeatures() & GF_CRUNCHED) {
			byte *buffer = new byte[size];
			if (in.read(buffer, size) != size)
				error("read_vga_from_datfile_2: read failed");
			dst = allocBlock (READ_BE_UINT32(buffer + size - 4) + extraBuffer);
			decrunchFile(buffer, dst, size);
			delete[] buffer;
		} else {
			dst = allocBlock(size + extraBuffer);
			if (in.read(dst, size) != size)
				error("read_vga_from_datfile_2: read failed");
		}
		in.close();

		return dst;
	} else {
		uint32 offs_a = _gameOffsetsPtr[id];
		uint32 size = _gameOffsetsPtr[id + 1] - offs_a;

		dst = allocBlock(size + extraBuffer);
		resfile_read(dst, offs_a, size);

		return dst;
	}
}

void SimonEngine::loadSound(uint sound, uint pan, uint vol, bool ambient) {
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

		byte *dst = (byte *)malloc(dstSize);
		decompressData(filename, dst, offset, srcSize, dstSize);
		_sound->playSoundData(dst, sound, pan, vol, ambient);
	} else {
		int offs = READ_LE_UINT32(_curSfxFile + sound * 4);
		_sound->playSoundData(_curSfxFile + offs, sound, pan, vol, ambient);
	}
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
