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
// MemoryStream does reading and writing over the buffer of chars stored in
// memory. Currently has rather trivial implementation. Does not own a buffer
// itself, but works with the provided std::vector reference, which means that
// the buffer *must* persist until stream is closed.
// TODO: perhaps accept const char* for reading mode, for compatibility with
// the older code, and maybe to let also read String objects?
// TODO: separate StringStream for reading & writing String object?
//
//=============================================================================

#ifndef AGS_SHARED_UTIL_MEMORY_STREAM_H
#define AGS_SHARED_UTIL_MEMORY_STREAM_H

#include "ags/lib/std/vector.h"
#include "ags/shared/util/data_stream.h"
#include "ags/shared/util/string.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

class MemoryStream : public DataStream {
public:
	// Construct memory stream in the read-only mode over a const std::vector;
	// vector must persist in memory until the stream is closed.
	MemoryStream(const std::vector<char> &cbuf, DataEndianess stream_endianess = kLittleEndian);
	// Construct memory stream in the read-only mode over a const String;
	// String object must persist in memory until the stream is closed.
	MemoryStream(const String &cbuf, DataEndianess stream_endianess = kLittleEndian);
	// Construct memory stream in the chosen mode over a given std::vector;
	// vector must persist in memory until the stream is closed.
	MemoryStream(std::vector<char> &buf, StreamWorkMode mode, DataEndianess stream_endianess = kLittleEndian);
	~MemoryStream() override;

	void    Close() override;
	bool    Flush() override;

	// Is stream valid (underlying data initialized properly)
	bool    IsValid() const override;
	// Is end of stream
	bool    EOS() const override;
	// Total length of stream (if known)
	soff_t  GetLength() const override;
	// Current position (if known)
	soff_t  GetPosition() const override;
	bool    CanRead() const override;
	bool    CanWrite() const override;
	bool    CanSeek() const override;

	size_t  Read(void *buffer, size_t size) override;
	int32_t ReadByte() override;
	size_t  Write(const void *buffer, size_t size) override;
	int32_t WriteByte(uint8_t b) override;

	bool    Seek(soff_t offset, StreamSeek origin) override;

private:
	const char *_cbuf;
	size_t _len;
	std::vector<char> *_buf;
	const StreamWorkMode _mode;
	soff_t _pos;
};

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
