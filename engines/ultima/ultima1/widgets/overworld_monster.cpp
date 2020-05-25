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
#include "ultima/ultima1/widgets/attack_effect.h"
#include "ultima/ultima1/core/party.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/game.h"
#include "ultima/shared/maps/map_widget.h"
#include "ultima/shared/core/utils.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {

OverworldMonster::OverworldMonster(Shared::Game *game, Shared::Maps::MapBase *map, uint tileNum, int hitPoints,
		const Point &pt, Shared::Maps::Direction dir) : OverworldWidget(game, map, tileNum, pt, dir),
		Shared::Maps::Creature(game, map, hitPoints) {
	_monsterId = (OverworldMonsterId)((tileNum - 19) / 2);
	
	Ultima1Game *g = static_cast<Ultima1Game *>(game);
	_name = g->_res->OVERWORLD_MONSTER_NAMES[_monsterId];
	_attackStrength = g->_res->OVERWORLD_MONSTER_DAMAGE[_monsterId];
}

void OverworldMonster::synchronize(Common::Serializer &s) {
	OverworldWidget::synchronize(s);
	Creature::synchronize(s);
	s.syncAsUint16LE(_monsterId);
	s.syncAsUint16LE(_attackStrength);
}

uint OverworldMonster::attackDistance() const {
	Point playerPos = _map->_playerWidget->_position;
	Point diff = playerPos - _position;

	int threshold = _tileNum == 23 || _tileNum == 25 || _tileNum == 31 || _tileNum == 47 ? 3 : 1;
	int distance = MIN(diff.x, diff.y);
	return distance <= threshold ? threshold : 0;
}

void OverworldMonster::movement() {
	// TODO
}

void OverworldMonster::attackParty() {
	Ultima1Game *game = dynamic_cast<Ultima1Game *>(_game);
	assert(game);
	Point playerPos = _map->_playerWidget->_position;
	Point diff = playerPos - _position;
	Point delta(SGN(diff.x), SGN(diff.y));
	Point tempDiff;
	//int maxDistance = attackDistance();
	Shared::Maps::MapTile mapTile;
	//Shared::Character &c = *_game->_party;
	//uint threshold, damage;

	// Print out the monster attacking
	
	addInfoMsg(Common::String::format("%s %s %s", _name.c_str(), game->_res->ATTACKS, _name.c_str()), false);

	/* TODO: Refactor to use attack effects
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

	} while (++distance <= maxDistance && mapTile._tileId != 3 && (tempDiff.x != 0 || tempDiff.y != 0));


	// Calculate damage threshold
	threshold = (c._stamina / 2) + (c._equippedArmour * 8) + 56;

	if (tempDiff.x == 0 && tempDiff.y == 0 && _game->getRandomNumber(1, 255) > threshold) {
		hit->_position = playerPos;
		_game->playFX(2);

		damage = _game->getRandomNumber(1, _attackStrength * 2 + 1);
		c._hitPoints -= damage;

		if (_name.size() > 8) {
			addInfoMsg("");
			addInfoMsg(Common::String::format("%s %2d %s", game->_res->HIT, damage, game->_res->DAMAGE));
		} else {
			addInfoMsg(Common::String::format(" %s", game->_res->HIT));
			addInfoMsg(Common::String::format("%2d %s", damage, game->_res->DAMAGE));
		}
	} else {
		if (_name.size() > 8)
			addInfoMsg("", true);
		else
			addInfoMsg(" ", false);
		addInfoMsg(game->_res->MISSED);
	}

	_map->removeWidget(hit);
	*/
}

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima
