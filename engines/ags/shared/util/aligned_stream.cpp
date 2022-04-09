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

#include "ags/shared/util/aligned_stream.h"
#include "ags/shared/util/math.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

AlignedStream::AlignedStream(Stream *stream, AlignedStreamMode mode, ObjectOwnershipPolicy stream_ownership_policy,
                             size_t base_alignment)
	: ProxyStream(stream, stream_ownership_policy)
	, _mode(mode)
	, _baseAlignment(base_alignment)
	, _maxAlignment(0)
	, _block(0) {
}

AlignedStream::~AlignedStream() {
	AlignedStream::Close();
}

void AlignedStream::Reset() {
	FinalizeBlock();
}

void AlignedStream::Close() {
	FinalizeBlock();
	ProxyStream::Close();
}

bool AlignedStream::CanRead() const {
	return (_mode == kAligned_Read && _stream->CanRead());
}

bool AlignedStream::CanWrite() const {
	return (_mode == kAligned_Write && _stream->CanWrite());
}

bool AlignedStream::CanSeek() const {
	// Aligned stream does not support seeking, hence that will break padding count
	return false;
}

size_t AlignedStream::Read(void *buffer, size_t size) {
	ReadPadding(sizeof(int8_t));
	size = _stream->Read(buffer, size);
	_block += size;
	return size;
}

int32_t AlignedStream::ReadByte() {
	ReadPadding(sizeof(uint8_t));
	int32_t b = _stream->ReadByte();
	_block += sizeof(uint8_t);
	return b;
}

int16_t AlignedStream::ReadInt16() {
	ReadPadding(sizeof(int16_t));
	int16_t val = _stream->ReadInt16();
	_block += sizeof(int16_t);
	return val;
}

int32_t AlignedStream::ReadInt32() {
	ReadPadding(sizeof(int32_t));
	int32_t val = _stream->ReadInt32();
	_block += sizeof(int32_t);
	return val;
}

int64_t AlignedStream::ReadInt64() {
	ReadPadding(sizeof(int64_t));
	int64_t val = _stream->ReadInt64();
	_block += sizeof(int64_t);
	return val;
}

size_t AlignedStream::ReadArray(void *buffer, size_t elem_size, size_t count) {
	ReadPadding(elem_size);
	count = _stream->ReadArray(buffer, elem_size, count);
	_block += count * elem_size;
	return count;
}

size_t AlignedStream::ReadArrayOfInt16(int16_t *buffer, size_t count) {
	ReadPadding(sizeof(int16_t));
	count = _stream->ReadArrayOfInt16(buffer, count);
	_block += count * sizeof(int16_t);
	return count;
}

size_t AlignedStream::ReadArrayOfInt32(int32_t *buffer, size_t count) {
	ReadPadding(sizeof(int32_t));
	count = _stream->ReadArrayOfInt32(buffer, count);
	_block += count * sizeof(int32_t);
	return count;
}

size_t AlignedStream::ReadArrayOfInt64(int64_t *buffer, size_t count) {
	ReadPadding(sizeof(int64_t));
	count = _stream->ReadArrayOfInt64(buffer, count);
	_block += count * sizeof(int64_t);
	return count;
}

size_t AlignedStream::Write(const void *buffer, size_t size) {
	WritePadding(sizeof(int8_t));
	size = _stream->Write(buffer, size);
	_block += size;
	return size;
}

int32_t AlignedStream::WriteByte(uint8_t b) {
	WritePadding(sizeof(uint8_t));
	int32_t res = _stream->WriteByte(b);
	_block += sizeof(uint8_t);
	return res;
}

size_t AlignedStream::WriteInt16(int16_t val) {
	WritePadding(sizeof(int16_t));
	size_t size = _stream->WriteInt16(val);
	_block += sizeof(int16_t);
	return size;
}

size_t AlignedStream::WriteInt32(int32_t val) {
	WritePadding(sizeof(int32_t));
	size_t size = _stream->WriteInt32(val);
	_block += sizeof(int32_t);
	return size;
}

size_t AlignedStream::WriteInt64(int64_t val) {
	WritePadding(sizeof(int64_t));
	size_t size = _stream->WriteInt64(val);
	_block += sizeof(int64_t);
	return size;
}

size_t AlignedStream::WriteArray(const void *buffer, size_t elem_size, size_t count) {
	WritePadding(elem_size);
	count = _stream->WriteArray(buffer, elem_size, count);
	_block += count * elem_size;
	return count;
}

size_t AlignedStream::WriteArrayOfInt16(const int16_t *buffer, size_t count) {
	WritePadding(sizeof(int16_t));
	count = _stream->WriteArrayOfInt16(buffer, count);
	_block += count * sizeof(int16_t);
	return count;
}

size_t AlignedStream::WriteArrayOfInt32(const int32_t *buffer, size_t count) {
	WritePadding(sizeof(int32_t));
	count = _stream->WriteArrayOfInt32(buffer, count);
	_block += count * sizeof(int32_t);
	return count;
}

size_t AlignedStream::WriteArrayOfInt64(const int64_t *buffer, size_t count) {
	WritePadding(sizeof(int64_t));
	count = _stream->WriteArrayOfInt64(buffer, count);
	_block += count * sizeof(int64_t);
	return count;
}

bool AlignedStream::Seek(soff_t offset, StreamSeek origin) {
	// TODO: split out Seekable Stream interface
	return false;
}

void AlignedStream::ReadPadding(size_t next_type) {
	if (next_type == 0) {
		return;
	}

	// The next is going to be evenly aligned data type,
	// therefore a padding check must be made
	if (next_type % _baseAlignment == 0) {
		int pad = _block % next_type;
		// Read padding only if have to
		if (pad) {
			// We do not know and should not care if the underlying stream
			// supports seek, so use read to skip the padding instead.
			for (size_t i = next_type - pad; i > 0; --i)
				_stream->ReadByte();
			_block += next_type - pad;
		}

		_maxAlignment = Math::Max(_maxAlignment, next_type);
		// Data is evenly aligned now
		if (_block % LargestPossibleType == 0) {
			_block = 0;
		}
	}
}

void AlignedStream::WritePadding(size_t next_type) {
	if (next_type == 0) {
		return;
	}

	// The next is going to be evenly aligned data type,
	// therefore a padding check must be made
	if (next_type % _baseAlignment == 0) {
		int pad = _block % next_type;
		// Write padding only if have to
		if (pad) {
			_stream->WriteByteCount(0, next_type - pad);
			_block += next_type - pad;
		}

		_maxAlignment = Math::Max(_maxAlignment, next_type);
		// Data is evenly aligned now
		if (_block % LargestPossibleType == 0) {
			_block = 0;
		}
	}
}

void AlignedStream::FinalizeBlock() {
	// Force the stream to read or write remaining padding to match the alignment
	if (_mode == kAligned_Read) {
		ReadPadding(_maxAlignment);
	} else if (_mode == kAligned_Write) {
		WritePadding(_maxAlignment);
	}

	_maxAlignment = 0;
	_block = 0;
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
