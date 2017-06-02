/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/movement/turn.h"

#include "math/matrix3.h"

#include "engines/stark/resources/anim.h"
#include "engines/stark/resources/item.h"

#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/stateprovider.h"

namespace Stark {

Turn::Turn(Resources::FloorPositionedItem *item) :
		Movement(item),
		_item3D(item),
		_turnSpeed(_defaultTurnAngleSpeed) {
}

Turn::~Turn() {
}

void Turn::onGameLoop() {
	// Compute the direction to turn towards
	Math::Vector3d direction = _targetDirection;
	direction.z() = 0;
	direction.normalize();

	// Compute the angle with the current character direction
	Math::Vector3d currentDirection = _item3D->getDirectionVector();
	float directionDeltaAngle = computeAngleBetweenVectorsXYPlane(currentDirection, direction);

	// If the angle between the current direction and the new one is too high,
	// make the character turn on itself until the angle is low enough
	TurnDirection turnDirection;
	if (ABS(directionDeltaAngle) > getAngularSpeed() + 0.1f) {
		turnDirection = directionDeltaAngle < 0 ? kTurnLeft : kTurnRight;
	} else {
		turnDirection = kTurnNone;
	}

	if (turnDirection == kTurnNone) {
		direction = _targetDirection;
	} else {
		// Make the character turn towards the target direction
		direction = currentDirection;

		Math::Matrix3 rot;
		rot.buildAroundZ(turnDirection == kTurnLeft ? -getAngularSpeed() : getAngularSpeed());
		rot.transformVector(&direction);
	}

	// Update the item's direction
	_item3D->setDirection(computeAngleBetweenVectorsXYPlane(direction, Math::Vector3d(1.0, 0.0, 0.0)));

	// Check if we are close enough to the destination to stop
	if (direction == _targetDirection) {
		stop();
	}
}

float Turn::getAngularSpeed() const {
	return _turnSpeed * StarkGlobal->getMillisecondsPerGameloop();
}

void Turn::setTargetDirection(const Math::Vector3d &direction) {
	_targetDirection = direction;
}

void Turn::setSpeed(float speed) {
	_turnSpeed = speed;
}

uint32 Turn::getType() const {
	return kTypeTurn;
}

void Turn::saveLoad(ResourceSerializer *serializer) {
	serializer->syncAsVector3d(_targetDirection);
	serializer->syncAsFloat(_turnSpeed);
}

} // End of namespace Stark
