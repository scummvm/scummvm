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

#include "engines/stark/movement/followpath.h"

#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/stateprovider.h"

#include "engines/stark/resources/anim.h"
#include "engines/stark/resources/floor.h"
#include "engines/stark/resources/item.h"
#include "engines/stark/resources/path.h"

namespace Stark {

FollowPath::FollowPath(Resources::ItemVisual *item) :
		Movement(item),
		_path(nullptr),
		_speed(0.0),
		_position(0.0),
		_previouslyEnabled(true),
		_anim(nullptr) {
}

FollowPath::~FollowPath() {
}

void FollowPath::start() {
	Movement::start();

	_previouslyEnabled = _item->isEnabled();
	_item->setEnabled(true);

	updateItemPosition(0, 0);
	changeItemAnim();
}

void FollowPath::stop(bool force) {
	Movement::stop(force);

	changeItemAnim();
	_item->setEnabled(_previouslyEnabled);
}

void FollowPath::onGameLoop() {
	// Compute the new position on the path
	_position += _speed * StarkGlobal->getMillisecondsPerGameloop();

	// Find the current path edge, and position on the path edge
	uint currentEdge = 0;
	float positionInEdge = _position;
	for (uint i = 0; i < _path->getEdgeCount(); i++) {
		float edgeLength = _path->getWeightedEdgeLength(i);
		if (positionInEdge < edgeLength) {
			break; // Found the current path edge
		}

		positionInEdge -= edgeLength;
		currentEdge++;
	}

	// Check if we went beyond the path's end
	if (currentEdge >= _path->getEdgeCount()) {
		stop();
		return;
	}

	updateItemPosition(currentEdge, positionInEdge);
}

void FollowPath::updateItemPosition(uint currentEdge, float positionInEdge) const {// Get the new position for the item
	Math::Vector3d newPosition = _path->getWeightedPositionInEdge(currentEdge, positionInEdge);

	// Update the item's properties in the scene
	if (is3D()) {
		Resources::FloorPositionedItem *item3D = Resources::Object::cast<Resources::FloorPositionedItem>(_item);
		Resources::Floor *floor = StarkGlobal->getCurrent()->getFloor();

		int32 floorFaceIndex = floor->findFaceContainingPoint(newPosition);
		if (floorFaceIndex >= 0) {
			item3D->setFloorFaceIndex(floorFaceIndex);
		} else {
			item3D->overrideSortKey(_path->getSortKey());
		}

		item3D->setPosition3D(newPosition);

		Math::Vector3d direction = _path->getEdgeDirection(currentEdge);
		item3D->setDirection(computeAngleBetweenVectorsXYPlane(direction, Math::Vector3d(1.0, 0.0, 0.0)));
	} else {
		Common::Point position2D = Common::Point(newPosition.x(), newPosition.y());
		_item->setPosition2D(position2D);
	}
}

void FollowPath::changeItemAnim() {
	if (_ended) {
		if (_anim) {
			_item->resetActionAnim();
		} else {
			_item->setAnimActivity(Resources::Anim::kActorActivityIdle);
		}
	} else {
		if (_anim) {
			_item->playActionAnim(_anim);
		} else {
			_item->setAnimActivity(Resources::Anim::kActorActivityWalk);
		}
	}
}

void FollowPath::setPath(Resources::Path *path) {
	_path = path;
}

void FollowPath::setSpeed(float speed) {
	_speed = speed;
}

bool FollowPath::is3D() const {
	return _path->getSubType() == Resources::Path::kPath3D;
}

void FollowPath::setAnim(Resources::Anim *anim) {
	_anim = anim;
}

uint32 FollowPath::getType() const {
	return kTypeFollowPath;
}

void FollowPath::saveLoad(ResourceSerializer *serializer) {
	serializer->syncAsResourceReference(&_path);
	serializer->syncAsResourceReference(&_anim);
	serializer->syncAsFloat(_position);
	serializer->syncAsFloat(_speed);
	serializer->syncAsUint32LE(_previouslyEnabled);
}

} // End of namespace Stark
