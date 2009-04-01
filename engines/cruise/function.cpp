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

#include "cruise/cruise.h"
#include "cruise/cruise_main.h"
#include "cruise/cell.h"
#include "cruise/sound.h"
#include "cruise/staticres.h"
#include "common/util.h"

namespace Cruise {

//#define FUNCTION_DEBUG

int16 Op_LoadOverlay(void) {
	char *pOverlayName;
	char overlayName[38] = "";
	int overlayLoadResult;

	pOverlayName = (char *)popPtr();

	if (strlen(pOverlayName) == 0)
		return 0;

	strcpy(overlayName, pOverlayName);
	strToUpper(overlayName);

	//gfxModuleData.field_84();
	//gfxModuleData.field_84();

	overlayLoadResult = loadOverlay(overlayName);

	updateAllScriptsImports();

	strcpy(nextOverlay, overlayName);

	return(overlayLoadResult);
}

int16 Op_Strcpy(void) {
	char *ptr1 = (char *)popPtr();
	char *ptr2 = (char *)popPtr();

	//printf("strcpy %s\n",ptr1);

	while (*ptr1) {
		*ptr2 = *ptr1;

		ptr2++;
		ptr1++;
	}

	*ptr2 = 0;

	return (0);
}

int16 Op_Exec(void) {
	int scriptIdx;
	int ovlIdx;
	uint8 *ptr;
	uint8 *ptr2;

	short int popTable[256];	// TODO: check original size;

	int numOfArgToPop = popVar();

	int i = 0;

	for (i = 0; i < numOfArgToPop; i++) {
		popTable[numOfArgToPop - i - 1] = popVar();
	}

	scriptIdx = popVar();
	ovlIdx = popVar();

	if (!ovlIdx) {
		ovlIdx = currentScriptPtr->overlayNumber;
	}

	ptr = attacheNewScriptToTail(&procHead, ovlIdx, scriptIdx, currentScriptPtr->type, currentScriptPtr->scriptNumber, currentScriptPtr->overlayNumber, scriptType_MinusPROC);

	if (!ptr)
		return (0);

	if (numOfArgToPop <= 0) {
		return (0);
	}

	ptr2 = ptr;

	for (i = 0; i < numOfArgToPop; i++) {
		saveShort(ptr2, popTable[i]);
		ptr2 += 2;
	}

	return (0);
}

int16 Op_AddProc(void) {
	int pop1 = popVar();
	int pop2;
	int overlay;
	int param[160];

	for (long int i = 0; i < pop1; i++) {
		param[i] = popVar();
	}

	pop2 = popVar();
	overlay = popVar();

	if (!overlay)
		overlay = currentScriptPtr->overlayNumber;

	if (!overlay)
		return (0);

	uint8* procBss = attacheNewScriptToTail(&procHead, overlay, pop2, currentScriptPtr->type, currentScriptPtr->scriptNumber, currentScriptPtr->overlayNumber, scriptType_PROC);

	if (procBss) {
		for (long int i = 0; i < pop1; i++) {
			int16* ptr = (int16*)(procBss + i * 2);
			*ptr = param[i];
			flipShort(ptr);
		}
	}

	return (0);
}

int16 Op_Narrator(void) {
	int pop1 = popVar();
	int pop2 = popVar();

	if (!pop2)
		pop2 = currentScriptPtr->overlayNumber;

	narratorOvl = pop2;
	narratorIdx = pop1;

	return (0);
}

int16 Op_GetMouseX(void) {	// TODO: implement properly
	int16 dummy;
	int16 mouseX;
	int16 mouseY;
	int16 mouseButton;

	getMouseStatus(&dummy, &mouseX, &mouseButton, &mouseY);

	return (mouseX);
}

int16 Op_GetMouseY(void) {	// TODO: implement properly
	int16 dummy;
	int16 mouseX;
	int16 mouseY;
	int16 mouseButton;

	getMouseStatus(&dummy, &mouseX, &mouseButton, &mouseY);

	return (mouseY);
}

int16 Op_Random(void) {		// TODO: implement
	int var = popVar();

	if (var < 2) {
		return (0);
	}

	return (_vm->_rnd.getRandomNumber(var - 1));
}

int16 Op_PlayFX(void) {		// TODO: implement
	popVar();
	popVar();
	popVar();
	popVar();

	// printf("Op_PlayFX, implement (sound related)\n");

	return (0);
}

int16 Op_FreeCT(void) {
	freeCTP();
	return (0);
}

void freeObjectList(cellStruct *pListHead) {
	int var_2 = 0;
	cellStruct *pCurrent = pListHead->next;

	while (pCurrent) {
		cellStruct *pNext = pCurrent->next;

		if (pCurrent->freeze == 0) {
			free(pCurrent->gfxPtr);
			free(pCurrent);
		}

		var_2 = 1;

		pCurrent = pNext;
	}

	if (var_2) {
		resetPtr(pListHead);
	}
}

int16 Op_FreeCell(void) {
	freeObjectList(&cellHead);
	return (0);
}

int16 Op_freeBackgroundInscrustList(void) {
	freeBackgroundIncrustList(&backgroundIncrustHead);
	return (0);
}


int16 Op_UnmergeBackgroundIncrust(void) {
	int obj = popVar();
	int ovl = popVar();

	if (!ovl) {
		ovl = currentScriptPtr->overlayNumber;
	}

	unmergeBackgroundIncrust(&backgroundIncrustHead, ovl, obj);

	return (0);
}

int16 Op_FreePreload(void) {
	// TODO: implement
	printf("Op_FreePreload, implement\n");
	return (0);
}

int16 Op_RemoveMessage(void) {
	int idx;
	int overlay;

	idx = popVar();
	overlay = popVar();

	if (!overlay) {
		overlay = currentScriptPtr->overlayNumber;
	}

	removeCell(&cellHead, overlay, idx, 5, masterScreen);

	return (0);
}

int16 Op_FindSet(void) {
	int16 i;
	char name[36] = "";
	char *ptr;

	ptr = (char *) popPtr();

	if (!ptr) {
		return -1;
	}

	strcpy(name, ptr);
	strToUpper(name);

	for (i = 0; i < 257; i++) {
		if (!strcmp(name, filesDatabase[i].subData.name)) {
			return (i);
		}
	}

	return -1;
}

int16 Op_RemoveFrame(void) {
	int var1 = popVar();
	int var2 = popVar();

	resetFileEntryRange(var2, var1);

	return (0);
}

int16 Op_comment(void) {
	char *var;

	var = (char *)popPtr();

	printf("COMMENT: \"%s\"\n", var);

	return (0);
}

int16 Op_RemoveProc(void) {
	int idx;
	int overlay;

	idx = popVar();
	overlay = popVar();

	if (!overlay) {
		overlay = currentScriptPtr->overlayNumber;
	}

	removeScript(overlay, idx, &procHead);

	return (0);
}

int16 Op_FreeOverlay(void) {
	char localName[36] = "";
	char *namePtr;

	namePtr = (char *) popPtr();

	strcpy(localName, namePtr);

	if (localName[0]) {
		strToUpper(localName);
		releaseOverlay((char *)localName);
	}

	return 0;
}

int16 Op_FindProc(void) {
	char name[36] = "";
	char *ptr;
	int param;

	ptr = (char *)popPtr();

	strcpy(name, ptr);

	param = getProcParam(popVar(), 20, name);

	return param;
}

int16 Op_KillMenu(void) {
	// TODO: implement
	printf("Op_KillMenu, implement\n");

	return 0;
}

int16 Op_UserMenu(void) {
	int oldValue = entrerMenuJoueur;
	entrerMenuJoueur = popVar();

	return oldValue;
}

int16 Op_UserOn(void) {
	int oldValue = userEnabled;
	int newValue = popVar();

	if (newValue != -1) {
		userEnabled = newValue;
	}

	return oldValue;
}

int16 Op_Display(void) {
	int oldValue = displayOn;
	int newValue = popVar();

	if (newValue != -1) {
		displayOn = newValue;
	}

	return oldValue;
}

int16 Op_FreezeParent(void) {
	if (currentScriptPtr->var1A == 20) {
		changeScriptParamInList(currentScriptPtr->var18, currentScriptPtr->var16, &procHead, -1, 9997);
	} else if (currentScriptPtr->var1A == 30) {
		changeScriptParamInList(currentScriptPtr->var18, currentScriptPtr->var16, &relHead, -1, 9997);
	}

	return 0;
}

int16 Op_LoadBackground(void) {
	int result = 0;
	char bgName[36] = "";
	char *ptr;
	int bgIdx;

	ptr = (char *) popPtr();

	strcpy(bgName, ptr);

	bgIdx = popVar();

	if (bgIdx >= 0 || bgIdx < 8) {
		strToUpper(bgName);

		gfxModuleData_gfxWaitVSync();
		gfxModuleData_gfxWaitVSync();

		result = loadBackground(bgName, bgIdx);
	}

	changeCursor(CURSOR_NORMAL);

	return result;
}

int16 Op_FrameExist(void) {
	int param;

	param = popVar();

	if (param < 0 || param > 255) {
		return 0;
	}

	if (filesDatabase[param].subData.ptr) {
		return 1;
	}

	return 0;
}

int16 Op_LoadFrame(void) {
	int param1;
	int param2;
	int param3;
	char name[36] = "";
	char *ptr;

	ptr = (char *) popPtr();

	strcpy(name, ptr);

	param1 = popVar();
	param2 = popVar();
	param3 = popVar();

	if (param3 >= 0 || param3 < 257) {
		strToUpper(name);

		gfxModuleData_gfxWaitVSync();
		gfxModuleData_gfxWaitVSync();

		lastAni[0] = 0;

		loadFileRange(name, param2, param3, param1);

		lastAni[0] = 0;
	}

	changeCursor(CURSOR_NORMAL);
	return 0;
}

int16 Op_LoadAbs(void) {
	int param1;
//  int param2;
//  int param3;
	char name[36] = "";
	char *ptr;
	int result = 0;

	ptr = (char *) popPtr();

	strcpy(name, ptr);

	param1 = popVar();

	if (param1 >= 0 || param1 < 257) {
		strToUpper(name);

		gfxModuleData_gfxWaitVSync();
		gfxModuleData_gfxWaitVSync();

		result = loadFullBundle(name, param1);
	}

	changeCursor(CURSOR_NORMAL);
	return result;
}

int16 Op_InitializeState(void) {
	int param1 = popVar();
	int objIdx = popVar();
	int ovlIdx = popVar();

	if (!ovlIdx)
		ovlIdx = currentScriptPtr->overlayNumber;

#ifdef FUNCTION_DEBUG
	printf("Init %s state to %d\n", getObjectName(objIdx, overlayTable[ovlIdx].ovlData->arrayNameObj), param1);
#endif

	objInit(ovlIdx, objIdx, param1);

	return (0);
}

int16 Op_GetlowMemory(void) {
	return lowMemory;
}

int16 Op_FadeOut(void) {
	for (long int i = 0; i < 256; i += 32) {
		for (long int j = 0; j < 256; j++) {
			int offsetTable[3];
			offsetTable[0] = -32;
			offsetTable[1] = -32;
			offsetTable[2] = -32;
			calcRGB(&workpal[3*j], &workpal[3*j], offsetTable);
		}
		gfxModuleData_setPal256(workpal);
		gfxModuleData_flipScreen();
	}

	memset(globalScreen, 0, 320 * 200);
	flip();

	fadeFlag = 1;
	PCFadeFlag = 1;

	return 0;
}

int16 isOverlayLoaded(const char * name) {
	int16 i;

	for (i = 1; i < numOfLoadedOverlay; i++) {
		if (!strcmp(overlayTable[i].overlayName, name) && overlayTable[i].alreadyLoaded) {
			return i;
		}
	}

	return 0;
}

int16 Op_FindOverlay(void) {
	char name[36] = "";
	char *ptr;

	ptr = (char *) popPtr();

	strcpy(name, ptr);
	strToUpper(name);

	return (isOverlayLoaded(name));
}

int16 Op_WriteObject(void) {
	int16 returnParam;

	int16 param1 = popVar();
	int16 param2 = popVar();
	int16 param3 = popVar();
	int16 param4 = popVar();

	getSingleObjectParam(param4, param3, param2, &returnParam);
	setObjectPosition(param4, param3, param2, param1);

	return returnParam;
}

int16 Op_ReadObject(void) {
	int16 returnParam;

	int member = popVar();
	int obj = popVar();
	int ovl = popVar();

	getSingleObjectParam(ovl, obj, member, &returnParam);

	return returnParam;
}

int16 Op_FadeIn(void) {
	doFade = 1;
	return 0;
}

int16 Op_GetMouseButton(void) {
	int16 dummy;
	int16 mouseX;
	int16 mouseY;
	int16 mouseButton;

	getMouseStatus(&dummy, &mouseX, &mouseButton, &mouseY);

	if (mouseButton)
		return 1;
	return 0;
}

int16 Op_AddCell(void) {
	int16 objType = popVar();
	int16 objIdx = popVar();
	int16 overlayIdx = popVar();

	if (!overlayIdx)
		overlayIdx = currentScriptPtr->overlayNumber;

	addCell(&cellHead, overlayIdx, objIdx, objType, masterScreen, currentScriptPtr->overlayNumber, currentScriptPtr->scriptNumber, currentScriptPtr->type);

	return 0;
}

int16 Op_AddBackgroundIncrust(void) {

	int16 objType = popVar();
	int16 objIdx = popVar();
	int16 overlayIdx = popVar();

	if (!overlayIdx)
		overlayIdx = currentScriptPtr->overlayNumber;

	addBackgroundIncrust(overlayIdx, objIdx, &backgroundIncrustHead, currentScriptPtr->scriptNumber, currentScriptPtr->overlayNumber, masterScreen, objType);

	return 0;
}

int16 Op_RemoveCell(void) {
	int objType = popVar();
	int objectIdx = popVar();
	int ovlNumber = popVar();

	if (!ovlNumber) {
		ovlNumber = currentScriptPtr->overlayNumber;
	}

	removeCell(&cellHead, ovlNumber, objectIdx, objType, masterScreen);

	return 0;
}

int16 fontFileIndex = -1;

int16 Op_SetFont(void) {
	fontFileIndex = popVar();

	return 0;
}

int16 Op_UnfreezeParent(void) {
	if (currentScriptPtr->var1A == 0x14) {
		changeScriptParamInList(currentScriptPtr->var18, currentScriptPtr->var16, &procHead, -1, 0);
	} else if (currentScriptPtr->var1A == 0x1E) {
		changeScriptParamInList(currentScriptPtr->var18, currentScriptPtr->var16, &relHead, -1, 0);
	}

	return 0;
}

int16 protectionCode = 0;

int16 Op_ProtectionFlag(void) {
	int16 temp = protectionCode;
	int16 newVar;

	newVar = popVar();
	if (newVar != -1) {
		protectionCode = newVar;
	}
	return temp;
}

int16 Op_AddMessage(void) {
	int16 color = popVar();
	int16 var_2 = popVar();
	int16 var_4 = popVar();
	int16 var_6 = popVar();
	int16 var_8 = popVar();
	int16 overlayIdx = popVar();

	if (!overlayIdx)
		overlayIdx = currentScriptPtr->overlayNumber;

	if (color == -1) {
		color = findHighColor();
	} else {
		if (CVTLoaded) {
			color = cvtPalette[color];
		}
	}

	createTextObject(&cellHead, overlayIdx, var_8, var_6, var_4, var_2, color, masterScreen, currentScriptPtr->overlayNumber, currentScriptPtr->scriptNumber);

	return 0;
}

int16 Op_Preload(void) {
	popPtr();
	popVar();

	return 0;
}

int16 Op_LoadCt(void) {
	return initCt((char*)popPtr());
}

int16 Op_EndAnim(void) {
	int param1 = popVar();
	int param2 = popVar();
	int overlay = popVar();

	if (!overlay)
		overlay = currentScriptPtr->overlayNumber;

	return isAnimFinished(overlay, param2, &actorHead, param1);
}

int16 Op_Protect(void) {
	popPtr();
	popVar();

	return 0;
}

int16 Op_AutoCell(void) {
	cellStruct *pObject;

	int signal = popVar();
	int loop = popVar();
	int wait = popVar();
	int animStep = popVar();
	int end = popVar();
	int start = popVar();
	int type = popVar();
	int change = popVar();
	int obj = popVar();
	int overlay = popVar();

	if (!overlay)
		overlay = currentScriptPtr->overlayNumber;

	pObject = addCell(&cellHead, overlay, obj, 4, masterScreen, currentScriptPtr->overlayNumber, currentScriptPtr->scriptNumber, currentScriptPtr->type);

	if (!pObject)
		return 0;

	pObject->animSignal = signal;
	pObject->animLoop = loop;
	pObject->animWait = wait;
	pObject->animStep = animStep;
	pObject->animEnd = end;
	pObject->animStart = start;
	pObject->animType = type;
	pObject->animChange = change;

	if (type) {
		if (currentScriptPtr->type == scriptType_PROC) {
			changeScriptParamInList(currentScriptPtr->overlayNumber, currentScriptPtr->scriptNumber, &procHead, -1, 9996);
		} else if (currentScriptPtr->type == scriptType_REL) {
			changeScriptParamInList(currentScriptPtr->overlayNumber, currentScriptPtr->scriptNumber, &relHead, -1, 9996);
		}
	}

	if (change == 5) {
		objInit(pObject->overlay, pObject->idx, start);
	} else {
		setObjectPosition(pObject->overlay, pObject->idx, pObject->animChange, start);
	}

	if (wait < 0) {
		objectParamsQuery params;

		getMultipleObjectParam(overlay, obj, &params);
		pObject->animCounter = params.state2 - 1;
	}

	return 0;
}

int16 Op_Sizeof(void) {
	objectParamsQuery params;
	int index = popVar();
	int overlay = popVar();

	if (!overlay)
		overlay = currentScriptPtr->overlayNumber;

	getMultipleObjectParam(overlay, index, &params);

	return params.nbState - 1;
}

int16 Op_SetActiveBackground(void) {
	int currentPlane = masterScreen;
	int newPlane = popVar();

	if (newPlane >= 0 && newPlane < 8) {
		if (backgroundScreens[newPlane]) {
			masterScreen = newPlane;
			switchPal = 1;
		}
	}

	return currentPlane;
}

int16 Op_RemoveBackground(void) {
	int backgroundIdx = popVar();

	if (backgroundIdx > 0 && backgroundIdx < 8) {
		if (backgroundScreens[backgroundIdx])
			free(backgroundScreens[backgroundIdx]);

		if (masterScreen == backgroundIdx)
			masterScreen = 0;

		strcpy(backgroundTable[backgroundIdx].name, "");
	} else {
		strcpy(backgroundTable[0].name, "");
	}

	return (0);
}

int vblLimit;

int16 Op_VBL(void) {
	vblLimit = popVar();
	return 0;
}

int op7BVar = 0;

int16 Op_Sec(void) {
	int di = popVar();
	int si = 1 - op7BVar;
	int sign;

	if (di) {
		sign = di / (ABS(di));
	} else {
		sign = 0;
	}

	op7BVar = -sign;

	return si;
}

int16 Op_RemoveBackgroundIncrust(void) {
	int idx = popVar();
	int overlay = popVar();

	if (!overlay) {
		overlay = currentScriptPtr->overlayNumber;
	}

	removeBackgroundIncrust(overlay, idx, &backgroundIncrustHead);

	return 0;
}

int16 Op_SetColor(void)	{
	int colorB = popVar();
	int colorG = popVar();
	int colorR = popVar();
	int endIdx = popVar();
	int startIdx = popVar();

	int i;

#define convertRatio 36.571428571428571428571428571429

	for (i = startIdx; i <= endIdx; i++) {
		int offsetTable[3];

		offsetTable[0] = (int)(colorR * convertRatio);
		offsetTable[1] = (int)(colorG * convertRatio);
		offsetTable[2] = (int)(colorB * convertRatio);

		if (CVTLoaded) {
			int colorIdx = cvtPalette[i];
			calcRGB(&palScreen[masterScreen][3*colorIdx], &workpal[3*colorIdx], offsetTable);
		} else {
			calcRGB(&palScreen[masterScreen][3*i], &workpal[3*i], offsetTable);
		}
	}

	gfxModuleData_setPal256(workpal);

	return 0;
}

int16 Op_Inventory(void) {
	int si = var41;

	var41 = popVar();

	return si;
}

int16 Op_RemoveOverlay(void) {
	int overlayIdx;

	overlayIdx = popVar();

	if (strlen(overlayTable[overlayIdx].overlayName)) {
		releaseOverlay(overlayTable[overlayIdx].overlayName);
	}

	return 0;
}

int16 Op_ComputeLine(void) {
	int y2 = popVar();
	int x2 = popVar();
	int y1 = popVar();
	int x1 = popVar();

	point* pDest = (point*)popPtr();

	int maxValue = cor_droite(x1, y1, x2, y2, pDest);

	flipGen(pDest, maxValue * 4);

	return maxValue;
}

int16 Op_FindMsg(void) {
	int si = popVar();
	popVar();

	return si;
}

int16 Op_SetZoom(void) {
	var46 = popVar();
	var45 = popVar();
	var42 = popVar();
	var39 = popVar();
	return 0;
}

int16 computeZoom(int param) {
	return (((param - var46) * (var39 - var42)) / (var45 - var46)) + var42;
}

int16 subOp23(int param1, int param2) {
	return (param1 * param2) >> 8;
}

int16 Op_GetStep(void) {
	int si = popVar();
	int dx = popVar();

	return subOp23(dx, si);
}

int16 Op_GetZoom(void) {
	return (computeZoom(popVar()));
}

actorStruct *addAnimation(actorStruct * pHead, int overlay, int objIdx, int param, int param2) {
	actorStruct *pPrevious = pHead;
	actorStruct *pCurrent = pHead->next;

	// go to the end of the list
	while (pCurrent) {
		pPrevious = pCurrent;
		pCurrent = pPrevious->next;
	}

	if (pCurrent && (pCurrent->overlayNumber == overlay)
	        && (pCurrent->idx == objIdx) && (pCurrent->type == param2)) {
		return NULL;
	}

	actorStruct *pNewElement = (actorStruct *) malloc(sizeof(actorStruct));
	if (!pNewElement)
		return NULL;

	memset(pNewElement, 0, sizeof(actorStruct));
	pNewElement->next = pPrevious->next;
	pPrevious->next = pNewElement;

	if (!pCurrent) {
		pCurrent = pHead;
	}

	pNewElement->prev = pCurrent->prev;
	pCurrent->prev = pNewElement;

	pNewElement->idx = objIdx;
	pNewElement->type = param2;
	pNewElement->pathId = -1;
	pNewElement->overlayNumber = overlay;
	pNewElement->startDirection = param;
	pNewElement->nextDirection = -1;
	pNewElement->stepX = 5;
	pNewElement->stepY = 2;
	pNewElement->phase = ANIM_PHASE_WAIT;
	pNewElement->flag = 0;
	pNewElement->freeze = 0;

	return pNewElement;
}

int removeAnimation(actorStruct * pHead, int overlay, int objIdx, int objType) {
	actorStruct* pl;
	actorStruct* pl2;
	actorStruct* pl3;
	actorStruct* pl4;

	int dir = 0;

	pl = pHead;
	pl2 = pl;
	pl = pl2->next;

	while (pl) {
		pl2 = pl;

		if (((pl->overlayNumber == overlay) || (overlay == -1)) &&
		        ((pl->idx == objIdx) || (objIdx == -1)) &&
		        ((pl->type == objType) || (objType == -1))) {
			pl->type = -1;
		}

		pl = pl2->next;
	}

	pl = pHead;
	pl2 = pl;
	pl = pl2->next;

	while (pl) {
		if (pl->type == -1) {
			pl4 = pl->next;
			pl2->next = pl4;
			pl3 = pl4;

			if (pl3 == NULL)
				pl3 = pHead;

			pl3->prev = pl->prev;

			dir = pl->startDirection;

			if (pl->idx >= 0)
				freePerso(pl->idx);

			free(pl);
			pl = pl4;
		} else {
			pl2 = pl;
			pl = pl2->next;
		}
	}

	return dir;
}

int flag_obstacle;		// computedVar14Bis

// add animation
int16 Op_AddAnimation(void) {
	int stepY = popVar();
	int stepX = popVar();
	int direction = popVar();
	int start = popVar();
	int type = popVar();
	int obj = popVar();
	int overlay = popVar();

	if (!overlay) {
		overlay = currentScriptPtr->overlayNumber;
	}

	if (direction >= 0 && direction <= 3) {
		actorStruct *si;

		si = addAnimation(&actorHead, overlay, obj, direction, type);

		if (si) {
			objectParamsQuery params;

			getMultipleObjectParam(overlay, obj, &params);

			si->x = params.X;
			si->y = params.Y;
			si->x_dest = -1;
			si->y_dest = -1;
			si->endDirection = -1;
			si->start = start;
			si->stepX = stepX;
			si->stepY = stepY;

			int newFrame = ABS(actor_end[direction][0]) - 1;

			int zoom = computeZoom(params.Y);

			if (actor_end[direction][0] < 0) {
				zoom = -zoom;
			}

			getPixel(params.X, params.Y);

			setObjectPosition(overlay, obj, 3, newFrame + start);
			setObjectPosition(overlay, obj, 4, zoom);
			setObjectPosition(overlay, obj, 5, computedVar14);

			animationStart = false;
		}
	}

	return 0;
}

int16 Op_RemoveAnimation(void) {
	int objType = popVar();
	int objIdx = popVar();
	int ovlIdx = popVar();

	if (!ovlIdx) {
		ovlIdx = currentScriptPtr->overlayNumber;
	}

	return removeAnimation(&actorHead, ovlIdx, objIdx, objType);
}

int16 Op_regenerateBackgroundIncrust(void) {
	regenerateBackgroundIncrust(&backgroundIncrustHead);
	return 0;
}

int16 Op_SetStringColors(void) {
	// TODO: here ignore if low color mode

	subColor = (uint8) popVar();
	itemColor = (uint8) popVar();
	selectColor = (uint8) popVar();
	titleColor = (uint8) popVar();

	return 0;
}

int16 Op_TrackAnim(void) {		// setup actor position
	actorStruct *pActor;

	int var0 = popVar();
	int actorY = popVar();
	int actorX = popVar();
	int var1 = popVar();
	int var2 = popVar();
	int overlay = popVar();

	if (!overlay) {
		overlay = currentScriptPtr->overlayNumber;
	}

	pActor = findActor(&actorHead, overlay, var2, var1);

	if (!pActor) {
		return 1;
	}

	animationStart = false;

	pActor->x_dest = actorX;
	pActor->y_dest = actorY;
	pActor->flag = 1;
	pActor->endDirection = var0;

	return 0;
}

int16 Op_BgName(void) {
	char* bgName = (char*)popPtr();
	int bgIdx = popVar();

	if ((bgIdx >= 0) && (bgIdx < 8) && bgName) {
		strcpy(bgName, backgroundTable[bgIdx].name);

		if (strlen(bgName))
			return 1;

		return 0;
	}

	return 0;
}

int16 Op_StopFX(void) {
	int fxIdx = popVar();

	printf("StopFX(%d)\n", fxIdx);

	return 0;
}

int16 Op_LoadSong(void) {
	const char *ptr = (const char *)popPtr();
	char buffer[33];

	strcpy(buffer, ptr);
	strToUpper(buffer);
	_vm->music().loadSong(buffer);

	changeCursor(CURSOR_NORMAL);
	return 0;
}

int16 Op_PlaySong(void) {
	if (_vm->music().songLoaded() && !_vm->music().songPlayed())
		_vm->music().startSong();

	return 0;
}

int16 Op_StopSong(void) {
	if (_vm->music().isPlaying())
		_vm->music().stop();

	return 0;
}

int16 Op_FadeSong(void) {
	_vm->music().fadeSong();
	
	return 0;
}

int16 Op_FreeSong(void) {
	_vm->music().stop();
	_vm->music().removeSong();
	return 0;
}

void setVar49Value(int value) {
	flagCt = value;
}

int16 Op_CTOn(void) {
	setVar49Value(1);
	return 0;
}

int16 Op_CTOff(void) {
	setVar49Value(0);
	return 0;
}

int16 Op_FreezeOverlay(void) {
	//int var0;
	//int var1;
	int temp;

	int var0 = popVar();
	int var1 = popVar();

	if (!var1) {
		var1 = currentScriptPtr->overlayNumber;
	}

	temp = overlayTable[var1].executeScripts;
	overlayTable[var1].executeScripts = var0;

	return temp;
}

int16 Op_FreezeCell(void) {
	int newFreezz = popVar();
	int oldFreeze = popVar();
	int backgroundPlante = popVar();
	int objType = popVar();
	int objIdx = popVar();
	int overlayIdx = popVar();

	if (!overlayIdx) {
		overlayIdx = currentScriptPtr->overlayNumber;
	}

	freezeCell(&cellHead, overlayIdx, objIdx, objType, backgroundPlante, oldFreeze, newFreezz);

	return 0;
}

void Op_60Sub(int overlayIdx, actorStruct * pActorHead, int _var0, int _var1, int _var2, int _var3) {
	actorStruct *pActor = findActor(pActorHead, overlayIdx, _var0, _var3);

	if (pActor) {
		if ((pActor->freeze == _var2) || (_var2 == -1)) {
			pActor->freeze = _var1;
		}
	}
}

int16 Op_FreezeAni(void) {
	/*
	 * int var0;
	 * int var1;
	 * int var2;
	 * int var3;
	 * int var4;
	 */

	int var0 = popVar();
	int var1 = popVar();
	int var2 = popVar();
	int var3 = popVar();
	int var4 = popVar();

	if (!var4) {
		var4 = currentScriptPtr->overlayNumber;
	}

	Op_60Sub(var4, &actorHead, var3, var0, var1, var2);

	return 0;
}

int16 Op_Itoa(void) {
	int nbp = popVar();
	int param[160];
	char txt[40];
	char format[30];
	char nbf[20];

	for (int i = nbp - 1; i >= 0; i--)
		param[i] = popVar();

	int val = popVar();
	char* pDest = (char*)popPtr();

	if (!nbp)
		sprintf(txt, "%d", val);
	else {
		strcpy(format, "%");
		sprintf(nbf, "%d", param[0]);
		strcat(format, nbf);
		strcat(format, "d");
		sprintf(txt, format, val);
	}

	for (int i = 0; txt[i]; i++)
		*(pDest++) = txt[i];
	*(pDest++) = '\0';

	return 0;
}

int16 Op_Strcat(void) {
	char *pSource = (char *)popPtr();
	char *pDest = (char *)popPtr();

	while (*pDest)
		pDest++;

	while (*pSource)
		*(pDest++) = *(pSource++);
	*(pDest++) = '\0';

	return 0;
}

int16 Op_FindSymbol(void) {
	int var0 = popVar();
	char *ptr = (char *)popPtr();
	int var1 = popVar();

	if (!var1)
		var1 = currentScriptPtr->overlayNumber;

	return getProcParam(var1, var0, ptr);
}

int16 Op_FindObject(void) {
	char var_26[36];
	char *ptr = (char *)popPtr();
	int overlayIdx;

	var_26[0] = 0;

	if (ptr) {
		strcpy(var_26, ptr);
	}

	overlayIdx = popVar();

	if (!overlayIdx)
		overlayIdx = currentScriptPtr->overlayNumber;

	return getProcParam(overlayIdx, 40, var_26);
}

int16 Op_SetObjectAtNode(void) {
	int16 node = popVar();
	int16 obj = popVar();
	int16 ovl = popVar();

	if (!ovl)
		ovl = currentScriptPtr->overlayNumber;

	int nodeInfo[2];

	if (!getNode(nodeInfo, node)) {
		setObjectPosition(ovl, obj, 0, nodeInfo[0]);
		setObjectPosition(ovl, obj, 1, nodeInfo[1]);
		setObjectPosition(ovl, obj, 2, nodeInfo[1]);
		setObjectPosition(ovl, obj, 4, computeZoom(nodeInfo[1]));
	}

	return 0;
}

int16 Op_GetNodeX(void) {
	int16 node = popVar();

	int nodeInfo[2];

	int result = getNode(nodeInfo, node);

	ASSERT(result == 0);

	return nodeInfo[0];
}

int16 Op_GetNodeY(void) {
	int16 node = popVar();

	int nodeInfo[2];

	int result = getNode(nodeInfo, node);

	ASSERT(result == 0);

	return nodeInfo[1];
}

int16 Op_SongExist(void) {
	char* songName = (char*)popPtr();

	printf("Unimplemented \"Op_SongExist\": %s\n", songName);

	return 0;
}

int16 Op_SetNodeState(void) {
	int16 state = popVar();
	int16 node = popVar();

	return setNodeState(node, state);
}

int16 Op_SetNodeColor(void) {
	int16 color = popVar();
	int16 node = popVar();

	return setNodeColor(node, color);
}

int16 Op_SetXDial(void) {
	int16 old;

	old = xdial;
	xdial = popVar();

	return old;
}

int16 Op_DialogOn(void) {
	dialogueObj = popVar();
	dialogueOvl = popVar();

	if (dialogueOvl == 0)
		dialogueOvl = currentScriptPtr->overlayNumber;

	dialogueEnabled = true;

	return 0;
}

int16 Op_DialogOff(void) {
	dialogueEnabled = false;

	objectReset();

	if (menuTable[0]) {
		freeMenu(menuTable[0]);
		menuTable[0] = NULL;
		changeCursor(CURSOR_NORMAL);
		currentActiveMenu = -1;
	}

	return 0;
}

int16 Op_LinkObjects(void) {
	int type = popVar();
	int obj2 = popVar();
	int ovl2 = popVar();
	int obj = popVar();
	int ovl = popVar();

	if (!ovl)
		ovl = currentScriptPtr->overlayNumber;
	if (!ovl2)
		ovl2 = currentScriptPtr->overlayNumber;

	linkCell(&cellHead, ovl, obj, type, ovl2, obj2);

	return 0;
}

int16 Op_UserDelay(void) {
	int delay = popVar();

	if (delay >= 0) {
		userDelay = delay;
	}

	return userDelay;
}

int16 Op_UserWait(void) {
	userWait = 1;
	if (currentScriptPtr->type == scriptType_PROC) {
		changeScriptParamInList(currentScriptPtr->overlayNumber, currentScriptPtr->scriptNumber, &procHead, -1, 9999);
	} else if (currentScriptPtr->type == scriptType_REL) {
		changeScriptParamInList(currentScriptPtr->overlayNumber, currentScriptPtr->scriptNumber, &relHead, -1, 9999);
	}

	return 0;
}

opcodeFunction opcodeTablePtr[] = {
	NULL, // 0x00
	Op_FadeIn,
	Op_FadeOut,
	Op_LoadBackground,
	Op_LoadAbs,
	Op_AddCell,
	Op_AddProc,
	Op_InitializeState,
	Op_RemoveCell,
	Op_FreeCell,
	Op_RemoveProc,
	Op_RemoveFrame,
	Op_LoadOverlay,
	Op_SetColor,
	Op_PlayFX,
	NULL,	// used to be debug

	Op_FreeOverlay, // 0x10
	Op_FindOverlay,
	NULL,	// used to be exec debug
	Op_AddMessage,
	Op_RemoveMessage,
	Op_UserWait,
	Op_FreezeCell,
	Op_LoadCt,
	Op_AddAnimation,
	Op_RemoveAnimation,
	Op_SetZoom,
	Op_SetObjectAtNode,
	Op_SetNodeState,
	Op_SetNodeColor,
	Op_TrackAnim,
	Op_GetNodeX,

	Op_GetNodeY, // 0x20
	Op_EndAnim,
	Op_GetZoom,
	Op_GetStep,
	Op_SetStringColors,
	NULL, // xClick
	NULL, // yClick
	NULL, // getPixel
	Op_UserOn,
	Op_FreeCT,
	Op_FindObject,
	Op_FindProc,
	Op_WriteObject,
	Op_ReadObject,
	Op_RemoveOverlay,
	Op_AddBackgroundIncrust,

	Op_RemoveBackgroundIncrust, // 0x30
	Op_UnmergeBackgroundIncrust,
	Op_freeBackgroundInscrustList,
	Op_DialogOn,
	Op_DialogOff,
	Op_UserDelay,
	NULL, // ThemeReset
	Op_Narrator,
	Op_RemoveBackground,
	Op_SetActiveBackground,
	Op_CTOn,
	Op_CTOff,
	Op_Random,
	Op_LoadSong,
	Op_FadeSong,
	Op_PlaySong,

	Op_FreeSong, // 0x40
	Op_FrameExist,
	NULL, // SetVolume
	Op_SongExist,
	NULL, // TrackPos
	Op_StopSong,
	NULL, // RestoreSong
	NULL, // SongSize
	NULL, // SetPattern
	NULL, // SongLoop
	NULL, // SongPlayed
	Op_LinkObjects,
	NULL, // UserClick
	NULL, // XMenuItem
	NULL, // YMenuItem
	NULL, // Menu

	NULL, // AutoControl 0x50
	NULL, // MouseMove
	NULL, // MouseEnd
	NULL, // MsgExist
	Op_SetFont,
	NULL, // MergeMsg
	Op_Display,
	Op_GetMouseX,
	Op_GetMouseY,
	Op_GetMouseButton,
	Op_FindSet,
	Op_regenerateBackgroundIncrust,
	Op_BgName,
	NULL, // loopFX
	Op_StopFX,
	NULL, // freqFX

	Op_FreezeAni, // 0x60
	Op_FindMsg,
	Op_FreezeParent,
	Op_UnfreezeParent,
	Op_Exec,
	Op_AutoCell,
	Op_Sizeof,
	Op_Preload,
	Op_FreePreload,
	NULL, // DeletePreload
	Op_VBL,
	Op_LoadFrame,
	Op_FreezeOverlay,
	Op_Strcpy,
	Op_Strcat,
	Op_Itoa,

	Op_comment, // 0x70
	Op_ComputeLine,
	Op_FindSymbol,
	Op_SetXDial,
	Op_GetlowMemory,
	NULL, // aniDir
	Op_Protect,
	NULL, // Cls
	Op_Inventory,
	Op_UserMenu,
	NULL, // GetChar
	Op_Sec,
	Op_ProtectionFlag,
	Op_KillMenu,
};

void setupOpcodeTable(void) {
//	int i;

	/*	for (i = 0; i < 256; i++) {
			opcodeTablePtr[i] = NULL;
		}

		opcodeTablePtr[0x1] = Op_FadeIn;
		opcodeTablePtr[0x2] = Op_FadeOut;
		opcodeTablePtr[0x3] = Op_LoadBackground;
		opcodeTablePtr[0x4] = Op_LoadAbs;
		opcodeTablePtr[0x5] = Op_AddCell;
		opcodeTablePtr[0x6] = Op_AddProc;
		opcodeTablePtr[0x7] = Op_InitializeState;
		opcodeTablePtr[0x8] = Op_RemoveCell;
		opcodeTablePtr[0x9] = Op_FreeCell;
		opcodeTablePtr[0xA] = Op_RemoveProc;
		opcodeTablePtr[0xB] = Op_RemoveFrame;
		opcodeTablePtr[0xC] = Op_LoadOverlay;
		opcodeTablePtr[0xD] = Op_SetColor;
		opcodeTablePtr[0xE] = Op_PlayFX;
		opcodeTablePtr[0xF] = NULL;	// used to be debug
		opcodeTablePtr[0x10] = Op_FreeOverlay;
		opcodeTablePtr[0x11] = Op_FindOverlay;
		opcodeTablePtr[0x12] = NULL;	// used to be exec debug
		opcodeTablePtr[0x13] = Op_AddMessage;
		opcodeTablePtr[0x14] = Op_RemoveMessage;
		opcodeTablePtr[0x15] = Op_UserWait;
		opcodeTablePtr[0x16] = Op_FreezeCell;
		opcodeTablePtr[0x17] = Op_LoadCt;
		opcodeTablePtr[0x18] = Op_AddAnimation;
		opcodeTablePtr[0x19] = Op_RemoveAnimation;
		opcodeTablePtr[0x1A] = Op_SetZoom;
		opcodeTablePtr[0x1B] = Op_SetObjectAtNode;
		opcodeTablePtr[0x1D] = Op_SetNodeColor;
		opcodeTablePtr[0x1E] = Op_TrackAnim;
		opcodeTablePtr[0x1F] = Op_GetNodeX;
		opcodeTablePtr[0x20] = Op_GetNodeY;
		opcodeTablePtr[0x21] = Op_EndAnim;
		opcodeTablePtr[0x22] = Op_GetZoom;
		opcodeTablePtr[0x23] = Op_GetStep;
		opcodeTablePtr[0x24] = Op_SetStringColors;
		opcodeTablePtr[0x28] = Op_UserOn;
		opcodeTablePtr[0x29] = Op_FreeCT;
		opcodeTablePtr[0x2A] = Op_FindObject;
		opcodeTablePtr[0x2B] = Op_FindProc;
		opcodeTablePtr[0x2C] = Op_WriteObject;
		opcodeTablePtr[0x2E] = Op_RemoveOverlay;
		opcodeTablePtr[0x2F] = Op_AddBackgroundIncrust;
		opcodeTablePtr[0x30] = Op_RemoveBackgroundIncrust;
		opcodeTablePtr[0x31] = Op_UnmergeBackgroundIncrust;
		opcodeTablePtr[0x32] = Op_freeBackgroundInscrustList;
		opcodeTablePtr[0x33] = Op_DialogOn;
		opcodeTablePtr[0x34] = Op_DialogOff;
		opcodeTablePtr[0x35] = Op_UserDelay;
		opcodeTablePtr[0x37] = Op_Narrator;
		opcodeTablePtr[0x38] = Op_RemoveBackground;
		opcodeTablePtr[0x39] = Op_SetActiveBackground;
		opcodeTablePtr[0x3A] = Op_CTOn;
		opcodeTablePtr[0x3B] = Op_CTOff;
		opcodeTablePtr[0x3C] = Op_Random;
		opcodeTablePtr[0x3D] = Op_LoadSong;
		opcodeTablePtr[0x3E] = Op_PlaySong;
		opcodeTablePtr[0x3F] = Op_FadeSong;
		opcodeTablePtr[0x40] = Op_FreeSong;
		opcodeTablePtr[0x41] = Op_FrameExist;
		opcodeTablePtr[0x43] = Op_SongExist;
		opcodeTablePtr[0x45] = Op_StopSong;
		opcodeTablePtr[0x4B] = Op_LinkObjects;
		opcodeTablePtr[0x54] = Op_SetFont;
		opcodeTablePtr[0x56] = Op_Display;
		opcodeTablePtr[0x57] = Op_GetMouseX;
		opcodeTablePtr[0x58] = Op_GetMouseY;
		opcodeTablePtr[0x59] = Op_GetMouseButton;
		opcodeTablePtr[0x5A] = Op_FindSet;
		opcodeTablePtr[0x5B] = Op_regenerateBackgroundIncrust;
		opcodeTablePtr[0x5C] = Op_BgName;
		opcodeTablePtr[0x5E] = Op_StopFX;
		opcodeTablePtr[0x60] = Op_FreezeAni;
		opcodeTablePtr[0x61] = Op_FindMsg;
		opcodeTablePtr[0x62] = Op_FreezeParent;
		opcodeTablePtr[0x63] = Op_UnfreezeParent;
		opcodeTablePtr[0x64] = Op_Exec;
		opcodeTablePtr[0x65] = Op_AutoCell;
		opcodeTablePtr[0x66] = Op_Sizeof;
		opcodeTablePtr[0x67] = Op_Preload;
		opcodeTablePtr[0x68] = Op_FreePreload;
		opcodeTablePtr[0x6A] = Op_VBL;
		opcodeTablePtr[0x6B] = Op_LoadFrame;
		opcodeTablePtr[0x6C] = Op_FreezeOverlay;
		opcodeTablePtr[0x6D] = Op_Strcpy;
		opcodeTablePtr[0x6E] = Op_Strcat;
		opcodeTablePtr[0x6F] = Op_Itoa;
		opcodeTablePtr[0x70] = Op_comment;
		opcodeTablePtr[0x71] = Op_ComputeLine;
		opcodeTablePtr[0x72] = Op_FindSymbol;
		opcodeTablePtr[0x73] = Op_SetXDial;
		opcodeTablePtr[0x74] = Op_GetlowMemory;
		opcodeTablePtr[0x76] = Op_Protect;
		opcodeTablePtr[0x79] = Op_UserMenu;
		opcodeTablePtr[0x78] = Op_Inventory;
		opcodeTablePtr[0x7B] = Op_Sec;
		opcodeTablePtr[0x7C] = Op_ProtectionFlag;
		opcodeTablePtr[0x7D] = Op_KillMenu;*/
	// TODO: copy the opcodes here
}

int32 opcodeType8(void) {
	int opcode = getByteFromScript();

	if (!opcode)
		return (-21);

	if (opcode > 0x100)
		return (-21);

	if (opcode < ARRAYSIZE(opcodeTablePtr) && opcodeTablePtr[opcode]) {
		//	printf("Function: %d\n",opcode);
		pushVar(opcodeTablePtr[opcode]());
		return (0);
	} else {
		printf("Unsupported opcode %d in opcode type 8\n", opcode);
		pushVar(0);
		// exit(1);
	}

	return 0;

}

} // End of namespace Cruise
