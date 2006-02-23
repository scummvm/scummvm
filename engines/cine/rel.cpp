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

#include "cine/cine.h"

relStruct relTable[NUM_MAX_REL];

void resetObjectScriptHead(void) {
	objScriptList.next = NULL;
	objScriptList.scriptIdx = -1;
}

void releaseObjectScripts(void) {
	prcLinkedListStruct *currentHead = objScriptList.next;

	while (currentHead) {
		prcLinkedListStruct *temp;

		ASSERT_PTR(currentHead);

		temp = currentHead->next;

		free(currentHead);

		currentHead = temp;
	}

	resetObjectScriptHead();
}

void loadRel(char *pRelName) {
	uint16 numEntry;
	uint16 i;
	uint8 *ptr;

	checkDataDisk(-1);

	for (i = 0; i < NUM_MAX_REL; i++) {
		if (relTable[i].ptr0) {
			ASSERT_PTR(relTable[i].ptr0);

			free(relTable[i].ptr0);

			relTable[i].ptr0 = NULL;

			relTable[i].var4 = 0;
		}
	}

	ptr = readBundleFile(findFileInBundle(pRelName));

	processPendingUpdates(1);

	numEntry = *(uint16 *) ptr;
	ptr += 2;
	flipU16(&numEntry);

	ASSERT(numEntry <= NUM_MAX_REL);

	for (i = 0; i < numEntry; i++) {
		relTable[i].var4 = *(uint16 *) ptr;
		ptr += 2;
		flipU16(&relTable[i].var4);

		relTable[i].var6 = *(uint16 *) ptr;
		ptr += 2;
		flipU16(&relTable[i].var6);

		relTable[i].var8 = *(uint16 *) ptr;
		ptr += 2;
		flipU16(&relTable[i].var8);

		relTable[i].varA = *(uint16 *) ptr;
		ptr += 2;
		flipU16(&relTable[i].varA);
	}

	for (i = 0; i < numEntry; i++) {
		if (relTable[i].var4) {
			relTable[i].ptr0 = (char *)malloc(relTable[i].var4);

			ASSERT_PTR(relTable[i].ptr0);

			memcpy(relTable[i].ptr0, ptr, relTable[i].var4);
			ptr += relTable[i].var4;
		}
	}

#ifdef DUMP_SCRIPTS_OBJ

	{
		uint16 i;
		uint8 buffer[256];

		for (i = 0; i < numEntry; i++) {
			if (relTable[i].var4) {
				sprintf(buffer, "%s_%03d.txt", pRelName, i);

				decompileScript(relTable[i].ptr0, NULL,
				    relTable[i].var4, i);
				dumpScript(buffer);
			}
		}
	}
#endif
}
