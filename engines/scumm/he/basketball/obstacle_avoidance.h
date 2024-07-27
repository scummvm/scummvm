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

#ifndef SCUMM_HE_BASKETBALL_OBSTACLE_AVOIDANCE_H
#define SCUMM_HE_BASKETBALL_OBSTACLE_AVOIDANCE_H

#include "scumm/he/intern_he.h"
#include "scumm/he/basketball/collision/bball_collision_support_obj.h"

#ifdef ENABLE_HE

namespace Scumm {

#define AVOIDANCE_LOOK_AHEAD_DISTANCE  4000.0F
#define OBSTACLE_AVOIDANCE_DISTANCE    75
#define AVOIDANCE_SAFETY_DISTANCE      1
#define MAX_AVOIDANCE_ANGLE_SIN        1.0F

enum EAvoidanceType {
	kSingleObject = 0,
	kMultipleObject = 1
};

} // End of namespace Scumm

#endif // ENABLE_HE

#endif // SCUMM_HE_BASKETBALL_OBSTACLE_AVOIDANCE_H
