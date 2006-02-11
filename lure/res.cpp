/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "lure/res.h"
#include "lure/disk.h"
#include "lure/scripts.h"
#include "lure/screen.h"

namespace Lure {

static Resources *int_resources = NULL;

Resources &Resources::getReference() {
	return *int_resources;
}

Resources::Resources() {
	int_resources = this;
	reloadData();
}

Resources::~Resources() {
	// Delete any unremoved active hotspots
	freeData();
}

void Resources::freeData() {
	_activeHotspots.clear();
	_roomData.clear();
	_hotspotData.clear();
	_hotspotOverrides.clear();
	_animData.clear();
	_exitJoins.clear();
	_delayList.clear();

	delete _paletteSubset;
	delete _scriptData;
	delete _script2Data;
	delete _hotspotScriptData;
	delete _messagesData;
}

struct AnimRecordTemp {
	uint16 *offset;
	MovementDataList *list;
};

void Resources::reloadData() {
	Disk &d = Disk::getReference();
	MemoryBlock *mb;
	uint16 *offset, offsetVal;
	int ctr;

	// Get the palette subset data
	_paletteSubset = new Palette(ALT_PALETTE_RESOURCE_ID);

	// Load room data 
	mb = d.getEntry(ROOM_DATA_RESOURCE_ID);
	offset = (uint16 *) mb->data();
	for (ctr = 0; READ_LE_UINT16(offset) != 0xffff; ++ctr, ++offset) {
		offsetVal = READ_LE_UINT16(offset);
		if (offsetVal != 0) {
			// Get room resource
			RoomResource *rec = (RoomResource *) (mb->data() + offsetVal);
			RoomData *newEntry = new RoomData(rec);
			_roomData.push_back(newEntry);

			if (rec->numExits > 0) {
				RoomExitResource *exitRes = (RoomExitResource *)
					(mb->data() + offsetVal + sizeof(RoomResource));

				for (uint16 exitCtr = 0; exitCtr < rec->numExits; ++exitCtr, ++exitRes) {
					RoomExitData *exit = new RoomExitData(exitRes);
					newEntry->exits.push_back(exit);
				}
			}
		}
	}
	delete mb;

	// Load room exits
	mb = d.getEntry(ROOM_EXITS_RESOURCE_ID);
	ctr = 0;
	for (;;) {
		offsetVal = READ_LE_UINT16(mb->data() + (ctr * 2));
		if (offsetVal == 0xffff) break;
		
		if (offsetVal != 0) {
			RoomData *room = getRoom(ctr);
			if (room) {
				RoomExitHotspotRecord *re = (RoomExitHotspotRecord *) 
					(mb->data() + offsetVal);
				while (READ_LE_UINT16(&re->hotspotId) != 0xffff) {
					RoomExitHotspotData *newEntry = new RoomExitHotspotData(re);
					room->exitHotspots.push_back(newEntry);
					++re;
				}
			}
		}
		++ctr;
	}
	delete mb;

	// Load room joins
	mb = d.getEntry(ROOM_EXIT_JOINS_RESOURCE_ID);
	RoomExitJoinRecord *joinRec = (RoomExitJoinRecord *) mb->data();
	while (READ_LE_UINT16(&joinRec->hotspot1Id) != 0xffff) {
		RoomExitJoinData *newEntry = new RoomExitJoinData(joinRec);
		_exitJoins.push_back(newEntry);
		++joinRec;
	}
	delete mb;

	// Load the hotspot list
	mb = d.getEntry(HOTSPOT_DATA_RESOURCE_ID);
	HotspotResource *hsRec = (HotspotResource *) mb->data();
	while (READ_LE_UINT16(&hsRec->hotspotId) != 0xffff) {
		HotspotData *newEntry = new HotspotData(hsRec);
		_hotspotData.push_back(newEntry);
		++hsRec;
	}
	delete mb;

	// Load the hotspot overrides
	mb = d.getEntry(HOTSPOT_OVERRIDE_DATA_RESOURCE_ID);
	HotspotOverrideResource *hsoRec = (HotspotOverrideResource *) mb->data();
	while (READ_LE_UINT16(&hsoRec->hotspotId) != 0xffff) {
		HotspotOverrideData *newEntry = new HotspotOverrideData(hsoRec);
		_hotspotOverrides.push_back(newEntry);
		++hsoRec;
	}
	delete mb;

	// Load the animation list
	mb = d.getEntry(ANIM_DATA_RESOURCE_ID);
	HotspotAnimResource *animRec = (HotspotAnimResource *) mb->data();
	while (READ_LE_UINT16(&animRec->animRecordId) != 0xffff) {
		HotspotAnimData *newEntry = new HotspotAnimData(animRec);
		_animData.push_back(newEntry);

		// Handle any direction frames
		AnimRecordTemp dirEntries[4] = {
			{&animRec->leftOffset, &newEntry->leftFrames},
			{&animRec->rightOffset, &newEntry->rightFrames},
			{&animRec->upOffset, &newEntry->upFrames},
			{&animRec->downOffset, &newEntry->downFrames}};
		for (int dirCtr = 0; dirCtr < 4; ++dirCtr) {
			offsetVal = READ_LE_UINT16(dirEntries[dirCtr].offset);
			if (offsetVal != 0) {
				MovementResource *moveRec = (MovementResource *)
					(mb->data() + offsetVal);
				while (READ_LE_UINT16(&moveRec->frameNumber) != 0xffff) {
					MovementData *newMove = new MovementData(moveRec);
					dirEntries[dirCtr].list->push_back(newMove);
					++moveRec;
				}
			}
		}

		++animRec;
	}
	delete mb;

	// Hotspot scripts
	mb = d.getEntry(HOTSPOT_SCRIPT_LIST_RESOURCE_ID);
	uint16 numEntries = mb->size() / 2;
	uint16 *srcVal = (uint16 *) mb->data();
	uint16 *destVal = _hotspotScriptData = (uint16 *) 
		Memory::alloc(numEntries * sizeof(uint16));
	for (ctr = 0; ctr < numEntries; ++ctr, ++srcVal, ++destVal) {
		*destVal = READ_LE_UINT16(srcVal);
	}
	delete mb;

	// Handle the hotspot action lists
	mb = d.getEntry(ACTION_LIST_RESOURCE_ID);
	uint16 *v = (uint16 *) mb->data();
	uint16 recordId;
	while ((recordId = READ_LE_UINT16(v)) != 0xffff) {
		++v;
		offsetVal = READ_LE_UINT16(v);
		++v;

		HotspotActionList *list = new HotspotActionList(
			recordId, mb->data() + offsetVal);
		_actionsList.push_back(list);
	}
	delete mb;

	_delayList.clear();

	// Load miscellaneous data
	_scriptData = d.getEntry(SCRIPT_DATA_RESOURCE_ID);
	_script2Data = d.getEntry(SCRIPT2_DATA_RESOURCE_ID);
	_messagesData = d.getEntry(MESSAGES_LIST_RESOURCE_ID);
}

RoomExitJoinData *Resources::getExitJoin(uint16 hotspotId) {
	RoomExitJoinList::iterator i;
	
	for (i = _exitJoins.begin(); i != _exitJoins.end(); ++i) {
		RoomExitJoinData *rec = *i;
		if ((rec->hotspot1Id == hotspotId) || (rec->hotspot2Id == hotspotId))
			return rec;
	}

	return NULL;
}

uint16 Resources::getHotspotScript(uint16 index) {
	return _hotspotScriptData[index];
}

RoomData *Resources::getRoom(uint16 roomNumber) {
	RoomDataList::iterator i;

	for (i = _roomData.begin(); i != _roomData.end(); ++i) {
		RoomData *rec = *i;
		if (rec->roomNumber == roomNumber) return rec;
		++rec;
	}

	return NULL;
}

void Resources::insertPaletteSubset(Palette &p) {
	p.palette()->copyFrom(_paletteSubset->palette(), 0, 129*4, 60*4);
	p.palette()->copyFrom(_paletteSubset->palette(), 60*4, 220*4, 8*4);
}

HotspotData *Resources::getHotspot(uint16 hotspotId) {
	HotspotDataList::iterator i;

	for (i = _hotspotData.begin(); i != _hotspotData.end(); ++i) {
		HotspotData *rec = *i;
		if (rec->hotspotId == hotspotId) return rec;
	}

	return NULL;
}

Hotspot *Resources::getActiveHotspot(uint16 hotspotId) {
	HotspotList::iterator i;

	for (i = _activeHotspots.begin(); i != _activeHotspots.end(); ++i) {
		Hotspot *rec = *i;
		if (rec->hotspotId() == hotspotId) return rec;
	}

	return NULL;
}


HotspotOverrideData *Resources::getHotspotOverride(uint16 hotspotId) {
	HotspotOverrideList::iterator i;

	for (i = _hotspotOverrides.begin(); i != _hotspotOverrides.end(); ++i) {
		HotspotOverrideData *rec = *i;
		if (rec->hotspotId == hotspotId) return rec;
	}

	return NULL;
}

HotspotAnimData *Resources::getAnimation(uint16 animRecordId) {
	HotspotAnimList::iterator i;

	for (i = _animData.begin(); i != _animData.end(); ++i) {
		HotspotAnimData *rec = *i;
		if (rec->animRecordId == animRecordId) return rec;
	}

	return NULL;
}

uint16 Resources::getHotspotAction(uint16 actionsOffset, Action action) {
	HotspotActionList *list = _actionsList.getActions(actionsOffset);
	if (!list) return 0;
	return list->getActionOffset(action);
}

HotspotActionList *Resources::getHotspotActions(uint16 actionsOffset) {
	return _actionsList.getActions(actionsOffset);
}

void Resources::activateHotspot(uint16 hotspotId) {
	HotspotData *res = getHotspot(hotspotId);
	if (!res) return;
	res->roomNumber &= 0x7fff; // clear any suppression bit in room #

	// Make sure that the hotspot isn't already active
	HotspotList::iterator i = _activeHotspots.begin();
	bool found = false;

	for (; i != _activeHotspots.end(); ++i) {
		Hotspot &h = *i.operator*();
		if (h.hotspotId() == res->hotspotId) {
			found = true;
			break;
		}
	}
	if (found) return;

	// Check the script load flag
	if (res->scriptLoadFlag) {
		// Execute a script rather than doing a standard load
		Script::execute(res->loadOffset);
	} else {
		// Standard load
		bool loadFlag = true;

		switch (res->loadOffset) {
		case 0x3afe:
			// Copy protection check - since the game is freeware now,
			// don't bother with it
			loadFlag = false;
			break;

		case 0x41BD:
			// Empty handler used to prevent loading hotspots that
			// are yet to be active (such as the straw fire)
			loadFlag = false;
			break;

		case 0x7172:
		case 0x7167:
			// Standard animation load
			break;

		case 0x88ac:
			// Torch in room #1
			loadFlag = _fieldList.getField(TORCH_HIDE) == 0;
			break;

		default:
			// All others simply activate the hotspot
			warning("Hotspot %d uses unknown load offset proc %d",
				res->hotspotId, res->loadOffset);
		}

		if (loadFlag) {
			Hotspot *hotspot = addHotspot(hotspotId);
//			if (res->loadOffset == 0x7167) hotspot->setPersistant(true);
			// DEBUG - for now only keep certain hotspots active
			hotspot->setPersistant((res->hotspotId >= 0x3e8) && (res->hotspotId <= 0x3ea));
		}
	}
}

Hotspot *Resources::addHotspot(uint16 hotspotId) {
	Hotspot *hotspot = new Hotspot(getHotspot(hotspotId));
	_activeHotspots.push_back(hotspot);
	return hotspot;
}

void Resources::deactivateHotspot(uint16 hotspotId) {
	HotspotList::iterator i = _activeHotspots.begin();

	while (i != _activeHotspots.end()) {
		Hotspot *h = *i;
		if (h->hotspotId() == hotspotId) 
			i = _activeHotspots.erase(i);
		else 
			i++;
	}
}

uint16 Resources::numInventoryItems() {
	uint16 numItems = 0;
	HotspotDataList &list = _hotspotData;
	HotspotDataList::iterator i;
	for (i = list.begin(); i != list.end(); ++i) {
		HotspotData *rec = *i;
		if (rec->roomNumber == PLAYER_ID) ++numItems;
	}

	return numItems;
}

} // end of namespace Lure
