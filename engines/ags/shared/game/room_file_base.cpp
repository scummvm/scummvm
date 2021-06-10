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

#include "ags/shared/game/room_file.h"
#include "ags/shared/util/file.h"
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
	case kRoomFblk_Main: return "Main";
	case kRoomFblk_Script: return "TextScript";
	case kRoomFblk_CompScript: return "CompScript";
	case kRoomFblk_CompScript2: return "CompScript2";
	case kRoomFblk_ObjectNames: return "ObjNames";
	case kRoomFblk_AnimBg: return "AnimBg";
	case kRoomFblk_CompScript3: return "CompScript3";
	case kRoomFblk_Properties: return "Properties";
	case kRoomFblk_ObjectScNames: return "ObjScNames";
	}
	return "unknown";
}

HRoomFileError OpenNextRoomBlock(Stream *in, RoomFileVersion data_ver, RoomFileBlock &block_id, String &ext_id, soff_t &block_len) {
	// The block meta format is shared with the main game file extensions
	//    - 1 byte - an old-style unsigned numeric ID:
	//               where 0 would indicate following string ID,
	//               and 0xFF indicates end of extension list.
	//    - 16 bytes - string ID of an extension (if numeric ID is 0).
	//    - 4 or 8 bytes - length of extension data, in bytes (size depends on format version).
	int b = in->ReadByte();
	if (b < 0)
		return new RoomFileError(kRoomFileErr_UnexpectedEOF);

	block_id = (RoomFileBlock)b;
	if (block_id == kRoomFile_EOF)
		return HRoomFileError::None(); // end of list

	if (block_id > 0) { // old-style block identified by a numeric id
		ext_id = GetRoomBlockName(block_id);
		block_len = data_ver < kRoomVersion_350 ? in->ReadInt32() : in->ReadInt64();
	} else { // new style block identified by a string id
		ext_id = String::FromStreamCount(in, 16);
		block_len = in->ReadInt64();
	}
	return HRoomFileError::None();
}

// Generic function that saves a block and automatically adds its size into header
static void WriteRoomBlock(const RoomStruct *room, RoomFileBlock block, const String &ext_id, PfnWriteRoomBlock writer, Stream *out) {
	// Write block's header
	out->WriteByte(block);
	if (block == kRoomFblk_None) // new-style string id
		ext_id.WriteCount(out, 16);
	soff_t sz_at = out->GetPosition();
	out->WriteInt64(0); // block size placeholder
						// Call writer to save actual block contents
	writer(room, out);

	// Now calculate the block's size...
	soff_t end_at = out->GetPosition();
	soff_t block_size = (end_at - sz_at) - sizeof(int64_t);
	// ...return back and write block's size in the placeholder
	out->Seek(sz_at, Shared::kSeekBegin);
	out->WriteInt64(block_size);
	// ...and get back to the end of the file
	out->Seek(0, Shared::kSeekEnd);
}

// Helper for new-style blocks with string id
void WriteRoomBlock(const RoomStruct *room, const String &ext_id, PfnWriteRoomBlock writer, Stream *out) {
	WriteRoomBlock(room, kRoomFblk_None, ext_id, writer, out);
}

// Helper for old-style blocks with only numeric id
void WriteRoomBlock(const RoomStruct *room, RoomFileBlock block, PfnWriteRoomBlock writer, Stream *out) {
	WriteRoomBlock(room, block, String(), writer, out);
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
