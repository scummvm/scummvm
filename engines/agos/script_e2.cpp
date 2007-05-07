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

#define OPCODE(x)	_OPCODE(AGOSEngine_Elvira2, x)

void AGOSEngine_Elvira2::setupOpcodes() {
	static const OpcodeEntryElvira2 opcodes[] = {
		/* 00 */
		OPCODE(o_invalid),
		OPCODE(o_at),
		OPCODE(o_notAt),
		OPCODE(o_invalid),
		/* 04 */
		OPCODE(o_invalid),
		OPCODE(o_carried),
		OPCODE(o_notCarried),
		OPCODE(o_isAt),
		/* 08 */
		OPCODE(oe1_isNotAt),
		OPCODE(oe1_sibling),
		OPCODE(oe1_notSibling),
		OPCODE(o_zero),
		/* 12 */
		OPCODE(o_notZero),
		OPCODE(o_eq),
		OPCODE(o_notEq),
		OPCODE(o_gt),
		/* 16 */
		OPCODE(o_lt),
		OPCODE(o_eqf),
		OPCODE(o_notEqf),
		OPCODE(o_ltf),
		/* 20 */
		OPCODE(o_gtf),
		OPCODE(oe1_isIn),
		OPCODE(oe1_isNotIn),
		OPCODE(o_chance),
		/* 24 */
		OPCODE(oe1_isPlayer),
		OPCODE(o_isRoom),
		OPCODE(o_isObject),
		OPCODE(o_state),
		/* 28 */
		OPCODE(o_oflag),
		OPCODE(oe1_canPut),
		OPCODE(o_invalid),
		OPCODE(o_destroy),
		/* 32 */
		OPCODE(o_invalid),
		OPCODE(o_place),
		OPCODE(oe1_copyof),
		OPCODE(oe1_copyfo),
		/* 36 */
		OPCODE(o_copyff),
		OPCODE(oe1_whatO),
		OPCODE(o_invalid),
		OPCODE(oe1_weigh),
		/* 40 */
		OPCODE(o_invalid),
		OPCODE(o_clear),
		OPCODE(o_let),
		OPCODE(o_add),
		/* 44 */
		OPCODE(o_sub),
		OPCODE(o_addf),
		OPCODE(o_subf),
		OPCODE(o_mul),
		/* 48 */
		OPCODE(o_div),
		OPCODE(o_mulf),
		OPCODE(o_divf),
		OPCODE(o_mod),
		/* 52 */
		OPCODE(o_modf),
		OPCODE(o_random),
		OPCODE(oe2_moveDirn),
		OPCODE(o_goto),
		/* 56 */
		OPCODE(o_oset),
		OPCODE(o_oclear),
		OPCODE(o_putBy),
		OPCODE(o_inc),
		/* 60 */
		OPCODE(o_dec),
		OPCODE(o_setState),
		OPCODE(o_print),
		OPCODE(o_message),
		/* 64 */
		OPCODE(o_msg),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 68 */
		OPCODE(o_end),
		OPCODE(o_done),
		OPCODE(o_invalid),
		OPCODE(o_process),
		/* 72 */
		OPCODE(oe2_doClass),
		OPCODE(oe2_pObj),
		OPCODE(oe1_pName),
		OPCODE(oe1_pcName),
		/* 76 */
		OPCODE(o_when),
		OPCODE(o_if1),
		OPCODE(o_if2),
		OPCODE(oe1_isCalled),
		/* 80 */
		OPCODE(o_is),
		OPCODE(o_invalid),
		OPCODE(o_debug),
		OPCODE(oe1_rescan),
		/* 84 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_comment),
		/* 88 */
		OPCODE(o_invalid),
		OPCODE(oe2_loadGame),
		OPCODE(o_getParent),
		OPCODE(o_getNext),
		/* 92 */
		OPCODE(o_getChildren),
		OPCODE(o_invalid),
		OPCODE(oe1_findMaster),
		OPCODE(oe1_nextMaster),
		/* 96 */
		OPCODE(o_picture),
		OPCODE(o_loadZone),
		OPCODE(oe1_animate),
		OPCODE(oe1_stopAnimate),
		/* 100 */
		OPCODE(o_killAnimate),
		OPCODE(o_defWindow),
		OPCODE(o_window),
		OPCODE(o_cls),
		/* 104 */
		OPCODE(o_closeWindow),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_addBox),
		/* 108 */
		OPCODE(o_delBox),
		OPCODE(o_enableBox),
		OPCODE(o_disableBox),
		OPCODE(o_moveBox),
		/* 112 */
		OPCODE(o_invalid),
		OPCODE(oe2_drawItem),
		OPCODE(o_doIcons),
		OPCODE(o_isClass),
		/* 116 */
		OPCODE(o_setClass),
		OPCODE(o_unsetClass),
		OPCODE(o_invalid),
		OPCODE(o_waitSync),
		/* 120*/
		OPCODE(o_sync),
		OPCODE(o_defObj),
		OPCODE(o_invalid),
		OPCODE(oe1_setTime),
		/* 124 */
		OPCODE(oe1_ifTime),
		OPCODE(o_here),
		OPCODE(o_doClassIcons),
		OPCODE(o_playTune),
		/* 128 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_setAdjNoun),
		OPCODE(o_invalid),
		/* 132 */
		OPCODE(o_saveUserGame),
		OPCODE(o_loadUserGame),
		OPCODE(o_invalid),
		OPCODE(oe2_pauseGame),
		/* 136 */
		OPCODE(o_copysf),
		OPCODE(o_restoreIcons),
		OPCODE(o_freezeZones),
		OPCODE(o_placeNoIcons),
		/* 140 */
		OPCODE(o_clearTimers),
		OPCODE(o_setDollar),
		OPCODE(o_isBox),
		OPCODE(oe2_doTable),
		/* 144 */
		OPCODE(oe2_setDoorOpen),
		OPCODE(oe2_setDoorClosed),
		OPCODE(oe2_setDoorLocked),
		OPCODE(oe2_setDoorClosed),
		/* 148 */
		OPCODE(oe2_ifDoorOpen),
		OPCODE(oe2_ifDoorClosed),
		OPCODE(oe2_ifDoorLocked),
		OPCODE(oe2_storeItem),
		/* 152 */
		OPCODE(oe2_getItem),
		OPCODE(oe2_bSet),
		OPCODE(oe2_bClear),
		OPCODE(oe2_bZero),
		/* 156 */
		OPCODE(oe2_bNotZero),
		OPCODE(oe2_getOValue),
		OPCODE(oe2_setOValue),
		OPCODE(o_invalid),
		/* 160 */
		OPCODE(oe2_ink),
		OPCODE(oe2_printStats),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 164 */
		OPCODE(o_invalid),
		OPCODE(oe2_setSuperRoom),
		OPCODE(oe2_getSuperRoom),
		OPCODE(oe2_setExitOpen),
		/* 168 */
		OPCODE(oe2_setExitClosed),
		OPCODE(oe2_setExitLocked),
		OPCODE(oe2_setExitClosed),
		OPCODE(oe2_ifExitOpen),
		/* 172 */
		OPCODE(oe2_ifExitClosed),
		OPCODE(oe2_ifExitLocked),
		OPCODE(oe2_unk174),
		OPCODE(oe2_getDollar2),
		/* 176 */
		OPCODE(oe2_setSRExit),
		OPCODE(oe2_unk177),
		OPCODE(oe2_unk178),
		OPCODE(oe2_isAdjNoun),
		/* 180 */
		OPCODE(oe2_b2Set),
		OPCODE(oe2_b2Clear),
		OPCODE(oe2_b2Zero),
		OPCODE(oe2_b2NotZero)
	};

	_opcodesElvira2 = opcodes;
	_numOpcodes = 184;
}

void AGOSEngine_Elvira2::executeOpcode(int opcode) {
	OpcodeProcElvira2 op = _opcodesElvira2[opcode].proc;
	(this->*op) ();
}

// -----------------------------------------------------------------------
// Elvira 2 Opcodes
// -----------------------------------------------------------------------

void AGOSEngine_Elvira2::oe2_moveDirn() {
	// 54: move direction
	int16 d = getVarOrByte();
	moveDirn(me(), d);
}

void AGOSEngine_Elvira2::oe2_doClass() {
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

void AGOSEngine_Elvira2::oe2_pObj() {
	// 73: print object
	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), 2);

	if (subObject != NULL && subObject->objectFlags & kOFText)
		showMessageFormat((const char *)getStringPtrByID(subObject->objectFlagValue[0]));
}

void AGOSEngine_Elvira2::oe2_loadGame() {
	// 89: load game
	uint16 stringId = getNextStringID();
	debug(0, "oe1_loadGame: stub (%s)", (const char *)getStringPtrByID(stringId));

	if (!scumm_stricmp(getFileName(GAME_RESTFILE), (const char *)getStringPtrByID(stringId))) {
		loadGame(getFileName(GAME_RESTFILE), true);
	} else {
		loadGame((const char *)getStringPtrByID(stringId));
	}
}

void AGOSEngine_Elvira2::oe2_drawItem() {
	// 113: draw item
	Item *i = getNextItemPtr();
	int a = getVarOrByte();
	int x = getVarOrWord();
	int y = getVarOrWord();
	mouseOff();
	drawIcon(_windowArray[a % 8], itemGetIconNumber(i), x, y);
	mouseOn();
}

void AGOSEngine_Elvira2::oe2_doTable() {
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

void AGOSEngine_Elvira2::oe2_pauseGame() {
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

void AGOSEngine_Elvira2::oe2_setDoorOpen() {
	// 144: set door open
	Item *i = getNextItemPtr();
	setDoorState(i, getVarOrByte(), 1);
}

void AGOSEngine_Elvira2::oe2_setDoorClosed() {
	// 145: set door closed
	Item *i = getNextItemPtr();
	setDoorState(i, getVarOrByte(), 2);
}

void AGOSEngine_Elvira2::oe2_setDoorLocked() {
	// 146: set door locked
	Item *i = getNextItemPtr();
	setDoorState(i, getVarOrByte(), 3);
}

void AGOSEngine_Elvira2::oe2_ifDoorOpen() {
	// 148: if door open
	Item *i = getNextItemPtr();
	uint16 d = getVarOrByte();
	setScriptCondition(getDoorState(i, d) == 1);
}

void AGOSEngine_Elvira2::oe2_ifDoorClosed() {
	// 149: if door closed
	Item *i = getNextItemPtr();
	uint16 d = getVarOrByte();
	setScriptCondition(getDoorState(i, d) == 2);
}

void AGOSEngine_Elvira2::oe2_ifDoorLocked() {
	// 150: if door locked
	Item *i=getNextItemPtr();
	uint16 d = getVarOrByte();
	setScriptCondition(getDoorState(i, d) == 3);
}

void AGOSEngine_Elvira2::oe2_storeItem() {
	// 151: set array6 to item
	uint var = getVarOrByte();
	Item *item = getNextItemPtr();
	_itemStore[var] = item;
}

void AGOSEngine_Elvira2::oe2_getItem() {
	// 152: set m1 to m3 to array 6
	Item *item = _itemStore[getVarOrByte()];
	uint var = getVarOrByte();
	if (var == 1) {
		_subjectItem = item;
	} else {
		_objectItem = item;
	}
}

void AGOSEngine_Elvira2::oe2_bSet() {
	// 153: set bit
	setBitFlag(getVarWrapper(), true);
}

void AGOSEngine_Elvira2::oe2_bClear() {
	// 154: clear bit
	setBitFlag(getVarWrapper(), false);
}

void AGOSEngine_Elvira2::oe2_bZero() {
	// 155: is bit clear
	setScriptCondition(!getBitFlag(getVarWrapper()));
}

void AGOSEngine_Elvira2::oe2_bNotZero() {
	// 156: is bit set
	uint bit = getVarWrapper();

	// WORKAROUND: For a script glitch in some versions
	if (getGameType() == GType_SIMON1 && _subroutine == 2962 && bit == 63) {
		bit = 50;
	}

	setScriptCondition(getBitFlag(bit));
}

void AGOSEngine_Elvira2::oe2_getOValue() {
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

void AGOSEngine_Elvira2::oe2_setOValue() {
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

void AGOSEngine_Elvira2::oe2_ink() {
	// 160
	setTextColor(getVarOrByte());
}

void AGOSEngine_Elvira2::oe2_printStats() {
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

void AGOSEngine_Elvira2::oe2_setSuperRoom() {
	// 165: set super room
	_superRoomNumber = getVarOrWord();
}

void AGOSEngine_Elvira2::oe2_getSuperRoom() {
	// 166: get super room
	writeNextVarContents(_superRoomNumber);
}

void AGOSEngine_Elvira2::oe2_setExitOpen() {
	// 167: set exit open
	Item *i = getNextItemPtr();
	uint16 n = getVarOrWord();
	uint16 d = getVarOrByte();
	setExitState(i, n, d, 1);
}

void AGOSEngine_Elvira2::oe2_setExitClosed() {
	// 168: set exit closed
	Item *i = getNextItemPtr();
	uint16 n = getVarOrWord();
	uint16 d = getVarOrByte();
	setExitState(i, n, d, 2);
}

void AGOSEngine_Elvira2::oe2_setExitLocked() {
	// 169: set exit locked
	Item *i = getNextItemPtr();
	uint16 n = getVarOrWord();
	uint16 d = getVarOrByte();
	setExitState(i, n, d, 3);
}

void AGOSEngine_Elvira2::oe2_ifExitOpen() {
	// 171: if exit open
	Item *i = getNextItemPtr();
	uint16 n = getVarOrWord();
	uint16 d = getVarOrByte();
	setScriptCondition(getExitState(i, n, d) == 1);
}

void AGOSEngine_Elvira2::oe2_ifExitClosed() {
	// 172: if exit closed
	Item *i = getNextItemPtr();
	uint16 n = getVarOrWord();
	uint16 d = getVarOrByte();
	setScriptCondition(getExitState(i, n, d) == 2);
}

void AGOSEngine_Elvira2::oe2_ifExitLocked() {
	// 173: if exit locked
	Item *i = getNextItemPtr();
	uint16 n = getVarOrWord();
	uint16 d = getVarOrByte();
	setScriptCondition(getExitState(i, n, d) == 3);
}

void AGOSEngine_Elvira2::oe2_unk174() {
	// 174:
	uint a = getVarOrWord();
	debug(0, "oe2_unk174: stub (%d)", a);
}

void AGOSEngine_Elvira2::oe2_getDollar2() {
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

void AGOSEngine_Elvira2::oe2_setSRExit() {
	// 176: set super room exit
	Item *i = getNextItemPtr();
	uint n = getVarOrWord();
	uint d = getVarOrByte();
	uint s = getVarOrByte();
	setSRExit(i, n, d, s);
}

void AGOSEngine_Elvira2::oe2_unk177() {
	// 177: set unknown vga event
	uint a = getVarOrByte();
	debug(0, "oe2_unk177: stub (%d)", a);
}

void AGOSEngine_Elvira2::oe2_unk178() {
	// 178: set unknown vga event
	uint a = getVarOrByte();
	debug(0, "oe2_unk178: stub (%d)", a);
}

void AGOSEngine_Elvira2::oe2_isAdjNoun() {
	// 179: item unk1 unk2 is
	Item *item = getNextItemPtr();
	int16 a = getNextWord(), b = getNextWord();
	setScriptCondition(item->adjective == a && item->noun == b);
}

void AGOSEngine_Elvira2::oe2_b2Set() {
	// 180: set bit2
	uint bit = getVarOrByte();
	_bitArrayTwo[bit / 16] |= (1 << (bit & 15));
}

void AGOSEngine_Elvira2::oe2_b2Clear() {
	// 181: clear bit2
	uint bit = getVarOrByte();
	_bitArrayTwo[bit / 16] &= ~(1 << (bit & 15));
}

void AGOSEngine_Elvira2::oe2_b2Zero() {
	// 182: is bit2 clear
	uint bit = getVarOrByte();
	setScriptCondition((_bitArrayTwo[bit / 16] & (1 << (bit & 15))) == 0);
}

void AGOSEngine_Elvira2::oe2_b2NotZero() {
	// 183: is bit2 set
	uint bit = getVarOrByte();
	setScriptCondition((_bitArrayTwo[bit / 16] & (1 << (bit & 15))) != 0);
}

} // End of namespace AGOS
