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

uint8 colorMode = 0;

uint8 *backgroundPtrtable[8] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };	// wasn't initialized in original, but it's probably better
backgroundTableStruct backgroundTable[8];

char hwPage[64000];

char *hwMemAddr[] = {
	hwPage,
};

short int cvtPalette[0x20];

int loadMEN(uint8 **ptr) {
	char *localPtr = (char *)*ptr;

	if (!strcmp(localPtr, "MEN")) {
		localPtr += 4;

		video4 = *(localPtr++);
		video3 = *(localPtr++);
		video2 = *(localPtr++);
		colorOfSelectedSaveDrive = *(localPtr++);

		*ptr = (uint8 *) localPtr;

		return 1;
	} else {
		return 0;
	}
}

int CVTLoaded;

int loadCVT(uint8 **ptr) {
	char *localPtr = (char *)*ptr;

	if (!strcmp(localPtr, "CVT")) {
		int i;
		localPtr += 4;

		for (i = 0; i < 0x20; i++) {
			cvtPalette[i] = *(localPtr++);
		}

		*ptr = (uint8 *) localPtr;

		CVTLoaded = 1;

		return 1;
	} else {
		CVTLoaded = 0;
		return 0;
	}
}

extern int lastFileSize;

int loadBackground(char *name, int idx) {
	uint8 *ptr;
	uint8 *ptr2;
	uint8 *ptrToFree;

	printf("Loading BG: %s\n", name);

	if (!backgroundPtrtable[idx]) {
		//if(!gfxModuleData.useEGA && !gfxModuleData.useVGA)
		{
			backgroundPtrtable[idx] =
			    (uint8 *) mallocAndZero(320 * 200 /*64000 */ );
		}
/*		else {
			backgroundPtrtable[idx] = hwMemAddr[idx];
		} */
	}

	if (!backgroundPtrtable[idx]) {
		backgroundTable[idx].name[0] = 0;
		return (-2);
	}

	ptrToFree = gfxModuleData.pPage10;
	if (loadFileSub1(&ptrToFree, (uint8 *) name, NULL) < 0) {
		if (ptrToFree != gfxModuleData.pPage10)
			free(ptrToFree);

		return (-18);
	}

	if (lastFileSize == 32078 || lastFileSize == 32080
	    || lastFileSize == 32034) {
		colorMode = 0;
	} else {
		colorMode = 1;
	}

	ptr = ptrToFree;
	ptr2 = ptrToFree;

	if (!strcmpuint8(name, "LOGO.PI1")) {
		bgVar3 = bgVar2;
		bgVar1 = 1;
		bgVar2 = 1;
	} else {
		if (bgVar1) {
			bgVar2 = bgVar3;
			bgVar1 = 0;
		}
	}

	if (!strcmpuint8(ptr, "PAL")) {
		printf("Pal loading unsupported !\n");
		exit(1);
	} else {
		if (!colorMode || ptr2[1] == 5) {
			ptr2 += 2;

			memcpy(palette, ptr2, 0x20);
			ptr2 += 0x20;
			flipGen(palette, 0x20);
			ptr2 += 0x7D00;

			loadMEN(&ptr2);
			loadCVT(&ptr2);

			gfxModuleData_gfxClearFrameBuffer(backgroundPtrtable
			    [idx]);
			gfxModuleData_field_60((char *)ptrToFree + 34, 20, 200,
			    (char *)backgroundPtrtable[idx], 0, 0);

			gfxModuleData_setPal((uint8 *) (palette + (idx << 6)));
		} else if (ptr2[1] == 8) {
			int i;
			ptr2 += 2;

			for (i = 0; i < 256 * 3; i++) {
				palette[i] = ptr2[i];
			}
			//memcpy(palette,ptr2,256*3);
			ptr2 += 256 * 3;

			memcpy(backgroundPtrtable[idx], ptr2, 320 * 200);

			gfxModuleData_setPal256(palette);
		}
	}

	//if(ptrToFree != gfxModuleData.pPage10)
	//  free(ptrToFree);

	if (gfxModuleData.useEGA || gfxModuleData.useTandy) {
		ASSERT(0);
	}

	if (gfxModuleData.useEGA || gfxModuleData.useTandy) {
		ASSERT(0);
	}

	strcpy(backgroundTable[idx].name, name);

	return (0);
}

} // End of namespace Cruise
