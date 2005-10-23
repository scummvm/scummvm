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
#include "scumm/sound.h"
#include "scumm/util.h"
#include "scumm/verbs.h"

namespace Scumm {

#define OPCODE(x)	_OPCODE(ScummEngine_v2, x)

void ScummEngine_v2::setupOpcodes() {
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
		OPCODE(o2_actorOps),
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
		OPCODE(o5_saveLoadGame),
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
		OPCODE(o2_actorOps),
		/* 54 */
		OPCODE(o5_setObjectName),
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
		OPCODE(o2_stopScript),
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
		OPCODE(o2_lights),
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
		OPCODE(o2_actorOps),
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
		OPCODE(o5_saveLoadGame),
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
		OPCODE(o2_switchCostumeSet),
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
		OPCODE(o2_actorOps),
		/* D4 */
		OPCODE(o5_setObjectName),
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
		OPCODE(o2_stopScript),
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
		OPCODE(o2_lights),
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

#define PARAM_1 0x80
#define PARAM_2 0x40
#define PARAM_3 0x20

void ScummEngine_v2::executeOpcode(byte i) {
	OpcodeProcV2 op = _opcodesV2[i].proc;
	(this->*op) ();
}

const char *ScummEngine_v2::getOpcodeDesc(byte i) {
	return _opcodesV2[i].desc;
}

int ScummEngine_v2::getVar() {
	return readVar(fetchScriptByte());
}

void ScummEngine_v2::decodeParseString() {
	byte buffer[512];
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

	int textSlot = 0;
	_string[textSlot].xpos = 0;
	_string[textSlot].ypos = 0;
	if (_platform == Common::kPlatformNES)
		_string[textSlot].right = 256;
	else
		_string[textSlot].right = 320;
	_string[textSlot].center = false;
	_string[textSlot].overhead = false;

	if (_gameId == GID_MANIAC && _actorToPrintStrFor == 0xFF) {
		if (_platform == Common::kPlatformC64) {
			_string[textSlot].color = 14;
		} else if (_demoMode) {
			_string[textSlot].color = (_version == 2) ? 15 : 1;
		}
	}

	actorTalk(buffer);
}

int ScummEngine_v2::readVar(uint var) {
	if (var >= 14 && var <= 16)
		var = _scummVars[var];

	checkRange(_numVariables - 1, 0, var, "Variable %d out of range(r)");
	debugC(DEBUG_VARS, "readvar(%d) = %d", var, _scummVars[var]);
	return _scummVars[var];
}

void ScummEngine_v2::writeVar(uint var, int value) {
	checkRange(_numVariables - 1, 0, var, "Variable %d out of range(r)");
	debugC(DEBUG_VARS, "writeVar(%d) = %d", var, value);

	_scummVars[var] = value;

	// HACK: Ender's hack around a bug in Maniac. If you take the last dime from
	//       Weird Ed's piggybank, this disables the New Kid option and runs the Jail
	//       cutscene. Script 116 sets var[175] to 1, which disables New Kid in
	//       script 164. Unfortunatly, when New Kid is reenabled (var[175] = 0) in
	//       script 89, script 164 isn't reran to redraw it. Why? Dunno. Hack? Yes.
	if ((var == 175) && (_gameId == GID_MANIAC) && (vm.slot[_currentScript].number == 89))
		runScript(164, 0, 0, 0);
}

void ScummEngine_v2::getResultPosIndirect() {
	_resultVarNumber = _scummVars[fetchScriptByte()];
}

void ScummEngine_v2::getResultPos() {
	_resultVarNumber = fetchScriptByte();
}

void ScummEngine_v2::setStateCommon(byte type) {
	int obj = getVarOrDirectWord(PARAM_1);
	putState(obj, getState(obj) | type);
}

void ScummEngine_v2::clearStateCommon(byte type) {
	int obj = getVarOrDirectWord(PARAM_1);
	putState(obj, getState(obj) & ~type);
}

void ScummEngine_v2::o2_setState08() {
	int obj = getVarOrDirectWord(PARAM_1);
	putState(obj, getState(obj) | 0x08);
	markObjectRectAsDirty(obj);
	clearDrawObjectQueue();
}

void ScummEngine_v2::o2_clearState08() {
	int obj = getVarOrDirectWord(PARAM_1);
	putState(obj, getState(obj) & ~0x08);
	markObjectRectAsDirty(obj);
	clearDrawObjectQueue();
}

void ScummEngine_v2::o2_setState04() {
	setStateCommon(0x04);
}

void ScummEngine_v2::o2_clearState04() {
	clearStateCommon(0x04);
}

void ScummEngine_v2::o2_setState02() {
	setStateCommon(0x02);
}

void ScummEngine_v2::o2_clearState02() {
	clearStateCommon(0x02);
}

void ScummEngine_v2::o2_setState01() {
	setStateCommon(0x01);
}

void ScummEngine_v2::o2_clearState01() {
	clearStateCommon(0x01);
}

void ScummEngine_v2::o2_assignVarWordIndirect() {
	getResultPosIndirect();
	setResult(getVarOrDirectWord(PARAM_1));
}

void ScummEngine_v2::o2_assignVarByte() {
	getResultPos();
	setResult(fetchScriptByte());
}

void ScummEngine_v2::o2_setObjPreposition() {
	int obj = getVarOrDirectWord(PARAM_1);
	int unk = fetchScriptByte();

	if (_platform == Common::kPlatformNES)
		return;

	if (whereIsObject(obj) != WIO_NOT_FOUND) {
		// FIXME: this might not work properly the moment we save and restore the game.
		byte *ptr = getOBCDFromObject(obj) + 12;
		*ptr &= 0x1F;
		*ptr |= unk << 5;
	}
}

void ScummEngine_v2::o2_getObjPreposition() {
	getResultPos();
	int obj = getVarOrDirectWord(PARAM_1);

	if (whereIsObject(obj) != WIO_NOT_FOUND) {
		byte *ptr = getOBCDFromObject(obj) + 12;
		setResult(*ptr >> 5);
	} else {
		setResult(0xFF);
	}
}

void ScummEngine_v2::o2_setBitVar() {
	int var = fetchScriptWord();
	byte a = getVarOrDirectByte(PARAM_1);

	int bit_var = var + a;
	int bit_offset = bit_var & 0x0f;
	bit_var >>= 4;

	if (getVarOrDirectByte(PARAM_2))
		_scummVars[bit_var] |= (1 << bit_offset);
	else
		_scummVars[bit_var] &= ~(1 << bit_offset);

}

void ScummEngine_v2::o2_getBitVar() {
	getResultPos();
	int var = fetchScriptWord();
	byte a = getVarOrDirectByte(PARAM_1);

	int bit_var = var + a;
	int bit_offset = bit_var & 0x0f;
	bit_var >>= 4;

	setResult((_scummVars[bit_var] & (1 << bit_offset)) ? 1 : 0);
}

void ScummEngine_v2::ifStateCommon(byte type) {
	int obj = getVarOrDirectWord(PARAM_1);

	if ((getState(obj) & type) == 0)
		o5_jumpRelative();
	else
		ignoreScriptWord();
}

void ScummEngine_v2::ifNotStateCommon(byte type) {
	int obj = getVarOrDirectWord(PARAM_1);

	if ((getState(obj) & type) != 0)
		o5_jumpRelative();
	else
		ignoreScriptWord();
}

void ScummEngine_v2::o2_ifState08() {
	ifStateCommon(0x08);
}

void ScummEngine_v2::o2_ifNotState08() {
	ifNotStateCommon(0x08);
}

void ScummEngine_v2::o2_ifState04() {
	ifStateCommon(0x04);
}

void ScummEngine_v2::o2_ifNotState04() {
	ifNotStateCommon(0x04);
}

void ScummEngine_v2::o2_ifState02() {
	ifStateCommon(0x02);
}

void ScummEngine_v2::o2_ifNotState02() {
	ifNotStateCommon(0x02);
}

void ScummEngine_v2::o2_ifState01() {
	ifStateCommon(0x01);
}

void ScummEngine_v2::o2_ifNotState01() {
	ifNotStateCommon(0x01);
}

void ScummEngine_v2::o2_addIndirect() {
	int a;
	getResultPosIndirect();
	a = getVarOrDirectWord(PARAM_1);
	_scummVars[_resultVarNumber] += a;
}

void ScummEngine_v2::o2_subIndirect() {
	int a;
	getResultPosIndirect();
	a = getVarOrDirectWord(PARAM_1);
	_scummVars[_resultVarNumber] -= a;
}

void ScummEngine_v2::o2_add() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(PARAM_1);
	_scummVars[_resultVarNumber] += a;
}

void ScummEngine_v2::o2_subtract() {
	int a;
	getResultPos();
	a = getVarOrDirectWord(PARAM_1);
	_scummVars[_resultVarNumber] -= a;
}

void ScummEngine_v2::o2_waitForActor() {
	Actor *a = derefActor(getVarOrDirectByte(PARAM_1), "o2_waitForActor");
	if (a->_moving) {
		_scriptPointer -= 2;
		o5_breakHere();
	}
}

void ScummEngine_v2::o2_waitForMessage() {

	if (VAR(VAR_HAVE_MSG)) {
		_scriptPointer--;
		o5_breakHere();
	}
}

void ScummEngine_v2::o2_waitForSentence() {
	if (!_sentenceNum && !isScriptInUse(SENTENCE_SCRIPT))
		return;

	_scriptPointer--;
	o5_breakHere();
}

void ScummEngine_v2::o2_actorOps() {
	int act = getVarOrDirectByte(PARAM_1);
	int arg = getVarOrDirectByte(PARAM_2);
	Actor *a;
	int i;

	_opcode = fetchScriptByte();
	if (act == 0 && _opcode == 5) {
		// This case happens in the Zak/MM bootscripts, to set the default talk color (9).
		_string[0].color = arg;
		return;
	}

	a = derefActor(act, "actorOps");

	switch (_opcode) {
	case 1: 	// SO_SOUND
		a->_sound[0] = arg;
		break;
	case 2:		// SO_PALETTE
		if (_version == 1)
			i = act;
		else
			i = fetchScriptByte();

		a->setPalette(i, arg);
		break;
	case 3:		// SO_ACTOR_NAME
		loadPtrToResource(rtActorName, a->_number, NULL);
		break;
	case 4:		// SO_COSTUME
		a->setActorCostume(arg);
		break;
	case 5:		// SO_TALK_COLOR
		if (_gameId == GID_MANIAC && _version == 2 && _demoMode && arg == 1)
			a->_talkColor = 15;
		else
			a->_talkColor = arg;
		break;
	default:
		error("o2_actorOps: opcode %d not yet supported", _opcode);
	}
}

void ScummEngine_v2::o2_restart() {
	restart();
}

void ScummEngine_v2::o2_drawObject() {
	int obj, idx, i;
	ObjectData *od;
	uint16 x, y, w, h;
	int xpos, ypos;

	obj = getVarOrDirectWord(PARAM_1);
	xpos = getVarOrDirectByte(PARAM_2);
	ypos = getVarOrDirectByte(PARAM_3);

	idx = getObjectIndex(obj);
	if (idx == -1)
		return;

	od = &_objs[idx];
	if (xpos != 0xFF) {
		od->walk_x += (xpos * 8) - od->x_pos;
		od->x_pos = xpos * 8;
		od->walk_y += (ypos * 8) - od->y_pos;
		od->y_pos = ypos * 8;
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

void ScummEngine_v2::o2_resourceRoutines() {
	const ResTypes resTypes[] = {
		rtNumTypes,	// Invalid
		rtNumTypes,	// Invalid
		rtCostume,
		rtRoom,
		rtNumTypes,	// Invalid
		rtScript,
		rtSound
	};
	int resid = getVarOrDirectByte(PARAM_1);
	int opcode = fetchScriptByte();

	ResTypes type = rtNumTypes;
	if (0 <= (opcode >> 4) && (opcode >> 4) < (int)ARRAYSIZE(resTypes))
		type = resTypes[opcode >> 4];

	if ((opcode & 0x0f) == 0 || type == rtNumTypes)
		return;

	// HACK V2 Maniac Mansion tries to load an invalid sound resource in demo script.
	if (_gameId == GID_MANIAC && _version == 2 && vm.slot[_currentScript].number == 9 && type == rtSound && resid == 1)
		return;

	if ((opcode & 0x0f) == 1) {
		ensureResourceLoaded(type, resid);
	} else {
		if (opcode & 1)
			res.lock(type, resid);
		else
			res.unlock(type, resid);
	}
}

void ScummEngine_v2::o2_verbOps() {
	int verb = fetchScriptByte();
	int slot, state;

	switch (verb) {
	case 0:		// SO_DELETE_VERBS
		slot = getVarOrDirectByte(PARAM_1) + 1;
		assert(0 < slot && slot < _numVerbs);

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
		int x = fetchScriptByte() * 8;
		int y = fetchScriptByte() * 8;
		slot = getVarOrDirectByte(PARAM_1) + 1;
		int prep = fetchScriptByte(); // Only used in V1?
		// V1 Maniac verbs are relative to the 'verb area' - under the sentence
		if (_platform == Common::kPlatformNES)
			x += 8;
		else if ((_gameId == GID_MANIAC) && (_version == 1))
			y += 8;

		//printf("o2_verbOps: verb = %d, slot = %d, x = %d, y = %d, unk = %d, name = %s\n",
		//		verb, slot, x, y, prep, _scriptPointer);

		VerbSlot *vs;
		assert(0 < slot && slot < _numVerbs);

		vs = &_verbs[slot];
		vs->verbid = verb;
		if (_platform == Common::kPlatformNES) {
			vs->color = 1;
			vs->hicolor = 1;
			vs->dimcolor = 1;
		} else if (_version == 1) {
			vs->color = (_gameId == GID_MANIAC && _demoMode) ? 16 : 5;
			vs->hicolor = 7;
			vs->dimcolor = 11;
		} else {
			vs->color = (_gameId == GID_MANIAC && _demoMode) ? 13 : 2;
			vs->hicolor = 14;
			vs->dimcolor = 8;
		}
		vs->type = kTextVerbType;
		vs->charset_nr = _string[0]._default.charset;
		vs->curmode = 1;
		vs->saveid = 0;
		vs->key = 0;
		vs->center = 0;
		vs->imgindex = 0;
		vs->prep = prep;

		vs->curRect.left = x;
		vs->curRect.top = y;

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

	// Force redraw of the modified verb slot
	drawVerb(slot, 0);
	verbMouseOver(0);
}

void ScummEngine_v2::o2_doSentence() {
	int a;
	SentenceTab *st;

	a = getVarOrDirectByte(PARAM_1);
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
	st->objectA = getVarOrDirectWord(PARAM_2);
	st->objectB = getVarOrDirectWord(PARAM_3);
	st->preposition = (st->objectB != 0);
	st->freezeCount = 0;

	// Execute or print the sentence
	_opcode = fetchScriptByte();
	switch (_opcode) {
	case 0:
		// Do nothing (besides setting up the sentence above)
		break;
	case 1:
		// Execute the sentence
		_sentenceNum--;

		if (st->verb == 254) {
			ScummEngine::stopObjectScript(st->objectA);
		} else {
			bool isBackgroundScript;
			bool isSpecialVerb;
			if (st->verb != 253 && st->verb != 250) {
				VAR(VAR_ACTIVE_VERB) = st->verb;
				VAR(VAR_ACTIVE_OBJECT1) = st->objectA;
				VAR(VAR_ACTIVE_OBJECT2) = st->objectB;

				isBackgroundScript = false;
				isSpecialVerb = false;
			} else {
				isBackgroundScript = (st->verb == 250);
				isSpecialVerb = true;
				st->verb = 253;
			}

			// Check if an object script for this object is already running. If
			// so, reuse its script slot. Note that we abuse two script flags:
			// freezeResistant and recursive. We use them to track two
			// script flags used in V1/V2 games. The main reason we do it this
			// ugly evil way is to avoid having to introduce yet another save
			// game revision.
			int slot = -1;
			ScriptSlot *ss;
			int i;

			ss = vm.slot;
			for (i = 0; i < NUM_SCRIPT_SLOT; i++, ss++) {
				if (st->objectA == ss->number &&
					ss->freezeResistant == isBackgroundScript &&
					ss->recursive == isSpecialVerb &&
					(ss->where == WIO_ROOM || ss->where == WIO_INVENTORY || ss->where == WIO_FLOBJECT)) {
					slot = i;
					break;
				}
			}

			runObjectScript(st->objectA, st->verb, isBackgroundScript, isSpecialVerb, NULL, slot);
		}
		break;
	case 2:
		// Print the sentence
		_sentenceNum--;

		VAR(VAR_SENTENCE_VERB) = st->verb;
		VAR(VAR_SENTENCE_OBJECT1) = st->objectA;
		VAR(VAR_SENTENCE_OBJECT2) = st->objectB;

		o2_drawSentence();
		break;
	default:
		error("o2_doSentence: unknown subopcode %d", _opcode);
	}
}

void ScummEngine_v2::o2_drawSentence() {
	Common::Rect sentenceline;
	static char sentence[256];
	const byte *temp;
	int slot = getVerbSlot(VAR(VAR_SENTENCE_VERB), 0);

	if (!((_userState & 32) || (_platform == Common::kPlatformNES && _userState & 0xe0)))
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

		// For V1 games, the engine must compute the preposition.
		// In all other Scumm versions, this is done by the sentence script.
		if ((_gameId == GID_MANIAC && _version == 1 && !(_platform == Common::kPlatformNES)) && (VAR(VAR_SENTENCE_PREPOSITION) == 0)) {
			if (_verbs[slot].prep == 0xFF) {
				byte *ptr = getOBCDFromObject(VAR(VAR_SENTENCE_OBJECT1));
				assert(ptr);
				VAR(VAR_SENTENCE_PREPOSITION) = (*(ptr + 12) >> 5);
			} else
				VAR(VAR_SENTENCE_PREPOSITION) = _verbs[slot].prep;
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
		if (_platform == Common::kPlatformNES) {
			strcat(sentence, (const char *)(getResourceAddress(rtCostume, 78) + VAR(VAR_SENTENCE_PREPOSITION) * 8 + 2));
		} else
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
	_string[2].ypos = virtscr[kVerbVirtScreen].topline;
	_string[2].xpos = 0;
	if (_platform == Common::kPlatformNES) {
		_string[2].xpos = 16;
		_string[2].color = 0;
	} else if (_version == 1)
		_string[2].color = 16;
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

	if (_platform == Common::kPlatformNES) {	// TODO - get multiline sentences working
		sentenceline.top = virtscr[kVerbVirtScreen].topline;
		sentenceline.bottom = virtscr[kVerbVirtScreen].topline + 16;
		sentenceline.left = 16;
		sentenceline.right = 255;
	} else {
		sentenceline.top = virtscr[kVerbVirtScreen].topline;
		sentenceline.bottom = virtscr[kVerbVirtScreen].topline + 8;
		sentenceline.left = 0;
		sentenceline.right = 319;
	}
	restoreBG(sentenceline);

	drawString(2, (byte*)sentence);
}

void ScummEngine_v2::o2_ifClassOfIs() {
	int obj = getVarOrDirectWord(PARAM_1);
	int clsop = getVarOrDirectByte(PARAM_2);
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

void ScummEngine_v2::o2_walkActorTo() {
	int x, y;
	Actor *a;

	int act = getVarOrDirectByte(PARAM_1);

	// FIXME: Work around for bug #1252606
	if (_gameId == GID_ZAK && _version == 1 && vm.slot[_currentScript].number == 115 && act == 249) {
		act = VAR(VAR_EGO);
	}

	a = derefActor(act, "o2_walkActorTo");

	x = getVarOrDirectByte(PARAM_2) * 8;
	y = getVarOrDirectByte(PARAM_3) * 2;

	a->startWalkActor(x, y, -1);
}

void ScummEngine_v2::o2_putActor() {
	int act = getVarOrDirectByte(PARAM_1);
	int x, y;
	Actor *a;

	a = derefActor(act, "o2_putActor");

	x = getVarOrDirectByte(PARAM_2) * 8;
	y = getVarOrDirectByte(PARAM_3) * 2;

	a->putActor(x, y, a->_room);
}

void ScummEngine_v2::o2_startScript() {
	int script = getVarOrDirectByte(PARAM_1);

	if (!_copyProtection) {
		// The enhanced version of Zak McKracken included in the
		// SelectWare Classic Collection bundle used CD check instead
		// of the usual key code check at airports.
		if ((_gameId == GID_ZAK) && (script == 15) && (_roomResource == 45))
			return;
	}

	runScript(script, 0, 0, 0);
}

void ScummEngine_v2::o2_stopScript() {
	int script;

	script = getVarOrDirectByte(PARAM_1);

	if ((_gameId == GID_ZAK) && (_roomResource == 7) && (vm.slot[_currentScript].number == 10001)) {
	// FIXME: Nasty hack for bug #771499
	// Don't let the exit script for room 7 stop the buy script (24),
	// switching to the number selection keypad (script 15)
		if ((script == 24) && isScriptRunning(15))
			return;
	}

	if (script == 0)
		script = vm.slot[_currentScript].number;

	if (_currentScript != 0 && vm.slot[_currentScript].number == script)
		stopObjectCode();
	else
		stopScript(script);
}

void ScummEngine_v2::o2_panCameraTo() {
	panCameraTo(getVarOrDirectByte(PARAM_1) * 8, 0);
}

void ScummEngine_v2::o2_walkActorToObject() {
	int obj;
	Actor *a;

	a = derefActor(getVarOrDirectByte(PARAM_1), "o2_walkActorToObject");
	obj = getVarOrDirectWord(PARAM_2);
	if (whereIsObject(obj) != WIO_NOT_FOUND) {
		int x, y, dir;
		getObjectXYPos(obj, x, y, dir);
		a->startWalkActor(x, y, dir);
	}
}

void ScummEngine_v2::o2_putActorAtObject() {
	int obj, x, y;
	Actor *a;

	a = derefActor(getVarOrDirectByte(PARAM_1), "o2_putActorAtObject");

	obj = getVarOrDirectWord(PARAM_2);
	if (whereIsObject(obj) != WIO_NOT_FOUND)
		getObjectXYPos(obj, x, y);
	else {
		x = 240;
		y = 120;
	}

	a->putActor(x, y, a->_room);
}

void ScummEngine_v2::o2_getActorElevation() {
	getResultPos();
	int act = getVarOrDirectByte(PARAM_1);
	Actor *a = derefActor(act, "o2_getActorElevation");
	setResult(a->getElevation() / 2);
}

void ScummEngine_v2::o2_setActorElevation() {
	int act = getVarOrDirectByte(PARAM_1);
	int elevation = (int8)getVarOrDirectByte(PARAM_2);

	Actor *a = derefActor(act, "o2_setActorElevation");
	a->setElevation(elevation * 2);
}

void ScummEngine_v2::o2_animateActor() {
	int act = getVarOrDirectByte(PARAM_1);
	int anim = getVarOrDirectByte(PARAM_2);

	Actor *a = derefActor(act, "o2_animateActor");
	a->animateActor(anim);
}

void ScummEngine_v2::o2_actorFromPos() {
	int x, y;
	getResultPos();
	x = getVarOrDirectByte(PARAM_1) * 8;
	y = getVarOrDirectByte(PARAM_2) * 2;
	setResult(getActorFromPos(x, y));
}

void ScummEngine_v2::o2_findObject() {
	int obj;
	getResultPos();
	int x = getVarOrDirectByte(PARAM_1) * 8;
	int y = getVarOrDirectByte(PARAM_2) * 2;
	obj = findObject(x, y);
	if (obj == 0 && (_platform == Common::kPlatformNES) && (_userState & 0x40)) {
		if (_mouseOverBoxV2 >= 0 && _mouseOverBoxV2 < 4)
			obj = findInventory(VAR(VAR_EGO), _mouseOverBoxV2 + _inventoryOffset + 1);
	}
	setResult(obj);
}

void ScummEngine_v2::o2_getActorX() {
	int a;
	getResultPos();

	a = getVarOrDirectByte(PARAM_1);
	setResult(getObjX(a) / 8);
}

void ScummEngine_v2::o2_getActorY() {
	int a;
	getResultPos();

	a = getVarOrDirectByte(PARAM_1);
	setResult(getObjY(a) / 2);
}

void ScummEngine_v2::o2_isGreater() {
	uint16 a = getVar();
	uint16 b = getVarOrDirectWord(PARAM_1);
	if (b > a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void ScummEngine_v2::o2_isGreaterEqual() {
	uint16 a = getVar();
	uint16 b = getVarOrDirectWord(PARAM_1);
	if (b >= a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void ScummEngine_v2::o2_isLess() {
	uint16 a = getVar();
	uint16 b = getVarOrDirectWord(PARAM_1);

	if (b < a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void ScummEngine_v2::o2_isLessEqual() {
	uint16 a = getVar();
	uint16 b = getVarOrDirectWord(PARAM_1);
	if (b <= a)
		ignoreScriptWord();
	else
		o5_jumpRelative();
}

void ScummEngine_v2::o2_lights() {
	int a, b, c;

	a = getVarOrDirectByte(PARAM_1);
	b = fetchScriptByte();
	c = fetchScriptByte();

	if (c == 0) {
		if (_gameId == GID_MANIAC && _version == 1 && !(_platform == Common::kPlatformNES)) {
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
		} else
			VAR(VAR_CURRENT_LIGHTS) = a;
	} else if (c == 1) {
		_flashlight.xStrips = a;
		_flashlight.yStrips = b;
	}
	_fullRedraw = true;
}

void ScummEngine_v2::o2_loadRoomWithEgo() {
	Actor *a;
	int obj, room, x, y, x2, y2, dir;

	obj = getVarOrDirectWord(PARAM_1);
	room = getVarOrDirectByte(PARAM_2);

	a = derefActor(VAR(VAR_EGO), "o2_loadRoomWithEgo");

	a->putActor(0, 0, room);
	_egoPositioned = false;

	x = (int8)fetchScriptByte() * 8;
	y = (int8)fetchScriptByte() * 2;

	startScene(a->_room, a, obj);

	getObjectXYPos(obj, x2, y2, dir);
	a->putActor(x2, y2, _currentRoom);
	a->setDirection(dir + 180);

	camera._dest.x = camera._cur.x = a->_pos.x;
	setCameraAt(a->_pos.x, a->_pos.y);
	setCameraFollows(a);

	_fullRedraw = true;

	resetSentence();

	if (x >= 0 && y >= 0) {
		a->startWalkActor(x, y, -1);
	}
	runScript(5, 0, 0, 0);
}

void ScummEngine_v2::o2_setOwnerOf() {
	int obj, owner;

	obj = getVarOrDirectWord(PARAM_1);
	owner = getVarOrDirectByte(PARAM_2);

	setOwnerOf(obj, owner);
}

void ScummEngine_v2::o2_delay() {
	int delay = fetchScriptByte();
	delay |= fetchScriptByte() << 8;
	delay |= fetchScriptByte() << 16;
	delay = 0xFFFFFF - delay;

	vm.slot[_currentScript].delay = delay;
	vm.slot[_currentScript].status = ssPaused;
	o5_breakHere();
}

void ScummEngine_v2::o2_setBoxFlags() {
	int a, b;

	a = getVarOrDirectByte(PARAM_1);
	b = fetchScriptByte();
	setBoxFlags(a, b);
}

void ScummEngine_v2::o2_setCameraAt() {
	setCameraAtEx(getVarOrDirectByte(PARAM_1) * 8);
}

void ScummEngine_v2::o2_roomOps() {
	int a = getVarOrDirectByte(PARAM_1);
	int b = getVarOrDirectByte(PARAM_2);

	_opcode = fetchScriptByte();
	switch (_opcode & 0x1F) {
	case 1:			// SO_ROOM_SCROLL
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
	case 2:			// SO_ROOM_COLOR
		if (_version == 1) {
			// V1 zak needs to know when room color is changed
			_roomPalette[0] = 255;
			_roomPalette[1] = a;
			_roomPalette[2] = b;
		} else {
			_roomPalette[b] = a;
		}
		_fullRedraw = true;
		break;
	}
}

void ScummEngine_v2::o2_cutscene() {
	vm.cutSceneData[0] = _userState | (_userPut ? 16 : 0);
	vm.cutSceneData[1] = (int16)VAR(VAR_CURSORSTATE);
	vm.cutSceneData[2] = _currentRoom;
	vm.cutSceneData[3] = camera._mode;

	VAR(VAR_CURSORSTATE) = 200;

	// FIXME allows quotes script (173) to start during introudction of
	// demo mode of V1 Maniac Mansion. setUserState was halting script
	// 173 before it started.
	if (!(_gameId == GID_MANIAC && _demoMode))
	// Hide inventory, freeze scripts, hide cursor
	setUserState(15);

	_sentenceNum = 0;
	stopScript(SENTENCE_SCRIPT);
	resetSentence();

	vm.cutScenePtr[0] = 0;
}

void ScummEngine_v2::o2_endCutscene() {
	vm.cutSceneStackPointer = 0;

	VAR(VAR_OVERRIDE) = 0;
	vm.cutSceneScript[0] = 0;
	vm.cutScenePtr[0] = 0;

	VAR(VAR_CURSORSTATE) = vm.cutSceneData[1];

	// Reset user state to values before cutscene
	setUserState(vm.cutSceneData[0] | 7);

	if ((_gameId == GID_MANIAC) && !(_platform == Common::kPlatformNES)) {
		camera._mode = (byte) vm.cutSceneData[3];
		if (camera._mode == kFollowActorCameraMode) {
			actorFollowCamera(VAR(VAR_EGO));
		} else if (vm.cutSceneData[2] != _currentRoom) {
			startScene(vm.cutSceneData[2], 0, 0);
		}
	} else {
		actorFollowCamera(VAR(VAR_EGO));
	}
}

void ScummEngine_v2::o2_beginOverride() {
	vm.cutScenePtr[0] = _scriptPointer - _scriptOrgPointer;
	vm.cutSceneScript[0] = _currentScript;

	// Skip the jump instruction following the override instruction
	fetchScriptByte();
	fetchScriptWord();
}

void ScummEngine_v2::o2_chainScript() {
	int data = getVarOrDirectByte(PARAM_1);
	stopScript(vm.slot[_currentScript].number);
	_currentScript = 0xFF;
	runScript(data, 0, 0, 0);
}

void ScummEngine_v2::o2_pickupObject() {
	int obj = getVarOrDirectWord(PARAM_1);

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
	if (_platform == Common::kPlatformNES)
		_sound->addSoundToQueue(51);	// play 'pickup' sound
}

void ScummEngine_v2::o2_cursorCommand() {	// TODO: Define the magic numbers
	uint16 cmd = getVarOrDirectWord(PARAM_1);
	byte state = cmd >> 8;

	if (cmd & 0xFF) {
		VAR(VAR_CURSORSTATE) = cmd & 0xFF;
	}

	setUserState(state);
}

void ScummEngine_v2::setUserState(byte state) {
	if (state & 4) {						// Userface
		if (_platform == Common::kPlatformNES)
			_userState = (_userState & ~0xE0) | (state & 0xE0);
		else
			_userState = state & (32 | 64 | 128);
	}

	if (state & 1) {						// Freeze
		if (state & 8)
			freezeScripts(0);
		else
			unfreezeScripts();
	}

	if (state & 2) {						// Cursor Show/Hide
		if (_platform == Common::kPlatformNES)
			_userState = (_userState & ~0x10) | (state & 0x10);
		if (state & 16) {
			_userPut = 1;
			_cursor.state = 1;
		} else {
			_userPut = 0;
			_cursor.state = 0;
		}
	}

	// Hide all verbs and inventory
	Common::Rect rect;
	rect.top = virtscr[kVerbVirtScreen].topline;
	rect.bottom = virtscr[kVerbVirtScreen].topline + 8 * 88;
	if (_platform == Common::kPlatformNES) {
		rect.left = 16;
		rect.right = 255;
	} else {
		rect.left = 0;
		rect.right = 319;
	}
	restoreBG(rect);

	// Draw all verbs and inventory
	redrawVerbs();
	runInventoryScript(1);
}

void ScummEngine_v2::o2_getActorWalkBox() {
	Actor *a;
	getResultPos();
	a = derefActor(getVarOrDirectByte(PARAM_1), "o2_getActorWalkbox");
	setResult(a->_walkbox);
}

void ScummEngine_v2::o2_dummy() {
	// Opcode 238 is used in maniac and zak but has no purpose
	if (_opcode != 238)
		warning("o2_dummy invoked (opcode %d)", _opcode);
}

void ScummEngine_v2::o2_switchCostumeSet() {
	// NES version of maniac uses this to switch between the two
	// groups of costumes it has
	if (_platform == Common::kPlatformNES)
		NES_loadCostumeSet(fetchScriptByte());
	else if (_platform == Common::kPlatformC64)
		fetchScriptByte();
	else
		o2_dummy();
}

void ScummEngine_v2::resetSentence() {
	VAR(VAR_SENTENCE_VERB) = VAR(VAR_BACKUP_VERB);
	VAR(VAR_SENTENCE_OBJECT1) = 0;
	VAR(VAR_SENTENCE_OBJECT2) = 0;
	VAR(VAR_SENTENCE_PREPOSITION) = 0;
}

void ScummEngine_v2::runInventoryScript(int i) {
	redrawV2Inventory();
}

#undef PARAM_1
#undef PARAM_2
#undef PARAM_3

} // End of namespace Scumm
