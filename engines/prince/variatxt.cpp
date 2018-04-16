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

#include "prince/variatxt.h"
#include "common/debug.h"

namespace Prince {

VariaTxt::VariaTxt() : _dataSize(0), _data(nullptr) {
}

VariaTxt::~VariaTxt() {
	_dataSize = 0;
	if (_data != nullptr) {
		free(_data);
		_data = nullptr;
	}
}


bool VariaTxt::loadStream(Common::SeekableReadStream &stream) {
	_dataSize = stream.size();
	_data = (byte *)malloc(_dataSize);
	stream.read(_data, _dataSize);
	return true;
}

byte *VariaTxt::getString(uint32 stringId) {
	uint32 stringOffset = READ_LE_UINT32(_data + stringId * 4);
	if (stringOffset > _dataSize) {
		assert(false);
	}
	return _data + stringOffset;
}

} // End of namespace Prince
