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

// Item script opcodes for Simon1/Simon2

#include "common/stdafx.h"

#include "common/system.h"

#include "agos/animation.h"
#include "agos/agos.h"

#ifdef _WIN32_WCE
extern bool isSmartphone(void);
#endif

namespace AGOS {

void AGOSEngine::setupCommonOpcodes(OpcodeProc *op) {
	// A common set of opcodes for Elvira 2 and later.

	op[1] = &AGOSEngine::o_at;
	op[2] = &AGOSEngine::o_notAt;
	op[5] = &AGOSEngine::o_carried;
	op[6] = &AGOSEngine::o_notCarried;
	op[7] = &AGOSEngine::o_isAt;
	op[11] = &AGOSEngine::o_zero;
	op[12] = &AGOSEngine::o_notZero;
	op[13] = &AGOSEngine::o_eq;
	op[14] = &AGOSEngine::o_notEq;
	op[15] = &AGOSEngine::o_gt;
	op[16] = &AGOSEngine::o_lt;
	op[17] = &AGOSEngine::o_eqf;
	op[18] = &AGOSEngine::o_notEqf;
	op[19] = &AGOSEngine::o_ltf;
	op[20] = &AGOSEngine::o_gtf;
	op[23] = &AGOSEngine::o_chance;
	op[25] = &AGOSEngine::o_isRoom;
	op[26] = &AGOSEngine::o_isObject;
	op[27] = &AGOSEngine::o_state;
	op[28] = &AGOSEngine::o_oflag;
	op[31] = &AGOSEngine::o_destroy;
	op[33] = &AGOSEngine::o_place;
	op[36] = &AGOSEngine::o_copyff;
	op[41] = &AGOSEngine::o_clear;
	op[42] = &AGOSEngine::o_let;
	op[43] = &AGOSEngine::o_add;
	op[44] = &AGOSEngine::o_sub;
	op[45] = &AGOSEngine::o_addf;
	op[46] = &AGOSEngine::o_subf;
	op[47] = &AGOSEngine::o_mul;
	op[48] = &AGOSEngine::o_div;
	op[49] = &AGOSEngine::o_mulf;
	op[50] = &AGOSEngine::o_divf;
	op[51] = &AGOSEngine::o_mod;
	op[52] = &AGOSEngine::o_modf;
	op[53] = &AGOSEngine::o_random;
	op[55] = &AGOSEngine::o_goto;
	op[56] = &AGOSEngine::o_oset;
	op[57] = &AGOSEngine::o_oclear;
	op[58] = &AGOSEngine::o_putBy;
	op[59] = &AGOSEngine::o_inc;
	op[60] = &AGOSEngine::o_dec;
	op[61] = &AGOSEngine::o_setState;
	op[62] = &AGOSEngine::o_print;
	op[63] = &AGOSEngine::o_message;
	op[64] = &AGOSEngine::o_msg;
	op[68] = &AGOSEngine::o_end;
	op[69] = &AGOSEngine::o_done;
	op[71] = &AGOSEngine::o_process;
	op[76] = &AGOSEngine::o_when;
	op[77] = &AGOSEngine::o_if1;
	op[78] = &AGOSEngine::o_if2;
	op[79] = &AGOSEngine::o_isCalled;
	op[80] = &AGOSEngine::o_is;
	op[82] = &AGOSEngine::o_debug;
	op[87] = &AGOSEngine::o_comment;
	op[90] = &AGOSEngine::o_getParent;
	op[91] = &AGOSEngine::o_getNext;
	op[92] = &AGOSEngine::o_getChildren;
	op[96] = &AGOSEngine::o_picture;
	op[97] = &AGOSEngine::o_loadZone;
	op[100] = &AGOSEngine::o_killAnimate;
	op[101] = &AGOSEngine::o_defWindow;
	op[102] = &AGOSEngine::o_window;
	op[103] = &AGOSEngine::o_cls;
	op[104] = &AGOSEngine::o_closeWindow;
	op[107] = &AGOSEngine::o_addBox;
	op[108] = &AGOSEngine::o_delBox;
	op[109] = &AGOSEngine::o_enableBox;
	op[110] = &AGOSEngine::o_disableBox;
	op[111] = &AGOSEngine::o_moveBox;
	op[114] = &AGOSEngine::o_doIcons;
	op[115] = &AGOSEngine::o_isClass;
	op[116] = &AGOSEngine::o_setClass;
	op[117] = &AGOSEngine::o_unsetClass;
	op[119] = &AGOSEngine::o_waitSync;
	op[120] = &AGOSEngine::o_sync;
	op[121] = &AGOSEngine::o_defObj;
	op[125] = &AGOSEngine::o_here;
	op[126] = &AGOSEngine::o_doClassIcons;
	op[130] = &AGOSEngine::o_setAdjNoun;
	op[132] = &AGOSEngine::o_saveUserGame;
	op[133] = &AGOSEngine::o_loadUserGame;
	op[135] = &AGOSEngine::o_pauseGame;
	op[136] = &AGOSEngine::o_copysf;
	op[137] = &AGOSEngine::o_restoreIcons;
	op[138] = &AGOSEngine::o_freezeZones;
	op[139] = &AGOSEngine::o_placeNoIcons;
	op[140] = &AGOSEngine::o_clearTimers;
	op[141] = &AGOSEngine::o_setDollar;
	op[142] = &AGOSEngine::o_isBox;
	op[143] = &AGOSEngine::oe2_doTable;
	op[151] = &AGOSEngine::oe2_storeItem;
	op[152] = &AGOSEngine::oe2_getItem;
	op[153] = &AGOSEngine::oe2_bSet;
	op[154] = &AGOSEngine::oe2_bClear;
	op[155] = &AGOSEngine::oe2_bZero;
	op[156] = &AGOSEngine::oe2_bNotZero;
	op[157] = &AGOSEngine::oe2_getOValue;
	op[158] = &AGOSEngine::oe2_setOValue;
	op[160] = &AGOSEngine::oe2_ink;
}

void AGOSEngine::setupOpcodes() {
	memset(_opcode_table, 0, sizeof(_opcode_table));
	_numOpcodes = ARRAYSIZE(_opcode_table);

	switch (getGameType()) {
	case GType_ELVIRA1:
		setupElvira1Opcodes(_opcode_table);
		break;
	case GType_ELVIRA2:
		setupElvira2Opcodes(_opcode_table);
		break;
	case GType_WW:
		setupWaxworksOpcodes(_opcode_table);
		break;
	case GType_SIMON1:
		setupSimon1Opcodes(_opcode_table);
		break;
	case GType_SIMON2:
		setupSimon2Opcodes(_opcode_table);
		break;
	case GType_FF:
		setupFeebleOpcodes(_opcode_table);
		break;
	case GType_PP:
		setupPuzzleOpcodes(_opcode_table);
		break;
	default:
		error("setupOpcodes: Unknown game");
	}
}

void AGOSEngine::setScriptCondition(bool cond) {
	_runScriptCondition[_recursionDepth] = cond;
}

bool AGOSEngine::getScriptCondition() {
	return _runScriptCondition[_recursionDepth];
}

void AGOSEngine::setScriptReturn(int ret) {
	_runScriptReturn[_recursionDepth] = ret;
}

int AGOSEngine::getScriptReturn() {
	return _runScriptReturn[_recursionDepth];
}

// -----------------------------------------------------------------------
// Common Opcodes
// -----------------------------------------------------------------------

void AGOSEngine::o_at() {
	// 1: ptrA parent is
	setScriptCondition(me()->parent == getNextItemID());
}

void AGOSEngine::o_notAt() {
	// 2: ptrA parent is not
	setScriptCondition(me()->parent != getNextItemID());
}

void AGOSEngine::o_carried() {
	// 5: parent is 1
	setScriptCondition(getNextItemPtr()->parent == getItem1ID());
}

void AGOSEngine::o_notCarried() {
	// 6: parent isnot 1
	setScriptCondition(getNextItemPtr()->parent != getItem1ID());
}

void AGOSEngine::o_isAt() {
	// 7: parent is
	Item *item = getNextItemPtr();
	setScriptCondition(item->parent == getNextItemID());
}

void AGOSEngine::o_zero() {
	// 11: is zero
	setScriptCondition(getNextVarContents() == 0);
}

void AGOSEngine::o_notZero() {
	// 12: isnot zero
	setScriptCondition(getNextVarContents() != 0);
}

void AGOSEngine::o_eq() {
	// 13: equal
	uint tmp = getNextVarContents();
	setScriptCondition(tmp == getVarOrWord());
}

void AGOSEngine::o_notEq() {
	// 14: not equal
	uint tmp = getNextVarContents();
	setScriptCondition(tmp != getVarOrWord());
}

void AGOSEngine::o_gt() {
	// 15: is greater
	uint tmp = getNextVarContents();
	setScriptCondition(tmp > getVarOrWord());
}

void AGOSEngine::o_lt() {
	// 16: is less
	uint tmp = getNextVarContents();
	setScriptCondition(tmp < getVarOrWord());
}

void AGOSEngine::o_eqf() {
	// 17: is eq f
	uint tmp = getNextVarContents();
	setScriptCondition(tmp == getNextVarContents());
}

void AGOSEngine::o_notEqf() {
	// 18: is not equal f
	uint tmp = getNextVarContents();
	setScriptCondition(tmp != getNextVarContents());
}

void AGOSEngine::o_ltf() {
	// 19: is greater f
	uint tmp = getNextVarContents();
	setScriptCondition(tmp < getNextVarContents());
}

void AGOSEngine::o_gtf() {
	// 20: is less f
	uint tmp = getNextVarContents();
	setScriptCondition(tmp > getNextVarContents());
}

void AGOSEngine::o_chance() {
	// 23: chance
	uint a = getVarOrWord();

	if (a == 0) {
		setScriptCondition(false);
		return;
	}

	if (a == 100) {
		setScriptCondition(true);
		return;
	}

	a += _chanceModifier;

	if (a <= 0) {
		_chanceModifier = 0;
		setScriptCondition(false);
	} else if ((uint)_rnd.getRandomNumber(99) < a) {
		if (_chanceModifier <= 0)
			_chanceModifier -= 5;
		else
			_chanceModifier = 0;
		setScriptCondition(true);
	} else {
		if (_chanceModifier >= 0)
			_chanceModifier += 5;
		else
			_chanceModifier = 0;
		setScriptCondition(false);
	}
}

void AGOSEngine::o_isRoom() {
	// 25: is room
	setScriptCondition(isRoom(getNextItemPtr()));
}

void AGOSEngine::o_isObject() {
	// 26: is object
	setScriptCondition(isObject(getNextItemPtr()));
}

void AGOSEngine::o_state() {
	// 27: item state is
	Item *item = getNextItemPtr();
	setScriptCondition((uint) item->state == getVarOrWord());
}

void AGOSEngine::o_oflag() {
	// 28: item has prop
	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), 2);
	uint num = getVarOrByte();
	setScriptCondition(subObject != NULL && (subObject->objectFlags & (1 << num)) != 0);
}

void AGOSEngine::o_destroy() {
	// 31: set no parent
	setItemParent(getNextItemPtr(), NULL);
}

void AGOSEngine::o_place() {
	// 33: set item parent
	Item *item = getNextItemPtr();
	setItemParent(item, getNextItemPtr());
}

void AGOSEngine::o_copyff() {
	// 36: copy var
	uint value = getNextVarContents();
	writeNextVarContents(value);
}

void AGOSEngine::o_clear() {
	// 41: zero var
	writeNextVarContents(0);
}

void AGOSEngine::o_let() {
	// 42: set var
	uint var = getVarWrapper();
	writeVariable(var, getVarOrWord());
}

void AGOSEngine::o_add() {
	// 43: add
	uint var = getVarWrapper();
	writeVariable(var, readVariable(var) + getVarOrWord());
}

void AGOSEngine::o_sub() {
	// 44: sub
	uint var = getVarWrapper();
	writeVariable(var, readVariable(var) - getVarOrWord());
}

void AGOSEngine::o_addf() {
	// 45: add f
	uint var = getVarWrapper();
	writeVariable(var, readVariable(var) + getNextVarContents());
}

void AGOSEngine::o_subf() {
	// 46: sub f
	uint var = getVarWrapper();
	writeVariable(var, readVariable(var) - getNextVarContents());
}

void AGOSEngine::o_mul() {
	// 47: mul
	uint var = getVarWrapper();
	writeVariable(var, readVariable(var) * getVarOrWord());
}

void AGOSEngine::o_div() {
	// 48: div
	uint var = getVarWrapper();
	int value = getVarOrWord();
	if (value == 0)
		error("o_div: Division by zero");
	writeVariable(var, readVariable(var) / value);
}

void AGOSEngine::o_mulf() {
	// 49: mul f
	uint var = getVarWrapper();
	writeVariable(var, readVariable(var) * getNextVarContents());
}

void AGOSEngine::o_divf() {
	// 50: div f
	uint var = getVarWrapper();
	int value = getNextVarContents();
	if (value == 0)
		error("o_divf: Division by zero");
	writeVariable(var, readVariable(var) / value);
}

void AGOSEngine::o_mod() {
	// 51: mod
	uint var = getVarWrapper();
	int value = getVarOrWord();
	if (value == 0)
		error("o_mod: Division by zero");
	writeVariable(var, readVariable(var) % value);
}

void AGOSEngine::o_modf() {
	// 52: mod f
	uint var = getVarWrapper();
	int value = getNextVarContents();
	if (value == 0)
		error("o_modf: Division by zero");
	writeVariable(var, readVariable(var) % value);
}

void AGOSEngine::o_random() {
	// 53: random
	uint var = getVarWrapper();
	uint value = (uint16)getVarOrWord();

	// Copy protection subtitles aren't displayed in Amiga version
	if (getGameId() == GID_SIMON1AMIGA && var == 51 && value == 8)
		writeVariable(var, 4);
	else
		writeVariable(var, _rnd.getRandomNumber(value - 1));
}

void AGOSEngine::o_goto() {
	// 55: set itemA parent
	uint item = getNextItemID();
	setItemParent(me(), _itemArrayPtr[item]);
}

void AGOSEngine::o_oset() {
	// 56: set child2 fr bit
	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), 2);
	int value = getVarOrByte();
	if (subObject != NULL && value >= 0x10)
		subObject->objectFlags |= (1 << value);
}

void AGOSEngine::o_oclear() {
	// 57: clear child2 fr bit
	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), 2);
	int value = getVarOrByte();
	if (subObject != NULL && value >= 0x10)
		subObject->objectFlags &= ~(1 << value);
}

void AGOSEngine::o_putBy() {
	// 58: make siblings
	Item *item = getNextItemPtr();
	setItemParent(item, derefItem(getNextItemPtr()->parent));
}

void AGOSEngine::o_inc() {
	// 59: item inc state
	Item *item = getNextItemPtr();
	if (item->state <= 30000)
		setItemState(item, item->state + 1);
}

void AGOSEngine::o_dec() {
	// 60: item dec state
	Item *item = getNextItemPtr();
	if (item->state >= 0)
		setItemState(item, item->state - 1);
}

void AGOSEngine::o_setState() {
	// 61: item set state
	Item *item = getNextItemPtr();
	int value = getVarOrWord();
	if (value < 0)
		value = 0;
	if (value > 30000)
		value = 30000;
	setItemState(item, value);
}

void AGOSEngine::o_print() {
	// 62: show int
	showMessageFormat("%d", getNextVarContents());
}

void AGOSEngine::o_message() {
	// 63: show string nl
	showMessageFormat("%s\n", getStringPtrByID(getNextStringID()));
}

void AGOSEngine::o_msg() {
	// 64: show string
	showMessageFormat("%s", getStringPtrByID(getNextStringID()));
}

void AGOSEngine::o_end() {
	// 68: exit interpreter
	shutdown();
}

void AGOSEngine::o_done() {
	// 69: return 1
	setScriptReturn(1);
}

void AGOSEngine::o_process() {
	// 71: start subroutine
	Subroutine *sub = getSubroutineByID(getVarOrWord());
	if (sub != NULL)
		startSubroutine(sub);
}

void AGOSEngine::o_when() {
	// 76: add timeout
	uint timeout = getVarOrWord();
	addTimeEvent(timeout, getVarOrWord());
}

void AGOSEngine::o_if1() {
	// 77: has item minus 1
	setScriptCondition(_subjectItem != NULL);
}

void AGOSEngine::o_if2() {
	// 78: has item minus 3
	setScriptCondition(_objectItem != NULL);
}

void AGOSEngine::o_isCalled() {
	// 79: childstruct fr2 is
	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), 2);
	uint stringId = getNextStringID();
	setScriptCondition((subObject != NULL) && subObject->objectName == stringId);
}

void AGOSEngine::o_is() {
	// 80: item equal
	setScriptCondition(getNextItemPtr() == getNextItemPtr());
}

void AGOSEngine::o_debug() {
	// 82: debug opcode
	getVarOrByte();
}

void AGOSEngine::o_comment() {
	// 87: comment
	getNextStringID();
}

void AGOSEngine::o_haltAnimation() {
	// 88: stop animation
	_lockWord |= 0x10;
}

void AGOSEngine::o_restartAnimation() {
	// 89: restart animation
	_lockWord &= ~0x10;
}

void AGOSEngine::o_getParent() {
	// 90: set minusitem to parent
	Item *i = getNextItemPtr();
	if (getVarOrByte() == 1)
		_subjectItem = derefItem(i->parent);
	else
		_objectItem = derefItem(i->parent);
}

void AGOSEngine::o_getNext() {
	// 91: set minusitem to next
	Item *i = getNextItemPtr();
	if (getVarOrByte() == 1)
		_subjectItem = derefItem(i->next);
	else
		_objectItem = derefItem(i->next);
}

void AGOSEngine::o_getChildren() {
	// 92: set minusitem to child
	Item *i = getNextItemPtr();
	if (getVarOrByte() == 1)
		_subjectItem = derefItem(i->child);
	else
		_objectItem = derefItem(i->child);
}

void AGOSEngine::o_picture() {
	// 96
	uint vga_res = getVarOrWord();
	uint mode = getVarOrByte();

	_picture8600 = (vga_res == 8600);
	setWindowImageEx(mode, vga_res);
}

void AGOSEngine::o_loadZone() {
	// 97: load zone
	uint vga_res = getVarOrWord();

	_lockWord |= 0x80;

	if (getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2 ||
		getGameType() == GType_WW) {
		vc27_resetSprite();
		vc29_stopAllSounds();
	}

	loadZone(vga_res);
	_lockWord &= ~0x80;
}

void AGOSEngine::o_killAnimate() {
	// 100: kill animations
	_lockWord |= 0x8000;
	vc27_resetSprite();
	_lockWord &= ~0x8000;
}

void AGOSEngine::o_defWindow() {
	// 101: define window
	uint num = getVarOrByte();
	uint x = getVarOrWord();
	uint y = getVarOrWord();
	uint w = getVarOrWord();
	uint h = getVarOrWord();
	uint flags = getVarOrWord();
	uint color = getVarOrWord();

	uint fillColor, textColor;
	if (getGameType() == GType_ELVIRA1 || getGameType() == GType_ELVIRA2 ||
		getGameType() == GType_WW) {
		fillColor = color % 100;
		textColor = color / 100;
	} else {
		fillColor = color;
		textColor = 0;
	}

	num &= 7;

	if (_windowArray[num])
		closeWindow(num);

	_windowArray[num] = openWindow(x, y, w, h, flags, fillColor, textColor);

	if (num == _curWindow) {
		_textWindow = _windowArray[num];
		justifyStart();
	}
}

void AGOSEngine::o_window() {
	// 102
	changeWindow(getVarOrByte() & 7);
}

void AGOSEngine::o_cls() {
	// 103
	mouseOff();
	removeIconArray(_curWindow);
	showMessageFormat("\x0C");
	_oracleMaxScrollY = 0;
	_noOracleScroll = 0;
	mouseOn();
}

void AGOSEngine::o_closeWindow() {
	// 104
	closeWindow(getVarOrByte() & 7);
}

void AGOSEngine::o_addBox() {
	// 107: add item box
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
		flags |= kBFDragBox;

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

void AGOSEngine::o_delBox() {
	// 108: delete box
	undefineBox(getVarOrWord());
}

void AGOSEngine::o_enableBox() {
	// 109: enable box
	enableBox(getVarOrWord());
}

void AGOSEngine::o_disableBox() {
	// 110: set hitarea bit 0x40
	disableBox(getVarOrWord());
}

void AGOSEngine::o_moveBox() {
	// 111: set hitarea xy
	uint hitarea_id = getVarOrWord();
	uint x = getVarOrWord();
	uint y = getVarOrWord();
	moveBox(hitarea_id, x, y);
}

void AGOSEngine::o_doIcons() {
	// 114
	Item *item = getNextItemPtr();
	uint num = getVarOrByte();
	mouseOff();
	drawIconArray(num, item, 0, 0);
	mouseOn();
}

void AGOSEngine::o_isClass() {
	// 115: item has flag
	Item *item = getNextItemPtr();
	setScriptCondition((item->classFlags & (1 << getVarOrByte())) != 0);
}

void AGOSEngine::o_setClass() {
	// 116: item set flag
	Item *item = getNextItemPtr();
	item->classFlags |= (1 << getVarOrByte());
}

void AGOSEngine::o_unsetClass() {
	// 117: item clear flag
	Item *item = getNextItemPtr();
	item->classFlags &= ~(1 << getVarOrByte());
}

void AGOSEngine::o_waitSync() {
	// 119: wait vga
	uint var = getVarOrWord();
	_scriptVar2 = (var == 200);

	if (var != 200 || !_skipVgaWait)
		waitForSync(var);
	_skipVgaWait = false;
}

void AGOSEngine::o_sync() {
	// 120: sync
	sendSync(getVarOrWord());
}

void AGOSEngine::o_defObj() {
	// 121: set vga item
	uint slot = getVarOrByte();
	_objectArray[slot] = getNextItemPtr();
}

void AGOSEngine::o_here() {
	// 125: item is sibling with item 1
	Item *item = getNextItemPtr();
	setScriptCondition(me()->parent == item->parent);
}

void AGOSEngine::o_doClassIcons() {
	// 126: do class icons
	Item *item = getNextItemPtr();
	uint num = getVarOrByte();
	uint a = 1 << getVarOrByte();
	mouseOff();
	drawIconArray(num, item, 1, a);
	mouseOn();
}

void AGOSEngine::o_setAdjNoun() {
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

void AGOSEngine::o_saveUserGame() {
	// 132: save user game
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
	userGame(false);
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
}

void AGOSEngine::o_loadUserGame() {
	// 133: load user game
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
	userGame(true);
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
}

void AGOSEngine::o_pauseGame() {
	// 135: pause game
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);

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
		if (_keyPressed == keyYes || _keyPressed == (keyYes - 32))
			shutdown();
		else if (_keyPressed == keyNo || _keyPressed == (keyNo - 32))
			break;
	}

	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
}

void AGOSEngine::o_copysf() {
	// 136: set var to item unk3
	Item *item = getNextItemPtr();
	writeNextVarContents(item->state);
}

void AGOSEngine::o_restoreIcons() {
	// 137
	uint num = getVarOrByte();
	WindowBlock *window = _windowArray[num & 7];
	if (window->iconPtr)
		drawIconArray(num, window->iconPtr->itemRef, window->iconPtr->line, window->iconPtr->classMask);
}

void AGOSEngine::o_freezeZones() {
	// 138: freeze zones
	freezeBottom();
}

void AGOSEngine::o_placeNoIcons() {
	// 139: set parent special
	Item *item = getNextItemPtr();
	_noParentNotify = true;
	setItemParent(item, getNextItemPtr());
	_noParentNotify = false;
}

void AGOSEngine::o_clearTimers() {
	// 140: clear timers
	killAllTimers();

	if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2)
		addTimeEvent(3, 160);
}

void AGOSEngine::o_setDollar() {
	// 141: set m1 to m3
	uint which = getVarOrByte();
	Item *item = getNextItemPtr();
	if (which == 1) {
		_subjectItem = item;
	} else {
		_objectItem = item;
	}
}

void AGOSEngine::o_isBox() {
	// 142: is box dead
	setScriptCondition(isBoxDead(getVarOrWord()));
}

// -----------------------------------------------------------------------

byte AGOSEngine::getByte() {
	return *_codePtr++;
}

int AGOSEngine::getNextWord() {
	int16 a = (int16)READ_BE_UINT16(_codePtr);
	_codePtr += 2;
	return a;
}

uint AGOSEngine::getNextStringID() {
	return (uint16)getNextWord();
}

uint AGOSEngine::getVarOrByte() {
	if (getGameType() == GType_ELVIRA1) {
		return getVarOrWord();
	} else {
		uint a = *_codePtr++;
		if (a != 255)
			return a;
		return readVariable(*_codePtr++);
	}
}

uint AGOSEngine::getVarOrWord() {
	uint a = READ_BE_UINT16(_codePtr);
	_codePtr += 2;
	if (getGameType() == GType_PP) {
		if (a >= 60000 && a < 62048) {
			return readVariable(a - 60000);
		}
	} else {
		if (a >= 30000 && a < 30512) {
			return readVariable(a - 30000);
		}
	}
	return a;
}

uint AGOSEngine::getVarWrapper() {
	if (getGameType() == GType_ELVIRA1 || getGameType() == GType_PP)
		return getVarOrWord();
	else
		return getVarOrByte();
}

uint AGOSEngine::getNextVarContents() {
	return (uint16)readVariable(getVarWrapper());
}

uint AGOSEngine::readVariable(uint variable) {
	if (variable >= _numVars)
		error("readVariable: Variable %d out of range", variable);

	if (getGameType() == GType_PP) {
		return (uint16)_variableArray[variable];
	} else if (getGameType() == GType_FF) {
		if (getBitFlag(83))
			return (uint16)_variableArray2[variable];
		else
			return (uint16)_variableArray[variable];
	} else {
			return _variableArray[variable];
	}
}

void AGOSEngine::writeNextVarContents(uint16 contents) {
	writeVariable(getVarWrapper(), contents);
}

void AGOSEngine::writeVariable(uint variable, uint16 contents) {
	if (variable >= _numVars)
		error("writeVariable: Variable %d out of range", variable);

	if (getGameType() == GType_FF && getBitFlag(83))
		_variableArray2[variable] = contents;
	else
		_variableArray[variable] = contents;
}

int AGOSEngine::runScript() {
	int opcode;
	bool flag;

	do {
		if (_continousMainScript)
			dumpOpcode(_codePtr);

		if (getGameType() == GType_ELVIRA1) {
			opcode = getVarOrWord();
			if (opcode == 10000)
				return 0;
		} else {
			opcode = getByte();
			if (opcode == 0xFF)
				return 0;
		}
		debug(1, "runScript: opcode %d", opcode);

		if (_runScriptReturn1)
			return 1;

		/* Invert condition? */
		flag = false;
		if (getGameType() == GType_ELVIRA1) {
			if (opcode == 203) {
				flag = true;
				opcode = getVarOrWord();
				debug(1, "runScript: opcode %d", opcode);
				if (opcode == 10000)
					return 0;
			}
		} else {
			if (opcode == 0) {
				flag = true;
				opcode = getByte();
				debug(1, "runScript: opcode %d", opcode);

				if (opcode == 0xFF)
					return 0;
			}
		}

		setScriptCondition(true);
		setScriptReturn(0);

		if (opcode > _numOpcodes || !_opcode_table[opcode])
			error("Invalid opcode '%d' encountered", opcode);

		(this->*_opcode_table[opcode]) ();
	} while (getScriptCondition() != flag && !getScriptReturn());

	return getScriptReturn();
}

void AGOSEngine::scriptMouseOn() {
	if ((getGameType() == GType_FF || getGameType() == GType_PP) && _mouseCursor != 5) {
		resetVerbs();
		_noRightClick = 0;
	} else if (getGameType() == GType_SIMON2 && getBitFlag(79)) {
		_mouseCursor = 0;
	}
	_mouseHideCount = 0;
}

void AGOSEngine::scriptMouseOff() {
	_lockWord |= 0x8000;
	vc34_setMouseOff();
	_lockWord &= ~0x8000;
}

void AGOSEngine::sendSync(uint a) {
	uint16 id = to16Wrapper(a);
	_lockWord |= 0x8000;
	_vcPtr = (byte *)&id;
	vc15_sync();
	_lockWord &= ~0x8000;
}

void AGOSEngine::setTextColor(uint color) {
	WindowBlock *window;

	window = _windowArray[_curWindow];
	window->text_color = color;
}

void AGOSEngine::stopAnimate(uint a) {
	uint16 b = to16Wrapper(a);
	_lockWord |= 0x8000;
	_vcPtr = (byte *)&b;
	vc60_stopAnimation();
	_lockWord &= ~0x8000;
}

void AGOSEngine::stopAnimateSimon2(uint a, uint b) {
	uint16 items[2];

	items[0] = to16Wrapper(a);
	items[1] = to16Wrapper(b);

	_lockWord |= 0x8000;
	_vcPtr = (byte *)&items;
	vc60_stopAnimation();
	_lockWord &= ~0x8000;
}

void AGOSEngine::waitForSync(uint a) {
	const uint maxCount = (getGameType() == GType_SIMON1) ? 500 : 1000;

	if (getGameType() == GType_SIMON1 && (getFeatures() & GF_TALKIE)) {
		if (a != 200) {
			uint16 tmp = _lastVgaWaitFor;
			_lastVgaWaitFor = 0;
			if (tmp == a)
				return;
		}
	}

	_vgaWaitFor = a;
	_syncCount = 0;
	_exitCutscene = false;
	_rightButtonDown = false;

	while (_vgaWaitFor != 0) {
		if (_rightButtonDown) {
			if (_vgaWaitFor == 200 && (getGameType() == GType_FF || !getBitFlag(14))) {
				skipSpeech();
				break;
			}
		}
		if (_exitCutscene) {
			if (getGameType() == GType_ELVIRA1) {
				if (_variableArray[105] == 0) {
					_variableArray[105] = 255;
					break;
				}
			} else if (getGameType() == GType_ELVIRA2 || getGameType() == GType_WW) {
				if (_vgaWaitFor == 51) {
					setBitFlag(244, 1);
					break;
				}
			} else {
				if (getBitFlag(9)) {
					endCutscene();
					break;
				}
			}
		}
		processSpecialKeys();

		if (_syncCount >= maxCount) {
			warning("waitForSync: wait timed out");
			break;
		}

		delay(1);
	}
}

} // End of namespace AGOS
