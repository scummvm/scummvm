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
#include "ultima/ultima2/data/map_monsters.h"
#include "ultima/ultima2/data/map.h"

namespace Ultima {
namespace Ultima2 {
namespace Data {

void MapMonsters::load(int mapEra, int mapType) {
	Common::File f;
	Common::String filename = monsterFilename(mapEra, mapType);
	if (!f.open(filename.c_str()))
		error("Could not open %s", filename.c_str());

	f.read(_mapX, MAP_MONSTER_COUNT);
	f.read(_mapY, MAP_MONSTER_COUNT);
	f.read(_spellHP, MAP_MONSTER_COUNT);
	f.read(_type, MAP_MONSTER_COUNT);
	f.read(_glyphTile, MAP_MONSTER_COUNT);
	f.read(_offerFlag, MAP_MONSTER_COUNT);
	f.read(_tempX, MAP_MONSTER_COUNT);
	f.read(_tempY, MAP_MONSTER_COUNT);
}

} // namespace Data
} // namespace Ultima2
} // namespace Ultima
