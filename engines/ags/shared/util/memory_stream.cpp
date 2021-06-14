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

#include "ags/lib/std/algorithm.h"
#include "ags/shared/util/memory_stream.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

MemoryStream::MemoryStream(const std::vector<char> &cbuf, DataEndianess stream_endianess)
	: DataStream(stream_endianess)
	, _cbuf(&cbuf.front())
	, _len(cbuf.size())
	, _buf(nullptr)
	, _mode(kStream_Read)
	, _pos(0) {
}

MemoryStream::MemoryStream(const String &cbuf, DataEndianess stream_endianess)
	: DataStream(stream_endianess)
	, _cbuf(cbuf.GetCStr())
	, _len(cbuf.GetLength())
	, _buf(nullptr)
	, _mode(kStream_Read)
	, _pos(0) {
}

MemoryStream::MemoryStream(std::vector<char> &buf, StreamWorkMode mode, DataEndianess stream_endianess)
	: DataStream(stream_endianess)
	, _len(buf.size())
	, _buf(&buf)
	, _mode(mode)
	, _pos(buf.size()) {
	_cbuf = (mode == kStream_Read) ? &buf.front() : nullptr;
}

MemoryStream::~MemoryStream() {
}

void MemoryStream::Close() {
	_cbuf = nullptr;
	_buf = nullptr;
	_pos = -1;
}

bool MemoryStream::Flush() {
	return true;
}

bool MemoryStream::IsValid() const {
	return _cbuf != nullptr || _buf != nullptr;
}

bool MemoryStream::EOS() const {
	return _pos >= (soff_t)_len;
}

soff_t MemoryStream::GetLength() const {
	return _len;
}

soff_t MemoryStream::GetPosition() const {
	return _pos;
}

bool MemoryStream::CanRead() const {
	return (_cbuf != nullptr) && (_mode == kStream_Read);
}

bool MemoryStream::CanWrite() const {
	return (_buf != nullptr) && (_mode == kStream_Write);
}

bool MemoryStream::CanSeek() const {
	return CanRead(); // TODO: support seeking in writable stream?
}

size_t MemoryStream::Read(void *buffer, size_t size) {
	if (EOS()) {
		return 0;
	}
	soff_t remain = _len - _pos;
	assert(remain > 0);
	size_t read_sz = std::min((size_t)remain, size);
	memcpy(buffer, _cbuf + _pos, read_sz);
	_pos += read_sz;
	return read_sz;
}

int32_t MemoryStream::ReadByte() {
	if (EOS()) {
		return -1;
	}
	return _cbuf[(size_t)(_pos++)];
}

size_t MemoryStream::Write(const void *buffer, size_t size) {
	if (!_buf) {
		return 0;
	}
	_buf->resize(_buf->size() + size);
	memcpy(_buf->data() + _pos, buffer, size);
	_pos += size;
	_len += size;
	return size;
}

int32_t MemoryStream::WriteByte(uint8_t val) {
	if (!_buf) {
		return -1;
	}
	_buf->push_back(val);
	_pos++; _len++;
	return val;
}

bool MemoryStream::Seek(soff_t offset, StreamSeek origin) {
	if (!CanSeek()) {
		return false;
	}
	switch (origin) {
	case kSeekBegin:    _pos = 0 + offset; break;
	case kSeekCurrent:  _pos = _pos + offset; break;
	case kSeekEnd:      _pos = _len + offset; break;
	default:
		return false;
	}
	_pos = std::max<soff_t>(0, _pos);
	_pos = std::min<soff_t>(_len, _pos); // clamp to EOS
	return true;
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
