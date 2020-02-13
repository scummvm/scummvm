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

#include "ultima/ultima1/widgets/princess.h"
#include "ultima/ultima1/maps/map_city_castle.h"
#include "ultima/shared/core/utils.h"

namespace Ultima {
namespace Ultima1 {
namespace Widgets {

void Princess::movement() {
	if (!areGuardsHostile()) {
		// Until guards turn hostile, the princesses exhibit standard wench behaviour 
		Wench::movement();
	} else {
		// When the guards are hostile, keep the princess moving towards the player
		Point playerPos = _map->_playerWidget->_position;
		Point delta(SGN(_position.x - playerPos.x), SGN(_position.y - playerPos.y));
		bool moved = false;

		// Randomly choose whether to give precedence to a X or Y move
		if (_game->getRandomNumber(1, 100) >= 50) {
			// Delta X comes first
			if (delta.x != 0)
				moved = canMoveTo(Point(delta.x, 0));
			if (!moved && delta.y != 0)
				moved = canMoveTo(Point(0, delta.y));
		} else {
			// Delta Y comes first
			if (delta.y != 0)
				moved = canMoveTo(Point(0, delta.y));
			if (!moved && delta.x != 0)
				moved = canMoveTo(Point(delta.x, 0));
		}

		if (moved)
			_game->playFX(4);
	}
}

bool Princess::subtractHitPoints(uint amount) {
	bool result = Person::subtractHitPoints(amount);
	if (result)
		// Princess is dead, you monster. So you can no longer be credited with freeing them
		static_cast<Maps::MapCastle *>(_map)->_freeingPrincess = false;

	return result;
}

} // End of namespace Widgets
} // End of namespace Ultima1
} // End of namespace Ultima
