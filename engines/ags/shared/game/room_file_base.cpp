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

#include "ags/shared/game/room_file.h"
#include "ags/shared/util/data_ext.h"
#include "ags/shared/util/file.h"
#include "ags/shared/debugging/out.h"
#include "ags/globals.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

RoomDataSource::RoomDataSource()
	: DataVersion(kRoomVersion_Undefined) {
}

String GetRoomFileErrorText(RoomFileErrorType err) {
	switch (err) {
	case kRoomFileErr_NoError:
		return "No error.";
	case kRoomFileErr_FileOpenFailed:
		return "Room file was not found or could not be opened.";
	case kRoomFileErr_FormatNotSupported:
		return "Format version not supported.";
	case kRoomFileErr_BlockListFailed:
		return "There was an error reading room data..";
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
	default:
		break;
	}
	return "Unknown error.";
}

HRoomFileError OpenRoomFile(const String &filename, RoomDataSource &src) {
	// Cleanup source struct
	src = RoomDataSource();
	// Try to open room file
	Stream *in = File::OpenFileRead(filename);
	if (in == nullptr)
		return new RoomFileError(kRoomFileErr_FileOpenFailed, String::FromFormat("Filename: %s.", filename.GetCStr()));
	src.Filename = filename;
	src.InputStream.reset(in);
	return ReadRoomHeader(src);
}

// Read room data header and check that we support this format
HRoomFileError ReadRoomHeader(RoomDataSource &src) {
	src.DataVersion = (RoomFileVersion)src.InputStream->ReadInt16();
	if (src.DataVersion < kRoomVersion_250b || src.DataVersion > kRoomVersion_Current)
		return new RoomFileError(kRoomFileErr_FormatNotSupported, String::FromFormat("Required format version: %d, supported %d - %d", src.DataVersion, kRoomVersion_250b, kRoomVersion_Current));
	return HRoomFileError::None();
}

String GetRoomBlockName(RoomFileBlock id) {
	switch (id) {
	case kRoomFblk_None: return "None";
	case kRoomFblk_Main: return "Main";
	case kRoomFblk_Script: return "TextScript";
	case kRoomFblk_CompScript: return "CompScript";
	case kRoomFblk_CompScript2: return "CompScript2";
	case kRoomFblk_ObjectNames: return "ObjNames";
	case kRoomFblk_AnimBg: return "AnimBg";
	case kRoomFblk_CompScript3: return "CompScript3";
	case kRoomFblk_Properties: return "Properties";
	case kRoomFblk_ObjectScNames: return "ObjScNames";
	case kRoomFile_EOF: return "EOF";
	default:
		break;
	}
	return "unknown";
}


static PfnWriteRoomBlock writer_writer;
static const RoomStruct *writer_room;
static void WriteRoomBlockWriter(Stream *out) {
	writer_writer(writer_room, out);
}

// Helper for new-style blocks with string id
void WriteRoomBlock(const RoomStruct *room, const String &ext_id, PfnWriteRoomBlock writer, Stream *out) {
	writer_writer = writer;
	writer_room = room;
	WriteExtBlock(ext_id, WriteRoomBlockWriter,
		kDataExt_NumID8 | kDataExt_File64, out);
}

// Helper for old-style blocks with only numeric id
void WriteRoomBlock(const RoomStruct *room, RoomFileBlock block, PfnWriteRoomBlock writer, Stream *out) {
	writer_writer = writer;
	writer_room = room;
	WriteExtBlock(block, WriteRoomBlockWriter,
		kDataExt_NumID8 | kDataExt_File64, out);
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
