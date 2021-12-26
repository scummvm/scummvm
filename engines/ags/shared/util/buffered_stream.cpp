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

//include <cstring>
#include "ags/lib/std/algorithm.h"
#include "ags/shared/util/buffered_stream.h"
#include "ags/shared/util/stdio_compat.h"
#include "ags/shared/util/string.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

BufferedStream::BufferedStream(const String &file_name, FileOpenMode open_mode, FileWorkMode work_mode, DataEndianess stream_endianess)
	: FileStream(file_name, open_mode, work_mode, stream_endianess), _buffer(BufferStreamSize), _bufferPosition(0), _position(0) {
	if (IsValid()) {
		if (FileStream::Seek(0, kSeekEnd) == false)
			error("Error determining stream end.");

		_end = -1;
		if (FileStream::Seek(0, kSeekEnd)) {
			_start = 0;
			_end = FileStream::GetPosition();
			if (!FileStream::Seek(0, kSeekBegin))
				_end = -1;
		}

		if (_end == -1) {
			FileStream::Close();
			error("Error determining stream end.");
		}
	}

	_buffer.resize(0);
}

void BufferedStream::FillBufferFromPosition(soff_t position) {
	FileStream::Seek(position, kSeekBegin);

	_buffer.resize(BufferStreamSize);
	auto sz = FileStream::Read(_buffer.data(), BufferStreamSize);
	_buffer.resize(sz);

	_bufferPosition = position;
}

bool BufferedStream::EOS() const {
	return _position == _end;
}

soff_t BufferedStream::GetPosition() const {
	return _position;
}

size_t BufferedStream::Read(void *toBuffer, size_t toSize) {
	auto to = static_cast<char *>(toBuffer);

	while (toSize > 0) {
		if (_position < _bufferPosition || _position >= _bufferPosition + _buffer.size()) {
			FillBufferFromPosition(_position);
		}
		if (_buffer.size() <= 0) {
			break;
		} // reached EOS
		assert(_position >= _bufferPosition && _position < _bufferPosition + _buffer.size());  // sanity check only, should be checked by above.

		soff_t bufferOffset = _position - _bufferPosition;
		assert(bufferOffset >= 0);
		size_t bytesLeft = _buffer.size() - (size_t)bufferOffset;
		size_t chunkSize = std::min<size_t>(bytesLeft, toSize);

		memcpy(to, _buffer.data() + bufferOffset, chunkSize);

		to += chunkSize;
		_position += chunkSize;
		toSize -= chunkSize;
	}

	return to - (char *)toBuffer;
}

int32_t BufferedStream::ReadByte() {
	uint8_t ch;
	auto bytesRead = Read(&ch, 1);
	if (bytesRead != 1) {
		return EOF;
	}
	return ch;
}

size_t BufferedStream::Write(const void *buffer, size_t size) {
	FileStream::Seek(_position, kSeekBegin);
	auto sz = FileStream::Write(buffer, size);
	if (_position == _end)
		_end += sz;
	_position += sz;
	return sz;
}

int32_t BufferedStream::WriteByte(uint8_t val) {
	auto sz = Write(&val, 1);
	if (sz != 1) {
		return -1;
	}
	return sz;
}

bool BufferedStream::Seek(soff_t offset, StreamSeek origin) {
	soff_t want_pos = -1;
	switch (origin) {
	case StreamSeek::kSeekCurrent:  want_pos = _position + offset; break;
	case StreamSeek::kSeekBegin:    want_pos = _start + offset; break;
	case StreamSeek::kSeekEnd:      want_pos = _end + offset; break;
	default: break;
	}

	// clamp
	_position = std::min(std::max(want_pos, (soff_t)_start), _end);
	return _position == want_pos;
}

BufferedSectionStream::BufferedSectionStream(const String &file_name, soff_t start_pos, soff_t end_pos,
	FileOpenMode open_mode, FileWorkMode work_mode, DataEndianess stream_endianess)
	: BufferedStream(file_name, open_mode, work_mode, stream_endianess) {
	assert(start_pos <= end_pos);
	start_pos = std::min(start_pos, end_pos);
	_start = std::min(start_pos, _end);
	_end = std::min(end_pos, _end);
	Seek(0, kSeekBegin);
}

soff_t BufferedSectionStream::GetPosition() const {
	return BufferedStream::GetPosition() - _start;
}

soff_t BufferedSectionStream::GetLength() const {
	return _end - _start;
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
