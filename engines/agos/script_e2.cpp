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
	op[79] = &AGOSEngine::oe1_isCalled;
	op[83] = &AGOSEngine::oe1_rescan;
	op[89] = &AGOSEngine::oe2_loadGame;
	op[94] = &AGOSEngine::oe1_findMaster;
	op[95] = &AGOSEngine::oe1_nextMaster;
	op[98] = &AGOSEngine::oe1_animate;
	op[99] = &AGOSEngine::oe1_stopAnimate;
	op[113] = &AGOSEngine::oe2_drawItem;
	op[123] = &AGOSEngine::oe1_setTime;
	op[124] = &AGOSEngine::oe1_ifTime;
	op[127] = &AGOSEngine::os1_playTune;
	op[135] = &AGOSEngine::oe2_pauseGame;
	op[144] = &AGOSEngine::oe2_setDoorOpen;
	op[145] = &AGOSEngine::oe2_setDoorClosed;
	op[146] = &AGOSEngine::oe2_setDoorLocked;
	op[147] = &AGOSEngine::oe2_setDoorClosed;
	op[148] = &AGOSEngine::oe2_ifDoorOpen;
	op[149] = &AGOSEngine::oe2_ifDoorClosed;
	op[150] = &AGOSEngine::oe2_ifDoorLocked;
	op[161] = &AGOSEngine::oe2_printStats;
	op[165] = &AGOSEngine::oe2_setSuperRoom;
	op[166] = &AGOSEngine::oe2_getSuperRoom;
	op[167] = &AGOSEngine::oe2_setExitOpen;
	op[168] = &AGOSEngine::oe2_setExitClosed;
	op[169] = &AGOSEngine::oe2_setExitLocked;
	op[170] = &AGOSEngine::oe2_setExitClosed;
	op[171] = &AGOSEngine::oe2_ifExitOpen;
	op[172] = &AGOSEngine::oe2_ifExitClosed;
	op[173] = &AGOSEngine::oe2_ifExitLocked;
	op[174] = &AGOSEngine::oe2_unk174;
	op[175] = &AGOSEngine::oe2_getDollar2;
	op[176] = &AGOSEngine::oe2_setSRExit;
	op[177] = &AGOSEngine::oe2_unk177;
	op[178] = &AGOSEngine::oe2_unk178;
	op[179] = &AGOSEngine::oe2_isAdjNoun;
	op[180] = &AGOSEngine::oe2_b2Set;
	op[181] = &AGOSEngine::oe2_b2Clear;
	op[182] = &AGOSEngine::oe2_b2Zero;
	op[183] = &AGOSEngine::oe2_b2NotZero;
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

void AGOSEngine::oe2_loadGame() {
	// 89: load game
	uint16 stringId = getNextStringID();

	debug(0, "oe1_loadGame: stub (%s)", (const char *)getStringPtrByID(stringId));
	loadGame((const char *)getStringPtrByID(stringId));
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

void AGOSEngine::oe2_doTable() {
	// 143: start item sub
	Item *i = getNextItemPtr();

	SubRoom *r = (SubRoom *)findChildOfType(i, 1);
	if (r != NULL) {
		Subroutine *sub = getSubroutineByID(r->subroutine_id);
		if (sub) {
			startSubroutine(sub);
			return;
		}
	}

	if (getGameType() == GType_ELVIRA2) {
		SubSuperRoom *sr = (SubSuperRoom *)findChildOfType(i, 4);
		if (sr != NULL) {
			Subroutine *sub = getSubroutineByID(sr->subroutine_id);
			if (sub) {
				startSubroutine(sub);
				return;
			}
		}
	}
}

void AGOSEngine::oe2_pauseGame() {
	// 135: pause game
	HitArea *ha;

	time_t pauseTime = time(NULL);
	haltAnimation();

	for (;;) {
		_lastHitArea = NULL;
		_lastHitArea3 = NULL;

		for (;;) {
			if (processSpecialKeys() != 0 || _lastHitArea3 != 0)
				break;
			delay(1);
		}

		ha = _lastHitArea;

		if (ha == NULL) {
		} else if (ha->id == 201) {
			break;
		}
	}

	restartAnimation();
	_gameStoppedClock = time(NULL) - pauseTime + _gameStoppedClock;
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

void AGOSEngine::oe2_storeItem() {
	// 151: set array6 to item
	uint var = getVarOrByte();
	Item *item = getNextItemPtr();
	_itemStore[var] = item;
}

void AGOSEngine::oe2_getItem() {
	// 152: set m1 to m3 to array 6
	Item *item = _itemStore[getVarOrByte()];
	uint var = getVarOrByte();
	if (var == 1) {
		_subjectItem = item;
	} else {
		_objectItem = item;
	}
}

void AGOSEngine::oe2_bSet() {
	// 153: set bit
	setBitFlag(getVarWrapper(), true);
}

void AGOSEngine::oe2_bClear() {
	// 154: clear bit
	setBitFlag(getVarWrapper(), false);
}

void AGOSEngine::oe2_bZero() {
	// 155: is bit clear
	setScriptCondition(!getBitFlag(getVarWrapper()));
}

void AGOSEngine::oe2_bNotZero() {
	// 156: is bit set
	uint bit = getVarWrapper();

	// WORKAROUND: Fix for glitch in some versions
	if (getGameType() == GType_SIMON1 && _subroutine == 2962 && bit == 63) {
		bit = 50;
	}

	setScriptCondition(getBitFlag(bit));
}

void AGOSEngine::oe2_getOValue() {
	// 157: get item int prop
	Item *item = getNextItemPtr();
	SubObject *subObject = (SubObject *)findChildOfType(item, 2);
	uint prop = getVarOrByte();

	if (subObject != NULL && subObject->objectFlags & (1 << prop) && prop < 16) {
		uint offs = getOffsetOfChild2Param(subObject, 1 << prop);
		writeNextVarContents(subObject->objectFlagValue[offs]);
	} else {
		writeNextVarContents(0);
	}
}

void AGOSEngine::oe2_setOValue() {
	// 158: set item prop
	Item *item = getNextItemPtr();
	SubObject *subObject = (SubObject *)findChildOfType(item, 2);
	uint prop = getVarOrByte();
	int value = getVarOrWord();

	if (subObject != NULL && subObject->objectFlags & (1 << prop) && prop < 16) {
		uint offs = getOffsetOfChild2Param(subObject, 1 << prop);
		subObject->objectFlagValue[offs] = value;
	}
}

void AGOSEngine::oe2_ink() {
	// 160
	setTextColor(getVarOrByte());
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

	// PP
	val = _variableArray[22];
	if (val < -99)
		val = -99;
	if (val > 99)
		val = 99;	
	writeChar(window, 16, 134, 6, val);

	// HP
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

void AGOSEngine::oe2_unk174() {
	// 174:
	uint a = getVarOrWord();
	debug(0, "oe2_unk174: stub (%d)", a);
}

void AGOSEngine::oe2_getDollar2() {
	// 175
	_showPreposition = true;

	setup_cond_c_helper();

	_objectItem = _hitAreaObjectItem;

	if (_objectItem == _dummyItem2)
		_objectItem = me();

	if (_objectItem == _dummyItem3)
		_objectItem = derefItem(me()->parent);

	if (_objectItem != NULL) {
		_scriptNoun2 = _objectItem->noun;
		_scriptAdj2 = _objectItem->adjective;
	} else {
		_scriptNoun2 = -1;
		_scriptAdj2 = -1;
	}

	_showPreposition = false;
}

void AGOSEngine::oe2_setSRExit() {
	// 176: set super room exit
	Item *i = getNextItemPtr();
	uint n = getVarOrWord();
	uint d = getVarOrByte();
	uint s = getVarOrByte();
	setSRExit(i, n, d, s);
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

void AGOSEngine::oe2_isAdjNoun() {
	// 179: item unk1 unk2 is
	Item *item = getNextItemPtr();
	int16 a = getNextWord(), b = getNextWord();
	setScriptCondition(item->adjective == a && item->noun == b);
}

void AGOSEngine::oe2_b2Set() {
	// 180: set bit2
	uint bit = getVarOrByte();
	_bitArrayTwo[bit / 16] |= (1 << (bit & 15));
}

void AGOSEngine::oe2_b2Clear() {
	// 181: clear bit2
	uint bit = getVarOrByte();
	_bitArrayTwo[bit / 16] &= ~(1 << (bit & 15));
}

void AGOSEngine::oe2_b2Zero() {
	// 182: is bit2 clear
	uint bit = getVarOrByte();
	setScriptCondition((_bitArrayTwo[bit / 16] & (1 << (bit & 15))) == 0);
}

void AGOSEngine::oe2_b2NotZero() {
	// 183: is bit2 set
	uint bit = getVarOrByte();
	setScriptCondition((_bitArrayTwo[bit / 16] & (1 << (bit & 15))) != 0);
}

} // End of namespace AGOS
