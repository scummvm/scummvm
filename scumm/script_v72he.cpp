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

#include "stdafx.h"

#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/system.h"

#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/intern.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/resource_v7he.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "scumm/util.h"
#include "scumm/verbs.h"

namespace Scumm {

#define OPCODE(x)	_OPCODE(ScummEngine_v72he, x)

void ScummEngine_v72he::setupOpcodes() {
	static const OpcodeEntryV72he opcodes[256] = {
		/* 00 */
		OPCODE(o6_pushByte),
		OPCODE(o6_pushWord),
		OPCODE(o72_pushDWord),
		OPCODE(o6_pushWordVar),
		/* 04 */
		OPCODE(o72_getScriptString),
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
		OPCODE(o72_resetCutscene),
		OPCODE(o6_invalid),
		OPCODE(o72_findObjectWithClassOf),
		OPCODE(o6_wordArrayInc),
		/* 54 */
		OPCODE(o72_getObjectImageX),
		OPCODE(o72_getObjectImageY),
		OPCODE(o72_captureWizImage),
		OPCODE(o6_wordVarDec),
		/* 58 */
		OPCODE(o72_getTimer),
		OPCODE(o72_setTimer),
		OPCODE(o72_getSoundElapsedTime),
		OPCODE(o6_wordArrayDec),
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
		OPCODE(o72_talkActor),
		OPCODE(o72_talkEgo),
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
		OPCODE(o72_kernelGetFunctions),
		OPCODE(o70_kernelSetFunctions),
		OPCODE(o6_delayFrames),
		OPCODE(o6_pickOneOf),
		/* CC */
		OPCODE(o6_pickOneOfDefault),
		OPCODE(o6_stampObject),
		OPCODE(o72_drawWizImage),
		OPCODE(o72_debugInput),
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
		OPCODE(o72_rename),
		/* E0 */
		OPCODE(o60_soundOps),
		OPCODE(o72_getPixel),
		OPCODE(o60_localizeArrayToScript),
		OPCODE(o72_pickVarRandom),
		/* E4 */
		OPCODE(o6_setBoxSet),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* E8 */
		OPCODE(o6_invalid),
		OPCODE(o70_seekFilePos),
		OPCODE(o72_redimArray),
		OPCODE(o60_readFilePos),
		/* EC */
		OPCODE(o70_copyString),
		OPCODE(o70_getStringWidth),
		OPCODE(o70_getStringLen),
		OPCODE(o70_appendString),
		/* F0 */
		OPCODE(o70_concatString),
		OPCODE(o70_compareString),
		OPCODE(o72_isResourceLoaded),
		OPCODE(o72_readINI),
		/* F4 */
		OPCODE(o72_writeINI),
		OPCODE(o70_getStringLenForWidth),
		OPCODE(o70_getCharIndexInString),
		OPCODE(o6_invalid),
		/* F8 */
		OPCODE(o72_getResourceSize),
		OPCODE(o72_setFilePath),
		OPCODE(o72_setWindowCaption),
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

static const int arrayDataSizes[] = { 0, 1, 4, 8, 8, 16, 32 };

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

	ah = (ArrayHeader *)res.createResource(rtString, id, size + sizeof(ArrayHeader));

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

int ScummEngine_v72he::setupStringArray(int size) {
	writeVar(0, 0);
	defineArray(0, kStringArray, 0, 0, 0, size + 1);
	writeArray(0, 0, 0, 0);
	return readVar(0);
}

void ScummEngine_v72he::readArrayFromIndexFile() {
	int num;
	int a, b, c;

	while ((num = _fileHandle->readUint16LE()) != 0) {
		a = _fileHandle->readUint16LE();
		b = _fileHandle->readUint16LE();
		c = _fileHandle->readUint16LE();

		if (c == 1)
			defineArray(num, kBitArray, 0, a, 0, b);
		else
			defineArray(num, kDwordArray, 0, a, 0, b);
	}
}

int ScummEngine_v72he::convertFilePath(byte *dst, bool setFilePath) {
	// Switch all \ to / for portablity
	int len = resStrLen(dst) + 1;
	for (int i = 0; i < len; i++) {
		if (dst[i] == '\\')
			dst[i] = '/';
	}

	// Strip path
	int r = 0;
	if (dst[0] == '.' && dst[1] == '/') {
		r = 2;
	} else if (dst[0] == 'c' && dst[1] == ':') {
		for (r = len; r != 0; r--) {
			if (dst[r - 1] == '/')
				break;
		}
	}

	if (setFilePath) {
		Common::File f;
		char filePath[256], newFilePath[256];

		sprintf(filePath, "%s%s", _gameDataPath.c_str(), dst + r);
		if (f.exists(filePath)) {
			sprintf(newFilePath, "%s%s", _gameDataPath.c_str(), dst + r);
		} else {
			sprintf(newFilePath, "%s%s", _saveFileMan->getSavePath(), dst + r);
		}

		len = resStrLen((const byte *)newFilePath);
		memcpy(dst, newFilePath, len);
		dst[len] = 0;
		debug(0, "convertFilePath: newFilePath is %s", newFilePath);
	}

	return r;
}	

void ScummEngine_v72he::copyScriptString(byte *dst, int dstSize) {
	byte string[1024];
	byte chr;
	int pos = 0;

	int array = pop();
	if (array == -1) {
		if (_stringLength == 1)
			error("String stack underflow");

		_stringLength -= 2;
		 while ((chr = _stringBuffer[_stringLength]) != 0) {
			string[pos] = chr;
			pos++;

			if (pos > dstSize)
				error("String too long to pop");

			_stringLength--;
		}

		string[pos] = 0;
		_stringLength++;

		// Reverse string
		int len = resStrLen(string);
		while (len--)
			*dst++ = string[len];
	} else {
		writeVar(0, array);
		while ((chr = readArray(0, 0, pos)) != 0) {
			*dst++ = chr;
			pos++;
		}
	}
	*dst = 0;
}

void ScummEngine_v72he::decodeScriptString(byte *dst, bool scriptString) {
	const byte *src;
	int args[31];
	int num, len, val;
	byte chr, string[1024];
	memset(args, 0, sizeof(args));
	memset(string, 0, sizeof(string));

	// Get stack list, plus one
	num = pop();
	for (int i = num; i >= 0; i--)
		args[i] = pop();

	// Get string
	if (scriptString) {
		len = resStrLen(_scriptPointer) + 1;
		memcpy(string, _scriptPointer, len);
		_scriptPointer += len;
	} else {
		copyScriptString(string, sizeof(string));
		len = resStrLen(string) + 1;
	}

	// Decode string
	num = 0;
	val = 0;
	while (len--) {
		chr = string[num++];
		if (chr == '%') {
			chr = string[num++];
			switch(chr) {
			case 'b':
				//dst += sprintf((char *)dst, "%b", args[val++]);
				break;
			case 'c':
				*dst++ = args[val++];
				break;
			case 'd':
				dst += sprintf((char *)dst, "%d", args[val++]);
				break;
			case 's':
				src = getStringAddress(args[val++]);
				if (src) {
					while (*src != 0)
						*dst++ = *src++;
				}
				break;
			case 'x':
				dst += sprintf((char *)dst, "%x", args[val++]);
				break;
			default:
				*dst++ = '%';
				num--;
				break;
			}
		} else {
			*dst++ = chr;
		}
	}
	*dst = 0;
}

byte *ScummEngine_v72he::heFindResourceData(uint32 tag, byte *ptr) {
	ptr = heFindResource(tag, ptr);

	if (ptr == NULL)
		return NULL;
	return ptr + _resourceHeaderSize;
}

byte *ScummEngine_v72he::heFindResource(uint32 tag, byte *searchin) {
	uint32 curpos, totalsize, size;

	debugC(DEBUG_RESOURCE, "heFindResource(%s, %lx)", tag2str(tag), searchin);

	assert(searchin);
	searchin += 4;
	_resourceLastSearchSize = totalsize = READ_BE_UINT32(searchin);
	curpos = 8;
	searchin += 4;

	while (curpos < totalsize) {
		if (READ_UINT32(searchin) == tag) {
			return searchin;
		}

		size = READ_BE_UINT32(searchin + 4);
		if ((int32)size <= 0) {
			error("(%s) Not found in %d... illegal block len %d", tag2str(tag), 0, size);
			return NULL;
		}

		curpos += size;
		searchin += size;
	}

	return NULL;
}

byte *ScummEngine_v72he::findWrappedBlock(uint32 tag, byte *ptr, int state, bool errorFlag) {
	if (READ_UINT32(ptr) == MKID('MULT')) {
		byte *offs, *wrap;
		uint32 size;

		wrap = heFindResource(MKID('WRAP'), ptr);
		if (wrap == NULL)
			return NULL;

		offs = heFindResourceData(MKID('OFFS'), wrap);
		if (offs == NULL)
			return NULL;

		size = getResourceDataSize(offs) / 4;
		assert((uint32)state <= (uint32)size);
			

		offs += READ_LE_UINT32(offs + state * sizeof(uint32));
		offs = heFindResourceData(tag, offs - 8);
		if (offs)
			return offs;

		offs = heFindResourceData(MKID('DEFA'), ptr);
		if (offs == NULL)
			return NULL;

		return heFindResourceData(tag, offs - 8);
	} else {
		return heFindResourceData(tag, ptr);
	}
}

int ScummEngine_v72he::findObject(int x, int y, int num, int *args) {
	int b, cls, i, result;

	for (i = 1; i < _numLocalObjects; i++) {
		result = 0;
		if ((_objs[i].obj_nr < 1) || getClass(_objs[i].obj_nr, kObjectClassUntouchable))
			continue;

		// Check polygon bounds
		if (_wiz.polygonDefined(_objs[i].obj_nr)) {
			if (_wiz.polygonHit(_objs[i].obj_nr, x, y))
				result = _objs[i].obj_nr;
			else if (VAR_POLYGONS_ONLY != 0xFF && VAR(VAR_POLYGONS_ONLY))
				continue;
		}

		if (!result) {
			// Check object bounds
			if (_objs[i].x_pos <= x && _objs[i].width + _objs[i].x_pos > x &&
			    _objs[i].y_pos <= y && _objs[i].height + _objs[i].y_pos > y)
					result = _objs[i].obj_nr;
		}

		if (result) {
			if (!num)
				return result;

			// Check object class
			cls = args[0];
			b = getClass(_objs[i].obj_nr, cls);
			if ((cls & 0x80 && b) || (!(cls & 0x80) && !b))
				return result;
		}
	}

	return 0;
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

void ScummEngine_v72he::o72_getScriptString() {
	byte chr;

	while ((chr = fetchScriptByte()) != 0) {
		_stringBuffer[_stringLength] = chr;
		_stringLength++;

		if (_stringLength >= 4096)
			error("String stack overflow");
	}

	_stringBuffer[_stringLength] = 0;
	_stringLength++;
}

void ScummEngine_v72he::o72_isAnyOf() {
	int args[128];
	int num, value;

	num = getStackList(args, ARRAYSIZE(args));
	value = pop();

	for (int i = 0; i < num; i++) {
		if (args[i] == value) {
			push(1);
			return;
		}
	}

	push(0);
}

void ScummEngine_v72he::o72_resetCutscene() {
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

void ScummEngine_v72he::o72_getObjectImageX() {
	int object = pop();
	int objnum = getObjectIndex(object);

	if (objnum == -1) {
		push(0);
		return;
	}

	push(_objs[objnum].x_pos / 8);
}

void ScummEngine_v72he::o72_getObjectImageY() {
	int object = pop();
	int objnum = getObjectIndex(object);

	if (objnum == -1) {
		push(0);
		return;
	}

	push(_objs[objnum].y_pos / 8);
}

void ScummEngine_v72he::o72_captureWizImage() {
	Common::Rect grab;
	grab.bottom = pop() + 1;
	grab.right = pop() + 1;
	grab.top = pop();
	grab.left = pop();
	captureWizImage(pop(), grab, false, true);	
}

void ScummEngine_v72he::o72_getTimer() {
	int timer = pop();
	int cmd = fetchScriptByte();

	if (cmd == 10 || cmd == 50) {
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

	if (cmd == 158 || cmd == 61) {
		checkRange(3, 1, timer, "o72_setTimer: Timer %d out of range(%d)");
		_timers[timer] = _system->getMillis();
	} else {
		error("TIMER command %d?", cmd);
	}
}

void ScummEngine_v72he::o72_getSoundElapsedTime() {
	int snd = pop();
	push(_sound->getSoundElapsedTime(snd) * 10);
	debug(1,"o72_getSoundElapsedTime (%d)", snd);
}

void ScummEngine_v72he::o72_startScript() {
	int args[25];
	int script;
	byte flags;

	getStackList(args, ARRAYSIZE(args));
	script = pop();
	flags = fetchScriptByte();
	runScript(script, (flags == 199 || flags == 200), (flags == 195 || flags == 200), args);
}

void ScummEngine_v72he::o72_startObject() {
	int args[25];
	int script, entryp;
	byte flags;

	getStackList(args, ARRAYSIZE(args));
	entryp = pop();
	script = pop();
	flags = fetchScriptByte();
	runObjectScript(script, entryp, (flags == 199 || flags == 200), (flags == 195 || flags == 200), args);
}

void ScummEngine_v72he::o72_drawObject() {
	byte subOp = fetchScriptByte();
	int state, y, x;

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
		y = x = -100;
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

	if (y != -100 && x != -100) {
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
	wi.flags = kWIFPrint;
	displayWizImage(&wi);
}

void ScummEngine_v72he::o72_getArrayDimSize() {
	byte subOp = fetchScriptByte();
	int32 val1, val2;
	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, readVar(fetchScriptWord()));
	if (!ah) {
		push(0);
		return;
	}

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
		copyScriptString(filename, sizeof(filename));
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
		setRoomPalette(a, b);
		break;

	default:
		error("o72_roomOps: default case %d", op);
	}
}

void ScummEngine_v72he::o72_actorOps() {
	Actor *a;
	int i, j, k;
	int args[32];
	byte subOp;
	byte string[256];

	subOp = fetchScriptByte();
	if (subOp == 197) {
		_curActor = pop();
		return;
	}

	a = derefActorSafe(_curActor, "o72_actorOps");
	if (!a)
		return;

	switch (subOp) {
	case 21: // HE 80+
		k = getStackList(args, ARRAYSIZE(args));
		for (i = 0; i < k; ++i) {
			a->setUserCondition(args[i] & 0x7F, args[i] & 0x80);
		}
		debug(1,"o72_actorOps: case 21 (%d)", k);
		break;
	case 24: // HE 80+
		k = pop();
		a->_heNoTalkAnimation = 1;
		a->setTalkCondition(k);
		debug(1,"o72_actorOps: case 24 (%d)", k);
		break;
	case 43: // HE 90+
		a->_layer = pop();
		if (_gameId != GID_FREDDICOVE) {
			// HE games use reverse order of layering, so we adjust
			a->_layer = -a->_layer;
		}
		a->_needRedraw = true;
		break;
	case 64:
		_actorClipOverride.bottom = pop();
		_actorClipOverride.right = pop();
		_actorClipOverride.top = pop();
		_actorClipOverride.left = pop();
		break;
	case 67: // HE 99+
		a->_clipOverride.bottom = pop();
		a->_clipOverride.right = pop();
		a->_clipOverride.top = pop();
		a->_clipOverride.left = pop();
		break;
	case 65: // HE 98+
		j = pop();
		i = pop();
		a->putActor(i, j, a->_room);
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
			a->_sound[i] = args[i];
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
		a->_needRedraw = true;
		break;
	case 87:		// SO_TALK_COLOR
		a->_talkColor = pop();
		break;
	case 88:		// SO_ACTOR_NAME
		copyScriptString(string, sizeof(string));
		loadPtrToResource(rtActorName, a->_number, string);
		break;
	case 89:		// SO_INIT_ANIMATION
		a->_initFrame = pop();
		break;
	case 91:		// SO_ACTOR_WIDTH
		a->_width = pop();
		break;
	case 92:		// SO_SCALE
		i = pop();
		a->setScale(i, i);
		break;
	case 93:		// SO_NEVER_ZCLIP
		a->_forceClip = 0;
		break;
	case 94:		// SO_ALWAYS_ZCLIP
		a->_forceClip = pop();
		break;
	case 95:		// SO_IGNORE_BOXES
		a->_ignoreBoxes = 1;
		a->_forceClip = 0;
		if (a->isInCurrentRoom())
			a->putActor(a->_pos.x, a->_pos.y, a->_room);
		break;
	case 96:		// SO_FOLLOW_BOXES
		a->_ignoreBoxes = 0;
		a->_forceClip = 0;
		if (a->isInCurrentRoom())
			a->putActor(a->_pos.x, a->_pos.y, a->_room);
		break;
	case 97:		// SO_ANIMATION_SPEED
		a->setAnimSpeed(pop());
		break;
	case 98:		// SO_SHADOW
		a->_shadowMode = pop();
		a->_needRedraw = true;
		debug(1, "Set actor XMAP idx to %d", a->_shadowMode);
		break;
	case 99:		// SO_TEXT_OFFSET
		a->_talkPosY = pop();
		a->_talkPosX = pop();
		break;
	case 156:		// HE 72+
		a->_charset = pop();
		break;
	case 175:		// HE 99+
		a->_hePaletteNum = pop();
		a->_needRedraw = true;
		break;
	case 198:		// SO_ACTOR_VARIABLE
		i = pop();
		a->setAnimVar(pop(), i);
		break;
	case 215:		// SO_ACTOR_IGNORE_TURNS_ON
		a->_ignoreTurns = true;
		break;
	case 216:		// SO_ACTOR_IGNORE_TURNS_OFF
		a->_ignoreTurns = false;
		break;
	case 217:		// SO_ACTOR_NEW
		a->initActor(2);
		break;
	case 218:		
		a->drawActorToBackBuf(a->_pos.x, a->_pos.y);
		break;
	case 219:
		a->_drawToBackBuf = false;
		a->_needRedraw = true;
		a->_needBgReset = true;
		break;
	case 225:
		{
		copyScriptString(string, sizeof(string));
		int slot = pop();

		int len = resStrLen(string) + 1;
		memcpy(a->_heTalkQueue[slot].sentence, string, len);

		a->_heTalkQueue[slot].posX = a->_talkPosX;
		a->_heTalkQueue[slot].posY = a->_talkPosY;
		a->_heTalkQueue[slot].color = a->_talkColor;
		break;
		}
	default:
		error("o72_actorOps: default case %d", subOp);
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
		copyScriptString(name, sizeof(name));
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
	byte string[1024];


	debug(1,"o72_arrayOps: case %d", subOp);
	switch (subOp) {
	case 7:			// SO_ASSIGN_STRING
		copyScriptString(string, sizeof(string));
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
		tmp2 = 0;
		while (dim2start <= dim2end) {
			tmp = dim1start;
			while (tmp <= dim1end) {
				writeArray(array, dim2start, tmp, list[tmp2++]);
				if (tmp2 == len)
					tmp2 = 0;
				tmp++;
			}
			dim2start++;
		}
		break;
	case 127:
		{
			int a2_dim1end = pop();
			int a2_dim1start = pop();
			int a2_dim2end = pop();
			int a2_dim2start = pop();
			int array2 = fetchScriptWord();
			int a1_dim1end = pop();
			int a1_dim1start = pop();
			int a1_dim2end = pop();
			int a1_dim2start = pop();
			if (a1_dim1end - a1_dim1start != a2_dim1end - a2_dim1start || a2_dim2end - a2_dim2start != a1_dim2end - a1_dim2start) {
				warning("Source and dest ranges size are mismatched");
			}
			copyArray(array, a1_dim2start, a1_dim2end, a1_dim1start, a1_dim1end, array2, a2_dim2start, a2_dim2end, a2_dim1start, a2_dim1end);
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
	case 194:
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
			defineArray(array, kDwordArray, 0, 0, 0, b + c - 1);
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

void ScummEngine_v72he::o72_talkActor() {
	Actor *a;

	int act = pop();

	_string[0].loadDefault();

	// A value of 225 can occur when examining the gold in the mine of pajama, after mining the gold.
	// This is a script bug, the script should set the subtitle color, not actor number.
	// This script bug was fixed in the updated version of pajama.
	if (act == 225) {
		_string[0].color = act;
	} else {
		_actorToPrintStrFor = act;
		if (_actorToPrintStrFor != 0xFF) {
			a = derefActor(_actorToPrintStrFor, "o72_talkActor");
			_string[0].color = a->_talkColor;
		}
	}

	actorTalk(_scriptPointer);

	_scriptPointer += resStrLen(_scriptPointer) + 1;
}

void ScummEngine_v72he::o72_talkEgo() {
	push(VAR(VAR_EGO));
	o72_talkActor();
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
	int data, dim1end, dim2end;

	byte type = fetchScriptByte();
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

	dim1end = pop();
	dim2end = pop();
	defineArray(fetchScriptWord(), data, 0, dim2end, 0, dim1end);
}

void ScummEngine_v72he::o72_traceStatus() {
	byte string[80];

	copyScriptString(string, sizeof(string));
	pop();
}

void ScummEngine_v72he::o72_kernelGetFunctions() {
	int args[29];
	ArrayHeader *ah;
	getStackList(args, ARRAYSIZE(args));

	switch (args[0]) {
	case 1:
		writeVar(0, 0);
		ah = defineArray(0, kByteArray, 0, 0, 0, virtScreenSave(0, args[1], args[2], args[3], args[4]));
		virtScreenSave(ah->data, args[1], args[2], args[3], args[4]);
		push(readVar(0));
		break;
	default:
		error("o72_kernelGetFunctions: default case %d", args[0]);
	}
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

void ScummEngine_v72he::o72_debugInput() {
	byte string[255];

	copyScriptString(string, sizeof(string));
	int len = resStrLen(string) + 1;

	writeVar(0, 0);
	ArrayHeader *ah = defineArray(0, kStringArray, 0, 0, 0, len);
	memcpy(ah->data, string, len);
	push(readVar(0));
	debug(1,"o72_debugInput: String %s", string);
}

void ScummEngine_v72he::o72_jumpToScript() {
	int args[25];
	int script;
	byte flags;

	getStackList(args, ARRAYSIZE(args));
	script = pop();
	flags = fetchScriptByte();
	stopObjectCode();
	runScript(script, (flags == 199 || flags == 200), (flags == 195 || flags == 200), args);
}

void ScummEngine_v72he::o72_openFile() {
	int mode, slot, i;
	byte filename[256];

	mode = pop();
	copyScriptString(filename, sizeof(filename));

	debug(0,"Original filename %s", filename);

	// There are Macintosh specific versions of HE7.2 games.
	if (_heversion >= 80 && _substResFileNameIndex > 0) {
		char buf1[128];

		generateSubstResFileName((char *)filename, buf1, sizeof(buf1));
		strcpy((char *)filename, buf1);
	}

	int r = convertFilePath(filename);
	debug(0,"Final filename to %s", filename + r);

	slot = -1;
	for (i = 1; i < 17; i++) {
		if (_hFileTable[i].isOpen() == false) {
			slot = i;
			break;
		}
	}

	if (slot != -1) {
		switch(mode) {
		case 1:
			_hFileTable[slot].open((char*)filename + r, Common::File::kFileReadMode, _saveFileMan->getSavePath());
			if (_hFileTable[slot].isOpen() == false)
				_hFileTable[slot].open((char*)filename + r, Common::File::kFileReadMode, _gameDataPath.c_str());
			break;
		case 2:
			_hFileTable[slot].open((char*)filename + r, Common::File::kFileWriteMode, _saveFileMan->getSavePath());
			break;
		default:
			error("o72_openFile(): wrong open file mode %d", mode);
		}

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

	if (_hFileTable[slot].isOpen())
		_hFileTable[slot].read(ah->data, size + 1);

	return readVar(0);
}

void ScummEngine_v72he::o72_readFile() {
	int slot, val;
	int32 size;
	byte subOp = fetchScriptByte();

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
	byte subOp = fetchScriptByte();

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
	int i;

	if (room != _currentRoom)
		error("o72_findAllObjects: current room is not %d", room);

	writeVar(0, 0);
	defineArray(0, kDwordArray, 0, 0, 0, _numLocalObjects);
	writeArray(0, 0, 0, _numLocalObjects);
	
	for (i = 1; i < _numLocalObjects; i++) {
		writeArray(0, 0, i, _objs[i].obj_nr);
	}
	
	push(readVar(0));
}

void ScummEngine_v72he::o72_deleteFile() {
	byte filename[256];

	copyScriptString(filename, sizeof(filename));
	debug(1, "stub o72_deleteFile(%s)", filename);
}

void ScummEngine_v72he::o72_rename() {
	byte oldFilename[100],newFilename[100];

	copyScriptString(newFilename, sizeof(newFilename));
	copyScriptString(oldFilename, sizeof(oldFilename));

	debug(1, "stub o72_rename(%s to %s)", oldFilename, newFilename);
}

void ScummEngine_v72he::o72_getPixel() {
	byte area;

	int y = pop();
	int x = pop();
	byte subOp = fetchScriptByte();

	VirtScreen *vs = findVirtScreen(y);
	if (vs == NULL || x > _screenWidth - 1 || x < 0) {
		push(-1);
		return;
	}

	switch (subOp) {
	case 9: // HE 100
	case 218:
		area = *vs->getBackPixels(x, y - vs->topline);
		break;
	case 8: // HE 100
	case 219:
		area = *vs->getPixels(x, y - vs->topline);
		break;
	default:
		error("o72_getPixel: default case %d", subOp);
	}
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
	int newX, newY;
	newY = pop();
	newX = pop();

	byte subOp = fetchScriptByte();
	switch (subOp) {
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
		error("o72_redimArray: default type %d", subOp);
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

void ScummEngine_v72he::checkArrayLimits(int array, int dim2start, int dim2end, int dim1start, int dim1end) {
	if (dim1end < dim1start) {
		warning("Across max %d smaller than min %d", dim1end, dim1start);
 	}
 	if (dim2end < dim2start) {
	 	warning("Down max %d smaller than min %d", dim2end, dim2start);
 	}
	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, readVar(array));
	assert(ah);
	if (ah->dim2start > dim2start || ah->dim2end < dim2end || ah->dim1start > dim1start || ah->dim1end < dim1end) {
		warning("Invalid array access (%d,%d,%d,%d) limit (%d,%d,%d,%d)", dim2start, dim2end, dim1start, dim1end, ah->dim2start, ah->dim2end, ah->dim1start, ah->dim1end);
	}
}

void ScummEngine_v72he::copyArray(int array1, int a1_dim2start, int a1_dim2end, int a1_dim1start, int a1_dim1end, 
				int array2, int a2_dim2start, int a2_dim2end, int a2_dim1start, int a2_dim1end)
{
	debug(5, "ScummEngine_v72he::copyArray(%d, [%d,%d,%d,%d], %d, [%d,%d,%d,%d])", array1, a1_dim2start, a1_dim2end, a1_dim1start, a1_dim1end, array2, a2_dim2start, a2_dim2end, a2_dim1start, a2_dim1end);
	byte *dst, *src;
	int dstPitch, srcPitch;
	int rowSize;
	checkArrayLimits(array1, a1_dim2start, a1_dim2end, a1_dim1start, a1_dim1end);
	checkArrayLimits(array2, a2_dim2start, a2_dim2end, a2_dim1start, a2_dim1end);
	int a12_num = a1_dim2end - a1_dim2start + 1;
	int a11_num = a1_dim1end - a1_dim1start + 1;	
	int a22_num = a2_dim2end - a2_dim2start + 1;
	int a21_num = a2_dim1end - a2_dim1start + 1;
	if (a22_num != a12_num || a21_num != a11_num) {
		warning("Operation size mismatch (%d vs %d)(%d vs %d)", a12_num, a22_num, a11_num, a21_num);
	}	

	if (array1 != array2) {
		ArrayHeader *ah1 = (ArrayHeader *)getResourceAddress(rtString, readVar(array1));
		assert(ah1);
		ArrayHeader *ah2 = (ArrayHeader *)getResourceAddress(rtString, readVar(array2));
		assert(ah2);
		if (FROM_LE_32(ah1->type) == FROM_LE_32(ah2->type)) {
			copyArrayHelper(ah1, a1_dim2start, a1_dim1start, a1_dim1end, &dst, &dstPitch, &rowSize);
			copyArrayHelper(ah2, a2_dim2start, a2_dim1start, a2_dim1end, &src, &srcPitch, &rowSize);
			for (; a1_dim2start <= a1_dim2end; ++a1_dim2start) {
				memcpy(dst, src, rowSize);
				dst += dstPitch;
				src += srcPitch;
			}
		} else {
			for (; a1_dim2start <= a1_dim2end; ++a1_dim2start, ++a2_dim2start) {
				int a2dim1 = a2_dim1start;
				int a1dim1 = a1_dim1start;
				for (; a1dim1 <= a1_dim1end; ++a1dim1, ++a2dim1) {
					int val = readArray(array2, a2_dim2start, a2dim1);
					writeArray(array1, a1_dim2start, a1dim1, val);
				}
			}
		}
	} else {
		if (a2_dim2start != a1_dim2start || a2_dim1start != a1_dim1start) {
			ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, readVar(array1));
			assert(ah);
			if (a2_dim2start > a1_dim2start) {
				copyArrayHelper(ah, a1_dim2start, a1_dim1start, a1_dim1end, &dst, &dstPitch, &rowSize);
				copyArrayHelper(ah, a2_dim2start, a2_dim1start, a2_dim1end, &src, &srcPitch, &rowSize);
			} else {
				copyArrayHelper(ah, a1_dim2end, a1_dim1start, a1_dim1end, &dst, &dstPitch, &rowSize);
				copyArrayHelper(ah, a2_dim2end, a2_dim1start, a2_dim1end, &src, &srcPitch, &rowSize);				
			}
			for (; a1_dim2start <= a1_dim2end; ++a1_dim2start) {
				memcpy(dst, src, rowSize);
				dst += dstPitch;
				src += srcPitch;
			}
		}
	}
}

void ScummEngine_v72he::copyArrayHelper(ArrayHeader *ah, int idx2, int idx1, int len1, byte **data, int *size, int *num) {
	const int pitch = FROM_LE_32(ah->dim1end) - FROM_LE_32(ah->dim1start) + 1;
	const int offset = pitch * (idx2 - FROM_LE_32(ah->dim2start)) + idx1 - FROM_LE_32(ah->dim1start);

	switch (FROM_LE_32(ah->type)) {
	case kByteArray:
	case kStringArray:
		*num = len1 - idx1 + 1;
		*size = pitch;
		*data = ah->data + offset;
		break;
	case kIntArray:
		*num = (len1 - idx1) * 2;
		*size = pitch * 2;
		*data = ah->data + offset * 2;
		break;
	case kDwordArray:
		*num = (len1 - idx1) * 4;
		*size = pitch * 4;
		*data = ah->data + offset * 4;
		break;
	default:
		error("Invalid array type", FROM_LE_32(ah->type));
	}
}

void ScummEngine_v72he::o72_isResourceLoaded() {
	// Reports percentage of resource loaded by queue
	int type;

	byte subOp = fetchScriptByte();
	/* int idx = */ pop();

	switch (subOp) {
	case 18:
		type = rtImage;
		break;
	case 226:
		type = rtRoom;
		break;
	case 227:
		type = rtCostume;
		break;
	case 228:
		type = rtSound;
		break;
	case 229:
		type = rtScript;
		break;
	default:
		error("o72_isResourceLoaded: default case %d", subOp);
	}

	push(100);
}

void ScummEngine_v72he::o72_readINI() {
	byte option[128];
	ArrayHeader *ah;
	const char *entry;
	int len, type;

	copyScriptString(option, sizeof(option));
	type = fetchScriptByte();

	switch (type) {
	case 43: // HE 100
	case 6: // number
		if (!strcmp((char *)option, "NoPrinting")) {
			push(1);
		} else if (!strcmp((char *)option, "TextOn")) {
			push(ConfMan.getBool("subtitles"));
		} else {
			push(ConfMan.getInt((char *)option));
		}
		break;
	case 77: // HE 100
	case 7: // string
		entry = (ConfMan.get((char *)option).c_str());

		writeVar(0, 0);
		len = resStrLen((const byte *)entry);
		ah = defineArray(0, kStringArray, 0, 0, 0, len);
		memcpy(ah->data, entry, len);

		push(readVar(0));
		break;
	default:
		error("o72_readINI: default type %d", type);
	}

	debug(0, "o72_readINI: Option %s", option);
}

void ScummEngine_v72he::o72_writeINI() {
	int type, value;
	byte option[256], string[1024];

	type = fetchScriptByte();

	switch (type) {
	case 43: // HE 100
	case 6: // number
		value = pop();
		copyScriptString(option, sizeof(option));
		ConfMan.set((char *)option, value);
		break;
	case 77: // HE 100
	case 7: // string
		copyScriptString(string, sizeof(string));
		copyScriptString(option, sizeof(option));

		// Filter out useless settings
		if (!strcmp((char *)option, "HETest"))
			return;

		// Filter out confusing subtitle setting
		if (!strcmp((char *)option, "TextOn"))

		// Filter out confusing path settings
		if (!strcmp((char *)option, "DownLoadPath") || !strcmp((char *)option, "GameResourcePath") || !strcmp((char *)option, "SaveGamePath"))
			return;

		ConfMan.set((char *)option, (char *)string); 
		break;
	default:
		error("o72_writeINI: default type %d", type);
	}

	ConfMan.flushToDisk();
}

void ScummEngine_v72he::o72_getResourceSize() {
	const byte *ptr;
	int size, type;

	int resid = pop();
	if (_heversion == 72) {
		push(getSoundResourceSize(resid));
		return;
	}

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 13:
		push (getSoundResourceSize(resid));
		return;
	case 14:
		type = rtRoomImage;
		break;
	case 15:
		type = rtImage;
		break;
	case 16:
		type = rtCostume;
		break;
	case 17:
		type = rtScript;
		break;
	default:
		error("o80_getResourceSize: default type %d", subOp);
	}

	ptr = getResourceAddress(type, resid);
	assert(ptr);
	size = READ_BE_UINT32(ptr + 4) - 8;
	push(size);
}

void ScummEngine_v72he::o72_setFilePath() {
	// File related
	byte filename[255];
	copyScriptString(filename, sizeof(filename));
	debug(1,"o72_setFilePath: %s", filename);
}

void ScummEngine_v72he::o72_setWindowCaption() {
	byte name[1024];
	copyScriptString(name, sizeof(name));
	int id = fetchScriptByte();

	debug(1,"o72_setWindowCaption: (%d) %s", id, name);
}

void ScummEngine_v72he::decodeParseString(int m, int n) {
	Actor *a;
	int i, colors, size;
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
		byte *dataPtr = getResourceAddress(rtTalkie, pop());
		byte *text = findWrappedBlock(MKID('TEXT'), dataPtr, 0, 0);
		size = getResourceDataSize(text);
		memcpy(name, text, size);
		printString(m, name);
		}
		break;
	case 0xF9:
		colors = pop();
		if (colors == 1) {
			_string[m].color = pop();
		} else {	
			push(colors);
			getStackList(args, ARRAYSIZE(args));
			for (i = 0; i < 16; i++)
				_charsetColorMap[i] = _charsetData[_string[1]._default.charset][i] = (unsigned char)args[i];
			_string[m].color = _charsetColorMap[0];
		}
		break;
	case 0xFE:
		_string[m].loadDefault();
		if (n) {
			_actorToPrintStrFor = pop();
			if (_actorToPrintStrFor != 0xFF) {
				a = derefActor(_actorToPrintStrFor, "decodeParseString");
				_string[0].color = a->_talkColor;
			}
		}
		break;
	case 0xFF:
		_string[m].saveDefault();
		break;
	default:
		error("decodeParseString: default case 0x%x", b);
	}
}

} // End of namespace Scumm
