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

#include "cine/cine.h"
#include "cine/various.h"

namespace Cine {

RelObjectScript relTable[NUM_MAX_REL];

void resetObjectScriptHead(void) {
	objScriptList.next = NULL;
	objScriptList.scriptIdx = -1;
}

void releaseObjectScripts(void) {
	prcLinkedListStruct *currentHead = objScriptList.next;

	while (currentHead) {
		prcLinkedListStruct *temp;

		assert(currentHead);

		temp = currentHead->next;

		free(currentHead);

		currentHead = temp;
	}

	resetObjectScriptHead();
}

void loadRel(char *pRelName) {
	uint16 numEntry;
	uint16 i;
	byte *ptr, *dataPtr;

	checkDataDisk(-1);

	for (i = 0; i < NUM_MAX_REL; i++) {
		if (relTable[i].data) {
			free(relTable[i].data);
			relTable[i].data = NULL;
			relTable[i].size = 0;
		}
	}

	ptr = dataPtr = readBundleFile(findFileInBundle(pRelName));

	setMouseCursor(MOUSE_CURSOR_DISK);

	numEntry = READ_BE_UINT16(ptr); ptr += 2;

	assert(numEntry <= NUM_MAX_REL);

	for (i = 0; i < numEntry; i++) {
		relTable[i].size = READ_BE_UINT16(ptr); ptr += 2;
		relTable[i].obj1Param1 = READ_BE_UINT16(ptr); ptr += 2;
		relTable[i].obj1Param2 = READ_BE_UINT16(ptr); ptr += 2;
		relTable[i].obj2Param = READ_BE_UINT16(ptr); ptr += 2;
	}

	for (i = 0; i < numEntry; i++) {
		if (relTable[i].size) {
			relTable[i].data = (byte *)malloc(relTable[i].size);

			assert(relTable[i].data);

			memcpy(relTable[i].data, ptr, relTable[i].size);
			ptr += relTable[i].size;
		}
	}

	free(dataPtr);

#ifdef DUMP_SCRIPTS

	{
		uint16 s;
		char buffer[256];

		for (s = 0; s < numEntry; s++) {
			if (relTable[s].size) {
				sprintf(buffer, "%s_%03d.txt", pRelName, s);

				decompileScript(relTable[s].data, NULL, relTable[s].size, s);
				dumpScript(buffer);
			}
		}
	}
#endif
}

} // End of namespace Cine
