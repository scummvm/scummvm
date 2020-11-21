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
//
// This unit provides functions for reading main game file into appropriate
// data structures. Main game file contains general game data, such as global
// options, lists of static game entities and compiled scripts modules.
//
//=============================================================================

#ifndef __AGS_CN_GAME_ROOMFILE_H
#define __AGS_CN_GAME_ROOMFILE_H

#include <memory>
#include <vector>
#include "game/room_version.h"
#include "util/error.h"
#include "util/stream.h"
#include "util/string.h"

struct SpriteInfo;
namespace AGS
{
namespace Common
{

class RoomStruct;

enum RoomFileErrorType
{
    kRoomFileErr_NoError,
    kRoomFileErr_FileOpenFailed,
    kRoomFileErr_FormatNotSupported,
    kRoomFileErr_UnexpectedEOF,
    kRoomFileErr_UnknownBlockType,
    kRoomFileErr_OldBlockNotSupported,
    kRoomFileErr_BlockDataOverlapping,
    kRoomFileErr_IncompatibleEngine,
    kRoomFileErr_ScriptLoadFailed,
    kRoomFileErr_InconsistentData,
    kRoomFileErr_PropertiesBlockFormat,
    kRoomFileErr_InvalidPropertyValues,
    kRoomFileErr_BlockNotFound
};

String GetRoomFileErrorText(RoomFileErrorType err);

typedef TypedCodeError<RoomFileErrorType, GetRoomFileErrorText> RoomFileError;
typedef ErrorHandle<RoomFileError> HRoomFileError;
typedef std::shared_ptr<Stream> PStream;


// RoomDataSource defines a successfully opened room file
struct RoomDataSource
{
    // Name of the asset file
    String              Filename;
    // Room file format version
    RoomFileVersion     DataVersion;
    // A ponter to the opened stream
    PStream             InputStream;

    RoomDataSource();
};

// Opens room file for reading from an arbitrary file
HRoomFileError OpenRoomFile(const String &filename, RoomDataSource &src);
// Reads room data
HRoomFileError ReadRoomData(RoomStruct *room, Stream *in, RoomFileVersion data_ver);
// Applies necessary updates, conversions and fixups to the loaded data
// making it compatible with current engine
HRoomFileError UpdateRoomData(RoomStruct *room, RoomFileVersion data_ver, bool game_is_hires, const std::vector<SpriteInfo> &sprinfos);
// Extracts text script from the room file, if it's available.
// Historically, text sources were kept inside packed room files before AGS 3.*.
HRoomFileError ExtractScriptText(String &script, Stream *in, RoomFileVersion data_ver);

HRoomFileError WriteRoomData(const RoomStruct *room, Stream *out, RoomFileVersion data_ver);

} // namespace Common
} // namespace AGS

#endif // __AGS_CN_GAME_ROOMFILE_H
