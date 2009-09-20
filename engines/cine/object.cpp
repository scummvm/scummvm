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
#include "common/scummsys.h"
#include "common/util.h"

#include "cine/cine.h"
#include "cine/object.h"
#include "cine/part.h"
#include "cine/various.h"

namespace Cine {

Common::Array<ObjectStruct> objectTable;
Common::List<overlay> overlayList;

/*! \brief Resets all elements in the object table. */
void resetObjectTable() {
	for (Common::Array<ObjectStruct>::iterator it = objectTable.begin(); it != objectTable.end(); it++) {
		it->clear();
	}
}

void loadObject(char *pObjectName) {
	uint16 numEntry;
	uint16 entrySize;
	uint16 i;
	byte *ptr, *dataPtr;

	checkDataDisk(-1);

	ptr = dataPtr = readBundleFile(findFileInBundle(pObjectName));

	setMouseCursor(MOUSE_CURSOR_DISK);

	numEntry = READ_BE_UINT16(ptr); ptr += 2;

	entrySize = READ_BE_UINT16(ptr); ptr += 2;

	assert(numEntry <= NUM_MAX_OBJECT);

	for (i = 0; i < numEntry; i++) {
		if (objectTable[i].costume != -2) {	// flag is keep ?
			Common::MemoryReadStream readS(ptr, entrySize);

			objectTable[i].x = readS.readSint16BE();
			objectTable[i].y = readS.readSint16BE();
			objectTable[i].mask = readS.readUint16BE();
			objectTable[i].frame = readS.readSint16BE();
			objectTable[i].costume = readS.readSint16BE();
			readS.read(objectTable[i].name, 20);
			objectTable[i].part = readS.readUint16BE();
		}
		ptr += entrySize;
	}

	if (!strcmp(pObjectName, "INTRO.OBJ")) {
		for (i = 0; i < 10; i++) {
			objectTable[i].costume = 0;
		}
	}

	free(dataPtr);
}

/*! \brief Remove overlay sprite from the list
 * \param objIdx Remove overlay associated with this object
 * \param param Remove overlay of this type
 */
int removeOverlay(uint16 objIdx, uint16 param) {
	Common::List<overlay>::iterator it;

	for (it = overlayList.begin(); it != overlayList.end(); ++it) {
		if (it->objIdx == objIdx && it->type == param) {
			overlayList.erase(it);
			return 1;
		}
	}

	return 0;
}

/*! \brief Add new overlay sprite to the list
 * \param objIdx Associate the overlay with this object
 * \param type Type of new overlay
 * \todo Why are x, y, width and color left uninitialized?
 */
void addOverlay(uint16 objIdx, uint16 type) {
	Common::List<overlay>::iterator it;
	overlay tmp;

	for (it = overlayList.begin(); it != overlayList.end(); ++it) {
		// This is done for both Future Wars and Operation Stealth
		if (objectTable[it->objIdx].mask >= objectTable[objIdx].mask) {
			break;
		}

		// There are additional checks in Operation Stealth's implementation
		if (g_cine->getGameType() == Cine::GType_OS && (it->type == 2 || it->type == 3)) {
			break;
		}
	}

	// In Operation Stealth's implementation we might bail out early
	if (g_cine->getGameType() == Cine::GType_OS && it != overlayList.end() && it->objIdx == objIdx && it->type == type) {
		return;
	}

	tmp.objIdx = objIdx;
	tmp.type = type;
	tmp.x = 0;
	tmp.y = 0;
	tmp.width = 0;
	tmp.color = 0;

	overlayList.insert(it, tmp);
}

/*! \brief Add new background mask overlay
 * \param objIdx Associate the overlay with this object
 * \param param source background index
 */
void addGfxElement(int16 objIdx, int16 param, int16 type) {
	Common::List<overlay>::iterator it;
	overlay tmp;

	for (it = overlayList.begin(); it != overlayList.end(); ++it) {
		if (objectTable[it->objIdx].mask >= objectTable[objIdx].mask || it->type == 2 || it->type == 3) {
			break;
		}
	}

	if (it != overlayList.end() && it->objIdx == objIdx && it->type == type && it->x == param) {
		return;
	}

	tmp.objIdx = objIdx;
	tmp.type = type;
	tmp.x = param;
	tmp.y = 0;
	tmp.width = 0;
	tmp.color = 0;

	overlayList.insert(it, tmp);
}

/*! \brief Remove background mask overlay
 * \param objIdx Remove overlay associated with this object
 * \param param Remove overlay using this background
 * \todo Check that it works
 */
void removeGfxElement(int16 objIdx, int16 param, int16 type) {
	Common::List<overlay>::iterator it;

	for (it = overlayList.begin(); it != overlayList.end(); ++it) {
		if (it->objIdx == objIdx && it->type == type && it->x == param) {
			overlayList.erase(it);
			return;
		}
	}
}

void setupObject(byte objIdx, uint16 param1, uint16 param2, uint16 param3, uint16 param4) {
	objectTable[objIdx].x = param1;
	objectTable[objIdx].y = param2;
	objectTable[objIdx].mask = param3;
	objectTable[objIdx].frame = param4;

	if (g_cine->getGameType() == Cine::GType_OS) {
		resetGfxEntityEntry(objIdx);
	} else { // Future Wars
		if (removeOverlay(objIdx, 0)) {
			addOverlay(objIdx, 0);
		}
	}
}

void subObjectParam(byte objIdx, byte paramIdx, int16 newValue) {
	addObjectParam(objIdx, paramIdx, -newValue);
}

void addObjectParam(byte objIdx, byte paramIdx, int16 newValue) {
	int16 currentValue = getObjectParam(objIdx, paramIdx);
	modifyObjectParam(objIdx, paramIdx, currentValue + newValue);
}

void modifyObjectParam(byte objIdx, byte paramIdx, int16 newValue) {
	// Operation Stealth checks object index range, Future Wars doesn't.
	if (g_cine->getGameType() == Cine::GType_OS && objIdx >= NUM_MAX_OBJECT)
		return;

	switch (paramIdx) {
	case 1:
		objectTable[objIdx].x = newValue;
		break;
	case 2:
		objectTable[objIdx].y = newValue;
		break;
	case 3:
		objectTable[objIdx].mask = newValue;

		if (g_cine->getGameType() == Cine::GType_OS) { // Operation Stealth specific
			resetGfxEntityEntry(objIdx);
		} else { // Future Wars specific
			if (removeOverlay(objIdx, 0)) {
				addOverlay(objIdx, 0);
			}
		}
		break;
	case 4:
		objectTable[objIdx].frame = newValue;
		break;
	case 5:
		// TODO: Test if this really breaks the newspaper machine on the airport in Operation Stealth.
		if (g_cine->getGameType() == Cine::GType_FW && newValue == -1) {
			objectTable[objIdx].costume = globalVars[0];
		} else {
			objectTable[objIdx].costume = newValue;
		}
		break;
	case 6:
		objectTable[objIdx].part = newValue;
		break;
	}
}

/**
 * Check if at least one of the range B's endpoints is inside range A,
 * not counting the starting and ending points of range A.
 * Used at least by Operation Stealth's opcode 0x8D i.e. 141.
 */
bool compareRanges(uint16 aStart, uint16 aEnd, uint16 bStart, uint16 bEnd) {
	return (bStart > aStart && bStart < aEnd) || (bEnd > aStart && bEnd < aEnd);
}

uint16 compareObjectParamRanges(uint16 objIdx1, uint16 xAdd1, uint16 yAdd1, uint16 maskAdd1, uint16 objIdx2, uint16 xAdd2, uint16 yAdd2, uint16 maskAdd2) {
	assert(objIdx1 < NUM_MAX_OBJECT && objIdx2 < NUM_MAX_OBJECT);
	const ObjectStruct &obj1 = objectTable[objIdx1];
	const ObjectStruct &obj2 = objectTable[objIdx2];

	if (compareRanges(obj1.x,    obj1.x    + xAdd1,    obj2.x,    obj2.x    + xAdd2) &&
		compareRanges(obj1.y,    obj1.y    + yAdd1,    obj2.y,    obj2.y    + yAdd2) &&
		compareRanges(obj1.mask, obj1.mask + maskAdd1, obj2.mask, obj2.mask + maskAdd2)) {
		return kCmpEQ;
	} else {
		return 0;
	}
}

uint16 compareObjectParam(byte objIdx, byte type, int16 value) {
	uint16 compareResult = 0;
	int16 objectParam = getObjectParam(objIdx, type);

	if (objectParam > value) {
		compareResult |= kCmpGT;
	} else if (objectParam < value) {
		compareResult |= kCmpLT;
	} else {
		compareResult |= kCmpEQ;
	}

	return compareResult;
}

/*! \bug In Operation Stealth, if you try to go downstairs to the sea in the
 * location between bank and hotel, getObjectParam is called with paramIdx 16
 * and crashes
 */
int16 getObjectParam(uint16 objIdx, uint16 paramIdx) {
	assert(objIdx <= NUM_MAX_OBJECT);

	paramIdx--;

	assert(paramIdx <= 5);

	switch (paramIdx) {
	case 0:
		return objectTable[objIdx].x;
	case 1:
		return objectTable[objIdx].y;
	case 2:
		return objectTable[objIdx].mask;
	case 3:
		return objectTable[objIdx].frame;
	case 4:
		return objectTable[objIdx].costume;
	case 5:
		return objectTable[objIdx].part;
	}

	return 0;
}

} // End of namespace Cine
