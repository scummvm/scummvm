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


#include "stdafx.h"
#include "scummsys.h"
#include "system.h"
#include "mixer.h"
#include "simon.h"


#include <errno.h>
#include <time.h>
#ifdef WIN32
#include <malloc.h>
#endif
#include <sys/stat.h>

int sdl_mouse_x, sdl_mouse_y;

byte *sdl_buf_3;
byte *sdl_buf;
byte *sdl_buf_attached;

SimonState *g_simon;


static const GameSpecificSettings simon1_settings = {
	1, /* VGA_DELAY_BASE */
	1576/4, /* TABLE_INDEX_BASE */
	1460/4, /* TEXT_INDEX_BASE */
	1700/4, /* NUM_GAME_OFFSETS */
	64, /* NUM_VIDEO_OP_CODES */
	1000000, /* VGA_MEM_SIZE */
	50000, /* TABLES_MEM_SIZE */
	3624, /* NUM_VOICE_RESOURCES */
	1316/4, /* MUSIC_INDEX_BASE */
	0,		/* SOUND_INDEX_BASE */
	"SIMON.GME",	/* gme_filename */
	"SIMON.WAV",	/* wav_filename */
	"GAMEPC",			/* gamepc_filename */
};

static const GameSpecificSettings simon2_settings = {
	5, /* VGA_DELAY_BASE */
	1580/4, /* TABLE_INDEX_BASE */
	1500/4, /* TEXT_INDEX_BASE */
	2116/4, /* NUM_GAME_OFFSETS */
	75, /* NUM_VIDEO_OP_CODES */
	2000000, /* VGA_MEM_SIZE */
	100000, /* TABLES_MEM_SIZE */
	12256, /* NUM_VOICE_RESOURCES */
	1128/4, /* MUSIC_INDEX_BASE */
	1660/4,			/* SOUND_INDEX_BASE */
	"SIMON2.GME", /* gme_filename */
	"SIMON2.WAV",	/* wav_filename */
	"GSPTR30",		/* gamepc_filename */
};

//#ifdef USE_2xSAI
//#define NUM_PALETTE_FADEOUT 32
//#else
#define NUM_PALETTE_FADEOUT 32
//#endif

void palette_fadeout(uint32 *pal_values,uint num) {
	byte *p = (byte*)pal_values;

//#ifdef USE_2xSAI
	do {
		if (p[0]>=8) p[0] -= 8; else p[0] = 0;
		if (p[1]>=8) p[1] -= 8; else p[1] = 0;
		if (p[2]>=8) p[2] -= 8; else p[2] = 0;
		p += sizeof(uint32);
	} while (--num);
//#else
//	do {
//		if (p[0]) p[0] -= 4;
//		if (p[1]) p[1] -= 4;
//		if (p[2]) p[2] -= 4;
//		p += sizeof(uint32);
//	} while (--num);

//#endif

}


uint fileReadItemID(FILE *in) {	
	uint32 val = fileReadBE32(in);
	if (val==0xFFFFFFFF)
		return 0;
	return val + 2;
}


/* debug code */
void SimonState::show_it(void *buf) {
	_system->copy_rect((byte*)buf, 320, 0, 0, 320, 200);
	_system->update_screen();
}

FILE *SimonState::fopen_maybe_lowercase(const char *filename) {
	char buf[256], *e;

	const char *s = _game_path;

	strcpy(buf, s);
	e = strchr(buf, 0);
	strcpy(e, filename);

#if defined(WIN32) || defined(__MORPHOS__)
	/* win32 is not case sensitive */
	return fopen(buf, "rb");
#else
	/* first try with the original filename */
	FILE *in = fopen(buf, "rb");

	if (in) return in;
	/* if that fails, convert the filename into lowercase and retry */

	do *e = tolower(*e); while(*e++);

	return fopen(buf, "rb");
#endif
}


byte *SimonState::allocateItem(uint size) {
	byte *org = _itemheap_ptr;
	size = (size + 3) & ~3;

	_itemheap_ptr += size;
	_itemheap_curpos += size;

	if (_itemheap_curpos > _itemheap_size)
		error("Itemheap overflow");

	return org;
}

void SimonState::alignTableMem() {
	if ((uint32)_tablesheap_ptr & 3) {
		_tablesheap_ptr += 2;
		_tablesheap_curpos += 2;
	}
}

byte *SimonState::allocateTable(uint size) {
	byte *org = _tablesheap_ptr;
	
	size = (size + 1) & ~1;

	_tablesheap_ptr += size;
	_tablesheap_curpos += size;

	if (_tablesheap_curpos > _tablesheap_size)
		error("Tablesheap overflow");

	return org;
}

int SimonState::allocGamePcVars(FILE *in) {
	uint item_array_size, item_array_inited, stringtable_num;
	uint32 version;
	uint i;
	
	item_array_size = fileReadBE32(in);
	version = fileReadBE32(in);
	item_array_inited = fileReadBE32(in);
	stringtable_num = fileReadBE32(in);

	item_array_inited += 2; /* first two items are predefined */
	item_array_size += 2;

	if (version != 0x80)
		error("Not a runtime database");

	_itemarray_ptr = (Item**)calloc(item_array_size, sizeof(Item*));
	if (_itemarray_ptr == NULL)
		error("Out of memory for Item array");

	_itemarray_size = item_array_size;
	_itemarray_inited = item_array_inited;

	for(i=2; i!=item_array_inited; i++) {
		_itemarray_ptr[i] = (Item*)allocateItem(sizeof(Item));
	}

	/* The rest is cleared automatically by calloc */
	allocateStringTable(stringtable_num + 10);
	_stringtab_num = stringtable_num;

	return item_array_inited;	
}


Item *SimonState::allocItem1() {
	Item *item = (Item*)allocateItem(sizeof(Item));
	_itemarray_ptr[1] = item;
	return item;
}

void SimonState::loginPlayerHelper(Item *item, int a, int b) {
	Child9 *child;

	child = (Child9*)findChildOfType(item, 9);
	if (child == NULL) {
		child = (Child9*)allocateChildBlock(item, 9, sizeof(Child9));
	}

	if (a>=0 && a<=3)
		child->array[a] = b;
}


void SimonState::loginPlayer() {
	Item *item;
	Child *child;

	item = _itemarray_ptr[1];
	item->unk2 = -1;
	item->unk1 = 10000;
	_item_1 = item;

	child = (Child*)allocateChildBlock(item, 3, sizeof(Child));
	if (child == NULL)
		error("player create failure");

	loginPlayerHelper(item, 0, 0);
}

void SimonState::allocateStringTable(int num) {
	_stringtab_ptr = (byte**)calloc(num, sizeof(byte*));
	_stringtab_pos = 0;
	_stringtab_numalloc = num;
}

void SimonState::setupStringTable(byte *mem, int num) {
	int i = 0;
	for(;;) {
		_stringtab_ptr[i++] = mem;
		if (--num == 0)
			break;
		for(;*mem;mem++);
		mem++;
	}

	_stringtab_pos = i;
}

void SimonState::setupLocalStringTable(byte *mem, int num) {
	int i = 0;
	for(;;) {
		_local_stringtable[i++] = mem;
		if (--num == 0)
			break;
		for(;*mem;mem++);
		mem++;
	}
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

void SimonState::readItemChildren(FILE *in, Item *item, uint tmp) {
	if (tmp == 1) {
		uint fr1 = fileReadBE16(in);
		uint fr2 = fileReadBE16(in);
		uint i, size;
		uint j, k;
		Child1 *child;

		size = sizeof(Child1);
		for(i=0,j=fr2; i!=6; i++, j>>=2)
			if (j&3)
				size += sizeof(child->array[0]);

		child = (Child1*)allocateChildBlock(item, 1, size);
		child->subroutine_id = fr1;
		child->fr2 = fr2;

		for(i=k=0,j=fr2; i!=6; i++, j>>=2)
			if (j&3)
				child->array[k++] = (uint16)fileReadItemID(in);
	} else if (tmp == 2) {
		uint32 fr = fileReadBE32(in);
		uint i,k,size;
		Child2 *child;

		size = sizeof(Child2);
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


void SimonState::readItemFromGamePc(FILE *in, Item *item) {
	uint32 tmp;

	item->unk2 = fileReadBE16(in);
	item->unk1 = fileReadBE16(in);
	item->unk3 = fileReadBE16(in);
	item->sibling = (uint16)fileReadItemID(in);
	item->child = (uint16)fileReadItemID(in);
	item->parent = (uint16)fileReadItemID(in);
	fileReadBE16(in);
	item->unk4 = fileReadBE16(in);
	item->children = NULL;

	tmp = fileReadBE32(in);
	while (tmp) {
		tmp = fileReadBE16(in);
		if (tmp != 0)
			readItemChildren(in, item, tmp);
	}
}


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

/* read_single_opcode */
byte *SimonState::readSingleOpcode(FILE *in, byte *ptr) {
	int i,l;
	const char *string_ptr;
	uint val;

	const char * const *table;

	switch(_game) {
	case GAME_SIMON1WIN: table = opcode_arg_table_simon1win; break;
	case GAME_SIMON2WIN: table = opcode_arg_table_simon2win; break;
	case GAME_SIMON1DOS: table = opcode_arg_table_simon1dos; break;
	default:
		error("Invalid game specified");
	}

	i = 0;

	string_ptr = table[*ptr++];
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

void SimonState::readSubroutineLine(FILE *in, SubroutineLine *sl, Subroutine *sub) {
	byte line_buffer[1024], *q = line_buffer;
	int size;
	
	if (sub->id == 0) {
		sl->cond_a = fileReadBE16(in);
		sl->cond_b = fileReadBE16(in);
		sl->cond_c = fileReadBE16(in);
	}

	while ( (*q = fileReadByte(in)) != 0xFF) {
		if (*q == 87) {
			fileReadBE16(in);
		} else {
			q = readSingleOpcode(in, q);
		}
	}

	size = q - line_buffer + 1;

	memcpy(allocateTable(size), line_buffer, size);
}

SubroutineLine *SimonState::createSubroutineLine(Subroutine *sub, int where) {
	SubroutineLine *sl, *cur_sl = NULL, *last_sl = NULL;

	if (sub->id == 0)
		sl = (SubroutineLine*)allocateTable(SUBROUTINE_LINE_BIG_SIZE);
	else
		sl = (SubroutineLine*)allocateTable(SUBROUTINE_LINE_SMALL_SIZE);

	/* where is what offset to insert the line at, locate the proper beginning line */
	if (sub->first != 0) {
		cur_sl = (SubroutineLine*) ((byte*)sub + sub->first);
		while (where) {
			last_sl = cur_sl;
			cur_sl = (SubroutineLine*) ((byte*)sub + cur_sl->next);
			if ((byte*)cur_sl == (byte*)sub)
				break;
			where--;
		}
	}

	if (last_sl != NULL) {
		/* Insert the subroutine line in the middle of the link */
		last_sl->next = (byte*)sl - (byte*)sub;
		sl->next = (byte*)cur_sl - (byte*)sub;
	} else {
		/* Insert the subroutine line at the head of the link */
		sl->next = sub->first;
		sub->first = (byte*)sl - (byte*)sub;
	}

	return sl;
}

void SimonState::readSubroutine(FILE *in, Subroutine *sub) {
	while (fileReadBE16(in) == 0) {
		readSubroutineLine(in,createSubroutineLine(sub, 0xFFFF),sub);
	}
}

Subroutine *SimonState::createSubroutine(uint id) {
	Subroutine *sub;
	
	alignTableMem();

	sub = (Subroutine*)allocateTable(sizeof(Subroutine));
	sub->id = id;
	sub->first = 0;
	sub->next = _subroutine_list;
	_subroutine_list = sub;
	return sub;
}

void SimonState::readSubroutineBlock(FILE *in) {
	while (fileReadBE16(in) == 0) {
		readSubroutine(in,createSubroutine(fileReadBE16(in)));
	}
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
	rewind(in);
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
	rewind(in);
	fread(_stripped_txt_mem, file_size, 1, in);
	fclose(in);

	return true;
}

Child *SimonState::findChildOfType(Item *i, uint type) {
	Child *child = i->children;
	for(;child; child = child->next)
		if (child->type == type)
			return child;
	return NULL;
}

bool SimonState::hasChildOfType1(Item *item) {
	return findChildOfType1(item) != NULL;
}

bool SimonState::hasChildOfType2(Item *item) {
	return findChildOfType2(item) != NULL;
}

Child1 *SimonState::findChildOfType1(Item *item) {
	return (Child1*)findChildOfType(item, 1);
}

Child2 *SimonState::findChildOfType2(Item *item) {
	return (Child2*)findChildOfType(item, 2);
}

Child3 *SimonState::findChildOfType3(Item *item) {
	return (Child3*)findChildOfType(item, 3);
}

uint SimonState::getOffsetOfChild2Param(Child2 *child, uint prop) {
	uint m = 1;
	uint offset = 0;
	while (m != prop) {
		if (child->avail_props & m)
			offset++;
		m<<=1;
	}
	return offset;
}


Child *SimonState::allocateChildBlock(Item *i, uint type, uint size) {
	Child *child = (Child*)allocateItem(size);
	child->next = i->children;
	i->children = child;
	child->type = type;
	return child;
}

void SimonState::allocItemHeap() {
	_itemheap_size = 10000;
	_itemheap_curpos = 0;
	_itemheap_ptr = (byte*)calloc(10000, 1);
}

void SimonState::allocTablesHeap() {
	_tablesheap_size = gss->TABLES_MEM_SIZE;
	_tablesheap_curpos = 0;
	_tablesheap_ptr = (byte*)calloc(gss->TABLES_MEM_SIZE, 1);
}

void SimonState::setItemUnk3(Item *item, int value) {
	item->unk3 = value;
}

int SimonState::startSubroutine(Subroutine *sub) {
	int result = -1;
	SubroutineLine *sl;
	byte *old_code_ptr;

//	warning("startSubroutine(%d)", sub->id);
#ifdef DUMP_START_MAINSCRIPT
	dumpSubroutine(sub);
#endif

	old_code_ptr = _code_ptr;

	if (++_recursion_depth > 40)
		error("Recursion error");

	sl = (SubroutineLine*)((byte*)sub + sub->first);

	while ((byte*)sl != (byte*)sub) {
		if (checkIfToRunSubroutineLine(sl, sub)) {
			result = 0;
			_code_ptr = (byte*)sl;
			if (sub->id) _code_ptr += 2; else _code_ptr += 8;

#ifdef DUMP_CONTINOUS_MAINSCRIPT
			fprintf(_dump_file,"; %d\n", sub->id);
#endif
			result = runScript();
			if (result != 0) {
				/* result -10 means restart subroutine */
				if (result == -10) {
					delay(0); /* maybe leave control to the VGA */
					sl = (SubroutineLine*)((byte*)sub + sub->first);
					continue;
				}
				break;
			}
		}
		sl = (SubroutineLine*)((byte*)sub + sl->next);
	}

	_code_ptr = old_code_ptr;

	_recursion_depth--;
	return result;
}

int SimonState::startSubroutineEx(Subroutine *sub) {
	_item_1_ptr = _item_1;
	return startSubroutine(sub);
}

bool SimonState::checkIfToRunSubroutineLine(SubroutineLine *sl, Subroutine *sub) {
	if (sub->id)
		return true;

	if (sl->cond_a != -1 && sl->cond_a != _script_cond_a &&
		  (sl->cond_a != -2 || _script_cond_a != -1))
		return false;
	
	if (sl->cond_b != -1 && sl->cond_b != _script_cond_b &&
		  (sl->cond_b != -2 || _script_cond_b != -1))
		return false;

	if (sl->cond_c != -1 && sl->cond_c != _script_cond_c &&
		  (sl->cond_c != -2 || _script_cond_c != -1))
		return false;

	return true;
}

int SimonState::runScript() {
	byte opcode;
	bool flag, condition;

	do {
#ifdef DUMP_CONTINOUS_MAINSCRIPT
	dumpOpcode(_code_ptr);
#endif	
		
	opcode = getByte();
	if (opcode==0xFF)
		return 0;

	if (_run_script_return_1)
		return 1;

	/* Invert condition? */
	flag = false;
	if (opcode==0) {
		flag = true;
		opcode = getByte();
		if (opcode==0xFF)
			return 0;
	}

	condition = true;
	
	switch(opcode) {
	case 1: { /* ptrA parent is */
		condition = (getItem1Ptr()->parent == getNextItemID());
	} break;

	case 2: { /* ptrA parent is not */
		condition = (getItem1Ptr()->parent != getNextItemID());
	} break;

	case 5: {	/* parent is 1 */
		condition = (getNextItemPtr()->parent == getItem1ID());
	} break;

	case 6: { /* parent isnot 1 */
		condition = (getNextItemPtr()->parent != getItem1ID());
	} break;

	case 7: { /* parent is */
		Item *item = getNextItemPtr();
		condition = (item->parent == getNextItemID());
	} break;

	case 11: { /* is zero */
		condition = (getNextVarContents() == 0);
	} break;

	case 12: { /* isnot zero */
		condition = (getNextVarContents() != 0);
	} break;

	case 13: { /* equal */
		uint tmp = getNextVarContents();
		condition = (tmp == getVarOrWord());
	} break;

	case 14: { /* not equal */
		uint tmp = getNextVarContents();
		condition = (tmp != getVarOrWord());
	} break;

	case 15: { /* is greater */
		uint tmp = getNextVarContents();
		condition = (tmp > getVarOrWord());
	} break;

	case 16: { /* is less */
		uint tmp = getNextVarContents();
		condition = (tmp < getVarOrWord());
	} break;

	case 17: { /* is eq f */
		uint tmp = getNextVarContents();
		condition = (tmp == getNextVarContents());
	} break;

	case 18: { /* is not equal f */
		uint tmp = getNextVarContents();
		condition = (tmp != getNextVarContents());
	} break;

	case 19: { /* is greater f */
		uint tmp = getNextVarContents();
		condition = (tmp < getNextVarContents());
	} break;

	case 20: { /* is less f */
		uint tmp = getNextVarContents();
		condition = (tmp > getNextVarContents());
	} break;

	case 23: {
		condition = o_unk_23(getVarOrWord());
	} break;

	case 25: { /* has child of type 1 */
		condition = hasChildOfType1(getNextItemPtr());
	} break;
		
	case 26: { /* has child of type 2 */
		condition = hasChildOfType2(getNextItemPtr());
	} break;

	case 27: { /* item unk3 is */
		Item *item = getNextItemPtr();
		condition = ((uint)item->unk3 == getVarOrWord());
	} break;

	case 28: { /* item has prop */
		Child2 *child = findChildOfType2(getNextItemPtr());
		byte num = getVarOrByte();
		condition = child!=NULL && (child->avail_props & (1<<num)) != 0;
	} break;

	case 31: { /* set no parent */
		setItemParent(getNextItemPtr(), NULL);
	} break;

	case 33: { /* set item parent */
		Item *item = getNextItemPtr();
		setItemParent(item, getNextItemPtr());
	} break;

	case 36: { /* copy var */
		uint value = getNextVarContents();
		writeNextVarContents(value);
	} break;

	case 41: { /* zero var */
		writeNextVarContents(0);
	} break;

	case 42: { /* set var */
		uint var = getVarOrByte();
		writeVariable(var, getVarOrWord());
	} break;

	case 43: { /* add */
		uint var = getVarOrByte();
		writeVariable(var, readVariable(var) + getVarOrWord());
	} break;

	case 44: { /* sub */
		uint var = getVarOrByte();
		writeVariable(var, readVariable(var) - getVarOrWord());
	} break;

	case 45: { /* add f */
		uint var = getVarOrByte();
		writeVariable(var, readVariable(var) + getNextVarContents());
	} break;

	case 46: { /* sub f */
		uint var = getVarOrByte();
		writeVariable(var, readVariable(var) - getNextVarContents());
	} break;
			
	case 47: { /* mul */
		uint var = getVarOrByte();
		writeVariable(var, readVariable(var) * getVarOrWord());
	} break;

	case 48: { /* div */
		uint var = getVarOrByte();
		int value = getVarOrWord();
		if (value == 0)
			error("Division by zero in div");
		writeVariable(var, readVariable(var) / value);
	} break;

	case 49: { /* mul f */
		uint var = getVarOrByte();
		writeVariable(var, readVariable(var) * getNextVarContents());
	} break;
		
	case 50: { /* div f */
		uint var = getVarOrByte();
		int value = getNextVarContents();
		if (value == 0)
			error("Division by zero in div f");
		writeVariable(var, readVariable(var) / value);
	} break;

	case 51: { /* mod */
		uint var = getVarOrByte();
		int value = getVarOrWord();
		if (value == 0)
			error("Division by zero in mod");
		writeVariable(var, readVariable(var) % value);
	} break;

	case 52: { /* mod f */
		uint var = getVarOrByte();
		int value = getNextVarContents();
		if (value == 0)
			error("Division by zero in mod f");
		writeVariable(var, readVariable(var) % value);
	} break;

	case 53: { /* random */
		uint var = getVarOrByte();
		uint value = (uint16)getVarOrWord();
		uint rand_value;
		
		for(;;) {
			uint value_2 = value;
			rand_value = rand() & 0x7FFF;

			if (value == 0)
				error("Invalid random range");
			
			value = 0x8000 / value;
			
			if (value == 0)
				error("Invalid random range");

			if (rand_value / value != value_2)
				break;
				
			value = value_2;
		}

		writeVariable(var, rand_value / value);
	} break;

	case 55: { /* set itemA parent */
		setItemParent(getItem1Ptr(), getNextItemPtr());
	} break;

	case 56: { /* set child2 fr bit */
		Child2 *child = findChildOfType2(getNextItemPtr());
		int value = getVarOrByte();
		if (child != NULL && value >= 0x10)
			child->avail_props |= 1<<value;
	} break;

	case 57: { /* clear child2 fr bit */
		Child2 *child = findChildOfType2(getNextItemPtr());
		int value = getVarOrByte();
		if (child != NULL && value >= 0x10)
			child->avail_props &= ~(1<<value);
	} break;

	case 58: { /* make siblings */
		Item *item = getNextItemPtr();
		setItemParent(item, derefItem(getNextItemPtr()->parent));
	} break;

	case 59: { /* item inc unk3 */
		Item *item = getNextItemPtr();
		if (item->unk3<=30000)
			setItemUnk3(item, item->unk3 + 1);
	} break;

	case 60: { /* item dec unk3 */
		Item *item = getNextItemPtr();
		if (item->unk3>=0)
			setItemUnk3(item, item->unk3 - 1);
	} break;

	case 61: { /* item set unk3 */
		Item *item = getNextItemPtr();
		int value = getVarOrWord();
		if (value<0) value = 0;
		if (value>30000) value = 30000;
		setItemUnk3(item, value);
	} break;
	
	case 62: { /* show int */
		showMessageFormat("%d", getNextVarContents());
	} break;

	case 63: { /* show string nl */
		showMessageFormat("%s\n", getStringPtrByID(getNextStringID()));
	} break;	

	case 64: { /* show string */
		showMessageFormat("%s", getStringPtrByID(getNextStringID()));
	} break;

	case 65: { /* add hit area */
		int id = getVarOrWord();
		int x = getVarOrWord();
		int y = getVarOrWord();
		int w = getVarOrWord();
		int h = getVarOrWord();
		int number = getVarOrByte();
		if (number < 20)
			addNewHitArea(id, x, y, w, h, (number<<8) + 129, 0xD0, &_dummy_item_2);
	} break;
	
	case 66: { /* set array 2 */
		uint var = getVarOrByte();
		uint string_id = getNextStringID();
		if (var < 20)
			_stringid_array_2[var] = string_id;
	} break;

	case 67: { /* set array 3 and 4 */
		if (_game == GAME_SIMON1WIN || _game&GAME_SIMON2) {
			uint var = getVarOrByte();
			uint string_id = getNextStringID();
			uint value = getNextWord();
			if (var < 20) {
				_stringid_array_3[var] = string_id;
				_array_4[var] = value;
			}
		} else {
			uint var = getVarOrByte();
			uint string_id = getNextStringID();
			if (var < 20) {
				_stringid_array_3[var] = string_id;
			}
		}
	} break;

	case 68: { /* exit interpreter */
		error("Exit interpreter opcode");
	} break;

	case 69: { /* return 1 */
		return 1;
	}

	case 70: { /* show string from array */
		const char *str = (const char*)getStringPtrByID(_stringid_array_3[getVarOrByte()]);

		if (_game & GAME_SIMON2) {
			writeVariable(51, strlen(str)/53 * 8 + 8);
		}

		showMessageFormat("%s\n", str);
	} break;

	case 71: { /* start subroutine */
		Subroutine *sub = getSubroutineByID(getVarOrWord());
		if (sub != NULL)
			startSubroutine(sub);
	} break;

	case 76: { /* add event */
		uint timeout = getVarOrWord();
		addTimeEvent(timeout, getVarOrWord());
	} break;

	case 77: { /* has item minus 1 */
		condition = _subject_item != NULL;
	} break;
			
	case 78: { /* has item minus 3 */
		condition = _object_item != NULL;
	} break;

	case 79: { /* childstruct fr2 is */
		Child2 *child = findChildOfType2(getNextItemPtr());
		uint string_id = getNextStringID();
		condition = (child != NULL) && child->string_id == string_id;
	} break;

	case 80: { /* item equal */
		condition = getNextItemPtr() == getNextItemPtr();
	} break;

	case 82: { /* dummy opcode? */
		getVarOrByte();
	} break;

	case 83: { /* restart subroutine */
		return -10;
	}

	case 87: { /* dummy opcode? */
		getNextStringID();
	} break;

	case 88: { /* or_lock_word */
		_lock_word |= 0x10;
	} break;

	case 89: { /* and lock word */
		_lock_word &= ~0x10;
	} break;

	case 90: { /* set minusitem to parent */
		Item *item = derefItem(getNextItemPtr()->parent);
		switch(getVarOrByte()) {
		case 0:
			_object_item = item;
			break;
		case 1:
			_subject_item = item;
			break;
		default:
			error("set minusitem to parent, invalid subcode");
		}
	} break;

	case 91: { /* set minusitem to sibling */
		Item *item = derefItem(getNextItemPtr()->sibling);
		switch(getVarOrByte()) {
		case 0:
			_object_item = item;
			break;
		case 1:
			_subject_item = item;
			break;
		default:
			error("set minusitem to sibling, invalid subcode");
		}
	} break;
		
	case 92: { /* set minusitem to child */
		Item *item = derefItem(getNextItemPtr()->child);
		switch(getVarOrByte()) {
		case 0:
			_object_item = item;
			break;
		case 1:
			_subject_item = item;
			break;
		default:
			error("set minusitem to child, invalid subcode");
		}
	} break;

	case 96: {
		uint val = getVarOrWord();
		o_set_video_mode(getVarOrByte(), val);
	} break;

	case 97: { /* load vga */
		ensureVgaResLoadedC(getVarOrWord());
	} break;

	case 98: {
		if (!(_game & GAME_SIMON2)) {
			uint a = getVarOrWord();
			uint b = getVarOrByte();
			uint c = getVarOrWord();
			uint d = getVarOrWord();
			uint f = getVarOrWord();
			start_vga_code(b, a/100, a, c, d, f);
		} else {
			uint a = getVarOrWord();
			uint b = getVarOrWord();
			uint c = getVarOrByte();
			uint d = getVarOrWord();
			uint e = getVarOrWord();
			uint f = getVarOrWord();
			start_vga_code(c,a,b,d,e,f);
		}
	} break;

	case 99: {
		if (!(_game & GAME_SIMON2)) {
			o_unk_99_simon1(getVarOrWord());
		} else {
			uint a = getVarOrWord();
			uint b = getVarOrWord();
			o_unk_99_simon2(a,b);
		}
	} break;

	case 100: {
		o_vga_reset();
	} break;

	case 101: {
		uint a = getVarOrByte();
		uint b = getVarOrWord();
		uint c = getVarOrWord();
		uint d = getVarOrWord();
		uint e = getVarOrWord();
		uint f = getVarOrWord();
		uint g = getVarOrWord();
		o_unk26_helper(a, b, c, d, e, f, g, 0);
	} break;

	case 102: {
		fcs_unk_2(getVarOrByte() & 7);
	} break;

	case 103: {
		o_unk_103();
	} break;

	case 104: {
		fcs_delete(getVarOrByte() & 7);
	} break;
	
	case 107: { /* ADD_ITEM_HITAREA(id,x,y,w,h,item,unk3) */
		uint flags = 0;
		uint id = getVarOrWord();
		uint params = id / 1000;
		uint x,y,w,h,unk3;
		Item *item;
		
		id = id % 1000;

		if (params & 1) flags |= 8;
		if (params & 2) flags |= 4;
		if (params & 4) flags |= 0x80;
		if (params & 8) flags |= 1;
		if (params & 16) flags |= 0x10;

		x = getVarOrWord();
		y = getVarOrWord();
		w = getVarOrWord();
		h = getVarOrWord();
		item = getNextItemPtrStrange();
		unk3 = getVarOrWord();
		if (x >= 1000) {
			unk3 += 0x4000;
			x -= 1000;
		}
		addNewHitArea(id, x, y, w, h, flags, unk3, item);
	} break;

	case 108: { /* delete hitarea */
		delete_hitarea(getVarOrWord());
	} break;

	case 109: { /* clear hitarea bit 0x40 */
		clear_hitarea_bit_0x40(getVarOrWord());
	} break;

	case 110: { /* set hitarea bit 0x40 */
		set_hitarea_bit_0x40(getVarOrWord());
	} break;

	case 111: { /* set hitarea xy */
		uint hitarea_id = getVarOrWord();
		uint x = getVarOrWord();
		uint y = getVarOrWord();
		set_hitarea_x_y(hitarea_id, x, y);
	} break;

	case 114: {
		Item *item = getNextItemPtr();
		uint fcs_index = getVarOrByte();
		lock();
		fcs_unk_proc_1(fcs_index, item, 0, 0);
		unlock();
	} break;

	case 115: { /* item has flag */
		Item *item = getNextItemPtr();
		condition = (item->unk4 & (1 << getVarOrByte())) != 0;
	} break;

	case 116: { /* item set flag */
		Item *item = getNextItemPtr();
		item->unk4 |= (1 << getVarOrByte());
	} break;
	
	case 117: { /* item clear flag */
		Item *item = getNextItemPtr();
		item->unk4 &= ~(1 << getVarOrByte());
	} break;

	case 119: { /* WAIT_VGA */
		uint var = getVarOrWord();
		_scriptvar_2 = (var==200);
		
		if (var!=200 || !_skip_vga_wait)
			o_wait_for_vga(var);
		_skip_vga_wait = false;
	} break;

	case 120: {
		o_unk_120(getVarOrWord());
	} break;

	case 121: { /* SET_VGA_ITEM */
		uint slot = getVarOrByte();
		_vc_item_array[slot] = getNextItemPtr();
	} break;
	
	case 125: { /* item is sibling with item 1 */
		Item *item = getNextItemPtr();
		condition = (getItem1Ptr()->parent == item->parent);
	} break;

	case 126: {
		Item *item = getNextItemPtr();
		uint fcs_index = getVarOrByte();
		uint a = 1<<getVarOrByte();
		lock();
		fcs_unk_proc_1(fcs_index, item, 1, a);
		unlock();
	} break;

	case 127: { /* deals with music */
		o_unk_127();
	} break;

	case 128: { /* dummy instruction? */
		getVarOrWord();
	} break;

	case 129: { /* dummy instruction? */
		getVarOrWord();
		condition = true;
	} break;

	case 130: { /* set script cond */
		uint a = getVarOrByte();
		if (a == 1) {
			getNextWord();
			_script_cond_b = getNextWord();
		} else {
			getNextWord();
			_script_cond_c = getNextWord();
		}
	} break;

	case 132: {
		o_save_game();
	} break;

	case 133: {
		o_load_game();
	} break;

	case 134: {
		warning("stopMidiMusic: not implemented");
		/* dummy proc */
	} break;

	case 135: {
		error("Quit if user presses Y unimplemented");
	} break;

	case 136: { /* set var to item unk3 */
		Item *item = getNextItemPtr();
		writeNextVarContents(item->unk3);
	} break;

	case 137: {
		o_unk_137(getVarOrByte());
	} break;

	case 138: {
		o_unk_138();
	} break;

	case 139: { /* SET_PARENT_SPECIAL */
		Item *item = getNextItemPtr();
		_no_parent_notify = true;
		setItemParent(item, getNextItemPtr());
		_no_parent_notify = false;
	} break;

	case 140: {
		killAllTimers();
		addTimeEvent(3, 0xA0);
	} break;
	
	case 141: {
		uint which = getVarOrByte();
		Item *item = getNextItemPtr();
		if(which == 1) {
			_subject_item = item;
		} else {
			_object_item = item;
		}
	} break;

	case 142: {
		condition = is_hitarea_0x40_clear(getVarOrWord());
	} break;

	case 143: { /* start item sub */
		Child1 *child = findChildOfType1(getNextItemPtr());
		if (child != NULL) {
			Subroutine *sub = getSubroutineByID(child->subroutine_id);
			if (sub)
				startSubroutine(sub);
		}
	} break;

	case 151: { /* set array6 to item */
		uint var = getVarOrByte();
		Item *item = getNextItemPtr();
		_item_array_6[var] = item;
	} break;

	case 152: { /* set m1 or m3 to array6 */
		Item *item = _item_array_6[getVarOrByte()];
		uint var = getVarOrByte();
		if (var==1) {
			_subject_item = item;
		} else {
			_object_item = item;
		}
	} break;

	case 153: { /* set bit */
		uint bit = getVarOrByte();
		_bit_array[bit>>4] |= 1<<(bit&15);
		break;
	}

	case 154: { /* clear bit */
		uint bit = getVarOrByte();
		_bit_array[bit>>4] &= ~(1<<(bit&15));
		break;
	}

	case 155: { /* is bit clear? */
		uint bit = getVarOrByte();
		condition = (_bit_array[bit>>4] & (1<<(bit&15))) == 0;
	} break;

	case 156: { /* is bit set? */
		uint bit = getVarOrByte();
		condition = (_bit_array[bit>>4] & (1<<(bit&15))) != 0;
	} break;

	case 157: { /* get item int prop */
		Item *item = getNextItemPtr();
		Child2 *child = findChildOfType2(item);
		uint prop = getVarOrByte();

		if (child != NULL && child->avail_props&(1<<prop) && prop < 16) {
			uint offs = getOffsetOfChild2Param(child, 1<<prop);
			writeNextVarContents(child->array[offs]);
		} else {
			writeNextVarContents(0);	
		}
	} break;

	case 158: { /* set item prop */
		Item *item = getNextItemPtr();
		Child2 *child = findChildOfType2(item);
		uint prop = getVarOrByte();
		int value = getVarOrWord();

		if (child != NULL && child->avail_props&(1<<prop) && prop < 16) {
			uint offs = getOffsetOfChild2Param(child, 1<<prop);
			child->array[offs] = value;
		}
	} break;

	case 160: {
		o_unk_160(getVarOrByte());
	} break;

	case 161: { /* setup text */
		uint value = getVarOrByte();
		ThreeValues *tv;

		switch(value) {
		case 1: tv = &_threevalues_1; break;
		case 2: tv = &_threevalues_2; break;
		case 101: tv = &_threevalues_3; break;
		case 102: tv = &_threevalues_4; break;
		default:
			error("setup text, invalid value %d", value);
		}

		tv->a = getVarOrWord();
		tv->b = getVarOrByte();
		tv->c = getVarOrWord();
	} break;

	case 162: {
		o_print_str();
	} break;

	case 163: {
		o_unk_163(getVarOrWord());
	} break;

	case 164: {
		_show_preposition = true;
		o_setup_cond_c();
		_show_preposition = false;
	} break;

	case 165: {
		Item *item = getNextItemPtr();
		int16 a = getNextWord(),
					b = getNextWord();
		condition = (item->unk2 == a && item->unk1 == b);
	} break;

	case 166: { /* set bit2 */
		uint bit = getVarOrByte();
		_bit_array[(bit>>4)+16] |= 1<<(bit&15);
	} break;
		
	case 167: { /* clear bit2 */
		uint bit = getVarOrByte();
		_bit_array[(bit>>4)+16] &= ~(1<<(bit&15));
	} break;

	case 168: { /* is bit clear? */
		uint bit = getVarOrByte();
		condition = (_bit_array[(bit>>4)+16] & (1<<(bit&15))) == 0;
	} break;

	case 169: { /* is bit set? */
		uint bit = getVarOrByte();
		condition = (_bit_array[(bit>>4)+16] & (1<<(bit&15))) != 0;
	} break;

	case 175: {
		o_unk_175();
	} break;

	case 176: {
		o_unk_176();
	} break;

	case 177: {
		o_177();
	} break;

	case 178: { /* path find */
		uint a = getVarOrWord();
		uint b = getVarOrWord();
		uint c = getVarOrByte();
		uint d = getVarOrByte();
		o_pathfind(a,b,c,d);
	} break;

	case 179: {
		if (_game == GAME_SIMON1WIN) {
			uint b = getVarOrByte();
			/*uint c = */getVarOrByte();
			uint a = getVarOrByte();
			uint d = _array_4[a];
			if (d!=0)
				talk_with_speech(d, b);
		} else if (_game == GAME_SIMON1DOS) {
			uint b = getVarOrByte();
			uint c = getVarOrByte();
			uint a = getVarOrByte();
			const char *s = (const char*)getStringPtrByID(_stringid_array_3[a]);
			ThreeValues *tv;
			
			switch(b) {
			case 1: tv = &_threevalues_1; break;
			case 2: tv = &_threevalues_2; break;
			case 101: tv = &_threevalues_3; break;
			case 102: tv = &_threevalues_4; break;
			default:
				error("setup text, invalid value %d", b);
			}

			talk_with_text(b, c, s, tv->a, tv->b, tv->c);
		} else if (_game == GAME_SIMON2WIN) {
			uint b = getVarOrByte();
			uint c = getVarOrByte();
			uint a = getVarOrByte();
			uint d;
			const char *s = (const char*)getStringPtrByID(_stringid_array_3[a]);
			ThreeValues *tv;
			
			switch(b) {
			case 1: tv = &_threevalues_1; break;
			case 2: tv = &_threevalues_2; break;
			case 101: tv = &_threevalues_3; break;
			case 102: tv = &_threevalues_4; break;
			default:
				error("setup text, invalid value %d", b);
			}

			d = _array_4[a];
			if (d!=0 && !_vk_t_toggle)
				talk_with_speech(d, b);

			if (s!=NULL && _vk_t_toggle)
				talk_with_text(b, c, s, tv->a, tv->b, tv->c);
		}
	} break;

	case 180: {
		o_force_unlock();
	} break;

	case 181: {
		o_force_lock();
		if (_game == GAME_SIMON2WIN) {
			fcs_unk_2(1);
			showMessageFormat("\xC");		
		}
	} break;
	
	case 182: {
		if (_game & GAME_SIMON2) goto invalid_opcode;
		o_read_vgares_328();
	} break;

	case 183: {
		if (_game & GAME_SIMON2) goto invalid_opcode;
		o_read_vgares_23();
	} break;

	case 184: {
		o_clear_vgapointer_entry(getVarOrWord());
	} break;

	case 185: {
		if (_game & GAME_SIMON2) goto invalid_opcode;
		getVarOrWord();
	} break;

	case 186: {
		o_unk_186();
	} break;

	case 187: {
		if (_game & GAME_SIMON2) goto invalid_opcode;
		o_fade_to_black();
	} break;

	case 188: 
		if (!(_game & GAME_SIMON2)) goto invalid_opcode;
		{
			uint i = getVarOrByte();
			uint str = getNextStringID();
			condition = (str<20 && _stringid_array_2[i] == str);
		} break;

	case 189: {
		if (!(_game & GAME_SIMON2)) goto invalid_opcode;
		_op_189_flags = 0;
	} break;

	case 190: {
		uint i;
		if (!(_game & GAME_SIMON2)) goto invalid_opcode;
		i = getVarOrByte();
		if (!(_op_189_flags&(1<<i)))
			o_190_helper(i);
	} break;

	default:
invalid_opcode:;
		error("Invalid opcode '%d'", opcode);
	}

	} while (condition != flag);

	return 0;
}

void SimonState::o_190_helper(uint i) {
	warning("o_190_helper not implemented");
}


bool SimonState::o_unk_23(uint a) {
	if (a == 0)
		return 0;

	if (a == 100)
		return 1;

	a += _script_unk_1;
	if (a<=0) {
		_script_unk_1 = 0;
		return 0;
	}
	
	if (((uint)(rand()>>5))%100 < a) {
		if (_script_unk_1 <= 0)
			_script_unk_1 -= 5;
		else
			_script_unk_1 = 0;
		return 1;
	}

	if (_script_unk_1 >= 0)
		_script_unk_1 += 5;
	else
		_script_unk_1 = 0;

	return 0;
}

void SimonState::o_177() {
	if (_game == GAME_SIMON1WIN) {
		uint a = getVarOrByte();
		/*uint b = */getVarOrByte();
		uint offs;
		Child2 *child = findChildOfType2(getNextItemPtr());
		if (child != NULL && child->avail_props&0x200) {
			offs = getOffsetOfChild2Param(child, 0x200);
			talk_with_speech(child->array[offs], a);
		} else if (child != NULL && child->avail_props&0x100) {
			offs = getOffsetOfChild2Param(child, 0x100);
			talk_with_speech(child->array[offs]+3550, a);
		}
	} else if (_game == GAME_SIMON1DOS) {
		uint a = getVarOrByte();
		uint b = getVarOrByte();
		Child2 *child = findChildOfType2(getNextItemPtr());
		if (child!=NULL && child->avail_props&1) {
			const char *s = (const char*)getStringPtrByID(child->array[0]);
			ThreeValues *tv;
			char buf[256];
			switch(a) {
			case 1: tv = &_threevalues_1; break;
			case 2: tv = &_threevalues_2; break;
			case 101: tv = &_threevalues_3; break;
			case 102: tv = &_threevalues_4; break;
			default:
				error("setup text, invalid value %d", a);
			}

			if (child->avail_props&0x100) {
				uint x = getOffsetOfChild2Param(child,0x100);
				sprintf(buf,"%d%s",child->array[x],s);
				s = buf;
			}

			talk_with_text(a,b,s,tv->a, tv->b,tv->c);
		}
	} else if (_game == GAME_SIMON2WIN) {
		uint a = getVarOrByte();
		uint b = getVarOrByte();
		Child2 *child = findChildOfType2(getNextItemPtr());
		const char *s = NULL;
		ThreeValues *tv = NULL;
		char buf[256];

		if (child != NULL && child->avail_props&1) {
			s = (const char*)getStringPtrByID(child->array[0]);
			switch(a) {
			case 1: tv = &_threevalues_1; break;
			case 2: tv = &_threevalues_2; break;
			case 101: tv = &_threevalues_3; break;
			case 102: tv = &_threevalues_4; break;
			default:
				error("setup text, invalid value %d", a);
			}
		}

		if (child != NULL && child->avail_props&0x200) {
			uint var200 = child->array[getOffsetOfChild2Param(child, 0x200)];

			if (child->avail_props&0x100) {
				uint var100 = child->array[getOffsetOfChild2Param(child, 0x100)];

				if (var200 == 116) var200 = var100 + 115;
				if (var200 == 92) var200 = var100 + 98;
				if (var200 == 99) var200 = 9;
				if (var200 == 97) {
					switch(var100) {
					case 12: var200 = 109; break;
					case 14: var200 = 108; break;
					case 18: var200 = 107; break;
					case 20: var200 = 106; break;
					case 22: var200 = 105; break;
					case 28: var200 = 104; break;
					case 90: var200 = 103; break;
					case 92: var200 = 102; break;
					case 100: var200 = 51; break;
					default:
						error("o_177: invalid case %d", var100);
					}
				}
			}

			if (!_vk_t_toggle)
				talk_with_speech(var200, a);
		} 

		if (!_vk_t_toggle)
			return;

		if (child==NULL || !(child->avail_props&1))
			return;
			
		if (child->avail_props&0x100) {
			sprintf(buf, "%d%s", child->array[getOffsetOfChild2Param(child, 0x100)], s);
			s = buf;
		}
		
		talk_with_text(a,b,s,tv->a, tv->b,tv->c); 
	}
}


void SimonState::o_unk_137(uint fcs_index) {
	FillOrCopyStruct *fcs;

	fcs = _fcs_ptr_array_3[fcs_index & 7];
	if (fcs->fcs_data == NULL)
		return;
	fcs_unk_proc_1(fcs_index, fcs->fcs_data->item_ptr, fcs->fcs_data->unk1, fcs->fcs_data->unk2);
}

byte SimonState::getByte() {
	return *_code_ptr++;
}

int SimonState::getNextWord() {
	_code_ptr += 2;
	return (int16)((_code_ptr[-2]<<8) | _code_ptr[-1]);
}

uint SimonState::getNextStringID() {
	return (uint16)getNextWord();
}

uint SimonState::getVarOrByte() {
	uint a = *_code_ptr++;
	if (a!=255)
		return a;
	return readVariable(*_code_ptr++);
}

uint SimonState::getVarOrWord() {
	uint a = (_code_ptr[0]<<8) | _code_ptr[1];
	_code_ptr += 2;
	if (a>=30000 && a<30512)
		return readVariable(a - 30000);
	return a;
}

Item *SimonState::getNextItemPtr() {
	int a = getNextWord();
	switch(a) {
	case -1: return _subject_item;
	case -3: return _object_item;
	case -5: return getItem1Ptr();
	case -7: return getItemPtrB();
	case -9: return derefItem(getItem1Ptr()->parent);
	default:
		return derefItem(a);
	}
}

Item *SimonState::getNextItemPtrStrange() {
	int a = getNextWord();
	switch(a) {
	case -1: return _subject_item;
	case -3: return _object_item;
	case -5: return &_dummy_item_2;
	case -7: return NULL;
	case -9: return &_dummy_item_3;
	default:
		return derefItem(a);
	}
}


uint SimonState::getNextItemID() {
	int a = getNextWord();
	switch(a) {
	case -1: return itemPtrToID(_subject_item);
	case -3: return itemPtrToID(_object_item);
	case -5: return getItem1ID();
	case -7: return 0;
	case -9: return getItem1Ptr()->parent;
	default:
		return a;
	}
}

Item *SimonState::getItem1Ptr() {
	if (_item_1_ptr)
		return _item_1_ptr;	
	return &_dummy_item_1;
}

Item *SimonState::getItemPtrB() {
	error("getItemPtrB: is this code ever used?");
	if (_item_ptr_B)
		return _item_ptr_B;	
	return &_dummy_item_1;
}

uint SimonState::getNextVarContents() {
	return (uint16)readVariable(getVarOrByte());
}

uint SimonState::readVariable(uint variable) {
	if (variable >= 255)
		error("Variable %d out of range in read", variable);
	return _variableArray[variable];
}

void SimonState::writeNextVarContents(uint16 contents) {
	writeVariable(getVarOrByte(), contents);
}

void SimonState::writeVariable(uint variable, uint16 contents) {
	if (variable >= 256)
		error("Variable %d out of range in write", variable);
	_variableArray[variable] = contents;
}

void SimonState::setItemParent(Item *item, Item *parent) {
	Item *old_parent = derefItem(item->parent);

	if (item==parent)
		error("Trying to set item as its own parent");
	
	/* unlink it if it has a parent */
	if (old_parent)
		unlinkItem(item);
	itemChildrenChanged(old_parent);
	linkItem(item, parent);
	itemChildrenChanged(parent);
}

void SimonState::itemChildrenChanged(Item *item) {
	int i;
	FillOrCopyStruct *fcs;

	if (_no_parent_notify)
		return;
		
	lock();

	for(i=0; i!=8; i++) {
		fcs = _fcs_ptr_array_3[i];
		if (fcs && fcs->fcs_data && fcs->fcs_data->item_ptr == item) {
			if (_fcs_data_1[i]) {
				_fcs_data_2[i] = true;
			} else {
				_fcs_data_2[i] = false;
				fcs_unk_proc_1(i, item, fcs->fcs_data->unk1, fcs->fcs_data->unk2);
			}
		}		
	}	
		
	unlock();		
}

void SimonState::unlinkItem(Item *item) {
	Item *first, *parent, *next;

	/* cannot unlink item without parent */
	if (item->parent == 0)
		return;

	/* get parent and first child of parent */
	parent = derefItem(item->parent);
	first = derefItem(parent->child);
	
	/* the node to remove is first in the parent's children? */
	if (first == item) {
		parent->child = item->sibling;
		item->parent = 0;
		item->sibling = 0;
		return;
	}

	for(;;) {
		if (!first)
			error("unlinkItem: parent empty");
		if (first->sibling == 0)
			error("unlinkItem: parent does not contain child");

		next = derefItem(first->sibling);
		if (next == item) {
			first->sibling = next->sibling;
			item->parent = 0;
			item->sibling = 0;
			return;			
		}
		first = next;
	}
}

void SimonState::linkItem(Item *item, Item *parent) {
	uint id;
	/* Don't allow that an item that is already linked is relinked */
	if (item->parent)
		return;

	id = itemPtrToID(parent);
	item->parent = id;

	if (parent != 0) {
		item->sibling = parent->child;
		parent->child = itemPtrToID(item);
	} else {
		item->sibling = 0;
	}
}

const byte *SimonState::getStringPtrByID(uint string_id) {
	const byte *string_ptr;
	byte *dst;

	_free_string_slot ^= 1;	

	if (string_id < 0x8000) {
		string_ptr = _stringtab_ptr[string_id];
	} else {
		string_ptr = getLocalStringByID(string_id);
	}

	dst = &_stringReturnBuffer[_free_string_slot][0];
	strcpy((char*)dst, (const char*)string_ptr);
	return dst;
}

const byte *SimonState::getLocalStringByID(uint string_id) {
	if (string_id < _string_id_local_min || string_id >= _string_id_local_max) {
		loadTextIntoMem(string_id);
	}
	return _local_stringtable[string_id - _string_id_local_min];
}

void SimonState::loadTextIntoMem(uint string_id) {
	byte *p;
	char filename[30];
	int i;
	uint base_min = 0x8000, base_max, size;

	_tablesheap_ptr = _tablesheap_ptr_new;
	_tablesheap_curpos = _tablesheap_curpos_new;

	p = _stripped_txt_mem;

	/* get filename */
	while (*p) {
		for(i=0;*p;p++,i++)
			filename[i] = *p;
		filename[i] = 0;
		p++;

		base_max = (p[0]<<8) | p[1];
		p += 2;

		if (string_id < base_max) {
			_string_id_local_min = base_min;
			_string_id_local_max = base_max;
			
			_local_stringtable = (byte**)_tablesheap_ptr;

			size = (base_max - base_min + 1) * sizeof(byte*);
			_tablesheap_ptr += size;
			_tablesheap_curpos += size;

			size = loadTextFile(filename, _tablesheap_ptr);

			setupLocalStringTable(_tablesheap_ptr, base_max - base_min + 1);

			_tablesheap_ptr += size;
			_tablesheap_curpos += size;

			if (_tablesheap_curpos > _tablesheap_size) {
				error("loadTextIntoMem: Out of table memory");
			}
			return;
		}

		base_min = base_max;
	}

	error("loadTextIntoMem: didn't find %d", string_id);
}

void SimonState::loadTablesIntoMem(uint subr_id) {
	byte *p;
	int i;
	uint min_num, max_num;
	char filename[30];
	FILE *in;

	p = _tbl_list;
	if (p == NULL)
			return;

	while (*p) {
		for(i=0;*p;p++,i++)
			filename[i] = *p;
		filename[i] = 0;
		p++;

		for(;;) {
			min_num = (p[0]<<8) | p[1];
			p += 2;

			if (min_num==0)
				break;

			max_num = (p[0]<<8) | p[1];
			p += 2;

			if (subr_id >= min_num && subr_id <= max_num) {
				_subroutine_list = _subroutine_list_org;
				_tablesheap_ptr = _tablesheap_ptr_org;
				_tablesheap_curpos = _tablesheap_curpos_org;
				_string_id_local_min = 1;
				_string_id_local_max = 0;

				in = openTablesFile(filename);
				readSubroutineBlock(in);
				closeTablesFile(in);

				memcpy(filename, "SFXXXX", 6);
				readSfxFile(filename);

				alignTableMem();
	
				_tablesheap_ptr_new = _tablesheap_ptr;
				_tablesheap_curpos_new = _tablesheap_curpos;

				if (_tablesheap_curpos > _tablesheap_size)
					error("loadTablesIntoMem: Out of table memory");
				return;
			}
		}
	}

	warning("loadTablesIntoMem: didn't find %d", subr_id);
}

Subroutine *SimonState::getSubroutineByID(uint subroutine_id) {
	Subroutine *cur;
		
	for(cur=_subroutine_list; cur; cur = cur->next) {
		if (cur->id == subroutine_id)
			return cur;
	}

	loadTablesIntoMem(subroutine_id);
	
	for(cur=_subroutine_list; cur; cur = cur->next) {
		if (cur->id == subroutine_id)
			return cur;
	}

	warning("getSubroutineByID: subroutine %d not found", subroutine_id);
	return NULL;
}

uint SimonState::loadTextFile_gme(const char *filename, byte *dst) {
	uint res;
	uint32 offs;
	uint32 size;

	res = atoi(filename + 4) + gss->TEXT_INDEX_BASE - 1;
	offs = _game_offsets_ptr[res];
	size = _game_offsets_ptr[res+1] - offs;

	resfile_read(dst, offs, size);

	return size;
}

FILE *SimonState::openTablesFile_gme(const char *filename) {
	uint res;
	uint32 offs;

	res = atoi(filename + 6) + gss->TABLE_INDEX_BASE - 1;
	offs = _game_offsets_ptr[res];

	fseek(_game_file, offs, SEEK_SET);
	return _game_file;
}

void SimonState::closeTablesFile_gme(FILE *in) {
	/* not needed */
}

/* Simon1DOS load tables file */
uint SimonState::loadTextFile_simon1(const char *filename, byte *dst) {
	FILE *fo = fopen_maybe_lowercase(filename);
	uint32 size;
	
	if (fo==NULL)
		error("loadTextFile: Cannot open '%s'", filename);

	fseek(fo, 0, SEEK_END);
	size = ftell(fo);
	rewind(fo);

	if (fread(dst, size,1, fo) != 1)
		error("loadTextFile: fread failed");
	fclose(fo);

	return size;
}


FILE *SimonState::openTablesFile_simon1(const char *filename) {
	FILE *fo = fopen_maybe_lowercase(filename);
	if (fo==NULL)
		error("openTablesFile: Cannot open '%s'", filename);
	return fo;
}

void SimonState::closeTablesFile_simon1(FILE *in) {
	fclose(in);
}

uint SimonState::loadTextFile(const char *filename, byte *dst) {
	if (_game == GAME_SIMON1DOS)
		return loadTextFile_simon1(filename, dst);
	else
		return loadTextFile_gme(filename, dst);
}

FILE *SimonState::openTablesFile(const char *filename) {
	if (_game == GAME_SIMON1DOS)
		return openTablesFile_simon1(filename);
	else
		return openTablesFile_gme(filename);
}

void SimonState::closeTablesFile(FILE *in) {
	if (_game == GAME_SIMON1DOS)
		closeTablesFile_simon1(in);
	else
		closeTablesFile_gme(in);
}

void SimonState::addTimeEvent(uint timeout, uint subroutine_id) {
	TimeEvent *te = (TimeEvent*)malloc(sizeof(TimeEvent)), *first, *last = NULL;
	time_t cur_time;

	time(&cur_time);

	te->time = cur_time + timeout - _base_time;
	te->subroutine_id = subroutine_id;

	first = _first_time_struct;
	while (first) {
		if (te->time <= first->time) {
			if (last) {
				last->next = te;
				te->next = first;
				return;
			}
			te->next = _first_time_struct;
			_first_time_struct = te;
			return;
		}

		last = first;
		first = first->next;
	}

	if (last) {
		last->next = te;
		te->next = NULL;
	} else {
		_first_time_struct = te;
		te->next = NULL;
	}
}

void SimonState::delTimeEvent(TimeEvent *te) {
	TimeEvent *cur;

	if (te == _pending_delete_time_event)
		_pending_delete_time_event = NULL;

	if (te == _first_time_struct) {
		_first_time_struct = te->next;
		free(te);
		return;
	}

	cur = _first_time_struct;
	if (cur == NULL)
		error("delTimeEvent: none available");
	
	for(;;) {
		if (cur->next == NULL)
			error("delTimeEvent: no such te");
		if (te == cur->next) {
			cur->next = te->next;
			free(te);
			return;
		}
		cur = cur->next;
	}
}

void SimonState::killAllTimers() {
	TimeEvent *cur, *next;

	for(cur=_first_time_struct; cur; cur = next) {
		next = cur->next;
		delTimeEvent(cur);
	}
}

bool SimonState::kickoffTimeEvents() {
	time_t cur_time;
	TimeEvent *te;
	bool result = false;

	time(&cur_time);
	cur_time -= _base_time;

	while ((te=_first_time_struct) != NULL && te->time<=(uint32)cur_time) {
		result = true;
		_pending_delete_time_event = te;
		invokeTimeEvent(te);
		if (_pending_delete_time_event) {
			_pending_delete_time_event = NULL;
			delTimeEvent(te);
		}
	}

	return result;
}

void SimonState::invokeTimeEvent(TimeEvent *te) {
	Subroutine *sub;

	_script_cond_a = 0;
	if (_run_script_return_1)
		return;
	sub = getSubroutineByID(te->subroutine_id);
	if (sub != NULL)
		startSubroutineEx(sub);
	_run_script_return_1 = false;
}

void SimonState::o_setup_cond_c() {
	Item *item = _item_1;

	setup_cond_c_helper();

	_item_1_ptr = item;
	_object_item = _hitarea_object_item;
	
	if (_object_item == &_dummy_item_2)
		_object_item = getItem1Ptr();
	
	if (_object_item == &_dummy_item_3)
		_object_item = derefItem(getItem1Ptr()->parent);

	if (_object_item != NULL) {
		_script_cond_c = _object_item->unk1;
	} else {
		_script_cond_c = -1;
	}
}

void SimonState::setup_cond_c_helper() {
	HitArea *last;

	if (_game == GAME_SIMON2WIN) {
		_mouse_cursor = 0;
		if (_hitarea_unk_4!=999) {
			_mouse_cursor = 9;
			_need_hitarea_recalc++;
			_hitarea_unk_4 = 0;
		}
	}

	_last_hitarea = 0;
	_hitarea_object_item = NULL;
	_hitarea_unk_6 = true;
	
	last = _last_hitarea_2_ptr;
	defocusHitarea();
	_last_hitarea_2_ptr = last;

	for(;;) {
		_last_hitarea = NULL;
		_last_hitarea_3 = 0;
		_left_button_down = 0;

		do {
			if (GetAsyncKeyState(VK_F5) != 0 && _bit_array[0]&0x200) {
				startSubroutine170();
				goto out_of_here;
			}

			delay(100);
		} while (_last_hitarea_3 == (HitArea*)0xFFFFFFFF || _last_hitarea_3 == 0);

		if (_last_hitarea == NULL) {
		} else if (_last_hitarea->id == 0x7FFB) {
			handle_unk2_hitarea(_last_hitarea->fcs);
		} else if (_last_hitarea->id == 0x7FFC) {
			handle_unk_hitarea(_last_hitarea->fcs);
		} else if (_last_hitarea->item_ptr != NULL) {
			_hitarea_object_item = _last_hitarea->item_ptr;
			_variableArray[60] = (_last_hitarea->flags&1) ? (_last_hitarea->flags>>8) : 0xFFFF;
			break;
		}
	}

out_of_here:
	_last_hitarea_3 = 0;
	_last_hitarea = 0;
	_last_hitarea_2_ptr = NULL;
	_hitarea_unk_6 = false;
}

void SimonState::startSubroutine170() {
	Subroutine *sub;

	/* XXX: stop speech */

	sub = getSubroutineByID(170);
	if (sub != NULL)
		startSubroutineEx(sub);

	_run_script_return_1 = true;
}

void SimonState::defocusHitarea() {
	HitArea *last;
	HitArea *ha;

	if (_game == GAME_SIMON2WIN) {
		if (_bit_array[4]&0x8000) {
			o_unk_120(202);
			_last_hitarea_2_ptr = NULL;
			return;
		}
	}
	
	last = _hitarea_ptr_5;

	if (last == _hitarea_ptr_7)
		return;

	hitareaChangedHelper();
	_hitarea_ptr_7 = last;

	if (last != NULL && _hitarea_unk_6 &&
			(ha = findHitAreaByID(200)) && (ha->flags&0x40) &&
		  !(last->flags&0x40))
			focusVerb(last->id);
}

static const char * const verb_names[] = {
	"Walk to",
	"Look at",
	"Open",
	"Move",

	"Consume",
	"Pick up",
	"Close",
	"Use",

	"Talk to",
	"Remove",
	"Wear",
	"Give"
};

static const char * const verb_prep_names[] = {
	"","","","",
	"","","","with what ?",
	"","","","to whom ?"
};

void SimonState::focusVerb(uint hitarea_id) {
	uint x;
	const char *txt;

	hitarea_id -= 101;

	CHECK_BOUNDS(hitarea_id, verb_prep_names);
	
	if (_show_preposition) {
		txt = verb_prep_names[hitarea_id];
	} else {
		txt = verb_names[hitarea_id];
	}
	x = (53 - strlen(txt)) * 3;
	showActionString(x, (const byte*)txt);

}

void SimonState::showActionString(uint x, const byte *string) {
	FillOrCopyStruct *fcs;

	fcs = _fcs_ptr_array_3[1];
	if (fcs ==  NULL || fcs->text_color==0)
		return;

	fcs->unk1 = x >> 3;
	fcs->unk3 = x & 7;

	for(;*string;string++)
		video_putchar(fcs, *string);
}


void SimonState::hitareaChangedHelper() {
	FillOrCopyStruct *fcs;

	if (_game == GAME_SIMON2WIN) {
		if (_bit_array[4]&0x8000)
			return;
	}

	fcs = _fcs_ptr_array_3[1];
	if (fcs != NULL && fcs->text_color != 0)
		video_fill_or_copy_from_3_to_2(fcs);

	_last_hitarea_2_ptr = NULL;
	_hitarea_ptr_7 = NULL;
}

HitArea *SimonState::findHitAreaByID(uint hitarea_id) {
	HitArea *ha = _hit_areas;
	uint count = ARRAYSIZE(_hit_areas);
	
	do{
		if (ha->id == hitarea_id)
			return ha;
	} while(ha++,--count);
	return NULL;
}

HitArea *SimonState::findEmptyHitArea() {
	HitArea *ha = _hit_areas;
	uint count = ARRAYSIZE(_hit_areas);
	
	do{
		if (ha->flags == 0)
			return ha;
	} while(ha++,--count);
	return NULL;
}

void SimonState::clear_hitarea_bit_0x40(uint hitarea) {
	HitArea *ha = findHitAreaByID(hitarea);
	if (ha != NULL)
		ha->flags &= ~0x40;
}

void SimonState::set_hitarea_bit_0x40(uint hitarea) {
	HitArea *ha = findHitAreaByID(hitarea);
	if (ha != NULL) {
		ha->flags |= 0x40;
		ha->flags &= ~2;
		if (hitarea == 102)
			hitarea_proc_1();
	}
}

void SimonState::set_hitarea_x_y(uint hitarea, int x, int y) {
	HitArea *ha = findHitAreaByID(hitarea);
	if (ha != NULL) {
		ha->x = x;
		ha->y = y;
	}
}

void SimonState::delete_hitarea(uint hitarea) {
	HitArea *ha = findHitAreaByID(hitarea);
	if (ha != NULL) {
		ha->flags = 0;
		if (ha == _last_hitarea_2_ptr)
			defocusHitarea();
		_need_hitarea_recalc++;
	}
}

bool SimonState::is_hitarea_0x40_clear(uint hitarea) {
	HitArea *ha = findHitAreaByID(hitarea);
	if (ha == NULL)
		return false;
	return (ha->flags & 0x40) == 0;
}

void SimonState::addNewHitArea(int id, int x, int y, int width, int height, 
	int flags, int unk3,Item *item_ptr) {
	
	HitArea *ha;
	delete_hitarea(id);

	ha = findEmptyHitArea();
	ha->x = x;
	ha->y = y;
	ha->width = width;
	ha->height = height;
	ha->flags = flags | 0x20;
	ha->id = ha->layer = id;
	ha->unk3 = unk3;
	ha->item_ptr = item_ptr;

	_need_hitarea_recalc++;
}

void SimonState::hitarea_proc_1() {
	uint id;
	HitArea *ha;

	if (_game & GAME_SIMON2) {
		id=2;
		if (!(_bit_array[4]&0x8000))
			id = (_mouse_y >= 136) ? 102 : 101;
	} else {
		id = (_mouse_y >= 136) ? 102 : 101;

	}

	_hitarea_unk_4 = id;

	ha = findHitAreaByID(id);
	if (ha == NULL)
		return;

	if (ha->flags & 0x40) {
		_hitarea_unk_4 = 999;
		_hitarea_ptr_5 = NULL;
	} else {
		_verb_hitarea = ha->unk3;
		handle_verb_hitarea(ha);
	}
}

void SimonState::handle_verb_hitarea(HitArea *ha) {
	HitArea *tmp = _hitarea_ptr_5;

	if (ha == tmp) 
		return;

	if (!(_game & GAME_SIMON2)) {
		if (tmp != NULL) {
			tmp->flags |= 8;
			video_toggle_colors(tmp, 0xd5, 0xd0, 0xd5, 0xA);
		}

		if (ha->flags & 2)
			video_toggle_colors(ha, 0xda, 0xd5, 0xd5, 5);
		else
			video_toggle_colors(ha, 0xdf, 0xda, 0xda, 0xA);

		ha->flags &= ~ (2 + 8);

	} else {
		if (ha->id<101)
			return;
		_mouse_cursor = ha->id - 101;
		_need_hitarea_recalc++;

	}

	_hitarea_ptr_5 = ha;
}

void SimonState::hitarea_leave(HitArea *ha) {
	if (!(_game & GAME_SIMON2)) {
		video_toggle_colors(ha, 0xdf, 0xd5, 0xda, 5);
	} else {
		video_toggle_colors(ha, 0xe7, 0xe5, 0xe6, 1);
	}
}

void SimonState::leaveHitAreaById(uint hitarea_id) {
	HitArea *ha = findHitAreaByID(hitarea_id);
	if (ha)
		hitarea_leave(ha);
}

void SimonState::handle_unk2_hitarea(FillOrCopyStruct *fcs) {
	uint index;

	index = get_fcs_ptr_3_index(fcs);

	if (fcs->fcs_data->unk1 == 0)
		return;

	lock();
	fcs_unk_proc_1(index, fcs->fcs_data->item_ptr, 
		fcs->fcs_data->unk1-1, fcs->fcs_data->unk2);
	unlock();
}

void SimonState::handle_unk_hitarea(FillOrCopyStruct *fcs) {
	uint index;

	index = get_fcs_ptr_3_index(fcs);

	lock();
	fcs_unk_proc_1(index, fcs->fcs_data->item_ptr, 
		fcs->fcs_data->unk1+1, fcs->fcs_data->unk2);
	unlock();
}

void SimonState::setup_hitarea_from_pos(uint x, uint y, uint mode) {
	HitArea *best_ha;

	if (_game & GAME_SIMON2) {
		if (_bit_array[4]&0x8000 || y < 134) {
			x += _x_scroll * 8;
		}
	}

	{
		HitArea *ha = _hit_areas;
		uint count = ARRAYSIZE(_hit_areas);
		uint16 layer = 0;
		const uint16 x_ = x;
		const uint16 y_ = y;

		best_ha = NULL;

		do{
			if (ha->flags & 0x20) {
				if (!(ha->flags & 0x40)) {
					if (x_ >= ha->x && y_ >= ha->y && 
							x_ - ha->x < ha->width && y_- ha->y < ha->height && 
							layer <= ha->layer) {
							layer = ha->layer;
							best_ha = ha;
					} else {
						if (ha->flags & 2) {
							hitarea_leave(ha);
							ha->flags &=~2;
						}
					}
				} else {
					ha->flags &= ~2;
				}
			}
		} while(ha++,--count);
	}

	if (best_ha == NULL) {
		defocusHitarea();
		return;
	}

	if (mode != 0 && mode != 3) {
		_last_hitarea = best_ha;
		_variableArray[1] = x;
		_variableArray[2] = y;
	}

	if (best_ha->flags&4) {
		defocusHitarea();
	} else if (best_ha != _last_hitarea_2_ptr) {
		new_current_hitarea(best_ha);
	}

	if (best_ha->flags&8  && !(best_ha->flags&2)) {
		hitarea_leave(best_ha);
		best_ha->flags |= 2;
	}

	return;
}

void SimonState::new_current_hitarea(HitArea *ha) {
	bool result;

	hitareaChangedHelper();
	if (ha->flags & 1) {
		result = hitarea_proc_2(ha->flags>>8);
	} else {
		result = hitarea_proc_3(ha->item_ptr);
	}	

	if (result)
		_last_hitarea_2_ptr = ha;
}

bool SimonState::hitarea_proc_2(uint a) {
	uint x;
	const byte *string_ptr;

	if (_game & GAME_SIMON2) {
		if (_bit_array[4]&0x8000) {
			Subroutine *sub;
			_variableArray[84] = a;
			sub = getSubroutineByID(5003);
			if (sub != NULL)
				startSubroutineEx(sub);
			return true;
		}
	}

	if (a >= 20)
		return false;

	string_ptr = getStringPtrByID(_stringid_array_2[a]);
	x = (53 - (strlen((const char*)string_ptr) - 1)) * 3;
	showActionString(x, string_ptr);

	return true;
}

bool SimonState::hitarea_proc_3(Item *item) {
	Child2 *child2;
	uint x;
	const byte *string_ptr;

	if (item == 0 || item==&_dummy_item_2 || item == &_dummy_item_3)
		return false;

	child2 = findChildOfType2(item);
	if (child2 == NULL)
		return false;

	string_ptr = getStringPtrByID(child2->string_id);
	x = (53 - (strlen((const char*)string_ptr) - 1)) * 3;
	showActionString(x, string_ptr);
	return true;
}

uint SimonState::get_fcs_ptr_3_index(FillOrCopyStruct *fcs) {
	uint i;

	for(i=0; i!=ARRAYSIZE(_fcs_ptr_array_3); i++)
		if (_fcs_ptr_array_3[i] == fcs)
			return i;

	error("get_fcs_ptr_3_index: not found");
}

/* Used only in Simon1 */
void SimonState::o_read_vgares_328() {
	if (_vga_res_328_loaded == false) {
		_vga_res_328_loaded = true;
		_lock_word |= 0x4000;
		read_vga_from_datfile_1(328);
		_lock_word &= ~0x4000;
	}
}

/* Used only in Simon1 */
void SimonState::o_read_vgares_23() {
	if (_vga_res_328_loaded == true) {
		_vga_res_328_loaded = false;
		_lock_word |= 0x4000;
		read_vga_from_datfile_1(23);
		_lock_word &= ~0x4000;
	}
}


void SimonState::lock() {
	_lock_counter++;
}

void SimonState::unlock() {
	_lock_word |= 1;

	if (_lock_counter != 0) {
		if (_lock_counter==1) {
			GetAsyncKeyState(VK_LBUTTON);
		}
		_lock_counter--;
	}
	_lock_word &= ~1;
}

void SimonState::handle_mouse_moved() {
	uint x;

	if (_lock_counter)
		return;

	pollMouseXY();

	if (_mouse_x >= 32768)
		_mouse_x = 0;
	if (_mouse_x >= 638/2)
		_mouse_x = 638/2;

	if (_mouse_y >= 32768)
		_mouse_y = 0;
	if (_mouse_y >= 199)
		_mouse_y = 199;

	if (_hitarea_unk_4) {
		uint id = 101;
		if (_mouse_y >= 136)
			id = 102;
		if (_hitarea_unk_4 != id)
			hitarea_proc_1();
	}


	if (_game & GAME_SIMON2) {
		if (_bit_array[4]&0x8000) {
			if (!_vga_var9) {
				if (_mouse_x >= 630/2 || _mouse_x < 9)
					goto get_out2;
				_vga_var9 = 1;
			}
			if (_vga_var2==0) {
				if (_mouse_x >= 631/2) {
					if (_x_scroll != _vga_var1)
						_vga_var3 = 1;
				} else if (_mouse_x < 8) {
					if (_x_scroll != 0)
						_vga_var3 = -1;
				}
			}
		} else {
	get_out2:;
			_vga_var9 = 0;
		}
	}


	if (_mouse_x != _mouse_x_old || _mouse_y != _mouse_y_old)
		_need_hitarea_recalc++;

	x = 0;
	if (_last_hitarea_3 == 0 && _left_button_down != 0) {
		_left_button_down = 0;
		x = 1;
	} else {
		if (_hitarea_unk_3==0 && _need_hitarea_recalc==0) goto get_out;
	}

	setup_hitarea_from_pos(_mouse_x, _mouse_y, x);
	_last_hitarea_3 = _last_hitarea;
	if (x==1 && _last_hitarea==NULL)
		_last_hitarea_3 = (HitArea*)-1;

get_out:
	draw_mouse_pointer();
	_need_hitarea_recalc = 0;
}

void SimonState::fcs_unk_proc_1(uint fcs_index, Item *item_ptr, int unk1, int unk2) {
	Item *item_ptr_org = item_ptr;
	FillOrCopyStruct *fcs_ptr;
	uint width_div_3, height_div_3;
	uint j,k,i,num_sibs_with_flag;
	bool item_again;
	uint x_pos, y_pos;

	fcs_ptr = _fcs_ptr_array_3[fcs_index&7];

	if (!(_game & GAME_SIMON2)) {
		width_div_3 = fcs_ptr->width / 3;
		height_div_3 = fcs_ptr->height / 3;
	} else {
		width_div_3 = 100;
		height_div_3 = 40;
	}

	i = 0;

	if (fcs_ptr == NULL)
		return;

	if (fcs_ptr->fcs_data)
		fcs_unk1(fcs_index);

	fcs_ptr->fcs_data = (FillOrCopyData*)malloc(sizeof(FillOrCopyData));
	fcs_ptr->fcs_data->item_ptr = item_ptr;
	fcs_ptr->fcs_data->unk3 = -1;
	fcs_ptr->fcs_data->unk4 = -1;
	fcs_ptr->fcs_data->unk1 = unk1;
	fcs_ptr->fcs_data->unk2 = unk2;

	item_ptr = derefItem(item_ptr->child);

	while (item_ptr && unk1-- != 0) {
		num_sibs_with_flag = 0;
		while (item_ptr && width_div_3 > num_sibs_with_flag) {
			if ((unk2==0 || item_ptr->unk4&unk2) && has_item_childflag_0x10(item_ptr)) 
				if (!(_game & GAME_SIMON2)) {
					num_sibs_with_flag++;
				} else {
					num_sibs_with_flag+=20;
				}
			item_ptr = derefItem(item_ptr->sibling);
		}
	}

	if (item_ptr == NULL) {
		fcs_ptr->fcs_data->unk1 = 0;
		item_ptr = derefItem(item_ptr_org->child);
	}

	x_pos = 0;
	y_pos = 0;
	item_again = false;
	k = 0;
	j = 0;

	while (item_ptr) {
		if ((unk2==0 || item_ptr->unk4&unk2) &&  has_item_childflag_0x10(item_ptr)) {
			if (item_again == false) {
				fcs_ptr->fcs_data->e[k].item = item_ptr;
				if (!(_game & GAME_SIMON2)) {
					draw_icon_c(fcs_ptr, item_get_icon_number(item_ptr), x_pos*3, y_pos);
					fcs_ptr->fcs_data->e[k].hit_area = 
						setup_icon_hit_area(fcs_ptr, x_pos*3, y_pos, 
							item_get_icon_number(item_ptr), item_ptr);
				} else {
					draw_icon_c(fcs_ptr, item_get_icon_number(item_ptr), x_pos, y_pos);
					fcs_ptr->fcs_data->e[k].hit_area = 
						setup_icon_hit_area(fcs_ptr, x_pos, y_pos, 
							item_get_icon_number(item_ptr), item_ptr);
				}
				k++;
			} else {
				fcs_ptr->fcs_data->e[k].item = NULL;
				j = 1;
			}
			x_pos+= (_game & GAME_SIMON2) ? 20 : 1;

			if (x_pos >= width_div_3) {
				x_pos = 0;

				y_pos+= (_game & GAME_SIMON2) ? 20 : 1;
				if (y_pos >= height_div_3)
					item_again = true;
			}
		}
		item_ptr = derefItem(item_ptr->sibling);
	}

	fcs_ptr->fcs_data->e[k].item = NULL;
	
	if (j!=0 || fcs_ptr->fcs_data->unk1!=0) {
		fcs_unk_proc_2(fcs_ptr, fcs_index);
	}
}

void SimonState::fcs_unk_proc_2(FillOrCopyStruct *fcs, uint fcs_index) {
	setup_hit_areas(fcs, fcs_index);

	fcs->fcs_data->unk3 = _scroll_up_hit_area;
	fcs->fcs_data->unk4 = _scroll_down_hit_area;
}

void SimonState::setup_hit_areas(FillOrCopyStruct *fcs, uint fcs_index) {
	HitArea *ha;

	ha = findEmptyHitArea();
	_scroll_up_hit_area = ha - _hit_areas;
	if (!(_game & GAME_SIMON2)) {
		ha->x = 308;
		ha->y = 149;
		ha->width = 12;
		ha->height = 17;
		ha->flags = 0x24;
		ha->id = 0x7FFB;
		ha->layer = 100;
		ha->fcs = fcs;
		ha->unk3 = 1;
	} else {
		ha->x = 81;
		ha->y = 158;
		ha->width = 12;
		ha->height = 26;
		ha->flags = 36;
		ha->id = 0x7FFB;
		ha->layer = 100;
		ha->fcs = fcs;
		ha->unk3 = 1;
	}

	ha = findEmptyHitArea();
	_scroll_down_hit_area = ha - _hit_areas;

	if (!(_game & GAME_SIMON2)) {
		ha->x = 308;
		ha->y = 176;
		ha->width = 12;
		ha->height = 17;
		ha->flags = 0x24;
		ha->id = 0x7FFC;
		ha->layer = 100;
		ha->fcs = fcs;
		ha->unk3 = 1;

		/* Simon1 specific */
		o_unk_99_simon1(0x80);
		start_vga_code(0, 1, 0x80, 0, 0, 0xE);
	} else {
		ha->x = 227;
		ha->y = 162;
		ha->width = 12;
		ha->height = 26;
		ha->flags = 36;
		ha->id = 0x7FFC;
		ha->layer = 100;
		ha->fcs = fcs;
		ha->unk3 = 1;
	}
}


bool SimonState::has_item_childflag_0x10(Item *item) {
	Child2 *child = findChildOfType2(item);
	return child && (child->avail_props & 0x10) != 0;
}

uint SimonState::item_get_icon_number(Item *item) {
	Child2 *child = findChildOfType2(item);
	uint offs;

	if (child==NULL || !(child->avail_props & 0x10))
		return 0;

	offs = getOffsetOfChild2Param(child, 0x10);
	return child->array[offs];
}

void SimonState::loadIconFile() {
	FILE *in = fopen_maybe_lowercase("ICON.DAT");
	uint size;

	if (in==NULL)
		error("Cannot open icon.dat");
	
	fseek(in, 0, SEEK_END);
	size = ftell(in);

	_icon_file_ptr = (byte*)malloc(size);
	if (_icon_file_ptr == NULL)
		error("Out of icon memory");

	rewind(in);

	fread(_icon_file_ptr, size, 1, in);
	fclose(in);
}


uint SimonState::setup_icon_hit_area(FillOrCopyStruct *fcs,uint x, uint y, uint icon_number, Item *item_ptr) {
	HitArea *ha;

	ha = findEmptyHitArea();

	if (!(_game & GAME_SIMON2)) {
		ha->x = (x+fcs->x) << 3;
		ha->y = y*25 + fcs->y;
		ha->item_ptr = item_ptr;
		ha->width = 24;
		ha->height = 24;
		ha->flags = 0xB0;
		ha->id = 0x7FFD;
		ha->layer = 100;
		ha->unk3 = 0xD0;
	} else {
		ha->x = x + 110;
		ha->y = fcs->y + y;
		ha->item_ptr = item_ptr;
		ha->width = 20;
		ha->height = 20;
		ha->flags = 0xB0;
		ha->id = 0x7FFD;
		ha->layer = 100;
		ha->unk3 = 0xD0;
	}

	return ha - _hit_areas;
}

void SimonState::hitarea_stuff() {
	HitArea *ha;
	uint id;

	_left_button_down = 0;
	_last_hitarea = 0;
	_verb_hitarea = 0;
	_hitarea_subject_item = NULL;
	_hitarea_object_item = NULL;

	hitarea_proc_1();

startOver:
	for(;;) {
		_last_hitarea = NULL;
		_last_hitarea_3 = NULL;
		for(;;) {
			processSpecialKeys();
			if (_last_hitarea_3 == (HitArea*)0xFFFFFFFF) goto startOver;	
			if (_last_hitarea_3 != 0)
				break;
			hitarea_stuff_helper();
			delay(100);
		}

		ha = _last_hitarea;

		if (ha == NULL) {
		} else if(ha->id == 0x7FFB) {
			handle_unk2_hitarea(ha->fcs);
		} else if (ha->id == 0x7FFC) {
			handle_unk_hitarea(ha->fcs);
		} else if (ha->id>=101 && ha->id<113) {
			_verb_hitarea = ha->unk3;
			handle_verb_hitarea(ha);
			_hitarea_unk_4 = 0;
		} else {
			if ( (_verb_hitarea != 0 || _hitarea_subject_item != ha->item_ptr && ha->flags&0x80) &&
					 ha->item_ptr) {
if_1:;
				_hitarea_subject_item = ha->item_ptr;
				id = 0xFFFF;
				if (ha->flags&1)
					id = ha->flags>>8;
				_variableArray[60] = id;
				new_current_hitarea(ha);
				if (_verb_hitarea != 0)
					break;
			} else {
				/* else 1 */
				if (ha->unk3 == 0) {
					if(ha->item_ptr) goto if_1;
				} else {
					_verb_hitarea = ha->unk3 & 0xBFFF;
					if (ha->unk3 & 0x4000) {
						_hitarea_subject_item = ha->item_ptr;
						break;
					}
					if (_hitarea_subject_item != NULL)
						break;
				}
			}
		}
	}

	_need_hitarea_recalc++;
}

void SimonState::hitarea_stuff_helper() {
	time_t cur_time;

	if (!(_game & GAME_SIMON2)) {
		uint subr_id = _variableArray[0x1FC/2];
		if (subr_id != 0) {
			Subroutine *sub = getSubroutineByID(subr_id);
			if (sub != NULL) {
				startSubroutineEx(sub);
				startUp_helper_2();
			}
			_variableArray[0x1FC/2] = 0;
			_run_script_return_1 = false;
		}
	} else {
		if (_variableArray[0x1FC/2] || _variableArray[0x1F2/2]) {
			hitarea_stuff_helper_2();
		}
	}

	time(&cur_time);
	if ((uint)cur_time != _last_time) {
		_last_time = cur_time;
		if (kickoffTimeEvents())
			startUp_helper_2();
	}
}

/* Simon 2 specific */
void SimonState::hitarea_stuff_helper_2() {
	uint subr_id;
	Subroutine *sub;

	subr_id = _variableArray[0x1F2/2];
	if (subr_id != 0) {
		sub = getSubroutineByID(subr_id);
		if (sub != NULL) {
			_variableArray[0x1F2/2] = 0;
			startSubroutineEx(sub);
			startUp_helper_2();
		}
		_variableArray[0x1F2/2] = 0;
	}

	subr_id = _variableArray[0x1FC/2];
	if (subr_id != 0) {
		sub = getSubroutineByID(subr_id);
		if (sub != NULL) {
			_variableArray[0x1FC/2] = 0;
			startSubroutineEx(sub);
			startUp_helper_2();
		}
		_variableArray[0x1FC/2] = 0;
	}

	_run_script_return_1 = false;
}


void SimonState::startUp_helper_2() {
	if (!_mortal_flag) {
		_mortal_flag = true;
		startUp_helper_3();
		_fcs_unk_1 = 0;
		if(_fcs_ptr_array_3[0]!=0) {
			_fcs_ptr_1 = _fcs_ptr_array_3[0];
			showmessage_helper_3(_fcs_ptr_1->unk6, _fcs_ptr_1->unk7);
		}
		_mortal_flag = false;
	}
}

void SimonState::startUp_helper_3() {
	showmessage_print_char(0);
}

void SimonState::showmessage_helper_3(uint a, uint b) {
	_print_char_unk_1 = a;
	_print_char_unk_2 = b;
	_num_letters_to_print = 0;
}

void SimonState::pollMouseXY() {
	_mouse_x = sdl_mouse_x;
	_mouse_y = sdl_mouse_y;
}

void SimonState::handle_verb_clicked(uint verb) {
	Subroutine *sub;
	int result;

	_item_1_ptr = _item_1;

	_object_item = _hitarea_object_item;
	if (_object_item == &_dummy_item_2) {
		_object_item = getItem1Ptr();
	}
	if (_object_item == &_dummy_item_3) {
		_object_item = derefItem(getItem1Ptr()->parent);
	}

	_subject_item = _hitarea_subject_item;
	if (_subject_item == &_dummy_item_2) {
		_subject_item = getItem1Ptr();
	}
	if (_subject_item == &_dummy_item_3) {
		_subject_item = derefItem(getItem1Ptr()->parent);
	}

	if (_subject_item) {
		_script_cond_b = _subject_item->unk1;
	} else {
		_script_cond_b = -1;
	}

	if (_object_item) {
		_script_cond_c = _object_item->unk1;
	} else {
		_script_cond_c = -1;
	}

	_script_cond_a = _verb_hitarea;

	sub = getSubroutineByID(0);
	if (sub==NULL)
		return;

	result = startSubroutine(sub);
	if (result == -1)
		showMessageFormat("I don't understand");

	_run_script_return_1 = false;

	sub = getSubroutineByID(100);
	if(sub) startSubroutine(sub);

	if (_game & GAME_SIMON2)
		_run_script_return_1 = false;

	startUp_helper_2();
}

void SimonState::o_print_str() {
	uint num_1 = getVarOrByte();
	uint num_2 = getVarOrByte();
	uint string_id = getNextStringID();
	const byte *string_ptr = NULL;
	uint speech_id = 0;
	ThreeValues *tv;


	switch(_game) {
	case GAME_SIMON1WIN:
		if (string_id != 0xFFFF)
			string_ptr = getStringPtrByID(string_id);

		speech_id = (uint16)getNextWord();
		break;

	case GAME_SIMON2WIN:
		if (string_id != 0xFFFF)
			string_ptr = getStringPtrByID(string_id);

		speech_id = (uint16)getNextWord();
		break;

	case GAME_SIMON1DOS:
		string_ptr = getStringPtrByID(string_id);
		break;
	}

	switch(num_1) {
	case 1:		tv = &_threevalues_1; break;
	case 2:		tv = &_threevalues_2; break;
	case 101: tv = &_threevalues_3; break;
	case 102: tv = &_threevalues_4; break;
	default:
		error("o_print_str, invalid value %d", num_1);
	}


	switch(_game) {
	case GAME_SIMON1WIN:
#ifdef USE_TEXT_HACK
		if (speech_id != 0) {
			if (string_ptr==NULL)
				talk_with_speech(speech_id, num_1);
			else if(speech_id!=9999)
				playVoice(speech_id);
		}
		
		if (string_ptr != NULL) {
			talk_with_text(num_1, num_2, (char*)string_ptr, tv->a, tv->b, tv->c);
		}
#else
		if (speech_id != 0) {
			talk_with_speech(speech_id, num_1);
		} else if(string_ptr  != NULL) {
			talk_with_text(num_1, num_2, (char*)string_ptr, tv->a, tv->b, tv->c);
		}
#endif
		break;

	case GAME_SIMON1DOS:
		talk_with_text(num_1, num_2, (char*)string_ptr, tv->a, tv->b, tv->c);	
		break;

	case GAME_SIMON2WIN:
		if (speech_id!=0 && num_1 == 1 && !_vk_t_toggle)
			talk_with_speech(speech_id, num_1);

		if (speech_id != 0 && !_vk_t_toggle)
			return;

		if (speech_id == 0)
			o_unk_99_simon2(2, num_1+2);

		talk_with_text(num_1, num_2, (char*)string_ptr, tv->a, tv->b, tv->c);	
		break;		
	}
}

void SimonState::ensureVgaResLoadedC(uint vga_res) {
	_lock_word |= 0x80;
	ensureVgaResLoaded(vga_res);
	_lock_word &= ~0x80;
}

void SimonState::ensureVgaResLoaded(uint vga_res) {
	VgaPointersEntry *vpe;
	
	CHECK_BOUNDS(vga_res, _vga_buffer_pointers);

	vpe = _vga_buffer_pointers + vga_res;
	if (vpe->vgaFile1 != NULL)
		return;

	vpe->vgaFile2 = read_vga_from_datfile_2(vga_res*2+1);
	vpe->vgaFile1 = read_vga_from_datfile_2(vga_res*2);
	
}

byte *SimonState::setup_vga_destination(uint32 size) {
	byte *dest, *end;

	_video_var_4 = 0;

	for(;;) {
		dest = _vga_buf_free_start;

		end = dest + size;

		if (end >= _vga_buf_end) {
			_vga_buf_free_start = _vga_buf_start;
		} else {
			_video_var_5 = false;
			vga_buf_unk_proc3(end);
			if (_video_var_5)
				continue;
			vga_buf_unk_proc1(end);
			if (_video_var_5)
				continue;
			delete_memptr_range(end);
			_vga_buf_free_start = end;
			return dest;
		}
	}
}

void SimonState::setup_vga_file_buf_pointers() {
	byte *alloced;

	alloced = (byte*)malloc(gss->VGA_MEM_SIZE);
	
	_vga_buf_free_start = alloced;
	_vga_buf_start = alloced;
	_vga_file_buf_org = alloced;
	_vga_file_buf_org_2 = alloced;
	_vga_buf_end = alloced + gss->VGA_MEM_SIZE;
}

void SimonState::vga_buf_unk_proc3(byte *end) {
	VgaPointersEntry *vpe;
	
	if (_video_var_7==0xFFFF)
		return;
	
	if (_video_var_4 == 2)
		error("vga_buf_unk_proc3: _video_var_4 == 2");

	vpe = &_vga_buffer_pointers[_video_var_7];

	if (_vga_buf_free_start <= vpe->vgaFile1 && end >= vpe->vgaFile1 ||
		  _vga_buf_free_start <= vpe->vgaFile2 && end >= vpe->vgaFile2) {
		_video_var_5 = 1;
		_video_var_4++;
		_vga_buf_free_start = vpe->vgaFile1 + 0x5000;
	} else {
		_video_var_5 = 0;
	}
}

void SimonState::vga_buf_unk_proc1(byte *end) {
	VgaSprite *vsp;
	if (_lock_word & 0x20)
		return;

	for(vsp = _vga_sprites; vsp->id; vsp++) {
		vga_buf_unk_proc2(vsp->unk7, end);
		if (_video_var_5 == true)
			return;
	}
}

void SimonState::delete_memptr_range(byte *end) {
	uint count = ARRAYSIZE(_vga_buffer_pointers);
	VgaPointersEntry *vpe = _vga_buffer_pointers;
	do {
		if (_vga_buf_free_start <= vpe->vgaFile1 && end >= vpe->vgaFile1 ||
			_vga_buf_free_start <= vpe->vgaFile2 && end >= vpe->vgaFile2) {
				vpe->dd = 0;
				vpe->vgaFile1 = NULL;
				vpe->vgaFile2 = NULL;
		}

	} while (++vpe,--count);
}

void SimonState::vga_buf_unk_proc2(uint a, byte *end) {
	VgaPointersEntry *vpe;
	
	vpe = &_vga_buffer_pointers[a];

	if (_vga_buf_free_start <= vpe->vgaFile1 && end >= vpe->vgaFile1 ||
		  _vga_buf_free_start <= vpe->vgaFile2 && end >= vpe->vgaFile2) {
		_video_var_5 = true;
		_video_var_4++;
		_vga_buf_free_start = vpe->vgaFile1 + 0x5000;
	} else {
		_video_var_5 = false;
	}
}

void SimonState::o_unk_138() {
	_vga_buf_start = _vga_buf_free_start;
	_vga_file_buf_org = _vga_buf_free_start;
}

void SimonState::o_unk_186() {
	_vga_buf_free_start = _vga_file_buf_org_2;
	_vga_buf_start = _vga_file_buf_org_2;
	_vga_file_buf_org = _vga_file_buf_org_2;
}

void SimonState::o_unk_175() {
	_vga_buf_start = _vga_buf_free_start;
}

void SimonState::o_unk_176() {
	_vga_buf_free_start = _vga_file_buf_org;
	_vga_buf_start = _vga_file_buf_org;
}

void SimonState::o_clear_vgapointer_entry(uint a) {
	VgaPointersEntry *vpe;
	
	vpe = &_vga_buffer_pointers[a];

	vpe->dd = 0;
	vpe->vgaFile1 = NULL;
	vpe->vgaFile2 = NULL;
}

void SimonState::o_set_video_mode(uint mode, uint vga_res) {
	if (mode == 4)
		vc_29_stop_all_sounds();

	if (_lock_word & 0x10) {
		error("o_set_video_mode_ex: _lock_word & 0x10");
//		_unk21_word_array[a] = b; 
	} else {
		set_video_mode(mode,vga_res);
	}
}

void SimonState::set_video_mode_internal(uint mode, uint vga_res_id) {
	uint num;
	VgaPointersEntry *vpe;
	byte *bb,*b;
	uint16 c;
	byte *vc_ptr_org;

	warning("Set video mode internal: %d, %d", mode, vga_res_id);

	_video_palette_mode = mode;
	_lock_word |= 0x20;

	if (vga_res_id == 0) {

		if (!(_game & GAME_SIMON2)) {
			_unk_pal_flag = true;
		} else {
			_dx_use_3_or_4_for_lock = true;
			_vga_var6 = true;
		}
	}

	_vga_cur_file_2 = num = vga_res_id / 100;

	for(;;) {
		vpe = &_vga_buffer_pointers[num];

		_cur_vga_file_1 = vpe->vgaFile1;
		_cur_vga_file_2 = vpe->vgaFile2;

		if (vpe->vgaFile1 != NULL)
			break;

		ensureVgaResLoaded(num);
	}

	/* ensure flipping complete */

	bb = _cur_vga_file_1;
	b = bb + READ_BE_UINT16_UNALIGNED(&((VgaFile1Header*)bb)->hdr2_start);
	c = READ_BE_UINT16_UNALIGNED(&((VgaFile1Header2*)b)->unk1);
	b = bb + READ_BE_UINT16_UNALIGNED(&((VgaFile1Header2*)b)->unk2_offs);

	while (READ_BE_UINT16_UNALIGNED(&((VgaFile1Struct0x8*)b)->id) != vga_res_id)
		b += sizeof(VgaFile1Struct0x8);

	if (!(_game & GAME_SIMON2)) {
		if (num == 16300) {
			dx_clear_attached_from_top(134);
			_use_palette_delay = true;
		}
	} else {
		_x_scroll = 0;
		_vga_var1 = 0;
		_vga_var2 = 0;
		_vga_var3 = 0;
		_vga_var5 = 134;
		if(_variableArray[34] != -1)
			_variableArray[502/2] = 0;
	}

	vc_ptr_org = _vc_ptr;

	_vc_ptr = _cur_vga_file_1 + READ_BE_UINT16_UNALIGNED(&((VgaFile1Struct0x8*)b)->script_offs);
//	dump_vga_script(_vc_ptr, num, vga_res_id);
	run_vga_script();
	_vc_ptr = vc_ptr_org;


	if (_game & GAME_SIMON2) {
		if (!_dx_use_3_or_4_for_lock) {
			uint num_lines = 	_video_palette_mode==4 ? 134: 200;
			_vga_var8 = num_lines;
			dx_copy_from_attached_to_2(0, 0, 320, num_lines);
			dx_copy_from_attached_to_3(num_lines);
			_sync_flag_2 = 1;
		}
		_dx_use_3_or_4_for_lock = false;
	} else {
		uint num_lines = 	_video_palette_mode==4 ? 134: 200;
		dx_copy_from_attached_to_2(0, 0, 320, num_lines);
		dx_copy_from_attached_to_3(num_lines);
		_sync_flag_2 = 1;
		_timer_5 = 0;
	}
	
	_lock_word &= ~0x20;
	/* XXX: fix */


	if (!(_game & GAME_SIMON2)) {
		if (_unk_pal_flag) {
			_unk_pal_flag = false;
			while (*(volatile int*)&_palette_color_count!=0) {
				delay(10);
			}
		}
	}
}

void SimonState::set_video_mode(uint mode, uint vga_res_id) {
	
	if (_lock_counter == 0) {
		lock();
		if (_lock_word == 0) {
			_sync_flag_1 = true;
			while ((*(volatile bool*)&_sync_flag_1) == true) {
				delay(10);
			}
		}
	}

	_lock_word |= 0x20;

//	while ((*(volatile uint16*)&_lock_word) & 2) {
//		delay(10);
//	}

	unlock();

	set_video_mode_internal(mode, vga_res_id);
}



typedef void (SimonState::*VgaOpcodeProc)();

static const uint16 vc_get_out_of_code = 0;

void SimonState::run_vga_script() {
	static const VgaOpcodeProc vga_opcode_table[] = {
		NULL,
		&SimonState::vc_1,
		&SimonState::vc_2,
		&SimonState::vc_3,
		&SimonState::vc_4,
		&SimonState::vc_5,
		&SimonState::vc_6_maybe_skip_3_inv,
		&SimonState::vc_7_maybe_skip_3,
		&SimonState::vc_8_maybe_skip_2,
		&SimonState::vc_9_maybe_skip,
		&SimonState::vc_10,
		&SimonState::vc_11_clear_pathfind_array,
		&SimonState::vc_12_sleep_variable,
		&SimonState::vc_13_offset_x,
		&SimonState::vc_14_offset_y,
		&SimonState::vc_15_start_funkystruct_by_id,
		&SimonState::vc_16_setup_funkystruct,
		&SimonState::vc_17_set_pathfind_item,
		&SimonState::vc_18_jump_rel,
		&SimonState::vc_19,
		&SimonState::vc_20,
		&SimonState::vc_21,
		&SimonState::vc_22,
		&SimonState::vc_23_set_pri,
		&SimonState::vc_24_set_image_xy,
		&SimonState::vc_25_del_sprite_and_get_out,
		&SimonState::vc_26,
		&SimonState::vc_27_reset,
		&SimonState::vc_28,
		&SimonState::vc_29_stop_all_sounds,
		&SimonState::vc_30_set_base_delay,
		&SimonState::vc_31_set_palette_mode,
		&SimonState::vc_32_copy_var,
		&SimonState::vc_33,
		&SimonState::vc_34,
		&SimonState::vc_35,
		&SimonState::vc_36,
		&SimonState::vc_37_sprite_unk3_add,
		&SimonState::vc_38_skip_if_var_zero,
		&SimonState::vc_39_set_var,
		&SimonState::vc_40_var_add,
		&SimonState::vc_41_var_sub,
		&SimonState::vc_42_delay_if_not_eq,
		&SimonState::vc_43_skip_if_bit_clear,
		&SimonState::vc_44_skip_if_bit_set,
		&SimonState::vc_45_set_x,
		&SimonState::vc_46_set_y,
		&SimonState::vc_47_add_var_f,
		&SimonState::vc_48,
		&SimonState::vc_49_set_bit,
		&SimonState::vc_50_clear_bit,
		&SimonState::vc_51_clear_hitarea_bit_0x40,
		&SimonState::vc_52,
//#ifdef SIMON2
//		NULL,
//		NULL,
//#endif
//#ifdef SIMON1
		&SimonState::vc_53_no_op,
		&SimonState::vc_54_no_op,
//#endif
		&SimonState::vc_55_offset_hit_area,
		&SimonState::vc_56_no_op,
		&SimonState::vc_57_no_op,
//#ifdef SIMON2
		&SimonState::vc_58,
//#endif
//#ifdef SIMON1
//		NULL,
//#endif
		&SimonState::vc_59,
		&SimonState::vc_60,
		&SimonState::vc_61_sprite_change,
		&SimonState::vc_62,
		&SimonState::vc_63,

//#ifdef SIMON2
		&SimonState::vc_64,
		&SimonState::vc_65,
		&SimonState::vc_66,
		&SimonState::vc_67,
		&SimonState::vc_68,
		&SimonState::vc_69,
		&SimonState::vc_70,
		&SimonState::vc_71,
		&SimonState::vc_72,
		&SimonState::vc_73,
		&SimonState::vc_74,
//#endif
	};


	for(;;) {
		uint opcode;

#ifdef DUMP_CONTINOUS_VGASCRIPT
		if ((void*)_vc_ptr != (void*)&vc_get_out_of_code) {
//			if (_vga_cur_sprite_id==62 && _vga_cur_file_id==68 ||
//			    _vga_cur_sprite_id==1 && _vga_cur_file_id==2) {
				fprintf(_dump_file,"%.5d %.5X: %5d %4d ", _vga_tick_counter, _vc_ptr -_cur_vga_file_1, _vga_cur_sprite_id, _vga_cur_file_id);
				dump_video_script(_vc_ptr, true);
//			}
		}
#endif

		if (!(_game & GAME_SIMON2)) {
			opcode = READ_BE_UINT16_UNALIGNED(_vc_ptr);
			_vc_ptr += 2;
		} else {
			opcode = *_vc_ptr++;
		}

		if (opcode >= gss->NUM_VIDEO_OP_CODES)
			error("Invalid VGA opcode '%d' encountered", opcode);

		if (opcode == 0)
			return;

		(this->*vga_opcode_table[opcode])();
	}
}

int SimonState::vc_read_var_or_word() {
	int16 var = vc_read_next_word();
	if (var < 0)
		var = vc_read_var(-var);
	return var;
}

uint SimonState::vc_read_next_word() {
	uint a = READ_BE_UINT16_UNALIGNED(_vc_ptr);
	_vc_ptr += 2;
	return a;
}

uint SimonState::vc_read_next_byte() {
	return *_vc_ptr++;
}


void SimonState::vc_skip_next_instruction() {
	static const byte opcode_param_len_simon1[] = {
		0, 6, 2,10, 6, 4, 2, 2,
		4, 4,10, 0, 2, 2, 2, 2,
		2, 0, 2, 0, 4, 2, 4, 2,
		8, 0,10, 0, 8, 0, 2, 2,
		4, 0, 0, 4, 4, 2, 2, 4,
		4, 4, 4, 2, 2, 2, 2, 4,
		0, 2, 2, 2, 2, 4, 6, 6,
		0, 0, 0, 0, 2, 6, 0, 0,
	};

	static const byte opcode_param_len_simon2[] = {
		0, 6, 2, 12, 6, 4, 2, 2,
		4, 4, 9, 0, 1, 2, 2, 2,
		2, 0, 2, 0, 4, 2, 4, 2,
		7, 0, 10, 0, 8, 0, 2, 2,
		4, 0, 0, 4, 4, 2, 2, 4,
		4, 4, 4, 2, 2, 2, 2, 4,
		0, 2, 2, 2, 2, 4, 6, 6,
		2, 0, 6, 6, 4, 6, 0, 0,
		0, 0, 4, 4, 4, 4, 4, 0,
		4, 2, 2
	};

	if (_game & GAME_SIMON2) {
		uint opcode = vc_read_next_byte();
		_vc_ptr += opcode_param_len_simon2[opcode];
	} else {
		uint opcode = vc_read_next_word();
		_vc_ptr += opcode_param_len_simon1[opcode];
	}

#ifdef DUMP_CONTINOUS_VGASCRIPT
	fprintf(_dump_file,"; skipped\n");
#endif
}

void SimonState::vc_1() {
	/* dummy opcode */
	_vc_ptr += 6;
}

void SimonState::vc_2() {
	VgaPointersEntry *vpe;
	uint num;
	uint res;
	byte *old_file_1, *old_file_2;
	byte *b,*bb, *vc_ptr_org;

	num = vc_read_var_or_word();

	old_file_1 = _cur_vga_file_1;
	old_file_2 = _cur_vga_file_2;

	for(;;) {
		res = num / 100;
		vpe = &_vga_buffer_pointers[res];

		_cur_vga_file_1 = vpe->vgaFile1;
		_cur_vga_file_2 = vpe->vgaFile2;
		if (vpe->vgaFile1 != NULL)
			break;
		if (_vga_cur_file_2 != res)
			_video_var_7 = _vga_cur_file_2;

		ensureVgaResLoaded(res);
		_video_var_7 = 0xFFFF;
	}	

	
	bb = _cur_vga_file_1;
	b = bb + READ_BE_UINT16_UNALIGNED(&((VgaFile1Header*)bb)->hdr2_start);
	b = bb + READ_BE_UINT16_UNALIGNED(&((VgaFile1Header2*)b)->unk2_offs);

	while (READ_BE_UINT16_UNALIGNED(&((VgaFile1Struct0x8*)b)->id) != num)
		b += sizeof(VgaFile1Struct0x8);

	vc_ptr_org = _vc_ptr;

	_vc_ptr = _cur_vga_file_1 + READ_BE_UINT16_UNALIGNED(&((VgaFile1Struct0x8*)b)->script_offs);
	
	
//	dump_vga_script(_vc_ptr, res, num);
	run_vga_script();
	
	_cur_vga_file_1 = old_file_1;
	_cur_vga_file_2 = old_file_2;

	_vc_ptr = vc_ptr_org;
}

void SimonState::vc_3() {
	uint16 a,b,c,d,e,f;
	uint16 res;
	VgaSprite *vsp;
	VgaPointersEntry *vpe;
	byte *p,*pp;

	a = vc_read_next_word(); /* 0 */

	if (_game & GAME_SIMON2) {
		f = vc_read_next_word(); /* 0 */
		b = vc_read_next_word(); /* 2 */
	} else {
		b = vc_read_next_word(); /* 2 */
		f = b / 100;
	}

	c = vc_read_next_word(); /* 4 */
	d = vc_read_next_word(); /* 6 */
	e = vc_read_next_word(); /* 8 */

	/* 2nd param ignored with simon1 */
	if (has_vgastruct_with_id(b,f))
		return;

	vsp = _vga_sprites;
	while (vsp->id) vsp++;

	vsp->base_color = e;
	vsp->unk6 = a;
	vsp->unk5 = 0;
	vsp->unk4 = 0;
	vsp->image = 0;
	vsp->x = c;
	vsp->y = d;
	vsp->id = b;
	vsp->unk7 = res = f;

	for(;;) {
		vpe = &_vga_buffer_pointers[res];
		_cur_vga_file_1 = vpe->vgaFile1;

		if (vpe->vgaFile1 != NULL)
			break;
		if (res != _vga_cur_file_2)
			_video_var_7 = res;

		ensureVgaResLoaded(res);
		_video_var_7 = 0xFFFF;
	}

	pp = _cur_vga_file_1;
	p = pp + READ_BE_UINT16_UNALIGNED(&((VgaFile1Header*)pp)->hdr2_start);
	p = pp + READ_BE_UINT16_UNALIGNED(&((VgaFile1Header2*)p)->id_table);
	
	while (READ_BE_UINT16_UNALIGNED(&((VgaFile1Struct0x6*)p)->id) != b)
		p += sizeof(VgaFile1Struct0x6);

#ifdef DUMP_FILE_NR
{
	static bool dumped=false;
	if (res == DUMP_FILE_NR && !dumped) {
		dumped = true;
		dump_vga_file(_cur_vga_file_1);
	}
}
#endif

#ifdef DUMP_BITMAPS_FILE_NR
{
	static bool dumped=false;
	if (res == DUMP_BITMAPS_FILE_NR && !dumped) {
		dumped = true;
		dump_vga_bitmaps(_cur_vga_file_2, _cur_vga_file_1, res);
	}
}
#endif

	dump_vga_script(_cur_vga_file_1 + READ_BE_UINT16_UNALIGNED(&((VgaFile1Struct0x6*)p)->script_offs), res, b);

	add_vga_timer(gss->VGA_DELAY_BASE, 
		_cur_vga_file_1 + READ_BE_UINT16_UNALIGNED(&((VgaFile1Struct0x6*)p)->script_offs),b , res);
}

void SimonState::vc_4() {
	/* dummy opcode */
	_vc_ptr += 6;
}

void SimonState::vc_5() {
	uint var = vc_read_next_word();
	uint value = vc_read_next_word();
	if (vc_read_var(var) != value)
		vc_skip_next_instruction();
}

void SimonState::vc_6_maybe_skip_3_inv() {
	if (!vc_maybe_skip_proc_3(vc_read_next_word()))
		vc_skip_next_instruction();
}

void SimonState::vc_7_maybe_skip_3() {
	if (vc_maybe_skip_proc_3(vc_read_next_word()))
		vc_skip_next_instruction();
}

void SimonState::vc_8_maybe_skip_2() {
	uint a = vc_read_next_word();
	uint b = vc_read_next_word();
	if (!vc_maybe_skip_proc_2(a,b))
		vc_skip_next_instruction();
}

void SimonState::vc_9_maybe_skip() {
	uint a = vc_read_next_word();
	uint b = vc_read_next_word();
	if (!vc_maybe_skip_proc_1(a,b))
		vc_skip_next_instruction();
}

struct VC10_state {
	int image;
	uint16 e;
	int x,y;

	byte base_color;

	uint draw_width, draw_height;
	uint x_skip, y_skip;

	byte *surf2_addr;
	uint surf2_pitch;

	byte *surf_addr;
	uint surf_pitch;

	byte dl,dh;

	byte *depack_src;
	int8 depack_cont;

	byte depack_dest[200];
};

byte *vc_10_depack_column(VC10_state *vs) {
	int8 a = vs->depack_cont;
	byte *src = vs->depack_src;
	byte *dst = vs->depack_dest;
	byte dh = vs->dh;
	byte color;

	if (a != -0x80)
		goto start_here;

	for(;;) {
		a = *src++;
start_here:;
		if (a>=0) {
			color = *src++;
			do {
				*dst++ = color;
				if (!--dh) {
					if (--a<0)
						a = -0x80;
					else
						src--;
					goto get_out;
				}
			} while(--a>=0);
		} else {
			do {
				*dst++ = *src++;
				if (!--dh) {
					if (++a==0)
						a = -0x80;
					goto get_out;
				}
			} while (++a!=0);
		}
	}

get_out:;
	vs->depack_src = src;
	vs->depack_cont = a;
	return vs->depack_dest + vs->y_skip;
}

void vc_10_skip_cols(VC10_state *vs) {
	vs->depack_cont = -0x80;
	while(vs->x_skip) {
		vc_10_depack_column(vs);
		vs->x_skip--;
	}
}

byte *SimonState::vc_10_depack_swap(byte *src, uint w, uint h) {
	w<<=3;

	{
		byte *dst_org = _video_buf_1 + w;
		byte color;
		int8 cur = -0x80;
		uint w_cur = w;

		do {
			byte *dst = dst_org;
			uint h_cur = h;
			
			if (cur == -0x80)
				cur = *src++;
			
			for(;;) {
				if (cur >= 0) {
					/* rle_same */
					color = *src++;
					do {
						*dst = color;
						dst += w;
						if (!--h_cur) {
							if (--cur<0)
								cur = -0x80;
							else
								src--;
							goto next_line;
						}
					} while (--cur>=0);
				} else {
					/* rle_diff */
					do {
						*dst = *src++;
						dst += w;
						if (!--h_cur) {
							if (++cur == 0)
								cur = -0x80;
							goto next_line;
						}
					} while (++cur != 0);
				}
				cur = *src++;
			}
	next_line:
			dst_org++;
		} while(--w_cur);
	}

	{
		byte *dst_org, *src_org;
		uint i;
	
		src_org = dst_org = _video_buf_1 + w;

		do {
			byte *dst = dst_org;
			for(i=0; i!=w; ++i) {
				byte b = src_org[i];
				b = (b>>4) | (b<<4);
				*--dst = b;
			}

			src_org += w;
			dst_org += w;
		} while (--h);

	}

	return _video_buf_1;

}

byte *vc_10_no_depack_swap(byte *src) {
	error("vc_10_no_depack_swap unimpl");
}

/* must not be const */
static uint16 _video_windows[128] = {
	0, 0, 20, 200,
	0, 0,  3, 136,
	17,0,  3, 136,
	0, 0, 20, 200,
	0, 0, 20, 134
};

/* simon2 specific */
void SimonState::vc_10_helper_8(byte *dst, byte *src) {
	const uint pitch = _dx_surface_pitch;
	int8 reps = (int8)0x80;
	byte color;
	byte *dst_org = dst;
	uint h = _vga_var5, w = 8;
	
	for(;;) {
		reps = *src++;
		if (reps >= 0) {
			color = *src++;
			
			do {	
				*dst = color;
				dst += pitch;

				/* reached bottom? */
				if (--h == 0) {
					/* reached right edge? */				
					if (--w == 0)
						return;
					dst = ++dst_org;
					h = _vga_var5;
				}
			} while (--reps >=0);
		} else {
			
			do {	
				*dst = *src++;
				dst += pitch;

				/* reached bottom? */
				if (--h == 0) {
					/* reached right edge? */				
					if (--w == 0)
						return;
					dst = ++dst_org;
					h = _vga_var5;
				}
			} while (++reps != 0);
		}
	}
}

void SimonState::vc_10() {
	byte *p2;
	uint width,height;
	byte flags;
	const uint16 *vlut;
	VC10_state state;
	
	int cur;

	state.image = (int16)vc_read_next_word();
	if (state.image==0)
		return;

//	if (_vga_cur_sprite_id != 802)
//		return;

	state.base_color = (_vc_ptr[1]<<4);
	_vc_ptr += 2;
	state.x = (int16)vc_read_next_word();
	if (_game & GAME_SIMON2) {
		state.x -= _x_scroll;
	}
	state.y = (int16)vc_read_next_word();

	if (!(_game & GAME_SIMON2)) {
		state.e = vc_read_next_word();
	} else {
		state.e = vc_read_next_byte();
	}

	if (state.image < 0)
		state.image = vc_read_var(-state.image);

	p2 = _cur_vga_file_2 + state.image * 8;
	state.depack_src = _cur_vga_file_2 + READ_BE_UINT32_UNALIGNED(&*(uint32*)p2);

	width = READ_BE_UINT16_UNALIGNED(p2+6)>>4;
	height = p2[5];
	flags = p2[4];

	if (height==0 || width==0)
		return;

#ifdef DUMP_DRAWN_BITMAPS
	dump_single_bitmap(_vga_cur_file_id, state.image, state.depack_src, width*16, height, state.base_color);
#endif

	if (flags&0x80 && !(state.e&0x10)) {
		if (state.e&1) {
			state.e&=~1;
			state.e|=0x10;
		} else {
			state.e|=0x8;
		}
	}

	if (_game & GAME_SIMON2 && width>=21) {
		byte *src,*dst;
		uint w;

		_vga_var1 = width*2-40;
		_vga_var7 = state.depack_src;
		_vga_var5 = height;
		if (_variableArray[34]==-1)
			state.x = _variableArray[502/2];

		_x_scroll = state.x;

		vc_write_var(0xfb, _x_scroll);

		dst = dx_lock_attached();
		src = state.depack_src + _x_scroll * 4;
		
		w = 40;
		do {
			vc_10_helper_8(dst, src + READ_BE_UINT32_UNALIGNED(&*(uint32*)src));
			dst += 8;
			src += 4;
		} while (--w);
		
		dx_unlock_attached();

	
		return;	
	}

	if (state.e&0x10)
		state.depack_src = vc_10_depack_swap(state.depack_src, width, height);
	else if (state.e&1)
		state.depack_src = vc_10_no_depack_swap(state.depack_src);


	vlut = &_video_windows[_video_palette_mode * 4];

	state.draw_width = width << 1; /* cl */
	state.draw_height = height;    /* ch */

	state.x_skip = 0; /* colums to skip = bh */
	state.y_skip = 0; /* rows to skip   = bl */
	
	cur = state.x;
	if (cur < 0) {
		do {
			if (!--state.draw_width) return;
			state.x_skip++;
		} while(++cur);
	}
	state.x = cur;

	cur += state.draw_width - (vlut[2]<<1);
	if (cur > 0) {
		do {
			if (!--state.draw_width) return;
		} while (--cur);
	}
	
	cur = state.y;
	if (cur < 0) {
		do {
			if (!--state.draw_height) return;
			state.y_skip++;
		} while(++cur);
	}
	state.y = cur;

	cur += state.draw_height - vlut[3];
	if (cur > 0) {
		do {
			if (!--state.draw_height) return;
		} while (--cur);
	}

	assert(state.draw_width!=0 && state.draw_height!=0);

	state.draw_width<<=2;

	state.surf2_addr = dx_lock_2();
	state.surf2_pitch = _dx_surface_pitch;

	state.surf_addr = dx_lock_attached();
	state.surf_pitch = _dx_surface_pitch;

	{
		uint offs = ((vlut[0] - _video_windows[16])*2 + state.x) * 8;
		uint offs2 = (vlut[1] - _video_windows[17] + state.y);

		state.surf2_addr += offs + offs2 * state.surf2_pitch;
		state.surf_addr += offs + offs2 * state.surf_pitch;
	}

	if (state.e & 0x20) {
		byte *mask, *src, *dst;
		byte h;
		uint w;

		state.x_skip<<=2;
		state.dl = width;
		state.dh = height;
		
		vc_10_skip_cols(&state);
		
		/* XXX: implement transparency */

		w = 0;
		do {
			mask = vc_10_depack_column(&state); /* esi */
			src = state.surf2_addr + w*2; /* ebx */
			dst = state.surf_addr + w*2;  /* edi */

			h = state.draw_height;
			do {
				if (mask[0] & 0xF0) dst[0] = src[0];
				if (mask[0] & 0x0F) dst[1] = src[1];
				mask++;
				dst += state.surf_pitch;
				src += state.surf2_pitch;
			} while(--h);
		} while(++w != state.draw_width);

		/* vc_10_helper_5 */
	} else if (_lock_word&0x20 && state.base_color==0 || state.base_color==0xC0) {
		byte *src,*dst;
		uint h,i;
	
		if (!(state.e&8)) {
			src = state.depack_src + (width * state.y_skip<<4) + (state.x_skip<<3);
			dst = state.surf_addr;

			state.draw_width *= 2;
			
			if(state.e&2) {
				/* no transparency */
				h = state.draw_height;
				do {
					memcpy(dst,src,state.draw_width);
					dst += 320;
					src += width * 16;
				} while(--h);
			} else {
				/* transparency */
				h = state.draw_height;
				do {
					for(i=0; i!=state.draw_width; i++)
						if(src[i])
							dst[i] = src[i];
					dst += 320;
					src += width * 16;
				} while(--h);
			}

		} else {
			byte *dst_org = state.surf_addr;
			src = state.depack_src;
			/* AAAAAAAA BBBBBBBB CCCCCCCC DDDDDDDD EEEEEEEE
			 * aaaaabbb bbcccccd ddddeeee efffffgg ggghhhhh
			 */
			
			if (state.e & 2) {
				/* no transparency */
				do {
					uint count = state.draw_width>>2;
					
					dst = dst_org;
					do {
						uint32 bits = (src[0]<<24) | (src[1]<<16) | (src[2]<<8) | (src[3]);
						
						dst[0] = (byte)((bits >> (32-5)) & 31);
						dst[1] = (byte)((bits >> (32-10)) & 31);
						dst[2] = (byte)((bits >> (32-15)) & 31);
						dst[3] = (byte)((bits >> (32-20)) & 31);
						dst[4] = (byte)((bits >> (32-25)) & 31);
						dst[5] = (byte)((bits >> (32-30)) & 31);

						bits = (bits<<8) | src[4];

						dst[6] = (byte)((bits >> (40-35)) & 31);
						dst[7] = (byte)((bits) & 31);

						dst += 8;
						src += 5;
					} while (--count);
					dst_org += 320;
				} while (--state.draw_height);
			} else {
				/* transparency */
				do {
					uint count = state.draw_width>>2;
					
					dst = dst_org;
					do {
						uint32 bits = (src[0]<<24) | (src[1]<<16) | (src[2]<<8) | (src[3]);
						byte tmp;
						
						tmp = (byte)((bits >> (32-5)) & 31); if (tmp) dst[0] = tmp;
						tmp = (byte)((bits >> (32-10)) & 31); if (tmp) dst[1] = tmp;
						tmp = (byte)((bits >> (32-15)) & 31); if (tmp) dst[2] = tmp;
						tmp = (byte)((bits >> (32-20)) & 31); if (tmp) dst[3] = tmp;
						tmp = (byte)((bits >> (32-25)) & 31); if (tmp) dst[4] = tmp;
						tmp = (byte)((bits >> (32-30)) & 31); if (tmp) dst[5] = tmp;

						bits = (bits<<8) | src[4];

						tmp = (byte)((bits >> (40-35)) & 31); if (tmp) dst[6] = tmp;
						tmp = (byte)((bits) & 31); if (tmp) dst[7] = tmp;

						dst += 8;
						src += 5;
						} while (--count);
					dst_org += 320;
				} while (--state.draw_height);
			}
		}
		/* vc_10_helper_4 */
	} else {
		if (_game&GAME_SIMON2 && state.e&0x4 && _bit_array[10]&0x800) {
			state.surf_addr = state.surf2_addr;
			state.surf_pitch = state.surf2_pitch;
			warning("vc_10: (state.e&0x4)");
		}	
		
		if (state.e & 0x8) {
			uint w,h;
			byte *src, *dst,*dst_org;

			state.x_skip <<= 2; /* reached */
			state.dl = width;
			state.dh = height;

			vc_10_skip_cols(&state);

			if (state.e&2) {
				dst_org = state.surf_addr;
				w = 0;
				do {
					src = vc_10_depack_column(&state);
					dst = dst_org;
					
					h = 0;
					do {
						dst[0] = (*src >> 4) | state.base_color;
						dst[1] = (*src&15) | state.base_color;
						dst += 320;
						src++;
					} while (++h != state.draw_height);
					dst_org += 2;
				} while (++w != state.draw_width);
			} else {
				dst_org = state.surf_addr;
				if (state.e & 0x40) { /* reached */
					dst_org += vc_read_var(252);
				}
				w = 0;
				do {
					byte color;

					src = vc_10_depack_column(&state);
					dst = dst_org;
					
					h = 0;
					do {
						color = (*src >> 4);
						if (color) dst[0] = color | state.base_color;
						color = (*src&15);
						if (color) dst[1] = color | state.base_color;
						dst += 320;
						src++;
					} while (++h != state.draw_height);
					dst_org += 2;
				} while (++w != state.draw_width);			
			}
			/* vc_10_helper_6 */
		} else {
			byte *src,*dst;
			uint count;

			src = state.depack_src + (width * state.y_skip) * 8;
			dst = state.surf_addr;
			state.x_skip <<= 2;
			if (state.e&2) {
				do {
					for(count=0; count!=state.draw_width; count++) {
						dst[count*2] = (src[count+state.x_skip]>>4) | state.base_color;
						dst[count*2+1] = (src[count+state.x_skip]&15) | state.base_color;
					}
					dst += 320;
					src += width * 8;
				} while (--state.draw_height);
			} else {
				do {
					for(count=0; count!=state.draw_width; count++) {
						byte color;
						color = (src[count+state.x_skip]>>4);
						if(color) dst[count*2] = color | state.base_color;
						color = (src[count+state.x_skip]&15);
						if (color) dst[count*2+1] = color | state.base_color;
					}
					dst += 320;
					src += width * 8;
				} while (--state.draw_height);

			}

			/* vc_10_helper_7 */
		}
	}

	dx_unlock_2();
	dx_unlock_attached();

}

void SimonState::vc_11_clear_pathfind_array() {
	memset(&_pathfind_array, 0, sizeof(_pathfind_array));
}

void SimonState::vc_12_sleep_variable() {
	uint num;

	if (!(_game & GAME_SIMON2)) {
		num = vc_read_var_or_word();
	} else {
		num = vc_read_next_byte() * _vga_base_delay;
	}

	add_vga_timer(num + gss->VGA_DELAY_BASE, _vc_ptr, _vga_cur_sprite_id, _vga_cur_file_id);
	_vc_ptr = (byte*)&vc_get_out_of_code;
}

void SimonState::vc_13_offset_x() {
	VgaSprite *vsp = find_cur_sprite();
	int16 a = vc_read_next_word();
	vsp->x += a;
	_vga_sprite_changed++;
}

void SimonState::vc_14_offset_y() {
	VgaSprite *vsp = find_cur_sprite();
	int16 a = vc_read_next_word();
	vsp->y += a;
	_vga_sprite_changed++;
}

/* wakeup_id */
void SimonState::vc_15_start_funkystruct_by_id() {
	VgaSleepStruct *vfs = _vga_sleep_structs, *vfs_tmp;
	uint16 id = vc_read_next_word();
	while (vfs->ident != 0) {
		if (vfs->ident == id) {
			add_vga_timer(gss->VGA_DELAY_BASE, vfs->code_ptr, vfs->sprite_id, vfs->cur_vga_file);
			vfs_tmp = vfs;
			do {
				memcpy(vfs_tmp, vfs_tmp + 1, sizeof(VgaSleepStruct));
				vfs_tmp++;
			} while (vfs_tmp->ident != 0);
		} else {
			vfs++;
		}
	}

	/* clear a wait event */
	if (id == _vga_wait_for)
		_vga_wait_for = 0;
}


/* sleep_on_id */
void SimonState::vc_16_setup_funkystruct() {
	VgaSleepStruct *vfs = _vga_sleep_structs;
	while (vfs->ident)
		vfs++;

	vfs->ident = vc_read_next_word();
	vfs->code_ptr = _vc_ptr;
	vfs->sprite_id = _vga_cur_sprite_id;
	vfs->cur_vga_file = _vga_cur_file_id;
	
	_vc_ptr = (byte*)&vc_get_out_of_code;
}

void SimonState::vc_17_set_pathfind_item() {
	uint a = vc_read_next_word();
	_pathfind_array[a - 1] = (uint16*)_vc_ptr;
	while ( READ_BE_UINT16_UNALIGNED(_vc_ptr) != 999)
		_vc_ptr += 4;
	_vc_ptr += 2;
}

void SimonState::vc_18_jump_rel() {
	int16 offs = vc_read_next_word();
	_vc_ptr += offs;
}

/* chain to script? */
void SimonState::vc_19() {
 /* XXX: not implemented */
	error("vc_19: chain to script not implemented");
}


/* helper routines */

/* write unaligned 16-bit */
static void write_16_le(void *p, uint16 a) {
	((byte*)p)[0] = (byte) (a);
	((byte*)p)[1] = (byte) (a >> 8);
}

/* read unaligned 16-bit */
static uint16 read_16_le(void *p) {
	return ((byte*)p)[0] | (((byte*)p)[1] << 8);
}

/* FIXME: unaligned access */
void SimonState::vc_20() {
	uint16 a = vc_read_next_word();
	write_16_le(_vc_ptr, a);
	_vc_ptr += 2;
}

/* FIXME: unaligned access */
void SimonState::vc_21() {
	if (!(_game & GAME_SIMON2)) {
		int16 a = vc_read_next_word();
		byte *tmp = _vc_ptr + a;
		uint16 val = read_16_le(tmp + 4);

		if (val != 0) {
			write_16_le(tmp + 4, val - 1);
			_vc_ptr = tmp + 6;
		}
	} else {
		int16 a = vc_read_next_word();
		byte *tmp = _vc_ptr + a;
		uint16 val = read_16_le(tmp + 3);

		if (val != 0) {
			write_16_le(tmp + 3, val - 1);
			_vc_ptr = tmp + 5;
		}
	}
}

void SimonState::vc_22() {
	uint a = vc_read_next_word();
	uint b = vc_read_next_word();
	uint num = a==0 ? 0x20 : 0x10;
	byte *palptr, *src;
	
	palptr = &_palette[(a<<6)];

	src = _cur_vga_file_1 + 6 + b*96;
	
	do {
		palptr[0] = src[0]<<2;
		palptr[1] = src[1]<<2;
		palptr[2] = src[2]<<2;
		palptr[3] = 0;

		palptr += 4;
		src += 3;
	} while (--num);

	_video_var_9 = 2;
	_vga_sprite_changed++;
}

void SimonState::vc_23_set_pri() {
	VgaSprite *vsp = find_cur_sprite(), *vus2;
	uint16 pri = vc_read_next_word();
	VgaSprite bak;

	if (vsp->id == 0) {
		warning("tried to set pri for unknown id %d", _vga_cur_sprite_id);
		return;
	}
	
	memcpy(&bak, vsp, sizeof(bak));
	bak.unk5 = pri;
	bak.unk6 |= 0x8000;

	vus2 = vsp;

	if (vsp != _vga_sprites && pri < vsp[-1].unk5) {
		do {
			vsp--;
		} while (vsp != _vga_sprites && pri < vsp[-1].unk5);
		do {
			memcpy(vus2, vus2-1, sizeof(VgaSprite));	
		} while (--vus2 != vsp);
		memcpy(vus2, &bak, sizeof(VgaSprite));
	} else if (vsp[1].id!=0 && pri >= vsp[1].unk5) {
		do {
			vsp++;
		} while (vsp[1].id!=0 && pri >= vsp[1].unk5);
		do {
			memcpy(vus2, vus2+1,sizeof(VgaSprite));
		} while (++vus2 != vsp);
		memcpy(vus2, &bak, sizeof(VgaSprite));
	} else {
		vsp->unk5 = pri;
	}
	_vga_sprite_changed++;
}

void SimonState::vc_24_set_image_xy() {
	VgaSprite *vsp = find_cur_sprite();
	vsp->image = vc_read_var_or_word();

	if (vsp->id==0) {
		warning("Trying to set XY of nonexistent sprite '%d'", _vga_cur_sprite_id);
	}

	vsp->x += (int16)vc_read_next_word();
	vsp->y += (int16)vc_read_next_word();
	if (!(_game & GAME_SIMON2)) {
		vsp->unk4 = vc_read_next_word();
	} else {
		vsp->unk4 = vc_read_next_byte();
	}

	_vga_sprite_changed++;
}

void SimonState::vc_25_del_sprite_and_get_out() {
	VgaSprite *vsp = find_cur_sprite();
	while (vsp->id != 0) {
		memcpy(vsp,vsp+1,sizeof(VgaSprite));
		vsp++;
	}
	_vc_ptr = (byte*)&vc_get_out_of_code;
	_vga_sprite_changed++;
}

void SimonState::vc_26() {
	uint16 *as = &_video_windows[vc_read_next_word()*4];
	as[0] = vc_read_next_word();
	as[1] = vc_read_next_word();
	as[2] = vc_read_next_word();
	as[3] = vc_read_next_word();
}

void SimonState::vc_27_reset_simon1() {
	VgaSprite bak,*vsp;
	VgaSleepStruct *vfs;
	VgaTimerEntry *vte,*vte2;

	_lock_word |= 8;

	memset(&bak,0,sizeof(bak));

	vsp = _vga_sprites;
	while (vsp->id) {
		if (vsp->id == 128) {
			memcpy(&bak,vsp,sizeof(VgaSprite));
		}
		vsp->id = 0;
		vsp++;
	}

	if (bak.id != 0)
		memcpy(_vga_sprites, &bak, sizeof(VgaSprite));

	vfs = _vga_sleep_structs;
	while (vfs->ident) {
		vfs->ident = 0;
		vfs++;
	}

	
	vte = _vga_timer_list;
	while (vte->delay) {
		if (vte->sprite_id != 0x80) {
			vte2 = vte;
			while (vte2->delay) {
				memcpy(vte2,vte2+1,sizeof(VgaTimerEntry));
				vte2++;
			}
		} else {
			vte++;
		}
	}

	vc_write_var(0xFE, 0);
	
	_lock_word &= ~8;
}


void SimonState::vc_27_reset_simon2() {
	_lock_word |= 8;
	
	{
		VgaSprite *vsp = _vga_sprites;
		while (vsp->id) {
			vsp->id = 0;
			vsp++;
		}
	}

	{
		VgaSleepStruct *vfs = _vga_sleep_structs;
		while (vfs->ident) {
			vfs->ident = 0;
			vfs++;
		}
	}

	{	
		VgaTimerEntry *vte = _vga_timer_list;
		while (vte->delay) {
			VgaTimerEntry *vte2 = vte;
			while (vte2->delay) {
				memcpy(vte2,vte2+1,sizeof(VgaTimerEntry));
				vte2++;
			}
		}
	}

	vc_write_var(0xFE, 0);
	
	_lock_word &= ~8;
}

void SimonState::vc_27_reset() {
	if (!(_game & GAME_SIMON2))
		vc_27_reset_simon1();
	else
		vc_27_reset_simon2();
}

void SimonState::vc_28() {
	/* dummy opcode */
	_vc_ptr += 8;
}

void SimonState::vc_29_stop_all_sounds() {
	/* XXX: implement */
//	warning("vc_29_stop_all_sounds unimplemented");
	
	_mixer->stop_all();
}

void SimonState::vc_30_set_base_delay() {
	_vga_base_delay = vc_read_next_word();
}

void SimonState::vc_31_set_palette_mode() {
	_video_palette_mode = vc_read_next_word();
}

uint SimonState::vc_read_var(uint var) {
	assert(var<255);
	return (uint16)_variableArray[var];
}

void SimonState::vc_write_var(uint var, int16 value) {
	_variableArray[var] = value;
}

void SimonState::vc_32_copy_var() {
	uint16 a = vc_read_var(vc_read_next_word());
	vc_write_var(vc_read_next_word(), a);
}

void SimonState::vc_33() {
	if (_lock_counter != 0) {
		_lock_counter = 1;
		unlock();
	}
}

void SimonState::vc_34() {
	lock();
	_lock_counter = 200;
	_left_button_down = 0;
}

void SimonState::vc_35() {
	/* not used? */
	_vc_ptr += 4;
	_vga_sprite_changed++;
}

void SimonState::vc_36() {
	uint vga_res = vc_read_next_word();
	uint mode = vc_read_next_word();

	if (!(_game & GAME_SIMON2)) {
		if (mode == 16) {
			_copy_partial_mode = 2;
		} else {
			set_video_mode_internal(mode,vga_res);
		}
	} else {
		set_video_mode_internal(mode,vga_res);
	}
}

void SimonState::vc_37_sprite_unk3_add() {
	VgaSprite *vsp = find_cur_sprite();
	vsp->y += vc_read_var(vc_read_next_word());
	_vga_sprite_changed++; 
}

void SimonState::vc_38_skip_if_var_zero() {
	uint var = vc_read_next_word();
	if (vc_read_var(var) == 0)
		vc_skip_next_instruction();
}

void SimonState::vc_39_set_var() {
	uint var = vc_read_next_word();
	int16 value = vc_read_next_word();
	vc_write_var(var,value);
}

void SimonState::vc_40_var_add() {
	uint var = vc_read_next_word();
	int16 value = vc_read_var(var) + vc_read_next_word();

	if (_game&GAME_SIMON2 && var==0xF && !(_bit_array[5]&1)) {
		int16 tmp;

		if (_vga_var2!=0) {
			if (_vga_var2>=0) goto no_scroll;
			_vga_var2 = 0;
		} else {
			if (_vga_var3 != 0) goto no_scroll;
		}

		if (value - _x_scroll >= 30) {
			_vga_var2 = 20;
			tmp = _vga_var1 - _x_scroll;
			if (tmp < 20)
				_vga_var2 = tmp;
			add_vga_timer(10, NULL, 0, 0); /* special timer */
		}
	}
no_scroll:;

	vc_write_var(var, value);
}

void SimonState::vc_41_var_sub() {
	uint var = vc_read_next_word();
	int16 value = vc_read_var(var) - vc_read_next_word();

	if (_game&GAME_SIMON2 && var==0xF && !(_bit_array[5]&1)) {
		int16 tmp;

		if (_vga_var2!=0) {
			if (_vga_var2<0) goto no_scroll;
			_vga_var2 = 0;
		} else {
			if (_vga_var3 != 0) goto no_scroll;
		}

		if ((uint16)(value - _x_scroll) < 11) {
			_vga_var2 = -20;
			tmp = _vga_var1 - _x_scroll;
			if (_x_scroll < 20)
				_vga_var2 = -_x_scroll;
			add_vga_timer(10, NULL, 0, 0); /* special timer */
		}
	}
no_scroll:;

	vc_write_var(var, value);
}

void SimonState::vc_42_delay_if_not_eq() {
	uint val = vc_read_var(vc_read_next_word());
	if (val == vc_read_next_word()) {
		
		add_vga_timer(_vga_base_delay + 1, _vc_ptr - 4, _vga_cur_sprite_id, _vga_cur_file_id);
		_vc_ptr = (byte*)&vc_get_out_of_code;
	}
}

void SimonState::vc_43_skip_if_bit_clear() {
	if (!vc_get_bit(vc_read_next_word())) {
		vc_skip_next_instruction();
	}
}

void SimonState::vc_44_skip_if_bit_set() {
	if (vc_get_bit(vc_read_next_word())) {
		vc_skip_next_instruction();
	}
}

void SimonState::vc_45_set_x() {
	VgaSprite *vsp = find_cur_sprite();
	vsp->x = vc_read_var(vc_read_next_word());
	_vga_sprite_changed++;
}

void SimonState::vc_46_set_y() {
	VgaSprite *vsp = find_cur_sprite();
	vsp->y = vc_read_var(vc_read_next_word());
	_vga_sprite_changed++;
}

void SimonState::vc_47_add_var_f() {
	uint var = vc_read_next_word();
	vc_write_var(var, vc_read_var(var) + vc_read_var(vc_read_next_word()));
}

void SimonState::vc_48() {
	uint a = (uint16)_variableArray[12];
	uint b = (uint16)_variableArray[13];
	int c = _variableArray[14];
	uint16 *p = _pathfind_array[a-1];
	int step;
	int y1,y2;
	int16 *vp;

	p += b*2 + 1;

	step = 2;
	if (c<0) {
		c = -c;
		step = -2;
	}

	vp = &_variableArray[20];

	do{
		y2 = READ_BE_UINT16_UNALIGNED(p);
		p += step;
		y1 = READ_BE_UINT16_UNALIGNED(p) - y2;

//		assert(READ_BE_UINT16_UNALIGNED(&p[1]) != 999);

		vp[0] = y1>>1;
		vp[1] = y1 - (y1>>1);

		vp += 2;
	} while (--c);

}

void SimonState::vc_set_bit_to(uint bit, bool value) {
	uint16 *bits = &_bit_array[bit>>4];
	*bits = (*bits & ~(1<<(bit&15))) | (value << (bit&15));
}

bool SimonState::vc_get_bit(uint bit) {
	uint16 *bits = &_bit_array[bit>>4];
	return (*bits & (1<<(bit&15))) != 0;
}

void SimonState::vc_49_set_bit() {
	vc_set_bit_to(vc_read_next_word(), true);
}

void SimonState::vc_50_clear_bit() {
	vc_set_bit_to(vc_read_next_word(), false);
}

void SimonState::vc_51_clear_hitarea_bit_0x40() {
	clear_hitarea_bit_0x40(vc_read_next_word());
}

void SimonState::vc_52() {
	uint16 a = vc_read_next_word();

	if (!(_game & GAME_SIMON2)) {
		playSound(a);
	} else {
		if (a >= 0x8000) {
			a = -a;
			warning("vc_52(%d): unimpl");		
		} else {
			playSound(a);
		}
	}
}

void SimonState::vc_53_no_op() {
	/* no op */
}

void SimonState::vc_54_no_op() {
	/* no op */
}

void SimonState::vc_55_offset_hit_area() {
	HitArea *ha = _hit_areas;
	uint count = ARRAYSIZE(_hit_areas);
	uint16 id = vc_read_next_word();
	int16 x = vc_read_next_word();
	int16 y = vc_read_next_word();

	for(;;) {
		if (ha->id == id) {
			ha->x += x;
			ha->y += y;
			break;
		}
		ha++;
		if (!--count)
			break;
	}
	
	_need_hitarea_recalc++;
}

void SimonState::vc_56_no_op() {
	/* No-Op in simon1 */
	if (_game & GAME_SIMON2) {
		uint num = vc_read_var_or_word() * _vga_base_delay;

#ifdef DUMP_CONTINOUS_VGASCRIPT
		fprintf(_dump_file,"; sleep_ex = %d\n", num + gss->VGA_DELAY_BASE);
#endif

		add_vga_timer(num + gss->VGA_DELAY_BASE, _vc_ptr, _vga_cur_sprite_id, _vga_cur_file_id);
		_vc_ptr = (byte*)&vc_get_out_of_code;
	}
}

void SimonState::vc_59() {
	if (_game & GAME_SIMON2) {
		uint file = vc_read_next_word();
		uint start = vc_read_next_word();
		uint end = vc_read_next_word() + 1;
		
		do {
			vc_kill_thread(file, start);
		} while (++start != end);
	} else {
		if (vc_59_helper())
			vc_skip_next_instruction();
	}
}

void SimonState::vc_58() {
	uint sprite = _vga_cur_sprite_id;
	uint file = _vga_cur_file_id;
	byte *vc_ptr;
	uint16 tmp;
		
	_vga_cur_file_id = vc_read_next_word();
	_vga_cur_sprite_id = vc_read_next_word();

	tmp = TO_BE_16(vc_read_next_word());
	
	vc_ptr = _vc_ptr;
	_vc_ptr = (byte*)&tmp;
	vc_23_set_pri();

	_vc_ptr = vc_ptr;
	_vga_cur_sprite_id = sprite;
	_vga_cur_file_id = file;
}

void SimonState::vc_57_no_op() {
	/* no op */
	
}

void SimonState::vc_kill_thread(uint file, uint sprite) {
	uint16 old_sprite_id, old_cur_file_id;
	VgaSleepStruct *vfs;
	VgaSprite *vsp;
	VgaTimerEntry *vte;
	byte *vc_org;

	old_sprite_id = _vga_cur_sprite_id;
	old_cur_file_id = _vga_cur_file_id;
	vc_org = _vc_ptr;
	
	_vga_cur_file_id = file;
	_vga_cur_sprite_id = sprite;
		
	vfs = _vga_sleep_structs;
	while (vfs->ident != 0) {
		if (vfs->sprite_id == _vga_cur_sprite_id
			 && (vfs->cur_vga_file == _vga_cur_file_id || !(_game & GAME_SIMON2))
		) {
			while (vfs->ident != 0){
				memcpy(vfs, vfs+1, sizeof(VgaSleepStruct));
				vfs++;
			}
			break;
		}
		vfs++;
	}

	vsp = find_cur_sprite();	
	if (vsp->id) {
		vc_25_del_sprite_and_get_out();

		vte = _vga_timer_list;
		while (vte->delay != 0) {
			if (vte->sprite_id == _vga_cur_sprite_id
			 && (vte->cur_vga_file == _vga_cur_file_id || !(_game & GAME_SIMON2) )
			) {
				delete_vga_timer(vte);
				break;
			}
			vte++;
		}
	}

	_vga_cur_file_id = old_cur_file_id;
	_vga_cur_sprite_id = old_sprite_id;
	_vc_ptr = vc_org;
}


/* kill thread */
void SimonState::vc_60() {
	uint file;

	if (_game & GAME_SIMON2) {
		file = vc_read_next_word();
	} else {
		file = _vga_cur_file_id;
	}
	uint sprite = vc_read_next_word();
	vc_kill_thread(file, sprite);
}

void SimonState::vc_61_sprite_change() {
	VgaSprite *vsp = find_cur_sprite();	

	vsp->image = vc_read_var_or_word();

	vsp->x += vc_read_next_word();
	vsp->y += vc_read_next_word();
	vsp->unk4 = 36;

	_vga_sprite_changed++;
}

void SimonState::vc_62() {
	uint i;
	byte *vc_ptr_org = _vc_ptr;


	vc_29_stop_all_sounds();
	
//	if (!_video_var_3) {
		_video_var_3 = true;
		_video_num_pal_colors = 256;
		if (_video_palette_mode == 4)
			_video_num_pal_colors = 208;
//	}

	memcpy(_video_buf_1, _palette_backup, _video_num_pal_colors * sizeof(uint32));
	for(i=NUM_PALETTE_FADEOUT;i!=0;--i) {
		palette_fadeout((uint32*)_video_buf_1, _video_num_pal_colors);
		_system->set_palette(_video_buf_1, 0, _video_num_pal_colors);
		_system->update_screen();
		delay(5);
	}

	if (!(_game & GAME_SIMON2))	{
		uint16 params[5]; /* parameters to vc_10 */
		VgaSprite *vsp;
		VgaPointersEntry *vpe;

		vsp = _vga_sprites;
		while (vsp->id != 0) {
			if (vsp->id == 128) {
				byte *f1 = _cur_vga_file_1;
				byte *f2 = _cur_vga_file_2;
				uint palmode = _video_palette_mode;

				vpe = &_vga_buffer_pointers[vsp->unk7];
				_cur_vga_file_1 = vpe->vgaFile1;
				_cur_vga_file_2 = vpe->vgaFile2;
				_video_palette_mode = vsp->unk6;

				params[0] = READ_BE_UINT16_UNALIGNED(&vsp->image);
				params[1] = READ_BE_UINT16_UNALIGNED(&vsp->base_color);
				params[2] = READ_BE_UINT16_UNALIGNED(&vsp->x);
				params[3] = READ_BE_UINT16_UNALIGNED(&vsp->y);
				params[4] = READ_BE_UINT16_UNALIGNED(&vsp->unk4);
				_vc_ptr = (byte*)params;
				vc_10();

				_video_palette_mode = palmode;
				_cur_vga_file_1 = f1;
				_cur_vga_file_2 = f2;
				break;
			}
			vsp++;
		}
	}

	dx_clear_surfaces(_video_palette_mode==4 ? 134 : 200);

	_vc_ptr = vc_ptr_org;
}

void SimonState::vc_63() {
	_palette_color_count = 208;
	if(_video_palette_mode != 4) {
		_palette_color_count = 256;
	}
	_video_var_3 = false;
}

/* Simon2 specific */
void SimonState::vc_64() {
	if (vc_59_helper())
		vc_skip_next_instruction();

}

/* Simon2 specific */
void SimonState::vc_65() {
	error("vc_65 unimplemented");
}

/* Simon2 specific */
void SimonState::vc_66() {
	uint a = vc_read_next_word();
	uint b = vc_read_next_word();

	if (vc_read_var(a) != vc_read_var(b))
		vc_skip_next_instruction();
}

/* Simon2 specific */
void SimonState::vc_67() {
	uint a = vc_read_next_word();
	uint b = vc_read_next_word();

	if (vc_read_var(a) >= vc_read_var(b))
		vc_skip_next_instruction();
}

/* Simon2 specific */
void SimonState::vc_68() {
	uint a = vc_read_next_word();
	uint b = vc_read_next_word();

	if (vc_read_var(a) <= vc_read_var(b))
		vc_skip_next_instruction();
}

/* Simon2 specific */
void SimonState::vc_69() {
	uint16 a = vc_read_next_word();
	uint16 b = vc_read_next_word();

	warning("vc_69(%d,%d): music stuff?", a, b);
}

/* Simon2 specific */
void SimonState::vc_70() {
	uint16 a = vc_read_next_word();
	uint16 b = vc_read_next_word();

	_vc70_var1 = a;
	_vc70_var2 = b;
	
	warning("vc_70(%d,%d): music stuff?", a, b);
}

/* Simon2 specific */
void SimonState::vc_71() {
	if (_vc72_var3==0xFFFF && _vc72_var1==0xFFFF)
		vc_skip_next_instruction();
}

/* Simon2 specific */
void SimonState::vc_72() {
	uint16 a = vc_read_next_word();
	uint16 b = vc_read_next_word();
	if (a != _vc72_var1) {
		_vc72_var2 = b;
		_vc72_var3 = a;
	}

	warning("vc_72(%d,%d): music stuff?", a, b);
}

/* Simon2 specific */
void SimonState::vc_73() {
	vc_read_next_byte();
	_op_189_flags |= 1<<vc_read_next_byte();
}

/* Simon2 specific */
void SimonState::vc_74() {
	vc_read_next_byte();
	_op_189_flags &= ~(1<<vc_read_next_byte());
}


void SimonState::o_fade_to_black() {
	uint i;

	memcpy(_video_buf_1, _palette_backup, 256*sizeof(uint32));

	i = NUM_PALETTE_FADEOUT;
	do {
		palette_fadeout((uint32*)_video_buf_1, 32);
		palette_fadeout((uint32*)_video_buf_1 + 32+16, 144);
		palette_fadeout((uint32*)_video_buf_1 + 32+16+144+16, 48);

		_system->set_palette(_video_buf_1, 0, 256);
		_system->update_screen();
		delay(5);
	} while (--i);

	memcpy(_palette_backup, _video_buf_1, 256*sizeof(uint32));
	memcpy(_palette, _video_buf_1, 256*sizeof(uint32));
}

void SimonState::delete_vga_timer(VgaTimerEntry *vte) {
	_lock_word |= 1;
	
	if (vte+1 <= _next_vga_timer_to_process) {
		_next_vga_timer_to_process--;
	}

	do {
		memcpy(vte,vte+1,sizeof(VgaTimerEntry));
		vte++;
	} while (vte->delay);
	
	_lock_word &= ~1;
}

void SimonState::expire_vga_timers() {
	if (_game & GAME_SIMON2) {
		VgaTimerEntry *vte = _vga_timer_list;
		
		_vga_tick_counter++;
		
		while (vte->delay) {
			/* not quite ok, good enough */
			if ((int16)(vte->delay-=5)<=0) {
				uint16 cur_file = vte->cur_vga_file;
				uint16 cur_unk = vte->sprite_id;
				byte *script_ptr = vte->script_pointer;
			
				_next_vga_timer_to_process = vte+1;
				delete_vga_timer(vte);

				if (script_ptr == NULL) {
					/* special scroll timer */
					scroll_timeout();
				} else {
					vc_resume_thread(script_ptr, cur_file, cur_unk);
				}
				vte = _next_vga_timer_to_process;
			} else {
				vte++;
			}
		}
	} else {
		VgaTimerEntry *vte = _vga_timer_list;
		
		_vga_tick_counter++;
		
		while (vte->delay) {
			if (!--vte->delay) {
				uint16 cur_file = vte->cur_vga_file;
				uint16 cur_unk = vte->sprite_id;
				byte *script_ptr = vte->script_pointer;
			
				_next_vga_timer_to_process = vte+1;
				delete_vga_timer(vte);

				vc_resume_thread(script_ptr, cur_file, cur_unk);
				vte = _next_vga_timer_to_process;
			} else {
				vte++;
			}
		}
	}
}

/* Simon2 specific */
void SimonState::scroll_timeout() {
	if (_vga_var2 == 0)
		return;
		
	if (_vga_var2 < 0) {
		if (_vga_var3!=-1) {
			_vga_var3 = -1;
			if (++_vga_var2 == 0)
				return;
		}
	} else {
		if (_vga_var3!=1) {
			_vga_var3 = 1;
			if (--_vga_var2 == 0)
				return;
		}
	}	

	add_vga_timer(10, NULL, 0, 0);
}

void SimonState::vc_resume_thread(byte *code_ptr, uint16 cur_file, uint16 cur_sprite) {
	VgaPointersEntry *vpe;

	_vga_cur_sprite_id = cur_sprite;
	
	_vga_cur_file_id = cur_file;
	_vga_cur_file_2 = cur_file;
	vpe = &_vga_buffer_pointers[cur_file];

	_cur_vga_file_1 = vpe->vgaFile1;
	_cur_vga_file_2 = vpe->vgaFile2;

	_vc_ptr = code_ptr;

	run_vga_script();
}


void SimonState::add_vga_timer(uint num, byte *code_ptr, uint cur_sprite, uint cur_file) {
	VgaTimerEntry *vte;

//	assert( (uint)READ_BE_UINT16_UNALIGNED(&*(uint16*)code_ptr) <= 63);

	_lock_word |= 1;

	for(vte = _vga_timer_list; vte->delay; vte++) { }

	vte->delay = num;
	vte->script_pointer = code_ptr;
	vte->sprite_id = cur_sprite;
	vte->cur_vga_file = cur_file;

	_lock_word &= ~1;
}

void SimonState::o_force_unlock() {
	if (_game&GAME_SIMON2 && _bit_array[4]&0x8000)
		_mouse_cursor = 0;
	_lock_counter = 0;
}

void SimonState::o_force_lock() {
	_lock_word |= 0x4000;
	vc_34();
	_lock_word &= ~0x4000;
}

void SimonState::o_save_game() {
	save_or_load_dialog(false);
}

void SimonState::o_load_game() {
	save_or_load_dialog(true);
}


int SimonState::o_unk_132_helper(bool *b, char *buf) {
	HitArea *ha;
	
	*b = true;

	
	if (!_saveload_flag) {
strange_jump:;
		_saveload_flag = false;
		savegame_dialog(buf);
	}	

start_over:;
	_key_pressed = 0;

start_over_2:;
	_last_hitarea = _last_hitarea_3 = 0;

	do {
		if (_key_pressed != 0) {
			if (_saveload_flag) {
				*b = false;	
				return _key_pressed;
			}
			goto start_over;
		}
		delay(100);
	} while (_last_hitarea_3 == 0);

	ha = _last_hitarea;

	if (ha == NULL || ha->id < 205) goto start_over_2;

	if (ha->id == 205) return ha->id;

	if (ha->id == 206) {
		if (_saveload_row_curpos == 1) goto start_over_2;
		if (_saveload_row_curpos < 7) _saveload_row_curpos = 1;
		else _saveload_row_curpos -= 6;

		goto strange_jump;
	}
	
	if (ha->id == 207) {
		if (!_savedialog_flag) goto start_over_2;
		_saveload_row_curpos += 6;
//		if (_saveload_row_curpos >= _num_savegame_rows)
//			_saveload_row_curpos = _num_savegame_rows;
		goto strange_jump;
	}
	
	if (ha->id >= 214) goto start_over_2;
	return ha->id - 208;
}

void SimonState::o_unk_132_helper_3() {
	for(int i=208; i!=208+6; i++)
		set_hitarea_bit_0x40(i);
}


int SimonState::display_savegame_list(int curpos, bool load, char *dst) {
	int slot, last_slot;
	FILE *in;

	showMessageFormat("\xC");
	
	memset(dst, 0, 18*6);

	slot = curpos;

	while (curpos + 6 > slot) {
		in = fopen(gen_savename(slot), "rb");
		if (!in) break;

		fread(dst, 1, 18, in);
		fclose(in);
		last_slot = slot;
		if (slot < 10)
			showMessageFormat(" ");
		showMessageFormat("%d", slot);
		showMessageFormat(".%s\n", dst);
		dst+=18	;
		slot++;
	}
	/* while_break */
	if (!load) {
		if (curpos + 6 == slot)
			slot++;
		else {
			if (slot < 10)
				showMessageFormat(" ");
			showMessageFormat("%d.\n", slot);
		}
	} else {
		if (curpos +6 == slot) {
			in = fopen(gen_savename(slot), "rb");
			if (in != NULL) {
				slot++;
				fclose(in);
			}
		}
	}

	return slot - curpos;
}


void SimonState::savegame_dialog(char *buf) {
	int i;

	o_unk_132_helper_3();
	
	i = display_savegame_list(_saveload_row_curpos, _save_or_load, buf);

	_savedialog_flag = true;

	if (i != 7) {
		i++;
		if (!_save_or_load)
			i++;
		_savedialog_flag = false;
	}

	if (!--i)
		return;

	do {
		clear_hitarea_bit_0x40(0xd0 + i - 1);
	} while (--i);
}

void SimonState::o_unk_132_helper_2(FillOrCopyStruct *fcs, int x) {
	byte old_text;

	video_putchar(fcs, x);
	old_text = fcs->text_color;
	fcs->text_color = fcs->fill_color;
	
	x += 120;
	if (x != 128) x=129;
	video_putchar(fcs, x);

	fcs->text_color = old_text;
	video_putchar(fcs, 8);
}

void SimonState::save_or_load_dialog(bool load) {
	time_t save_time;
	int num = _number_of_savegames;
	int i;
	int unk132_result;
	FillOrCopyStruct *fcs;
	char *name;
	int name_len;
	bool b;
	char buf[108];

	_save_or_load = load;

	save_time = time(NULL);

	_copy_partial_mode = 1;
	
	num = _number_of_savegames;
	if (!load) num++;
	num -= 6;
	if (num<0) num = 0;
	num++;
	_num_savegame_rows = num;

	_saveload_row_curpos = 1;
	if (!load)
		_saveload_row_curpos = num;
	
	_saveload_flag = false;

restart:;
	do {
		i = o_unk_132_helper(&b, buf);
	} while (!b);

	if (i == 205) goto get_out;
	if (!load) {
		/* if_1 */
if_1:;
		unk132_result = i;

		set_hitarea_bit_0x40(0xd0 + i);
		leaveHitAreaById(0xd0 + i);

		/* some code here */

		fcs = _fcs_ptr_array_3[5];

		fcs->unk2 = unk132_result;
		fcs->unk1 = 2;
		fcs->unk3 = 2;
		fcs->unk6 = 3;

		name = buf + i * 18;

		name_len = 0;
		while (name[name_len]) {
			fcs->unk6++;
			fcs->unk3 += 4;
			if (name[name_len] == 'i' || name[name_len] == 'l')
				fcs->unk3 += 2;
			if (fcs->unk3 >= 8) {
				fcs->unk3 -= 8;
				fcs->unk1++;
			}
			name_len++;
		}
		/* while_1_end */

		/* do_3_start */
		for(;;) {
			video_putchar(fcs, 0x7f);

			_saveload_flag = true;

			/* do_2 */
			do {
				i = o_unk_132_helper(&b, buf);

				if (b) {
					if (i == 205) goto get_out;
					clear_hitarea_bit_0x40(0xd0 + unk132_result);
					if (_saveload_flag) {
						o_unk_132_helper_2(_fcs_ptr_array_3[5], 8);
						/* move code */
					} goto if_1;
				}

				/* is_not_b */
				if (!_saveload_flag) {
					clear_hitarea_bit_0x40(0xd0 + unk132_result);
					goto restart;
				}
			}	while (i >= 0x80 || i == 0);

			/* after_do_2 */
			o_unk_132_helper_2(_fcs_ptr_array_3[5], 8);
			if (i==10 || i==13) break;
			if (i==8) {
				/* do_backspace */
				if (name_len != 0) {
					int x;

					name_len--;
					
					x = (name[name_len] == 'i' || name[name_len]=='l') ? 1 : 8;
					name[name_len] = 0;

					o_unk_132_helper_2(_fcs_ptr_array_3[5], x);
				}
			} else if (i>=32 && name_len!=17) {
				name[name_len++] = i;

				video_putchar(_fcs_ptr_array_3[5], i);
			}
		}

		/* do_save */
		if (!save_game(_saveload_row_curpos + unk132_result, buf + unk132_result * 18))
			warning("Save failed");
	} else {
		if (!load_game(_saveload_row_curpos + i))
			warning("Load failed");
	}

get_out:;
	o_unk_132_helper_3();
//			clear_keydowns();

	_base_time = time(NULL) - save_time + _base_time;
	_copy_partial_mode = 0;
	
	dx_copy_rgn_from_3_to_2(94, 208, 46, 80);

	i = _timer_4;
	do {
		delay(10);
	} while (i == _timer_4);
}

void SimonState::o_unk_127() {
	if (_game & GAME_SIMON2) {
		uint a = getVarOrWord();
		uint b = getVarOrWord();
		uint c = getVarOrByte();

		warning("o_unk_127(%d,%d,%d) not implemented properly", a, b, c);	

		if (a!=_last_music_played) {
			_last_music_played = a;
			playMusic(a);
		}
	} else {
		uint a = getVarOrWord();
		/*uint b = */getVarOrWord();

		if (a!=_last_music_played) {
			_last_music_played = a;
			playMusic(a);
		}
	}
}

void SimonState::o_unk_120(uint a) {
	uint16 id = TO_BE_16(a);
	_lock_word |= 0x4000;
	_vc_ptr = (byte*)&id;
	vc_15_start_funkystruct_by_id();
	_lock_word &= ~0x4000;
}

void SimonState::o_wait_for_vga(uint a) {
	_vga_wait_for = a;
	_timer_1 = 0;
	_exit_cutscene = false;
	while (_vga_wait_for != 0) {
		if (_exit_cutscene) {
			if (vc_get_bit(9)) {
				startSubroutine170();
				break;
			}
		} else {
			processSpecialKeys();
		}

		delay(10);
	
//		if (_timer_1 >= 500) {
//			warning("wait timed out");
//			break;
//		}
			
	}
//	warning("waiting on %d done", a);
}

void SimonState::timer_vga_sprites() {
	VgaSprite *vsp;
	VgaPointersEntry *vpe;
	byte *vc_ptr_org = _vc_ptr;
	uint16 params[5]; /* parameters to vc_10 */

	if (_video_var_9 == 2)
		_video_var_9 = 1;

#ifdef DRAW_THREE_STARS
	fprintf(_dump_file,"***\n");
#endif

	if (_game&GAME_SIMON2 && _vga_var3) {
		timer_vga_sprites_helper();
	}

	vsp = _vga_sprites;
	while (vsp->id != 0) {
		vsp->unk6 &= 0x7FFF;

		vpe = &_vga_buffer_pointers[vsp->unk7];
		_cur_vga_file_1 = vpe->vgaFile1;
		_cur_vga_file_2 = vpe->vgaFile2;
		_video_palette_mode = vsp->unk6;
		_vga_cur_sprite_id = vsp->id;

		params[0] = READ_BE_UINT16_UNALIGNED(&vsp->image);
		params[1] = READ_BE_UINT16_UNALIGNED(&vsp->base_color);
		params[2] = READ_BE_UINT16_UNALIGNED(&vsp->x);
		params[3] = READ_BE_UINT16_UNALIGNED(&vsp->y);

		if(_game & GAME_SIMON2) {
			*(byte*)(&params[4]) = (byte)vsp->unk4;
		} else {
			params[4] = READ_BE_UINT16_UNALIGNED(&vsp->unk4);
		}

		_vc_ptr = (byte*)params;
		vc_10();

		vsp++;
	}

#ifdef DRAW_IMAGES_DEBUG
	memset(sdl_buf_attached, 0, 320*200);
#endif
	_video_var_8++;
	_vc_ptr = vc_ptr_org;
}

void SimonState::timer_vga_sprites_helper() {
	byte *dst = dx_lock_2(), *src;
	uint x;

	if (_vga_var3<0) {
		memmove(dst+8,dst,320*_vga_var5-8);
	} else {
		memmove(dst, dst+8, 320*_vga_var5-8);
	}

	x = _x_scroll-1;

	if (_vga_var3>0) {
		dst += 320-8;
		x += 41;
	}

	src = _vga_var7 + x*4;
	vc_10_helper_8(dst,src+READ_BE_UINT32_UNALIGNED(&*((uint32*)src)));

	dx_unlock_2();


	memcpy(sdl_buf_attached, sdl_buf, 320*200);
	dx_copy_from_attached_to_3(_vga_var5);


	_x_scroll += _vga_var3;

	vc_write_var(0xfB, _x_scroll);

	_vga_var3 = 0;
}

#ifdef DRAW_IMAGES_DEBUG
void SimonState::timer_vga_sprites_2() {
	VgaSprite *vsp;
	VgaPointersEntry *vpe;
	byte *vc_ptr_org = _vc_ptr;
	uint16 params[5]; /* parameters to vc_10 */

	if (_video_var_9 == 2)
		_video_var_9 = 1;

	vsp = _vga_sprites;
	while (vsp->id != 0) {
		vsp->unk6 &= 0x7FFF;

		vpe = &_vga_buffer_pointers[vsp->unk7];
		_cur_vga_file_1 = vpe->vgaFile1;
		_cur_vga_file_2 = vpe->vgaFile2;
		_video_palette_mode = vsp->unk6;
		_vga_cur_sprite_id = vsp->id;

		if (vsp->image)
			fprintf(_dump_file,"id:%5d image:%3d base-color:%3d x:%3d y:%3d flags:%x\n", 
				vsp->id, vsp->image, vsp->base_color, vsp->x, vsp->y, vsp->unk4 );
		params[0] = READ_BE_UINT16_UNALIGNED(&vsp->image);
		params[1] = READ_BE_UINT16_UNALIGNED(&vsp->base_color);
		params[2] = READ_BE_UINT16_UNALIGNED(&vsp->x);
		params[3] = READ_BE_UINT16_UNALIGNED(&vsp->y);
		params[4] = READ_BE_UINT16_UNALIGNED(&vsp->unk4);
		_vc_ptr = (byte*)params;
		vc_10();

		vsp++;
	}

#ifdef DRAW_THREE_STARS
	fprintf(_dump_file,"***\n");
#endif

	_video_var_8++;
	_vc_ptr = vc_ptr_org;
}
#endif

void SimonState::timer_proc1() {
	_timer_4++;
	
	if(_lock_word & 0xC0E9 || _lock_word & 2)
		return;

	_timer_1++;
	
	_lock_word |= 2;

	if (!(_lock_word&0x10)) {
		if (!(_game & GAME_SIMON2)) {
			expire_vga_timers();
			expire_vga_timers();
			_cepe_flag^=1;
			if (!_cepe_flag)
				expire_vga_timers();

			_sync_flag_2 ^= 1;
		} else {
			_sync_flag_2^=1;

			if (!_sync_flag_2)
				expire_vga_timers();
			
			if (_lock_counter!=0 && !_sync_flag_2) {
				_lock_word &= ~2;
				return;
			}
		}

//		if (_lock_counter !=0 && _sync_flag_2==1) {
//			printf("skipping draw...\n");
//			goto get_out;
//		}
	}
	
	timer_vga_sprites();
#ifdef DRAW_IMAGES_DEBUG
	timer_vga_sprites_2();
#endif

	if (!(_game&GAME_SIMON2)) {
		if (_copy_partial_mode == 1) {
			dx_copy_from_2_to_attached(80, 46, 208-80, 94-46);
		}

		if (_copy_partial_mode==2) {
			/* copy partial from attached to 2 */
			dx_copy_from_attached_to_2(176, 61, 320-176, 134-61);
			_copy_partial_mode = 0;
		}
	}

	/* XXX: more stuff here */
	if (_video_var_8){
		handle_mouse_moved();
		/* XXX: more stuff here */
		dx_update_screen_and_palette();
		_sync_flag_1 = false;
		_video_var_8 = false;
	}


	_lock_word &= ~2;	
}

void SimonState::timer_callback() {
//	uint32 start, end;

	if (_timer_5 != 0) {
		_sync_flag_2 = true;
		_timer_5 --;
	} else {
//		start = timeGetTime();
		timer_proc1();
//		end = timeGetTime();

//		if (start + 45 < end) {
//			_timer_5 = (uint16)( (end - start) / 45);
//		}
	}
}

void SimonState::checkTimerCallback() {
	if (_invoke_timer_callback && !_in_callback) {
		_in_callback = true;
		_invoke_timer_callback = 0;
		timer_callback();
		_in_callback = false;
	}
}


void SimonState::o_unk_163(uint a) {
	playSound(a);
}

void SimonState::o_unk_160(uint a) {
	fcs_proc_1(_fcs_ptr_array_3[_fcs_unk_1], a);
}

void SimonState::fcs_proc_1(FillOrCopyStruct *fcs, uint value) {
	fcs->text_color = value;
}

void SimonState::o_unk_103() {
	lock();
	fcs_unk1(_fcs_unk_1);
	showMessageFormat("\x0C");
	unlock();
}

void SimonState::o_vga_reset() {
	_lock_word |= 0x4000;
	vc_27_reset();
	_lock_word &= ~0x4000;
}

void SimonState::o_unk_99_simon1(uint a) {
	uint16 b = TO_BE_16(a);
	_lock_word |= 0x4000;
	_vc_ptr = (byte*)&b;
	vc_60();
	_lock_word &= ~0x4000;
}

void SimonState::o_unk_99_simon2(uint a, uint b) {
	uint16 items[2];
	
	items[0] = TO_BE_16(a);
	items[1] = TO_BE_16(a);

	_lock_word |= 0x4000;
	_vc_ptr = (byte*)&items;
	vc_60();
	_lock_word &= ~0x4000;
}

bool SimonState::vc_maybe_skip_proc_3(uint16 a) {
	Item *item;

	CHECK_BOUNDS(a, _vc_item_array);

	item = _vc_item_array[a];
	if (item == NULL)
		return true;

	return getItem1Ptr()->parent == item->parent;
}

bool SimonState::vc_maybe_skip_proc_2(uint16 a, uint16 b) {
	Item *item_a, *item_b;

	CHECK_BOUNDS(a, _vc_item_array);
	CHECK_BOUNDS(b, _vc_item_array);

	item_a = _vc_item_array[a];
	item_b = _vc_item_array[b];

	if (item_a == NULL || item_b == NULL)
		return true;

	return derefItem(item_a->parent) == item_b;
}

bool SimonState::vc_maybe_skip_proc_1(uint16 a, int16 b) {
	Item *item;

	CHECK_BOUNDS(a, _vc_item_array);

	item = _vc_item_array[a];
	if (item == NULL)
		return true;
	return item->unk3 == b;
}


/* OK */
void SimonState::fcs_delete(uint a) {
	if (_fcs_ptr_array_3[a] == NULL)
		return;
	fcs_unk1(a);
	video_copy_if_flag_0x8_c(_fcs_ptr_array_3[a]);
	_fcs_ptr_array_3[a] = NULL;
	if (_fcs_unk_1 == a) {
		_fcs_ptr_1 = NULL;
		fcs_unk_2(0);
	}
}

/* OK */
void SimonState::fcs_unk_2(uint a) {
	a &= 7;

	if (_fcs_ptr_array_3[a] == NULL || _fcs_unk_1 == a)
		return;

	_fcs_unk_1 = a;
	startUp_helper_3();
	_fcs_ptr_1 = _fcs_ptr_array_3[a];
	
	showmessage_helper_3(_fcs_ptr_1->unk6, _fcs_ptr_1->unk7);
}

/* OK */
void SimonState::o_unk26_helper(uint a, uint b, uint c, uint d, uint e, uint f, uint g, uint h) {
	a &= 7;

	if (_fcs_ptr_array_3[a])
		fcs_delete(a);

	_fcs_ptr_array_3[a] = fcs_alloc(b,c,d,e,f,g,h);
	
	if (a == _fcs_unk_1) {
		_fcs_ptr_1 = _fcs_ptr_array_3[a];
		showmessage_helper_3(_fcs_ptr_1->unk6, _fcs_ptr_1->unk7);
	}
}

/* OK */
FillOrCopyStruct *SimonState::fcs_alloc(uint x, uint y, uint w, uint h, uint flags, uint fill_color, uint unk4) {
	FillOrCopyStruct *fcs;
	
	fcs = _fcs_list;
	while(fcs->mode != 0) fcs++;

	fcs->mode = 2;
	fcs->x = x;
	fcs->y = y;
	fcs->width = w;
	fcs->height = h;
	fcs->flags = flags;
	fcs->fill_color = fill_color;
	fcs->text_color = unk4;
	fcs->unk1 = 0;
	fcs->unk2 = 0;
	fcs->unk3 = 0;
	fcs->unk7 = fcs->width * 8 / 6;
	return fcs;
}

Item *SimonState::derefItem(uint item) {
	if (item >= _itemarray_size)
		error("derefItem: invalid item %d", item);
	return _itemarray_ptr[item];
}

uint SimonState::itemPtrToID(Item *id) {
	uint i;
	for(i = 0; i!=_itemarray_size; i++)
		if (_itemarray_ptr[i] == id)
			return i;
	error("itemPtrToID: not found");
}

void SimonState::o_pathfind(int x,int y,uint var_1,uint var_2) {
	uint16 *p;
	uint i, j;
	uint prev_i;
	uint x_diff, y_diff;
	uint best_i=0, best_j=0, best_dist = 0xFFFFFFFF;

	if (_game & GAME_SIMON2) {
		x += _x_scroll * 8;
	}

	prev_i = 21 - _variableArray[12];
	for(i=20; i!=0; --i) {
		p = (uint16*)_pathfind_array[20-i];
		if (!p)
			continue;
		for(j=0; READ_BE_UINT16_UNALIGNED(&p[0]) != 999; j++,p+=2) { /* 0xE703 = byteswapped 999 */
			x_diff = abs(READ_BE_UINT16_UNALIGNED(&p[0]) - x);
			y_diff = abs(READ_BE_UINT16_UNALIGNED(&p[1]) - 12 - y);

			if (x_diff < y_diff) {
				x_diff >>= 2;
				y_diff <<= 2;
			}
			x_diff += y_diff >> 2;

			if (x_diff < best_dist || x_diff==best_dist && prev_i==i) {
				best_dist = x_diff;
				best_i = 21 - i;
				best_j = j;
			}
		}
	}
	
	_variableArray[var_1] = best_i;
	_variableArray[var_2] = best_j;
}


/* ok */
void SimonState::fcs_unk1(uint fcs_index) {
	FillOrCopyStruct *fcs;
	uint16 fcsunk1;
	uint16 i;

	fcs = _fcs_ptr_array_3[fcs_index&7];
	fcsunk1 = _fcs_unk_1;
	
	if (fcs==NULL || fcs->fcs_data==NULL)
		return;

	fcs_unk_2(fcs_index);
	fcs_putchar(12);
	fcs_unk_2(fcsunk1);
	
	for(i = 0;fcs->fcs_data->e[i].item != NULL;i++) {
		delete_hitarea_by_index(fcs->fcs_data->e[i].hit_area);
	}

	if (fcs->fcs_data->unk3 != -1) {
		delete_hitarea_by_index(fcs->fcs_data->unk3);
	}

	if (fcs->fcs_data->unk4 != -1) {
		delete_hitarea_by_index(fcs->fcs_data->unk4);
		fcs_unk_5(fcs, fcs_index);
	}

	free(fcs->fcs_data);
	fcs->fcs_data = NULL;

	_fcs_data_1[fcs_index] = 0;
	_fcs_data_2[fcs_index] = 0;
}

/* ok */
void SimonState::fcs_unk_5(FillOrCopyStruct *fcs, uint fcs_index) {
	if (!(_game & GAME_SIMON2)) {
		o_unk_99_simon1(0x80);
	}
}

void SimonState::delete_hitarea_by_index(uint index) {
	CHECK_BOUNDS(index, _hit_areas);
	_hit_areas[index].flags = 0;
}

/* ok */
void SimonState::fcs_putchar(uint a) {
	if (_fcs_ptr_1 != _fcs_ptr_array_3[0])
		video_putchar(_fcs_ptr_1, a);
}

/* ok */
void SimonState::video_fill_or_copy_from_3_to_2(FillOrCopyStruct *fcs) {
	if (fcs->flags & 0x10)
		copy_img_from_3_to_2(fcs);
	else
		video_erase(fcs);

	fcs->unk1 = 0;
	fcs->unk2 = 0;
	fcs->unk3 = 0;
	fcs->unk6 = 0;
}

/* ok */
void SimonState::copy_img_from_3_to_2(FillOrCopyStruct *fcs) {
	_lock_word |= 0x8000;

	if (!(_game & GAME_SIMON2)) {
		dx_copy_rgn_from_3_to_2(
			fcs->y + fcs->height*8 + ((fcs==_fcs_ptr_array_3[2])?1:0), 
			(fcs->x+fcs->width)*8,
			fcs->y,
			fcs->x*8);
	} else {
		if (_vga_var6 && _fcs_ptr_array_3[2]==fcs) {
			fcs = _fcs_ptr_array_3[0x18/4];
			_vga_var6 = 0;
		}

		dx_copy_rgn_from_3_to_2(
			fcs->y + fcs->height*8,
			(fcs->x+fcs->width)*8,
			fcs->y,
			fcs->x*8);
	}

	_lock_word &= ~0x8000;
}

void SimonState::video_erase(FillOrCopyStruct *fcs) {
	byte *dst;
	uint h;
	
	_lock_word |= 0x8000;

	dst = dx_lock_2();
	dst += _dx_surface_pitch * fcs->y + fcs->x*8;

	h = fcs->height * 8;
	do {
		memset(dst, fcs->fill_color, fcs->width*8);
		dst += _dx_surface_pitch;
	} while (--h);

	dx_unlock_2();
	_lock_word &= ~0x8000;
}

VgaSprite *SimonState::find_cur_sprite() {
	if (_game & GAME_SIMON2) {
		VgaSprite *vsp = _vga_sprites;
		while (vsp->id) {
			if (vsp->id == _vga_cur_sprite_id
				&& vsp->unk7 == _vga_cur_file_id)
				break;
			vsp++;
		}
		return vsp;
	} else {
		VgaSprite *vsp = _vga_sprites;
		while (vsp->id) {
			if (vsp->id == _vga_cur_sprite_id)
				break;
			vsp++;
		}
		return vsp;
	}
}

bool SimonState::has_vgastruct_with_id(uint16 id, uint16 file) {
	if (_game & GAME_SIMON2) {
		VgaSprite *vsp = _vga_sprites;
		while (vsp->id) {
			if (vsp->id == id	&& vsp->unk7==file)
				return true;
			vsp++;
		}
		return false;
	} else {
		VgaSprite *vsp = _vga_sprites;
		while (vsp->id) {
			if (vsp->id == id)
				return true;
			vsp++;
		}
		return false;
	}
}

void SimonState::processSpecialKeys() {
}

void SimonState::draw_mouse_pointer() {
}


void decompress_icon(byte *dst, byte *src, uint w, uint h_org, byte base, uint pitch) {
	int8 reps;
	byte color_1, color_2;
	byte *dst_org = dst;
	uint h = h_org;

	for(;;) {
		reps = *src++;
		if (reps < 0) {
			reps--;
			color_1 = *src >> 4;
			if (color_1 != 0) color_1 |= base;
			color_2 = *src++ & 0xF;
			if (color_2 != 0) color_2 |= base;
			
			do {	
				if (color_1 != 0) *dst = color_1;
				dst += pitch;
				if (color_2 != 0) *dst = color_2;
				dst += pitch;

				/* reached bottom? */
				if (--h == 0) {
					/* reached right edge? */				
					if (--w == 0)
						return;
					dst = ++dst_org;
					h = h_org;
				}
			} while (++reps != 0);
		} else {
			do {	
				color_1 = *src >> 4;
				if (color_1 != 0) *dst = color_1 | base;
				dst += pitch;

				color_2 = *src++ & 0xF;
				if (color_2 != 0) *dst = color_2 | base;
				dst += pitch;

				/* reached bottom? */
				if (--h == 0) {
					/* reached right edge? */				
					if (--w == 0)
						return;
					dst = ++dst_org;
					h = h_org;
				}
			} while (--reps >= 0);
		}
	}
}


void SimonState::draw_icon_c(FillOrCopyStruct *fcs, uint icon, uint x, uint y) {
	byte *dst;
	byte *src;

	if (!(_game & GAME_SIMON2)) {
		_lock_word |= 0x8000;

		dst = dx_lock_2();
		dst += (x + fcs->x) * 8;
		dst += (y*25 + fcs->y) * _dx_surface_pitch;

		src = _icon_file_ptr;
		src += READ_LE_UINT16(&((uint16*)src)[icon]);

		decompress_icon(dst, src, 24, 12, 0xE0,_dx_surface_pitch);

		dx_unlock_2();
		_lock_word &= ~0x8000;
	} else {
		_lock_word |= 0x8000;	
		dst = dx_lock_2();

		dst += 110;
		dst += x;
		dst += (y+fcs->y)*_dx_surface_pitch;

		src = _icon_file_ptr;
		src += READ_LE_UINT16(&((uint16*)src)[icon*2+0]);
		decompress_icon(dst, src, 20, 10, 0xE0,_dx_surface_pitch);

		src = _icon_file_ptr;
		src += READ_LE_UINT16(&((uint16*)src)[icon*2+1]);
		decompress_icon(dst, src, 20, 10, 0xD0,_dx_surface_pitch);

		dx_unlock_2();
		_lock_word &= ~0x8000;
	}
}

void SimonState::video_toggle_colors(HitArea *ha, byte a, byte b, byte c, byte d) {
	byte *src, color;
	uint w,h,i;

	_lock_word |= 0x8000;
	src = dx_lock_2() + ha->y*_dx_surface_pitch + ha->x;

	w = ha->width;
	h = ha->height;

	if(!(h>0 && w>0 && ha->x + w<=320 && ha->y+h<=200)) {
		warning("Invalid coordinates in video_toggle_colors (%d,%d,%d,%d)", ha->x, ha->y,ha->width, ha->height);
		return;
	}

	do {
		for(i=0; i!=w; ++i) {
			color = src[i];
			if (a>=color && b<color) {
				if (c >= color)
					color += d;
				else
					color -= d;
				src[i] = color;
			}
		}
		src += _dx_surface_pitch;
	} while(--h);


	dx_unlock_2();
	_lock_word &= ~0x8000;
}

bool SimonState::vc_59_helper() {
#ifdef USE_TEXT_HACK
	return true;
#else
	if (_voice_file==NULL)
		return false;
	return _voice_sound == 0;
#endif
}

void SimonState::video_copy_if_flag_0x8_c(FillOrCopyStruct *fcs) {
	if (fcs->flags&8)
		copy_img_from_3_to_2(fcs);
	fcs->mode = 0;
}

void SimonState::showMessageFormat(const char *s, ...) {
	char buf[1024],*str;
	va_list va;

	va_start(va, s);
	vsprintf(buf, s, va);
	va_end(va);

	if (!_fcs_data_1[_fcs_unk_1]) {
		showmessage_helper_2();
		if (!_showmessage_flag) {
			_fcs_ptr_array_3[0] = _fcs_ptr_1;
			showmessage_helper_3(_fcs_ptr_1->unk6, _fcs_ptr_1->unk7);
		}
		_showmessage_flag = true;
		_fcs_data_1[_fcs_unk_1] = 1;
	}

	for(str=buf;*str;str++)
		showmessage_print_char(*str);
}

void SimonState::showmessage_helper_2() {
	if (_fcs_ptr_1)
		return;

	_fcs_ptr_1 = fcs_alloc(8, 0x90, 0x18, 6, 1, 0, 0xF);
}

void SimonState::readSfxFile(const char *filename) {
	if (!(_game & GAME_SIMON2)) {
		FILE *in;
		uint32 size;

		in = fopen_maybe_lowercase(filename);

		if(in==NULL) {
			warning("readSfxFile: Cannot load sfx file %s", filename);
			return;
		}

		fseek(in, 0, SEEK_END);
		size = ftell(in);

		rewind(in);
		
		/* stop all sounds */
		_mixer->stop_all();

		if (_sfx_heap) free(_sfx_heap);

		_sfx_heap = (byte*)malloc(size);

		if (_sfx_heap == NULL)
			error("readSfxFile: Not enough SFX memory");

		fread(_sfx_heap, size, 1, in);

		fclose(in);
	} else {
		int res;
		uint32 offs;
		int size;

		vc_29_stop_all_sounds();

		if (_sfx_heap) free(_sfx_heap);

		res = atoi(filename + 6) + gss->SOUND_INDEX_BASE - 1;
		offs = _game_offsets_ptr[res];
		size = _game_offsets_ptr[res+1] - offs;

		if (size == 0)
			return;

		_sfx_heap = (byte*)malloc(size);

		resfile_read(_sfx_heap, offs, size);
	}
}

void SimonState::video_putchar(FillOrCopyStruct *fcs, byte c) {
	if (c == 0xC) {
		video_fill_or_copy_from_3_to_2(fcs);
	} else if (c == 0xD || c==0xA) {
		video_putchar_helper(fcs);
	} else if (c==8 || c==1) {
		int8 val = (c==8) ? 6 : 4;
		if (fcs->unk6!=0) {
			fcs->unk6--;
			fcs->unk3 -= val;
			if ((int8)fcs->unk3 < val) {
				fcs->unk3 += 8;
				fcs->unk1--;
			}
		}
	} else if (c>=0x20) {
		if (fcs->unk6 == fcs->unk7) {
			video_putchar_helper(fcs);
		} else if (fcs->unk2 == fcs->height) {
			video_putchar_helper(fcs);
			fcs->unk2--;
		}

		video_putchar_helper_2(fcs, fcs->unk1 + fcs->x, fcs->unk2 * 8 + fcs->y, c);

		fcs->unk6++;
		fcs->unk3 += 4;
		if (c != 'i' && c != 'l')
			fcs->unk3 += 2;

		if (fcs->unk3 >= 8) {
			fcs->unk3 -= 8;
			fcs->unk1++;
		}
	}
}

void SimonState::video_putchar_helper(FillOrCopyStruct *fcs) {
	fcs->unk3 = 0;
	fcs->unk6 = 0;
	fcs->unk1 = 0;

	if (fcs->unk2 != fcs->height)
		fcs->unk2++;
}

static const byte video_font[] = {
  0,  0,  0,  0,  0,  0,  0,  0,
 32,112,112, 32, 32,  0, 32,  0,
 48, 48, 96,  0,  0,  0,  0,  0,
  0,144,  0, 96,144,144,104,  0,
  0,144,  0, 96,144,144, 96,  0,
  0,144,  0,144,144,144, 96,  0,
  0, 16, 40, 16, 42, 68, 58,  0,
 48, 48, 96,  0,  0,  0,  0,  0,
  0,  4,  8,  8,  8,  8,  4,  0,
  0, 32, 16, 16, 16, 16, 32,  0,
  0,  0, 20,  8, 62,  8, 20,  0,
  0,112,136,240,136,136,240,  0,
  0,  0,  0,  0,  0, 48, 48, 96,
  0,  0,  0,240,  0,  0,  0,  0,
  0,  0,  0,  0,  0, 48, 48,  0,
 16, 32,  0,120,112, 64, 56,  0,
112,136,152,168,200,136,112,  0,
 32, 96, 32, 32, 32, 32,112,  0,
112,136,  8, 48, 64,136,248,  0,
112,136,  8, 48,  8,136,112,  0,
 16, 48, 80,144,248, 16, 56,  0,
248,128,240,  8,  8,136,112,  0,
 48, 64,128,240,136,136,112,  0,
248,136,  8, 16, 32, 32, 32,  0,
112,136,136,112,136,136,112,  0,
112,136,136,120,  8, 16, 96,  0,
  0,  0, 48, 48,  0, 48, 48,  0,
 32, 16,  0,112,  8,248,120,  0,
 32, 80,  0,144,144,144,104,  0,
 32, 16,  0,112,248,128,112,  0,
 32, 80,  0,112,248,128,112,  0,
112,136,  8, 16, 32,  0, 32,  0,
 32, 80,  0,192, 64, 64,224,  0,
112,136,136,248,136,136,136,  0,
240, 72, 72,112, 72, 72,240,  0,
 48, 72,128,128,128, 72, 48,  0,
224, 80, 72, 72, 72, 80,224,  0,
248, 72, 64,112, 64, 72,248,  0,
248, 72, 64,112, 64, 64,224,  0,
 48, 72,128,152,136, 72, 56,  0,
136,136,136,248,136,136,136,  0,
248, 32, 32, 32, 32, 32,248,  0,
 24,  8,  8,  8,136,136,112,  0,
200, 72, 80, 96, 80, 72,200,  0,
224, 64, 64, 64, 64, 72,248,  0,
136,216,168,168,136,136,136,  0,
136,200,168,152,136,136,136,  0,
112,136,136,136,136,136,112,  0,
240, 72, 72,112, 64, 64,224,  0,
112,136,136,136,136,168,112,  8,
240, 72, 72,112, 72, 72,200,  0,
112,136,128,112,  8,136,112,  0,
248,168, 32, 32, 32, 32,112,  0,
136,136,136,136,136,136,120,  0,
136,136,136, 80, 80, 32, 32,  0,
136,136,136,136,168,216,136,  0,
136,136, 80, 32, 80,136,136,  0,
136,136,136,112, 32, 32,112,  0,
248,136, 16, 32, 64,136,248,  0,
  0, 14,  8,  8,  8,  8, 14,  0,
  0,128, 64, 32, 16,  8,  4,  0,
  0,112, 16, 16, 16, 16,112,  0,
  0, 48, 72, 64, 72, 48, 16, 48,
  0, 80,  0, 96, 32, 40, 48,  0,
 32, 16,  0,152,144,144,232,  0,
  0,  0,112,  8,120,136,120,  0,
192, 64, 80,104, 72, 72,112,  0,
  0,  0,112,136,128,136,112,  0,
 24, 16, 80,176,144,144,112,  0,
  0,  0,112,136,248,128,112,  0,
 48, 72, 64,224, 64, 64,224,  0,
  0,  0,104,144,144,112,136,112,
192, 64, 80,104, 72, 72,200,  0,
 64,  0,192, 64, 64, 64,224,  0,
  8,  0,  8,  8,  8,  8,136,112,
192, 64, 72, 80, 96, 80,200,  0,
192, 64, 64, 64, 64, 64,224,  0,
  0,  0,144,216,168,136,136,  0,
  0,  0,240,136,136,136,136,  0,
  0,  0,112,136,136,136,112,  0,
  0,  0,176, 72, 72,112, 64,224,
  0,  0,104,144,144,112, 16, 56,
  0,  0,176, 72, 72, 64,224,  0,
  0,  0,120,128,112,  8,240,  0,
 64, 64,240, 64, 64, 72, 48,  0,
  0,  0,144,144,144,144,104,  0,
  0,  0,136,136,136, 80, 32,  0,
  0,  0,136,136,168,216,144,  0,
  0,  0,136, 80, 32, 80,136,  0,
  0,  0,136,136,136,112, 32,192,
  0,  0,248,144, 32, 72,248,  0,
 32, 80,  0, 96,144,144, 96,  0,
  0, 14,  8, 48,  8,  8, 14,  0,
  0,  8,  8,  8,  8,  8,  8,  0,
  0,112, 16, 12, 16, 16,112,  0,
  0,  0,  0,  0,  0,  0,248,  0,
252,252,252,252,252,252,252,252,
240,240,240,240,240,240,240,240,
};

void SimonState::video_putchar_helper_2(FillOrCopyStruct *fcs, uint x, uint y, byte chr) {
	const byte *src;
	byte color, *dst;
	uint h,i;

	_lock_word |= 0x8000;

	dst = dx_lock_2();
	dst += y * _dx_surface_pitch + x*8 + fcs->unk3;

	src = video_font + (chr-0x20) * 8;

	color = fcs->text_color;

	h = 8;
	do {
		int8 b = *src++;
		i = 0;
		do {
			if (b<0) dst[i] = color;
			b<<=1;
		} while (++i!=6);
		dst += _dx_surface_pitch;
	} while (--h);

	dx_unlock_2();

	_lock_word &= ~0x8000;
}

void SimonState::start_vga_code(uint b, uint vga_res, uint vga_struct_id,
																		uint c, uint d, uint f) {
	VgaSprite *vsp;
	VgaPointersEntry *vpe;
	byte *p,*pp;
	uint count;
	
	_lock_word |= 0x40;

	if (has_vgastruct_with_id(vga_struct_id,vga_res))
		return;

	vsp = _vga_sprites;
	while(vsp->id!=0) vsp++;

	vsp->unk6 = b;
	vsp->unk5 = 0;
	vsp->unk4 = 0;

	vsp->y = d;
	vsp->x = c;
	vsp->image = 0;
	vsp->base_color = f;
	vsp->id = vga_struct_id;
	vsp->unk7 = vga_res;

	for(;;) {
		vpe = &_vga_buffer_pointers[vga_res];
		_vga_cur_file_2 = vga_res;
		_cur_vga_file_1 = vpe->vgaFile1;
		if (vpe->vgaFile1 != NULL)
			break;
		ensureVgaResLoaded(vga_res);
	}
	
	pp = _cur_vga_file_1;
	p = pp + READ_BE_UINT16_UNALIGNED(&((VgaFile1Header*)pp)->hdr2_start);

	count = READ_BE_UINT16_UNALIGNED(&((VgaFile1Header2*)p)->id_count);
	p = pp + READ_BE_UINT16_UNALIGNED(&((VgaFile1Header2*)p)->id_table);
	
	for(;;) {
		if (READ_BE_UINT16_UNALIGNED(&((VgaFile1Struct0x6*)p)->id) == vga_struct_id) {
			
			dump_vga_script(pp + READ_BE_UINT16_UNALIGNED(&((VgaFile1Struct0x6*)p)->script_offs), vga_res, vga_struct_id);

			add_vga_timer(gss->VGA_DELAY_BASE, 
				pp + READ_BE_UINT16_UNALIGNED(&((VgaFile1Struct0x6*)p)->script_offs),
				vga_struct_id, vga_res);
			break;
		}
		p += sizeof(VgaFile1Struct0x6);
		if (!--count) {
			vsp->id = 0;
			break;
		}
	}

	_lock_word &= ~0x40;
}

void SimonState::dump_vga_script_always(byte *ptr, uint res, uint sprite_id) {
	fprintf(_dump_file,"; address=%x, vgafile=%d  vgasprite=%d\n", 
		ptr - _vga_buffer_pointers[res].vgaFile1, res, sprite_id);
	dump_video_script(ptr, false);
	fprintf(_dump_file,"; end\n");
}

void SimonState::dump_vga_script(byte *ptr, uint res, uint sprite_id) {
#ifdef DUMP_START_VGASCRIPT
	dump_Vga_script_always(ptr,res,sprite_id);
#endif
}

void SimonState::talk_with_speech(uint speech_id, uint num_1) {
	if (!(_game & GAME_SIMON2)) {
		if (speech_id == 9999) {
			if (!(_bit_array[0] & 0x4000) && !(_bit_array[1]&0x1000)) {
				_bit_array[0]|=0x4000;
				_variableArray[0xc8/2] = 0xF;
				start_vga_code(4, 1, 0x82, 0, 0, 0);
				o_wait_for_vga(0x82);
			}
			_skip_vga_wait = true;
			return;
		}

		if (num_1 < 100) {
			o_unk_99_simon1(num_1 + 201);
		}

		playVoice(speech_id);

		if (num_1 < 100) {
			start_vga_code(4, 2, num_1+201,0,0,0);
		}
	} else {
		if (speech_id  == 0xFFFF) {
			if (_vk_t_toggle)
				return;
			if (!(_bit_array[0] & 0x4000 || _bit_array[1] & 0x1000)) {			
				_bit_array[0] |= 0x4000;
		
				start_vga_code(4, 1, 0x1e, 0, 0, 0);
				o_wait_for_vga(0x82);
			}
			_skip_vga_wait = true;
		} else {
			if (_vk_t_toggle && _scriptvar_2) {
				start_vga_code(4, 2, 5, 0, 0, 0);
				o_wait_for_vga(0xcd);
				o_unk_99_simon2(2,5);
			}
			o_unk_99_simon2(2,num_1+2);
			playVoice(speech_id);

			start_vga_code(4, 2, num_1+2, 0, 0, 0);
		}
	}
}

void SimonState::talk_with_text(uint num_1, uint num_2, const char *string_ptr, uint threeval_a, int threeval_b, uint width) {
	char print_str_buf[0x140];
	char *char_buf;
	const char *string_ptr_2, *string_ptr_3;
	int j;
	uint letters_per_row, len_div_3, num_of_rows;
	uint m, n;
	uint height;

	char_buf = print_str_buf;
	string_ptr_3 = string_ptr_2 = string_ptr;

	height = 10;
	j = 0;

	letters_per_row = width / 6;

	len_div_3 = (strlen(string_ptr) + 3) / 3;

	if (!(_game & GAME_SIMON2)) {
		if (_variableArray[141] == 0)
			_variableArray[141] = 9;
		_variableArray[85] = _variableArray[141] * len_div_3;
	} else {
		if (_variableArray[86] == 0) len_div_3 >>= 1;
		if (_variableArray[86] == 2) len_div_3 <<= 1;
		_variableArray[85] = len_div_3 * 5;
	}

	num_of_rows = strlen(string_ptr) / letters_per_row;

	while(num_of_rows==1 && j!=-1) {
		m = strlen(string_ptr) >> 1;
		m -= j;
		string_ptr_2 += m;

		while (*string_ptr_2++ != ' ' && m <= letters_per_row) m++;

		if (m <= letters_per_row && strlen(string_ptr_2) < letters_per_row) {
			/* if_1 */
			n = (letters_per_row - m + 1) >> 1;

			while (n != 0) {
				*char_buf++ = ' ';
				n--;
			}
			strncpy(char_buf, string_ptr, m);
			char_buf += m;
			*char_buf++= 10;

			height += 10;
			threeval_b -= 10;

			if (threeval_b < 2)
				threeval_b = 2;
			j = -1;
		} else {
			/* else_1 */
			j -= 4;
			if (j == -12) {
				j = 0;
				num_of_rows = 2;
			}
			string_ptr_2 = string_ptr_3;
		}
	}

	if (j != -1 && width*30 > 8000)
		num_of_rows = 4;

	while (num_of_rows==2 && j!=-1) {
		m = strlen(string_ptr) / 3;
		m += j;
		string_ptr_2 += m;

		while (*string_ptr_2++ != ' ' && m <= letters_per_row) m++;

		if (m <= letters_per_row) {
			/* if_4 */
			n = (letters_per_row - m + 1) >> 1;
			while (n) {
				*char_buf++ = ' ';
				n--;
			}
			strncpy(char_buf, string_ptr, m);
			char_buf += m;
			*char_buf++= 10;

			string_ptr = string_ptr_2;
			string_ptr_2 += m;

			while (*string_ptr_2-- != ' ' && m > 0) m--;
			/* while_6_end */

			string_ptr_2 += 2;

			if (strlen(string_ptr_2) <= m && m>0) {
				/* if_6 */
				n = (letters_per_row - m + 1) >> 1;
				while (n) {
					*char_buf++ = ' ';
					n--;
				}
				strncpy(char_buf, string_ptr, m);
				char_buf += m;
				*char_buf++ = 10;
				height += 20;
				threeval_b -= 20;

				if (threeval_b < 2) threeval_b = 2;
				j = -1;
			} else {
				/* else_6 */
				j += 2;
				string_ptr_2 = string_ptr_3;
				string_ptr = string_ptr_3;
				char_buf = print_str_buf;
			}
		} else {
			num_of_rows = 3;
			string_ptr_2 = string_ptr_3;
			string_ptr = string_ptr_3;
			char_buf = print_str_buf;
			j = 0;
		}
	}

	if (j!=-1 && width*40 > 8000)
		num_of_rows = 4;

	/* while_8 */
	while (num_of_rows==3 && j!=-1) {
		m = strlen(string_ptr) >> 2;
		m += j;
		string_ptr_2 += m;
		while (*string_ptr_2++ != ' ' && m <= letters_per_row) m++;

		if (m <= letters_per_row) {
			/* if_10 */
			n = (letters_per_row - m + 1) >> 1;
			while(n) {
				*char_buf++ = ' ';
				n--;
			}
			strncpy(char_buf, string_ptr, m);
			char_buf += m;
			*char_buf++ = ' ';
			string_ptr = string_ptr_2;
			string_ptr_2 += m;
			while (*string_ptr_2-- != ' ' && m>0) m--;
			string_ptr_2 += 2;

			if (strlen(string_ptr_2) < m*2 && m>0) {
				/* if_11 */
				n = (letters_per_row - m + 1) >> 1;
				while (n) {
					*char_buf++ = ' ';
					n--;
				}
				strncpy(char_buf, string_ptr, m);
				char_buf += m;
				*char_buf++ = 10;
				string_ptr = string_ptr_2;
				string_ptr_2 += m;

				while(*string_ptr_2-- != ' ' && m>0) m--;
				string_ptr_2 += 2;

				if (strlen(string_ptr_2) <= m && m>0) {
					/* if_15 */
					n = (letters_per_row - m + 1) >> 1;
					while (n) {
						*char_buf++ = ' ';
						n--;
					}
					strncpy(char_buf, string_ptr, m);
					char_buf += m;
					*char_buf++ = ' ';
					height += 30;
					threeval_b -= 30;
					if (threeval_b < 2) threeval_b = 2;
					j = -1;
				} else {
					/* else_15 */
					j += 2;
					string_ptr_2 = string_ptr_3;
					string_ptr = string_ptr_3;
					char_buf = print_str_buf;
				}
			} else {
				/* else_11 */
				j += 2;
				string_ptr_2 = string_ptr_3;
				string_ptr = string_ptr_3;
				char_buf = print_str_buf;
			}
		} else {
			/* else_10 */
			num_of_rows = 4;
			string_ptr = string_ptr_3;
			string_ptr_2 = string_ptr_3;
			char_buf = print_str_buf;
		}
	}

	/* while_8_end */
	if (num_of_rows == 4) {
		while (strlen(string_ptr) > letters_per_row) {
			m = letters_per_row;
			string_ptr_2 += m;
			while (*string_ptr_2-- != ' ') m--;
			string_ptr_2 += 2;
			n = (letters_per_row - m + 1) >> 1;
			while (n) {
				*char_buf++ = ' ';
				n--;
			}
			strncpy(char_buf, string_ptr, m);
			char_buf += m;
			*char_buf++ = 10;
			height += 10;
			threeval_b -= 10;
			if (threeval_b < 2) threeval_b = 2;
			string_ptr = string_ptr_2;
		}
	}

	n = (letters_per_row - strlen(string_ptr_2) + 1) >> 1;
	while(n) {
		*char_buf++ = ' ';
		n--;
	}

	strcpy(char_buf, string_ptr_2);
	if (!(_game & GAME_SIMON2)) {
		o_unk_99_simon1(199 + num_1);
	} else {
		o_unk_99_simon2(2, num_1);
	}
	num_2 = num_2 * 3 + 192;

	render_string(num_1, num_2, width, height, print_str_buf);
	num_of_rows = 4;
	if (!(_bit_array[8] & 0x20))
		num_of_rows = 3;

	if (!(_game & GAME_SIMON2)) {
		start_vga_code(num_of_rows, 2, 199 + num_1, threeval_a >> 3, threeval_b, 12);
	} else {
		start_vga_code(num_of_rows, 2, num_1, threeval_a >> 3, threeval_b, 12);
	}
}

void SimonState::render_string(uint num_1, uint color, uint width, uint height, const char *txt) {
	VgaPointersEntry *vpe = &_vga_buffer_pointers[2];
	byte *src,*dst,*p,*dst_org,chr;
	uint count;

	if (num_1 >= 100) {
		num_1 -= 100;
		vpe++;
	}

	src = dst = vpe->vgaFile2;

	count = 4000;
	if (num_1 == 1)
		count *= 2;

	p = dst + num_1 * 8;

	*(uint16*)(p+4) = TO_BE_16(height);
	*(uint16*)(p+6) = TO_BE_16(width);
	dst += READ_BE_UINT32_UNALIGNED(p);

	memset(dst, 0, count);

	dst_org = dst;
	while ((chr=*txt++) != 0) {
		if (chr == 10) {
			dst_org += width * 10;
			dst = dst_org;
		} else if ((chr -= ' ') == 0) {
			dst += 6;
		} else {
			byte *img_hdr = src + 48 + chr * 4;
			uint img_height = img_hdr[2];
			uint img_width = img_hdr[3],i;
			byte *img = src + READ_LE_UINT16(img_hdr);
			byte *cur_dst = dst;

			assert(img_width > 0 && img_width < 50 && img_height>0 && img_height<50);

			do {
				for(i=0; i!=img_width; i++) {
					chr = *img++;
					if (chr) {
						if (chr == 0xF) chr = 207; else chr += color;
						cur_dst[i] = chr;
					}
				}
				cur_dst += width;
			} while(--img_height);

			dst += img_width - 1;
		}
	}

}

void SimonState::showmessage_print_char(byte chr) {
	if (chr == 12) {
		_num_letters_to_print = 0;
		_print_char_unk_1 = 0;
		print_char_helper_1(&chr, 1);
		print_char_helper_5(_fcs_ptr_1);
	} else if (chr==0 || chr==' ' || chr==10) {
		if (_print_char_unk_2 - _print_char_unk_1 >= _num_letters_to_print) {
			_print_char_unk_1 += _num_letters_to_print;
			print_char_helper_1(_letters_to_print_buf, _num_letters_to_print);
			
			if (_print_char_unk_1 == _print_char_unk_2) {
				_print_char_unk_1 = 0;
			} else {
				if (chr)
					print_char_helper_1(&chr, 1);
				if (chr==10)
					_print_char_unk_1 = 0;
				else if (chr!=0)
					_print_char_unk_1++;
			}
		} else {
			const byte newline_character = 10;
			_print_char_unk_1 = _num_letters_to_print;
			print_char_helper_1(&newline_character, 1);
			print_char_helper_1(_letters_to_print_buf, _num_letters_to_print);
			if (chr == ' ') {
				print_char_helper_1(&chr, 1);
				_print_char_unk_1++;
			} else {
				print_char_helper_1(&chr,1);
				_print_char_unk_1 = 0;
			}
		}
		_num_letters_to_print = 0;
	} else {
		_letters_to_print_buf[_num_letters_to_print++] = chr;
	}
}

void SimonState::print_char_helper_1(const byte *src, uint len) {
	uint ind;

	if (_fcs_ptr_1 == NULL)
		return;

	while (len-- != 0) {
		if (*src != 12 && _fcs_ptr_1->fcs_data!=NULL && 
			_fcs_data_1[ind=get_fcs_ptr_3_index(_fcs_ptr_1)]!=2) {

			_fcs_data_1[ind] = 2;
			_fcs_data_2[ind] = 1;
		}

		fcs_putchar(*src++);
	}
}

void SimonState::print_char_helper_5(FillOrCopyStruct *fcs) {
	uint index = get_fcs_ptr_3_index(fcs);
	print_char_helper_6(index);
	_fcs_data_1[index] = 0;
}

void SimonState::print_char_helper_6(uint i) {
	FillOrCopyStruct *fcs;

	if (_fcs_data_2[i]) {
		lock();
		fcs = _fcs_ptr_array_3[i];
		fcs_unk_proc_1(i, fcs->fcs_data->item_ptr, fcs->fcs_data->unk1, fcs->fcs_data->unk2);
		_fcs_data_2[i] = 0;
		unlock();
	}
}

void SimonState::read_vga_from_datfile_1(uint vga_id) {
	if (_game == GAME_SIMON1DOS) {
		FILE *in;
		char buf[50];
		uint32 size;

		sprintf(buf, "%.3d%d.VGA", vga_id>>1, (vga_id&1)+1);

		in = fopen_maybe_lowercase(buf);
		if (in==NULL)
			error("read_vga_from_datfile_1: cannot open %s", buf);

		fseek(in, 0, SEEK_END);
		size = ftell(in);
		rewind(in);

		if (fread(_vga_buffer_pointers[11].vgaFile2, size, 1, in) != 1)
			error("read_vga_from_datfile_1: read failed");

		fclose(in);
	} else {
		uint32 offs_a = _game_offsets_ptr[vga_id];
		uint32 size = _game_offsets_ptr[vga_id + 1] - offs_a;

		resfile_read(_vga_buffer_pointers[11].vgaFile2, offs_a, size);
	}
}

byte *SimonState::read_vga_from_datfile_2(uint id) {
	if (_game == GAME_SIMON1DOS) {
		FILE *in;
		char buf[50];
		uint32 size;
		byte *dst;

		sprintf(buf, "%.3d%d.VGA", id>>1, (id&1)+1);

		in = fopen_maybe_lowercase(buf);
		if (in==NULL)
			error("read_vga_from_datfile_2: cannot open %s", buf);

		fseek(in, 0, SEEK_END);
		size = ftell(in);
		rewind(in);

		dst = setup_vga_destination(size);

		if (fread(dst, size, 1, in) != 1)
			error("read_vga_from_datfile_2: read failed");

		fclose(in);

		return dst;
	} else {
		uint32 offs_a = _game_offsets_ptr[id];
		uint32 size = _game_offsets_ptr[id + 1] - offs_a;
		byte *dst;

		dst = setup_vga_destination(size);
		resfile_read(dst, offs_a, size);

		return dst;
	}
}

void SimonState::resfile_read(void *dst, uint32 offs, uint32 size) {
	if (fseek(_game_file, offs, SEEK_SET) != 0)
		error("resfile_read(%d,%d) seek failed", offs, size);
	if (fread(dst, size, 1, _game_file)!=1)
		error("resfile_read(%d,%d) read failed", offs, size);
}


void SimonState::openGameFile() {
	if (_game != GAME_SIMON1DOS) {
		_game_file = fopen_maybe_lowercase(gss->gme_filename);

		if (_game_file==NULL)
			error("cannot open game file '%s'", gss->gme_filename);

		_game_offsets_ptr = (uint32*)malloc(gss->NUM_GAME_OFFSETS*sizeof(uint32));
		if (_game_offsets_ptr == NULL)
			error("out of memory, game offsets");

		resfile_read(_game_offsets_ptr, 0, gss->NUM_GAME_OFFSETS*sizeof(uint32));
	}

	loadIconFile();
	
	_system->init_size(320,200);
	
	startUp(1);
}

void SimonState::startUp(uint a) {
	if (a == 1)
		startUp_helper();
}

void SimonState::startUp_helper() {
	runSubroutine101();
	startUp_helper_2();
}	

void SimonState::runSubroutine101() {
	Subroutine *sub;

	sub = getSubroutineByID(101);
	if (sub != NULL)
		startSubroutineEx(sub);
	
	startUp_helper_2();
}

#if 0
void SimonState::generateSound(byte *ptr, int len) {
	uint cur;

	cur = _voice_size;
	if (cur >	(uint)len) cur=(uint)len;
	_voice_size -= cur;

	if (cur!=0) {
		fread(ptr, cur, 1, _voice_file);
	}

	memset(ptr + cur, 0x80, len - cur);

	cur = _sound_size;
	if (cur) {
		uint i;

		if (cur > (uint)len) cur = (uint)len;

		for(i=0;i!=cur;i++) {
			ptr[i] += _sound_ptr[i] ^0x80;
		}
		
		_sound_size -= cur;
		_sound_ptr += cur;
	}
}
#endif

//static void fill_sound(void *userdata, int16 *stream, int len) {
//	((SimonState*)userdata)->generateSound((byte*)stream, len*2);
//}

void SimonState::dx_copy_rgn_from_3_to_2(uint b, uint r, uint y, uint x) {
	byte *dst, *src;
	uint i;
	
	dst = dx_lock_2();
	src = sdl_buf_3;

	dst += y * _dx_surface_pitch;
	src += y * _dx_surface_pitch;

	while (y < b) {
		for(i=x; i<r; i++)
			dst[i] = src[i];
		y++;
		dst += _dx_surface_pitch;
		src += _dx_surface_pitch;
	}
	
	dx_unlock_2();
}

void SimonState::dx_clear_surfaces(uint num_lines) {
	memset(sdl_buf_attached, 0, num_lines*320);
	
	_system->copy_rect(sdl_buf_attached, 320, 0, 0, 320, 200);

	if (_dx_use_3_or_4_for_lock) {
		memset(sdl_buf, 0, num_lines*320);
		memset(sdl_buf_3, 0, num_lines*320);
	}
}

void SimonState::dx_clear_attached_from_top(uint lines) {
	memset(sdl_buf_attached, 0, lines*320);
}

void SimonState::dx_copy_from_attached_to_2(uint x, uint y, uint w, uint h) {
	uint offs = x + y*320;
	byte *s = sdl_buf_attached + offs;
	byte *d = sdl_buf + offs;
	
	do {
		memcpy(d,s,w);
		d+=320;
		s+=320;
	} while(--h);
}

void SimonState::dx_copy_from_2_to_attached(uint x, uint y, uint w, uint h) {
	uint offs = x + y*320;
	byte *s = sdl_buf + offs;
	byte *d = sdl_buf_attached + offs;
	
	do {
		memcpy(d,s,w);
		d+=320;
		s+=320;
	} while(--h);
}



void SimonState::dx_copy_from_attached_to_3(uint lines) {
	memcpy(sdl_buf_3, sdl_buf_attached, lines*320);
}

void SimonState::dx_update_screen_and_palette() {
	_num_screen_updates++;

	if (_palette_color_count == 0 && _video_var_9==1) {
		_video_var_9 = 0;
		if (memcmp(_palette,_palette_backup,256*4)!=0) {
			memcpy(_palette_backup, _palette, 256*4);
			_system->set_palette(_palette, 0, 256);
		}
	}

	if (!_fast_mode || !(rand()&7)) {
		
		if (_mouse_pos_changed) {
			_mouse_pos_changed = false;
			_system->set_mouse_pos(sdl_mouse_x, sdl_mouse_y);
		}
		_system->copy_rect(sdl_buf_attached, 320, 0, 0, 320, 200);
		_system->update_screen();
	}

	memcpy(sdl_buf_attached, sdl_buf, 320*200);

	if (_palette_color_count != 0) {
		if (!(_game&GAME_SIMON2) && _use_palette_delay) {
			delay(100);
			_use_palette_delay = false;
		}
		realizePalette();
	}
}


void SimonState::realizePalette() {
	if (_palette_color_count&0x8000) {
		error("_palette_color_count&0x8000");
	}
	_video_var_9 = false;
	memcpy(_palette_backup, _palette, 256*4);

	_system->set_palette(_palette, 0, _palette_color_count);
	_palette_color_count = 0;

}


void SimonState::go() {
	if (!_dump_file)
		_dump_file = stdout;

	/* allocate buffers */
	sdl_buf_3 = (byte*)calloc(320*200,1);
	sdl_buf = (byte*)calloc(320*200,1);
	sdl_buf_attached = (byte*)calloc(320*200,1);

	if (_game & GAME_SIMON2) {
		gss = &simon2_settings;
	} else {
		gss = &simon1_settings;
	}

	allocItemHeap();
	allocTablesHeap();

	setup_vga_file_buf_pointers();

	initSound();
	
	if (!loadGamePcFile(gss->gamepc_filename))
		error("Error loading gamepc file '%s' (or one of the files it depends on)", gss->gamepc_filename);

	addTimeEvent(0, 1);
	openGameFile();

	_last_music_played = (uint)-1;
	_vga_base_delay = 1;
	_vk_t_toggle = true;

	_system->property(OSystem::PROP_SHOW_DEFAULT_CURSOR, 1);

	while(1) {
		hitarea_stuff();
		handle_verb_clicked(_verb_hitarea);
		delay(100);
	}
}

void SimonState::shutdown() {
	if (_game_file) {
		fclose(_game_file);
		_game_file = NULL;
	}
}

void SimonState::delay(uint delay) {
		OSystem::Event event;

	uint32 start = _system->get_msecs();
	uint32 cur = start;
	const uint vga_period = _fast_mode ? 10 : 50;

	do {
		while (!_in_callback && cur >= _last_vga_tick + vga_period) {
			_last_vga_tick += vga_period;
			
			/* don't get too many frames behind */
			if (cur >= _last_vga_tick + vga_period*2) 
				_last_vga_tick = cur;

			_in_callback = true;
			timer_callback();
			_in_callback = false;
		}

		while (_system->poll_event(&event)) {
			switch(event.event_code) {
			case OSystem::EVENT_KEYDOWN:
				if (event.kbd.keycode=='t') {
					_vk_t_toggle ^= 1;
				} else if (event.kbd.flags==OSystem::KBD_CTRL) {
					if (event.kbd.keycode=='f') {
						_fast_mode^=1;
					}
				}
				_key_pressed = (byte)event.kbd.ascii;
				break;
			case OSystem::EVENT_MOUSEMOVE:
				sdl_mouse_x = event.mouse.x;
				sdl_mouse_y = event.mouse.y;
				_mouse_pos_changed = true;
				break;

			case OSystem::EVENT_LBUTTONDOWN:
				_left_button_down++;
				break;

			case OSystem::EVENT_RBUTTONDOWN:
				_exit_cutscene = true;
				break;
			}
		}

		if (delay==0) break;

		{
			uint this_delay = _fast_mode ? 1 : 20;
			if (this_delay > delay) this_delay = delay;
			_system->delay_msecs(this_delay);
		}
		cur = _system->get_msecs();
	} while (cur < start + delay);
}


bool SimonState::save_game(uint slot, const char *caption) {
	FILE *f;
	uint item_index, num_item, i;
	TimeEvent *te;

	_lock_word |= 0x100;
	
	errno = 0;
	
	f = fopen(gen_savename(slot), "wb");
	if (f==NULL)
		return false;

	fwrite(caption, 1, 0x12, f);

	fileWriteBE32(f, _itemarray_inited-1);
	fileWriteBE32(f, 0xFFFFFFFF);
	fileWriteBE32(f, 0);
	fileWriteBE32(f, 0);

	i=0;
	for(te = _first_time_struct; te; te = te->next)
		i++;

	fileWriteBE32(f, i);
	for(te = _first_time_struct; te; te = te->next) {
		fileWriteBE32(f, te->time + _base_time);
		fileWriteBE16(f, te->subroutine_id);
	}

	item_index = 1;
	for(num_item = _itemarray_inited-1; num_item; num_item--) {
		Item *item = _itemarray_ptr[item_index++];

		fileWriteBE16(f, item->parent);
		fileWriteBE16(f, item->sibling);
		fileWriteBE16(f, item->unk3);
		fileWriteBE16(f, item->unk4);

		{
			Child1 *child1 = findChildOfType1(item);
			if (child1) {
				fileWriteBE16(f, child1->fr2);
			}
		}

		{
			Child2 *child2 = findChildOfType2(item);
			uint i,j;

			if (child2) {
				fileWriteBE32(f, child2->avail_props);
				i = child2->avail_props&1;

				for(j=1; j<16; j++) {
					if ((1<<j) & child2->avail_props) {
						fileWriteBE16(f, child2->array[i++]);
					}
				}
			}
		}

		{
			Child9 *child9 = (Child9*)findChildOfType(item, 9);
			if (child9) {
				uint i;
				for(i=0; i!=4; i++) {
					fileWriteBE16(f, child9->array[i]);
				}
			}
		}
	}

	/* write the 255 variables */
	for(i=0; i!=255; i++) {
		fileWriteBE16(f, readVariable(i));
	}

	/* write the items in array 6 */
	for(i=0; i!=10; i++) {
		fileWriteBE16(f, itemPtrToID(_item_array_6[i]));
	}

	/* Write the bits in array 1 & 2*/
	for(i=0; i!=32; i++)
		fileWriteBE16(f, _bit_array[i]);
	
	fclose(f);

	_lock_word &= ~0x100;

	return true;
}

char *SimonState::gen_savename(int slot) {
	static char buf[128];
	sprintf(buf, "SAVE.%.3d", slot);
	return buf;
}

bool SimonState::load_game(uint slot) {
	char ident[18];
	FILE *f;
	uint num, item_index, i;
	
	_lock_word |= 0x100;

	errno = 0;

	f = fopen_maybe_lowercase(gen_savename(slot));
	if (f==NULL)
		return false;

	fread(ident, 1, 18, f);
	
	num = fileReadBE32(f);

	if (fileReadBE32(f) != 0xFFFFFFFF || num != _itemarray_inited-1) {
		fclose(f);
		return false;
	}

	fileReadBE32(f);
	fileReadBE32(f);

	_no_parent_notify = true;

	
	/* add all timers */
	killAllTimers();
	for(num = fileReadBE32(f);num;num--) {
		uint32 timeout = fileReadBE32(f);
		uint16 func_to_call = fileReadBE16(f);
		addTimeEvent(timeout, func_to_call);
	}

	item_index = 1;
	for(num=_itemarray_inited-1; num; num--) {
		Item *item = _itemarray_ptr[item_index++], *parent_item;

		uint parent = fileReadBE16(f);
		uint sibling = fileReadBE16(f);

		parent_item = derefItem(parent);

		setItemParent(item, parent_item);

		if (parent_item == NULL) {
			item->parent = parent;
			item->sibling = sibling;
		}

		item->unk3 = fileReadBE16(f);
		item->unk4 = fileReadBE16(f);

		{
			Child1 *child1 = findChildOfType1(item);
			if (child1 != NULL) {
				child1->fr2 = fileReadBE16(f);
			}
		}

		{
			Child2 *child2 = findChildOfType2(item);
			uint i,j;
			if (child2 != NULL) {
				child2->avail_props = fileReadBE32(f);
				i = child2->avail_props&1;

				for(j=1; j<16; j++) {
					if ((1<<j) & child2->avail_props) {
						child2->array[i++] = fileReadBE16(f);
					}
				}
			}
		}

		{
			Child9 *child9 = (Child9*)findChildOfType(item, 9);
			if (child9) {
				uint i;
				for(i=0; i!=4; i++) {
					child9->array[i] = fileReadBE16(f);
				}
			}
		}
	}

	
	/* read the 255 variables */
	for(i=0; i!=255; i++) {
		writeVariable(i, fileReadBE16(f));
	}

	/* write the items in array 6 */
	for(i=0; i!=10; i++) {
		_item_array_6[i] = derefItem(fileReadBE16(f));
	}

	/* Write the bits in array 1 & 2*/
	for(i=0; i!=32; i++)
		_bit_array[i] = fileReadBE16(f);
	
	fclose(f);

	_no_parent_notify = false;

	_lock_word &= ~0x100;

	if (errno != 0)
		error("load failed");

	return true;
}

void SimonState::initSound() {
	/* only read voice file in windows game */
	if (_game & GAME_WIN) {
		const char *s = gss->wav_filename;

		_voice_offsets = NULL;

		_voice_file = fopen_maybe_lowercase(s);
		if (_voice_file == NULL) {
			warning("Cannot open %s",s);
			return;
		}

		_voice_offsets = (uint32*)malloc(gss->NUM_VOICE_RESOURCES * sizeof(uint32));
		if (_voice_offsets == NULL)
			error("Out of memory for voice offsets");

		if (fread(_voice_offsets, gss->NUM_VOICE_RESOURCES * sizeof(uint32), 1, _voice_file) != 1)
			error("Cannot read voice offsets");
	}
}

struct WaveHeader {
	uint32 riff;
	uint32 unk;
	uint32 wave;
	uint32 fmt;

  uint32 size;

	uint16 format_tag;
	uint16 channels;
	uint32 samples_per_sec;
	uint32 avg_bytes;

	uint16 block_align;
	uint16 bits_per_sample;
};

void SimonState::playVoice(uint voice) {
	WaveHeader wave_hdr;
	uint32 data[2];

//	assert(voice < 14496/4);

	if (_voice_offsets == NULL)
		return;

	_mixer->stop(_voice_sound);

	fseek(_voice_file, _voice_offsets[voice], SEEK_SET);

	if (fread(&wave_hdr, sizeof(wave_hdr), 1, _voice_file)!=1 ||
		wave_hdr.riff!='FFIR' || wave_hdr.wave!='EVAW' || wave_hdr.fmt!=' tmf' ||
		wave_hdr.format_tag!=1 || wave_hdr.channels!=1 || wave_hdr.bits_per_sample!=8) {
			warning("playVoice(%d): cannot read RIFF header", voice);
			return;
		}

	fseek(_voice_file, wave_hdr.size - sizeof(wave_hdr) + 20, SEEK_CUR);

	if (fread(data, sizeof(data), 1, _voice_file) != 1 ||
		data[0] != 'atad' ) {
			warning("playVoice(%d): cannot read data header",voice);
			return;
		}

	_mixer->play_raw(&_voice_sound, _voice_file, data[1], wave_hdr.samples_per_sec, 
		SoundMixer::FLAG_FILE|SoundMixer::FLAG_UNSIGNED);
}


void SimonState::playSound(uint sound) {
	if (_game & GAME_WIN) {
		byte *p;
		
		_mixer->stop(_playing_sound);

		/* Check if _sfx_heap is NULL */
		if (_sfx_heap == NULL) {
			warning("playSound(%d) cannot play. No voice file loaded", sound);
			return;
		}
		
		p = _sfx_heap + ((uint32*)_sfx_heap)[sound];

		for(;;) {
			p = (byte*)memchr(p, 'd', 1000);
			if (!p) {
				error("playSound(%d): didn't find", sound);
				return;
			}
			if (p[1]=='a' && p[2]=='t' && p[3]=='a')
				break;

			p++;
		}

		_mixer->play_raw(&_playing_sound, p+8,*(uint32*)(p+4),22050,SoundMixer::FLAG_UNSIGNED);
	} else {
		warning("playSound(%d)", sound);
	}
}

void SimonState::playMusic(uint music) {
	FILE *f;

	midi.shutdown();

	/* FIXME: not properly implemented */
	if (_game & GAME_WIN) {
		fseek(_game_file, _game_offsets_ptr[gss->MUSIC_INDEX_BASE + music],SEEK_SET);
		f = _game_file;
	
		midi.read_all_songs(f);
	} else {
		char buf[50];
		sprintf(buf, "MOD%d.MUS", music);
		f = fopen_maybe_lowercase(buf);
		if (f==NULL) {
			warning("Cannot load music from '%s'", buf);
			return;
		}
		midi.read_all_songs_old(f);
		fclose(f);
	}

	midi.initialize();
	midi.play();
}

byte *SimonState::dx_lock_2() {
	_dx_surface_pitch = 320;
	return sdl_buf;
}

void SimonState::dx_unlock_2() {
}

byte *SimonState::dx_lock_attached() {
	_dx_surface_pitch = 320;
	return _dx_use_3_or_4_for_lock ? sdl_buf_3 : sdl_buf_attached;
}

void SimonState::dx_unlock_attached() {
}



/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/

void decompressIcon(byte *dst, byte *src, uint pitch, byte base) {
	int8 reps = (int8)0x80;
	byte color_1, color_2;
	byte *dst_org = dst;
	uint h = 12, w = 24;

	for(;;) {
		reps = *src++;
		if (reps < 0) {
			color_1 = *src & 0xF;
			if (color_1 != 0) color_1 += base;
			color_2 = *src++ >> 4;
			if (color_2 != 0) color_2 += base;
			
			do {	
				if (color_1 != 0) *dst = color_1;
				dst += pitch;
				if (color_2 != 0) *dst = color_2;
				dst += pitch;

				/* reached bottom? */
				if (--h == 0) {
					/* reached right edge? */				
					if (--w == 0)
						return;
					dst = ++dst_org;
					h = 12;
				}
			} while (++reps != 0);
		} else {
			
			do {	
				color_1 = *src & 0xF;
				if (color_1 != 0) *dst = color_1 | base;
				dst += pitch;

				color_2 = *src++ >> 4;
				if (color_2 != 0) *dst = color_2 | base;
				dst += pitch;

				/* reached bottom? */
				if (--h == 0) {
					/* reached right edge? */				
					if (--w == 0)
						return;
					dst = ++dst_org;
					h = 12;
				}
			} while (--reps >= 0);
		}
	}
}

#define SIMON2
#define SIMON2WIN

static const char * const opcode_name_table[256] = {
	/* 0 */
	"|INV_COND",
	"IJ|PTRA_PARENT_IS",
	"IJ|PTRA_PARENT_ISNOT",
	NULL,
	/* 4 */
	NULL,
	"IJ|PARENT_IS_1",
	"IJ|PARENT_ISNOT_1",
	"IIJ|PARENT_IS",
	/* 8 */
	NULL,
	NULL,
	NULL,
	"VJ|IS_ZERO",
	/* 12 */
	"VJ|ISNOT_ZERO",
	"VWJ|IS_EQ",
	"VWJ|IS_NEQ",
	"VWJ|IS_LE",
	/* 16 */
	"VWJ|IS_GE",
	"VVJ|IS_EQF",
	"VVJ|IS_NEQF",
	"VVJ|IS_LEF",
	/* 20 */
	"VVJ|IS_GEF",
	NULL,
	NULL,
	"WJ|UNK23",
	/* 24 */
	NULL,
	"IJ|HAS_CHILD_1",
	"IJ|HAS_CHILD_2",
	"IWJ|ITEM_UNK3_IS",
	/* 28 */
	"IBJ|CHILD_HAS_FLAG",
	NULL,
	NULL,
	"I|SET_NO_PARENT",
	/* 32 */
	NULL,
	"II|SET_PARENT",
	NULL,
	NULL,
	/* 36 */
	"VV|MOVE",
	NULL,
	NULL,
	NULL,
	/* 40 */
	NULL,
	"V|ZERO",
	"VW|SET",
	"VW|ADD",
	/* 44 */
	"VW|SUB",
	"VV|ADDF",
	"VV|SUBF",
	"VW|MUL",
	/* 48 */
	"VW|DIV",
	"VV|MULF",
	"VV|DIVF",
	"VW|MOD",
	/* 52 */
	"VV|MODF",
	"VW|RANDOM",
	NULL,
	"I|SET_A_PARENT",
	/* 56 */
	"IB|SET_CHILD2_BIT",
	"IB|CLEAR_CHILD2_BIT",
	"II|MAKE_SIBLING",
	"I|INC_UNK3",
	/* 60 */
	"I|DEC_UNK3",
	"IW|SET_UNK3",
	"V|SHOW_INT",
	"T|SHOW_STRING_NL",
	/* 64 */
	"T|SHOW_STRING",
	"WWWWWB|ADD_HITAREA",
	"BT|SET_ITEM_NAME",
#if defined SIMON1WIN || defined SIMON2
	"BTw|SET_ITEM_DESC",
#endif
#ifdef SIMON1DOS
	"BT|SET_ITEM_DESC",
#endif
	/* 68 */
	"x|HALT",
	"x|RET1",
	"V|SHOW_STRING_AR3",
	"W|START_SUB",
	/* 72 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 76 */
	"WW|ADD_TIMEOUT",
	"J|IS_M1_EMPTY",
	"J|IS_M3_EMPTY",
	"ITJ|CHILD_FR2_IS",
	/* 80 */
	"IIJ|IS_ITEM_EQ",
	NULL,
	"B|UNK82",
	"|RETM10",
	/* 84 */
	NULL,
	NULL,
	NULL,
	"W|UNK87",
	/* 88 */
	"|OR_SCRIPT_WORD_10",
	"|AND_SCRIPT_WORD_10",
	"IB|SET_M_TO_PARENT",
	"IB|SET_M_TO_SIBLING",
	/* 92 */
	"IB|SET_M_TO_CHILD",
	NULL,
	NULL,
	NULL,
	/* 96 */
	"WB|UNK96",
	"W|LOAD_VGA",
#ifdef SIMON2
	"WWBWWW|START_VGA",
#else
	"WBWWW|START_VGA",
#endif
#ifdef SIMON2
	"WW|KILL_THREAD",
#else
	"W|KILL_THREAD",
#endif
	/* 100 */
	"|VGA_RESET",
	"BWWWWWW|UNK101",
	"B|UNK102",
	"|UNK103",
	/* 104 */
	"B|UNK104",
	NULL,
	NULL,
	"WWWWWIW|ADD_ITEM_HITAREA",
	/* 108 */
	"W|DEL_HITAREA",
	"W|CLEAR_HITAREA_0x40",
	"W|SET_HITAREA_0x40",
	"WWW|SET_HITAREA_XY",
	/* 112 */
	NULL,
	NULL,
	"IB|UNK114",
	"IBJ|HAS_FLAG",
	/* 116 */
	"IB|SET_FLAG",
	"IB|CLEAR_FLAG",
	NULL,
	"W|WAIT_VGA",
	/* 120 */
	"W|UNK120",
	"BI|SET_VGA_ITEM",
	NULL,
	NULL,
	/* 124 */
	NULL,
	"IJ|IS_SIBLING_WITH_A",
	"IBB|UNK126",
	"WW|UNK127",
	/* 128 */
	"W|GET_DUMMY_WORD",
	"W|GET_WORD_COND_TRUE",
	"Bww|UNK131",
	NULL,	/* opcode 131 doesn't exist */
	/* 132 */
	"|SAVE_GAME",
	"|LOAD_GAME",
	"|DUMMYPROC_134",
	"|QUIT_IF_USER_PRESSES_Y",
	/* 136 */
	"IV|GET_ITEM_UNK3",
	"B|UNK137",
	"|VGA_POINTER_OP_4",
	"II|SET_PARENT_SPECIAL",
	/* 140 */
	"|DEL_TE_AND_ADD_ONE",
	"BI|SET_M1_OR_M3",
	"WJ|IS_HITAREA_0x40_CLEAR",
	"I|START_ITEM_SUB",
	/* 144 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 148 */
	NULL,
	NULL,
	NULL,
	"BI|SET_ARRAY6_TO",
	/* 152 */
	"BB|SET_M1_M3_TO_ARRAY6",
	"B|SET_BIT",
	"B|CLEAR_BIT",
	"BJ|IS_BIT_CLEAR",
	/* 156 */
	"BJ|IS_BIT_SET",
	"IBB|GET_ITEM_PROP",
	"IBW|SET_ITEM_PROP",
	NULL,
	/* 160 */
	"B|UNK160",
	"BWBW|SETUP_TEXT",
#if defined SIMON1WIN || defined SIMON2
	"BBTW|PRINT_STR",
#endif
#ifdef SIMON1DOS
	"BBT|PRINT_STR",
#endif
	"W|SOUND_1",
	/* 164 */
	"|UNK164",
	"IWWJ|ITEM_UNK1_UNK2_IS",
	"B|SET_BIT2",
	"B|CLEAR_BIT2",
	/* 168 */
	"BJ|IS_BIT2_CLEAR",
	"BJ|IS_BIT2_SET",
	NULL,
	NULL,
	/* 172 */
	NULL,
	NULL,
	NULL,
	"|VGA_POINTER_OP_1",
	/* 176 */
	"|VGA_POINTER_OP_2",
	"BBI|UNK177",
	"WWBB|PATHFIND",
	"BBB|UNK179",
	/* 180 */
	"|FORCE_UNLOCK",
	"|FORCE_LOCK",
	"|READ_VGARES_328",
	"|READ_VGARES_23",
	/* 184 */
	"W|CLEAR_VGAPOINTER_ENTRY",
	"W|DUMMY_185",
	"|VGA_POINTER_OP_3",
	"|FADE_TO_BLACK",
#ifdef SIMON2
	/* 188 */
	"BSJ|STRING2_IS",
	"|UNK189",
	"B|UNK190",
#endif
};

byte *SimonState::dumpOpcode(byte *p) {
	byte opcode;	
	const char *s, *st;

	opcode = *p++;
	if (opcode == 255)
		return NULL;
	st = s = opcode_name_table[opcode];
	if (s == NULL) {
		error("INVALID OPCODE %d\n", opcode);
		return NULL;
	}
	while (*st != '|') st++;
	fprintf(_dump_file,"%s ", st+1);

	for(;;) {
		switch(*s++) {
		case 'x':
			fprintf(_dump_file,"\n");
			return NULL;
		case '|':
			fprintf(_dump_file,"\n");
			return p;
		case 'B': {
			byte b = *p++;
			if (b==255)
				fprintf(_dump_file,"[%d] ", *p++);
			else
				fprintf(_dump_file,"%d ", b);
			break;
		}
		case 'V': {
			byte b = *p++;
			if (b==255)
				fprintf(_dump_file,"[[%d]] ", *p++);
			else
				fprintf(_dump_file,"[%d] ", b);
			break;
		}

		case 'W': {
			int n = (int16)((p[0]<<8)|p[1]);
			p+=2;
			if (n>=30000 && n<30512)
				fprintf(_dump_file,"[%d] ", n - 30000);
			else
				fprintf(_dump_file,"%d ", n);
			break;
		}

		case 'w': {
			int n = (int16)((p[0]<<8)|p[1]);
			p+=2;
			fprintf(_dump_file,"%d ", n);
			break;
		}

		case 'I': {
			int n = (int16)((p[0]<<8)|p[1]);;
			p+=2;
			if (n == -1)
				fprintf(_dump_file,"ITEM_M1 ");
			else if (n == -3)
				fprintf(_dump_file,"ITEM_M3 ");
			else if (n == -5)
				fprintf(_dump_file,"ITEM_1 ");
			else if (n == -7)
				fprintf(_dump_file,"ITEM_0 ");
			else if (n == -9)
				fprintf(_dump_file,"ITEM_A_PARENT ");
			else
				fprintf(_dump_file,"<%d> ", n);
			break;
		}
		case 'J': {
			fprintf(_dump_file,"-> ");
		} break;


		case 'T': {
			uint n = ((p[0]<<8)|p[1]);
			p+=2;
			if (n != 0xFFFF)
				fprintf(_dump_file,"\"%s\"(%d) ", getStringPtrByID(n), n);
			else
				fprintf(_dump_file,"NULL_STRING ");
		} break;
		}
	}
}

void SimonState::dumpSubroutineLine(SubroutineLine *sl, Subroutine *sub) {
	byte *p;
	
	
	printf("; ****\n");

	p = (byte*)sl + SUBROUTINE_LINE_SMALL_SIZE;
	if (sub->id == 0) {
		fprintf(_dump_file,"; cond_a=%d, cond_b=%d, cond_c=%d\n", sl->cond_a, sl->cond_b, sl->cond_c);
		p = (byte*)sl + SUBROUTINE_LINE_BIG_SIZE;
	}

	for(;;) {
		p = dumpOpcode(p);
		if (p==NULL)
			break;
	}
}

void SimonState::dumpSubroutine(Subroutine *sub) {
	SubroutineLine *sl;

	fprintf(_dump_file,"\n******************************************\n;Subroutine, ID=%d:\nSUB_%d:\n", sub->id, sub->id);
	sl = (SubroutineLine*) ((byte*)sub + sub->first);
	for(;(byte*)sl != (byte*)sub; sl = (SubroutineLine*) ((byte*)sub + sl->next) ) {
		dumpSubroutineLine(sl, sub);
	}
	fprintf(_dump_file,"\nEND ******************************************\n");
	fflush(_dump_file);
}

void SimonState::dumpSubroutines() {
	Subroutine *sub = _subroutine_list;
	for(;sub;sub = sub->next) {
		dumpSubroutine(sub);
	}
}

const char * const video_opcode_name_table[] = {
	/* 0 */
	"x|RET",
	"ddd|DUMMY",
	"d|CALL",
	"ddddd|NEW_THREAD",
	/* 4 */
	"ddd|DUMMY_2",
	"vd|SKIP_IF_NEQ",
	"d|SKIP_IFN_SIB_WITH_A",
	"d|SKIP_IF_SIB_WITH_A",
	/* 8 */
	"dd|SKIP_IF_PARENT_IS",
	"dd|SKIP_IF_UNK3_IS",
#ifdef SIMON2
	"ddddb|DRAW",
#else
	"ddddd|DRAW",
#endif
	"|CLEAR_PATHFIND_ARRAY",
	/* 12 */
#ifdef SIMON2
	"b|DELAY",
#else
	"d|DELAY",
#endif
	"d|OFFSET_X",
	"d|OFFSET_Y",
	"d|IDENT_WAKEUP",
	/* 16 */
	"d|IDENT_SLEEP",
	"dq|SET_PATHFIND_ITEM",
	"i|JUMP_REL",
	"|CHAIN_TO",
	/* 20 */
	"dd|SET_CODE_WORD",
	"i|JUMP_IF_CODE_WORD",
	"dd|SET_PAL",
	"d|SET_PRI",
	/* 24 */
	"diid|SET_IMG_XY",
	"x|HALT_THREAD",
	"ddddd|SET_WINDOW",
	"|RESET",
	/* 28 */
	"dddd|DUMMY_3",
	"|STOP_ALL_SOUNDS",
	"d|SET_BASE_DELAY",
	"d|SET_PALETTE_MODE",
	/* 32 */
	"vv|COPY_VAR",
	"|FORCE_UNLOCK",
	"|FORCE_LOCK",
	"dd|DUMMY_4",
	/* 36 */
	"dd|SAVELOAD_THING",
	"v|OFFSET_Y_F",
	"v|SKIP_IF_VAR_ZERO",
	"vd|SET_VAR",
	/* 40 */
	"vd|ADD_VAR",
	"vd|SUB_VAR",
	"vd|SLEEP_UNTIL_SET",
	"d|SKIP_IF_BIT_CLEAR",
	/* 44 */
	"d|SKIP_IF_BIT_SET",	
	"v|SET_X_F",
	"v|SET_Y_F",
	"vv|ADD_VAR_F",
	/* 48 */
	"|VC_48",
	"d|SET_BIT",
	"d|CLEAR_BIT",
	"d|CLEAR_HITAREA_BIT_0x40",
	/* 52 */
	"d|VC_52",
	"dd|DUMMY_5",
	"ddd|DUMMY_6",
	"ddd|OFFSET_HIT_AREA",
	/* 56 */
#ifdef SIMON2
	"i|SLEEP_EX",
#else
	"|DUMMY_7",
#endif
	"|DUMMY_8",
	"|DUMMY_9",
#ifdef SIMON2
	"ddd|KILL_MULTI_THREAD",
#else
	"|SKIP_IF_SOUND??",
#endif
	/* 60 */
#ifdef SIMON2
	"dd|KILL_THREAD",
#else
	"d|KILL_THREAD",
#endif
	"ddd|INIT_SPRITE",
	"|PALETTE_THING",
	"|PALETTE_THING_2",
#ifdef SIMON2
	/* 64 */
	"|UNK64",
	"|UNK65",
	"|UNK66",
	"|UNK67",
	/* 68 */
	"|UNK68",
	"dd|UNK69",
	"dd|UNK70",
	"|UNK71",
	/* 72 */
	"dd|UNK72",
	"bb|UNK73",
	"bb|UNK74",
#endif
};

void SimonState::dump_video_script(byte *src, bool one_opcode_only) {
	uint opcode;
	const char *str, *strn;

	do {
		if (!(_game & GAME_SIMON2)) {
			opcode = READ_BE_UINT16_UNALIGNED(src);
			src+=2;
		} else {
			opcode = *src++;
		}

		if (opcode >= gss->NUM_VIDEO_OP_CODES) {
			error("Invalid opcode %x\n", opcode);
			return;
		}

		strn = str = video_opcode_name_table[opcode];
		while (*strn != '|') strn++;
		fprintf(_dump_file,"%.2d: %s ", opcode, strn + 1);

		for (;*str != '|';str++) {
			switch(*str) {
			case 'x': fprintf(_dump_file,"\n"); return;
			case 'b':	fprintf(_dump_file,"%d ", *src++); break;
			case 'd':	fprintf(_dump_file,"%d ", READ_BE_UINT16_UNALIGNED(src)); src+=2; break;
			case 'v':	fprintf(_dump_file,"[%d] ", READ_BE_UINT16_UNALIGNED(src)); src+=2; break;
			case 'i':	fprintf(_dump_file,"%d ", (int16)READ_BE_UINT16_UNALIGNED(src)); src+=2; break;
			case 'q': 
				while (READ_BE_UINT16_UNALIGNED(src) != 999) {
					fprintf(_dump_file,"(%d,%d) ", READ_BE_UINT16_UNALIGNED(src), READ_BE_UINT16_UNALIGNED(src+2));
					src += 4;
				}
				src++;
				break;
			default:
				error("Invalid fmt string '%c' in decompile VGA", *str);
			}
		}

		fprintf(_dump_file,"\n");
	} while(!one_opcode_only);
}

void SimonState::dump_vga_file(byte *vga) {
	{
		byte *pp;
		byte *p;
		int count;

		pp = vga;
		p = pp + READ_BE_UINT16_UNALIGNED(&((VgaFile1Header*)pp)->hdr2_start);
		count = READ_BE_UINT16_UNALIGNED(&((VgaFile1Header2*)p)->id_count);
		p = pp + READ_BE_UINT16_UNALIGNED(&((VgaFile1Header2*)p)->id_table);
		while (--count >= 0) {
			int id = READ_BE_UINT16_UNALIGNED(&((VgaFile1Struct0x6*)p)->id);

			dump_vga_script_always(vga + READ_BE_UINT16_UNALIGNED(&((VgaFile1Struct0x6*)p)->script_offs), id/100, id);
			p += sizeof(VgaFile1Struct0x6);
		}
	}

	{
		byte *bb, *b;
		int c;

		bb = vga;
		b = bb + READ_BE_UINT16_UNALIGNED(&((VgaFile1Header*)bb)->hdr2_start);
		c = READ_BE_UINT16_UNALIGNED(&((VgaFile1Header2*)b)->unk1);
		b = bb + READ_BE_UINT16_UNALIGNED(&((VgaFile1Header2*)b)->unk2_offs);

		while (--c >= 0) {
			int id = READ_BE_UINT16_UNALIGNED(&((VgaFile1Struct0x8*)b)->id);

			dump_vga_script_always(vga + READ_BE_UINT16_UNALIGNED(&((VgaFile1Struct0x8*)b)->script_offs), id/100, id);
			b += sizeof(VgaFile1Struct0x8);
		}
	}
}



const byte bmp_hdr[] = {
0x42,0x4D,
0x9E,0x14,0x00,0x00, /* offset 2, file size */
0x00,0x00,0x00,0x00,
0x36,0x04,0x00,0x00,
0x28,0x00,0x00,0x00,

0x3C,0x00,0x00,0x00, /* image width */
0x46,0x00,0x00,0x00, /* image height */
0x01,0x00,0x08,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,

0x00,0x01,0x00,0x00,
0x00,0x01,0x00,0x00,
};

void dump_bmp(const char *filename, int w, int h, const byte *bytes, const uint32 *palette) {
	FILE *out = fopen(filename, "wb");
	byte my_hdr[sizeof(bmp_hdr)];
	int i;

	if (out == NULL) {
		printf("DUMP ERROR\n");
		return;
	}

	memcpy(my_hdr, bmp_hdr, sizeof(bmp_hdr));

	*(uint32*)(my_hdr + 2) = w*h + 1024 + sizeof(bmp_hdr);
	*(uint32*)(my_hdr + 18) = w;
	*(uint32*)(my_hdr + 22) = h;

	
	fwrite(my_hdr, 1, sizeof(my_hdr), out);
	
	for(i=0; i!=256; i++,palette++) {
		byte color[4];
		color[0] = (byte)(*palette >> 16);
		color[1] = (byte)(*palette >> 8);
		color[2] = (byte)(*palette);
		color[3] = 0;
		fwrite(color, 1, 4, out);
	}

	while (--h >= 0) {
		fwrite(bytes + h * ((w+3)&~3), ((w+3)&~3), 1, out);
	}

	fclose(out);
}

void dump_bitmap(const char *filename, byte *offs, int w, int h, int flags, const byte *palette, byte base) {
	/* allocate */
	byte *b = (byte*)malloc(w*h);
	int i,j;

	VC10_state state;

	state.depack_cont = -0x80;
	state.depack_src = offs;
	state.dh = h;
	state.y_skip = 0;

	for(i=0; i!=w; i+=2) {
		byte *c = vc_10_depack_column(&state);
		for(j=0;j!=h;j++) {
			byte pix = c[j];
			b[j*w+i] = (pix>>4)|base;
			b[j*w+i+1] = (pix&0xF)|base;

		}
	}

	dump_bmp(filename, w, h, b, (uint32*)palette);
	free(b);
}

void SimonState::dump_single_bitmap(int file, int image, byte *offs, int w, int h, byte base) {
/* Only supported for win32 atm. mkdir doesn't work otherwise. */
#if defined (WIN32) && !defined(_WIN32_WCE)
	char buf[255], buf2[255];
	struct stat statbuf;

	sprintf(buf, "bmp_%d\\%d.bmp", file, image);

	if (stat(buf, &statbuf) == 0)
		return;

	sprintf(buf2, "bmp_%d", file);
	mkdir(buf2);

	dump_bitmap(buf, offs, w, h, 0, _palette, base);
#endif
}

SimonState *SimonState::create(OSystem *syst, MidiDriver *driver) {
	SimonState *s =  new SimonState;

	s->_system = syst;

	/* Setup midi driver */
	s->midi.set_driver(driver);
	
	/* Setup mixer */
	if (!s->_mixer->bind_to_system(syst))
		warning("Sound initialization failed. "
		        "Features of the game that depend on sound synchronization will most likely break");

	return s;

}

void SimonState::set_volume(byte volume) {
	_mixer->set_volume(volume * 256 / 100);
}

