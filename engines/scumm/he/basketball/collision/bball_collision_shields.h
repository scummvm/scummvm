/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SCUMM_HE_BASKETBALL_COLLISION_BBALL_COLLISION_SHIELDS_H
#define SCUMM_HE_BASKETBALL_COLLISION_BBALL_COLLISION_SHIELDS_H

#ifdef ENABLE_HE

#include "scumm/he/intern_he.h"
#include "scumm/he/basketball/collision/bball_collision_box.h"

namespace Scumm {

#define WEST_SHIELD_ID    0
#define NORTH_SHIELD_ID   1
#define EAST_SHIELD_ID    2
#define SOUTH_SHIELD_ID   3
#define TOP_SHIELD_ID     4
#define ALL_SHIELD_ID     5
#define MAX_SHIELD_COUNT  5

#define BACKUP_WEST_SHIELD_ID  6
#define BACKUP_NORTH_SHIELD_ID 7
#define BACKUP_EAST_SHIELD_ID  8
#define BACKUP_SOUTH_SHIELD_ID 9

#define PLAYER_SHIELD_INCREMENT_VALUE 25

#define SHIELD_DEPTH      1000
#define SHIELD_HEIGHT     50000
#define SHIELD_EXTENSION  20000
#define BUFFER_WIDTH      800

class CCollisionShieldVector : public Common::Array<CCollisionBox> {
public:
	CCollisionShieldVector();
	~CCollisionShieldVector();

	int _shieldUpCount;
};

} // End of namespace Scumm

#endif // ENABLE_HE

#endif // SCUMM_HE_BASKETBALL_COLLISION_BBALL_COLLISION_SHIELDS_H
