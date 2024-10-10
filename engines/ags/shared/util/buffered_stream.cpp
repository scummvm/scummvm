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
#include "common/std/algorithm.h"
#include "ags/shared/util/buffered_stream.h"
#include "ags/shared/util/stdio_compat.h"
#include "ags/shared/util/string.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

//-----------------------------------------------------------------------------
// BufferedStream
//-----------------------------------------------------------------------------

const size_t BufferedStream::BufferSize;

BufferedStream::BufferedStream(const String &file_name, FileOpenMode open_mode, FileWorkMode work_mode, DataEndianess stream_endianess)
	: FileStream(file_name, open_mode, work_mode, stream_endianess) {
	if (IsValid()) {
		soff_t end_pos = FileStream::Seek(0, kSeekEnd);
		if (end_pos >= 0) {
			_start = 0;
			_end = end_pos;
			if (FileStream::Seek(0, kSeekBegin) < 0)
				_end = -1;
		}

		if (_end == -1) {
			FileStream::Close();
			error("Error determining stream end.");
		}
	}
}

BufferedStream::~BufferedStream() {
	BufferedStream::Close();
}

void BufferedStream::FillBufferFromPosition(soff_t position) {
	FileStream::Seek(position, kSeekBegin);
	// remember to restrict to the end position!
	size_t fill_size = std::min(BufferSize, static_cast<size_t>(_end - position));
	_buffer.resize(fill_size);
	auto sz = FileStream::Read(_buffer.data(), fill_size);
	_buffer.resize(sz);
	_bufferPosition = position;
}

void BufferedStream::FlushBuffer(soff_t position) {
	size_t sz = _buffer.size() > 0 ? FileStream::Write(_buffer.data(), _buffer.size()) : 0u;
	_buffer.clear(); // will start from the clean buffer next time
	_bufferPosition += sz;
	if (position != _bufferPosition) {
		FileStream::Seek(position, kSeekBegin);
		_bufferPosition = position;
	}
}

bool BufferedStream::EOS() const {
	return _position == _end;
}

soff_t BufferedStream::GetLength() const {
	return _end - _start;
}

soff_t BufferedStream::GetPosition() const {
	return _position - _start;
}

void BufferedStream::Close() {
	if (GetWorkMode() == kFile_Write)
		FlushBuffer(_position);
	FileStream::Close();
}

bool BufferedStream::Flush() {
	if (GetWorkMode() == kFile_Write)
		FlushBuffer(_position);
	return FileStream::Flush();
}

size_t BufferedStream::Read(void *buffer, size_t size) {
	// If the read size is larger than the internal buffer size,
	// then read directly into the user buffer and bail out.
	if (size >= BufferSize) {
		FileStream::Seek(_position, kSeekBegin);
		// remember to restrict to the end position!
		size_t fill_size = std::min(size, static_cast<size_t>(_end - _position));
		size_t sz = FileStream::Read(buffer, fill_size);
		_position += sz;
		return sz;
	}

	auto *to = static_cast<uint8_t*>(buffer);

	while(size > 0) {
		if (_position < _bufferPosition || _position >= _bufferPosition + _buffer.size()) {
			FillBufferFromPosition(_position);
		}
		if (_buffer.empty()) { break; } // reached EOS
		assert(_position >= _bufferPosition && _position < _bufferPosition + _buffer.size());

		soff_t bufferOffset = _position - _bufferPosition;
		assert(bufferOffset >= 0);
		size_t bytesLeft = _buffer.size() - (size_t)bufferOffset;
		size_t chunkSize = MIN<size_t>(bytesLeft, size);

		memcpy(to, _buffer.data() + bufferOffset, chunkSize);

		to += chunkSize;
		_position += chunkSize;
		size -= chunkSize;
	}
	return to - static_cast<uint8_t*>(buffer);
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
	const uint8_t *from = static_cast<const uint8_t*>(buffer);
	while (size > 0) {
		if (_position < _bufferPosition || // seek'd before buffer pos
			_position > _bufferPosition + _buffer.size() || // seek'd beyond buffer pos
			_position >= _bufferPosition + (soff_t) BufferSize) // seek'd, or exceeded buffer limit
		{
			FlushBuffer(_position);
		}
		size_t pos_in_buff = static_cast<size_t>(_position - _bufferPosition);
		size_t chunk_sz = std::min(size, BufferSize - pos_in_buff);
		if (_buffer.size() < pos_in_buff + chunk_sz)
			_buffer.resize(pos_in_buff + chunk_sz);
		memcpy(_buffer.data() + pos_in_buff, from, chunk_sz);
		_position += chunk_sz;
		from += chunk_sz;
		size -= chunk_sz;
	}

	_end = std::max(_end, _position);
	return from - static_cast<const uint8_t*>(buffer);

}

int32_t BufferedStream::WriteByte(uint8_t val) {
	auto sz = Write(&val, 1);
	if (sz != 1) {
		return -1;
	}
	return val;
}

soff_t BufferedStream::Seek(soff_t offset, StreamSeek origin) {
	soff_t want_pos = -1;
	switch (origin) {
		case StreamSeek::kSeekCurrent:  want_pos = _position + offset; break;
		case StreamSeek::kSeekBegin:    want_pos = _start + offset; break;
		case StreamSeek::kSeekEnd:      want_pos = _end + offset; break;
		default: return -1;
	}
	// clamp to the valid range
	_position = MIN(MAX(want_pos, _start), _end);
	return _position - _start; // convert to a stream section pos
}

//-----------------------------------------------------------------------------
// BufferedSectionStream
//-----------------------------------------------------------------------------

BufferedSectionStream::BufferedSectionStream(const String &file_name, soff_t start_pos, soff_t end_pos,
	FileOpenMode open_mode, FileWorkMode work_mode, DataEndianess stream_endianess)
	: BufferedStream(file_name, open_mode, work_mode, stream_endianess) {
	assert(start_pos <= end_pos);
	start_pos = MIN(start_pos, end_pos);
	_start = MIN(start_pos, _end);
	_end = MIN(end_pos, _end);
	BufferedStream::Seek(0, kSeekBegin);
}


} // namespace Shared
} // namespace AGS
} // namespace AGS3
