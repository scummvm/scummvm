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

#include "ultima/ultima1/spells/open_unlock.h"
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/maps/map_tile.h"
#include "ultima/ultima1/widgets/dungeon_item.h"

namespace Ultima {
namespace Ultima1 {
namespace Spells {

void OpenUnlock::dungeonCast(Maps::MapDungeon *map) {
	Maps::U1MapTile tile;
	map->getTileAt(map->getPosition(), &tile);

	Widgets::DungeonItem *item = dynamic_cast<Widgets::DungeonItem *>(tile._widget);
	if (item) {
		addInfoMsg(item->_name, false);
		openItem(map, item);
		_game->endOfTurn();
	} else {
		Spell::dungeonCast(map);
	}
}

void OpenUnlock::openItem(Maps::MapDungeon *map, Widgets::DungeonItem *item) {
	// Say opened, and remove the coffin/chest
	map->removeWidget(item);
	addInfoMsg(Common::String::format(" %s", _game->_res->OPENED));
	addInfoMsg(_game->_res->THOU_DOST_FIND, false);

	uint coins = _game->getRandomNumber(3, map->getLevel() * map->getLevel() + 9);
	_game->giveTreasure(coins, 0);
}

} // End of namespace Spells
} // End of namespace Ultima1
} // End of namespace Ultima
