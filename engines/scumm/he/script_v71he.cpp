/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * $URL$
 * $Id$
 *
 */

#include "scumm/actor.h"
#include "scumm/he/intern_he.h"
#include "scumm/scumm.h"
#include "scumm/util.h"

namespace Scumm {

#define OPCODE(i, x)	_opcodes[i]._OPCODE(ScummEngine_v71he, x)

void ScummEngine_v71he::setupOpcodes() {
	/* 00 */
	OPCODE(0x00, o6_pushByte);
	OPCODE(0x01, o6_pushWord);
	OPCODE(0x02, o6_pushByteVar);
	OPCODE(0x03, o6_pushWordVar);
	/* 04 */
	OPCODE(0x06, o6_byteArrayRead);
	OPCODE(0x07, o6_wordArrayRead);
	/* 08 */
	OPCODE(0x0a, o6_byteArrayIndexedRead);
	OPCODE(0x0b, o6_wordArrayIndexedRead);
	/* 0C */
	OPCODE(0x0c, o6_dup);
	OPCODE(0x0d, o6_not);
	OPCODE(0x0e, o6_eq);
	OPCODE(0x0f, o6_neq);
	/* 10 */
	OPCODE(0x10, o6_gt);
	OPCODE(0x11, o6_lt);
	OPCODE(0x12, o6_le);
	OPCODE(0x13, o6_ge);
	/* 14 */
	OPCODE(0x14, o6_add);
	OPCODE(0x15, o6_sub);
	OPCODE(0x16, o6_mul);
	OPCODE(0x17, o6_div);
	/* 18 */
	OPCODE(0x18, o6_land);
	OPCODE(0x19, o6_lor);
	OPCODE(0x1a, o6_pop);
	/* 1C */
	/* 20 */
	/* 24 */
	/* 28 */
	/* 2C */
	/* 30 */
	/* 34 */
	/* 38 */
	/* 3C */
	/* 40 */
	OPCODE(0x42, o6_writeByteVar);
	OPCODE(0x43, o6_writeWordVar);
	/* 44 */
	OPCODE(0x46, o6_byteArrayWrite);
	OPCODE(0x47, o6_wordArrayWrite);
	/* 48 */
	OPCODE(0x4a, o6_byteArrayIndexedWrite);
	OPCODE(0x4b, o6_wordArrayIndexedWrite);
	/* 4C */
	OPCODE(0x4e, o6_byteVarInc);
	OPCODE(0x4f, o6_wordVarInc);
	/* 50 */
	OPCODE(0x52, o6_byteArrayInc);
	OPCODE(0x53, o6_wordArrayInc);
	/* 54 */
	OPCODE(0x56, o6_byteVarDec);
	OPCODE(0x57, o6_wordVarDec);
	/* 58 */
	OPCODE(0x5a, o6_byteArrayDec);
	OPCODE(0x5b, o6_wordArrayDec);
	/* 5C */
	OPCODE(0x5c, o6_if);
	OPCODE(0x5d, o6_ifNot);
	OPCODE(0x5e, o6_startScript);
	OPCODE(0x5f, o6_startScriptQuick);
	/* 60 */
	OPCODE(0x60, o6_startObject);
	OPCODE(0x61, o6_drawObject);
	OPCODE(0x62, o6_drawObjectAt);
	/* 64 */
	OPCODE(0x65, o6_stopObjectCode);
	OPCODE(0x66, o6_stopObjectCode);
	OPCODE(0x67, o6_endCutscene);
	/* 68 */
	OPCODE(0x68, o6_cutscene);
	OPCODE(0x69, o6_stopMusic);
	OPCODE(0x6a, o6_freezeUnfreeze);
	OPCODE(0x6b, o6_cursorCommand);
	/* 6C */
	OPCODE(0x6c, o6_breakHere);
	OPCODE(0x6d, o6_ifClassOfIs);
	OPCODE(0x6e, o6_setClass);
	OPCODE(0x6f, o6_getState);
	/* 70 */
	OPCODE(0x70, o60_setState);
	OPCODE(0x71, o6_setOwner);
	OPCODE(0x72, o6_getOwner);
	OPCODE(0x73, o6_jump);
	/* 74 */
	OPCODE(0x74, o70_startSound);
	OPCODE(0x75, o6_stopSound);
	OPCODE(0x76, o6_startMusic);
	OPCODE(0x77, o6_stopObjectScript);
	/* 78 */
	OPCODE(0x78, o6_panCameraTo);
	OPCODE(0x79, o6_actorFollowCamera);
	OPCODE(0x7a, o6_setCameraAt);
	OPCODE(0x7b, o6_loadRoom);
	/* 7C */
	OPCODE(0x7c, o6_stopScript);
	OPCODE(0x7d, o6_walkActorToObj);
	OPCODE(0x7e, o6_walkActorTo);
	OPCODE(0x7f, o6_putActorAtXY);
	/* 80 */
	OPCODE(0x80, o6_putActorAtObject);
	OPCODE(0x81, o6_faceActor);
	OPCODE(0x82, o6_animateActor);
	OPCODE(0x83, o6_doSentence);
	/* 84 */
	OPCODE(0x84, o70_pickupObject);
	OPCODE(0x85, o6_loadRoomWithEgo);
	OPCODE(0x87, o6_getRandomNumber);
	/* 88 */
	OPCODE(0x88, o6_getRandomNumberRange);
	OPCODE(0x8a, o6_getActorMoving);
	OPCODE(0x8b, o6_isScriptRunning);
	/* 8C */
	OPCODE(0x8c, o70_getActorRoom);
	OPCODE(0x8d, o6_getObjectX);
	OPCODE(0x8e, o6_getObjectY);
	OPCODE(0x8f, o6_getObjectOldDir);
	/* 90 */
	OPCODE(0x90, o6_getActorWalkBox);
	OPCODE(0x91, o6_getActorCostume);
	OPCODE(0x92, o6_findInventory);
	OPCODE(0x93, o6_getInventoryCount);
	/* 94 */
	OPCODE(0x94, o6_getVerbFromXY);
	OPCODE(0x95, o6_beginOverride);
	OPCODE(0x96, o6_endOverride);
	OPCODE(0x97, o6_setObjectName);
	/* 98 */
	OPCODE(0x98, o6_isSoundRunning);
	OPCODE(0x99, o6_setBoxFlags);
	OPCODE(0x9b, o70_resourceRoutines);
	/* 9C */
	OPCODE(0x9c, o60_roomOps);
	OPCODE(0x9d, o60_actorOps);
	OPCODE(0x9e, o6_verbOps);
	OPCODE(0x9f, o6_getActorFromXY);
	/* A0 */
	OPCODE(0xa0, o6_findObject);
	OPCODE(0xa1, o6_pseudoRoom);
	OPCODE(0xa2, o6_getActorElevation);
	OPCODE(0xa3, o6_getVerbEntrypoint);
	/* A4 */
	OPCODE(0xa4, o6_arrayOps);
	OPCODE(0xa5, o6_saveRestoreVerbs);
	OPCODE(0xa6, o6_drawBox);
	OPCODE(0xa7, o6_pop);
	/* A8 */
	OPCODE(0xa8, o6_getActorWidth);
	OPCODE(0xa9, o6_wait);
	OPCODE(0xaa, o6_getActorScaleX);
	OPCODE(0xab, o6_getActorAnimCounter);
	/* AC */
	OPCODE(0xad, o6_isAnyOf);
	OPCODE(0xae, o70_systemOps);
	OPCODE(0xaf, o6_isActorInBox);
	/* B0 */
	OPCODE(0xb0, o6_delay);
	OPCODE(0xb1, o6_delaySeconds);
	OPCODE(0xb2, o6_delayMinutes);
	OPCODE(0xb3, o6_stopSentence);
	/* B4 */
	OPCODE(0xb4, o6_printLine);
	OPCODE(0xb5, o6_printText);
	OPCODE(0xb6, o6_printDebug);
	OPCODE(0xb7, o6_printSystem);
	/* B8 */
	OPCODE(0xb8, o6_printActor);
	OPCODE(0xb9, o6_printEgo);
	OPCODE(0xba, o6_talkActor);
	OPCODE(0xbb, o6_talkEgo);
	/* BC */
	OPCODE(0xbc, o6_dimArray);
	OPCODE(0xbd, o6_stopObjectCode);
	OPCODE(0xbe, o6_startObjectQuick);
	OPCODE(0xbf, o6_startScriptQuick2);
	/* C0 */
	OPCODE(0xc0, o6_dim2dimArray);
	/* C4 */
	OPCODE(0xc4, o6_abs);
	OPCODE(0xc5, o6_distObjectObject);
	OPCODE(0xc6, o6_distObjectPt);
	OPCODE(0xc7, o6_distPtPt);
	/* C8 */
	OPCODE(0xc8, o60_kernelGetFunctions);
	OPCODE(0xc9, o71_kernelSetFunctions);
	OPCODE(0xca, o6_delayFrames);
	OPCODE(0xcb, o6_pickOneOf);
	/* CC */
	OPCODE(0xcc, o6_pickOneOfDefault);
	OPCODE(0xcd, o6_stampObject);
	/* D0 */
	OPCODE(0xd0, o6_getDateTime);
	OPCODE(0xd1, o6_stopTalking);
	OPCODE(0xd2, o6_getAnimateVariable);
	/* D4 */
	OPCODE(0xd4, o6_shuffle);
	OPCODE(0xd5, o6_jumpToScript);
	OPCODE(0xd6, o6_band);
	OPCODE(0xd7, o6_bor);
	/* D8 */
	OPCODE(0xd8, o6_isRoomScriptRunning);
	OPCODE(0xd9, o60_closeFile);
	OPCODE(0xda, o60_openFile);
	OPCODE(0xdb, o60_readFile);
	/* DC */
	OPCODE(0xdc, o60_writeFile);
	OPCODE(0xdd, o6_findAllObjects);
	OPCODE(0xde, o60_deleteFile);
	OPCODE(0xdf, o60_rename);
	/* E0 */
	OPCODE(0xe0, o60_soundOps);
	OPCODE(0xe1, o6_getPixel);
	OPCODE(0xe2, o60_localizeArrayToScript);
	OPCODE(0xe3, o6_pickVarRandom);
	/* E4 */
	OPCODE(0xe4, o6_setBoxSet);
	/* E8 */
	OPCODE(0xe9, o60_seekFilePos);
	OPCODE(0xea, o60_redimArray);
	OPCODE(0xeb, o60_readFilePos);
	/* EC */
	OPCODE(0xec, o71_copyString);
	OPCODE(0xed, o71_getStringWidth);
	OPCODE(0xee, o70_getStringLen);
	OPCODE(0xef, o71_appendString);
	/* F0 */
	OPCODE(0xf0, o71_concatString);
	OPCODE(0xf1, o71_compareString);
	OPCODE(0xf2, o70_isResourceLoaded);
	OPCODE(0xf3, o70_readINI);
	/* F4 */
	OPCODE(0xf4, o70_writeINI);
	OPCODE(0xf5, o71_getStringLenForWidth);
	OPCODE(0xf6, o71_getCharIndexInString);
	OPCODE(0xf7, o71_findBox);
	/* F8 */
	OPCODE(0xf9, o70_createDirectory);
	OPCODE(0xfa, o70_setSystemMessage);
	OPCODE(0xfb, o71_polygonOps);
	/* FC */
	OPCODE(0xfc, o71_polygonHit);
}

byte *ScummEngine_v71he::heFindResourceData(uint32 tag, byte *ptr) {
	ptr = heFindResource(tag, ptr);

	if (ptr == NULL)
		return NULL;
	return ptr + _resourceHeaderSize;
}

byte *ScummEngine_v71he::heFindResource(uint32 tag, byte *searchin) {
	uint32 curpos, totalsize, size;

	debugC(DEBUG_RESOURCE, "heFindResource(%s, %lx)", tag2str(tag), searchin);

	assert(searchin);
	searchin += 4;
	_resourceLastSearchSize = totalsize = READ_BE_UINT32(searchin);
	curpos = 8;
	searchin += 4;

	while (curpos < totalsize) {
		if (READ_BE_UINT32(searchin) == tag) {
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

byte *ScummEngine_v71he::findWrappedBlock(uint32 tag, byte *ptr, int state, bool errorFlag) {
	if (READ_BE_UINT32(ptr) == MKID_BE('MULT')) {
		byte *offs, *wrap;
		uint32 size;

		wrap = heFindResource(MKID_BE('WRAP'), ptr);
		if (wrap == NULL)
			return NULL;

		offs = heFindResourceData(MKID_BE('OFFS'), wrap);
		if (offs == NULL)
			return NULL;

		size = getResourceDataSize(offs) / 4;
		assert((uint32)state <= (uint32)size);


		offs += READ_LE_UINT32(offs + state * sizeof(uint32));
		offs = heFindResourceData(tag, offs - 8);
		if (offs)
			return offs;

		offs = heFindResourceData(MKID_BE('DEFA'), ptr);
		if (offs == NULL)
			return NULL;

		return heFindResourceData(tag, offs - 8);
	} else {
		return heFindResourceData(tag, ptr);
	}
}

int ScummEngine_v71he::getStringCharWidth(byte chr) {
	int charset = _string[0]._default.charset;

	byte *ptr = getResourceAddress(rtCharset, charset);
	assert(ptr);
	ptr += 29;

	int spacing = 0;

	int offs = READ_LE_UINT32(ptr + chr * 4 + 4);
	if (offs) {
		spacing = ptr[offs] + (signed char)ptr[offs + 2];
	}

	return spacing;
}

int ScummEngine_v71he::setupStringArray(int size) {
	writeVar(0, 0);
	defineArray(0, kStringArray, 0, size + 1);
	writeArray(0, 0, 0, 0);
	return readVar(0);
}

void ScummEngine_v71he::appendSubstring(int dst, int src, int srcOffs, int len) {
	int dstOffs, value;
	int i = 0;

	if (len == -1) {
		len = resStrLen(getStringAddress(src));
		srcOffs = 0;
	}

	dstOffs = resStrLen(getStringAddress(dst));

	len -= srcOffs;
	len++;

	while (i < len) {
		writeVar(0, src);
		value = readArray(0, 0, srcOffs + i);
		writeVar(0, dst);
		writeArray(0, 0, dstOffs + i, value);
		i++;
	}

	writeArray(0, 0, dstOffs + i, 0);
}

void ScummEngine_v71he::adjustRect(Common::Rect &rect) {
	// Scripts can set all rect positions to -1
	if (rect.right != -1)
		rect.right += 1;

	if (rect.bottom != -1)
		rect.bottom += 1;
}

void ScummEngine_v71he::o71_kernelSetFunctions() {
	int args[29];
	int num;
	ActorHE *a;

	num = getStackList(args, ARRAYSIZE(args));

	switch (args[0]) {
	case 1:
		// Used to restore images when decorating cake in
		// Fatty Bear's Birthday Surprise
		virtScreenLoad(args[1], args[2], args[3], args[4], args[5]);
		break;
	case 20: // HE72+
		a = (ActorHE *)derefActor(args[1], "o71_kernelSetFunctions: 20");
		queueAuxBlock(a);
		break;
	case 21:
		_skipDrawObject = 1;
		break;
	case 22:
		_skipDrawObject = 0;
		break;
	case 23:
		clearCharsetMask();
		_fullRedraw = true;
		break;
	case 24:
		_skipProcessActors = 1;
		redrawAllActors();
		break;
	case 25:
		_skipProcessActors = 0;
		redrawAllActors();
		break;
	case 26:
		a = (ActorHE *)derefActor(args[1], "o71_kernelSetFunctions: 26");
		a->_auxBlock.r.left = 0;
		a->_auxBlock.r.right = -1;
		a->_auxBlock.r.top = 0;
		a->_auxBlock.r.bottom = -2;
		break;
	case 30:
		a = (ActorHE *)derefActor(args[1], "o71_kernelSetFunctions: 30");
		a->_clipOverride.bottom = args[2];
		break;
	case 42:
		_wiz->_rectOverrideEnabled = true;
		_wiz->_rectOverride.left = args[1];
		_wiz->_rectOverride.top = args[2];
		_wiz->_rectOverride.right = args[3];
		_wiz->_rectOverride.bottom = args[4];
		adjustRect(_wiz->_rectOverride);
		break;
	case 43:
		_wiz->_rectOverrideEnabled = false;
		break;
	default:
		error("o71_kernelSetFunctions: default case %d (param count %d)", args[0], num);
	}
}

void ScummEngine_v71he::o71_copyString() {
	int dst, size;
	int src = pop();

	size = resStrLen(getStringAddress(src)) + 1;
	dst = setupStringArray(size);

	appendSubstring(dst, src, -1, -1);

	push(dst);
}

void ScummEngine_v71he::o71_getStringWidth() {
	int array, pos, len;
	int chr, width = 0;

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
		if (chr == 0)
			break;
		width += getStringCharWidth(chr);
		pos++;
	}

	push(width);
}

void ScummEngine_v71he::o71_appendString() {
	int dst, size;

	int len = pop();
	int srcOffs = pop();
	int src = pop();

	size = len - srcOffs + 2;
	dst = setupStringArray(size);

	appendSubstring(dst, src, srcOffs, len);

	push(dst);
}

void ScummEngine_v71he::o71_concatString() {
	int dst, size;

	int src2 = pop();
	int src1 = pop();

	size = resStrLen(getStringAddress(src1));
	size += resStrLen(getStringAddress(src2)) + 1;
	dst = setupStringArray(size);

	appendSubstring(dst, src1, 0, -1);
	appendSubstring(dst, src2, 0, -1);

	push(dst);
}

void ScummEngine_v71he::o71_compareString() {
	int result;

	int array1 = pop();
	int array2 = pop();

	byte *string1 = getStringAddress(array1);
	if (!string1)
		error("o71_compareString: Reference to zeroed array pointer (%d)", array1);

	byte *string2 = getStringAddress(array2);
	if (!string2)
		error("o71_compareString: Reference to zeroed array pointer (%d)", array2);

	while (*string1 == *string2) {
		if (*string2 == 0) {
			push(0);
			return;
		}

		string1++;
		string2++;
	}

	result = (*string1 > *string2) ? -1 : 1;
	push(result);
}

void ScummEngine_v71he::o71_getStringLenForWidth() {
	int chr, max;
	int array, len, pos, width = 0;

	max = pop();
	pos = pop();
	array = pop();

	len = resStrLen(getStringAddress(array));

	writeVar(0, array);
	while (pos <= len) {
		chr = readArray(0, 0, pos);
		width += getStringCharWidth(chr);
		if (width >= max) {
			push(pos);
			return;
		}
		pos++;
	}

	push(len);
}

void ScummEngine_v71he::o71_getCharIndexInString() {
	int array, end, len, pos, value;

	value = pop();
	end = pop();
	pos = pop();
	array = pop();

	if (end >= 0) {
		len = resStrLen(getStringAddress(array));
		if (len < end)
			end = len;
	} else {
		end = 0;
	}

	if (pos < 0)
		pos = 0;

	writeVar(0, array);
	if (end > pos) {
		while (end >= pos) {
			if (readArray(0, 0, pos) == value) {
				push(pos);
				return;
			}
			pos++;
		}
	} else {
		while (end <= pos) {
			if (readArray(0, 0, pos) == value) {
				push(pos);
				return;
			}
			pos--;
		}
	}

	push(-1);
}

void ScummEngine_v71he::o71_findBox() {
	int y = pop();
	int x = pop();
	push(getSpecialBox(x, y));
}

void ScummEngine_v71he::o71_polygonOps() {
	int vert1x, vert1y, vert2x, vert2y, vert3x, vert3y, vert4x, vert4y;
	int id, fromId, toId;
	bool flag;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 68: // HE 100
	case 69: // HE 100
	case 246:
	case 248:
		vert4y = pop();
		vert4x = pop();
		vert3y = pop();
		vert3x = pop();
		vert2y = pop();
		vert2x = pop();
		vert1y = pop();
		vert1x = pop();
		flag = (subOp == 69 || subOp == 248);
		id = pop();
		_wiz->polygonStore(id, flag, vert1x, vert1y, vert2x, vert2y, vert3x, vert3y, vert4x, vert4y);
		break;
	case 28: // HE 100
	case 247:
		toId = pop();
		fromId = pop();
		_wiz->polygonErase(fromId, toId);
		break;
	default:
		error("o71_polygonOps: default case %d", subOp);
	}
}

void ScummEngine_v71he::o71_polygonHit() {
	int y = pop();
	int x = pop();
	push(_wiz->polygonHit(0, x, y));
}

} // End of namespace Scumm
