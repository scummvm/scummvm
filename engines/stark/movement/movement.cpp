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

#include "engines/stark/movement/movement.h"

#include "engines/stark/movement/walk.h"
#include "engines/stark/movement/followpath.h"
#include "engines/stark/movement/followpathlight.h"
#include "engines/stark/movement/turn.h"

#include "engines/stark/resources/item.h"

#include "common/textconsole.h"

namespace Stark {

Movement *Movement::construct(uint32 type, Resources::ItemVisual *item) {
	switch (type) {
		case kTypeWalk:
			return new Walk(Resources::Object::cast<Resources::FloorPositionedItem>(item));
		case kTypeFollowPath:
			return new FollowPath(item);
		case kTypeFollowPathLight:
			return new FollowPathLight(item);
		case kTypeTurn:
			return new Turn(Resources::Object::cast<Resources::FloorPositionedItem>(item));
		default:
			error("Unexepected movement type '%d'", type);
	}
}

Movement::Movement(Resources::ItemVisual *item) :
		_ended(false),
		_item(item),
		_defaultTurnAngleSpeed(18.0f * 30.0f / 1000.0f) { // 18 degrees per gameloop at 30 fps
}

Movement::~Movement() {
}

void Movement::start() {
	_ended = false;
}

void Movement::stop(bool force) {
	_ended = true;
}

bool Movement::hasEnded() const {
	return _ended;
}

float Movement::computeAngleBetweenVectorsXYPlane(const Math::Vector3d &v1, const Math::Vector3d &v2) const {
	Math::Vector3d v1XY = v1;
	v1XY.z() = 0.0;

	Math::Vector3d v2XY = v2;
	v2XY.z() = 0.0;

	Math::Angle angle = Math::Vector3d::angle(v1XY, v2XY);
	Math::Vector3d cross = Math::Vector3d::crossProduct(v1XY, v2XY);
	if (cross.z() < 0) {
		angle = -angle;
	}

	return angle.getDegrees();
}

bool Movement::hasReachedDestination() const {
	return true;
}
} // End of namespace Stark
