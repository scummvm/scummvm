/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
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

#include "common/stdafx.h"
#include "common/endian.h"

#include "cine/cine.h"
#include "cine/bg_list.h"
#include "cine/object.h"
#include "cine/sfx_player.h"
#include "cine/sound_driver.h"
#include "cine/various.h"

namespace Cine {

enum {
	kCmpEQ = (1 << 0),
	kCmpGT = (1 << 1),
	kCmpLT = (1 << 2)
};

prcLinkedListStruct *_currentScriptElement;
byte *_currentScriptPtr;
uint16 _currentScriptParams;
uint16 _currentPosition;
uint16 _currentLine;
uint16 _closeScript;

typedef void (*OpcodeProc) ();
const OpcodeProc *_opcodeTable;
int _numOpcodes;

void setupOpcodes() {
	static const OpcodeProc opcodeTableFW[] = {
		/* 00 */
		o1_modifyObjectParam,
		o1_getObjectParam,
		o1_addObjectParam,
		o1_subObjectParam,
		/* 04 */
		o1_add2ObjectParam,
		o1_sub2ObjectParam,
		o1_compareObjectParam,
		o1_setupObject,
		/* 08 */
		o1_checkCollision,
		o1_loadVar,
		o1_addVar,
		o1_subVar,
		/* 0C */
		o1_mulVar,
		o1_divVar,
		o1_compareVar,
		o1_modifyObjectParam2,
		/* 10 */
		NULL,
		NULL,
		NULL,
		o1_loadMask0,
		/* 14 */
		o1_unloadMask0,
		o1_addToBgList,
		o1_loadMask1,
		o1_unloadMask1,
		/* 18 */
		o1_loadMask4,
		o1_unloadMask4,
		o1_addSpriteFilledToBgList,
		o1_op1B,
		/* 1C */
		NULL,
		o1_label,
		o1_goto,
		o1_gotoIfSup,
		/* 20 */
		o1_gotoIfSupEqu,
		o1_gotoIfInf,
		o1_gotoIfInfEqu,
		o1_gotoIfEqu,
		/* 24 */
		o1_gotoIfDiff,
		o1_removeLabel,
		o1_loop,
		NULL,
		/* 28 */
		NULL,
		NULL,
		NULL,
		NULL,
		/* 2C */
		NULL,
		NULL,
		NULL,
		NULL,
		/* 30 */
		NULL,
		o1_startGlobalScript,
		o1_endGlobalScript,
		NULL,
		/* 34 */
		NULL,
		NULL,
		NULL,
		NULL,
		/* 38 */
		NULL,
		NULL,
		NULL,
		o1_loadAnim,
		/* 3C */
		o1_loadBg,
		o1_loadCt,
		NULL,
		o1_loadPart,
		/* 40 */
		o1_closePart,
		o1_loadNewPrcName,
		o1_requestCheckPendingDataLoad,
		NULL,
		/* 44 */
		NULL,
		o1_blitAndFade,
		o1_fadeToBlack,
		o1_transformPaletteRange,
		/* 48 */
		NULL,
		o1_setDefaultMenuColor2,
		o1_palRotate,
		NULL,
		/* 4C */
		NULL,
		NULL,
		NULL,
		o1_break,
		/* 50 */
		o1_endScript,
		o1_message,
		o1_loadGlobalVar,
		o1_compareGlobalVar,
		/* 54 */
		NULL,
		NULL,
		NULL,
		NULL,
		/* 58 */
		NULL,
		o1_declareFunctionName,
		o1_freePartRange,
		o1_unloadAllMasks,
		// 5C */
		NULL,
		NULL,
		NULL,
		NULL,
		/* 60 */
		NULL,
		NULL,
		NULL,
		o1_op63,
		/* 64 */
		o1_op64,
		o1_initializeZoneData,
		o1_setZoneDataEntry,
		o1_getZoneDataEntry,
		/* 68 */
		o1_setDefaultMenuColor,
		o1_allowPlayerInput,
		o1_disallowPlayerInput,
		o1_changeDataDisk,
		/* 6C */
		NULL,
		o1_loadMusic,
		o1_playMusic,
		o1_fadeOutMusic,
		/* 70 */
		o1_stopSample,
		o1_op71,
		o1_op72,
		o1_op73,
		/* 74 */
		NULL,
		NULL,
		NULL,
		o1_playSample,
		/* 78 */
		o1_playSample,
		o1_allowSystemMenu,
		o1_loadMask5,
		o1_unloadMask5
	};

	// TODO: We need to verify the Operation Stealth opcodes.

	static const OpcodeProc opcodeTableOS[] = {
		/* 00 */
		o1_modifyObjectParam,
		o1_getObjectParam,
		o1_addObjectParam,
		o1_subObjectParam,
		/* 04 */
		o1_add2ObjectParam,
		o1_sub2ObjectParam,
		o1_compareObjectParam,
		o1_setupObject,
		/* 08 */
		o1_checkCollision,
		o1_loadVar,
		o1_addVar,
		o1_subVar,
		/* 0C */
		o1_mulVar,
		o1_divVar,
		o1_compareVar,
		o1_modifyObjectParam2,
		/* 10 */
		NULL,
		NULL,
		NULL,
		o1_loadMask0,
		/* 14 */
		o1_unloadMask0,
		o1_addToBgList,
		o1_loadMask1,
		o1_unloadMask1,
		/* 18 */
		o1_loadMask4,
		o1_unloadMask4,
		o1_addSpriteFilledToBgList,
		o1_op1B,
		/* 1C */
		NULL,
		o1_label,
		o1_goto,
		o1_gotoIfSup,
		/* 20 */
		o1_gotoIfSupEqu,
		o1_gotoIfInf,
		o1_gotoIfInfEqu,
		o1_gotoIfEqu,
		/* 24 */
		o1_gotoIfDiff,
		o1_removeLabel,
		o1_loop,
		NULL,
		/* 28 */
		NULL,
		NULL,
		NULL,
		NULL,
		/* 2C */
		NULL,
		NULL,
		NULL,
		NULL,
		/* 30 */
		NULL,
		o1_startGlobalScript,
		o1_endGlobalScript,
		NULL,
		/* 34 */
		NULL,
		NULL,
		NULL,
		NULL,
		/* 38 */
		NULL,
		NULL,
		NULL,
		o1_loadAnim,
		/* 3C */
		o1_loadBg,
		o1_loadCt,
		NULL,
		o2_loadPart,
		/* 40 */
		NULL,
		o1_loadNewPrcName,
		o1_requestCheckPendingDataLoad,
		NULL,
		/* 44 */
		NULL,
		o1_blitAndFade,
		o1_fadeToBlack,
		o1_transformPaletteRange,
		/* 48 */
		NULL,
		o1_setDefaultMenuColor2,
		o1_palRotate,
		NULL,
		/* 4C */
		NULL,
		NULL,
		NULL,
		o1_break,
		/* 50 */
		o1_endScript,
		o1_message,
		o1_loadGlobalVar,
		o1_compareGlobalVar,
		/* 54 */
		NULL,
		NULL,
		NULL,
		NULL,
		/* 58 */
		NULL,
		o1_declareFunctionName,
		o1_freePartRange,
		o1_unloadAllMasks,
		// 5C */
		NULL,
		NULL,
		NULL,
		NULL,
		/* 60 */
		NULL,
		NULL,
		NULL,
		o1_op63,
		/* 64 */
		o1_op64,
		o1_initializeZoneData,
		o1_setZoneDataEntry,
		o1_getZoneDataEntry,
		/* 68 */
		o1_setDefaultMenuColor,
		o1_allowPlayerInput,
		o1_disallowPlayerInput,
		o1_changeDataDisk,
		/* 6C */
		NULL,
		o1_loadMusic,
		o1_playMusic,
		o1_fadeOutMusic,
		/* 70 */
		o1_stopSample,
		o1_op71,
		o1_op72,
		o1_op72,
		/* 74 */
		NULL,
		NULL,
		NULL,
		o1_playSample,
		/* 78 */
		o2_op78,
		o1_allowSystemMenu,
		o1_loadMask5,
		o1_unloadMask5,
		/* 7C */
		NULL,
		NULL,
		NULL,
		o2_addSeqListElement,
		/* 80 */
		o2_removeSeq,
		o2_op81,
		o2_op82,
		o2_isSeqRunning,
		/* 84 */
		o2_gotoIfSupNearest,
		o2_gotoIfSupEquNearest,
		o2_gotoIfInfNearest,
		o2_gotoIfInfEquNearest,
		/* 88 */
		o2_gotoIfEquNearest,
		o2_gotoIfDiffNearest,
		NULL,
		o2_startObjectScript,
		/* 8C */
		o2_stopObjectScript,
		o2_op8D,
		o2_addBackground,
		o2_removeBackground,
		/* 90 */
		o2_loadAbs,
		o2_loadBg,
		NULL,
		NULL,
		/* 94 */
		NULL,
		o1_changeDataDisk,
		NULL,
		NULL,
		/* 98 */
		NULL,
		NULL,
		o2_wasZoneChecked,
		o2_op9B,
		/* 9C */
		o2_op9C,
		o2_useBgScroll,
		o2_setAdditionalBgVScroll,
		o2_op9F,
		/* A0 */
		o2_addGfxElementA0,
		o2_opA1,
		o2_opA2,
		o2_opA3,
		/* A4 */
		o2_loadMask22,
		o2_unloadMask22,
		NULL,
		NULL,
		/* A8 */
		NULL,
		o1_changeDataDisk
	};

	if (gameType == Cine::GID_FW) {
		_opcodeTable = opcodeTableFW;
		_numOpcodes = ARRAYSIZE(opcodeTableFW);
	} else {
		_opcodeTable = opcodeTableOS;
		_numOpcodes = ARRAYSIZE(opcodeTableOS);
	}
}

byte getNextByte() {
	byte val = *(_currentScriptPtr + _currentPosition);
	_currentPosition++;
	return val;
}

uint16 getNextWord() {
	uint16 val = READ_BE_UINT16(_currentScriptPtr + _currentPosition);
	_currentPosition += 2;
	return val;
}

const char *getNextString() {
	const char *val = (const char *)(_currentScriptPtr + _currentPosition);
	_currentPosition += strlen(val);
	return val;
}

void addGfxElementA0(int16 param1, int16 param2) {
	overlayHeadElement *currentHead = &overlayHead;
	overlayHeadElement *tempHead = currentHead;
	overlayHeadElement *newElement;

	currentHead = tempHead->next;

	while (currentHead) {
		if (objectTable[currentHead->objIdx].mask == objectTable[param1].mask) {
			if (currentHead->type == 2 || currentHead->objIdx == 3) {
				break;
			}
		}

		tempHead = currentHead;
		currentHead = currentHead->next;
	}

	if (currentHead && currentHead->objIdx == param1 && currentHead->type == 20 && currentHead->x == param2)
		return;

	newElement = (overlayHeadElement *)malloc(sizeof(overlayHeadElement));

	newElement->next = tempHead->next;
	tempHead->next = newElement;

	newElement->objIdx = param1;
	newElement->type = 20;

	newElement->x = param2;
	newElement->y = 0;
	newElement->width = 0;
	newElement->color = 0;

	if (!currentHead)
		currentHead = &overlayHead;

	newElement->previous = currentHead->previous;

	currentHead->previous = newElement;
}

void removeSeq(uint16 param1, uint16 param2, uint16 param3) {
	SeqListElement *currentHead = &seqList;
	SeqListElement *tempHead = currentHead;

	while (currentHead && (currentHead->var6 != param1 || currentHead->var4 != param2 || currentHead->varE != param3)) {
		tempHead = currentHead;
		currentHead = tempHead->next;
	}

	if (currentHead && currentHead->var6 == param1 && currentHead->var4 == param2 && currentHead->varE == param3) {
		currentHead->var4 = -1;
	}
}

uint16 isSeqRunning(uint16 param1, uint16 param2, uint16 param3) {
	SeqListElement *currentHead = &seqList;
	SeqListElement *tempHead = currentHead;

	while (currentHead && (currentHead->var6 != param1 || currentHead->var4 != param2 || currentHead->varE != param3)) {
		tempHead = currentHead;
		currentHead = tempHead->next;
	}

	if (currentHead && currentHead->var6 == param1 && currentHead->var4 == param2 && currentHead->varE == param3) {
		return 1;
	}

	return 0;
}

scriptStruct scriptTable[NUM_MAX_SCRIPT];

void createVar9Element(int16 objIdx, int16 param) {
}

void addToBGList(int16 objIdx) {
	int16 x;
	int16 y;
	int16 width;
	int16 height;
	int16 part;

	x = objectTable[objIdx].x;
	y = objectTable[objIdx].y;

	width = animDataTable[objectTable[objIdx].frame].var1;
	height = animDataTable[objectTable[objIdx].frame].height;

	part = objectTable[objIdx].part;

	if (gameType == Cine::GID_OS) {
		drawSpriteRaw2(animDataTable[objectTable[objIdx].frame].ptr1, objectTable[objIdx].part, width, height, page2Raw, x, y);
	} else {
		drawSpriteRaw(animDataTable[objectTable[objIdx].frame].ptr1, animDataTable[objectTable[objIdx].frame].ptr2, width, height, page2Raw, x, y);
	}

	createVar9Element(objIdx, 0);
}

void stopGlobalScript(uint16 scriptIdx) {
	prcLinkedListStruct *currentHead = &globalScriptsHead;
	prcLinkedListStruct *tempHead = currentHead;

	currentHead = tempHead->next;

	while (currentHead && (currentHead->scriptIdx != scriptIdx)) {
		tempHead = currentHead;
		currentHead = tempHead->next;
	}

	if (!currentHead) {
		return;
	}

	if (currentHead->scriptIdx != scriptIdx) {
		return;
	}

	currentHead->scriptIdx = -1;
}

uint16 computeScriptStackSub(byte mode, byte *scriptPtr, int16 *stackPtr, uint16 scriptSize, byte param1, uint16 startOffset) {
	byte *localScriptPtr = scriptPtr;
	uint16 exitScript;
	uint16 i;
	uint16 position;
	uint16 di;

	assert(scriptPtr);
	assert(stackPtr);

	if (mode == 1) {
		for (i = 0; i < SCRIPT_STACK_SIZE; i++) {
			stackPtr[i] = -1;
		}

		position = 0;
	} else {
		position = startOffset;
	}

	exitScript = 0;

	do {
		uint16 opcode = *(localScriptPtr + position);
		position++;

		//printf("Opcode: %X\n",opcode-1);

		switch (opcode - 1) {
		case -1:
		case 0x1B:
			{
				break;
			}
		case 0x89:
		case 0x32:
		case 0x7A:
		case 0x91:
		case 0x9D:
		case 0x8F:
		case 0x7B:
		case 0x8C:
		case 0x8B:
		case 0x85:
		case 0x86:
		case 0x84:
		case 0x88:
			{
				position++;
				break;
			}
		case 0x80:
		case 0x83:
		case 0x26:
			{
				position += 2;
				break;
			}
		case 0xF:
		case 0x1:
		case 0x66:
		case 0x4A:
			{
				position += 3;
				break;
			}
		case 0x0:
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0xA0:
		case 0xA1:
		case 0xA2:
		case 0xA3:
			{
				position += 4;
				break;
			}
		case 0x9:
		case 0xA:
		case 0xB:
		case 0xC:
		case 0xD:
		case 0xE:
		case 0x52:
		case 0x53:
			{
				byte param;
				position++;

				param = *(localScriptPtr + position);
				position++;

				if (param) {
					position++;
				} else {
					position += 2;
				}
				break;
			}
		case 0x9E:
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				if (param) {
					position++;
				} else {
					position += 2;
				}
				break;
			}
		case 0x82:
			{
				position += 7;
				break;
			}
		case 0x47:
			{
				position += 8;
				break;
			}
		case 0x51:
		case 0x7:
		case 0x77:
		case 0x78:
		case 0x8:
			{
				position += 9;
				break;
			}
		case 0x7F:
			{
				position += 10;
				break;
			}
		case 0x1D:
			{
				di = *(localScriptPtr + position);
				position++;

				if (mode == 1) {
					stackPtr[di] = position;
				} else {
					if (param1 == di) {
						return position;
					}
				}

				break;
			}
		case 0x59:
		case 0x3B:
		case 0x3C:
		case 0x3D:
		case OP_loadPart:	// skipString
		case 0x6D:
		case 0x8E:
			{
				do {
					position++;
				} while (*(localScriptPtr + position));
				break;
			}
		case 0x90:
		case OP_loadNewPrcName:	//skipVarAndString
			{
				di = *(localScriptPtr + position);
				position++;

				do {
					position++;
				} while (*(localScriptPtr + position));

				break;
			}
		case 0x46:
		case 0x65:
		case 0x4F:
		case 0x40:
		case 0x6A:
		case 0x69:
		case 0x45:
		case 0x6E:
		case 0x6F:
		case 0x70:
			{
				break;
			}
		case 0x1E:
		case 0x1F:
		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
		case 0x24:
		case 0x25:
		case 0x68:
		case 0x49:
		case 0x31:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1A:
			{
				position++;
				break;
			}
		case 0x5A:
			{
				position += 2;
				break;
			}
		case 0x5B:
			{
				break;
			}
		case OP_changeDataDisk:	// skipVar
		case OP_79:
			{
				di = *(localScriptPtr + position);
				position++;

				break;
			}
		case OP_endScript:	// end
			{
				exitScript = 1;
				break;
			}
		case OP_requestCheckPendingDataLoad:	// nop
			{
				break;
			}
		default:
			{
				error
				    ("Unsupported opcode %X in computeScriptStack",
				    opcode - 1);
			}
		}

		if (position > scriptSize) {
			exitScript = 1;
		}

	} while (!exitScript);

	return position;
}

void computeScriptStack(byte *scriptPtr, int16 *stackPtr, uint16 scriptSize) {
	computeScriptStackSub(1, scriptPtr, stackPtr, scriptSize, 0, 0);
}

uint16 computeScriptStackFromScript(byte *scriptPtr, uint16 currentPosition, uint16 labelIdx, uint16 scriptSize) {
	return computeScriptStackSub(0, scriptPtr, (int16 *)&dummyU16, (uint16)scriptSize, labelIdx, currentPosition);
}

void palRotate(byte a, byte b, byte c) {
	int16 i;
	uint16 currentColor;

	if (c == 1) {
		currentColor = c_palette[b];

		for (i = b; i > a; i--) {
			c_palette[i] = c_palette[i - 1];
		}

		c_palette[a] = currentColor;
	}
}

void addScriptToList0(uint16 idx) {
	uint16 i;
	prcLinkedListStruct *pNewElement;
	prcLinkedListStruct *currentHead = &globalScriptsHead;
	prcLinkedListStruct *tempHead = currentHead;

	assert(idx <= NUM_MAX_SCRIPT);

	currentHead = tempHead->next;

	while (currentHead) {
		tempHead = currentHead;

		assert(tempHead);

		currentHead = tempHead->next;
	}

	pNewElement =(prcLinkedListStruct *)malloc(sizeof(prcLinkedListStruct));

	assert(pNewElement);

	pNewElement->next = tempHead->next;
	tempHead->next = pNewElement;

	// copy the stack into the script instance
	for (i = 0; i < SCRIPT_STACK_SIZE; i++) {
		pNewElement->stack[i] = scriptTable[idx].stack[i];
	}

	for (i = 0; i < 50; i++) {
		pNewElement->localVars[i] = 0;
	}

	pNewElement->compareResult = 0;
	pNewElement->scriptPosition = 0;

	pNewElement->scriptPtr = scriptTable[idx].ptr;
	pNewElement->scriptIdx = idx;
}

#ifdef _DEBUG
#define DEBUG_SCRIPT debugScript
void debugScript(int currentLine, const char *string, ...) {
	va_list va;

	va_start(va, string);
	vprintf(string, va);
	va_end(va);
	printf("\n");
}
#else
#define DEBUG_SCRIPT debugScriptInline
void debugScriptInline(int currentLine, const char *string, ...) {
}
#endif

int16 endScript0(uint16 scriptIdx) {
	prcLinkedListStruct *currentHead = &globalScriptsHead;
	prcLinkedListStruct *tempHead = currentHead;

	//assert(scriptIdx <= NUM_MAX_SCRIPT);

	currentHead = tempHead->next;

	while (currentHead && currentHead->scriptIdx != scriptIdx) {
		tempHead = currentHead;
		currentHead = tempHead->next;
	}

	if (!currentHead) {
		return -1;
	}

	if (currentHead->scriptIdx != scriptIdx) {
		return -1;
	}

	currentHead->scriptIdx = -1;

	return 0;
}

int16 endScript1(uint16 scriptIdx) {
	prcLinkedListStruct *currentHead = &objScriptList;
	prcLinkedListStruct *tempHead = currentHead;

	currentHead = tempHead->next;

	while (currentHead && currentHead->scriptIdx != scriptIdx) {
		tempHead = currentHead;
		currentHead = tempHead->next;
	}

	if (!currentHead) {
		return -1;
	}

	if (currentHead->scriptIdx != scriptIdx) {
		return -1;
	}

	currentHead->scriptIdx = -1;

	return 0;
}

int16 getZoneFromPosition(byte *page, int16 x, int16 y, int16 width) {
	byte *ptr = page + (y * width) + x / 2;
	byte zoneVar;

	if (!(x % 2)) {
		zoneVar = (*(ptr) >> 4) & 0xF;
	} else {
		zoneVar = (*(ptr)) & 0xF;
	}

	return zoneVar;
}

int16 getZoneFromPositionRaw(byte *page, int16 x, int16 y, int16 width) {
	byte *ptr = page + (y * width) + x;
	byte zoneVar;

	zoneVar = (*(ptr)) & 0xF;

	return zoneVar;
}

int16 checkCollision(int16 objIdx, int16 x, int16 y, int16 numZones, int16 zoneIdx) {
	int16 i;
	int16 lx;
	int16 ly;

	lx = objectTable[objIdx].x + x;
	ly = objectTable[objIdx].y + y;

	for (i = 0; i < numZones; i++) {
		int16 idx;

		// if (gameType == GAME_OS)
		{
			idx = getZoneFromPositionRaw(page3Raw, lx + i, ly, 320);
		}
		/*  else
		 * {
		 * idx = getZoneFromPosition(page3, lx + i, ly, 160);
		 * } */

		assert(idx >= 0 && idx <= NUM_MAX_ZONE);

		if (zoneData[idx] == zoneIdx) {
			return 1;
		}
	}

	return 0;
}

uint16 compareVars(int16 a, int16 b) {
	uint16 flag = 0;

	if (a == b) {
		flag |= kCmpEQ;
	}

	if (a > b) {
		flag |= kCmpGT;
	}

	if (a < b) {
		flag |= kCmpLT;
	}

	return flag;
}

// ------------------------------------------------------------------------
// FUTURE WARS opcodes
// ------------------------------------------------------------------------

void o1_modifyObjectParam() {
	byte objIdx = getNextByte();
	byte paramIdx = getNextByte();
	int16 newValue = getNextWord();

	DEBUG_SCRIPT(_currentLine, "modifyObjectParam(objIdx:%d,paramIdx:%d,newValue:%d)", objIdx, paramIdx, newValue);

	modifyObjectParam(objIdx, paramIdx, newValue);
}

void o1_getObjectParam() {
	byte objIdx = getNextByte();
	byte paramIdx = getNextByte();
	byte newValue = getNextByte();

	DEBUG_SCRIPT(_currentLine, "getObjectParam(objIdx:%d,paramIdx:%d,var:%d)", objIdx, paramIdx, newValue);

	_currentScriptElement->localVars[newValue] = getObjectParam(objIdx, paramIdx);
}

void o1_addObjectParam() {
	byte objIdx = getNextByte();
	byte paramIdx = getNextByte();
	int16 newValue = getNextWord();

	DEBUG_SCRIPT(_currentLine, "addObjectParam(objIdx:%d,paramIdx:%d,newValue:%d)", objIdx, paramIdx, newValue);

	addObjectParam(objIdx, paramIdx, newValue);
}

void o1_subObjectParam() {
	byte objIdx = getNextByte();
	byte paramIdx = getNextByte();
	int16 newValue = getNextWord();

	DEBUG_SCRIPT(_currentLine, "subObjectParam(objIdx:%d,paramIdx:%d,newValue:%d)", objIdx, paramIdx, newValue);

	subObjectParam(objIdx, paramIdx, newValue);
}

void o1_add2ObjectParam() {
	warning("STUB: o1_add2ObjectParam()");
}

void o1_sub2ObjectParam() {
	warning("STUB: o1_sub2ObjectParam()");
}

void o1_compareObjectParam() {
	byte objIdx = getNextByte();
	byte param1 = getNextByte();
	int16 param2 = getNextWord();

	DEBUG_SCRIPT(_currentLine, "compareObjectParam(objIdx:%d,type:%d,value:%d)", objIdx, param1, param2);

	_currentScriptElement->compareResult = compareObjectParam(objIdx, param1, param2);
}

void o1_setupObject() {
	byte objIdx = getNextByte();
	int16 param1 = getNextWord();
	int16 param2 = getNextWord();
	int16 param3 = getNextWord();
	int16 param4 = getNextWord();

	DEBUG_SCRIPT(_currentLine, "setupObject(objIdx:%d,%d,%d,%d,%d)", objIdx, param1, param2, param3, param4);

	setupObject(objIdx, param1, param2, param3, param4);
}

void o1_checkCollision() {
	byte objIdx = getNextByte();
	int16 param1 = getNextWord();
	int16 param2 = getNextWord();
	int16 param3 = getNextWord();
	int16 param4 = getNextWord();

	DEBUG_SCRIPT(_currentLine, "checkCollision(objIdx:%d,%d,%d,%d,%d)", objIdx, param1, param2, param3, param4);

	_currentScriptElement->compareResult = checkCollision(objIdx, param1, param2, param3, param4);
}

void o1_loadVar() {
	byte varIdx = getNextByte();
	byte varType = getNextByte();

	if (varType) {
		byte dataIdx = getNextByte();
		int16 var;

		switch (varType) {
		case 1:
			DEBUG_SCRIPT(_currentLine, "var[%d] = var[%d]", varIdx, dataIdx);
			_currentScriptElement->localVars[varIdx] = _currentScriptElement->localVars[dataIdx];
			break;
		case 2:
			DEBUG_SCRIPT(_currentLine, "var[%d] = globalVars[%d]", varIdx, dataIdx);
			_currentScriptElement->localVars[varIdx] = globalVars[dataIdx];
			break;
		case 3:
			DEBUG_SCRIPT(_currentLine, "var[%d] = mouseX", varIdx, dataIdx);
			getMouseData(mouseUpdateStatus, &dummyU16, (uint16 *)&var, (uint16 *)&dummyU16);
			_currentScriptElement->localVars[varIdx] = var;
			break;
		case 4:
			DEBUG_SCRIPT(_currentLine, "var[%d] = mouseY", varIdx, dataIdx);
			getMouseData(mouseUpdateStatus, &dummyU16, (uint16 *)&dummyU16, (uint16 *)&var);
			_currentScriptElement->localVars[varIdx] = var;
			break;
		case 5:
			DEBUG_SCRIPT(_currentLine, "var[%d] = rand mod %d", varIdx, dataIdx);
			_currentScriptElement->localVars[varIdx] = rand() % dataIdx;
			break;
		case 8:
			DEBUG_SCRIPT(_currentLine, "var[%d] = file[%d].packedSize", varIdx, dataIdx);
			_currentScriptElement->localVars[varIdx] = partBuffer[dataIdx].packedSize;
			break;
		case 9:
			DEBUG_SCRIPT(_currentLine, "var[%d] = file[%d].unpackedSize", varIdx, dataIdx);
			_currentScriptElement->localVars[varIdx] = partBuffer[dataIdx].unpackedSize;
			break;
		default:
			error("executeScript: o1_loadVar: Unknown variable type %d", varType);
		}
	} else {
		int16 value = getNextWord();

		DEBUG_SCRIPT(_currentLine, "var[%d] = %d", varIdx, value);
		_currentScriptElement->localVars[varIdx] = value;
	}
}

void o1_addVar() {
	byte varIdx = getNextByte();
	byte varType = getNextByte();

	if (varType) {
		byte dataIdx = getNextByte();

		DEBUG_SCRIPT(_currentLine, "var[%d] += var[%d]", varIdx, dataIdx);
		_currentScriptElement->localVars[varIdx] += _currentScriptElement->localVars[dataIdx];
	} else {
		int16 value = getNextWord();

		DEBUG_SCRIPT(_currentLine, "var[%d] += %d", varIdx, value);
		_currentScriptElement->localVars[varIdx] += value;
	}
}

void o1_subVar() {
	byte varIdx = getNextByte();
	byte varType = getNextByte();

	if (varType) {
		byte dataIdx = getNextByte();

		DEBUG_SCRIPT(_currentLine, "var[%d] -= var[%d]", varIdx, dataIdx);
		_currentScriptElement->localVars[varIdx] -= _currentScriptElement->localVars[dataIdx];
	} else {
		int16 value = getNextWord();

		DEBUG_SCRIPT(_currentLine, "var[%d] -= %d", varIdx, value);
		_currentScriptElement->localVars[varIdx] -= value;
	}
}

void o1_mulVar() {
	byte varIdx = getNextByte();
	byte varType = getNextByte();

	if (varType) {
		byte dataIdx = getNextByte();

		DEBUG_SCRIPT(_currentLine, "var[%d] *= var[%d]", varIdx, dataIdx);
		_currentScriptElement->localVars[varIdx] *= _currentScriptElement->localVars[dataIdx];
	} else {
		int16 value = getNextWord();

		DEBUG_SCRIPT(_currentLine, "var[%d] *= %d", varIdx, value);
		_currentScriptElement->localVars[varIdx] *= value;
	}
}

void o1_divVar() {
	byte varIdx = getNextByte();
	byte varType = getNextByte();

	if (varType) {
		byte dataIdx = getNextByte();

		DEBUG_SCRIPT(_currentLine, "var[%d] /= var[%d]", varIdx, dataIdx);
		_currentScriptElement->localVars[varIdx] /= _currentScriptElement->localVars[dataIdx];
	} else {
		int16 value = getNextWord();

		DEBUG_SCRIPT(_currentLine, "var[%d] /= %d", varIdx, value);
		_currentScriptElement->localVars[varIdx] /= value;
	}
}

void o1_compareVar() {
	byte varIdx = getNextByte();
	byte varType = getNextByte();

	if (varType) {
		byte dataIdx = getNextByte();

		// printf("Val: %d\n", dataIdx);

		if (varType == 1) {
			assert(varIdx < 50);
			assert(dataIdx < 50);

			DEBUG_SCRIPT(_currentLine, "compare var[%d] and var[%d]", varIdx, dataIdx);
			_currentScriptElement->compareResult = compareVars(_currentScriptElement->localVars[varIdx], _currentScriptElement->localVars[dataIdx]);
		} else if (varType == 2) {
			assert(varIdx < 50);

			DEBUG_SCRIPT(_currentLine, "compare var[%d] and globalVar[%d]", varIdx, dataIdx);
			_currentScriptElement->compareResult = compareVars(_currentScriptElement->localVars[varIdx], globalVars[dataIdx]);
		}
	} else {
		int16 value = getNextWord();

		DEBUG_SCRIPT(_currentLine, "compare var[%d] and %d", varIdx, value);
		_currentScriptElement->compareResult = compareVars(_currentScriptElement->localVars[varIdx], value);
	}
}

void o1_modifyObjectParam2() {
	byte objIdx = getNextByte();
	byte paramIdx = getNextByte();
	byte newValue = getNextByte();

	DEBUG_SCRIPT(_currentLine, "modifyObjectParam2(objIdx:%d,paramIdx:%d,var[%d])", objIdx, paramIdx, newValue);

	modifyObjectParam(objIdx, paramIdx, _currentScriptElement->localVars[newValue]);
}

void o1_loadMask0() {
	// OP_loadV7Element
	byte param = getNextByte();

	DEBUG_SCRIPT(_currentLine, "addSpriteOverlay(%d)", param);
	loadOverlayElement(param, 0);
}

void o1_unloadMask0() {
	byte param = getNextByte();

	DEBUG_SCRIPT(_currentLine, "removeSpriteOverlay(%d)", param);
	freeOverlay(param, 0);
}

void o1_addToBgList() {
	byte param = getNextByte();

	DEBUG_SCRIPT(_currentLine, "addToBGList(%d)", param);
	addToBGList(param);
}

void o1_loadMask1() {
	byte param = getNextByte();

	DEBUG_SCRIPT(_currentLine, "addOverlay1(%d)", param);
	loadOverlayElement(param, 1);
}

void o1_unloadMask1() {
	byte param = getNextByte();

	DEBUG_SCRIPT(_currentLine, "removeOverlay1(%d)", param);
	freeOverlay(param, 1);
}

void o1_loadMask4() {
	byte param = getNextByte();

	DEBUG_SCRIPT(_currentLine, "addOverlayType4(%d)", param);
	loadOverlayElement(param, 4);
}

void o1_unloadMask4() {
	byte param = getNextByte();

	DEBUG_SCRIPT(_currentLine, "removeSpriteOverlay4(%d)", param);
	freeOverlay(param, 4);
}

void o1_addSpriteFilledToBgList() {
	byte param = getNextByte();

	DEBUG_SCRIPT(_currentLine, "op1A(%d) -> TODO !", param);
	addSpriteFilledToBGList(param);
}

void o1_op1B() {
	DEBUG_SCRIPT(_currentLine, "closeEngine7");
	closeEngine7();
}

void o1_label() {
	byte labelIdx = getNextByte();

	DEBUG_SCRIPT(_currentLine, "label(%d)", labelIdx);
	_currentScriptElement->stack[labelIdx] = _currentPosition;
}

void o1_goto() {
	byte labelIdx = getNextByte();

	assert(_currentScriptElement->stack[labelIdx] != -1);

	DEBUG_SCRIPT(_currentLine, "goto label(%d)", labelIdx);
	_currentPosition = _currentScriptElement->stack[labelIdx];
}

void o1_gotoIfSup() {
	byte labelIdx = getNextByte();

	if (_currentScriptElement->compareResult == kCmpGT) {
		assert(_currentScriptElement->stack[labelIdx] != -1);

		DEBUG_SCRIPT(_currentLine, "if(>) goto %d (true)", labelIdx);
		_currentPosition = _currentScriptElement->stack[labelIdx];
	} else {
		DEBUG_SCRIPT(_currentLine, "if(>) goto %d (false)", labelIdx);
	}
}

void o1_gotoIfSupEqu() {
	byte labelIdx = getNextByte();

	if (_currentScriptElement->compareResult & (kCmpGT | kCmpEQ)) {
		assert(_currentScriptElement->stack[labelIdx] != -1);

		DEBUG_SCRIPT(_currentLine, "if(>=) goto %d (true)", labelIdx);
		_currentPosition = _currentScriptElement->stack[labelIdx];
	} else {
		DEBUG_SCRIPT(_currentLine, "if(>=) goto %d (false)", labelIdx);
	}
}

void o1_gotoIfInf() {
	byte labelIdx = getNextByte();

	if (_currentScriptElement->compareResult == kCmpLT) {
		assert(_currentScriptElement->stack[labelIdx] != -1);

		DEBUG_SCRIPT(_currentLine, "if(<) goto %d (true)", labelIdx);
		_currentPosition = _currentScriptElement->stack[labelIdx];
	} else {
		DEBUG_SCRIPT(_currentLine, "if(<) goto %d (false)", labelIdx);
	}
}

void o1_gotoIfInfEqu() {
	byte labelIdx = getNextByte();

	if (_currentScriptElement->compareResult & (kCmpLT | kCmpEQ)) {
		assert(_currentScriptElement->stack[labelIdx] != -1);

		DEBUG_SCRIPT(_currentLine, "if(<=) goto %d (true)", labelIdx);
		_currentPosition = _currentScriptElement->stack[labelIdx];
	} else {
		DEBUG_SCRIPT(_currentLine, "if(<=) goto %d (false)", labelIdx);
	}
}

void o1_gotoIfEqu() {
	byte labelIdx = getNextByte();

	if (_currentScriptElement->compareResult == kCmpEQ) {
		assert(_currentScriptElement->stack[labelIdx] != -1);

		DEBUG_SCRIPT(_currentLine, "if(==) goto %d (true)", labelIdx);
		_currentPosition = _currentScriptElement->stack[labelIdx];
	} else {
		DEBUG_SCRIPT(_currentLine, "if(==) goto %d (false)", labelIdx);
	}
}

void o1_gotoIfDiff() {
	byte labelIdx = getNextByte();

	if (_currentScriptElement->compareResult != kCmpEQ) {
		assert(_currentScriptElement->stack[labelIdx] != -1);

		DEBUG_SCRIPT(_currentLine, "if(!=) goto %d (true)", labelIdx);
		_currentPosition = _currentScriptElement->stack[labelIdx];
	} else {
		DEBUG_SCRIPT(_currentLine, "if(!=) goto %d (false)", labelIdx);
	}
}

void o1_removeLabel() {
	// TODO: verify this
	byte labelIdx = getNextByte();

	DEBUG_SCRIPT(_currentLine, "removeLabel(%d)", labelIdx);
	_currentScriptElement->stack[labelIdx] = -1;
}

void o1_loop() {
	byte varIdx = getNextByte();
	byte labelIdx = getNextByte();

	_currentScriptElement->localVars[varIdx]--;

	if (_currentScriptElement->localVars[varIdx] >= 0) {
		assert(_currentScriptElement->stack[labelIdx] != -1);

		DEBUG_SCRIPT(_currentLine, "loop(var[%]) goto %d (continue)", varIdx, labelIdx);
		_currentPosition = _currentScriptElement->stack[labelIdx];
	} else {
		DEBUG_SCRIPT(_currentLine, "loop(var[%]) goto %d (stop)", varIdx, labelIdx);
	}
}

void o1_startGlobalScript() {
	// OP_startScript
	byte param = getNextByte();

	assert(param < NUM_MAX_SCRIPT);

	DEBUG_SCRIPT(_currentLine, "startScript(%d)", param);
	addScriptToList0(param);
}

void o1_endGlobalScript() {
	byte scriptIdx = getNextByte();

	DEBUG_SCRIPT(_currentLine, "stopGlobalScript(%d)", scriptIdx);
	stopGlobalScript(scriptIdx);
}

void o1_loadAnim() {
	// OP_loadResource
	const char *param = getNextString();

	DEBUG_SCRIPT(_currentLine, "loadResource(\"%s\")", param);
	loadResource(param);
}

void o1_loadBg() {
	const char *param = getNextString();

	DEBUG_SCRIPT(_currentLine, "loadBg(\"%s\")", param);

	loadBg(param);
	closeEngine7();
	bgVar0 = 0;
}

void o1_loadCt() {
	const char *param = getNextString();

	DEBUG_SCRIPT(_currentLine, "loadCt(\"%s\")", param);
	loadCt(param);
}

void o1_loadPart() {
	const char *param = getNextString();

	DEBUG_SCRIPT(_currentLine, "loadPart(\"%s\")", param);
	loadPart(param);
}

void o1_closePart() {
	DEBUG_SCRIPT(_currentLine, "closePart");
	closePart();
}

void o1_loadNewPrcName() {
	// OP_loadData
	byte param1 = getNextByte();
	const char *param2 = getNextString();

	assert(param1 <= 3);

	switch (param1) {
	case 0:
		DEBUG_SCRIPT(_currentLine, "loadPrc(\"%s\")", param2);
		strcpy(newPrcName, param2);
		break;
	case 1:
		DEBUG_SCRIPT(_currentLine, "loadRel(\"%s\")", param2);
		strcpy(newRelName, param2);
		break;
	case 2:
		DEBUG_SCRIPT(_currentLine, "loadObject(\"%s\")", param2);
		strcpy(newObjectName, param2);
		break;
	case 3:
		DEBUG_SCRIPT(_currentLine, "loadMsg(\"%s\")", param2);
		strcpy(newMsgName, param2);
		break;
	}
}

void o1_requestCheckPendingDataLoad() {
	DEBUG_SCRIPT(_currentLine, "request data load");
	checkForPendingDataLoadSwitch = 1;
}

void o1_blitAndFade() {
	DEBUG_SCRIPT(_currentLine, "request fadein");
	// TODO: use real code

	memcpy(c_palette, tempPalette, sizeof(uint16) * 16);
	drawOverlays();
	flip();

	fadeRequired = 1;
}

void o1_fadeToBlack() {
	DEBUG_SCRIPT(_currentLine, "request fadeout");
	//fadeToBlack();
	warning("STUB: o1_fadeToBlack()");
}

void o1_transformPaletteRange() {
	byte startColor = getNextByte();
	byte numColor = getNextByte();
	uint16 r = getNextWord();
	uint16 g = getNextWord();
	uint16 b = getNextWord();

	DEBUG_SCRIPT(_currentLine, "transformPaletteRange(from:%d,numIdx:%d,r:%d,g:%d,b:%d) -> unimplemented", startColor, numColor, r, g, b);

	transformPaletteRange(startColor, numColor, r, g, b);
}

void o1_setDefaultMenuColor2() {
	byte param = getNextByte();

	DEBUG_SCRIPT(_currentLine, "setDefaultMenuColor2(%d)", param);
	defaultMenuBoxColor2 = param;
}

void o1_palRotate() {
	byte a = getNextByte();
	byte b = getNextByte();
	byte c = getNextByte();

	DEBUG_SCRIPT(_currentLine, "palRotate(%d,%d,%d)", a, b, c);
	palRotate(a, b, c);
}

void o1_break() {
	DEBUG_SCRIPT(_currentLine, "break");

	_currentScriptElement->scriptPosition = _currentPosition;
	_closeScript = 1;
}

void o1_endScript() {
	DEBUG_SCRIPT(_currentLine, "endScript");

	if (_currentScriptParams == 0) {
		endScript0(_currentScriptElement->scriptIdx);
	} else {
		endScript1(_currentScriptElement->scriptIdx);
	}

	_closeScript = 1;
}

void o1_message() {
	byte param1 = getNextByte();
	uint16 param2 = getNextWord();
	uint16 param3 = getNextWord();
	uint16 param4 = getNextWord();
	uint16 param5 = getNextWord();

	DEBUG_SCRIPT(_currentLine, "message(%d,%d,%d,%d,%d)", param1, param2, param3, param4, param5);

	addMessage(param1, param2, param3, param4, param5);
}

void o1_loadGlobalVar() {
	byte varIdx = getNextByte();
	byte varType = getNextByte();

	if (varType) {
		byte dataIdx = getNextByte();

		if (varType == 1) {
			DEBUG_SCRIPT(_currentLine, "globalVars[%d] = var[%d]", varIdx, dataIdx);
			globalVars[varIdx] = _currentScriptElement->localVars[dataIdx];
		} else {
			DEBUG_SCRIPT(_currentLine, "globalVars[%d] = globalVars[%d]", varIdx, dataIdx);
			globalVars[varIdx] = globalVars[dataIdx];
		}
	} else {
		uint16 value = getNextWord();

		DEBUG_SCRIPT(_currentLine, "globalVars[%d] = %d", varIdx, value);
		globalVars[varIdx] = value;
	}
}

void o1_compareGlobalVar() {
	byte varIdx = getNextByte();
	byte varType = getNextByte();

	if (varType) {
		byte value = getNextByte();

		DEBUG_SCRIPT(_currentLine, "compare globalVars[%d] and var[%d]", varIdx, value);
		_currentScriptElement->compareResult = compareVars(globalVars[varIdx], _currentScriptElement->localVars[value]);
	} else {
		uint16 value = getNextWord();

		DEBUG_SCRIPT(_currentLine, "compare globalVars[%d] and %d", varIdx, value);

		if (varIdx == 255 && (gameType == Cine::GID_FW)) {	// TODO: fix
			_currentScriptElement->compareResult = 1;
		} else {
			_currentScriptElement->compareResult = compareVars(globalVars[varIdx], value);
		}
	}
}

void o1_declareFunctionName() {
	const char *param = getNextString();

	DEBUG_SCRIPT(_currentLine, "comment(%s)", param);
}

void o1_freePartRange() {
	byte startIdx = getNextByte();
	byte numIdx = getNextByte();

	assert(startIdx + numIdx <= NUM_MAX_ANIMDATA);

	DEBUG_SCRIPT(_currentLine, "freePartRange(%d,%d)", startIdx, numIdx);
	freePartRange(startIdx, numIdx);
}

void o1_unloadAllMasks() {
	DEBUG_SCRIPT(_currentLine, "unloadAllMasks()");
	unloadAllMasks();
}

void o1_op63() {
	warning("STUB: o1_op63()");
	getNextWord();
	getNextWord();
	getNextWord();
	getNextWord();
	// setupScreenParam
}

void o1_op64() {
	warning("STUB: o1_op64()");
}

void o1_initializeZoneData() {
	DEBUG_SCRIPT(_currentLine, "initializeZoneData()");

	for (int i = 0; i < NUM_MAX_ZONE; i++) {
		zoneData[i] = i;
	}
}

void o1_setZoneDataEntry() {
	byte zoneIdx = getNextByte();
	uint16 var = getNextWord();

	DEBUG_SCRIPT(_currentLine, "setZone[%d] = %d", zoneIdx, var);
	zoneData[zoneIdx] = var;
}

void o1_getZoneDataEntry() {
	warning("STUB: o1_getZoneDataEntry()");
}

void o1_setDefaultMenuColor() {
	byte param = getNextByte();

	DEBUG_SCRIPT(_currentLine, "setDefaultMenuColor(%d)", param);
	defaultMenuBoxColor = param;
}

void o1_allowPlayerInput() {
	DEBUG_SCRIPT(_currentLine, "allowPlayerInput()");
	allowPlayerInput = 1;
}

void o1_disallowPlayerInput() {
	DEBUG_SCRIPT(_currentLine, "dissallowPlayerInput()");
	allowPlayerInput = 0;
}

void o1_changeDataDisk() {
	byte newDisk = getNextByte();

	DEBUG_SCRIPT(_currentLine, "changeDataDisk(%d)", newDisk);
	checkDataDisk(newDisk);
}

void o1_loadMusic() {
	const char *param = getNextString();

	DEBUG_SCRIPT(_currentLine, "loadMusic(%s)", param);
	g_sfxPlayer->load(param);
}

void o1_playMusic() {
	DEBUG_SCRIPT(_currentLine, "playMusic()");
	g_sfxPlayer->play();
}

void o1_fadeOutMusic() {
	DEBUG_SCRIPT(_currentLine, "fadeOutMusic()");
	g_sfxPlayer->fadeOut();
}

void o1_stopSample() {
	DEBUG_SCRIPT(_currentLine, "stopSample()");
	g_sfxPlayer->stop();
}

void o1_op71() {
	warning("STUB: o1_op71()");
	getNextByte();
	getNextWord();
}

void o1_op72() {
	warning("STUB: o1_op72()");
	getNextWord();
	getNextByte();
	getNextWord();
}

void o1_op73() {
	// I believe this opcode is identical to o1_op72(). In fact, Operation
	// Stealth doesn't even have it. It uses o1_op72() instead.
	warning("STUB: o1_op73()");
	getNextWord();
	getNextByte();
	getNextWord();
}

void o1_playSample() {
	DEBUG_SCRIPT(_currentLine, "playSample()");

	byte anim = getNextByte();
	byte channel = getNextByte();

	getNextWord();
	getNextByte();

	int16 volume = getNextWord();
	uint16 flag = getNextWord();

	if (volume > 63)
		volume = 63;
	if (volume < 0)
		volume = 63;

	if (animDataTable[anim].ptr1) {
		if (channel >= 10) {
			channel -= 10;
		}
		if (volume < 50) {
			volume = 50;
		}

		g_sfxPlayer->stop();
					
		if (flag == 0xFFFF) {
			g_soundDriver->playSound(animDataTable[anim].ptr1, channel, volume);
		} else {
			g_soundDriver->resetChannel(channel);
		}
	}
}

void o1_allowSystemMenu() {
	byte param = getNextByte();

	DEBUG_SCRIPT(_currentLine, "OP79 load var22 to %d -> TODO", param);
	var22 = param;
}

void o1_loadMask5() {
	byte param = getNextByte();

	DEBUG_SCRIPT(_currentLine, "addOverlay5(%d)", param);
	loadOverlayElement(param, 5);
}

void o1_unloadMask5() {
	byte param = getNextByte();

	DEBUG_SCRIPT(_currentLine, "freeOverlay5(%d)", param);
	freeOverlay(param, 5);
}

// ------------------------------------------------------------------------
// OPERATION STEALTH opcodes
// ------------------------------------------------------------------------

void o2_loadPart() {
	const char *param = getNextString();

	DEBUG_SCRIPT(_currentLine, "loadPart(\"%s\")", param);
}

void o2_op78() {
	warning("STUB: o2_op78()");
	// This is probably wrong, but preserve the old behaviour for now.
	o1_playSample();
}

void o2_addSeqListElement() {
	byte param1 = getNextByte();
	byte param2 = getNextByte();
	byte param3 = getNextByte();
	byte param4 = getNextByte();
	uint16 param5 = getNextWord();
	uint16 param6 = getNextWord();
	uint16 param7 = getNextWord();

	DEBUG_SCRIPT(_currentLine, "addSeqListElement(%d,%d,%d,%d,%d)", param1, param2, param3, param4, param5, param6, param7);
	addSeqListElement(param1, 0, param2, param3, param4, param5, param6, 0, param7);
}

void o2_removeSeq() {
	byte a = getNextByte();
	byte b = getNextByte();

	DEBUG_SCRIPT(_currentLine, "removeSeq(%d,%d) -> TODO", a, b);
	removeSeq(a, 0, b);
}

void o2_op81() {
	warning("STUB: o2_op81()");
	// freeUnkList();
}

void o2_op82() {
	warning("STUB: o2_op82()");
	getNextByte();
	getNextByte();
	getNextWord();
}

void o2_isSeqRunning() {
	byte a = getNextByte();
	byte b = getNextByte();

	DEBUG_SCRIPT(_currentLine, "OP83(%d,%d) -> TODO", a, b);

	if (isSeqRunning(a, 0, b)) {
		_currentScriptElement->compareResult = 1;
	} else {
		_currentScriptElement->compareResult = 0;
	}
}

void o2_gotoIfSupNearest() {
	byte labelIdx = getNextByte();

	if (_currentScriptElement->compareResult == kCmpGT) {
		assert(_currentScriptElement->stack[labelIdx] != -1);

		DEBUG_SCRIPT(_currentLine, "if(>) goto nearest %d (true)", labelIdx);
		_currentPosition = computeScriptStackFromScript(_currentScriptElement->scriptPtr, _currentPosition, labelIdx, scriptTable[_currentScriptElement->scriptIdx].size);
	} else {
		DEBUG_SCRIPT(_currentLine, "if(>) goto nearest %d (false)", labelIdx);
	}
}

void o2_gotoIfSupEquNearest() {
	byte labelIdx = getNextByte();

	if (_currentScriptElement->compareResult & (kCmpGT | kCmpEQ)) {
		assert(_currentScriptElement->stack[labelIdx] != -1);

		DEBUG_SCRIPT(_currentLine, "if(>=) goto nearest %d (true)", labelIdx);
		_currentPosition = computeScriptStackFromScript(_currentScriptElement->scriptPtr, _currentPosition, labelIdx, scriptTable[_currentScriptElement->scriptIdx].size);
	} else {
		DEBUG_SCRIPT(_currentLine, "if(>=) goto nearest %d (false)", labelIdx);
	}
}

void o2_gotoIfInfNearest() {
	byte labelIdx = getNextByte();

	if (_currentScriptElement->compareResult == kCmpLT) {
		assert(_currentScriptElement->stack[labelIdx] != -1);

		DEBUG_SCRIPT(_currentLine, "if(<) goto nearest %d (true)", labelIdx);
		_currentPosition = computeScriptStackFromScript(_currentScriptElement->scriptPtr, _currentPosition, labelIdx, scriptTable[_currentScriptElement->scriptIdx].size);
	} else {
		DEBUG_SCRIPT(_currentLine, "if(<) goto nearest %d (false)", labelIdx);
	}
}

void o2_gotoIfInfEquNearest() {
	byte labelIdx = getNextByte();

	if (_currentScriptElement->compareResult & (kCmpLT | kCmpEQ)) {
		assert(_currentScriptElement->stack[labelIdx] != -1);

		DEBUG_SCRIPT(_currentLine, "if(<=) goto nearest %d (true)", labelIdx);
		_currentPosition = computeScriptStackFromScript(_currentScriptElement->scriptPtr, _currentPosition, labelIdx, scriptTable[_currentScriptElement->scriptIdx].size);
	} else {
		DEBUG_SCRIPT(_currentLine, "if(<=) goto nearest %d (false)", labelIdx);
	}
}

void o2_gotoIfEquNearest() {
	byte labelIdx = getNextByte();

	if (_currentScriptElement->compareResult == kCmpEQ) {
		assert(_currentScriptElement->stack[labelIdx] != -1);

		DEBUG_SCRIPT(_currentLine, "if(==) goto nearest %d (true)", labelIdx);
		_currentPosition = computeScriptStackFromScript(_currentScriptElement->scriptPtr, _currentPosition, labelIdx, scriptTable[_currentScriptElement->scriptIdx].size);
	} else {
		DEBUG_SCRIPT(_currentLine, "if(==) goto nearest %d (false)", labelIdx);
	}
}

void o2_gotoIfDiffNearest() {
	byte labelIdx = getNextByte();

	if (_currentScriptElement->compareResult != kCmpEQ) {
		assert(_currentScriptElement->stack[labelIdx] != -1);

		DEBUG_SCRIPT(_currentLine, "if(!=) goto nearest %d (true)", labelIdx);
		_currentPosition = computeScriptStackFromScript(_currentScriptElement->scriptPtr, _currentPosition, labelIdx, scriptTable[_currentScriptElement->scriptIdx].size);
	} else {
		DEBUG_SCRIPT(_currentLine, "if(!=) goto nearest %d (false)", labelIdx);
	}
}

void o2_startObjectScript() {
	byte param = getNextByte();

	DEBUG_SCRIPT(_currentLine, "startObjectScript(%d)", param);
	runObjectScript(param);
}

void o2_stopObjectScript() {
	byte param = getNextByte();

	DEBUG_SCRIPT(_currentLine, "stopObjectScript(%d)", param);
	stopObjectScript(param);
}

void o2_op8D() {
	warning("STUB: o2_op8D()");
	getNextWord();
	getNextWord();
	getNextWord();
	getNextWord();
	getNextWord();
	getNextWord();
	getNextWord();
	getNextWord();
	// _currentScriptElement->compareResult = ...
}

void o2_addBackground() {
	byte param1 = getNextByte();
	const char *param2 = getNextString();

	DEBUG_SCRIPT(_currentLine, "addBackground(%s,%d)", param2, param1);
	addBackground(param2, param1);
}

void o2_removeBackground() {
	byte param = getNextByte();

	assert(param);

	DEBUG_SCRIPT(_currentLine, "removeBackground(%d)", param);

	if (additionalBgTable[param]) {
		free(additionalBgTable[param]);
		additionalBgTable[param] = NULL;
	}

	if (currentAdditionalBgIdx == param) {
		currentAdditionalBgIdx = 0;
	}

	if (currentAdditionalBgIdx2 == param) {
		currentAdditionalBgIdx2 = 0;
	}

	strcpy(currentBgName[param], "");
}

void o2_loadAbs() {
	byte param1 = getNextByte();
	const char *param2 = getNextString();

	DEBUG_SCRIPT(_currentLine, "loadABS(%d,%s)", param1, param2);
	loadAbs(param2, param1);
}

void o2_loadBg() {
	byte param = getNextByte();

	assert(param <= 8);

	DEBUG_SCRIPT(_currentLine, "useBg(%d)", param);

	if (additionalBgTable[param]) {
		currentAdditionalBgIdx = param;
		//if (adBgVar0 == 0) {
		//	adBgVar1 = 1;
		//}
	}
}

void o2_wasZoneChecked() {
	warning("STUB: o2_wasZoneChecked()");
}

void o2_op9B() {
	warning("STUB: o2_op9B()");
	getNextWord();
	getNextWord();
	getNextWord();
	getNextWord();
	getNextWord();
	getNextWord();
	getNextWord();
	getNextWord();
}

void o2_op9C() {
	warning("STUB: o2_op9C()");
	getNextWord();
	getNextWord();
	getNextWord();
	getNextWord();
}

void o2_useBgScroll() {
	byte param = getNextByte();

	assert(param <= 8);

	DEBUG_SCRIPT(_currentLine, "useBgScroll(%d)", param);

	if (additionalBgTable[param]) {
		currentAdditionalBgIdx2 = param;
	}
}

void o2_setAdditionalBgVScroll() {
	byte param1 = getNextByte();

	if (param1) {
		byte param2 = getNextByte();

		DEBUG_SCRIPT(_currentLine, "additionalBgVScroll = var[%d]", param2);
		additionalBgVScroll = _currentScriptElement->localVars[param2];
	} else {
		uint16 param2 = getNextWord();

		DEBUG_SCRIPT(_currentLine, "additionalBgVScroll = %d", param2);
		additionalBgVScroll = param2;
	}
}

void o2_op9F() {
	warning("o2_op9F()");
	getNextWord();
	getNextWord();
}

void o2_addGfxElementA0() {
	uint16 param1 = getNextWord();
	uint16 param2 = getNextWord();

	DEBUG_SCRIPT(_currentLine, "addGfxElementA0(%d,%d)", param1, param2);
	addGfxElementA0(param1, param2);
}

void o2_opA1() {
	warning("STUB: o2_opA1()");
	getNextWord();
	getNextWord();
	// removeGfxElementA0( ... );
}

void o2_opA2() {
	warning("STUB: o2_opA2()");
	getNextWord();
	getNextWord();
	// addGfxElementA2();
}

void o2_opA3() {
	warning("STUB: o2_opA3()");
	getNextWord();
	getNextWord();
	// removeGfxElementA2();
}

void o2_loadMask22() {
	byte param = getNextByte();

	DEBUG_SCRIPT(_currentLine, "addOverlay22(%d)", param);
	loadOverlayElement(param, 22);
}

void o2_unloadMask22() {
	byte param = getNextByte();

	DEBUG_SCRIPT(_currentLine, "removeOverlay22(%d)", param);
	freeOverlay(param, 22);
}

// ------------------------------------------------------------------------

void executeScript(prcLinkedListStruct *scriptElement, uint16 params) {
	assert(scriptElement);

	if (scriptElement->scriptIdx == -1) {
		return;
	}

	assert(scriptElement->scriptPtr);

	_currentScriptElement = scriptElement;
	_currentScriptParams = params;
	_currentScriptPtr = scriptElement->scriptPtr;
	_currentPosition = scriptElement->scriptPosition;

	_closeScript = 0;

	while (!_closeScript) {
		_currentLine = _currentPosition;

		byte opcode = getNextByte();

		if (opcode && opcode < _numOpcodes) {
			if (_opcodeTable[opcode - 1])
				(_opcodeTable[opcode - 1]) ();
			else
				warning("Undefined opcode 0x%02X", opcode - 1);
		}
	}
}

void executeList1(void) {
	prcLinkedListStruct *currentHead = objScriptList.next;

	while (currentHead) {
		prcLinkedListStruct *tempHead;

		tempHead = currentHead->next;

		executeScript(currentHead, 1);

		currentHead = tempHead;
	}
}

void executeList0(void) {
	prcLinkedListStruct *currentHead = globalScriptsHead.next;

	while (currentHead) {
		prcLinkedListStruct *tempHead;

		executeScript(currentHead, 0);

		tempHead = currentHead->next;
		currentHead = tempHead;
	}
}

void purgeList1(void) {
}

void purgeList0(void) {
}

////////////////////////////////////
// SCRIPT DECOMPILER

#ifdef DUMP_SCRIPTS

byte decompileBuffer[10000][1000];
uint16 decompileBufferPosition = 0;

byte bufferDec[256];

byte compareString1[256];
byte compareString2[256];

byte *getObjPramName(byte paramIdx) {
	switch (paramIdx) {
	case 1:
		return ".X";
	case 2:
		return ".Y";
	case 3:
		return ".mask";
	case 4:
		return ".frame";
	case 5:
		return ".status";
	case 6:
		return ".costume";
	default:
		sprintf(bufferDec, ".param%d", paramIdx);
		return bufferDec;
	}
}

void decompileScript(byte *scriptPtr, int16 *stackPtr, uint16 scriptSize, uint16 scriptIdx) {
	byte lineBuffer[256];
	byte *localScriptPtr = scriptPtr;
	uint16 exitScript;
	uint32 position = 0;

	assert(scriptPtr);
	// assert(stackPtr);

	exitScript = 0;

	sprintf(decompileBuffer[decompileBufferPosition++], "--------- SCRIPT %d ---------\n", scriptIdx);

	do {
		uint16 opcode = *(localScriptPtr + position);
		position++;

		if (position == scriptSize) {
			opcode = 0;
		}

		printf("%X\n", opcode - 1);

		strcpy(lineBuffer, "");

		switch (opcode - 1) {
		case -1:
			{
				break;
			}
		case 0x0:
			{
				byte param1;
				byte param2;
				int16 param3;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				param3 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				sprintf(lineBuffer, "obj[%d]%s = %d\n", param1, getObjPramName(param2), param3);

				break;
			}
		case 0x1:
			{
				byte param1;
				byte param2;
				byte param3;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				param3 = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "var[%d]=obj[%d]%s\n", param3, param1, getObjPramName(param2));
				break;
			}
		case 0x2:
			{
				byte param1;
				byte param2;
				int16 param3;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				param3 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				sprintf(lineBuffer, "obj[%d]%s+=%d\n", param1, getObjPramName(param2), param3);

				break;
			}
		case 0x3:
			{
				byte param1;
				byte param2;
				int16 param3;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				param3 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				sprintf(lineBuffer, "obj[%d]%s-=%d\n", param1, getObjPramName(param2), param3);

				break;
			}
		case 0x4:
			{
				byte param1;
				byte param2;
				int16 param3;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				param3 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				sprintf(lineBuffer, "obj[%d]%s+=obj[%d]%s\n", param1, getObjPramName(param2), param3, getObjPramName(param2));

				break;
			}
		case 0x5:
			{
				byte param1;
				byte param2;
				int16 param3;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				param3 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				sprintf(lineBuffer, "obj[%d]%s-=obj[%d]%s\n", param1, getObjPramName(param2), param3, getObjPramName(param2));

				break;
			}
		case 0x6:
			{
				byte param1;
				byte param2;
				int16 param3;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				param3 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				sprintf(compareString1, "obj[%d]%s", param1, getObjPramName(param2));
				sprintf(compareString2, "%d", param3);

				break;
			}
		case 0x7:
			{
				byte param1;
				int16 param2;
				int16 param3;
				int16 param4;
				int16 param5;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				param3 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				param4 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				param5 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				sprintf(lineBuffer, "setupObject(Idx:%d,X:%d,Y:%d,mask:%d,frame:%d)\n", param1, param2, param3, param4, param5);

				break;
			}
		case 0x8:
			{
				byte param1;
				int16 param2;
				int16 param3;
				int16 param4;
				int16 param5;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				param3 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				param4 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				param5 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				sprintf(lineBuffer, "checkCollision(%d,%d,%d,%d,%d)\n", param1, param2, param3, param4, param5);

				break;
			}
		case 0x9:
			{
				byte param1;
				int16 param2;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				if (param2) {
					byte param3;

					param3 = *(localScriptPtr + position);
					position++;

					if (param2 == 1) {
						sprintf(lineBuffer, "var[%d]=var[%d]\n", param1, param3);
					} else if (param2 == 2) {
						sprintf(lineBuffer, "var[%d]=globalVar[%d]\n", param1, param3);
					} else if (param2 == 3) {
						sprintf(lineBuffer, "var[%d]=mouse.X\n", param1);
					} else if (param2 == 4) {
						sprintf(lineBuffer, "var[%d]=mouse.Y\n", param1);
					} else if (param2 == 5) {
						sprintf(lineBuffer, "var[%d]=rand() mod %d\n", param1, param3);
					} else {
						error("decompileScript: 0x09: param2 = %d", param2);
					}
				} else {
					int16 param3;

					param3 = READ_BE_UINT16(localScriptPtr + position);
					position += 2;

					sprintf(lineBuffer, "var[%d]=%d\n", param1, param3);
				}

				break;
			}
		case 0xA:
			{
				byte param1;
				byte param2;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				if (param2) {
					byte param3;

					param3 = *(localScriptPtr + position);
					position++;

					sprintf(lineBuffer, "var[%d]+=var[%d]\n", param1, param3);
				} else {
					int16 param3;

					param3 = READ_BE_UINT16(localScriptPtr +  position);
					position += 2;

					sprintf(lineBuffer, "var[%d]+=%d\n", param1, param3);
				}
				break;
			}
		case 0xB:
			{
				byte param1;
				byte param2;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				if (param2) {
					byte param3;

					param3 = *(localScriptPtr + position);
					position++;

					sprintf(lineBuffer, "var[%d]-=var[%d]\n", param1, param3);
				} else {
					int16 param3;

					param3 = READ_BE_UINT16(localScriptPtr + position);
					position += 2;

					sprintf(lineBuffer, "var[%d]-=%d\n", param1, param3);
				}
				break;
			}
		case 0xC:
			{
				byte param1;
				byte param2;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				if (param2) {
					byte param3;

					param3 = *(localScriptPtr + position);
					position++;

					sprintf(lineBuffer, "var[%d]*=var[%d]\n", param1, param3);
				} else {
					int16 param3;

					param3 = READ_BE_UINT16(localScriptPtr + position);
					position += 2;

					sprintf(lineBuffer, "var[%d]*=%d\n", param1, param3);
				}
				break;
			}
		case 0xD:
			{
				byte param1;
				byte param2;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				if (param2) {
					byte param3;

					param3 = *(localScriptPtr + position);
					position++;

					sprintf(lineBuffer, "var[%d]/=var[%d]\n", param1, param3);
				} else {
					int16 param3;

					param3 = READ_BE_UINT16(localScriptPtr + position);
					position += 2;

					sprintf(lineBuffer, "var[%d]/=%d\n", param1, param3);
				}
				break;
			}
		case 0xE:
			{
				byte param1;
				byte param2;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				if (param2) {
					byte param3;

					param3 = *(localScriptPtr + position);
					position++;

					if (param2 == 1) {
						sprintf(compareString1, "var[%d]", param1);
						sprintf(compareString2, "var[%d]", param3);

					} else if (param2 == 2) {
						sprintf(compareString1, "var[%d]", param1);
						sprintf(compareString2, "globalVar[%d]", param3);
					} else {
						error("decompileScript: 0x0E: param2 = %d", param2);
					}
				} else {
					int16 param3;

					param3 = READ_BE_UINT16(localScriptPtr + position);
					position += 2;

					sprintf(compareString1, "var[%d]", param1);
					sprintf(compareString2, "%d", param3);
				}
				break;
			}
		case 0xF:
			{
				byte param1;
				byte param2;
				byte param3;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				param3 = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "obj[%d]%s=var[%d]\n", param1, getObjPramName(param2), param3);

				break;
			}
		case 0x13:
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "loadMask0(%d)\n", param);

				break;
			}
		case 0x14:
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "unloadMask0(%d)\n", param);

				break;
			}
		case 0x15:
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "OP_15(%d)\n", param);

				break;
			}
		case 0x16:
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "loadMask1(%d)\n", param);

				break;
			}
		case 0x17:
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "unloadMask0(%d)\n", param);

				break;
			}
		case 0x18:
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "loadMask4(%d)\n", param);

				break;
			}
		case 0x19:
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "unloadMask4(%d)\n", param);

				break;
			}
		case 0x1A:
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "OP_1A(%d)\n", param);

				break;
			}
		case 0x1B:
			{
				sprintf(lineBuffer, "closeEngine7()\n");
				break;
			}
		case 0x1D:
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "label(%d)\n", param);

				break;
			}
		case 0x1E:
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "goto(%d)\n", param);

				break;
			}
		case 0x1F:
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "if(%s>%s) goto(%d)\n", compareString1, compareString2, param);

				break;
			}
		case 0x20:
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "if(%s>=%s) goto(%d)\n", compareString1, compareString2, param);

				break;
			}
		case 0x21:
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "if(%s<%s) goto(%d)\n", compareString1, compareString2, param);

				break;
			}
		case 0x22:
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "if(%s<=%s) goto(%d)\n", compareString1, compareString2, param);

				break;
			}
		case 0x23:
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "if(%s==%s) goto(%d)\n", compareString1, compareString2, param);

				break;
			}
		case 0x24:
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "if(%s!=%s) goto(%d)\n", compareString1, compareString2, param);

				break;
			}
		case 0x25:
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "removeLabel(%d)\n", param);

				break;
			}
		case 0x26:
			{
				byte param1;
				byte param2;

				param1 = *(localScriptPtr + position);
				position++;
				param2 = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "loop(--var[%d]) -> label(%d)\n", param1, param2);

				break;
			}
		case 0x31:
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "startGlobalScript(%d)\n", param);

				break;
			}
		case 0x32:
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "endGlobalScript(%d)\n", param);

				break;
			}
		case 0x3B:
			{
				sprintf(lineBuffer, "loadResource(%s)\n", localScriptPtr + position);

				position += strlen(localScriptPtr + position) + 1;
				break;
			}
		case 0x3C:
			{
				sprintf(lineBuffer, "loadBg(%s)\n",	localScriptPtr + position);

				position += strlen(localScriptPtr + position) + 1;
				break;
			}
		case 0x3D:
			{
				sprintf(lineBuffer, "loadCt(%s)\n", localScriptPtr + position);

				position += strlen(localScriptPtr + position) + 1;
				break;
			}
		case OP_loadPart:
			{
				sprintf(lineBuffer, "loadPart(%s)\n", localScriptPtr + position);

				position += strlen(localScriptPtr + position) + 1;
				break;
			}
		case 0x40:
			{
				sprintf(lineBuffer, "closePart()\n");
				break;
			}
		case OP_loadNewPrcName:
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "loadPrc(%d,%s)\n", param, localScriptPtr + position);

				position += strlen(localScriptPtr + position) + 1;
				break;
			}
		case OP_requestCheckPendingDataLoad:	// nop
			{
				sprintf(lineBuffer, "requestCheckPendingDataLoad()\n");
				break;
			}
		case 0x45:
			{
				sprintf(lineBuffer, "blitAndFade()\n");
				break;
			}
		case 0x46:
			{
				sprintf(lineBuffer, "fadeToBlack()\n");
				break;
			}
		case 0x47:
			{
				byte param1;
				byte param2;
				int16 param3;
				int16 param4;
				int16 param5;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				param3 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				param4 = READ_BE_UINT16)(localScriptPtr + position);
				position += 2;

				param5 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				sprintf(lineBuffer, "transformPaletteRange(%d,%d,%d,%d,%d)\n", param1, param2, param3, param4, param5);

				break;
			}
		case 0x49:
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "setDefaultMenuColor2(%d)\n", param);

				break;
			}
		case 0x4F:
			{
				sprintf(lineBuffer, "break()\n");
				exitScript = 1;
				break;
			}
		case 0x50:
			{
				sprintf(lineBuffer, "endScript()\n\n");
				break;
			}
		case 0x51:
			{
				byte param1;
				int16 param2;
				int16 param3;
				int16 param4;
				int16 param5;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				param3 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				param4 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				param5 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				sprintf(lineBuffer, "message(%d,%d,%d,%d,%d)\n", param1, param2, param3, param4, param5);

				break;
			}
		case 0x52:
			{
				byte param1;
				byte param2;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				if (param2) {
					byte param3;

					param3 = *(localScriptPtr + position);
					position++;

					if (param2 == 1) {
						sprintf(lineBuffer, "globalVar[%d] = var[%d]\n", param1, param3);
					} else if (param2 == 2) {
						sprintf(lineBuffer, "globalVar[%d] = globalVar[%d]\n", param1, param3);
					} else {
						error("decompileScript: 0x52: param2 = %d", param2);
					}
				} else {
					int16 param3;

					param3 = READ_BE_UINT16(localScriptPtr + position);
					position += 2;

					sprintf(lineBuffer, "globalVar[%d] = %d\n", param1, param3);
				}
				break;
			}
		case 0x53:
			{
				byte param1;
				byte param2;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				if (param2) {
					byte param3;

					param3 = *(localScriptPtr + position);
					position++;

					if (param2 == 1) {
						sprintf(compareString1, "globalVar[%d]", param1);
						sprintf(compareString2, "var[%d]", param3);
					} else if (param2 == 2) {
						sprintf(compareString1, "globalVar[%d]", param1);
						sprintf(compareString2, "globalVar[%d]", param3);
					} else {
						error("decompileScript: 0x53: param2 = %d", param2);
					}
				} else {
					int16 param3;

					param3 = READ_BE_UINT16(localScriptPtr + position);
					position += 2;

					sprintf(compareString1, "globalVar[%d]", param1);
					sprintf(compareString2, "%d", param3);
				}
				break;
			}
		case 0x59:
			{
				sprintf(lineBuffer, "comment: %s\n", localScriptPtr + position);

				position += strlen(localScriptPtr + position);
				break;
			}
		case 0x5A:
			{
				byte param1;
				byte param2;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "freePartRang(%d,%d)\n", param1, param2);

				break;
			}
		case 0x5B:
			{
				sprintf(lineBuffer, "unloadAllMasks()\n");
				break;
			}
		case 0x65:
			{
				sprintf(lineBuffer, "setupTableUnk1()\n");
				break;
			}
		case 0x66:
			{
				byte param1;
				int16 param2;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				sprintf(lineBuffer, "tableUnk1[%d] = %d\n", param1, param2);

				break;
			}
		case 0x68:
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "setDefaultMenuBoxColor(%d)\n", param);

				break;
			}
		case 0x69:
			{
				sprintf(lineBuffer, "allowPlayerInput()\n");
				break;
			}
		case 0x6A:
			{
				sprintf(lineBuffer, "disallowPlayerInput()\n");
				break;
			}
		case OP_changeDataDisk:
			{
				byte newDisk;

				newDisk = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "changeDataDisk(%d)\n", newDisk);

				break;
			}
		case 0x6D:
			{
				sprintf(lineBuffer, "loadDat(%s)\n", localScriptPtr + position);

				position += strlen(localScriptPtr + position) + 1;
				break;
			}
		case 0x6E:	// nop
			{
				sprintf(lineBuffer, "updateDat()\n");
				break;
			}
		case 0x6F:
			{
				sprintf(lineBuffer, "OP_6F() -> dat related\n");
				break;
			}
		case 0x70:
			{
				sprintf(lineBuffer, "stopSample()\n");
				break;
			}
		case OP_79:
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "allowSystemMenu(%d)\n", param);

				break;
			}
		case 0x77:
			{
				byte param1;
				byte param2;
				int16 param3;
				byte param4;
				int16 param5;
				int16 param6;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				param3 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				param4 = *(localScriptPtr + position);
				position++;

				param5 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				param6 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				sprintf(lineBuffer, "playSample(%d,%d,%d,%d,%d,%d)\n", param1, param2, param3, param4, param5, param6);

				break;
			}
		case 0x78:
			{
				byte param1;
				byte param2;
				int16 param3;
				byte param4;
				int16 param5;
				int16 param6;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				param3 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				param4 = *(localScriptPtr + position);
				position++;

				param5 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				param6 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				sprintf(lineBuffer, "OP_78(%d,%d,%d,%d,%d,%d)\n", param1, param2, param3, param4, param5, param6);

				break;
			}
		case 0x7A:
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "OP_7A(%d)\n", param);

				break;
			}
		case 0x7B:	// OS only
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "OP_7B(%d)\n", param);

				break;
			}
		case 0x7F:	// OS only
			{
				byte param1;
				byte param2;
				byte param3;
				byte param4;
				int16 param5;
				int16 param6;
				int16 param7;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				param3 = *(localScriptPtr + position);
				position++;

				param4 = *(localScriptPtr + position);
				position++;

				param5 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				param6 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				param7 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				sprintf(lineBuffer, "OP_7F(%d,%d,%d,%d,%d)\n", param1, param2, param3, param4, param5, param6, param7);

				break;
			}
		case 0x80:	// OS only
			{
				byte param1;
				byte param2;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "OP_80(%d,%d)\n", param1, param2);

				break;
			}
		case 0x82:	// OS only
			{
				byte param1;
				byte param2;
				uint16 param3;
				uint16 param4;
				byte param5;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				param3 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				param4 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				param5 = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "OP_82(%d,%d,%d,%d,%d)\n", param1, param2, param3, param4, param5);

				break;
			}
		case 0x83:	// OS only
			{
				byte param1;
				byte param2;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "OP_83(%d,%d)\n", param1, param2);

				break;
			}
		case 0x89:	// OS only
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "if(%s!=%s) goto next label(%d)\n", compareString1, compareString2, param);

				break;
			}
		case 0x8B:	// OS only
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "OP_8B(%d)\n", param);

				break;
			}
		case 0x8C:	// OS only
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "OP_8C(%d)\n", param);

				break;
			}
		case 0x8D:	// OS only
			{
				int16 param1;
				int16 param2;
				int16 param3;
				int16 param4;
				int16 param5;
				int16 param6;
				int16 param7;
				int16 param8;

				param1 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				param2 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				param3 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				param4 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				param5 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				param6 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				param7 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				param8 = READ_BE_UINT16(localScriptPtr + position);
				position += 2;

				sprintf(compareString1, "obj[%d]", param1);
				sprintf(compareString2, "{%d,%d,%d,%d,%d,%d,%d}", param2, param3, param4, param5, param6, param7, param8);

				break;
			}
		case 0x8E:	// OS only
			{
				byte param1;

				param1 = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "ADDBG(%d,%s)\n", param1, localScriptPtr + position);

				position += strlen(localScriptPtr + position);

				break;
			}
		case 0x8F:	// OS only
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "OP_8F(%d)\n", param);

				break;
			}
		case 0x90:	// OS only
			{
				byte param1;

				param1 = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "loadABS(%d,%s)\n", param1, localScriptPtr + position);

				position += strlen(localScriptPtr + position);

				break;
			}
		case 0x91:	// OS only
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "OP_91(%d)\n", param);

				break;
			}
		case 0x9D:	// OS only
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "OP_9D(%d) -> flip img idx\n", param);

				break;
			}
		case 0x9E:	// OS only
			{
				byte param;

				param = *(localScriptPtr + position);
				position++;

				if (param) {
					byte param2;

					param2 = *(localScriptPtr + position);
					position++;

					sprintf(lineBuffer, "OP_9E(%d,%d)\n", param, param2);
				} else {
					int16 param2;

					param2 = READ_BE_UINT16(localScriptPtr + position);
					position += 2;

					sprintf(lineBuffer, "OP_9E(%d,%d)\n", param, param2);
				}

				break;
			}
		case 0xA0:	// OS only
			{
				byte param1;
				byte param2;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "OP_A0(%d,%d)\n", param1, param2);

				break;
			}
		case 0xA1:	// OS only
			{
				byte param1;
				byte param2;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "OP_A1(%d,%d)\n", param1, param2);

				break;
			}
		default:
			{
				sprintf(lineBuffer, "Unsupported opcode %X in decompileScript\n\n", opcode - 1);
				position = scriptSize;
				break;
			}
		}

		// printf(lineBuffer);
		strcpy(decompileBuffer[decompileBufferPosition++], lineBuffer);

		exitScript = 0;
		if (position >= scriptSize) {
			exitScript = 1;
		}

	} while (!exitScript);
}

void dumpScript(byte *dumpName) {
	File *fHandle;
	uint16 i;

	fHandle = fopen(dumpName, "wt+");

	for (i = 0; i < decompileBufferPosition; i++) {
		fprintf(fHandle, decompileBuffer[i]);
	}

	fclose(fHandle);

	decompileBufferPosition = 0;
}

#endif

} // End of namespace Cine
