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

#ifndef __lure_res_h__
#define __lure_res_h__

#include "lure/luredefs.h"
#include "lure/memory.h"
#include "common/list.h"
#include "lure/res_struct.h"
#include "lure/hotspots.h"
#include "lure/palette.h"

namespace Lure {

class Resources {
private:
	Common::RandomSource _rnd;
	Palette *_paletteSubset;
	RoomDataList _roomData;
	HotspotDataList _hotspotData;
	HotspotOverrideList _hotspotOverrides;
	HotspotAnimList _animData;
	MemoryBlock *_scriptData;
	MemoryBlock *_script2Data;
	MemoryBlock *_messagesData;
	uint16 *_hotspotScriptData;
	RoomExitJoinList _exitJoins;
	HotspotList _activeHotspots;
	ValueTableData _fieldList;
	HotspotActionSet _actionsList;
	SequenceDelayList _delayList;

	void freeData();
public:
	Resources();
	~Resources();
	static Resources &getReference();
	void reloadData();

	byte *getResource(uint16 resId);
	RoomDataList &roomData() { return _roomData; }
	RoomData *getRoom(uint16 roomNumber);
	void insertPaletteSubset(Palette &p);

	HotspotDataList &hotspotData() { return _hotspotData; }
	HotspotOverrideList &hotspotOverrides() { return _hotspotOverrides; }
	HotspotAnimList &animRecords() { return _animData; }
	MemoryBlock *scriptData() { return _scriptData; }
	MemoryBlock *hotspotScriptData() { return _script2Data; }
	MemoryBlock *messagesData() { return _messagesData; }
	uint16 getHotspotScript(uint16 index);
	HotspotList &activeHotspots() { return _activeHotspots; }
	uint16 random() { return _rnd.getRandomNumber(65536) & 0xffff; }
	HotspotData *getHotspot(uint16 hotspotId);
	Hotspot *getActiveHotspot(uint16 hotspotId);
	HotspotOverrideData *getHotspotOverride(uint16 hotspotId);
	HotspotAnimData *getAnimation(uint16 animRecordId);
	RoomExitJoinList &exitJoins() { return _exitJoins; }
	RoomExitJoinData *getExitJoin(uint16 hotspotId);
	uint16 getHotspotAction(uint16 actionsOffset, Action action);
	HotspotActionList *getHotspotActions(uint16 actionsOffset);
	ValueTableData &fieldList() { return _fieldList; }
	SequenceDelayList &delayList() { return _delayList; }
	uint16 numInventoryItems();
	
	void activateHotspot(uint16 hotspotId);
	Hotspot *addHotspot(uint16 hotspotId);
	void deactivateHotspot(uint16 hotspotId);

};

} // End of namespace Lure

#endif
