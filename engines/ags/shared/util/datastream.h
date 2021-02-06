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

//=============================================================================
//
// Standard AGS stream implementation for reading raw data with support for
// converting to opposite endianess. Most I/O devices should inherit this
// class and provide implementation for basic reading and writing only.
//
//=============================================================================

#ifndef AGS_SHARED_UTIL_DATASTREAM_H
#define AGS_SHARED_UTIL_DATASTREAM_H

#include "ags/shared/util/bbop.h"
#include "ags/shared/util/stream.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

class DataStream : public Stream {
public:
	DataStream(DataEndianess stream_endianess = kLittleEndian);
	~DataStream() override;

	int16_t ReadInt16() override;
	int32_t ReadInt32() override;
	int64_t ReadInt64() override;

	//
	// Read- and WriteArray methods return number of full elements (NOT bytes)
	// read or written, or -1 if end of stream is reached
	//
	// Note that ReadArray and WriteArray do NOT convert byte order even when
	// work with data of different endianess; they are meant for optimal
	// reading and writing blocks of raw bytes
	inline size_t ReadArray(void *buffer, size_t elem_size, size_t count) override {
		return Read(buffer, elem_size * count) / elem_size;
	}

	inline size_t ReadArrayOfInt16(int16_t *buffer, size_t count) override {
		return MustSwapBytes() ?
			ReadAndConvertArrayOfInt16(buffer, count) : ReadArray(buffer, sizeof(int16_t), count);
	}

	inline size_t ReadArrayOfInt32(int32_t *buffer, size_t count) override {
		return MustSwapBytes() ?
			ReadAndConvertArrayOfInt32(buffer, count) : ReadArray(buffer, sizeof(int32_t), count);
	}

	inline size_t ReadArrayOfInt64(int64_t *buffer, size_t count) override {
		return MustSwapBytes() ?
			ReadAndConvertArrayOfInt64(buffer, count) : ReadArray(buffer, sizeof(int64_t), count);
	}

	size_t  WriteInt16(int16_t val) override;
	size_t  WriteInt32(int32_t val) override;
	size_t  WriteInt64(int64_t val) override;

	inline size_t WriteArray(const void *buffer, size_t elem_size, size_t count) override {
		return Write(buffer, elem_size * count) / elem_size;
	}

	inline size_t WriteArrayOfInt16(const int16_t *buffer, size_t count) override {
		return MustSwapBytes() ?
			WriteAndConvertArrayOfInt16(buffer, count) : WriteArray(buffer, sizeof(int16_t), count);
	}

	inline size_t WriteArrayOfInt32(const int32_t *buffer, size_t count) override {
		return MustSwapBytes() ?
			WriteAndConvertArrayOfInt32(buffer, count) : WriteArray(buffer, sizeof(int32_t), count);
	}

	inline size_t WriteArrayOfInt64(const int64_t *buffer, size_t count) override {
		return MustSwapBytes() ?
			WriteAndConvertArrayOfInt64(buffer, count) : WriteArray(buffer, sizeof(int64_t), count);
	}

protected:
	DataEndianess _streamEndianess;

	// Helper methods for reading/writing arrays of basic types and
	// converting their elements to opposite endianess (swapping bytes).
	size_t  ReadAndConvertArrayOfInt16(int16_t *buffer, size_t count);
	size_t  ReadAndConvertArrayOfInt32(int32_t *buffer, size_t count);
	size_t  ReadAndConvertArrayOfInt64(int64_t *buffer, size_t count);
	size_t  WriteAndConvertArrayOfInt16(const int16_t *buffer, size_t count);
	size_t  WriteAndConvertArrayOfInt32(const int32_t *buffer, size_t count);
	size_t  WriteAndConvertArrayOfInt64(const int64_t *buffer, size_t count);

	inline bool MustSwapBytes() {
		return kDefaultSystemEndianess != _streamEndianess;
	}

	inline void ConvertInt16(int16_t &val) {
		if (MustSwapBytes()) val = BBOp::SwapBytesInt16(val);
	}
	inline void ConvertInt32(int32_t &val) {
		if (MustSwapBytes()) val = BBOp::SwapBytesInt32(val);
	}
	inline void ConvertInt64(int64_t &val) {
		if (MustSwapBytes()) val = BBOp::SwapBytesInt64(val);
	}
};

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
