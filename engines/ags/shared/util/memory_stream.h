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
 // MemoryStream does reading and writing over the buffer of bytes stored in
 // memory. Currently has rather trivial implementation. Does not own a buffer
 // itself, but works with the provided C-buffer pointer, which means that the
 // buffer object *must* persist until stream is closed.
 //
 // VectorStream is a specialized implementation that works with std::vector.
 // Unlike base MemoryStream provides continiously resizing buffer for writing.
 // TODO: separate StringStream for reading & writing String object?
 //
 //=============================================================================

#ifndef AGS_SHARED_UTIL_MEMORY_STREAM_H
#define AGS_SHARED_UTIL_MEMORY_STREAM_H

#include "common/std/vector.h"
#include "ags/shared/util/data_stream.h"
#include "ags/shared/util/string.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

class MemoryStream : public DataStream {
public:
	// Construct memory stream in the read-only mode over a const C-buffer;
	// reading will never exceed buf_sz bytes;
	// buffer must persist in memory until the stream is closed.
	MemoryStream(const uint8_t *cbuf, size_t buf_sz, DataEndianess stream_endianess = kLittleEndian);
	// Construct memory stream in the chosen mode over a given C-buffer;
	// neither reading nor writing will ever exceed buf_sz bytes;
	// buffer must persist in memory until the stream is closed.
	MemoryStream(uint8_t *buf, size_t buf_sz, StreamWorkMode mode, DataEndianess stream_endianess = kLittleEndian);
	~MemoryStream() override {}

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

	soff_t  Seek(soff_t offset, StreamSeek origin) override;

protected:
	const uint8_t           *_cbuf = nullptr; // readonly buffer ptr
	size_t                   _buf_sz = 0u; // hard buffer limit
	size_t                   _len = 0u; // calculated length of stream
	const StreamWorkMode     _mode;
	size_t                   _pos = 0u; // current stream pos

private:
	uint8_t                 *_buf = nullptr; // writeable buffer ptr
};


class VectorStream : public MemoryStream {
public:
	// Construct memory stream in the read-only mode over a const std::vector;
	// vector must persist in memory until the stream is closed.
	VectorStream(const std::vector<uint8_t> &cbuf, DataEndianess stream_endianess = kLittleEndian);
	// Construct memory stream in the chosen mode over a given std::vector;
	// vector must persist in memory until the stream is closed.
	VectorStream(std::vector<uint8_t> &buf, StreamWorkMode mode, DataEndianess stream_endianess = kLittleEndian);
	~VectorStream() override {}

	void    Close() override;

	bool    CanRead() const override;
	bool    CanWrite() const override;

	size_t  Write(const void *buffer, size_t size) override;
	int32_t WriteByte(uint8_t b) override;

private:
	std::vector<uint8_t> *_vec = nullptr; // writeable vector (may be null)
};

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
