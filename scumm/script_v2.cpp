/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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

#include "stdafx.h"
#include "scumm.h"
#include "actor.h"
#include "charset.h"
#include "intern.h"
#include "sound.h"
#include "verbs.h"

#define OPCODE(x)	{ &Scumm_v2::x, #x }

void Scumm_v2::setupOpcodes() {
	static const OpcodeEntryV2 opcodes[256] = {
		/* 00 */
		OPCODE(o5_stopObjectCode),
		OPCODE(o5_putActor),
		OPCODE(o5_startMusic),
		OPCODE(o5_getActorRoom),
		/* 04 */
		OPCODE(o5_isGreaterEqual),
		OPCODE(o5_drawObject),
		OPCODE(o5_getActorElevation),
		OPCODE(o2_setState80),
		/* 08 */
		OPCODE(o5_isNotEqual),
		OPCODE(o5_faceActor),
		OPCODE(o2_assignVarWordDirect),
		OPCODE(o2_setObjY),
		/* 0C */
		OPCODE(o5_resourceRoutines),
		OPCODE(o5_walkActorToActor),
		OPCODE(o5_putActorAtObject),
		OPCODE(o5_getObjectState),
		/* 10 */
		OPCODE(o5_getObjectOwner),
		OPCODE(o5_animateActor),
		OPCODE(o5_panCameraTo),
		OPCODE(o5_actorSet),
		/* 14 */
		OPCODE(o5_print),
		OPCODE(o5_actorFromPos),
		OPCODE(o5_getRandomNr),
		OPCODE(o2_setState40),
		/* 18 */
		OPCODE(o5_jumpRelative),
		OPCODE(o5_doSentence),
		OPCODE(o5_move),
		OPCODE(o2_setBitVar),
		/* 1C */
		OPCODE(o5_startSound),
		OPCODE(o5_ifClassOfIs),
		OPCODE(o5_walkActorTo),
		OPCODE(o2_ifState20),
		/* 20 */
		OPCODE(o5_stopMusic),
		OPCODE(o5_putActor),
		OPCODE(o5_getAnimCounter),
		OPCODE(o5_getActorY),
		/* 24 */
		OPCODE(o5_loadRoomWithEgo),
		OPCODE(o5_pickupObject),
		OPCODE(o5_setVarRange),
		OPCODE(o2_ifNotState40),
		/* 28 */
		OPCODE(o5_equalZero),
		OPCODE(o5_setOwnerOf),
		OPCODE(o2_addDirect),
		OPCODE(o5_delayVariable),
		/* 2C */
		OPCODE(o2_ifNotState40),
		OPCODE(o5_putActorInRoom),
		OPCODE(o5_delay),
		OPCODE(o2_assignVarByte),
		/* 30 */
		OPCODE(o5_matrixOps),
		OPCODE(o2_getBitVar),
		OPCODE(o5_setCameraAt),
		OPCODE(o5_roomOps),
		/* 34 */
		OPCODE(o5_getDist),
		OPCODE(o5_findObject),
		OPCODE(o5_walkActorToObject),
		OPCODE(o2_setState10),
		/* 38 */
		OPCODE(o5_lessOrEqual),
		OPCODE(o5_doSentence),
		OPCODE(o5_subtract),
		OPCODE(o2_waitForActor),
		/* 3C */
		OPCODE(o5_stopSound),
		OPCODE(o5_getActorElevation),
		OPCODE(o5_walkActorTo),
		OPCODE(o2_ifNotState10),
		/* 40 */
		OPCODE(o5_cutscene),
		OPCODE(o5_putActor),
		OPCODE(o5_startScript),
		OPCODE(o5_getActorX),
		/* 44 */
		OPCODE(o5_isLess),
		OPCODE(o5_drawObject),
		OPCODE(o5_increment),
		OPCODE(o2_setState80),
		/* 48 */
		OPCODE(o5_isEqual),
		OPCODE(o5_faceActor),
		OPCODE(o5_chainScript),
		OPCODE(o2_setObjY),
		/* 4C */
		OPCODE(o2_waitForSentence),
		OPCODE(o5_walkActorToActor),
		OPCODE(o5_putActorAtObject),
		OPCODE(o2_ifState80),
		/* 50 */
		OPCODE(o5_pickupObjectOld),
		OPCODE(o5_animateActor),
		OPCODE(o5_actorFollowCamera),
		OPCODE(o5_actorSet),
		/* 54 */
		OPCODE(o5_setObjectName),
		OPCODE(o5_actorFromPos),
		OPCODE(o5_getActorMoving),
		OPCODE(o2_setState20),
		/* 58 */
		OPCODE(o5_overRide),
		OPCODE(o5_doSentence),
		OPCODE(o5_add),
		OPCODE(o2_setBitVar),
		/* 5C */
		OPCODE(o5_oldRoomEffect),
		OPCODE(o5_ifClassOfIs),
		OPCODE(o5_walkActorTo),
		OPCODE(o2_ifNotState20),
		/* 60 */
		OPCODE(o5_cursorCommand),
		OPCODE(o5_putActor),
		OPCODE(o5_stopScript),
		OPCODE(o5_getActorFacing),
		/* 64 */
		OPCODE(o5_loadRoomWithEgo),
		OPCODE(o5_pickupObject),
		OPCODE(o5_getClosestObjActor),
		OPCODE(o2_clearState40),
		/* 68 */
		OPCODE(o5_isScriptRunning),
		OPCODE(o5_setOwnerOf),
		OPCODE(o2_subDirect),
		OPCODE(o5_dummy),
		/* 6C */
		OPCODE(o2_getObjY),
		OPCODE(o5_putActorInRoom),
		OPCODE(o5_dummy),
		OPCODE(o2_ifState40),
		/* 70 */
		OPCODE(o5_lights),
		OPCODE(o5_getActorCostume),
		OPCODE(o5_loadRoom),
		OPCODE(o5_roomOps),
		/* 74 */
		OPCODE(o5_getDist),
		OPCODE(o5_findObject),
		OPCODE(o5_walkActorToObject),
		OPCODE(o2_clearState10),
		/* 78 */
		OPCODE(o5_isGreater),
		OPCODE(o5_doSentence),
		OPCODE(o5_verbOps),
		OPCODE(o5_getActorWalkBox),
		/* 7C */
		OPCODE(o5_isSoundRunning),
		OPCODE(o5_getActorElevation),
		OPCODE(o5_walkActorTo),
		OPCODE(o5_drawBox),
		/* 80 */
		OPCODE(o5_breakHere),
		OPCODE(o5_putActor),
		OPCODE(o5_startMusic),
		OPCODE(o5_getActorRoom),
		/* 84 */
		OPCODE(o5_isGreaterEqual),
		OPCODE(o5_drawObject),
		OPCODE(o5_getActorElevation),
		OPCODE(o2_setState80),
		/* 88 */
		OPCODE(o5_isNotEqual),
		OPCODE(o5_faceActor),
		OPCODE(o2_assignVarWordDirect),
		OPCODE(o2_setObjY),
		/* 8C */
		OPCODE(o5_resourceRoutines),
		OPCODE(o5_walkActorToActor),
		OPCODE(o5_putActorAtObject),
		OPCODE(o2_ifNotState80),
		/* 90 */
		OPCODE(o5_getObjectOwner),
		OPCODE(o5_animateActor),
		OPCODE(o5_panCameraTo),
		OPCODE(o5_actorSet),
		/* 94 */
		OPCODE(o5_print),
		OPCODE(o5_actorFromPos),
		OPCODE(o5_getRandomNr),
		OPCODE(o2_setState20),
		/* 98 */
		OPCODE(o2_restart),
		OPCODE(o5_doSentence),
		OPCODE(o2_assignVarWord),
		OPCODE(o2_setBitVar),
		/* 9C */
		OPCODE(o5_startSound),
		OPCODE(o5_ifClassOfIs),
		OPCODE(o5_walkActorTo),
		OPCODE(o2_ifState20),
		/* A0 */
		OPCODE(o5_stopObjectCode),
		OPCODE(o5_putActor),
		OPCODE(o5_getAnimCounter),
		OPCODE(o5_getActorY),
		/* A4 */
		OPCODE(o5_loadRoomWithEgo),
		OPCODE(o5_pickupObject),
		OPCODE(o5_setVarRange),
		OPCODE(o2_setState40),
		/* A8 */
		OPCODE(o5_notEqualZero),
		OPCODE(o5_setOwnerOf),
		OPCODE(o2_addDirect),
		OPCODE(o5_dummy),
		/* AC */
		OPCODE(o5_delay),
		OPCODE(o5_putActorInRoom),
		OPCODE(o2_waitForSentence),
		OPCODE(o2_ifNotState40),
		/* B0 */
		OPCODE(o5_matrixOps),
		OPCODE(o2_getBitVar),
		OPCODE(o5_setCameraAt),
		OPCODE(o5_roomOps),
		/* B4 */
		OPCODE(o5_getDist),
		OPCODE(o5_findObject),
		OPCODE(o5_walkActorToObject),
		OPCODE(o2_setState20),
		/* B8 */
		OPCODE(o5_lessOrEqual),
		OPCODE(o5_doSentence),
		OPCODE(o5_subtract),
		OPCODE(o2_waitForActor),
		/* BC */
		OPCODE(o5_stopSound),
		OPCODE(o5_getActorElevation),
		OPCODE(o5_walkActorTo),
		OPCODE(o2_ifNotState10),
		/* C0 */
		OPCODE(o5_endCutscene),
		OPCODE(o5_putActor),
		OPCODE(o5_startScript),
		OPCODE(o5_getActorX),
		/* C4 */
		OPCODE(o5_isLess),
		OPCODE(o5_drawObject),
		OPCODE(o5_decrement),
		OPCODE(o2_clearState80),
		/* C8 */
		OPCODE(o5_isEqual),
		OPCODE(o5_faceActor),
		OPCODE(o5_chainScript),
		OPCODE(o2_setObjY),
		/* CC */
		OPCODE(o5_pseudoRoom),
		OPCODE(o5_walkActorToActor),
		OPCODE(o5_putActorAtObject),
		OPCODE(o2_ifState80),
		/* D0 */
		OPCODE(o5_pickupObjectOld),
		OPCODE(o5_animateActor),
		OPCODE(o5_actorFollowCamera),
		OPCODE(o5_actorSet),
		/* D4 */
		OPCODE(o5_setObjectName),
		OPCODE(o5_actorFromPos),
		OPCODE(o5_getActorMoving),
		OPCODE(o2_setState20),
		/* D8 */
		OPCODE(o5_printEgo),
		OPCODE(o5_doSentence),
		OPCODE(o5_add),
		OPCODE(o5_divide),
		/* DC */
		OPCODE(o5_oldRoomEffect),
		OPCODE(o5_ifClassOfIs),
		OPCODE(o5_walkActorTo),
		OPCODE(o2_ifNotState20),
		/* E0 */
		OPCODE(o5_cursorCommand),
		OPCODE(o5_putActor),
		OPCODE(o5_stopScript),
		OPCODE(o5_getActorFacing),
		/* E4 */
		OPCODE(o5_loadRoomWithEgo),
		OPCODE(o5_pickupObject),
		OPCODE(o5_getClosestObjActor),
		OPCODE(o2_clearState40),
		/* E8 */
		OPCODE(o5_isScriptRunning),
		OPCODE(o5_setOwnerOf),
		OPCODE(o2_subDirect),
		OPCODE(o5_dummy),
		/* EC */
		OPCODE(o2_getObjY),
		OPCODE(o5_putActorInRoom),
		OPCODE(o5_dummy),
		OPCODE(o2_ifState40),
		/* F0 */
		OPCODE(o5_lights),
		OPCODE(o5_getActorCostume),
		OPCODE(o5_loadRoom),
		OPCODE(o5_roomOps),
		/* F4 */
		OPCODE(o5_getDist),
		OPCODE(o5_findObject),
		OPCODE(o5_walkActorToObject),
		OPCODE(o2_clearState10),
		/* F8 */
		OPCODE(o5_isGreater),
		OPCODE(o5_doSentence),
		OPCODE(o5_verbOps),
		OPCODE(o5_getActorWalkBox),
		/* FC */
		OPCODE(o5_isSoundRunning),
		OPCODE(o5_getActorElevation),
		OPCODE(o5_walkActorTo),
		OPCODE(o2_ifState10)
	};

	_opcodesV2 = opcodes;
}

void Scumm_v2::executeOpcode(int i) {
	OpcodeProcV2 op = _opcodesV2[i].proc;
	(this->*op) ();
}

const char *Scumm_v2::getOpcodeDesc(int i) {
	return _opcodesV2[i].desc;
}

void Scumm_v2::o2_setState80() {
	int obj = getVarOrDirectWord(0x80);
	putState(obj, getState(obj) | 0x80);
	removeObjectFromRoom(obj);
	clearDrawObjectQueue();
}

void Scumm_v2::o2_clearState80() {
	int obj = getVarOrDirectWord(0x80);
	putState(obj, getState(obj) & 0x7f);
	removeObjectFromRoom(obj);
	clearDrawObjectQueue();
}

void Scumm_v2::o2_setState40() {
	int obj = getVarOrDirectWord(0x80);
	putState(obj, getState(obj) | 0x40);
}

void Scumm_v2::o2_clearState40() {
	int obj = getVarOrDirectWord(0x80);
	putState(obj, getState(obj) & 0x40);
}

void Scumm_v2::o2_setState20() {
	int obj = getVarOrDirectWord(0x80);
	putState(obj, getState(obj) | 0x20);
}

void Scumm_v2::o2_clearState20() {
	int obj = getVarOrDirectWord(0x80);
	putState(obj, getState(obj) & 0x20);
}

void Scumm_v2::o2_setState10() {
	int obj = getVarOrDirectWord(0x80);
	putState(obj, getState(obj) | 0x10);
}

void Scumm_v2::o2_clearState10() {
	int obj = getVarOrDirectWord(0x80);
	putState(obj, getState(obj) & 0x10);
}

void Scumm_v2::getResultPos() {
	_resultVarNumber = fetchScriptByte();
}

void Scumm_v2::getResultPosDirect() {
	_resultVarNumber = _vars[fetchScriptByte()];
}

void Scumm_v2::o2_assignVarWordDirect() {
	getResultPosDirect();
	_vars[_resultVarNumber] = fetchScriptWord();
}

void Scumm_v2::o2_assignVarByteDirect() {
	getResultPosDirect();
	_vars[_resultVarNumber] = fetchScriptByte();
}

void Scumm_v2::o2_assignVarByte() {
	getResultPos();
	_vars[_resultVarNumber] = fetchScriptByte();
}

void Scumm_v2::o2_assignVarWord() {
	getResultPos();
	_vars[_resultVarNumber] = fetchScriptWord();
}

void Scumm_v2::o2_setObjY() {
	int obj = getVarOrDirectWord(0x80);
	int y = fetchScriptByte();

	if (whereIsObject(obj) != WIO_NOT_FOUND) {
		ObjectData *od = &_objs[getObjectIndex(obj)];
		od->walk_y = y;
	}
}

void Scumm_v2::o2_getObjY() {
	int obj = getVarOrDirectWord(0x80);

	if (whereIsObject(obj) != WIO_NOT_FOUND) {
		ObjectData *od = &_objs[getObjectIndex(obj)];
		_vars[_resultVarNumber] = od->walk_y;
	} else {
		_vars[_resultVarNumber] = 0xFF;
	}
}

void Scumm_v2::o2_setBitVar() {
	byte hi = fetchScriptByte();
	byte lo = fetchScriptByte();
	byte a = getVarOrDirectByte(0x80);

	int bit_var = (hi << 8) + lo + a;
	int bit_offset = bit_var & 0x0f;
  bit_var >>= 4;

	if (getVarOrDirectByte(0x80))
		_bitVars[bit_var >> 3] |= (1 << bit_offset);
	else
		_bitVars[bit_var >> 3] &= ~(1 << bit_offset);
}

void Scumm_v2::o2_getBitVar() {
	getResultPos();
	byte hi = fetchScriptByte();
	byte lo = fetchScriptByte();
	byte a = getVarOrDirectByte(0x80);

	int bit_var = (hi << 8) + lo + a;
	int bit_offset = bit_var & 0x0f;
  bit_var >>= 4;

	_vars[_resultVarNumber] = (_bitVars[bit_var >> 3] & (1 << bit_offset)) ? 1 : 0;
}

void Scumm_v2::ifStateGeneral(byte type) {
	int obj = getVarOrDirectWord(0x80);

	if ((getState(obj) & type) == 0)
		o5_jumpRelative();
	else
		ignoreScriptWord();
}

void Scumm_v2::ifNotStateGeneral(byte type) {
	int obj = getVarOrDirectWord(0x80);

	if ((getState(obj) & type) != 0)
		o5_jumpRelative();
	else
		ignoreScriptWord();
}

void Scumm_v2::o2_ifState80() {
	ifStateGeneral(0x80);
}

void Scumm_v2::o2_ifNotState80() {
	ifNotStateGeneral(0x80);
}

void Scumm_v2::o2_ifState40() {
	ifStateGeneral(0x40);
}

void Scumm_v2::o2_ifNotState40() {
	ifNotStateGeneral(0x40);
}

void Scumm_v2::o2_ifState20() {
	ifStateGeneral(0x20);
}

void Scumm_v2::o2_ifNotState20() {
	ifNotStateGeneral(0x20);
}

void Scumm_v2::o2_ifState10() {
	ifStateGeneral(0x10);
}

void Scumm_v2::o2_ifNotState10() {
	ifNotStateGeneral(0x10);
}

void Scumm_v2::o2_addDirect() {
	int a;
	getResultPosDirect();
	a = getVarOrDirectWord(0x80);
	setResult(readVar(_resultVarNumber) + a);
}

void Scumm_v2::o2_subDirect() {
	int a;
	getResultPosDirect();
	a = getVarOrDirectWord(0x80);
	setResult(readVar(_resultVarNumber) - a);
}

void Scumm_v2::o2_waitForActor() {
	byte *oldaddr = _scriptPointer - 1;
	if (derefActorSafe(getVarOrDirectByte(0x80), "o5_wait")->moving) {
		_scriptPointer = oldaddr;
		o5_breakHere();
	}
}

void Scumm_v2::o2_waitForSentence() {
	if (_sentenceNum) {
		if (_sentence[_sentenceNum - 1].freezeCount && !isScriptInUse(_vars[VAR_SENTENCE_SCRIPT]))
			return;
	} else if (!isScriptInUse(_vars[VAR_SENTENCE_SCRIPT]))
		return;

	_scriptPointer--;
	o5_breakHere();
}

void Scumm_v2::o2_restart() {
}
