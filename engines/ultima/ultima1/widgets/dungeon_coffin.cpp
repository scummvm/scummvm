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

#include "ultima/ultima1/widgets/dungeon_coffin.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/maps/map_tile.h"
#include "ultima/ultima1/maps/map_dungeon.h"
#include "ultima/ultima1/game.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {

DungeonCoffin::DungeonCoffin(Ultima1Game *game, Maps::MapBase *map, const Point &pt) :
		DungeonItem(game, map, UITEM_COFFIN, pt) {
	_name = game->_res->DUNGEON_ITEM_NAMES[1];
}

DungeonCoffin::DungeonCoffin(Ultima1Game *game, Maps::MapBase *map) : DungeonItem(game, map, UITEM_COFFIN) {
	_name = game->_res->DUNGEON_ITEM_NAMES[1];
}

bool DungeonCoffin::open() {
	Ultima1Game *game = static_cast<Ultima1Game *>(_game);
	Maps::MapDungeon *map = static_cast<Maps::MapDungeon *>(_map);
	Point deltaPos = _map->getDeltaPosition(_map->getDirectionDelta());
	Maps::U1MapTile deltaTile;
	_map->getTileAt(deltaPos, &deltaTile);
	addInfoMsg("");

	if (_game->getRandomNumber(1, 255) < 104 && !deltaTile._isWall && !deltaTile._isSecretDoor && !deltaTile._widget) {
		spawnMonsterAt(deltaPos);
	} else {
		addInfoMsg(game->_res->FIND);
		game->giveTreasure(game->getRandomNumber(3, map->getLevel() * map->getLevel() + 9), 0);
		map->removeWidget(this);
	}

	return true;
}

void DungeonCoffin::spawnMonsterAt(const Point &newPos) {
	Maps::U1MapTile tile;
	_map->getTileAt(newPos, &tile);

	if (tile._isHallway || tile._isLadderUp || tile._isLadderDown || tile._widget) {
		Maps::MapDungeon *map = static_cast<Maps::MapDungeon *>(_map);
		map->spawnMonsterAt(newPos);
	}
}

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima
