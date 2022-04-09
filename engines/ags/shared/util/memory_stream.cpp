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

#include "ags/lib/std/algorithm.h"
#include "ags/shared/util/memory_stream.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

MemoryStream::MemoryStream(const uint8_t *cbuf, size_t buf_sz, DataEndianess stream_endianess)
	: DataStream(stream_endianess)
	, _cbuf(cbuf)
	, _buf_sz(buf_sz)
	, _len(buf_sz)
	, _buf(nullptr)
	, _mode(kStream_Read)
	, _pos(0) {
}

MemoryStream::MemoryStream(uint8_t *buf, size_t buf_sz, StreamWorkMode mode, DataEndianess stream_endianess)
	: DataStream(stream_endianess)
	, _buf(buf)
	, _buf_sz(buf_sz)
	, _len(0)
	, _cbuf(nullptr)
	, _mode(mode)
	, _pos(0) {
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

size_t MemoryStream::Write(const void *buffer, size_t size) {
	if (_pos >= (soff_t)_buf_sz) { return 0; }
	size = std::min(size, _buf_sz - (size_t)_pos);
	memcpy(_buf + _pos, buffer, size);
	_pos += size;
	_len += size;
	return size;
}

int32_t MemoryStream::WriteByte(uint8_t val) {
	if (_pos >= (soff_t)_buf_sz) { return -1; }
	*(_buf + _pos) = val;
	_pos++; _len++;
	return val;
}


VectorStream::VectorStream(const std::vector<uint8_t> &cbuf, DataEndianess stream_endianess)
	: MemoryStream(&cbuf.front(), cbuf.size(), stream_endianess)
	, _vec(nullptr) {
}

VectorStream::VectorStream(std::vector<uint8_t> &buf, StreamWorkMode mode, DataEndianess stream_endianess)
	: MemoryStream((mode == kStream_Read) ? &buf.front() : nullptr, buf.size(), mode, stream_endianess)
	, _vec(&buf) {
}

void VectorStream::Close() {
	_vec = nullptr;
	MemoryStream::Close();
}

size_t VectorStream::Write(const void *buffer, size_t size) {
	_vec->resize(_vec->size() + size);
	memcpy(_vec->data() + _pos, buffer, size);
	_pos += size;
	_len += size;
	return size;
}

int32_t VectorStream::WriteByte(uint8_t val) {
	_vec->push_back(val);
	_pos++; _len++;
	return val;
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
