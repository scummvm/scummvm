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

#ifndef AGS_SHARED_UTIL_BUFFEREDSTREAM_H
#define AGS_SHARED_UTIL_BUFFEREDSTREAM_H

#include "ags/lib/std/vector.h"
#include "ags/shared/util/file_stream.h"
#include "ags/shared/util/file.h" // TODO: extract filestream mode constants

namespace AGS3 {
namespace AGS {
namespace Shared {

// Needs tuning depending on the platform.
const auto BufferStreamSize = 8 * 1024;

class BufferedStream : public FileStream {
public:
	// Represents an open _buffered_ file object
	// The constructor may raise std::runtime_error if
	// - there is an issue opening the file (does not exist, locked, permissions, etc)
	// - the open mode could not be determined
	// - could not determine the length of the stream
	// It is recommended to use File::OpenFile to safely construct this object.
	BufferedStream(const String &file_name, FileOpenMode open_mode, FileWorkMode work_mode, DataEndianess stream_endianess = kLittleEndian);

	bool    EOS() const override; ///< Is end of stream
	soff_t  GetPosition() const override; ///< Current position (if known)

	size_t  Read(void *buffer, size_t size) override;
	int32_t ReadByte() override;
	size_t  Write(const void *buffer, size_t size) override;
	int32_t WriteByte(uint8_t b) override;

	bool    Seek(soff_t offset, StreamSeek origin) override;

protected:
	soff_t _start;
	soff_t _end;

private:
	void FillBufferFromPosition(soff_t position);

	soff_t _position;
	soff_t _bufferPosition;
	std::vector<char> _buffer;
};


// Creates a BufferedStream limited by an arbitrary offset range
class BufferedSectionStream : public BufferedStream {
public:
	BufferedSectionStream(const String &file_name, soff_t start_pos, soff_t end_pos,
		FileOpenMode open_mode, FileWorkMode work_mode, DataEndianess stream_endianess = kLittleEndian);

	soff_t  GetPosition() const override;
	soff_t  GetLength() const override;
};

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
