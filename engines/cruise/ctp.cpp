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
#include "common/util.h"

namespace Cruise {

ctpVar19Struct *ptr_something;
ctpVar19Struct *polyStruct;
ctpVar19Struct *ctpVar11;
ctpVar19Struct *ctpVar13;
ctpVar19Struct *ctpVar15;

uint8 *ctpVar17;
ctpVar19Struct *ctpVar19;

int currentWalkBoxCenterX;
int currentWalkBoxCenterY;
int currentWalkBoxCenterXBis;
int currentWalkBoxCenterYBis;

int ctpVarUnk;
uint8 walkboxTable[0x12];

int ctpProc2(int varX, int varY, int paramX, int paramY) {
	int diffX = ABS(paramX - varX);
	int diffY = ABS(paramY - varY);

	if (diffX > diffY) {
		diffY = diffX;
	}

	ctpVar14 = diffY;	// highest difference
	return (diffY);
}

// this function process path finding coordinates
void loadCtpSub2(short int coordCount, short int *ptr) {
// coordCount = ctp_routeCoordCount, ptr = ctpVar8
	int i;
	int offset = 0;

	short int *cur_ctp_routeCoords = (short int *)ctp_routeCoords;	// coordinates table
	int8 *cur_ctp_routes = (int8 *) ctp_routes;

	for (i = 0; i < coordCount; i++) {	// for i < ctp_routeCoordCount
		int varX = cur_ctp_routeCoords[0];	// x
		int varY = cur_ctp_routeCoords[1];	// y 

		int di = 0;
		int var4Offset = 2;

		while (*(int16 *) cur_ctp_routes > di) {	// while (coordCount > counter++)
			int idx = *(int16 *) (cur_ctp_routes + var4Offset);
			ptr[offset + idx] =
			    ctpProc2(varX, varY, ctp_routeCoords[idx][0],
			    ctp_routeCoords[idx * 2][1]);

			var4Offset += 2;
			di++;
		}

		offset += 10;
		cur_ctp_routes += 20;
		cur_ctp_routeCoords += 2;
	}
}

void getWalkBoxCenter(int boxIdx, uint16 *_walkboxTable) {
	int minX = 1000;
	int minY = 1000;
	int maxX = -1;
	int maxY = -1;

	ASSERT(boxIdx <= 15);	// max number of walkboxes is 16
	ASSERT(_walkboxTable[boxIdx * 40]);	// we should never have an empty walk box

	if (_walkboxTable[boxIdx * 40] > 0) {
		int numPoints = _walkboxTable[boxIdx * 40];
		uint16 *pCurrentPtr = _walkboxTable + (boxIdx * 40) + 1;

		int i;

		for (i = 0; i < numPoints; i++) {
			int X = *(pCurrentPtr++);
			int Y = *(pCurrentPtr++);;

			if (X < minX)
				minX = X;

			if (X > maxX)
				maxX = X;

			if (Y < minY)
				minY = Y;

			if (Y > maxY)
				maxY = Y;
		}
	}

	currentWalkBoxCenterX = ((maxX - minX) / 2) + minX;
	currentWalkBoxCenterY = ((maxY - minY) / 2) + minY;
}

// ax dx bx
void renderCTPWalkBox(int X1, int Y1, int X2, int scale, int Y2,
	    uint16 *walkboxData) {
	int numPoints;
	int wbSelf1;
	int wbSelf2;
	int i;
	int16 *destination;

	wbSelf1 = upscaleValue(X1, scale) - X2;
	wbSelf2 = upscaleValue(Y1, scale) - Y2;

	numPoints = *(walkboxData++);

	destination = polyBuffer2;

	for (i = 0; i < numPoints; i++) {
		int pointX = *(walkboxData++);
		int pointY = *(walkboxData++);

		int scaledX = upscaleValue(pointX, scale) - wbSelf1;
		int scaledY = upscaleValue(pointY, scale) - wbSelf2;

		*(destination++) = scaledX >> 16;
		*(destination++) = scaledY >> 16;
	}

	m_color = 0;
	ctpVarUnk = 0;

	for (i = 0; i < numPoints; i++) {
		walkboxTable[i] = i;
	}

	drawPolyMode2((char *)walkboxTable, numPoints);
}

// this process the walkboxes
void loadCtpSub1(int boxIdx, int scale, uint16 *_walkboxTable,
	    ctpVar19Struct *param4) {
	int minX = 1000;
	int minY = 1000;
	int maxX = -1;
	int maxY = -1;

	ctpVar19Struct *var_1C;
	ctpVar19Struct *var_12;
	int16 *var_18;
	int16 *si;
	//  int16* di;
	//  uint8* cx;
	//  int bx;
	//  int ax;
	//  int var_2;
	int var_E;
	//int var_C = 1000;
	//int var_A = 0;
	ctpVar19SubStruct *subStruct;

	ASSERT(boxIdx <= 15);

	if (_walkboxTable[boxIdx * 40] > 0) {	// is walkbox used ?
		getWalkBoxCenter(boxIdx, _walkboxTable);

		currentWalkBoxCenterYBis = currentWalkBoxCenterY;
		currentWalkBoxCenterXBis = currentWalkBoxCenterX;
		// + 512
		renderCTPWalkBox(currentWalkBoxCenterX, currentWalkBoxCenterY,
		    currentWalkBoxCenterX, scale + 0x200,
		    currentWalkBoxCenterY, _walkboxTable + boxIdx * 40);

		var_1C = param4;
		var_12 = var_1C + 1;	// next

		var_18 = polyBuffer3;
		var_E = 0;

		si = &polyBuffer3[1];
		/* if(*si>=0)
		 * {
		 * di = si;
		 * cx = var_12;
		 * 
		 * do
		 * {
		 * di++;
		 * bx = di[-1];
		 * ax = di[0];
		 * di++;
		 * 
		 * var_2 = ax;
		 * if(var_C < bx)
		 * {
		 * var_C = bx;
		 * }
		 * 
		 * if(var_2 < var_A)
		 * {
		 * var_A = var_2;
		 * }
		 * 
		 * *cx = bx;
		 * cx++;
		 * *cx = var_2;
		 * cx++;
		 * var_E ++;
		 * }while(di);
		 * 
		 * var_12 = cx;
		 * } */

		/*************/
		{
			int i;
			int numPoints;
			uint16 *pCurrentPtr = _walkboxTable + boxIdx * 40;

			numPoints = *(pCurrentPtr++);

			for (i = 0; i < numPoints; i++) {
				int X = *(pCurrentPtr++);
				int Y = *(pCurrentPtr++);

				if (X < minX)
					minX = X;

				if (X > maxX)
					maxX = X;

				if (Y < minY)
					minY = Y;

				if (Y > maxY)
					maxY = Y;
			}
		}
		/************/

		var_1C->field_0 = var_12;
		ctpVar13 = var_12;
		var_12->field_0 = (ctpVar19Struct *) (-1);

		subStruct = &var_1C->subStruct;

		subStruct->boxIdx = boxIdx;
		subStruct->type = walkboxType[boxIdx];
		subStruct->minX = minX;
		subStruct->maxX = maxX;
		subStruct->minY = minY;
		subStruct->maxY = maxY;
	}
}

int loadCtp(uint8 *ctpName) {
	int walkboxCounter;	// si
	uint8 *ptr;
	uint8 *dataPointer;	// ptr2
	char fileType[5];	// string2
	short int segementSizeTable[7];	// tempTable
	char string[32];

	if (ctpVar1 == 0) {
		int i;

		for (i = 0; i < 10; i++) {
			persoTable[i] = NULL;
		}
	}

	if (!loadFileSub1(&ptr, ctpName, 0)) {
		free(ptr);
		return (-18);
	}

	dataPointer = ptr;

	fileType[4] = 0;
	memcpy(fileType, dataPointer, 4);	// get the file type, first 4 bytes of the CTP file
	dataPointer += 4;

	if (strcmp(fileType, "CTP ")) {
		free(ptr);
		return (0);
	}

	memcpy(&ctp_routeCoordCount, dataPointer, 2);	// get the number of path-finding coordinates
	dataPointer += 2;
	flipShort(&ctp_routeCoordCount);

	memcpy(segementSizeTable, dataPointer, 0xE);
	dataPointer += 0xE;	// + 14
	flipGen(segementSizeTable, 0xE);

	memcpy(ctp_routeCoords, dataPointer, segementSizeTable[0]);	// get the path-finding coordinates
	dataPointer += segementSizeTable[0];
	flipGen(ctp_routeCoords, segementSizeTable[0]);

	memcpy(ctp_routes, dataPointer, segementSizeTable[1]);	// get the path-finding line informations (indexing the routeCoords array)
	dataPointer += segementSizeTable[1];
	flipGen(ctp_routes, segementSizeTable[1]);

	memcpy(ctp_walkboxTable, dataPointer, segementSizeTable[2]);	// get the walkbox coordinates and lines
	dataPointer += segementSizeTable[2];
	flipGen(ctp_walkboxTable, segementSizeTable[2]);

	if (ctpVar1) {
		dataPointer += segementSizeTable[3];
		dataPointer += segementSizeTable[4];
	} else {
		memcpy(walkboxType, dataPointer, segementSizeTable[3]);	// get the walkbox type
		dataPointer += segementSizeTable[3];
		flipGen(walkboxType, segementSizeTable[3]);	// Type: 0x00 - non walkable, 0x01 - walkable, 0x02 - exit zone

		memcpy(walkboxChange, dataPointer, segementSizeTable[4]);	// change indicator, walkbox type can change, i.e. blocked by object (values are either 0x00 or 0x01)
		dataPointer += segementSizeTable[4];
		flipGen(walkboxChange, segementSizeTable[4]);
	}

	memcpy(ctpVar6, dataPointer, segementSizeTable[5]);	// unknown? always 2*16 bytes (used by S24.CTP, S33.CTP, S33_2.CTP, S34.CTP, S35.CTP, S36.CTP; values can be 0x00, 0x01, 0x03, 0x05)
	dataPointer += segementSizeTable[5];
	flipGen(ctpVar6, segementSizeTable[5]);

	memcpy(ctp_scale, dataPointer, segementSizeTable[6]);	// scale values for the walkbox coordinates (don't know why there is a need for scaling walkboxes)
	dataPointer += segementSizeTable[6];
	flipGen(ctp_scale, segementSizeTable[6]);	// ok

	free(ptr);

	strcpyuint8(string, currentCtpName);

	numberOfWalkboxes = segementSizeTable[6] / 2;	// get the number of walkboxes

	loadCtpSub2(ctp_routeCoordCount, ctpVar8);	// process path-finding stuff

	polyStruct = ctpVar11 = ctpVar13 = ptr_something;

	ptr = (uint8 *) polyStruct;

	walkboxCounter = numberOfWalkboxes;

	while ((--walkboxCounter) >= 0) {
		loadCtpSub1(walkboxCounter, 0, ctp_walkboxTable, ctpVar13);
	}

	ctpVar15 = ctpVar13 + 1;	// was after the -1 thing

	walkboxCounter = numberOfWalkboxes;

	while (--walkboxCounter) {
		loadCtpSub1(walkboxCounter, ctp_scale[walkboxCounter] * 20,
		    ctp_walkboxTable, ctpVar13);
	}

	//ctpVar17 = ctpVar13 - ptr + 4;

	{
		int numOfUsedEntries = ctpVar13 - (ctpVar19Struct *) ptr;
		numOfUsedEntries++;	// there is a -1 entry at the end... Original was only mallocing numOfUsedEntries*sizeof(ctpVar19Struct)+4, but this is a bit ugly...
		ctpVar13 = ctpVar11 = polyStruct =
		    (ctpVar19Struct *) malloc(numOfUsedEntries *
		    sizeof(ctpVar19Struct));
	}

	walkboxCounter = numberOfWalkboxes;
	while ((--walkboxCounter) >= 0) {
		loadCtpSub1(walkboxCounter, 0, ctp_walkboxTable, ctpVar13);
	}

	ctpVar15 = ctpVar13 + 1;

	walkboxCounter = numberOfWalkboxes;
	while (--walkboxCounter) {
		loadCtpSub1(walkboxCounter, ctp_scale[walkboxCounter] * 20,
		    ctp_walkboxTable, ctpVar13);
	}

	ctpVar19 = ctpVar11;

	return (1);
}

} // End of namespace Cruise
