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

//=============================================================================
//
// This unit provides functions for reading compiled room file (CRM)
// into the RoomStruct structure, as well as extracting separate components,
// such as room scripts.
//
//=============================================================================

#ifndef AGS_SHARED_GAME_ROOM_FILE_H
#define AGS_SHARED_GAME_ROOM_FILE_H

#include "ags/lib/std/memory.h"
#include "ags/lib/std/vector.h"
#include "ags/shared/core/platform.h"
#include "ags/shared/game/room_version.h"
#include "ags/shared/util/error.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/util/string.h"

namespace AGS3 {

struct SpriteInfo;
namespace AGS {
namespace Shared {

class RoomStruct;

enum RoomFileErrorType {
	kRoomFileErr_NoError,
	kRoomFileErr_FileOpenFailed,
	kRoomFileErr_FormatNotSupported,
	kRoomFileErr_BlockListFailed,
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

enum RoomFileBlock {
	kRoomFblk_None = 0,
	// Main room data
	kRoomFblk_Main = 1,
	// Room script text source (was present in older room formats)
	kRoomFblk_Script = 2,
	// Old versions of compiled script (no longer supported)
	kRoomFblk_CompScript = 3,
	kRoomFblk_CompScript2 = 4,
	// Names of the room objects
	kRoomFblk_ObjectNames = 5,
	// Secondary room backgrounds
	kRoomFblk_AnimBg = 6,
	// Contemporary compiled script
	kRoomFblk_CompScript3 = 7,
	// Custom properties
	kRoomFblk_Properties = 8,
	// Script names of the room objects
	kRoomFblk_ObjectScNames = 9,
	// End of room data tag
	kRoomFile_EOF = 0xFF
};

String GetRoomFileErrorText(RoomFileErrorType err);
String GetRoomBlockName(RoomFileBlock id);

typedef TypedCodeError<RoomFileErrorType, GetRoomFileErrorText> RoomFileError;
typedef ErrorHandle<RoomFileError> HRoomFileError;
#ifdef AGS_PLATFORM_SCUMMVM
typedef std::shared_ptr<Stream> UStream;
#else
typedef std::unique_ptr<Stream> UStream;
#endif


// RoomDataSource defines a successfully opened room file
struct RoomDataSource {
	// Name of the asset file
	String              Filename;
	// Room file format version
	RoomFileVersion     DataVersion;
	// A ponter to the opened stream
	UStream             InputStream;

	RoomDataSource();
};

// Opens room data for reading from an arbitrary file
HRoomFileError OpenRoomFile(const String &filename, RoomDataSource &src);
// Opens room data for reading from asset of a given name
HRoomFileError OpenRoomFileFromAsset(const String &filename, RoomDataSource &src);
// Reads room data
HRoomFileError ReadRoomData(RoomStruct *room, Stream *in, RoomFileVersion data_ver);
// Applies necessary updates, conversions and fixups to the loaded data
// making it compatible with current engine
HRoomFileError UpdateRoomData(RoomStruct *room, RoomFileVersion data_ver, bool game_is_hires, const std::vector<SpriteInfo> &sprinfos);
// Extracts text script from the room file, if it's available.
// Historically, text sources were kept inside packed room files before AGS 3.*.
HRoomFileError ExtractScriptText(String &script, Stream *in, RoomFileVersion data_ver);
// Writes all room data to the stream
HRoomFileError WriteRoomData(const RoomStruct *room, Stream *out, RoomFileVersion data_ver);

// Reads room data header using stream assigned to RoomDataSource;
// tests and saves its format index if successful
HRoomFileError ReadRoomHeader(RoomDataSource &src);

typedef void(*PfnWriteRoomBlock)(const RoomStruct *room, Stream *out);
// Writes room block with a new-style string id
void WriteRoomBlock(const RoomStruct *room, const String &ext_id, PfnWriteRoomBlock writer, Stream *out);
// Writes room block with a old-style numeric id
void WriteRoomBlock(const RoomStruct *room, RoomFileBlock block, PfnWriteRoomBlock writer, Stream *out);

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
