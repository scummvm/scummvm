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

#include "ags/shared/util/datastream.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

DataStream::DataStream(DataEndianess stream_endianess)
	: _streamEndianess(stream_endianess) {
}

DataStream::~DataStream() {}

int16 DataStream::ReadInt16() {
	int16 val = 0;
	Read(&val, sizeof(int16));
	ConvertInt16(val);
	return val;
}

int DataStream::ReadInt32() {
	int val = 0;
	Read(&val, sizeof(int));
	ConvertInt32(val);
	return val;
}

int64 DataStream::ReadInt64() {
	int64 val = 0;
	Read(&val, sizeof(int64));
	ConvertInt64(val);
	return val;
}

size_t DataStream::WriteInt16(int16 val) {
	ConvertInt16(val);
	return Write(&val, sizeof(int16));
}

size_t DataStream::WriteInt32(int val) {
	ConvertInt32(val);
	return Write(&val, sizeof(int));
}

size_t DataStream::WriteInt64(int64 val) {
	ConvertInt64(val);
	return Write(&val, sizeof(int64));
}

size_t DataStream::ReadAndConvertArrayOfInt16(int16 *buffer, size_t count) {
	if (!CanRead() || !buffer) {
		return 0;
	}

	count = ReadArray(buffer, sizeof(int16), count);
	for (size_t i = 0; i < count; ++i, ++buffer) {
		*buffer = BBOp::SwapBytesInt16(*buffer);
	}
	return count;
}

size_t DataStream::ReadAndConvertArrayOfInt32(int *buffer, size_t count) {
	if (!CanRead() || !buffer) {
		return 0;
	}

	count = ReadArray(buffer, sizeof(int), count);
	for (size_t i = 0; i < count; ++i, ++buffer) {
		*buffer = BBOp::SwapBytesInt32(*buffer);
	}
	return count;
}

size_t DataStream::ReadAndConvertArrayOfInt64(int64 *buffer, size_t count) {
	if (!CanRead() || !buffer) {
		return 0;
	}

	count = ReadArray(buffer, sizeof(int64), count);
	for (size_t i = 0; i < count; ++i, ++buffer) {
		*buffer = BBOp::SwapBytesInt64(*buffer);
	}
	return count;
}

size_t DataStream::WriteAndConvertArrayOfInt16(const int16 *buffer, size_t count) {
	if (!CanWrite() || !buffer) {
		return 0;
	}

	size_t elem;
	for (elem = 0; elem < count && !EOS(); ++elem, ++buffer) {
		int16 val = *buffer;
		ConvertInt16(val);
		if (Write(&val, sizeof(int16)) < sizeof(int16)) {
			break;
		}
	}
	return elem;
}

size_t DataStream::WriteAndConvertArrayOfInt32(const int *buffer, size_t count) {
	if (!CanWrite() || !buffer) {
		return 0;
	}

	size_t elem;
	for (elem = 0; elem < count && !EOS(); ++elem, ++buffer) {
		int val = *buffer;
		ConvertInt32(val);
		if (Write(&val, sizeof(int)) < sizeof(int)) {
			break;
		}
	}
	return elem;
}

size_t DataStream::WriteAndConvertArrayOfInt64(const int64 *buffer, size_t count) {
	if (!CanWrite() || !buffer) {
		return 0;
	}

	size_t elem;
	for (elem = 0; elem < count && !EOS(); ++elem, ++buffer) {
		int64 val = *buffer;
		ConvertInt64(val);
		if (Write(&val, sizeof(int64)) < sizeof(int64)) {
			break;
		}
	}
	return elem;
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
