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

#include "engines/stark/movement/walk.h"

#include "engines/stark/movement/shortestpath.h"
#include "engines/stark/movement/stringpullingpath.h"

#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"

#include "engines/stark/resources/anim.h"
#include "engines/stark/resources/item.h"
#include "engines/stark/resources/floor.h"
#include "engines/stark/resources/floorface.h"

namespace Stark {

Walk::Walk(Resources::FloorPositionedItem *item) :
	Movement(item),
	_running(false),
	_turnDirection(kTurnNone) {
	_path = new StringPullingPath();
}

Walk::~Walk() {
	delete _path;
}

void Walk::start() {
	Movement::start();

	updatePath();
	changeItemAnim();
}

void Walk::updatePath() const {
	Resources::Floor *floor = StarkGlobal->getCurrent()->getFloor();

	Math::Vector3d startPosition = _item->getPosition3D();
	int32 startFloorFaceIndex = _item->getFloorFaceIndex();
	Resources::FloorFace *startFloorFace = floor->getFace(startFloorFaceIndex);
	Resources::FloorEdge *startFloorEdge = startFloorFace->findNearestEdge(startPosition);


	int32 destinationFloorFaceIndex = floor->findFaceContainingPoint(_destination);
	Resources::FloorFace *destinationFloorFace = floor->getFace(destinationFloorFaceIndex);
	Resources::FloorEdge *destinationFloorEdge = destinationFloorFace->findNearestEdge(_destination);

	ShortestPath pathSearch;
	ShortestPath::NodeList edgePath = pathSearch.search(startFloorEdge, destinationFloorEdge);

	_path->reset();

	for (ShortestPath::NodeList::const_iterator it = edgePath.begin(); it != edgePath.end(); it++) {
		_path->addStep((*it)->getPosition());
	}

	_path->addStep(_destination);
}

void Walk::onGameLoop() {
	Resources::Floor *floor = StarkGlobal->getCurrent()->getFloor();

	// Get the target to walk to
	Math::Vector3d currentPosition = _item->getPosition3D();
	Math::Vector3d target = _path->computeWalkTarget(currentPosition);

	// Compute the direction to walk into
	Math::Vector3d direction = target - currentPosition;
	direction.z() = 0;
	direction.normalize();

	// Compute the angle with the current character direction
	Math::Vector3d currentDirection = _item->getDirectionVector();
	float directionDeltaAngle = computeAngleBetweenVectorsXYPlane(currentDirection, direction);

	// If the angle between the current direction and the new one is too high,
	// make the character turn on itself until the angle is low enough
	if (ABS(directionDeltaAngle) > _defaultTurnAngleSpeed + 0.1f) {
		_turnDirection = directionDeltaAngle < 0 ? kTurnLeft : kTurnRight;
	} else {
		_turnDirection = kTurnNone;
	}

	float distancePerGameloop = computeDistancePerGameLoop();

	Math::Vector3d newPosition;
	if (_turnDirection == kTurnNone) {
		// Compute the new position using the distance per gameloop
		if (currentPosition.getDistanceTo(target) > distancePerGameloop) {
			newPosition = currentPosition + direction * distancePerGameloop;
		} else {
			newPosition = target;
		}
	} else {
		// The character does not change position when it is turning
		newPosition = currentPosition;
		direction = currentDirection;

		Math::Matrix3 rot;
		rot.buildAroundZ(_turnDirection == kTurnLeft ? -_defaultTurnAngleSpeed : _defaultTurnAngleSpeed);
		rot.transformVector(&direction);
	}

	// Some scripts expect the character position to be the exact destination
	if (newPosition == _destination) {
		stop();
	}

	// Update the new position's height according to the floor
	int32 newFloorFaceIndex = floor->findFaceContainingPoint(newPosition);
	if (newFloorFaceIndex >= 0) {
		floor->computePointHeightInFace(newPosition, newFloorFaceIndex);
	} else {
		warning("Item %s is walking off the floor", _item->getName().c_str());
	}

	// Update the item's properties
	_item->setPosition3D(newPosition);
	_item->setDirection(computeAngleBetweenVectorsXYPlane(direction, Math::Vector3d(1.0, 0.0, 0.0)));
	_item->setFloorFaceIndex(newFloorFaceIndex);

	changeItemAnim();
}

float Walk::computeDistancePerGameLoop() const {
	Resources::Anim *anim = _item->getAnim();
	float distancePerGameloop = anim->getMovementSpeed() * StarkGlobal->getMillisecondsPerGameloop() / 1000.0;

	return distancePerGameloop;
}

void Walk::setDestination(const Math::Vector3d &destination) {
	_destination = destination;
}

void Walk::setRunning() {
	_running = true;
	changeItemAnim();
}

void Walk::changeItemAnim() {
	if (_ended) {
		_item->setAnimKind(Resources::Anim::kActorUsageIdle);
	} else if (_turnDirection != kTurnNone) {
		_item->setAnimKind(Resources::Anim::kActorUsageIdle);
	} else if (_running) {
		_item->setAnimKind(Resources::Anim::kActorUsageRun);
	} else {
		_item->setAnimKind(Resources::Anim::kActorUsageWalk);
	}
}

} // End of namespace Stark