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

#ifndef ULTIMA8_USECODE_UCSTACK_H
#define ULTIMA8_USECODE_UCSTACK_H

#include "common/scummsys.h"
#include "ultima/shared/std/misc.h"

namespace Ultima {
namespace Ultima8 {

// A little-endian stack for use with usecode
class BaseUCStack {
protected:
	uint8 *_buf;
	uint8 *_bufPtr;
	uint32 _size;
public:

	BaseUCStack(uint32 len, uint8 *b) : _buf(b), _size(len) {
		// stack grows downward, so start at the end of the buffer
		_bufPtr = _buf + _size;
	}
	virtual ~BaseUCStack() { }

	inline uint32 getSize() const {
		return _size;
	}

	inline uint32 stacksize() const {
		return _size - (_bufPtr - _buf);
	}

	inline void addSP(const int32 offset) {
		_bufPtr += offset;
	}

	inline unsigned int getSP() const {
		return static_cast<unsigned int>(_bufPtr - _buf);
	}

	inline void setSP(unsigned int pos) {
		_bufPtr = _buf + pos;
	}

	//
	// Push values to the stack
	//

	inline void push1(uint8 val) {
		_bufPtr--;
		_bufPtr[0] = val;
	}

	inline void push2(uint16 val) {
		_bufPtr -= 2;
		_bufPtr[0] = static_cast<uint8>(val     & 0xFF);
		_bufPtr[1] = static_cast<uint8>((val >> 8) & 0xFF);
	}
	inline void push4(uint32 val) {
		_bufPtr -= 4;
		_bufPtr[0] = static_cast<uint8>(val      & 0xFF);
		_bufPtr[1] = static_cast<uint8>((val >> 8)  & 0xFF);
		_bufPtr[2] = static_cast<uint8>((val >> 16) & 0xFF);
		_bufPtr[3] = static_cast<uint8>((val >> 24) & 0xFF);
	}
	// Push an arbitrary number of bytes of 0
	inline void push0(const uint32 count) {
		_bufPtr -= count;
		Std::memset(_bufPtr, 0, count);
	}
	// Push an arbitrary number of bytes
	inline void push(const uint8 *in, const uint32 count) {
		_bufPtr -= count;
		Std::memcpy(_bufPtr, in, count);
	}

	//
	// Pop values from the stack
	//

	inline uint16 pop2() {
		uint8 b0, b1;
		b0 = *_bufPtr++;
		b1 = *_bufPtr++;
		return (b0 | (b1 << 8));
	}
	inline uint32 pop4() {
		uint8 b0, b1, b2, b3;
		b0 = *_bufPtr++;
		b1 = *_bufPtr++;
		b2 = *_bufPtr++;
		b3 = *_bufPtr++;
		return (b0 | (b1 << 8) | (b2 << 16) | (b3 << 24));
	}
	inline void pop(uint8 *out, const uint32 count) {
		Std::memcpy(out, _bufPtr, count);
		_bufPtr += count;
	}

	//
	// Access a value from a location in the stack
	//

	inline uint8 access1(const uint32 offset) const {
		return _buf[offset];
	}
	inline uint16 access2(const uint32 offset) const {
		return (_buf[offset] | (_buf[offset + 1] << 8));
	}
	inline uint32 access4(const uint32 offset) const {
		return _buf[offset] | (_buf[offset + 1] << 8) |
		       (_buf[offset + 2] << 16) | (_buf[offset + 3] << 24);
	}
	inline uint8 *access(const uint32 offset) {
		return _buf + offset;
	}
	inline uint8 *access() {
		return _bufPtr;
	}

	//
	// Assign a value to a location in the stack
	//

	inline void assign1(const uint32 offset, const uint8 val) {
		const_cast<uint8 *>(_buf)[offset]   = static_cast<uint8>(val     & 0xFF);
	}
	inline void assign2(const uint32 offset, const uint16 val) {
		const_cast<uint8 *>(_buf)[offset]   = static_cast<uint8>(val     & 0xFF);
		const_cast<uint8 *>(_buf)[offset + 1] = static_cast<uint8>((val >> 8) & 0xFF);
	}
	inline void assign4(const uint32 offset, const uint32 val) {
		const_cast<uint8 *>(_buf)[offset]   = static_cast<uint8>(val      & 0xFF);
		const_cast<uint8 *>(_buf)[offset + 1] = static_cast<uint8>((val >> 8)  & 0xFF);
		const_cast<uint8 *>(_buf)[offset + 2] = static_cast<uint8>((val >> 16) & 0xFF);
		const_cast<uint8 *>(_buf)[offset + 3] = static_cast<uint8>((val >> 24) & 0xFF);
	}
	inline void assign(const uint32 offset, const uint8 *in, const uint32 len) {
		Std::memcpy(const_cast<uint8 *>(_buf) + offset, in, len);
	}
};

class DynamicUCStack : public BaseUCStack {
public:
	DynamicUCStack(uint32 len = 0x1000) : BaseUCStack(len, new uint8[len]) { }
	~DynamicUCStack() override {
		delete [] _buf;
	}

#ifdef USE_DYNAMIC_UCSTACK
#define UCStack DynamicUCStack
	void save(Common::WriteStream *ws);
	bool load(Common::ReadStream *rs, uint32 version);
#endif
};

#ifndef USE_DYNAMIC_UCSTACK
class UCStack : public BaseUCStack {
	uint8   _bufArray[0x1000];
public:
	UCStack() : BaseUCStack(0x1000, _bufArray) { }
	~UCStack() override { }

	void save(Common::WriteStream *ws);
	bool load(Common::ReadStream *rs, uint32 version);
};
#endif

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
