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

#ifndef SCOOBY_ACTOR_MOVEMENT_POINT_H
#define SCOOBY_ACTOR_MOVEMENT_POINT_H

#include "common/scummsys.h"

// One queued actor destination with the signed 16.16 steps and position
// identity used to reach it.

namespace Scooby {

struct ActorMovementPoint {
	int16 TargetX;        // Signed destination X integer coordinate.
	int16 TargetY;        // Signed destination Y integer coordinate.
	int16 HorizontalStep; // Signed horizontal 8.8 multiplier applied to the actor's scale-derived step.
	int16 VerticalStep;   // Signed vertical 8.8 multiplier applied to the actor's scale-derived step.
	int16 PositionIndex;  // Signed authored position selected when this destination becomes active.

	ActorMovementPoint() : TargetX(0), TargetY(0), HorizontalStep(0), VerticalStep(0), PositionIndex(0) {
	}

	ActorMovementPoint(int16 targetX, int16 targetY, int16 horizontalStep,
					   int16 verticalStep, int16 positionIndex)
		: TargetX(targetX), TargetY(targetY), HorizontalStep(horizontalStep), VerticalStep(verticalStep),
		  PositionIndex(positionIndex) {
	}
};
} // namespace Scooby

#endif // SCOOBY_ACTOR_MOVEMENT_POINT_H
