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
#include "engines/stark/services/stateprovider.h"

#include "engines/stark/resources/anim.h"
#include "engines/stark/resources/floor.h"
#include "engines/stark/resources/floorface.h"
#include "engines/stark/resources/item.h"
#include "engines/stark/resources/location.h"

#include "math/vector2d.h"

namespace Stark {

Walk::Walk(Resources::FloorPositionedItem *item) :
		Movement(item),
		_item3D(item),
		_running(false),
		_reachedDestination(false),
		_turnDirection(kTurnNone),
		_collisionWaitTimeout(-1),
		_collisionWaitCount(0) {
	_path = new StringPullingPath();
}

Walk::~Walk() {
	delete _path;
}

void Walk::start() {
	Movement::start();

	updatePath();
	changeItemAnim();

	Resources::Location *location = StarkGlobal->getCurrent()->getLocation();
	location->startFollowingCharacter();
}

void Walk::stop() {
	Movement::stop();

	changeItemAnim();
}

void Walk::updatePath() const {
	_path->reset();

	Resources::Floor *floor = StarkGlobal->getCurrent()->getFloor();

	Math::Vector3d startPosition = _item3D->getPosition3D();
	int32 startFloorFaceIndex = floor->findFaceContainingPoint(startPosition);
	if (startFloorFaceIndex == -1) {
		startFloorFaceIndex = 0;
	}

	Resources::FloorFace *startFloorFace = floor->getFace(startFloorFaceIndex);
	Resources::FloorEdge *startFloorEdge = startFloorFace->findNearestEdge(startPosition);
	if (!startFloorEdge) {
		// Unable to find enabled start edge
		return;
	}

	int32 destinationFloorFaceIndex = floor->findFaceContainingPoint(_destination);
	if (destinationFloorFaceIndex < 0) {
		// Unable to find the destination's face
		return;
	}

	Resources::FloorFace *destinationFloorFace = floor->getFace(destinationFloorFaceIndex);
	Resources::FloorEdge *destinationFloorEdge = destinationFloorFace->findNearestEdge(_destination);
	if (!destinationFloorEdge) {
		// Unable to find enabled destination edge
		return;
	}

	ShortestPath pathSearch;
	ShortestPath::NodeList edgePath = pathSearch.search(startFloorEdge, destinationFloorEdge);

	for (ShortestPath::NodeList::const_iterator it = edgePath.begin(); it != edgePath.end(); it++) {
		_path->addStep((*it)->getPosition());
	}

	_path->addStep(_destination);
}

void Walk::onGameLoop() {
	Resources::ItemVisual *interactiveItem = StarkGlobal->getCurrent()->getInteractive();

	if (_item != interactiveItem) {
		// NPCs have a simple collision handling strategy.
		// They stop when they collide with other items,
		// and wait for their path to be clear.
		doWalkCollisionSimple();
	} else {
		doWalk(); // TODO: Advanced collision handling
	}
}

void Walk::doWalk() {
	if (!_path->hasSteps()) {
		// There is no path to the destination
		stop();
		return;
	}

	Resources::Floor *floor = StarkGlobal->getCurrent()->getFloor();

	// Get the target to walk to
	Math::Vector3d currentPosition = _item3D->getPosition3D();
	Math::Vector3d target = _path->computeWalkTarget(currentPosition);

	// Compute the direction to walk into
	Math::Vector3d direction = target - currentPosition;
	direction.z() = 0;
	direction.normalize();

	// Compute the angle with the current character direction
	Math::Vector3d currentDirection = _item3D->getDirectionVector();
	float directionDeltaAngle = computeAngleBetweenVectorsXYPlane(currentDirection, direction);

	// If the angle between the current direction and the new one is too high,
	// make the character turn on itself until the angle is low enough
	if (ABS(directionDeltaAngle) > getAngularSpeed() + 0.1f) {
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
		rot.buildAroundZ(_turnDirection == kTurnLeft ? -getAngularSpeed() : getAngularSpeed());
		rot.transformVector(&direction);
	}

	_previousPosition = currentPosition;
	_newPosition = newPosition;

	// Some scripts expect the character position to be the exact destination
	if (newPosition == _destination) {
		_reachedDestination = true;
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
	_item3D->setPosition3D(newPosition);
	if (direction.getMagnitude() != 0.f) {
		_item3D->setDirection(computeAngleBetweenVectorsXYPlane(direction, Math::Vector3d(1.0, 0.0, 0.0)));
	}
	if (newFloorFaceIndex >= 0) {
		// When unable to find the face containing the new position, keep the previous one
		// to prevent draw order glitches.
		_item3D->setFloorFaceIndex(newFloorFaceIndex);
	}

	changeItemAnim();
}

bool Walk::isPointNearPath(const Math::Vector3d &point3d, const Math::Vector3d &pathStart3d, const Math::Vector3d &pathEnd3d) {
	Math::Vector2d point     = Math::Vector2d(point3d.x(), point3d.y());
	Math::Vector2d pathStart = Math::Vector2d(pathStart3d.x(), pathStart3d.y());
	Math::Vector2d pathEnd   = Math::Vector2d(pathEnd3d.x(), pathEnd3d.y());

	// Project the point onto the path
	Math::Vector2d pointToStart = point - pathStart;
	Math::Vector2d path = pathEnd - pathStart;
	float dot = pointToStart.dotProduct(path);
	float len = path.getSquareMagnitude();

	float t = dot / len;

	Math::Vector2d projection;
	if (0.f <= t && t < 1.f) {
		projection = path * t + pathStart;
	} else {
		projection = pathEnd;
	}

	// Check if the projection is near the actual point
	return point.getDistanceTo(projection) <= (15.f + 15.f);
}

void Walk::doWalkCollisionSimple() {
	if (_collisionWaitTimeout > 0) {
		_collisionWaitTimeout -= StarkGlobal->getMillisecondsPerGameloop();
		return;
	} else {
		_collisionWaitTimeout = -1;
	}

	Resources::Location *location = StarkGlobal->getCurrent()->getLocation();
	Common::Array<Resources::ItemVisual *> characters = location->listCharacters();

	// Check if any of the other characters is in our way
	for (uint i = 0; i < characters.size(); i++) {
		Resources::FloorPositionedItem *otherItem = dynamic_cast<Resources::FloorPositionedItem *>(characters[i]);
		if (!otherItem || !otherItem->isEnabled() || otherItem == _item) continue;

		Math::Vector3d otherPosition = otherItem->getPosition3D();

		if (isPointNearPath(otherPosition, _previousPosition, _newPosition)) {
			if (_previousPosition.getDistanceTo(otherPosition) <= 15.f * 3.f) {
				if (_collisionWaitCount >= 10) {
					doWalk();
					return;
				}

				// A collision is detected. Remove the walk animation, and wait a bit.
				if (_item->getAnimActivity() != Resources::Anim::kActorActivityIdle) {
					_item->setAnimActivity(Resources::Anim::kActorActivityIdle);
				}

				_collisionWaitCount++;
				_collisionWaitTimeout = 500; // ms
				return;
			}
		}
	}

	// The path is clear, walk normally
	_collisionWaitCount = 0;
	doWalk();
}

float Walk::getAngularSpeed() const {
	return _defaultTurnAngleSpeed * StarkGlobal->getMillisecondsPerGameloop();
}

float Walk::computeDistancePerGameLoop() const {
	Resources::Anim *anim = _item->getAnim();
	float distancePerGameloop = anim->getMovementSpeed() * StarkGlobal->getMillisecondsPerGameloop() / 1000.f;

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
		_item->setAnimActivity(Resources::Anim::kActorActivityIdle);
	} else if (_turnDirection != kTurnNone) {
		_item->setAnimActivity(Resources::Anim::kActorActivityIdle);
	} else if (_running) {
		_item->setAnimActivity(Resources::Anim::kActorActivityRun);
	} else {
		_item->setAnimActivity(Resources::Anim::kActorActivityWalk);
	}
}

void Walk::changeDestination(const Math::Vector3d &destination) {
	_collisionWaitTimeout = -1;
	setDestination(destination);
	updatePath();
}

bool Walk::hasReachedDestination() const {
	return _reachedDestination;
}

uint32 Walk::getType() const {
	return kTypeWalk;
}

void Walk::saveLoad(ResourceSerializer *serializer) {
	serializer->syncAsVector3d(_destination);
	serializer->syncAsUint32LE(_running);
}

} // End of namespace Stark
