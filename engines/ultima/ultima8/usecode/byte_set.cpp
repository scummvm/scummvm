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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/usecode/byte_set.h"

namespace Ultima {
namespace Ultima8 {

ByteSet::ByteSet() : _size(0), _data(nullptr) {
}


ByteSet::ByteSet(unsigned int size) : _data(nullptr) {
	setSize(size);
}

ByteSet::~ByteSet() {
	delete[] _data;
}

void ByteSet::setSize(unsigned int size) {
	if (_data) delete[] _data;

	_size = size;
	_data = new uint8[_size];
	for (unsigned int i = 0; i < _size; ++i)
		_data[i] = 0;
}

uint32 ByteSet::getEntries(unsigned int pos, unsigned int n) const {
	assert(n <= 2);
	assert(pos + n <= _size);
	if (n == 0) return 0;

    if (n == 1) {
        return _data[pos];
    } else if (n == 2) {
        return (_data[pos] << 8) | _data[pos + 1];
    }
	return 0;
}

void ByteSet::setEntries(unsigned int pos, unsigned int n, uint32 val) {
	assert(n <= 2);
	assert(pos + n <= _size);
	if (n == 0) return;

    if (n == 1) {
        _data[pos] = static_cast<uint8>(val);
    } else if (n == 2) {
        _data[pos] = static_cast<uint8>((val & 0xFF00) >> 8);
        _data[pos + 1] = static_cast<uint8>(val & 0xFF);
    }
}

void ByteSet::save(Common::WriteStream *ws) {
	ws->writeUint32LE(_size);
	ws->write(_data, _size);
}

bool ByteSet::load(Common::ReadStream *rs, uint32 version) {
	uint32 s = rs->readUint32LE();
	setSize(s);
	rs->read(_data, _size);

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
