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
#include "ultima/ultima2/ultima2.h"
#include "ultima/ultima2/logic/overworld_logic.h"
#include "ultima/ultima2/logic/city_castle_logic.h"
#include "ultima/shared/gfx/view.h"

namespace Ultima {
namespace Ultima2 {
namespace Data {

Common::String mapFilename(int mapEra, int mapType) {
	return Common::String::format("MAPX%c%c", '0' + mapEra, '0' + mapType);
}

Common::String monsterFilename(int mapEra, int mapType) {
	return Common::String::format("MONX%c%c", '0' + mapEra, '0' + mapType);
}

void Map::load(int mapEra, int mapType) {
	Common::File f;
	Common::String filename = mapFilename(mapEra, mapType);
	if (!f.open(filename.c_str()))
		error("Could not open %s", filename.c_str());

	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x)
			_tiles[y][x] = (TileId)(f.readByte() / 4);
	}

	_monsters.load(mapEra, mapType);

	if (mapType == 0)
		_G(logic) = Common::SharedPtr<Logic::Logic>(new Logic::OverworldLogic());
	else
		_G(logic) = Common::SharedPtr<Logic::Logic>(new Logic::CityCastleLogic());

	Common::String mapViewName = (mapType == 0) ? "OverworldMap" : "LocationMap";

	if (!g_engine->isPresent("Game")) {
		// Set up the game view and force it to draw immediately
		g_engine->replaceView("Game", true);
		g_engine->focusedView()->draw();

		// Now open the map view on top of it
		g_engine->addView(mapViewName);
	} else {
		// Just switching from one map view to another
		g_engine->replaceView(mapViewName);
	}
}

} // namespace Data
} // namespace Ultima2
} // namespace Ultima
