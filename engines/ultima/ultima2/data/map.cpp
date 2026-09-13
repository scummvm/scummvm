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

#include "common/file.h"
#include "ultima/ultima2/data/map.h"

namespace Ultima {
namespace Ultima2 {
namespace Data {

Common::String mapFilename(int mapNum1, int mapNum2) {
	return Common::String::format("MAPX%c%c", '0' + mapNum1, '0' + mapNum2);
}

Common::String monsterFilename(int mapNum1, int mapNum2) {
	return Common::String::format("MONX%c%c", '0' + mapNum1, '0' + mapNum2);
}

void Map::load(int mapNum1, int mapNum2) {
	Common::File f;
	Common::String filename = mapFilename(mapNum1, mapNum2);
	if (!f.open(filename.c_str()))
		error("Could not open %s", filename.c_str());

	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x)
			_tiles[y][x] = (TileId)(f.readByte() / 4);
	}

	_monsters.load(mapNum1, mapNum2);
}

} // namespace Data
} // namespace Ultima2
} // namespace Ultima
