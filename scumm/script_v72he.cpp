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
		OPCODE(o72_pushDWordVar),
		OPCODE(o6_pushWordVar),
		/* 04 */
		OPCODE(o72_getString),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_wordArrayRead),
		/* 08 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_wordArrayIndexedRead),
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
		OPCODE(o6_invalid),
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
		OPCODE(o6_wordArrayWrite),
		/* 48 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_wordArrayIndexedWrite),
		/* 4C */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_wordVarInc),
		/* 50 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_wordArrayInc),
		/* 54 */
		OPCODE(o72_objectX),
		OPCODE(o72_objectY),
		OPCODE(o6_byteVarDec),
		OPCODE(o6_wordVarDec),
		/* 58 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_wordArrayDec),
		/* 5C */
		OPCODE(o6_if),
		OPCODE(o6_ifNot),
		OPCODE(o72_startScript),
		OPCODE(o6_startScriptQuick),
		/* 60 */
		OPCODE(o72_startObject),
		OPCODE(o72_drawObject),
		OPCODE(o72_unknown62),
		OPCODE(o72_getArrayDimSize),
		/* 64 */
		OPCODE(o6_invalid),
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
		OPCODE(o7_pickupObject),
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
		OPCODE(o6_actorOps),
		OPCODE(o6_verbOps),
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
		OPCODE(o6_kernelSetFunctions),
		OPCODE(o6_delayFrames),
		OPCODE(o6_pickOneOf),
		/* CC */
		OPCODE(o6_pickOneOfDefault),
		OPCODE(o6_stampObject),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* D0 */
		OPCODE(o6_getDateTime),
		OPCODE(o6_stopTalking),
		OPCODE(o6_getAnimateVariable),
		OPCODE(o6_invalid),
		/* D4 */
		OPCODE(o6_shuffle),
		OPCODE(o72_jumpToScript),
		OPCODE(o6_band),
		OPCODE(o6_bor),
		/* D8 */
		OPCODE(o6_isRoomScriptRunning),
		OPCODE(o6_closeFile),
		OPCODE(o6_openFile),
		OPCODE(o6_readFile),
		/* DC */
		OPCODE(o6_writeFile),
		OPCODE(o72_findAllObjects),
		OPCODE(o6_deleteFile),
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
		OPCODE(o6_invalid),
		OPCODE(o72_stringLen),
		OPCODE(o6_invalid),
		/* F0 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o72_readINI),
		/* F4 */
		OPCODE(o72_unknownF4),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* F8 */
		OPCODE(o6_invalid),
		OPCODE(o7_unknownF9),
		OPCODE(o72_unknownFA),
		OPCODE(o72_unknownFB),
		/* FC */
		OPCODE(o6_invalid),
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
	assert(0 <= type && type <= 5);

	
	if (type == kBitArray || type == kNibbleArray)
		type = kByteArray;

	nukeArray(array);

	id = findFreeArrayId();

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
	if (readVar(array) == 0)
		error("readArray: Reference to zeroed array pointer");

	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, readVar(array));

	if (ah == NULL || ah->data == NULL)
		error("readArray: invalid array %d (%d)", array, readVar(array));

	if (idx2 < FROM_LE_32(ah->dim2start) || idx2 >= FROM_LE_32(ah->dim2end) || 
		idx1 < FROM_LE_32(ah->dim1start) || idx1 >= FROM_LE_32(ah->dim2end)) {
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
	if (readVar(array) == 0)
		error("writeArray: Reference to zeroed array pointer");

	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, readVar(array));

	if (!ah)
		error("writeArray: Invalid array (%d) reference", readVar(array));

	if (idx2 < FROM_LE_32(ah->dim2start) || idx2 >= FROM_LE_32(ah->dim2end) || 
		idx1 < FROM_LE_32(ah->dim1start) || idx1 >= FROM_LE_32(ah->dim2end)) {
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

	case kIntArray:
		WRITE_LE_UINT16(ah->data + offset * 2, value);

	case kDwordArray:
		WRITE_LE_UINT32(ah->data + offset * 4, value);
	}
}

void ScummEngine_v72he::o72_pushDWordVar() {
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

void ScummEngine_v72he::o72_getString() {
	int len;
	
	len = resStrLen(_scriptPointer);
	warning("stub o72_getString(\"%s\")", _scriptPointer);
	_scriptPointer += len;
	fetchScriptWord();
	fetchScriptWord();
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
	default:
		warning("o72_drawObject: default case %d", subOp);
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

void ScummEngine_v72he::o72_unknown62() {
	int a = pop();
	// unknown62(a, 0, 0, 4);
	warning("o72_unknown62 stub (%d)", a);
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
		len = resStrLen(_scriptPointer);
		ah = defineArray(array, kStringArray, 0, 0, 0, len + 1);
		copyScriptString(ah->data);
		break;
	case 205:		// SO_ASSIGN_STRING
		b = pop();
		len = resStrLen(_scriptPointer);
		ah = defineArray(array, kStringArray, 0, 0, 0, len + 1);
		copyScriptString(ah->data + b);
		break;
	case 208:		// SO_ASSIGN_INT_LIST
		b = pop();
		c = pop();
		d = readVar(array);
		if (d == 0) {
			defineArray(array, kIntArray, 0, 0, 0, b + c);
		}
		while (c--) {
			writeArray(array, 0, b + c, pop());
		}
		break;
	case 212:		// SO_ASSIGN_2DIM_LIST
		b = pop();
		len = getStackList(list, ARRAYSIZE(list));
		d = readVar(array);
		if (d == 0)
			error("Must DIM a two dimensional array before assigning");
		c = pop();
		while (--len >= 0) {
			writeArray(array, c, b + len, list[len]);
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
	case 5:		// SO_INT_ARRAY
		data = kIntArray;
		break;
	case 2:		// SO_BIT_ARRAY
		data = kBitArray;
		break;
	case 3:		// SO_NIBBLE_ARRAY
		data = kNibbleArray;
		break;
	case 4:		// SO_BYTE_ARRAY
		data = kByteArray;
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

void ScummEngine_v72he::o72_jumpToScript() {
	int args[16];
	int script, flags;

	getStackList(args, ARRAYSIZE(args));
	script = pop();
	flags = fetchScriptByte();
	stopObjectCode();
	runScript(script, (flags == 199 || flags == 200), (flags == 195 || flags == 200), args);
}

void ScummEngine_v72he::o72_findAllObjects() {
	int a = pop();
	int i = 1;

	if (a != _currentRoom)
		warning("o72_findAllObjects: current room is not %d", a);
	writeVar(0, 0);
	defineArray(0, kDwordArray, 0, 0, 0, _numLocalObjects + 1);
	writeArray(0, 0, 0, _numLocalObjects);
	
	while (i < _numLocalObjects) {
		writeArray(0, 0, i, _objs[i].obj_nr);
		i++;
	}
	
	push(readVar(0));
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
		break;
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

	newSize *= (newDim1end - newDim1start + 1) * (newDim2end - newDim2end + 1);
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


void ScummEngine_v72he::o72_stringLen() {
	int a, len;
	byte *addr;

	a = pop();

	addr = getStringAddress(a);
	if (!addr) {
		// FIXME: should be error here
		warning("o72_stringLen: Reference to zeroed array pointer (%d)", a);
		push(0);
		return;
	}

	len = strlen((char *)getStringAddress(a));
	push(len);
}

void ScummEngine_v72he::o72_readINI() {
	int type;
	int retval;

	// we pretend that we don't have .ini file
	type = fetchScriptByte();
	switch (type) {
	case 6: // number
		push(0);
		break;
	case 7: // string
		defineArray(0, kStringArray, 0, 0, 0, 0);
		retval = readVar(0);
		writeArray(0, 0, 0, 0);
		push(retval); // var ID string
		break;
	default:
		warning("o72_readINI(..., %d): read-ini string not implemented", type);
	}
}

void ScummEngine_v72he::o72_unknownF4() {
	byte b;
	b = fetchScriptByte();

	switch (b) {
	case 6:
		pop();
		break;
	case 7:
		break;
	}
	warning("o72_unknownF4 stub");
}

void ScummEngine_v72he::o72_unknownFA() {
	warning("stub o72_unknownFA(%d)", fetchScriptByte());
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
	warning("o72_unknownFB stub");
}

} // End of namespace Scumm
