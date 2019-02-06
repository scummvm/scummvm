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

#include "engines/stark/movement/followpathlight.h"

#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/stateprovider.h"

#include "engines/stark/resources/anim.h"
#include "engines/stark/resources/floor.h"
#include "engines/stark/resources/item.h"
#include "engines/stark/resources/light.h"
#include "engines/stark/resources/path.h"

namespace Stark {

FollowPathLight::FollowPathLight(Resources::ItemVisual *item) :
		Movement(item),
		_light(nullptr),
		_path(nullptr),
		_speed(0.0),
		_position(0.0),
		_previouslyEnabled(true) {
}

FollowPathLight::~FollowPathLight() {
}

void FollowPathLight::start() {
	Movement::start();

	_previouslyEnabled = _item->isEnabled();
	_item->setEnabled(true);

	Math::Vector3d newPosition = _path->getWeightedPositionInEdge(0, 0);
	_light->setPosition(newPosition);
}

void FollowPathLight::stop(bool force) {
	Movement::stop(force);

	_item->setEnabled(_previouslyEnabled);
}

void FollowPathLight::onGameLoop() {
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

	// Set the new position for the light
	Math::Vector3d newPosition = _path->getWeightedPositionInEdge(currentEdge, positionInEdge);
	_light->setPosition(newPosition);
}

void FollowPathLight::setPath(Resources::Path *path) {
	_path = path;
}

void FollowPathLight::setSpeed(float speed) {
	_speed = speed;
}

void FollowPathLight::setLight(Resources::Light *light) {
	_light = light;
}

uint32 FollowPathLight::getType() const {
	return kTypeFollowPathLight;
}

void FollowPathLight::saveLoad(ResourceSerializer *serializer) {
	serializer->syncAsResourceReference(&_path);
	serializer->syncAsResourceReference(&_light);
	serializer->syncAsFloat(_position);
	serializer->syncAsFloat(_speed);
	serializer->syncAsUint32LE(_previouslyEnabled);
}

} // End of namespace Stark
