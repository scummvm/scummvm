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

#include "freescape/freescape.h"

namespace Freescape {

void FreescapeEngine::gotoArea(uint16 areaID, int entranceID) {
	error("Function \"%s\" not implemented", __FUNCTION__);
}

void FreescapeEngine::changePlayerHeight(int index) {
	int scale = _currentArea->getScale();
	_position.setValue(1, _position.y() - scale * _playerHeight);
	_playerHeight = _playerHeights[index];
	_position.setValue(1, _position.y() + scale * _playerHeight);
}

void FreescapeEngine::rise() {
	debugC(1, kFreescapeDebugMove, "playerHeightNumber: %d", _playerHeightNumber);
	int previousAreaID = _currentArea->getAreaID();
	int scale = _currentArea->getScale();

	if (_flyMode) {
		_position.setValue(1, _position.y() + scale * 32);
	} else {
		if (_playerHeightNumber == int(_playerHeights.size()) - 1)
			return;

		_playerHeightNumber++;
		changePlayerHeight(_playerHeightNumber);
	}

	bool collided = checkCollisions(true);
	if (collided) {
		if (_currentArea->getAreaID() == previousAreaID) {
			if (_flyMode)
				_position = _lastPosition;
			else {
				_playerHeightNumber--;
				changePlayerHeight(_playerHeightNumber);
			}
		}
	}

	_lastPosition = _position;
	debugC(1, kFreescapeDebugMove, "new player position: %f, %f, %f", _position.x(), _position.y(), _position.z());
}

void FreescapeEngine::lower() {
	debugC(1, kFreescapeDebugMove, "playerHeightNumber: %d", _playerHeightNumber);
	int previousAreaID = _currentArea->getAreaID();
	int scale = _currentArea->getScale();

	if (_flyMode) {
		_position.setValue(1, _position.y() - scale * 32);
		bool collided = checkCollisions(true);
		if (collided) {
			if (_currentArea->getAreaID() == previousAreaID) {
				_position = _lastPosition;
			}
		}
	} else {
		if (_playerHeightNumber == 0)
			return;

		_playerHeightNumber--;
		changePlayerHeight(_playerHeightNumber);
	}

	_lastPosition = _position;
	debugC(1, kFreescapeDebugMove, "new player position: %f, %f, %f", _position.x(), _position.y(), _position.z());
}


void FreescapeEngine::move(CameraMovement direction, uint8 scale, float deltaTime) {
	debugC(1, kFreescapeDebugMove, "old player position: %f, %f, %f", _position.x(), _position.y(), _position.z());
	int previousAreaID = _currentArea->getAreaID();
	int areaScale = _currentArea->getScale();

	float velocity = _movementSpeed * deltaTime * areaScale;
	float positionY = _position.y();
	switch (direction) {
	case FORWARD:
		_position = _position + _cameraFront * velocity;
		break;
	case BACKWARD:
		_position = _position - _cameraFront * velocity;
		break;
	case RIGHT:
		_position = _position - _cameraRight * velocity;
		break;
	case LEFT:
		_position = _position + _cameraRight * velocity;
		break;
	}

	// restore y coordinate
	if (!_flyMode)
		_position.set(_position.x(), positionY, _position.z());

	for (int i = 0; i < 3; i++) {
		if (_position.getValue(i) < 0)
			_position.setValue(i, 0);
		else if (_position.getValue(i) > 8128)
			_position.setValue(i, 8128);
	}

	bool collided = checkCollisions(false);

	if (!collided) {
		bool hasFloor = checkFloor(_position);
		if (!hasFloor  && !_flyMode) {
			int fallen;
			for (fallen = 1; fallen < 65 + 1; fallen++) {
				_position.set(_position.x(), positionY - fallen * areaScale, _position.z());
				if (tryStepDown(_position))
					break;
			}
			fallen++;
			fallen++;
			if (fallen >= 67) {
				_position = _lastPosition; //error("NASTY FALL!");
				return;
			}
			_position.set(_position.x(), positionY - fallen * areaScale, _position.z());
			playSound(3);
		}
		debugC(1, kFreescapeDebugCode, "Runing effects:");
		checkCollisions(true); // run the effects
	} else {
		debugC(1, kFreescapeDebugCode, "Runing effects: at: %f, %f, %f", _position.x(), _position.y(), _position.z());

		checkCollisions(true); // run the effects
		if (_currentArea->getAreaID() == previousAreaID) {
			if (_flyMode)
				_position = _lastPosition;
			else {
				bool stepUp = tryStepUp(_position);
				if (stepUp) {
					playSound(4);
					debugC(1, kFreescapeDebugCode, "Runing effects:");
					checkCollisions(true); // run the effects (again)
				} else {
					playSound(2);
					_position = _lastPosition;
				}
			}
		}
	}
	areaScale = _currentArea->getScale();

	_lastPosition = _position;
	debugC(1, kFreescapeDebugMove, "new player position: %f, %f, %f", _position.x(), _position.y(), _position.z());
	debugC(1, kFreescapeDebugMove, "player height: %f", _position.y() - areaScale * _playerHeight);
	executeLocalGlobalConditions(false, true); // Only execute "on collision" room/global conditions
}

bool FreescapeEngine::checkFloor(Math::Vector3d currentPosition) {
	debugC(1, kFreescapeDebugMove, "Checking floor under the player");
	int areaScale = _currentArea->getScale();
	bool collided = checkCollisions(false);
	assert(!collided);

	_position.set(_position.x(), _position.y() - 2 * areaScale, _position.z());
	collided = checkCollisions(false);
	_position = currentPosition;
	return collided;
}

bool FreescapeEngine::tryStepUp(Math::Vector3d currentPosition) {
	debugC(1, kFreescapeDebugMove, "Try to step up!");
	int areaScale = _currentArea->getScale();
	_position.set(_position.x(), _position.y() + 64 * areaScale, _position.z());
	bool collided = checkCollisions(false);
	if (collided) {
		_position = currentPosition;
		return false;
	} else {
		// Try to step down
		return true;
	}
}

bool FreescapeEngine::tryStepDown(Math::Vector3d currentPosition) {
	debugC(1, kFreescapeDebugMove, "Try to step down!");
	int areaScale = _currentArea->getScale();
	_position.set(_position.x(), _position.y() - areaScale, _position.z());
	if (checkFloor(_position)) {
		return true;
	} else {
		_position = currentPosition;
		return false;
	}
}


bool FreescapeEngine::checkCollisions(bool executeCode) {
	int areaScale = _currentArea->getScale();
	Math::Vector3d v1(_position.x() - areaScale * 0.75 * _playerWidth, _position.y() - areaScale * _playerHeight , _position.z() - areaScale * 0.75 * _playerDepth);
	Math::Vector3d v2(_position.x() + areaScale * 0.75 * _playerWidth, _position.y() + areaScale                 , _position.z() + areaScale * 0.75 * _playerDepth);

	const Math::AABB boundingBox(v1, v2);
	Object *obj = _currentArea->checkCollisions(boundingBox);

	if (obj != nullptr) {
		debugC(1, kFreescapeDebugMove, "Collided with object id %d of size %f %f %f", obj->getObjectID(), obj->getSize().x(), obj->getSize().y(), obj->getSize().z());
		GeometricObject *gobj = (GeometricObject*) obj;
		if (!executeCode) // Avoid executing code
			return true;

		executeObjectConditions(gobj, false, true);
		return true;
	}
	return false;
}

} // End of namespace