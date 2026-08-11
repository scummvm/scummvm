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

#include "common/debug.h"
#include "darkseed/anm.h"

namespace Darkseed {
bool Anm::load(const Common::Path &filename, int deltaOffset) {
	close();
	if (!_file.open(filename)) {
		return false;
	}
	_numRecords = _file.readUint16LE();
	_assetFileId = _file.readUint16LE();
	_deltaOffset = deltaOffset;
	return true;
}

void Anm::close() {
	if (_file.isOpen()) {
		_file.close();
	}
}

bool Anm::getImg(uint16 index, Img &img, bool includesPosition) {
	_file.seek(4 + index * 2);
	int offset = _file.readUint16LE() + _deltaOffset;
	_file.seek((offset * 16) + (4 + _numRecords * 2));
	if (includesPosition) {
		img.load(_file);
	} else {
		img.loadWithoutPosition(_file);
	}
	debug("Loaded %d (%d,%d) (%d,%d) %x", index, img.getX(), img.getY(), img.getWidth(), img.getHeight(), 0);

	return false;
}

int Anm::numImages() const {
	return _numRecords;
}

} // namespace Darkseed
