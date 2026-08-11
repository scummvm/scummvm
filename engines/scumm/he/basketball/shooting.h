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

#ifndef SCUMM_HE_BASKETBALL_SHOOTING_H
#define SCUMM_HE_BASKETBALL_SHOOTING_H

#ifdef ENABLE_HE

#include "scumm/he/intern_he.h"
#include "scumm/he/logic_he.h"

#include "scumm/he/basketball/collision/bball_collision_support_obj.h"

namespace Scumm {

#define MAX_SHOT_ANGLE        90  // Unless you're doing a behind the back shot, this is the highest angle possible to shoot from
#define TEN_FOOT_SHOT_ANGLE   75  // The shot angle used from ten feet out; the shot angle curve is defined by this point
#define MIN_SHOT_ANGLE        45  // This is the lowest angle a player will ever shoot from
#define SHOT_ANGLE_CUTOFF     3   // This is the horizontal distance / vertical distance ratio where the shot angle is at its lowest; any shot behind this point uses the minimum shot angle
#define SHOT_OFFSET_DISTANCE  200 // This is the distance above the basket that the shooter shoots at

} // End of namespace Scumm

#endif // ENABLE_HE

#endif // SCUMM_HE_BASKETBALL_SHOOTING_H
