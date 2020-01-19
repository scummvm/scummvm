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

#include "ultima/ultima1/widgets/overworld_monster.h"
#include "ultima/ultima1/widgets/hit.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/game.h"
#include "ultima/shared/core/utils.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {

uint OverworldMonster::attackDistance() const {
	Point playerPos = _map->_currentTransport->_position;
	Point diff = playerPos - _position;

	int threshold = _tileNum == 23 || _tileNum == 25 || _tileNum == 31 || _tileNum == 47 ? 3 : 1;
	int distance = MIN(diff.x, diff.y);
	return distance <= threshold ? threshold : 0;
}

void OverworldMonster::attack() {
	Ultima1Game *game = dynamic_cast<Ultima1Game *>(_game);
	Point playerPos = _map->_currentTransport->_position;
	Point diff = playerPos - _position;
	Point delta(SGN(diff.x), SGN(diff.y));
	Point tempDiff;
	int maxDistance = attackDistance();
	Shared::MapTile mapTile;

	// Print out the monster attacking
	addInfoMsg(Common::String::format(game->_res->MONSTER_ATTACKS, _name.c_str()));

	// Set up widget for displaying the moving hit circle
	Hit *hit = new Hit(_game, _map);
	_map->addWidget(hit);

	//
	int distance = 1;
	do {
		tempDiff.x = delta.x * distance + diff.x;
		tempDiff.y = delta.y * distance + diff.y;
		hit->_position = playerPos + tempDiff;
		_game->sleep(50);

	} while (++distance <= maxDistance && mapTile._tileNum != 3 && (tempDiff.x != 0 || tempDiff.y != 0));
	// TODO: Stuff

	_map->removeWidget(hit);
}

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima
