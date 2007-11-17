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


#include "common/endian.h"
#include "common/stream.h"

#include "cine/cine.h"
#include "cine/main_loop.h"
#include "cine/object.h"
#include "cine/various.h"
#include "cine/bg_list.h"

namespace Cine {

uint32 var8;
BGIncrustList *bgIncrustList;

void addToBGList(int16 objIdx, bool addList) {
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

	if (g_cine->getGameType() == GType_OS) {
		drawSpriteRaw2(animDataTable[objectTable[objIdx].frame].ptr1, objectTable[objIdx].part, width, height, page2Raw, x, y);
	} else {
		drawSpriteRaw(animDataTable[objectTable[objIdx].frame].ptr1, animDataTable[objectTable[objIdx].frame].ptr2, width, height, page2Raw, x, y);
	}

	if (addList)
		createBgIncrustListElement(objIdx, 0);
}

void addSpriteFilledToBGList(int16 objIdx, bool addList) {
	int16 x;
	int16 y;
	int16 width;
	int16 height;

	x = objectTable[objIdx].x;
	y = objectTable[objIdx].y;

	width = animDataTable[objectTable[objIdx].frame].width;
	height = animDataTable[objectTable[objIdx].frame].height;

	if (animDataTable[objectTable[objIdx].frame].ptr1) {
		gfxFillSprite(animDataTable[objectTable[objIdx].frame].ptr1, width / 2, height, page2Raw, x, y);
	}

	if (addList)
		createBgIncrustListElement(objIdx, 1);
}

void createBgIncrustListElement(int16 objIdx, int16 param) {
	BGIncrustList *bgIncrustPtr = bgIncrustList;
	BGIncrustList *bgIncrustPtrP = 0;

	// Find first empty element
	while (bgIncrustPtr) {
		bgIncrustPtrP = bgIncrustPtr;
		bgIncrustPtr = bgIncrustPtr->next;
	}

	bgIncrustPtr = (BGIncrustList *)malloc(sizeof(BGIncrustList));
	if (bgIncrustPtrP)
		bgIncrustPtrP->next = bgIncrustPtr;
	else
		bgIncrustList = bgIncrustPtr;

	bgIncrustPtr->next = 0;

	bgIncrustPtr->objIdx = objIdx;
	bgIncrustPtr->param = param;
    bgIncrustPtr->x = objectTable[objIdx].x;
    bgIncrustPtr->y = objectTable[objIdx].y;
    bgIncrustPtr->frame = objectTable[objIdx].frame;
    bgIncrustPtr->part = objectTable[objIdx].part;
}

void freeBgIncrustList(void) {
	BGIncrustList *bgIncrustPtr = bgIncrustList;
	BGIncrustList *bgIncrustPtrN;

	while (bgIncrustPtr) {
		bgIncrustPtrN = bgIncrustPtr->next;
		free(bgIncrustPtr);
		bgIncrustPtr = bgIncrustPtrN;
	}

	resetBgIncrustList();
}

void resetBgIncrustList(void) {
	bgIncrustList = NULL;
	var8 = 0;
}

void loadBgIncrustFromSave(Common::InSaveFile *fHandle) {
	BGIncrustList *bgIncrustPtr = bgIncrustList;
	BGIncrustList *bgIncrustPtrP = 0;

	// Find first empty element
	while (bgIncrustPtr) {
		bgIncrustPtrP = bgIncrustPtr;
		bgIncrustPtr = bgIncrustPtr->next;
	}

	bgIncrustPtr = (BGIncrustList *)malloc(sizeof(BGIncrustList));
	if (bgIncrustPtrP)
		bgIncrustPtrP->next = bgIncrustPtr;
	else
		bgIncrustList = bgIncrustPtr;

	bgIncrustPtr->next = 0;

	fHandle->readUint32BE();
	fHandle->readUint32BE();

	bgIncrustPtr->objIdx = fHandle->readUint16BE();
	bgIncrustPtr->param = fHandle->readUint16BE();
    bgIncrustPtr->x = fHandle->readUint16BE();
    bgIncrustPtr->y = fHandle->readUint16BE();
    bgIncrustPtr->frame = fHandle->readUint16BE();
    bgIncrustPtr->part = fHandle->readUint16BE();
}

void reincrustAllBg(void) {
	BGIncrustList *bgIncrustPtr = bgIncrustList;

	while (bgIncrustPtr) {
#if 0
		objectTable[bgIncrustPtr->objIdx].x = bgIncrustPtr->x;
		objectTable[bgIncrustPtr->objIdx].y = bgIncrustPtr->y;
		objectTable[bgIncrustPtr->objIdx].frame = bgIncrustPtr->frame;
		objectTable[bgIncrustPtr->objIdx].part = bgIncrustPtr->part;
#endif
		if (bgIncrustPtr->param == 0) {
			addToBGList(bgIncrustPtr->objIdx, false);
		} else {
			addSpriteFilledToBGList(bgIncrustPtr->objIdx, false);
		}

		bgIncrustPtr = bgIncrustPtr->next;
	}
}

} // End of namespace Cine
