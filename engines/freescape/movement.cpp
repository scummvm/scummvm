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

void FreescapeEngine::traverseEntrance(uint16 entranceID) {
	Entrance *entrance = (Entrance *)_currentArea->entranceWithID(entranceID);
	assert(entrance);

	int scale = _currentArea->getScale();
	assert(scale > 0);

	Math::Vector3d diff = _lastPosition - _position;
	Math::Vector3d rotation = entrance->getRotation();
	_position = entrance->getOrigin();
	_pitch = rotation.x();
	if (ABS(diff.x()) > ABS(diff.z()) && _lastPosition != Math::Vector3d(0, 0, 0)) {
		_yaw = rotation.y() - 90;
	} else {
		_yaw = rotation.y() + 90;
	}
	debugC(1, kFreescapeDebugMove, "entrace position: %f %f %f", _position.x(), _position.y(), _position.z());

	int delta = 0;
	if (scale == 2)
		delta = 8;
	else if (scale == 4)
		delta = 12;

	_playerHeight = _playerHeights[_playerHeightNumber] + delta;
	debugC(1, kFreescapeDebugMove, "player height: %d", _playerHeight);
	_position.setValue(1, _position.y() + _playerHeight);
}

void FreescapeEngine::shoot() {
	//_mixer->stopHandle(_soundFxHandle);
	playSound(1, true);
	_gfx->setViewport(_fullscreenViewArea);
	_gfx->renderShoot(0, _crossairPosition, _viewArea);
	_gfx->setViewport(_viewArea);

	Common::Point center(_viewArea.left + _viewArea.width() / 2, _viewArea.top + _viewArea.height() / 2);
	float xoffset = _crossairPosition.x - center.x;
	float yoffset = _crossairPosition.y - center.y;

	Math::Vector3d direction = directionToVector(_pitch + yoffset, _yaw - xoffset);
	Math::Ray ray(_position, direction);
	Object *shot = _currentArea->shootRay(ray);
	if (shot) {
		GeometricObject *gobj = (GeometricObject *)shot;
		debugC(1, kFreescapeDebugMove, "Shot object %d with flags %x", gobj->getObjectID(), gobj->getObjectFlags());

		if (!gobj->_conditionSource.empty())
			debugC(1, kFreescapeDebugMove, "Must use shot = true when executing: %s", gobj->_conditionSource.c_str());

		executeObjectConditions(gobj, true, false);
	}
	executeLocalGlobalConditions(true, false); // Only execute "on shot" room/global conditions
}

void FreescapeEngine::changePlayerHeight(int index) {
	int scale = _currentArea->getScale();
	int delta = 0;
	if (scale == 2)
		delta = 8;
	else if (scale == 4)
		delta = 12;

	_position.setValue(1, _position.y() - _playerHeight);
	_playerHeight = _playerHeights[index] + delta;
	_position.setValue(1, _position.y() + _playerHeight);
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

	float velocity = _movementSpeed * deltaTime;
	float positionY = _position.y();
	switch (direction) {
	case kForwardMovement:
		_position = _position + _cameraFront * velocity;
		break;
	case kBackwardMovement:
		_position = _position - _cameraFront * velocity;
		break;
	case kRightMovement:
		_position = _position - _cameraRight * velocity;
		break;
	case kLeftMovement:
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
		if (!hasFloor && !_flyMode) {
			int fallen;
			for (fallen = 1; fallen < 65 + 1; fallen++) {
				_position.set(_position.x(), positionY - fallen , _position.z());
				if (tryStepDown(_position))
					break;
			}
			fallen++;
			fallen++;
			if (fallen >= 67) {
				_position = _lastPosition; // error("NASTY FALL!");
				return;
			}
			_position.set(_position.x(), positionY - fallen, _position.z());
			playSound(3, true);
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
					playSound(4, true);
					debugC(1, kFreescapeDebugCode, "Runing effects:");
					checkCollisions(true); // run the effects (again)
				} else {
					playSound(2, true);
					_position = _lastPosition;
				}
			}
		}
	}

	_lastPosition = _position;
	debugC(1, kFreescapeDebugMove, "new player position: %f, %f, %f", _position.x(), _position.y(), _position.z());
	//debugC(1, kFreescapeDebugMove, "player height: %f", _position.y() - areaScale * _playerHeight);
	executeLocalGlobalConditions(false, true); // Only execute "on collision" room/global conditions
}

bool FreescapeEngine::checkFloor(Math::Vector3d currentPosition) {
	debugC(1, kFreescapeDebugMove, "Checking floor under the player");
	bool collided = checkCollisions(false);
	assert(!collided);

	_position.set(_position.x(), _position.y() - 2, _position.z());
	collided = checkCollisions(false);
	_position = currentPosition;
	return collided;
}

bool FreescapeEngine::tryStepUp(Math::Vector3d currentPosition) {
	debugC(1, kFreescapeDebugMove, "Try to step up!");
	_position.set(_position.x(), _position.y() + 64, _position.z());
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
	_position.set(_position.x(), _position.y() - 1, _position.z());
	if (checkFloor(_position)) {
		return true;
	} else {
		_position = currentPosition;
		return false;
	}
}

bool FreescapeEngine::checkCollisions(bool executeCode) {
	if (_noClipMode)
		return false;
	Math::AABB boundingBox(_lastPosition, _lastPosition);

	Math::Vector3d v1(_position.x() - 1, _position.y() - _playerHeight, _position.z() - 1);
	Math::Vector3d v2(_position.x() + 1, _position.y() + 1, _position.z() + 1);

	boundingBox.expand(v1);
	boundingBox.expand(v2);
	Object *obj = _currentArea->checkCollisions(boundingBox);

	if (obj != nullptr) {
		debugC(1, kFreescapeDebugMove, "Collided with object id %d of size %f %f %f", obj->getObjectID(), obj->getSize().x(), obj->getSize().y(), obj->getSize().z());
		GeometricObject *gobj = (GeometricObject *)obj;
		if (!executeCode) // Avoid executing code
			return true;

		executeObjectConditions(gobj, false, true);
		return true;
	}
	return false;
}

} // namespace Freescape
