/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
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

#include "common/config-manager.h"

#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/intern.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/resource_v7he.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "scumm/verbs.h"
#include "scumm/smush/smush_player.h"

#include "sound/mididrv.h"
#include "sound/mixer.h"

namespace Scumm {

#define OPCODE(x)	{ &ScummEngine_v72he::x, #x }

void ScummEngine_v72he::setupOpcodes() {
	static const OpcodeEntryV72he opcodes[256] = {
		/* 00 */
		OPCODE(o6_pushByte),
		OPCODE(o6_pushWord),
		OPCODE(o72_pushDWord),
		OPCODE(o6_pushWordVar),
		/* 04 */
		OPCODE(o72_addMessageToStack),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o72_wordArrayRead),
		/* 08 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o72_wordArrayIndexedRead),
		/* 0C */
		OPCODE(o6_dup),
		OPCODE(o6_not),
		OPCODE(o6_eq),
		OPCODE(o6_neq),
		/* 10 */
		OPCODE(o6_gt),
		OPCODE(o6_lt),
		OPCODE(o6_le),
		OPCODE(o6_ge),
		/* 14 */
		OPCODE(o6_add),
		OPCODE(o6_sub),
		OPCODE(o6_mul),
		OPCODE(o6_div),
		/* 18 */
		OPCODE(o6_land),
		OPCODE(o6_lor),
		OPCODE(o6_pop),
		OPCODE(o72_compareStackList),
		/* 1C */
		OPCODE(o72_unknown1C),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 20 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 24 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 28 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 2C */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 30 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 34 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 38 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 3C */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 40 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_writeWordVar),
		/* 44 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o72_wordArrayWrite),
		/* 48 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o72_wordArrayIndexedWrite),
		/* 4C */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_wordVarInc),
		/* 50 */
		OPCODE(o72_unknown50),
		OPCODE(o6_invalid),
		OPCODE(o72_findObject),
		OPCODE(o72_wordArrayInc),
		/* 54 */
		OPCODE(o72_objectX),
		OPCODE(o72_objectY),
		OPCODE(o6_invalid),
		OPCODE(o6_wordVarDec),
		/* 58 */
		OPCODE(o72_getTimer),
		OPCODE(o72_setTimer),
		OPCODE(o72_unknown5A),
		OPCODE(o72_wordArrayDec),
		/* 5C */
		OPCODE(o6_if),
		OPCODE(o6_ifNot),
		OPCODE(o72_startScript),
		OPCODE(o6_startScriptQuick),
		/* 60 */
		OPCODE(o72_startObject),
		OPCODE(o72_drawObject),
		OPCODE(o72_printWizImage),
		OPCODE(o72_getArrayDimSize),
		/* 64 */
		OPCODE(o72_getNumFreeArrays),
		OPCODE(o6_stopObjectCode),
		OPCODE(o6_stopObjectCode),
		OPCODE(o6_endCutscene),
		/* 68 */
		OPCODE(o6_cutscene),
		OPCODE(o6_stopMusic),
		OPCODE(o6_freezeUnfreeze),
		OPCODE(o7_cursorCommand),
		/* 6C */
		OPCODE(o6_breakHere),
		OPCODE(o6_ifClassOfIs),
		OPCODE(o6_setClass),
		OPCODE(o6_getState),
		/* 70 */
		OPCODE(o6_setState),
		OPCODE(o6_setOwner),
		OPCODE(o6_getOwner),
		OPCODE(o6_jump),
		/* 74 */
		OPCODE(o7_startSound),
		OPCODE(o6_stopSound),
		OPCODE(o6_startMusic),
		OPCODE(o6_stopObjectScript),
		/* 78 */
		OPCODE(o6_panCameraTo),
		OPCODE(o6_actorFollowCamera),
		OPCODE(o6_setCameraAt),
		OPCODE(o6_loadRoom),
		/* 7C */
		OPCODE(o6_stopScript),
		OPCODE(o6_walkActorToObj),
		OPCODE(o6_walkActorTo),
		OPCODE(o6_putActorAtXY),
		/* 80 */
		OPCODE(o6_putActorAtObject),
		OPCODE(o6_faceActor),
		OPCODE(o6_animateActor),
		OPCODE(o6_doSentence),
		/* 84 */
		OPCODE(o72_pickupObject),
		OPCODE(o6_loadRoomWithEgo),
		OPCODE(o6_invalid),
		OPCODE(o6_getRandomNumber),
		/* 88 */
		OPCODE(o6_getRandomNumberRange),
		OPCODE(o6_invalid),
		OPCODE(o6_getActorMoving),
		OPCODE(o6_isScriptRunning),
		/* 8C */
		OPCODE(o7_getActorRoom),
		OPCODE(o6_getObjectX),
		OPCODE(o6_getObjectY),
		OPCODE(o6_getObjectOldDir),
		/* 90 */
		OPCODE(o6_getActorWalkBox),
		OPCODE(o6_getActorCostume),
		OPCODE(o6_findInventory),
		OPCODE(o6_getInventoryCount),
		/* 94 */
		OPCODE(o6_getVerbFromXY),
		OPCODE(o6_beginOverride),
		OPCODE(o6_endOverride),
		OPCODE(o6_setObjectName),
		/* 98 */
		OPCODE(o6_isSoundRunning),
		OPCODE(o6_setBoxFlags),
		OPCODE(o6_invalid),
		OPCODE(o7_resourceRoutines),
		/* 9C */
		OPCODE(o6_roomOps),
		OPCODE(o72_actorOps),
		OPCODE(o72_verbOps),
		OPCODE(o6_getActorFromXY),
		/* A0 */
		OPCODE(o6_findObject),
		OPCODE(o6_pseudoRoom),
		OPCODE(o6_getActorElevation),
		OPCODE(o6_getVerbEntrypoint),
		/* A4 */
		OPCODE(o72_arrayOps),
		OPCODE(o6_saveRestoreVerbs),
		OPCODE(o6_drawBox),
		OPCODE(o6_pop),
		/* A8 */
		OPCODE(o6_getActorWidth),
		OPCODE(o6_wait),
		OPCODE(o6_getActorScaleX),
		OPCODE(o6_getActorAnimCounter1),
		/* AC */
		OPCODE(o6_invalid),
		OPCODE(o6_isAnyOf),
		OPCODE(o7_quitPauseRestart),
		OPCODE(o6_isActorInBox),
		/* B0 */
		OPCODE(o6_delay),
		OPCODE(o6_delaySeconds),
		OPCODE(o6_delayMinutes),
		OPCODE(o6_stopSentence),
		/* B4 */
		OPCODE(o6_printLine),
		OPCODE(o6_printCursor),
		OPCODE(o6_printDebug),
		OPCODE(o6_printSystem),
		/* B8 */
		OPCODE(o6_printActor),
		OPCODE(o6_printEgo),
		OPCODE(o6_talkActor),
		OPCODE(o6_talkEgo),
		/* BC */
		OPCODE(o72_dimArray),
		OPCODE(o6_dummy),
		OPCODE(o6_startObjectQuick),
		OPCODE(o6_startScriptQuick2),
		/* C0 */
		OPCODE(o72_dim2dimArray),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* C4 */
		OPCODE(o6_abs),
		OPCODE(o6_distObjectObject),
		OPCODE(o6_distObjectPt),
		OPCODE(o6_distPtPt),
		/* C8 */
		OPCODE(o6_kernelGetFunctions),
		OPCODE(o7_kernelSetFunctions),
		OPCODE(o6_delayFrames),
		OPCODE(o6_pickOneOf),
		/* CC */
		OPCODE(o6_pickOneOfDefault),
		OPCODE(o6_stampObject),
		OPCODE(o72_drawWizImage),
		OPCODE(o6_invalid),
		/* D0 */
		OPCODE(o6_getDateTime),
		OPCODE(o6_stopTalking),
		OPCODE(o6_getAnimateVariable),
		OPCODE(o6_invalid),
		/* D4 */
		OPCODE(o72_shuffle),
		OPCODE(o72_jumpToScript),
		OPCODE(o6_band),
		OPCODE(o6_bor),
		/* D8 */
		OPCODE(o6_isRoomScriptRunning),
		OPCODE(o6_closeFile),
		OPCODE(o72_openFile),
		OPCODE(o72_readFile),
		/* DC */
		OPCODE(o72_writeFile),
		OPCODE(o72_findAllObjects),
		OPCODE(o72_deleteFile),
		OPCODE(o6_rename),
		/* E0 */
		OPCODE(o6_soundOps),
		OPCODE(o72_getPixel),
		OPCODE(o6_localizeArray),
		OPCODE(o72_pickVarRandom),
		/* E4 */
		OPCODE(o6_setBoxSet),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* E8 */
		OPCODE(o6_invalid),
		OPCODE(o6_seekFilePos),
		OPCODE(o72_redimArray),
		OPCODE(o6_readFilePos),
		/* EC */
		OPCODE(o6_invalid),
		OPCODE(o72_unknownED),
		OPCODE(o7_stringLen),
		OPCODE(o72_unknownEF),
		/* F0 */
		OPCODE(o6_invalid),
		OPCODE(o72_unknownF1),
		OPCODE(o72_unknownF2),
		OPCODE(o72_readINI),
		/* F4 */
		OPCODE(o72_writeINI),
		OPCODE(o72_unknownF5),
		OPCODE(o72_unknownF6),
		OPCODE(o6_invalid),
		/* F8 */
		OPCODE(o72_unknownF8),
		OPCODE(o72_unknownF9),
		OPCODE(o72_unknownFA),
		OPCODE(o72_unknownFB),
		/* FC */
		OPCODE(o7_unknownFC),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
	};

	_opcodesV72he = opcodes;
}

void ScummEngine_v72he::executeOpcode(byte i) {
	OpcodeProcV72he op = _opcodesV72he[i].proc;
	(this->*op) ();
}

const char *ScummEngine_v72he::getOpcodeDesc(byte i) {
	return _opcodesV72he[i].desc;
}

static int arrayDataSizes[] = {0, 1, 4, 8, 8, 16, 32};

ScummEngine_v72he::ArrayHeader *ScummEngine_v72he::defineArray(int array, int type, int dim2start, int dim2end,
											int dim1start, int dim1end) {
	int id;
	int size;
	ArrayHeader *ah;
	
	assert(dim2start >= 0 && dim2start <= dim2end);
	assert(dim1start >= 0 && dim1start <= dim1end);
	assert(0 <= type && type <= 6);

	
	if (type == kBitArray || type == kNibbleArray)
		type = kByteArray;

	nukeArray(array);

	id = findFreeArrayId();

	debug(5,"defineArray (array %d, dim2start %d, dim2end %d dim1start %d dim1end %d", id, dim2start, dim2end, dim1start, dim1end);

	if (array & 0x80000000) {
		error("Can't define bit variable as array pointer");
	}

	size = arrayDataSizes[type];

	writeVar(array, id);

	size *= dim2end - dim2start + 1;
	size *= dim1end - dim1start + 1;
	size >>= 3;

	ah = (ArrayHeader *)createResource(rtString, id, size + sizeof(ArrayHeader));

	ah->type = TO_LE_32(type);
	ah->dim1start = TO_LE_32(dim1start);
	ah->dim1end = TO_LE_32(dim1end);
	ah->dim2start = TO_LE_32(dim2start);
	ah->dim2end = TO_LE_32(dim2end);

	return ah;
}

int ScummEngine_v72he::readArray(int array, int idx2, int idx1) {
	debug(5, "readArray (array %d, idx2 %d, idx1 %d)", readVar(array), idx2, idx1);

	if (readVar(array) == 0)
		error("readArray: Reference to zeroed array pointer");

	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, readVar(array));

	if (ah == NULL || ah->data == NULL)
		error("readArray: invalid array %d (%d)", array, readVar(array));

	if (idx2 < (int)FROM_LE_32(ah->dim2start) || idx2 > (int)FROM_LE_32(ah->dim2end) || 
		idx1 < (int)FROM_LE_32(ah->dim1start) || idx1 > (int)FROM_LE_32(ah->dim1end)) {
		error("readArray: array %d out of bounds: [%d, %d] exceeds [%d..%d, %d..%d]",
			  array, idx1, idx2, FROM_LE_32(ah->dim1start), FROM_LE_32(ah->dim1end),
			  FROM_LE_32(ah->dim2start), FROM_LE_32(ah->dim2end));
	}

	const int offset = (FROM_LE_32(ah->dim1end) - FROM_LE_32(ah->dim1start) + 1) *
		(idx2 - FROM_LE_32(ah->dim2start)) - FROM_LE_32(ah->dim1start) + idx1;

	switch (FROM_LE_32(ah->type)) {
	case kByteArray:
	case kStringArray:
		return ah->data[offset];

	case kIntArray:
		return (int16)READ_LE_UINT16(ah->data + offset * 2);

	case kDwordArray:
		return (int32)READ_LE_UINT32(ah->data + offset * 4);
	}

	return 0;
}

void ScummEngine_v72he::writeArray(int array, int idx2, int idx1, int value) {
	debug(5, "writeArray (array %d, idx2 %d, idx1 %d, value %d)", readVar(array), idx2, idx1, value);

	if (readVar(array) == 0)
		error("writeArray: Reference to zeroed array pointer");

	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, readVar(array));

	if (!ah)
		error("writeArray: Invalid array (%d) reference", readVar(array));

	if (idx2 < (int)FROM_LE_32(ah->dim2start) || idx2 > (int)FROM_LE_32(ah->dim2end) || 
		idx1 < (int)FROM_LE_32(ah->dim1start) || idx1 > (int)FROM_LE_32(ah->dim1end)) {
		error("writeArray: array %d out of bounds: [%d, %d] exceeds [%d..%d, %d..%d]",
			  array, idx1, idx2, FROM_LE_32(ah->dim1start), FROM_LE_32(ah->dim1end),
			  FROM_LE_32(ah->dim2start), FROM_LE_32(ah->dim2end));
	}

	const int offset = (FROM_LE_32(ah->dim1end) - FROM_LE_32(ah->dim1start) + 1) *
		(idx2 - FROM_LE_32(ah->dim2start)) - FROM_LE_32(ah->dim1start) + idx1;

	switch (FROM_LE_32(ah->type)) {
	case kByteArray:
	case kStringArray:
		ah->data[offset] = value;
		break;

	case kIntArray:
		WRITE_LE_UINT16(ah->data + offset * 2, value);
		break;

	case kDwordArray:
		WRITE_LE_UINT32(ah->data + offset * 4, value);
		break;
	}
}

void ScummEngine_v72he::readArrayFromIndexFile() {
	int num;
	int a, b, c;

	while ((num = _fileHandle.readUint16LE()) != 0) {
		a = _fileHandle.readUint16LE();
		b = _fileHandle.readUint16LE();
		c = _fileHandle.readUint16LE();

		if (c == 1)
			defineArray(num, kBitArray, 0, a, 0, b);
		else
			defineArray(num, kDwordArray, 0, a, 0, b);
	}
}

int ScummEngine_v72he::copyScriptString(byte *dst) {
	int i = 0;
	byte b;

	int array = pop();
	if (array == -1) {
		int len = resStrLen(_stringBuffer) + 1;
		while (len--)
			*dst++ = _stringBuffer[i++];
	} else {
		writeVar(0, array);
		while (b = readArray(0, 0, i) != 0) {
			*dst++ = b;
			i++;
		}
	}
	*dst = 0;

	return i;
}

void ScummEngine_v72he::decodeScriptString(byte *dst, bool scriptString) {
	int args[31];
	int num = 0, val = 0;
	int len;
	byte chr, name[256];

	getStackList(args, ARRAYSIZE(args));
	pop();

	if (scriptString) {
		addMessageToStack(_scriptPointer, name, sizeof(name));
		len = resStrLen(_scriptPointer);
		_scriptPointer += len + 1;
	} else {
		len = copyScriptString(name);
	}

	while (len--) {
		chr = name[num++];
		if (chr == 0x25) {
			chr = name[num++];
			if (chr == 0x64)
				dst += snprintf((char *)dst, 5, "%d", args[val++]);
			else if (chr == 0x73)
				dst += addStringToStack(dst, 100, args[val++]);
			continue;
		}
		*dst++ = chr;
	}
	*dst = 0;
}

void ScummEngine_v72he::o72_pushDWord() {
	int a;
	if (*_lastCodePtr + sizeof(MemBlkHeader) != _scriptOrgPointer) {
		uint32 oldoffs = _scriptPointer - _scriptOrgPointer;
		getScriptBaseAddress();
		_scriptPointer = _scriptOrgPointer + oldoffs;
	}
	a = READ_LE_UINT32(_scriptPointer);
	_scriptPointer += 4;
	push(a);
}

void ScummEngine_v72he::o72_addMessageToStack() {
	_stringLength = resStrLen(_scriptPointer) + 1;
	addMessageToStack(_scriptPointer, _stringBuffer, _stringLength);

	debug(0,"o72_addMessageToStack(\"%s\")", _scriptPointer);

	_scriptPointer += _stringLength;
}

void ScummEngine_v72he::o72_wordArrayRead() {
	int base = pop();
	push(readArray(fetchScriptWord(), 0, base));
}

void ScummEngine_v72he::o72_wordArrayIndexedRead() {
	int base = pop();
	int idx = pop();
	push(readArray(fetchScriptWord(), idx, base));
}

void ScummEngine_v72he::o72_compareStackList() {
	int args[128], i;
	int num = getStackList(args, ARRAYSIZE(args));
	int value = pop();

	if (num) {
		for (i = 1; i < num; i++) {
			if (args[i] == value) {
				push(1);
				break;
			}
		}
	} else {
		push(0);
	}
}

void ScummEngine_v72he::o72_unknown1C() {
	// HE 90+ specific
	if (_heversion < 90)
		error("Invalid opcode '%x' at %x", _opcode, _scriptPointer - _scriptOrgPointer);

	// For Pajame Sam 2 demo
	// Incomplete
	int value = fetchScriptByte();
	value -= 46;

	if (value == 10) {
		pop();
		pop();
		pop();
		pop();
		pop();
	}

	warning("o72_unknown1C stub (%d)", value);
}

void ScummEngine_v72he::o72_wordArrayWrite() {
	int a = pop();
	writeArray(fetchScriptWord(), 0, pop(), a);
}

void ScummEngine_v72he::o72_wordArrayIndexedWrite() {
	int val = pop();
	int base = pop();
	writeArray(fetchScriptWord(), pop(), base, val);
}

void ScummEngine_v72he::o72_unknown50() {
	int idx;

	idx = vm.cutSceneStackPointer;
	vm.cutSceneStackPointer = 0;
	vm.cutScenePtr[idx] = 0;
	vm.cutSceneScript[idx] = 0;

	VAR(VAR_OVERRIDE) = 0;
}

void ScummEngine_v72he::o72_findObject() {
	int args[16];

	getStackList(args, ARRAYSIZE(args));
	int y = pop();
	int x = pop();
	int r = findObject(x, y);
	push(r);
}

void ScummEngine_v72he::o72_wordArrayInc() {
	int var = fetchScriptWord();
	int base = pop();
	writeArray(var, 0, base, readArray(var, 0, base) + 1);
}

void ScummEngine_v72he::o72_objectX() {
	int object = pop();
	int objnum = getObjectIndex(object);

	if (objnum == -1) {
		push(0);
		return;
	}

	push(_objs[objnum].x_pos);
}


void ScummEngine_v72he::o72_objectY() {
	int object = pop();
	int objnum = getObjectIndex(object);

	if (objnum == -1) {
		push(0);
		return;
	}

	push(_objs[objnum].y_pos);
}

void ScummEngine_v72he::o72_getTimer() {
	int timer = pop();
	int cmd = fetchScriptByte();

	if (cmd == 10) {
		checkRange(3, 1, timer, "o72_getTimer: Timer %d out of range(%d)");
		int diff = _system->get_msecs() - _timers[timer];
		push(diff);
	} else {
		push(0);
	}
}

void ScummEngine_v72he::o72_setTimer() {
	int timer = pop();
	int cmd = fetchScriptByte();

	if (cmd == 158) {
		checkRange(3, 1, timer, "o72_setTimer: Timer %d out of range(%d)");
		_timers[timer] = _system->get_msecs();
	} else {
		error("TIMER command %d?", cmd);
	}
}

void ScummEngine_v72he::o72_unknown5A() {
	int value = pop();
	push(4);
	warning("o72_unknown5A stub (%d)", value);
}

void ScummEngine_v72he::o72_wordArrayDec() {
	int var = fetchScriptWord();
	int base = pop();
	writeArray(var, 0, base, readArray(var, 0, base) - 1);
}

void ScummEngine_v72he::o72_startScript() {
	int args[16];
	int script, flags;

	getStackList(args, ARRAYSIZE(args));
	script = pop();
	flags = fetchScriptByte();
	
	runScript(script, (flags == 199 || flags == 200), (flags == 195 || flags == 200), args);
}

void ScummEngine_v72he::o72_startObject() {
	int args[16];
	int script, entryp;
	int flags;
	getStackList(args, ARRAYSIZE(args));
	entryp = pop();
	script = pop();
	flags = fetchScriptByte();
	runObjectScript(script, entryp, (flags == 199 || flags == 200), (flags == 195 || flags == 200), args);
}

void ScummEngine_v72he::o72_drawObject() {
	int subOp = fetchScriptByte();
	int state = 0, y = -1, x = -1;

	switch (subOp) {
	case 62:
		state = pop();
		y = pop();
		x = pop();
		break;
	case 63:
		state = pop();
		if (state == 0)
			state = 1;
		break;
	case 65:
		state = 1;
		y = pop();
		x = pop();
		break;
	default:
		error("o72_drawObject: default case %d", subOp);
	}

	int object = pop();
	int objnum = getObjectIndex(object);
	if (objnum == -1)
		return;

	if (y != -1 && x != -1) {
		_objs[objnum].x_pos = x * 8;
		_objs[objnum].y_pos = y * 8;
	}

	if (state != -1) {
		addObjectToDrawQue(objnum);
		putState(object, state);
	}
}

void ScummEngine_v72he::o72_printWizImage() {
	int resnum = pop();
	drawWizImage(rtImage, resnum, 0, 0, 4);
}

void ScummEngine_v72he::o72_getArrayDimSize() {
	int subOp = fetchScriptByte();
	int32 val1, val2;
	ArrayHeader *ah;

	switch (subOp) {
	case 1:
	case 3:
		ah = (ArrayHeader *)getResourceAddress(rtString, readVar(fetchScriptWord()));
		val1 = FROM_LE_32(ah->dim1end);
		val2 = FROM_LE_32(ah->dim1start);
		push(val1 - val2 + 1);
		break;
	case 2:
		ah = (ArrayHeader *)getResourceAddress(rtString, readVar(fetchScriptWord()));
		val1 = FROM_LE_32(ah->dim2end);
		val2 = FROM_LE_32(ah->dim2start);
		push(val1 - val2 + 1);
		break;
	default:
		error("o72_getArrayDimSize: default case %d", subOp);
	}
}

void ScummEngine_v72he::o72_getNumFreeArrays() {
	byte **addr = _baseArrays;
	int i, num = 0;

	for (i = 1; i < _numArray; i++) {
		if (!addr[i])
			num++;
	}

	push (num);
}

void ScummEngine_v72he::o72_pickupObject() {
	int obj, room;

	room = pop();
	obj = pop();
	if (room == 0)
		room = getObjectRoom(obj);

	addObjectToInventory(obj, room);
	putOwner(obj, VAR(VAR_EGO));
	runInventoryScript(obj);
}

void ScummEngine_v72he::o72_actorOps() {
	Actor *a;
	int i, j, k;
	int args[32];
	byte b;
	byte name[256];

	b = fetchScriptByte();
	if (b == 197) {
		_curActor = pop();
		return;
	}

	a = derefActorSafe(_curActor, "o6_actorOps");
	if (!a)
		return;

	switch (b) {
	case 21:
		// HE 7.3 (Pajama Sam onwards)
		k = getStackList(args, ARRAYSIZE(args));
		break;
	case 64:
		_actorClipOverride.bottom = pop();
		_actorClipOverride.right = pop();
		_actorClipOverride.top = pop();
		_actorClipOverride.left = pop();
		break;
	case 76:		// SO_COSTUME
		a->setActorCostume(pop());
		break;
	case 77:		// SO_STEP_DIST
		j = pop();
		i = pop();
		a->setActorWalkSpeed(i, j);
		break;
	case 78:		// SO_SOUND
		k = getStackList(args, ARRAYSIZE(args));
		for (i = 0; i < k; i++)
			a->sound[i] = args[i];
		break;
	case 79:		// SO_WALK_ANIMATION
		a->walkFrame = pop();
		break;
	case 80:		// SO_TALK_ANIMATION
		a->talkStopFrame = pop();
		a->talkStartFrame = pop();
		break;
	case 81:		// SO_STAND_ANIMATION
		a->standFrame = pop();
		break;
	case 82:		// SO_ANIMATION
		// dummy case in scumm6
		pop();
		pop();
		pop();
		break;
	case 83:		// SO_DEFAULT
		a->initActor(0);
		break;
	case 84:		// SO_ELEVATION
		a->setElevation(pop());
		break;
	case 85:		// SO_ANIMATION_DEFAULT
		a->initFrame = 1;
		a->walkFrame = 2;
		a->standFrame = 3;
		a->talkStartFrame = 4;
		a->talkStopFrame = 5;
		break;
	case 86:		// SO_PALETTE
		j = pop();
		i = pop();
		checkRange(255, 0, i, "Illegal palette slot %d");
		a->remapActorPaletteColor(i, j);
		break;
	case 87:		// SO_TALK_COLOR
		a->talkColor = pop();
		break;
	case 88:		// SO_ACTOR_NAME
		copyScriptString(name);
		loadPtrToResource(rtActorName, a->number, name);
		break;
	case 89:		// SO_INIT_ANIMATION
		a->initFrame = pop();
		break;
	case 91:		// SO_ACTOR_WIDTH
		a->width = pop();
		break;
	case 92:		// SO_SCALE
		i = pop();
		a->setScale(i, i);
		break;
	case 93:		// SO_NEVER_ZCLIP
		a->forceClip = 0;
		break;
	case 94:		// SO_ALWAYS_ZCLIP
		a->forceClip = pop();
		break;
	case 95:		// SO_IGNORE_BOXES
		a->ignoreBoxes = 1;
		a->forceClip = 0;
		if (a->isInCurrentRoom())
			a->putActor(a->_pos.x, a->_pos.y, a->room);
		break;
	case 96:		// SO_FOLLOW_BOXES
		a->ignoreBoxes = 0;
		a->forceClip = 0;
		if (a->isInCurrentRoom())
			a->putActor(a->_pos.x, a->_pos.y, a->room);
		break;
	case 97:		// SO_ANIMATION_SPEED
		a->setAnimSpeed(pop());
		break;
	case 98:		// SO_SHADOW
		a->shadow_mode = pop();
		break;
	case 99:		// SO_TEXT_OFFSET
		a->talkPosY = pop();
		a->talkPosX = pop();
		break;
	case 156:		// HE 7.2
		a->charset = pop();
		break;
	case 198:		// SO_ACTOR_VARIABLE
		i = pop();
		a->setAnimVar(pop(), i);
		break;
	case 215:		// SO_ACTOR_IGNORE_TURNS_ON
		a->ignoreTurns = true;
		break;
	case 216:		// SO_ACTOR_IGNORE_TURNS_OFF
		a->ignoreTurns = false;
		break;
	case 217:		// SO_ACTOR_NEW
		a->initActor(2);
		break;
	case 218:		
		{
			int top_actor = a->top;
			int bottom_actor = a->bottom;
			a->drawToBackBuf = true;
			a->needRedraw = true;
			a->drawActorCostume();
			a->drawToBackBuf = false;
			a->needRedraw = true;
			a->drawActorCostume();
			a->needRedraw = false;

			if (a->top > top_actor)
				a->top = top_actor;
			if (a->bottom < bottom_actor)
				a->bottom = bottom_actor;

		}
		break;
	case 219:
		a->drawToBackBuf = false;
		a->needRedraw = true;
		a->needBgReset = true;
		break;
	case 225:
		{
		byte string[128];
		copyScriptString(string);
		int slot = pop();

		int len = resStrLen(string) + 1;
		addMessageToStack(string, _queueTalkString[slot], len);

		_queueTalkPosX[slot] = a->talkPosX;
		_queueTalkPosY[slot] = a->talkPosY;
		_queueTalkColor[slot] = a->talkColor;
		break;
		}
	default:
		error("o72_actorOps: default case %d", b);
	}
}

void ScummEngine_v72he::o72_verbOps() {
	int slot, a, b;
	VerbSlot *vs;
	byte op;
	byte name[200];

	op = fetchScriptByte();
	if (op == 196) {
		_curVerb = pop();
		_curVerbSlot = getVerbSlot(_curVerb, 0);
		checkRange(_numVerbs - 1, 0, _curVerbSlot, "Illegal new verb slot %d");
		return;
	}
	vs = &_verbs[_curVerbSlot];
	slot = _curVerbSlot;
	switch (op) {
	case 124:		// SO_VERB_IMAGE
		a = pop();
		if (_curVerbSlot) {
			setVerbObject(_roomResource, a, slot);
			vs->type = kImageVerbType;
			vs->imgindex = a;
		}
		break;
	case 125:		// SO_VERB_NAME
		copyScriptString(name);
		loadPtrToResource(rtVerb, slot, name);
		vs->type = kTextVerbType;
		vs->imgindex = 0;
		break;
	case 126:		// SO_VERB_COLOR
		vs->color = pop();
		break;
	case 127:		// SO_VERB_HICOLOR
		vs->hicolor = pop();
		break;
	case 128:		// SO_VERB_AT
		vs->curRect.top = pop();
		vs->curRect.left = pop();
		break;
	case 129:		// SO_VERB_ON
		vs->curmode = 1;
		break;
	case 130:		// SO_VERB_OFF
		vs->curmode = 0;
		break;
	case 131:		// SO_VERB_DELETE
		slot = getVerbSlot(pop(), 0);
		killVerb(slot);
		break;
	case 132:		// SO_VERB_NEW
		slot = getVerbSlot(_curVerb, 0);
		if (slot == 0) {
			for (slot = 1; slot < _numVerbs; slot++) {
				if (_verbs[slot].verbid == 0)
					break;
			}
			if (slot == _numVerbs)
				error("Too many verbs");
			_curVerbSlot = slot;
		}
		vs = &_verbs[slot];
		vs->verbid = _curVerb;
		vs->color = 2;
		vs->hicolor = 0;
		vs->dimcolor = 8;
		vs->type = kTextVerbType;
		vs->charset_nr = _string[0]._default.charset;
		vs->curmode = 0;
		vs->saveid = 0;
		vs->key = 0;
		vs->center = 0;
		vs->imgindex = 0;
		break;
	case 133:		// SO_VERB_DIMCOLOR
		vs->dimcolor = pop();
		break;
	case 134:		// SO_VERB_DIM
		vs->curmode = 2;
		break;
	case 135:		// SO_VERB_KEY
		vs->key = pop();
		break;
	case 136:		// SO_VERB_CENTER
		vs->center = 1;
		break;
	case 137:		// SO_VERB_NAME_STR
		a = pop();
		if (a == 0) {
			loadPtrToResource(rtVerb, slot, (const byte *)"");
		} else {
			loadPtrToResource(rtVerb, slot, getStringAddress(a));
		}
		vs->type = kTextVerbType;
		vs->imgindex = 0;
		break;
	case 139:		// SO_VERB_IMAGE_IN_ROOM
		b = pop();
		a = pop();

		if (slot && a != vs->imgindex) {
			setVerbObject(b, a, slot);
			vs->type = kImageVerbType;
			vs->imgindex = a;
		}
		break;
	case 140:		// SO_VERB_BAKCOLOR
		vs->bkcolor = pop();
		break;
	case 255:
		drawVerb(slot, 0);
		verbMouseOver(0);
		break;
	default:
		error("o72_verbops: default case %d", op);
	}
}

void ScummEngine_v72he::o72_arrayOps() {
	byte subOp = fetchScriptByte();
	int array = fetchScriptWord();
	int b, c, d, len;
	ArrayHeader *ah;
	int list[128];

	switch (subOp) {
	case 7:			// SO_ASSIGN_STRING
		ah = defineArray(array, kStringArray, 0, 0, 0, 1024);
		copyScriptString(ah->data);
		break;
	case 194:			// SO_ASSIGN_STRING
		ah = defineArray(array, kStringArray, 0, 0, 0, 4096);
		decodeScriptString(ah->data);
		break;
	case 208:		// SO_ASSIGN_INT_LIST
		b = pop();
		c = pop();
		d = readVar(array);
		if (d == 0) {
			defineArray(array, kDwordArray, 0, 0, 0, b + c);
		}
		while (--c) {
			writeArray(array, 0, b + c, pop());
		}
		break;
	case 212:		// SO_ASSIGN_2DIM_LIST
		len = getStackList(list, ARRAYSIZE(list));
		d = readVar(array);
		if (d == 0)
			error("Must DIM a two dimensional array before assigning");
		c = pop();
		while (--len >= 0) {
			writeArray(array, c, len, list[len]);
		}
		break;
	default:
		error("o72_arrayOps: default case %d (array %d)", subOp, array);
	}
}

void ScummEngine_v72he::o72_dimArray() {
	int data;
	int type = fetchScriptByte();

	switch (type) {
	case 2:		// SO_BIT_ARRAY
		data = kBitArray;
		break;
	case 3:		// SO_NIBBLE_ARRAY
		data = kNibbleArray;
		break;
	case 4:		// SO_BYTE_ARRAY
		data = kByteArray;
		break;
	case 5:		// SO_INT_ARRAY
		data = kIntArray;
		break;
	case 6:
		data = kDwordArray;
		break;
	case 7:		// SO_STRING_ARRAY
		data = kStringArray;
		break;
	case 204:		// SO_UNDIM_ARRAY
		nukeArray(fetchScriptWord());
		return;
	default:
		error("o72_dimArray: default case %d", type);
	}

	defineArray(fetchScriptWord(), data, 0, 0, 0, pop());
}


void ScummEngine_v72he::o72_dim2dimArray() {
	int a, b, data;
	int type = fetchScriptByte();
	switch (type) {
	case 2:		// SO_BIT_ARRAY
		data = kBitArray;
		break;
	case 3:		// SO_NIBBLE_ARRAY
		data = kNibbleArray;
		break;
	case 4:		// SO_BYTE_ARRAY
		data = kByteArray;
		break;
	case 5:		// SO_INT_ARRAY
		data = kIntArray;
		break;
	case 6:		
		data = kDwordArray;
		break;
	case 7:		// SO_STRING_ARRAY
		data = kStringArray;
		break;
	default:
		error("o72_dim2dimArray: default case %d", type);
	}

	b = pop();
	a = pop();
	defineArray(fetchScriptWord(), data, 0, a, 0, b);
}

void ScummEngine_v72he::drawWizImage(int restype, int resnum, int x1, int y1, int flags) {
	const uint8 *dataPtr = getResourceAddress(restype, resnum);
	if (dataPtr) {
		const uint8 *wizh = findResourceData(MKID('WIZH'), dataPtr);
		assert(wizh);
		uint32 comp   = READ_LE_UINT32(wizh + 0x0);
		uint32 width  = READ_LE_UINT32(wizh + 0x4);
		uint32 height = READ_LE_UINT32(wizh + 0x8);
		if (comp != 1) {
			warning("%d has invalid compression type %d", resnum, comp);
		}
		const uint8 *wizd = findResourceData(MKID('WIZD'), dataPtr);
		assert(wizd);
		if (flags & 1) {
			const uint8 *pal = findResourceData(MKID('RGBS'), dataPtr);
			assert(pal);
			setPaletteFromPtr(pal, 256);
		}
		if (flags & 2) {
			warning("unhandled Wiz image w/ rmap");
		}
		if (flags & 4) {
			warning("printing Wiz image is unimplemented");
			return;
		}

		uint8 *dst;
		VirtScreen *pvs = &virtscr[kMainVirtScreen];
		if (flags & 0x10) {
			dst = pvs->getPixels(0, pvs->topline);
		} else {
			dst = pvs->getBackPixels(0, pvs->topline);
		}
		Common::Rect rScreen(0, 0, pvs->w, pvs->h);
		if (flags & 2) {			
			warning("unhandled Wiz image w/ rmap");
		} else {
			copyWizImage(dst, wizd, pvs->w, pvs->h, x1, y1, width, height, &rScreen);
		}

		Common::Rect rImage(x1, y1, x1 + width, y1 + height);
		if (rImage.intersects(rScreen)) {
			rImage.clip(rScreen);
			if (flags & 0x18) {
				++rImage.bottom;
				markRectAsDirty(kMainVirtScreen, rImage);
			} else {
				gdi.copyVirtScreenBuffers(rImage.left, rImage.top, rImage.right - 1, rImage.bottom - 1);
			}
		}
	}
}

void ScummEngine_v72he::redrawBGAreas() {
	ScummEngine_v7he::redrawBGAreas();
	flushWizBuffer();
}

void ScummEngine_v72he::flushWizBuffer() {
	for (int i = 0; i < _wizImagesNum; ++i) {
		WizImage *pwi = &_wizImages[i];
		drawWizImage(rtImage, pwi->resnum, pwi->x1, pwi->y1, pwi->flags);
	}
	_wizImagesNum = 0;
}

void ScummEngine_v72he::copyWizImage(uint8 *dst, const uint8 *src, int dst_w, int dst_h, int src_x, int src_y, int src_w, int src_h, Common::Rect *rect) {
	Common::Rect r1(0, 0, src_w, src_h), r2(src_x, src_y, src_x + src_w, src_y + src_h);
	Common::Rect r3;
	int diff;

	if (rect) {
		r3 = *rect;
		Common::Rect r4(0, 0, dst_w, dst_h);
		if (!r3.intersects(r4)) {
			return;
		} else {
			r3.clip(r4);
		}
	} else {
		r3 = Common::Rect(0, 0, dst_w, dst_h);
	}
	diff = r2.left - r3.left;
	if (diff < 0) {
		r1.left -= diff;
		r2.left -= diff;
	}
	diff = r2.right - r3.right;
	if (diff > 0) {
		r1.right -= diff;
		r2.right -= diff;
	}
	diff = r2.top - r3.top;
	if (diff < 0) {
		r1.top -= diff;
		r2.top -= diff;
	}
	diff = r2.bottom - r3.bottom;
	if (diff > 0) {
		r1.bottom -= diff;
		r2.bottom -= diff;
	}
	if (r1.isValidRect() && r2.isValidRect()) {
		gdi.decompressImageHE(dst, dst_w, &r2, src, &r1);
	}
}

void ScummEngine_v72he::o72_drawWizImage() {
	int flags = pop();
	int y1 = pop();
	int x1 = pop();
	int resnum = pop();
	if (_fullRedraw) {
		assert(_wizImagesNum < ARRAYSIZE(_wizImages));
		WizImage *pwi = &_wizImages[_wizImagesNum];
		pwi->resnum = resnum;
		pwi->x1 = x1;
		pwi->y1 = y1;
		pwi->flags = flags;
		++_wizImagesNum;
	} else {
		drawWizImage(rtImage, resnum, x1, y1, flags);
	}
}

void ScummEngine_v72he::shuffleArray(int num, int minIdx, int maxIdx) {
	int range = maxIdx - minIdx;
	int count = range * 2;

	// Shuffle the array 'num'
	while (count--) {
		// Determine two random elements...
		int rand1 = _rnd.getRandomNumber(range) + minIdx;
		int rand2 = _rnd.getRandomNumber(range) + minIdx;
		
		// ...and swap them
		int val1 = readArray(num, 0, rand1);
		int val2 = readArray(num, 0, rand2);
		writeArray(num, 0, rand1, val2);
		writeArray(num, 0, rand2, val1);
	}
}

void ScummEngine_v72he::o72_shuffle() {
	int b = pop();
	int a = pop();
	shuffleArray(fetchScriptWord(), a, b);
}

void ScummEngine_v72he::o72_jumpToScript() {
	int args[16];
	int script, flags;

	getStackList(args, ARRAYSIZE(args));
	script = pop();
	flags = fetchScriptByte();
	stopObjectCode();
	runScript(script, (flags == 199 || flags == 200), (flags == 195 || flags == 200), args);
}

void ScummEngine_v72he::o72_openFile() {
	int mode, slot, l, r;
	byte filename[100];

	mode = pop();
	copyScriptString(filename);
	debug(1,"File %s", filename);
	
	for (r = strlen((char*)filename); r != 0; r--) {
		if (filename[r - 1] == '\\')
			break;
	}
	
	slot = -1;
	for (l = 0; l < 17; l++) {
		if (_hFileTable[l].isOpen() == false) {
			slot = l;
			break;
		}
	}

	if (slot != -1) {
		if (mode == 1)
			_hFileTable[slot].open((char*)filename + r, File::kFileReadMode);
		else if (mode == 2)
			_hFileTable[slot].open((char*)filename + r, File::kFileWriteMode);
		else
			error("o6_openFile(): wrong open file mode %d", mode);

		if (_hFileTable[slot].isOpen() == false)
			slot = -1;

	}
	debug(1, "o72_openFile: slot %d, mode %d", slot, mode);
	push(slot);
}

int ScummEngine_v72he::readFileToArray(int slot, int32 size) {
	if (size == 0)
		size = _hFileTable[slot].size() - _hFileTable[slot].pos();

	writeVar(0, 0);

	ArrayHeader *ah = defineArray(0, kByteArray, 0, 0, 0, size);
	_hFileTable[slot].read(ah->data, size);

	return readVar(0);
}

void ScummEngine_v72he::o72_readFile() {
	int slot, val;
	int32 size;
	int subOp = fetchScriptByte();

	switch (subOp) {
	case 4:
		slot = pop();
		val = _hFileTable[slot].readByte();
		push(val);
		break;
	case 5:
		slot = pop();
		val = _hFileTable[slot].readUint16LE();
		push(val);
		break;
	case 6:
		slot = pop();
		val = _hFileTable[slot].readUint32LE();
		push(val);
		break;
	case 8:
		fetchScriptByte();
		size = pop();
		slot = pop();
		val = readFileToArray(slot, size);
		push(val);
		break;
	default:
		error("o72_readFile: default case %d", subOp);
	}
	debug(1, "o72_readFile: slot %d, subOp %d val %d", slot, subOp, val);
}

void ScummEngine_v72he::writeFileFromArray(int slot, int resID) {
	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, resID);
	int32 size = (FROM_LE_32(ah->dim1end) - FROM_LE_32(ah->dim1start) + 1) *
		(FROM_LE_32(ah->dim2end) - FROM_LE_32(ah->dim2start) + 1);

	_hFileTable[slot].write(ah->data, size);
}

void ScummEngine_v72he::o72_writeFile() {
	int16 resID = pop();
	int slot = pop();
	int subOp = fetchScriptByte();

	switch (subOp) {
	case 4:
		_hFileTable[slot].writeByte(resID);
		break;
	case 5:
		_hFileTable[slot].writeUint16LE(resID);
		break;
	case 6:
		_hFileTable[slot].writeUint32LE(resID);
		break;
	case 8:
		writeFileFromArray(slot, resID);
		break;
	default:
		error("o72_writeFile: default case %d", subOp);
	}
	debug(1, "o72_writeFile: slot %d, subOp %d", slot, subOp);
}

void ScummEngine_v72he::o72_findAllObjects() {
	int room = pop();
	int i = 1;

	if (room != _currentRoom)
		warning("o72_findAllObjects: current room is not %d", room);
	writeVar(0, 0);
	defineArray(0, kDwordArray, 0, 0, 0, _numLocalObjects + 1);
	writeArray(0, 0, 0, _numLocalObjects);
	
	while (i < _numLocalObjects) {
		writeArray(0, 0, i, _objs[i].obj_nr);
		i++;
	}
	
	push(readVar(0));
}

void ScummEngine_v72he::o72_deleteFile() {
	byte filename[100];

	copyScriptString(filename);

	debug(1, "stub o72_deleteFile(%s)", filename);
}

void ScummEngine_v72he::o72_getPixel() {
	byte area;
	int x = pop();
	int y = pop();
	int subOp = fetchScriptByte();

	if (subOp != 218 && subOp != 219)
		return;

	VirtScreen *vs = findVirtScreen(y);
	if (vs == NULL || x > _screenWidth - 1 || x < 0) {
		push(-1);
		return;
	}

	if (subOp == 218)
		area = *vs->getBackPixels(x, y - vs->topline);
	else
		area = *vs->getPixels(x, y - vs->topline);
	push(area);
}

void ScummEngine_v72he::o72_pickVarRandom() {
	int num;
	int args[100];
	int32 var_A;

	num = getStackList(args, ARRAYSIZE(args));
	int value = fetchScriptWord();

	if (readVar(value) == 0) {
		defineArray(value, kDwordArray, 0, 0, 0, num + 1);
		if (num > 0) {
			int16 counter = 0;
			do {
				writeArray(value, 0, counter + 1, args[counter]);
			} while (++counter < num);
		}

		shuffleArray(value, 1, num-1);
		writeArray(value, 0, 0, 2);
		push(readArray(value, 0, 1));
		return;
	}

	num = readArray(value, 0, 0);

	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, num);
	// FIXME
	if (!ah)
		var_A = 0;
	else
		var_A = FROM_LE_32(ah->dim1end);

	if (var_A-1 <= num) {
		int16 var_2 = readArray(value, 0, num - 1);
		shuffleArray(value, 1, num - 1);
		if (readArray(value, 0, 1) == var_2) {
			num = 2;
		} else {
			num = 1;
		}
	}

	writeArray(value, 0, 0, num + 1);
	push(readArray(value, 0, num));
}

void ScummEngine_v72he::o72_redimArray() {
	int subcode, newX, newY;
	newY = pop();
	newX = pop();

	subcode = fetchScriptByte();
	switch (subcode) {
	case 5:
		redimArray(fetchScriptWord(), 0, newX, 0, newY, kIntArray);
		break;
	case 4:
		redimArray(fetchScriptWord(), 0, newX, 0, newY, kByteArray);
		break;
	case 6:
		redimArray(fetchScriptWord(), 0, newX, 0, newY, kDwordArray);
		break;
	default:
		error("o72_redimArray: default type %d", subcode);
	}
}

void ScummEngine_v72he::redimArray(int arrayId, int newDim2start, int newDim2end, 
								   int newDim1start, int newDim1end, int type) {
	int newSize, oldSize;

	if (readVar(arrayId) == 0)
		error("redimArray: Reference to zeroed array pointer");

	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, readVar(arrayId));

	if (!ah)
		error("redimArray: Invalid array (%d) reference", readVar(arrayId));

	newSize = arrayDataSizes[type];
	oldSize = arrayDataSizes[FROM_LE_32(ah->type)];

	newSize *= (newDim1end - newDim1start + 1) * (newDim2end - newDim2start + 1);
	oldSize *= (FROM_LE_32(ah->dim1end) - FROM_LE_32(ah->dim1start) + 1) *
		(FROM_LE_32(ah->dim2end) - FROM_LE_32(ah->dim2start) + 1);

	newSize >>= 3;
	oldSize >>= 3;

	if (newSize != oldSize)
		error("redimArray: array %d redim mismatch", readVar(arrayId));

	ah->type = TO_LE_32(type);
	ah->dim1start = TO_LE_32(newDim1start);
	ah->dim1end = TO_LE_32(newDim1end);
	ah->dim2start = TO_LE_32(newDim2start);
	ah->dim2end = TO_LE_32(newDim2end);
}


void ScummEngine_v72he::o72_unknownED() {
	int array, pos, len;
	int letter = 0, result = 0;

	len = pop();
	pos = pop();
	array = pop();

	if (len == -1) {
		pos = 0;
		len = resStrLen(getStringAddress(array));
	}

	writeVar(0, array);
	while (pos <= len) {
		letter = readArray(0, 0, pos);
		if (letter)
			result += getCharsetOffset(letter);
		pos++;
	}

	push(result);
	debug(1,"stub o72_unknownED");
}

void ScummEngine_v72he::o72_unknownEF() {
	int value;
	int array, array2, len, len2, len3, offset;
	int b, size;
	len = pop();
	b = pop();
	array2 = pop();

	size = len - b + 2;

	writeVar(0, 0);
	defineArray(0, kStringArray, 0, 0, 0, size);
	writeArray(0, 0, 0, 0);

	array = readVar(0);

	len2 = len;
	if (len == -1) {
		len2 = resStrLen(getStringAddress(array2));
		len = 0;
	} else {
		len = b;
	}
	len3 = resStrLen(getStringAddress(array));

	offset = 0;
	len2 -= len;
	len2++;
	while (offset < len2) {
		writeVar(0, array2);
		value = readArray(0, 0, offset + len);
		writeVar(0, array);
		writeArray(0, 0, offset + len3, value);
		offset++;
	}

	writeArray(0, 0, len3 + offset, 0);

	push(array);
	debug(1,"stub o72_unknownEF (array %d, array2 %d)", array, array2);
}

void ScummEngine_v72he::o72_unknownF1() {
	int a = pop();
	int b = pop();
	debug(1,"o7_unknownF1 stub (%d, %d)", b, a);
	push(-1);
}

void ScummEngine_v72he::o72_unknownF2() {
	int a = pop();
	int b = pop();
	debug(1,"o7_unknownF2 stub (%d, %d)", b, a);
	push(-1);
}

void ScummEngine_v72he::o72_readINI() {
	byte option[100];
	int type, retval;

	// we pretend that we don't have .ini file
	copyScriptString(option);
	type = fetchScriptByte();

	switch (type) {
	case 6: // number
		if (!strcmp((char *)option, "ReadPagesAutomatically"))
			push(1);
		else if (!strcmp((char *)option, "NoPrinting"))
			push(1);
		else
			push(0);
		break;
	case 7: // string
		defineArray(0, kStringArray, 0, 0, 0, 0);
		retval = readVar(0);
		writeArray(0, 0, 0, 0);
		push(retval); // var ID string
		break;
	default:
		error("o72_readINI: default type %d", type);
	}
	debug(1, "o72_readINI (%d) %s", type, option);
}

void ScummEngine_v72he::o72_writeINI() {
	int type, value;
	byte option[256], option2[1024];

	type = fetchScriptByte();

	switch (type) {
	case 6: // number
		value = pop();
		copyScriptString(option);
		debug(1,"o72_writeINI: %s set to %d", option, value);
		break;
	case 7: // string
		copyScriptString(option);
		copyScriptString(option2);
		debug(1,"o72_writeINI: %s set to %s", option, option2);
		break;
	default:
		error("o72_writeINI: default type %d", type);
	}
}

void ScummEngine_v72he::o72_unknownF5() {
	int letter, ebx;
	int array, len, pos, result = 0;
	ebx = pop();
	pos = pop();
	array = pop();

	len = resStrLen(getStringAddress(array));
	writeVar(0, array);

	while (pos < len) {
		letter = readArray(0, 0, pos);
		result += getCharsetOffset(letter);
		if (result >= ebx)
			break;
		pos++;
	}

	push(result);
	debug(1,"stub o72_unknownF5 (%d)", result);
}

void ScummEngine_v72he::o72_unknownF6() {
	int len, len2, pos, value, array;
	value = pop();
	len = pop();
	pos = pop();
	array = pop();

	if (len >= 0) {
		len2 = resStrLen(getStringAddress(array));
		if (len2 < len)
			len = len2;
	} else {
		len = 12;
	}

	if (pos < 0)
		pos = 0;

	writeVar(0, array);
	if (pos > len) {
		while (pos > len) {
			if (readArray(0, 0, pos) == value) {
				push(pos);
				return;
			}
			pos--;
		}
	} else {
		while (pos < len) {
			if (readArray(0, 0, pos) == value) {
				push(pos);
				return;
			}
			pos++;
		}
	}

	push(-1);
	debug(1,"stub o72_unknownF6");
}

void ScummEngine_v72he::o72_unknownF8() {
	int a = fetchScriptByte();
	push(1);

	debug(1,"stub o72_unknownF8(%d)", a);
}

void ScummEngine_v72he::o72_unknownF9() {
	// File related
	byte filename[100];
	copyScriptString(filename);
	debug(1,"o72_unknownF9: %s", filename);
}

void ScummEngine_v72he::o72_unknownFA() {
	byte name[100];
	copyScriptString(name);
	int id = fetchScriptByte();

	debug(1,"o72_unknownFA: (%d) %s", id, name);
}

void ScummEngine_v72he::o72_unknownFB() {
	byte b;
	b = fetchScriptByte();

	switch (b) {
	case 246:
	case 248:
		pop();
		pop();
		pop();
		pop();
		pop();
		pop();
		pop();
		pop();
		pop();
		break;
	case 247:
		pop();
		pop();
		break;
	}
	debug(1, "o72_unknownFB stub");
}

void ScummEngine_v72he::decodeParseString(int m, int n) {
	byte b;
	int i, color;
	int args[31];
	byte name[1024];

	b = fetchScriptByte();

	switch (b) {
	case 65:		// SO_AT
		_string[m].ypos = pop();
		_string[m].xpos = pop();
		_string[m].overhead = false;
		break;
	case 66:		// SO_COLOR
		_string[m].color = pop();
		break;
	case 67:		// SO_CLIPPED
		_string[m].right = pop();
		break;
	case 69:		// SO_CENTER
		_string[m].center = true;
		_string[m].overhead = false;
		break;
	case 71:		// SO_LEFT
		_string[m].center = false;
		_string[m].overhead = false;
		break;
	case 72:		// SO_OVERHEAD
		_string[m].overhead = true;
		_string[m].no_talk_anim = false;
		break;
	case 73:		// SO_SAY_VOICE
		error("decodeParseString: case 73");
		break;
	case 74:		// SO_MUMBLE
		_string[m].no_talk_anim = true;
		break;
	case 75:		// SO_TEXTSTRING
		switch (m) {
		case 0:
			actorTalk(_scriptPointer);
			break;
		case 1:
			drawString(1, _scriptPointer);
			break;
		case 2:
			unkMessage1(_scriptPointer);
			break;
		case 3:
			unkMessage2(_scriptPointer);
			break;
		}
		_scriptPointer += resStrLen(_scriptPointer) + 1;

		break;
	case 194:
		decodeScriptString(name, true);
		switch (m) {
		case 0:
			actorTalk(name);
			break;
		case 1:
			drawString(1, name);
			break;
		case 2:
			unkMessage1(name);
			break;
		case 3:
			unkMessage2(name);
			break;
		}
		break;
	case 0xF9:
		color = pop();
		if (color == 1) {
			_string[m].color = pop();
		} else {	
			push(color);
			getStackList(args, ARRAYSIZE(args));
			for (i = 0; i < 16; i++)
				_charsetColorMap[i] = _charsetData[_string[1]._default.charset][i] = (unsigned char)args[i];
			_string[m].color = color;
		}
		break;
	case 0xFE:
		_string[m].loadDefault();
		if (n)
			_actorToPrintStrFor = pop();
		break;
	case 0xFF:
		_string[m].saveDefault();
		break;
	default:
		error("decodeParseString: default case 0x%x", b);
	}
}

} // End of namespace Scumm
