//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

#include <string.h> // memset
#include <stdlib.h> // free
#include "ac/common.h"
#include "ac/game_version.h"
#include "ac/roomstatus.h"
#include "game/customproperties.h"
#include "game/savegame_components.h"
#include "util/alignedstream.h"

using namespace AGS::Common;
using namespace AGS::Engine;

RoomStatus::RoomStatus()
{
    beenhere = 0;
    numobj = 0;
    memset(&flagstates, 0, sizeof(flagstates));
    tsdatasize = 0;
    tsdata = nullptr;
    
    memset(&hotspot_enabled, 0, sizeof(hotspot_enabled));
    memset(&region_enabled, 0, sizeof(region_enabled));
    memset(&walkbehind_base, 0, sizeof(walkbehind_base));
    memset(&interactionVariableValues, 0, sizeof(interactionVariableValues));
}

RoomStatus::~RoomStatus()
{
    if (tsdata)
        delete [] tsdata;
}

void RoomStatus::FreeScriptData()
{
    if (tsdata)
        delete [] tsdata;
    tsdata = nullptr;
    tsdatasize = 0;
}

void RoomStatus::FreeProperties()
{
    roomProps.clear();
    for (int i = 0; i < MAX_ROOM_HOTSPOTS; ++i)
    {
        hsProps[i].clear();
    }
    for (int i = 0; i < MAX_ROOM_OBJECTS; ++i)
    {
        objProps[i].clear();
    }
}

void RoomStatus::ReadFromFile_v321(Stream *in)
{
    beenhere = in->ReadInt32();
    numobj = in->ReadInt32();
    ReadRoomObjects_Aligned(in);
    in->ReadArrayOfInt16(flagstates, MAX_FLAGS);
    tsdatasize = in->ReadInt32();
    in->ReadInt32(); // tsdata
    for (int i = 0; i < MAX_ROOM_HOTSPOTS; ++i)
    {
        intrHotspot[i].ReadFromSavedgame_v321(in);
    }
    for (int i = 0; i < MAX_ROOM_OBJECTS; ++i)
    {
        intrObject[i].ReadFromSavedgame_v321(in);
    }
    for (int i = 0; i < MAX_ROOM_REGIONS; ++i)
    {
        intrRegion[i].ReadFromSavedgame_v321(in);
    }
    intrRoom.ReadFromSavedgame_v321(in);
    in->ReadArrayOfInt8((int8_t*)hotspot_enabled, MAX_ROOM_HOTSPOTS);
    in->ReadArrayOfInt8((int8_t*)region_enabled, MAX_ROOM_REGIONS);
    in->ReadArrayOfInt16(walkbehind_base, MAX_WALK_BEHINDS);
    in->ReadArrayOfInt32(interactionVariableValues, MAX_GLOBAL_VARIABLES);

    if (loaded_game_file_version >= kGameVersion_340_4)
    {
        Properties::ReadValues(roomProps, in);
        for (int i = 0; i < MAX_ROOM_HOTSPOTS; ++i)
        {
            Properties::ReadValues(hsProps[i], in);
        }
        for (int i = 0; i < MAX_ROOM_OBJECTS; ++i)
        {
            Properties::ReadValues(objProps[i], in);
        }
    }
}

void RoomStatus::ReadRoomObjects_Aligned(Common::Stream *in)
{
    AlignedStream align_s(in, Common::kAligned_Read);
    for (int i = 0; i < MAX_ROOM_OBJECTS; ++i)
    {
        obj[i].ReadFromFile(&align_s);
        align_s.Reset();
    }
}

void RoomStatus::ReadFromSavegame(Stream *in)
{
    FreeScriptData();
    FreeProperties();

    beenhere = in->ReadInt8();
    numobj = in->ReadInt32();
    for (int i = 0; i < numobj; ++i)
    {
        obj[i].ReadFromFile(in);
        Properties::ReadValues(objProps[i], in);
        if (loaded_game_file_version <= kGameVersion_272)
            SavegameComponents::ReadInteraction272(intrObject[i], in);
    }
    for (int i = 0; i < MAX_ROOM_HOTSPOTS; ++i)
    {
        hotspot_enabled[i] = in->ReadInt8();
        Properties::ReadValues(hsProps[i], in);
        if (loaded_game_file_version <= kGameVersion_272)
            SavegameComponents::ReadInteraction272(intrHotspot[i], in);
    }
    for (int i = 0; i < MAX_ROOM_REGIONS; ++i)
    {
        region_enabled[i] = in->ReadInt8();
        if (loaded_game_file_version <= kGameVersion_272)
            SavegameComponents::ReadInteraction272(intrRegion[i], in);
    }
    for (int i = 0; i < MAX_WALK_BEHINDS; ++i)
    {
        walkbehind_base[i] = in->ReadInt32();
    }

    Properties::ReadValues(roomProps, in);
    if (loaded_game_file_version <= kGameVersion_272)
    {
        SavegameComponents::ReadInteraction272(intrRoom, in);
        in->ReadArrayOfInt32(interactionVariableValues, MAX_GLOBAL_VARIABLES);
    }

    tsdatasize = in->ReadInt32();
    if (tsdatasize)
    {
        tsdata = new char[tsdatasize];
        in->Read(tsdata, tsdatasize);
    }
}

void RoomStatus::WriteToSavegame(Stream *out) const
{
    out->WriteInt8(beenhere);
    out->WriteInt32(numobj);
    for (int i = 0; i < numobj; ++i)
    {
        obj[i].WriteToFile(out);
        Properties::WriteValues(objProps[i], out);
        if (loaded_game_file_version <= kGameVersion_272)
            SavegameComponents::WriteInteraction272(intrObject[i], out);
    }
    for (int i = 0; i < MAX_ROOM_HOTSPOTS; ++i)
    {
        out->WriteInt8(hotspot_enabled[i]);
        Properties::WriteValues(hsProps[i], out);
        if (loaded_game_file_version <= kGameVersion_272)
            SavegameComponents::WriteInteraction272(intrHotspot[i], out);
    }
    for (int i = 0; i < MAX_ROOM_REGIONS; ++i)
    {
        out->WriteInt8(region_enabled[i]);
        if (loaded_game_file_version <= kGameVersion_272)
            SavegameComponents::WriteInteraction272(intrRegion[i], out);
    }
    for (int i = 0; i < MAX_WALK_BEHINDS; ++i)
    {
        out->WriteInt32(walkbehind_base[i]);
    }

    Properties::WriteValues(roomProps, out);
    if (loaded_game_file_version <= kGameVersion_272)
    {
        SavegameComponents::WriteInteraction272(intrRoom, out);
        out->WriteArrayOfInt32(interactionVariableValues, MAX_GLOBAL_VARIABLES);
    }

    out->WriteInt32(tsdatasize);
    if (tsdatasize)
        out->Write(tsdata, tsdatasize);
}

// JJS: Replacement for the global roomstats array in the original engine.

RoomStatus* room_statuses[MAX_ROOMS];

// Replaces all accesses to the roomstats array
RoomStatus* getRoomStatus(int room)
{
    if (room_statuses[room] == nullptr)
    {
        // First access, allocate and initialise the status
        room_statuses[room] = new RoomStatus();
    }
    return room_statuses[room];
}

// Used in places where it is only important to know whether the player
// had previously entered the room. In this case it is not necessary
// to initialise the status because a player can only have been in
// a room if the status is already initialised.
bool isRoomStatusValid(int room)
{
    return (room_statuses[room] != nullptr);
}

void resetRoomStatuses()
{
    for (int i = 0; i < MAX_ROOMS; i++)
    {
        if (room_statuses[i] != nullptr)
        {
            delete room_statuses[i];
            room_statuses[i] = nullptr;
        }
    }
}
