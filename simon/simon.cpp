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
#include "simon/simon.h"
#include "simon/intern.h"
#include "common/gameDetector.h"
#include <errno.h>
#include <time.h>

#ifdef _WIN32_WCE

extern bool toolbar_drawn;
extern bool draw_keyboard;

#endif


static const GameSpecificSettings simon1_settings = {
	1,														/* VGA_DELAY_BASE */
	1576 / 4,											/* TABLE_INDEX_BASE */
	1460 / 4,											/* TEXT_INDEX_BASE */
	1700 / 4,											/* NUM_GAME_OFFSETS */
	64,														/* NUM_VIDEO_OP_CODES */
	1000000,											/* VGA_MEM_SIZE */
	50000,												/* TABLES_MEM_SIZE */
	3624,													/* NUM_VOICE_RESOURCES */
	141,													/* NUM_EFFECT_RESOURCES */
	1316 / 4,											/* MUSIC_INDEX_BASE */
	0,														/* SOUND_INDEX_BASE */
	"SIMON.GME",									/* gme_filename */
	"SIMON.WAV",									/* wav_filename */
	"SIMON.VOC",									/* wav_filename2 */
	"EFFECTS.VOC",								/* effects_filename */
	"GAMEPC",											/* gamepc_filename */
};

static const GameSpecificSettings simon2_settings = {
	5,														/* VGA_DELAY_BASE */
	1580 / 4,											/* TABLE_INDEX_BASE */
	1500 / 4,											/* TEXT_INDEX_BASE */
	2116 / 4,											/* NUM_GAME_OFFSETS */
	75,														/* NUM_VIDEO_OP_CODES */
	2000000,											/* VGA_MEM_SIZE */
	100000,												/* TABLES_MEM_SIZE */
	12256,												/* NUM_VOICE_RESOURCES */
	0,
	1128 / 4,											/* MUSIC_INDEX_BASE */
	1660 / 4,											/* SOUND_INDEX_BASE */
	"SIMON2.GME",									/* gme_filename */
	"SIMON2.WAV",									/* wav_filename */
	NULL,
	"",
	"GSPTR30",										/* gamepc_filename */
};

static const GameSpecificSettings simon2win_settings = {
	5,														/* VGA_DELAY_BASE */
	1580 / 4,											/* TABLE_INDEX_BASE */
	1500 / 4,											/* TEXT_INDEX_BASE */
	2116 / 4,											/* NUM_GAME_OFFSETS */
	75,														/* NUM_VIDEO_OP_CODES */
	2000000,											/* VGA_MEM_SIZE */
	100000,												/* TABLES_MEM_SIZE */
	12256,												/* NUM_VOICE_RESOURCES */
	0,
	1128 / 4,											/* MUSIC_INDEX_BASE */
	1660 / 4,											/* SOUND_INDEX_BASE */
	"SIMON2.GME",									/* gme_filename */
	"SIMON2.WAV",									/* wav_filename */
	NULL,
	"",
	"GSPTR30",										/* gamepc_filename */
};

static const GameSpecificSettings simon2dos_settings = {
	5,														/* VGA_DELAY_BASE */
	1580 / 4,											/* TABLE_INDEX_BASE */
	1500 / 4,											/* TEXT_INDEX_BASE */
	2116 / 4,											/* NUM_GAME_OFFSETS */
	75,														/* NUM_VIDEO_OP_CODES */
	2000000,											/* VGA_MEM_SIZE */
	100000,												/* TABLES_MEM_SIZE */
	12256,												/* NUM_VOICE_RESOURCES */
	0,
	1128 / 4,											/* MUSIC_INDEX_BASE */
	1660 / 4,											/* SOUND_INDEX_BASE */
	"SIMON2.GME",									/* gme_filename */
	"SIMON2.WAV",									/* wav_filename */
	NULL,
	"",
	"GAME32",											/* gamepc_filename */
};


SimonState::SimonState(GameDetector *detector, OSystem *syst)
	: Engine(detector, syst)
{
	MidiDriver *driver = detector->createMidi();

	_dummy_item_1 = new Item();
	_dummy_item_2 = new Item();
	_dummy_item_3 = new Item();
	
	_fcs_list = new FillOrCopyStruct[16];

	/* Setup midi driver */
	midi.set_driver(driver);

	_game = detector->_gameId;

	/* Setup mixer */
	if (!_mixer->bind_to_system(syst))
		warning("Sound initialization failed. "
						"Features of the game that depend on sound synchronization will most likely break");
	set_volume(detector->_sfx_volume);
}

SimonState::~SimonState()
{
	delete _dummy_item_1;
	delete _dummy_item_2;
	delete _dummy_item_3;
	
	delete [] _fcs_list;
}

void palette_fadeout(uint32 *pal_values, uint num)
{
	byte *p = (byte *)pal_values;

	do {
		if (p[0] >= 8)
			p[0] -= 8;
		else
			p[0] = 0;
		if (p[1] >= 8)
			p[1] -= 8;
		else
			p[1] = 0;
		if (p[2] >= 8)
			p[2] -= 8;
		else
			p[2] = 0;
		p += sizeof(uint32);
	} while (--num);
}

byte *SimonState::allocateItem(uint size)
{
	byte *org = _itemheap_ptr;
	size = (size + 3) & ~3;

	_itemheap_ptr += size;
	_itemheap_curpos += size;

	if (_itemheap_curpos > _itemheap_size)
		error("Itemheap overflow");

	return org;
}

void SimonState::alignTableMem()
{
	if ((uint32)_tablesheap_ptr & 3) {
		_tablesheap_ptr += 2;
		_tablesheap_curpos += 2;
	}
}

byte *SimonState::allocateTable(uint size)
{
	byte *org = _tablesheap_ptr;

	size = (size + 1) & ~1;

	_tablesheap_ptr += size;
	_tablesheap_curpos += size;

	if (_tablesheap_curpos > _tablesheap_size)
		error("Tablesheap overflow");

	return org;
}

int SimonState::allocGamePcVars(FILE *in)
{
	uint item_array_size, item_array_inited, stringtable_num;
	uint32 version;
	uint i;

	item_array_size = fileReadBE32(in);
	version = fileReadBE32(in);
	item_array_inited = fileReadBE32(in);
	stringtable_num = fileReadBE32(in);

	item_array_inited += 2;				/* first two items are predefined */
	item_array_size += 2;

	if (version != 0x80)
		error("Not a runtime database");

	_itemarray_ptr = (Item **)calloc(item_array_size, sizeof(Item *));
	if (_itemarray_ptr == NULL)
		error("Out of memory for Item array");

	_itemarray_size = item_array_size;
	_itemarray_inited = item_array_inited;

	for (i = 2; i != item_array_inited; i++) {
		_itemarray_ptr[i] = (Item *)allocateItem(sizeof(Item));
	}

	/* The rest is cleared automatically by calloc */
	allocateStringTable(stringtable_num + 10);
	_stringtab_num = stringtable_num;

	return item_array_inited;
}


Item *SimonState::allocItem1()
{
	Item *item = (Item *)allocateItem(sizeof(Item));
	_itemarray_ptr[1] = item;
	return item;
}

void SimonState::loginPlayerHelper(Item *item, int a, int b)
{
	Child9 *child;

	child = (Child9 *) findChildOfType(item, 9);
	if (child == NULL) {
		child = (Child9 *) allocateChildBlock(item, 9, sizeof(Child9));
	}

	if (a >= 0 && a <= 3)
		child->array[a] = b;
}


void SimonState::loginPlayer()
{
	Item *item;
	Child *child;

	item = _itemarray_ptr[1];
	item->unk2 = -1;
	item->unk1 = 10000;
	_item_1 = item;

	child = (Child *)allocateChildBlock(item, 3, sizeof(Child));
	if (child == NULL)
		error("player create failure");

	loginPlayerHelper(item, 0, 0);
}

void SimonState::allocateStringTable(int num)
{
	_stringtab_ptr = (byte **)calloc(num, sizeof(byte *));
	_stringtab_pos = 0;
	_stringtab_numalloc = num;
}

void SimonState::setupStringTable(byte *mem, int num)
{
	int i = 0;
	for (;;) {
		_stringtab_ptr[i++] = mem;
		if (--num == 0)
			break;
		for (; *mem; mem++);
		mem++;
	}

	_stringtab_pos = i;
}

void SimonState::setupLocalStringTable(byte *mem, int num)
{
	int i = 0;
	for (;;) {
		_local_stringtable[i++] = mem;
		if (--num == 0)
			break;
		for (; *mem; mem++);
		mem++;
	}
}

void SimonState::readSubroutineLine(FILE *in, SubroutineLine *sl, Subroutine *sub)
{
	byte line_buffer[1024], *q = line_buffer;
	int size;

	if (sub->id == 0) {
		sl->cond_a = fileReadBE16(in);
		sl->cond_b = fileReadBE16(in);
		sl->cond_c = fileReadBE16(in);
	}

	while ((*q = fileReadByte(in)) != 0xFF) {
		if (*q == 87) {
			fileReadBE16(in);
		} else {
			q = readSingleOpcode(in, q);
		}
	}

	size = q - line_buffer + 1;

	memcpy(allocateTable(size), line_buffer, size);
}

SubroutineLine *SimonState::createSubroutineLine(Subroutine *sub, int where)
{
	SubroutineLine *sl, *cur_sl = NULL, *last_sl = NULL;

	if (sub->id == 0)
		sl = (SubroutineLine *)allocateTable(SUBROUTINE_LINE_BIG_SIZE);
	else
		sl = (SubroutineLine *)allocateTable(SUBROUTINE_LINE_SMALL_SIZE);

	/* where is what offset to insert the line at, locate the proper beginning line */
	if (sub->first != 0) {
		cur_sl = (SubroutineLine *)((byte *)sub + sub->first);
		while (where) {
			last_sl = cur_sl;
			cur_sl = (SubroutineLine *)((byte *)sub + cur_sl->next);
			if ((byte *)cur_sl == (byte *)sub)
				break;
			where--;
		}
	}

	if (last_sl != NULL) {
		/* Insert the subroutine line in the middle of the link */
		last_sl->next = (byte *)sl - (byte *)sub;
		sl->next = (byte *)cur_sl - (byte *)sub;
	} else {
		/* Insert the subroutine line at the head of the link */
		sl->next = sub->first;
		sub->first = (byte *)sl - (byte *)sub;
	}

	return sl;
}

void SimonState::readSubroutine(FILE *in, Subroutine *sub)
{
	while (fileReadBE16(in) == 0) {
		readSubroutineLine(in, createSubroutineLine(sub, 0xFFFF), sub);
	}
}

Subroutine *SimonState::createSubroutine(uint id)
{
	Subroutine *sub;

	alignTableMem();

	sub = (Subroutine *)allocateTable(sizeof(Subroutine));
	sub->id = id;
	sub->first = 0;
	sub->next = _subroutine_list;
	_subroutine_list = sub;
	return sub;
}

void SimonState::readSubroutineBlock(FILE *in)
{
	while (fileReadBE16(in) == 0) {
		readSubroutine(in, createSubroutine(fileReadBE16(in)));
	}
}


Child *SimonState::findChildOfType(Item *i, uint type)
{
	Child *child = i->children;
	for (; child; child = child->next)
		if (child->type == type)
			return child;
	return NULL;
}

bool SimonState::hasChildOfType1(Item *item)
{
	return findChildOfType1(item) != NULL;
}

bool SimonState::hasChildOfType2(Item *item)
{
	return findChildOfType2(item) != NULL;
}

Child1 *SimonState::findChildOfType1(Item *item)
{
	return (Child1 *)findChildOfType(item, 1);
}

Child2 *SimonState::findChildOfType2(Item *item)
{
	return (Child2 *)findChildOfType(item, 2);
}

Child3 *SimonState::findChildOfType3(Item *item)
{
	return (Child3 *) findChildOfType(item, 3);
}

uint SimonState::getOffsetOfChild2Param(Child2 *child, uint prop)
{
	uint m = 1;
	uint offset = 0;
	while (m != prop) {
		if (child->avail_props & m)
			offset++;
		m <<= 1;
	}
	return offset;
}

Child *SimonState::allocateChildBlock(Item *i, uint type, uint size)
{
	Child *child = (Child *)allocateItem(size);
	child->next = i->children;
	i->children = child;
	child->type = type;
	return child;
}

void SimonState::allocItemHeap()
{
	_itemheap_size = 10000;
	_itemheap_curpos = 0;
	_itemheap_ptr = (byte *)calloc(10000, 1);
}

void SimonState::allocTablesHeap()
{
	_tablesheap_size = gss->TABLES_MEM_SIZE;
	_tablesheap_curpos = 0;
	_tablesheap_ptr = (byte *)calloc(gss->TABLES_MEM_SIZE, 1);
}

void SimonState::setItemUnk3(Item *item, int value)
{
	item->unk3 = value;
}


int SimonState::getNextWord()
{
	_code_ptr += 2;
	return (int16)((_code_ptr[-2] << 8) | _code_ptr[-1]);
}

uint SimonState::getNextStringID()
{
	return (uint16)getNextWord();
}

uint SimonState::getVarOrByte()
{
	uint a = *_code_ptr++;
	if (a != 255)
		return a;
	return readVariable(*_code_ptr++);
}

uint SimonState::getVarOrWord()
{
	uint a = (_code_ptr[0] << 8) | _code_ptr[1];
	_code_ptr += 2;
	if (a >= 30000 && a < 30512)
		return readVariable(a - 30000);
	return a;
}

Item *SimonState::getNextItemPtr()
{
	int a = getNextWord();
	switch (a) {
	case -1:
		return _subject_item;
	case -3:
		return _object_item;
	case -5:
		return getItem1Ptr();
	case -7:
		return getItemPtrB();
	case -9:
		return derefItem(getItem1Ptr()->parent);
	default:
		return derefItem(a);
	}
}

Item *SimonState::getNextItemPtrStrange()
{
	int a = getNextWord();
	switch (a) {
	case -1:
		return _subject_item;
	case -3:
		return _object_item;
	case -5:
		return _dummy_item_2;
	case -7:
		return NULL;
	case -9:
		return _dummy_item_3;
	default:
		return derefItem(a);
	}
}


uint SimonState::getNextItemID()
{
	int a = getNextWord();
	switch (a) {
	case -1:
		return itemPtrToID(_subject_item);
	case -3:
		return itemPtrToID(_object_item);
	case -5:
		return getItem1ID();
	case -7:
		return 0;
	case -9:
		return getItem1Ptr()->parent;
	default:
		return a;
	}
}

Item *SimonState::getItem1Ptr()
{
	if (_item_1_ptr)
		return _item_1_ptr;
	return _dummy_item_1;
}

Item *SimonState::getItemPtrB()
{
	error("getItemPtrB: is this code ever used?");
	if (_item_ptr_B)
		return _item_ptr_B;
	return _dummy_item_1;
}

uint SimonState::getNextVarContents()
{
	return (uint16)readVariable(getVarOrByte());
}

uint SimonState::readVariable(uint variable)
{
	if (variable >= 255)
		error("Variable %d out of range in read", variable);
	return _variableArray[variable];
}

void SimonState::writeNextVarContents(uint16 contents)
{
	writeVariable(getVarOrByte(), contents);
}

void SimonState::writeVariable(uint variable, uint16 contents)
{
	if (variable >= 256)
		error("Variable %d out of range in write", variable);
	_variableArray[variable] = contents;
}

void SimonState::setItemParent(Item *item, Item *parent)
{
	Item *old_parent = derefItem(item->parent);

	if (item == parent)
		error("Trying to set item as its own parent");

	/* unlink it if it has a parent */
	if (old_parent)
		unlinkItem(item);
	itemChildrenChanged(old_parent);
	linkItem(item, parent);
	itemChildrenChanged(parent);
}

void SimonState::itemChildrenChanged(Item *item)
{
	int i;
	FillOrCopyStruct *fcs;

	if (_no_parent_notify)
		return;

	lock();

	for (i = 0; i != 8; i++) {
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

void SimonState::unlinkItem(Item *item)
{
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

	for (;;) {
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

void SimonState::linkItem(Item *item, Item *parent)
{
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

const byte *SimonState::getStringPtrByID(uint string_id)
{
	const byte *string_ptr;
	byte *dst;

	_free_string_slot ^= 1;

	if (string_id < 0x8000) {
		string_ptr = _stringtab_ptr[string_id];
	} else {
		string_ptr = getLocalStringByID(string_id);
	}

	dst = &_stringReturnBuffer[_free_string_slot][0];
	strcpy((char *)dst, (const char *)string_ptr);
	return dst;
}

const byte *SimonState::getLocalStringByID(uint string_id)
{
	if (string_id < _string_id_local_min || string_id >= _string_id_local_max) {
		loadTextIntoMem(string_id);
	}
	return _local_stringtable[string_id - _string_id_local_min];
}

void SimonState::loadTextIntoMem(uint string_id)
{
	byte *p;
	char filename[30];
	int i;
	uint base_min = 0x8000, base_max, size;

	_tablesheap_ptr = _tablesheap_ptr_new;
	_tablesheap_curpos = _tablesheap_curpos_new;

	p = _stripped_txt_mem;

	/* get filename */
	while (*p) {
		for (i = 0; *p; p++, i++)
			filename[i] = *p;
		filename[i] = 0;
		p++;

		base_max = (p[0] << 8) | p[1];
		p += 2;

		if (string_id < base_max) {
			_string_id_local_min = base_min;
			_string_id_local_max = base_max;

			_local_stringtable = (byte **)_tablesheap_ptr;

			size = (base_max - base_min + 1) * sizeof(byte *);
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

void SimonState::loadTablesIntoMem(uint subr_id)
{
	byte *p;
	int i;
	uint min_num, max_num;
	char filename[30];
	FILE *in;

	p = _tbl_list;
	if (p == NULL)
		return;

	while (*p) {
		for (i = 0; *p; p++, i++)
			filename[i] = *p;
		filename[i] = 0;
		p++;

		for (;;) {
			min_num = (p[0] << 8) | p[1];
			p += 2;

			if (min_num == 0)
				break;

			max_num = (p[0] << 8) | p[1];
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

Subroutine *SimonState::getSubroutineByID(uint subroutine_id)
{
	Subroutine *cur;

	for (cur = _subroutine_list; cur; cur = cur->next) {
		if (cur->id == subroutine_id)
			return cur;
	}

	loadTablesIntoMem(subroutine_id);

	for (cur = _subroutine_list; cur; cur = cur->next) {
		if (cur->id == subroutine_id)
			return cur;
	}

	warning("getSubroutineByID: subroutine %d not found", subroutine_id);
	return NULL;
}

uint SimonState::loadTextFile_gme(const char *filename, byte *dst)
{
	uint res;
	uint32 offs;
	uint32 size;

	res = atoi(filename + 4) + gss->TEXT_INDEX_BASE - 1;
	offs = _game_offsets_ptr[res];
	size = _game_offsets_ptr[res + 1] - offs;

	resfile_read(dst, offs, size);

	return size;
}

FILE *SimonState::openTablesFile_gme(const char *filename)
{
	uint res;
	uint32 offs;

	res = atoi(filename + 6) + gss->TABLE_INDEX_BASE - 1;
	offs = _game_offsets_ptr[res];

	fseek(_game_file, offs, SEEK_SET);
	return _game_file;
}

void SimonState::closeTablesFile_gme(FILE *in)
{
	/* not needed */
}

/* Simon1DOS load tables file */
uint SimonState::loadTextFile_simon1(const char *filename, byte *dst)
{
	FILE *fo = fopen_maybe_lowercase(filename);
	uint32 size;

	if (fo == NULL)
		error("loadTextFile: Cannot open '%s'", filename);

	fseek(fo, 0, SEEK_END);
	size = ftell(fo);
	fseek(fo, 0, SEEK_SET);

	if (fread(dst, size, 1, fo) != 1)
		error("loadTextFile: fread failed");
	fclose(fo);

	return size;
}


FILE *SimonState::openTablesFile_simon1(const char *filename)
{
	FILE *fo = fopen_maybe_lowercase(filename);
	if (fo == NULL)
		error("openTablesFile: Cannot open '%s'", filename);
	return fo;
}

void SimonState::closeTablesFile_simon1(FILE *in)
{
	fclose(in);
}

uint SimonState::loadTextFile(const char *filename, byte *dst)
{
	if (_game == GAME_SIMON1DOS)
		return loadTextFile_simon1(filename, dst);
	else
		return loadTextFile_gme(filename, dst);
}

FILE *SimonState::openTablesFile(const char *filename)
{
	if (_game == GAME_SIMON1DOS)
		return openTablesFile_simon1(filename);
	else
		return openTablesFile_gme(filename);
}

void SimonState::closeTablesFile(FILE *in)
{
	if (_game == GAME_SIMON1DOS)
		closeTablesFile_simon1(in);
	else
		closeTablesFile_gme(in);
}

void SimonState::addTimeEvent(uint timeout, uint subroutine_id)
{
	TimeEvent *te = (TimeEvent *)malloc(sizeof(TimeEvent)), *first, *last = NULL;
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

void SimonState::delTimeEvent(TimeEvent *te)
{
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

	for (;;) {
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

void SimonState::killAllTimers()
{
	TimeEvent *cur, *next;

	for (cur = _first_time_struct; cur; cur = next) {
		next = cur->next;
		delTimeEvent(cur);
	}
}

bool SimonState::kickoffTimeEvents()
{
	time_t cur_time;
	TimeEvent *te;
	bool result = false;

	time(&cur_time);
	cur_time -= _base_time;

	while ((te = _first_time_struct) != NULL && te->time <= (uint32)cur_time) {
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

void SimonState::invokeTimeEvent(TimeEvent *te)
{
	Subroutine *sub;

	_script_cond_a = 0;
	if (_run_script_return_1)
		return;
	sub = getSubroutineByID(te->subroutine_id);
	if (sub != NULL)
		startSubroutineEx(sub);
	_run_script_return_1 = false;
}

void SimonState::o_setup_cond_c()
{
	Item *item = _item_1;

	setup_cond_c_helper();

	_item_1_ptr = item;
	_object_item = _hitarea_object_item;

	if (_object_item == _dummy_item_2)
		_object_item = getItem1Ptr();

	if (_object_item == _dummy_item_3)
		_object_item = derefItem(getItem1Ptr()->parent);

	if (_object_item != NULL) {
		_script_cond_c = _object_item->unk1;
	} else {
		_script_cond_c = -1;
	}
}

void SimonState::setup_cond_c_helper()
{
	HitArea *last;

	if ((_game == GAME_SIMON2WIN) || (_game == GAME_SIMON2DOS)) {
		_mouse_cursor = 0;
		if (_hitarea_unk_4 != 999) {
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

	for (;;) {
		_last_hitarea = NULL;
		_last_hitarea_3 = 0;
		_left_button_down = 0;

		do {
#ifdef WIN32
			if (GetAsyncKeyState(VK_F5) != 0 && _bit_array[0] & 0x200) {
#else
			if (_bit_array[0] & 0x200) {
#endif
				startSubroutine170();
				goto out_of_here;
			}

			delay(100);
		} while (_last_hitarea_3 == (HitArea *) 0xFFFFFFFF || _last_hitarea_3 == 0);

		if (_last_hitarea == NULL) {
		} else if (_last_hitarea->id == 0x7FFB) {
			handle_unk2_hitarea(_last_hitarea->fcs);
		} else if (_last_hitarea->id == 0x7FFC) {
			handle_unk_hitarea(_last_hitarea->fcs);
		} else if (_last_hitarea->item_ptr != NULL) {
			_hitarea_object_item = _last_hitarea->item_ptr;
			_variableArray[60] = (_last_hitarea->flags & 1) ? (_last_hitarea->flags >> 8) : 0xFFFF;
			break;
		}
	}

out_of_here:
	_last_hitarea_3 = 0;
	_last_hitarea = 0;
	_last_hitarea_2_ptr = NULL;
	_hitarea_unk_6 = false;
}

void SimonState::startSubroutine170()
{
	Subroutine *sub;

	/* XXX: stop speech */

	sub = getSubroutineByID(170);
	if (sub != NULL)
		startSubroutineEx(sub);

	_run_script_return_1 = true;
}

uint SimonState::get_fcs_ptr_3_index(FillOrCopyStruct *fcs)
{
	uint i;

	for (i = 0; i != ARRAYSIZE(_fcs_ptr_array_3); i++)
		if (_fcs_ptr_array_3[i] == fcs)
			return i;

	error("get_fcs_ptr_3_index: not found");
}



void SimonState::lock()
{
	_lock_counter++;
}

void SimonState::unlock()
{
	_lock_word |= 1;

	if (_lock_counter != 0) {
		if (_lock_counter == 1) {
#ifdef WIN32
			GetAsyncKeyState(VK_LBUTTON);
#endif
		}
		_lock_counter--;
	}
	_lock_word &= ~1;
}

void SimonState::handle_mouse_moved()
{
	uint x;

	if (_lock_counter)
		return;

	pollMouseXY();

	if (_mouse_x >= 32768)
		_mouse_x = 0;
	if (_mouse_x >= 638 / 2)
		_mouse_x = 638 / 2;

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
		if (_bit_array[4] & 0x8000) {
			if (!_vga_var9) {
				if (_mouse_x >= 630 / 2 || _mouse_x < 9)
					goto get_out2;
				_vga_var9 = 1;
			}
			if (_vga_var2 == 0) {
				if (_mouse_x >= 631 / 2) {
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
		if (_hitarea_unk_3 == 0 && _need_hitarea_recalc == 0)
			goto get_out;
	}

	setup_hitarea_from_pos(_mouse_x, _mouse_y, x);
	_last_hitarea_3 = _last_hitarea;
	if (x == 1 && _last_hitarea == NULL)
		_last_hitarea_3 = (HitArea *) - 1;

get_out:
	draw_mouse_pointer();
	_need_hitarea_recalc = 0;
}

void SimonState::fcs_unk_proc_1(uint fcs_index, Item *item_ptr, int unk1, int unk2)
{
	Item *item_ptr_org = item_ptr;
	FillOrCopyStruct *fcs_ptr;
	uint width_div_3, height_div_3;
	uint j, k, i, num_sibs_with_flag;
	bool item_again;
	uint x_pos, y_pos;

	fcs_ptr = _fcs_ptr_array_3[fcs_index & 7];

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

	fcs_ptr->fcs_data = (FillOrCopyData *) malloc(sizeof(FillOrCopyData));
	fcs_ptr->fcs_data->item_ptr = item_ptr;
	fcs_ptr->fcs_data->unk3 = -1;
	fcs_ptr->fcs_data->unk4 = -1;
	fcs_ptr->fcs_data->unk1 = unk1;
	fcs_ptr->fcs_data->unk2 = unk2;

	item_ptr = derefItem(item_ptr->child);

	while (item_ptr && unk1-- != 0) {
		num_sibs_with_flag = 0;
		while (item_ptr && width_div_3 > num_sibs_with_flag) {
			if ((unk2 == 0 || item_ptr->unk4 & unk2) && has_item_childflag_0x10(item_ptr))
				if (!(_game & GAME_SIMON2)) {
					num_sibs_with_flag++;
				} else {
					num_sibs_with_flag += 20;
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
		if ((unk2 == 0 || item_ptr->unk4 & unk2) && has_item_childflag_0x10(item_ptr)) {
			if (item_again == false) {
				fcs_ptr->fcs_data->e[k].item = item_ptr;
				if (!(_game & GAME_SIMON2)) {
					draw_icon_c(fcs_ptr, item_get_icon_number(item_ptr), x_pos * 3, y_pos);
					fcs_ptr->fcs_data->e[k].hit_area =
						setup_icon_hit_area(fcs_ptr, x_pos * 3, y_pos,
																item_get_icon_number(item_ptr), item_ptr);
				} else {
					draw_icon_c(fcs_ptr, item_get_icon_number(item_ptr), x_pos, y_pos);
					fcs_ptr->fcs_data->e[k].hit_area =
						setup_icon_hit_area(fcs_ptr, x_pos, y_pos, item_get_icon_number(item_ptr), item_ptr);
				}
				k++;
			} else {
				fcs_ptr->fcs_data->e[k].item = NULL;
				j = 1;
			}
			x_pos += (_game & GAME_SIMON2) ? 20 : 1;

			if (x_pos >= width_div_3) {
				x_pos = 0;

				y_pos += (_game & GAME_SIMON2) ? 20 : 1;
				if (y_pos >= height_div_3)
					item_again = true;
			}
		}
		item_ptr = derefItem(item_ptr->sibling);
	}

	fcs_ptr->fcs_data->e[k].item = NULL;

	if (j != 0 || fcs_ptr->fcs_data->unk1 != 0) {
		fcs_unk_proc_2(fcs_ptr, fcs_index);
	}
}

void SimonState::fcs_unk_proc_2(FillOrCopyStruct *fcs, uint fcs_index)
{
	setup_hit_areas(fcs, fcs_index);

	fcs->fcs_data->unk3 = _scroll_up_hit_area;
	fcs->fcs_data->unk4 = _scroll_down_hit_area;
}

void SimonState::setup_hit_areas(FillOrCopyStruct *fcs, uint fcs_index)
{
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


bool SimonState::has_item_childflag_0x10(Item *item)
{
	Child2 *child = findChildOfType2(item);
	return child && (child->avail_props & 0x10) != 0;
}

uint SimonState::item_get_icon_number(Item *item)
{
	Child2 *child = findChildOfType2(item);
	uint offs;

	if (child == NULL || !(child->avail_props & 0x10))
		return 0;

	offs = getOffsetOfChild2Param(child, 0x10);
	return child->array[offs];
}

void SimonState::loadIconFile()
{
	FILE *in = fopen_maybe_lowercase("ICON.DAT");
	uint size;

	if (in == NULL)
		error("Cannot open icon.dat");

	fseek(in, 0, SEEK_END);
	size = ftell(in);

	_icon_file_ptr = (byte *)malloc(size);
	if (_icon_file_ptr == NULL)
		error("Out of icon memory");

	fseek(in, 0, SEEK_SET);

	fread(_icon_file_ptr, size, 1, in);
	fclose(in);
}


uint SimonState::setup_icon_hit_area(FillOrCopyStruct *fcs, uint x, uint y, uint icon_number,
																		 Item *item_ptr)
{
	HitArea *ha;

	ha = findEmptyHitArea();

	if (!(_game & GAME_SIMON2)) {
		ha->x = (x + fcs->x) << 3;
		ha->y = y * 25 + fcs->y;
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

void SimonState::hitarea_stuff()
{
	HitArea *ha;
	uint id;

	_left_button_down = 0;
	_last_hitarea = 0;
	_verb_hitarea = 0;
	_hitarea_subject_item = NULL;
	_hitarea_object_item = NULL;

	hitarea_proc_1();

startOver:
	for (;;) {
		_last_hitarea = NULL;
		_last_hitarea_3 = NULL;
		for (;;) {
			processSpecialKeys();
			if (_last_hitarea_3 == (HitArea *) 0xFFFFFFFF)
				goto startOver;
			if (_last_hitarea_3 != 0)
				break;
			hitarea_stuff_helper();
			delay(100);
		}

		ha = _last_hitarea;

		if (ha == NULL) {
		} else if (ha->id == 0x7FFB) {
			handle_unk2_hitarea(ha->fcs);
		} else if (ha->id == 0x7FFC) {
			handle_unk_hitarea(ha->fcs);
		} else if (ha->id >= 101 && ha->id < 113) {
			_verb_hitarea = ha->unk3;
			handle_verb_hitarea(ha);
			_hitarea_unk_4 = 0;
		} else {
			if ((_verb_hitarea != 0 || _hitarea_subject_item != ha->item_ptr && ha->flags & 0x80) &&
					ha->item_ptr) {
			if_1:;
				_hitarea_subject_item = ha->item_ptr;
				id = 0xFFFF;
				if (ha->flags & 1)
					id = ha->flags >> 8;
				_variableArray[60] = id;
				new_current_hitarea(ha);
				if (_verb_hitarea != 0)
					break;
			} else {
				/* else 1 */
				if (ha->unk3 == 0) {
					if (ha->item_ptr)
						goto if_1;
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

void SimonState::hitarea_stuff_helper()
{
	time_t cur_time;

	if (!(_game & GAME_SIMON2)) {
		uint subr_id = _variableArray[0x1FC / 2];
		if (subr_id != 0) {
			Subroutine *sub = getSubroutineByID(subr_id);
			if (sub != NULL) {
				startSubroutineEx(sub);
				startUp_helper_2();
			}
			_variableArray[0x1FC / 2] = 0;
			_run_script_return_1 = false;
		}
	} else {
		if (_variableArray[0x1FC / 2] || _variableArray[0x1F2 / 2]) {
			hitarea_stuff_helper_2();
		}
	}

	time(&cur_time);
	if ((uint) cur_time != _last_time) {
		_last_time = cur_time;
		if (kickoffTimeEvents())
			startUp_helper_2();
	}
}

/* Simon 2 specific */
void SimonState::hitarea_stuff_helper_2()
{
	uint subr_id;
	Subroutine *sub;

	subr_id = _variableArray[0x1F2 / 2];
	if (subr_id != 0) {
		sub = getSubroutineByID(subr_id);
		if (sub != NULL) {
			_variableArray[0x1F2 / 2] = 0;
			startSubroutineEx(sub);
			startUp_helper_2();
		}
		_variableArray[0x1F2 / 2] = 0;
	}

	subr_id = _variableArray[0x1FC / 2];
	if (subr_id != 0) {
		sub = getSubroutineByID(subr_id);
		if (sub != NULL) {
			_variableArray[0x1FC / 2] = 0;
			startSubroutineEx(sub);
			startUp_helper_2();
		}
		_variableArray[0x1FC / 2] = 0;
	}

	_run_script_return_1 = false;
}


void SimonState::startUp_helper_2()
{
	if (!_mortal_flag) {
		_mortal_flag = true;
		startUp_helper_3();
		_fcs_unk_1 = 0;
		if (_fcs_ptr_array_3[0] != 0) {
			_fcs_ptr_1 = _fcs_ptr_array_3[0];
			showmessage_helper_3(_fcs_ptr_1->textLength,
                                 _fcs_ptr_1->textMaxLength);
		}
		_mortal_flag = false;
	}
}

void SimonState::startUp_helper_3()
{
	showmessage_print_char(0);
}

void SimonState::showmessage_helper_3(uint a, uint b)
{
	_print_char_unk_1 = a;
	_print_char_unk_2 = b;
	_num_letters_to_print = 0;
}

void SimonState::pollMouseXY()
{
	_mouse_x = _sdl_mouse_x;
	_mouse_y = _sdl_mouse_y;
}

void SimonState::handle_verb_clicked(uint verb)
{
	Subroutine *sub;
	int result;

	_item_1_ptr = _item_1;

	_object_item = _hitarea_object_item;
	if (_object_item == _dummy_item_2) {
		_object_item = getItem1Ptr();
	}
	if (_object_item == _dummy_item_3) {
		_object_item = derefItem(getItem1Ptr()->parent);
	}

	_subject_item = _hitarea_subject_item;
	if (_subject_item == _dummy_item_2) {
		_subject_item = getItem1Ptr();
	}
	if (_subject_item == _dummy_item_3) {
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
	if (sub == NULL)
		return;

	result = startSubroutine(sub);
	if (result == -1)
		showMessageFormat("I don't understand");

	_run_script_return_1 = false;

	sub = getSubroutineByID(100);
	if (sub)
		startSubroutine(sub);

	if (_game & GAME_SIMON2)
		_run_script_return_1 = false;

	startUp_helper_2();
}

void SimonState::o_print_str()
{
	uint num_1 = getVarOrByte();
	uint num_2 = getVarOrByte();
	uint string_id = getNextStringID();
	const byte *string_ptr = NULL;
	uint speech_id = 0;
	ThreeValues *tv;


	switch (_game) {
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

	case GAME_SIMON2DOS:
		if (string_id != 0xFFFF)
			string_ptr = getStringPtrByID(string_id);

		speech_id = (uint16)getNextWord();
		break;

	case GAME_SIMON1DOS:
		string_ptr = getStringPtrByID(string_id);
		break;
	}

	switch (num_1) {
	case 1:
		tv = &_threevalues_1;
		break;
	case 2:
		tv = &_threevalues_2;
		break;
	case 101:
		tv = &_threevalues_3;
		break;
	case 102:
		tv = &_threevalues_4;
		break;
	default:
		error("o_print_str, invalid value %d", num_1);
	}


	switch (_game) {
	case GAME_SIMON1WIN:
#ifdef USE_TEXT_HACK
		if (speech_id != 0) {
			if (string_ptr == NULL)
				talk_with_speech(speech_id, num_1);
			else if (speech_id != 9999)
				playVoice(speech_id);
		}

		if (string_ptr != NULL) {
			talk_with_text(num_1, num_2, (char *)string_ptr, tv->a, tv->b, tv->c);
		}
#else
		if (speech_id != 0) {
			talk_with_speech(speech_id, num_1);
		} else if (string_ptr != NULL) {
			talk_with_text(num_1, num_2, (char *)string_ptr, tv->a, tv->b, tv->c);
		}
#endif
		break;

	case GAME_SIMON1DOS:
		talk_with_text(num_1, num_2, (char *)string_ptr, tv->a, tv->b, tv->c);
		break;

	case GAME_SIMON2DOS:
		if (speech_id != 0 && num_1 == 1 && !_vk_t_toggle)
			talk_with_speech(speech_id, num_1);

		if (speech_id != 0 && !_vk_t_toggle)
			return;

		if (speech_id == 0)
			o_unk_99_simon2(2, num_1 + 2);

		talk_with_text(num_1, num_2, (char *)string_ptr, tv->a, tv->b, tv->c);
		break;


	case GAME_SIMON2WIN:
		if (speech_id != 0 && num_1 == 1 && !_vk_t_toggle)
			talk_with_speech(speech_id, num_1);

		if (speech_id != 0 && !_vk_t_toggle)
			return;

		if (speech_id == 0)
			o_unk_99_simon2(2, num_1 + 2);

		talk_with_text(num_1, num_2, (char *)string_ptr, tv->a, tv->b, tv->c);
		break;
	}
}

void SimonState::ensureVgaResLoadedC(uint vga_res)
{
	_lock_word |= 0x80;
	ensureVgaResLoaded(vga_res);
	_lock_word &= ~0x80;
}

void SimonState::ensureVgaResLoaded(uint vga_res)
{
	VgaPointersEntry *vpe;

	CHECK_BOUNDS(vga_res, _vga_buffer_pointers);

	vpe = _vga_buffer_pointers + vga_res;
	if (vpe->vgaFile1 != NULL)
		return;

	vpe->vgaFile2 = read_vga_from_datfile_2(vga_res * 2 + 1);
	vpe->vgaFile1 = read_vga_from_datfile_2(vga_res * 2);

}

byte *SimonState::setup_vga_destination(uint32 size)
{
	byte *dest, *end;

	_video_var_4 = 0;

	for (;;) {
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

void SimonState::setup_vga_file_buf_pointers()
{
	byte *alloced;

	alloced = (byte *)malloc(gss->VGA_MEM_SIZE);

	_vga_buf_free_start = alloced;
	_vga_buf_start = alloced;
	_vga_file_buf_org = alloced;
	_vga_file_buf_org_2 = alloced;
	_vga_buf_end = alloced + gss->VGA_MEM_SIZE;
}

void SimonState::vga_buf_unk_proc3(byte *end)
{
	VgaPointersEntry *vpe;

	if (_video_var_7 == 0xFFFF)
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

void SimonState::vga_buf_unk_proc1(byte *end)
{
	VgaSprite *vsp;
	if (_lock_word & 0x20)
		return;

	for (vsp = _vga_sprites; vsp->id; vsp++) {
		vga_buf_unk_proc2(vsp->unk7, end);
		if (_video_var_5 == true)
			return;
	}
}

void SimonState::delete_memptr_range(byte *end)
{
	uint count = ARRAYSIZE(_vga_buffer_pointers);
	VgaPointersEntry *vpe = _vga_buffer_pointers;
	do {
		if (_vga_buf_free_start <= vpe->vgaFile1 && end >= vpe->vgaFile1 ||
				_vga_buf_free_start <= vpe->vgaFile2 && end >= vpe->vgaFile2) {
			vpe->dd = 0;
			vpe->vgaFile1 = NULL;
			vpe->vgaFile2 = NULL;
		}

	} while (++vpe, --count);
}

void SimonState::vga_buf_unk_proc2(uint a, byte *end)
{
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


void SimonState::o_clear_vgapointer_entry(uint a)
{
	VgaPointersEntry *vpe;

	vpe = &_vga_buffer_pointers[a];

	vpe->dd = 0;
	vpe->vgaFile1 = NULL;
	vpe->vgaFile2 = NULL;
}

void SimonState::o_set_video_mode(uint mode, uint vga_res)
{
	if (mode == 4)
		vc_29_stop_all_sounds();

	if (_lock_word & 0x10) {
		error("o_set_video_mode_ex: _lock_word & 0x10");
//    _unk21_word_array[a] = b; 
	} else {
		set_video_mode(mode, vga_res);
	}
}

void SimonState::set_video_mode_internal(uint mode, uint vga_res_id)
{
	uint num;
	VgaPointersEntry *vpe;
	byte *bb, *b;
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

	for (;;) {
		vpe = &_vga_buffer_pointers[num];

		_cur_vga_file_1 = vpe->vgaFile1;
		_cur_vga_file_2 = vpe->vgaFile2;

		if (vpe->vgaFile1 != NULL)
			break;

		ensureVgaResLoaded(num);
	}

	/* ensure flipping complete */

	bb = _cur_vga_file_1;
	b = bb + READ_BE_UINT16_UNALIGNED(&((VgaFile1Header *) bb)->hdr2_start);
	c = READ_BE_UINT16_UNALIGNED(&((VgaFile1Header2 *) b)->unk1);
	b = bb + READ_BE_UINT16_UNALIGNED(&((VgaFile1Header2 *) b)->unk2_offs);

	while (READ_BE_UINT16_UNALIGNED(&((VgaFile1Struct0x8 *) b)->id) != vga_res_id)
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
		if (_variableArray[34] != -1)
			_variableArray[502 / 2] = 0;
	}

	vc_ptr_org = _vc_ptr;

	_vc_ptr = _cur_vga_file_1 + READ_BE_UINT16_UNALIGNED(&((VgaFile1Struct0x8 *) b)->script_offs);
//  dump_vga_script(_vc_ptr, num, vga_res_id);
	run_vga_script();
	_vc_ptr = vc_ptr_org;


	if (_game & GAME_SIMON2) {
		if (!_dx_use_3_or_4_for_lock) {
			uint num_lines = _video_palette_mode == 4 ? 134 : 200;
			_vga_var8 = num_lines;
			dx_copy_from_attached_to_2(0, 0, 320, num_lines);
			dx_copy_from_attached_to_3(num_lines);
			_sync_flag_2 = 1;
		}
		_dx_use_3_or_4_for_lock = false;
	} else {
		uint num_lines = _video_palette_mode == 4 ? 134 : 200;
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
			while (*(volatile int *)&_palette_color_count != 0) {
				delay(10);
			}
		}
	}
}

void SimonState::set_video_mode(uint mode, uint vga_res_id)
{

	if (_lock_counter == 0) {
		lock();
		if (_lock_word == 0) {
			_sync_flag_1 = true;
			while ((*(volatile bool *)&_sync_flag_1) == true) {
				delay(10);
			}
		}
	}

	_lock_word |= 0x20;

//  while ((*(volatile uint16*)&_lock_word) & 2) {
//    delay(10);
//  }

	unlock();

	set_video_mode_internal(mode, vga_res_id);
}






void SimonState::o_fade_to_black()
{
	uint i;

	memcpy(_video_buf_1, _palette_backup, 256 * sizeof(uint32));

	i = NUM_PALETTE_FADEOUT;
	do {
		palette_fadeout((uint32 *)_video_buf_1, 32);
		palette_fadeout((uint32 *)_video_buf_1 + 32 + 16, 144);
		palette_fadeout((uint32 *)_video_buf_1 + 32 + 16 + 144 + 16, 48);

		_system->set_palette(_video_buf_1, 0, 256);
		_system->update_screen();
		delay(5);
	} while (--i);

	memcpy(_palette_backup, _video_buf_1, 256 * sizeof(uint32));
	memcpy(_palette, _video_buf_1, 256 * sizeof(uint32));
}

void SimonState::delete_vga_timer(VgaTimerEntry * vte)
{
	_lock_word |= 1;

	if (vte + 1 <= _next_vga_timer_to_process) {
		_next_vga_timer_to_process--;
	}

	do {
		memcpy(vte, vte + 1, sizeof(VgaTimerEntry));
		vte++;
	} while (vte->delay);

	_lock_word &= ~1;
}

void SimonState::expire_vga_timers()
{
	if (_game & GAME_SIMON2) {
		VgaTimerEntry *vte = _vga_timer_list;

		_vga_tick_counter++;

		while (vte->delay) {
			/* not quite ok, good enough */
			if ((int16)(vte->delay -= 5) <= 0) {
				uint16 cur_file = vte->cur_vga_file;
				uint16 cur_unk = vte->sprite_id;
				byte *script_ptr = vte->script_pointer;

				_next_vga_timer_to_process = vte + 1;
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

				_next_vga_timer_to_process = vte + 1;
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
void SimonState::scroll_timeout()
{
	if (_vga_var2 == 0)
		return;

	if (_vga_var2 < 0) {
		if (_vga_var3 != -1) {
			_vga_var3 = -1;
			if (++_vga_var2 == 0)
				return;
		}
	} else {
		if (_vga_var3 != 1) {
			_vga_var3 = 1;
			if (--_vga_var2 == 0)
				return;
		}
	}

	add_vga_timer(10, NULL, 0, 0);
}

void SimonState::vc_resume_thread(byte *code_ptr, uint16 cur_file, uint16 cur_sprite)
{
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


void SimonState::add_vga_timer(uint num, byte *code_ptr, uint cur_sprite, uint cur_file)
{
	VgaTimerEntry *vte;

//  assert( (uint)READ_BE_UINT16_UNALIGNED(&*(uint16*)code_ptr) <= 63);

	_lock_word |= 1;

	for (vte = _vga_timer_list; vte->delay; vte++) {
	}

	vte->delay = num;
	vte->script_pointer = code_ptr;
	vte->sprite_id = cur_sprite;
	vte->cur_vga_file = cur_file;

	_lock_word &= ~1;
}

void SimonState::o_force_unlock()
{
	if (_game & GAME_SIMON2 && _bit_array[4] & 0x8000)
		_mouse_cursor = 0;
	_lock_counter = 0;
}

void SimonState::o_force_lock()
{
	_lock_word |= 0x4000;
	vc_34();
	_lock_word &= ~0x4000;
}

void SimonState::o_save_game()
{
	save_or_load_dialog(false);
}

void SimonState::o_load_game()
{
	save_or_load_dialog(true);
}



int SimonState::display_savegame_list(int curpos, bool load, char *dst)
{
	int slot, last_slot;
	FILE *in;

	showMessageFormat("\xC");

	memset(dst, 0, 18 * 6);

	slot = curpos;

	while (curpos + 6 > slot) {
		in = fopen(gen_savename(slot), "rb");
		if (!in)
			break;

		fread(dst, 1, 18, in);
		fclose(in);
		last_slot = slot;
		if (slot < 10)
			showMessageFormat(" ");
		showMessageFormat("%d", slot);
		showMessageFormat(".%s\n", dst);
		dst += 18;
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
		if (curpos + 6 == slot) {
			in = fopen(gen_savename(slot), "rb");
			if (in != NULL) {
				slot++;
				fclose(in);
			}
		}
	}

	return slot - curpos;
}


void SimonState::savegame_dialog(char *buf)
{
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


void SimonState::save_or_load_dialog(bool load)
{
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
	if (!load)
		num++;
	num -= 6;
	if (num < 0)
		num = 0;
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

	if (i == 205)
		goto get_out;
	if (!load) {
		/* if_1 */
	if_1:;
		unk132_result = i;

		set_hitarea_bit_0x40(0xd0 + i);
		leaveHitAreaById(0xd0 + i);

		/* some code here */

		fcs = _fcs_ptr_array_3[5];

		fcs->textRow = unk132_result;

        // init x offset with a 2 character savegame number + a period (18 pix)
		fcs->textColumn = 2;
		fcs->textColumnOffset = 2;
		fcs->textLength = 3;

		name = buf + i * 18;

        // now process entire savegame name to get correct x offset for cursor
		name_len = 0;
		while (name[name_len]) {
			fcs->textLength++;
			fcs->textColumnOffset += 6;
			if (name[name_len] == 'i' || name[name_len] == 'l')
				fcs->textColumnOffset -= 2;
			if (fcs->textColumnOffset >= 8) {
				fcs->textColumnOffset -= 8;
				fcs->textColumn++;
			}
			name_len++;
		}
		/* while_1_end */

		/* do_3_start */
		for (;;) {
			video_putchar(fcs, 0x7f);

			_saveload_flag = true;

			/* do_2 */
			do {
				i = o_unk_132_helper(&b, buf);

				if (b) {
					if (i == 205)
						goto get_out;
					clear_hitarea_bit_0x40(0xd0 + unk132_result);
					if (_saveload_flag) {
						o_unk_132_helper_2(_fcs_ptr_array_3[5], 8);
						/* move code */
					}
					goto if_1;
				}

				/* is_not_b */
				if (!_saveload_flag) {
					clear_hitarea_bit_0x40(0xd0 + unk132_result);
					goto restart;
				}
			} while (i >= 0x80 || i == 0);

			/* after_do_2 */
			o_unk_132_helper_2(_fcs_ptr_array_3[5], 8);
			if (i == 10 || i == 13)
				break;
			if (i == 8) {
				/* do_backspace */
				if (name_len != 0) {
					int x;

					name_len--;

					x = (name[name_len] == 'i' || name[name_len] == 'l') ? 1 : 8;
					name[name_len] = 0;

					o_unk_132_helper_2(_fcs_ptr_array_3[5], x);
				}
			} else if (i >= 32 && name_len != 17) {
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
//      clear_keydowns();

	_base_time = time(NULL) - save_time + _base_time;
	_copy_partial_mode = 0;

	dx_copy_rgn_from_3_to_2(94, 208, 46, 80);

	i = _timer_4;
	do {
		delay(10);
	} while (i == _timer_4);

#ifdef _WIN32_WCE

	if (draw_keyboard) {
		draw_keyboard = false;
		toolbar_drawn = false;
	}

#endif
}


void SimonState::o_wait_for_vga(uint a)
{
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

//    if (_timer_1 >= 500) {
//      warning("wait timed out");
//      break;
//    }

	}
//  warning("waiting on %d done", a);
}

void SimonState::timer_vga_sprites()
{
	VgaSprite *vsp;
	VgaPointersEntry *vpe;
	byte *vc_ptr_org = _vc_ptr;
	uint16 params[5];							/* parameters to vc_10 */

	if (_video_var_9 == 2)
		_video_var_9 = 1;

#ifdef DRAW_THREE_STARS
	fprintf(_dump_file, "***\n");
#endif

	if (_game & GAME_SIMON2 && _vga_var3) {
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

		if (_game & GAME_SIMON2) {
			*(byte *)(&params[4]) = (byte)vsp->unk4;
		} else {
			params[4] = READ_BE_UINT16_UNALIGNED(&vsp->unk4);
		}

		_vc_ptr = (byte *)params;
		vc_10();

		vsp++;
	}

#ifdef DRAW_IMAGES_DEBUG
	memset(_sdl_buf_attached, 0, 320 * 200);
#endif
	_video_var_8++;
	_vc_ptr = vc_ptr_org;
}

void SimonState::timer_vga_sprites_helper()
{
	byte *dst = dx_lock_2(), *src;
	uint x;

	if (_vga_var3 < 0) {
		memmove(dst + 8, dst, 320 * _vga_var5 - 8);
	} else {
		memmove(dst, dst + 8, 320 * _vga_var5 - 8);
	}

	x = _x_scroll - 1;

	if (_vga_var3 > 0) {
		dst += 320 - 8;
		x += 41;
	}

	src = _vga_var7 + x * 4;
	vc_10_helper_8(dst, src + READ_BE_UINT32_UNALIGNED(&*((uint32 *)src)));

	dx_unlock_2();


	memcpy(_sdl_buf_attached, _sdl_buf, 320 * 200);
	dx_copy_from_attached_to_3(_vga_var5);


	_x_scroll += _vga_var3;

	vc_write_var(0xfB, _x_scroll);

	_vga_var3 = 0;
}

#ifdef DRAW_IMAGES_DEBUG
void SimonState::timer_vga_sprites_2()
{
	VgaSprite *vsp;
	VgaPointersEntry *vpe;
	byte *vc_ptr_org = _vc_ptr;
	uint16 params[5];							/* parameters to vc_10 */

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
			fprintf(_dump_file, "id:%5d image:%3d base-color:%3d x:%3d y:%3d flags:%x\n",
							vsp->id, vsp->image, vsp->base_color, vsp->x, vsp->y, vsp->unk4);
		params[0] = READ_BE_UINT16_UNALIGNED(&vsp->image);
		params[1] = READ_BE_UINT16_UNALIGNED(&vsp->base_color);
		params[2] = READ_BE_UINT16_UNALIGNED(&vsp->x);
		params[3] = READ_BE_UINT16_UNALIGNED(&vsp->y);
		params[4] = READ_BE_UINT16_UNALIGNED(&vsp->unk4);
		_vc_ptr = (byte *)params;
		vc_10();

		vsp++;
	}

#ifdef DRAW_THREE_STARS
	fprintf(_dump_file, "***\n");
#endif

	_video_var_8++;
	_vc_ptr = vc_ptr_org;
}
#endif

void SimonState::timer_proc1()
{
	_timer_4++;

	if (_lock_word & 0xC0E9 || _lock_word & 2)
		return;

	_timer_1++;

	_lock_word |= 2;

	if (!(_lock_word & 0x10)) {
		if (!(_game & GAME_SIMON2)) {
			expire_vga_timers();
			expire_vga_timers();
			_cepe_flag ^= 1;
			if (!_cepe_flag)
				expire_vga_timers();

			_sync_flag_2 ^= 1;
		} else {
			_sync_flag_2 ^= 1;

			if (!_sync_flag_2)
				expire_vga_timers();

			if (_lock_counter != 0 && !_sync_flag_2) {
				_lock_word &= ~2;
				return;
			}
		}

//    if (_lock_counter !=0 && _sync_flag_2==1) {
//      printf("skipping draw...\n");
//      goto get_out;
//    }
	}

	timer_vga_sprites();
#ifdef DRAW_IMAGES_DEBUG
	timer_vga_sprites_2();
#endif

	if (_copy_partial_mode == 1) {
		dx_copy_from_2_to_attached(80, 46, 208 - 80, 94 - 46);
	}

	if (_copy_partial_mode == 2) {
		/* copy partial from attached to 2 */
		dx_copy_from_attached_to_2(176, 61, 320 - 176, 134 - 61);
		_copy_partial_mode = 0;
	}

	/* XXX: more stuff here */
	if (_video_var_8) {
		handle_mouse_moved();
		/* XXX: more stuff here */
		dx_update_screen_and_palette();
		_sync_flag_1 = false;
		_video_var_8 = false;
	}


	_lock_word &= ~2;
}

void SimonState::timer_callback()
{
//  uint32 start, end;

	if (_timer_5 != 0) {
		_sync_flag_2 = true;
		_timer_5--;
	} else {
//    start = timeGetTime();
		timer_proc1();
//    end = timeGetTime();

//    if (start + 45 < end) {
//      _timer_5 = (uint16)( (end - start) / 45);
//    }
	}
}

void SimonState::checkTimerCallback()
{
	if (_invoke_timer_callback && !_in_callback) {
		_in_callback = true;
		_invoke_timer_callback = 0;
		timer_callback();
		_in_callback = false;
	}
}



void SimonState::fcs_setTextColor(FillOrCopyStruct *fcs, uint value)
{
	fcs->text_color = value;
}

void SimonState::o_vga_reset()
{
	_lock_word |= 0x4000;
	vc_27_reset();
	_lock_word &= ~0x4000;
}

bool SimonState::vc_maybe_skip_proc_3(uint16 a)
{
	Item *item;

	CHECK_BOUNDS(a, _vc_item_array);

	item = _vc_item_array[a];
	if (item == NULL)
		return true;

	return getItem1Ptr()->parent == item->parent;
}

bool SimonState::vc_maybe_skip_proc_2(uint16 a, uint16 b)
{
	Item *item_a, *item_b;

	CHECK_BOUNDS(a, _vc_item_array);
	CHECK_BOUNDS(b, _vc_item_array);

	item_a = _vc_item_array[a];
	item_b = _vc_item_array[b];

	if (item_a == NULL || item_b == NULL)
		return true;

	return derefItem(item_a->parent) == item_b;
}

bool SimonState::vc_maybe_skip_proc_1(uint16 a, int16 b)
{
	Item *item;

	CHECK_BOUNDS(a, _vc_item_array);

	item = _vc_item_array[a];
	if (item == NULL)
		return true;
	return item->unk3 == b;
}


/* OK */
void SimonState::fcs_delete(uint a)
{
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
void SimonState::fcs_unk_2(uint a)
{
	a &= 7;

	if (_fcs_ptr_array_3[a] == NULL || _fcs_unk_1 == a)
		return;

	_fcs_unk_1 = a;
	startUp_helper_3();
	_fcs_ptr_1 = _fcs_ptr_array_3[a];

	showmessage_helper_3(_fcs_ptr_1->textLength, _fcs_ptr_1->textMaxLength);
}


/* OK */
FillOrCopyStruct *SimonState::fcs_alloc(uint x, uint y, uint w, uint h, uint flags, uint fill_color,
																				uint unk4)
{
	FillOrCopyStruct *fcs;

	fcs = _fcs_list;
	while (fcs->mode != 0)
		fcs++;

	fcs->mode = 2;
	fcs->x = x;
	fcs->y = y;
	fcs->width = w;
	fcs->height = h;
	fcs->flags = flags;
	fcs->fill_color = fill_color;
	fcs->text_color = unk4;
	fcs->textColumn = 0;
	fcs->textRow = 0;
	fcs->textColumnOffset = 0;
	fcs->textMaxLength = fcs->width * 8 / 6; // characters are 6 pixels
	return fcs;
}

Item *SimonState::derefItem(uint item)
{
	if (item >= _itemarray_size)
		error("derefItem: invalid item %d", item);
	return _itemarray_ptr[item];
}

uint SimonState::itemPtrToID(Item *id)
{
	uint i;
	for (i = 0; i != _itemarray_size; i++)
		if (_itemarray_ptr[i] == id)
			return i;
	error("itemPtrToID: not found");
}

void SimonState::o_pathfind(int x, int y, uint var_1, uint var_2)
{
	uint16 *p;
	uint i, j;
	uint prev_i;
	uint x_diff, y_diff;
	uint best_i = 0, best_j = 0, best_dist = 0xFFFFFFFF;

	if (_game & GAME_SIMON2) {
		x += _x_scroll * 8;
	}

	prev_i = 21 - _variableArray[12];
	for (i = 20; i != 0; --i) {
		p = (uint16 *)_pathfind_array[20 - i];
		if (!p)
			continue;
		for (j = 0; READ_BE_UINT16_UNALIGNED(&p[0]) != 999; j++, p += 2) {	/* 0xE703 = byteswapped 999 */
			x_diff = abs((int)(READ_BE_UINT16_UNALIGNED(&p[0]) - x));
			y_diff = abs((int)(READ_BE_UINT16_UNALIGNED(&p[1]) - 12 - y));

			if (x_diff < y_diff) {
				x_diff >>= 2;
				y_diff <<= 2;
			}
			x_diff += y_diff >> 2;

			if (x_diff < best_dist || x_diff == best_dist && prev_i == i) {
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
void SimonState::fcs_unk1(uint fcs_index)
{
	FillOrCopyStruct *fcs;
	uint16 fcsunk1;
	uint16 i;

	fcs = _fcs_ptr_array_3[fcs_index & 7];
	fcsunk1 = _fcs_unk_1;

	if (fcs == NULL || fcs->fcs_data == NULL)
		return;

	fcs_unk_2(fcs_index);
	fcs_putchar(12);
	fcs_unk_2(fcsunk1);

	for (i = 0; fcs->fcs_data->e[i].item != NULL; i++) {
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
void SimonState::fcs_unk_5(FillOrCopyStruct *fcs, uint fcs_index)
{
	if (!(_game & GAME_SIMON2)) {
		o_unk_99_simon1(0x80);
	}
}

void SimonState::delete_hitarea_by_index(uint index)
{
	CHECK_BOUNDS(index, _hit_areas);
	_hit_areas[index].flags = 0;
}

/* ok */
void SimonState::fcs_putchar(uint a)
{
	if (_fcs_ptr_1 != _fcs_ptr_array_3[0])
		video_putchar(_fcs_ptr_1, a);
}

/* ok */
void SimonState::video_fill_or_copy_from_3_to_2(FillOrCopyStruct *fcs)
{
	if (fcs->flags & 0x10)
		copy_img_from_3_to_2(fcs);
	else
		video_erase(fcs);

	fcs->textColumn = 0;
	fcs->textRow = 0;
	fcs->textColumnOffset = 0;
	fcs->textLength = 0;
}

/* ok */
void SimonState::copy_img_from_3_to_2(FillOrCopyStruct *fcs)
{
	_lock_word |= 0x8000;

	if (!(_game & GAME_SIMON2)) {
		dx_copy_rgn_from_3_to_2(fcs->y + fcs->height * 8 + ((fcs == _fcs_ptr_array_3[2]) ? 1 : 0),
														(fcs->x + fcs->width) * 8, fcs->y, fcs->x * 8);
	} else {
		if (_vga_var6 && _fcs_ptr_array_3[2] == fcs) {
			fcs = _fcs_ptr_array_3[0x18 / 4];
			_vga_var6 = 0;
		}

		dx_copy_rgn_from_3_to_2(fcs->y + fcs->height * 8,
														(fcs->x + fcs->width) * 8, fcs->y, fcs->x * 8);
	}

	_lock_word &= ~0x8000;
}

void SimonState::video_erase(FillOrCopyStruct *fcs)
{
	byte *dst;
	uint h;

	_lock_word |= 0x8000;

	dst = dx_lock_2();
	dst += _dx_surface_pitch * fcs->y + fcs->x * 8;

	h = fcs->height * 8;
	do {
		memset(dst, fcs->fill_color, fcs->width * 8);
		dst += _dx_surface_pitch;
	} while (--h);

	dx_unlock_2();
	_lock_word &= ~0x8000;
}

VgaSprite *SimonState::find_cur_sprite()
{
	if (_game & GAME_SIMON2) {
		VgaSprite *vsp = _vga_sprites;
		while (vsp->id) {
			if (vsp->id == _vga_cur_sprite_id && vsp->unk7 == _vga_cur_file_id)
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

bool SimonState::has_vgastruct_with_id(uint16 id, uint16 file)
{
	if (_game & GAME_SIMON2) {
		VgaSprite *vsp = _vga_sprites;
		while (vsp->id) {
			if (vsp->id == id && vsp->unk7 == file)
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

void SimonState::processSpecialKeys()
{
}

void SimonState::draw_mouse_pointer()
{
}


void decompress_icon(byte *dst, byte *src, uint w, uint h_org, byte base, uint pitch)
{
	int8 reps;
	byte color_1, color_2;
	byte *dst_org = dst;
	uint h = h_org;

	for (;;) {
		reps = *src++;
		if (reps < 0) {
			reps--;
			color_1 = *src >> 4;
			if (color_1 != 0)
				color_1 |= base;
			color_2 = *src++ & 0xF;
			if (color_2 != 0)
				color_2 |= base;

			do {
				if (color_1 != 0)
					*dst = color_1;
				dst += pitch;
				if (color_2 != 0)
					*dst = color_2;
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
				if (color_1 != 0)
					*dst = color_1 | base;
				dst += pitch;

				color_2 = *src++ & 0xF;
				if (color_2 != 0)
					*dst = color_2 | base;
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


void SimonState::draw_icon_c(FillOrCopyStruct *fcs, uint icon, uint x, uint y)
{
	byte *dst;
	byte *src;

	if (!(_game & GAME_SIMON2)) {
		_lock_word |= 0x8000;

		dst = dx_lock_2();
		dst += (x + fcs->x) * 8;
		dst += (y * 25 + fcs->y) * _dx_surface_pitch;

		src = _icon_file_ptr;
		src += READ_LE_UINT16(&((uint16 *)src)[icon]);

		decompress_icon(dst, src, 24, 12, 0xE0, _dx_surface_pitch);

		dx_unlock_2();
		_lock_word &= ~0x8000;
	} else {
		_lock_word |= 0x8000;
		dst = dx_lock_2();

		dst += 110;
		dst += x;
		dst += (y + fcs->y) * _dx_surface_pitch;

		src = _icon_file_ptr;
		src += READ_LE_UINT16(&((uint16 *)src)[icon * 2 + 0]);
		decompress_icon(dst, src, 20, 10, 0xE0, _dx_surface_pitch);

		src = _icon_file_ptr;
		src += READ_LE_UINT16(&((uint16 *)src)[icon * 2 + 1]);
		decompress_icon(dst, src, 20, 10, 0xD0, _dx_surface_pitch);

		dx_unlock_2();
		_lock_word &= ~0x8000;
	}
}

void SimonState::video_toggle_colors(HitArea * ha, byte a, byte b, byte c, byte d)
{
	byte *src, color;
	uint w, h, i;

	_lock_word |= 0x8000;
	src = dx_lock_2() + ha->y * _dx_surface_pitch + ha->x;

	w = ha->width;
	h = ha->height;

	if (!(h > 0 && w > 0 && ha->x + w <= 320 && ha->y + h <= 200)) {
		warning("Invalid coordinates in video_toggle_colors (%d,%d,%d,%d)", ha->x, ha->y, ha->width,
						ha->height);
		_lock_word &= ~0x8000;
		return;
	}

	do {
		for (i = 0; i != w; ++i) {
			color = src[i];
			if (a >= color && b < color) {
				if (c >= color)
					color += d;
				else
					color -= d;
				src[i] = color;
			}
		}
		src += _dx_surface_pitch;
	} while (--h);


	dx_unlock_2();
	_lock_word &= ~0x8000;
}

bool SimonState::vc_59_helper()
{
#ifdef USE_TEXT_HACK
	return true;
#else
	if (_voice_file == NULL)
		return false;
	return _voice_sound == 0;
#endif
}

void SimonState::video_copy_if_flag_0x8_c(FillOrCopyStruct *fcs)
{
	if (fcs->flags & 8)
		copy_img_from_3_to_2(fcs);
	fcs->mode = 0;
}

void SimonState::showMessageFormat(const char *s, ...)
{
	char buf[1024], *str;
	va_list va;

	va_start(va, s);
	vsprintf(buf, s, va);
	va_end(va);

	if (!_fcs_data_1[_fcs_unk_1]) {
		showmessage_helper_2();
		if (!_showmessage_flag) {
			_fcs_ptr_array_3[0] = _fcs_ptr_1;
			showmessage_helper_3(_fcs_ptr_1->textLength,
                                 _fcs_ptr_1->textMaxLength);
		}
		_showmessage_flag = true;
		_fcs_data_1[_fcs_unk_1] = 1;
	}

	for (str = buf; *str; str++)
		showmessage_print_char(*str);
}

void SimonState::showmessage_helper_2()
{
	if (_fcs_ptr_1)
		return;

	_fcs_ptr_1 = fcs_alloc(8, 0x90, 0x18, 6, 1, 0, 0xF);
}

void SimonState::readSfxFile(const char *filename)
{
	if (!(_game & GAME_SIMON2)) {
		FILE *in;
		uint32 size;

		in = fopen_maybe_lowercase(filename);

		if (in == NULL) {
			warning("readSfxFile: Cannot load sfx file %s", filename);
			return;
		}

		fseek(in, 0, SEEK_END);
		size = ftell(in);

		fseek(in, 0, SEEK_SET);

		/* stop all sounds */
		_mixer->stop_all();

		if (_sfx_heap)
			free(_sfx_heap);

		_sfx_heap = (byte *)malloc(size);

		if (_sfx_heap == NULL)
			error("readSfxFile: Not enough SFX memory");

		fread(_sfx_heap, size, 1, in);

		fclose(in);
	} else {
		int res;
		uint32 offs;
		int size;

		vc_29_stop_all_sounds();

		if (_sfx_heap)
			free(_sfx_heap);

		res = atoi(filename + 6) + gss->SOUND_INDEX_BASE - 1;
		offs = _game_offsets_ptr[res];
		size = _game_offsets_ptr[res + 1] - offs;

		if (size == 0)
			return;

		_sfx_heap = (byte *)malloc(size);

		resfile_read(_sfx_heap, offs, size);
	}
}

void SimonState::video_putchar(FillOrCopyStruct *fcs, byte c)
{
	if (c == 0xC) {
		video_fill_or_copy_from_3_to_2(fcs);
	} else if (c == 0xD || c == 0xA) {
		video_putchar_newline(fcs);
	} else if (c == 8 || c == 1) {
		int8 val = (c == 8) ? 6 : 4;
		if (fcs->textLength != 0) {
			fcs->textLength--;
			fcs->textColumnOffset -= val;
			if ((int8)fcs->textColumnOffset < val) {
				fcs->textColumnOffset += 8;
				fcs->textColumn--;
			}
		}
	} else if (c >= 0x20) {
		if (fcs->textLength == fcs->textMaxLength) {
			video_putchar_newline(fcs);
		} else if (fcs->textRow == fcs->height) {
			video_putchar_newline(fcs);
			fcs->textRow--;
		}

		video_putchar_drawchar(fcs, fcs->textColumn + fcs->x, 
                               fcs->textRow * 8 + fcs->y, c);

		fcs->textLength++;
		fcs->textColumnOffset += 6;
		if (c == 'i' || c == 'l')
			fcs->textColumnOffset -= 2;

		if (fcs->textColumnOffset >= 8) {
			fcs->textColumnOffset -= 8;
			fcs->textColumn++;
		}
	}
}

void SimonState::video_putchar_newline(FillOrCopyStruct *fcs)
{
	fcs->textColumnOffset = 0;
	fcs->textLength = 0;
	fcs->textColumn = 0;

	if (fcs->textRow != fcs->height)
		fcs->textRow++;
}

static const byte video_font[] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	32, 112, 112, 32, 32, 0, 32, 0,
	48, 48, 96, 0, 0, 0, 0, 0,
	0, 144, 0, 96, 144, 144, 104, 0,
	0, 144, 0, 96, 144, 144, 96, 0,
	0, 144, 0, 144, 144, 144, 96, 0,
	0, 16, 40, 16, 42, 68, 58, 0,
	48, 48, 96, 0, 0, 0, 0, 0,
	0, 4, 8, 8, 8, 8, 4, 0,
	0, 32, 16, 16, 16, 16, 32, 0,
	0, 0, 20, 8, 62, 8, 20, 0,
	0, 112, 136, 240, 136, 136, 240, 0,
	0, 0, 0, 0, 0, 48, 48, 96,
	0, 0, 0, 240, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 48, 48, 0,
	16, 32, 0, 120, 112, 64, 56, 0,
	112, 136, 152, 168, 200, 136, 112, 0,
	32, 96, 32, 32, 32, 32, 112, 0,
	112, 136, 8, 48, 64, 136, 248, 0,
	112, 136, 8, 48, 8, 136, 112, 0,
	16, 48, 80, 144, 248, 16, 56, 0,
	248, 128, 240, 8, 8, 136, 112, 0,
	48, 64, 128, 240, 136, 136, 112, 0,
	248, 136, 8, 16, 32, 32, 32, 0,
	112, 136, 136, 112, 136, 136, 112, 0,
	112, 136, 136, 120, 8, 16, 96, 0,
	0, 0, 48, 48, 0, 48, 48, 0,
	32, 16, 0, 112, 8, 248, 120, 0,
	32, 80, 0, 144, 144, 144, 104, 0,
	32, 16, 0, 112, 248, 128, 112, 0,
	32, 80, 0, 112, 248, 128, 112, 0,
	112, 136, 8, 16, 32, 0, 32, 0,
	32, 80, 0, 192, 64, 64, 224, 0,
	112, 136, 136, 248, 136, 136, 136, 0,
	240, 72, 72, 112, 72, 72, 240, 0,
	48, 72, 128, 128, 128, 72, 48, 0,
	224, 80, 72, 72, 72, 80, 224, 0,
	248, 72, 64, 112, 64, 72, 248, 0,
	248, 72, 64, 112, 64, 64, 224, 0,
	48, 72, 128, 152, 136, 72, 56, 0,
	136, 136, 136, 248, 136, 136, 136, 0,
	248, 32, 32, 32, 32, 32, 248, 0,
	24, 8, 8, 8, 136, 136, 112, 0,
	200, 72, 80, 96, 80, 72, 200, 0,
	224, 64, 64, 64, 64, 72, 248, 0,
	136, 216, 168, 168, 136, 136, 136, 0,
	136, 200, 168, 152, 136, 136, 136, 0,
	112, 136, 136, 136, 136, 136, 112, 0,
	240, 72, 72, 112, 64, 64, 224, 0,
	112, 136, 136, 136, 136, 168, 112, 8,
	240, 72, 72, 112, 72, 72, 200, 0,
	112, 136, 128, 112, 8, 136, 112, 0,
	248, 168, 32, 32, 32, 32, 112, 0,
	136, 136, 136, 136, 136, 136, 120, 0,
	136, 136, 136, 80, 80, 32, 32, 0,
	136, 136, 136, 136, 168, 216, 136, 0,
	136, 136, 80, 32, 80, 136, 136, 0,
	136, 136, 136, 112, 32, 32, 112, 0,
	248, 136, 16, 32, 64, 136, 248, 0,
	0, 14, 8, 8, 8, 8, 14, 0,
	0, 128, 64, 32, 16, 8, 4, 0,
	0, 112, 16, 16, 16, 16, 112, 0,
	0, 48, 72, 64, 72, 48, 16, 48,
	0, 80, 0, 96, 32, 40, 48, 0,
	32, 16, 0, 152, 144, 144, 232, 0,
	0, 0, 112, 8, 120, 136, 120, 0,
	192, 64, 80, 104, 72, 72, 112, 0,
	0, 0, 112, 136, 128, 136, 112, 0,
	24, 16, 80, 176, 144, 144, 112, 0,
	0, 0, 112, 136, 248, 128, 112, 0,
	48, 72, 64, 224, 64, 64, 224, 0,
	0, 0, 104, 144, 144, 112, 136, 112,
	192, 64, 80, 104, 72, 72, 200, 0,
	64, 0, 192, 64, 64, 64, 224, 0,
	8, 0, 8, 8, 8, 8, 136, 112,
	192, 64, 72, 80, 96, 80, 200, 0,
	192, 64, 64, 64, 64, 64, 224, 0,
	0, 0, 144, 216, 168, 136, 136, 0,
	0, 0, 240, 136, 136, 136, 136, 0,
	0, 0, 112, 136, 136, 136, 112, 0,
	0, 0, 176, 72, 72, 112, 64, 224,
	0, 0, 104, 144, 144, 112, 16, 56,
	0, 0, 176, 72, 72, 64, 224, 0,
	0, 0, 120, 128, 112, 8, 240, 0,
	64, 64, 240, 64, 64, 72, 48, 0,
	0, 0, 144, 144, 144, 144, 104, 0,
	0, 0, 136, 136, 136, 80, 32, 0,
	0, 0, 136, 136, 168, 216, 144, 0,
	0, 0, 136, 80, 32, 80, 136, 0,
	0, 0, 136, 136, 136, 112, 32, 192,
	0, 0, 248, 144, 32, 72, 248, 0,
	32, 80, 0, 96, 144, 144, 96, 0,
	0, 14, 8, 48, 8, 8, 14, 0,
	0, 8, 8, 8, 8, 8, 8, 0,
	0, 112, 16, 12, 16, 16, 112, 0,
	0, 0, 0, 0, 0, 0, 248, 0,
	252, 252, 252, 252, 252, 252, 252, 252,
	240, 240, 240, 240, 240, 240, 240, 240,
};

void SimonState::video_putchar_drawchar(FillOrCopyStruct *fcs, uint x, uint y, byte chr)
{
	const byte *src;
	byte color, *dst;
	uint h, i;

	_lock_word |= 0x8000;

	dst = dx_lock_2();
	dst += y * _dx_surface_pitch + x * 8 + fcs->textColumnOffset;

	src = video_font + (chr - 0x20) * 8;

	color = fcs->text_color;

	h = 8;
	do {
		int8 b = *src++;
		i = 0;
		do {
			if (b < 0)
				dst[i] = color;
			b <<= 1;
		} while (++i != 6);
		dst += _dx_surface_pitch;
	} while (--h);

	dx_unlock_2();

	_lock_word &= ~0x8000;
}

void SimonState::start_vga_code(uint b, uint vga_res, uint vga_struct_id, uint c, uint d, uint f)
{
	VgaSprite *vsp;
	VgaPointersEntry *vpe;
	byte *p, *pp;
	uint count;

	_lock_word |= 0x40;

	if (has_vgastruct_with_id(vga_struct_id, vga_res)) {
		_lock_word &= ~0x40;
		return;
	}

	vsp = _vga_sprites;
	while (vsp->id != 0)
		vsp++;

	vsp->unk6 = b;
	vsp->unk5 = 0;
	vsp->unk4 = 0;

	vsp->y = d;
	vsp->x = c;
	vsp->image = 0;
	vsp->base_color = f;
	vsp->id = vga_struct_id;
	vsp->unk7 = vga_res;

	for (;;) {
		vpe = &_vga_buffer_pointers[vga_res];
		_vga_cur_file_2 = vga_res;
		_cur_vga_file_1 = vpe->vgaFile1;
		if (vpe->vgaFile1 != NULL)
			break;
		ensureVgaResLoaded(vga_res);
	}

	pp = _cur_vga_file_1;
	p = pp + READ_BE_UINT16_UNALIGNED(&((VgaFile1Header *) pp)->hdr2_start);

	count = READ_BE_UINT16_UNALIGNED(&((VgaFile1Header2 *) p)->id_count);
	p = pp + READ_BE_UINT16_UNALIGNED(&((VgaFile1Header2 *) p)->id_table);

	for (;;) {
		if (READ_BE_UINT16_UNALIGNED(&((VgaFile1Struct0x6 *) p)->id) == vga_struct_id) {

			//dump_vga_script(pp + READ_BE_UINT16_UNALIGNED(&((VgaFile1Struct0x6*)p)->script_offs), vga_res, vga_struct_id);

			add_vga_timer(gss->VGA_DELAY_BASE,
										pp + READ_BE_UINT16_UNALIGNED(&((VgaFile1Struct0x6 *) p)->script_offs),
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

void SimonState::talk_with_speech(uint speech_id, uint num_1)
{
	if (!(_game & GAME_SIMON2)) {
		if (speech_id == 9999) {
			if (!(_bit_array[0] & 0x4000) && !(_bit_array[1] & 0x1000)) {
				_bit_array[0] |= 0x4000;
				_variableArray[0xc8 / 2] = 0xF;
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
			start_vga_code(4, 2, num_1 + 201, 0, 0, 0);
		}
	} else {
		if (speech_id == 0xFFFF) {
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
				o_unk_99_simon2(2, 5);
			}
			o_unk_99_simon2(2, num_1 + 2);
			playVoice(speech_id);

			start_vga_code(4, 2, num_1 + 2, 0, 0, 0);
		}
	}
}

void SimonState::talk_with_text(uint num_1, uint num_2, const char *string_ptr, uint threeval_a,
																int threeval_b, uint width)
{
	char print_str_buf[0x140];
	char *char_buf;
	const char *string_ptr_2, *string_ptr_3;
	int j;
	uint letters_per_row, len_div_3, num_of_rows;
	uint m, n;
	uint height;

	if (num_1 >= 100)							// FIXME: Simon1 Mine - Fix text for dwarf song
		num_1 -= 100;

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
		if (_variableArray[86] == 0)
			len_div_3 >>= 1;
		if (_variableArray[86] == 2)
			len_div_3 <<= 1;
		_variableArray[85] = len_div_3 * 5;
	}

	num_of_rows = strlen(string_ptr) / letters_per_row;

	while (num_of_rows == 1 && j != -1) {
		m = strlen(string_ptr) >> 1;
		m -= j;
		string_ptr_2 += m;

		while (*string_ptr_2++ != ' ' && m <= letters_per_row)
			m++;

		if (m <= letters_per_row && strlen(string_ptr_2) < letters_per_row) {
			/* if_1 */
			n = (letters_per_row - m + 1) >> 1;

			while (n != 0) {
				*char_buf++ = ' ';
				n--;
			}
			strncpy(char_buf, string_ptr, m);
			char_buf += m;
			*char_buf++ = 10;

			height += 10;
			threeval_b -= 10;
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

	if (j != -1 && width * 30 > 8000)
		num_of_rows = 4;

	while (num_of_rows == 2 && j != -1) {
		m = strlen(string_ptr) / 3;
		m += j;
		string_ptr_2 += m;

		while (*string_ptr_2++ != ' ' && m <= letters_per_row)
			m++;

		if (m <= letters_per_row) {
			/* if_4 */
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

			while (*string_ptr_2-- != ' ' && m > 0)
				m--;
			/* while_6_end */

			string_ptr_2 += 2;

			if (strlen(string_ptr_2) <= m && m > 0) {
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

	if (j != -1 && width * 40 > 8000)
		num_of_rows = 4;

	/* while_8 */
	while (num_of_rows == 3 && j != -1) {
		m = strlen(string_ptr) >> 2;
		m += j;
		string_ptr_2 += m;
		while (*string_ptr_2++ != ' ' && m <= letters_per_row)
			m++;

		if (m <= letters_per_row) {
			/* if_10 */
			n = (letters_per_row - m + 1) >> 1;
			while (n) {
				*char_buf++ = ' ';
				n--;
			}
			strncpy(char_buf, string_ptr, m);
			char_buf += m;
			*char_buf++ = '\n';
			string_ptr = string_ptr_2;
			string_ptr_2 += m;
			while (*string_ptr_2-- != ' ' && m > 0)
				m--;
			string_ptr_2 += 2;

			if (strlen(string_ptr_2) < m * 2 && m > 0) {
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

				while (*string_ptr_2-- != ' ' && m > 0)
					m--;
				string_ptr_2 += 2;

				if (strlen(string_ptr_2) <= m && m > 0) {
					/* if_15 */
					n = (letters_per_row - m + 1) >> 1;
					while (n) {
						*char_buf++ = ' ';
						n--;
					}
					strncpy(char_buf, string_ptr, m);
					char_buf += m;
					*char_buf++ = '\n';
					height += 30;
					threeval_b -= 30;
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
			while (*string_ptr_2-- != ' ')
				m--;
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
			string_ptr = string_ptr_2;
		}
	}

	n = (letters_per_row - strlen(string_ptr_2) + 1) >> 1;
	while (n) {
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


	if (threeval_b < 2)
		threeval_b = 2;							// Fixme (pos): look at mine
	// ladder, look at gorge, etc

	if (!(_game & GAME_SIMON2)) {
		start_vga_code(num_of_rows, 2, 199 + num_1, threeval_a >> 3, threeval_b, 12);
	} else {
		start_vga_code(num_of_rows, 2, num_1, threeval_a >> 3, threeval_b, 12);
	}
}

void SimonState::render_string(uint num_1, uint color, uint width, uint height, const char *txt)
{
	VgaPointersEntry *vpe = &_vga_buffer_pointers[2];
	byte *src, *dst, *p, *dst_org, chr;
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

	*(uint16 *)(p + 4) = TO_BE_16(height);
	*(uint16 *)(p + 6) = TO_BE_16(width);
	dst += READ_BE_UINT32_UNALIGNED(p);

	memset(dst, 0, count);

	dst_org = dst;
	while ((chr = *txt++) != 0) {
		if (chr == 10) {
			dst_org += width * 10;
			dst = dst_org;
		} else if ((chr -= ' ') == 0) {
			dst += 6;
		} else {
			byte *img_hdr = src + 48 + chr * 4;
			uint img_height = img_hdr[2];
			uint img_width = img_hdr[3], i;
			byte *img = src + READ_LE_UINT16(img_hdr);
			byte *cur_dst = dst;

			assert(img_width > 0 && img_width < 50 && img_height > 0 && img_height < 50);

			do {
				for (i = 0; i != img_width; i++) {
					chr = *img++;
					if (chr) {
						if (chr == 0xF)
							chr = 207;
						else
							chr += color;
						cur_dst[i] = chr;
					}
				}
				cur_dst += width;
			} while (--img_height);

			dst += img_width - 1;
		}
	}

}

void SimonState::showmessage_print_char(byte chr)
{
	if (chr == 12) {
		_num_letters_to_print = 0;
		_print_char_unk_1 = 0;
		print_char_helper_1(&chr, 1);
		print_char_helper_5(_fcs_ptr_1);
	} else if (chr == 0 || chr == ' ' || chr == 10) {
		if (_print_char_unk_2 - _print_char_unk_1 >= _num_letters_to_print) {
			_print_char_unk_1 += _num_letters_to_print;
			print_char_helper_1(_letters_to_print_buf, _num_letters_to_print);

			if (_print_char_unk_1 == _print_char_unk_2) {
				_print_char_unk_1 = 0;
			} else {
				if (chr)
					print_char_helper_1(&chr, 1);
				if (chr == 10)
					_print_char_unk_1 = 0;
				else if (chr != 0)
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
				print_char_helper_1(&chr, 1);
				_print_char_unk_1 = 0;
			}
		}
		_num_letters_to_print = 0;
	} else {
		_letters_to_print_buf[_num_letters_to_print++] = chr;
	}
}

void SimonState::print_char_helper_1(const byte *src, uint len)
{
	uint ind;

	if (_fcs_ptr_1 == NULL)
		return;

	while (len-- != 0) {
		if (*src != 12 && _fcs_ptr_1->fcs_data != NULL &&
				_fcs_data_1[ind = get_fcs_ptr_3_index(_fcs_ptr_1)] != 2) {

			_fcs_data_1[ind] = 2;
			_fcs_data_2[ind] = 1;
		}

		fcs_putchar(*src++);
	}
}

void SimonState::print_char_helper_5(FillOrCopyStruct *fcs)
{
	uint index = get_fcs_ptr_3_index(fcs);
	print_char_helper_6(index);
	_fcs_data_1[index] = 0;
}

void SimonState::print_char_helper_6(uint i)
{
	FillOrCopyStruct *fcs;

	if (_fcs_data_2[i]) {
		lock();
		fcs = _fcs_ptr_array_3[i];
		fcs_unk_proc_1(i, fcs->fcs_data->item_ptr, fcs->fcs_data->unk1, fcs->fcs_data->unk2);
		_fcs_data_2[i] = 0;
		unlock();
	}
}

void SimonState::read_vga_from_datfile_1(uint vga_id)
{
	if (_game == GAME_SIMON1DOS) {
		FILE *in;
		char buf[50];
		uint32 size;

        // FIXME - weird hack to make the beard show up when wearing it (see bug #590800)
        if (vga_id == 328)
            sprintf(buf, "0119.VGA");
        else
		sprintf(buf, "%.3d%d.VGA", vga_id >> 1, (vga_id & 1) + 1);

		in = fopen_maybe_lowercase(buf);
		if (in == NULL) {
			warning("read_vga_from_datfile_1: cannot open %s", buf);
			return;
		}

		fseek(in, 0, SEEK_END);
		size = ftell(in);
		fseek(in, 0, SEEK_SET);

		if (fread(_vga_buffer_pointers[11].vgaFile2, size, 1, in) != 1)
			error("read_vga_from_datfile_1: read failed");

		fclose(in);
	} else {
		uint32 offs_a = _game_offsets_ptr[vga_id];
		uint32 size = _game_offsets_ptr[vga_id + 1] - offs_a;

		resfile_read(_vga_buffer_pointers[11].vgaFile2, offs_a, size);
	}
}

byte *SimonState::read_vga_from_datfile_2(uint id)
{
	if (_game == GAME_SIMON1DOS) {
		FILE *in;
		char buf[50];
		uint32 size;
		byte *dst;

		sprintf(buf, "%.3d%d.VGA", id >> 1, (id & 1) + 1);

		in = fopen_maybe_lowercase(buf);
		if (in == NULL)
			error("read_vga_from_datfile_2: cannot open %s", buf);

		fseek(in, 0, SEEK_END);
		size = ftell(in);
		fseek(in, 0, SEEK_SET);

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

void SimonState::resfile_read(void *dst, uint32 offs, uint32 size)
{
	if (fseek(_game_file, offs, SEEK_SET) != 0)
		error("resfile_read(%d,%d) seek failed", offs, size);
	if (fread(dst, size, 1, _game_file) != 1)
		error("resfile_read(%d,%d) read failed", offs, size);
}


void SimonState::openGameFile()
{
	if (_game != GAME_SIMON1DOS) {
		_game_file = fopen_maybe_lowercase(gss->gme_filename);

		if (_game_file == NULL)
			error("cannot open game file '%s'", gss->gme_filename);

		_game_offsets_ptr = (uint32 *)malloc(gss->NUM_GAME_OFFSETS * sizeof(uint32));
		if (_game_offsets_ptr == NULL)
			error("out of memory, game offsets");

		resfile_read(_game_offsets_ptr, 0, gss->NUM_GAME_OFFSETS * sizeof(uint32));
#if defined(SCUMM_BIG_ENDIAN)
		for (int r = 0; r < gss->NUM_GAME_OFFSETS; r++)
			_game_offsets_ptr[r] = READ_LE_UINT32(&_game_offsets_ptr[r]);
#endif
	}

	loadIconFile();

	_system->init_size(320, 200);

	startUp(1);
}

void SimonState::startUp(uint a)
{
	if (a == 1)
		startUp_helper();
}

void SimonState::startUp_helper()
{
	runSubroutine101();
	startUp_helper_2();
}

void SimonState::runSubroutine101()
{
	Subroutine *sub;

	sub = getSubroutineByID(101);
	if (sub != NULL)
		startSubroutineEx(sub);

	startUp_helper_2();
}

#if 0
void SimonState::generateSound(byte *ptr, int len)
{
	uint cur;

	cur = _voice_size;
	if (cur > (uint) len)
		cur = (uint) len;
	_voice_size -= cur;

	if (cur != 0) {
		fread(ptr, cur, 1, _voice_file);
	}

	memset(ptr + cur, 0x80, len - cur);

	cur = _sound_size;
	if (cur) {
		uint i;

		if (cur > (uint) len)
			cur = (uint) len;

		for (i = 0; i != cur; i++) {
			ptr[i] += _sound_ptr[i] ^ 0x80;
		}

		_sound_size -= cur;
		_sound_ptr += cur;
	}
}
#endif

//static void fill_sound(void *userdata, int16 *stream, int len) {
//  ((SimonState*)userdata)->generateSound((byte*)stream, len*2);
//}

void SimonState::dx_copy_rgn_from_3_to_2(uint b, uint r, uint y, uint x)
{
	byte *dst, *src;
	uint i;

	dst = dx_lock_2();
	src = _sdl_buf_3;

	dst += y * _dx_surface_pitch;
	src += y * _dx_surface_pitch;

	while (y < b) {
		for (i = x; i < r; i++)
			dst[i] = src[i];
		y++;
		dst += _dx_surface_pitch;
		src += _dx_surface_pitch;
	}

	dx_unlock_2();
}

void SimonState::dx_clear_surfaces(uint num_lines)
{
	memset(_sdl_buf_attached, 0, num_lines * 320);

	_system->copy_rect(_sdl_buf_attached, 320, 0, 0, 320, 200);

	if (_dx_use_3_or_4_for_lock) {
		memset(_sdl_buf, 0, num_lines * 320);
		memset(_sdl_buf_3, 0, num_lines * 320);
	}
}

void SimonState::dx_clear_attached_from_top(uint lines)
{
	memset(_sdl_buf_attached, 0, lines * 320);
}

void SimonState::dx_copy_from_attached_to_2(uint x, uint y, uint w, uint h)
{
	uint offs = x + y * 320;
	byte *s = _sdl_buf_attached + offs;
	byte *d = _sdl_buf + offs;

	do {
		memcpy(d, s, w);
		d += 320;
		s += 320;
	} while (--h);
}

void SimonState::dx_copy_from_2_to_attached(uint x, uint y, uint w, uint h)
{
	uint offs = x + y * 320;
	byte *s = _sdl_buf + offs;
	byte *d = _sdl_buf_attached + offs;

	do {
		memcpy(d, s, w);
		d += 320;
		s += 320;
	} while (--h);
}



void SimonState::dx_copy_from_attached_to_3(uint lines)
{
	memcpy(_sdl_buf_3, _sdl_buf_attached, lines * 320);
}

void SimonState::dx_update_screen_and_palette()
{
	_num_screen_updates++;

	if (_palette_color_count == 0 && _video_var_9 == 1) {
		_video_var_9 = 0;
		if (memcmp(_palette, _palette_backup, 256 * 4) != 0) {
			memcpy(_palette_backup, _palette, 256 * 4);
			_system->set_palette(_palette, 0, 256);
		}
	}

	if (!_fast_mode || !(rand() & 7)) {

		if (_mouse_pos_changed) {
			_mouse_pos_changed = false;
			_system->set_mouse_pos(_sdl_mouse_x, _sdl_mouse_y);
		}
		_system->copy_rect(_sdl_buf_attached, 320, 0, 0, 320, 200);
		_system->update_screen();
	}

	memcpy(_sdl_buf_attached, _sdl_buf, 320 * 200);

	if (_palette_color_count != 0) {
		if (!(_game & GAME_SIMON2) && _use_palette_delay) {
			delay(100);
			_use_palette_delay = false;
		}
		realizePalette();
	}
}


void SimonState::realizePalette()
{
	if (_palette_color_count & 0x8000) {
		error("_palette_color_count&0x8000");
	}
	_video_var_9 = false;
	memcpy(_palette_backup, _palette, 256 * 4);

	_system->set_palette(_palette, 0, _palette_color_count);
	_palette_color_count = 0;

}


void SimonState::go()
{
	OSystem::Property prop;

	if (!_dump_file)
		_dump_file = stdout;

	/* allocate buffers */
	_sdl_buf_3 = (byte *)calloc(320 * 200, 1);
	_sdl_buf = (byte *)calloc(320 * 200, 1);
	_sdl_buf_attached = (byte *)calloc(320 * 200, 1);

	if (_game == GAME_SIMON2WIN) {
		gss = &simon2win_settings;
	} else if (_game == GAME_SIMON2DOS) {
		gss = &simon2dos_settings;
	} else {
		gss = &simon1_settings;
	}

	allocItemHeap();
	allocTablesHeap();

	setup_vga_file_buf_pointers();

	initSound();

	if (!loadGamePcFile(gss->gamepc_filename))
		error("Error loading gamepc file '%s' (or one of the files it depends on)",
					gss->gamepc_filename);

	addTimeEvent(0, 1);
	openGameFile();

	_last_music_played = (uint) - 1;
	_vga_base_delay = 1;
	_vk_t_toggle = true;

	prop.show_cursor = true;
	_system->property(OSystem::PROP_SHOW_DEFAULT_CURSOR, &prop);

	while (1) {
		hitarea_stuff();
		handle_verb_clicked(_verb_hitarea);
		delay(100);
	}
}

void SimonState::shutdown()
{
	if (_game_file) {
		fclose(_game_file);
		_game_file = NULL;
	}
}

void SimonState::delay(uint delay)
{
	OSystem::Event event;

	uint32 start = _system->get_msecs();
	uint32 cur = start;
	const uint vga_period = _fast_mode ? 10 : 50;

	do {
		while (!_in_callback && cur >= _last_vga_tick + vga_period) {
			_last_vga_tick += vga_period;

			/* don't get too many frames behind */
			if (cur >= _last_vga_tick + vga_period * 2)
				_last_vga_tick = cur;

			_in_callback = true;
			timer_callback();
			_in_callback = false;
		}

		while (_system->poll_event(&event)) {
			switch (event.event_code) {
				case OSystem::EVENT_KEYDOWN:
				if (event.kbd.keycode == 't') {
					_vk_t_toggle ^= 1;
				} else if (event.kbd.flags == OSystem::KBD_CTRL) {
					if (event.kbd.keycode == 'f') {
						_fast_mode ^= 1;
					}
				}
				_key_pressed = (byte)event.kbd.ascii;
				break;
				case OSystem::EVENT_MOUSEMOVE:
				_sdl_mouse_x = event.mouse.x;
				_sdl_mouse_y = event.mouse.y;
				_mouse_pos_changed = true;
				break;

				case OSystem::EVENT_LBUTTONDOWN:
				_left_button_down++;
#ifdef _WIN32_WCE
				_sdl_mouse_x = event.mouse.x;
				_sdl_mouse_y = event.mouse.y;
#endif
				break;

				case OSystem::EVENT_RBUTTONDOWN:
				_exit_cutscene = true;
				break;
			}
		}

		if (delay == 0)
			break;

		{
			uint this_delay = _fast_mode ? 1 : 20;
			if (this_delay > delay)
				this_delay = delay;
			_system->delay_msecs(this_delay);
		}
		cur = _system->get_msecs();
	} while (cur < start + delay);
}


bool SimonState::save_game(uint slot, const char *caption)
{
	FILE *f;
	uint item_index, num_item, i;
	TimeEvent *te;

	_lock_word |= 0x100;

#ifndef _WIN32_WCE
	errno = 0;
#endif

	f = fopen(gen_savename(slot), "wb");
	if (f == NULL) {
		_lock_word &= ~0x100;
		return false;
	}

	fwrite(caption, 1, 0x12, f);

	fileWriteBE32(f, _itemarray_inited - 1);
	fileWriteBE32(f, 0xFFFFFFFF);
	fileWriteBE32(f, 0);
	fileWriteBE32(f, 0);

	i = 0;
	for (te = _first_time_struct; te; te = te->next)
		i++;

	fileWriteBE32(f, i);
	for (te = _first_time_struct; te; te = te->next) {
		fileWriteBE32(f, te->time + _base_time);
		fileWriteBE16(f, te->subroutine_id);
	}

	item_index = 1;
	for (num_item = _itemarray_inited - 1; num_item; num_item--) {
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
			uint i, j;

			if (child2) {
				fileWriteBE32(f, child2->avail_props);
				i = child2->avail_props & 1;

				for (j = 1; j < 16; j++) {
					if ((1 << j) & child2->avail_props) {
						fileWriteBE16(f, child2->array[i++]);
					}
				}
			}
		}

		{
			Child9 *child9 = (Child9 *) findChildOfType(item, 9);
			if (child9) {
				uint i;
				for (i = 0; i != 4; i++) {
					fileWriteBE16(f, child9->array[i]);
				}
			}
		}
	}

	/* write the 255 variables */
	for (i = 0; i != 255; i++) {
		fileWriteBE16(f, readVariable(i));
	}

	/* write the items in array 6 */
	for (i = 0; i != 10; i++) {
		fileWriteBE16(f, itemPtrToID(_item_array_6[i]));
	}

	/* Write the bits in array 1 & 2 */
	for (i = 0; i != 32; i++)
		fileWriteBE16(f, _bit_array[i]);

	fclose(f);

	_lock_word &= ~0x100;

	return true;
}

char *SimonState::gen_savename(int slot)
{
	static char buf[256];
	const char *dir = getSavePath();

	sprintf(buf, "%sSAVE.%.3d", dir, slot);
	return buf;
}

bool SimonState::load_game(uint slot)
{
	char ident[18];
	FILE *f;
	uint num, item_index, i;

	_lock_word |= 0x100;

#ifndef _WIN32_WCE
	errno = 0;
#endif

	f = fopen(gen_savename(slot), "rb");
	if (f == NULL) {
		_lock_word &= ~0x100;
		return false;
	}

	fread(ident, 1, 18, f);

	num = fileReadBE32(f);

	if (fileReadBE32(f) != 0xFFFFFFFF || num != _itemarray_inited - 1) {
		fclose(f);
		_lock_word &= ~0x100;
		return false;
	}

	fileReadBE32(f);
	fileReadBE32(f);

	_no_parent_notify = true;


	/* add all timers */
	killAllTimers();
	for (num = fileReadBE32(f); num; num--) {
		uint32 timeout = fileReadBE32(f);
		uint16 func_to_call = fileReadBE16(f);
		addTimeEvent(timeout, func_to_call);
	}

	item_index = 1;
	for (num = _itemarray_inited - 1; num; num--) {
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
			uint i, j;
			if (child2 != NULL) {
				child2->avail_props = fileReadBE32(f);
				i = child2->avail_props & 1;

				for (j = 1; j < 16; j++) {
					if ((1 << j) & child2->avail_props) {
						child2->array[i++] = fileReadBE16(f);
					}
				}
			}
		}

		{
			Child9 *child9 = (Child9 *) findChildOfType(item, 9);
			if (child9) {
				uint i;
				for (i = 0; i != 4; i++) {
					child9->array[i] = fileReadBE16(f);
				}
			}
		}
	}


	/* read the 255 variables */
	for (i = 0; i != 255; i++) {
		writeVariable(i, fileReadBE16(f));
	}

	/* write the items in array 6 */
	for (i = 0; i != 10; i++) {
		_item_array_6[i] = derefItem(fileReadBE16(f));
	}

	/* Write the bits in array 1 & 2 */
	for (i = 0; i != 32; i++)
		_bit_array[i] = fileReadBE16(f);

	fclose(f);

	_no_parent_notify = false;

	_lock_word &= ~0x100;

#ifndef _WIN32_WCE
	if (errno != 0)
		error("load failed");
#endif

	return true;
}

void SimonState::initSound()
{
	/* only read voice file in windows game */
	if (_game & GAME_WIN) {
		const char *s = gss->wav_filename;
		const char *s2 = gss->wav_filename2;
		const char *e = gss->effects_filename;

		_voice_offsets = NULL;

		_voice_file = fopen_maybe_lowercase(s);
		if (_voice_file == NULL) {
			warning("Cannot open voice file %s, trying %s", s, s2);
			if (s2) {
				_voice_file = fopen_maybe_lowercase(s2);
				if (_voice_file == NULL) {
					warning("Cannot open voice file %s", s2);
					return;
				}
			} else
				return;
		}

		_voice_offsets = (uint32 *)malloc(gss->NUM_VOICE_RESOURCES * sizeof(uint32));
		if (_voice_offsets == NULL)
			error("Out of memory for voice offsets");

		if (fread(_voice_offsets, gss->NUM_VOICE_RESOURCES * sizeof(uint32), 1, _voice_file) != 1)
			error("Cannot read voice offsets");

		_effects_offsets = NULL;
		_effects_file = fopen_maybe_lowercase(e);
		if (_effects_file != NULL)
		{
			_effects_offsets = (uint32 *)malloc(gss->NUM_EFFECTS_RESOURCES * sizeof(uint32));
			if (_effects_offsets == NULL)
				error("Out of memory for effects offsets");

			if (fread(_effects_offsets, gss->NUM_EFFECTS_RESOURCES * sizeof(uint32), 1, _effects_file) != 1)
				error("Cannot read effects offsets");
		}

#if defined(SCUMM_BIG_ENDIAN)
		for (int r = 0; r < gss->NUM_VOICE_RESOURCES; r++)
			_voice_offsets[r] = READ_LE_UINT32(&_voice_offsets[r]);

		if (_effects_offsets)
			for (int r = 0; r < gss->NUM_EFFECTS_RESOURCES; r++)
				_effects_offsets[r] = READ_LE_UINT32(&_effects_offsets[r]);
#endif
	}
}

#if !defined(__GNUC__)
#pragma START_PACK_STRUCTS
#endif

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
} GCC_PACK;

struct VocHeader {
	uint8 desc[20];
	uint16 datablock_offset;
	uint16 version;
	uint16 id;
	uint8 blocktype;
} GCC_PACK;

struct VocBlockHeader {
	uint8 tc;
	uint8 pack;
} GCC_PACK;


#if !defined(__GNUC__)
#pragma END_PACK_STRUCTS
#endif


void SimonState::playVoice(uint voice)
{
	_mixer->stop(_voice_sound);
	fseek(_voice_file, _voice_offsets[voice], SEEK_SET);

	if (!_effects_offsets) {			/* WAVE audio */
		WaveHeader wave_hdr;
		uint32 data[2];

		if (fread(&wave_hdr, sizeof(wave_hdr), 1, _voice_file) != 1 ||
				wave_hdr.riff != MKID('RIFF') || wave_hdr.wave != MKID('WAVE')
				|| wave_hdr.fmt != MKID('fmt ') || READ_LE_UINT16(&wave_hdr.format_tag) != 1
				|| READ_LE_UINT16(&wave_hdr.channels) != 1
				|| READ_LE_UINT16(&wave_hdr.bits_per_sample) != 8) {
			warning("playVoice(%d): cannot read RIFF header", voice);
			return;
		}

		fseek(_voice_file, READ_LE_UINT32(&wave_hdr.size) - sizeof(wave_hdr) + 20, SEEK_CUR);

		data[0] = fileReadLE32(_voice_file);
		data[1] = fileReadLE32(_voice_file);
		if (												//fread(data, sizeof(data), 1, _voice_file) != 1 ||
				 data[0] != 'atad') {
			warning("playVoice(%d): cannot read data header", voice);
			return;
		}

		byte *buffer = (byte *)malloc(data[1]);
		fread(buffer, data[1], 1, _voice_file);

		_mixer->play_raw(&_voice_sound, buffer, data[1], READ_LE_UINT32(&wave_hdr.samples_per_sec),
										 SoundMixer::FLAG_UNSIGNED);
	} else {											/* VOC audio */
		VocHeader voc_hdr;
		VocBlockHeader voc_block_hdr;
		uint32 size;

		if (fread(&voc_hdr, sizeof(voc_hdr), 1, _voice_file) != 1 ||
				strncmp((char *)voc_hdr.desc, "Creative Voice File\x1A", 10) != 0) {
			warning("playVoice(%d): cannot read voc header", voice);
			return;
		}

		fread(&size, 4, 1, _voice_file);
		size = size & 0xffffff;
		fseek(_voice_file, -1, SEEK_CUR);
		fread(&voc_block_hdr, sizeof(voc_block_hdr), 1, _voice_file);

		uint32 samples_per_sec = 1000000L / (256L - (long)voc_block_hdr.tc);

		byte *buffer = (byte *)malloc(size);
		fread(buffer, size, 1, _voice_file);

		_mixer->play_raw(&_voice_sound, buffer, size, samples_per_sec, SoundMixer::FLAG_UNSIGNED);
	}
}


void SimonState::playSound(uint sound)
{
	if (_game & GAME_WIN) {
		if (_effects_offsets) {			/* VOC sound file */
			VocHeader voc_hdr;
			VocBlockHeader voc_block_hdr;
			uint32 size;

			_mixer->stop(_effects_sound);
			fseek(_effects_file, _effects_offsets[sound], SEEK_SET);


			if (fread(&voc_hdr, sizeof(voc_hdr), 1, _effects_file) != 1 ||
					strncmp((char *)voc_hdr.desc, "Creative Voice File\x1A", 10) != 0) {
				warning("playSound(%d): cannot read voc header", sound);
				return;
			}

			fread(&size, 4, 1, _effects_file);
			size = size & 0xffffff;
			fseek(_effects_file, -1, SEEK_CUR);
			fread(&voc_block_hdr, sizeof(voc_block_hdr), 1, _effects_file);

			uint32 samples_per_sec = 1000000L / (256L - (long)voc_block_hdr.tc);

			byte *buffer = (byte *)malloc(size);
			fread(buffer, size, 1, _effects_file);

			_mixer->play_raw(&_effects_sound, buffer, size, samples_per_sec, SoundMixer::FLAG_UNSIGNED);
		} else {
			byte *p;

			_mixer->stop(_playing_sound);

			/* Check if _sfx_heap is NULL */
			if (_sfx_heap == NULL) {
				warning("playSound(%d) cannot play. No voice file loaded", sound);
				return;
			}

			p = _sfx_heap + READ_LE_UINT32(&((uint32 *)_sfx_heap)[sound]);

			for (;;) {
				p = (byte *)memchr(p, 'd', 1000);
				if (!p) {
					error("playSound(%d): didn't find", sound);
					return;
				}
				if (p[1] == 'a' && p[2] == 't' && p[3] == 'a')
					break;

				p++;
			}

			_mixer->play_raw(&_playing_sound, p + 8, READ_LE_UINT32(p + 4), 22050,
											 SoundMixer::FLAG_UNSIGNED);
		}
	} else {
		warning("playSound(%d)", sound);
	}
}

void SimonState::playMusic(uint music)
{
	FILE *f;

	midi.shutdown();

	/* FIXME: not properly implemented */
	if (_game & GAME_WIN) {
		fseek(_game_file, _game_offsets_ptr[gss->MUSIC_INDEX_BASE + music] - 1, SEEK_SET);
		f = _game_file;

		midi.read_all_songs(f);
	} else {
		char buf[50];
		sprintf(buf, "MOD%d.MUS", music);
		f = fopen_maybe_lowercase(buf);
		if (f == NULL) {
			warning("Cannot load music from '%s'", buf);
			return;
		}
		midi.read_all_songs_old(f);
		fclose(f);
	}

	midi.initialize();
	midi.play();
}

byte *SimonState::dx_lock_2()
{
	_dx_surface_pitch = 320;
	return _sdl_buf;
}

void SimonState::dx_unlock_2()
{
}

byte *SimonState::dx_lock_attached()
{
	_dx_surface_pitch = 320;
	return _dx_use_3_or_4_for_lock ? _sdl_buf_3 : _sdl_buf_attached;
}

void SimonState::dx_unlock_attached()
{
}

void SimonState::set_volume(byte volume)
{
	_mixer->set_volume(volume);
}


byte SimonState::getByte()
{
	return *_code_ptr++;
}
