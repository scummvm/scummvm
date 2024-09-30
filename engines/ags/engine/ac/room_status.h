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
#include "ags/engine/game/savegame.h"
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

// Savegame data format for RoomStatus
enum RoomStatSvgVersion {
	kRoomStatSvgVersion_Initial = 0, // [UNSUPPORTED] from 3.5.0 pre-alpha
	// NOTE: in 3.5.0 "Room States" had lower index than "Loaded Room State" by mistake
	kRoomStatSvgVersion_350_Mismatch = 0, // an incorrect "Room States" version from 3.5.0
	kRoomStatSvgVersion_350     = 1, // new movelist format (along with pathfinder)
	kRoomStatSvgVersion_36016   = 2, // hotspot and object names
	kRoomStatSvgVersion_36025   = 3, // object animation volume
	kRoomStatSvgVersion_36041   = 4, // room state's contentFormat
	kRoomStatSvgVersion_36109   = 5, // removed movelists, save externally
	kRoomStatSvgVersion_Current = kRoomStatSvgVersion_36109
};

// RoomStatus contains everything about a room that could change at runtime.
struct RoomStatus {
	int   beenhere = 0;
	uint32_t numobj = 0;
	std::vector<RoomObject> obj;
	uint32_t tsdatasize = 0;
	std::vector<char> tsdata;
	Interaction intrHotspot[MAX_ROOM_HOTSPOTS];
	std::vector<Interaction> intrObject;
	Interaction intrRegion[MAX_ROOM_REGIONS];
	Interaction intrRoom;

	Shared::StringIMap roomProps;
	Shared::StringIMap hsProps[MAX_ROOM_HOTSPOTS];
	std::vector<Shared::StringIMap> objProps;
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

	// A version of a save this RoomStatus was restored from.
	// This is used as a hint when merging RoomStatus with the loaded room file (upon room enter).
	// We need this for cases when an old format save is restored within an upgraded game
	// (for example, game was upgraded from 3.4.0 to 3.6.0, but player tries loading 3.4.0 save),
	// because room files are only loaded once entered, so we cannot fixup all RoomStatuses at once.
	RoomStatSvgVersion contentFormat;

	RoomStatus();
	~RoomStatus();

	void FreeScriptData();
	void FreeProperties();

	void ReadFromSavegame_v321(Shared::Stream *in, GameDataVersion data_ver);
	void ReadFromSavegame(Shared::Stream *in, GameDataVersion data_ver, RoomStatSvgVersion save_ver);
	void WriteToSavegame(Shared::Stream *out, GameDataVersion data_ver) const;
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
