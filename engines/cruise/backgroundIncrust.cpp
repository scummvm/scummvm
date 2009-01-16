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
void addBackgroundIncrustSub1(int fileIdx, int X, int Y, char *ptr2, int16 scale, char *destBuffer, char *dataPtr) {
	if (*dataPtr == 0) {
		ASSERT(0);
	}

	buildPolyModel(X, Y, scale, ptr2, destBuffer, dataPtr);
}

void backupBackground(backgroundIncrustStruct *pIncrust, int X, int Y, int width, int height, uint8* pBackground) {
	pIncrust->saveWidth = width;
	pIncrust->saveHeight = height;
	pIncrust->saveSize = width * height;
	pIncrust->savedX = X;
	pIncrust->savedY = Y;

	pIncrust->ptr = (uint8*)malloc(width * height);
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			pIncrust->ptr[i*width+j] = pBackground[(i+Y)*320+j+X];
		}
	}
}

void restoreBackground(backgroundIncrustStruct *pIncrust) {
	if (pIncrust->type != 1)
		return;
	if (pIncrust->ptr == NULL)
		return;

	uint8* pBackground = backgroundPtrtable[pIncrust->backgroundIdx];
	if (pBackground == NULL)
		return;

	int X = pIncrust->savedX;
	int Y = pIncrust->savedY;
	int width = pIncrust->saveWidth;
	int height = pIncrust->saveHeight;

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			pBackground[(i+Y)*320+j+X] = pIncrust->ptr[i*width+j];
		}
	}
}

backgroundIncrustStruct *addBackgroundIncrust(int16 overlayIdx,	int16 objectIdx, backgroundIncrustStruct *pHead, int16 scriptNumber, int16 scriptOverlay, int16 backgroundIdx, int16 saveBuffer) {
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

	if (filesDatabase[params.fileIdx].subData.resourceType != 4 && filesDatabase[params.fileIdx].subData.resourceType != 8) {
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

	newElement = (backgroundIncrustStruct *)mallocAndZero(sizeof(backgroundIncrustStruct));

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
	newElement->type = saveBuffer;
	newElement->backgroundIdx = backgroundIdx;
	newElement->overlayIdx = overlayIdx;
	newElement->scriptNumber = scriptNumber;
	newElement->scriptOverlayIdx = scriptOverlay;
	newElement->X = params.X;
	newElement->Y = params.Y;
	newElement->scale = params.scale;
	newElement->field_E = params.fileIdx;
	newElement->spriteId = filesDatabase[params.fileIdx].subData.index;
	newElement->ptr = NULL;
	strcpy(newElement->name, filesDatabase[params.fileIdx].subData.name);

	if (filesDatabase[params.fileIdx].subData.resourceType == 4) {	// sprite
		int width = filesDatabase[params.fileIdx].width;
		int height = filesDatabase[params.fileIdx].height;

		if (saveBuffer == 1) {
			backupBackground(newElement, newElement->X, newElement->Y, width, height, backgroundPtr);
		}

		drawSprite(width, height, NULL, (char *)filesDatabase[params.fileIdx].subData.ptr, newElement->Y, newElement->X, (char *)backgroundPtr, (char *)filesDatabase[params.fileIdx].subData.ptrMask);
	} else {			// poly
		if (saveBuffer == 1) {
			int newX;
			int newY;
			int newScale;
			char *newFrame;

			int sizeTable[4];	// 0 = left, 1 = right, 2 = bottom, 3 = top

			// this function checks if the dataPtr is not 0, else it retrives the data for X, Y, scale and DataPtr again (OLD: mainDrawSub1Sub1)
			flipPoly(params.fileIdx, (int16*)filesDatabase[params.fileIdx].subData.ptr, params.scale, &newFrame, newElement->X, newElement->Y, &newX, &newY, &newScale);

			// this function fills the sizeTable for the poly (OLD: mainDrawSub1Sub2)
			getPolySize(newX, newY, newScale, sizeTable, (unsigned char*)newFrame);

			int width = (sizeTable[1] + 2) - (sizeTable[0] - 2) + 1;
			int height = sizeTable[3] - sizeTable[2] + 1;

			backupBackground(newElement, sizeTable[0] - 2, sizeTable[2], width, height, backgroundPtr);
		}

		addBackgroundIncrustSub1(params.fileIdx, newElement->X, newElement->Y, NULL, params.scale, (char *)backgroundPtr, (char *)filesDatabase[params.fileIdx].subData.ptr);
	}

	return newElement;
}

void saveIncrust(Common::OutSaveFile& currentSaveFile) {
	int count = 0;

	backgroundIncrustStruct *pl = backgroundIncrustHead.next;
	while (pl) {
		count++;
		pl = pl->next;
	}

	currentSaveFile.writeSint16LE(count);

	pl = backgroundIncrustHead.next;
	while (pl) {
		char dummy[4] = {0, 0, 0, 0};
		currentSaveFile.write(dummy, 2);
		currentSaveFile.write(dummy, 2);

		currentSaveFile.writeSint16LE(pl->objectIdx);
		currentSaveFile.writeSint16LE(pl->type);
		currentSaveFile.writeSint16LE(pl->overlayIdx);
		currentSaveFile.writeSint16LE(pl->X);
		currentSaveFile.writeSint16LE(pl->Y);
		currentSaveFile.writeSint16LE(pl->field_E);
		currentSaveFile.writeSint16LE(pl->scale);
		currentSaveFile.writeSint16LE(pl->backgroundIdx);
		currentSaveFile.writeSint16LE(pl->scriptNumber);
		currentSaveFile.writeSint16LE(pl->scriptOverlayIdx);
		currentSaveFile.write(dummy, 4);
		currentSaveFile.writeSint16LE(pl->saveWidth / 2);
		currentSaveFile.writeSint16LE(pl->saveHeight);
		currentSaveFile.writeSint16LE(pl->saveSize);
		currentSaveFile.writeSint16LE(pl->savedX);
		currentSaveFile.writeSint16LE(pl->savedY);
		currentSaveFile.write(pl->name, 13);
		currentSaveFile.write(dummy, 1);
		currentSaveFile.writeSint16LE(pl->spriteId);
		currentSaveFile.write(dummy, 2);

		if (pl->saveSize) {
			char* buffer = (char*)malloc(pl->saveSize);
			memset(buffer, 0, pl->saveSize);
			currentSaveFile.write(buffer, pl->saveSize);
			free(buffer);
		}

		pl = pl->next;
	}
}

void loadBackgroundIncrustFromSave(Common::InSaveFile& currentSaveFile) {
	int16 numEntry;
	int32 i;

	numEntry = currentSaveFile.readSint16LE();

	backgroundIncrustStruct *pl = &backgroundIncrustHead;
	backgroundIncrustStruct *pl1 = &backgroundIncrustHead;

	for (i = 0; i < numEntry; i++) {
		backgroundIncrustStruct *pl2 = (backgroundIncrustStruct *)mallocAndZero(sizeof(backgroundIncrustStruct));

		currentSaveFile.skip(2);
		currentSaveFile.skip(2);

		pl2->objectIdx = currentSaveFile.readSint16LE();
		pl2->type = currentSaveFile.readSint16LE();
		pl2->overlayIdx = currentSaveFile.readSint16LE();
		pl2->X = currentSaveFile.readSint16LE();
		pl2->Y = currentSaveFile.readSint16LE();
		pl2->field_E = currentSaveFile.readSint16LE();
		pl2->scale = currentSaveFile.readSint16LE();
		pl2->backgroundIdx = currentSaveFile.readSint16LE();
		pl2->scriptNumber = currentSaveFile.readSint16LE();
		pl2->scriptOverlayIdx = currentSaveFile.readSint16LE();
		currentSaveFile.skip(4);
		pl2->saveWidth = currentSaveFile.readSint16LE() * 2;
		pl2->saveHeight = currentSaveFile.readSint16LE();
		pl2->saveSize = currentSaveFile.readUint16LE();
		pl2->savedX = currentSaveFile.readSint16LE();
		pl2->savedY = currentSaveFile.readSint16LE();
		currentSaveFile.read(pl2->name, 13);
		currentSaveFile.skip(1);
		pl2->spriteId = currentSaveFile.readSint16LE();
		currentSaveFile.skip(2);

		if (pl2->saveSize) {
			/*pl2->ptr = (uint8 *) mallocAndZero(pl2->size);
			currentSaveFile.read(pl2->ptr, pl2->size);*/

			currentSaveFile.skip(pl2->saveSize);

			int width = pl2->saveWidth;
			int height = pl2->saveHeight;
			pl2->ptr = (uint8*)malloc(width * height);
			memset(pl2->ptr, 0, width * height);

			// TODO: convert graphic format here
		}

		pl2->next = NULL;
		pl->next = pl2;

		pl2->prev = pl1->prev;
		pl1->prev = pl2;

		pl = pl2;
	}
}

void regenerateBackgroundIncrust(backgroundIncrustStruct *pHead) {

	lastAni[0] = 0;

	backgroundIncrustStruct* pl = pHead->next;

	while (pl) {
		backgroundIncrustStruct* pl2 = pl->next;

		bool bLoad = false;
		int frame = pl->field_E;
		//int screen = pl->backgroundIdx;

		if ((filesDatabase[frame].subData.ptr == NULL) || (strcmp(pl->name, filesDatabase[frame].subData.name))) {
			frame = 257 - 1;
			if (loadFile(pl->name, frame, pl->spriteId) >= 0) {
				bLoad = true;
			} else {
				frame = -1;
			}
		}

		if (frame >= -1) {
			if (filesDatabase[frame].subData.resourceType == 4) {	// sprite
				int width = filesDatabase[frame].width;
				int height = filesDatabase[frame].height;

				drawSprite(width, height, NULL, (char *)filesDatabase[frame].subData.ptr, pl->Y, pl->X, (char*)backgroundPtrtable[pl->backgroundIdx], (char *)filesDatabase[frame].subData.ptrMask);
			} else {			// poly
				addBackgroundIncrustSub1(frame, pl->X, pl->Y, NULL, pl->scale, (char*)backgroundPtrtable[pl->backgroundIdx], (char *)filesDatabase[frame].subData.ptr);
			}
		}

		pl = pl2;
	}

	lastAni[0] = 0;
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
		if ((pCurrent->overlayIdx == overlay || overlay == -1) && (pCurrent->objectIdx == idx || idx == -1) && (pCurrent->X == var_4) && (pCurrent->Y == var_6)) {
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

void unmergeBackgroundIncrust(backgroundIncrustStruct * pHead, int ovl, int idx) {
	backgroundIncrustStruct *pl;
	backgroundIncrustStruct *pl2;

	objectParamsQuery params;
	getMultipleObjectParam(ovl, idx, &params);

	int x = params.X;
	int y = params.Y;

	pl = pHead;
	pl2 = pl;
	pl = pl2->next;

	while (pl) {
		pl2 = pl;
		if ((pl->overlayIdx == ovl) || (ovl == -1))
			if ((pl->objectIdx == idx) || (idx == -1))
				if ((pl->X == x) && (pl->Y == y))
					restoreBackground(pl);

		pl = pl2->next;
	}

}

} // End of namespace Cruise
