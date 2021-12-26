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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_ONCALL_H
#define SAGA2_ONCALL_H

namespace Saga2 {

class HandleArray {
private:
	Common::Array<byte*> _handles;
	uint32 _tileID;
	byte *(*_loader)(hResID, bool);
public:
	HandleArray(uint16 size, byte*(*loadfunction)(hResID, bool), uint32 newID) {
		for (int i = 0; i < size; ++i)
			_handles.push_back(nullptr);
		_loader = loadfunction;
		_tileID = newID;
	}

	void flush() {
		for (unsigned int i = 0; i < _handles.size(); ++i) {
			if (_handles[i]) {
				free(_handles[i]);
				_handles[i] = nullptr;
			}
		}
	}

	byte *operator[](uint32 ind) {
		if (_handles[ind])
			return _handles[ind];

		return _handles[ind] = _loader(_tileID + MKTAG(0, 0, 0, ind), false);
	}
};

} // end of namespace Saga2

#endif
