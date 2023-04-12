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

#include "mm/mm1/game/leprechaun.h"
#include "mm/mm1/maps/map00.h"
#include "mm/mm1/globals.h"

#define TOWN_NUM 0x2fe
#define LEPRECHAUN_MAP_ID1 0x2ff
#define LEPRECHAUN_MAP_ID2 0x304
#define LEPRECHAUN_MAP_X 0x309
#define LEPRECHAUN_MAP_Y 0x30E

namespace MM {
namespace MM1 {
namespace Game {

void Leprechaun::teleportToTown(char townNum) {
	Maps::Maps &maps = *g_maps;
	Maps::Map00 &map = *static_cast<Maps::Map00 *>(g_maps->_currentMap);

	map[TOWN_NUM] = townNum;

	// Scan the party for someone with any gems
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		if (c._gems) {
			c._gems--;

			int townIndex = map[TOWN_NUM] - Common::KEYCODE_1;
			maps._mapPos.x = map[LEPRECHAUN_MAP_X + townIndex];
			maps._mapPos.y = map[LEPRECHAUN_MAP_Y + townIndex];
			maps.changeMap(
				map[LEPRECHAUN_MAP_ID1 + townIndex] |
				(map[LEPRECHAUN_MAP_ID2 + townIndex] << 8),
				1);

			g_events->redraw();
			return;
		}
	}

	maps._mapPos = Common::Point(8, 5);
	map.updateGame();
}

} // namespace Game
} // namespace MM1
} // namespace MM
