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

//=============================================================================
//
// This unit provides functions for reading main game file into appropriate
// data structures. Main game file contains general game data, such as global
// options, lists of static game entities and compiled scripts modules.
//
//=============================================================================

#ifndef AGS_SHARED_GAME_ROOMFILE_H
#define AGS_SHARED_GAME_ROOMFILE_H

#include "ags/std/memory.h"
#include "ags/std/vector.h"
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
struct RoomDataSource {
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

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
