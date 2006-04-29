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

#include "common/stdafx.h"

#include "simon/simon.h"
#include "simon/intern.h"

using Common::File;

namespace Simon {

Subroutine *SimonEngine::getSubroutineByID(uint subroutine_id) {
	Subroutine *cur;

	_subroutine = subroutine_id;

	for (cur = _subroutineList; cur; cur = cur->next) {
		if (cur->id == subroutine_id)
			return cur;
	}

	loadTablesIntoMem(subroutine_id);

	for (cur = _subroutineList; cur; cur = cur->next) {
		if (cur->id == subroutine_id)
			return cur;
	}

	if (subroutine_id != 160)
		debug(0,"getSubroutineByID: subroutine %d not found", subroutine_id);
	return NULL;
}

void SimonEngine::alignTableMem() {
	if ((unsigned long)_tablesHeapPtr & 3) {
		_tablesHeapPtr += 2;
		_tablesHeapCurPos += 2;
	}
}

byte *SimonEngine::allocateTable(uint size) {
	byte *org = _tablesHeapPtr;

	size = (size + 1) & ~1;

	_tablesHeapPtr += size;
	_tablesHeapCurPos += size;

	if (_tablesHeapCurPos > _tablesHeapSize)
		error("Tablesheap overflow");

	return org;
}

File *SimonEngine::openTablesFile(const char *filename) {
	if (getFeatures() & GF_OLD_BUNDLE)
		return openTablesFile_simon1(filename);
	else
		return openTablesFile_gme(filename);
}

File *SimonEngine::openTablesFile_simon1(const char *filename) {
	File *fo = new File();
	fo->open(filename);
	if (fo->isOpen() == false)
		error("openTablesFile: Can't open '%s'", filename);
	return fo;
}

File *SimonEngine::openTablesFile_gme(const char *filename) {
	uint res;
	uint32 offs;

	res = atoi(filename + 6) + TABLE_INDEX_BASE - 1;
	offs = _gameOffsetsPtr[res];

	_gameFile->seek(offs, SEEK_SET);
	return _gameFile;
}

void SimonEngine::loadTablesIntoMem(uint subr_id) {
	byte *p;
	int i;
	uint min_num, max_num;
	char filename[30];
	File *in;

	p = _tblList;
	if (p == NULL)
		return;

	while (*p) {
		for (i = 0; *p; p++, i++)
			filename[i] = *p;
		filename[i] = 0;
		p++;

		for (;;) {
			min_num = (p[0] * 256) | p[1];
			p += 2;

			if (min_num == 0)
				break;

			max_num = (p[0] * 256) | p[1];
			p += 2;

			if (subr_id >= min_num && subr_id <= max_num) {
				_subroutineList = _subroutineListOrg;
				_tablesHeapPtr = _tablesHeapPtrOrg;
				_tablesHeapCurPos = _tablesHeapCurPosOrg;
				_stringIdLocalMin = 1;
				_stringIdLocalMax = 0;

				in = openTablesFile(filename);
				readSubroutineBlock(in);
				closeTablesFile(in);
				if (getGameType() == GType_FF) {
					// TODO
				} else if (getGameType() == GType_SIMON2) {
					_sound->loadSfxTable(_gameFile, _gameOffsetsPtr[atoi(filename + 6) - 1 + SOUND_INDEX_BASE]);
				} else if (getPlatform() == Common::kPlatformWindows) {
					memcpy(filename, "SFXXXX", 6);
					_sound->readSfxFile(filename);
				}

				alignTableMem();

				_tablesheapPtrNew = _tablesHeapPtr;
				_tablesHeapCurPosNew = _tablesHeapCurPos;

				if (_tablesHeapCurPos > _tablesHeapSize)
					error("loadTablesIntoMem: Out of table memory");
				return;
			}
		}
	}

	debug(1,"loadTablesIntoMem: didn't find %d", subr_id);
}

void SimonEngine::closeTablesFile(File *in) {
	if (getFeatures() & GF_OLD_BUNDLE) {
		in->close();
		delete in;
	}
}

Subroutine *SimonEngine::createSubroutine(uint id) {
	Subroutine *sub;

	alignTableMem();

	sub = (Subroutine *)allocateTable(sizeof(Subroutine));
	sub->id = id;
	sub->first = 0;
	sub->next = _subroutineList;
	_subroutineList = sub;
	return sub;
}

SubroutineLine *SimonEngine::createSubroutineLine(Subroutine *sub, int where) {
	SubroutineLine *sl, *cur_sl = NULL, *last_sl = NULL;

	if (sub->id == 0)
		sl = (SubroutineLine *)allocateTable(SUBROUTINE_LINE_BIG_SIZE);
	else
		sl = (SubroutineLine *)allocateTable(SUBROUTINE_LINE_SMALL_SIZE);

	// where is what offset to insert the line at, locate the proper beginning line
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
		// Insert the subroutine line in the middle of the link
		last_sl->next = (byte *)sl - (byte *)sub;
		sl->next = (byte *)cur_sl - (byte *)sub;
	} else {
		// Insert the subroutine line at the head of the link
		sl->next = sub->first;
		sub->first = (byte *)sl - (byte *)sub;
	}

	return sl;
}

int SimonEngine::startSubroutine(Subroutine *sub) {
	int result = -1;
	SubroutineLine *sl;
	const byte *old_code_ptr;

	if (_startMainScript)
		dumpSubroutine(sub);

	old_code_ptr = _codePtr;

	if (++_recursionDepth > 40)
		error("Recursion error");

	sl = (SubroutineLine *)((byte *)sub + sub->first);

	while ((byte *)sl != (byte *)sub) {
		if (checkIfToRunSubroutineLine(sl, sub)) {
			result = 0;
			_codePtr = (byte *)sl;
			if (sub->id)
				_codePtr += 2;
			else
				_codePtr += 8;

			if (_continousMainScript)
				fprintf(_dumpFile, "; %d\n", sub->id);
			result = runScript();
			if (result != 0) {
				/* result -10 means restart subroutine */
				if (result == -10) {
					delay(0);							/* maybe leave control to the VGA */
					sl = (SubroutineLine *)((byte *)sub + sub->first);
					continue;
				}
				break;
			}
		}
		sl = (SubroutineLine *)((byte *)sub + sl->next);
	}

	_codePtr = old_code_ptr;

	_recursionDepth--;
	return result;
}

int SimonEngine::startSubroutineEx(Subroutine *sub) {
	return startSubroutine(sub);
}

bool SimonEngine::checkIfToRunSubroutineLine(SubroutineLine *sl, Subroutine *sub) {
	if (sub->id)
		return true;

	if (sl->verb != -1 && sl->verb != _scriptVerb &&
			(sl->verb != -2 || _scriptVerb != -1))
		return false;

	if (sl->noun1 != -1 && sl->noun1 != _scriptNoun1 &&
			(sl->noun1 != -2 || _scriptNoun1 != -1))
		return false;

	if (sl->noun2 != -1 && sl->noun2 != _scriptNoun2 &&
			(sl->noun2 != -2 || _scriptNoun2 != -1))
		return false;

	return true;
}

void SimonEngine::readSubroutine(File *in, Subroutine *sub) {
	while (in->readUint16BE() == 0) {
		readSubroutineLine(in, createSubroutineLine(sub, 0xFFFF), sub);
	}
}

void SimonEngine::readSubroutineLine(File *in, SubroutineLine *sl, Subroutine *sub) {
	byte line_buffer[1024], *q = line_buffer;
	int size;

	if (sub->id == 0) {
		sl->verb = in->readUint16BE();
		sl->noun1 = in->readUint16BE();
		sl->noun2 = in->readUint16BE();
	}

	while ((*q = in->readByte()) != 0xFF) {
		if (*q == 87) {
			in->readUint16BE();
		} else {
			q = readSingleOpcode(in, q);
		}
	}

	size = q - line_buffer + 1;

	memcpy(allocateTable(size), line_buffer, size);
}

void SimonEngine::readSubroutineBlock(File *in) {
	while (in->readUint16BE() == 0) {
		readSubroutine(in, createSubroutine(in->readUint16BE()));
	}
}

} // End of namespace Simon
