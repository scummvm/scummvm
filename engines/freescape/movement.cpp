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

#include "common/translation.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/standard-actions.h"

#include "freescape/freescape.h"

namespace Freescape {

Common::Array<Common::Keymap *> FreescapeEngine::initKeymaps(const char *target) {
	Common::Keymap *engineKeyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, "freescape", "Freescape game");
	Common::Action *act;

	act = new Common::Action(Common::kStandardActionMoveUp, _("Up"));
	act->setKeyEvent(Common::KEYCODE_UP);
	act->allowKbdRepeats();
	act->addDefaultInputMapping("JOY_UP");
	engineKeyMap->addAction(act);

	act = new Common::Action(Common::kStandardActionMoveDown, _("Down"));
	act->setKeyEvent(Common::KEYCODE_DOWN);
	act->allowKbdRepeats();
	act->addDefaultInputMapping("JOY_DOWN");
	engineKeyMap->addAction(act);

	act = new Common::Action(Common::kStandardActionMoveLeft, _("Strafe Left"));
	act->setKeyEvent(Common::KEYCODE_LEFT);
	act->allowKbdRepeats();
	act->addDefaultInputMapping("JOY_LEFT");
	engineKeyMap->addAction(act);

	act = new Common::Action(Common::kStandardActionMoveRight, _("Strafe Right"));
	act->setKeyEvent(Common::KEYCODE_RIGHT);
	act->allowKbdRepeats();
	act->addDefaultInputMapping("JOY_RIGHT");
	engineKeyMap->addAction(act);

	act = new Common::Action("SHOOT", _("Shoot"));
	act->setKeyEvent(Common::KeyState(Common::KEYCODE_0, '0'));
	act->allowKbdRepeats();
	act->setLeftClickEvent();
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Common::Action("RISE", _("Rise/Fly up"));
	act->setKeyEvent(Common::KeyState(Common::KEYCODE_r, 'r'));
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Common::Action("LOWER", _("Lower/Fly down"));
	act->setKeyEvent(Common::KeyState(Common::KEYCODE_f, 'f'));
	engineKeyMap->addAction(act);

	act = new Common::Action("SWITCH", _("Change mode"));
	act->setKeyEvent(Common::KeyState(Common::KEYCODE_SPACE, Common::ASCII_SPACE));
	act->addDefaultInputMapping("JOY_X");
	engineKeyMap->addAction(act);

	act = new Common::Action("ROTL", _("Rotate Left"));
	act->setKeyEvent(Common::KeyState(Common::KEYCODE_q, 'q'));
	engineKeyMap->addAction(act);

	act = new Common::Action("ROTR", _("Rotate Right"));
	act->setKeyEvent(Common::KeyState(Common::KEYCODE_w, 'w'));
	engineKeyMap->addAction(act);

	act = new Common::Action("MENU", _("Info Menu"));
	act->setKeyEvent(Common::KeyState(Common::KEYCODE_i, 'i'));
	act->addDefaultInputMapping("JOY_GUIDE");
	engineKeyMap->addAction(act);

	return Common::Keymap::arrayOf(engineKeyMap);
}

Math::AABB createPlayerAABB(Math::Vector3d const position, int playerHeight) {
	Math::AABB boundingBox(position, position);

	Math::Vector3d v1(position.x() + 1, position.y() + 1, position.z() + 1);
	Math::Vector3d v2(position.x() - 1, position.y() - playerHeight, position.z() - 1);

	boundingBox.expand(v1);
	boundingBox.expand(v2);
	return boundingBox;
}

void FreescapeEngine::gotoArea(uint16 areaID, int entranceID) {
	error("Function \"%s\" not implemented", __FUNCTION__);
}

void FreescapeEngine::traverseEntrance(uint16 entranceID) {
	Entrance *entrance = (Entrance *)_currentArea->entranceWithID(entranceID);
	assert(entrance);

	int scale = _currentArea->getScale();
	assert(scale > 0);

	Math::Vector3d rotation = entrance->getRotation();
	_position = entrance->getOrigin();

	if (scale == 1) {
		_position.x() = _position.x() + 16;
		_position.z() = _position.z() + 16;
	} else if (scale == 5) {
		_position.x() = _position.x() + 4;
		_position.z() = _position.z() + 4;
	}

	_pitch = rotation.x();
	if (rotation.y() > 0 && rotation.y() <= 45)
		_yaw = rotation.y();
	else if (rotation.y() <= 0 || (rotation.y() >= 180 && rotation.y() < 270))
		_yaw = rotation.y() + 90;
	else
		_yaw = rotation.y() - 90;

	debugC(1, kFreescapeDebugMove, "entrace position: %f %f %f", _position.x(), _position.y(), _position.z());

	int delta = 0;
	if (scale == 2)
		delta = 8;
	else if (scale == 4)
		delta = 12;

	if (_playerHeightNumber >= 0)
		_playerHeight = _playerHeights[_playerHeightNumber] + delta;
	else
		_playerHeight = 2;
	debugC(1, kFreescapeDebugMove, "player height: %d", _playerHeight);
	_position.setValue(1, _position.y() + _playerHeight);

	_sensors = _currentArea->getSensors();
}

void FreescapeEngine::activate() {
	Common::Point center(_viewArea.left + _viewArea.width() / 2, _viewArea.top + _viewArea.height() / 2);
	float xoffset = _crossairPosition.x - center.x;
	float yoffset = _crossairPosition.y - center.y;
	xoffset = xoffset * 0.33;
	yoffset = yoffset * 0.50;

	Math::Vector3d direction = directionToVector(_pitch - yoffset, _yaw - xoffset);
	Math::Ray ray(_position, direction);
	Object *interacted = _currentArea->shootRay(ray);
	if (interacted) {
		GeometricObject *gobj = (GeometricObject *)interacted;
		debugC(1, kFreescapeDebugMove, "Interact with object %d with flags %x", gobj->getObjectID(), gobj->getObjectFlags());

		if (!gobj->_conditionSource.empty())
			debugC(1, kFreescapeDebugMove, "Must use interact = true when executing: %s", gobj->_conditionSource.c_str());

		executeObjectConditions(gobj, false, false, true);
	}
	//executeLocalGlobalConditions(true, false, false); // Only execute "on shot" room/global conditions
}


void FreescapeEngine::shoot() {
	playSound(1, false);
	g_system->delayMillis(2);
	_shootingFrames = 4;

	Common::Point center(_viewArea.left + _viewArea.width() / 2, _viewArea.top + _viewArea.height() / 2);
	float xoffset = _crossairPosition.x - center.x;
	float yoffset = _crossairPosition.y - center.y;
	xoffset = xoffset * 0.33;
	yoffset = yoffset * 0.50;

	Math::Vector3d direction = directionToVector(_pitch - yoffset, _yaw - xoffset);
	Math::Ray ray(_position, direction);
	Object *shot = _currentArea->shootRay(ray);
	if (shot) {
		GeometricObject *gobj = (GeometricObject *)shot;
		debugC(1, kFreescapeDebugMove, "Shot object %d with flags %x", gobj->getObjectID(), gobj->getObjectFlags());

		if (!gobj->_conditionSource.empty())
			debugC(1, kFreescapeDebugMove, "Must use shot = true when executing: %s", gobj->_conditionSource.c_str());

		executeObjectConditions(gobj, true, false, false);
	}
	executeLocalGlobalConditions(true, false, false); // Only execute "on shot" room/global conditions
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

void FreescapeEngine::increaseStepSize() {
	if (_playerStepIndex == int(_playerSteps.size()) - 1)
		return;

	_playerStepIndex++;
}

void FreescapeEngine::decreaseStepSize() {
	if (_playerStepIndex == 0)
		return;

	_playerStepIndex--;
}

void FreescapeEngine::rise() {
	debugC(1, kFreescapeDebugMove, "playerHeightNumber: %d", _playerHeightNumber);
	int previousAreaID = _currentArea->getAreaID();
	if (_flyMode) {
		Math::Vector3d destination = _position;
		destination.y() = destination.y() + _playerSteps[_playerStepIndex];
		resolveCollisions(destination);
	} else {
		if (_playerHeightNumber == int(_playerHeights.size()) - 1)
			return;

		_playerHeightNumber++;
		changePlayerHeight(_playerHeightNumber);

		Math::AABB boundingBox = createPlayerAABB(_position, _playerHeight);
		ObjectArray objs = _currentArea->checkCollisions(boundingBox);
		bool collided = objs.size() > 0;
		if (collided) {
			if (_currentArea->getAreaID() == previousAreaID) {
				_playerHeightNumber--;
				changePlayerHeight(_playerHeightNumber);
			}
		}
	}
	checkIfStillInArea();
	_lastPosition = _position;
	debugC(1, kFreescapeDebugMove, "new player position: %f, %f, %f", _position.x(), _position.y(), _position.z());
	executeMovementConditions();
}

void FreescapeEngine::lower() {
	debugC(1, kFreescapeDebugMove, "playerHeightNumber: %d", _playerHeightNumber);
	if (_flyMode) {
		Math::Vector3d destination = _position;
		destination.y() = destination.y() - _playerSteps[_playerStepIndex];
		resolveCollisions(destination);
	} else {
		if (_playerHeightNumber == 0)
			return;

		_playerHeightNumber--;
		changePlayerHeight(_playerHeightNumber);
	}
	checkIfStillInArea();
	_lastPosition = _position;
	debugC(1, kFreescapeDebugMove, "new player position: %f, %f, %f", _position.x(), _position.y(), _position.z());
	executeMovementConditions();
}

void FreescapeEngine::checkIfStillInArea() {
	for (int i = 0; i < 3; i++) {
		if (_position.getValue(i) < 0)
			_position.setValue(i, 0);
		else if (_position.getValue(i) > 8128)
			_position.setValue(i, 8128);
	}
	if (_position.y() >= 2016)
		_position.y() = _lastPosition.z();
}

void FreescapeEngine::move(CameraMovement direction, uint8 scale, float deltaTime) {
	debugC(1, kFreescapeDebugMove, "old player position: %f, %f, %f", _position.x(), _position.y(), _position.z());
	int previousAreaID = _currentArea->getAreaID();

	Math::Vector3d stepFront = _cameraFront * (float(_playerSteps[_playerStepIndex]) / 2 / _cameraFront.length());
	Math::Vector3d stepRight = _cameraRight * (float(_playerSteps[_playerStepIndex]) / 2 / _cameraRight.length());

	stepFront.x() = floor(stepFront.x()) + 0.5;
	stepFront.z() = floor(stepFront.z()) + 0.5;

	float positionY = _position.y();
	Math::Vector3d destination;
	switch (direction) {
	case kForwardMovement:
		destination = _position + stepFront;
		break;
	case kBackwardMovement:
		destination = _position - stepFront;
		break;
	case kRightMovement:
		destination = _position - stepRight;
		break;
	case kLeftMovement:
		destination = _position + stepRight;
		break;
	}

	if (!_flyMode)
		destination.y() = positionY;
	resolveCollisions(destination);
	checkIfStillInArea();

	_lastPosition = _position;
	debugC(1, kFreescapeDebugMove, "new player position: %f, %f, %f", _position.x(), _position.y(), _position.z());
	//debugC(1, kFreescapeDebugMove, "player height: %f", _position.y() - areaScale * _playerHeight);
	if (_currentArea->getAreaID() == previousAreaID)
		executeMovementConditions();
	_gotoExecuted = false;
	clearGameBit(31);
}

void FreescapeEngine::resolveCollisions(Math::Vector3d const position) {
	if (_noClipMode) {
		_position = position;
		return;
	}

	Math::Vector3d newPosition = position;
	Math::Vector3d lastPosition = _lastPosition;

	_gotoExecuted = false;
	bool executed = runCollisionConditions(lastPosition, newPosition);
	if (_gotoExecuted) {
		_gotoExecuted = false;
		return;
	}

	newPosition = _currentArea->resolveCollisions(lastPosition, newPosition, _playerHeight);

	if (_flyMode) {
		if ((lastPosition - newPosition).length() < 1) { // Something is blocking the player
			if (!executed)
				setGameBit(31);
			playSound(4, false);
		}
		_position = newPosition;
		return;
	}

	if ((lastPosition - newPosition).length() < 1) { // If the player has not moved
		// Try to step up
		newPosition = position;
		newPosition.y() = newPosition.y() + 64;

		lastPosition = _lastPosition;
		lastPosition.y() = lastPosition.y() + 64;

		newPosition = _currentArea->resolveCollisions(lastPosition, newPosition, _playerHeight);
	}

	if ((lastPosition - newPosition).length() < 1) { // Something is blocking the player
		if (!executed)
			setGameBit(31);
		playSound(4, false);
	}

	lastPosition = newPosition;
	newPosition.y() = -8192;
	newPosition = _currentArea->resolveCollisions(lastPosition, newPosition, _playerHeight);
	int fallen = lastPosition.y() - newPosition.y();

	if (fallen > 64)
		_hasFallen = !_disableFalling;

	if (!_hasFallen && fallen > 0) {
		// Position in Y was changed, let's re-run effects
		runCollisionConditions(lastPosition, newPosition);
	}
	_position = newPosition;
}

bool FreescapeEngine::runCollisionConditions(Math::Vector3d const lastPosition, Math::Vector3d const newPosition) {
	bool executed = false;
	// We need to make sure the bounding box touches the floor so we will expand it and run the collision checking
	uint tolerance = isCastle() ? 1 : 3;

	int yDifference = _flyMode ? tolerance : -_playerHeight - tolerance;
	Math::Vector3d v(newPosition.x() - 1, newPosition.y() + yDifference, newPosition.z() - 1);
	Math::AABB boundingBox(lastPosition, lastPosition);
	boundingBox.expand(v);

	ObjectArray objs = _currentArea->checkCollisions(boundingBox);

	// sort so the condition from those objects that are larger are executed last
	struct {
		bool operator()(Object *object1, Object *object2) {
			return object1->getSize().length() < object2->getSize().length();
		};
	} compareObjectsSizes;

	Common::sort(objs.begin(), objs.end(), compareObjectsSizes);
	uint16 areaID = _currentArea->getAreaID();

	bool largeObjectWasBlocking = false;
	for (auto &obj : objs) {
		GeometricObject *gobj = (GeometricObject *)obj;
		debugC(1, kFreescapeDebugMove, "Collided with object id %d of size %f %f %f", gobj->getObjectID(), gobj->getSize().x(), gobj->getSize().y(), gobj->getSize().z());
		// The following check stops the player from going through big solid objects such as walls
		// FIXME: find a better workaround of this
		if (gobj->getSize().length() > 3000) {
			if (largeObjectWasBlocking && !(isDriller() && _currentArea->getAreaID() == 14))
				continue;
			largeObjectWasBlocking = true;
		}

		executed |= executeObjectConditions(gobj, false, true, false);

		if (areaID != _currentArea->getAreaID())
			break;
	}
	return executed;
}

} // namespace Freescape
