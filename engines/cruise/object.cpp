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
 * $URL:https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/branches/gsoc2007-fsnode/engines/cruise/object.cpp $
 * $Id:object.cpp 26949 2007-05-26 20:23:24Z david_corrales $
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

	switch (ptr->type) {
	case 0:
		{
			ptr2 = &ovlData->objData2SourceTable[ptr->var5];

			var_14 = globalVars[*(int16 *) (&overlayTable[overlayIdx].state + ptr->stateTableIdx)];

			var_A = ptr2->state;

			break;
		}
	case 1:
		{
			ptr2 = &ovlData->objData2WorkTable[ptr->var4];

			var_A = var_14 = ptr2->state;
			size = var_A + ptr->var5;

			if (ptr->var5 + var_14 <= ovlData->size8) {
				var_A = ovlData->objData2SourceTable[ptr->var5 + var_14].state;
			}
			break;
		}
	default:
		{
			printf("unsupported case %d in getMultipleObjectParam\n", ptr->type);
			exit(1);
		}
	}

	returnParam->X = ptr2->X;
	returnParam->Y = ptr2->Y;
	returnParam->baseFileIdx = ptr2->Z;
	returnParam->fileIdx = ptr2->frame;
	returnParam->scale = ptr2->scale;
	returnParam->var5 = var_14;
	returnParam->var6 = var_A;
	returnParam->var7 = ptr->var3;

	return 0;
}

void setObjectPosition(int16 ovlIdx, int16 objIdx, int16 param3, int16 param4) {
	objDataStruct *ptr;
	objectParams *ptr2;

	ptr = getObjectDataFromOverlay(ovlIdx, objIdx);

	if (!ptr) {
		return;
		ASSERT(0);
	}
	//overlayTable[param1].ovlData

	switch (ptr->type) {
	case 1:
		{
			ptr2 =  &overlayTable[ovlIdx].ovlData->objData2WorkTable[ptr->var4];

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
			case 2:	// z
				{
					ptr2->Z = param4;
					sortCells(ovlIdx, objIdx, &cellHead);
					break;
				}
			case 3:
				{
					ptr2->frame = param4;
					break;
				}
			case 4:	// scale
				{
					ptr2->scale = param4;
					break;
				}
			case 5:	// box colision
				{
					ptr2->state = param4;
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

int16 objInit(int ovlIdx, int objIdx, int newState) {
	objDataStruct *ptr;
//  uint16 param;
	ovlDataStruct *ovlData;

	ptr = getObjectDataFromOverlay(ovlIdx, objIdx);

	if (!ptr)
		return -11;

	ovlData = overlayTable[ovlIdx].ovlData;

	switch (ptr->type) {
	case THEME:
	case MULTIPLE:
		{
			globalVars[overlayTable[ovlIdx].state + ptr->stateTableIdx] = newState;
			sortCells(ovlIdx, objIdx, &cellHead);
			break;
		}
	case UNIQUE:
		break;
	case VARIABLE:
		{
			objectParams *destEntry;
			objectParams *sourceEntry;

			if (ptr->var5 + newState > ovlData->size8) {
				return 0;
			}

			destEntry = &ovlData->objData2WorkTable[ptr->var4];
			sourceEntry = &ovlData->objData2SourceTable[ptr->var5 + newState];

			memcpy(destEntry, sourceEntry, sizeof(objectParams));

			destEntry->state = newState;

			sortCells(ovlIdx, objIdx, &cellHead);
			break;
		}
	default:
		{
			printf("Unsupported param = %d in objInit\n", ptr->type);
			// exit(1);
		}
	}

	return 0;
}

int16 getSingleObjectParam(int16 overlayIdx, int16 param2, int16 param3, int16 *returnParam) {
	int var_A = 0;
	//char* ptr3 = NULL;
	objDataStruct *ptr;
	ovlDataStruct *ovlData;
	objectParams *ptr2;

	ptr = getObjectDataFromOverlay(overlayIdx, param2);

	if (!ptr)
		return -11;

	ovlData = overlayTable[overlayIdx].ovlData;

	switch (ptr->type) {
	case 0:
	case 3:
		{
			var_A = globalVars[ptr->stateTableIdx];

			ptr2 = &ovlData->objData2SourceTable[ptr->var5];
			break;
		}
	case 1:
		{
			ptr2 = &ovlData->objData2WorkTable[ptr->var4];

			var_A = ptr2->state;
			break;
		}
	default:
		{
			printf("Unsupported case %d in getSingleObjectParam\n",ptr->type);
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
			*returnParam = ptr2->Z;
			break;
		}
	case 3:
		{
			*returnParam = ptr2->frame;
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
			printf("Unsupported case %d in getSingleObjectParam case 1\n", param3);
			exit(1);
		}
	}

	return 0;
}

} // End of namespace Cruise
