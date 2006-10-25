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

#include "common/stdafx.h"

#include "agos/agos.h"

namespace AGOS {

void AGOSEngine::setupElvira2Opcodes(OpcodeProc *op) {
	setupCommonOpcodes(op);

	op[8] = &AGOSEngine::oe1_isNotAt;
	op[9] = &AGOSEngine::oe1_sibling;
	op[10] = &AGOSEngine::oe1_notSibling;
	op[21] = &AGOSEngine::oe1_isIn;
	op[22] = &AGOSEngine::oe1_isNotIn;
	op[24] = &AGOSEngine::oe1_isPlayer;
	op[29] = &AGOSEngine::oe1_canPut;
	op[34] = &AGOSEngine::oe1_copyof;
	op[35] = &AGOSEngine::oe1_copyfo;
	op[37] = &AGOSEngine::oe1_whatO;
	op[39] = &AGOSEngine::oe1_weigh;
	op[54] = &AGOSEngine::oe2_moveDirn;
	op[72] = &AGOSEngine::oe2_doClass;
	op[73] = &AGOSEngine::oe2_pObj;
	op[74] = &AGOSEngine::oe1_pName;
	op[75] = &AGOSEngine::oe1_pcName;
	op[83] = &AGOSEngine::o1_rescan;
	op[89] = &AGOSEngine::oe2_loadUserGame;
	op[94] = &AGOSEngine::oe1_findMaster;
	op[95] = &AGOSEngine::oe1_nextMaster;
	op[98] = &AGOSEngine::o1_animate;
	op[99] = &AGOSEngine::o1_stopAnimate;
	op[113] = &AGOSEngine::oe2_drawItem;
	op[123] = &AGOSEngine::oe1_setTime;
	op[124] = &AGOSEngine::oe1_ifTime;
	op[127] = &AGOSEngine::o1_playTune;
	op[144] = &AGOSEngine::oe2_setDoorOpen;
	op[145] = &AGOSEngine::oe2_setDoorClosed;
	op[146] = &AGOSEngine::oe2_setDoorLocked;
	op[147] = &AGOSEngine::oe2_setDoorClosed;
	op[148] = &AGOSEngine::oe2_ifDoorOpen;
	op[149] = &AGOSEngine::oe2_ifDoorClosed;
	op[150] = &AGOSEngine::oe2_ifDoorLocked;
	op[161] = &AGOSEngine::oe2_printStats;
	op[162] = &AGOSEngine::oe2_unk162;
	op[165] = &AGOSEngine::oe2_setSuperRoom;
	op[166] = &AGOSEngine::oe2_getSuperRoom;
	op[167] = &AGOSEngine::oe2_setExitOpen;
	op[168] = &AGOSEngine::oe2_setExitClosed;
	op[169] = &AGOSEngine::oe2_setExitLocked;
	op[170] = &AGOSEngine::oe2_setExitClosed;
	op[171] = &AGOSEngine::oe2_ifExitOpen;
	op[172] = &AGOSEngine::oe2_ifExitClosed;
	op[173] = &AGOSEngine::oe2_ifExitLocked;
	op[175] = &AGOSEngine::o_getDollar2;
	op[177] = &AGOSEngine::oe2_unk177;
	op[178] = &AGOSEngine::oe2_unk178;
	op[179] = &AGOSEngine::o_isAdjNoun;
	op[180] = &AGOSEngine::o_b2Set;
	op[181] = &AGOSEngine::o_b2Clear;
	op[182] = &AGOSEngine::o_b2Zero;
	op[183] = &AGOSEngine::o_b2NotZero;
}

// -----------------------------------------------------------------------
// Elvira 2 Opcodes
// -----------------------------------------------------------------------

void AGOSEngine::oe2_moveDirn() {
	// 54: move direction
	int16 d = getVarOrByte();
	moveDirn_e2(me(), d);
}

void AGOSEngine::oe2_doClass() {
	// 72: do class
	Item *i = getNextItemPtr();
	byte cm = getByte();
	int16 num = getVarOrWord();

	_classMask = (cm != 0xFF) ? 1 << cm : 0;
	_classLine = (SubroutineLine *)((byte *)_currentTable + _currentLine->next);
	if (num == 1) {
		_subjectItem = findInByClass(i, (1 << cm));
		if (_subjectItem)
			_classMode1 = 1;
		else
			_classMode1 = 0;
	} else {
		_objectItem = findInByClass(i, (1 << cm));
		if (_objectItem)
			_classMode2 = 1;
		else
			_classMode2 = 0;
	}
}

void AGOSEngine::oe2_pObj() {
	// 73: print object
	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), 2);

	if (subObject != NULL && subObject->objectFlags & kOFText)
		showMessageFormat((const char *)getStringPtrByID(subObject->objectFlagValue[0]));
}

void AGOSEngine::oe2_loadUserGame() {
	// 89: load user game
	getStringPtrByID(getNextStringID());
}

void AGOSEngine::oe2_drawItem() {
	// 113: draw item
	Item *i = getNextItemPtr();
	int a = getVarOrByte();
	int x = getVarOrWord();
	int y = getVarOrWord();
	mouseOff();
	drawIcon(_windowArray[a % 8], itemGetIconNumber(i), x, y);
	mouseOn();
}

void AGOSEngine::oe2_setDoorOpen() {
	// 144: set door open
	Item *i = getNextItemPtr();
	setDoorState(i, getVarOrByte(), 1);
}

void AGOSEngine::oe2_setDoorClosed() {
	// 145: set door closed
	Item *i = getNextItemPtr();
	setDoorState(i, getVarOrByte(), 2);
}

void AGOSEngine::oe2_setDoorLocked() {
	// 146: set door locked
	Item *i = getNextItemPtr();
	setDoorState(i, getVarOrByte(), 3);
}

void AGOSEngine::oe2_ifDoorOpen() {
	// 148: if door open
	Item *i = getNextItemPtr();
	uint16 d = getVarOrByte();
	setScriptCondition(getDoorState(i, d) == 1);
}

void AGOSEngine::oe2_ifDoorClosed() {
	// 149: if door closed
	Item *i = getNextItemPtr();
	uint16 d = getVarOrByte();
	setScriptCondition(getDoorState(i, d) == 2);
}

void AGOSEngine::oe2_ifDoorLocked() {
	// 150: if door locked
	Item *i=getNextItemPtr();
	uint16 d = getVarOrByte();
	setScriptCondition(getDoorState(i, d) == 3);
}

void AGOSEngine::oe2_printStats() {
	// 161: print stats
	WindowBlock *window = _dummyWindow;
	int val;

	window->flags = 1;

	mouseOff();

	// Level
	val = _variableArray[20];
	if (val < -99)
		val = -99;
	if (val > 99)
		val = 99;	
	writeChar(window, 10, 134, 0, val);

	// HP
	val = _variableArray[22];
	if (val < -99)
		val = -99;
	if (val > 99)
		val = 99;	
	writeChar(window, 16, 134, 6, val);

	// PP
	val = _variableArray[23];
	if (val < -99)
		val = -99;
	if (val > 99)
		val = 99;	
	writeChar(window, 23, 134, 4, val);

	// Experience
	val = _variableArray[21];
	if (val < -99)
		val = -99;
	if (val > 9999)
		val = 9999;	
	writeChar(window, 30, 134, 6, val / 100);
	writeChar(window, 32, 134, 2, val);

	mouseOn();
}

void AGOSEngine::oe2_unk162() {
	// 162: print string?
	showMessageFormat("%s\n", getStringPtrByID(getNextStringID()));
	uint a = getVarOrByte();
	debug(0, "oe2_unk162: stub (%d)", a);
}

void AGOSEngine::oe2_setSuperRoom() {
	// 165: set super room
	_superRoomNumber = getVarOrWord();
}

void AGOSEngine::oe2_getSuperRoom() {
	// 166: get super room
	writeNextVarContents(_superRoomNumber);
}

void AGOSEngine::oe2_setExitOpen() {
	// 167: set exit open
	Item *i = getNextItemPtr();
	uint16 n = getVarOrWord();
	uint16 d = getVarOrByte();
	setExitState(i, n, d, 1);
}

void AGOSEngine::oe2_setExitClosed() {
	// 168: set exit closed
	Item *i = getNextItemPtr();
	uint16 n = getVarOrWord();
	uint16 d = getVarOrByte();
	setExitState(i, n, d, 2);
}

void AGOSEngine::oe2_setExitLocked() {
	// 169: set exit locked
	Item *i = getNextItemPtr();
	uint16 n = getVarOrWord();
	uint16 d = getVarOrByte();
	setExitState(i, n, d, 3);
}

void AGOSEngine::oe2_ifExitOpen() {
	// 171: if exit open
	Item *i = getNextItemPtr();
	uint16 n = getVarOrWord();
	uint16 d = getVarOrByte();
	setScriptCondition(getExitState(i, n, d) == 1);
}

void AGOSEngine::oe2_ifExitClosed() {
	// 172: if exit closed
	Item *i = getNextItemPtr();
	uint16 n = getVarOrWord();
	uint16 d = getVarOrByte();
	setScriptCondition(getExitState(i, n, d) == 2);
}

void AGOSEngine::oe2_ifExitLocked() {
	// 173: if exit locked
	Item *i = getNextItemPtr();
	uint16 n = getVarOrWord();
	uint16 d = getVarOrByte();
	setScriptCondition(getExitState(i, n, d) == 3);
}

void AGOSEngine::oe2_unk177() {
	// 177: set unknown vga event
	uint a = getVarOrByte();
	debug(0, "oe2_unk177: stub (%d)", a);
}

void AGOSEngine::oe2_unk178() {
	// 178: set unknown vga event
	uint a = getVarOrByte();
	debug(0, "oe2_unk178: stub (%d)", a);
}

} // End of namespace AGOS
