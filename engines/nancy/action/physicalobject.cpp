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

#include "common/random.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/action/physicalobject.h"

namespace Nancy {
namespace Action {

// Below these a speed or a difference counts as none
static const double kEpsilon = 1e-5;
static const double kMinSpeed = 0.01;
// Faster than this, an object always feels its force
static const double kRestSpeed = 30.0;
// How far an object that bounced is pushed back in
static const double kBounceDistance = 2.0;
// A pin sends an object off at a slightly different angle each time
static const double kPinNormalJitter = 0.0001;

double PhysicalObject::getAngle(double x, double y) {
	double angle = atan2(-y, x);
	if (angle < 0) {
		angle += 2 * M_PI;
	}

	return angle;
}

Common::Rect PhysicalObject::getRectAt(double x, double y) const {
	int left = (int)x - _width / 2;
	int top = (int)y - _height / 2;
	return Common::Rect(left, top, left + _width, top + _height);
}

Common::Rect PhysicalObject::getRect() const {
	return getRectAt(_x, _y);
}

bool PhysicalObject::isInsideBounds(const Common::Rect &rect) const {
	return _bounds.isEmpty() || _bounds.contains(rect);
}

double PhysicalObject::getBoundsNormal(const Common::Rect &rect) const {
	if (isInsideBounds(rect)) {
		return -1;
	}

	if (rect.left <= _bounds.left) {
		return 0;
	} else if (rect.right >= _bounds.right) {
		return M_PI;
	} else if (rect.top <= _bounds.top) {
		return 3 * M_PI / 2;
	} else if (rect.bottom >= _bounds.bottom) {
		return M_PI / 2;
	}

	return -1;
}

bool PhysicalObject::isClearOfObstacle(const Obstacle &obstacle, const Common::Rect &rect) const {
	if (obstacle.type == Obstacle::kBlock) {
		return !rect.intersects(obstacle.rect);
	}

	double dx = (rect.left + rect.width() * 0.5) - (obstacle.rect.left + obstacle.rect.width() * 0.5);
	double dy = (rect.top + rect.height() * 0.5) - (obstacle.rect.top + obstacle.rect.height() * 0.5);
	return sqrt(dx * dx + dy * dy) > rect.width();
}

bool PhysicalObject::isClearOfObstacles(const Common::Rect &rect) const {
	if (!_obstacles) {
		return true;
	}

	for (const Obstacle &obstacle : *_obstacles) {
		if (!isClearOfObstacle(obstacle, rect)) {
			return false;
		}
	}

	return true;
}

double PhysicalObject::getObstacleNormal(const Common::Rect &rect) const {
	if (!_obstacles) {
		return -1;
	}

	for (const Obstacle &obstacle : *_obstacles) {
		if (isClearOfObstacle(obstacle, rect)) {
			continue;
		}

		const Common::Rect &block = obstacle.rect;
		if (obstacle.type == Obstacle::kPin) {
			double dx = (rect.left + rect.width() * 0.5) - (block.left + block.width() * 0.5);
			double dy = (rect.top + rect.height() * 0.5) - (block.top + block.height() * 0.5);
			return getAngle(dx, dy) + g_nancy->_randomSource->getRandomNumber(999) * kPinNormalJitter;
		}

		// Off the side of the block that the rect sticks out of. One coming
		// down on the block from right above it is let through.
		if (rect.left <= block.right && block.right < rect.right) {
			return 0;
		} else if (block.left <= rect.right && rect.left < block.left) {
			return M_PI;
		} else if (rect.top <= block.bottom && block.bottom < rect.bottom) {
			return 3 * M_PI / 2;
		}
	}

	return -1;
}

void PhysicalObject::setSpeed(double speed) {
	_speed = speed;

	if (_hasMaxSpeed && _speed > _maxSpeed) {
		_speed = _maxSpeed;
	}

	if (fabs(_speed) < kMinSpeed) {
		_speed = 0;
	}
}

void PhysicalObject::setHeading(double heading) {
	_heading = fmod(heading, 2 * M_PI);
	if (_heading < 0) {
		_heading += 2 * M_PI;
	}
}

void PhysicalObject::moveTo(double x, double y) {
	if (_boundsMode == kBounceOffBounds) {
		bounceOffBounds(x, y);
	} else {
		stopAtBounds(x, y);
	}
}

void PhysicalObject::stopAtBounds(double x, double y) {
	Common::Rect rect = getRectAt(x, y);
	if (isInsideBounds(rect) && isClearOfObstacles(rect)) {
		_x = x;
		_y = y;
	} else {
		_speed = 0;
		_hasHitBounds = true;
	}
}

void PhysicalObject::bounceOffBounds(double x, double y) {
	Common::Rect rect = getRectAt(x, y);
	double normal = getBoundsNormal(rect);
	if (normal < 0) {
		normal = getObstacleNormal(rect);
	}

	if (normal < 0) {
		_x = x;
		_y = y;
		return;
	}

	// Back in a little, along the way out of the bounds
	double normalX = cos(normal);
	double normalY = -sin(normal);
	_x += normalX * kBounceDistance;
	_y += normalY * kBounceDistance;

	// The heading turns as if mirrored by the edge
	double headingX = cos(_heading);
	double headingY = -sin(_heading);
	double dot = 2 * (headingX * normalX + headingY * normalY);
	_heading = getAngle(headingX - dot * normalX, headingY - dot * normalY);

	_hasHitBounds = true;
}

void PhysicalObject::accelerate(double numTicks) {
	if (sqrt(_forceX * _forceX + _forceY * _forceY) <= kEpsilon) {
		return;
	}

	double velocityX = cos(_heading) * _speed + _forceX * numTicks;
	double velocityY = -sin(_heading) * _speed + _forceY * numTicks;
	_speed = sqrt(velocityX * velocityX + velocityY * velocityY);
	_heading = getAngle(velocityX, velocityY);
}

void PhysicalObject::applyFriction(double numTicks) {
	if (fabs(_speed) <= kEpsilon) {
		setSpeed(0);
		return;
	}

	// Slows the object down, but never turns it around
	bool wasBackwards = _speed < 0;
	double change = _friction * numTicks;
	setSpeed(wasBackwards ? _speed + change : _speed - change);

	if ((_speed < 0) != wasBackwards) {
		setSpeed(0);
	}
}

void PhysicalObject::move(double numTicks) {
	double dx = cos(_heading) * _speed / 50 * numTicks;
	double dy = -sin(_heading) * _speed / 50 * numTicks;

	if (fabs(dx) > 1 || fabs(dy) > 1) {
		// In halves, until each step is no longer than a pixel
		move(numTicks / 2);
		move(numTicks / 2);
		return;
	}

	moveTo(_x + dx, _y + dy);
}

void PhysicalObject::update(uint32 numTicks) {
	// A slow object only feels its force when that changes what it does
	double forceLength = sqrt(_forceX * _forceX + _forceY * _forceY);
	if (_speed > kRestSpeed || forceLength - _friction > kEpsilon || getAngle(_forceX, _forceY) - _heading > kEpsilon) {
		accelerate(numTicks);
	}

	applyFriction(numTicks);
	move(numTicks);
}

} // End of namespace Action
} // End of namespace Nancy
