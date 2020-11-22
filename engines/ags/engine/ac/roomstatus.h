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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef AGS_ENGINE_AC_ROOMSTATUS_H
#define AGS_ENGINE_AC_ROOMSTATUS_H

#include "ac/roomobject.h"
#include "game/roomstruct.h"
#include "game/interactions.h"
#include "util/string_types.h"

namespace AGS3 {

// Forward declaration
namespace AGS {
namespace Shared {
class Stream;
} // namespace Shared
} // namespace AGS

using AGS::Shared::Stream;
using AGS::Shared::Interaction;

// This struct is saved in the save games - it contains everything about
// a room that could change
struct RoomStatus {
	int   beenhere;
	int   numobj;
	RoomObject obj[MAX_ROOM_OBJECTS];
	short flagstates[MAX_FLAGS];
	int   tsdatasize;
	char *tsdata;
	Interaction intrHotspot[MAX_ROOM_HOTSPOTS];
	Interaction intrObject[MAX_ROOM_OBJECTS];
	Interaction intrRegion[MAX_ROOM_REGIONS];
	Interaction intrRoom;

	Common::StringIMap roomProps;
	Common::StringIMap hsProps[MAX_ROOM_HOTSPOTS];
	Common::StringIMap objProps[MAX_ROOM_OBJECTS];
	// [IKM] 2012-06-22: not used anywhere
#ifdef UNUSED_CODE
	EventBlock hscond[MAX_ROOM_HOTSPOTS];
	EventBlock objcond[MAX_ROOM_OBJECTS];
	EventBlock misccond;
#endif
	char  hotspot_enabled[MAX_ROOM_HOTSPOTS];
	char  region_enabled[MAX_ROOM_REGIONS];
	short walkbehind_base[MAX_WALK_BEHINDS];
	int   interactionVariableValues[MAX_GLOBAL_VARIABLES];

	RoomStatus();
	~RoomStatus();

	void FreeScriptData();
	void FreeProperties();

	void ReadFromFile_v321(Common::Stream *in);
	void ReadRoomObjects_Aligned(Common::Stream *in);
	void ReadFromSavegame(Common::Stream *in);
	void WriteToSavegame(Common::Stream *out) const;
};

// Replaces all accesses to the roomstats array
RoomStatus *getRoomStatus(int room);
// Used in places where it is only important to know whether the player
// had previously entered the room. In this case it is not necessary
// to initialise the status because a player can only have been in
// a room if the status is already initialised.
bool isRoomStatusValid(int room);
void resetRoomStatuses();

} // namespace AGS3

#endif
