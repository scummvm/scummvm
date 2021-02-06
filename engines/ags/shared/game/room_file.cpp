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

#include "ac/common.h" // update_polled_stuff
#include "ac/common_defines.h"
#include "ac/gamestructdefines.h"
#include "ac/wordsdictionary.h" // TODO: extract string decryption
#include "core/assetmanager.h"
#include "debug/out.h"
#include "game/customproperties.h"
#include "game/room_file.h"
#include "game/roomstruct.h"
#include "gfx/bitmap.h"
#include "script/cc_error.h"
#include "script/cc_script.h"
#include "util/compress.h"
#include "util/string_utils.h"

// default number of hotspots to read from the room file
#define MIN_ROOM_HOTSPOTS  20
#define LEGACY_HOTSPOT_NAME_LEN 30
#define LEGACY_ROOM_PASSWORD_LENGTH 11
#define LEGACY_ROOM_PASSWORD_SALT 60
#define ROOM_MESSAGE_FLAG_DISPLAYNEXT 200
#define ROOM_LEGACY_OPTIONS_SIZE 10
#define LEGACY_TINT_IS_ENABLED 0x80000000

namespace AGS
{
namespace Common
{

RoomDataSource::RoomDataSource()
    : DataVersion(kRoomVersion_Undefined)
{
}

String GetRoomFileErrorText(RoomFileErrorType err)
{
    switch (err)
    {
    case kRoomFileErr_NoError:
        return "No error.";
    case kRoomFileErr_FileOpenFailed:
        return "Room file was not found or could not be opened.";
    case kRoomFileErr_FormatNotSupported:
        return "Format version not supported.";
    case kRoomFileErr_UnexpectedEOF:
        return "Unexpected end of file.";
    case kRoomFileErr_UnknownBlockType:
        return "Unknown block type.";
    case kRoomFileErr_OldBlockNotSupported:
        return "Block type is too old and not supported by this version of the engine.";
    case kRoomFileErr_BlockDataOverlapping:
        return "Block data overlapping.";
    case kRoomFileErr_IncompatibleEngine:
        return "This engine cannot handle requested room content.";
    case kRoomFileErr_ScriptLoadFailed:
        return "Script load failed.";
    case kRoomFileErr_InconsistentData:
        return "Inconsistent room data, or file is corrupted.";
    case kRoomFileErr_PropertiesBlockFormat:
        return "Unknown format of the custom properties block.";
    case kRoomFileErr_InvalidPropertyValues:
        return "Errors encountered when reading custom properties.";
    case kRoomFileErr_BlockNotFound:
        return "Required block was not found.";
    }
    return "Unknown error.";
}

HRoomFileError OpenRoomFile(const String &filename, RoomDataSource &src)
{
    // Cleanup source struct
    src = RoomDataSource();
    // Try to open room file
    Stream *in = AssetManager::OpenAsset(filename);
    if (in == nullptr)
        return new RoomFileError(kRoomFileErr_FileOpenFailed, String::FromFormat("Filename: %s.", filename.GetCStr()));
    // Read room header
    src.Filename = filename;
    src.DataVersion = (RoomFileVersion)in->ReadInt16();
    if (src.DataVersion < kRoomVersion_250b || src.DataVersion > kRoomVersion_Current)
        return new RoomFileError(kRoomFileErr_FormatNotSupported, String::FromFormat("Required format version: %d, supported %d - %d", src.DataVersion, kRoomVersion_250b, kRoomVersion_Current));
    // Everything is fine, return opened stream
    src.InputStream.reset(in);
    return HRoomFileError::None();
}


enum RoomFileBlock
{
    kRoomFblk_None              = 0,
    // Main room data
    kRoomFblk_Main              = 1,
    // Room script text source (was present in older room formats)
    kRoomFblk_Script            = 2,
    // Old versions of compiled script (no longer supported)
    kRoomFblk_CompScript        = 3,
    kRoomFblk_CompScript2       = 4,
    // Names of the room objects
    kRoomFblk_ObjectNames       = 5,
    // Secondary room backgrounds
    kRoomFblk_AnimBg            = 6,
    // Contemporary compiled script
    kRoomFblk_CompScript3       = 7,
    // Custom properties
    kRoomFblk_Properties        = 8,
    // Script names of the room objects
    kRoomFblk_ObjectScNames     = 9,
    // End of room data tag
    kRoomFile_EOF               = 0xFF
};


void ReadRoomObject(RoomObjectInfo &obj, Stream *in)
{
    obj.Sprite = in->ReadInt16();
    obj.X = in->ReadInt16();
    obj.Y = in->ReadInt16();
    obj.Room = in->ReadInt16();
    obj.IsOn = in->ReadInt16() != 0;
}

void WriteRoomObject(const RoomObjectInfo &obj, Stream *out)
{
    // TODO: expand serialization into 32-bit values at least for the sprite index!!
    out->WriteInt16((int16_t)obj.Sprite);
    out->WriteInt16((int16_t)obj.X);
    out->WriteInt16((int16_t)obj.Y);
    out->WriteInt16((int16_t)obj.Room);
    out->WriteInt16(obj.IsOn ? 1 : 0);
}


// Main room data
HRoomFileError ReadMainBlock(RoomStruct *room, Stream *in, RoomFileVersion data_ver)
{
    int bpp;
    if (data_ver >= kRoomVersion_208)
        bpp = in->ReadInt32();
    else
        bpp = 1;

    if (bpp < 1)
        bpp = 1;

    room->BackgroundBPP = bpp;
    room->WalkBehindCount = in->ReadInt16();
    if (room->WalkBehindCount > MAX_WALK_BEHINDS)
        return new RoomFileError(kRoomFileErr_IncompatibleEngine, String::FromFormat("Too many walk-behinds (in room: %d, max: %d).", room->WalkBehindCount, MAX_WALK_BEHINDS));

    // Walk-behinds baselines
    for (size_t i = 0; i < room->WalkBehindCount; ++i)
        room->WalkBehinds[i].Baseline = in->ReadInt16();

    room->HotspotCount = in->ReadInt32();
    if (room->HotspotCount == 0)
        room->HotspotCount = MIN_ROOM_HOTSPOTS;
    if (room->HotspotCount > MAX_ROOM_HOTSPOTS)
        return new RoomFileError(kRoomFileErr_IncompatibleEngine, String::FromFormat("Too many hotspots (in room: %d, max: %d).", room->HotspotCount, MAX_ROOM_HOTSPOTS));

    // Hotspots walk-to points
    for (size_t i = 0; i < room->HotspotCount; ++i)
    {
        room->Hotspots[i].WalkTo.X = in->ReadInt16();
        room->Hotspots[i].WalkTo.Y = in->ReadInt16();
    }

    // Hotspots names and script names
    for (size_t i = 0; i < room->HotspotCount; ++i)
    {
        if (data_ver >= kRoomVersion_3415)
            room->Hotspots[i].Name = StrUtil::ReadString(in);
        else if (data_ver >= kRoomVersion_303a)
            room->Hotspots[i].Name = String::FromStream(in);
        else
            room->Hotspots[i].Name = String::FromStreamCount(in, LEGACY_HOTSPOT_NAME_LEN);
    }

    if (data_ver >= kRoomVersion_270)
    {
        for (size_t i = 0; i < room->HotspotCount; ++i)
        {
            if (data_ver >= kRoomVersion_3415)
                room->Hotspots[i].ScriptName = StrUtil::ReadString(in);
            else
                room->Hotspots[i].ScriptName = String::FromStreamCount(in, MAX_SCRIPT_NAME_LEN);
        }
    }

    // TODO: remove from format later
    size_t polypoint_areas = in->ReadInt32();
    if (polypoint_areas > 0)
        return new RoomFileError(kRoomFileErr_IncompatibleEngine, "Legacy poly-point areas are no longer supported.");
    /* NOTE: implementation hidden in room_file_deprecated.cpp
        for (size_t i = 0; i < polypoint_areas; ++i)
            wallpoints[i].Read(in);
    */

    update_polled_stuff_if_runtime();

    room->Edges.Top = in->ReadInt16();
    room->Edges.Bottom = in->ReadInt16();
    room->Edges.Left = in->ReadInt16();
    room->Edges.Right = in->ReadInt16();

    // Room objects
    room->ObjectCount = in->ReadInt16();
    if (room->ObjectCount > MAX_ROOM_OBJECTS)
        return new RoomFileError(kRoomFileErr_IncompatibleEngine, String::FromFormat("Too many objects (in room: %d, max: %d).", room->ObjectCount, MAX_ROOM_OBJECTS));

    for (size_t i = 0; i < room->ObjectCount; ++i)
        ReadRoomObject(room->Objects[i], in);

    // Legacy interactions
    if (data_ver >= kRoomVersion_253)
    {
        size_t localvar_count = in->ReadInt32();
        if (localvar_count > 0)
        {
            room->LocalVariables.resize(localvar_count);
            for (size_t i = 0; i < localvar_count; ++i)
                room->LocalVariables[i].Read(in);
        }
    }

    if (data_ver >= kRoomVersion_241 && data_ver < kRoomVersion_300a)
    {
        for (size_t i = 0; i < room->HotspotCount; ++i)
            room->Hotspots[i].Interaction.reset(Interaction::CreateFromStream(in));
        for (size_t i = 0; i < room->ObjectCount; ++i)
            room->Objects[i].Interaction.reset(Interaction::CreateFromStream(in));
        room->Interaction.reset(Interaction::CreateFromStream(in));
    }

    if (data_ver >= kRoomVersion_255b)
    {
        room->RegionCount = in->ReadInt32();
        if (room->RegionCount > MAX_ROOM_REGIONS)
            return new RoomFileError(kRoomFileErr_IncompatibleEngine, String::FromFormat("Too many regions (in room: %d, max: %d).", room->RegionCount, MAX_ROOM_REGIONS));

        if (data_ver < kRoomVersion_300a)
        {
            for (size_t i = 0; i < room->RegionCount; ++i)
                room->Regions[i].Interaction.reset(Interaction::CreateFromStream(in));
        }
    }

    // Event script links
    if (data_ver >= kRoomVersion_300a)
    {
        room->EventHandlers.reset(InteractionScripts::CreateFromStream(in));
        for (size_t i = 0; i < room->HotspotCount; ++i)
            room->Hotspots[i].EventHandlers.reset(InteractionScripts::CreateFromStream(in));
        for (size_t i = 0; i < room->ObjectCount; ++i)
            room->Objects[i].EventHandlers.reset(InteractionScripts::CreateFromStream(in));
        for (size_t i = 0; i < room->RegionCount; ++i)
            room->Regions[i].EventHandlers.reset(InteractionScripts::CreateFromStream(in));
    }

    if (data_ver >= kRoomVersion_200_alpha)
    {
        for (size_t i = 0; i < room->ObjectCount; ++i)
            room->Objects[i].Baseline = in->ReadInt32();
        room->Width = in->ReadInt16();
        room->Height = in->ReadInt16();
    }

    if (data_ver >= kRoomVersion_262)
        for (size_t i = 0; i < room->ObjectCount; ++i)
            room->Objects[i].Flags = in->ReadInt16();

    if (data_ver >= kRoomVersion_200_final)
        room->MaskResolution = in->ReadInt16();

    room->WalkAreaCount = MAX_WALK_AREAS;
    if (data_ver >= kRoomVersion_240)
        room->WalkAreaCount = in->ReadInt32();
    if (room->WalkAreaCount > MAX_WALK_AREAS + 1)
        return new RoomFileError(kRoomFileErr_IncompatibleEngine, String::FromFormat("Too many walkable areas (in room: %d, max: %d).", room->WalkAreaCount, MAX_WALK_AREAS + 1));

    if (data_ver >= kRoomVersion_200_alpha7)
        for (size_t i = 0; i < room->WalkAreaCount; ++i)
            room->WalkAreas[i].ScalingFar = in->ReadInt16();
    if (data_ver >= kRoomVersion_214)
        for (size_t i = 0; i < room->WalkAreaCount; ++i)
            room->WalkAreas[i].Light = in->ReadInt16();
    if (data_ver >= kRoomVersion_251)
    {
        for (size_t i = 0; i < room->WalkAreaCount; ++i)
            room->WalkAreas[i].ScalingNear = in->ReadInt16();
        for (size_t i = 0; i < room->WalkAreaCount; ++i)
            room->WalkAreas[i].Top = in->ReadInt16();
        for (size_t i = 0; i < room->WalkAreaCount; ++i)
            room->WalkAreas[i].Bottom = in->ReadInt16();
    }

    in->Seek(LEGACY_ROOM_PASSWORD_LENGTH); // skip password
    room->Options.StartupMusic = in->ReadInt8();
    room->Options.SaveLoadDisabled = in->ReadInt8() != 0;
    room->Options.PlayerCharOff = in->ReadInt8() != 0;
    room->Options.PlayerView = in->ReadInt8();
    room->Options.MusicVolume = (RoomVolumeMod)in->ReadInt8();
    in->Seek(ROOM_LEGACY_OPTIONS_SIZE - 5);

    room->MessageCount = in->ReadInt16();
    if (room->MessageCount > MAX_MESSAGES)
        return new RoomFileError(kRoomFileErr_IncompatibleEngine, String::FromFormat("Too many room messages (in room: %d, max: %d).", room->MessageCount, MAX_MESSAGES));

    if (data_ver >= kRoomVersion_272)
        room->GameID = in->ReadInt32();

    if (data_ver >= kRoomVersion_pre114_3)
    {
        for (size_t i = 0; i < room->MessageCount; ++i)
        {
            room->MessageInfos[i].DisplayAs = in->ReadInt8();
            room->MessageInfos[i].Flags = in->ReadInt8();
        }
    }

    char buffer[3000];
    for (size_t i = 0; i < room->MessageCount; ++i)
    {
        if (data_ver >= kRoomVersion_261)
            read_string_decrypt(in, buffer, sizeof(buffer));
        else
            StrUtil::ReadCStr(buffer, in, sizeof(buffer));
        room->Messages[i] = buffer;
    }

    // Very old format legacy room animations (FullAnimation)
    if (data_ver >= kRoomVersion_pre114_6)
    {
        // TODO: remove from format later
        size_t fullanim_count = in->ReadInt16();
        if (fullanim_count > 0)
            return new RoomFileError(kRoomFileErr_IncompatibleEngine, "Room animations are no longer supported.");
        /* NOTE: implementation hidden in room_file_deprecated.cpp
            in->ReadArray(&fullanims[0], sizeof(FullAnimation), fullanim_count);
        */
    }

    // Ancient "graphical scripts". We currently don't support them because
    // there's no knowledge on how to convert them to modern engine.
    if ((data_ver >= kRoomVersion_pre114_4) && (data_ver < kRoomVersion_250a))
    {
        return new RoomFileError(kRoomFileErr_IncompatibleEngine, "Pre-2.5 graphical scripts are no longer supported.");
        /* NOTE: implementation hidden in room_file_deprecated.cpp
            ReadPre250Scripts(in);
        */
    }

    if (data_ver >= kRoomVersion_114)
    {
        for (size_t i = 0; i < (size_t)MAX_WALK_AREAS + 1; ++i)
            room->WalkAreas[i].Light = in->ReadInt16();
    }
    if (data_ver >= kRoomVersion_255b)
    {
        for (size_t i = 0; i < room->RegionCount; ++i)
            room->Regions[i].Light = in->ReadInt16();
        for (size_t i = 0; i < room->RegionCount; ++i)
            room->Regions[i].Tint = in->ReadInt32();
    }

    update_polled_stuff_if_runtime();
    // Primary background
    Bitmap *mask = nullptr;
    if (data_ver >= kRoomVersion_pre114_5)
        load_lzw(in, &mask, room->BackgroundBPP, room->Palette);
    else
        loadcompressed_allegro(in, &mask, room->Palette);
    room->BgFrames[0].Graphic.reset(mask);

    update_polled_stuff_if_runtime();
    // Mask bitmaps
    if (data_ver >= kRoomVersion_255b)
    {
        loadcompressed_allegro(in, &mask, room->Palette);
    }
    else if (data_ver >= kRoomVersion_114)
    {
        // an old version - clear the 'shadow' area into a blank regions bmp
        loadcompressed_allegro(in, &mask, room->Palette);
        delete mask;
        mask = nullptr;
    }
    room->RegionMask.reset(mask);
    update_polled_stuff_if_runtime();
    loadcompressed_allegro(in, &mask, room->Palette);
    room->WalkAreaMask.reset(mask);
    update_polled_stuff_if_runtime();
    loadcompressed_allegro(in, &mask, room->Palette);
    room->WalkBehindMask.reset(mask);
    update_polled_stuff_if_runtime();
    loadcompressed_allegro(in, &mask, room->Palette);
    room->HotspotMask.reset(mask);
    return HRoomFileError::None();
}

// Room script sources (original text)
HRoomFileError ReadScriptBlock(char *&buf, Stream *in, RoomFileVersion data_ver)
{
    size_t len = in->ReadInt32();
    buf = new char[len + 1];
    in->Read(buf, len);
    buf[len] = 0;
    for (size_t i = 0; i < len; ++i)
        buf[i] += passwencstring[i % 11];
    return HRoomFileError::None();
}

// Compiled room script
HRoomFileError ReadCompSc3Block(RoomStruct *room, Stream *in, RoomFileVersion data_ver)
{
    room->CompiledScript.reset(ccScript::CreateFromStream(in));
    if (room->CompiledScript == nullptr)
        return new RoomFileError(kRoomFileErr_ScriptLoadFailed, ccErrorString);
    return HRoomFileError::None();
}

// Room object names
HRoomFileError ReadObjNamesBlock(RoomStruct *room, Stream *in, RoomFileVersion data_ver)
{
    int name_count = in->ReadByte();
    if (name_count != room->ObjectCount)
        return new RoomFileError(kRoomFileErr_InconsistentData,
            String::FromFormat("In the object names block, expected name count: %d, got %d", room->ObjectCount, name_count));

    for (size_t i = 0; i < room->ObjectCount; ++i)
    {
        if (data_ver >= kRoomVersion_3415)
            room->Objects[i].Name = StrUtil::ReadString(in);
        else
            room->Objects[i].Name.ReadCount(in, LEGACY_MAXOBJNAMELEN);
    }
    return HRoomFileError::None();
}

// Room object script names
HRoomFileError ReadObjScNamesBlock(RoomStruct *room, Stream *in, RoomFileVersion data_ver)
{
    int name_count = in->ReadByte();
    if (name_count != room->ObjectCount)
        return new RoomFileError(kRoomFileErr_InconsistentData,
            String::FromFormat("In the object script names block, expected name count: %d, got %d", room->ObjectCount, name_count));

    for (size_t i = 0; i < room->ObjectCount; ++i)
    {
        if (data_ver >= kRoomVersion_3415)
            room->Objects[i].ScriptName = StrUtil::ReadString(in);
        else
            room->Objects[i].ScriptName.ReadCount(in, MAX_SCRIPT_NAME_LEN);
    }
    return HRoomFileError::None();
}

// Secondary backgrounds
HRoomFileError ReadAnimBgBlock(RoomStruct *room, Stream *in, RoomFileVersion data_ver)
{
    room->BgFrameCount = in->ReadByte();
    if (room->BgFrameCount > MAX_ROOM_BGFRAMES)
        return new RoomFileError(kRoomFileErr_IncompatibleEngine, String::FromFormat("Too many room backgrounds (in room: %d, max: %d).", room->BgFrameCount, MAX_ROOM_BGFRAMES));

    room->BgAnimSpeed = in->ReadByte();
    if (data_ver >= kRoomVersion_255a)
    {
        for (size_t i = 0; i < room->BgFrameCount; ++i)
            room->BgFrames[i].IsPaletteShared = in->ReadInt8() != 0;
    }

    for (size_t i = 1; i < room->BgFrameCount; ++i)
    {
        update_polled_stuff_if_runtime();
        Bitmap *frame = nullptr;
        load_lzw(in, &frame, room->BackgroundBPP, room->BgFrames[i].Palette);
        room->BgFrames[i].Graphic.reset(frame);
    }
    return HRoomFileError::None();
}

// Read custom properties
HRoomFileError ReadPropertiesBlock(RoomStruct *room, Stream *in, RoomFileVersion data_ver)
{
    int prop_ver = in->ReadInt32();
    if (prop_ver != 1)
        return new RoomFileError(kRoomFileErr_PropertiesBlockFormat, String::FromFormat("Expected version %d, got %d", 1, prop_ver));

    int errors = 0;
    errors += Properties::ReadValues(room->Properties, in);
    for (size_t i = 0; i < room->HotspotCount; ++i)
        errors += Properties::ReadValues(room->Hotspots[i].Properties, in);
    for (size_t i = 0; i < room->ObjectCount; ++i)
        errors += Properties::ReadValues(room->Objects[i].Properties, in);

    if (errors > 0)
        return new RoomFileError(kRoomFileErr_InvalidPropertyValues);
    return HRoomFileError::None();
}

HRoomFileError ReadRoomBlock(RoomStruct *room, Stream *in, RoomFileBlock block, RoomFileVersion data_ver)
{
    soff_t block_len = data_ver < kRoomVersion_350 ? in->ReadInt32() : in->ReadInt64();
    soff_t block_end = in->GetPosition() + block_len;

    HRoomFileError err;
    switch (block)
    {
    case kRoomFblk_Main:
        err = ReadMainBlock(room, in, data_ver);
        break;
    case kRoomFblk_Script:
        in->Seek(block_len); // no longer read source script text into RoomStruct
        break;
    case kRoomFblk_CompScript3:
        err = ReadCompSc3Block(room, in, data_ver);
        break;
    case kRoomFblk_ObjectNames:
        err = ReadObjNamesBlock(room, in, data_ver);
        break;
    case kRoomFblk_ObjectScNames:
        err = ReadObjScNamesBlock(room, in, data_ver);
        break;
    case kRoomFblk_AnimBg:
        err = ReadAnimBgBlock(room, in, data_ver);
        break;
    case kRoomFblk_Properties:
        err = ReadPropertiesBlock(room, in, data_ver);
        break;
    case kRoomFblk_CompScript:
    case kRoomFblk_CompScript2:
        return new RoomFileError(kRoomFileErr_OldBlockNotSupported,
            String::FromFormat("Type: %d.", block));
    default:
        return new RoomFileError(kRoomFileErr_UnknownBlockType,
            String::FromFormat("Type: %d, known range: %d - %d.", block, kRoomFblk_Main, kRoomFblk_ObjectScNames));
    }

    if (!err)
        return err;

    soff_t cur_pos = in->GetPosition();
    if (cur_pos > block_end)
    {
        return new RoomFileError(kRoomFileErr_BlockDataOverlapping,
            String::FromFormat("Type: %d, expected to end at offset: %u, finished reading at %u.", block, block_end, cur_pos));
    }
    else if (cur_pos < block_end)
    {
        Debug::Printf(kDbgMsg_Warn, "WARNING: room data blocks nonsequential, block type %d expected to end at %u, finished reading at %u",
            block, block_end, cur_pos);
        in->Seek(block_end, Common::kSeekBegin);
    }
    return HRoomFileError::None();
}


HRoomFileError ReadRoomData(RoomStruct *room, Stream *in, RoomFileVersion data_ver)
{
    room->DataVersion = data_ver;

    RoomFileBlock block;
    do
    {
        update_polled_stuff_if_runtime();
        int b = in->ReadByte();
        if (b < 0)
            return new RoomFileError(kRoomFileErr_UnexpectedEOF);
        block = (RoomFileBlock)b;
        if (block != kRoomFile_EOF)
        {
            HRoomFileError err = ReadRoomBlock(room, in, block, data_ver);
            if (!err)
                return err;
        }
    }
    while (block != kRoomFile_EOF);
    return HRoomFileError::None();
}

HRoomFileError UpdateRoomData(RoomStruct *room, RoomFileVersion data_ver, bool game_is_hires, const std::vector<SpriteInfo> &sprinfos)
{
    if (data_ver < kRoomVersion_200_final)
        room->MaskResolution = room->BgFrames[0].Graphic->GetWidth() > 320 ? kRoomHiRes : kRoomLoRes;
    if (data_ver < kRoomVersion_3508)
    {
        // Save legacy resolution if it DOES NOT match game's;
        // otherwise it gets promoted to "real resolution"
        if (room->MaskResolution == 1 && game_is_hires)
            room->SetResolution(kRoomLoRes);
        else if (room->MaskResolution > 1 && !game_is_hires)
            room->SetResolution(kRoomHiRes);
    }

    // Old version - copy walkable areas to regions
    if (data_ver < kRoomVersion_255b)
    {
        if (!room->RegionMask)
            room->RegionMask.reset(BitmapHelper::CreateBitmap(room->WalkAreaMask->GetWidth(), room->WalkAreaMask->GetHeight(), 8));
        room->RegionMask->Blit(room->WalkAreaMask.get(), 0, 0, 0, 0, room->RegionMask->GetWidth(), room->RegionMask->GetHeight());
        for (size_t i = 0; i < MAX_ROOM_REGIONS; ++i)
        {
            room->Regions[i].Light = room->WalkAreas[i].Light;
            room->Regions[i].Tint = 255;
        }
    }

    // Fill in dummy interaction objects into unused slots
    // TODO: remove this later, need to rework the legacy interaction usage around the engine code to prevent crashes
    if (data_ver < kRoomVersion_300a)
    {
        if (!room->Interaction)
            room->Interaction.reset(new Interaction());
        for (size_t i = 0; i < (size_t)MAX_ROOM_HOTSPOTS; ++i)
            if (!room->Hotspots[i].Interaction)
                room->Hotspots[i].Interaction.reset(new Interaction());
        for (size_t i = 0; i < (size_t)MAX_ROOM_OBJECTS; ++i)
            if (!room->Objects[i].Interaction)
                room->Objects[i].Interaction.reset(new Interaction());
        for (size_t i = 0; i < (size_t)MAX_ROOM_REGIONS; ++i)
            if (!room->Regions[i].Interaction)
                room->Regions[i].Interaction.reset(new Interaction());
    }

    // Upgade room object script names
    if (data_ver < kRoomVersion_300a)
    {
        for (size_t i = 0; i < room->ObjectCount; ++i)
        {
            if (room->Objects[i].ScriptName.GetLength() > 0)
            {
                String jibbledScriptName;
                jibbledScriptName.Format("o%s", room->Objects[i].ScriptName.GetCStr());
                jibbledScriptName.MakeLower();
                if (jibbledScriptName.GetLength() >= 2)
                    jibbledScriptName.SetAt(1, toupper(jibbledScriptName[1u]));
                room->Objects[i].ScriptName = jibbledScriptName;
            }
        }
    }

    // Pre-3.0.3, multiply up co-ordinates for high-res games to bring them
    // to the proper game coordinate system.
    // If you change this, also change convert_room_coordinates_to_data_res
    // function in the engine
    if (data_ver < kRoomVersion_303b && game_is_hires)
    {
        const int mul = HIRES_COORD_MULTIPLIER;
        for (size_t i = 0; i < room->ObjectCount; ++i)
        {
            room->Objects[i].X *= mul;
            room->Objects[i].Y *= mul;
            if (room->Objects[i].Baseline > 0)
            {
                room->Objects[i].Baseline *= mul;
            }
        }

        for (size_t i = 0; i < room->HotspotCount; ++i)
        {
            room->Hotspots[i].WalkTo.X *= mul;
            room->Hotspots[i].WalkTo.Y *= mul;
        }

        for (size_t i = 0; i < room->WalkBehindCount; ++i)
        {
            room->WalkBehinds[i].Baseline *= mul;
        }

        room->Edges.Left *= mul;
        room->Edges.Top *= mul;
        room->Edges.Bottom *= mul;
        room->Edges.Right *= mul;
        room->Width *= mul;
        room->Height *= mul;
    }

    // Adjust object Y coordinate by adding sprite's height
    // NOTE: this is impossible to do without game sprite information loaded beforehand
    // NOTE: this should be done after coordinate conversion above for simplicity
    if (data_ver < kRoomVersion_300a)
    {
        for (size_t i = 0; i < room->ObjectCount; ++i)
            room->Objects[i].Y += sprinfos[room->Objects[i].Sprite].Height;
    }

    if (data_ver >= kRoomVersion_251)
    {
        // if they set a contiuously scaled area where the top
        // and bottom zoom levels are identical, set it as a normal
        // scaled area
        for (size_t i = 0; i < room->WalkAreaCount; ++i)
        {
            if (room->WalkAreas[i].ScalingFar == room->WalkAreas[i].ScalingNear)
                room->WalkAreas[i].ScalingNear = NOT_VECTOR_SCALED;
        }
    }

    // Convert the old format region tint saturation
    if (data_ver < kRoomVersion_3404)
    {
        for (size_t i = 0; i < room->RegionCount; ++i)
        {
            if ((room->Regions[i].Tint & LEGACY_TINT_IS_ENABLED) != 0)
            {
                room->Regions[i].Tint &= ~LEGACY_TINT_IS_ENABLED;
                // older versions of the editor had a bug - work around it
                int tint_amount = (room->Regions[i].Light > 0 ? room->Regions[i].Light : 50);
                room->Regions[i].Tint |= (tint_amount & 0xFF) << 24;
                room->Regions[i].Light = 255;
            }
        }
    }

    // Older format room messages had flags appended to the message string
    // TODO: find out which data versions had these; is it safe to assume this was before kRoomVersion_pre114_3?
    for (size_t i = 0; i < room->MessageCount; ++i)
    {
        if (!room->Messages[i].IsEmpty() && room->Messages[i].GetLast() == (char)ROOM_MESSAGE_FLAG_DISPLAYNEXT)
        {
            room->Messages[i].ClipRight(1);
            room->MessageInfos[i].Flags |= MSG_DISPLAYNEXT;
        }
    }

    // sync bpalettes[0] with room.pal
    memcpy(room->BgFrames[0].Palette, room->Palette, sizeof(color) * 256);
    return HRoomFileError::None();
}

HRoomFileError ExtractScriptText(String &script, Stream *in, RoomFileVersion data_ver)
{
    RoomFileBlock block;
    do
    {
        int b = in->ReadByte();
        if (b < 0)
            return new RoomFileError(kRoomFileErr_UnexpectedEOF);
        block = (RoomFileBlock)b;
        soff_t block_len = data_ver < kRoomVersion_350 ? in->ReadInt32() : in->ReadInt64();
        if (block == kRoomFblk_Script)
        {
            char *buf = nullptr;
            HRoomFileError err = ReadScriptBlock(buf, in, data_ver);
            if (err)
            {
                script = buf;
                delete buf;
            }
            return err;
        }
        if (block != kRoomFile_EOF)
            in->Seek(block_len); // skip block
    } while (block != kRoomFile_EOF);
    return new RoomFileError(kRoomFileErr_BlockNotFound);
}


// Type of function that writes single room block.
typedef void(*PfnWriteBlock)(const RoomStruct *room, Stream *out);
// Generic function that saves a block and automatically adds its size into header
void WriteBlock(const RoomStruct *room, RoomFileBlock block, PfnWriteBlock writer, Stream *out)
{
    // Write block's header
    out->WriteByte(block);
    soff_t sz_at = out->GetPosition();
    out->WriteInt64(0); // block size placeholder
    // Call writer to save actual block contents
    writer(room, out);

    // Now calculate the block's size...
    soff_t end_at = out->GetPosition();
    soff_t block_size = (end_at - sz_at) - sizeof(int64_t);
    // ...return back and write block's size in the placeholder
    out->Seek(sz_at, Common::kSeekBegin);
    out->WriteInt64(block_size);
    // ...and get back to the end of the file
    out->Seek(0, Common::kSeekEnd);
}

void WriteInteractionScripts(const InteractionScripts *interactions, Stream *out)
{
    out->WriteInt32(interactions->ScriptFuncNames.size());
    for (size_t i = 0; i < interactions->ScriptFuncNames.size(); ++i)
        interactions->ScriptFuncNames[i].Write(out);
}

void WriteMainBlock(const RoomStruct *room, Stream *out)
{
    out->WriteInt32(room->BackgroundBPP);
    out->WriteInt16((int16_t)room->WalkBehindCount);
    for (size_t i = 0; i < room->WalkBehindCount; ++i)
        out->WriteInt16(room->WalkBehinds[i].Baseline);

    out->WriteInt32(room->HotspotCount);
    for (size_t i = 0; i < room->HotspotCount; ++i)
    {
        out->WriteInt16(room->Hotspots[i].WalkTo.X);
        out->WriteInt16(room->Hotspots[i].WalkTo.Y);
    }
    for (size_t i = 0; i < room->HotspotCount; ++i)
        Common::StrUtil::WriteString(room->Hotspots[i].Name, out);
    for (size_t i = 0; i < room->HotspotCount; ++i)
        Common::StrUtil::WriteString(room->Hotspots[i].ScriptName, out);

    out->WriteInt32(0); // legacy poly-point areas

    out->WriteInt16(room->Edges.Top);
    out->WriteInt16(room->Edges.Bottom);
    out->WriteInt16(room->Edges.Left);
    out->WriteInt16(room->Edges.Right);

    out->WriteInt16((int16_t)room->ObjectCount);
    for (size_t i = 0; i < room->ObjectCount; ++i)
    {
        WriteRoomObject(room->Objects[i], out);
    }

    out->WriteInt32(0); // legacy interaction vars
    out->WriteInt32(MAX_ROOM_REGIONS);

    WriteInteractionScripts(room->EventHandlers.get(), out);
    for (size_t i = 0; i < room->HotspotCount; ++i)
        WriteInteractionScripts(room->Hotspots[i].EventHandlers.get(), out);
    for (size_t i = 0; i < room->ObjectCount; ++i)
        WriteInteractionScripts(room->Objects[i].EventHandlers.get(), out);
    for (size_t i = 0; i < room->RegionCount; ++i)
        WriteInteractionScripts(room->Regions[i].EventHandlers.get(), out);

    for (size_t i = 0; i < room->ObjectCount; ++i)
        out->WriteInt32(room->Objects[i].Baseline);
    out->WriteInt16(room->Width);
    out->WriteInt16(room->Height);
    for (size_t i = 0; i < room->ObjectCount; ++i)
        out->WriteInt16(room->Objects[i].Flags);
    out->WriteInt16(room->MaskResolution);

    out->WriteInt32(MAX_WALK_AREAS + 1);
    for (size_t i = 0; i < (size_t)MAX_WALK_AREAS + 1; ++i)
        out->WriteInt16(room->WalkAreas[i].ScalingFar);
    for (size_t i = 0; i < (size_t)MAX_WALK_AREAS + 1; ++i)
        out->WriteInt16(room->WalkAreas[i].Light);
    for (size_t i = 0; i < (size_t)MAX_WALK_AREAS + 1; ++i)
        out->WriteInt16(room->WalkAreas[i].ScalingNear);
    for (size_t i = 0; i < (size_t)MAX_WALK_AREAS + 1; ++i)
        out->WriteInt16(room->WalkAreas[i].Top);
    for (size_t i = 0; i < (size_t)MAX_WALK_AREAS + 1; ++i)
        out->WriteInt16(room->WalkAreas[i].Bottom);

    out->WriteByteCount(0, LEGACY_ROOM_PASSWORD_LENGTH);
    out->WriteInt8(room->Options.StartupMusic);
    out->WriteInt8(room->Options.SaveLoadDisabled ? 1 : 0);
    out->WriteInt8(room->Options.PlayerCharOff ? 1 : 0);
    out->WriteInt8(room->Options.PlayerView);
    out->WriteInt8(room->Options.MusicVolume);
    out->WriteByteCount(0, ROOM_LEGACY_OPTIONS_SIZE - 5);
    out->WriteInt16((int16_t)room->MessageCount);
    out->WriteInt32(room->GameID);
    for (size_t i = 0; i < room->MessageCount; ++i)
    {
        out->WriteInt8(room->MessageInfos[i].DisplayAs);
        out->WriteInt8(room->MessageInfos[i].Flags);
    }
    for (size_t i = 0; i < room->MessageCount; ++i)
        write_string_encrypt(out, room->Messages[i]);

    out->WriteInt16(0); // legacy room animations

    for (size_t i = 0; i < (size_t)MAX_WALK_AREAS + 1; ++i)
        out->WriteInt16(room->WalkAreas[i].Light);
    for (size_t i = 0; i < (size_t)MAX_ROOM_REGIONS; ++i)
        out->WriteInt16(room->Regions[i].Light);
    for (size_t i = 0; i < (size_t)MAX_ROOM_REGIONS; ++i)
        out->WriteInt32(room->Regions[i].Tint);

    save_lzw(out, room->BgFrames[0].Graphic.get(), room->Palette);
    savecompressed_allegro(out, room->RegionMask.get(), room->Palette);
    savecompressed_allegro(out, room->WalkAreaMask.get(), room->Palette);
    savecompressed_allegro(out, room->WalkBehindMask.get(), room->Palette);
    savecompressed_allegro(out, room->HotspotMask.get(), room->Palette);
}

void WriteCompSc3Block(const RoomStruct *room, Stream *out)
{
    room->CompiledScript->Write(out);
}

void WriteObjNamesBlock(const RoomStruct *room, Stream *out)
{
    out->WriteByte((int8_t)room->ObjectCount);
    for (size_t i = 0; i < room->ObjectCount; ++i)
        Common::StrUtil::WriteString(room->Objects[i].Name, out);
}

void WriteObjScNamesBlock(const RoomStruct *room, Stream *out)
{
    out->WriteByte((int8_t)room->ObjectCount);
    for (size_t i = 0; i < room->ObjectCount; ++i)
        Common::StrUtil::WriteString(room->Objects[i].ScriptName, out);
}

void WriteAnimBgBlock(const RoomStruct *room, Stream *out)
{
    out->WriteByte((int8_t)room->BgFrameCount);
    out->WriteByte(room->BgAnimSpeed);

    for (size_t i = 0; i < room->BgFrameCount; ++i)
        out->WriteInt8(room->BgFrames[i].IsPaletteShared ? 1 : 0);
    for (size_t i = 1; i < room->BgFrameCount; ++i)
        save_lzw(out, room->BgFrames[i].Graphic.get(), room->BgFrames[i].Palette);
}

void WritePropertiesBlock(const RoomStruct *room, Stream *out)
{
    out->WriteInt32(1);  // Version 1 of properties block
    Properties::WriteValues(room->Properties, out);
    for (size_t i = 0; i < room->HotspotCount; ++i)
        Properties::WriteValues(room->Hotspots[i].Properties, out);
    for (size_t i = 0; i < room->ObjectCount; ++i)
        Properties::WriteValues(room->Objects[i].Properties, out);
}

HRoomFileError WriteRoomData(const RoomStruct *room, Stream *out, RoomFileVersion data_ver)
{
    if (data_ver < kRoomVersion_Current)
        return new RoomFileError(kRoomFileErr_FormatNotSupported, "We no longer support saving room in the older format.");

    // Header
    out->WriteInt16(data_ver);
    // Main data
    WriteBlock(room, kRoomFblk_Main, WriteMainBlock, out);
    // Compiled script
    if (room->CompiledScript)
        WriteBlock(room, kRoomFblk_CompScript3, WriteCompSc3Block, out);
    // Object names
    if (room->ObjectCount > 0)
    {
        WriteBlock(room, kRoomFblk_ObjectNames, WriteObjNamesBlock, out);
        WriteBlock(room, kRoomFblk_ObjectScNames, WriteObjScNamesBlock, out);
    }
    // Secondary background frames
    if (room->BgFrameCount > 1)
        WriteBlock(room, kRoomFblk_AnimBg, WriteAnimBgBlock, out);
    // Custom properties
    WriteBlock(room, kRoomFblk_Properties, WritePropertiesBlock, out);

    // Write end of room file
    out->WriteByte(kRoomFile_EOF);
    return HRoomFileError::None();
}

} // namespace Common
} // namespace AGS
