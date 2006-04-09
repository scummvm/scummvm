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

// Item script opcodes for Simon1/Simon2

#include "common/stdafx.h"
#include "simon/simon.h"
#include "simon/intern.h"

#include "common/system.h"

#ifdef _WIN32_WCE
extern bool isSmartphone(void);
#endif

namespace Simon {

// Opcode table
void SimonEngine::setupOpcodes() {
	// This opcode table is for Simon 1. Changes for Simon 2 and FF are
	// made below.

	static OpcodeProc opcode_table[200] = {
		// 0 - 4
		NULL,
		&SimonEngine::o1_at,
		&SimonEngine::o1_notAt,
		NULL,
		NULL,
		// 5 - 9
		&SimonEngine::o1_carried,
		&SimonEngine::o1_notCarried,
		&SimonEngine::o1_isAt,
		NULL,
		NULL,
		// 10 - 14
		NULL,
		&SimonEngine::o1_zero,
		&SimonEngine::o1_notZero,
		&SimonEngine::o1_eq,
		&SimonEngine::o1_notEq,
		// 15 - 19
		&SimonEngine::o1_gt,
		&SimonEngine::o1_lt,
		&SimonEngine::o1_eqf,
		&SimonEngine::o1_notEqf,
		&SimonEngine::o1_ltf,
		// 20 - 24
		&SimonEngine::o1_gtf,
		NULL,
		NULL,
		&SimonEngine::o1_chance,
		NULL,
		// 25 - 29
		&SimonEngine::o1_isRoom,
		&SimonEngine::o1_isObject,
		&SimonEngine::o1_state,
		&SimonEngine::o1_oflag,
		NULL,
		// 30 - 34
		NULL,
		&SimonEngine::o1_destroy,
		NULL,
		&SimonEngine::o1_place,
		NULL,
		// 35 - 39
		NULL,
		&SimonEngine::o1_copyff,
		NULL,
		NULL,
		NULL,
		// 40 - 44
		NULL,
		&SimonEngine::o1_clear,
		&SimonEngine::o1_let,
		&SimonEngine::o1_add,
		&SimonEngine::o1_sub,
		// 45 - 49
		&SimonEngine::o1_addf,
		&SimonEngine::o1_subf,
		&SimonEngine::o1_mul,
		&SimonEngine::o1_div,
		&SimonEngine::o1_mulf,
		// 50 - 54
		&SimonEngine::o1_divf,
		&SimonEngine::o1_mod,
		&SimonEngine::o1_modf,
		&SimonEngine::o1_random,
		NULL,
		// 55 - 59
		&SimonEngine::o1_goto,
		&SimonEngine::o1_oset,
		&SimonEngine::o1_oclear,
		&SimonEngine::o1_putBy,
		&SimonEngine::o1_inc,
		// 60 - 64
		&SimonEngine::o1_dec,
		&SimonEngine::o1_setState,
		&SimonEngine::o1_print,
		&SimonEngine::o1_message,
		&SimonEngine::o1_msg,
		// 65 - 69
		&SimonEngine::o1_addTextBox,
		&SimonEngine::o1_setShortText,
		&SimonEngine::o1_setLongText,
		&SimonEngine::o1_end,
		&SimonEngine::o1_done,
		// 70 - 74
		&SimonEngine::o1_printLongText,
		&SimonEngine::o1_process,
		NULL,
		NULL,
		NULL,
		// 75 - 79
		NULL,
		&SimonEngine::o1_when,
		&SimonEngine::o1_if1,
		&SimonEngine::o1_if2,
		&SimonEngine::o1_isCalled,
		// 80 - 84
		&SimonEngine::o1_is,
		NULL,
		&SimonEngine::o1_debug,
		&SimonEngine::o1_rescan,
		NULL,
		// 85 - 89
		NULL,
		NULL,
		&SimonEngine::o1_comment,
		&SimonEngine::o1_haltAnimation,
		&SimonEngine::o1_restartAnimation,
		// 90 - 94
		&SimonEngine::o1_getParent,
		&SimonEngine::o1_getNext,
		&SimonEngine::o1_getChildren,
		NULL,
		NULL,
		// 95 - 99
		NULL,
		&SimonEngine::o1_picture,
		&SimonEngine::o1_loadZone,
		&SimonEngine::o1_animate,
		&SimonEngine::o1_stopAnimate,
		// 100 - 104
		&SimonEngine::o1_killAnimate,
		&SimonEngine::o1_defWindow,
		&SimonEngine::o1_window,
		&SimonEngine::o1_cls,
		&SimonEngine::o1_closeWindow,
		// 105 - 109
		NULL,
		NULL,
		&SimonEngine::o1_addBox,
		&SimonEngine::o1_delBox,
		&SimonEngine::o1_enableBox,
		// 110 - 114
		&SimonEngine::o1_disableBox,
		&SimonEngine::o1_moveBox,
		NULL,
		NULL,
		&SimonEngine::o1_doIcons,
		// 115 - 119
		&SimonEngine::o1_isClass,
		&SimonEngine::o1_setClass,
		&SimonEngine::o1_unsetClass,
		NULL,
		&SimonEngine::o1_waitSync,
		// 120 - 124
		&SimonEngine::o1_sync,
		&SimonEngine::o1_defObj,
		NULL,
		NULL,
		NULL,
		// 125 - 129
		&SimonEngine::o1_here,
		&SimonEngine::o1_doClassIcons,
		&SimonEngine::o1_playTune,
		&SimonEngine::o1_waitEndTune,
		&SimonEngine::o1_ifEndTune,
		// 130 - 134
		&SimonEngine::o1_setAdjNoun,
		NULL,
		&SimonEngine::o1_saveUserGame,
		&SimonEngine::o1_loadUserGame,
		&SimonEngine::o1_stopTune,
		// 135 - 139
		&SimonEngine::o1_pauseGame,
		&SimonEngine::o1_copysf,
		&SimonEngine::o1_restoreIcons,
		&SimonEngine::o1_freezeZones,
		&SimonEngine::o1_placeNoIcons,
		// 140 - 144
		&SimonEngine::o1_clearTimers,
		&SimonEngine::o1_setDollar,
		&SimonEngine::o1_isBox,
		&SimonEngine::o1_doTable,
		NULL,
		// 145 - 149
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		// 150 - 154
		NULL,
		&SimonEngine::o1_storeItem,
		&SimonEngine::o1_getItem,
		&SimonEngine::o1_bSet,
		&SimonEngine::o1_bClear,
		// 155 - 159
		&SimonEngine::o1_bZero,
		&SimonEngine::o1_bNotZero,
		&SimonEngine::o1_getOValue,
		&SimonEngine::o1_setOValue,
		NULL,
		// 160 - 164
		&SimonEngine::o1_ink,
		&SimonEngine::o1_screenTextBox,
		&SimonEngine::o1_screenTextMsg,
		&SimonEngine::o1_playEffect,
		&SimonEngine::o1_getDollar2,
		// 165 - 169
		&SimonEngine::o1_isAdjNoun,
		&SimonEngine::o1_b2Set,
		&SimonEngine::o1_b2Clear,
		&SimonEngine::o1_b2Zero,
		&SimonEngine::o1_b2NotZero,
		// 170 - 174
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		// 175 - 179
		&SimonEngine::o1_lockZones,
		&SimonEngine::o1_unlockZones,
		&SimonEngine::o1_screenTextPObj,
		&SimonEngine::o1_getPathPosn,
		&SimonEngine::o1_scnTxtLongText,
		// 180 - 184
		&SimonEngine::o1_mouseOn,
		&SimonEngine::o1_mouseOff,
		&SimonEngine::o1_loadBeard,
		&SimonEngine::o1_unloadBeard,
		&SimonEngine::o1_unloadZone,
		// 185 - 189
		&SimonEngine::o1_loadStrings,
		&SimonEngine::o1_unfreezeZones,
		&SimonEngine::o1_specialFade,
		NULL,
		NULL,
		// 190 - 194
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		// 195 - 199
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	};

	_opcode_table = opcode_table;
	_numOpcodes = ARRAYSIZE(opcode_table);

	switch (getGameType()) {
	case GType_SIMON1:
		break;
	case GType_SIMON2:
		opcode_table[70] = &SimonEngine::o2_printLongText;
		opcode_table[83] = &SimonEngine::o2_rescan;
		opcode_table[98] = &SimonEngine::o2_animate;
		opcode_table[99] = &SimonEngine::o2_stopAnimate;
		opcode_table[181] = &SimonEngine::o2_mouseOff;
		opcode_table[185] = NULL;
		opcode_table[187] = NULL;
		opcode_table[188] = &SimonEngine::o2_isShortText;
		opcode_table[189] = &SimonEngine::o2_clearMarks;
		opcode_table[190] = &SimonEngine::o2_waitMark;
		break;
	case GType_FF:
		opcode_table[37] = &SimonEngine::o3_jumpOut;
		opcode_table[65] = &SimonEngine::o3_addTextBox;
		opcode_table[70] = &SimonEngine::o3_printLongText;
		opcode_table[83] = &SimonEngine::o2_rescan;
		opcode_table[98] = &SimonEngine::o2_animate;
		opcode_table[99] = &SimonEngine::o2_stopAnimate;
		opcode_table[122] = &SimonEngine::o3_oracleTextDown;
		opcode_table[123] = &SimonEngine::o3_oracleTextUp;
		opcode_table[124] = &SimonEngine::o3_ifTime;
		opcode_table[131] = &SimonEngine::o3_setTime;
		opcode_table[133] = &SimonEngine::o3_loadUserGame;
		opcode_table[134] = &SimonEngine::o3_listSaveGames;
		opcode_table[135] = &SimonEngine::o3_checkCD;
		opcode_table[161] = &SimonEngine::o3_screenTextBox;
		opcode_table[171] = &SimonEngine::o3_hyperLinkOn;
		opcode_table[172] = &SimonEngine::o3_hyperLinkOff;
		opcode_table[173] = &SimonEngine::o3_checkPaths;
		opcode_table[181] = &SimonEngine::o3_mouseOff;
		opcode_table[182] = &SimonEngine::o3_loadSmack;
		opcode_table[183] = &SimonEngine::o3_playSmack;
		opcode_table[185] = NULL;
		opcode_table[187] = &SimonEngine::o3_centreScroll;
		opcode_table[188] = &SimonEngine::o2_isShortText;
		opcode_table[189] = &SimonEngine::o2_clearMarks;
		opcode_table[190] = &SimonEngine::o2_waitMark;
		opcode_table[191] = &SimonEngine::o3_resetPVCount;
		opcode_table[192] = &SimonEngine::o3_setPathValues;
		opcode_table[193] = &SimonEngine::o3_stopClock;
		opcode_table[194] = &SimonEngine::o3_restartClock;
		opcode_table[195] = &SimonEngine::o3_setColour;
		opcode_table[196] = &SimonEngine::o3_b3Set;
		opcode_table[197] = &SimonEngine::o3_b3Clear;
		opcode_table[198] = &SimonEngine::o3_b3Zero;
		opcode_table[199] = &SimonEngine::o3_b3NotZero;
		break;
	default:
		error("setupOpcodes: Unknown game");
	}
}

void SimonEngine::setScriptCondition(bool cond) {
	_runScriptCondition[_recursionDepth] = cond;
}

bool SimonEngine::getScriptCondition() {
	return _runScriptCondition[_recursionDepth];
}

void SimonEngine::setScriptReturn(int ret) {
	_runScriptReturn[_recursionDepth] = ret;
}

int SimonEngine::getScriptReturn() {
	return _runScriptReturn[_recursionDepth];
}

// -----------------------------------------------------------------------
// Simon 1 Opcodes
// -----------------------------------------------------------------------

void SimonEngine::o1_at() {
	// 1: ptrA parent is
	setScriptCondition(getItem1Ptr()->parent == getNextItemID());
}

void SimonEngine::o1_notAt() {
	// 2: ptrA parent is not
	setScriptCondition(getItem1Ptr()->parent != getNextItemID());
}

void SimonEngine::o1_carried() {
	// 5: parent is 1
	setScriptCondition(getNextItemPtr()->parent == getItem1ID());
}

void SimonEngine::o1_notCarried() {
	// 6: parent isnot 1
	setScriptCondition(getNextItemPtr()->parent != getItem1ID());
}

void SimonEngine::o1_isAt() {
	// 7: parent is
	Item *item = getNextItemPtr();
	setScriptCondition(item->parent == getNextItemID());
}

void SimonEngine::o1_zero() {
	// 11: is zero
	setScriptCondition(getNextVarContents() == 0);
}

void SimonEngine::o1_notZero() {
	// 12: isnot zero
	setScriptCondition(getNextVarContents() != 0);
}

void SimonEngine::o1_eq() {
	// 13: equal
	uint tmp = getNextVarContents();
	setScriptCondition(tmp == getVarOrWord());
}

void SimonEngine::o1_notEq() {
	// 14: not equal
	uint tmp = getNextVarContents();
	setScriptCondition(tmp != getVarOrWord());
}

void SimonEngine::o1_gt() {
	// 15: is greater
	uint tmp = getNextVarContents();
	setScriptCondition(tmp > getVarOrWord());
}

void SimonEngine::o1_lt() {
	// 16: is less
	uint tmp = getNextVarContents();
	setScriptCondition(tmp < getVarOrWord());
}

void SimonEngine::o1_eqf() {
	// 17: is eq f
	uint tmp = getNextVarContents();
	setScriptCondition(tmp == getNextVarContents());
}

void SimonEngine::o1_notEqf() {
	// 18: is not equal f
	uint tmp = getNextVarContents();
	setScriptCondition(tmp != getNextVarContents());
}

void SimonEngine::o1_ltf() {
	// 19: is greater f
	uint tmp = getNextVarContents();
	setScriptCondition(tmp < getNextVarContents());
}

void SimonEngine::o1_gtf() {
	// 20: is less f
	uint tmp = getNextVarContents();
	setScriptCondition(tmp > getNextVarContents());
}

void SimonEngine::o1_chance() {
	// 23
	setScriptCondition(o_chance(getVarOrWord()));
}

void SimonEngine::o1_isRoom() {
	// 25: is room
	setScriptCondition(isRoom(getNextItemPtr()));
}

void SimonEngine::o1_isObject() {
	// 26: is object
	setScriptCondition(isObject(getNextItemPtr()));
}

void SimonEngine::o1_state() {
	// 27: item state is
	Item *item = getNextItemPtr();
	setScriptCondition((uint) item->state == getVarOrWord());
}

void SimonEngine::o1_oflag() {
	// 28: item has prop
	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), 2);
	byte num = getVarOrByte();
	setScriptCondition(subObject != NULL && (subObject->objectFlags & (1 << num)) != 0);
}

void SimonEngine::o1_destroy() {
	// 31: set no parent
	setItemParent(getNextItemPtr(), NULL);
}

void SimonEngine::o1_place() {
	// 33: set item parent
	Item *item = getNextItemPtr();
	setItemParent(item, getNextItemPtr());
}

void SimonEngine::o1_copyff() {
	// 36: copy var
	uint value = getNextVarContents();
	writeNextVarContents(value);
}

void SimonEngine::o1_clear() {
	// 41: zero var
	writeNextVarContents(0);
}

void SimonEngine::o1_let() {
	// 42: set var
	uint var = getVarOrByte();
	writeVariable(var, getVarOrWord());
}

void SimonEngine::o1_add() {
	// 43: add
	uint var = getVarOrByte();
	writeVariable(var, readVariable(var) + getVarOrWord());
}

void SimonEngine::o1_sub() {
	// 44: sub
	uint var = getVarOrByte();
	writeVariable(var, readVariable(var) - getVarOrWord());
}

void SimonEngine::o1_addf() {
	// 45: add f
	uint var = getVarOrByte();
	writeVariable(var, readVariable(var) + getNextVarContents());
}

void SimonEngine::o1_subf() {
	// 46: sub f
	uint var = getVarOrByte();
	writeVariable(var, readVariable(var) - getNextVarContents());
}

void SimonEngine::o1_mul() {
	// 47: mul
	uint var = getVarOrByte();
	writeVariable(var, readVariable(var) * getVarOrWord());
}

void SimonEngine::o1_div() {
	// 48: div
	uint var = getVarOrByte();
	int value = getVarOrWord();
	if (value == 0)
		error("o1_div: Division by zero");
	writeVariable(var, readVariable(var) / value);
}

void SimonEngine::o1_mulf() {
	// 49: mul f
	uint var = getVarOrByte();
	writeVariable(var, readVariable(var) * getNextVarContents());
}

void SimonEngine::o1_divf() {
	// 50: div f
	uint var = getVarOrByte();
	int value = getNextVarContents();
	if (value == 0)
		error("o1_divf: Division by zero");
	writeVariable(var, readVariable(var) / value);
}

void SimonEngine::o1_mod() {
	// 51: mod
	uint var = getVarOrByte();
	int value = getVarOrWord();
	if (value == 0)
		error("o1_mod: Division by zero");
	writeVariable(var, readVariable(var) % value);
}

void SimonEngine::o1_modf() {
	// 52: mod f
	uint var = getVarOrByte();
	int value = getNextVarContents();
	if (value == 0)
		error("o1_modf: Division by zero");
	writeVariable(var, readVariable(var) % value);
}

void SimonEngine::o1_random() {
	// 53: random
	uint var = getVarOrByte();
	uint value = (uint16)getVarOrWord();

	// Disable random in simon1amiga for now
	// Since copy protection screen is currently unreadable
	if (getPlatform() == Common::kPlatformAmiga)
		writeVariable(var, 4);
	else
		writeVariable(var, _rnd.getRandomNumber(value - 1));
}

void SimonEngine::o1_goto() {
	// 55: set itemA parent
	setItemParent(getItem1Ptr(), getNextItemPtr());
}

void SimonEngine::o1_oset() {
	// 56: set child2 fr bit
	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), 2);
	int value = getVarOrByte();
	if (subObject != NULL && value >= 0x10)
		subObject->objectFlags |= (1 << value);
}

void SimonEngine::o1_oclear() {
	// 57: clear child2 fr bit
	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), 2);
	int value = getVarOrByte();
	if (subObject != NULL && value >= 0x10)
		subObject->objectFlags &= ~(1 << value);
}

void SimonEngine::o1_putBy() {
	// 58: make siblings
	Item *item = getNextItemPtr();
	setItemParent(item, derefItem(getNextItemPtr()->parent));
}

void SimonEngine::o1_inc() {
	// 59: item inc state
	Item *item = getNextItemPtr();
	if (item->state <= 30000)
		setItemState(item, item->state + 1);
}

void SimonEngine::o1_dec() {
	// 60: item dec state
	Item *item = getNextItemPtr();
	if (item->state >= 0)
		setItemState(item, item->state - 1);
}

void SimonEngine::o1_setState() {
	// 61: item set state
	Item *item = getNextItemPtr();
	int value = getVarOrWord();
	if (value < 0)
		value = 0;
	if (value > 30000)
		value = 30000;
	setItemState(item, value);
}

void SimonEngine::o1_print() {
	// 62: show int
	showMessageFormat("%d", getNextVarContents());
}

void SimonEngine::o1_message() {
	// 63: show string nl
	showMessageFormat("%s\n", getStringPtrByID(getNextStringID()));
}

void SimonEngine::o1_msg() {
	// 64: show string
	showMessageFormat("%s", getStringPtrByID(getNextStringID()));
}

void SimonEngine::o1_addTextBox() {
	// 65: add hit area
	int id = getVarOrWord();
	int x = getVarOrWord();
	int y = getVarOrWord();
	int w = getVarOrWord();
	int h = getVarOrWord();
	int number = getVarOrByte();
	if (number < 20)
		defineBox(id, x, y, w, h, (number << 8) + 129, 208, _dummyItem2);
}

void SimonEngine::o1_setShortText() {
	// 66: set item name
	uint var = getVarOrByte();
	uint stringId = getNextStringID();
	if (var < 20)
		_stringIdArray2[var] = stringId;
}

void SimonEngine::o1_setLongText() {
	// 67: set item description
	uint var = getVarOrByte();
	uint stringId = getNextStringID();
	if (getFeatures() & GF_TALKIE) {
		uint speechId = getNextWord();
		if (var < 20) {
			_stringIdArray3[var] = stringId;
			_speechIdArray4[var] = speechId;
		}
	} else {
		if (var < 20) {
			_stringIdArray3[var] = stringId;
		}
	}
}

void SimonEngine::o1_end() {
	// 68: exit interpreter
	shutdown();
}

void SimonEngine::o1_done() {
	// 69: return 1
	setScriptReturn(1);
}

void SimonEngine::o1_printLongText() {
	// 70: show string from array
	const char *str = (const char *)getStringPtrByID(_stringIdArray3[getVarOrByte()]);
	showMessageFormat("%s\n", str);
}

void SimonEngine::o1_process() {
	// 71: start subroutine
	Subroutine *sub = getSubroutineByID(getVarOrWord());
	if (sub != NULL)
		startSubroutine(sub);
}

void SimonEngine::o1_when() {
	// 76: add timeout
	uint timeout = getVarOrWord();
	addTimeEvent(timeout, getVarOrWord());
}

void SimonEngine::o1_if1() {
	// 77: has item minus 1
	setScriptCondition(_subjectItem != NULL);
}

void SimonEngine::o1_if2() {
	// 78: has item minus 3
	setScriptCondition(_objectItem != NULL);
}

void SimonEngine::o1_isCalled() {
	// 79: childstruct fr2 is
	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), 2);
	uint stringId = getNextStringID();
	setScriptCondition((subObject != NULL) && subObject->objectName == stringId);
}

void SimonEngine::o1_is() {
	// 80: item equal
	setScriptCondition(getNextItemPtr() == getNextItemPtr());
}

void SimonEngine::o1_debug() {
	// 82: debug opcode
	getVarOrByte();
}

void SimonEngine::o1_rescan() {
	// 83: restart subroutine
	setScriptReturn(-10);
}

void SimonEngine::o1_comment() {
	// 87: comment
	getNextStringID();
}

void SimonEngine::o1_haltAnimation() {
	// 88: stop animation
	_lockWord |= 0x10;
}

void SimonEngine::o1_restartAnimation() {
	// 89: restart animation
	_lockWord &= ~0x10;
}

void SimonEngine::o1_getParent() {
	// 90: set minusitem to parent
	Item *item = derefItem(getNextItemPtr()->parent);
	switch (getVarOrByte()) {
	case 0:
		_objectItem = item;
		break;
	case 1:
		_subjectItem = item;
		break;
	default:
		error("o1_getParent: invalid subcode");
	}
}

void SimonEngine::o1_getNext() {
	// 91: set minusitem to sibling
	Item *item = derefItem(getNextItemPtr()->sibling);
	switch (getVarOrByte()) {
	case 0:
		_objectItem = item;
		break;
	case 1:
		_subjectItem = item;
		break;
	default:
		error("o1_getNext: invalid subcode");
	}
}

void SimonEngine::o1_getChildren() {
	// 92: set minusitem to child
	Item *item = derefItem(getNextItemPtr()->child);
	switch (getVarOrByte()) {
	case 0:
		_objectItem = item;
		break;
	case 1:
		_subjectItem = item;
		break;
	default:
		error("o1_getChildren: invalid subcode");
	}
}

void SimonEngine::o1_picture() {
	// 96
	uint val = getVarOrWord();
	o_set_video_mode(getVarOrByte(), val);
}

void SimonEngine::o1_loadZone() {
	// 97: load vga
	o_loadZone(getVarOrWord());
}

void SimonEngine::o1_animate() {
	// 98: start vga
	uint vga_res, vgaSpriteId, windowNum, x, y, palette;
	vgaSpriteId = getVarOrWord();
	vga_res = vgaSpriteId / 100;
	windowNum = getVarOrByte();
	x = getVarOrWord();
	y = getVarOrWord();
	palette = getVarOrWord();
	loadSprite(windowNum, vga_res, vgaSpriteId, x, y, palette);
}

void SimonEngine::o1_stopAnimate() {
	// 99: kill sprite
	o_kill_sprite_simon1(getVarOrWord());
}

void SimonEngine::o1_killAnimate() {
	// 100: vga reset
	o_vga_reset();
}

void SimonEngine::o1_defWindow() {
	// 101
	uint num = getVarOrByte();
	uint x = getVarOrWord();
	uint y = getVarOrWord();
	uint w = getVarOrWord();
	uint h = getVarOrWord();
	uint flags = getVarOrWord();
	uint fill_color = getVarOrWord();
	o_defineWindow(num, x, y, w, h, flags, fill_color, 0);
}

void SimonEngine::o1_window() {
	// 102
	changeWindow(getVarOrByte() & 7);
}

void SimonEngine::o1_cls() {
	// 103
	o_unk_103();
}

void SimonEngine::o1_closeWindow() {
	// 104
	closeWindow(getVarOrByte() & 7);
}

void SimonEngine::o1_addBox() {
	// 107: add item hitarea
	uint flags = 0;
	uint id = getVarOrWord();
	uint params = id / 1000;
	uint x, y, w, h, verb;
	Item *item;

	id = id % 1000;

	if (params & 1)
		flags |= kBFInvertTouch;
	if (params & 2)
		flags |= kBFNoTouchName;
	if (params & 4)
		flags |= kBFBoxItem;
	if (params & 8)
		flags |= kBFTextBox;
	if (params & 16)
		flags |= 0x10;

	x = getVarOrWord();
	y = getVarOrWord();
	w = getVarOrWord();
	h = getVarOrWord();
	item = getNextItemPtrStrange();
	verb = getVarOrWord();
	if (x >= 1000) {
		verb += 0x4000;
		x -= 1000;
	}
	defineBox(id, x, y, w, h, flags, verb, item);
}

void SimonEngine::o1_delBox() {
	// 108: delete hitarea
	delete_hitarea(getVarOrWord());
}

void SimonEngine::o1_enableBox() {
	// 109: clear hitarea bit 0x40
	clear_hitarea_bit_0x40(getVarOrWord());
}

void SimonEngine::o1_disableBox() {
	// 110: set hitarea bit 0x40
	set_hitarea_bit_0x40(getVarOrWord());
}

void SimonEngine::o1_moveBox() {
	// 111: set hitarea xy
	uint hitarea_id = getVarOrWord();
	uint x = getVarOrWord();
	uint y = getVarOrWord();
	moveBox(hitarea_id, x, y);
}

void SimonEngine::o1_doIcons() {
	// 114
	Item *item = getNextItemPtr();
	uint num = getVarOrByte();
	mouseOff();
	drawIconArray(num, item, 0, 0);
	mouseOn();
}

void SimonEngine::o1_isClass() {
	// 115: item has flag
	Item *item = getNextItemPtr();
	setScriptCondition((item->classFlags & (1 << getVarOrByte())) != 0);
}

void SimonEngine::o1_setClass() {
	// 116: item set flag
	Item *item = getNextItemPtr();
	item->classFlags |= (1 << getVarOrByte());
}

void SimonEngine::o1_unsetClass() {
	// 117: item clear flag
	Item *item = getNextItemPtr();
	item->classFlags &= ~(1 << getVarOrByte());
}

void SimonEngine::o1_waitSync() {
	// 119: wait vga
	uint var = getVarOrWord();
	_scriptVar2 = (var == 200);

	if (var != 200 || !_skipVgaWait)
		o_waitForSync(var);
	_skipVgaWait = false;
}

void SimonEngine::o1_sync() {
	// 120: sync
	o_sync(getVarOrWord());
}

void SimonEngine::o1_defObj() {
	// 121: set vga item
	uint slot = getVarOrByte();
	_vcItemArray[slot] = getNextItemPtr();
}

void SimonEngine::o1_here() {
	// 125: item is sibling with item 1
	Item *item = getNextItemPtr();
	setScriptCondition(getItem1Ptr()->parent == item->parent);
}

void SimonEngine::o1_doClassIcons() {
	// 126
	Item *item = getNextItemPtr();
	uint num = getVarOrByte();
	uint a = 1 << getVarOrByte();
	mouseOff();
	drawIconArray(num, item, 1, a);
	mouseOn();
}

void SimonEngine::o1_playTune() {
	// 127: deals with music
	o_playMusic();
}

void SimonEngine::o1_waitEndTune() {
	// 128: dummy instruction
	getVarOrWord();
}

void SimonEngine::o1_ifEndTune() {
	// 129: dummy instruction
	getVarOrWord();
	setScriptCondition(true);
}

void SimonEngine::o1_setAdjNoun() {
	// 130: set adj noun
	uint var = getVarOrByte();
	if (var == 1) {
		_scriptAdj1 = getNextWord();
		_scriptNoun1 = getNextWord();
	} else {
		_scriptAdj2 = getNextWord();
		_scriptNoun2 = getNextWord();
	}
}

void SimonEngine::o1_saveUserGame() {
	// 132: save game
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
	o_saveGame();
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
}

void SimonEngine::o1_loadUserGame() {
	// 133: load game
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
	o_loadGame();
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
}

void SimonEngine::o1_stopTune() {
	// 134: dummy opcode?
	midi.stop();
	_lastMusicPlayed = -1;
}

void SimonEngine::o1_pauseGame() {
	// 135: quit if user presses y
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
	o_confirmQuit();
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
}

void SimonEngine::o1_copysf() {
	// 136: set var to item unk3
	Item *item = getNextItemPtr();
	writeNextVarContents(item->state);
}

void SimonEngine::o1_restoreIcons() {
	// 137
	o_restoreIconArray(getVarOrByte());
}

void SimonEngine::o1_freezeZones() {
	// 138: vga pointer op 4
	o_freezeBottom();
}

void SimonEngine::o1_placeNoIcons() {
	// 139: set parent special
	Item *item = getNextItemPtr();
	_noParentNotify = true;
	setItemParent(item, getNextItemPtr());
	_noParentNotify = false;
}

void SimonEngine::o1_clearTimers() {
	// 140: del te and add one
	killAllTimers();
	addTimeEvent(3, 0xA0);
}

void SimonEngine::o1_setDollar() {
	// 141: set m1 to m3
	uint which = getVarOrByte();
	Item *item = getNextItemPtr();
	if (which == 1) {
		_subjectItem = item;
	} else {
		_objectItem = item;
	}
}

void SimonEngine::o1_isBox() {
	// 142: is hitarea 0x40 clear
	setScriptCondition(is_hitarea_0x40_clear(getVarOrWord()));
}

void SimonEngine::o1_doTable() {
	// 143: start item sub
	SubRoom *subRoom = (SubRoom *)findChildOfType(getNextItemPtr(), 1);
	if (subRoom != NULL) {
		Subroutine *sub = getSubroutineByID(subRoom->subroutine_id);
		if (sub)
			startSubroutine(sub);
	}
}

void SimonEngine::o1_storeItem() {
	// 151: set array6 to item
	uint var = getVarOrByte();
	Item *item = getNextItemPtr();
	_itemArray6[var] = item;
}

void SimonEngine::o1_getItem() {
	// 152: set m1 to m3 to array 6
	Item *item = _itemArray6[getVarOrByte()];
	uint var = getVarOrByte();
	if (var == 1) {
		_subjectItem = item;
	} else {
		_objectItem = item;
	}
}

void SimonEngine::o1_bSet() {
	// 153: set bit
	setBitFlag(getVarOrByte(), true);
}

void SimonEngine::o1_bClear() {
	// 154: clear bit
	setBitFlag(getVarOrByte(), false);
}

void SimonEngine::o1_bZero() {
	// 155: is bit clear
	setScriptCondition(!getBitFlag(getVarOrByte()));
}

void SimonEngine::o1_bNotZero() {
	// 156: is bit set
	uint bit = getVarOrByte();

	// WORKAROUND: Fix for glitch in some versions
	if (getGameType() == GType_SIMON1 && _subroutine == 2962 && bit == 63) {
		bit = 50;
	}

	setScriptCondition(getBitFlag(bit));
}

void SimonEngine::o1_getOValue() {
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

void SimonEngine::o1_setOValue() {
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

void SimonEngine::o1_ink() {
	// 160
	o_setTextColor(getVarOrByte());
}

void SimonEngine::o1_screenTextBox() {
	// 161: setup text
	TextLocation *tl = getTextLocation(getVarOrByte());

	tl->x = getVarOrWord();
	tl->y = getVarOrByte();
	tl->width = getVarOrWord();
}
	
void SimonEngine::o1_screenTextMsg() {
	// 162: print string
	o_printStr();
}

void SimonEngine::o1_playEffect() {
	// 163: play sound
	o_playSFX(getVarOrWord());
}

void SimonEngine::o1_getDollar2() {
	// 164
	_showPreposition = true;
	o_setup_cond_c();
	_showPreposition = false;
}

void SimonEngine::o1_isAdjNoun() {
	// 165: item unk1 unk2 is
	Item *item = getNextItemPtr();
	int16 a = getNextWord(), b = getNextWord();
	setScriptCondition(item->adjective == a && item->noun == b);
}

void SimonEngine::o1_b2Set() {
	// 166: set bit2
	setBitFlag(256 + getVarOrByte(), true);
}

void SimonEngine::o1_b2Clear() {
	// 167: clear bit2
	setBitFlag(256 + getVarOrByte(), false);
}

void SimonEngine::o1_b2Zero() {
	// 168: is bit2 clear
	setScriptCondition(!getBitFlag(256 + getVarOrByte()));
}

void SimonEngine::o1_b2NotZero() {
	// 169: is bit2 set
	setScriptCondition(getBitFlag(256 + getVarOrByte()));
}

void SimonEngine::o1_lockZones() {
	// 175: vga pointer op 1
	o_lockZone();
}

void SimonEngine::o1_unlockZones() {
	// 176: vga pointer op 2
	o_unlockZone();
}

void SimonEngine::o1_screenTextPObj() {
	// 177: inventory descriptions
	o_inventory_descriptions();
}

void SimonEngine::o1_getPathPosn() {
	// 178: path find
	uint a = getVarOrWord();
	uint b = getVarOrWord();
	uint c = getVarOrByte();
	uint d = getVarOrByte();
	o_pathfind(a, b, c, d);
}

void SimonEngine::o1_scnTxtLongText() {
	// 179: conversation responses and room descriptions
	uint vgaSpriteId = getVarOrByte();
	uint color = getVarOrByte();
	uint stringId = getVarOrByte();
	uint speechId = 0;
	TextLocation *tl;

	const char *string_ptr = (const char *)getStringPtrByID(_stringIdArray3[stringId]);
	if (getFeatures() & GF_TALKIE)
		speechId = _speechIdArray4[stringId];

	if (getGameType() == GType_FF)
		vgaSpriteId = 1;
	tl = getTextLocation(vgaSpriteId);

	if (_speech && speechId != 0)
		playSpeech(speechId, vgaSpriteId);
	if (string_ptr != NULL && _subtitles)
		printScreenText(vgaSpriteId, color, string_ptr, tl->x, tl->y, tl->width);
}

void SimonEngine::o1_mouseOn() {
	// 180: force mouseOn
	o_mouseOn();
}

void SimonEngine::o1_mouseOff() {
	// 181: force mouseOff
	o_mouseOff();
}

void SimonEngine::o1_loadBeard() {
	// 182: load beard
	o_loadBeard();
}

void SimonEngine::o1_unloadBeard() {
	// 183: unload beard
	o_unloadBeard();
}

void SimonEngine::o1_unloadZone() {
	// 184: clear vgapointer entry
	o_unloadZone(getVarOrWord());
}

void SimonEngine::o1_loadStrings() {
	// 185: load sound files
	_soundFileId = getVarOrWord();
	if (getPlatform() == Common::kPlatformAmiga && getFeatures() & GF_TALKIE) {
		char buf[10];
		sprintf(buf, "%d%s", _soundFileId, "Effects");
		_sound->readSfxFile(buf);
		sprintf(buf, "%d%s", _soundFileId, "simon");
		_sound->readVoiceFile(buf);
	}
}

void SimonEngine::o1_unfreezeZones() {
	// 186: vga pointer op 3
	o_unfreezeBottom();
}

void SimonEngine::o1_specialFade() {
	// 187: fade to black
	o_fadeToBlack();
}

// -----------------------------------------------------------------------
// Simon 2 Opcodes
// -----------------------------------------------------------------------

void SimonEngine::o2_printLongText() {
	// 70: show string from array
	const char *str = (const char *)getStringPtrByID(_stringIdArray3[getVarOrByte()]);
	writeVariable(51, strlen(str) / 53 * 8 + 8);
	showMessageFormat("%s\n", str);
}

void SimonEngine::o2_rescan() {
	// 83: restart subroutine
	if (_exitCutscene) {
		if (getBitFlag(9)) {
			endCutscene();
		}
	} else {
		processSpecialKeys();
	}

	setScriptReturn(-10);
}

void SimonEngine::o2_animate() {
	// 98: start vga
	uint vga_res, vgaSpriteId, windowNum, x, y, palette;
	vga_res = getVarOrWord();
	vgaSpriteId = getVarOrWord();
	windowNum = getVarOrByte();
	x = getVarOrWord();
	y = getVarOrWord();
	palette = getVarOrWord();
	loadSprite(windowNum, vga_res, vgaSpriteId, x, y, palette);
}

void SimonEngine::o2_stopAnimate() {
	// 99: kill sprite
	uint a = getVarOrWord();
	uint b = getVarOrWord();
	o_kill_sprite_simon2(a, b);
}

void SimonEngine::o2_mouseOff() {
	// 181: force mouseOff
	o_mouseOff();
	changeWindow(1);
	showMessageFormat("\xC");
}

void SimonEngine::o2_isShortText() {
	// 188: string2 is
	uint i = getVarOrByte();
	uint str = getNextStringID();
	setScriptCondition(str < 20 && _stringIdArray2[i] == str);
}

void SimonEngine::o2_clearMarks() {
	// 189: clear_op189_flag
	_marks = 0;
}

void SimonEngine::o2_waitMark() {
	// 190
	uint i = getVarOrByte();
	if (!(_marks & (1 << i)))
		o_waitForMark(i);
}

// -----------------------------------------------------------------------
// Feeble Files Opcodes
// -----------------------------------------------------------------------

void SimonEngine::o3_jumpOut() {
	// 37
	getVarOrByte();
	setScriptReturn(1);
}

void SimonEngine::o3_addTextBox() {
	// 65: add hit area
	uint flags = kBFTextBox | kBFBoxItem;
	uint id = getVarOrWord();
	uint params = id / 1000;
	uint x, y, w, h, num;

	id %= 1000;

	if (params & 1)
		flags |= kBFInvertTouch;

	x = getVarOrWord();
	y = getVarOrWord();
	w = getVarOrWord();
	h = getVarOrWord();
	num = getVarOrByte();
	if (num < 20)
		defineBox(id, x, y, w, h, flags + (num << 8), 208, _dummyItem1);
}

void SimonEngine::o3_printLongText() {
	// 70: show string from array
	int num = getVarOrByte();
	const char *str = (const char *)getStringPtrByID(_stringIdArray3[num]);
	printInteractText(num, str);
}

void SimonEngine::o3_oracleTextDown() {
	// 122: oracle text down
	oracleTextDown();
}

void SimonEngine::o3_oracleTextUp() {
	// 123: oracle text up
	oracleTextUp();
}

void SimonEngine::o3_ifTime() {
	// 124: if time
	time_t t;

	uint a = getVarOrWord();
	time(&t);
	t -= _gameStoppedClock;
	t -= a;
	if (t >= _timeStore)
		setScriptCondition(true);
	else
		setScriptCondition(false);
}

void SimonEngine::o3_setTime() {
	// 131
	time(&_timeStore);
	_timeStore -= _gameStoppedClock;
}

void SimonEngine::o3_loadUserGame() {
	// 133: load game
	loadGame(readVariable(55));
}

void SimonEngine::o3_listSaveGames() {
	// 134: dummy opcode?
	listSaveGames(1);
}

void SimonEngine::o3_checkCD() {
	// 135: switch CD
	debug(1, "Switch to CD number %d", readVariable(97));
}

void SimonEngine::o3_screenTextBox() {
	// 161: setup text
	TextLocation *tl = getTextLocation(getVarOrByte());

	tl->x = getVarOrWord();
	tl->y = getVarOrWord();
	tl->width = getVarOrWord();
}

void SimonEngine::o3_hyperLinkOn() {
	// 171: oracle hyperlink on
	hyperLinkOn(getVarOrWord());
}

void SimonEngine::o3_hyperLinkOff() {
	// 172: oracle hyperlink off
	hyperLinkOff();
}

void SimonEngine::o3_checkPaths() {
	// 173
	warning("STUB: script opcode 173");
}

void SimonEngine::o3_mouseOff() {
	// 181: force mouseOff
	o_mouseOff();
	clearName();
}

void SimonEngine::o3_loadSmack() {
	// 182: load video file
	debug(1,"Load video file: %s", getStringPtrByID(getNextStringID()));
}

void SimonEngine::o3_playSmack() {
	// 183: play video
	debug(1, "Play video");
}

void SimonEngine::o3_centreScroll() {
	// 187
	centreScroll();
}

void SimonEngine::o3_resetPVCount() {
	// 191
	if (getBitFlag(83)) {
		_PVCount1 = 0;
		_GPVCount1 = 0;
	} else {
		_PVCount = 0;
		_GPVCount = 0;
	}
}

void SimonEngine::o3_setPathValues() {
	// 192
	uint8 a = getVarOrByte();
	uint8 b = getVarOrByte();
	uint8 c = getVarOrByte();
	uint8 d = getVarOrByte();
	if (getBitFlag(83)) {
		_pathValues1[_PVCount1++] = a;
		_pathValues1[_PVCount1++] = b;
		_pathValues1[_PVCount1++] = c;
		_pathValues1[_PVCount1++] = d;
	} else {
		_pathValues[_PVCount++] = a;
		_pathValues[_PVCount++] = b;
		_pathValues[_PVCount++] = c;
		_pathValues[_PVCount++] = d;
	}
}

void SimonEngine::o3_stopClock() {
	// 193: pause clock
	_clockStopped = time(NULL);
}

void SimonEngine::o3_restartClock() {
	// 194: resume clock
	if (_clockStopped != 0)
		_gameStoppedClock += time(NULL) - _clockStopped;
	_clockStopped = 0;
}

void SimonEngine::o3_setColour() {
	// 195: set palette colour?
	uint blue = getVarOrByte();
	uint green = getVarOrByte();
	uint red = getVarOrByte();
	uint color = getVarOrByte();
	warning("STUB: script opcode 195 (%d, %d, %d, %d)", blue, green, red, color);
}

void SimonEngine::o3_b3Set() {
	// 196: set bit3
	setBitFlag(512 + getVarOrByte(), true);
}

void SimonEngine::o3_b3Clear() {
	// 197: clear bit3
	setBitFlag(512 + getVarOrByte(), false);
}

void SimonEngine::o3_b3Zero() {
	// 198: is bit3 clear
	setScriptCondition(!getBitFlag(512 + getVarOrByte()));
}

void SimonEngine::o3_b3NotZero() {
	// 199: is bit3 set
	setScriptCondition(getBitFlag(512 + getVarOrByte()));
}

// -----------------------------------------------------------------------

int SimonEngine::runScript() {
	byte opcode;
	bool flag;

	do {
		if (_continousMainScript)
			dumpOpcode(_codePtr);

		opcode = getByte();
		if (opcode == 0xFF)
			return 0;

		if (_runScriptReturn1)
			return 1;

		/* Invert condition? */
		flag = false;
		if (opcode == 0) {
			flag = true;
			opcode = getByte();
			if (opcode == 0xFF)
				return 0;
		}

		setScriptCondition(true);
		setScriptReturn(0);

		if (opcode > _numOpcodes || !_opcode_table[opcode])
			error("Invalid opcode '%d' encountered", opcode);

		(this->*_opcode_table[opcode]) ();
	} while (getScriptCondition() != flag && !getScriptReturn());

	return getScriptReturn();
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

void SimonEngine::o_waitForMark(uint i) {
	_exitCutscene = false;
	while (!(_marks & (1 << i))) {
		if (_exitCutscene) {
			if (getBitFlag(9)) {
				endCutscene();
				break;
			}
		} else {
			processSpecialKeys();
		}

		delay(10);
	}
}


bool SimonEngine::o_chance(uint a) {
	if (a == 0)
		return 0;

	if (a == 100)
		return 1;

	a += _scriptUnk1;
	if (a <= 0) {
		_scriptUnk1 = 0;
		return 0;
	}

	if ((uint)_rnd.getRandomNumber(99) < a) {
		if (_scriptUnk1 <= 0)
			_scriptUnk1 -= 5;
		else
			_scriptUnk1 = 0;
		return 1;
	}

	if (_scriptUnk1 >= 0)
		_scriptUnk1 += 5;
	else
		_scriptUnk1 = 0;

	return 0;
}

void SimonEngine::o_inventory_descriptions() {
	uint vgaSpriteId = getVarOrByte();
	uint color = getVarOrByte();
	const char *string_ptr = NULL;
	TextLocation *tl = NULL;
	char buf[256];

	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), 2);
	if (subObject != NULL && subObject->objectFlags & kOFText) {
		string_ptr = (const char *)getStringPtrByID(subObject->objectFlagValue[0]);
		tl = getTextLocation(vgaSpriteId);
	}

	if ((getGameType() == GType_SIMON2) && (getFeatures() & GF_TALKIE)) {
		if (subObject != NULL && subObject->objectFlags & kOFVoice) {
			uint speechId = subObject->objectFlagValue[getOffsetOfChild2Param(subObject, kOFVoice)];

			if (subObject->objectFlags & kOFNumber) {
				uint speechIdOffs = subObject->objectFlagValue[getOffsetOfChild2Param(subObject, kOFNumber)];

				if (speechId == 116)
					speechId = speechIdOffs + 115;
				if (speechId == 92)
					speechId = speechIdOffs + 98;
				if (speechId == 99)
					speechId = 9;
				if (speechId == 97) {
					switch (speechIdOffs) {
					case 12:
						speechId = 109;
						break;
					case 14:
						speechId = 108;
						break;
					case 18:
						speechId = 107;
						break;
					case 20:
						speechId = 106;
						break;
					case 22:
						speechId = 105;
						break;
					case 28:
						speechId = 104;
						break;
					case 90:
						speechId = 103;
						break;
					case 92:
						speechId = 102;
						break;
					case 100:
						speechId = 51;
						break;
					default:
						error("o_177: invalid case %d", speechIdOffs);
					}
				}
			}

			if (_speech)
				playSpeech(speechId, vgaSpriteId);
		}

	} else if (getFeatures() & GF_TALKIE) {
		if (subObject != NULL && subObject->objectFlags & kOFVoice) {
			uint offs = getOffsetOfChild2Param(subObject, kOFVoice);
			playSpeech(subObject->objectFlagValue[offs], vgaSpriteId);
		} else if (subObject != NULL && subObject->objectFlags & kOFNumber) {
			uint offs = getOffsetOfChild2Param(subObject, kOFNumber);
			playSpeech(subObject->objectFlagValue[offs] + 3550, vgaSpriteId);
		}
	}

	if (subObject != NULL && (subObject->objectFlags & kOFText) && _subtitles) {
		if (subObject->objectFlags & kOFNumber) {
			sprintf(buf, "%d%s", subObject->objectFlagValue[getOffsetOfChild2Param(subObject, kOFNumber)], string_ptr);
			string_ptr = buf;
		}
		if (string_ptr != NULL)
			printScreenText(vgaSpriteId, color, string_ptr, tl->x, tl->y, tl->width);
	}
}

void SimonEngine::o_confirmQuit() {
	// If all else fails, use English as fallback.
	byte keyYes = 'y';
	byte keyNo = 'n';

	switch (_language) {
	case Common::RU_RUS:
		break;
	case Common::PL_POL:
		keyYes = 't';
		break;
	case Common::HB_ISR:
		keyYes = 'f';
		break;
	case Common::ES_ESP:
		keyYes = 's';
		break;
	case Common::IT_ITA:
		keyYes = 's';
		break;
	case Common::FR_FRA:
		keyYes = 'o';
		break;
	case Common::DE_DEU:
		keyYes = 'j';
		break;
	default:
		break;
	}

	for (;;) {
		delay(1);
#ifdef _WIN32_WCE
		if (isSmartphone()) {
			if (_keyPressed) {
				if (_keyPressed == 13)
					shutdown();
				else
					break;
			}
		}
#endif
		if (_keyPressed == keyYes)
			shutdown();
		else if (_keyPressed == keyNo)
			break;
	}
}

void SimonEngine::o_restoreIconArray(uint num) {
	WindowBlock *window;

	window = _windowArray[num & 7];
	if (window->iconPtr == NULL)
		return;
	drawIconArray(num, window->iconPtr->itemRef, window->iconPtr->line, window->iconPtr->classMask);
}

void SimonEngine::o_freezeBottom() {
	_vgaBufStart = _vgaBufFreeStart;
	_vgaFileBufOrg = _vgaBufFreeStart;
}

void SimonEngine::o_unfreezeBottom() {
	_vgaBufFreeStart = _vgaFileBufOrg2;
	_vgaBufStart = _vgaFileBufOrg2;
	_vgaFileBufOrg = _vgaFileBufOrg2;
}

void SimonEngine::o_lockZone() {
	_vgaBufStart = _vgaBufFreeStart;
}

void SimonEngine::o_unlockZone() {
	_vgaBufFreeStart = _vgaFileBufOrg;
	_vgaBufStart = _vgaFileBufOrg;
}

void SimonEngine::o_playMusic() {
	int music = getVarOrWord();
	int track = getVarOrWord();

	// Jamieson630:
	// This appears to be a "load or play music" command.
	// The music resource is specified, and optionally
	// a track as well. Normally we see two calls being
	// made, one to load the resource and another to
	// actually start a track (so the resource is
	// effectively preloaded so there's no latency when
	// starting playback).
	if (getGameType() == GType_SIMON2) {
		int loop = getVarOrByte();

		midi.setLoop (loop != 0);
		if (_lastMusicPlayed != music)
			_nextMusicToPlay = music;
		else
			midi.startTrack (track);
	} else {
		if (music != _lastMusicPlayed) {
			_lastMusicPlayed = music;
			loadMusic (music);
			midi.startTrack (track);
		}
	}
}

void SimonEngine::o_sync(uint a) {
	uint16 id = to16Wrapper(a);
	_lockWord |= 0x8000;
	_vcPtr = (byte *)&id;
	vc15_wakeup_id();
	_lockWord &= ~0x8000;
}

void SimonEngine::o_playSFX(uint sound_id) {
	if (getGameId() == GID_SIMON1DOS)
		playSting(sound_id);
	else
		_sound->playEffects(sound_id);
}

void SimonEngine::o_setTextColor(uint color) {
	WindowBlock *window;

	window = _windowArray[_curWindow];
	window->text_color = color;
}

void SimonEngine::o_unk_103() {
	mouseOff();
	removeIconArray(_curWindow);
	if (getGameType() == GType_FF)
		showMessageFormat("\x0E");
	else
		showMessageFormat("\x0C");
	_oracleMaxScrollY = 0;
	_noOracleScroll = 0;
	mouseOn();
}

void SimonEngine::o_kill_sprite_simon1(uint a) {
	uint16 b = to16Wrapper(a);
	_lockWord |= 0x8000;
	_vcPtr = (byte *)&b;
	vc60_killSprite();
	_lockWord &= ~0x8000;
}

void SimonEngine::o_kill_sprite_simon2(uint a, uint b) {
	uint16 items[2];

	items[0] = to16Wrapper(a);
	items[1] = to16Wrapper(b);

	_lockWord |= 0x8000;
	_vcPtr = (byte *)&items;
	vc60_killSprite();
	_lockWord &= ~0x8000;
}

/* OK */
void SimonEngine::o_defineWindow(uint num, uint x, uint y, uint w, uint h, uint flags, uint fill_color, uint text_color) {
	num &= 7;

	if (_windowArray[num])
		closeWindow(num);

	_windowArray[num] = openWindow(x, y, w, h, flags, fill_color, text_color);

	if (num == _curWindow) {
		_textWindow = _windowArray[num];
		if (getGameType() == GType_FF)
			showmessage_helper_3(_textWindow->textColumn, _textWindow->width);
		else
			showmessage_helper_3(_textWindow->textLength, _textWindow->textMaxLength);
	}
}

} // End of namespace Simon
