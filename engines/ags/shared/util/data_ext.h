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
 // A simple data extension format which may be useful as an ammendment to
 // any data file in the game.
 //
 // Format consists of a list of "blocks", each preceded with an integer and an
 // optional string identifier, and a size in bytes which lets a reader to skip
 // the block completely if necessary.
 // Because the serialization algorithm was accomodated to be shared among
 // several existing data files, few things in the meta info may be read
 // slightly differently depending on flags passed into the function.
 //
 //-----------------------------------------------------------------------------
 //
 // Extension format description.
 //
 // Each block starts with the header.
 // * 1 or 4 bytes (depends on flags) - an old-style unsigned numeric ID :
 //   - where 0 would indicate following string ID,
 //   - and -1 (0xFF in case of 1 byte ID) indicates an end of the block list.
 // * 16 bytes - fixed-len string ID of an extension (if num ID == 0).
 // * 4 bytes - total length of the data in bytes;
 //   - does not include the size of the header (only following data).
 //   - new style blocks (w string ID) always use 8 bytes for a block len;
 // * Then goes regular data.
 //
 // After the block is read the stream position supposed to be at
 // (start + length of block), where "start" is a first byte of the header.
 // If it's further - the reader bails out with error. If it's not far enough -
 // the function logs a warning and skips remaining bytes.
 // Blocks are read until meeting ID == -1 in the next header, which indicates
 // the end of extension list. An EOF met earlier is considered an error.
 //
 //=============================================================================

#ifndef AGS_SHARED_UTIL_DATA_EXT_H
#define AGS_SHARED_UTIL_DATA_EXT_H

#include "ags/shared/util/error.h"
#include "ags/shared/util/string.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

enum DataExtFlags {
	// Size of a numeric ID in bytes
	kDataExt_NumID8 = 0x0000, // default
	kDataExt_NumID32 = 0x0001,
	// 32-bit or 64-bit file offset support
	kDataExt_File32 = 0x0000, // default
	kDataExt_File64 = 0x0002
};

enum DataExtErrorType {
	kDataExtErr_NoError,
	kDataExtErr_UnexpectedEOF,
	kDataExtErr_BlockDataOverlapping
};

String GetDataExtErrorText(DataExtErrorType err);
typedef TypedCodeError<DataExtErrorType, GetDataExtErrorText> DataExtError;


// Tries to opens a next block from the stream, fills in identifier and length on success
HError OpenExtBlock(Stream *in, int flags, int &block_id, String &ext_id, soff_t &block_len);
// Type of function that reads a single data block and tells whether to continue reading
typedef HError(*PfnReadExtBlock)(Stream *in, int block_id, const String &ext_id,
	soff_t block_len, bool &read_next);
// Parses stream as a block list, passing each found block into callback;
// does not read any actual data itself
HError ReadExtData(PfnReadExtBlock reader, int flags, Stream *in);

// Type of function that writes a single data block.
typedef void(*PfnWriteExtBlock)(Stream *out);
void WriteExtBlock(int block, const String &ext_id, PfnWriteExtBlock writer, int flags, Stream *out);
// Writes a block with a new-style string id
inline void WriteExtBlock(const String &ext_id, PfnWriteExtBlock writer, int flags, Stream *out) {
	WriteExtBlock(0, ext_id, writer, flags, out);
}
// Writes a block with a old-style numeric id
inline void WriteExtBlock(int block, PfnWriteExtBlock writer, int flags, Stream *out) {
	WriteExtBlock(block, String(), writer, flags, out);
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
