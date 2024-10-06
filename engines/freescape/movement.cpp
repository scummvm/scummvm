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

void FreescapeEngine::initKeymaps(Common::Keymap *engineKeyMap, Common::Keymap *infoScreenKeyMap, const char *target) {
	Common::Action *act;

	act = new Common::Action(Common::kStandardActionMoveUp, _("Up"));
	act->setCustomEngineActionEvent(kActionMoveUp);
	act->addDefaultInputMapping("UP");
	act->addDefaultInputMapping("JOY_UP");
	act->addDefaultInputMapping("o");
	engineKeyMap->addAction(act);

	act = new Common::Action(Common::kStandardActionMoveDown, _("Down"));
	act->setCustomEngineActionEvent(kActionMoveDown);
	act->addDefaultInputMapping("DOWN");
	act->addDefaultInputMapping("JOY_DOWN");
	act->addDefaultInputMapping("k");
	engineKeyMap->addAction(act);

	act = new Common::Action(Common::kStandardActionMoveLeft, _("Strafe Left"));
	act->setCustomEngineActionEvent(kActionMoveLeft);
	act->addDefaultInputMapping("LEFT");
	act->addDefaultInputMapping("JOY_LEFT");
	// act->addDefaultInputMapping("q");
	engineKeyMap->addAction(act);

	act = new Common::Action(Common::kStandardActionMoveRight, _("Strafe Right"));
	act->setCustomEngineActionEvent(kActionMoveRight);
	act->addDefaultInputMapping("RIGHT");
	act->addDefaultInputMapping("JOY_RIGHT");
	// act->addDefaultInputMapping("w");
	engineKeyMap->addAction(act);

	act = new Common::Action("SHOOT", _("Shoot"));
	act->setCustomEngineActionEvent(kActionShoot);
	act->addDefaultInputMapping("JOY_A");
	act->addDefaultInputMapping("KP5");
	act->addDefaultInputMapping("5");
	act->addDefaultInputMapping("KP0");
	act->addDefaultInputMapping("0");
	engineKeyMap->addAction(act);

	act = new Common::Action("ROTUP", _("Rotate up"));
	act->setCustomEngineActionEvent(kActionRotateUp);
	act->addDefaultInputMapping("p");
	engineKeyMap->addAction(act);

	act = new Common::Action("ROTDN", _("Rotate down"));
	act->setCustomEngineActionEvent(kActionRotateDown);
	act->addDefaultInputMapping("l");
	engineKeyMap->addAction(act);

	act = new Common::Action("SKIP", _("Skip"));
	act->setCustomEngineActionEvent(kActionSkip);
	act->addDefaultInputMapping("SPACE");
	act->addDefaultInputMapping("RETURN");
	act->addDefaultInputMapping("JOY_X");
	engineKeyMap->addAction(act);

  // I18N: Toggles between cursor lock modes, switching between free cursor movement and camera/head movement.
	act = new Common::Action("SWITCH", _("Change mode"));
	act->setCustomEngineActionEvent(kActionChangeMode);
	act->addDefaultInputMapping("SPACE");
	act->addDefaultInputMapping("JOY_X");
	engineKeyMap->addAction(act);

	act = new Common::Action("ESCAPE", _("Escape"));
	act->setCustomEngineActionEvent(kActionEscape);
	act->addDefaultInputMapping("ESCAPE");
	engineKeyMap->addAction(act);

	act = new Common::Action("MENU", _("Info Menu"));
	act->setCustomEngineActionEvent(kActionInfoMenu);
	act->addDefaultInputMapping("i");
	act->addDefaultInputMapping("JOY_GUIDE");
	engineKeyMap->addAction(act);
}

Math::AABB createPlayerAABB(Math::Vector3d const position, int playerHeight) {
	Math::AABB boundingBox(position, position);

	Math::Vector3d v1(position.x() + 1, position.y() - 1, position.z() + 1);
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
	float y = rotation.y();

	// Adjust _yaw based on normalized angle
	if (y >= 0 && y < 90)
		_yaw = 90 - y;			// 0 to 90 maps to 90 to 0 (yaw should be 90 to 0)
	else if (y >= 90 && y <= 180)
		_yaw = 450 - y;			// 90 to 180 maps to 360 to 270 (yaw should be 360 to 270)
	else if (y > 180 && y <= 225)
		_yaw = y;				// 180 to 225 maps to 180 to 225 (yaw should be 180 to 225)
	else if (y > 225 && y < 270)
		_yaw = y - 90;			// 180 to 270 maps to 90 to 0 (yaw should be 90 to 0)
	else
		_yaw = 360 + 90 - y;	// 270 to 360 maps to 90 to 180 (yaw should be 90 to 180)

	debugC(1, kFreescapeDebugMove, "entrace position: %f %f %f", _position.x(), _position.y(), _position.z());
	// Set the player height
	_playerHeight = 0;
	changePlayerHeight(_playerHeightNumber);
	debugC(1, kFreescapeDebugMove, "player height: %d", _playerHeight);

	_sensors = _currentArea->getSensors();
	_gfx->_scale = _currentArea->_scale;
}

void FreescapeEngine::activate() {
	Common::Point center(_viewArea.left + _viewArea.width() / 2, _viewArea.top + _viewArea.height() / 2);
	float xoffset = _crossairPosition.x - center.x;
	float yoffset = _crossairPosition.y - center.y;
	xoffset = xoffset * 0.33;
	yoffset = yoffset * 0.50;

	Math::Vector3d direction = directionToVector(_pitch - yoffset, _yaw - xoffset, false);
	Math::Ray ray(_position, direction);
	Object *interacted = _currentArea->checkCollisionRay(ray, 8192);
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
	if (_shootingFrames > 0) // No more than one shot at a time
		return;

	playSound(_soundIndexShoot, false);
	g_system->delayMillis(2);
	_shootingFrames = 10;

	Common::Point center(_viewArea.left + _viewArea.width() / 2, _viewArea.top + _viewArea.height() / 2);
	float xoffset = _crossairPosition.x - center.x;
	float yoffset = _crossairPosition.y - center.y;
	xoffset = xoffset * 0.33;
	yoffset = yoffset * 0.50;

	Math::Vector3d direction = directionToVector(_pitch - yoffset, _yaw - xoffset, false);
	Math::Ray ray(_position, direction);
	Object *shot = _currentArea->checkCollisionRay(ray, 8192);
	if (shot) {
		GeometricObject *gobj = (GeometricObject *)shot;
		debugC(1, kFreescapeDebugMove, "Shot object %d with flags %x", gobj->getObjectID(), gobj->getObjectFlags());

		if (!gobj->_conditionSource.empty())
			debugC(1, kFreescapeDebugMove, "Must use shot = true when executing: %s", gobj->_conditionSource.c_str());

		_delayedShootObject = gobj;
	}

	executeLocalGlobalConditions(true, false, false); // Only execute "on shot" room/global conditions
}

void FreescapeEngine::changeAngle() {
	_angleRotationIndex++;
	_angleRotationIndex = _angleRotationIndex % int(_angleRotations.size());
}

void FreescapeEngine::changePlayerHeight(int index) {
	int scale = _currentArea->getScale();

	_position.setValue(1, _position.y() - _playerHeight);
	_playerHeight = 32 * (index + 1) - 16 / float(scale);
	assert(_playerHeight > 0);
	_position.setValue(1, _position.y() + _playerHeight);
}

void FreescapeEngine::changeStepSize() {
	_playerStepIndex++;
	_playerStepIndex = _playerStepIndex % int(_playerSteps.size());
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
		if (_playerHeightNumber >= _playerHeightMaxNumber)
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
	int maxPositiveDistance = 8192;
	int maxNegativeDistance	= 0;

	if (_currentArea->isOutside()) {
		maxPositiveDistance = 16384;
		maxNegativeDistance = -16384;
	}

	for (int i = 0; i < 3; i++) {
		if (_position.getValue(i) < maxNegativeDistance)
			_position.setValue(i, maxNegativeDistance);
		else if (_position.getValue(i) > maxPositiveDistance)
			_position.setValue(i, maxPositiveDistance);
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
			playSound(_soundIndexClimb, false);
		}
		_position = newPosition;
		return;
	}

	if ((lastPosition - newPosition).length() < 1) { // If the player has not moved
		// Try to step up
		newPosition = position;
		newPosition.y() = newPosition.y() + _stepUpDistance;

		lastPosition = _lastPosition;
		lastPosition.y() = lastPosition.y() + _stepUpDistance;

		newPosition = _currentArea->resolveCollisions(lastPosition, newPosition, _playerHeight);
		if (_lastPosition.y() < newPosition.y())
			playSound(_soundIndexClimb, false);
	}

	if ((lastPosition - newPosition).length() < 1) { // Something is blocking the player
		if (!executed)
			setGameBit(31);

		playSound(_soundIndexCollide, false);
	}

	lastPosition = newPosition;
	newPosition.y() = -8192;
	newPosition = _currentArea->resolveCollisions(lastPosition, newPosition, _playerHeight);
	int fallen = lastPosition.y() - newPosition.y();

	if (fallen > _maxFallingDistance) {
		_hasFallen = !_disableFalling;
		_avoidRenderingFrames = 60 * 3;
		if (isEclipse()) // No need for an variable index, since these are special types of sound
			playSoundFx(0, true);
	}

	if (!_hasFallen && fallen > 0) {
		playSound(_soundIndexFall, false);

		// Position in Y was changed, let's re-run effects
		runCollisionConditions(lastPosition, newPosition);
	}
	_position = newPosition;
}

bool FreescapeEngine::runCollisionConditions(Math::Vector3d const lastPosition, Math::Vector3d const newPosition) {
	bool executed = false;
	uint16 areaID = _currentArea->getAreaID();
	GeometricObject *gobj = nullptr;
	Object *collided = nullptr;

	Math::Ray ray(newPosition, -_upVector);
	collided = _currentArea->checkCollisionRay(ray, _playerHeight + 3);
	if (collided) {
		gobj = (GeometricObject *)collided;
		debugC(1, kFreescapeDebugMove, "Collided down with object id %d of size %f %f %f", gobj->getObjectID(), gobj->getSize().x(), gobj->getSize().y(), gobj->getSize().z());
		executed |= executeObjectConditions(gobj, false, true, false);
	}

	if (areaID != _currentArea->getAreaID())
		return collided;

	Math::Vector3d direction = newPosition - lastPosition;
	direction.normalize();
	int rayLenght = 45;
	if (_currentArea->getScale() >= 5)
		rayLenght = MAX(5, 45 / (2 * _currentArea->getScale()));

	for (int i = 0; i <= 2; i++) {
		Math::Vector3d rayPosition = lastPosition;
		rayPosition.y() = rayPosition.y() - _playerHeight * (i / 4.0);
		ray = Math::Ray(rayPosition, direction);
		collided = _currentArea->checkCollisionRay(ray, rayLenght);
		if (collided) {
			gobj = (GeometricObject *)collided;
			debugC(1, kFreescapeDebugMove, "Collided with object id %d of size %f %f %f", gobj->getObjectID(), gobj->getSize().x(), gobj->getSize().y(), gobj->getSize().z());
			executed |= executeObjectConditions(gobj, false, true, false);
			break;
		}
	}

	return executed;
}

} // namespace Freescape
