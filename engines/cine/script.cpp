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

#include "cine/cine.h"
#include "cine/bg_list.h"
#include "cine/flip_support.h"
#include "cine/object.h"
#include "cine/sfx_player.h"
#include "cine/sound_driver.h"
#include "cine/various.h"

namespace Cine {

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
	newElement->var10 = 0;
	newElement->var12 = 0;

	if (!currentHead)
		currentHead = &overlayHead;

	newElement->previous = currentHead->previous;

	currentHead->previous = newElement;
}

void removeSeq(uint16 param1, uint16 param2, uint16 param3) {
	unkListElementStruct *currentHead = &unkList;
	unkListElementStruct *tempHead = currentHead;

	while (currentHead && (currentHead->var6 != param1 || currentHead->var4 != param2 || currentHead->varE != param3)) {
		tempHead = currentHead;
		currentHead = tempHead->next;
	}

	if (currentHead && currentHead->var6 == param1 && currentHead->var4 == param2 && currentHead->varE == param3) {
		currentHead->var4 = -1;
	}
}

uint16 isSeqRunning(uint16 param1, uint16 param2, uint16 param3) {
	unkListElementStruct *currentHead = &unkList;
	unkListElementStruct *tempHead = currentHead;

	while (currentHead && (currentHead->var6 != param1 || currentHead->var4 != param2 || currentHead->varE != param3)) {
		tempHead = currentHead;
		currentHead = tempHead->next;
	}

	if (currentHead && currentHead->var6 == param1 && currentHead->var4 == param2 && currentHead->varE == param3) {
		return (1);
	}

	return (0);
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

uint16 computeScriptStackSub(uint8 mode, byte *scriptPtr, int16 *stackPtr, uint16 scriptSize, uint8 param1, uint16 startOffset) {
	byte *localScriptPtr = scriptPtr;
	uint16 exitScript;
	uint16 i;
	uint16 position;
	uint16 di;

	ASSERT_PTR(scriptPtr);
	ASSERT_PTR(stackPtr);

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
				uint8 param;
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
				uint8 param;

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
						return (position);
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

	return (position);
}

void computeScriptStack(byte *scriptPtr, int16 *stackPtr, uint16 scriptSize) {
	computeScriptStackSub(1, scriptPtr, stackPtr, scriptSize, 0, 0);
}

uint16 computeScriptStackFromScript(byte *scriptPtr, uint16 currentPosition, uint16 labelIdx, uint16 scriptSize) {
	return computeScriptStackSub(0, scriptPtr, (int16 *)&dummyU16, (uint16)scriptSize, labelIdx, currentPosition);
}

void palRotate(uint8 a, uint8 b, uint8 c) {
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

	ASSERT(idx <= NUM_MAX_SCRIPT);

	currentHead = tempHead->next;

	while (currentHead) {
		tempHead = currentHead;

		ASSERT_PTR(tempHead);

		currentHead = tempHead->next;
	}

	pNewElement =(prcLinkedListStruct *)malloc(sizeof(prcLinkedListStruct));

	ASSERT_PTR(pNewElement);

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

	//ASSERT(scriptIdx <= NUM_MAX_SCRIPT);

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

	return (0);
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

	return (0);
}

int16 getZoneFromPosition(uint8 *page, int16 x, int16 y, int16 width) {
	uint8 *ptr = page + (y * width) + x / 2;
	uint8 zoneVar;

	if (!(x % 2)) {
		zoneVar = (*(ptr) >> 4) & 0xF;
	} else {
		zoneVar = (*(ptr)) & 0xF;
	}

	return (zoneVar);
}

int16 getZoneFromPositionRaw(uint8 *page, int16 x, int16 y, int16 width) {
	uint8 *ptr = page + (y * width) + x;
	uint8 zoneVar;

	zoneVar = (*(ptr)) & 0xF;

	return (zoneVar);
}

int16 checkCollision(int16 objIdx, int16 x, int16 y, int16 numZones, int16 zoneIdx) {
	int16 i;
	int16 lx;
	int16 ly;

	lx = objectTable[objIdx].x + x;
	ly = objectTable[objIdx].y + y;

	for (i = 0; i < numZones; i++) {
		int16 idx;

		// if(gameType == GAME_OS)
		{
			idx = getZoneFromPositionRaw(page3Raw, lx + i, ly, 320);
		}
		/*  else
		 * {
		 * idx = getZoneFromPosition(page3, lx + i, ly, 160);
		 * } */

		ASSERT(idx >= 0 && idx <= NUM_MAX_ZONE);

		if (zoneData[idx] == zoneIdx) {
			return 1;
		}
	}

	return 0;
}

uint16 compareVars(int16 a, int16 b) {
	uint16 flag = 0;

	if (a == b) {
		flag |= 1;
	}

	if (a > b) {
		flag |= 2;
	}

	if (a < b) {
		flag |= 4;
	}

	return flag;
}

void executeScript(prcLinkedListStruct *scriptElement, uint16 params) {
	byte *currentScriptPtr;
	uint16 closeScript;
	uint16 currentPosition;

	ASSERT_PTR(scriptElement);

	if (scriptElement->scriptIdx == -1) {
		return;
	}

	currentScriptPtr = scriptElement->scriptPtr;

	ASSERT_PTR(currentScriptPtr);

	currentPosition = scriptElement->scriptPosition;
	closeScript = 0;

	while (!closeScript) {
		uint16 currentLine;
		uint16 opcode;

		currentLine = currentPosition;

		opcode = *(currentScriptPtr + currentPosition);
		currentPosition++;

		//printf("Op: %X\n", opcode - 1);

		switch (opcode - 1) {
		case -1:
			{
				break;
			}
		case 0x0:	//OP_modifyObjectParam
			{
				uint8 objIdx;
				uint8 paramIdx;
				int16 newValue;

				objIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				paramIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				newValue = READ_BE_UINT16(currentScriptPtr + currentPosition);
				currentPosition += 2;

				DEBUG_SCRIPT(currentLine, "modifyObjectParam(objIdx:%d,paramIdx:%d,newValue:%d)", objIdx, paramIdx, newValue);

				modifyObjectParam(objIdx, paramIdx, newValue);

				break;
			}
		case 0x1:	//OP_getObjectParam
			{
				uint8 objIdx;
				uint8 paramIdx;
				uint8 newValue;

				objIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				paramIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				newValue = *(currentScriptPtr + currentPosition);
				currentPosition++;

				DEBUG_SCRIPT(currentLine, "getObjectParam(objIdx:%d,paramIdx:%d,var:%d)", objIdx, paramIdx, newValue);

				scriptElement->localVars[newValue] = getObjectParam(objIdx, paramIdx);

				break;
			}
		case 0x2:
			{
				uint8 objIdx;
				uint8 paramIdx;
				int16 newValue;

				objIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				paramIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				newValue = READ_BE_UINT16(currentScriptPtr + currentPosition);
				currentPosition += 2;

				DEBUG_SCRIPT(currentLine, "addObjectParam(objIdx:%d,paramIdx:%d,newValue:%d)", objIdx, paramIdx, newValue);

				addObjectParam(objIdx, paramIdx, newValue);

				break;
			}
		case 0x3:
			{
				uint8 objIdx;
				uint8 paramIdx;
				int16 newValue;

				objIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				paramIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				newValue = READ_BE_UINT16(currentScriptPtr + currentPosition);
				currentPosition += 2;

				DEBUG_SCRIPT(currentLine, "subObjectParam(objIdx:%d,paramIdx:%d,newValue:%d)", objIdx, paramIdx, newValue);

				subObjectParam(objIdx, paramIdx, newValue);

				break;
			}
		case 0x6:
			{
				uint8 objIdx;
				uint8 param1;
				int16 param2;

				objIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				param1 = *(currentScriptPtr + currentPosition);
				currentPosition++;

				param2 = READ_BE_UINT16(currentScriptPtr + currentPosition);
				currentPosition += 2;

				DEBUG_SCRIPT(currentLine, "compareObjectParam(objIdx:%d,type:%d,value:%d)", objIdx, param1, param2);

				scriptElement->compareResult = compareObjectParam(objIdx, param1, param2);

				break;
			}
		case 0x7:
			{
				uint8 objIdx;
				int16 param1;
				int16 param2;
				int16 param3;
				int16 param4;

				objIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				param1 = READ_BE_UINT16(currentScriptPtr + currentPosition);
				currentPosition += 2;

				param2 = READ_BE_UINT16(currentScriptPtr + currentPosition);
				currentPosition += 2;

				param3 = READ_BE_UINT16(currentScriptPtr + currentPosition);
				currentPosition += 2;

				param4 = READ_BE_UINT16(currentScriptPtr + currentPosition);
				currentPosition += 2;

				DEBUG_SCRIPT(currentLine, "setupObject(objIdx:%d,%d,%d,%d,%d)", objIdx, param1, param2, param3, param4);

				setupObject(objIdx, param1, param2, param3, param4);

				break;
			}
		case 0x8:
			{
				uint8 objIdx;
				int16 param1;
				int16 param2;
				int16 param3;
				int16 param4;

				objIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				param1 = READ_BE_UINT16(currentScriptPtr + currentPosition);
				currentPosition += 2;

				param2 = READ_BE_UINT16(currentScriptPtr + currentPosition);
				currentPosition += 2;

				param3 = READ_BE_UINT16(currentScriptPtr + currentPosition);
				currentPosition += 2;

				param4 = READ_BE_UINT16(currentScriptPtr + currentPosition);
				currentPosition += 2;

				DEBUG_SCRIPT(currentLine, "checkCollision(objIdx:%d,%d,%d,%d,%d)", objIdx, param1, param2, param3, param4);

				scriptElement->compareResult = checkCollision(objIdx, param1, param2, param3, param4);

				break;
			}
		case 0x9:	// OP_loadVar
			{
				uint8 varIdx;
				uint8 varType;

				varIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				varType = *(currentScriptPtr + currentPosition);
				currentPosition++;

				if (varType) {
					uint8 dataIdx;

					dataIdx = *(currentScriptPtr + currentPosition);
					currentPosition++;

					switch (varType) {
					case 1:
						{
							DEBUG_SCRIPT(currentLine, "var[%d] = var[%d]", varIdx, dataIdx);
							scriptElement->localVars[varIdx] = scriptElement->localVars[dataIdx];
							break;
						}
					case 2:
						{
							DEBUG_SCRIPT(currentLine, "var[%d] = globalVars[%d]", varIdx, dataIdx);
							scriptElement->localVars[varIdx] = globalVars[dataIdx];
							break;
						}
					case 3:
						{
							int16 var;

							DEBUG_SCRIPT(currentLine, "var[%d] = mouseX", varIdx, dataIdx);
							getMouseData(mouseUpdateStatus, &dummyU16, (uint16 *)&var, (uint16 *)&dummyU16);
							scriptElement->localVars[varIdx] = var;
							break;
						}
					case 4:
						{
							int16 var;

							DEBUG_SCRIPT(currentLine, "var[%d] = mouseY", varIdx, dataIdx);
							getMouseData(mouseUpdateStatus, &dummyU16, (uint16 *)&dummyU16, (uint16 *)&var);
							scriptElement->localVars[varIdx] = var;
							break;
						}
					case 5:
						{
							DEBUG_SCRIPT(currentLine, "var[%d] = rand mod %d", varIdx, dataIdx);
							scriptElement->localVars[varIdx] = rand() % dataIdx;
							break;
						}
					case 8:
						{
							DEBUG_SCRIPT(currentLine, "var[%d] = file[%d].packedSize", varIdx, dataIdx);
							scriptElement->localVars[varIdx] = partBuffer[dataIdx].packedSize;
							break;
						}
					case 9:
						{
							DEBUG_SCRIPT(currentLine, "var[%d] = file[%d].unpackedSize", varIdx, dataIdx);
							scriptElement->localVars[varIdx] = partBuffer[dataIdx].unpackedSize;
							break;
						}
					default:
						{
							ASSERT(0);
						}
					}
				} else {
					int16 newData;

					newData = READ_BE_UINT16(currentScriptPtr + currentPosition);
					currentPosition += 2;

					DEBUG_SCRIPT(currentLine, "var[%d] = %d", varIdx, newData);

					scriptElement->localVars[varIdx] = newData;
				}
				break;
			}
		case 0xA:	// OP_addVar
			{
				uint8 param1;
				uint8 type;

				param1 = *(currentScriptPtr + currentPosition);
				currentPosition++;

				type = *(currentScriptPtr + currentPosition);
				currentPosition++;

				if (type) {
					uint8 param2;

					param2 = *(currentScriptPtr + currentPosition);
					currentPosition++;

					DEBUG_SCRIPT(currentLine, "var[%d] += var[%d]", param1, param2);

					scriptElement->localVars[param1] += scriptElement->localVars[param2];
				} else {
					int16 param2;

					param2 = READ_BE_UINT16(currentScriptPtr + currentPosition);
					currentPosition += 2;

					DEBUG_SCRIPT(currentLine, "var[%d] += %d", param1, param2);

					scriptElement->localVars[param1] += param2;
				}

				break;
			}
		case 0xB:	// OP_subVar
			{
				uint8 param1;
				uint8 type;

				param1 = *(currentScriptPtr + currentPosition);
				currentPosition++;

				type = *(currentScriptPtr + currentPosition);
				currentPosition++;

				if (type) {
					uint8 param2;

					param2 = *(currentScriptPtr + currentPosition);
					currentPosition++;

					DEBUG_SCRIPT(currentLine, "var[%d] -= var[%d]", param1, param2);

					scriptElement->localVars[param1] = scriptElement->localVars[param1] - scriptElement->localVars[param2];
				} else {
					int16 param2;

					param2 = READ_BE_UINT16(currentScriptPtr + currentPosition);
					currentPosition += 2;

					DEBUG_SCRIPT(currentLine, "var[%d] -= %d", param1, param2);

					scriptElement->localVars[param1] = scriptElement->localVars[param1] - param2;
				}

				break;
			}
		case 0xC:	// OP_mulVar
			{
				uint8 param1;
				uint8 type;

				param1 = *(currentScriptPtr + currentPosition);
				currentPosition++;

				type = *(currentScriptPtr + currentPosition);
				currentPosition++;

				if (type) {
					uint8 param2;

					param2 = *(currentScriptPtr + currentPosition);
					currentPosition++;

					DEBUG_SCRIPT(currentLine, "var[%d] *= var[%d]", param1, param2);

					scriptElement->localVars[param1] = scriptElement->localVars[param1] * scriptElement->localVars[param2];
				} else {
					int16 param2;

					param2 = READ_BE_UINT16(currentScriptPtr + currentPosition);
					currentPosition += 2;

					DEBUG_SCRIPT(currentLine, "var[%d] *= %d", param1, param2);

					scriptElement->localVars[param1] = scriptElement->localVars[param1] * param2;
				}

				break;
			}
		case 0xD:	// OP_modVar
			{
				uint8 param1;
				uint8 type;

				param1 = *(currentScriptPtr + currentPosition);
				currentPosition++;

				type = *(currentScriptPtr + currentPosition);
				currentPosition++;

				if (type) {
					uint8 param2;

					param2 = *(currentScriptPtr + currentPosition);
					currentPosition++;

					DEBUG_SCRIPT(currentLine, "var[%d] /= var[%d]", param1, param2);

					scriptElement->localVars[param1] = scriptElement->localVars[param1] / scriptElement->localVars[param2];
				} else {
					int16 param2;

					param2 = READ_BE_UINT16(currentScriptPtr + currentPosition);
					currentPosition += 2;

					DEBUG_SCRIPT(currentLine, "var[%d] /= %d", param1, param2);

					scriptElement->localVars[param1] = scriptElement->localVars[param1] / param2;
				}

				break;
			}
		case 0xE:	// OP_ compareVar
			{
				uint8 varIdx;
				uint8 varType;

				varIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				varType = *(currentScriptPtr + currentPosition);
				currentPosition++;

				if (varType) {
					uint8 value;

					value = *(currentScriptPtr + currentPosition);
					currentPosition++;

					// printf("Val: %d\n", value);

					if (varType == 1) {
						DEBUG_SCRIPT(currentLine, "compare var[%d] and var[%d]", varIdx, value);

						ASSERT(varIdx < 50);
						ASSERT(value < 50);

						scriptElement->compareResult = compareVars(scriptElement->localVars[varIdx], scriptElement->localVars[value]);
					} else if (varType == 2) {
						DEBUG_SCRIPT(currentLine, "compare var[%d] and globalVar[%d]", varIdx, value);

						ASSERT(varIdx < 50);

						scriptElement->compareResult = compareVars(scriptElement->localVars[varIdx], globalVars[value]);
					}
				} else {
					int16 value;

					value = READ_BE_UINT16(currentScriptPtr + currentPosition);
					currentPosition += 2;

					DEBUG_SCRIPT(currentLine, "compare var[%d] and %d", varIdx, value);

					scriptElement->compareResult = compareVars(scriptElement->localVars[varIdx], value);
				}

				break;
			}
		case 0xF:	//OP_modifyObjectParam2
			{
				uint8 objIdx;
				uint8 paramIdx;
				uint8 newValue;

				objIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				paramIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				newValue = *(currentScriptPtr + currentPosition);
				currentPosition++;

				DEBUG_SCRIPT(currentLine, "modifyObjectParam2(objIdx:%d,paramIdx:%d,var[%d])", objIdx, paramIdx, newValue);

				modifyObjectParam(objIdx, paramIdx, scriptElement->localVars[newValue]);

				break;
			}
		case 0x13:	// OP_loadV7Element
			{
				uint8 param;

				param = *(currentScriptPtr + currentPosition);
				currentPosition++;

				DEBUG_SCRIPT(currentLine, "addSpriteOverlay(%d)", param);

				loadOverlayElement(param, 0);

				break;
			}
		case 0x14:
			{
				uint8 param;

				param = *(currentScriptPtr + currentPosition);
				currentPosition++;

				DEBUG_SCRIPT(currentLine, "removeSpriteOverlay(%d)", param);

				freeOverlay(param, 0);

				break;
			}
		case 0x15:
			{
				uint8 param;

				param = *(currentScriptPtr + currentPosition);
				currentPosition++;

				DEBUG_SCRIPT(currentLine, "addToBGList(%d)", param);

				addToBGList(param);

				break;
			}
		case 0x16:
			{
				uint8 param;

				param = *(currentScriptPtr + currentPosition);
				currentPosition++;

				DEBUG_SCRIPT(currentLine, "addOverlay1(%d)", param);

				loadOverlayElement(param, 1);

				break;
			}
		case 0x17:
			{
				uint8 param;

				param = *(currentScriptPtr + currentPosition);
				currentPosition++;

				DEBUG_SCRIPT(currentLine, "removeOverlay1(%d)", param);

				freeOverlay(param, 1);

				break;
			}
		case 0x18:
			{
				uint8 param;

				param = *(currentScriptPtr + currentPosition);
				currentPosition++;

				DEBUG_SCRIPT(currentLine, "addOverlayType4(%d)", param);

				loadOverlayElement(param, 4);

				break;
			}
		case 0x19:
			{
				uint8 param;

				param = *(currentScriptPtr + currentPosition);
				currentPosition++;

				DEBUG_SCRIPT(currentLine, "removeSpriteOverlay4(%d)", param);

				freeOverlay(param, 4);

				break;
			}
		case 0x1A:
			{
				uint8 param;

				param = *(currentScriptPtr + currentPosition);
				currentPosition++;

				DEBUG_SCRIPT(currentLine, "op1A(%d) -> TODO !", param);

				addSpriteFilledToBGList(param);

				break;
			}
		case 0x1B:
			{
				DEBUG_SCRIPT(currentLine, "closeEngine7");
				closeEngine7();
				break;
			}
		case 0x1D:	// OP_label
			{
				uint8 labelIdx;

				labelIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				DEBUG_SCRIPT(currentLine, "label(%d)", labelIdx);

				scriptElement->stack[labelIdx] = currentPosition;

				break;
			}
		case 0x1E:	// OP_goto
			{
				uint8 labelIdx;

				labelIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				DEBUG_SCRIPT(currentLine, "goto label(%d)", labelIdx);

				ASSERT(scriptElement->stack[labelIdx] != -1);
				currentPosition = scriptElement->stack[labelIdx];

				break;
			}
		case 0x1F:	// OP_gotoIfSup
			{
				uint8 labelIdx;

				labelIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				if ((scriptElement->compareResult & 2) && !(scriptElement->compareResult & 1)) {
					DEBUG_SCRIPT(currentLine, "if(>) goto %d (true)", labelIdx);
					ASSERT(scriptElement->stack[labelIdx] != -1);
					currentPosition = scriptElement->stack[labelIdx];
				} else {
					DEBUG_SCRIPT(currentLine, "if(>) goto %d (false)", labelIdx);
				}

				break;
			}
		case 0x20:	// OP_gotoIfSupEqu
			{
				uint8 labelIdx;

				labelIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				if (scriptElement->compareResult & 2 || scriptElement->compareResult & 1) {
					DEBUG_SCRIPT(currentLine, "if(>=) goto %d (true)", labelIdx);
					ASSERT(scriptElement->stack[labelIdx] != -1);
					currentPosition = scriptElement->stack[labelIdx];
				} else {
					DEBUG_SCRIPT(currentLine, "if(>=) goto %d (false)",
					    labelIdx);
				}

				break;
			}
		case 0x21:	// OP_gotoIfInf
			{
				uint8 labelIdx;

				labelIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				if ((scriptElement->compareResult & 4) && !(scriptElement->compareResult & 1)) {
					DEBUG_SCRIPT(currentLine, "if(<) goto %d (true)", labelIdx);
					ASSERT(scriptElement->stack[labelIdx] != -1);
					currentPosition = scriptElement->stack[labelIdx];
				} else {
					DEBUG_SCRIPT(currentLine, "if(<) goto %d (false)", labelIdx);
				}

				break;
			}
		case 0x22:	// OP_gotoIfInfEqu
			{
				uint8 labelIdx;

				labelIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				if ((scriptElement->compareResult & 4) || (scriptElement->compareResult & 1)) {
					DEBUG_SCRIPT(currentLine, "if(<=) goto %d (true)", labelIdx);
					ASSERT(scriptElement->stack[labelIdx] != -1);
					currentPosition = scriptElement->stack[labelIdx];
				} else {
					DEBUG_SCRIPT(currentLine, "if(<=) goto %d (false)", labelIdx);
				}

				break;
			}
		case 0x23:	// OP_gotoIfEqu
			{
				uint8 labelIdx;

				labelIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				if (scriptElement->compareResult & 1) {
					DEBUG_SCRIPT(currentLine, "if(==) goto %d (true)", labelIdx);
					ASSERT(scriptElement->stack[labelIdx] != -1);
					currentPosition = scriptElement->stack[labelIdx];
				} else {
					DEBUG_SCRIPT(currentLine, "if(==) goto %d (false)", labelIdx);
				}

				break;
			}
		case 0x24:	// OP_gotoIfDiff
			{
				uint8 labelIdx;

				labelIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				if (!(scriptElement->compareResult & 1)) {
					DEBUG_SCRIPT(currentLine, "if(!=) goto %d (true)", labelIdx);
					ASSERT(scriptElement->stack[labelIdx] != -1);
					currentPosition = scriptElement->stack[labelIdx];
				} else {
					DEBUG_SCRIPT(currentLine, "if(!=) goto %d (false)", labelIdx);
				}

				break;
			}
		case 0x26:	// loop
			{
				uint8 varIdx;
				uint8 labelIdx;

				varIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				labelIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				scriptElement->localVars[varIdx]--;

				if (scriptElement->localVars[varIdx] >= 0) {
					DEBUG_SCRIPT(currentLine, "loop(var[%]) goto %d (continue)", varIdx, labelIdx);
					ASSERT(scriptElement->stack[labelIdx] != -1);
					currentPosition = scriptElement->stack[labelIdx];
				} else {
					DEBUG_SCRIPT(currentLine, "loop(var[%]) goto %d (stop)", varIdx, labelIdx);
				}

				break;
			}
		case 0x31:	// OP_startScript
			{
				uint8 param;

				param = *(currentScriptPtr + currentPosition);
				currentPosition++;

				ASSERT(param < NUM_MAX_SCRIPT);

				DEBUG_SCRIPT(currentLine, "startScript(%d)", param);

				addScriptToList0(param);
				break;
			}
		case 0x32:
			{
				uint8 scriptIdx;

				scriptIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				DEBUG_SCRIPT(currentLine, "stopGlobalScript(%d)", scriptIdx);

				stopGlobalScript(scriptIdx);
				break;
			}
		case 0x3B:	// OP_loadResource
			{
				DEBUG_SCRIPT(currentLine, "loadResource(\"%s\")", currentScriptPtr + currentPosition);

				loadResource((char *)(currentScriptPtr + currentPosition));

				currentPosition += strlen((char *)(currentScriptPtr + currentPosition));

				break;
			}
		case 0x3C:	// OP_loadBg
			{
				DEBUG_SCRIPT(currentLine, "loadBg(\"%s\")", currentScriptPtr + currentPosition);

				loadBg((char *)(currentScriptPtr + currentPosition));

				currentPosition += strlen((char *)(currentScriptPtr + currentPosition));

				closeEngine7();

				bgVar0 = 0;

				break;
			}
		case 0x3D:	// OP_loadCt
			{
				DEBUG_SCRIPT(currentLine, "loadCt(\"%s\")", currentScriptPtr + currentPosition);

				loadCt((char *)(currentScriptPtr + currentPosition));

				currentPosition += strlen((char *)(currentScriptPtr + currentPosition));

				break;
			}
		case 0x3F:	// OP_loadPart
			{
				DEBUG_SCRIPT(currentLine, "loadPart(\"%s\")", currentScriptPtr + currentPosition);

				if (gameType == Cine::GID_FW)
					loadPart((char *)(currentScriptPtr + currentPosition));

				currentPosition += strlen((char *)(currentScriptPtr + currentPosition));
				break;
			}
		case 0x40:
			{
				DEBUG_SCRIPT(currentLine, "closePart");

				closePart();

				break;
			}
		case 0x41:	// OP_loadData
			{
				uint8 param;

				param = *(currentScriptPtr + currentPosition);
				currentPosition++;

				ASSERT(param <= 3);

				switch (param) {
				case 0:
					{
						DEBUG_SCRIPT(currentLine, "loadPrc(\"%s\")", currentScriptPtr + currentPosition);
						strcpy(newPrcName, (char *)(currentScriptPtr + currentPosition));
						break;
					}
				case 1:
					{
						DEBUG_SCRIPT(currentLine, "loadRel(\"%s\")", currentScriptPtr + currentPosition);
						strcpy(newRelName, (char *)(currentScriptPtr + currentPosition));
						break;
					}
				case 2:
					{
						DEBUG_SCRIPT(currentLine, "loadObject(\"%s\")", currentScriptPtr + currentPosition);
						strcpy(newObjectName, (char *)(currentScriptPtr + currentPosition));
						break;
					}
				case 3:
					{
						DEBUG_SCRIPT(currentLine, "loadMsg(\"%s\")", currentScriptPtr + currentPosition);
						strcpy(newMsgName, (char *)(currentScriptPtr + currentPosition));
						break;
					}
				}

				currentPosition += strlen((char *)(currentScriptPtr + currentPosition)) + 1;
				break;
			}
		case 0x42:
			{
				DEBUG_SCRIPT(currentLine, "request data load");
				checkForPendingDataLoadSwitch = 1;
				break;
			}
		case 0x45:
			{
				DEBUG_SCRIPT(currentLine, "request fadein");
				// TODO: use real code

				memcpy(c_palette, tempPalette, sizeof(uint16) * 16);
				drawOverlays();
				flip();

				fadeRequired = 1;
				break;
			}
		case 0x46:
			{
				DEBUG_SCRIPT(currentLine, "request fadeout");
				//fadeToBlack();
				break;
			}
		case 0x47:
			{
				uint8 startColor;
				uint8 numColor;
				uint16 r;
				uint16 g;
				uint16 b;

				startColor = *(currentScriptPtr + currentPosition);
				currentPosition++;

				numColor = *(currentScriptPtr + currentPosition);
				currentPosition++;

				r = READ_BE_UINT16(currentScriptPtr + currentPosition);
				currentPosition += 2;

				g = READ_BE_UINT16(currentScriptPtr + currentPosition);
				currentPosition += 2;

				b = READ_BE_UINT16(currentScriptPtr + currentPosition);
				currentPosition += 2;

				DEBUG_SCRIPT(currentLine, "transformPaletteRange(from:%d,numIdx:%d,r:%d,g:%d,b:%d) -> unimplemented", startColor, numColor, r, g, b);

				transformPaletteRange(startColor, numColor, r, g, b);

				break;
			}
		case 0x4A:
			{
				uint8 a;
				uint8 b;
				uint8 c;

				a = *(currentScriptPtr + currentPosition);
				currentPosition++;

				b = *(currentScriptPtr + currentPosition);
				currentPosition++;

				c = *(currentScriptPtr + currentPosition);
				currentPosition++;

				DEBUG_SCRIPT(currentLine, "palRotate(%d,%d,%d)", a, b, c);

				palRotate(a, b, c);
				break;
			}
		case 0x4F:	// break;
			{
				DEBUG_SCRIPT(currentLine, "break");

				scriptElement->scriptPosition = currentPosition;
				closeScript = 1;
				break;
			}
		case 0x49:
			{
				defaultMenuBoxColor2 = *(currentScriptPtr + currentPosition);
				currentPosition++;

				DEBUG_SCRIPT(currentLine, "setDefaultMenuColor2(%d)", defaultMenuBoxColor2);

				break;
			}
		case 0x50:	// OP_endScript
			{
				DEBUG_SCRIPT(currentLine, "endScript");

				if (params == 0) {
					endScript0(scriptElement->scriptIdx);
				} else {
					endScript1(scriptElement->scriptIdx);
				}

				closeScript = 1;

				break;
			}
		case 0x51:
			{
				uint8 param1;
				uint16 param2;
				uint16 param3;
				uint16 param4;
				uint16 param5;

				param1 = *(currentScriptPtr + currentPosition);
				currentPosition++;

				param2 = READ_BE_UINT16(currentScriptPtr + currentPosition);
				currentPosition += 2;

				param3 = READ_BE_UINT16(currentScriptPtr + currentPosition);
				currentPosition += 2;

				param4 = READ_BE_UINT16(currentScriptPtr + currentPosition);
				currentPosition += 2;

				param5 = READ_BE_UINT16(currentScriptPtr + currentPosition);
				currentPosition += 2;

				DEBUG_SCRIPT(currentLine, "message(%d,%d,%d,%d,%d)", param1, param2, param3, param4, param5);

				addMessage(param1, param2, param3, param4, param5);

				break;
			}
		case 0x52:	// OP_loadGlobalVar
			{
				uint8 idx;
				uint8 type;

				idx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				type = *(currentScriptPtr + currentPosition);
				currentPosition++;

				if (type) {
					uint8 idx2;

					idx2 = *(currentScriptPtr + currentPosition);
					currentPosition++;

					if (type == 1) {
						DEBUG_SCRIPT(currentLine, "globalVars[%d] = var[%d]", idx, idx2);

						globalVars[idx] = scriptElement->localVars[idx2];
					} else {
						DEBUG_SCRIPT(currentLine, "globalVars[%d] = globalVars[%d]", idx, idx2);

						globalVars[idx] = globalVars[idx2];
					}
				} else {
					uint16 newData;

					newData = READ_BE_UINT16(currentScriptPtr + currentPosition);
					currentPosition += 2;

					DEBUG_SCRIPT(currentLine, "globalVars[%d] = %d", idx, newData);

					globalVars[idx] = newData;
				}

				break;
			}
		case 0x53:	// OP_compareGlobalVar
			{
				uint8 idx;
				uint8 type;

				idx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				type = *(currentScriptPtr + currentPosition);
				currentPosition++;

				if (type) {
					uint8 b;

					b = *(currentScriptPtr + currentPosition);
					currentPosition++;

					DEBUG_SCRIPT(currentLine, "compare globalVars[%d] and var[%d]", idx, b);

					scriptElement->compareResult = compareVars(globalVars[idx], scriptElement->localVars[b]);
				} else {
					uint16 newData;

					newData = READ_BE_UINT16(currentScriptPtr + currentPosition);
					currentPosition += 2;

					DEBUG_SCRIPT(currentLine,
					    "compare globalVars[%d] and %d",
					    idx, newData);

					if (idx == 255 && (gameType == Cine::GID_FW)) {	// TODO: fix
						scriptElement->compareResult = 1;
					} else {
						scriptElement->compareResult = compareVars(globalVars[idx], newData);
					}
				}

				break;
			}
		case 0x59:
			{
				DEBUG_SCRIPT(currentLine, "comment(%s)", currentScriptPtr + currentPosition);

				currentPosition += strlen((char *)(currentScriptPtr + currentPosition));
				break;
			}
		case 0x5A:
			{
				uint8 startIdx;
				uint8 numIdx;

				startIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				numIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				ASSERT(startIdx + numIdx <= NUM_MAX_PARTDATA);

				DEBUG_SCRIPT(currentLine, "freePartRange(%d,%d)", startIdx, numIdx);

				freePartRange(startIdx, numIdx);

				break;
			}
		case 0x5B:
			{
				DEBUG_SCRIPT(currentLine, "unloadAllMasks()");

				unloadAllMasks();

				break;
			}
		case 0x65:
			{
				uint8 i;

				DEBUG_SCRIPT(currentLine, "initializeZoneData()");

				for (i = 0; i < NUM_MAX_ZONE; i++) {
					zoneData[i] = i;
				}

				break;
			}
		case 0x66:
			{
				uint8 zoneIdx;
				uint16 var;

				zoneIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				var = READ_BE_UINT16(currentScriptPtr + currentPosition);
				currentPosition += 2;

				DEBUG_SCRIPT(currentLine, "setZone[%d] = %d", zoneIdx, var);

				zoneData[zoneIdx] = var;

				break;
			}
		case 0x68:
			{
				defaultMenuBoxColor = *(currentScriptPtr + currentPosition);
				currentPosition++;

				DEBUG_SCRIPT(currentLine, "setDefaultMenuColor(%d)", defaultMenuBoxColor2);

				break;
			}
		case 0x69:	// OP_allowPlayerInput
			{
				DEBUG_SCRIPT(currentLine, "allowPlayerInput()");

				allowPlayerInput = 1;
				break;
			}
		case 0x6A:	// OP_dissallowPlayerInput
			{
				DEBUG_SCRIPT(currentLine, "dissallowPlayerInput()");

				allowPlayerInput = 0;
				break;
			}
		case 0x6B:	// OP_changeDataDisk
			{
				uint8 newDisk;

				newDisk = *(currentScriptPtr + currentPosition);
				currentPosition++;

				DEBUG_SCRIPT(currentLine, "changeDataDisk(%d)", newDisk);

				checkDataDisk(newDisk);
				break;
			}
		case 0x6D:
			{
				DEBUG_SCRIPT(currentLine, "loadMusic(%s)", currentScriptPtr + currentPosition);
				snd_loadSong((char *)(currentScriptPtr + currentPosition));

				currentPosition += strlen((char *)(currentScriptPtr + currentPosition)) + 1;
				break;
			}
		case 0x6E:
			{
				DEBUG_SCRIPT(currentLine, "playMusic()");
				snd_playSong();
				break;
			}
		case 0x6F:
			{
				DEBUG_SCRIPT(currentLine, "fadeOutMusic()");
				snd_fadeOutSong();
				break;
			}
		case 0x70:
			{
				DEBUG_SCRIPT(currentLine, "stopSample()");
				snd_stopSong();
				break;
			}
		case 0x77:
		case 0x78:
			{
				DEBUG_SCRIPT(currentLine, "playSample()");
				uint8 anim, channel;
				int16 volume;
				uint16 flag;

				anim = *(currentScriptPtr + currentPosition);
				currentPosition++;

				channel = *(currentScriptPtr + currentPosition);
				currentPosition++;

				/* unused */
				currentPosition += 2;

				/* unused */

				currentPosition++;

				volume = READ_BE_UINT16(currentScriptPtr + currentPosition);
				currentPosition += 2;

				flag = READ_BE_UINT16(currentScriptPtr + currentPosition);
				currentPosition += 2;

				if (volume > 63)
					volume = 63;
				if (volume < 0)
					volume = 63;

				if (animDataTable[anim].ptr1) {
					if (channel >= 10)
						channel -= 10;
					if (volume < 50)
						volume = 50;
					if (snd_songIsPlaying)
						snd_stopSong();
					if (flag == 0xFFFF)
						(*snd_driver.playSound)(animDataTable[anim].ptr1, channel, volume);
					else
						snd_resetChannel(channel);
				}
				break;
			}
		case 0x79:
			{
				var22 = *(currentScriptPtr + currentPosition);

				DEBUG_SCRIPT(currentLine, "OP79 load var22 to %d -> TODO", var22);

				currentPosition++;
				break;
			}
		case 0x7A:
			{
				uint8 param;

				param = *(currentScriptPtr + currentPosition);
				currentPosition++;

				DEBUG_SCRIPT(currentLine, "addOverlay5(%d)", param);

				loadOverlayElement(param, 5);

				break;
			}
		case 0x7B:
			{
				uint8 param;

				param = *(currentScriptPtr + currentPosition);
				currentPosition++;

				DEBUG_SCRIPT(currentLine, "freeOverlay5(%d)", param);

				freeOverlay(param, 5);

				break;
			}
		case 0x7F:
			{
				uint8 param1;
				uint8 param2;
				uint8 param3;
				uint8 param4;
				uint16 param5;
				uint16 param6;
				uint16 param7;

				param1 = *(currentScriptPtr + currentPosition);
				currentPosition++;

				param2 = *(currentScriptPtr + currentPosition);
				currentPosition++;

				param3 = *(currentScriptPtr + currentPosition);
				currentPosition++;

				param4 = *(currentScriptPtr + currentPosition);
				currentPosition++;

				param5 = READ_BE_UINT16(currentScriptPtr + currentPosition);
				currentPosition += 2;

				param6 = READ_BE_UINT16(currentScriptPtr + currentPosition);
				currentPosition += 2;

				param7 = READ_BE_UINT16(currentScriptPtr + currentPosition);
				currentPosition += 2;

				DEBUG_SCRIPT(currentLine, "addUnkListElement(%d,%d,%d,%d,%d)", param1, param2, param3, param4, param5, param6, param7);

				addUnkListElement(param1, 0, param2, param3, param4, param5, param6, 0, param7);

				break;
			}
		case 0x80:
			{
				uint8 a;
				uint8 b;

				a = *(currentScriptPtr + currentPosition);
				currentPosition++;

				b = *(currentScriptPtr + currentPosition);
				currentPosition++;

				DEBUG_SCRIPT(currentLine, "removeSeq(%d,%d) -> TODO", a, b);

				removeSeq(a, 0, b);

				break;
			}
		case 0x83:
			{
				uint8 a;
				uint8 b;

				a = *(currentScriptPtr + currentPosition);
				currentPosition++;

				b = *(currentScriptPtr + currentPosition);
				currentPosition++;

				DEBUG_SCRIPT(currentLine, "OP83(%d,%d) -> TODO", a, b);

				if (isSeqRunning(a, 0, b)) {
					scriptElement->compareResult = 1;
				} else {
					scriptElement->compareResult = 0;
				}
				break;
			}
		case 0x84:	// OP_gotoIfSup nearest
			{
				uint8 labelIdx;

				labelIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				if ((scriptElement->compareResult & 2)
				    && !(scriptElement->compareResult & 1)) {
					DEBUG_SCRIPT(currentLine, "if(>) goto nearest %d (true)", labelIdx);
					ASSERT(scriptElement->stack[labelIdx] != -1);
					currentPosition = computeScriptStackFromScript(scriptElement->scriptPtr, currentPosition, labelIdx,
									scriptTable[scriptElement->scriptIdx].size);
				} else {
					DEBUG_SCRIPT(currentLine, "if(>) goto nearest %d (false)", labelIdx);
				}

				break;
			}
		case 0x85:	// OP_gotoIfSupEqu nearest
			{
				uint8 labelIdx;

				labelIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				if (scriptElement->compareResult & 2 || scriptElement->compareResult & 1) {
					DEBUG_SCRIPT(currentLine, "if(>=) goto nearest %d (true)", labelIdx);
					ASSERT(scriptElement->stack[labelIdx] != -1);
					currentPosition = computeScriptStackFromScript(scriptElement->scriptPtr, currentPosition, labelIdx,
								    scriptTable[scriptElement->scriptIdx].size);
				} else {
					DEBUG_SCRIPT(currentLine, "if(>=) goto nearest %d (false)", labelIdx);
				}

				break;
			}
		case 0x86:	// OP_gotoIfInf nearest
			{
				uint8 labelIdx;

				labelIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				if ((scriptElement->compareResult & 4) && !(scriptElement->compareResult & 1)) {
					DEBUG_SCRIPT(currentLine, "if(<) goto nearest %d (true)", labelIdx);
					ASSERT(scriptElement->stack[labelIdx] != -1);
					currentPosition = computeScriptStackFromScript(scriptElement->scriptPtr,
									currentPosition, labelIdx, scriptTable[scriptElement->scriptIdx].size);
				} else {
					DEBUG_SCRIPT(currentLine, "if(<) goto nearest %d (false)", labelIdx);
				}

				break;
			}
		case 0x87:	// OP_gotoIfInfEqu nearest
			{
				uint8 labelIdx;

				labelIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				if ((scriptElement->compareResult & 4) || (scriptElement->compareResult & 1)) {
					DEBUG_SCRIPT(currentLine, "if(<=) goto nearest %d (true)", labelIdx);
					ASSERT(scriptElement->stack[labelIdx] != -1);
					currentPosition = computeScriptStackFromScript(scriptElement->scriptPtr,
								    currentPosition, labelIdx, scriptTable[scriptElement->scriptIdx].size);
				} else {
					DEBUG_SCRIPT(currentLine, "if(<=) goto nearest %d (false)", labelIdx);
				}

				break;
			}
		case 0x88:	// OP_gotoIfEqu nearest
			{
				uint8 labelIdx;

				labelIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				if (scriptElement->compareResult & 1) {
					DEBUG_SCRIPT(currentLine, "if(==) goto nearest %d (true)", labelIdx);
					ASSERT(scriptElement->stack[labelIdx] != -1);
					currentPosition = computeScriptStackFromScript(scriptElement->scriptPtr,
								    currentPosition, labelIdx, scriptTable[scriptElement->scriptIdx].size);
				} else {
					DEBUG_SCRIPT(currentLine, "if(==) goto nearest %d (false)", labelIdx);
				}

				break;
			}
		case 0x89:	// OP_gotoIfDiff nearest
			{
				uint8 labelIdx;

				labelIdx = *(currentScriptPtr + currentPosition);
				currentPosition++;

				if (!(scriptElement->compareResult & 1)) {
					DEBUG_SCRIPT(currentLine, "if(!=) goto nearest %d (true)", labelIdx);
					ASSERT(scriptElement->stack[labelIdx] != -1);
					currentPosition = computeScriptStackFromScript(scriptElement->scriptPtr,
								    currentPosition, labelIdx, scriptTable[scriptElement->scriptIdx].size);
				} else {
					DEBUG_SCRIPT(currentLine, "if(!=) goto nearest %d (false)", labelIdx);
				}

				break;
			}
		case 0x8B:
			{
				uint8 temp = *(currentScriptPtr + currentPosition);
				currentPosition++;

				DEBUG_SCRIPT(currentLine, "startObjectScript(%d,%d)", temp, currentScriptPtr + currentPosition);

				runObjectScript(temp);
				break;
			}
		case 0x8C:
			{
				uint8 temp = *(currentScriptPtr + currentPosition);
				currentPosition++;

				DEBUG_SCRIPT(currentLine, "startObjectScript(%d,%d)", temp, currentScriptPtr + currentPosition);

				stopObjectScript(temp);
				break;
			}
		case 0x8E:
			{
				uint8 temp = *(currentScriptPtr + currentPosition);
				currentPosition++;

				DEBUG_SCRIPT(currentLine, "addBackground(%s,%d)", currentScriptPtr + currentPosition, temp);

				addBackground((char *)(currentScriptPtr + currentPosition), temp);

				currentPosition += strlen((char *)(currentScriptPtr + currentPosition));
				break;
			}
		case 0x8F:
			{
				uint8 temp = *(currentScriptPtr + currentPosition);
				currentPosition++;

				DEBUG_SCRIPT(currentLine, "removeBackground(%d)", temp);

				ASSERT(temp);
				if (additionalBgTable[temp]) {
					free(additionalBgTable[temp]);
					additionalBgTable[temp] = NULL;
				}

				if (currentAdditionalBgIdx == temp) {
					currentAdditionalBgIdx = 0;
				}

				if (currentAdditionalBgIdx2 == temp) {
					currentAdditionalBgIdx = 0;
				}

				strcpy(currentBgName[temp], "");

				break;
			}
		case 0x90:
			{
				uint8 temp = *(currentScriptPtr + currentPosition);
				currentPosition++;

				DEBUG_SCRIPT(currentLine, "loadABS(%d,%s)", temp, currentScriptPtr + currentPosition);

				loadAbs((char *)(currentScriptPtr + currentPosition), temp);

				currentPosition += strlen((char *)(currentScriptPtr + currentPosition));

				break;
			}
		case 0x91:
			{
				uint8 temp = *(currentScriptPtr + currentPosition);
				currentPosition++;

				DEBUG_SCRIPT(currentLine, "useBg(%d)", temp);

				ASSERT(temp <= 8);

				if (additionalBgTable[temp]) {
					currentAdditionalBgIdx = temp;
					//if (adBgVar0 == 0) {
					//	adBgVar1 = 1;
					//}
				}
				break;
			}
		case 0x9D:
			{
				uint8 temp = *(currentScriptPtr + currentPosition);
				currentPosition++;

				DEBUG_SCRIPT(currentLine, "useBgScroll(%d)", temp);

				ASSERT(temp <= 8);

				if (additionalBgTable[temp]) {
					currentAdditionalBgIdx2 = temp;
				}
				break;
			}
		case 0x9E:
			{
				uint8 type;

				type = *(currentScriptPtr + currentPosition);
				currentPosition++;

				if (type) {
					uint8 param2;

					param2 = *(currentScriptPtr + currentPosition);
					currentPosition++;

					DEBUG_SCRIPT(currentLine, "additionalBgVScroll = var[%d]", param2);

					additionalBgVScroll = scriptElement->localVars[param2];
				} else {
					uint16 param2;

					param2 = READ_BE_UINT16(currentScriptPtr + currentPosition);
					currentPosition += 2;

					DEBUG_SCRIPT(currentLine, "additionalBgVScroll = %d", param2);

					additionalBgVScroll = param2;
				}

				break;
			}
		case 0xA0:
			{
				uint16 param1;
				uint16 param2;

				param1 = READ_BE_UINT16(currentScriptPtr + currentPosition);
				currentPosition += 2;

				param2 = READ_BE_UINT16(currentScriptPtr + currentPosition);
				currentPosition += 2;

				DEBUG_SCRIPT(currentLine, "addGfxElementA0(%d,%d)", param1, param2);

				addGfxElementA0(param1, param2);
				break;
			}
		case 0xA1:
			{
				currentPosition += 4;
				break;
			}
		case 0xA2:
			{
				currentPosition += 4;
				break;
			}
		case 0xA3:
			{
				currentPosition += 4;
				break;
			}
		default:
			{
				error("Unsupported opcode %X", opcode - 1);
			}
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

uint8 decompileBuffer[10000][1000];
uint16 decompileBufferPosition = 0;

uint8 bufferDec[256];

uint8 compareString1[256];
uint8 compareString2[256];

uint8 *getObjPramName(uint8 paramIdx) {
	switch (paramIdx) {
	case 1:
		{
			return (".X");
		}
	case 2:
		{
			return (".Y");
		}
	case 3:
		{
			return (".mask");
		}
	case 4:
		{
			return (".frame");
		}
	case 5:
		{
			return (".status");
		}
	case 6:
		{
			return (".costume");
		}
	default:
		{
			sprintf(bufferDec, ".param%d", paramIdx);
			return (bufferDec);
		}
	}
}

void decompileScript(uint8 *scriptPtr, int16 *stackPtr, uint16 scriptSize, uint16 scriptIdx) {
	uint8 lineBuffer[256];
	uint8 *localScriptPtr = scriptPtr;
	uint16 exitScript;
	uint32 position = 0;

	ASSERT_PTR(scriptPtr);
	// ASSERT_PTR(stackPtr);

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
				uint8 param1;
				uint8 param2;
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
				uint8 param1;
				uint8 param2;
				uint8 param3;

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
				uint8 param1;
				uint8 param2;
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
				uint8 param1;
				uint8 param2;
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
				uint8 param1;
				uint8 param2;
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
				uint8 param1;
				uint8 param2;
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
				uint8 param1;
				uint8 param2;
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
				uint8 param1;
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
				uint8 param1;
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
				uint8 param1;
				int16 param2;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				if (param2) {
					uint8 param3;

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
						ASSERT(0);
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
				uint8 param1;
				uint8 param2;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				if (param2) {
					uint8 param3;

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
				uint8 param1;
				uint8 param2;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				if (param2) {
					uint8 param3;

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
				uint8 param1;
				uint8 param2;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				if (param2) {
					uint8 param3;

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
				uint8 param1;
				uint8 param2;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				if (param2) {
					uint8 param3;

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
				uint8 param1;
				uint8 param2;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				if (param2) {
					uint8 param3;

					param3 = *(localScriptPtr + position);
					position++;

					if (param2 == 1) {
						sprintf(compareString1, "var[%d]", param1);
						sprintf(compareString2, "var[%d]", param3);

					} else if (param2 == 2) {
						sprintf(compareString1, "var[%d]", param1);
						sprintf(compareString2, "globalVar[%d]", param3);
					} else {
						ASSERT(0);
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
				uint8 param1;
				uint8 param2;
				uint8 param3;

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
				uint8 param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "loadMask0(%d)\n", param);

				break;
			}
		case 0x14:
			{
				uint8 param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "unloadMask0(%d)\n", param);

				break;
			}
		case 0x15:
			{
				uint8 param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "OP_15(%d)\n", param);

				break;
			}
		case 0x16:
			{
				uint8 param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "loadMask1(%d)\n", param);

				break;
			}
		case 0x17:
			{
				uint8 param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "unloadMask0(%d)\n", param);

				break;
			}
		case 0x18:
			{
				uint8 param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "loadMask4(%d)\n", param);

				break;
			}
		case 0x19:
			{
				uint8 param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "unloadMask4(%d)\n", param);

				break;
			}
		case 0x1A:
			{
				uint8 param;

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
				uint8 param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "label(%d)\n", param);

				break;
			}
		case 0x1E:
			{
				uint8 param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "goto(%d)\n", param);

				break;
			}
		case 0x1F:
			{
				uint8 param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "if(%s>%s) goto(%d)\n", compareString1, compareString2, param);

				break;
			}
		case 0x20:
			{
				uint8 param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "if(%s>=%s) goto(%d)\n", compareString1, compareString2, param);

				break;
			}
		case 0x21:
			{
				uint8 param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "if(%s<%s) goto(%d)\n", compareString1, compareString2, param);

				break;
			}
		case 0x22:
			{
				uint8 param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "if(%s<=%s) goto(%d)\n", compareString1, compareString2, param);

				break;
			}
		case 0x23:
			{
				uint8 param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "if(%s==%s) goto(%d)\n", compareString1, compareString2, param);

				break;
			}
		case 0x24:
			{
				uint8 param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "if(%s!=%s) goto(%d)\n", compareString1, compareString2, param);

				break;
			}
		case 0x25:
			{
				uint8 param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "removeLabel(%d)\n", param);

				break;
			}
		case 0x26:
			{
				uint8 param1;
				uint8 param2;

				param1 = *(localScriptPtr + position);
				position++;
				param2 = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "loop(--var[%d]) -> label(%d)\n", param1, param2);

				break;
			}
		case 0x31:
			{
				uint8 param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "startGlobalScript(%d)\n", param);

				break;
			}
		case 0x32:
			{
				uint8 param;

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
				uint8 param;

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
				uint8 param1;
				uint8 param2;
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
				uint8 param;

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
				uint8 param1;
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
				uint8 param1;
				uint8 param2;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				if (param2) {
					uint8 param3;

					param3 = *(localScriptPtr + position);
					position++;

					if (param2 == 1) {
						sprintf(lineBuffer, "globalVar[%d] = var[%d]\n", param1, param3);
					} else if (param2 == 2) {
						sprintf(lineBuffer, "globalVar[%d] = globalVar[%d]\n", param1, param3);
					} else {
						ASSERT(0);
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
				uint8 param1;
				uint8 param2;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				if (param2) {
					uint8 param3;

					param3 = *(localScriptPtr + position);
					position++;

					if (param2 == 1) {
						sprintf(compareString1, "globalVar[%d]", param1);
						sprintf(compareString2, "var[%d]", param3);
					} else if (param2 == 2) {
						sprintf(compareString1, "globalVar[%d]", param1);
						sprintf(compareString2, "globalVar[%d]", param3);
					} else {
						ASSERT(0);
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
				uint8 param1;
				uint8 param2;

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
				uint8 param1;
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
				uint8 param;

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
				uint8 newDisk;

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
				uint8 param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "allowSystemMenu(%d)\n", param);

				break;
			}
		case 0x77:
			{
				uint8 param1;
				uint8 param2;
				int16 param3;
				uint8 param4;
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
				uint8 param1;
				uint8 param2;
				int16 param3;
				uint8 param4;
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
				uint8 param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "OP_7A(%d)\n", param);

				break;
			}
		case 0x7B:	// OS only
			{
				uint8 param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "OP_7B(%d)\n", param);

				break;
			}
		case 0x7F:	// OS only
			{
				uint8 param1;
				uint8 param2;
				uint8 param3;
				uint8 param4;
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
				uint8 param1;
				uint8 param2;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "OP_80(%d,%d)\n", param1, param2);

				break;
			}
		case 0x82:	// OS only
			{
				uint8 param1;
				uint8 param2;
				uint16 param3;
				uint16 param4;
				uint8 param5;

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
				uint8 param1;
				uint8 param2;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "OP_83(%d,%d)\n", param1, param2);

				break;
			}
		case 0x89:	// OS only
			{
				uint8 param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "if(%s!=%s) goto next label(%d)\n", compareString1, compareString2, param);

				break;
			}
		case 0x8B:	// OS only
			{
				uint8 param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "OP_8B(%d)\n", param);

				break;
			}
		case 0x8C:	// OS only
			{
				uint8 param;

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
				uint8 param1;

				param1 = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "ADDBG(%d,%s)\n", param1, localScriptPtr + position);

				position += strlen(localScriptPtr + position);

				break;
			}
		case 0x8F:	// OS only
			{
				uint8 param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "OP_8F(%d)\n", param);

				break;
			}
		case 0x90:	// OS only
			{
				uint8 param1;

				param1 = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "loadABS(%d,%s)\n", param1, localScriptPtr + position);

				position += strlen(localScriptPtr + position);

				break;
			}
		case 0x91:	// OS only
			{
				uint8 param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "OP_91(%d)\n", param);

				break;
			}
		case 0x9D:	// OS only
			{
				uint8 param;

				param = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "OP_9D(%d) -> flip img idx\n", param);

				break;
			}
		case 0x9E:	// OS only
			{
				uint8 param;

				param = *(localScriptPtr + position);
				position++;

				if (param) {
					uint8 param2;

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
				uint8 param1;
				uint8 param2;

				param1 = *(localScriptPtr + position);
				position++;

				param2 = *(localScriptPtr + position);
				position++;

				sprintf(lineBuffer, "OP_A0(%d,%d)\n", param1, param2);

				break;
			}
		case 0xA1:	// OS only
			{
				uint8 param1;
				uint8 param2;

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

void dumpScript(uint8 *dumpName) {
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
