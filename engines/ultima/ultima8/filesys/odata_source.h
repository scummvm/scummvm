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

#ifndef ULTIMA8_FILESYS_ODATASOURCE_H
#define ULTIMA8_FILESYS_ODATASOURCE_H

#include "ultima/ultima8/misc/common_types.h"
#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/shared/std/containers.h"
#include "common/stream.h"
#include "common/algorithm.h"

namespace Ultima {
namespace Ultima8 {

class ODataSource : public Common::SeekableWriteStream {
public:
	ODataSource() {}
	virtual ~ODataSource() {}

	virtual void writeUint24LE(uint32 val) {
		writeByte(static_cast<byte>(val & 0xff));
		writeByte(static_cast<byte>((val >> 8) & 0xff));
		writeByte(static_cast<byte>((val >> 16) & 0xff));
	}

	void writeX(uint32 val, uint32 num_bytes) {
		assert(num_bytes > 0 && num_bytes <= 4);
		if (num_bytes == 1) writeByte(static_cast<byte>(val));
		else if (num_bytes == 2) writeUint16LE(static_cast<uint16>(val));
		else if (num_bytes == 3) writeUint24LE(val);
		else writeUint32LE(val);
	}

	virtual void skip(int32 delta) {
		seek(delta, SEEK_CUR);
	};
};


class OAutoBufferDataSource: public ODataSource {
protected:
	uint8 *_buf;
	uint8 *_bufPtr;
	uint32 _size;
	uint32 _loc;
	uint32 _allocated;

	void checkResize(uint32 num_bytes) {
		// Increment loc
		_loc += num_bytes;

		// Don't need to resize
		if (_loc <= _size) return;

		// Reallocate the buffer
		if (_loc > _allocated) {
			// The old pointer position
			uint32 position = static_cast<uint32>(_bufPtr - _buf);

			// The new buffer and size (2 times what is needed)
			_allocated = _loc * 2;
			uint8 *new_buf = new uint8[_allocated];

			memcpy(new_buf, _buf, _size);
			delete [] _buf;

			_buf = new_buf;
			_bufPtr = _buf + position;
		}

		// Update size
		_size = _loc;
	}

public:
	OAutoBufferDataSource(uint32 initial_len) : _size(0), _loc(0), _allocated(initial_len) {
		// Make the min allocated size 16 bytes
		if (_allocated < 16)
			_allocated = 16;
		_buf = _bufPtr = new uint8[_allocated];
	};

	//! Get a pointer to the data buffer.
	const uint8 *getData() {
		return _buf;
	}

	~OAutoBufferDataSource() override {
		delete [] _buf;
	}

	uint32 write(const void *b, uint32 len) override {
		checkResize(len);
		Common::copy((const byte *)b, (const byte *)b + len, _bufPtr);
		_bufPtr += len;
		return len;
	};

	bool seek(int32 position, int whence = SEEK_SET) override {
		assert(whence == SEEK_SET);
		// No seeking past the end of the buffer
		if (position <= static_cast<int32>(_size)) _loc = position;
		else _loc = _size;

		_bufPtr = const_cast<unsigned char *>(_buf) + _loc;
		return true;
	};

	void skip(int32 position) override {
		// No seeking past the end
		if (position >= 0) {
			_loc += position;
			if (_loc > _size) _loc = _size;
		}
		// No seeking past the start
		else {
			uint32 invpos = -position;
			if (invpos > _loc) invpos = _loc;
			_loc -= invpos;
		}
		_bufPtr = const_cast<unsigned char *>(_buf) + _loc;
	};

	int32 size() const override {
		return _size;
	};

	int32 pos() const override {
		return static_cast<int32>(_bufPtr - _buf);
	};

	// Don't actually do anything substantial
	virtual void clear() {
		_bufPtr = _buf;
		_size = 0;
		_loc = 0;
	}
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
