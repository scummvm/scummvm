/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2005 The ScummVM project
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
 * $Header$
 *
 */

// Resource file routines for Simon1/Simon2
#include "common/stdafx.h"
#include "common/file.h"
#include "simon/simon.h"
#include "simon/intern.h"

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

	loginPlayer();
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
		Child1 *child;

		size = CHILD1_SIZE;
		for (i = 0, j = fr2; i != 6; i++, j >>= 2)
			if (j & 3)
				size += sizeof(child->array[0]);

		child = (Child1 *)allocateChildBlock(item, 1, size);
		child->subroutine_id = fr1;
		child->fr2 = fr2;

		for (i = k = 0, j = fr2; i != 6; i++, j >>= 2)
			if (j & 3)
				child->array[k++] = (uint16)fileReadItemID(in);
	} else if (type == 2) {
		uint32 fr = in->readUint32BE();
		uint i, k, size;
		Child2 *child;

		size = CHILD2_SIZE;
		for (i = 0; i != 16; i++)
			if (fr & (1 << i))
				size += sizeof(child->array[0]);

		child = (Child2 *)allocateChildBlock(item, 2, size);
		child->avail_props = fr;

		k = 0;
		if (fr & 1) {
			child->array[k++] = (uint16)in->readUint32BE();
		}
		for (i = 1; i != 16; i++)
			if (fr & (1 << i))
				child->array[k++] = in->readUint16BE();

		child->string_id = (uint16)in->readUint32BE();
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

} // End of namespace Simon
