/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef AGS_ENGINE_AC_ROOM_STATUS_H
#define AGS_ENGINE_AC_ROOM_STATUS_H

#include "ags/engine/ac/room_object.h"
#include "ags/shared/game/room_struct.h"
#include "ags/shared/game/interactions.h"
#include "ags/shared/util/string_types.h"

namespace AGS3 {

// Forward declaration
namespace AGS {
namespace Shared {
class Stream;
} // namespace Shared
} // namespace AGS

using AGS::Shared::Stream;
using AGS::Shared::Interaction;

struct HotspotState {
	bool Enabled = false;
	Shared::String Name;

	void ReadFromSavegame(Shared::Stream *in, int save_ver);
	void WriteToSavegame(Shared::Stream *out) const;
};

// This struct is saved in the save games - it contains everything about
// a room that could change
struct RoomStatus {
	int   beenhere = 0;
	int   numobj = 0;
	RoomObject obj[MAX_ROOM_OBJECTS];
	int   tsdatasize = 0;
	char *tsdata = nullptr;
	Interaction intrHotspot[MAX_ROOM_HOTSPOTS];
	Interaction intrObject[MAX_ROOM_OBJECTS];
	Interaction intrRegion[MAX_ROOM_REGIONS];
	Interaction intrRoom;

	Shared::StringIMap roomProps;
	Shared::StringIMap hsProps[MAX_ROOM_HOTSPOTS];
	Shared::StringIMap objProps[MAX_ROOM_OBJECTS];
	HotspotState hotspot[MAX_ROOM_HOTSPOTS];
	int8  region_enabled[MAX_ROOM_REGIONS];
	short walkbehind_base[MAX_WALK_BEHINDS];
	int32_t interactionVariableValues[MAX_GLOBAL_VARIABLES];

	// Likely pre-2.5 data
#if defined (OBSOLETE)
	short flagstates[MAX_LEGACY_ROOM_FLAGS]{};
	EventBlock hscond[MAX_ROOM_HOTSPOTS];
	EventBlock objcond[MAX_ROOM_OBJECTS];
	EventBlock misccond;
#endif

	RoomStatus();
	~RoomStatus();

	void FreeScriptData();
	void FreeProperties();

	void ReadFromFile_v321(Shared::Stream *in);
	void ReadRoomObjects_Aligned(Shared::Stream *in);
	void ReadFromSavegame(Shared::Stream *in, int save_ver);
	void WriteToSavegame(Shared::Stream *out) const;
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
