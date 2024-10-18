//=============================================================================
//
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

//=============================================================================
//
// IAGSStream is a contract for stream class, provided by engine to plugin
// on the need, such as saving/restoring the game.
// The user is advised to use advanced helper methods, such as Read/WriteX
// and Read/WriteArrayOfX to allow the stream implementation properly control
// endianness conversions and data padding, when needed.
//
//=============================================================================

#ifndef AGS_SHARED_API_STREAM_API_H
#define AGS_SHARED_API_STREAM_API_H

// TODO: it would probably be better to not include core definition headers
// in API class headers, but make separate core headers specifically for
// plugins, and let plugin developers include them manually in plugin sources.
#include "ags/shared/core/types.h"

namespace AGS3 {

namespace AGS {
namespace Shared {

enum StreamSeek {
	kSeekBegin,
	kSeekCurrent,
	kSeekEnd
};

class IAGSStream {
public:
	virtual ~IAGSStream() {}

	virtual void        Close() = 0;

	virtual bool        IsValid() const = 0;
	virtual bool        EOS() const = 0;
	virtual soff_t      GetLength() const = 0;
	virtual soff_t      GetPosition() const = 0;
	virtual bool        CanRead() const = 0;
	virtual bool        CanWrite() const = 0;
	virtual bool        CanSeek() const = 0;

	// Reads number of bytes in the provided buffer
	virtual size_t      Read(void *buffer, size_t size) = 0;
	// ReadByte conforms to fgetc behavior:
	// - if stream is valid, then returns an *unsigned char* packed in the int
	// - if EOS, then returns -1
	virtual int32_t     ReadByte() = 0;
	// Writes number of bytes from the provided buffer
	virtual size_t      Write(const void *buffer, size_t size) = 0;
	// WriteByte conforms to fputc behavior:
	// - on success, returns the unsigned char packed in the int
	// - on failure, returns -1
	virtual int32_t     WriteByte(uint8_t b) = 0;

	// Convenience methods for reading values of particular size
	virtual int8_t      ReadInt8() = 0;
	virtual int16_t     ReadInt16() = 0;
	virtual int32_t     ReadInt32() = 0;
	virtual int64_t     ReadInt64() = 0;
	virtual bool        ReadBool() = 0;
	virtual size_t      ReadArray(void *buffer, size_t elem_size, size_t count) = 0;
	virtual size_t      ReadArrayOfInt8(int8_t *buffer, size_t count) = 0;
	virtual size_t      ReadArrayOfInt16(int16_t *buffer, size_t count) = 0;
	virtual size_t      ReadArrayOfInt32(int32_t *buffer, size_t count) = 0;
	virtual size_t      ReadArrayOfInt64(int64_t *buffer, size_t count) = 0;

	// Convenience methods for writing values of particular size
	virtual size_t      WriteInt8(int8_t val) = 0;
	virtual size_t      WriteInt16(int16_t val) = 0;
	virtual size_t      WriteInt32(int32_t val) = 0;
	virtual size_t      WriteInt64(int64_t val) = 0;
	virtual size_t      WriteBool(bool val) = 0;
	virtual size_t      WriteArray(const void *buffer, size_t elem_size, size_t count) = 0;
	virtual size_t      WriteArrayOfInt8(const int8_t *buffer, size_t count) = 0;
	virtual size_t      WriteArrayOfInt16(const int16_t *buffer, size_t count) = 0;
	virtual size_t      WriteArrayOfInt32(const int32_t *buffer, size_t count) = 0;
	virtual size_t      WriteArrayOfInt64(const int64_t *buffer, size_t count) = 0;

	virtual soff_t      Seek(soff_t offset, StreamSeek origin = kSeekCurrent) = 0;
};

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
