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
		OPCODE(o_askDisk),
		OPCODE(o_unknown1),
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
		OPCODE(o_unknown13),
		/* 14 */
		OPCODE(o5_print),
		OPCODE(o2_actorFromPos),
		OPCODE(o5_getRandomNr),
		OPCODE(o2_clearState08),
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
		OPCODE(o_freezeScript),
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
		OPCODE(o_unknown1),
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
		OPCODE(o_unknown4D),
		OPCODE(o_putActorAtObject),
		OPCODE(o2_clearState02),
		/* 50 */
		OPCODE(o_nop),
		OPCODE(o_animateActor),
		OPCODE(o5_actorFollowCamera),
		OPCODE(o5_startSound),
		/* 54 */
		OPCODE(o5_setObjectName),
		OPCODE(o2_actorFromPos),
		OPCODE(o5_getActorMoving),
		OPCODE(o2_clearState08),
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
		OPCODE(o5_printEgo),
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
		OPCODE(o_unknown1),
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
		OPCODE(o_unknown93),
		/* 94 */
		OPCODE(o5_print),
		OPCODE(o2_actorFromPos),
		OPCODE(o_stopCurrentScript),
		OPCODE(o2_setState08),
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
		OPCODE(o_unfreezeScript),
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
		OPCODE(o_unknown1),
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
		OPCODE(o_unknownCD),
		OPCODE(o_putActorAtObject),
		OPCODE(o2_setState02),
		/* D0 */
		OPCODE(o_nop),
		OPCODE(o_animateActor),
		OPCODE(o5_actorFollowCamera),
		OPCODE(o5_stopSound),
		/* D4 */
		OPCODE(o5_setObjectName),
		OPCODE(o2_actorFromPos),
		OPCODE(o5_getActorMoving),
		OPCODE(o2_setState08),
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
		OPCODE(o2_loadRoomWithEgo),
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

void ScummEngine_c64::o_unknown13() {
	debug(0, "o_unknown13 (Actor %d)", fetchScriptByte());
}

void ScummEngine_c64::o_loadActor() {
	debug(0, "o_loadActor (%d)", getVarOrDirectByte(PARAM_1));
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

void ScummEngine_c64::o_cursorCommand() {
	// TODO

	byte state = fetchScriptByte();
	debug(0, "o_cursorCommand(%d)", state);

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

	_fullRedraw = true;
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
	byte var = fetchScriptByte();
	byte a = getVarOrDirectByte(PARAM_1);

	int bit_var = var + a;
	int bit_offset = bit_var & 0x0f;
	bit_var >>= 4;

	//if (getVarOrDirectByte(PARAM_2))
	//	_scummVars[bit_var] |= (1 << bit_offset);
	//else
	//	_scummVars[bit_var] &= ~(1 << bit_offset);
	warning("STUB: o_setActorBitVar(%d, %d, %d)", a, bit_var, bit_offset);
}

void ScummEngine_c64::o_getActorBitVar() {
	getResultPos();
	byte var = fetchScriptByte();
	byte a = getVarOrDirectByte(PARAM_1);

	int bit_var = var + a;
	int bit_offset = bit_var & 0x0f;
	bit_var >>= 4;

	//setResult((_scummVars[bit_var] & (1 << bit_offset)) ? 1 : 0);
	setResult(0);
	warning("STUB: o_getActorBitVar(%d, %d, %d)", a, bit_var, bit_offset);
}

void ScummEngine_c64::o_print_c64() {
	_actorToPrintStrFor = getVarOrDirectByte(PARAM_1);
	decodeParseString();
	warning("STUB: o_print_c64()");
}

void ScummEngine_c64::o_printEgo_c64() {
	_actorToPrintStrFor = (byte)VAR(VAR_EGO);
	decodeParseString();
	warning("STUB: o_printEgo_c64()");
}

void ScummEngine_c64::o_unknown1() {
	byte var1 = fetchScriptByte();
	byte var2 = fetchScriptByte();
	byte var3 = fetchScriptByte();
	warning("STUB: o_unknown1(%d, %d, %d)", var1, var2, var3);
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

void ScummEngine_c64::o_askDisk() {
	warning("STUB: o_askDisk");
}

void ScummEngine_c64::o_unknownCD() {
	debug(0, "o_unknownCD(%d)", fetchScriptByte());
}

void ScummEngine_c64::o_beginOverride() {
	fetchScriptByte();
	fetchScriptByte();
	fetchScriptByte();
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
