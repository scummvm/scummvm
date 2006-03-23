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
#include "cine/auto00.h"
#include "cine/various.h"

#include "common/config-manager.h"

namespace Cine {

prcLinkedListStruct globalScriptsHead;
prcLinkedListStruct objScriptList;

//char currentPrcName[20];

void resetglobalScriptsHead(void) {
	globalScriptsHead.next = NULL;
	globalScriptsHead.scriptIdx = -1;
}

void freePrcLinkedList(void) {
	prcLinkedListStruct *currentHead = globalScriptsHead.next;

	while (currentHead) {
		prcLinkedListStruct *temp;

		assert(currentHead);

		temp = currentHead->next;

		free(currentHead);

		currentHead = temp;
	}

	resetglobalScriptsHead();
}

void loadPrc(const char *pPrcName) {
	byte i;
	uint16 numScripts;
	const byte *scriptPtr;

	assert(pPrcName);

	for (i = 0; i < NUM_MAX_SCRIPT; i++) {
		if (scriptTable[i].ptr) {
			assert(scriptTable[i].ptr);
			free(scriptTable[i].ptr);
			scriptTable[i].ptr = NULL;
			scriptTable[i].size = 0;
		}
	}

	checkDataDisk(-1);
	if ((gameType == Cine::GID_FW) && (!strcmp(pPrcName, "AUTO00.PRC"))) {
		if (!ConfMan.getBool("copy_protection"))
			scriptPtr = AUT000;
		else
			scriptPtr = readFile(pPrcName);
	} else {
		scriptPtr = readBundleFile(findFileInBundle(pPrcName));
		assert(scriptPtr);
	}

	setMouseCursor(MOUSE_CURSOR_DISK);

	numScripts = READ_BE_UINT16(scriptPtr); scriptPtr += 2;
	assert(numScripts <= NUM_MAX_SCRIPT);

	for (i = 0; i < numScripts; i++) {
		scriptTable[i].size = READ_BE_UINT16(scriptPtr); scriptPtr += 2;
	}

	for (i = 0; i < numScripts; i++) {
		uint16 size = scriptTable[i].size;
		if (size) {
			scriptTable[i].ptr = (byte *) malloc(size);
			assert(scriptTable[i].ptr);
			memcpy(scriptTable[i].ptr, scriptPtr, size);
			scriptPtr += size;
			computeScriptStack(scriptTable[i].ptr, scriptTable[i].stack, size);
		}
	}
}

} // End of namespace Cine
