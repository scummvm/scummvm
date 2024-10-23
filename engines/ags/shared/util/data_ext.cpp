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

#include "ags/shared/util/data_ext.h"
#include "ags/shared/debugging/out.h"
#include "ags/shared/util/stream.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

String GetDataExtErrorText(DataExtErrorType err) {
	switch (err) {
	case kDataExtErr_NoError:
		return "No error.";
	case kDataExtErr_UnexpectedEOF:
		return "Unexpected end of file.";
	case kDataExtErr_BlockDataOverlapping:
		return "Block data overlapping.";
	case kDataExtErr_BlockNotFound:
		return "Block not found.";
	default: return "Unknown error.";
	}
}

HError DataExtParser::OpenBlock() {
	//    - 1 or 4 bytes - an old-style unsigned numeric ID:
	//               where 0 would indicate following string ID,
	//               and -1 indicates end of the block list.
	//    - 16 bytes - string ID of an extension (if numeric ID is 0).
	//    - 4 or 8 bytes - length of extension data, in bytes.
	_blockID = ((_flags & kDataExt_NumID32) != 0) ?
		_in->ReadInt32() :
		_in->ReadInt8();

	if (_blockID < 0)
		return HError::None(); // end of list
	if (_in->EOS())
		return new DataExtError(kDataExtErr_UnexpectedEOF);

	if (_blockID > 0) { // old-style block identified by a numeric id
		_blockLen = ((_flags & kDataExt_File64) != 0) ? _in->ReadInt64() : _in->ReadInt32();
		_extID = GetOldBlockName(_blockID);
	} else { // new style block identified by a string id
		_extID = String::FromStreamCount(_in, 16);
		_blockLen = _in->ReadInt64();
	}
	_blockStart = _in->GetPosition();
	return HError::None();
}

void DataExtParser::SkipBlock() {
	if (_blockID >= 0)
		_in->Seek(_blockStart + _blockLen, kSeekBegin);
}

HError DataExtParser::PostAssert() {
	const soff_t cur_pos = _in->GetPosition();
	const soff_t block_end = _blockStart + _blockLen;
	if (cur_pos > block_end) {
		String err = String::FromFormat("Block: '%s', expected to end at offset: %llu, finished reading at %llu.",
			_extID.GetCStr(), static_cast<int64>(block_end), static_cast<int64>(cur_pos));
		if (cur_pos <= block_end + GetOverLeeway(_blockID))
			Debug::Printf(kDbgMsg_Warn, err);
		else
			return new DataExtError(kDataExtErr_BlockDataOverlapping, err);
	} else if (cur_pos < block_end) {
		Debug::Printf(kDbgMsg_Warn, "WARNING: data blocks nonsequential, block '%s' expected to end at %llu, finished reading at %llu",
			_extID.GetCStr(), static_cast<int64>(block_end), static_cast<int64>(cur_pos));
		_in->Seek(block_end, Shared::kSeekBegin);
	}
	return HError::None();
}

HError DataExtParser::FindOne(int id) {
	if (id <= 0) return new DataExtError(kDataExtErr_BlockNotFound);

	HError err = HError::None();
	for (err = OpenBlock(); err && !AtEnd(); err = OpenBlock()) {
		if (id == _blockID)
			return HError::None();
		_in->Seek(_blockLen); // skip it
	}
	if (!err)
		return err;
	return new DataExtError(kDataExtErr_BlockNotFound);
}

HError DataExtReader::Read() {
	HError err = HError::None();
	bool read_next = true;
	for (err = OpenBlock(); err && !AtEnd() && read_next; err = OpenBlock()) {
		// Call the reader function to read current block's data
		read_next = true;
		err = ReadBlock(_blockID, _extID, _blockLen, read_next);
		if (!err)
			return err;
		// Test that we did not read too much or too little
		err = PostAssert();
		if (!err)
			return err;
	}
	return err;
}

// Generic function that saves a block and automatically adds its size into header
void WriteExtBlock(int block, const String &ext_id, const PfnWriteExtBlock &writer, int flags, Stream *out) {
	const bool is_id32 = (flags & kDataExt_NumID32) != 0;
	// 64-bit file offsets are written for blocks with ext_id, OR File64 flag
	const bool is_file64 = (block == 0) || ((flags & kDataExt_File64) != 0);
	// Write block's header
	is_id32 != 0 ?
		out->WriteInt32(block) :
		out->WriteInt8(static_cast<int8_t>(block));
	if (block == 0) // new-style string id
		ext_id.WriteCount(out, 16);
	soff_t sz_at = out->GetPosition();
	// block size placeholder
	is_file64 ?
		out->WriteInt64(0) :
		out->WriteInt32(0);
	soff_t start_at = out->GetPosition();

	// Call writer to save actual block contents
	writer(out);

	// Now calculate the block's size...
	soff_t end_at = out->GetPosition();
	soff_t block_size = (end_at - start_at);
	// ...return back and write block's size in the placeholder
	out->Seek(sz_at, Shared::kSeekBegin);
	is_file64 ?
		out->WriteInt64(block_size) :
		out->WriteInt32((int32_t)block_size);
	// ...and get back to the end of the file
	out->Seek(0, Shared::kSeekEnd);
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
