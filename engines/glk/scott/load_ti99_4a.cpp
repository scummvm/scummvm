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
	return 0;
}

uint16_t fixWord(uint16_t word) {
	return 0;
}

uint16_t getWord(uint8_t *mem) {
	return 0;
}

void getMaxTI99Messages(DataHeader dh) {
	uint8_t *msg;
	uint16_t msg1;

	msg = _G(_entireFile) + fixAddress(fixWord(dh._pMessage));
	msg1 = fixAddress(getWord(msg));
	_G(_maxMessages) = (msg1 - fixAddress(fixWord(dh._pMessage))) / 2;
}

void getMaxTI99Items(DataHeader dh) {

}

int tryLoadingTI994A(DataHeader dh, int loud) {
	return 0;
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
