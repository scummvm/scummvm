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

#include "tesoro/core/archive.h"

namespace Tesoro {

bool Archive::open(const Common::Path &datName, const Common::Path &treName) {
	close();

	if (!treName.empty()) {
		Common::File tre;
		if (!tre.open(treName))
			return false;
		const uint32 size = (uint32)tre.size();
		byte *data = new byte[size];
		const uint32 got = tre.read(data, size);
		// builds the index from the TRE file data
		_index.load(data, got);
		delete[] data;
	}

	return _dat.open(datName);
}

void Archive::close() {
	_dat.close();
}

Common::SeekableReadStream *Archive::seekEntry(uint32 offset) {
	_dat.seek(offset, SEEK_SET);
	return &_dat;
}

uint32 Archive::pos() const {
	return (uint32)_dat.pos();
}

uint32 Archive::seekSavePos(uint32 offset) {
	const uint32 previous = pos();
	_dat.seek(offset, SEEK_SET);
	return previous;
}

} // End of namespace Tesoro
