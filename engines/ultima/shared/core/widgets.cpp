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

#include "ultima/shared/core/widgets.h"
#include "ultima/shared/early/game.h"
#include "ultima/shared/core/game_state.h"

namespace Ultima {
namespace Shared {

bool DungeonWidget::canMoveTo(const Point &destPos) {
	if (!MapWidget::canMoveTo(destPos))
		return false;

	// Get the details of the position
	MapTile currTile, destTile;
	_map->getTileAt(_map->getPosition(), &currTile);
	_map->getTileAt(destPos, &destTile);

	// Can't move onto certain dungeon tile types
	if (destTile._isWall || destTile._isSecretDoor || destTile._isBeams)
		return false;

	// Can't move to directly adjoining doorway cells (they'd be in parralel to each other, not connected)
	if (destTile._isDoor && currTile._isDoor)
		return false;

	return true;
}

/*------------------------------------------------------------------------*/

void Creature::update(bool isPreUpdate) {
	if (isPreUpdate) {
		// Check whether creature can attack
		_isAttacking = canAttack();
		if (!_isAttacking)
			movement();
	} else if (_isAttacking && !_game->_gameState->isPartyDead()) {
		attack(canAttack());
	}
}

} // End of namespace Ultima1
} // End of namespace Ultima
