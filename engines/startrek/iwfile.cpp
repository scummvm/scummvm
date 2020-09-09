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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "startrek/iwfile.h"
#include "startrek/resource.h"

namespace StarTrek {

IWFile::IWFile(StarTrekEngine *vm, const Common::String &filename) {
	debug(6, "IW File: %s", filename.c_str());

	_vm = vm;

	Common::MemoryReadStreamEndian *file = _vm->_resource->loadFile(filename);
	_numEntries = file->readUint16();

	assert(_numEntries < MAX_KEY_POSITIONS);

	for (int i = 0; i < MAX_KEY_POSITIONS; i++) {
		int16 x = file->readUint16();
		int16 y = file->readUint16();
		_keyPositions[i] = Common::Point(x, y);
	}

	for (int i = 0; i < _numEntries; i++) {
		file->read(_iwEntries[i], _numEntries);
	}

	delete file;
}

// FIXME: same issue with sorting as with "compareSpritesByLayer" in graphics.cpp.
bool iwSorter(const Common::Point &p1, const Common::Point &p2) {
	return p1.y < p2.y;
}

int IWFile::getClosestKeyPosition(int16 x, int16 y) {
	// This is a sorted list of indices from 0 to _numEntries-1.
	// The index is stored in Point.x, and the "cost" (distance from position) is stored
	// in Point.y for sorting purposes.
	Common::Point sortedIndices[MAX_KEY_POSITIONS];

	for (int i = 0; i < _numEntries; i++) {
		sortedIndices[i].x = i;
		sortedIndices[i].y = (int16)sqrt((double)_keyPositions[i].sqrDist(Common::Point(x, y)));
	}

	sort(sortedIndices, sortedIndices + _numEntries, &iwSorter);

	// Iterate through positions from closest to furthest
	for (int i = 0; i < _numEntries; i++) {
		int index = sortedIndices[i].x;
		Common::Point dest = _keyPositions[index];
		if (_vm->directPathExists(x, y, dest.x, dest.y))
			return index;
	}

	return -1;
}

} // End of namespace StarTrek
