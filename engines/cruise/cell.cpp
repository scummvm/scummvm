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

void freeMessageList(cellStruct *objPtr) {
/*	if (objPtr) {
		 if(objPtr->next)
		 free(objPtr->next);

		free(objPtr);
	} */
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

cellStruct *addCell(cellStruct *pHead, int16 overlayIdx, int16 objIdx, int16 type, int16 backgroundPlane, int16 scriptOverlay, int16 scriptNumber, int16 scriptType) {
	int16 var;

	cellStruct *newElement;
	cellStruct *currentHead = pHead;
	cellStruct *currentHead2;
	cellStruct *currentHead3;

	if (getSingleObjectParam(overlayIdx, objIdx, 2, &var) < 0) {
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

			getSingleObjectParam(currentHead2->overlay, currentHead2->idx, 2, &lvar2);

			if (lvar2 > var)
				break;
		}

		currentHead3 = currentHead2;
		currentHead2 = currentHead2->next;
	}

	if (currentHead2) {
		if ((currentHead2->overlay == overlayIdx) &&
		    (currentHead2->backgroundPlane == backgroundPlane) &&
		    (currentHead2->idx == objIdx) &&
		    (currentHead2->type == type))

			return NULL;
	}

	currentHead = currentHead2;

	newElement = (cellStruct *) mallocAndZero(sizeof(cellStruct));

	if (!newElement)
		return 0;

	newElement->next = currentHead3->next;
	currentHead3->next = newElement;

	newElement->idx = objIdx;
	newElement->type = type;
	newElement->backgroundPlane = backgroundPlane;
	newElement->overlay = overlayIdx;
	newElement->freeze = 0;
	newElement->parent = scriptNumber;
	newElement->parentOverlay = scriptOverlay;
	newElement->gfxPtr = NULL;
	newElement->followObjectIdx = objIdx;
	newElement->followObjectOverlayIdx = overlayIdx;
	newElement->parentType = scriptType;

	newElement->animStart = 0;
	newElement->animEnd = 0;
	newElement->animWait = 0;
	newElement->animSignal = 0;
	newElement->animCounter = 0;
	newElement->animType = 0;
	newElement->animStep = 0;
	newElement->animLoop = 0;

	if (currentHead) {
		newElement->prev = currentHead->prev;
		currentHead->prev = newElement;
	} else {
		newElement->prev = pHead->prev;
		pHead->prev = newElement;
	}

	return newElement;
}

void createTextObject(int overlayIdx, int oldVar8, cellStruct *pObject, int scriptNumber, int scriptOverlayNumber, int backgroundPlane, int16 color, int oldVar2, int oldVar4, int oldVar6) {

	char *ax;
	cellStruct *savePObject = pObject;
	cellStruct *cx;

	cellStruct *pNewElement;
	cellStruct *si = pObject->next;
	cellStruct *var_2;

	while (si) {
		pObject = si;
		si = si->next;
	}

	var_2 = si;

	pNewElement = (cellStruct *) malloc(sizeof(cellStruct));

	pNewElement->next = pObject->next;
	pObject->next = pNewElement;

	pNewElement->idx = oldVar8;
	pNewElement->type = 5;
	pNewElement->backgroundPlane = backgroundPlane;
	pNewElement->overlay = overlayIdx;
	pNewElement->x = oldVar6;
	pNewElement->field_C = oldVar4;
	pNewElement->spriteIdx = oldVar2;
	pNewElement->color = color;
	pNewElement->freeze = 0;
	pNewElement->parent = scriptNumber;
	pNewElement->parentOverlay = scriptOverlayNumber;
	pNewElement->gfxPtr = NULL;

	if (var_2) {
		cx = var_2;
	} else {
		cx = savePObject;
	}

	pNewElement->prev = cx->prev;
	cx->prev = pNewElement;

	ax = getText(oldVar8, overlayIdx);

	if (ax) {
		pNewElement->gfxPtr = renderText(oldVar2, (uint8 *) ax);
	}
}

void removeCell(cellStruct *objPtr, int ovlNumber, int objectIdx, int objType, int backgroundPlane ) {
	cellStruct *currentObj = objPtr->next;
	cellStruct *previous;

	while (currentObj) {
		if (((currentObj->overlay == ovlNumber) || (ovlNumber == -1)) &&
		    ((currentObj->idx == objectIdx) || (objectIdx == -1)) &&
		    ((currentObj->type == objType) || (objType == -1)) &&
		    ((currentObj->backgroundPlane == backgroundPlane) || (backgroundPlane == -1))) {
			currentObj->type = -1;
		}

		currentObj = currentObj->next;
	}

	previous = objPtr;
	currentObj = objPtr->next;

	while (currentObj) {
		cellStruct *si;

		si = currentObj;

		if (si->type == -1) {
			cellStruct *dx;
			previous->next = si->next;

			dx = si->next;

			if (!si->next) {
				dx = objPtr;
			}

			dx->prev = si->prev;

			// TODO: complelty wrong
			//freeMessageList(si);

			free(si);

			currentObj = dx;
		} else {
			currentObj = si->next;
			previous = si;
		}
	}
}

void freezeCell(cellStruct * pObject, int overlayIdx, int objIdx, int objType, int backgroundPlane, int oldFreeze, int newFreeze ) {
	while (pObject) {
		if ((pObject->overlay == overlayIdx) || (overlayIdx == -1)) {
			if ((pObject->idx == objIdx) || (objIdx == -1)) {
				if ((pObject->type == objType) || (objType == -1)) {
					if ((pObject->backgroundPlane == backgroundPlane) || (backgroundPlane == -1)) {
						if ((pObject->freeze == oldFreeze) || (oldFreeze == -1)) {
							pObject->freeze = newFreeze;
						}
					}
				}
			}
		}

		pObject = pObject->next;
	}
}

void sortCells(int16 param1, int16 param2, cellStruct *objPtr) {
	int16 var;
	cellStruct *var8_;
	cellStruct *var40;
	cellStruct *var3E;
	cellStruct *currentObjPtrPrevious;
	cellStruct *currentObjPtr2;
	cellStruct *match;

	getSingleObjectParam(param1, param2, 2, &var);

	currentObjPtrPrevious = objPtr;
	currentObjPtr2 = objPtr->next;

	match = NULL;
	var40 = NULL;
	var3E = NULL;
	var8_ = objPtr;

	while (currentObjPtr2) {
		if ((currentObjPtr2->overlay == param1) && (currentObjPtr2->idx == param2)) {// found
			currentObjPtrPrevious->next = currentObjPtr2->next;

			if (currentObjPtr2->next) {
				currentObjPtr2->next->prev =
				    currentObjPtr2->prev;
			} else {
				objPtr->prev = currentObjPtr2->prev;
			}

			if (var40) {
				var40->prev = currentObjPtr2;
			} else {
				var3E = currentObjPtr2;
			}

			currentObjPtr2->prev = NULL;

			currentObjPtr2->next = var40;

			var40 = currentObjPtr2;

			if (match == NULL) {
				match = currentObjPtr2;
			}
		} else {
			if (currentObjPtr2->type == 5) {
				var2 = 32000;
			} else {
				int16 varC;

				getSingleObjectParam(currentObjPtr2->overlay,
				    currentObjPtr2->idx, 2, &varC);

				var2 = varC;
			}

			if (var > var2) {
				var8_ = currentObjPtr2;
			}

			currentObjPtrPrevious = currentObjPtrPrevious->next;
		}

		currentObjPtr2 = currentObjPtr2->next;
	}

	if (match) {
		cellStruct *temp;

		temp = var8_->next;

		var8_->next = var40;
		match->next = temp;

		if (objPtr != var8_) {
			var40->prev = var8_;
		}

		if (!temp) {
			temp = match;
		}

		temp->prev = match;
	}
}

} // End of namespace Cruise
