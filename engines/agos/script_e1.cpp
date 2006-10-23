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

void AGOSEngine::setupElvira1Opcodes(OpcodeProc *op) {
	op[0] = &AGOSEngine::o_at;
	op[1] = &AGOSEngine::o_notAt;
	op[2] = &AGOSEngine::oe1_present;
	op[3] = &AGOSEngine::oe1_notPresent;
	op[4] = &AGOSEngine::oe1_worn;
	op[5] = &AGOSEngine::oe1_notWorn;
	op[6] = &AGOSEngine::o_carried;
	op[7] = &AGOSEngine::o_notCarried;
	op[8] = &AGOSEngine::o_isAt;
	op[9] = &AGOSEngine::oe1_isNotAt;
	op[10] = &AGOSEngine::oe1_sibling;
	op[11] = &AGOSEngine::oe1_notSibling;
	op[12] = &AGOSEngine::o_zero;
	op[13] = &AGOSEngine::o_notZero;
	op[14] = &AGOSEngine::o_eq;
	op[15] = &AGOSEngine::o_notEq;
	op[16] = &AGOSEngine::o_gt;
	op[17] = &AGOSEngine::o_lt;
	op[18] = &AGOSEngine::o_eqf;
	op[19] = &AGOSEngine::o_notEqf;
	op[20] = &AGOSEngine::o_ltf;
	op[21] = &AGOSEngine::o_gtf;
	op[22] = &AGOSEngine::oe1_isIn;
	op[23] = &AGOSEngine::oe1_isNotIn;

	op[29] = &AGOSEngine::o_chance;
	op[30] = &AGOSEngine::oe1_isPlayer;

	op[32] = &AGOSEngine::o_isRoom;
	op[33] = &AGOSEngine::o_isObject;
	op[34] = &AGOSEngine::o_state;

	op[36] = &AGOSEngine::o_oflag;
	op[37] = &AGOSEngine::oe1_canPut;

	op[48] = &AGOSEngine::o_destroy;

	op[51] = &AGOSEngine::o_place;

	op[54] = &AGOSEngine::oe1_copyof;
	op[55] = &AGOSEngine::oe1_copyfo;
	op[56] = &AGOSEngine::o_copyff;
	op[57] = &AGOSEngine::oe1_whatO;

	op[59] = &AGOSEngine::oe1_weigh;
	op[60] = &AGOSEngine::oe1_setFF;
	op[61] = &AGOSEngine::o_clear;

	op[64] = &AGOSEngine::o_let;
	op[65] = &AGOSEngine::o_add;
	op[66] = &AGOSEngine::o_sub;
	op[67] = &AGOSEngine::o_addf;
	op[68] = &AGOSEngine::o_subf;
	op[69] = &AGOSEngine::o_mul;
	op[70] = &AGOSEngine::o_div;
	op[71] = &AGOSEngine::o_mulf;
	op[72] = &AGOSEngine::o_divf;
	op[73] = &AGOSEngine::o_mod;
	op[74] = &AGOSEngine::o_modf;
	op[75] = &AGOSEngine::o_random;

	op[76] = &AGOSEngine::oe1_moveDirn;
	op[77] = &AGOSEngine::o_goto;

	op[80] = &AGOSEngine::o_oset;
	op[81] = &AGOSEngine::o_oclear;

	op[84] = &AGOSEngine::o_putBy;
	op[85] = &AGOSEngine::o_inc;
	op[86] = &AGOSEngine::o_dec;
	op[87] = &AGOSEngine::o_setState;

	op[89] = &AGOSEngine::o_print;
	op[90] = &AGOSEngine::oe1_score;
	op[91] = &AGOSEngine::o_message;
	op[92] = &AGOSEngine::o_msg;

	op[97] = &AGOSEngine::o_end;
	op[98] = &AGOSEngine::o_done;

	op[105] = &AGOSEngine::o_process;
	op[106] = &AGOSEngine::oe1_doClass;

	op[112] = &AGOSEngine::oe1_pObj;

	op[114] = &AGOSEngine::oe1_pName;
	op[115] = &AGOSEngine::oe1_pcName;

	op[119] = &AGOSEngine::o_when;

	op[128] = &AGOSEngine::o_if1;
	op[129] = &AGOSEngine::o_if2;

	op[135] = &AGOSEngine::o_isCalled;
	op[136] = &AGOSEngine::o_is;

	op[152] = &AGOSEngine::o_debug;

	op[162] = &AGOSEngine::oe1_cFlag;

	op[164] = &AGOSEngine::o1_rescan;
	op[165] = &AGOSEngine::oe1_means;

	op[176] = &AGOSEngine::oe1_setUserItem;
	op[177] = &AGOSEngine::oe1_getUserItem;
	op[178] = &AGOSEngine::oe1_clearUserItem;

	op[180] = &AGOSEngine::oe1_whereTo;
	op[181] = &AGOSEngine::oe1_doorExit;

	op[198] = &AGOSEngine::o_comment;

	op[206] = &AGOSEngine::o_getParent;
	op[207] = &AGOSEngine::o_getNext;
	op[208] = &AGOSEngine::o_getChildren;

	op[219] = &AGOSEngine::oe1_findMaster;
	op[220] = &AGOSEngine::oe1_nextMaster;

	op[224] = &AGOSEngine::o_picture;
	op[225] = &AGOSEngine::o_loadZone;
	op[226] = &AGOSEngine::o1_animate;
	op[227] = &AGOSEngine::o1_stopAnimate;
	op[228] = &AGOSEngine::o_killAnimate;
	op[229] = &AGOSEngine::o_defWindow;
	op[230] = &AGOSEngine::o_window;
	op[231] = &AGOSEngine::o_cls;
	op[232] = &AGOSEngine::o_closeWindow;
	op[233] = &AGOSEngine::oe1_menu;

	op[235] = &AGOSEngine::o_addBox;
	op[236] = &AGOSEngine::o_delBox;
	op[237] = &AGOSEngine::o_enableBox;
	op[238] = &AGOSEngine::o_disableBox;
	op[239] = &AGOSEngine::o_moveBox;

	op[242] = &AGOSEngine::o_doIcons;
	op[243] = &AGOSEngine::o_isClass;

	op[249] = &AGOSEngine::o_setClass;
	op[250] = &AGOSEngine::o_unsetClass;

	op[251] = &AGOSEngine::oe1_bitClear;
	op[252] = &AGOSEngine::oe1_bitSet;
	op[253] = &AGOSEngine::oe1_bitTest;

	op[255] = &AGOSEngine::o_waitSync;
	op[256] = &AGOSEngine::o_sync;
	op[257] = &AGOSEngine::o_defObj;

	op[258] = &AGOSEngine::oe1_enableInput;
	op[259] = &AGOSEngine::oe1_setTime;
	op[260] = &AGOSEngine::oe1_ifTime;
	op[261] = &AGOSEngine::o_here;
	op[262] = &AGOSEngine::o_doClassIcons;
	op[263] = &AGOSEngine::o1_playTune;
	op[264] = &AGOSEngine::o_waitEndTune;
	op[265] = &AGOSEngine::o_ifEndTune;
	op[266] = &AGOSEngine::o_setAdjNoun;
	op[267] = &AGOSEngine::oe1_zoneDisk;
	op[268] = &AGOSEngine::o_saveUserGame;
	op[269] = &AGOSEngine::o_loadUserGame;
	op[270] = &AGOSEngine::oe1_printStats;
	op[271] = &AGOSEngine::o_stopTune;
	op[272] = &AGOSEngine::oe1_printPlayerDamage;
	op[273] = &AGOSEngine::oe1_printMonsterDamage;
	op[274] = &AGOSEngine::o_pauseGame;
	op[275] = &AGOSEngine::o_copysf;
	op[276] = &AGOSEngine::o_restoreIcons;
	op[277] = &AGOSEngine::oe1_printPlayerHit;
	op[278] = &AGOSEngine::oe1_printMonsterHit;
	op[279] = &AGOSEngine::o_freezeZones;
	op[280] = &AGOSEngine::o_placeNoIcons;
	op[281] = &AGOSEngine::o_clearTimers;
	op[282] = &AGOSEngine::o_setDollar;
	op[283] = &AGOSEngine::o_isBox;
}

// -----------------------------------------------------------------------
// Elvira 1 Opcodes
// -----------------------------------------------------------------------

void AGOSEngine::oe1_present() {
	// 2: present (here or carried)
	Item *item = getNextItemPtr();
	setScriptCondition(item->parent == getItem1ID() || item->parent == me()->parent);
}

void AGOSEngine::oe1_notPresent() {
	// 3: not present (neither here nor carried)
	Item *item = getNextItemPtr();
	setScriptCondition(item->parent != getItem1ID() && item->parent != me()->parent);
}

void AGOSEngine::oe1_worn() {
	// 4: worn
	Item *item = getNextItemPtr();
	SubObject *subObject = (SubObject *)findChildOfType(item, 2);

	if (item->parent != getItem1ID() || subObject == NULL)
		setScriptCondition(false);
	else
		setScriptCondition((subObject->objectFlags & kOFWorn) != 0);
}

void AGOSEngine::oe1_notWorn() {
	// 5: not worn
	Item *item = getNextItemPtr();
	SubObject *subObject = (SubObject *)findChildOfType(item, 2);

	if (item->parent != getItem1ID() || subObject == NULL)
		setScriptCondition(false);
	else
		setScriptCondition((subObject->objectFlags & kOFWorn) == 0);
}

void AGOSEngine::oe1_isNotAt() {
	// 9: parent is not
	Item *item = getNextItemPtr();
	setScriptCondition(item->parent != getNextItemID());
}

void AGOSEngine::oe1_sibling() {
	// 10: sibling
	Item *item1 = getNextItemPtr();
	Item *item2 = getNextItemPtr();
	setScriptCondition(item1->parent == item2->parent);
}

void AGOSEngine::oe1_notSibling() {
	// 11: not sibling
	Item *item1 = getNextItemPtr();
	Item *item2 = getNextItemPtr();
	setScriptCondition(item1->parent != item2->parent);
}

void AGOSEngine::oe1_isIn() {
	// 22: is in
	Item *item1 = getNextItemPtr();
	Item *item2 = getNextItemPtr();
	setScriptCondition(contains(item1, item2) != 0);
}

void AGOSEngine::oe1_isNotIn() {
	// 23: is not in
	Item *item1 = getNextItemPtr();
	Item *item2 = getNextItemPtr();
	setScriptCondition(contains(item1, item2) == 0);
}

void AGOSEngine::oe1_isPlayer() {
	// 30: is player
	setScriptCondition(isPlayer(getNextItemPtr()));
}

void AGOSEngine::oe1_canPut() {
	// 37: can put
	Item *item1 = getNextItemPtr();
	Item *item2 = getNextItemPtr();
	setScriptCondition(canPlace(item1, item2) == 0);
}

void AGOSEngine::oe1_copyof() {
	// 54: copy of
	Item *item = getNextItemPtr();
	uint tmp = getVarOrByte();
	writeNextVarContents(getUserFlag(item, tmp));
}

void AGOSEngine::oe1_copyfo() {
	// 55: copy fo
	uint tmp = getNextVarContents();
	Item *item = getNextItemPtr();
	setUserFlag(item, getVarOrByte(), tmp);
}

void AGOSEngine::oe1_whatO() {
	// 57: what o
	int a = getVarOrWord();	

	if (a == 1)
		_subjectItem = findMaster(_scriptAdj1,_scriptNoun1);
	else
		_objectItem = findMaster(_scriptAdj2, _scriptNoun2);
}

void AGOSEngine::oe1_weigh() {
	// 59: weight
	Item *item = getNextItemPtr();
	writeNextVarContents(weighUp(item));
}

void AGOSEngine::oe1_setFF() {
	// 60: set FF
	writeNextVarContents(255);
}

void AGOSEngine::oe1_moveDirn() {
	// 54: move direction
	int16 d = readVariable(getVarOrWord());
	moveDirn_e1(me(), d);
}

void AGOSEngine::oe1_score() {
	// 90: score
	SubPlayer *p = (SubPlayer *) findChildOfType(me(), 3);
	showMessageFormat("Your score is %ld.\n", p->score);
}

void AGOSEngine::oe1_doClass() {
	// 106: do class
	Item *i = getNextItemPtr();
	int16 cm = getVarOrWord();
	int16 num = getVarOrWord();

	_classMask = (cm != -1) ? 1 << cm : 0;
	//_classLine = (SubroutineLine *)((uint32)_currentLine->next+(uint32)_currentTable);

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

void AGOSEngine::oe1_pObj() {
	// 112: print object
	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), 2);
	getVarOrWord();

	if (subObject != NULL)
		showMessageFormat("%s", (const char *)getStringPtrByID(subObject->objectName));
}

void AGOSEngine::oe1_pName() {
	// 114:
	Item *i = getNextItemPtr();
	showMessageFormat("%s", (const char *)getStringPtrByID(i->itemName));
}

void AGOSEngine::oe1_pcName() {
	// 115:
	Item *i = getNextItemPtr();
	Common::String name = (const char *)getStringPtrByID(i->itemName);
	name.toUppercase();
	showMessageFormat("%s", name.c_str());
}

void AGOSEngine::oe1_cFlag() {
	// 162: check container flag
	SubContainer *c = (SubContainer *)findChildOfType(getNextItemPtr(), 7);
	uint bit = getVarOrWord();

	if (c == NULL)
		setScriptCondition(false);
	else
		setScriptCondition((c->flags & (1 << bit)) != 0);
}

void AGOSEngine::oe1_means() {
	// 165: means
	_scriptVerb = getNextWord();
	_scriptNoun1 = getNextWord();
	_scriptNoun2 = getNextWord();

	if (getVarOrWord()) {
		int16 tmp = _scriptNoun1;
		_scriptNoun1 = _scriptNoun2;
		_scriptNoun2 = tmp;
	}
}

void AGOSEngine::oe1_setUserItem() {
	// 176: set user item
	Item *i = getNextItemPtr();
	uint tmp = getVarOrWord();
	setUserItem(i, tmp, getNextItemID());
}

void AGOSEngine::oe1_getUserItem() {
	// 177: get user item
	Item *i = getNextItemPtr();
	int n = getVarOrWord();

	if (getVarOrWord() == 1)
		_subjectItem = derefItem(getUserItem(i, n));
	else
		_objectItem = derefItem(getUserItem(i, n));
}

void AGOSEngine::oe1_whereTo() {
	// 180: where to
	Item *i = getNextItemPtr();
	int16 d = getVarOrWord();
	int16 f = getVarOrWord();

	if (f == 1)
		_subjectItem = derefItem(getExitOf_e1(i, d));
	else
		_objectItem = derefItem(getExitOf_e1(i, d));
}

void AGOSEngine::oe1_doorExit() {
	// 181: door exit
	Item *x;
	Item *a = (Item *)-1;
	SubUserChain *c;
	Item *i = getNextItemPtr();
	Item *d = getNextItemPtr();
	int16 f = getVarOrWord();
	int16 ct = 0;

	c = (SubUserChain *)findChildOfType(d, 8);
	if (c)
		a = derefItem(c->chChained);
	while (ct < 6) {
		x = derefItem(getDoorOf(i, ct));
		if ((x == d) | (x == a)) {
			writeVariable(f, ct);
			return;
		}
		ct++;
	}
	writeVariable(f, 255);
}

void AGOSEngine::oe1_clearUserItem() {
	// 178: clear user item
	Item *i = getNextItemPtr();
	uint tmp = getVarOrWord();
	setUserItem(i, tmp, 0);
}

void AGOSEngine::oe1_findMaster() {
	// 219: find master
	int16 ad, no;
	int16 d = getVarOrByte();

	ad = (d == 1) ? _scriptAdj1 : _scriptAdj2;
	no = (d == 1) ? _scriptNoun1 : _scriptNoun2;

	d = getVarOrByte();
	if (d == 1)
		_subjectItem = findMaster(ad, no);
	else
		_objectItem = findMaster(ad, no);
}

void AGOSEngine::oe1_nextMaster() {
	// 220: next master
	int16 ad, no;
	Item *item = getNextItemPtr();
	int16 d = getVarOrByte();

	ad = (d == 1) ? _scriptAdj1 : _scriptAdj2;
	no = (d == 1) ? _scriptNoun1 : _scriptNoun2;

	d = getVarOrByte();
	if (d == 1)
		_subjectItem = nextMaster(item, ad, no);
	else
		_objectItem = nextMaster(item, ad, no);
}

void AGOSEngine::oe1_menu() {
	// 233: agos menu
	uint b = getVarOrWord();
	uint a = getVarOrWord();
	drawMenuStrip(a, b);
}

void AGOSEngine::oe1_bitClear() {
	// 251: set bit off
	int var = getVarOrWord();
	int bit = getVarOrWord();

	writeVariable(var, _variableArray[var] & ~(1 << bit));
}

void AGOSEngine::oe1_bitSet() {
	// 252: set bit on
	int var = getVarOrWord();
	int bit = getVarOrWord();

	writeVariable(var, _variableArray[var] | (1 << bit));
}

void AGOSEngine::oe1_bitTest() {
	// 253: bit test
	int var = getVarOrWord();
	int bit = getVarOrWord();

	setScriptCondition((_variableArray[var] & (1 << bit)) != 0);
}

void AGOSEngine::oe1_enableInput() {
	// 258: enable input
	_variableArray[500] = 0;

	for (int i = 120; i < 130; i++)
		disableBox(i);

	// XXX
	_lastHitArea = 0;
	_verbHitArea = 0;
	_hitAreaSubjectItem = NULL;
	_hitAreaObjectItem = NULL;
}

void AGOSEngine::oe1_setTime() {
	// 259: set time
	time(&_timeStore);
}

void AGOSEngine::oe1_ifTime() {
	// 260: if time
	time_t t;

	uint a = getVarOrWord();
	time(&t);
	t -= a;
	if (t >= _timeStore)
		setScriptCondition(true);
	else
		setScriptCondition(false);
}

void AGOSEngine::oe1_zoneDisk() {
	// 267: set disk number of each zone
	getVarOrWord();
	getVarOrWord();
}

void AGOSEngine::oe1_printStats() {
	// 270: print stats
	WindowBlock *window = _dummyWindow;
	int val;

	window->flags = 1;

	mouseOff();

	// Strength
	val = _variableArray[0];
	if (val < -99)
		val = -99;
	if (val > 99)
		val = 99;	
	writeChar(window, 5, 133, 6, val);

	// Resolution
	val = _variableArray[1];
	if (val < -99)
		val = -99;
	if (val > 99)
		val = 99;	
	writeChar(window, 11, 133, 6, val);

	// Dexterity
	val = _variableArray[2];
	if (val < -99)
		val = -99;
	if (val > 99)
		val = 99;	
	writeChar(window, 18, 133, 0, val);

	// Skill
	val = _variableArray[3];
	if (val < -99)
		val = -99;
	if (val > 99)
		val = 99;	
	writeChar(window, 24, 133, 0, val);

	// Life
	val = _variableArray[5];
	if (val < -99)
		val = -99;
	if (val > 99)
		val = 99;	
	writeChar(window, 30, 133, 2, val);

	// Experience
	val = _variableArray[6];
	if (val < -99)
		val = -99;
	if (val > 99)
		val = 99;	
	writeChar(window, 36, 133, 4, val);

	mouseOn();
}

void AGOSEngine::oe1_printPlayerDamage() {
	// 272: print player damage
	WindowBlock *window = _dummyWindow;
	window->flags = 1;

	mouseOff();
	writeChar(window, 36, 38, 2, _variableArray[441]);
	mouseOn();
}

void AGOSEngine::oe1_printMonsterDamage() {
	// 273: print monster damage
	WindowBlock *window = _dummyWindow;
	window->flags = 1;

	mouseOff();
	writeChar(window, 36, 88, 2, _variableArray[442]);
	mouseOn();
}

void AGOSEngine::oe1_printPlayerHit() {
	// 277: print player hit
	WindowBlock *window = _dummyWindow;
	window->flags = 1;

	mouseOff();
	writeChar(window, 3, 166, 0, _variableArray[414]);
	mouseOn();
}

void AGOSEngine::oe1_printMonsterHit() {
	// 278: print monster hit
	WindowBlock *window = _dummyWindow;
	window->flags = 1;

	mouseOff();
	writeChar(window, 35, 166, 4, _variableArray[415]);
	mouseOn();
}

} // End of namespace AGOS
