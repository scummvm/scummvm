/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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

// Resource file routines for Simon1/Simon2
#include "stdafx.h"
#include "simon.h"

// Script opcodes to load into memory
static const char * const opcode_arg_table_simon1win[256] = {
" ","I ","I ","I ","I ","I ","I ","II ","II ","II ","II ","B ","B ","BN ","BN ","BN ",
"BN ","BB ","BB ","BB ","BB ","II ","II ","N ","I ","I ","I ","IN ","IB ","II ","I ","I ",
"II ","II ","IBB ","BIB ","BB ","B ","BI ","IB ","B ","B ","BN ","BN ","BN ","BB ","BB ","BN ",
"BN ","BB ","BB ","BN ","BB ","BN ","B ","I ","IB ","IB ","II ","I ","I ","IN ","B ","T ",
"T ","NNNNNB ","BT ","BTS ","T "," ","B ","N ","IBN ","I ","I ","I ","NN "," "," ","IT ",
"II ","I ","B "," ","IB ","IBB ","IIB ","T "," "," ","IB ","IB ","IB ","B ","BB ","IBB ",
"NB ","N ","NBNNN ","N "," ","BNNNNNN ","B "," ","B ","B ","BB ","NNNNNIN ","N ","N ","N ","NNN ",
"NBNN ","IBNN ","IB ","IB ","IB ","IB ","N ","N ","N ","BI "," "," ","N ","I ","IBB ","NN ",
"N ","N ","Ban ","BB "," "," "," "," ","IB ","B "," ","II "," ","BI ","N ","I ",
"IB ","IB ","IB ","IB ","IB ","IB ","IB ","BI ","BB ","B ","B ","B ","B ","IBB ","IBN ","IB ",
"B ","BNBN ","BBTS ","N "," ","Ian ","B ","B ","B ","B ","T ","T ","B "," ","I "," ",
" ","BBI ","NNBB ","BBB "," "," "," "," ","N ","N "," "," ",
};

static const char * const opcode_arg_table_simon1dos[256] = {
" ","I ","I ","I ","I ","I ","I ","II ","II ","II ","II ","B ","B ","BN ","BN ","BN ",
"BN ","BB ","BB ","BB ","BB ","II ","II ","N ","I ","I ","I ","IN ","IB ","II ","I ","I ",
"II ","II ","IBB ","BIB ","BB ","B ","BI ","IB ","B ","B ","BN ","BN ","BN ","BB ","BB ","BN ",
"BN ","BB ","BB ","BN ","BB ","BN ","B ","I ","IB ","IB ","II ","I ","I ","IN ","B ","T ",
"T ","NNNNNB ","BT ","BT ","T "," ","B ","N ","IBN ","I ","I ","I ","NN "," "," ","IT ",
"II ","I ","B "," ","IB ","IBB ","IIB ","T "," "," ","IB ","IB ","IB ","B ","BB ","IBB ",
"NB ","N ","NBNNN ","N "," ","BNNNNNN ","B "," ","B ","B ","BB ","NNNNNIN ","N ","N ","N ","NNN ",
"NBNN ","IBNN ","IB ","IB ","IB ","IB ","N ","N ","N ","BI "," "," ","N ","I ","IBB ","NN ",
"N ","N ","Ban ","BB "," "," "," "," ","IB ","B "," ","II "," ","BI ","N ","I ",
"IB ","IB ","IB ","IB ","IB ","IB ","IB ","BI ","BB ","B ","B ","B ","B ","IBB ","IBN ","IB ",
"B ","BNBN ","BBT ","N "," ","Ian ","B ","B ","B ","B ","T ","T ","B "," ","I "," ",
" ","BBI ","NNBB ","BBB "," "," "," "," ","N ","N "," "," ",
};

static const char * const opcode_arg_table_simon2win[256] = {
" ","I ","I ","I ","I ","I ","I ","II ","II ","II ","II ","B ","B ","BN ","BN ","BN ",
"BN ","BB ","BB ","BB ","BB ","II ","II ","N ","I ","I ","I ","IN ","IB ","II ","I ","I ",
"II ","II ","IBB ","BIB ","BB ","B ","BI ","IB ","B ","B ","BN ","BN ","BN ","BB ","BB ","BN ",
"BN ","BB ","BB ","BN ","BB ","BN ","B ","I ","IB ","IB ","II ","I ","I ","IN ","B ","T ",
"T ","NNNNNB ","BT ","BTS ","T "," ","B ","N ","IBN ","I ","I ","I ","NN "," "," ","IT ",
"II ","I ","B "," ","IB ","IBB ","IIB ","T "," "," ","IB ","IB ","IB ","B ","BB ","IBB ",
"NB ","N ","NNBNNN ","NN "," ","BNNNNNN ","B "," ","B ","B ","BB ","NNNNNIN ","N ","N ","N ","NNN ",
"NBNN ","IBNN ","IB ","IB ","IB ","IB ","N ","N ","N ","BI "," "," ","N ","I ","IBB ","NNB ",
"N ","N ","Ban ","BB "," "," "," "," ","IB ","B "," ","II "," ","BI ","N ","I ",
"IB ","IB ","IB ","IB ","IB ","IB ","IB ","BI ","BB ","B ","B ","B ","B ","IBB ","IBN ","IB ",
"B ","BNBN ","BBTS ","N "," ","Ian ","B ","B ","B ","B ","T ","T ","B "," ","I "," ",
" ","BBI ","NNBB ","BBB "," "," "," "," ","N ","N "," "," ","BT "," ","B "};

static const char * const opcode_arg_table_simon2dos[256] = {
" ","I ","I ","I ","I ","I ","I ","II ","II ","II ","II ","B ","B ","BN ","BN ","BN ",
"BN ","BB ","BB ","BB ","BB ","II ","II ","N ","I ","I ","I ","IN ","IB ","II ","I ","I ",
"II ","II ","IBB ","BIB ","BB ","B ","BI ","IB ","B ","B ","BN ","BN ","BN ","BB ","BB ","BN ",
"BN ","BB ","BB ","BN ","BB ","BN ","B ","I ","IB ","IB ","II ","I ","I ","IN ","B ","T ",
"T ","NNNNNB ","BT ","BT ","T "," ","B ","N ","IBN ","I ","I ","I ","NN "," "," ","IT ",
"II ","I ","B "," ","IB ","IBB ","IIB ","T "," "," ","IB ","IB ","IB ","B ","BB ","IBB ",
"NB ","N ","NNBNNN ","NN "," ","BNNNNNN ","B "," ","B ","B ","BB ","NNNNNIN ","N ","N ","N ","NNN ",
"NBNN ","IBNN ","IB ","IB ","IB ","IB ","N ","N ","N ","BI "," "," ","N ","I ","IBB ","NNB ",
"N ","N ","Ban ","BB "," "," "," "," ","IB ","B "," ","II "," ","BI ","N ","I ",
"IB ","IB ","IB ","IB ","IB ","IB ","IB ","BI ","BB ","B ","B ","B ","B ","IBB ","IBN ","IB ",
"B ","BNBN ","BBT ","N "," ","Ian ","B ","B ","B ","B ","T ","T ","B "," ","I "," ",
" ","BBI ","NNBB ","BBB "," "," "," "," ","N ","N "," "," ","BT "," ","B "};

FILE *SimonState::fopen_maybe_lowercase(const char *filename) {
	FILE *in;
	char buf[256], dotbuf[256], *e;
	const char *s = _game_path;

	strcpy(buf, s); strcat(buf, filename);	
	strcpy(dotbuf, buf); strcat(dotbuf, ".");	// '.' appended version
												// for dumb vfat drivers 
	
	/* original filename */
	in = fopen(buf, "rb");
	if (in) return in;

	/* lowercase original filename */
	e = buf + strlen(s); do *e = tolower(*e); while(*e++);	
	in = fopen(buf, "rb");	
	if (in) return in;

	if (strchr(buf, '.'))
		return NULL;

	/* dot appended original filename */
	in = fopen(dotbuf, "rb");
	if (in) return in;

	/* lowercase dot appended */
	e = dotbuf + strlen(s); do *e = tolower(*e); while(*e++);
	in = fopen(dotbuf, "rb");

	return in;		
}

bool SimonState::loadGamePcFile(const char *filename) {
	FILE *in;
	int num_inited_objects;
	int i, file_size;

	/* read main gamepc file */
	in = fopen_maybe_lowercase(filename);
	if (in==NULL) return false;

	num_inited_objects = allocGamePcVars(in);

	allocItem1();
	loginPlayer();
	readGamePcText(in);

	for(i=2; i<num_inited_objects; i++) {
		readItemFromGamePc(in, _itemarray_ptr[i]);
	}

	readSubroutineBlock(in);

	fclose(in);

	/* Read list of TABLE resources */
	in = fopen_maybe_lowercase("TBLLIST");
	if (in==NULL) return false;

	fseek(in, 0, SEEK_END);
	file_size = ftell(in);

	_tbl_list = (byte*)malloc(file_size);
	if (_tbl_list == NULL)
		error("Out of memory for strip table list");
	fseek(in, 0, SEEK_SET);
	fread(_tbl_list, file_size, 1, in);
	fclose(in);

	/* Remember the current state */
	_subroutine_list_org = _subroutine_list;
	_tablesheap_ptr_org = _tablesheap_ptr;
	_tablesheap_curpos_org = _tablesheap_curpos;
	
	/* Read list of TEXT resources */
	in = fopen_maybe_lowercase("STRIPPED.TXT");
	if (in==NULL) return false;

	fseek(in, 0, SEEK_END);
	file_size = ftell(in);
	_stripped_txt_mem = (byte*)malloc(file_size);
	if (_stripped_txt_mem == NULL)
		error("Out of memory for strip text list");
	fseek(in, 0, SEEK_SET);
	fread(_stripped_txt_mem, file_size, 1, in);
	fclose(in);

	return true;
}

void SimonState::readGamePcText(FILE *in) {
	uint text_size;
	byte *text_mem;

	_text_size = text_size = fileReadBE32(in);
	text_mem = (byte*)malloc(text_size);
	if (text_mem == NULL)
		error("Out of text memory");

	fread(text_mem, text_size, 1, in);

	setupStringTable(text_mem, _stringtab_num);
}

void SimonState::readItemFromGamePc(FILE *in, Item *item) {
	uint32 type;

	item->unk2 = fileReadBE16(in);
	item->unk1 = fileReadBE16(in);
	item->unk3 = fileReadBE16(in);
	item->sibling = (uint16)fileReadItemID(in);
	item->child = (uint16)fileReadItemID(in);
	item->parent = (uint16)fileReadItemID(in);
	fileReadBE16(in);
	item->unk4 = fileReadBE16(in);
	item->children = NULL;

	type  = fileReadBE32(in);
	while (type) {
		type = fileReadBE16(in);
		if (type != 0)
			readItemChildren(in, item, type);
	}
}

void SimonState::readItemChildren(FILE *in, Item *item, uint type) {
	if (type == 1) {
		uint fr1 = fileReadBE16(in);
		uint fr2 = fileReadBE16(in);
		uint i, size;
		uint j, k;
		Child1 *child;

		size = CHILD1_SIZE;
		for(i=0,j=fr2; i!=6; i++, j>>=2)
			if (j&3)
				size += sizeof(child->array[0]);

		child = (Child1*)allocateChildBlock(item, 1, size);
		child->subroutine_id = fr1;
		child->fr2 = fr2;

		for(i=k=0,j=fr2; i!=6; i++, j>>=2)
			if (j&3)
				child->array[k++] = (uint16)fileReadItemID(in);
	} else if (type == 2) {
		uint32 fr = fileReadBE32(in);
		uint i,k,size;
		Child2 *child;

		size = CHILD2_SIZE;
		for(i=0; i!=16; i++)
			if (fr & (1<<i))
				size += sizeof(child->array[0]);

		child = (Child2*)allocateChildBlock(item, 2, size);
		child->avail_props = fr;

		k = 0;
		if (fr & 1) {
			child->array[k++] = (uint16)fileReadBE32(in);
		}
		for(i=1; i!=16; i++)
			if(fr & (1<<i))
				child->array[k++] = fileReadBE16(in);

		child->string_id = (uint16)fileReadBE32(in);
	} else {
		error("readItemChildren: invalid mode");
	}
}

uint fileReadItemID(FILE *in) {	
	uint32 val = fileReadBE32(in);
	if (val==0xFFFFFFFF)
		return 0;
	return val + 2;
}

byte *SimonState::readSingleOpcode(FILE *in, byte *ptr) {
	int i,l;
	const char *string_ptr;
	uint val;

	const char * const *table;

	switch(_game) {
	case GAME_SIMON1WIN: table = opcode_arg_table_simon1win; break;
	case GAME_SIMON2DOS: table = opcode_arg_table_simon2win; printf("right opcode table\n"); break;
	case GAME_SIMON2WIN: table = opcode_arg_table_simon2win; break;
	case GAME_SIMON1DOS: table = opcode_arg_table_simon1dos; break;
	default:
		error("Invalid game specified");
	}

	i = 0;

	string_ptr = table[*ptr++];
	if (!string_ptr)
		error("Unable to locate opcode table. Perhaps you are using the wrong game target?");

	for(;;) {
		if (string_ptr[i] == ' ')
			return ptr;

		l = string_ptr[i++];
		switch(l) {
		case 'N':
		case 'S':
		case 'a':
		case 'n':
		case 'p':
		case 'v':
			val = fileReadBE16(in);
			*ptr++ = val >> 8;
			*ptr++ = val & 255;
			break;
	
		case 'B':
			*ptr++ = fileReadByte(in);
			if (ptr[-1] == 0xFF) {
				*ptr++ = fileReadByte(in);
			}
			break;

		case 'I':
			val = fileReadBE16(in);
			switch(val) {
			case 1: val = 0xFFFF; break;
			case 3: val = 0xFFFD; break;
			case 5: val = 0xFFFB; break;
			case 7: val = 0xFFF9; break;
			case 9: val = 0xFFF7; break;
			default:
				val = fileReadItemID(in);;
			}
			*ptr++ = val >> 8;
			*ptr++ = val & 255;
			break;

		case 'T':
			val = fileReadBE16(in);
			switch(val) {
			case 0:
				val = 0xFFFF;
				break;
			case 3:
				val = 0xFFFD;
				break;
			default:
				val = (uint16)fileReadBE32(in);
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
