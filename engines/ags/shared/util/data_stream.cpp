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

#include "ags/shared/util/data_stream.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

DataStream::DataStream(DataEndianess stream_endianess)
	: _streamEndianess(stream_endianess) {
}

DataStream::~DataStream() {}

int16_t DataStream::ReadInt16() {
	int16_t val = 0;
	Read(&val, sizeof(int16_t));
	ConvertInt16(val);
	return val;
}

int32_t DataStream::ReadInt32() {
	int32_t val = 0;
	Read(&val, sizeof(int32_t));
	ConvertInt32(val);
	return val;
}

int64_t DataStream::ReadInt64() {
	int64_t val = 0;
	Read(&val, sizeof(int64_t));
	ConvertInt64(val);
	return val;
}

size_t DataStream::WriteInt16(int16_t val) {
	ConvertInt16(val);
	return Write(&val, sizeof(int16_t));
}

size_t DataStream::WriteInt32(int32_t val) {
	ConvertInt32(val);
	return Write(&val, sizeof(int32_t));
}

size_t DataStream::WriteInt64(int64_t val) {
	ConvertInt64(val);
	return Write(&val, sizeof(int64_t));
}

size_t DataStream::ReadAndConvertArrayOfInt16(int16_t *buffer, size_t count) {
	count = ReadArray(buffer, sizeof(int16_t), count);
	for (size_t i = 0; i < count; ++i, ++buffer) {
		*buffer = BBOp::SwapBytesInt16(*buffer);
	}
	return count;
}

size_t DataStream::ReadAndConvertArrayOfInt32(int32_t *buffer, size_t count) {
	count = ReadArray(buffer, sizeof(int32_t), count);
	for (size_t i = 0; i < count; ++i, ++buffer) {
		*buffer = BBOp::SwapBytesInt32(*buffer);
	}
	return count;
}

size_t DataStream::ReadAndConvertArrayOfInt64(int64_t *buffer, size_t count) {
	count = ReadArray(buffer, sizeof(int64_t), count);
	for (size_t i = 0; i < count; ++i, ++buffer) {
		*buffer = BBOp::SwapBytesInt64(*buffer);
	}
	return count;
}

size_t DataStream::WriteAndConvertArrayOfInt16(const int16_t *buffer, size_t count) {
	size_t elem;
	for (elem = 0; elem < count && !EOS(); ++elem, ++buffer) {
		int16_t val = *buffer;
		ConvertInt16(val);
		if (Write(&val, sizeof(int16_t)) < sizeof(int16_t)) {
			break;
		}
	}
	return elem;
}

size_t DataStream::WriteAndConvertArrayOfInt32(const int32_t *buffer, size_t count) {
	size_t elem;
	for (elem = 0; elem < count && !EOS(); ++elem, ++buffer) {
		int32_t val = *buffer;
		ConvertInt32(val);
		if (Write(&val, sizeof(int32_t)) < sizeof(int32_t)) {
			break;
		}
	}
	return elem;
}

size_t DataStream::WriteAndConvertArrayOfInt64(const int64_t *buffer, size_t count) {
	size_t elem;
	for (elem = 0; elem < count && !EOS(); ++elem, ++buffer) {
		int64_t val = *buffer;
		ConvertInt64(val);
		if (Write(&val, sizeof(int64_t)) < sizeof(int64_t)) {
			break;
		}
	}
	return elem;
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
