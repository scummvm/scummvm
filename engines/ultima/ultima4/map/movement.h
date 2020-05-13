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

#ifndef ULTIMA4_MAP_MOVEMENT_H
#define ULTIMA4_MAP_MOVEMENT_H

#include "ultima/ultima4/map/direction.h"
#include "ultima/ultima4/map/map.h"

namespace Ultima {
namespace Ultima4 {

class Object;
class Map;
class Tile;

enum SlowedType {
	SLOWED_BY_NOTHING,
	SLOWED_BY_TILE,
	SLOWED_BY_WIND
};

enum MoveResult {
	MOVE_SUCCEEDED          = 0x0001,
	MOVE_END_TURN           = 0x0002,
	MOVE_BLOCKED            = 0x0004,
	MOVE_MAP_CHANGE         = 0x0008,
	MOVE_TURNED             = 0x0010,  /* dungeons and ship movement */
	MOVE_DRIFT_ONLY         = 0x0020,  /* balloon -- no movement */
	MOVE_EXIT_TO_PARENT     = 0x0040,
	MOVE_SLOWED             = 0x0080,
	MOVE_MUST_USE_SAME_EXIT = 0x0100
};

class MoveEvent {
public:
	MoveEvent(Direction d, bool user) : _dir(d), _userEvent(user), _result(MOVE_SUCCEEDED) {}

	Direction _dir;              /**< the direction of the move */
	bool _userEvent;             /**< whether the user initiated the move */
	MoveResult _result;          /**< how the movement was resolved */
};

/**
 * Attempt to move the avatar in the given direction.  User event
 * should be set if the avatar is being moved in response to a
 * keystroke.  Returns zero if the avatar is blocked.
 */
void moveAvatar(MoveEvent &event);

/**
 * Moves the avatar while in dungeon view
 */
void moveAvatarInDungeon(MoveEvent &event);

/**
 * Moves an object on the map according to its movement behavior
 * Returns 1 if the object was moved successfully, 0 if slowed,
 * tile direction changed, or object simply cannot move
 * (fixed objects, nowhere to go, etc.)
 */
int moveObject(class Map *map, class Creature *obj, MapCoords avatar);

/**
 * Moves an object in combat according to its chosen combat action
 */
int moveCombatObject(int action, class Map *map, class Creature *obj, MapCoords target);

/**
 * Moves a party member during combat screens
 */
void movePartyMember(MoveEvent &event);

/**
 * Default handler for slowing movement.
 * Returns true if slowed, false if not slowed
 */
bool slowedByTile(const Tile *tile);

/**
 * Slowed depending on the direction of object with respect to wind direction
 * Returns true if slowed, false if not slowed
 */
bool slowedByWind(int direction);

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
