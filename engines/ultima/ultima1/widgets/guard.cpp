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

#include "ultima/ultima1/widgets/guard.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/shared/core/utils.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {

uint Guard::attackDistance() const {
	Point diff = _position - _map->_playerWidget->_position;
	return areGuardsHostile() && ABS(diff.x) < 2 && ABS(diff.y) < 2 ? 1 : 0;
}

void Guard::movement() {
	// Don't move if the guards aren't hostile, or they're already within attack distance
	if (!areGuardsHostile() || attackDistance())
		return;

	Point diff = _position - _map->_playerWidget->_position;
	Point delta(SGN(diff.x), SGN(diff.y));
	int totalDiff = ABS(diff.x) + ABS(diff.y);
	if (totalDiff >= 13)
		return;

	// Try moving horizontally or vertically towards the player
	bool moved = moveBy(Point(delta.x, 0));
	if (!moved)
		moved = moveBy(Point(0, delta.y));
	if (moved)
		_game->playFX(4);
}

void Guard::attackParty() {
	Ultima1Game *game = static_cast<Ultima1Game *>(_game);
	Shared::Character &c = *game->_party;
	addInfoMsg(Common::String::format(game->_res->ATTACKED_BY, _name.c_str()));
	game->playFX(7);

	uint threshold = (c._stamina / 2) + (c._equippedArmour * 8) + 56;
	if (_game->getRandomNumber(1, 255) > threshold) {
		int damage = _game->getRandomNumber(2, c._hitPoints / 128 + 15);
		addInfoMsg(Common::String::format("%s...%2d %s", game->_res->HIT, damage, game->_res->DAMAGE));
		game->playFX(2);
		c._hitPoints -= damage;
	} else {
		addInfoMsg(game->_res->MISSED);
	}
}

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima
