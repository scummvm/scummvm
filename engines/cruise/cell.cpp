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

#include "cruise/cell.h"
#include "cruise/cruise_main.h"

namespace Cruise {

void resetPtr(cellStruct *ptr) {
	ptr->next = NULL;
	ptr->prev = NULL;
}

void loadSavegameDataSub2(FILE *f) {
	unsigned short int n_chunks;
	int i;
	cellStruct *p;
	cellStruct *t;

	cellHead.next = NULL;	// Not in ASM code, but I guess the variable is defaulted
	// to this value in the .exe

	fread(&n_chunks, 2, 1, f);
	// BIG ENDIAN MACHINES, PLEASE SWAP IT

	p = &cellHead;

	for (i = 0; i < n_chunks; i++) {
		t = (cellStruct *) mallocAndZero(sizeof(cellStruct));

		fseek(f, 4, SEEK_CUR);
		fread(&t->idx, 1, 0x30, f);

		t->next = NULL;
		p->next = t;
		t->prev = cellHead.prev;
		cellHead.prev = t;
		p = t;
	}
}

cellStruct *addCell(int16 overlayIdx, int16 param2, cellStruct *pHead,
	    int16 scriptType, int16 scriptNumber, int16 scriptOverlay, int16 param3,
    	int16 param4) {
	int16 var;

	cellStruct *newElement;
	cellStruct *currentHead = pHead;
	cellStruct *currentHead2;
	cellStruct *currentHead3;

	if (getSingleObjectParam(overlayIdx, param2, 2, &var) < 0) {
		return 0;
	}

	currentHead3 = currentHead;
	currentHead2 = currentHead->next;

	while (currentHead2) {
		if (currentHead2->type == 3) {
			break;
		}

		if (currentHead2->type != 5) {
			int16 lvar2;

			getSingleObjectParam(currentHead2->overlay,
			    currentHead2->idx, 2, &lvar2);

			if (lvar2 > var)
				break;
		}

		currentHead3 = currentHead2;
		currentHead2 = currentHead2->next;
	}

	if (currentHead2) {
		if ((currentHead2->overlay == overlayIdx) &&
		    (currentHead2->backgroundPlane == param3) &&
		    (currentHead2->idx == param2) &&
		    (currentHead2->type == param4))

			return NULL;
	}

	currentHead = currentHead2;

	newElement = (cellStruct *) mallocAndZero(sizeof(cellStruct));

	if (!newElement)
		return 0;

	newElement->next = currentHead3->next;
	currentHead3->next = newElement;

	newElement->idx = param2;
	newElement->type = param4;
	newElement->backgroundPlane = param3;
	newElement->overlay = overlayIdx;
	newElement->freeze = 0;
	newElement->field_16 = scriptNumber;
	newElement->field_18 = scriptOverlay;
	newElement->gfxPtr = NULL;
	newElement->followObjectIdx = param2;
	newElement->followObjectOverlayIdx = overlayIdx;
	newElement->field_1A = scriptType;
	newElement->field_20 = 0;
	newElement->field_22 = 0;
	newElement->nextAnimDelay = 0;
	newElement->field_2C = 0;
	newElement->currentAnimDelay = 0;
	newElement->field_2A = 0;
	newElement->animStep = 0;
	newElement->field_30 = 0;

	if (currentHead) {
		newElement->prev = currentHead->prev;
		currentHead->prev = newElement;
	} else {
		newElement->prev = pHead->prev;
		pHead->prev = newElement;
	}

	return newElement;
}

} // End of namespace Cruise
