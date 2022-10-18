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

/*
 * Based on ScottFree interpreter version 1.14 developed by Swansea
 * University Computer Society without disassembly of any other game
 * drivers, only of game databases as permitted by EEC law (for purposes
 * of compatibility).
 *
 * Licensed under GPLv2
 *
 * https://github.com/angstsmurf/spatterlight/tree/master/terps/scott
 */

#include "glk/scott/scott.h"
#include "glk/scott/globals.h"
#include "glk/scott/resource.h"
#include "glk/scott/game_info.h"
#include "glk/scott/load_ti99_4a.h"

namespace Glk {
namespace Scott {

struct DataHeader {
	uint8_t _numObjects;			/* number of objects */
	uint8_t _numVerbs;				/* number of verbs */
	uint8_t _numNouns;				/* number of nouns */
	uint8_t _redRoom;				/* the red room (dead room) */
	uint8_t _maxItemsCarried;		/* max number of items can be carried */
	uint8_t _beginLocn;				/* room to start in */
	uint8_t _numTreasures;			/* number of treasures */
	uint8_t _cmdLength;				/* number of letters in commands */
	uint16_t _lightTurns;			/* max number of turns light lasts */
	uint8_t _treasureLocn;			/* location of where to store treasures */
	uint8_t _strange;				/* !?! not known. */

	uint16_t _pObjTable;			/* pointer to object table */
	uint16_t _pOrigItems;			/* pointer to original items */
	uint16_t _pObjLink;				/* pointer to link table from noun to object */
	uint16_t _pObjDescr;			/* pointer to object descriptions */
	uint16_t _pMessage;				/* pointer to message pointers */
	uint16_t _pRoomExit;			/* pointer to room exits table */
	uint16_t _pRoomDescr;			/* pointer to room descr table */

	uint16_t _pNounTable;			/* pointer to noun table */
	uint16_t _pVerbTable;			/* pointer to verb table */

	uint16_t _pExplicit;			/* pointer to explicit action table */
	uint16_t _pImplicit;			/* pointer to implicit actions */
};

uint16_t fixAddress(uint16_t ina) {
	return (ina - 0x380 + _G(_fileBaselineOffset));
}

uint16_t fixWord(uint16_t word) {
	return (((word & 0xFF) << 8) | ((word >> 8) & 0xFF));
}

uint16_t getWord(uint8_t *mem) {
	uint16_t x = *(uint16_t *)mem;
	return fixWord(x);
}

void getMaxTI99Messages(DataHeader dh) {
	uint8_t *msg;
	uint16_t msg1;

	msg = _G(_entireFile) + fixAddress(fixWord(dh._pMessage));
	msg1 = fixAddress(getWord(msg));
	_G(_maxMessages) = (msg1 - fixAddress(fixWord(dh._pMessage))) / 2;
}

void getMaxTI99Items(DataHeader dh) {
	uint8_t *msg;
	uint16_t msg1;

	msg = _G(_entireFile) + fixAddress(fixWord(dh._pObjDescr));
	msg1 = fixAddress(getWord(msg));
	_G(_maxItemDescr) = (msg1 - fixAddress(fixWord(dh._pObjDescr))) / 2;
}

uint8_t *getTI994AWord(uint8_t* string, uint8_t** result, size_t* length) {
	uint8_t *msg;

	msg = string;
	*length = msg[0];
	if (*length == 0 || *length > 100) {
		*length = 0;
		*result = nullptr;
		return nullptr;
	}
	msg++;
	*result = new uint8_t[*length];
	memcpy(*result, msg, *length);

	msg += *length;

	return (msg);
}

char *getTI994AString(uint16_t table, int tableOffset) {
	uint8_t *msgx, *msgy, *nextword;
	char *result;
	uint16_t msg1, msg2;
	uint8_t buffer[1024];
	size_t length, totalLength = 0;

	uint8_t *game = _G(_entireFile);

	msgx = game + fixAddress(fixWord(table));

	msgx += tableOffset * 2;
	msg1 = fixAddress(getWord((uint8_t *)msgx));
	msg2 = fixAddress(getWord((uint8_t *)msgx + 2));

	msgy = game + msg2;
	msgx = game + msg1;

	while (msgx < msgy) {
		msgx = getTI994AWord(msgx, &nextword, &length);
		if (length == 0 || nextword == nullptr) {
			return nullptr;
		}
		if (length > 100) {
			delete[] nextword;
			return nullptr;
		}
		memcpy(buffer + totalLength, nextword, length);
		delete[] nextword;
		totalLength += length;
		if (totalLength > 1000)
			break;
		if (msgx < msgy)
			buffer[totalLength++] = ' ';
	}
	if (totalLength == 0)
		return nullptr;
	totalLength++;
	result = new char[totalLength];
	memcpy(result, buffer, totalLength);
	result[totalLength - 1] = '\0';
	return result;
}

void loadTI994ADict(int vorn, uint16_t table, int numWords, Common::StringArray &dict) {
	uint16_t *wtable;
	int i;
	int wordLen;
	char *w1;
	char *w2;

	/* table is either verb or noun table */
	wtable = (uint16_t *)(_G(_entireFile) + fixAddress(fixWord(table)));

	for (i = 0; i <= numWords; i++) {
		do {
			w1 = (char *)_G(_entireFile) + fixAddress(getWord((uint8_t *)wtable + (i * 2)));
			w2 = (char *)_G(_entireFile) + fixAddress(getWord((uint8_t *)wtable + ((1 + i) * 2)));
		} while (w1 == w2);

		wordLen = w2 - w1;

		if (wordLen < 20) {
			char *text = new char[wordLen + 1];
			strncpy(text, w1, wordLen);
			text[wordLen] = 0;
			dict[i] = text;
		}
	}
}

void readTI99ImplicitActions(DataHeader dh) {
	uint8_t *ptr, *implicitStart;
	int loopFlag;

	implicitStart = _G(_entireFile) + fixAddress(fixWord(dh._pImplicit));
	ptr = implicitStart;
	loopFlag = 0;

	/* fall out, if no auto acts in the game. */
	if (*ptr == 0x0)
		loopFlag = 1;

	while (loopFlag == 0) {
		if (ptr[1] == 0)
			loopFlag = 1;

		/* skip code chunk */
		ptr += 1 + ptr[1];
	}

	_G(_ti99ImplicitExtent) = MIN(_G(_fileLength), static_cast<size_t>(ptr - _G(_entireFile)));
	if (_G(_ti99ImplicitExtent)) {
		_G(_ti99ImplicitActions) = new uint8_t[_G(_ti99ImplicitExtent)];
		memcpy(_G(_ti99ImplicitActions), implicitStart, _G(_ti99ImplicitExtent));
	}
}

void readTI99ExplicitActions(DataHeader dh) {
	uint8_t *ptr, *start, *end, *blockstart;
	uint16_t address;
	int loopFlag;
	int i;

	start = _G(_entireFile) + _G(_fileLength);
	end = _G(_entireFile);

	size_t explicitOffset = fixAddress(fixWord(dh._pExplicit));
	blockstart = _G(_entireFile) + explicitOffset;

	_G(_verbActionOffsets) = new uint8_t*[dh._numVerbs + 1];

	for (i = 0; i <= dh._numVerbs; i++) {
		ptr = blockstart;
		address = getWord(ptr + ((i)*2));

		_G(_verbActionOffsets)[i] = nullptr;

		if (address != 0) {
			ptr = _G(_entireFile) + fixAddress(address);
			if (ptr < start)
				start = ptr;
			_G(_verbActionOffsets)[i] = ptr;
			loopFlag = 0;

			while (loopFlag != 1) {
				if (ptr[1] == 0)
					loopFlag = 1;

				/* go to next block. */
				ptr += 1 + ptr[1];
				if (ptr > end)
					end = ptr;
			}
		}
	}

	_G(_ti99ExplicitExtent) = end - start;
	_G(_ti99ExplicitActions) = new uint8_t[_G(_ti99ExplicitExtent)];
	memcpy(_G(_ti99ExplicitActions), start, _G(_ti99ExplicitExtent));
	for (i = 0; i <= dh._numVerbs; i++) {
		if (_G(_verbActionOffsets)[i] != nullptr) {
			_G(_verbActionOffsets)[i] = _G(_ti99ExplicitActions) + (_G(_verbActionOffsets)[i] - start);
		}
	}
}

uint8_t *loadTitleScreen() {
	char buf[3074];
	int offset = 0;
	uint8_t *p;
	int lines;

	/* title screen offset starts at 0x80 */
	p = _G(_entireFile) + 0x80 + _G(_fileBaselineOffset);
	if (static_cast<size_t>(p - _G(_entireFile)) > _G(_fileLength))
		return nullptr;
	int parens = 0;
	for (lines = 0; lines < 24; lines++) {
		for (int i = 0; i < 40; i++) {
			char c = *(p++);
			if (static_cast<size_t>(p - _G(_entireFile)) >= _G(_fileLength))
				return nullptr;
			if (c & 0x80) /* if not 7-bit ascii */
				c = '?';
			switch (c) {
			case '\\':
				c = ' ';
				break;
			case '(':
				parens = 1;
				break;
			case ')':
				if (!parens)
					c = '@';
				parens = 0;
				break;
			case '|':
				if (*p != ' ')
					c = 12;
				break;
			default:
				break;
			}
			buf[offset++] = c;
			if (offset >= 3072)
				return nullptr;
		}
		buf[offset++] = '\n';
	}

	buf[offset] = '\0';
	uint8_t *result = new uint8_t[offset + 1];
	memcpy(result, buf, offset + 1);
	return result;
}

int tryLoadingTI994A(DataHeader dh, int loud) {
	int ni, nw, nr, mc, pr, tr, wl, lt, mn, trm;
	int ct;

	Room *rp;
	Item *ip;
	/* Load the header */

	ni = dh._numObjects;
	nw = MAX(dh._numVerbs, dh._numNouns);
	nr = dh._redRoom;
	mc = dh._maxItemsCarried;
	pr = dh._beginLocn;
	tr = 0;
	trm = dh._treasureLocn;
	wl = dh._cmdLength;
	lt = fixWord(dh._lightTurns);
	mn = _G(_maxMessages);

	uint8_t *ptr = _G(_entireFile);

	_G(_gameHeader)->_numItems = ni;
	_G(_items).resize(ni + 1);
	_G(_gameHeader)->_numActions = 0;
	_G(_gameHeader)->_numWords = nw;
	_G(_gameHeader)->_wordLength = wl;
	_G(_verbs).resize(nw + 2);
	_G(_nouns).resize(nw + 2);
	_G(_gameHeader)->_numRooms = nr;
	_G(_rooms).resize(nr + 1);
	_G(_gameHeader)->_maxCarry = mc;
	_G(_gameHeader)->_playerRoom = pr;
	_G(_gameHeader)->_lightTime = lt;
	_G(_lightRefill) = lt;
	_G(_gameHeader)->_numMessages = mn;
	_G(_messages).resize(mn + 1);
	_G(_gameHeader)->_treasureRoom = trm;

	int offset;

	if (seekIfNeeded(fixAddress(fixWord(dh._pRoomDescr)), &offset, &ptr) == 0)
		return 0;

	ct = 0;
	rp = &_G(_rooms)[0];

	do {
		char *res = getTI994AString(dh._pRoomDescr, ct);
		rp->_text = res ? res : ".\0";
		if (loud)
			debug("Room %d: %s", ct, rp->_text.c_str());
		rp->_image = 255;
		ct++;
		rp++;
	} while (ct < nr + 1);

	ct = 0;
	while (ct < mn + 1) {
		char *res = getTI994AString(dh._pMessage, ct);
		_G(_messages)[ct] = res ? res : ".\0";
		if (loud)
			debug("Message %d: %s", ct, _G(_messages)[ct].c_str());
		ct++;
	}

	ct = 0;
	ip = &_G(_items)[0];
	do {
		char *res = getTI994AString(dh._pObjDescr, ct);
		ip->_text = res ? res : ".\0";
		if (ip->_text.size() && ip->_text[0] == '*')
			tr++;
		if (loud)
			debug("Item %d: %s", ct, ip->_text.c_str());
		ct++;
		ip++;
	} while (ct < ni + 1);

	_G(_gameHeader)->_treasures = tr;
	if (loud)
		debug("Number of treasures %d", _G(_gameHeader)->_treasures);

	if (seekIfNeeded(fixAddress(fixWord(dh._pRoomExit)), &offset, &ptr) == 0)
		return 0;

	ct = 0;
	rp = &_G(_rooms)[0];

	while (ct < nr + 1) {
		for (int j = 0; j < 6; j++) {
			rp->_exits[j] = *(ptr++ - _G(_fileBaselineOffset));
		}
		ct++;
		rp++;
	}

	if (seekIfNeeded(fixAddress(fixWord(dh._pOrigItems)), &offset, &ptr) == 0)
		return 0;

	ct = 0;
	ip = &_G(_items)[0];
	while (ct < ni + 1) {
		ip->_location = *(ptr++ - _G(_fileBaselineOffset));
		ip->_initialLoc = ip->_location;
		ip++;
		ct++;
	}

	loadTI994ADict(0, dh._pVerbTable, dh._numVerbs + 1, _G(_verbs));
	loadTI994ADict(1, dh._pNounTable, dh._numNouns + 1, _G(_nouns));

	for (int i = 1; i <= dh._numNouns - dh._numVerbs; i++)
		_G(_verbs)[dh._numVerbs + i] = ".\0";

	for (int i = 1; i <= dh._numVerbs - dh._numNouns; i++)
		_G(_nouns)[dh._numNouns + i] = ".\0";

	if (loud) {
		for (int i = 0; i <= _G(_gameHeader)->_numWords; i++)
			debug("Verb %d: %s", i, _G(_verbs)[i].c_str());
		for (int i = 0; i <= _G(_gameHeader)->_numWords; i++)
			debug("Noun %d: %s", i, _G(_nouns)[i].c_str());
	}

	ct = 0;
	ip = &_G(_items)[0];

	int *objectlinks = new int[ni + 1];

	if (seekIfNeeded(fixAddress(fixWord(dh._pObjLink)), &offset, &ptr) == 0)
		return 0;

	do {
		objectlinks[ct] = *(ptr++ - _G(_fileBaselineOffset));
		if (objectlinks[ct] && objectlinks[ct] <= nw) {
			ip->_autoGet = _G(_nouns)[objectlinks[ct]];
			if (ct == 3 && scumm_strnicmp("bird", _G(_items)[ct]._text.c_str(), 4) == 0)
				ip->_autoGet = "BIRD";
		} else {
			ip->_autoGet = "";
		}
		ct++;
		ip++;
	} while (ct < ni + 1);

	readTI99ImplicitActions(dh);
	readTI99ExplicitActions(dh);

	_G(_autoInventory) = 1;
	_G(_sys)[INVENTORY] = "I'm carrying: ";

	_G(_titleScreen) = (char *)loadTitleScreen();
	delete[] _G(_entireFile);

	for (int i = 0; i < MAX_SYSMESS && g_sysDictTI994A[i] != nullptr; i++) {
		_G(_sys)[i] = g_sysDictTI994A[i];
	}

	_G(_options) |= TI994A_STYLE;
	return TI994A;
}

void readHeader(Common::SeekableReadStream *f, DataHeader &dh) {
	f->seek(0);
	f->seek(_G(_fileBaselineOffset) + 0x8a0);
	dh._numObjects = f->readByte();
	dh._numVerbs = f->readByte();
	dh._numNouns = f->readByte();
	dh._redRoom = f->readByte();
	dh._maxItemsCarried = f->readByte();
	dh._beginLocn = f->readByte();
	dh._numTreasures = f->readByte();
	dh._cmdLength = f->readByte();
	dh._lightTurns = f->readUint16LE();
	dh._treasureLocn = f->readByte();
	dh._strange = f->readByte();

	dh._pObjTable = f->readUint16LE();
	dh._pOrigItems = f->readUint16LE();
	dh._pObjLink = f->readUint16LE();
	dh._pObjDescr = f->readUint16LE();
	dh._pMessage = f->readUint16LE();
	dh._pRoomExit = f->readUint16LE();
	dh._pRoomDescr = f->readUint16LE();

	dh._pNounTable = f->readUint16LE();
	dh._pVerbTable = f->readUint16LE();

	dh._pExplicit = f->readUint16LE();
	dh._pImplicit = f->readUint16LE();
}

GameIDType detectTI994A(Common::SeekableReadStream *f, uint8_t **sf, size_t *extent) {
	int offset = findCode("\x30\x30\x30\x30\x00\x30\x30\x00\x28\x28", 0);
	if (offset == -1)
		return UNKNOWN_GAME;

	_G(_fileBaselineOffset) = offset - 0x589;

	DataHeader dh;
	readHeader(f, dh);

	getMaxTI99Messages(dh);
	getMaxTI99Items(dh);

	return static_cast<GameIDType>(tryLoadingTI994A(dh, 0));
}

} // End of namespace Scott
} // End of namespace Glk
