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
		OPCODE(o72_isAnyOf),
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
		OPCODE(o72_unknown50),
		OPCODE(o6_invalid),
		OPCODE(o72_findObjectWithClassOf),
		OPCODE(o72_wordArrayInc),
		/* 54 */
		OPCODE(o72_objectX),
		OPCODE(o72_objectY),
		OPCODE(o72_unknown56),
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
		OPCODE(o6_cursorCommand),
		/* 6C */
		OPCODE(o6_breakHere),
		OPCODE(o6_ifClassOfIs),
		OPCODE(o6_setClass),
		OPCODE(o6_getState),
		/* 70 */
		OPCODE(o60_setState),
		OPCODE(o6_setOwner),
		OPCODE(o6_getOwner),
		OPCODE(o6_jump),
		/* 74 */
		OPCODE(o70_startSound),
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
		OPCODE(o70_pickupObject),
		OPCODE(o6_loadRoomWithEgo),
		OPCODE(o6_invalid),
		OPCODE(o6_getRandomNumber),
		/* 88 */
		OPCODE(o6_getRandomNumberRange),
		OPCODE(o6_invalid),
		OPCODE(o6_getActorMoving),
		OPCODE(o6_isScriptRunning),
		/* 8C */
		OPCODE(o70_getActorRoom),
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
		OPCODE(o70_resourceRoutines),
		/* 9C */
		OPCODE(o72_roomOps),
		OPCODE(o72_actorOps),
		OPCODE(o72_verbOps),
		OPCODE(o6_getActorFromXY),
		/* A0 */
		OPCODE(o72_findObject),
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
		OPCODE(o60_wait),
		OPCODE(o6_getActorScaleX),
		OPCODE(o6_getActorAnimCounter1),
		/* AC */
		OPCODE(o6_invalid),
		OPCODE(o6_isAnyOf),
		OPCODE(o70_quitPauseRestart),
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
		OPCODE(o6_stopObjectCode),
		OPCODE(o6_startObjectQuick),
		OPCODE(o6_startScriptQuick2),
		/* C0 */
		OPCODE(o72_dim2dimArray),
		OPCODE(o72_traceStatus),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* C4 */
		OPCODE(o6_abs),
		OPCODE(o6_distObjectObject),
		OPCODE(o6_distObjectPt),
		OPCODE(o6_distPtPt),
		/* C8 */
		OPCODE(o60_kernelGetFunctions),
		OPCODE(o70_kernelSetFunctions),
		OPCODE(o6_delayFrames),
		OPCODE(o6_pickOneOf),
		/* CC */
		OPCODE(o6_pickOneOfDefault),
		OPCODE(o6_stampObject),
		OPCODE(o72_drawWizImage),
		OPCODE(o72_unknownCF),
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
		OPCODE(o60_closeFile),
		OPCODE(o72_openFile),
		OPCODE(o72_readFile),
		/* DC */
		OPCODE(o72_writeFile),
		OPCODE(o72_findAllObjects),
		OPCODE(o72_deleteFile),
		OPCODE(o60_rename),
		/* E0 */
		OPCODE(o60_soundOps),
		OPCODE(o72_getPixel),
		OPCODE(o60_localizeArray),
		OPCODE(o72_pickVarRandom),
		/* E4 */
		OPCODE(o6_setBoxSet),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* E8 */
		OPCODE(o6_invalid),
		OPCODE(o60_seekFilePos),
		OPCODE(o72_redimArray),
		OPCODE(o60_readFilePos),
		/* EC */
		OPCODE(o72_unknownEC),
		OPCODE(o72_unknownED),
		OPCODE(o70_stringLen),
		OPCODE(o72_unknownEF),
		/* F0 */
		OPCODE(o72_unknownF0),
		OPCODE(o72_unknownF1),
		OPCODE(o72_checkGlobQueue),
		OPCODE(o72_readINI),
		/* F4 */
		OPCODE(o72_writeINI),
		OPCODE(o72_unknownF5),
		OPCODE(o72_unknownF6),
		OPCODE(o6_invalid),
		/* F8 */
		OPCODE(o72_unknownF8),
		OPCODE(o72_setFilePath),
		OPCODE(o72_unknownFA),
		OPCODE(o70_polygonOps),
		/* FC */
		OPCODE(o70_polygonHit),
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

void ScummEngine_v72he::arrrays_unk2(int dst, int src, int len2, int len) {
	int edi, value;
	int i = 0;

	if (len == -1) {
		len = resStrLen(getStringAddress(src));
		len2 = 0;
	}

	edi = resStrLen(getStringAddress(dst));

	len -= len2;
	len++;

	while (i < len) {
		writeVar(0, src);
		value = readArray(0, 0, len2 + i);
		writeVar(0, dst);
		writeArray(0, 0, edi + i, value);
		i++;
	}

	writeArray(0, 0, edi + i, 0);
}

void ScummEngine_v72he::copyScriptString(byte *dst) {
	int i = 0;
	byte b;

	int array = pop();
	if (array == -1) {
		int len = resStrLen(_stringBuffer) + 1;
		while (len--)
			*dst++ = _stringBuffer[i++];
	} else {
		writeVar(0, array);
		while ((b = readArray(0, 0, i)) != 0) {
			*dst++ = b;
			i++;
		}
	}
	*dst = 0;
}

void ScummEngine_v72he::decodeScriptString(byte *dst, bool scriptString) {
	int args[31];
	int num = 0, val;
	int len, id;
	byte chr, string[256];
	memset(string, 0, sizeof(string));

	val = getStackList(args, ARRAYSIZE(args));
	args[val] = id = pop();

	if (scriptString) {
		addMessageToStack(_scriptPointer, string, sizeof(string));
		len = resStrLen(_scriptPointer);
		_scriptPointer += len + 1;
	} else {
		copyScriptString(string);
		len = resStrLen(string) + 1;
	}

	while (len--) {
		chr = string[num++];
		if (chr == '%') {
			chr = string[num++];
			if (chr == 'd') {
				dst += snprintf((char *)dst, 10, "%d", args[val--]);
				continue;
			} else if (chr == 's') {
				dst += addStringToStack(dst, 512, args[val--]);
				continue;
			}
		}
		*dst++ = chr;
	}
	*dst = 0;
}

int ScummEngine_v72he::findObject(int x, int y, int num, int *args) {
	int i, b, result;
	int cond, cls, tmp;
	byte a;
	const int mask = 0xF;

	for (i = 1; i < _numLocalObjects; i++) {
		result = 0;
		if ((_objs[i].obj_nr < 1) || getClass(_objs[i].obj_nr, kObjectClassUntouchable))
			continue;

		// Check polygon bounds
		if (polygonDefined(_objs[i].obj_nr)) {
			if (polygonHit(_objs[i].obj_nr, x, y) != 0)
				result = _objs[i].obj_nr;
			else if (VAR(VAR_POLYGONS_ONLY))
				continue;
		}

		if (!result) {
			// Check object bounds
			b = i;
			do {
				a = _objs[b].parentstate;
				b = _objs[b].parent;
				if (b == 0) {
					if (_objs[i].x_pos <= x && _objs[i].width + _objs[i].x_pos > x &&
					    _objs[i].y_pos <= y && _objs[i].height + _objs[i].y_pos > y)
						result = _objs[i].obj_nr;
					break;
				}
			} while ((_objs[b].state & mask) == a);
		}


		if (result) {
			if (!num)
				return result;

			// Check object class
			cond = 1;
			tmp = num;
			while (--tmp >= 0) {
				cls = args[tmp];
				b = getClass(i, cls);
				if ((cls & 0x80 && !b) || (!(cls & 0x80) && b))
					cond = 0;
			}

			if (cond)
				return result;
		}
	}

	return 0;
}

const byte *ScummEngine_v72he::findWrappedBlock(uint32 tag, const byte *ptr, int state, bool errorFlag) {
	if (READ_UINT32(ptr) == MKID('MULT')) {
		const byte *offs, *wrap;
		uint32 size;

		wrap = findResource(MKID('WRAP'), ptr);
		if (wrap == NULL)
			return NULL;

		offs = findResourceData(MKID('OFFS'), wrap);
		if (offs == NULL)
			return NULL;

		size = getResourceDataSize(offs) / 4;
		if ((uint32)state >= (uint32)size)
			return NULL;

		offs += READ_LE_UINT32(offs + state * sizeof(uint32));
		offs = findResourceData(tag, offs - 8);
;		if (offs)
			return offs;

		offs = findResourceData(MKID('DEFA'), ptr);
		if (offs == NULL)
			return NULL;

		return findResourceData(tag, offs - 8);
	} else {
		return findResourceData(tag, ptr);
	}
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

	// Filter out pointless trace messages, which often flood
	if (strcmp((char *)_stringBuffer, "no trace") && strcmp((char *)_stringBuffer, "trace on"))
		debug(0,"o72_addMessageToStack(\"%s\")", _scriptPointer);

	_scriptPointer += _stringLength;
}

void ScummEngine_v72he::o72_isAnyOf() {
	int args[128], i = 0;
	int num = getStackList(args, ARRAYSIZE(args));
	int value = pop();

	while (i < num) {
		if (args[i] == value) {
			push(1);
			return;
		}
		i++;
	}

	push(0);
}

void ScummEngine_v72he::o72_unknown50() {
	int idx;

	idx = vm.cutSceneStackPointer;
	vm.cutSceneStackPointer = 0;
	vm.cutScenePtr[idx] = 0;
	vm.cutSceneScript[idx] = 0;

	VAR(VAR_OVERRIDE) = 0;
}

void ScummEngine_v72he::o72_findObjectWithClassOf() {
	int args[16], num;

	num = getStackList(args, ARRAYSIZE(args));
	int y = pop();
	int x = pop();
	int r = findObject(x, y, num, args);
	push(r);
}

void ScummEngine_v72he::o72_wordArrayInc() {
	int var = fetchScriptWord();
	int base = pop();
	writeArray(var, 0, base, readArray(var, 0, base) + 1);
}

void ScummEngine_v72he::o72_wordArrayDec() {
	int var = fetchScriptWord();
	int base = pop();
	writeArray(var, 0, base, readArray(var, 0, base) - 1);
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

void ScummEngine_v72he::o72_unknown56() {
	// Drawing related
	int a = pop();
	int b = pop();
	int c = pop();
	int d = pop();
	int e = pop();

	debug(1, "stub o72_unknown56(%d, %d, %d, %d, %d)", a, b, c, d, e);
}

void ScummEngine_v72he::o72_getTimer() {
	int timer = pop();
	int cmd = fetchScriptByte();

	if (cmd == 10) {
		checkRange(3, 1, timer, "o72_getTimer: Timer %d out of range(%d)");
		int diff = _system->getMillis() - _timers[timer];
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
		_timers[timer] = _system->getMillis();
	} else {
		error("TIMER command %d?", cmd);
	}
}

void ScummEngine_v72he::o72_unknown5A() {
	// Seems to get length of sound already played
	int snd = pop();
	int r = _mixer->getChannelElapsedTime(_sound->_musicChannelHandle);

	push(r * 10);
	debug(1,"o72_unknown5A stub (%d)", snd);
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
	WizImage wi;
	wi.resNum = pop();
	wi.x1 = wi.y1 = 0;
	wi.state = 0;
	wi.flags = 4;
	drawWizImage(rtImage, &wi);
}

void ScummEngine_v72he::o72_getArrayDimSize() {
	int subOp = fetchScriptByte();
	int32 val1, val2;
	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, readVar(fetchScriptWord()));

	switch (subOp) {
	case 1:
	case 3:
		val1 = FROM_LE_32(ah->dim1end);
		val2 = FROM_LE_32(ah->dim1start);
		push(val1 - val2 + 1);
		break;
	case 2:
		val1 = FROM_LE_32(ah->dim2end);
		val2 = FROM_LE_32(ah->dim2start);
		push(val1 - val2 + 1);
		break;
	case 4:
		push(FROM_LE_32(ah->dim1start));
		break;
	case 5:
		push(FROM_LE_32(ah->dim1end));
		break;
	case 6:
		push(FROM_LE_32(ah->dim2start));
		break;
	case 7:
		push(FROM_LE_32(ah->dim2end));
		break;
	default:
		error("o72_getArrayDimSize: default case %d", subOp);
	}
}

void ScummEngine_v72he::o72_getNumFreeArrays() {
	byte **addr = res.address[rtString];
	int i, num = 0;

	for (i = 1; i < _numArray; i++) {
		if (!addr[i])
			num++;
	}

	push (num);
}

void ScummEngine_v72he::o72_roomOps() {
	int a, b, c, d, e;
	byte op;
	byte filename[100];

	op = fetchScriptByte();

	switch (op) {
	case 172:		// SO_ROOM_SCROLL
		b = pop();
		a = pop();
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

	case 174:		// SO_ROOM_SCREEN
		b = pop();
		a = pop();
		initScreens(a, _screenHeight);
		break;

	case 175:		// SO_ROOM_PALETTE
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		setPalColor(d, a, b, c);
		break;

	case 179:		// SO_ROOM_INTENSITY
		c = pop();
		b = pop();
		a = pop();
		darkenPalette(a, a, a, b, c);
		break;

	case 180:		// SO_ROOM_SAVEGAME
		_saveTemporaryState = true;
		_saveLoadSlot = pop();
		_saveLoadFlag = pop();
		break;

	case 181:		// SO_ROOM_FADE
		// Defaults to 1 but doesn't use fade effects
		a = pop();
		break;

	case 182:		// SO_RGB_ROOM_INTENSITY
		e = pop();
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		darkenPalette(a, b, c, d, e);
		break;

	case 213:		// SO_ROOM_NEW_PALETTE
		a = pop();
		setPalette(a);
		break;

	case 220:
		a = pop();
		b = pop();
		copyPalColor(a, b);
		break;

	case 221:
		copyScriptString(filename);
		_saveLoadFlag = pop();
		_saveLoadSlot = 1;
		_saveTemporaryState = true;
		break;

	case 234:
		b = pop();
		a = pop();
		swapObjects(a, b);
		break;

	case 236:
		b = pop();
		a = pop();
		warning("o72_roomOps: case %d (%d, %d)", op, b, a);
		break;

	default:
		error("o72_roomOps: default case %d", op);
	}
}

void ScummEngine_v72he::o72_actorOps() {
	Actor *a;
	int i, j, k;
	int args[32];
	byte b;
	byte string[256];

	b = fetchScriptByte();
	if (b == 197) {
		_curActor = pop();
		return;
	}

	a = derefActorSafe(_curActor, "o72_actorOps");
	if (!a)
		return;

	switch (b) {
	case 21: // HE 80+
		k = getStackList(args, ARRAYSIZE(args));
		for (i = 0; i < k; ++i) {
			a->setUserCondition(args[i] & 0x7F, args[i] & 0x80);
		}
		debug(1,"o72_actorOps: case 21 (%d)", k);
		break;
	case 24: // HE 80+
		k = pop();
		a->talkUnk = 1;
		a->setTalkCondition(k);
		debug(1,"o72_actorOps: case 24 (%d)", k);
		break;
	case 43: // HE 90+
		// Uses reverse order of layering, so we adjust
		a->_layer = -pop();
		break;
	case 64:
	case 67: // HE 99+
		_actorClipOverride.bottom = pop();
		_actorClipOverride.right = pop();
		_actorClipOverride.top = pop();
		_actorClipOverride.left = pop();
		break;
	case 65: // HE 98+
		i = pop();
		j = pop();
		a->putActor(i, j, a->room);
		break;
	case 68: // HE 90+
		k = pop();
		debug(1,"o72_actorOps: case 68 (%d)", k);
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
		a->_walkFrame = pop();
		break;
	case 80:		// SO_TALK_ANIMATION
		a->_talkStopFrame = pop();
		a->_talkStartFrame = pop();
		break;
	case 81:		// SO_STAND_ANIMATION
		a->_standFrame = pop();
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
		a->_initFrame = 1;
		a->_walkFrame = 2;
		a->_standFrame = 3;
		a->_talkStartFrame = 4;
		a->_talkStopFrame = 5;
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
		copyScriptString(string);
		loadPtrToResource(rtActorName, a->number, string);
		break;
	case 89:		// SO_INIT_ANIMATION
		a->_initFrame = pop();
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
		a->_shadowMode = pop();
		debug(0, "Set actor XMAP idx to %d", a->_shadowMode);
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
		copyScriptString(string);
		int slot = pop();

		int len = resStrLen(string) + 1;
		addMessageToStack(string, a->talkQueue[slot].sentence, len);

		a->talkQueue[slot].posX = a->talkPosX;
		a->talkQueue[slot].posY = a->talkPosY;
		a->talkQueue[slot].color = a->talkColor;
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

void ScummEngine_v72he::o72_findObject() {
	int y = pop();
	int x = pop();
	int r = findObject(x, y, 0, 0);
	push(r);
}

void ScummEngine_v72he::o72_arrayOps() {
	byte subOp = fetchScriptByte();
	int array = fetchScriptWord();
	int offs, tmp, tmp2, tmp3;
	int dim1end, dim1start, dim2end, dim2start;
	int id, len, b, c;
	ArrayHeader *ah;
	int list[128];
	byte string[2048];


	debug(1,"o72_arrayOps: case %d", subOp);
	switch (subOp) {
	case 7:			// SO_ASSIGN_STRING
		copyScriptString(string);
		len = resStrLen(string) + 1;
		ah = defineArray(array, kStringArray, 0, 0, 0, len);
		memcpy(ah->data, string, len);
		break;

	case 126:
		len = getStackList(list, ARRAYSIZE(list));
		dim1end = pop();
		dim1start = pop();
		dim2end = pop();
		dim2start = pop();
		id = readVar(array);
		if (id == 0) {
			defineArray(array, kDwordArray, dim2start, dim2end, dim1start, dim1end);
		}
		tmp2 = len;
		while (dim2start <= dim2end) {
			tmp = dim1start;
			while (tmp <= dim1end) {
				writeArray(array, dim2start, tmp, list[--tmp2]);
				if (tmp2 == 0)
					tmp2 = len;
				tmp++;
			}
			dim2start++;
		}
		break;
	case 127:
		{
		// TODO
		// Array copy and cat?
		//Array1
		dim1end = pop();
		dim1start = pop();
		dim2end = pop();
		dim2start = pop();
		//Array2
		array = fetchScriptWord();
		dim1end = pop();
		dim1start = pop();
		dim2end = pop();
		dim2start = pop();
		}
		break;
	case 128:
		b = pop();
		c = pop();
		dim1end = pop();
		dim1start = pop();
		dim2end = pop();
		dim2start = pop();
		id = readVar(array);
		if (id == 0) {
			defineArray(array, kDwordArray, dim2start, dim2end, dim1start, dim1end);
		}

		len = c - b;
		len |= dim2end;
		len = len - dim2end + 1;
		offs = (b >= c) ? 1 : -1;
		tmp2 = c;
		tmp3 = len;
		while (dim2start <= dim2end) {
			tmp = dim1start;
			while (tmp <= dim1end) {
				writeArray(array, dim2start, tmp, tmp2);
				if (--tmp3 == 0) {
					tmp2 = c;
					tmp3 = len;
				} else {
					tmp2 += offs;
				}
				tmp++;
			}
			dim2start++;
		}
		break;
	case 194:			// SO_ASSIGN_STRING
		decodeScriptString(string);
		len = resStrLen(string) + 1;
		ah = defineArray(array, kStringArray, 0, 0, 0, len);
		memcpy(ah->data, string, len);
		break;
	case 208:		// SO_ASSIGN_INT_LIST
		b = pop();
		c = pop();
		id = readVar(array);
		if (id == 0) {
			defineArray(array, kDwordArray, 0, 0, 0, b + c);
		}
		while (c--) {
			writeArray(array, 0, b + c, pop());
		}
		break;
	case 212:		// SO_ASSIGN_2DIM_LIST
		len = getStackList(list, ARRAYSIZE(list));
		id = readVar(array);
		if (id == 0)
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

void ScummEngine_v72he::o72_traceStatus() {
	byte string[80];

	copyScriptString(string);
	pop();
}

void ScummEngine_v72he::displayWizImage(const WizImage *pwi) {
	if (_fullRedraw) {
		assert(_wizImagesNum < ARRAYSIZE(_wizImages));
		memcpy(&_wizImages[_wizImagesNum], pwi, sizeof(WizImage));
		++_wizImagesNum;
	} else if (pwi->flags & 0x40) {
		drawWizPolygon(pwi->resNum, pwi->state, pwi->x1, pwi->flags);
	} else {
		drawWizImage(rtImage, pwi);
	}
}

void ScummEngine_v72he::getWizImageDim(int resnum, int state, uint32 &w, uint32 &h) {
	const uint8 *dataPtr = getResourceAddress(rtImage, resnum);
	if (dataPtr) {
		const uint8 *wizh = findWrappedBlock(MKID('WIZH'), dataPtr, state, 0);
		w = READ_LE_UINT32(wizh + 0x4);
		h = READ_LE_UINT32(wizh + 0x8);
	} else {
		w = 0;
		h = 0;
	}
}

uint8 *ScummEngine_v72he::drawWizImage(int restype, const WizImage *pwi) {
	debug(1, "drawWizImage(%d, %d, %d, %d, 0x%X)", restype, pwi->resNum, pwi->x1, pwi->y1, pwi->flags);
	uint8 *dst = NULL;
	const uint8 *dataPtr = getResourceAddress(restype, pwi->resNum);
	if (dataPtr) {
		const uint8 *wizh = findWrappedBlock(MKID('WIZH'), dataPtr, pwi->state, 0);
		if (!wizh) {
			warning("WIZH not found");
			return 0;
		}
		uint32 comp   = READ_LE_UINT32(wizh + 0x0);
		uint32 width  = READ_LE_UINT32(wizh + 0x4);
		uint32 height = READ_LE_UINT32(wizh + 0x8);
		if (comp != 1) {
			warning("%d has invalid compression type %d", pwi->resNum, comp);
			return 0;
		}
		const uint8 *wizd = findWrappedBlock(MKID('WIZD'), dataPtr, pwi->state, 0);
		assert(wizd);
		if (pwi->flags & 1) {
			const uint8 *pal = findWrappedBlock(MKID('RGBS'), dataPtr, pwi->state, 0);
			assert(pal);
			setPaletteFromPtr(pal, 256);
		}
		if (pwi->flags & 2) {
			const uint8 *rmap = findWrappedBlock(MKID('RMAP'), dataPtr, pwi->state, 0);
			assert(rmap);
			const uint8 *rgbs = findWrappedBlock(MKID('RGBS'), dataPtr, pwi->state, 0);
			assert(rgbs);
//			drawWizImageHelper1(rmap + 4, _currentPalette, rgbs);
			warning("drawWizImage() unhandled flag 0x2");
		}
		uint32 cw, ch;
		if (pwi->flags & 0x24) { // printing (0x4) or rendering to memory (0x20)
			dst = (uint8 *)malloc(width * height);
			if (pwi->flags & 0x20) {
				int color = 255; // FIXME: should be (VAR_WIZ_TCOLOR != 0xFF) ? VAR(VAR_WIZ_TCOLOR) : 5;
				memset(dst, color, width * height);
			}
			cw = width;
			ch = height;
		} else {
			VirtScreen *pvs = &virtscr[kMainVirtScreen];
			if (pwi->flags & 0x10) {
				dst = pvs->getPixels(0, pvs->topline);
			} else {
				dst = pvs->getBackPixels(0, pvs->topline);
			}
			cw = pvs->w;
			ch = pvs->h;
		}
		Common::Rect rScreen(cw, ch);
		if (pwi->flags & 0x80) {
//  		drawWizImageHelper2(p, wizd, cw, ch, x1, y1, width, height, &rScreen, 0, 2);
			warning("drawWizImage() unhandled flag 0x80");
		} else if (pwi->flags & 0x100) {
//  		drawWizImageHelper2(p, wizd, cw, ch, x1, y1, width, height, &rScreen, 0, 1);  			
			warning("drawWizImage() unhandled flag 0x100");
		} else {
			gdi.copyWizImage(dst, wizd, cw, ch, pwi->x1, pwi->y1, width, height, &rScreen);
		}
		if (pwi->flags & 4) {
			warning("printing Wiz image is unimplemented");
			dst = NULL;
		} else if (!(pwi->flags & 0x20)) {
			Common::Rect rImage(pwi->x1, pwi->y1, pwi->x1 + width, pwi->y1 + height);
			if (rImage.intersects(rScreen)) {
				rImage.clip(rScreen);
				if (pwi->flags & 0x18) {
					++rImage.bottom;
					markRectAsDirty(kMainVirtScreen, rImage);
				} else {
					gdi.copyVirtScreenBuffers(rImage);
				}
			}
		}
	}
	return dst;
}

struct PolygonDrawData {
	struct InterArea {
		bool valid;
		int32 xmin;
		int32 xmax;
		int32 x1;
		int32 y1;
		int32 x2;
		int32 y2;
	};
	Common::Point pts[4];
	InterArea *ia;
	int areasNum;
	
	PolygonDrawData(int n) {
		memset(pts, 0, sizeof(pts));
		areasNum = n;
		ia = new InterArea[areasNum];
		memset(ia, 0, sizeof(InterArea) * areasNum);
	}
	
	~PolygonDrawData() {
		delete[] ia;
	}
	
	void calcIntersection(const Common::Point *p1, const Common::Point *p2, const Common::Point *p3, const Common::Point *p4) {
		int32 x1_acc = p1->x << 0x10;
		int32 x3_acc = p3->x << 0x10;
		int32 y3_acc = p3->y << 0x10;
  		uint16 dy = ABS(p2->y - p1->y) + 1;
  		int32 x1_step = ((p2->x - p1->x) << 0x10) / dy;
  		int32 x3_step = ((p4->x - p3->x) << 0x10) / dy;
  		int32 y3_step = ((p4->y - p3->y) << 0x10) / dy;

  		int iaidx = p1->y - pts[0].y;
  		while (dy--) {
  			assert(iaidx >= 0 && iaidx < areasNum);
  			InterArea *pia = &ia[iaidx];
  			int32 tx1 = x1_acc >> 0x10;
  			int32 tx3 = x3_acc >> 0x10;
  			int32 ty3 = y3_acc >> 0x10;
  			
  			if (!pia->valid || pia->xmin > tx1) {
  				pia->xmin = tx1;
  				pia->x1 = tx3;
  				pia->y1 = ty3;
			}
  			if (!pia->valid || pia->xmax < tx1) {
  				pia->xmax = tx1;
  				pia->x2 = tx3;
  				pia->y2 = ty3;
			}
  			pia->valid = true;

  			x1_acc += x1_step;
  			x3_acc += x3_step;
  			y3_acc += y3_step;
  			
  			if (p2->y <= p1->y) {
  				--iaidx;
  			} else {
  				++iaidx;
  			}  			
  		}
	}
};

void ScummEngine_v72he::drawWizPolygon(int resnum, int state, int id, int flags) {
	int i;
	WizPolygon *wp = NULL;
	for (i = 0; i < _wizNumPolygons; ++i) {
		if (_wizPolygons[i].id == id) {
			wp = &_wizPolygons[i];
			break;
		}
	}
	if (!wp) {
		error("Polygon %d is not defined", id);
	}
	if (wp->numVerts != 5) {
		error("Invalid point count %d for Polygon %d", wp->numVerts, id);
	}
	WizImage wi;
	wi.resNum = resnum;
	wi.state = state;
	wi.x1 = wi.y1 = 0;
	wi.flags = 0x20;
	uint8 *srcWizBuf = drawWizImage(rtImage, &wi);
	if (srcWizBuf) {
		uint8 *dst;
		VirtScreen *pvs = &virtscr[kMainVirtScreen];
		if (flags & 0x10) {
			dst = pvs->getPixels(0, 0);
		} else {
			dst = pvs->getBackPixels(0, 0);
		}
		if (wp->bound.left < 0 || wp->bound.top < 0 || wp->bound.right >= pvs->w || wp->bound.bottom >= pvs->h) {
			error("Invalid coords polygon %d", wp->id);
		}

		uint32 wizW, wizH;
		getWizImageDim(resnum, state, wizW, wizH);
		Common::Point bbox[4];
		bbox[0].x = 0;
		bbox[0].y = 0;
		bbox[1].x = wizW - 1;
		bbox[1].y = 0;
		bbox[2].x = wizW - 1;
		bbox[2].y = wizH - 1;
		bbox[3].x = 0;
		bbox[3].y = wizH - 1;

  		int16 xmin_p, xmax_p, ymin_p, ymax_p;
  		xmin_p = xmax_p = wp->vert[0].x;
  		ymin_p = ymax_p = wp->vert[0].y;
  		for (i = 1; i < 4; ++i) {
  			xmin_p = MIN(wp->vert[i].x, xmin_p);
  			xmax_p = MAX(wp->vert[i].x, xmax_p);
  			ymin_p = MIN(wp->vert[i].y, ymin_p);
  			ymax_p = MAX(wp->vert[i].y, ymax_p);
  		}
  		
  		int16 xmin_b, xmax_b, ymin_b, ymax_b;
  		xmin_b = 0;
  		xmax_b = wizW - 1;
  		ymin_b = 0;
  		ymax_b = wizH - 1;

		PolygonDrawData pdd(ymax_p - ymin_p + 1);
		pdd.pts[0].x = xmin_p;
		pdd.pts[0].y = ymin_p;
		pdd.pts[1].x = xmax_p;
		pdd.pts[1].y = ymax_p;
		pdd.pts[2].x = xmin_b;
		pdd.pts[2].y = ymin_b;
		pdd.pts[3].x = xmax_b;
		pdd.pts[3].y = ymax_b;
		
		for (i = 0; i < 3; ++i) {
			pdd.calcIntersection(&wp->vert[i], &wp->vert[i + 1], &bbox[i], &bbox[i + 1]);
		}
		pdd.calcIntersection(&wp->vert[3], &wp->vert[0], &bbox[3], &bbox[0]);
		
		uint yoff = pdd.pts[0].y * pvs->w;
		for (i = 0; i < pdd.areasNum; ++i) {
			PolygonDrawData::InterArea *pia = &pdd.ia[i];
			uint16 dx = pia->xmax - pia->xmin + 1;
			uint8 *dstPtr = dst + pia->xmin + yoff;
			int32 x_acc = pia->x1 << 0x10;
			int32 y_acc = pia->y1 << 0x10;
			int32 x_step = ((pia->x2 - pia->x1) << 0x10) / dx;
			int32 y_step = ((pia->y2 - pia->y1) << 0x10) / dx;
			while (dx--) {
				uint srcWizOff = (y_acc >> 0x10) * wizW + (x_acc >> 0x10);
				assert(srcWizOff < wizW * wizH);
				x_acc += x_step;
				y_acc += y_step;
				*dstPtr++ = srcWizBuf[srcWizOff];
			}
			yoff += pvs->w;
		}

		if (flags & 0x10) {
			markRectAsDirty(kMainVirtScreen, wp->bound);
		} else {
			gdi.copyVirtScreenBuffers(wp->bound);
		}

		free(srcWizBuf);
	}
}

void ScummEngine_v72he::redrawBGAreas() {
	ScummEngine_v70he::redrawBGAreas();
	flushWizBuffer();
}

void ScummEngine_v72he::flushWizBuffer() {
	for (int i = 0; i < _wizImagesNum; ++i) {
		WizImage *pwi = &_wizImages[i];
		if (pwi->flags & 0x40) {
			drawWizPolygon(pwi->resNum, pwi->state, pwi->x1, pwi->flags);
		} else {
			drawWizImage(rtImage, pwi);
		}
	}
	_wizImagesNum = 0;
}

void ScummEngine_v72he::o72_drawWizImage() {
	WizImage wi;
	wi.flags = pop();
	wi.y1 = pop();
	wi.x1 = pop();
	wi.resNum = pop();
	wi.state = 0;
	displayWizImage(&wi);
}

void ScummEngine_v72he::o72_unknownCF() {
	byte string[255];

	copyScriptString(string);
	writeVar(0, 0);
	defineArray(0, kStringArray, 0, 0, 0, 0);
	writeArray(0, 0, 0, 0);
	push(readVar(0));
	debug(1,"o72_unknownCF: String %s", string);
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
	byte filename[256];

	mode = pop();
	copyScriptString(filename);

	// HACK bb2demo uses incorrect filename
	if (!strcmp((char *)filename,".HE9")) {
		memset(filename, 0, sizeof(filename));
		sprintf((char *)filename, "%s.he9", _gameName.c_str());
	}

	for (r = strlen((char*)filename); r != 0; r--) {
		if (filename[r - 1] == '\\')
			break;
	}
	
	debug(0,"Filename %s", filename + r);

	// HACK: Convert paths
	if (filename[0] == ':') {
		int len = resStrLen(filename);
		int i = 1, j = 0;
		while(len--) {
			if (filename[i] == ':')
				filename[j] = '/';
			else
				filename[j] = filename[i];

			i++;
			j++;
		}
		debug(0,"Converted Filename %s", filename + r);
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
			error("o72_openFile(): wrong open file mode %d", mode);

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
		fetchScriptByte();
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
	int r;

	copyScriptString(filename);
	for (r = strlen((char*)filename); r != 0; r--) {
		if (filename[r - 1] == '\\')
			break;
	}

	debug(1, "stub o72_deleteFile(%s)", filename + r);
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
	int32 dim1end;

	num = getStackList(args, ARRAYSIZE(args));
	int value = fetchScriptWord();

	if (readVar(value) == 0) {
		defineArray(value, kDwordArray, 0, 0, 0, num);
		if (num > 0) {
			int16 counter = 0;
			do {
				writeArray(value, 0, counter + 1, args[counter]);
			} while (++counter < num);
		}

		shuffleArray(value, 1, num);
		writeArray(value, 0, 0, 2);
		push(readArray(value, 0, 1));
		return;
	}

	num = readArray(value, 0, 0);

	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, readVar(value));
	dim1end = FROM_LE_32(ah->dim1end);

	if (dim1end < num) {
		int16 var_2 = readArray(value, 0, num - 1);
		shuffleArray(value, 1, dim1end);
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

void ScummEngine_v72he::o72_unknownEC() {
	int dst, size;
	int src = pop();

	size = resStrLen(getStringAddress(src)) + 1;

	writeVar(0, 0);
	defineArray(0, kStringArray, 0, 0, 0, size);
	writeArray(0, 0, 0, 0);
	dst = readVar(0);

	arrrays_unk2(dst, src, -1, -1);

	push(dst);
	debug(1,"stub o72_unknownEC");
}

void ScummEngine_v72he::o72_unknownED() {
	int array, pos, len;
	int chr, result = 0;

	len = pop();
	pos = pop();
	array = pop();

	if (len == -1) {
		pos = 0;
		len = resStrLen(getStringAddress(array));
	}

	writeVar(0, array);
	while (pos <= len) {
		chr = readArray(0, 0, pos);
		if (chr)
			result += _charset->getCharWidth(chr);
		pos++;
	}

	push(result);
	debug(1,"stub o72_unknownED");
}

void ScummEngine_v72he::o72_unknownEF() {
	int dst, size;
	int b = pop();
	int a = pop();
	int src = pop();

	size = b - a + 2;

	writeVar(0, 0);
	defineArray(0, kStringArray, 0, 0, 0, size);
	writeArray(0, 0, 0, 0);

	dst = readVar(0);

	arrrays_unk2(dst, src, a, b);

	push(dst);
	debug(1,"stub o72_unknownEF");
}

void ScummEngine_v72he::o72_unknownF0() {
	int dst, size;

	int src2 = pop();
	int src1 = pop();

	size = resStrLen(getStringAddress(src1));
	size += resStrLen(getStringAddress(src2)) + 1;

	writeVar(0, 0);
	defineArray(0, kStringArray, 0, 0, 0, size);
	writeArray(0, 0, 0, 0);

	dst = readVar(0);

	arrrays_unk2(dst, src1, 0, -1);
	arrrays_unk2(dst, src2, 0, -1);

	push(dst);
	debug(1,"stub o72_unknownF0");
}

void ScummEngine_v72he::o72_unknownF1() {
	byte *addr, *addr2;
	int i = 0;

	int id = pop();
	int id2 = pop();

	addr = getStringAddress(id);
	if (!addr)
		error("o72_stringLen: Reference to zeroed array pointer (%d)", id);

	addr2 = getStringAddress(id2);
	if (!addr)
		error("o72_stringLen: Reference to zeroed array pointer (%d)", id);

	while (*addr == *addr2) {
		if (*addr == 0) {
			push(0);
			return;
		}
		addr++;
		addr2++;
		i++;
	}

	push (i);
	debug(1,"o70_unknownF1 stub (%d, %d, %d)", id, id2, i);
}

void ScummEngine_v72he::o72_checkGlobQueue() {
	int subOp = fetchScriptByte();
	int idx = pop();

	debug(1,"o72_checkGlobQueue stub (%d, %d)", subOp, idx);
	push(100);
}

void ScummEngine_v72he::o72_readINI() {
	byte option[100];
	int type, retval;

	// we pretend that we don't have .ini file
	copyScriptString(option);
	type = fetchScriptByte();

	switch (type) {
	case 6: // number
		if (!strcmp((char *)option, "NoPrinting"))
			push(1);
		else
			push(0);
		break;
	case 7: // string
		writeVar(0, 0);
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
	byte option[256], string[1024];

	type = fetchScriptByte();

	switch (type) {
	case 6: // number
		value = pop();
		copyScriptString(option);
		debug(1,"o72_writeINI: %s set to %d", option, value);
		break;
	case 7: // string
		copyScriptString(string);
		copyScriptString(option);
		debug(1,"o72_writeINI: %s set to %s", option, string);
		break;
	default:
		error("o72_writeINI: default type %d", type);
	}
}

void ScummEngine_v72he::o72_unknownF5() {
	int chr, max;
	int array, len, pos, result = 0;
	max = pop();
	pos = pop();
	array = pop();

	len = resStrLen(getStringAddress(array));

	writeVar(0, array);
	while (pos <= len) {
		chr = readArray(0, 0, pos);
		result += _charset->getCharWidth(chr);
		if (result >= max) {
			push(pos);
			return;
		}
		pos++;
	}

	push(len);
	debug(1,"stub o72_unknownF5 (%d)", result);
}

void ScummEngine_v72he::o72_unknownF6() {
	int len, edi, pos, value, id;
	value = pop();
	edi = pop();
	pos = pop();
	id = pop();

	if (edi >= 0) {
		len = resStrLen(getStringAddress(id));
		if (len < edi)
			edi = len;
	} else {
		edi = 0;
	}

	if (pos < 0)
		pos = 0;

	writeVar(0, id);
	if (edi > pos) {
		while (edi >= pos) {
			if (readArray(0, 0, pos) == value) {
				push(pos);
				return;
			}
			pos++;
		}
	} else {
		while (edi <= pos) {
			if (readArray(0, 0, pos) == value) {
				push(pos);
				return;
			}
			pos--;
		}
	}

	push(-1);
	debug(1,"stub o72_unknownF6");
}

void ScummEngine_v72he::o72_unknownF8() {
	int id = pop();
	byte subOp = fetchScriptByte();
	push(10);

	debug(1,"stub o72_unknownF8: subOp %d, id %d", subOp, id);
}

void ScummEngine_v72he::o72_setFilePath() {
	// File related
	byte filename[100];
	copyScriptString(filename);
	debug(1,"o72_setFilePath: %s", filename);
}

void ScummEngine_v72he::o72_unknownFA() {
	byte name[100];
	copyScriptString(name);
	int id = fetchScriptByte();

	debug(1,"o72_unknownFA: (%d) %s", id, name);
}

void ScummEngine_v72he::decodeParseString(int m, int n) {
	int i, color, size;
	int args[31];
	byte name[1024];

	byte b = fetchScriptByte();

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
		printString(m, _scriptPointer);
		_scriptPointer += resStrLen(_scriptPointer) + 1;
		break;
	case 194:
		decodeScriptString(name, true);
		printString(m, name);
		break;
	case 0xE1:
		{
		const byte *dataPtr = getResourceAddress(rtTalkie, pop());
		const byte *text = findWrappedBlock(MKID('TEXT'), dataPtr, 0, 0);
		size = getResourceDataSize(text);
		memcpy(name, text, size);
		printString(m, name);
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
