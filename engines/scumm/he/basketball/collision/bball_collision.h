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

#ifndef SCUMM_HE_BASKETBALL_COLLISION_BBALL_COLLISION_H
#define SCUMM_HE_BASKETBALL_COLLISION_BBALL_COLLISION_H

#ifdef ENABLE_HE

#include "scumm/he/basketball/collision/bball_collision_support_obj.h"

namespace Scumm {

#define RIM_CE                       0.4F
#define BACKBOARD_CE                 0.3F
#define FLOOR_CE                     0.65F

#define LEFT_BASKET                  0
#define RIGHT_BASKET                 1

#define RIM_WIDTH                    (WORLD_UNIT_MULTIPLIER / 11)
#define RIM_RADIUS                   ((3 * WORLD_UNIT_MULTIPLIER) / 4)

#define MAX_BALL_COLLISION_PASSES    10
#define MAX_PLAYER_COLLISION_PASSES  3

} // End of namespace Scumm

#endif // ENABLE_HE

#endif // SCUMM_HE_BASKETBALL_COLLISION_BBALL_COLLISION_H
