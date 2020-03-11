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

#ifndef ULTIMA4_MOVEMENT_H
#define ULTIMA4_MOVEMENT_H

#include "ultima/ultima4/direction.h"
#include "ultima/ultima4/map.h"

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
    MoveEvent(Direction d, bool user) : dir(d), userEvent(user), result(MOVE_SUCCEEDED) {}

    Direction dir;              /**< the direction of the move */
    bool userEvent;             /**< whether the user initiated the move */
    MoveResult result;          /**< how the movement was resolved */
};

void moveAvatar(MoveEvent &event);
void moveAvatarInDungeon(MoveEvent &event);
int moveObject(class Map *map, class Creature *obj, MapCoords avatar);
int moveCombatObject(int action, class Map *map, class Creature *obj, MapCoords target);
void movePartyMember(MoveEvent &event);
bool slowedByTile(const Tile *tile);
bool slowedByWind(int direction);

extern bool collisionOverride;

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
