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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
		OPCODE(o_doSentence),
		/* 04 */
		OPCODE(o_isGreaterEqual),
		OPCODE(o_stopCurrentScript),
		OPCODE(o5_getDist),
		OPCODE(o5_getActorRoom),
		/* 08 */
		OPCODE(o_isNotEqual),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_setActorBitVar),
		/* 0C */
		OPCODE(o_loadSound),
		OPCODE(o_printEgo_c64),
		OPCODE(o_putActorAtObject),
		OPCODE(o2_clearState02),
		/* 10 */
		OPCODE(o5_breakHere),
		OPCODE(o_animateActor),
		OPCODE(o2_panCameraTo),
		OPCODE(o_lockActor),
		/* 14 */
		OPCODE(o_print_c64),
		OPCODE(o2_actorFromPos),
		OPCODE(o5_getRandomNr),
		OPCODE(o_clearState08),
		/* 18 */
		OPCODE(o_jumpRelative),
		OPCODE(o_stopCurrentScript),
		OPCODE(o5_move),
		OPCODE(o_getActorBitVar),
		/* 1C */
		OPCODE(o5_startSound),
		OPCODE(o2_setBitVar),
		OPCODE(o2_walkActorTo),
		OPCODE(o2_ifState04),
		/* 20 */
		OPCODE(o5_stopMusic),
		OPCODE(o2_putActor),
		OPCODE(o5_saveLoadGame),
		OPCODE(o_stopCurrentScript),
		/* 24 */
		OPCODE(o_unknown2),
		OPCODE(o5_loadRoom),
		OPCODE(o_getClosestObjActor),
		OPCODE(o2_getActorY),
		/* 28 */
		OPCODE(o_equalZero),
		OPCODE(o2_setOwnerOf),
		OPCODE(o2_delay),
		OPCODE(o_setActorBitVar),
		/* 2C */
		OPCODE(o_stopCurrentScript),
		OPCODE(o5_putActorInRoom),
		OPCODE(o_print_c64),
		OPCODE(o2_ifState08),
		/* 30 */
		OPCODE(o_loadActor),
		OPCODE(o2_getBitVar),
		OPCODE(o2_setCameraAt),
		OPCODE(o_lockScript),
		/* 34 */
		OPCODE(o5_getDist),
		OPCODE(o_stopCurrentScript),
		OPCODE(o2_walkActorToObject),
		OPCODE(o2_clearState04),
		/* 38 */
		OPCODE(o_isLessEqual),
		OPCODE(o_stopCurrentScript),
		OPCODE(o2_subtract),
		OPCODE(o_stopCurrentScript),
		/* 3C */
		OPCODE(o5_stopSound),
		OPCODE(o2_setBitVar),
		OPCODE(o2_walkActorTo),
		OPCODE(o2_ifState02),
		/* 40 */
		OPCODE(o2_cutscene),
		OPCODE(o2_putActor),
		OPCODE(o2_startScript),
		OPCODE(o_doSentence),
		/* 44 */
		OPCODE(o_isLess),
		OPCODE(o_stopCurrentScript),
		OPCODE(o5_increment),
		OPCODE(o2_getActorX),
		/* 48 */
		OPCODE(o_isEqual),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_loadRoom),
		OPCODE(o_setActorBitVar),
		/* 4C */
		OPCODE(o_loadScript),
		OPCODE(o_lockRoom),
		OPCODE(o_putActorAtObject),
		OPCODE(o2_clearState02),
		/* 50 */
		OPCODE(o_nop),
		OPCODE(o_animateActor),
		OPCODE(o5_actorFollowCamera),
		OPCODE(o_lockSound),
		/* 54 */
		OPCODE(o5_setObjectName),
		OPCODE(o2_actorFromPos),
		OPCODE(o_getActorMoving),
		OPCODE(o_clearState08),
		/* 58 */
		OPCODE(o_beginOverride),
		OPCODE(o_stopCurrentScript),
		OPCODE(o2_add),
		OPCODE(o_getActorBitVar),
		/* 5C */
		OPCODE(o5_startSound),
		OPCODE(o2_setBitVar),
		OPCODE(o2_walkActorTo),
		OPCODE(o2_ifState04),
		/* 60 */
		OPCODE(o_cursorCommand),
		OPCODE(o2_putActor),
		OPCODE(o2_stopScript),
		OPCODE(o_stopCurrentScript),
		/* 64 */
		OPCODE(o_unknown3),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_getClosestObjActor),
		OPCODE(o5_getActorFacing),
		/* 68 */
		OPCODE(o5_isScriptRunning),
		OPCODE(o2_setOwnerOf),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_setActorBitVar),
		/* 6C */
		OPCODE(o_stopCurrentScript),
		OPCODE(o5_putActorInRoom),
		OPCODE(o2_dummy),
		OPCODE(o2_ifState08),
		/* 70 */
		OPCODE(o_lights),
		OPCODE(o2_getBitVar),
		OPCODE(o_nop),
		OPCODE(o5_getObjectOwner),
		/* 74 */
		OPCODE(o5_getDist),
		OPCODE(o_printEgo_c64),
		OPCODE(o2_walkActorToObject),
		OPCODE(o2_clearState04),
		/* 78 */
		OPCODE(o_isGreater),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_stopCurrentScript),
		/* 7C */
		OPCODE(o5_isSoundRunning),
		OPCODE(o2_setBitVar),
		OPCODE(o2_walkActorTo),
		OPCODE(o2_ifNotState02),
		/* 80 */
		OPCODE(o_stopCurrentScript),
		OPCODE(o2_putActor),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_doSentence),
		/* 84 */
		OPCODE(o_isGreaterEqual),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_badOpcode),
		OPCODE(o5_getActorRoom),
		/* 88 */
		OPCODE(o_isNotEqual),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_setActorBitVar),
		/* 8C */
		OPCODE(o_loadSound),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_putActorAtObject),
		OPCODE(o2_setState02),
		/* 90 */
		OPCODE(o2_pickupObject),
		OPCODE(o_animateActor),
		OPCODE(o2_panCameraTo),
		OPCODE(o_unlockActor),
		/* 94 */
		OPCODE(o5_print),
		OPCODE(o2_actorFromPos),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_setState08),
		/* 98 */
		OPCODE(o2_restart),
		OPCODE(o_stopCurrentScript),
		OPCODE(o5_move),
		OPCODE(o_getActorBitVar),
		/* 9C */
		OPCODE(o5_startSound),
		OPCODE(o2_setBitVar),
		OPCODE(o2_walkActorTo),
		OPCODE(o2_ifNotState04),
		/* A0 */
		OPCODE(o5_stopObjectCode),
		OPCODE(o2_putActor),
		OPCODE(o5_saveLoadGame),
		OPCODE(o_stopCurrentScript),
		/* A4 */
		OPCODE(o_unknown2),
		OPCODE(o5_loadRoom),
		OPCODE(o_stopCurrentScript),
		OPCODE(o2_getActorY),
		/* A8 */
		OPCODE(o_notEqualZero),
		OPCODE(o2_setOwnerOf),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_setActorBitVar),
		/* AC */
		OPCODE(o_stopCurrentScript),
		OPCODE(o5_putActorInRoom),
		OPCODE(o_print_c64),
		OPCODE(o2_ifNotState08),
		/* B0 */
		OPCODE(o_loadActor),
		OPCODE(o2_getBitVar),
		OPCODE(o2_setCameraAt),
		OPCODE(o_unlockScript),
		/* B4 */
		OPCODE(o5_getDist),
		OPCODE(o_stopCurrentScript),
		OPCODE(o2_walkActorToObject),
		OPCODE(o2_setState04),
		/* B8 */
		OPCODE(o_isLessEqual),
		OPCODE(o_stopCurrentScript),
		OPCODE(o2_subtract),
		OPCODE(o_stopCurrentScript),
		/* BC */
		OPCODE(o5_stopSound),
		OPCODE(o2_setBitVar),
		OPCODE(o2_walkActorTo),
		OPCODE(o2_ifNotState02),
		/* C0 */
		OPCODE(o2_endCutscene),
		OPCODE(o2_putActor),
		OPCODE(o2_startScript),
		OPCODE(o_doSentence),
		/* C4 */
		OPCODE(o_isLess),
		OPCODE(o_stopCurrentScript),
		OPCODE(o5_decrement),
		OPCODE(o2_getActorX),
		/* C8 */
		OPCODE(o_isEqual),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_loadRoom),
		OPCODE(o_setActorBitVar),
		/* CC */
		OPCODE(o_loadScript),
		OPCODE(o_unlockRoom),
		OPCODE(o_putActorAtObject),
		OPCODE(o2_setState02),
		/* D0 */
		OPCODE(o_nop),
		OPCODE(o_animateActor),
		OPCODE(o5_actorFollowCamera),
		OPCODE(o_unlockSound),
		/* D4 */
		OPCODE(o5_setObjectName),
		OPCODE(o2_actorFromPos),
		OPCODE(o_getActorMoving),
		OPCODE(o_setState08),
		/* D8 */
		OPCODE(o_stopCurrentScript),
		OPCODE(o_stopCurrentScript),
		OPCODE(o2_add),
		OPCODE(o_getActorBitVar),
		/* DC */
		OPCODE(o5_startSound),
		OPCODE(o2_setBitVar),
		OPCODE(o2_walkActorTo),
		OPCODE(o2_ifNotState04),
		/* E0 */
		OPCODE(o_cursorCommand),
		OPCODE(o2_putActor),
		OPCODE(o2_stopScript),
		OPCODE(o_stopCurrentScript),
		/* E4 */
		OPCODE(o_unknown3),
		OPCODE(o_loadRoomWithEgo),
		OPCODE(o_stopCurrentScript),
		OPCODE(o5_getActorFacing),
		/* E8 */
		OPCODE(o5_isScriptRunning),
		OPCODE(o2_setOwnerOf),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_setActorBitVar),
		/* EC */
		OPCODE(o_stopCurrentScript),
		OPCODE(o5_putActorInRoom),
		OPCODE(o2_dummy),
		OPCODE(o2_ifNotState08),
		/* F0 */
		OPCODE(o_lights),
		OPCODE(o2_getBitVar),
		OPCODE(o_nop),
		OPCODE(o5_getObjectOwner),
		/* F4 */
		OPCODE(o5_getDist),
		OPCODE(o_stopCurrentScript),
		OPCODE(o2_walkActorToObject),
		OPCODE(o2_setState04),
		/* F8 */
		OPCODE(o_isGreater),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_stopCurrentScript),
		OPCODE(o_stopCurrentScript),
		/* FC */
		OPCODE(o5_isSoundRunning),
		OPCODE(o2_setBitVar),
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

int ScummEngine_c64::getVarOrDirectWord(byte mask) {
	return getVarOrDirectByte(mask);
}

uint ScummEngine_c64::fetchScriptWord() {
	return fetchScriptByte();
}

const char *ScummEngine_c64::getOpcodeDesc(byte i) {
	return _opcodesC64[i].desc;
}

int ScummEngine_c64::getObjectFlag() {
	if (_opcode & 0x40)
		return _activeObject;
	return fetchScriptByte();
}

void ScummEngine_c64::decodeParseString() {
	byte buffer[512];
	byte *ptr = buffer;
	byte c;
	bool insertSpace = false;

	while ((c = fetchScriptByte())) {

		insertSpace = (c & 0x80) != 0;
		c &= 0x7f;

		if (c == '/') {
			*ptr++ = 13;
		} else {
			*ptr++ = c;
		}

		if (insertSpace)
			*ptr++ = ' ';

	}
	*ptr = 0;

	int textSlot = 0;
	_string[textSlot].xpos = 0;
	_string[textSlot].ypos = 0;
	_string[textSlot].right = 320;
	_string[textSlot].center = false;
	_string[textSlot].overhead = false;

	if (_actorToPrintStrFor == 0xFF)
		_string[textSlot].color = 14;

	actorTalk(buffer);
}

void ScummEngine_c64::setStateCommon(byte type) {
	int obj = getObjectFlag();
	putState(obj, getState(obj) | type);
}

void ScummEngine_c64::clearStateCommon(byte type) {
	int obj = getObjectFlag();
	putState(obj, getState(obj) & ~type);
}

void ScummEngine_c64::ifStateCommon(byte type) {
	int obj = getObjectFlag();

	if ((getState(obj) & type) == 0) {
		o_jumpRelative();
	} else {
		fetchScriptByte();
		fetchScriptByte();
	}
}

void ScummEngine_c64::ifNotStateCommon(byte type) {
	int obj = getObjectFlag();

	if ((getState(obj) & type) != 0) {
		o_jumpRelative();
	} else {
		fetchScriptByte();
		fetchScriptByte();
	}
}

void ScummEngine_c64::o_setState08() {
	int obj = getObjectFlag();
	putState(obj, getState(obj) | 0x08);
	markObjectRectAsDirty(obj);
	clearDrawObjectQueue();
}

void ScummEngine_c64::o_clearState08() {
	int obj = getObjectFlag();
	putState(obj, getState(obj) & ~0x08);
	markObjectRectAsDirty(obj);
	clearDrawObjectQueue();
}

void ScummEngine_c64::o_stopCurrentScript() {
	int script;

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

void ScummEngine_c64::o_lockSound() {
	int resid = fetchScriptByte();
	res.lock(rtSound, resid);
	debug(0, "o_lockSound (%d)", resid);
}

void ScummEngine_c64::o_unlockSound() {
	int resid = fetchScriptByte();
	res.unlock(rtSound, resid);
	debug(0, "o_unlockSound (%d)", resid);
}

void ScummEngine_c64::o_loadActor() {
	debug(0, "o_loadActor (%d)", getVarOrDirectByte(PARAM_1));
}

void ScummEngine_c64::o_lockActor() {
	debug(0, "o_lockActor (%d)", fetchScriptByte());
}

void ScummEngine_c64::o_unlockActor() {
	debug(0, "o_unlockActor (%d)", fetchScriptByte());
}

void ScummEngine_c64::o_loadScript() {
	int resid = getVarOrDirectByte(PARAM_1);
	ensureResourceLoaded(rtScript, resid);
}

void ScummEngine_c64::o_lockScript() {
	int resid = fetchScriptByte();
	res.lock(rtScript, resid);
	debug(0, "o_lockScript (%d)", resid);
}

void ScummEngine_c64::o_unlockScript() {
	int resid = fetchScriptByte();
	res.unlock(rtScript, resid);
	debug(0, "o_unlockScript (%d)", resid);
}

void ScummEngine_c64::o_loadRoom() {
	int resid = getVarOrDirectByte(PARAM_1);
	ensureResourceLoaded(rtRoom, resid);
}

void ScummEngine_c64::o_loadRoomWithEgo() {
	Actor *a;
	int obj, room, x, y, dir;

	obj = fetchScriptByte();
	room = fetchScriptByte();

	a = derefActor(VAR(VAR_EGO), "o_loadRoomWithEgo");

	a->putActor(0, 0, room);
	_egoPositioned = false;

	startScene(a->_room, a, obj);

	getObjectXYPos(obj, x, y, dir);
	a->putActor(x, y, _currentRoom);
	a->setDirection(dir + 180);

	camera._dest.x = camera._cur.x = a->_pos.x;
	setCameraAt(a->_pos.x, a->_pos.y);
	setCameraFollows(a);

	_fullRedraw = true;

	resetSentence();

	if (x >= 0 && y >= 0) {
		a->startWalkActor(x, y, -1);
	}
}

void ScummEngine_c64::o_lockRoom() {
	int resid = fetchScriptByte();
	res.lock(rtRoom, resid);
	debug(0, "o_lockRoom (%d)", resid);
}

void ScummEngine_c64::o_unlockRoom() {
	int resid = fetchScriptByte();
	res.unlock(rtRoom, resid);
	debug(0, "o_unlockRoom (%d)", resid);
}

void ScummEngine_c64::o_cursorCommand() {
	// TODO
	int state = 0;

	_currentMode = fetchScriptByte();
	switch (_currentMode) {
	case 0:
		state = 15;
		break;
	case 1:
		state = 31;
		break;
	case 2:
		break;
	case 3:
		state = 247;
		break;
	}

	setUserState(state);
	debug(0, "o_cursorCommand(%d)", _currentMode);
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

	_fullRedraw = true;
}

void ScummEngine_c64::o_animateActor() {
	int act = getVarOrDirectByte(PARAM_1);
	int anim = getVarOrDirectByte(PARAM_2);
	int unk = fetchScriptByte();
	debug(0,"o_animateActor: unk %d", unk);

	Actor *a = derefActor(act, "o_animateActor");
	a->animateActor(anim);
}

void ScummEngine_c64::o_getActorMoving() {
	getResultPos();
	int act = getVarOrDirectByte(PARAM_1);
	Actor *a = derefActor(act, "o_getActorMoving");
	if (a->_moving)
		setResult(1);
	else
		setResult(2);
}

void ScummEngine_c64::o_putActorAtObject() {
	int obj, x, y;
	Actor *a;

	a = derefActor(getVarOrDirectByte(PARAM_1), "o_putActorAtObject");

	obj = fetchScriptByte();
	if (whereIsObject(obj) != WIO_NOT_FOUND)
		getObjectXYPos(obj, x, y);
	else {
		x = 30;
		y = 60;
	}

	a->putActor(x, y, a->_room);
}

void ScummEngine_c64::o_badOpcode() {
	warning("Bad opcode 0x86 encountered");
}

void ScummEngine_c64::o_nop() {
}

void ScummEngine_c64::o_setActorBitVar() {
	byte flag = getVarOrDirectByte(PARAM_1);
	byte mask = getVarOrDirectByte(PARAM_2);
	byte mod = getVarOrDirectByte(PARAM_3);

	//if (mod)
	//	_miscFlags[flag] |= mask;
	//else
	//	_miscFlags[flag] &= ~mash;

	warning("STUB: o_setActorBitVar(%d, %d, %d)", flag, mask, mod);
}

void ScummEngine_c64::o_getActorBitVar() {
	getResultPos();
	byte flag = getVarOrDirectByte(PARAM_1);
	byte mask = getVarOrDirectByte(PARAM_2);

	//setResult((_miscFlags[flag] & mask) ? 1 : 0);

	setResult(0);
	warning("STUB: o_getActorBitVar(%d, %d)", flag, mask);
}

void ScummEngine_c64::o_print_c64() {
	_actorToPrintStrFor = fetchScriptByte();
	decodeParseString();
}

void ScummEngine_c64::o_printEgo_c64() {
	_actorToPrintStrFor = (byte)VAR(VAR_EGO);
	decodeParseString();
}

void ScummEngine_c64::o_doSentence() {
	byte var1 = fetchScriptByte();
	byte var2 = fetchScriptByte();
	byte var3 = fetchScriptByte();
	warning("STUB: o_doSentence(%d, %d, %d)", var1, var2, var3);
}

void ScummEngine_c64::o_unknown2() {
	byte var1 = fetchScriptByte();
	warning("STUB: o_unknown2(%d)", var1);
}

void ScummEngine_c64::o_unknown3() {
	byte var1 = fetchScriptByte();
	warning("STUB: o_unknown3(%d)", var1);
}

void ScummEngine_c64::o_getClosestObjActor() {
	int obj;
	int act;
	int dist;

	// This code can't detect any actors farther away than 255 units
	// (pixels in newer games, characters in older ones.) But this is
	// perfectly OK, as it is exactly how the original behaved.

	int closest_obj = 0xFF, closest_dist = 0xFF;

	getResultPos();

	act = getVarOrDirectByte(PARAM_1);
	obj = (_opcode & 0x40) ? 25 : 7;

	do {
		dist = getObjActToObjActDist(act, obj);
		if (dist < closest_dist) {
			closest_dist = dist;
			closest_obj = obj;
		}
	} while (--obj);

	setResult(closest_obj);
}

void ScummEngine_c64::o_beginOverride() {
	const int idx = vm.cutSceneStackPointer;
	assert(0 <= idx && idx < 5);

	vm.cutScenePtr[idx] = _scriptPointer - _scriptOrgPointer;
	vm.cutSceneScript[idx] = _currentScript;

	// Skip the jump instruction following the override instruction
	// (the jump is responsible for "skipping" cutscenes, and the reason
	// why we record the current script position in vm.cutScenePtr).
	fetchScriptByte();
	ScummEngine::fetchScriptWord();

	// This is based on disassembly
	VAR(VAR_OVERRIDE) = 0;
}

void ScummEngine_c64::o_isEqual() {
	int16 a, b;
	int var;

	var = fetchScriptByte();
	a = readVar(var);
	b = getVarOrDirectByte(PARAM_1);

	if (b == a)
		ScummEngine::fetchScriptWord();
	else
		o_jumpRelative();

}

void ScummEngine_c64::o_isGreater() {
	int16 a = getVar();
	int16 b = getVarOrDirectByte(PARAM_1);
	if (b > a)
		ScummEngine::fetchScriptWord();
	else
		o_jumpRelative();
}

void ScummEngine_c64::o_isGreaterEqual() {
	int16 a = getVar();
	int16 b = getVarOrDirectByte(PARAM_1);
	if (b >= a)
		ScummEngine::fetchScriptWord();
	else
		o_jumpRelative();
}

void ScummEngine_c64::o_isLess() {
	int16 a = getVar();
	int16 b = getVarOrDirectByte(PARAM_1);
	if (b < a)
		ScummEngine::fetchScriptWord();
	else
		o_jumpRelative();
}

void ScummEngine_c64::o_isLessEqual() {
	int16 a = getVar();
	int16 b = getVarOrDirectByte(PARAM_1);

	if (b <= a)
		ScummEngine::fetchScriptWord();
	else
		o_jumpRelative();
}

void ScummEngine_c64::o_isNotEqual() {
	int16 a = getVar();
	int16 b = getVarOrDirectByte(PARAM_1);
	if (b != a)
		ScummEngine::fetchScriptWord();
	else
		o_jumpRelative();
}

void ScummEngine_c64::o_notEqualZero() {
	int a = getVar();
	if (a != 0)
		ScummEngine::fetchScriptWord();
	else
		o_jumpRelative();
}

void ScummEngine_c64::o_equalZero() {
	int a = getVar();
	if (a == 0)
		ScummEngine::fetchScriptWord();
	else
		o_jumpRelative();
}

void ScummEngine_c64::o_jumpRelative() {
	int16 offset = (int16)ScummEngine::fetchScriptWord();
	_scriptPointer += offset;
}



#undef PARAM_1
#undef PARAM_2
#undef PARAM_3

} // End of namespace Scumm
