/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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

#include "common/stdafx.h"
#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/intern.h"
#include "scumm/object.h"
#include "scumm/scumm.h"
#include "scumm/util.h"
#include "scumm/verbs.h"

namespace Scumm {

#define OPCODE(x)	_OPCODE(ScummEngine_c64, x)

void ScummEngine_c64::setupOpcodes() {
	static const OpcodeEntryC64 opcodes[256] = {
		/* 00 */
		OPCODE(o5_stopObjectCode),
		OPCODE(o2_putActor),
		OPCODE(o5_startMusic),
		OPCODE(o5_getActorRoom),
		/* 04 */
		OPCODE(o_isGreaterEqual),
		OPCODE(o_stopScript),
		OPCODE(o_getDist),
		OPCODE(o5_getActorRoom),
		/* 08 */
		OPCODE(o_isNotEqual),
		OPCODE(o5_faceActor),
		OPCODE(o2_assignVarWordIndirect),
		OPCODE(o2_setObjPreposition),
		/* 0C */
		OPCODE(o_loadSound),
		OPCODE(o5_walkActorToActor),
		OPCODE(o_putActorAtObject),
		OPCODE(o2_ifNotState08),
		/* 10 */
		OPCODE(o5_breakHere),
		OPCODE(o_animateActor),
		OPCODE(o2_panCameraTo),
		OPCODE(o_unknown13),
		/* 14 */
		OPCODE(o5_print),
		OPCODE(o2_actorFromPos),
		OPCODE(o5_getRandomNr),
		OPCODE(o_clearState08),
		/* 18 */
		OPCODE(o5_jumpRelative),
		OPCODE(o2_doSentence),
		OPCODE(o_move),
		OPCODE(o2_setBitVar),
		/* 1C */
		OPCODE(o5_startSound),
		OPCODE(o2_ifClassOfIs),
		OPCODE(o2_walkActorTo),
		OPCODE(o2_ifState02),
		/* 20 */
		OPCODE(o5_stopMusic),
		OPCODE(o2_putActor),
		OPCODE(o5_saveLoadGame),
		OPCODE(o2_getActorY),
		/* 24 */
		OPCODE(o2_loadRoomWithEgo),
		OPCODE(o5_loadRoom),
		OPCODE(o5_setVarRange),
		OPCODE(o2_getActorY),
		/* 28 */
		OPCODE(o5_equalZero),
		OPCODE(o2_setOwnerOf),
		OPCODE(o2_delay),
		OPCODE(o5_delayVariable),
		/* 2C */
		OPCODE(o2_assignVarByte),
		OPCODE(o5_putActorInRoom),
		OPCODE(o2_delay),
		OPCODE(o2_ifNotState04),
		/* 30 */
		OPCODE(o_loadActor),
		OPCODE(o2_getBitVar),
		OPCODE(o2_setCameraAt),
		OPCODE(o_freezeScript),
		/* 34 */
		OPCODE(o_getDist),
		OPCODE(o2_findObject),
		OPCODE(o_walkActorToObject),
		OPCODE(o2_setState01),
		/* 38 */
		OPCODE(o_isLessEqual),
		OPCODE(o2_doSentence),
		OPCODE(o_subtract),
		OPCODE(o2_waitForActor),
		/* 3C */
		OPCODE(o5_stopSound),
		OPCODE(o2_setActorElevation),
		OPCODE(o2_walkActorTo),
		OPCODE(o2_ifState02),
		/* 40 */
		OPCODE(o2_cutscene),
		OPCODE(o2_putActor),
		OPCODE(o2_startScript),
		OPCODE(o2_getActorX),
		/* 44 */
		OPCODE(o_isLess),
		OPCODE(o2_drawObject),
		OPCODE(o_increment),
		OPCODE(o2_getActorX),
		/* 48 */
		OPCODE(o_isEqual),
		OPCODE(o5_faceActor),
		OPCODE(o_loadRoom),
		OPCODE(o2_setObjPreposition),
		/* 4C */
		OPCODE(o_loadScript),
		OPCODE(o_unknown4D),
		OPCODE(o_putActorAtObject),
		OPCODE(o2_ifState08),
		/* 50 */
		OPCODE(o2_pickupObject),
		OPCODE(o_animateActor),
		OPCODE(o5_actorFollowCamera),
		OPCODE(o_unknown53),
		/* 54 */
		OPCODE(o5_setObjectName),
		OPCODE(o2_actorFromPos),
		OPCODE(o5_getActorMoving),
		OPCODE(o_clearState08),
		/* 58 */
		OPCODE(o2_beginOverride),
		OPCODE(o2_doSentence),
		OPCODE(o_add),
		OPCODE(o2_setBitVar),
		/* 5C */
		OPCODE(o5_startSound),
		OPCODE(o2_ifClassOfIs),
		OPCODE(o2_walkActorTo),
		OPCODE(o2_ifNotState02),
		/* 60 */
		OPCODE(o_cursorCommand),
		OPCODE(o2_putActor),
		OPCODE(o_stopScript),
		OPCODE(o5_getActorFacing),
		/* 64 */
		OPCODE(o2_loadRoomWithEgo),
		OPCODE(o2_drawObject),
		OPCODE(o5_getClosestObjActor),
		OPCODE(o2_clearState04),
		/* 68 */
		OPCODE(o5_isScriptRunning),
		OPCODE(o2_setOwnerOf),
		OPCODE(o2_subIndirect),
		OPCODE(o2_dummy),
		/* 6C */
		OPCODE(o2_getObjPreposition),
		OPCODE(o5_putActorInRoom),
		OPCODE(o2_dummy),
		OPCODE(o2_ifState08),
		/* 70 */
		OPCODE(o_lights),
		OPCODE(o5_getActorCostume),
		OPCODE(o5_loadRoom),
		OPCODE(o_getObjectOwner),
		/* 74 */
		OPCODE(o_getDist),
		OPCODE(o5_printEgo),
		OPCODE(o_walkActorToObject),
		OPCODE(o2_clearState01),
		/* 78 */
		OPCODE(o_isGreater),
		OPCODE(o2_doSentence),
		OPCODE(o2_verbOps),
		OPCODE(o2_getActorWalkBox),
		/* 7C */
		OPCODE(o5_isSoundRunning),
		OPCODE(o2_setActorElevation),
		OPCODE(o2_walkActorTo),
		OPCODE(o2_ifState01),
		/* 80 */
		OPCODE(o5_breakHere),
		OPCODE(o2_putActor),
		OPCODE(o5_startMusic),
		OPCODE(o5_getActorRoom),
		/* 84 */
		OPCODE(o_isGreaterEqual),
		OPCODE(o2_drawObject),
		OPCODE(o2_getActorElevation),
		OPCODE(o5_getActorRoom),
		/* 88 */
		OPCODE(o_isNotEqual),
		OPCODE(o5_faceActor),
		OPCODE(o2_assignVarWordIndirect),
		OPCODE(o2_setObjPreposition),
		/* 8C */
		OPCODE(o2_resourceRoutines),
		OPCODE(o5_walkActorToActor),
		OPCODE(o_putActorAtObject),
		OPCODE(o2_ifNotState08),
		/* 90 */
		OPCODE(o_pickupObject),
		OPCODE(o_animateActor),
		OPCODE(o2_panCameraTo),
		OPCODE(o_unknown93),
		/* 94 */
		OPCODE(o5_print),
		OPCODE(o2_actorFromPos),
		OPCODE(o5_getRandomNr),
		OPCODE(o_setState08),
		/* 98 */
		OPCODE(o2_restart),
		OPCODE(o2_doSentence),
		OPCODE(o_move),
		OPCODE(o2_setBitVar),
		/* 9C */
		OPCODE(o5_startSound),
		OPCODE(o2_ifClassOfIs),
		OPCODE(o2_walkActorTo),
		OPCODE(o2_ifNotState04),
		/* A0 */
		OPCODE(o5_stopObjectCode),
		OPCODE(o2_putActor),
		OPCODE(o5_saveLoadGame),
		OPCODE(o2_getActorY),
		/* A4 */
		OPCODE(o2_loadRoomWithEgo),
		OPCODE(o5_loadRoom),
		OPCODE(o5_setVarRange),
		OPCODE(o2_getActorY),
		/* A8 */
		OPCODE(o5_notEqualZero),
		OPCODE(o2_setOwnerOf),
		OPCODE(o2_addIndirect),
		OPCODE(o2_switchCostumeSet),
		/* AC */
		OPCODE(o2_drawSentence),
		OPCODE(o5_putActorInRoom),
		OPCODE(o2_waitForMessage),
		OPCODE(o2_ifNotState08),
		/* B0 */
		OPCODE(o_loadActor),
		OPCODE(o2_getBitVar),
		OPCODE(o2_setCameraAt),
		OPCODE(o_unfreezeScript),
		/* B4 */
		OPCODE(o_getDist),
		OPCODE(o2_findObject),
		OPCODE(o_walkActorToObject),
		OPCODE(o2_setState04),
		/* B8 */
		OPCODE(o_isLessEqual),
		OPCODE(o2_doSentence),
		OPCODE(o_subtract),
		OPCODE(o2_waitForActor),
		/* BC */
		OPCODE(o5_stopSound),
		OPCODE(o2_setActorElevation),
		OPCODE(o2_walkActorTo),
		OPCODE(o2_ifNotState01),
		/* C0 */
		OPCODE(o2_endCutscene),
		OPCODE(o2_putActor),
		OPCODE(o2_startScript),
		OPCODE(o2_getActorX),
		/* C4 */
		OPCODE(o_isLess),
		OPCODE(o2_drawObject),
		OPCODE(o_decrement),
		OPCODE(o2_getActorX),
		/* C8 */
		OPCODE(o_isEqual),
		OPCODE(o5_faceActor),
		OPCODE(o_loadRoom),
		OPCODE(o2_setObjPreposition),
		/* CC */
		OPCODE(o_loadScript),
		OPCODE(o5_walkActorToActor),
		OPCODE(o_putActorAtObject),
		OPCODE(o2_ifState08),
		/* D0 */
		OPCODE(o2_pickupObject),
		OPCODE(o_animateActor),
		OPCODE(o5_actorFollowCamera),
		OPCODE(o2_actorOps),
		/* D4 */
		OPCODE(o5_setObjectName),
		OPCODE(o2_actorFromPos),
		OPCODE(o5_getActorMoving),
		OPCODE(o_setState08),
		/* D8 */
		OPCODE(o5_printEgo),
		OPCODE(o2_doSentence),
		OPCODE(o_add),
		OPCODE(o2_setBitVar),
		/* DC */
		OPCODE(o5_startSound),
		OPCODE(o2_ifClassOfIs),
		OPCODE(o2_walkActorTo),
		OPCODE(o2_ifNotState04),
		/* E0 */
		OPCODE(o2_cursorCommand),
		OPCODE(o2_putActor),
		OPCODE(o_stopScript),
		OPCODE(o5_getActorFacing),
		/* E4 */
		OPCODE(o2_loadRoomWithEgo),
		OPCODE(o2_drawObject),
		OPCODE(o5_getClosestObjActor),
		OPCODE(o2_clearState04),
		/* E8 */
		OPCODE(o5_isScriptRunning),
		OPCODE(o2_setOwnerOf),
		OPCODE(o2_subIndirect),
		OPCODE(o2_dummy),
		/* EC */
		OPCODE(o2_getObjPreposition),
		OPCODE(o5_putActorInRoom),
		OPCODE(o2_dummy),
		OPCODE(o2_ifNotState08),
		/* F0 */
		OPCODE(o_lights),
		OPCODE(o5_getActorCostume),
		OPCODE(o5_loadRoom),
		OPCODE(o_getObjectOwner),
		/* F4 */
		OPCODE(o_getDist),
		OPCODE(o2_findObject),
		OPCODE(o_walkActorToObject),
		OPCODE(o2_setState04),
		/* F8 */
		OPCODE(o_isGreater),
		OPCODE(o2_doSentence),
		OPCODE(o2_verbOps),
		OPCODE(o2_getActorWalkBox),
		/* FC */
		OPCODE(o5_isSoundRunning),
		OPCODE(o2_setActorElevation),
		OPCODE(o2_walkActorTo),
		OPCODE(o2_ifState02)
	};

	_opcodesC64 = opcodes;
}

#define SENTENCE_SCRIPT 2

#define PARAM_1 0x80
#define PARAM_2 0x40
#define PARAM_3 0x20

void ScummEngine_c64::executeOpcode(byte i) {
	OpcodeProcC64 op = _opcodesC64[i].proc;
	(this->*op) ();
}

const char *ScummEngine_c64::getOpcodeDesc(byte i) {
	return _opcodesC64[i].desc;
}

void ScummEngine_c64::setStateCommon(byte type) {
	int obj = fetchScriptByte();
	putState(obj, getState(obj) | type);
}

void ScummEngine_c64::clearStateCommon(byte type) {
	int obj = fetchScriptByte();
	putState(obj, getState(obj) & ~type);
}

void ScummEngine_c64::ifStateCommon(byte type) {
	int obj = fetchScriptByte();

	if ((getState(obj) & type) == 0)
		o5_jumpRelative();
	else
		ignoreScriptWord();
}

void ScummEngine_c64::ifNotStateCommon(byte type) {
	int obj = fetchScriptByte();

	if ((getState(obj) & type) != 0)
		o5_jumpRelative();
	else
		ignoreScriptWord();
}

void ScummEngine_c64::o_setState08() {
	int obj = fetchScriptByte();
	putState(obj, getState(obj) | 0x08);
	markObjectRectAsDirty(obj);
	clearDrawObjectQueue();
}

void ScummEngine_c64::o_clearState08() {
	int obj = fetchScriptByte();
	putState(obj, getState(obj) & ~0x08);
	markObjectRectAsDirty(obj);
	clearDrawObjectQueue();
}

void ScummEngine_c64::o_stopScript() {
	int script;

	if (_opcode == 0x05) {
		script = 0;
	} else {
		script = getVarOrDirectByte(PARAM_1);
	}

	if (script == 0)
		script = vm.slot[_currentScript].number;

	if (_currentScript != 0 && vm.slot[_currentScript].number == script)
		stopObjectCode();
	else
		stopScript(script);
}

void ScummEngine_c64::o_loadSound() {
	int resid = fetchScriptByte();
	ensureResourceLoaded(rtSound, resid);
}

void ScummEngine_c64::o_move() {
	getResultPos();
	setResult(getVarOrDirectByte(PARAM_1));
}

void ScummEngine_c64::o_unknown13() {
	debug(0, "o_unknown13 (Actor %d)", fetchScriptByte());
}

void ScummEngine_c64::o_loadActor() {
	debug(0, "o_loadActor (%d)", getVarOrDirectByte(PARAM_1));
}

void ScummEngine_c64::o_isEqual() {
	int16 a, b;
	int var;

	var = fetchScriptByte();
	a = readVar(var);
	b = getVarOrDirectByte(PARAM_1);

	printf("o_isEqual: a %d b %d\n", a , b);

	if (b == a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void ScummEngine_c64::o_isNotEqual() {
	int16 a = getVar();
	int16 b = getVarOrDirectByte(PARAM_1);
	if (b != a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void ScummEngine_c64::o_isGreater() {
	uint16 a = getVar();
	uint16 b = getVarOrDirectByte(PARAM_1);
	if (b > a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void ScummEngine_c64::o_isGreaterEqual() {
	uint16 a = getVar();
	uint16 b = getVarOrDirectByte(PARAM_1);
	if (b >= a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void ScummEngine_c64::o_isLess() {
	uint16 a = getVar();
	uint16 b = getVarOrDirectByte(PARAM_1);
	if (b < a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void ScummEngine_c64::o_isLessEqual() {
	uint16 a = getVar();
	uint16 b = getVarOrDirectByte(PARAM_1);
	if (b <= a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void ScummEngine_c64::o_loadRoom() {
	int resid = getVarOrDirectByte(PARAM_1);
	ensureResourceLoaded(rtRoom, resid);
}

void ScummEngine_c64::o_unknown4D() {
	debug(0, "o_unknown4D (Actor %d)", fetchScriptByte());
}

void ScummEngine_c64::o_loadScript() {
	int resid = getVarOrDirectByte(PARAM_1);
	ensureResourceLoaded(rtScript, resid);
}

void ScummEngine_c64::o_unknown53() {
	debug(0, "o_unknown53 (%d)", fetchScriptByte());
}

void ScummEngine_c64::o_cursorCommand() {
	// TODO

	byte state = fetchScriptByte();
	debug(0, "o_cursorCommand (%d)", state);

	if (state >= 1) {
		_userPut = 1;
		_cursor.state = 1;
	} else {
		_userPut = 0;
		_cursor.state = 0;
	}
}

void ScummEngine_c64::o_lights() {
	int a;

	a = getVarOrDirectByte(PARAM_1);
	// Convert older light mode values into
	// equivalent values.of later games
	// 0 Darkness
	// 1 Flashlight
	// 2 Lighted area
	if (a == 2)
		VAR(VAR_CURRENT_LIGHTS) = 11;
	else if (a == 1)
		VAR(VAR_CURRENT_LIGHTS) = 4;
	else
		VAR(VAR_CURRENT_LIGHTS) = 0;

	_fullRedraw = 1;
}

void ScummEngine_c64::o_getObjectOwner() {
	getResultPos();
	setResult(getOwner(getVarOrDirectByte(PARAM_1)));
}

void ScummEngine_c64::o_add() {
	int a;
	getResultPos();
	a = getVarOrDirectByte(PARAM_1);
	setResult(readVar(_resultVarNumber) + a);
}

void ScummEngine_c64::o_subtract() {
	int a;
	getResultPos();
	a = getVarOrDirectByte(PARAM_1);
	_scummVars[_resultVarNumber] -= a;
}

void ScummEngine_c64::o_increment() {
	int var = getVarOrDirectByte(PARAM_1);
	setResult(readVar(var) + 1);
}

void ScummEngine_c64::o_decrement() {
	int var = getVarOrDirectByte(PARAM_1);
	setResult(readVar(var) - 1);
}

void ScummEngine_c64::o_unknown93() {
	debug(0, "o_unknown93 (Actor %d)", fetchScriptByte());
}

void ScummEngine_c64::o_freezeScript() {
	int scr = fetchScriptByte();
	vm.slot[scr].status &= 0x80;
	vm.slot[scr].freezeCount = 1;
}

void ScummEngine_c64::o_unfreezeScript() {
	int scr = fetchScriptByte();
	vm.slot[scr].status &= 0x7F;
	vm.slot[scr].freezeCount = 0;
}

void ScummEngine_c64::o_getDist() {
	int o1, o2;
	int r;
	getResultPos();
	o1 = getVarOrDirectByte(PARAM_1);
	o2 = getVarOrDirectByte(PARAM_2);
	r = getObjActToObjActDist(o1, o2);
	setResult(r);
}

void ScummEngine_c64::o_pickupObject() {
	int obj = fetchScriptByte();

	if (obj < 1) {
		error("pickupObject received invalid index %d (script %d)", obj, vm.slot[_currentScript].number);
	}

	if (getObjectIndex(obj) == -1)
		return;

	if (whereIsObject(obj) == WIO_INVENTORY)	/* Don't take an */
		return;											/* object twice */

	addObjectToInventory(obj, _roomResource);
	markObjectRectAsDirty(obj);
	putOwner(obj, VAR(VAR_EGO));
	putState(obj, getState(obj) | 0xA);
	clearDrawObjectQueue();

	runInventoryScript(1);
}

void ScummEngine_c64::o_animateActor() {
	int act = getVarOrDirectByte(PARAM_1);
	int anim = getVarOrDirectByte(PARAM_2);
	int unk = fetchScriptByte();
	debug(0,"o_animateActor: unk %d", unk);

	Actor *a = derefActor(act, "o_animateActor");
	a->animateActor(anim);
}

void ScummEngine_c64::o_putActorAtObject() {
	int obj, x, y;
	Actor *a;

	a = derefActor(getVarOrDirectByte(PARAM_1), "o_putActorAtObject");

	obj = fetchScriptByte();
	if (whereIsObject(obj) != WIO_NOT_FOUND)
		getObjectXYPos(obj, x, y);
	else {
		x = 240;
		y = 120;
	}

	a->putActor(x, y, a->_room);
}

void ScummEngine_c64::o_walkActorToObject() {
	int obj;
	Actor *a;

	a = derefActor(getVarOrDirectByte(PARAM_1), "o_walkActorToObject");
	obj = fetchScriptByte();
	if (whereIsObject(obj) != WIO_NOT_FOUND) {
		int x, y, dir;
		getObjectXYPos(obj, x, y, dir);
		a->startWalkActor(x, y, dir);
	}
}

#undef PARAM_1
#undef PARAM_2
#undef PARAM_3

} // End of namespace Scumm
