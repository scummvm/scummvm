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

#include "ags/shared/ac/common.h"
#include "ags/shared/ac/game_version.h"
#include "ags/engine/ac/room_status.h"
#include "ags/shared/game/custom_properties.h"
#include "ags/engine/game/savegame_components.h"
#include "ags/shared/util/string_utils.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

void HotspotState::ReadFromSavegame(Shared::Stream *in, int save_ver) {
	Enabled = in->ReadInt8() != 0;
	if (save_ver >= kRoomStatSvgVersion_36016) {
		Name = StrUtil::ReadString(in);
	}
}

void HotspotState::WriteToSavegame(Shared::Stream *out) const {
	out->WriteInt8(Enabled);
	StrUtil::WriteString(Name, out);
}

RoomStatus::RoomStatus() {
	contentFormat = kRoomStatSvgVersion_Current; // set current to avoid fixups
	beenhere = 0;
	numobj = 0;
	tsdatasize = 0;

	memset(&region_enabled, 0, sizeof(region_enabled));
	memset(&walkbehind_base, 0, sizeof(walkbehind_base));
	memset(&interactionVariableValues, 0, sizeof(interactionVariableValues));
}

RoomStatus::~RoomStatus() {}

void RoomStatus::FreeScriptData() {
	tsdata.clear();
	tsdatasize = 0;
}

void RoomStatus::FreeProperties() {
	roomProps.clear();
	for (int i = 0; i < MAX_ROOM_HOTSPOTS; ++i) {
		hsProps[i].clear();
	}
	objProps.clear();
}

void RoomStatus::ReadFromSavegame_v321(Stream *in, GameDataVersion data_ver) {
	FreeScriptData();
	FreeProperties();

	contentFormat = kRoomStatSvgVersion_Initial;
	obj.resize(MAX_ROOM_OBJECTS_v300);
	objProps.resize(MAX_ROOM_OBJECTS_v300);
	intrObject.resize(MAX_ROOM_OBJECTS_v300);

	beenhere = in->ReadInt32();
	numobj = in->ReadInt32();
	// NOTE: legacy format always contained max object slots
	for (auto &o : obj) {
		o.ReadFromSavegame(in, -1 /* legacy save with padding */);
	}

	in->Seek(MAX_LEGACY_ROOM_FLAGS * sizeof(int16_t)); // flagstates (OBSOLETE)
	in->ReadInt16(); // alignment padding to int32
	tsdatasize = static_cast<uint32_t>(in->ReadInt32());
	in->ReadInt32(); // tsdata
	for (int i = 0; i < MAX_ROOM_HOTSPOTS; ++i) {
		intrHotspot[i].ReadFromSavedgame_v321(in);
	}
	for (auto &intr : intrObject) {
		intr.ReadFromSavedgame_v321(in);
	}
	for (int i = 0; i < MAX_ROOM_REGIONS; ++i) {
		intrRegion[i].ReadFromSavedgame_v321(in);
	}
	intrRoom.ReadFromSavedgame_v321(in);
	for (size_t i = 0; i < MAX_ROOM_HOTSPOTS; ++i)
		hotspot[i].Enabled = in->ReadInt8() != 0;
	in->ReadArrayOfInt8((int8_t *)region_enabled, MAX_ROOM_REGIONS);
	in->ReadArrayOfInt16(walkbehind_base, MAX_WALK_BEHINDS);
	in->ReadInt16(); // alignment padding to int32 (66 int8 + 16 int16 = 49 int16 -> 50)
	in->ReadArrayOfInt32(interactionVariableValues, MAX_GLOBAL_VARIABLES);

	if (data_ver >= kGameVersion_340_4) {
		Properties::ReadValues(roomProps, in);
		for (int i = 0; i < MAX_ROOM_HOTSPOTS; ++i) {
			Properties::ReadValues(hsProps[i], in);
		}
		for (auto &props : objProps) {
			Properties::ReadValues(props, in);
		}
	}
}

void RoomStatus::ReadFromSavegame(Stream *in, GameDataVersion data_ver, RoomStatSvgVersion save_ver) {
	FreeScriptData();
	FreeProperties();

	beenhere = in->ReadInt8();
	numobj = static_cast<uint32_t>(in->ReadInt32());
	obj.resize(numobj);
	objProps.resize(numobj);
	intrObject.resize(numobj);
	for (uint32_t i = 0; i < numobj; ++i) {
		obj[i].ReadFromSavegame(in, save_ver);
		Properties::ReadValues(objProps[i], in);
		if (data_ver <= kGameVersion_272)
			SavegameComponents::ReadInteraction272(intrObject[i], in);
	}
	for (int i = 0; i < MAX_ROOM_HOTSPOTS; ++i) {
		hotspot[i].ReadFromSavegame(in, save_ver);
		Properties::ReadValues(hsProps[i], in);
		if (data_ver <= kGameVersion_272)
			SavegameComponents::ReadInteraction272(intrHotspot[i], in);
	}
	for (int i = 0; i < MAX_ROOM_REGIONS; ++i) {
		region_enabled[i] = in->ReadInt8();
		if (data_ver <= kGameVersion_272)
			SavegameComponents::ReadInteraction272(intrRegion[i], in);
	}
	for (int i = 0; i < MAX_WALK_BEHINDS; ++i) {
		walkbehind_base[i] = in->ReadInt32();
	}

	Properties::ReadValues(roomProps, in);
	if (data_ver <= kGameVersion_272) {
		SavegameComponents::ReadInteraction272(intrRoom, in);
		in->ReadArrayOfInt32(interactionVariableValues, MAX_GLOBAL_VARIABLES);
	}

	tsdatasize = static_cast<uint32_t>(in->ReadInt32());
	if (tsdatasize) {
		tsdata.resize(tsdatasize);
		in->Read(tsdata.data(), tsdatasize);
	}

	contentFormat = save_ver;
	if (save_ver >= kRoomStatSvgVersion_36041) {
		contentFormat = (RoomStatSvgVersion)in->ReadInt32();
		in->ReadInt32(); // reserved
		in->ReadInt32();
		in->ReadInt32();
	}
}

void RoomStatus::WriteToSavegame(Stream *out, GameDataVersion data_ver) const {
	out->WriteInt8(beenhere);
	out->WriteInt32(numobj);
	for (uint32_t i = 0; i < numobj; ++i) {
		obj[i].WriteToSavegame(out);
		Properties::WriteValues(objProps[i], out);
		if (data_ver <= kGameVersion_272)
			SavegameComponents::WriteInteraction272(intrObject[i], out);
	}
	for (int i = 0; i < MAX_ROOM_HOTSPOTS; ++i) {
		hotspot[i].WriteToSavegame(out);
		Properties::WriteValues(hsProps[i], out);
		if (data_ver <= kGameVersion_272)
			SavegameComponents::WriteInteraction272(intrHotspot[i], out);
	}
	for (int i = 0; i < MAX_ROOM_REGIONS; ++i) {
		out->WriteInt8(region_enabled[i]);
		if (data_ver <= kGameVersion_272)
			SavegameComponents::WriteInteraction272(intrRegion[i], out);
	}
	for (int i = 0; i < MAX_WALK_BEHINDS; ++i) {
		out->WriteInt32(walkbehind_base[i]);
	}

	Properties::WriteValues(roomProps, out);
	if (data_ver <= kGameVersion_272) {
		SavegameComponents::WriteInteraction272(intrRoom, out);
		out->WriteArrayOfInt32(interactionVariableValues, MAX_GLOBAL_VARIABLES);
	}

	out->WriteInt32(static_cast<int32_t>(tsdatasize));
	if (tsdatasize)
		out->Write(tsdata.data(), tsdatasize);

	// kRoomStatSvgVersion_36041
	out->WriteInt32(contentFormat);
	out->WriteInt32(0); // reserved
	out->WriteInt32(0);
	out->WriteInt32(0);
}

// Replaces all accesses to the roomstats array
RoomStatus *getRoomStatus(int room) {
	if (!_G(room_statuses)[room]) {
		// First access, allocate and initialise the status
		_G(room_statuses)[room].reset(new RoomStatus());
	}
	return _G(room_statuses)[room].get();
}

// Used in places where it is only important to know whether the player
// had previously entered the room. In this case it is not necessary
// to initialise the status because a player can only have been in
// a room if the status is already initialised.
bool isRoomStatusValid(int room) {
	return (_G(room_statuses)[room] != nullptr);
}

void resetRoomStatuses() {
	for (int i = 0; i < MAX_ROOMS; i++) {
		_G(room_statuses)[i].reset();
	}
}

} // namespace AGS3
