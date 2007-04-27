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

namespace Cruise {

//10 values:
/*

0 = X
1 = Y
3 = fileIdx
4 = scale

*/

objDataStruct *getObjectDataFromOverlay(int ovlIdx, int objIdx) {
	objDataStruct *var_6;

	if (ovlIdx < 1 || objIdx < 0)
		return NULL;

	if (!overlayTable[ovlIdx].ovlData)
		return NULL;

	if (overlayTable[ovlIdx].ovlData->numObjData <= objIdx)
		return NULL;

	var_6 = overlayTable[ovlIdx].ovlData->objDataTable;

	if (!var_6)
		return NULL;

	return (&var_6[objIdx]);
}

int16 getMultipleObjectParam(int16 overlayIdx, int16 objectIdx,
	    objectParamsQuery *returnParam) {
	int16 size;
	int16 var_A;
	int16 var_14;
	objectParams *ptr2;
	objDataStruct *ptr;
	ovlDataStruct *ovlData;
//  int16 type;

	ptr = getObjectDataFromOverlay(overlayIdx, objectIdx);

	if (!ptr)
		return -11;

	ovlData = overlayTable[overlayIdx].ovlData;

	switch (ptr->var1) {
	case 0:
		{
			ptr2 = &ovlData->objData2SourceTable[ptr->var5];

			var_14 = globalVars[*(int16 *) (&overlayTable[overlayIdx].field_14 + ptr->var6)];

			var_A = ptr2->var5;

			break;
		}
	case 1:
		{
			ptr2 = &ovlData->objData2WorkTable[ptr->var4];

			var_A = var_14 = ptr2->var5;
			size = var_A + ptr->var5;

			if (ptr->var5 + var_14 <= ovlData->size8) {
				var_A = ovlData->objData2SourceTable[ptr->var5 + var_14].var5;
			}
			break;
		}
	default:
		{
			printf("unsupported case %d in getMultipleObjectParam\n", ptr->var1);
			exit(1);
		}
	}

	returnParam->X = ptr2->X;
	returnParam->Y = ptr2->Y;
	returnParam->baseFileIdx = ptr2->baseFileIdx;
	returnParam->fileIdx = ptr2->var3;
	returnParam->scale = ptr2->scale;
	returnParam->var5 = var_14;
	returnParam->var6 = var_A;
	returnParam->var7 = ptr->var3;

	return 0;
}

void setObjectPosition(int16 param1, int16 objIdx, int16 param3, int16 param4) {
	objDataStruct *ptr;
	objectParams *ptr2;

	ptr = getObjectDataFromOverlay(param1, objIdx);

	if (!ptr) {
		return;
		ASSERT(0);
	}
	//overlayTable[param1].ovlData

	switch (ptr->var1) {
	case 1:
		{
			ptr2 =  &overlayTable[param1].ovlData->objData2WorkTable[ptr->var4];

			switch (param3) {
			case 0:	// x
				{
					ptr2->X = param4;
					break;
				}
			case 1:	// y
				{
					ptr2->Y = param4;
					break;
				}
			case 2:	// base file
				{
					ptr2->baseFileIdx = param4;
					break;
				}
			case 3:
				{
					ptr2->var3 = param4;
					break;
				}
			case 4:	// scale
				{
					ptr2->scale = param4;
					break;
				}
			case 5:	// box colision
				{
					ptr2->var5 = param4;
					break;
				}
			default:
				{
					ASSERT(0);
				}
			}

			break;
		}
	default:
		{
			ASSERT(0);
		}
	}
}

void Op_InitializeStateSub1(int16 param1, int16 param2, cellStruct *objPtr) {
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

int16 Op_InitializeStateSub(int ovlIdx, int objIdx, int param2) {
	objDataStruct *ptr;
//  uint16 param;
	ovlDataStruct *ovlData;

	ptr = getObjectDataFromOverlay(ovlIdx, objIdx);

	if (!ptr)
		return -11;

	ovlData = overlayTable[ovlIdx].ovlData;

	switch (ptr->var1) {
	case 0:
		{
			globalVars[overlayTable[ovlIdx].field_14 + ptr->var6] =
			    param2;
			Op_InitializeStateSub1(ovlIdx, param2, &cellHead);
			break;
		}
	case 1:
		{
			objectParams *destEntry;
			objectParams *sourceEntry;

			if (ptr->var5 + param2 > ovlData->size8) {
				return 0;
			}

			destEntry = &ovlData->objData2WorkTable[ptr->var4];
			sourceEntry =
			    &ovlData->objData2SourceTable[ptr->var5 + param2];

			memcpy(destEntry, sourceEntry, sizeof(objectParams));

			destEntry->var5 = param2;

			Op_InitializeStateSub1(ovlIdx, param2, &cellHead);
			break;
		}
	default:
		{
			printf
			    ("Unsupported param = %d in Op_InitializeStateSub\n",
			    ptr->var1);
			// exit(1);
		}
	}

	return 0;
}

int16 getSingleObjectParam(int16 overlayIdx, int16 param2, int16 param3,
	    int16 *returnParam) {
	int var_A = 0;
	//char* ptr3 = NULL;
	objDataStruct *ptr;
	ovlDataStruct *ovlData;
	objectParams *ptr2;

	ptr = getObjectDataFromOverlay(overlayIdx, param2);

	if (!ptr)
		return -11;

	ovlData = overlayTable[overlayIdx].ovlData;

	switch (ptr->var1) {
	case 0:
	case 3:
		{
			var_A = globalVars[ptr->var6];

			ptr2 = &ovlData->objData2SourceTable[ptr->var5];
			break;
		}
	case 1:
		{
			ptr2 = &ovlData->objData2WorkTable[ptr->var4];

			var_A = ptr2->var5;
			break;
		}
	default:
		{
			printf("Unsupported case %d in getSingleObjectParam\n",
			    ptr->var1);
			exit(1);
		}
	}

	switch (param3) {
	case 0:
		{
			*returnParam = ptr2->X;
			break;
		}
	case 1:
		{
			*returnParam = ptr2->Y;
			break;
		}
	case 2:
		{
			*returnParam = ptr2->baseFileIdx;
			break;
		}
	case 3:
		{
			*returnParam = ptr2->var3;
			break;
		}
	case 4:
		{
			*returnParam = ptr2->scale;
			break;
		}
	case 5:
		{
			*returnParam = var_A;
			break;
		}
	default:
		{
			printf
			    ("Unsupported case %d in getSingleObjectParam case 1\n",
			    param3);
			exit(1);
		}
	}

	return 0;
}

} // End of namespace Cruise
