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
#include "cine/sound.h"
#include "cine/various.h"

namespace Cine {

prcLinkedListStruct *_currentScriptElement;
byte *_currentScriptPtr;
uint16 _currentScriptParams;
uint16 _currentPosition;
uint16 _currentLine;
uint16 _closeScript;

struct Opcode {
	void (*proc)();
	const char *args;
};

const Opcode *_opcodeTable;
int _numOpcodes;

void setupOpcodes() {
	static const Opcode opcodeTableFW[] = {
		/* 00 */
		{ o1_modifyObjectParam, "bbw" },
		{ o1_getObjectParam, "bbb" },
		{ o1_addObjectParam, "bbw" },
		{ o1_subObjectParam, "bbw" },
		/* 04 */
		{ o1_add2ObjectParam, "bbw" },
		{ o1_sub2ObjectParam, "bbw" },
		{ o1_compareObjectParam, "bbw" },
		{ o1_setupObject, "bwwww" },
		/* 08 */
		{ o1_checkCollision, "bwwww" },
		{ o1_loadVar, "bc" },
		{ o1_addVar, "bc" },
		{ o1_subVar, "bc" },
		/* 0C */
		{ o1_mulVar, "bc" },
		{ o1_divVar, "bc" },
		{ o1_compareVar, "bc" },
		{ o1_modifyObjectParam2, "bbb" },
		/* 10 */
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ o1_loadMask0, "b" },
		/* 14 */
		{ o1_unloadMask0, "b" },
		{ o1_addToBgList, "b" },
		{ o1_loadMask1, "b" },
		{ o1_unloadMask1, "b" },
		/* 18 */
		{ o1_loadMask4, "b" },
		{ o1_unloadMask4, "b" },
		{ o1_addSpriteFilledToBgList, "b" },
		{ o1_op1B, "" },
		/* 1C */
		{ 0, 0 },
		{ o1_label, "l" },
		{ o1_goto, "b" },
		{ o1_gotoIfSup, "b" },
		/* 20 */
		{ o1_gotoIfSupEqu, "b" },
		{ o1_gotoIfInf, "b" },
		{ o1_gotoIfInfEqu, "b" },
		{ o1_gotoIfEqu, "b" },
		/* 24 */
		{ o1_gotoIfDiff, "b" },
		{ o1_removeLabel, "b" },
		{ o1_loop, "bb" },
		{ 0, 0 },
		/* 28 */
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		/* 2C */
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		/* 30 */
		{ 0, 0 },
		{ o1_startGlobalScript, "b" },
		{ o1_endGlobalScript, "b" },
		{ 0, 0 },
		/* 34 */
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		/* 38 */
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ o1_loadAnim, "s" },
		/* 3C */
		{ o1_loadBg, "s" },
		{ o1_loadCt, "s" },
		{ 0, 0 },
		{ o1_loadPart, "s" },
		/* 40 */
		{ o1_closePart, "" },
		{ o1_loadNewPrcName, "bs" },
		{ o1_requestCheckPendingDataLoad, "" },
		{ 0, 0 },
		/* 44 */
		{ 0, 0 },
		{ o1_blitAndFade, "" },
		{ o1_fadeToBlack, "" },
		{ o1_transformPaletteRange, "bbwww" },
		/* 48 */
		{ 0, 0 },
		{ o1_setDefaultMenuColor2, "b" },
		{ o1_palRotate, "bbb" },
		{ 0, 0 },
		/* 4C */
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ o1_break, "" },
		/* 50 */
		{ o1_endScript, "x" },
		{ o1_message, "bwwww" },
		{ o1_loadGlobalVar, "bc" },
		{ o1_compareGlobalVar, "bc" },
		/* 54 */
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		/* 58 */
		{ 0, 0 },
		{ o1_declareFunctionName, "s" },
		{ o1_freePartRange, "bb" },
		{ o1_unloadAllMasks, "" },
		// 5C */
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		/* 60 */
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ o1_op63, "wwww" },
		/* 64 */
		{ o1_op64, "" },
		{ o1_initializeZoneData, "" },
		{ o1_setZoneDataEntry, "bw" },
		{ o1_getZoneDataEntry, "bb" },
		/* 68 */
		{ o1_setDefaultMenuColor, "b" },
		{ o1_allowPlayerInput, "" },
		{ o1_disallowPlayerInput, "" },
		{ o1_changeDataDisk, "b" },
		/* 6C */
		{ 0, 0 },
		{ o1_loadMusic, "s" },
		{ o1_playMusic, "" },
		{ o1_fadeOutMusic, "" },
		/* 70 */
		{ o1_stopSample, "" },
		{ o1_op71, "bw" },
		{ o1_op72, "wbw" },
		{ o1_op73, "wbw" },
		/* 74 */
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ o1_playSample, "bbwbww" },
		/* 78 */
		{ o1_playSample, "bbwbww" },
		{ o1_disableSystemMenu, "b" },
		{ o1_loadMask5, "b" },
		{ o1_unloadMask5, "b" }
	};

	// TODO: We need to verify the Operation Stealth opcodes.

	static const Opcode opcodeTableOS[] = {
		/* 00 */
		{ o1_modifyObjectParam, "bbw" },
		{ o1_getObjectParam, "bbb" },
		{ o1_addObjectParam, "bbw" },
		{ o1_subObjectParam, "bbw" },
		/* 04 */
		{ o1_add2ObjectParam, "bbw" },
		{ o1_sub2ObjectParam, "bbw" },
		{ o1_compareObjectParam, "bbw" },
		{ o1_setupObject, "bwwww" },
		/* 08 */
		{ o1_checkCollision, "bwwww" },
		{ o1_loadVar, "bc" },
		{ o1_addVar, "bc" },
		{ o1_subVar, "bc" },
		/* 0C */
		{ o1_mulVar, "bc" },
		{ o1_divVar, "bc" },
		{ o1_compareVar, "bc" },
		{ o1_modifyObjectParam2, "bbb" },
		/* 10 */
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ o1_loadMask0, "b" },
		/* 14 */
		{ o1_unloadMask0, "b" },
		{ o1_addToBgList, "b" },
		{ o1_loadMask1, "b" },
		{ o1_unloadMask1, "b" },
		/* 18 */
		{ o1_loadMask4, "b" },
		{ o1_unloadMask4, "b" },
		{ o1_addSpriteFilledToBgList, "b" },
		{ o1_op1B, "" },
		/* 1C */
		{ 0, 0 },
		{ o1_label, "l" },
		{ o1_goto, "b" },
		{ o1_gotoIfSup, "b" },
		/* 20 */
		{ o1_gotoIfSupEqu, "b" },
		{ o1_gotoIfInf, "b" },
		{ o1_gotoIfInfEqu, "b" },
		{ o1_gotoIfEqu, "b" },
		/* 24 */
		{ o1_gotoIfDiff, "b" },
		{ o1_removeLabel, "b" },
		{ o1_loop, "bb" },
		{ 0, 0 },
		/* 28 */
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		/* 2C */
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		/* 30 */
		{ 0, 0 },
		{ o1_startGlobalScript, "b" },
		{ o1_endGlobalScript, "b" },
		{ 0, 0 },
		/* 34 */
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		/* 38 */
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ o1_loadAnim, "s" },
		/* 3C */
		{ o1_loadBg, "s" },
		{ o1_loadCt, "s" },
		{ 0, 0 },
		{ o2_loadPart, "s" },
		/* 40 */
		{ 0, 0 },
		{ o1_loadNewPrcName, "bs" },
		{ o1_requestCheckPendingDataLoad, "" },
		{ 0, 0 },
		/* 44 */
		{ 0, 0 },
		{ o1_blitAndFade, "" },
		{ o1_fadeToBlack, "" },
		{ o1_transformPaletteRange, "bbwww" },
		/* 48 */
		{ 0, 0 },
		{ o1_setDefaultMenuColor2, "b" },
		{ o1_palRotate, "bbb" },
		{ 0, 0 },
		/* 4C */
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ o1_break, "" },
		/* 50 */
		{ o1_endScript, "x" },
		{ o1_message, "bwwww" },
		{ o1_loadGlobalVar, "bc" },
		{ o1_compareGlobalVar, "bc" },
		/* 54 */
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		/* 58 */
		{ 0, 0 },
		{ o1_declareFunctionName, "s" },
		{ o1_freePartRange, "bb" },
		{ o1_unloadAllMasks, "" },
		// 5C */
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		/* 60 */
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ o1_op63, "wwww" },
		/* 64 */
		{ o1_op64, "" },
		{ o1_initializeZoneData, "" },
		{ o1_setZoneDataEntry, "bw" },
		{ o1_getZoneDataEntry, "bb" },
		/* 68 */
		{ o1_setDefaultMenuColor, "b" },
		{ o1_allowPlayerInput, "" },
		{ o1_disallowPlayerInput, "" },
		{ o1_changeDataDisk, "b" },
		/* 6C */
		{ 0, 0 },
		{ o1_loadMusic, "s" },
		{ o1_playMusic, "" },
		{ o1_fadeOutMusic, "" },
		/* 70 */
		{ o1_stopSample, "" },
		{ o1_op71, "bw" },
		{ o1_op72, "wbw" },
		{ o1_op72, "wbw" },
		/* 74 */
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ o2_playSample, "bbwbww" },
		/* 78 */
		{ o2_playSampleAlt, "bbwbww" },
		{ o1_disableSystemMenu, "b" },
		{ o1_loadMask5, "b" },
		{ o1_unloadMask5, "b" },
		/* 7C */
		{ 0, 0 },
		{ 0, 0 },
		{ 0, 0 },
		{ o2_addSeqListElement, "bbbbwww" },
		/* 80 */
		{ o2_removeSeq, "bb" },
		{ o2_op81, "" },
		{ o2_op82, "bbw" },
		{ o2_isSeqRunning, "bb" },
		/* 84 */
		{ o2_gotoIfSupNearest, "b" },
		{ o2_gotoIfSupEquNearest, "b" },
		{ o2_gotoIfInfNearest, "b" },
		{ o2_gotoIfInfEquNearest, "b" },
		/* 88 */
		{ o2_gotoIfEquNearest, "b" },
		{ o2_gotoIfDiffNearest, "b" },
		{ 0, 0 },
		{ o2_startObjectScript, "b" },
		/* 8C */
		{ o2_stopObjectScript, "b" },
		{ o2_op8D, "wwwwwwww" },
		{ o2_addBackground, "bs" },
		{ o2_removeBackground, "b" },
		/* 90 */
		{ o2_loadAbs, "bs" },
		{ o2_loadBg, "b" },
		{ 0, 0 },
		{ 0, 0 },
		/* 94 */
		{ 0, 0 },
		{ o1_changeDataDisk, "b" },
		{ 0, 0 },
		{ 0, 0 },
		/* 98 */
		{ 0, 0 },
		{ 0, 0 },
		{ o2_wasZoneChecked, "" },
		{ o2_op9B, "wwwwwwww" },
		/* 9C */
		{ o2_op9C, "wwww" },
		{ o2_useBgScroll, "b" },
		{ o2_setAdditionalBgVScroll, "c" },
		{ o2_op9F, "ww" },
		/* A0 */
		{ o2_addGfxElementA0, "ww" },
		{ o2_opA1, "ww" },
		{ o2_opA2, "ww" },
		{ o2_opA3, "ww" },
		/* A4 */
		{ o2_loadMask22, "b" },
		{ o2_unloadMask22, "b" },
		{ 0, 0 },
		{ 0, 0 },
		/* A8 */
		{ 0, 0 },
		{ o1_changeDataDisk, "b" }
	};

	if (g_cine->getGameType() == Cine::GType_FW) {
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
	_currentPosition += strlen(val) + 1;
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

uint16 computeScriptStackSub(bool computeAllLabels, byte *scriptPtr, int16 *stackPtr, uint16 scriptSize, byte labelIndex, uint16 startOffset) {
	uint16 position;

	if (computeAllLabels) {
		for (int i = 0; i < SCRIPT_STACK_SIZE; i++) {
			stackPtr[i] = -1;
		}
		position = 0;
	} else {
		position = startOffset;
	}
	while (position < scriptSize) {
		uint8 opcode = scriptPtr[position];
		position++;
		if (opcode == 0 || opcode > _numOpcodes) {
			continue;
		}
		if (!_opcodeTable[opcode - 1].args) {
			warning("Undefined opcode 0x%02X in computeScriptStackSub", opcode - 1);
			continue;
		}
		for (const char *p = _opcodeTable[opcode - 1].args; *p; ++p) {
			switch (*p) {
			case 'b': // byte
				position++;
				break;
			case 'w': // word
				position += 2;
				break;
			case 'c': { // byte != 0 ? byte : word
					uint8 test = scriptPtr[position];
					position++;
					if (test) {
						position++;
					} else {
						position += 2;
					}
				}
				break;
			case 'l': { // label
					uint8 index = scriptPtr[position];
					position++;
					if (computeAllLabels) {
						stackPtr[index] = position;
					} else {
						if (labelIndex == index) {
							return position;
						}
					}
				}
				break;
			case 's': // string
				while (scriptPtr[position++] != 0);
				break;
			case 'x': // exit script
				return position;
			}
		}
	}
	return position;
}

void computeScriptStack(byte *scriptPtr, int16 *stackPtr, uint16 scriptSize) {
	computeScriptStackSub(true, scriptPtr, stackPtr, scriptSize, 0, 0);
}

uint16 computeScriptStackFromScript(byte *scriptPtr, uint16 currentPosition, uint16 labelIdx, uint16 scriptSize) {
	return computeScriptStackSub(false, scriptPtr, (int16 *)&dummyU16, (uint16)scriptSize, labelIdx, currentPosition);
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

		idx = getZoneFromPositionRaw(page3Raw, lx + i, ly, 320);

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

	debugC(5, kCineDebugScript, "Line: %d: modifyObjectParam(objIdx:%d,paramIdx:%d,newValue:%d)", _currentLine, objIdx, paramIdx, newValue);

	modifyObjectParam(objIdx, paramIdx, newValue);
}

void o1_getObjectParam() {
	byte objIdx = getNextByte();
	byte paramIdx = getNextByte();
	byte newValue = getNextByte();

	debugC(5, kCineDebugScript, "Line: %d: getObjectParam(objIdx:%d,paramIdx:%d,var:%d)", _currentLine, objIdx, paramIdx, newValue);

	_currentScriptElement->localVars[newValue] = getObjectParam(objIdx, paramIdx);
}

void o1_addObjectParam() {
	byte objIdx = getNextByte();
	byte paramIdx = getNextByte();
	int16 newValue = getNextWord();

	debugC(5, kCineDebugScript, "Line: %d: addObjectParam(objIdx:%d,paramIdx:%d,newValue:%d)", _currentLine, objIdx, paramIdx, newValue);

	addObjectParam(objIdx, paramIdx, newValue);
}

void o1_subObjectParam() {
	byte objIdx = getNextByte();
	byte paramIdx = getNextByte();
	int16 newValue = getNextWord();

	debugC(5, kCineDebugScript, "Line: %d: subObjectParam(objIdx:%d,paramIdx:%d,newValue:%d)", _currentLine, objIdx, paramIdx, newValue);

	subObjectParam(objIdx, paramIdx, newValue);
}

void o1_add2ObjectParam() {
	getNextByte();
	getNextByte();
	getNextWord();
	warning("STUB: o1_add2ObjectParam()");
}

void o1_sub2ObjectParam() {
	getNextByte();
	getNextByte();
	getNextWord();
	warning("STUB: o1_sub2ObjectParam()");
}

void o1_compareObjectParam() {
	byte objIdx = getNextByte();
	byte param1 = getNextByte();
	int16 param2 = getNextWord();

	debugC(5, kCineDebugScript, "Line: %d: compareObjectParam(objIdx:%d,type:%d,value:%d)", _currentLine, objIdx, param1, param2);

	_currentScriptElement->compareResult = compareObjectParam(objIdx, param1, param2);
}

void o1_setupObject() {
	byte objIdx = getNextByte();
	int16 param1 = getNextWord();
	int16 param2 = getNextWord();
	int16 param3 = getNextWord();
	int16 param4 = getNextWord();

	debugC(5, kCineDebugScript, "Line: %d: setupObject(objIdx:%d,%d,%d,%d,%d)", _currentLine, objIdx, param1, param2, param3, param4);

	setupObject(objIdx, param1, param2, param3, param4);
}

void o1_checkCollision() {
	byte objIdx = getNextByte();
	int16 param1 = getNextWord();
	int16 param2 = getNextWord();
	int16 param3 = getNextWord();
	int16 param4 = getNextWord();

	debugC(5, kCineDebugScript, "Line: %d: checkCollision(objIdx:%d,%d,%d,%d,%d)", _currentLine, objIdx, param1, param2, param3, param4);

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
			debugC(5, kCineDebugScript, "Line: %d: var[%d] = var[%d]", _currentLine, varIdx, dataIdx);
			_currentScriptElement->localVars[varIdx] = _currentScriptElement->localVars[dataIdx];
			break;
		case 2:
			debugC(5, kCineDebugScript, "Line: %d: var[%d] = globalVars[%d]", _currentLine, varIdx, dataIdx);
			_currentScriptElement->localVars[varIdx] = globalVars[dataIdx];
			break;
		case 3:
			debugC(5, kCineDebugScript, "Line: %d: var[%d] = mouseX", _currentLine, varIdx);
			getMouseData(mouseUpdateStatus, &dummyU16, (uint16 *)&var, (uint16 *)&dummyU16);
			_currentScriptElement->localVars[varIdx] = var;
			break;
		case 4:
			debugC(5, kCineDebugScript, "Line: %d: var[%d] = mouseY", _currentLine, varIdx);
			getMouseData(mouseUpdateStatus, &dummyU16, (uint16 *)&dummyU16, (uint16 *)&var);
			_currentScriptElement->localVars[varIdx] = var;
			break;
		case 5:
			debugC(5, kCineDebugScript, "Line: %d: var[%d] = rand mod %d", _currentLine, varIdx, dataIdx);
			_currentScriptElement->localVars[varIdx] = rand() % dataIdx;
			break;
		case 8:
			debugC(5, kCineDebugScript, "Line: %d: var[%d] = file[%d].packedSize", _currentLine, varIdx, dataIdx);
			_currentScriptElement->localVars[varIdx] = partBuffer[dataIdx].packedSize;
			break;
		case 9:
			debugC(5, kCineDebugScript, "Line: %d: var[%d] = file[%d].unpackedSize", _currentLine, varIdx, dataIdx);
			_currentScriptElement->localVars[varIdx] = partBuffer[dataIdx].unpackedSize;
			break;
		default:
			error("executeScript: o1_loadVar: Unknown variable type %d", varType);
		}
	} else {
		int16 value = getNextWord();

		debugC(5, kCineDebugScript, "Line: %d: var[%d] = %d", _currentLine, varIdx, value);
		_currentScriptElement->localVars[varIdx] = value;
	}
}

void o1_addVar() {
	byte varIdx = getNextByte();
	byte varType = getNextByte();

	if (varType) {
		byte dataIdx = getNextByte();

		debugC(5, kCineDebugScript, "Line: %d: var[%d] += var[%d]", _currentLine, varIdx, dataIdx);
		_currentScriptElement->localVars[varIdx] += _currentScriptElement->localVars[dataIdx];
	} else {
		int16 value = getNextWord();

		debugC(5, kCineDebugScript, "Line: %d: var[%d] += %d", _currentLine, varIdx, value);
		_currentScriptElement->localVars[varIdx] += value;
	}
}

void o1_subVar() {
	byte varIdx = getNextByte();
	byte varType = getNextByte();

	if (varType) {
		byte dataIdx = getNextByte();

		debugC(5, kCineDebugScript, "Line: %d: var[%d] -= var[%d]", _currentLine, varIdx, dataIdx);
		_currentScriptElement->localVars[varIdx] -= _currentScriptElement->localVars[dataIdx];
	} else {
		int16 value = getNextWord();

		debugC(5, kCineDebugScript, "Line: %d: var[%d] -= %d", _currentLine, varIdx, value);
		_currentScriptElement->localVars[varIdx] -= value;
	}
}

void o1_mulVar() {
	byte varIdx = getNextByte();
	byte varType = getNextByte();

	if (varType) {
		byte dataIdx = getNextByte();

		debugC(5, kCineDebugScript, "Line: %d: var[%d] *= var[%d]", _currentLine, varIdx, dataIdx);
		_currentScriptElement->localVars[varIdx] *= _currentScriptElement->localVars[dataIdx];
	} else {
		int16 value = getNextWord();

		debugC(5, kCineDebugScript, "Line: %d: var[%d] *= %d", _currentLine, varIdx, value);
		_currentScriptElement->localVars[varIdx] *= value;
	}
}

void o1_divVar() {
	byte varIdx = getNextByte();
	byte varType = getNextByte();

	if (varType) {
		byte dataIdx = getNextByte();

		debugC(5, kCineDebugScript, "Line: %d: var[%d] /= var[%d]", _currentLine, varIdx, dataIdx);
		_currentScriptElement->localVars[varIdx] /= _currentScriptElement->localVars[dataIdx];
	} else {
		int16 value = getNextWord();

		debugC(5, kCineDebugScript, "Line: %d: var[%d] /= %d", _currentLine, varIdx, value);
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

			debugC(5, kCineDebugScript, "Line: %d: compare var[%d] and var[%d]", _currentLine, varIdx, dataIdx);
			_currentScriptElement->compareResult = compareVars(_currentScriptElement->localVars[varIdx], _currentScriptElement->localVars[dataIdx]);
		} else if (varType == 2) {
			assert(varIdx < 50);

			debugC(5, kCineDebugScript, "Line: %d: compare var[%d] and globalVar[%d]", _currentLine, varIdx, dataIdx);
			_currentScriptElement->compareResult = compareVars(_currentScriptElement->localVars[varIdx], globalVars[dataIdx]);
		}
	} else {
		int16 value = getNextWord();

		debugC(5, kCineDebugScript, "Line: %d: compare var[%d] and %d", _currentLine, varIdx, value);
		_currentScriptElement->compareResult = compareVars(_currentScriptElement->localVars[varIdx], value);
	}
}

void o1_modifyObjectParam2() {
	byte objIdx = getNextByte();
	byte paramIdx = getNextByte();
	byte newValue = getNextByte();

	debugC(5, kCineDebugScript, "Line: %d: modifyObjectParam2(objIdx:%d,paramIdx:%d,var[%d])", _currentLine, objIdx, paramIdx, newValue);

	modifyObjectParam(objIdx, paramIdx, _currentScriptElement->localVars[newValue]);
}

void o1_loadMask0() {
	// OP_loadV7Element
	byte param = getNextByte();

	debugC(5, kCineDebugScript, "Line: %d: addSpriteOverlay(%d)", _currentLine, param);
	loadOverlayElement(param, 0);
}

void o1_unloadMask0() {
	byte param = getNextByte();

	debugC(5, kCineDebugScript, "Line: %d: removeSpriteOverlay(%d)", _currentLine, param);
	freeOverlay(param, 0);
}

void o1_addToBgList() {
	byte param = getNextByte();

	debugC(5, kCineDebugScript, "Line: %d: addToBGList(%d)", _currentLine, param);
	addToBGList(param);
}

void o1_loadMask1() {
	byte param = getNextByte();

	debugC(5, kCineDebugScript, "Line: %d: addOverlay1(%d)", _currentLine, param);
	loadOverlayElement(param, 1);
}

void o1_unloadMask1() {
	byte param = getNextByte();

	debugC(5, kCineDebugScript, "Line: %d: removeOverlay1(%d)", _currentLine, param);
	freeOverlay(param, 1);
}

void o1_loadMask4() {
	byte param = getNextByte();

	debugC(5, kCineDebugScript, "Line: %d: addOverlayType4(%d)", _currentLine, param);
	loadOverlayElement(param, 4);
}

void o1_unloadMask4() {
	byte param = getNextByte();

	debugC(5, kCineDebugScript, "Line: %d: removeSpriteOverlay4(%d)", _currentLine, param);
	freeOverlay(param, 4);
}

void o1_addSpriteFilledToBgList() {
	byte param = getNextByte();

	debugC(5, kCineDebugScript, "Line: %d: op1A(%d) -> TODO !", _currentLine, param);
	addSpriteFilledToBGList(param);
}

void o1_op1B() {
	debugC(5, kCineDebugScript, "Line: %d: freeBgIncrustList", _currentLine);
	freeBgIncrustList();
}

void o1_label() {
	byte labelIdx = getNextByte();

	debugC(5, kCineDebugScript, "Line: %d: label(%d)", _currentLine, labelIdx);
	_currentScriptElement->stack[labelIdx] = _currentPosition;
}

void o1_goto() {
	byte labelIdx = getNextByte();

	assert(_currentScriptElement->stack[labelIdx] != -1);

	debugC(5, kCineDebugScript, "Line: %d: goto label(%d)", _currentLine, labelIdx);
	_currentPosition = _currentScriptElement->stack[labelIdx];
}

void o1_gotoIfSup() {
	byte labelIdx = getNextByte();

	if (_currentScriptElement->compareResult == kCmpGT) {
		assert(_currentScriptElement->stack[labelIdx] != -1);

		debugC(5, kCineDebugScript, "Line: %d: if(>) goto %d (true)", _currentLine, labelIdx);
		_currentPosition = _currentScriptElement->stack[labelIdx];
	} else {
		debugC(5, kCineDebugScript, "Line: %d: if(>) goto %d (false)", _currentLine, labelIdx);
	}
}

void o1_gotoIfSupEqu() {
	byte labelIdx = getNextByte();

	if (_currentScriptElement->compareResult & (kCmpGT | kCmpEQ)) {
		assert(_currentScriptElement->stack[labelIdx] != -1);

		debugC(5, kCineDebugScript, "Line: %d: if(>=) goto %d (true)", _currentLine, labelIdx);
		_currentPosition = _currentScriptElement->stack[labelIdx];
	} else {
		debugC(5, kCineDebugScript, "Line: %d: if(>=) goto %d (false)", _currentLine, labelIdx);
	}
}

void o1_gotoIfInf() {
	byte labelIdx = getNextByte();

	if (_currentScriptElement->compareResult == kCmpLT) {
		assert(_currentScriptElement->stack[labelIdx] != -1);

		debugC(5, kCineDebugScript, "Line: %d: if(<) goto %d (true)", _currentLine, labelIdx);
		_currentPosition = _currentScriptElement->stack[labelIdx];
	} else {
		debugC(5, kCineDebugScript, "Line: %d: if(<) goto %d (false)", _currentLine, labelIdx);
	}
}

void o1_gotoIfInfEqu() {
	byte labelIdx = getNextByte();

	if (_currentScriptElement->compareResult & (kCmpLT | kCmpEQ)) {
		assert(_currentScriptElement->stack[labelIdx] != -1);

		debugC(5, kCineDebugScript, "Line: %d: if(<=) goto %d (true)", _currentLine, labelIdx);
		_currentPosition = _currentScriptElement->stack[labelIdx];
	} else {
		debugC(5, kCineDebugScript, "Line: %d: if(<=) goto %d (false)", _currentLine, labelIdx);
	}
}

void o1_gotoIfEqu() {
	byte labelIdx = getNextByte();

	if (_currentScriptElement->compareResult == kCmpEQ) {
		assert(_currentScriptElement->stack[labelIdx] != -1);

		debugC(5, kCineDebugScript, "Line: %d: if(==) goto %d (true)", _currentLine, labelIdx);
		_currentPosition = _currentScriptElement->stack[labelIdx];
	} else {
		debugC(5, kCineDebugScript, "Line: %d: if(==) goto %d (false)", _currentLine, labelIdx);
	}
}

void o1_gotoIfDiff() {
	byte labelIdx = getNextByte();

	if (_currentScriptElement->compareResult != kCmpEQ) {
		assert(_currentScriptElement->stack[labelIdx] != -1);

		debugC(5, kCineDebugScript, "Line: %d: if(!=) goto %d (true)", _currentLine, labelIdx);
		_currentPosition = _currentScriptElement->stack[labelIdx];
	} else {
		debugC(5, kCineDebugScript, "Line: %d: if(!=) goto %d (false)", _currentLine, labelIdx);
	}
}

void o1_removeLabel() {
	byte labelIdx = getNextByte();

	debugC(5, kCineDebugScript, "Line: %d: removeLabel(%d)", _currentLine, labelIdx);
	_currentScriptElement->stack[labelIdx] = -1;
}

void o1_loop() {
	byte varIdx = getNextByte();
	byte labelIdx = getNextByte();

	_currentScriptElement->localVars[varIdx]--;

	if (_currentScriptElement->localVars[varIdx] >= 0) {
		assert(_currentScriptElement->stack[labelIdx] != -1);

		debugC(5, kCineDebugScript, "Line: %d: loop(var[%d]) goto %d (continue)", _currentLine, varIdx, labelIdx);
		_currentPosition = _currentScriptElement->stack[labelIdx];
	} else {
		debugC(5, kCineDebugScript, "Line: %d: loop(var[%d]) goto %d (stop)", _currentLine, varIdx, labelIdx);
	}
}

void o1_startGlobalScript() {
	// OP_startScript
	byte param = getNextByte();

	assert(param < NUM_MAX_SCRIPT);

	debugC(5, kCineDebugScript, "Line: %d: startScript(%d)", _currentLine, param);
	addScriptToList0(param);
}

void o1_endGlobalScript() {
	byte scriptIdx = getNextByte();

	debugC(5, kCineDebugScript, "Line: %d: stopGlobalScript(%d)", _currentLine, scriptIdx);
	stopGlobalScript(scriptIdx);
}

void o1_loadAnim() {
	// OP_loadResource
	const char *param = getNextString();

	debugC(5, kCineDebugScript, "Line: %d: loadResource(\"%s\")", _currentLine, param);
	loadResource(param);
}

void o1_loadBg() {
	const char *param = getNextString();

	debugC(5, kCineDebugScript, "Line: %d: loadBg(\"%s\")", _currentLine, param);

	loadBg(param);
	freeBgIncrustList();
	bgVar0 = 0;
}

void o1_loadCt() {
	const char *param = getNextString();

	debugC(5, kCineDebugScript, "Line: %d: loadCt(\"%s\")", _currentLine, param);
	loadCt(param);
}

void o1_loadPart() {
	const char *param = getNextString();

	debugC(5, kCineDebugScript, "Line: %d: loadPart(\"%s\")", _currentLine, param);
	loadPart(param);
}

void o1_closePart() {
	debugC(5, kCineDebugScript, "Line: %d: closePart", _currentLine);
	closePart();
}

void o1_loadNewPrcName() {
	// OP_loadData
	byte param1 = getNextByte();
	const char *param2 = getNextString();

	assert(param1 <= 3);

	switch (param1) {
	case 0:
		debugC(5, kCineDebugScript, "Line: %d: loadPrc(\"%s\")", _currentLine, param2);
		strcpy(newPrcName, param2);
		break;
	case 1:
		debugC(5, kCineDebugScript, "Line: %d: loadRel(\"%s\")", _currentLine, param2);
		strcpy(newRelName, param2);
		break;
	case 2:
		debugC(5, kCineDebugScript, "Line: %d: loadObject(\"%s\")", _currentLine, param2);
		strcpy(newObjectName, param2);
		break;
	case 3:
		debugC(5, kCineDebugScript, "Line: %d: loadMsg(\"%s\")", _currentLine, param2);
		strcpy(newMsgName, param2);
		break;
	}
}

void o1_requestCheckPendingDataLoad() {
	debugC(5, kCineDebugScript, "Line: %d: request data load", _currentLine);
	checkForPendingDataLoadSwitch = 1;
}

void o1_blitAndFade() {
	debugC(5, kCineDebugScript, "Line: %d: request fadein", _currentLine);
	// TODO: use real code

	memcpy(c_palette, tempPalette, sizeof(uint16) * 16);
	drawOverlays();
	flip();

	fadeRequired = 1;
}

void o1_fadeToBlack() {
	debugC(5, kCineDebugScript, "Line: %d: request fadeout", _currentLine);

	fadeToBlack();
}

void o1_transformPaletteRange() {
	byte startColor = getNextByte();
	byte numColor = getNextByte();
	uint16 r = getNextWord();
	uint16 g = getNextWord();
	uint16 b = getNextWord();

	debugC(5, kCineDebugScript, "Line: %d: transformPaletteRange(from:%d,numIdx:%d,r:%d,g:%d,b:%d)", _currentLine, startColor, numColor, r, g, b);

	transformPaletteRange(startColor, numColor, r, g, b);
}

void o1_setDefaultMenuColor2() {
	byte param = getNextByte();

	debugC(5, kCineDebugScript, "Line: %d: setDefaultMenuColor2(%d)", _currentLine, param);
	defaultMenuBoxColor2 = param;
}

void o1_palRotate() {
	byte a = getNextByte();
	byte b = getNextByte();
	byte c = getNextByte();

	debugC(5, kCineDebugScript, "Line: %d: palRotate(%d,%d,%d)", _currentLine, a, b, c);
	palRotate(a, b, c);
}

void o1_break() {
	debugC(5, kCineDebugScript, "Line: %d: break", _currentLine);

	_currentScriptElement->scriptPosition = _currentPosition;
	_closeScript = 1;
}

void o1_endScript() {
	debugC(5, kCineDebugScript, "Line: %d: endScript", _currentLine);

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

	debugC(5, kCineDebugScript, "Line: %d: message(%d,%d,%d,%d,%d)", _currentLine, param1, param2, param3, param4, param5);

	addMessage(param1, param2, param3, param4, param5);
}

void o1_loadGlobalVar() {
	byte varIdx = getNextByte();
	byte varType = getNextByte();

	if (varType) {
		byte dataIdx = getNextByte();

		if (varType == 1) {
			debugC(5, kCineDebugScript, "Line: %d: globalVars[%d] = var[%d]", _currentLine, varIdx, dataIdx);
			globalVars[varIdx] = _currentScriptElement->localVars[dataIdx];
		} else {
			debugC(5, kCineDebugScript, "Line: %d: globalVars[%d] = globalVars[%d]", _currentLine, varIdx, dataIdx);
			globalVars[varIdx] = globalVars[dataIdx];
		}
	} else {
		uint16 value = getNextWord();

		debugC(5, kCineDebugScript, "Line: %d: globalVars[%d] = %d", _currentLine, varIdx, value);
		globalVars[varIdx] = value;
	}
}

void o1_compareGlobalVar() {
	byte varIdx = getNextByte();
	byte varType = getNextByte();

	if (varType) {
		byte value = getNextByte();

		debugC(5, kCineDebugScript, "Line: %d: compare globalVars[%d] and var[%d]", _currentLine, varIdx, value);
		_currentScriptElement->compareResult = compareVars(globalVars[varIdx], _currentScriptElement->localVars[value]);
	} else {
		uint16 value = getNextWord();

		debugC(5, kCineDebugScript, "Line: %d: compare globalVars[%d] and %d", _currentLine, varIdx, value);

		if (varIdx == 255 && (g_cine->getGameType() == Cine::GType_FW)) {	// TODO: fix
			_currentScriptElement->compareResult = 1;
		} else {
			_currentScriptElement->compareResult = compareVars(globalVars[varIdx], value);
		}
	}
}

void o1_declareFunctionName() {
	const char *param = getNextString();

	debugC(5, kCineDebugScript, "Line: %d: comment(%s)", _currentLine, param);
}

void o1_freePartRange() {
	byte startIdx = getNextByte();
	byte numIdx = getNextByte();

	assert(startIdx + numIdx <= NUM_MAX_ANIMDATA);

	debugC(5, kCineDebugScript, "Line: %d: freePartRange(%d,%d)", _currentLine, startIdx, numIdx);
	freeAnimDataRange(startIdx, numIdx);
}

void o1_unloadAllMasks() {
	debugC(5, kCineDebugScript, "Line: %d: unloadAllMasks()", _currentLine);
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
	debugC(5, kCineDebugScript, "Line: %d: initializeZoneData()", _currentLine);

	for (int i = 0; i < NUM_MAX_ZONE; i++) {
		zoneData[i] = i;
	}
}

void o1_setZoneDataEntry() {
	byte zoneIdx = getNextByte();
	uint16 var = getNextWord();

	debugC(5, kCineDebugScript, "Line: %d: setZone[%d] = %d", _currentLine, zoneIdx, var);
	zoneData[zoneIdx] = var;
}

void o1_getZoneDataEntry() {
	byte zoneIdx = getNextByte();
	byte var = getNextByte();
	
	_currentScriptElement->localVars[var] = zoneData[zoneIdx];
}

void o1_setDefaultMenuColor() {
	byte param = getNextByte();

	debugC(5, kCineDebugScript, "Line: %d: setDefaultMenuColor(%d)", _currentLine, param);
	defaultMenuBoxColor = param;
}

void o1_allowPlayerInput() {
	debugC(5, kCineDebugScript, "Line: %d: allowPlayerInput()", _currentLine);
	allowPlayerInput = 1;
}

void o1_disallowPlayerInput() {
	debugC(5, kCineDebugScript, "Line: %d: dissallowPlayerInput()", _currentLine);
	allowPlayerInput = 0;
}

void o1_changeDataDisk() {
	byte newDisk = getNextByte();

	debugC(5, kCineDebugScript, "Line: %d: changeDataDisk(%d)", _currentLine, newDisk);
	checkDataDisk(newDisk);
}

void o1_loadMusic() {
	const char *param = getNextString();

	debugC(5, kCineDebugScript, "Line: %d: loadMusic(%s)", _currentLine, param);
	g_sound->loadMusic(param);
}

void o1_playMusic() {
	debugC(5, kCineDebugScript, "Line: %d: playMusic()", _currentLine);
	g_sound->playMusic();
}

void o1_fadeOutMusic() {
	debugC(5, kCineDebugScript, "Line: %d: fadeOutMusic()", _currentLine);
	g_sound->fadeOutMusic();
}

void o1_stopSample() {
	debugC(5, kCineDebugScript, "Line: %d: stopSample()", _currentLine);
	g_sound->stopMusic();
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
	debugC(5, kCineDebugScript, "Line: %d: playSample()", _currentLine);

	byte anim = getNextByte();
	byte channel = getNextByte();

	uint16 freq = getNextWord();
	byte repeat = getNextByte();

	int16 volume = getNextWord();
	uint16 size = getNextWord();

	if (!animDataTable[anim].ptr1) {
		return;
	}

	if (g_cine->getPlatform() == Common::kPlatformAmiga || g_cine->getPlatform() == Common::kPlatformAtariST) {
		if (size == 0xFFFF) {
			size = animDataTable[anim].width * animDataTable[anim].height;
		}
		if (channel < 10) { // || _currentOpcode == 0x78
			int channel1, channel2;
			if (channel == 0) {
				channel1 = 0;
				channel2 = 1;
			} else {
				channel1 = 2;
				channel2 = 3;			
			}
			g_sound->playSound(channel1, freq, animDataTable[anim].ptr1, size, -1, volume, 63, repeat);
			g_sound->playSound(channel2, freq, animDataTable[anim].ptr1, size,  1, volume,  0, repeat);
		} else {
			channel -= 10;
			if (volume > 63) {
				volume = 63;
			}
			g_sound->playSound(channel, freq, animDataTable[anim].ptr1, size, 0, 0, volume, repeat);
		}
	} else {
		if (volume > 63 || volume < 0) {
			volume = 63;
		}
		if (channel >= 10) {
			channel -= 10;
		}
		if (volume < 50) {
			volume = 50;
		}
		g_sound->stopMusic();
		if (size == 0xFFFF) {
			g_sound->playSound(channel, 0, animDataTable[anim].ptr1, 0, 0, 0, volume, 0);
		} else {
			g_sound->stopSound(channel);
		}
	}
}

void o1_disableSystemMenu() {
	byte param = getNextByte();

	debugC(5, kCineDebugScript, "Line: %d: disableSystemMenu(%d)", _currentLine, param);
	disableSystemMenu = (param != 0);
}

void o1_loadMask5() {
	byte param = getNextByte();

	debugC(5, kCineDebugScript, "Line: %d: addOverlay5(%d)", _currentLine, param);
	loadOverlayElement(param, 5);
}

void o1_unloadMask5() {
	byte param = getNextByte();

	debugC(5, kCineDebugScript, "Line: %d: freeOverlay5(%d)", _currentLine, param);
	freeOverlay(param, 5);
}

// ------------------------------------------------------------------------
// OPERATION STEALTH opcodes
// ------------------------------------------------------------------------

void o2_loadPart() {
	const char *param = getNextString();

	debugC(5, kCineDebugScript, "Line: %d: loadPart(\"%s\")", _currentLine, param);
}

void o2_playSample() {
	if (g_cine->getPlatform() == Common::kPlatformAmiga || g_cine->getPlatform() == Common::kPlatformAtariST) {
		// no-op in these versions
		getNextByte();
		getNextByte();
		getNextWord();
		getNextByte();
		getNextWord();
		getNextWord();
		return;
	}
	o1_playSample();
}

void o2_playSampleAlt() {
	byte num = getNextByte();
	byte channel = getNextByte();
	uint16 frequency = getNextWord();
	getNextByte();
	getNextWord();
	uint16 size = getNextWord();

	if (size == 0xFFFF) {
		size = animDataTable[num].width * animDataTable[num].height;
	}
	if (animDataTable[num].ptr1) {
		if (g_cine->getPlatform() == Common::kPlatformPC) {
			// if speaker output is enabled, play sound on it
			// if it's another device, don't play anything
		} else {
			g_sound->playSound(channel, frequency, animDataTable[num].ptr1, size, 0, 0, 63, 0);
		}
	}
}

void o2_addSeqListElement() {
	byte param1 = getNextByte();
	byte param2 = getNextByte();
	byte param3 = getNextByte();
	byte param4 = getNextByte();
	uint16 param5 = getNextWord();
	uint16 param6 = getNextWord();
	uint16 param7 = getNextWord();

	debugC(5, kCineDebugScript, "Line: %d: addSeqListElement(%d,%d,%d,%d,%d,%d,%d)", _currentLine, param1, param2, param3, param4, param5, param6, param7);
	addSeqListElement(param1, 0, param2, param3, param4, param5, param6, 0, param7);
}

void o2_removeSeq() {
	byte a = getNextByte();
	byte b = getNextByte();

	debugC(5, kCineDebugScript, "Line: %d: removeSeq(%d,%d) -> TODO", _currentLine, a, b);
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

	debugC(5, kCineDebugScript, "Line: %d: OP83(%d,%d) -> TODO", _currentLine, a, b);

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

		debugC(5, kCineDebugScript, "Line: %d: if(>) goto nearest %d (true)", _currentLine, labelIdx);
		_currentPosition = computeScriptStackFromScript(_currentScriptElement->scriptPtr, _currentPosition, labelIdx, scriptTable[_currentScriptElement->scriptIdx].size);
	} else {
		debugC(5, kCineDebugScript, "Line: %d: if(>) goto nearest %d (false)", _currentLine, labelIdx);
	}
}

void o2_gotoIfSupEquNearest() {
	byte labelIdx = getNextByte();

	if (_currentScriptElement->compareResult & (kCmpGT | kCmpEQ)) {
		assert(_currentScriptElement->stack[labelIdx] != -1);

		debugC(5, kCineDebugScript, "Line: %d: if(>=) goto nearest %d (true)", _currentLine, labelIdx);
		_currentPosition = computeScriptStackFromScript(_currentScriptElement->scriptPtr, _currentPosition, labelIdx, scriptTable[_currentScriptElement->scriptIdx].size);
	} else {
		debugC(5, kCineDebugScript, "Line: %d: if(>=) goto nearest %d (false)", _currentLine, labelIdx);
	}
}

void o2_gotoIfInfNearest() {
	byte labelIdx = getNextByte();

	if (_currentScriptElement->compareResult == kCmpLT) {
		assert(_currentScriptElement->stack[labelIdx] != -1);

		debugC(5, kCineDebugScript, "Line: %d: if(<) goto nearest %d (true)", _currentLine, labelIdx);
		_currentPosition = computeScriptStackFromScript(_currentScriptElement->scriptPtr, _currentPosition, labelIdx, scriptTable[_currentScriptElement->scriptIdx].size);
	} else {
		debugC(5, kCineDebugScript, "Line: %d: if(<) goto nearest %d (false)", _currentLine, labelIdx);
	}
}

void o2_gotoIfInfEquNearest() {
	byte labelIdx = getNextByte();

	if (_currentScriptElement->compareResult & (kCmpLT | kCmpEQ)) {
		assert(_currentScriptElement->stack[labelIdx] != -1);

		debugC(5, kCineDebugScript, "Line: %d: if(<=) goto nearest %d (true)", _currentLine, labelIdx);
		_currentPosition = computeScriptStackFromScript(_currentScriptElement->scriptPtr, _currentPosition, labelIdx, scriptTable[_currentScriptElement->scriptIdx].size);
	} else {
		debugC(5, kCineDebugScript, "Line: %d: if(<=) goto nearest %d (false)", _currentLine, labelIdx);
	}
}

void o2_gotoIfEquNearest() {
	byte labelIdx = getNextByte();

	if (_currentScriptElement->compareResult == kCmpEQ) {
		assert(_currentScriptElement->stack[labelIdx] != -1);

		debugC(5, kCineDebugScript, "Line: %d: if(==) goto nearest %d (true)", _currentLine, labelIdx);
		_currentPosition = computeScriptStackFromScript(_currentScriptElement->scriptPtr, _currentPosition, labelIdx, scriptTable[_currentScriptElement->scriptIdx].size);
	} else {
		debugC(5, kCineDebugScript, "Line: %d: if(==) goto nearest %d (false)", _currentLine, labelIdx);
	}
}

void o2_gotoIfDiffNearest() {
	byte labelIdx = getNextByte();

	if (_currentScriptElement->compareResult != kCmpEQ) {
		assert(_currentScriptElement->stack[labelIdx] != -1);

		debugC(5, kCineDebugScript, "Line: %d: if(!=) goto nearest %d (true)", _currentLine, labelIdx);
		_currentPosition = computeScriptStackFromScript(_currentScriptElement->scriptPtr, _currentPosition, labelIdx, scriptTable[_currentScriptElement->scriptIdx].size);
	} else {
		debugC(5, kCineDebugScript, "Line: %d: if(!=) goto nearest %d (false)", _currentLine, labelIdx);
	}
}

void o2_startObjectScript() {
	byte param = getNextByte();

	debugC(5, kCineDebugScript, "Line: %d: startObjectScript(%d)", _currentLine, param);
	runObjectScript(param);
}

void o2_stopObjectScript() {
	byte param = getNextByte();

	debugC(5, kCineDebugScript, "Line: %d: stopObjectScript(%d)", _currentLine, param);
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

	debugC(5, kCineDebugScript, "Line: %d: addBackground(%s,%d)", _currentLine, param2, param1);
	addBackground(param2, param1);
}

void o2_removeBackground() {
	byte param = getNextByte();

	assert(param);

	debugC(5, kCineDebugScript, "Line: %d: removeBackground(%d)", _currentLine, param);

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

	debugC(5, kCineDebugScript, "Line: %d: loadABS(%d,%s)", _currentLine, param1, param2);
	loadAbs(param2, param1);
}

void o2_loadBg() {
	byte param = getNextByte();

	assert(param <= 8);

	debugC(5, kCineDebugScript, "Line: %d: useBg(%d)", _currentLine, param);

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

	debugC(5, kCineDebugScript, "Line: %d: useBgScroll(%d)", _currentLine, param);

	if (additionalBgTable[param]) {
		currentAdditionalBgIdx2 = param;
	}
}

void o2_setAdditionalBgVScroll() {
	byte param1 = getNextByte();

	if (param1) {
		byte param2 = getNextByte();

		debugC(5, kCineDebugScript, "Line: %d: additionalBgVScroll = var[%d]", _currentLine, param2);
		additionalBgVScroll = _currentScriptElement->localVars[param2];
	} else {
		uint16 param2 = getNextWord();

		debugC(5, kCineDebugScript, "Line: %d: additionalBgVScroll = %d", _currentLine, param2);
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

	debugC(5, kCineDebugScript, "Line: %d: addGfxElementA0(%d,%d)", _currentLine, param1, param2);
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

	debugC(5, kCineDebugScript, "Line: %d: addOverlay22(%d)", _currentLine, param);
	loadOverlayElement(param, 22);
}

void o2_unloadMask22() {
	byte param = getNextByte();

	debugC(5, kCineDebugScript, "Line: %d: removeOverlay22(%d)", _currentLine, param);
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
			if (_opcodeTable[opcode - 1].proc)
				(_opcodeTable[opcode - 1].proc) ();
			else
				warning("Undefined opcode 0x%02X in executeScript", opcode - 1);
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

char decompileBuffer[10000][1000];
uint16 decompileBufferPosition = 0;

char bufferDec[256];

char compareString1[256];
char compareString2[256];

const char *getObjPramName(byte paramIdx) {
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
	char lineBuffer[256];
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
					} else if (param2 == 8) {
						sprintf(lineBuffer, "var[%d]=file[%d].packedSize\n", param1, param3);
					} else if (param2 == 9) {
						sprintf(lineBuffer, "var[%d]=file[%d].unpackedSize\n", param1, param3);
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
				sprintf(lineBuffer, "freeBgIncrustList()\n");
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

				position += strlen((char *)localScriptPtr + position) + 1;
				break;
			}
		case 0x3C:
			{
				sprintf(lineBuffer, "loadBg(%s)\n",	localScriptPtr + position);

				position += strlen((char *)localScriptPtr + position) + 1;
				break;
			}
		case 0x3D:
			{
				sprintf(lineBuffer, "loadCt(%s)\n", localScriptPtr + position);

				position += strlen((char *)localScriptPtr + position) + 1;
				break;
			}
		case OP_loadPart:
			{
				sprintf(lineBuffer, "loadPart(%s)\n", localScriptPtr + position);

				position += strlen((char *)localScriptPtr + position) + 1;
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

				position += strlen((char *)localScriptPtr + position) + 1;
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

				param4 = READ_BE_UINT16(localScriptPtr + position);
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

				position += strlen((char *)localScriptPtr + position);
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

				position += strlen((char *)localScriptPtr + position) + 1;
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

				sprintf(lineBuffer, "disableSystemMenu(%d)\n", param);

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

				sprintf(lineBuffer, "OP_7F(%d,%d,%d,%d,%d,%d,%d)\n", param1, param2, param3, param4, param5, param6, param7);

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

				position += strlen((char *)localScriptPtr + position);

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

				position += strlen((char *)localScriptPtr + position);

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

void dumpScript(char *dumpName) {
    Common::File fHandle;
	uint16 i;

	fHandle.open(dumpName, Common::File::kFileWriteMode);

	for (i = 0; i < decompileBufferPosition; i++) {
		fHandle.writeString(Common::String(decompileBuffer[i]));
	}

	fHandle.close();

	decompileBufferPosition = 0;
}

#endif

} // End of namespace Cine
