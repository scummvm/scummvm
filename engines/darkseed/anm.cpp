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
bool Anm::load(const Common::Path &filename) {
	if (file.isOpen()) {
		file.close();
	}
	if (!file.open(filename)) {
		return false;
	}
	numRecords = file.readUint16LE();
	assetOffset = file.readUint16LE();

	return true;
}

bool Anm::getImg(uint16 index, Img &img, bool includesPosition) {
	file.seek(4 + index * 2);
	int offset = file.readUint16LE();
	file.seek((offset * 16) + (4 + numRecords * 2));
	if (includesPosition) {
		img.load(file);
	} else {
		img.loadWithoutPosition(file);
	}
	debug("Loaded %d (%d,%d) (%d,%d) %x", index, img.getX(), img.getY(), img.getWidth(), img.getHeight(), 0);

	return false;
}

int Anm::numImages() {
	return numRecords;
}

} // namespace Darkseed
