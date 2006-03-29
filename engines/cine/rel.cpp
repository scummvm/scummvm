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

#include "common/stdafx.h"
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
	byte *ptr;

	checkDataDisk(-1);

	for (i = 0; i < NUM_MAX_REL; i++) {
		if (relTable[i].data) {
			free(relTable[i].data);
			relTable[i].data = NULL;
			relTable[i].size = 0;
		}
	}

	ptr = readBundleFile(findFileInBundle(pRelName));

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
			relTable[i].data = (char *)malloc(relTable[i].size);

			assert(relTable[i].data);

			memcpy(relTable[i].data, ptr, relTable[i].size);
			ptr += relTable[i].size;
		}
	}

#ifdef DUMP_SCRIPTS_OBJ

	{
		uint16 i;
		byte buffer[256];

		for (i = 0; i < numEntry; i++) {
			if (relTable[i].var4) {
				sprintf(buffer, "%s_%03d.txt", pRelName, i);

				decompileScript(relTable[i].data, NULL, relTable[i].size, i);
				dumpScript(buffer);
			}
		}
	}
#endif
}

} // End of namespace Cine
