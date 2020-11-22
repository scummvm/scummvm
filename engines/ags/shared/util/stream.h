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
// Base stream class.
//
// Provides default implementation for a few helper methods.
//
// Only streams with uncommon behavior should be derived directly from Stream.
// Most I/O devices should inherit DataStream instead.
// Streams that wrap other streams should inherit ProxyStream.
//
//=============================================================================

#ifndef AGS_SHARED_UTIL_STREAM_H
#define AGS_SHARED_UTIL_STREAM_H

#include "api/stream_api.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

class Stream : public IAGSStream {
public:
	// Tells if the stream has errors
	virtual bool HasErrors() const {
		return false;
	}
	// Flush stream buffer to the underlying device
	virtual bool Flush() = 0;

	//-----------------------------------------------------
	// Helper methods
	//-----------------------------------------------------
	inline int8_t ReadInt8() override {
		return ReadByte();
	}

	inline size_t WriteInt8(int8_t val) override {
		int32_t ival = WriteByte(val);
		return ival >= 0 ? ival : 0;
	}

	inline bool ReadBool() override {
		return ReadInt8() != 0;
	}

	inline size_t WriteBool(bool val) override {
		return WriteInt8(val ? 1 : 0);
	}

	// Practically identical to Read() and Write(), these two helpers' only
	// meaning is to underline the purpose of data being (de)serialized
	inline size_t ReadArrayOfInt8(int8_t *buffer, size_t count) override {
		return Read(buffer, count);
	}
	inline size_t WriteArrayOfInt8(const int8_t *buffer, size_t count) override {
		return Write(buffer, count);
	}

	// Fill the requested number of bytes with particular value
	size_t WriteByteCount(uint8_t b, size_t count);
};

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
