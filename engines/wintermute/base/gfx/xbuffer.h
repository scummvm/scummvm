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

#ifndef WINTERMUTE_XBUFFER_H
#define WINTERMUTE_XBUFFER_H

#include <common/types.h>
#include <common/util.h>

namespace Wintermute {

struct DXBuffer {
private:

	byte *_ptr;
	uint64 _size;

public:

	DXBuffer() {
		_ptr = nullptr;
		_size = 0;
	}

	DXBuffer(uint64 size) {
		_ptr = new uint8_t[size];
		if (_ptr == nullptr) {
			size = 0;
			return;
		}
		_size = size;
	}

	DXBuffer(const uint8 *ptr, uint64 size) {
		_ptr = new uint8[size];
		if (_ptr == nullptr) {
			size = 0;
			return;
		}
		memcpy(_ptr, ptr, size);
		_size = size;
	}

	void free() {
		delete[] _ptr;
		_ptr = nullptr;
		_size = 0;
	}

	byte *ptr() const {
		return _ptr;
	}

	uint64 size() const {
		return _size;
	}
};

} // namespace Wintermute

#endif
