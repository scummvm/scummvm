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
		OPCODE(o2_putActor),
		OPCODE(o5_startMusic),
		OPCODE(o5_getActorRoom),
		/* 04 */
		OPCODE(o2_isGreaterEqual),
		OPCODE(o2_drawObject),
		OPCODE(o2_getActorElevation),
		OPCODE(o2_setState08),
		/* 08 */
		OPCODE(o5_isNotEqual),
		OPCODE(o5_faceActor),
		OPCODE(o2_assignVarWordIndirect),
		OPCODE(o2_setObjPreposition),
		/* 0C */
		OPCODE(o2_resourceRoutines),
		OPCODE(o5_walkActorToActor),
		OPCODE(o2_putActorAtObject),
		OPCODE(o2_ifNotState08),
		/* 10 */
		OPCODE(o5_getObjectOwner),
		OPCODE(o2_animateActor),
		OPCODE(o2_panCameraTo),
		OPCODE(o2_actorSet),
		/* 14 */
		OPCODE(o5_print),
		OPCODE(o2_actorFromPos),
		OPCODE(o5_getRandomNr),
		OPCODE(o2_clearState02),
		/* 18 */
		OPCODE(o5_jumpRelative),
		OPCODE(o2_doSentence),
		OPCODE(o5_move),
		OPCODE(o2_setBitVar),
		/* 1C */
		OPCODE(o5_startSound),
		OPCODE(o2_ifClassOfIs),
		OPCODE(o2_walkActorTo),
		OPCODE(o2_ifState02),
		/* 20 */
		OPCODE(o5_stopMusic),
		OPCODE(o2_putActor),
		OPCODE(o2_saveLoadGame),
		OPCODE(o2_getActorY),
		/* 24 */
		OPCODE(o2_loadRoomWithEgo),
		OPCODE(o2_drawObject),
		OPCODE(o5_setVarRange),
		OPCODE(o2_setState04),
		/* 28 */
		OPCODE(o5_equalZero),
		OPCODE(o2_setOwnerOf),
		OPCODE(o2_addIndirect),
		OPCODE(o5_delayVariable),
		/* 2C */
		OPCODE(o2_assignVarByte),
		OPCODE(o5_putActorInRoom),
		OPCODE(o2_delay),
		OPCODE(o2_ifNotState04),
		/* 30 */
		OPCODE(o2_setBoxFlags),
		OPCODE(o2_getBitVar),
		OPCODE(o2_setCameraAt),
		OPCODE(o2_roomOps),
		/* 34 */
		OPCODE(o5_getDist),
		OPCODE(o2_findObject),
		OPCODE(o2_walkActorToObject),
		OPCODE(o2_setState01),
		/* 38 */
		OPCODE(o2_isLessEqual),
		OPCODE(o2_doSentence),
		OPCODE(o2_subtract),
		OPCODE(o2_waitForActor),
		/* 3C */
		OPCODE(o5_stopSound),
		OPCODE(o2_setActorElevation),
		OPCODE(o2_walkActorTo),
		OPCODE(o2_ifNotState01),
		/* 40 */
		OPCODE(o2_cutscene),
		OPCODE(o2_putActor),
		OPCODE(o2_startScript),
		OPCODE(o2_getActorX),
		/* 44 */
		OPCODE(o2_isLess),
		OPCODE(o2_drawObject),
		OPCODE(o5_increment),
		OPCODE(o2_clearState08),
		/* 48 */
		OPCODE(o5_isEqual),
		OPCODE(o5_faceActor),
		OPCODE(o2_chainScript),
		OPCODE(o2_setObjPreposition),
		/* 4C */
		OPCODE(o2_waitForSentence),
		OPCODE(o5_walkActorToActor),
		OPCODE(o2_putActorAtObject),
		OPCODE(o2_ifState08),
		/* 50 */
		OPCODE(o2_pickupObject),
		OPCODE(o2_animateActor),
		OPCODE(o5_actorFollowCamera),
		OPCODE(o2_actorSet),
		/* 54 */
		OPCODE(o2_setObjectName),
		OPCODE(o2_actorFromPos),
		OPCODE(o5_getActorMoving),
		OPCODE(o2_setState02),
		/* 58 */
		OPCODE(o2_beginOverride),
		OPCODE(o2_doSentence),
		OPCODE(o2_add),
		OPCODE(o2_setBitVar),
		/* 5C */
		OPCODE(o2_dummy),
		OPCODE(o2_ifClassOfIs),
		OPCODE(o2_walkActorTo),
		OPCODE(o2_ifNotState02),
		/* 60 */
		OPCODE(o2_cursorCommand),
		OPCODE(o2_putActor),
		OPCODE(o5_stopScript),
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
		OPCODE(o2_ifState04),
		/* 70 */
		OPCODE(o5_lights),
		OPCODE(o5_getActorCostume),
		OPCODE(o5_loadRoom),
		OPCODE(o2_roomOps),
		/* 74 */
		OPCODE(o5_getDist),
		OPCODE(o2_findObject),
		OPCODE(o2_walkActorToObject),
		OPCODE(o2_clearState01),
		/* 78 */
		OPCODE(o2_isGreater),
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
		OPCODE(o2_isGreaterEqual),
		OPCODE(o2_drawObject),
		OPCODE(o2_getActorElevation),
		OPCODE(o2_setState08),
		/* 88 */
		OPCODE(o5_isNotEqual),
		OPCODE(o5_faceActor),
		OPCODE(o2_assignVarWordIndirect),
		OPCODE(o2_setObjPreposition),
		/* 8C */
		OPCODE(o2_resourceRoutines),
		OPCODE(o5_walkActorToActor),
		OPCODE(o2_putActorAtObject),
		OPCODE(o2_ifNotState08),
		/* 90 */
		OPCODE(o5_getObjectOwner),
		OPCODE(o2_animateActor),
		OPCODE(o2_panCameraTo),
		OPCODE(o2_actorSet),
		/* 94 */
		OPCODE(o5_print),
		OPCODE(o2_actorFromPos),
		OPCODE(o5_getRandomNr),
		OPCODE(o2_clearState02),
		/* 98 */
		OPCODE(o2_restart),
		OPCODE(o2_doSentence),
		OPCODE(o5_move),
		OPCODE(o2_setBitVar),
		/* 9C */
		OPCODE(o5_startSound),
		OPCODE(o2_ifClassOfIs),
		OPCODE(o2_walkActorTo),
		OPCODE(o2_ifState02),
		/* A0 */
		OPCODE(o5_stopObjectCode),
		OPCODE(o2_putActor),
		OPCODE(o2_saveLoadGame),
		OPCODE(o2_getActorY),
		/* A4 */
		OPCODE(o2_loadRoomWithEgo),
		OPCODE(o2_drawObject),
		OPCODE(o5_setVarRange),
		OPCODE(o2_setState04),
		/* A8 */
		OPCODE(o5_notEqualZero),
		OPCODE(o2_setOwnerOf),
		OPCODE(o2_addIndirect),
		OPCODE(o2_dummy),
		/* AC */
		OPCODE(o2_drawSentence),
		OPCODE(o5_putActorInRoom),
		OPCODE(o2_waitForMessage),
		OPCODE(o2_ifNotState04),
		/* B0 */
		OPCODE(o2_setBoxFlags),
		OPCODE(o2_getBitVar),
		OPCODE(o2_setCameraAt),
		OPCODE(o2_roomOps),
		/* B4 */
		OPCODE(o5_getDist),
		OPCODE(o2_findObject),
		OPCODE(o2_walkActorToObject),
		OPCODE(o2_setState01),
		/* B8 */
		OPCODE(o2_isLessEqual),
		OPCODE(o2_doSentence),
		OPCODE(o2_subtract),
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
		OPCODE(o2_isLess),
		OPCODE(o2_drawObject),
		OPCODE(o5_decrement),
		OPCODE(o2_clearState08),
		/* C8 */
		OPCODE(o5_isEqual),
		OPCODE(o5_faceActor),
		OPCODE(o2_chainScript),
		OPCODE(o2_setObjPreposition),
		/* CC */
		OPCODE(o5_pseudoRoom),
		OPCODE(o5_walkActorToActor),
		OPCODE(o2_putActorAtObject),
		OPCODE(o2_ifState08),
		/* D0 */
		OPCODE(o2_pickupObject),
		OPCODE(o2_animateActor),
		OPCODE(o5_actorFollowCamera),
		OPCODE(o2_actorSet),
		/* D4 */
		OPCODE(o2_setObjectName),
		OPCODE(o2_actorFromPos),
		OPCODE(o5_getActorMoving),
		OPCODE(o2_setState02),
		/* D8 */
		OPCODE(o5_printEgo),
		OPCODE(o2_doSentence),
		OPCODE(o2_add),
		OPCODE(o2_setBitVar),
		/* DC */
		OPCODE(o2_dummy),
		OPCODE(o2_ifClassOfIs),
		OPCODE(o2_walkActorTo),
		OPCODE(o2_ifNotState02),
		/* E0 */
		OPCODE(o2_cursorCommand),
		OPCODE(o2_putActor),
		OPCODE(o5_stopScript),
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
		OPCODE(o2_ifState04),
		/* F0 */
		OPCODE(o5_lights),
		OPCODE(o5_getActorCostume),
		OPCODE(o5_loadRoom),
		OPCODE(o2_roomOps),
		/* F4 */
		OPCODE(o5_getDist),
		OPCODE(o2_findObject),
		OPCODE(o2_walkActorToObject),
		OPCODE(o2_clearState01),
		/* F8 */
		OPCODE(o2_isGreater),
		OPCODE(o2_doSentence),
		OPCODE(o2_verbOps),
		OPCODE(o2_getActorWalkBox),
		/* FC */
		OPCODE(o5_isSoundRunning),
		OPCODE(o2_setActorElevation),
		OPCODE(o2_walkActorTo),
		OPCODE(o2_ifState01)
	};

	_opcodesV2 = opcodes;
}

#define SENTENCE_SCRIPT 2

void Scumm_v2::executeOpcode(byte i) {
	OpcodeProcV2 op = _opcodesV2[i].proc;
	(this->*op) ();
}

const char *Scumm_v2::getOpcodeDesc(byte i) {
	return _opcodesV2[i].desc;
}

int Scumm_v2::getVar() {
	return readVar(fetchScriptByte());
}

void Scumm_v2::decodeParseString() {
	byte buffer[256];
	byte *ptr = buffer;
	byte c;
	bool insertSpace = false;

	while ((c = fetchScriptByte())) {

		insertSpace = (c & 0x80) != 0;
		c &= 0x7f;

		if (c < 8) {
			// Special codes as seen in CHARSET_1 etc. My guess is that they
			// have a similar function as the corresponding embedded stuff in modern
			// games. Hence for now we convert them to the modern format.
			// This might allow us to reuse the existing code.
			*ptr++ = 0xFF;
			*ptr++ = c;
			if (c > 3) {
				*ptr++ = fetchScriptByte();
				*ptr++ = 0;
			}
		} else
			*ptr++ = c;

		if (insertSpace)
			*ptr++ = ' ';
		
	}
	*ptr = 0;

	// For now, always use textSlot 0. Not sure if there are any situations where we might
	// need to us another one?	
	int textSlot = 0;
	
	_string[textSlot].xpos = 0;
	_string[textSlot].ypos = 0;
	_string[textSlot].right = 320;
	_string[textSlot].center = false;
	_string[textSlot].overhead = false;
//	_string[textSlot].color = 9;	// light blue

	_messagePtr = buffer;
	switch (textSlot) {
	case 0:
		actorTalk();
		break;
	case 1:
		drawString(1);
		break;
	case 2:
		unkMessage1();
		break;
	case 3:
		unkMessage2();
		break;
	}
}

int Scumm_v2::readVar(uint var) {
	if (var >= 14 && var <= 16)
		var = _scummVars[var];

	checkRange(_numVariables - 1, 0, var, "Variable %d out of range(r)");
	debug(6, "readvar(%d) = %d", var, _scummVars[var]);
	return _scummVars[var];
}

void Scumm_v2::writeVar(uint var, int value) {
	checkRange(_numVariables - 1, 0, var, "Variable %d out of range(r)");
	debug(6, "writeVar(%d) = %d", var, value);
	_scummVars[var] = value;
}

void Scumm_v2::getResultPosIndirect() {
	_resultVarNumber = _scummVars[fetchScriptByte()];
}

void Scumm_v2::getResultPos() {
	_resultVarNumber = fetchScriptByte();
}

void Scumm_v2::setStateCommon(byte type) {
	int obj = getVarOrDirectWord(0x80);
	putState(obj, getState(obj) | type);
}

void Scumm_v2::clearStateCommon(byte type) {
	int obj = getVarOrDirectWord(0x80);
	putState(obj, getState(obj) & ~type);
}

void Scumm_v2::o2_setState08() {
	int obj = getVarOrDirectWord(0x80);
	putState(obj, getState(obj) | 0x08);
	removeObjectFromRoom(obj);
	clearDrawObjectQueue();
}

void Scumm_v2::o2_clearState08() {
	int obj = getVarOrDirectWord(0x80);
	putState(obj, getState(obj) & ~0x08);
	removeObjectFromRoom(obj);
	clearDrawObjectQueue();
}

void Scumm_v2::o2_setState04() {
	setStateCommon(0x04);
}

void Scumm_v2::o2_clearState04() {
	clearStateCommon(0x04);
}

void Scumm_v2::o2_setState02() {
	setStateCommon(0x02);
}

void Scumm_v2::o2_clearState02() {
	clearStateCommon(0x02);
}

void Scumm_v2::o2_setState01() {
	setStateCommon(0x01);
}

void Scumm_v2::o2_clearState01() {
	clearStateCommon(0x01);
}

void Scumm_v2::o2_assignVarWordIndirect() {
	getResultPosIndirect();
	setResult(getVarOrDirectWord(0x80));
}

void Scumm_v2::o2_assignVarByte() {
	getResultPos();
	setResult(fetchScriptByte());
}

void Scumm_v2::o2_setObjPreposition() {
	int obj = getVarOrDirectWord(0x80);
	int unk = fetchScriptByte();

	if (whereIsObject(obj) != WIO_NOT_FOUND) {
		// FIXME: this might not work properly the moment we save and restore the game.
		byte *ptr = getOBCDFromObject(obj) + 12;
		*ptr &= 0x1F;
		*ptr |= unk << 5;
	}
}

void Scumm_v2::o2_getObjPreposition() {
	getResultPos();
	int obj = getVarOrDirectWord(0x80);

	if (whereIsObject(obj) != WIO_NOT_FOUND) {
		byte *ptr = getOBCDFromObject(obj) + 12;
		setResult(*ptr >> 5);
	} else {
		setResult(0xFF);
	}
}

void Scumm_v2::o2_setBitVar() {
	int var = fetchScriptWord();
	byte a = getVarOrDirectByte(0x80);

	int bit_var = var + a;
	int bit_offset = bit_var & 0x0f;
	bit_var >>= 4;

	if (getVarOrDirectByte(0x40))
		_scummVars[bit_var] |= (1 << bit_offset);
	else
		_scummVars[bit_var] &= ~(1 << bit_offset);

}

void Scumm_v2::o2_getBitVar() {
	getResultPos();
	int var = fetchScriptWord();
	byte a = getVarOrDirectByte(0x80);

	int bit_var = var + a;
	int bit_offset = bit_var & 0x0f;
	bit_var >>= 4;

	setResult((_scummVars[bit_var] & (1 << bit_offset)) ? 1 : 0);
}

void Scumm_v2::ifStateCommon(byte type) {
	int obj = getVarOrDirectWord(0x80);

	if ((getState(obj) & type) == 0)
		o5_jumpRelative();
	else
		ignoreScriptWord();
}

void Scumm_v2::ifNotStateCommon(byte type) {
	int obj = getVarOrDirectWord(0x80);

	if ((getState(obj) & type) != 0)
		o5_jumpRelative();
	else
		ignoreScriptWord();
}

void Scumm_v2::o2_ifState08() {
	ifStateCommon(0x08);
}

void Scumm_v2::o2_ifNotState08() {
	ifNotStateCommon(0x08);
}

void Scumm_v2::o2_ifState04() {
	ifStateCommon(0x04);
}

void Scumm_v2::o2_ifNotState04() {
	ifNotStateCommon(0x04);
}

void Scumm_v2::o2_ifState02() {
	ifStateCommon(0x02);
}

void Scumm_v2::o2_ifNotState02() {
	ifNotStateCommon(0x02);
}

void Scumm_v2::o2_ifState01() {
	ifStateCommon(0x01);
}

void Scumm_v2::o2_ifNotState01() {
	ifNotStateCommon(0x01);
}

void Scumm_v2::o2_addIndirect() {
	int a;
	getResultPosIndirect();
	a = getVarOrDirectWord(0x80);
	_scummVars[_resultVarNumber] += a;
}

void Scumm_v2::o2_subIndirect() {
	int a;
	getResultPosIndirect();
	a = getVarOrDirectWord(0x80);
	_scummVars[_resultVarNumber] -= a;
}

void Scumm_v2::o2_add() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(0x80);
	_scummVars[_resultVarNumber] += a;
}

void Scumm_v2::o2_subtract() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(0x80);
	_scummVars[_resultVarNumber] -= a;
}

void Scumm_v2::o2_waitForActor() {
	Actor *a = derefActor(getVarOrDirectByte(0x80), "o2_waitForActor");
	if (a->moving) {
		_scriptPointer -= 2;
		o5_breakHere();
	}
}

void Scumm_v2::o2_waitForMessage() {
	
	if (VAR(VAR_HAVE_MSG)) {
		_scriptPointer--;
		o5_breakHere();
	}
}

void Scumm_v2::o2_waitForSentence() {
	if (!_sentenceNum && !isScriptInUse(SENTENCE_SCRIPT))
		return;

	_scriptPointer--;
	o5_breakHere();
}

void Scumm_v2::o2_actorSet() {
	int act = getVarOrDirectByte(0x80);
	int arg = getVarOrDirectByte(0x40);
	Actor *a;
	int i;

	_opcode = fetchScriptByte();
	if (act == 0 && _opcode == 5) {
		// This case happens in the Zak/MM bootscripts, to set the default talk color (9).
		_string[0].color = arg;
		return;
	}
	
	a = derefActor(act, "actorSet");

	switch (_opcode) {
		case 1: 	// Actor Sound
			a->sound[0] = arg;
			break;

		case 2:		// Actor Set Color
			if (_version == 1) {
				i = 0;
			} else {
				i = fetchScriptByte();
			}
			a->palette[i] = arg;
			a->needRedraw = true;
			break;

		case 3:		// Actor Name
			loadPtrToResource(rtActorName, a->number, NULL);
			break;

		case 4:		// Actor Costume
			a->setActorCostume(arg);
			break;

		case 5:		// Talk Color
			a->talkColor = arg;
			break;
		default:
			warning("o2_actorSet: opcode %d not yet supported", _opcode);
	}
}

void Scumm_v2::o2_restart() {
	restart();
}

void Scumm_v2::o2_drawObject() {
	int obj, idx, i;
	ObjectData *od;
	uint16 x, y, w, h;
	int xpos, ypos;

	obj = getVarOrDirectWord(0x80);
	xpos = getVarOrDirectByte(0x40);
	ypos = getVarOrDirectByte(0x20);

	idx = getObjectIndex(obj);
	if (idx == -1)
		return;

	od = &_objs[idx];
	if (xpos != 0xFF) {
		od->walk_x += (xpos << 3) - od->x_pos;
		od->x_pos = xpos << 3;
		od->walk_y += (ypos << 3) - od->y_pos;
		od->y_pos = ypos << 3;
	}
	addObjectToDrawQue(idx);

	x = od->x_pos;
	y = od->y_pos;
	w = od->width;
	h = od->height;

	i = _numLocalObjects;
	while (i--) {
		if (_objs[i].obj_nr && _objs[i].x_pos == x && _objs[i].y_pos == y && _objs[i].width == w && _objs[i].height == h)
			putState(_objs[i].obj_nr, getState(_objs[i].obj_nr) & ~0x08);
	}

	putState(obj, getState(od->obj_nr) | 0x08);
}

void Scumm_v2::o2_resourceRoutines() {
	const ResTypes resTypes[] = {
			rtNumTypes,	// Unknown / invalid
			rtNumTypes,	// Unknown / invalid
			rtCostume,
			rtRoom,
			rtNumTypes,	// Unknown / invalid
			rtScript,
			rtSound
		};
	int resid = getVarOrDirectByte(0x80);
	int opcode = fetchScriptByte();

	ResTypes type = rtNumTypes;
	if (0 <= (opcode >> 4) && (opcode >> 4) < (int)ARRAYSIZE(resTypes))
		type = resTypes[opcode >> 4];

	if (type == rtNumTypes) {
		warning("o2_resourceRoutines: unknown restype %d", (opcode >> 4));
		return;
	}

	if (((opcode & 0x0f) == 0) || ((opcode & 0x0f) == 1)) {
		if (opcode & 1) {
			ensureResourceLoaded(type, resid);
		} else {
			// Seems the nuke opcodes do nothing?
			warning("o2_resourceRoutines: nuking resType %d, id %d does nothing", type, resid);
		}
	} else {
		if (opcode & 1) {
			lock(type, resid);
		} else {
			unlock(type, resid);
		}
	}
}

void Scumm_v2::o2_verbOps() {
	int verb = fetchScriptByte();
	int slot, state;
	
	switch (verb) {
	case 0:		// Delete Verb
		slot = getVarOrDirectByte(0x80) + 1;
		assert(0 < slot && slot < _maxVerbs);

		//printf("o2_verbOps delete slot = %d\n", slot);
		killVerb(slot);
		break;
	
	case 0xFF:	// Verb On/Off
		verb = fetchScriptByte();
		state = fetchScriptByte();
		slot = getVerbSlot(verb, 0);

		//printf("o2_verbOps Verb On/Off: verb = %d, slot = %d, state = %d\n", verb, slot, state);
		
		_verbs[slot].curmode = state;

		break;
	
	default: {	// New Verb
		int x = fetchScriptByte() << 3;
		int y = fetchScriptByte() << 3;
		slot = getVarOrDirectByte(0x80) + 1;
		int prep = fetchScriptByte(); // Only used in V1?
		printf("Setting prep %d for slot %d\n", prep, slot);
		// V1 Maniac verbs are relative to the 'verb area' - under the sentence
		if ((_gameId == GID_MANIAC) && (_version == 1))
			y+=8;

		//printf("o2_verbOps: verb = %d, slot = %d, x = %d, y = %d, unk = %d, name = %s\n",
		//		verb, slot, x, y, unk, _scriptPointer);

		VerbSlot *vs;
		assert(0 < slot && slot < _maxVerbs);

		vs = &_verbs[slot];
		vs->verbid = verb;
		if (_version == 1) {
			vs->color = 5;
			vs->hicolor = 7;
			vs->dimcolor = 11;
		} else {
			vs->color = 2;
			vs->hicolor = 14;
			vs->dimcolor = 8;
		}
		vs->type = kTextVerbType;
		vs->charset_nr = _string[0].t_charset;
		vs->curmode = 1;
		vs->saveid = 0;
		vs->key = 0;
		vs->center = 0;
		vs->imgindex = 0;
		vs->prep = prep;
		
		vs->x = x;
		vs->y = y;
		
		// FIXME: again, this map depends on the language of the game.
		// E.g. a german keyboard has 'z' and 'y' swapped, while a french
		// keyboard starts with "awert", etc.
		const char keyboard[] = {
				'q','w','e','r','t',
				'a','s','d','f','g',
				'z','x','c','v','b'
			};
		if (1 <= slot && slot <= ARRAYSIZE(keyboard))
			vs->key = keyboard[slot - 1];

		// It follows the verb name
		loadPtrToResource(rtVerb, slot, NULL);
		}
		break;
	}

	// FIXME - hack!
	drawVerb(slot, 0);
	verbMouseOver(0);
}

void Scumm_v2::o2_doSentence() {
	int a;
	SentenceTab *st;

	a = getVarOrDirectByte(0x80);
	if (a == 0xFC) {
		_sentenceNum = 0;
		stopScript(SENTENCE_SCRIPT);
		return;
	}
	if (a == 0xFB) {
		resetSentence();
		return;
	}

	st = &_sentence[_sentenceNum++];

	st->verb = a;
	st->objectA = getVarOrDirectWord(0x40);
	st->objectB = getVarOrDirectWord(0x20);
	st->preposition = (st->objectB != 0);
	st->freezeCount = 0;
	
	// TODO
	switch(fetchScriptByte()) {
	case 1:
		// Execute the sentence
		_sentenceNum--;
		
		if (st->verb == 254) {
			stopObjectScript(st->objectA, true);
		} else if (st->verb != 253 && st->verb != 250) {
			VAR(VAR_ACTIVE_VERB) = st->verb;
			VAR(VAR_ACTIVE_OBJECT1) = st->objectA;	
			VAR(VAR_ACTIVE_OBJECT2) = st->objectB;

			runObjectScript(st->objectA, st->verb, false, false, NULL);
		} else
				runObjectScript(st->objectA, 253, (st->verb == 250), true, NULL);
		break;
	case 2:
		// Print the sentence
		_sentenceNum--;
		//warning("TODO o2_doSentence(%d, %d, %d): print", st->verb, st->objectA, st->objectB);
		
		VAR(VAR_SENTENCE_VERB) = st->verb;
		VAR(VAR_SENTENCE_OBJECT1) = st->objectA;
		VAR(VAR_SENTENCE_OBJECT2) = st->objectB;

		o2_drawSentence();
		break;
	}
}

void Scumm_v2::o2_drawSentence() {
	ScummVM::Rect sentenceline;
	static char sentence[256];
	const byte *temp;
	int slot = getVerbSlot(VAR(VAR_SENTENCE_VERB),0);

	if (!(_userState & 32))
		return;

	if (getResourceAddress(rtVerb, slot))
		strcpy(sentence, (char*)getResourceAddress(rtVerb, slot));
	else
		return;

	if (VAR(VAR_SENTENCE_OBJECT1) > 0) {
		temp = getObjOrActorName(VAR(VAR_SENTENCE_OBJECT1));
		if (temp) {
			strcat(sentence, " ");
			strcat(sentence, (const char*)temp);
		}
	
		if ((_version == 1) && (VAR(VAR_SENTENCE_PREPOSITION) == 0)) {
			byte *ptr = getOBCDFromObject(VAR(VAR_SENTENCE_OBJECT1)) + 12;
			int prep = (*ptr >> 5);
			VerbSlot *vs = &_verbs[slot];
	
			if (vs->prep == 0xFF)
				VAR(VAR_SENTENCE_PREPOSITION) = (*ptr >> 5);
			else
				VAR(VAR_SENTENCE_PREPOSITION) = vs->prep;
		}
	}

	if (0 < VAR(VAR_SENTENCE_PREPOSITION) && VAR(VAR_SENTENCE_PREPOSITION) <= 4) {
		// The prepositions, like the fonts, were hard code in the engine. Thus
		// we have to do that, too, and provde localized versions for all the
		// languages MM/Zak are available in.
		//
		// The order here matches the one defined in gameDetector.h
		const char *prepositions[][5] = {
			{ " ", " in", " with", " on", " to" },	// English
			{ " ", " mit", " mit", " mit", " zu" },	// German
			{ " ", " dans", " avec", " sur", " <" },	// French
			{ " ", " in", " con", " su", " a" },	// Italian
			{ " ", " in", " with", " on", " to" },	// Portugese
			{ " ", " en", " con", " en", " a" },	// Spanish
			{ " ", " in", " with", " on", " to" },	// Japanese
			{ " ", " in", " with", " on", " to" },	// Chinese
			{ " ", " in", " with", " on", " to" }	// Korean
			};
		int lang = (_language <= 8) ? _language : 0;	// Default to english
		strcat(sentence, prepositions[lang][VAR(VAR_SENTENCE_PREPOSITION)]);
	}

	if (VAR(VAR_SENTENCE_OBJECT2) > 0) {
		temp = getObjOrActorName(VAR(VAR_SENTENCE_OBJECT2));
		if (temp) {
			strcat(sentence, " ");
			strcat(sentence, (const char*)temp);
		}
	}

	_string[2].charset = 1;
	_string[2].ypos = virtscr[2].topline;
	_string[2].xpos = 0;
	if(_version == 1)
		_string[2].color = 4;
	else
		_string[2].color = 13;

	char *ptr = sentence;
	int n = 0;

	// Maximum length: 40 printable characters
	while (*ptr) {
		if (*ptr != '@')
			n++;
		if (n > 40) {
			*ptr = 0;
			break;
		}
		ptr++;
	}

	_messagePtr = (byte*)sentence;

	sentenceline.top = virtscr[2].topline;
	sentenceline.bottom = virtscr[2].topline + 8;
	sentenceline.left = 0;
	sentenceline.right = 319;
	restoreBG(sentenceline);

	drawString(2);
}

void Scumm_v2::o2_ifClassOfIs() {
	int obj = getVarOrDirectWord(0x80);
	int clsop = getVarOrDirectByte(0x40);
	byte *obcd = getOBCDFromObject(obj);

	if (obcd == 0) {
		o5_jumpRelative();
		return;
	}

	byte cls = *(obcd + 6);
	if ((cls & clsop) != clsop) {
		o5_jumpRelative();
		return;
	}
	ignoreScriptWord();
}

void Scumm_v2::o2_walkActorTo() {
	int x, y;
	Actor *a;
	a = derefActor(getVarOrDirectByte(0x80), "o2_walkActorTo");

	x = getVarOrDirectByte(0x40) * 8;
	y = getVarOrDirectByte(0x20) * 2;

	a->startWalkActor(x, y, -1);
}

void Scumm_v2::o2_putActor() {
	int act = getVarOrDirectByte(0x80);
	int x, y;
	Actor *a;

	a = derefActor(act, "o2_putActor");

	x = getVarOrDirectByte(0x40) * 8;
	y = getVarOrDirectByte(0x20) * 2;

	a->putActor(x, y, a->room);
}

void Scumm_v2::o2_startScript() {
	int script = getVarOrDirectByte(0x80);
	runScript(script, 0, 0, 0);
}

void Scumm_v2::o2_panCameraTo() {
	panCameraTo(getVarOrDirectByte(0x80) * 8, 0);
}

void Scumm_v2::o2_walkActorToObject() {
	int obj;
	Actor *a;

	a = derefActor(getVarOrDirectByte(0x80), "o2_walkActorToObject");
	obj = getVarOrDirectWord(0x40);
	if (whereIsObject(obj) != WIO_NOT_FOUND) {
		int x, y, dir;
		getObjectXYPos(obj, x, y, dir);
		a->startWalkActor(x, y, dir);
	}
}

void Scumm_v2::o2_putActorAtObject() {
	int obj, x, y;
	Actor *a;

	a = derefActor(getVarOrDirectByte(0x80), "o2_putActorAtObject");

	obj = getVarOrDirectWord(0x40);
	if (whereIsObject(obj) != WIO_NOT_FOUND)
		getObjectXYPos(obj, x, y);
	else {
		x = 240;
		y = 120;
	}

	a->putActor(x, y, a->room);
}

void Scumm_v2::o2_getActorElevation() {
	getResultPos();
	int act = getVarOrDirectByte(0x80);
	Actor *a = derefActor(act, "o2_getActorElevation");
	setResult(a->elevation / 2);
}

void Scumm_v2::o2_setActorElevation() {
	int act = getVarOrDirectByte(0x80);
	int elevation = (int8)getVarOrDirectByte(0x40);

	Actor *a = derefActor(act, "o2_setActorElevation");
	a->elevation = elevation * 2;
}

void Scumm_v2::o2_animateActor() {
	int act = getVarOrDirectByte(0x80);
	int anim = getVarOrDirectByte(0x40);

	Actor *a = derefActor(act, "o2_animateActor");
	a->animateActor(anim);
}

void Scumm_v2::o2_actorFromPos() {
	int x, y;
	getResultPos();
	x = getVarOrDirectByte(0x80) * 8;
	y = getVarOrDirectByte(0x40) * 2;
	setResult(getActorFromPos(x, y));
}

void Scumm_v2::o2_findObject() {
	getResultPos();
	int x = getVarOrDirectByte(0x80) * 8;
	int y = getVarOrDirectByte(0x40) * 2;
	setResult(findObject(x, y));
}

void Scumm_v2::o2_saveLoadGame() {
	getResultPos();
	byte a = getVarOrDirectByte(0x80);

	warning("TODO: o2_saveLoadGame(%d)", a);
	setResult(0);
}

void Scumm_v2::o2_getActorX() {
	int a;
	getResultPos();

	a = getVarOrDirectByte(0x80);
	setResult(getObjX(a) / 8);
}

void Scumm_v2::o2_getActorY() {
	int a;
	getResultPos();

	a = getVarOrDirectByte(0x80);
	setResult(getObjY(a) / 2);
}

void Scumm_v2::o2_isGreater() {
	uint16 a = getVar();
	uint16 b = getVarOrDirectWord(0x80);
	if (b > a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void Scumm_v2::o2_isGreaterEqual() {
	uint16 a = getVar();
	uint16 b = getVarOrDirectWord(0x80);
	if (b >= a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void Scumm_v2::o2_isLess() {
	uint16 a = getVar();
	uint16 b = getVarOrDirectWord(0x80);

	if (b < a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void Scumm_v2::o2_isLessEqual() {
	uint16 a = getVar();
	uint16 b = getVarOrDirectWord(0x80);
	if (b <= a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void Scumm_v2::o2_loadRoomWithEgo() {
	Actor *a;
	int obj, room, x, y;

	obj = getVarOrDirectWord(0x80);
	room = getVarOrDirectByte(0x40);

	a = derefActor(VAR(VAR_EGO), "o2_loadRoomWithEgo");

	a->putActor(0, 0, room);
	_egoPositioned = false;

	x = (int8)fetchScriptByte() * 8;
	y = (int8)fetchScriptByte() * 2;

	startScene(a->room, a, obj);

	camera._dest.x = camera._cur.x = a->x;
	setCameraAt(a->x, a->y);
	setCameraFollows(a);

	_fullRedraw = 1;

	resetSentence();

	if (x >= 0 && y >= 0) {
		a->startWalkActor(x, y, -1);
	}
	runScript(5, 0, 0, 0);
}

void Scumm_v2::o2_setOwnerOf() {
	int obj, owner;

	obj = getVarOrDirectWord(0x80);
	owner = getVarOrDirectByte(0x40);

	setOwnerOf(obj, owner);
}

void Scumm_v2::o2_delay() {
	int delay = fetchScriptByte();
	delay |= fetchScriptByte() << 8;
	delay |= fetchScriptByte() << 16;
	delay = 0xFFFFFF - delay;

	vm.slot[_currentScript].delay = delay;
	vm.slot[_currentScript].status = 1;
	o5_breakHere();
}

void Scumm_v2::o2_setBoxFlags() {
	int a, b;

	a = getVarOrDirectByte(0x80);
	b = fetchScriptByte();
	setBoxFlags(a, b);
}

void Scumm_v2::o2_setCameraAt() {
	setCameraAtEx(getVarOrDirectByte(0x80) * 8);
}

void Scumm_v2::o2_roomOps() {
	int a = getVarOrDirectByte(0x80);
	int b = getVarOrDirectByte(0x40);

	_opcode = fetchScriptByte();
	switch (_opcode & 0x1F) {
	case 1:											/* room scroll */
		a *= 8;
		b *= 8;
		if (a < (_screenWidth / 2))
			a = (_screenWidth / 2);
		if (b < (_screenWidth / 2))
			b = (_screenWidth / 2);
		if (a > _roomWidth - (_screenWidth / 2))
			a = _roomWidth - (_screenWidth / 2);
		if (b > _roomWidth - (_screenWidth / 2))
			b = _roomWidth - (_screenWidth / 2);
		VAR(VAR_CAMERA_MIN_X) = a;
		VAR(VAR_CAMERA_MAX_X) = b;
		break;
	case 2:											/* room color */
		_shadowPalette[b] = a;
		_fullRedraw = true;
		break;
	}
}

void Scumm_v2::o2_cutscene() {
	//warning("TODO o2_cutscene()");

	vm.cutSceneData[0] = _userState | (_userPut ? 16 : 0);
	vm.cutSceneData[1] = (int16)VAR(VAR_CURSORSTATE);
	vm.cutSceneData[2] = _currentRoom;
	vm.cutSceneData[3] = camera._mode;
	
	VAR(VAR_CURSORSTATE) = 200;
	
	// Hide inventory, freeze scripts, hide cursor
	setUserState(15);
	
	_sentenceNum = 0;
	stopScript(SENTENCE_SCRIPT);
	resetSentence();

	vm.cutScenePtr[0] = 0;
}

void Scumm_v2::o2_endCutscene() {
	//warning("TODO o2_endCutscene()");

	vm.cutSceneStackPointer = 0;

	VAR(VAR_OVERRIDE) = 0;
	vm.cutSceneScript[0] = 0;
	vm.cutScenePtr[0] = 0;
	
	VAR(VAR_CURSORSTATE) = vm.cutSceneData[1];

	// Reset user state to values before cutscene
	setUserState(vm.cutSceneData[0] | 7);
	
	if (_gameId == GID_MANIAC) {
		camera._mode = (byte) vm.cutSceneData[3];
		if (camera._mode == CM_FOLLOW_ACTOR) {
			actorFollowCamera(VAR(VAR_EGO));
		} else if (vm.cutSceneData[2] != _currentRoom) {
			startScene(vm.cutSceneData[2], 0, 0);
		}
	} else {
		actorFollowCamera(VAR(VAR_EGO));
	}
}

void Scumm_v2::o2_beginOverride() {
	vm.cutScenePtr[0] = _scriptPointer - _scriptOrgPointer;
	vm.cutSceneScript[0] = _currentScript;

	// Skip the jump instruction following the override instruction
	fetchScriptByte();
	fetchScriptWord();
}

void Scumm_v2::o2_chainScript() {
	int data = getVarOrDirectByte(0x80);
	stopScript(vm.slot[_currentScript].number);
	_currentScript = 0xFF;
	runScript(data, 0, 0, 0);
}

void Scumm_v2::o2_pickupObject() {
	int obj = getVarOrDirectWord(0x80);

	if (obj < 1) {
		error("pickupObject received invalid index %d (script %d)", obj, vm.slot[_currentScript].number);
	}

	if (getObjectIndex(obj) == -1)
		return;

	if (whereIsObject(obj) == WIO_INVENTORY)	/* Don't take an */
		return;											/* object twice */

	addObjectToInventory(obj, _roomResource);
	removeObjectFromRoom(obj);
	putOwner(obj, VAR(VAR_EGO));
	putState(obj, getState(obj) | 0xA);
	clearDrawObjectQueue();

	runInventoryScript(1);
}

void Scumm_v2::o2_setObjectName() {
	int obj = getVarOrDirectWord(0x80);
	int size = 0;
	int a;
	int i = 0;
	byte *name = NULL;
	byte work[256];
	
	// Read in new name
	do {
		a = fetchScriptByte();
		work[i++] = a;
	} while (a);

	if (obj < _numActors)
		error("Can't set actor %d name with new-name-of", obj);

	// TODO: Would be nice if we used rtObjectName resource for pre-V6
	// games, too. The only problem with that which I can see is that this
	// would break savegames. I.e. it would require yet another change to
	// the save/load system.

	// FIXME: This is rather nasty code.
	// Find the object name in the OBCD resource.
	byte *objptr;
	objptr = getOBCDFromObject(obj);
	if (objptr == NULL)
		return;	// Silently fail for now
	name = objptr + *(objptr + 14);

	while (name[size++])
		;

	if (i > size) {
		warning("New name of object %d too long (old *%s* new *%s*)", obj, name, work);
		i = size;
	}

	while (i < size) {
		work[i-1] = '@';
		i++;
	}
	work[i-1] = 0;
	
	memcpy(name, work, i);
	runInventoryScript(0);
}

void Scumm_v2::o2_cursorCommand() {	// TODO: Define the magic numbers
	uint16 cmd = getVarOrDirectWord(0x80);
	byte state = cmd >> 8;

	if (cmd & 0xFF) {
		VAR(VAR_CURSORSTATE) = cmd & 0xFF;
	}
	
	setUserState(state);
}

void Scumm_v2::setUserState(byte state) {
	if (state & 4) {						// Userface
		_userState = state & (32 | 64 | 128);
		runInventoryScript(0);
	}

	if (state & 1) {						// Freeze
		if (state & 8)
			freezeScripts(0);
		else
			unfreezeScripts();
	}

	if (state & 2) {						// Cursor Show/Hide

		// Mark verbs as hidden/visible
		for (int i = 0; i < _maxVerbs; i++) {
			_verbs[i].saveid = !(state & 16);
		}

		if (state & 16) {
			_userPut = 1;
			_cursor.state = 1;
			
			// Draw all verbs
			redrawVerbs();
		} else {
			_userPut = 0;
			_cursor.state = 0;
			
			// Hide all verbs
			ScummVM::Rect rect;
			rect.top = virtscr[2].topline;
			rect.bottom = virtscr[2].topline + 6*88;
			rect.left = 0;
			rect.right = 319;
			restoreBG(rect);
		}
	}
}

void Scumm_v2::o2_getActorWalkBox() {
	Actor *a;
	getResultPos();
	a = derefActor(getVarOrDirectByte(0x80), "o2_getActorWalkbox");
	setResult(a->walkbox);
}

void Scumm_v2::o2_dummy() {
	warning("o2_dummy invoked (opcode %d)", _opcode);
}

void Scumm_v2::resetSentence() {
	VAR(VAR_SENTENCE_VERB) = VAR(VAR_BACKUP_VERB);
	VAR(VAR_SENTENCE_OBJECT1) = 0;
	VAR(VAR_SENTENCE_OBJECT2) = 0;
	VAR(VAR_SENTENCE_PREPOSITION) = 0;
}

enum {
	ssDead = 0,
	ssPaused = 1,
	ssRunning = 2
};

void Scumm_v2::runObjectScript(int object, int entry, bool freezeResistant, bool recursive, int *vars) {
	ScriptSlot *s;
	uint32 obcd;
	int slot, where, offs;

	if (!object)
		return;

	stopObjectScript(object, recursive);

	where = whereIsObject(object);

	if (where == WIO_NOT_FOUND) {
		warning("Code for object %d not in room %d", object, _roomResource);
		return;
	}

	obcd = getOBCDOffs(object);
	slot = getScriptSlot();

	offs = getVerbEntrypoint(object, entry);
	if (offs == 0)
		return;

	s = &vm.slot[slot];
	s->number = object;
	s->offs = obcd + offs;
	s->status = ssRunning;
	s->where = where;
	s->freezeResistant = freezeResistant;
	s->recursive = recursive;
	s->freezeCount = 0;
	s->delayFrameCount = 0;

	initializeLocals(slot, vars);

	runScriptNested(slot);
}

/* Stop an object script 'script'*/
void Scumm_v2::stopObjectScript(int script, bool recursive) {
	ScriptSlot *ss;
	NestedScript *nest;
	int i, num;

	if (script == 0)
		return;

	nest = vm.nest;
	num = _numNestedScripts;

	while (num > 0) {
		if (nest->number == script &&
			vm.slot[nest->slot].recursive == recursive && 
		    (nest->where == WIO_ROOM || nest->where == WIO_INVENTORY || nest->where == WIO_FLOBJECT)) {
			nest->number = 0xFF;
			nest->slot = 0xFF;
			nest->where = 0xFF;
		}
		nest++;
		num--;
	}

	ss = vm.slot;
	for (i = 0; i < NUM_SCRIPT_SLOT; i++, ss++) {
		if (script == ss->number && ss->status != ssDead &&
		    ss->recursive == recursive &&
		    (ss->where == WIO_ROOM || ss->where == WIO_INVENTORY || ss->where == WIO_FLOBJECT)) {
			if (ss->cutsceneOverride)
				error("Object %d stopped with active cutscene/override", script);
			ss->number = 0;
			ss->status = ssDead;
			if (_currentScript == i)
				_currentScript = 0xFF;
		}
	}
}
