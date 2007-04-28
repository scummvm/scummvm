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

#include "cruise/cruise_main.h"
#include "cruise/cell.h"
#include "common/util.h"

namespace Cruise {

opcodeFunction opcodeTablePtr[256];

int16 Op_LoadOverlay(void) {
	uint8 *originalScriptName;
	uint8 scriptName[38];
	int returnValue;

	scriptName[0] = 0;

	originalScriptName = (uint8 *) popPtr();

	if (originalScriptName) {
		strcpyuint8(scriptName, originalScriptName);
	}

	if (!scriptName[0] || !originalScriptName) {
		return (0);
	}

	strToUpper(scriptName);

	//gfxModuleData.field_84();
	//gfxModuleData.field_84();

	returnValue = loadOverlay(scriptName);

	updateAllScriptsImports();

	strcpyuint8(scriptNameBuffer, scriptName);

	return (returnValue);
}

int16 Op_strcpy(void) {
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

int16 Op_startScript(void) {
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

	ptr =
	    attacheNewScriptToTail(ovlIdx, &procHead, scriptIdx,
	    currentScriptPtr->type, currentScriptPtr->scriptNumber,
	    currentScriptPtr->overlayNumber, scriptType_MinusPROC);

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

	if (pop1 - 1 > 0) {
		printf("Unsuported arg pop in Op_6!\n");
		exit(1);
	}

	pop2 = popVar();
	overlay = popVar();

	if (!overlay)
		overlay = currentScriptPtr->overlayNumber;

	if (!overlay)
		return (0);

	attacheNewScriptToTail(overlay, &procHead, pop2, currentScriptPtr->type, currentScriptPtr->scriptNumber, currentScriptPtr->overlayNumber, scriptType_PROC);

	if (pop1 > 0) {
		printf("Unsupported art send in op6!\n");
		exit(1);
	}

	return (0);
}

int16 Op_37(void) {
	int pop1 = popVar();
	int pop2 = popVar();

	if (!pop2)
		pop2 = currentScriptPtr->overlayNumber;

	var30 = pop2;
	var31 = pop1;

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

int16 Op_rand(void) {		// TODO: implement
	int var = popVar();

	if (var < 2) {
		return (0);
	}
//      return(8);
	return (rand() % var);
}

int16 Op_PlayFX(void) {		// TODO: implement
	popVar();
	popVar();
	popVar();
	popVar();

	// printf("Op_PlayFX, implement (sound related)\n");

	return (0);
}

int16 Op_freeAllPerso(void) {
	freeAllPerso();
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

int16 Op_removeBackground(void) {
	int backgroundIdx;

	backgroundIdx = popVar();

	printf("Op_removeBackground: remove background %d\n", backgroundIdx);
	return (0);
}

int16 Op_freeMediumVar(void) {
	// TODO: implement
	printf("Op_freeMediumVar, implement\n");
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

	removeCell(&cellHead, overlay, idx, 5, currentActiveBackgroundPlane);

	return (0);
}

int16 Op_isFileLoaded(void) {
	int16 i;
	uint8 name[36] = "";
	uint8 *ptr;

	ptr = (uint8 *) popPtr();

	if (!ptr) {
		return -1;
	}

	strcpyuint8(name, ptr);
	strToUpper(name);

	for (i = 0; i < 257; i++) {
		if (!strcmpuint8(name, filesDatabase[i].subData.name)) {
			return (i);
		}
	}

	return -1;
}

int16 Op_RemoveFrame(void) {
	//int var1;
	//int var2;

	var1 = popVar();
	var2 = popVar();

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
	uint8 localName[36] = "";
	uint8 *namePtr;

	namePtr = (uint8 *) popPtr();

	strcpyuint8(localName, namePtr);

	if (localName[0]) {
		strToUpper(localName);
		releaseOverlay((char *)localName);
	}

	return 0;
}

int16 Op_2B(void) {
	uint8 name[36] = "";
	uint8 *ptr;
	int param;

	ptr = (uint8 *) popPtr();

	strcpyuint8(name, ptr);

	param = getProcParam(popVar(), 20, name);

	return param;
}

int16 Op_freeAllMenu(void) {
	// TODO: implement
	printf("Op_freeAllMenu, implement\n");

	return 0;
}

int16 Op_PlayFXnterPlayerMenu(void) {
	int oldValue = entrerMenuJoueur;
	entrerMenuJoueur = popVar();

	return oldValue;
}

int16 Op_ChangeSaveAllowedState(void) {
	int oldValue = userEnabled;
	int newValue = popVar();

	if (newValue != -1) {
		userEnabled = newValue;
	}

	return oldValue;
}

int16 Op_changeCutSceneState(void) {
	int oldValue = affichePasMenuJoueur;
	int newValue = popVar();

	if (newValue != -1) {
		affichePasMenuJoueur = newValue;
	}

	return oldValue;
}

int16 Op_62(void) {
	if (currentScriptPtr->var1A == 20) {
		changeScriptParamInList(currentScriptPtr->var18,
		    currentScriptPtr->var16, &procHead, 9997, -1);
	} else if (currentScriptPtr->var1A == 30) {
		changeScriptParamInList(currentScriptPtr->var18,
		    currentScriptPtr->var16, &relHead, 9997, -1);
	}

	return 0;
}

int16 Op_LoadBackground(void) {
	int result = 0;
	uint8 bgName[36] = "";
	uint8 *ptr;
	int bgIdx;

	ptr = (uint8 *) popPtr();

	strcpyuint8(bgName, ptr);

	bgIdx = popVar();

	if (bgIdx >= 0 || bgIdx < 8) {
		strToUpper(bgName);

		gfxModuleData_gfxWaitVSync();
		gfxModuleData_gfxWaitVSync();

		result = loadBackground((char *)bgName, bgIdx);
	}

	changeCursor(0);

	return result;
}

int16 Op_isFileLoaded2(void) {
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

int16 Op_loadFile(void) {
	int param1;
	int param2;
	int param3;
	uint8 name[36] = "";
	uint8 *ptr;

	ptr = (uint8 *) popPtr();

	strcpyuint8(name, ptr);

	param1 = popVar();
	param2 = popVar();
	param3 = popVar();

	if (param3 >= 0 || param3 < 257) {
		strToUpper(name);

		gfxModuleData_gfxWaitVSync();
		gfxModuleData_gfxWaitVSync();

		saveVar6[0] = 0;

		loadFileMode2(name, param3, param2, param1);

		saveVar6[0] = 0;
	}

	changeCursor(0);
	return 0;
}

int16 Op_LoadAbs(void) {
	int param1;
//  int param2;
//  int param3;
	uint8 name[36] = "";
	uint8 *ptr;
	int result = 0;

	ptr = (uint8 *) popPtr();

	strcpyuint8(name, ptr);

	param1 = popVar();

	if (param1 >= 0 || param1 < 257) {
		strToUpper(name);

		gfxModuleData_gfxWaitVSync();
		gfxModuleData_gfxWaitVSync();

		result = loadFullBundle(name, param1);
	}

	changeCursor(0);
	return result;
}

int16 Op_InitializeState(void) {
	int param1 = popVar();
	int objIdx = popVar();
	int ovlIdx = popVar();

	if (!ovlIdx)
		ovlIdx = currentScriptPtr->overlayNumber;

	objInit(ovlIdx, objIdx, param1);

	return (0);
}

int16 Op_GetInitVar1(void) {
	return initVar1;
}

int16 Op_FadeOut(void) {
	printf("Op_FadeOut dummy\n");
	return 0;
}

int16 isOverlayLoaded(uint8 * name) {
	int16 i;

	for (i = 1; i < numOfLoadedOverlay; i++) {
		if (!strcmpuint8(overlayTable[i].overlayName, name)
		    && overlayTable[i].alreadyLoaded) {
			return i;
		}
	}

	return 0;
}

int16 Op_FindOverlay(void) {
	uint8 name[36] = "";
	uint8 *ptr;

	ptr = (uint8 *) popPtr();

	strcpyuint8(name, ptr);
	strToUpper(name);

	return (isOverlayLoaded(name));
}

int16 Op_2C(void) {
	int16 returnParam;

	int16 param1 = popVar();
	int16 param2 = popVar();
	int16 param3 = popVar();
	int16 param4 = popVar();

	getSingleObjectParam(param4, param3, param2, &returnParam);
	setObjectPosition(param4, param3, param2, param1);

	return returnParam;
}

int16 Op_FadeIn(void) {
	main5 = 1;
	return 0;
}

int16 Op_GetMouseClick3(void) {
	int16 dummy;
	int16 mouseX;
	int16 mouseY;
	int16 mouseButton;

	getMouseStatus(&dummy, &mouseX, &mouseButton, &mouseY);

	if (mouseButton & 4)
		return 1;
	return 0;
}

int16 Op_AddCell(void) {
	int16 objType = popVar();
	int16 objIdx = popVar();
	int16 overlayIdx = popVar();

	if (!overlayIdx)
		overlayIdx = currentScriptPtr->overlayNumber;

	addCell(&cellHead, overlayIdx, objIdx, objType, currentActiveBackgroundPlane, currentScriptPtr->overlayNumber, currentScriptPtr->scriptNumber, currentScriptPtr->type);

	return 0;
}

int16 Op_AddBackgroundIncrust(void) {

	int16 objType = popVar();
	int16 objIdx = popVar();
	int16 overlayIdx = popVar();

	if (!overlayIdx)
		overlayIdx = currentScriptPtr->overlayNumber;

	addBackgroundIncrust(overlayIdx, objIdx, &backgroundIncrustHead, currentScriptPtr->scriptNumber, currentScriptPtr->overlayNumber, currentActiveBackgroundPlane, objType);

	return 0;
}

int16 Op_RemoveCell(void) {
	int objType = popVar();
	int objectIdx = popVar();
	int ovlNumber = popVar();

	if (!ovlNumber) {
		ovlNumber = currentScriptPtr->overlayNumber;
	}

	removeCell(&cellHead, ovlNumber, objectIdx, objType, currentActiveBackgroundPlane);

	return 0;
}

int16 fontFileIndex;

int16 Op_SetFontFileIndex(void) {
	fontFileIndex = popVar();

	return 0;
}

int16 Op_63(void) {
	if (currentScriptPtr->var1A == 0x14) {
		changeScriptParamInList(currentScriptPtr->var18,
		    currentScriptPtr->var16, &procHead, 0, -1);
	} else if (currentScriptPtr->var1A == 0x1E) {
		changeScriptParamInList(currentScriptPtr->var18,
		    currentScriptPtr->var16, &relHead, 0, -1);
	}

	return 0;
}

int16 op7CVar = 0;

int16 Op_InitializeStateC(void) {
	int16 temp = op7CVar;
	int16 newVar;

	newVar = popVar();
	if (newVar != -1) {
		op7CVar = newVar;
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
		color = 0;
		ASSERT(0);
		//color = calcTabSomething();
	} else {
		if (CVTLoaded) {
			color = cvtPalette[color];
		}
	}

	createTextObject(overlayIdx, var_8, &cellHead,
	    currentScriptPtr->scriptNumber, currentScriptPtr->overlayNumber,
	    currentActiveBackgroundPlane, color, var_2, var_4, var_6);

	return 0;
}

int16 Op_loadAudioResource(void) {
	popPtr();
	popVar();

	return 0;
}

int16 Op_LoadCt(void) {
	return loadCtp((uint8 *) popPtr());
}

int16 Op_loadMusic(void) {
	popPtr();
	return 0;
}

int16 Op_21(void) {
	int param1 = popVar();
	int param2 = popVar();
	int overlay = popVar();

	if (!overlay)
		overlay = currentScriptPtr->overlayNumber;

	return mainProc13(overlay, param2, &actorHead, param1);
}

int16 Op_InitializeState6(void) {
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

	pObject = addCell(&cellHead, overlay, obj, 4, currentActiveBackgroundPlane, currentScriptPtr->overlayNumber, currentScriptPtr->scriptNumber, currentScriptPtr->type);

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
			changeScriptParamInList(currentScriptPtr->overlayNumber, currentScriptPtr->scriptNumber, &procHead, 9996, -1);
		} else if (currentScriptPtr->type == scriptType_REL) {
			changeScriptParamInList(currentScriptPtr->overlayNumber, currentScriptPtr->scriptNumber, &relHead, 9996, -1);
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
		pObject->animCounter = params.var6 - 1;
	}

	return 0;
}

int16 Op_66(void) {
	objectParamsQuery params;
	int index = popVar();
	int overlay = popVar();

	if (!overlay)
		overlay = currentScriptPtr->overlayNumber;

	getMultipleObjectParam(overlay, index, &params);

	return params.var7;
}

int16 Op_SetActiveBackgroundPlane(void) {
	int currentPlane = currentActiveBackgroundPlane;
	int newPlane = popVar();

	if (newPlane >= 0 && newPlane < 8) {
		if (backgroundPtrtable[newPlane]) {
			currentActiveBackgroundPlane = newPlane;
			initVar3 = 1;
		}
	}

	return currentPlane;
}

int op6AVar;

int16 Op_6A(void) {
	op6AVar = popVar();
	return 0;
}

int op7BVar = 0;

int16 Op_InitializeStateB(void) {
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

	int R,G,B;

#define convertRatio 36.571428571428571428571428571429

	for(i=startIdx; i<=endIdx; i++) {
		R = (int)(colorR*convertRatio);
		G = (int)(colorG*convertRatio);
		B = (int)(colorB*convertRatio);

		if (R > 0xFF)
			R = 0xFF;
		if (G > 0xFF)
			G = 0xFF;
		if (B > 0xFF)
			B = 0xFF;

		if (CVTLoaded) {
			int colorIdx = cvtPalette[i];
			
			lpalette[colorIdx].R = R;
			lpalette[colorIdx].G = G;
			lpalette[colorIdx].B = B;
		} else {
			lpalette[i].R = R;
			lpalette[i].G = G;
			lpalette[i].B = B;
		}
	}

	return 0;
}

int16 Op_InitializeState8(void) {
	int si = var41;

	var41 = popVar();

	return si;
}

int16 Op_releaseOverlay(void) {
	int overlayIdx;

	overlayIdx = popVar();

	if (strlen(overlayTable[overlayIdx].overlayName)) {
		releaseOverlay(overlayTable[overlayIdx].overlayName);
	}

	return 0;
}

int16 Op_SetColorrawLine(void) {
	/*
	 * int di = popVar();
	 * int var_2 = popVar();
	 * int var_4 = popVar();
	 * int var_6 = popVar();
	 * uint8* ptr = (uint8*)popPtr();
	 */

	popVar();
	popVar();
	popVar();
	popVar();
	popPtr();

	//drawLinePtr(var_6, var_4, var_2, ptr);

	// flipGen(ptr);

	return 0;
}

int16 Op_61(void) {
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

int16 Op_23(void) {
	int si = popVar();
	int dx = popVar();

	return subOp23(dx, si);
}

int16 Op_22(void) {
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

int removeAnimation(actorStruct * pHead, int overlay, int objIdx, int objType)
{
	actorStruct* pl;
	actorStruct* pl2;
	actorStruct* pl3;
	actorStruct* pl4;

	int dir = 0;

	pl = pHead;
	pl2 = pl;
	pl = pl2->next;

	while(pl)
	{
		pl2 = pl;

		if(((pl->overlayNumber == overlay) || (overlay == -1)) &&
			((pl->idx == objIdx) || (objIdx == -1)) &&
			((pl->type == objType) || (objType == -1)))
		{
			pl->type = -1;
		}

		pl = pl2->next;
	}

	pl = pHead;
	pl2 = pl;
	pl = pl2->next;

	while(pl)
	{
		if(pl->type == -1)
		{
			pl4 = pl->next;
			pl2->next = pl4;
			pl3 = pl4;

			if(pl3 == NULL)
				pl3 = pHead;

			pl3->prev = pl->prev;

			dir = pl->startDirection;

			if(pl->idx >= 0)
				freePerso(pl->idx);

			free(pl);
			pl = pl4;
		}
		else
		{
			pl2 = pl;
			pl = pl2->next;
		}
	}

	return dir;
}

int flag_obstacle;		// computedVar14Bis

void checkCollisionWithWalkBoxesBoundingBoxes(int x, int y) {
	ctpVar19Struct *di = ctpVar19;

	do {
		int minX;
		int maxX;
		int minY;
		int maxY;

		ctpVar19SubStruct *subStruct;

		if (-1 == (int)di->field_0)	// ok, ugly, but it's in the original
		{
			flag_obstacle = 0;
			return;
		}

		subStruct = &di->subStruct;

		minX = subStruct->minX;
		maxX = subStruct->maxX;
		minY = subStruct->minY;
		maxY = subStruct->maxY;

		computedVar14 = subStruct->boxIdx;	// Box index

		if (!(walkboxChange[subStruct->boxIdx]) && (minY >= x)
		    && (maxY <= x) && (minX >= y) && (maxX <= y)) {
      /**************/

			flag_obstacle = walkboxType[computedVar14];

      /**************/
		}

		di = di->field_0;
	} while (1);

	flag_obstacle = 0;
}

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

			int newFrame = ABS(raoul_end[direction][0]) - 1;

			int zoom = computeZoom(params.Y);

			if (raoul_end[direction][0] < 0) {
				zoom = -zoom;
			}

			checkCollisionWithWalkBoxesBoundingBoxes(params.X, params.Y);

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

	colorOfSelectedSaveDrive = (uint8) popVar();
	video2 = (uint8) popVar();
	video3 = (uint8) popVar();
	video4 = (uint8) popVar();

	return 0;
}

int16 Op_1E(void) {		// setup actor position
	actorStruct *pActor;

	var0 = popVar();
	int actorY = popVar();
	int actorX = popVar();
	var1 = popVar();
	var2 = popVar();
	int overlay = popVar();

	if (!overlay) {
		overlay = currentScriptPtr->overlayNumber;
	}

	pActor = findActor(overlay, var2, &actorHead, var1);

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

int16 Op_45(void) {
	printf("Partial op 45 stop sound\n");

	return 0;
}

int16 Op_AddCellC(void) {
	popPtr();
	popVar();

	printf("Partial op 5C\n");

	return 0;
}

int16 Op_AddCellE(void) {
	popVar();

	printf("Partial op 5E (sound related)\n");

	return 0;
}

int16 Op_3E(void) {
	printf("Partial op 3E (sound related)\n");

	return 0;
}

void setVar49Value(int value) {
	flagCt = value;
}

int16 Op_3A(void) {
	setVar49Value(1);
	return 0;
}

int16 Op_3B(void) {
	setVar49Value(0);
	return 0;
}

int16 Op_3F(void) {
	printf("Partial op 3F (sound related)\n");
	return 0;
}

int16 Op_40(void) {
	printf("Partial op 40 (sound related)\n");
	//freeStuff1();
	freeStuff2();

	var24 = 0;
	var23 = 0;
	return 0;
}

int16 Op_6C(void) {
	//int var0;
	//int var1;
	int temp;

	var0 = popVar();
	var1 = popVar();

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

void Op_60Sub(int overlayIdx, actorStruct * pActorHead, int _var0, int _var1,
	    int _var2, int _var3) {
	actorStruct *pActor = findActor(overlayIdx, _var0, pActorHead, _var3);

	if (pActor) {
		if ((pActor->freeze == _var2) || (_var2 == -1)) {
			pActor->freeze = _var1;
		}
	}
}

int16 Op_60(void) {
	/*
	 * int var0;
	 * int var1;
	 * int var2;
	 * int var3;
	 * int var4;
	 */

	var0 = popVar();
	var1 = popVar();
	var2 = popVar();
	var3 = popVar();
	var4 = popVar();

	if (!var4) {
		var4 = currentScriptPtr->overlayNumber;
	}

	Op_60Sub(var4, &actorHead, var3, var0, var1, var2);

	return 0;
}

int16 Op_6F(void) {
	int numArgs = popVar();

	assert(numArgs == 0);

	{
		popVar();
		char *string = (char *)popPtr();

		printf("partial opcode 6F sprintf (%s)\n", string);
	}

	return 0;
}

int16 Op_6E(void) {
	char *ptr0 = (char *)popPtr();
	char *ptr1 = (char *)popPtr();

	printf("partial opcode 6E (%s)(%s)\n", ptr0, ptr1);

	return 0;
}

int16 Op_InitializeState2(void) {
	var0 = popVar();
	char *ptr = (char *)popPtr();
	var1 = popVar();

	if (!var1)
		var1 = currentScriptPtr->overlayNumber;

	return getProcParam(var1, var0, (uint8 *) ptr);
}

int16 Op_2A(void) {
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

	return getProcParam(overlayIdx, 40, (uint8 *) var_26);
}

void setupOpcodeTable(void) {
	int i;

	for (i = 0; i < 256; i++) {
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
	opcodeTablePtr[0x15] = NULL;	// user wait
	opcodeTablePtr[0x16] = Op_FreezeCell;
	opcodeTablePtr[0x17] = Op_LoadCt;
	opcodeTablePtr[0x18] = Op_AddAnimation;
	opcodeTablePtr[0x19] = Op_RemoveAnimation;
	opcodeTablePtr[0x1A] = Op_SetZoom;
	opcodeTablePtr[0x1E] = Op_1E;
	opcodeTablePtr[0x21] = Op_21;
	opcodeTablePtr[0x22] = Op_22;
	opcodeTablePtr[0x24] = Op_SetStringColors;
	opcodeTablePtr[0x28] = Op_ChangeSaveAllowedState;
	opcodeTablePtr[0x29] = Op_freeAllPerso;
	opcodeTablePtr[0x2A] = Op_2A;
	opcodeTablePtr[0x2B] = Op_2B;
	opcodeTablePtr[0x2C] = Op_2C;
	opcodeTablePtr[0x2E] = Op_releaseOverlay;
	opcodeTablePtr[0x2F] = Op_AddBackgroundIncrust;
	opcodeTablePtr[0x30] = Op_RemoveBackgroundIncrust;
	opcodeTablePtr[0x32] = Op_freeBackgroundInscrustList;
	opcodeTablePtr[0x37] = Op_37;
	opcodeTablePtr[0x38] = Op_removeBackground;
	opcodeTablePtr[0x39] = Op_SetActiveBackgroundPlane;
	opcodeTablePtr[0x3A] = Op_3A;
	opcodeTablePtr[0x3B] = Op_3B;
	opcodeTablePtr[0x3C] = Op_rand;
	opcodeTablePtr[0x3D] = Op_loadMusic;
	opcodeTablePtr[0x3E] = Op_3E;
	opcodeTablePtr[0x3F] = Op_3F;
	opcodeTablePtr[0x40] = Op_40;
	opcodeTablePtr[0x41] = Op_isFileLoaded2;
	opcodeTablePtr[0x45] = Op_45;
	opcodeTablePtr[0x54] = Op_SetFontFileIndex;
	opcodeTablePtr[0x56] = Op_changeCutSceneState;
	opcodeTablePtr[0x57] = Op_GetMouseX;
	opcodeTablePtr[0x58] = Op_GetMouseY;
	opcodeTablePtr[0x59] = Op_GetMouseClick3;
	opcodeTablePtr[0x5A] = Op_isFileLoaded;
	opcodeTablePtr[0x5B] = Op_regenerateBackgroundIncrust;
	opcodeTablePtr[0x5C] = Op_AddCellC;
	opcodeTablePtr[0x5E] = Op_AddCellE;
	opcodeTablePtr[0x60] = Op_60;
	opcodeTablePtr[0x61] = Op_61;
	opcodeTablePtr[0x62] = Op_62;
	opcodeTablePtr[0x63] = Op_63;
	opcodeTablePtr[0x64] = Op_startScript;
	opcodeTablePtr[0x65] = Op_AutoCell;
	opcodeTablePtr[0x66] = Op_66;
	opcodeTablePtr[0x67] = Op_loadAudioResource;
	opcodeTablePtr[0x68] = Op_freeMediumVar;
	opcodeTablePtr[0x6A] = Op_6A;
	opcodeTablePtr[0x6B] = Op_loadFile;
	opcodeTablePtr[0x6C] = Op_6C;
	opcodeTablePtr[0x6D] = Op_strcpy;
	opcodeTablePtr[0x6E] = Op_6E;
	opcodeTablePtr[0x6F] = Op_6F;
	opcodeTablePtr[0x70] = Op_comment;
	opcodeTablePtr[0x71] = Op_SetColorrawLine;
	opcodeTablePtr[0x72] = Op_InitializeState2;
	opcodeTablePtr[0x74] = Op_GetInitVar1;
	opcodeTablePtr[0x76] = Op_InitializeState6;
	opcodeTablePtr[0x79] = Op_PlayFXnterPlayerMenu;
	opcodeTablePtr[0x78] = Op_InitializeState8;
	opcodeTablePtr[0x7B] = Op_InitializeStateB;
	opcodeTablePtr[0x7C] = Op_InitializeStateC;
	opcodeTablePtr[0x7D] = Op_freeAllMenu;
	// TODO: copy the opcodes here
}

int32 opcodeType8(void) {
	int opcode = getByteFromScript();

	if (!opcode)
		return (-21);

	if (opcode > 0x100)
		return (-21);

	if (opcodeTablePtr[opcode]) {
		//printf("Function: %X\n",opcode);
		pushVar(opcodeTablePtr[opcode] ());
		return (0);
	} else {
		printf("Unsupported opcode %X in opcode type 8\n", opcode);
		// exit(1);
	}

	return 0;

}

} // End of namespace Cruise
