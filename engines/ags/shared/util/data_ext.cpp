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
	}
	return "Unknown error.";
}

HError DataExtParser::OpenBlock() {
	//    - 1 or 4 bytes - an old-style unsigned numeric ID:
	//               where 0 would indicate following string ID,
	//               and -1 indicates end of the block list.
	//    - 16 bytes - string ID of an extension (if numeric ID is 0).
	//    - 4 or 8 bytes - length of extension data, in bytes.
	_block_id = ((_flags & kDataExt_NumID32) != 0) ?
		_in->ReadInt32() :
		_in->ReadInt8();

	if (_block_id < 0)
		return HError::None(); // end of list
	if (_in->EOS())
		return new DataExtError(kDataExtErr_UnexpectedEOF);

	if (_block_id > 0) { // old-style block identified by a numeric id
		_block_len = ((_flags & kDataExt_File64) != 0) ? _in->ReadInt64() : _in->ReadInt32();
		_ext_id = GetOldBlockName(_block_id);
	} else { // new style block identified by a string id
		_ext_id = String::FromStreamCount(_in, 16);
		_block_len = _in->ReadInt64();
	}
	_block_start = _in->GetPosition();
	return HError::None();
}

void DataExtParser::SkipBlock() {
	if (_block_id >= 0)
		_in->Seek(_block_len);
}

HError DataExtParser::PostAssert() {
	const soff_t cur_pos = _in->GetPosition();
	const soff_t block_end = _block_start + _block_len;
	if (cur_pos > block_end) {
		String err = String::FromFormat("Block: '%s', expected to end at offset: %lld, finished reading at %lld.",
			_ext_id.GetCStr(), block_end, cur_pos);
		if (cur_pos <= block_end + GetOverLeeway(_block_id))
			Debug::Printf(kDbgMsg_Warn, err);
		else
			return new DataExtError(kDataExtErr_BlockDataOverlapping, err);
	} else if (cur_pos < block_end) {
		Debug::Printf(kDbgMsg_Warn, "WARNING: data blocks nonsequential, block '%s' expected to end at %lld, finished reading at %lld",
			_ext_id.GetCStr(), block_end, cur_pos);
		_in->Seek(block_end, Shared::kSeekBegin);
	}
	return HError::None();
}

HError DataExtParser::FindOne(int id) {
	if (id <= 0) return new DataExtError(kDataExtErr_BlockNotFound);

	HError err = HError::None();
	for (err = OpenBlock(); err && !AtEnd(); err = OpenBlock()) {
		if (id == _block_id)
			return HError::None();
		_in->Seek(_block_len); // skip it
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
		err = ReadBlock(_block_id, _ext_id, _block_len, read_next);
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
void WriteExtBlock(int block, const String &ext_id, PfnWriteExtBlock writer, int flags, Stream *out) {
	// Write block's header
	(flags & kDataExt_NumID32) != 0 ?
		out->WriteInt32(block) :
		out->WriteInt8(block);
	if (block == 0) // new-style string id
		ext_id.WriteCount(out, 16);
	soff_t sz_at = out->GetPosition();
	// block size placeholder
	((flags & kDataExt_File64) != 0) ?
		out->WriteInt64(0) :
		out->WriteInt32(0);

	// Call writer to save actual block contents
	writer(out);

	// Now calculate the block's size...
	soff_t end_at = out->GetPosition();
	soff_t block_size = (end_at - sz_at) - sizeof(int64_t);
	// ...return back and write block's size in the placeholder
	out->Seek(sz_at, Shared::kSeekBegin);
	((flags & kDataExt_File64) != 0) ?
		out->WriteInt64(block_size) :
		out->WriteInt32((int32_t)block_size);
	// ...and get back to the end of the file
	out->Seek(0, Shared::kSeekEnd);
}

} // namespace Common
} // namespace AGS
} // namespace AGS3
