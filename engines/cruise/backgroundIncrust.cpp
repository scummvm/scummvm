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

#include "cruise/cruise_main.h"

namespace Cruise {

backgroundIncrustStruct backgroundIncrustHead;

void resetBackgroundIncrustList(backgroundIncrustStruct *pHead) {
	pHead->next = NULL;
	pHead->prev = NULL;
}

// blit background to another one
void addBackgroundIncrustSub1(int fileIdx, int X, int Y, char *ptr2,
	    int16 scale, char *destBuffer, char *dataPtr) {
	if (*dataPtr == 0) {
		ASSERT(0);
	}

	buildPolyModel(X, Y, scale, ptr2, destBuffer, dataPtr);
}

backgroundIncrustStruct *addBackgroundIncrust(int16 overlayIdx,	int16 objectIdx, backgroundIncrustStruct *pHead, int16 scriptNumber, int16 scriptOverlay, int16 backgroundIdx, int16 param4) {
	uint8 *backgroundPtr;
	uint8 *ptr;
	objectParamsQuery params;
	backgroundIncrustStruct *newElement;
	backgroundIncrustStruct *currentHead;
	backgroundIncrustStruct *currentHead2;

	getMultipleObjectParam(overlayIdx, objectIdx, &params);

	ptr = filesDatabase[params.fileIdx].subData.ptr;

	if (!ptr) {
		return NULL;
	}

	if (filesDatabase[params.fileIdx].subData.resourceType != 4
	    && filesDatabase[params.fileIdx].subData.resourceType != 8) {
		return NULL;
	}

	backgroundPtr = backgroundPtrtable[backgroundIdx];

	if (!backgroundPtr) {
		ASSERT(0);
		return NULL;
	}

	currentHead = pHead;
	currentHead2 = currentHead->next;

	while (currentHead2) {
		currentHead = currentHead2;
		currentHead2 = currentHead->next;
	}

	newElement =
	    (backgroundIncrustStruct *)
	    mallocAndZero(sizeof(backgroundIncrustStruct));

	if (!newElement)
		return NULL;

	newElement->next = currentHead->next;
	currentHead->next = newElement;

	if (!currentHead2) {
		currentHead2 = pHead;
	}

	newElement->prev = currentHead2->prev;
	currentHead2->prev = newElement;

	newElement->objectIdx = objectIdx;
	newElement->type = param4;
	newElement->backgroundIdx = backgroundIdx;
	newElement->overlayIdx = overlayIdx;
	newElement->scriptNumber = scriptNumber;
	newElement->scriptOverlayIdx = scriptOverlay;
	newElement->X = params.X;
	newElement->Y = params.Y;
	newElement->scale = params.scale;
	newElement->field_E = params.fileIdx;
	newElement->aniX = filesDatabase[params.fileIdx].subData.index;
	newElement->ptr = NULL;
	strcpy(newElement->name, filesDatabase[params.fileIdx].subData.name);

	if (filesDatabase[params.fileIdx].subData.resourceType == 4) {	// sprite
		int width = filesDatabase[params.fileIdx].width;
		int height = filesDatabase[params.fileIdx].height;

		drawSprite(width, height, NULL, (char *)filesDatabase[params.fileIdx].subData.ptr, newElement->Y, newElement->X, (char *)backgroundPtr, (char *)filesDatabase[params.fileIdx].subData.ptrMask);
		//   ASSERT(0);
	} else {			// poly
		/* if (param4 == 1)
		 * {
		 * int var_A;
		 * int var_8;
		 * int var_6;
		 * char* var_10;
		 * 
		 * mainDrawSub1Sub1(lvar[3], newElement->X, newElement->Y, &var_A, &var_8, &var_6, &var_10, lvar[4], filesDatabase[lvar[3]].subData.ptr);
		 * ASSERT(0);
		 * } */

		addBackgroundIncrustSub1(params.fileIdx, newElement->X, newElement->Y, NULL, params.scale, (char *)backgroundPtr, (char *)filesDatabase[params.fileIdx].subData.ptr);
	}

	return newElement;
}

void loadBackgroundIncrustFromSave(Common::File& currentSaveFile) {
	int16 numEntry;
	backgroundIncrustStruct *ptr1;
	backgroundIncrustStruct *ptr2;
	int32 i;

	numEntry = currentSaveFile.readSint16LE();

	ptr1 = &backgroundIncrustHead;
	ptr2 = &backgroundIncrustHead;

	for (i = 0; i < numEntry; i++) {
		backgroundIncrustStruct *current = (backgroundIncrustStruct *)mallocAndZero(sizeof(backgroundIncrustStruct));

		currentSaveFile.skip(2);
		currentSaveFile.skip(2);
		
		current->objectIdx = currentSaveFile.readSint16LE();
		current->type = currentSaveFile.readSint16LE();
		current->overlayIdx = currentSaveFile.readSint16LE();
		current->X = currentSaveFile.readSint16LE();
		current->Y = currentSaveFile.readSint16LE();
		current->field_E = currentSaveFile.readSint16LE();
		current->scale = currentSaveFile.readSint16LE();
		current->backgroundIdx = currentSaveFile.readSint16LE();
		current->scriptNumber = currentSaveFile.readSint16LE();
		current->scriptOverlayIdx = currentSaveFile.readSint16LE();
		currentSaveFile.skip(4);
		current->field_1C = currentSaveFile.readSint32LE();
		current->size = currentSaveFile.readSint16LE();
		current->field_22 = currentSaveFile.readSint16LE();
		current->field_24 = currentSaveFile.readSint16LE();
		currentSaveFile.read(current->name, 13);
		currentSaveFile.skip(1);
		current->aniX = currentSaveFile.readSint16LE();
		currentSaveFile.skip(2);

		if (current->size) {
			current->ptr = (uint8 *) mallocAndZero(current->size);
			currentSaveFile.read(current->ptr, current->size);
		}

		current->next = NULL;
		ptr2 = current;
		current->prev = backgroundIncrustHead.prev;
		backgroundIncrustHead.prev = current;
		ptr2 = current->next;
	}
}

void regenerateBackgroundIncrust(backgroundIncrustStruct *pHead) {
	printf("Need to regenerate backgroundIncrust\n");
}

void freeBackgroundIncrustList(backgroundIncrustStruct *pHead) {
	backgroundIncrustStruct *pCurrent = pHead->next;

	while (pCurrent) {
		backgroundIncrustStruct *pNext = pCurrent->next;

		if (pCurrent->ptr) {
			free(pCurrent->ptr);
		}

		free(pCurrent);

		pCurrent = pNext;
	}

	resetBackgroundIncrustList(pHead);
}

void removeBackgroundIncrust(int overlay, int idx, backgroundIncrustStruct * pHead) {
	objectParamsQuery params;
	int var_4;
	int var_6;

	backgroundIncrustStruct *pCurrent;
	backgroundIncrustStruct *pCurrentHead;

	getMultipleObjectParam(overlay, idx, &params);

	var_4 = params.X;
	var_6 = params.Y;

	pCurrent = pHead->next;

	while (pCurrent) {
		if ((pCurrent->overlayIdx == overlay || overlay == -1) &&
		    (pCurrent->objectIdx == idx || idx == -1) &&
		    (pCurrent->X == var_4) && (pCurrent->Y == var_6)) {
			pCurrent->type = - 1;
		}

		pCurrent = pCurrent->next;
	}

	pCurrentHead = pHead;
	pCurrent = pHead->next;

	while (pCurrent) {
		if (pCurrent->type == - 1) {
			backgroundIncrustStruct *pNext = pCurrent->next;
			backgroundIncrustStruct *bx = pCurrentHead;
			backgroundIncrustStruct *cx;

			bx->next = pNext;
			cx = pNext;

			if (!pNext) {
				cx = pHead;
			}

			bx = cx;
			bx->prev = pCurrent->next;

			if (pCurrent->ptr) {
				free(pCurrent->ptr);
			}

			free(pCurrent);

			pCurrent = pNext;
		} else {
			pCurrentHead = pCurrent;
			pCurrent = pCurrent->next;
		}
	}
}

} // End of namespace Cruise
