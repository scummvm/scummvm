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

#include "ultima/shared/maps/creature.h"
#include "ultima/shared/early/game.h"

namespace Ultima {
namespace Shared {
namespace Maps {

void Creature::synchronize(Common::Serializer &s) {
	s.syncAsSint32LE(_hitPoints);
}

void Creature::update(bool isPreUpdate) {
	if (isPreUpdate) {
		// Check whether creature can attack
		movement();
		_isAttacking = attackDistance() != 0;
	} else if (_isAttacking && !_gameRef->_party->isDead()) {
		attackParty();
	}
}

bool Creature::subtractHitPoints(uint amount) {
	if ((int)amount >= _hitPoints) {
		_hitPoints = 0;
		return true;
	} else {
		_hitPoints -= amount;
		return false;
	}
}

} // End of namespace Maps
} // End of namespace Shared
} // End of namespace Ultima
