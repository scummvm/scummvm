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

#include "ultima/ultima8/misc/debugger.h"

#include "ultima/ultima8/world/map_glob.h"

namespace Ultima {
namespace Ultima8 {

MapGlob::MapGlob() {
}


MapGlob::~MapGlob() {
}

void MapGlob::read(Common::SeekableReadStream *rs) {
	unsigned int itemcount = rs->readUint16LE();
	assert(rs->size() >= static_cast<int32>(2 + itemcount * 6));
	_contents.clear();
	_contents.resize(itemcount);

	for (unsigned int i = 0; i < itemcount; ++i) {
		GlobItem item;

		item.x = rs->readByte();
		item.y = rs->readByte();
		item.z = rs->readByte();
		item.shape = rs->readUint16LE();
		item.frame = rs->readByte();

		_contents[i] = item;
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
