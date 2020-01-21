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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima1/actions/climb.h"
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/maps/map.h"
#include "ultima/ultima1/maps/map_tile.h"
#include "ultima/ultima1/maps/map_dungeon.h"
#include "ultima/ultima1/core/resources.h"

namespace Ultima {
namespace Ultima1 {
namespace Actions {

BEGIN_MESSAGE_MAP(Climb, Action)
	ON_MESSAGE(ClimbMsg)
END_MESSAGE_MAP()

bool Climb::ClimbMsg(CClimbMsg &msg) {
	Maps::Ultima1Map *map = getMap();
	Maps::U1MapTile mapTile;

	map->getTileAt(map->getPosition(), &mapTile);
	
	if (mapTile._tileId != Maps::DTILE_LADDER_UP && mapTile._tileId != Maps::DTILE_LADDER_DOWN) {
		playFX(1);
	} else if (map->getDirection() == Shared::Maps::DIR_LEFT || map->getDirection() == Shared::Maps::DIR_RIGHT) {
		playFX(1);
	} else if (mapTile._tileId == Maps::DTILE_LADDER_UP) {
		ladderUp();
	} else {
		ladderDown();
	}

	return true;
}

void Climb::ladderUp() {
	Maps::Ultima1Map *map = getMap();

	if (!map->changeLevel(-1)) {
		map->load(Maps::MAPID_OVERWORLD);
	}
}

void Climb::ladderDown() {
	Maps::Ultima1Map *map = getMap();
	map->changeLevel(1);
}

} // End of namespace Actions
} // End of namespace Ultima1
} // End of namespace Ultima
