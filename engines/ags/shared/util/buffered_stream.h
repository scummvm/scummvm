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

// BufferedStream represents a buffered file stream; uses memory buffer
// during read and write operations to limit number reads and writes on disk
// and thus improve i/o performance.
//
// BufferedSectionStream is a subclass stream that limits reading by an
// arbitrary offset range.

#ifndef AGS_SHARED_UTIL_BUFFEREDSTREAM_H
#define AGS_SHARED_UTIL_BUFFEREDSTREAM_H

#include "common/std/vector.h"
#include "ags/shared/util/file_stream.h"
#include "ags/shared/util/file.h" // TODO: extract filestream mode constants

namespace AGS3 {
namespace AGS {
namespace Shared {

class BufferedStream : public FileStream {
public:
	// Needs tuning depending on the platform.
	static const size_t BufferSize = 1024u * 8;

	// The constructor may raise std::runtime_error if
	// - there is an issue opening the file (does not exist, locked, permissions, etc)
	// - the open mode could not be determined
	// - could not determine the length of the stream
	// It is recommended to use File::OpenFile to safely construct this object.
	BufferedStream(const String &file_name, FileOpenMode open_mode, FileWorkMode work_mode, DataEndianess stream_endianess = kLittleEndian);
	~BufferedStream();

	// Is end of stream
	bool    EOS() const override;
	// Total length of stream (if known)
	soff_t  GetLength() const override;
	// Current position (if known)
	soff_t  GetPosition() const override;

	void    Close() override;
	bool    Flush() override;

	size_t  Read(void *buffer, size_t size) override;
	int32_t ReadByte() override;
	size_t  Write(const void *buffer, size_t size) override;
	int32_t WriteByte(uint8_t b) override;

	soff_t  Seek(soff_t offset, StreamSeek origin) override;

protected:
	soff_t _start = 0; // valid section starting offset
	soff_t _end = -1; // valid section ending offset

private:
	// Reads a chunk of file into the buffer, starting from the given offset
	void FillBufferFromPosition(soff_t position);
	// Writes a buffer into the file, and reposition to the new offset
	void FlushBuffer(soff_t position);

	soff_t _position = 0; // absolute read/write offset
	soff_t _bufferPosition = 0; // buffer's location relative to file
	std::vector<uint8_t> _buffer;
};


// Creates a BufferedStream limited by an arbitrary offset range
class BufferedSectionStream : public BufferedStream {
public:
	BufferedSectionStream(const String &file_name, soff_t start_pos, soff_t end_pos,
		FileOpenMode open_mode, FileWorkMode work_mode, DataEndianess stream_endianess = kLittleEndian);
};

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
