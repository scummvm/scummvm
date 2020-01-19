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

#include "ultima/ultima1/widgets/dungeon_monster.h"
#include "ultima/ultima1/map/map.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/game.h"
#include "ultima/shared/early/ultima_early.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {

#define SGN(v) ((v) > 0 ? 1 : -1)

bool DungeonMonster::isBlockingView() const {
	return _monsterId != MONSTER_INVISIBLE_SEEKER && _monsterId != MONSTER_MIMIC
		&& _monsterId != MONSTER_GELATINOUS_CUBE;
}

void DungeonMonster::draw(Shared::DungeonSurface &s, uint distance) {
	if (distance < 5) {
		if (_monsterId == MONSTER_GELATINOUS_CUBE) {
			s.drawWall(distance);
			s.drawLeftEdge(distance);
			s.drawRightEdge(distance);
		} else {
			Ultima1Game *game = static_cast<Ultima1Game *>(g_vm->_game);
			Widgets::DungeonWidget::drawWidget(s, _monsterId, distance, game->_edgeColor);
		}
	}
}

void DungeonMonster::update(bool isPreUpdate) {
	assert(isPreUpdate);
	Point playerPos = _map->_currentTransport->_position;
	Point delta = playerPos - _position;
	int distance = ABS(delta.x) + ABS(delta.y);

	if (distance == 1) {
		attack(true);
	} else if (distance < 8) {
		movement();
	}
}

void DungeonMonster::movement() {
	Point playerPos = _map->_currentTransport->_position;
	Point delta = playerPos - _position;

	if (delta.x != 0 && canMoveTo(Point(_position.x + SGN(delta.x), _position.y)))
		_position.x += SGN(delta.x);
	else if (delta.y != 0 && canMoveTo(Point(_position.x, _position.y + SGN(delta.y))))
		_position.y += SGN(delta.y);
}

bool DungeonMonster::canMoveTo(const Point &destPos) {
	if (!MapWidget::canMoveTo(destPos))
		return false;

	return DungeonMonster::canMoveTo(_map, this, destPos);
}

bool DungeonMonster::canMoveTo(Shared::Map::MapBase *map, MapWidget *widget, const Point &destPos) {
	// Get the details of the position
	Shared::MapTile currTile, destTile;
	
	map->getTileAt(map->getPosition(), &currTile);
	map->getTileAt(destPos, &destTile);

	// Can't move onto certain dungeon tile types
	if (destTile._isWall || destTile._isSecretDoor || destTile._isBeams)
		return false;

	// Can't move to directly adjoining doorway cells (they'd be in parralel to each other, not connected)
	if (destTile._isDoor && currTile._isDoor)
		return false;

	return true;
}

void DungeonMonster::attack(bool isAllowed) {
	Ultima1Game *game = static_cast<Ultima1Game *>(_game);
	Point playerPos = _map->_currentTransport->_position;
	Point delta = playerPos - _position;

	// Get tile details for both the player and the attacking creature
	Map::U1MapTile playerTile,creatureTile;
	_map->getTileAt(playerPos, &playerTile);
	_map->getTileAt(_position, &creatureTile);

	if (playerTile._isBeams || (creatureTile._isDoor && (playerTile._isDoor || playerTile._isWall || playerTile._isSecretDoor)))
		return;

	// Write attack line
	addInfoMsg(Common::String::format(game->_res->ATTACKED_BY, _name.c_str()));

	// TODO: rest of monster attacks
}

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima
