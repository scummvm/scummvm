/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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

#include "neverhood/modules/module2200_sprites.h"

namespace Neverhood {

AsScene2201CeilingFan::AsScene2201CeilingFan(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1100) {

	_x = 403;
	_y = 259;
	createSurface(100, 233, 96);
	startAnimation(0x8600866, 0, -1);
	SetUpdateHandler(&AnimatedSprite::update);
}

AsScene2201Door::AsScene2201Door(NeverhoodEngine *vm, Klaymen *klaymen, Sprite *ssDoorLight, bool isOpen)
	: AnimatedSprite(vm, 1100), _klaymen(klaymen), _ssDoorLight(ssDoorLight), _countdown(0), _isOpen(isOpen) {

	_x = 408;
	_y = 290;
	createSurface(900, 63, 266);
	SetUpdateHandler(&AsScene2201Door::update);
	SetMessageHandler(&AsScene2201Door::handleMessage);
	if (_isOpen) {
		startAnimation(0xE2CB0412, -1, -1);
		_countdown = 48;
		_newStickFrameIndex = STICK_LAST_FRAME;
	} else {
		startAnimation(0xE2CB0412, 0, -1);
		_newStickFrameIndex = 0;
		_ssDoorLight->setVisible(false);
	}
}

void AsScene2201Door::update() {
	if (_countdown != 0 && _isOpen && (--_countdown == 0))
		stCloseDoor();
	AnimatedSprite::update();
}

uint32 AsScene2201Door::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x11001090) {
			if (_isOpen)
				_ssDoorLight->setVisible(true);
		} else if (param.asInteger() == 0x11283090) {
			if (!_isOpen)
				_ssDoorLight->setVisible(false);
		}
		break;
	case NM_ANIMATION_UPDATE:
		if (_isOpen)
			_countdown = 144;
		messageResult = _isOpen ? 1 : 0;
		break;
	case NM_ANIMATION_STOP:
		gotoNextState();
		break;
	case NM_KLAYMEN_OPEN_DOOR:
		_countdown = 144;
		if (!_isOpen)
			stOpenDoor();
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene2201Door::stOpenDoor() {
	_isOpen = true;
	startAnimation(0xE2CB0412, 0, -1);
	_newStickFrameIndex = STICK_LAST_FRAME;
	playSound(0, calcHash("fxDoorOpen33"));
}

void AsScene2201Door::stCloseDoor() {
	_isOpen = false;
	startAnimation(0xE2CB0412, -1, -1);
	_playBackwards = true;
	_newStickFrameIndex = 0;
	playSound(0, calcHash("fxDoorClose33"));
}

SsScene2201PuzzleCube::SsScene2201PuzzleCube(NeverhoodEngine *vm, uint32 positionIndex, uint32 cubeIndex)
	: StaticSprite(vm, 900) {

	createSurface(100, 16, 16);
	loadSprite(kSsScene2201PuzzleCubeFileHashes[cubeIndex], kSLFCenteredDrawOffset | kSLFSetPosition, 0,
		kSsScene2201PuzzleCubePoints[positionIndex].x, kSsScene2201PuzzleCubePoints[positionIndex].y);
}

static const NPoint kSsScene2202PuzzleCubePoints[] = {
	{196, 105}, {323, 102}, {445, 106},
	{192, 216}, {319, 220}, {446, 216},
	{188, 320}, {319, 319}, {443, 322}
};

static const uint32 kSsScene2202PuzzleCubeFileHashes1[] = {
	0xA500800C, 0x2182910C, 0x2323980C,
	0x23049084, 0x21008080, 0x2303900C,
	0x6120980C, 0x2504D808
};

static const uint32 kSsScene2202PuzzleCubeFileHashes2[] = {
	0x0AAD8080, 0x0A290291, 0x0A2BA398,
	0x822B8490, 0x86298080, 0x0A2B8390,
	0x0A69A098, 0x0E2D84D8
};

SsScene2202PuzzleCube::SsScene2202PuzzleCube(NeverhoodEngine *vm, Scene *parentScene, int16 cubePosition, int16 cubeSymbol)
	: StaticSprite(vm, 900), _parentScene(parentScene), _cubeSymbol(cubeSymbol), _cubePosition(cubePosition), _isMoving(false) {

	int surfacePriority;

	SetUpdateHandler(&SsScene2202PuzzleCube::update);
	SetMessageHandler(&SsScene2202PuzzleCube::handleMessage);
	if (_cubePosition >= 0 && _cubePosition <= 2)
		surfacePriority = 100;
	else if (_cubePosition >= 3 && _cubePosition <= 5)
		surfacePriority = 300;
	else
		surfacePriority = 500;
	debug(1, "TODO: Unused SurfacePriority: %d", surfacePriority);
	loadSprite(kSsScene2202PuzzleCubeFileHashes2[_cubeSymbol], kSLFCenteredDrawOffset | kSLFSetPosition | kSLFDefCollisionBoundsOffset, 0,
		kSsScene2202PuzzleCubePoints[_cubePosition].x, kSsScene2202PuzzleCubePoints[_cubePosition].y);
	loadSound(0, 0x40958621);
	loadSound(1, 0x51108241);
}

void SsScene2202PuzzleCube::update() {
	handleSpriteUpdate();
	updatePosition();
}

uint32 SsScene2202PuzzleCube::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (!_isMoving && !getGlobalVar(V_TILE_PUZZLE_SOLVED))
			sendMessage(_parentScene, 0x2000, _cubePosition);
		messageResult = 1;
		break;
	case 0x2001:
		_isMoving = true;
		moveCube(param.asInteger());
		break;
	default:
		break;
	}
	return messageResult;
}

void SsScene2202PuzzleCube::suMoveCubeX() {

	bool done = false;

	if (_counterDirection) {
		if (_counter > 2)
			_counter -= 2;
	} else {
		if (_counter < 20)
			_counter += 2;
	}

	for (int16 i = 0; i < _counter; i++) {
		_x += _xIncr;
		_errValue += _yDelta;
		if (_errValue >= _xDelta) {
			_errValue -= _xDelta;
			_y += _yIncr;
		}
		if (_x == _newX && _y == _newY) {
			done = true;
			break;
		}
		if (_x == _xFlagPos)
			_counterDirection = true;
	}

	if (done)
		stopMoving();

	updateBounds();

}

void SsScene2202PuzzleCube::suMoveCubeY() {

	bool done = false;

	if (_counterDirection) {
		if (_counter > 2)
			_counter -= 2;
	} else {
		if (_counter < 20)
			_counter += 2;
	}

	for (int16 i = 0; i < _counter; i++) {
		_y += _yIncr;
		_errValue += _xDelta;
		if (_errValue >= _yDelta) {
			_errValue -= _yDelta;
			_x += _xIncr;
		}
		if (_x == _newX && _y == _newY) {
			done = true;
			break;
		}
		if (_x == _xFlagPos)
			_counterDirection = true;
	}

	if (done)
		stopMoving();

	updateBounds();

}

void SsScene2202PuzzleCube::moveCube(int16 newCubePosition) {

	loadSprite(kSsScene2202PuzzleCubeFileHashes1[_cubeSymbol], kSLFCenteredDrawOffset);

	setSubVar(VA_CUBE_POSITIONS, _cubePosition, (uint32)-1);
	setSubVar(VA_CUBE_POSITIONS, newCubePosition, (uint32)_cubeSymbol);

	_cubePosition = newCubePosition;
	_errValue = 0;
	_counterDirection = false;
	_counter = 0;
	_newX = kSsScene2202PuzzleCubePoints[newCubePosition].x;
	_newY = kSsScene2202PuzzleCubePoints[newCubePosition].y;

	if (_x == _newX && _y == _newY)
		return;

	if (_x <= _newX) {
		if (_y <= _newY) {
			_xDelta = _newX - _x;
			_yDelta = _newY - _y;
			_xIncr = 1;
			_yIncr = 1;
		} else {
			_xDelta = _newX - _x;
			_yDelta = _y - _newY;
			_xIncr = 1;
			_yIncr = -1;
		}
	} else {
		if (_y <= _newY) {
			_xDelta = _x - _newX;
			_yDelta = _newY - _y;
			_xIncr = -1;
			_yIncr = 1;
		} else {
			_xDelta = _x - _newX;
			_yDelta = _y - _newY;
			_xIncr = -1;
			_yIncr = -1;
		}
	}

	if (_xDelta > _yDelta) {
		SetSpriteUpdate(&SsScene2202PuzzleCube::suMoveCubeX);
		if (_xIncr > 0) {
			if (_newX - _x >= 180)
				_xFlagPos = _newX - 90;
			else
				_xFlagPos = _x + _newX / 2;
		} else {
			if (_x - _newX >= 180)
				_xFlagPos = _x + 90;
			else
				_xFlagPos = _x / 2 + _newX;
		}
		playSound(0);
	} else {
		SetSpriteUpdate(&SsScene2202PuzzleCube::suMoveCubeY);
		if (_yIncr > 0) {
			if (_newY - _y >= 180)
				_xFlagPos = _newY - 90;
			else
				_xFlagPos = _y + _newY / 2;
		} else {
			if (_y - _newY >= 180)
				_xFlagPos = _y + 90;
			else
				_xFlagPos = _y / 2 + _newY;
		}
		playSound(1);
	}

}

void SsScene2202PuzzleCube::stopMoving() {
	loadSprite(kSsScene2202PuzzleCubeFileHashes2[_cubeSymbol], kSLFCenteredDrawOffset);
	SetSpriteUpdate(NULL);
	_isMoving = false;
	sendMessage(_parentScene, NM_POSITION_CHANGE, _cubePosition);
}

static const uint32 kAsCommonKeyFileHashes[] = {
	0x2450D850,	0x0C9CE8D0,	0x2C58A152
};

AsCommonKey::AsCommonKey(NeverhoodEngine *vm, Scene *parentScene, int keyIndex, int surfacePriority, int16 x, int16 y)
	: AnimatedSprite(vm, kAsCommonKeyFileHashes[keyIndex], surfacePriority, x, y), _parentScene(parentScene), _keyIndex(keyIndex) {

	if (!getSubVar(VA_HAS_KEY, _keyIndex) && !getSubVar(VA_IS_KEY_INSERTED, _keyIndex)) {
		SetMessageHandler(&AsCommonKey::handleMessage);
	} else {
		// If Klaymen already has the key or it's already inserted then don't show it
		setVisible(false);
		SetMessageHandler(NULL);
	}
}

uint32 AsCommonKey::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		sendMessage(_parentScene, 0x4826, 0);
		messageResult = 1;
		break;
	case NM_KLAYMEN_USE_OBJECT:
		setSubVar(VA_HAS_KEY, _keyIndex, 1);
		setVisible(false);
		SetMessageHandler(NULL);
		break;
	default:
		break;
	}
	return messageResult;
}

static const uint32 kAsScene2203DoorFileHashes[] = {
	0x7868AE10,	0x1A488110
};

AsScene2203Door::AsScene2203Door(NeverhoodEngine *vm, Scene *parentScene, uint doorIndex)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _doorIndex(doorIndex) {

	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2203Door::handleMessage);
	_x = 320;
	_y = 240;
	createSurface1(kAsScene2203DoorFileHashes[_doorIndex], 900);
	if (getGlobalVar(V_LARGE_DOOR_NUMBER) == _doorIndex) {
		startAnimation(kAsScene2203DoorFileHashes[_doorIndex], -1, -1);
		_newStickFrameIndex = STICK_LAST_FRAME;
	} else {
		startAnimation(kAsScene2203DoorFileHashes[_doorIndex], 0, -1);
		_newStickFrameIndex = 0;
	}
}

uint32 AsScene2203Door::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_doorIndex == getGlobalVar(V_LARGE_DOOR_NUMBER))
			sendMessage(_parentScene, NM_POSITION_CHANGE, 0);
		else
			sendMessage(_parentScene, 0x2001, 0);
		messageResult = 1;
		break;
	case NM_ANIMATION_UPDATE:
		_otherDoor = (Sprite*)param.asEntity();
		break;
	case NM_ANIMATION_STOP:
		if (_doorIndex == getGlobalVar(V_LARGE_DOOR_NUMBER))
			sendMessage(_parentScene, NM_KLAYMEN_OPEN_DOOR, 0);
		stopAnimation();
		break;
	case NM_KLAYMEN_OPEN_DOOR:
		setGlobalVar(V_LARGE_DOOR_NUMBER, _doorIndex);
		sendMessage(_otherDoor, NM_KLAYMEN_CLOSE_DOOR, 0);
		openDoor();
		break;
	case NM_KLAYMEN_CLOSE_DOOR:
		closeDoor();
		sendMessage(_parentScene, 0x2003, 0);
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene2203Door::openDoor() {
	playSound(0, 0x341014C4);
	startAnimation(kAsScene2203DoorFileHashes[_doorIndex], 1, -1);
}

void AsScene2203Door::closeDoor() {
	startAnimation(kAsScene2203DoorFileHashes[_doorIndex], -1, -1);
	_playBackwards = true;
	_newStickFrameIndex = 0;
}

SsScene2205DoorFrame::SsScene2205DoorFrame(NeverhoodEngine *vm)
	: StaticSprite(vm, 900) {

	SetMessageHandler(&SsScene2205DoorFrame::handleMessage);
	createSurface(1100, 45, 206);
	loadSprite(getGlobalVar(V_LIGHTS_ON) ? 0x24306227 : 0xD90032A0, kSLFDefDrawOffset | kSLFDefPosition);
}

uint32 SsScene2205DoorFrame::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_UPDATE:
		loadSprite(getGlobalVar(V_LIGHTS_ON) ? 0x24306227 : 0xD90032A0, kSLFDefDrawOffset | kSLFDefPosition);
		break;
	default:
		break;
	}
	return messageResult;
}

static const int16 kAsScene2206DoorSpikesXDeltasOpen[] = {
	-24, -28, -18, 6, 9, -8
};

static const int16 kAsScene2206DoorSpikesXDeltasClose[] = {
	-8, 7, 11, 26, 13, 14
};

AsScene2206DoorSpikes::AsScene2206DoorSpikes(NeverhoodEngine *vm, uint32 fileHash)
	: StaticSprite(vm, fileHash, 200) {

	if (getGlobalVar(V_SPIKES_RETRACTED))
		_x -= 63;
	SetUpdateHandler(&AsScene2206DoorSpikes::update);
	SetMessageHandler(&AsScene2206DoorSpikes::handleMessage);
	SetSpriteUpdate(NULL);
}

void AsScene2206DoorSpikes::update() {
	handleSpriteUpdate();
	updatePosition();
}

uint32 AsScene2206DoorSpikes::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_KLAYMEN_OPEN_DOOR:
		_deltaIndex = 0;
		playSound(0, 0x032746E0);
		SetMessageHandler(NULL);
		SetSpriteUpdate(&AsScene2206DoorSpikes::suOpen);
		break;
	case NM_KLAYMEN_CLOSE_DOOR:
		_deltaIndex = 0;
		playSound(0, 0x002642C0);
		SetMessageHandler(NULL);
		SetSpriteUpdate(&AsScene2206DoorSpikes::suClose);
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene2206DoorSpikes::suOpen() {
	if (_deltaIndex < 6) {
		_x += kAsScene2206DoorSpikesXDeltasOpen[_deltaIndex];
		_deltaIndex++;
	} else {
		SetMessageHandler(&AsScene2206DoorSpikes::handleMessage);
		SetSpriteUpdate(NULL);
	}
}

void AsScene2206DoorSpikes::suClose() {
	if (_deltaIndex < 6) {
		_x += kAsScene2206DoorSpikesXDeltasClose[_deltaIndex];
		_deltaIndex++;
	} else {
		SetMessageHandler(&AsScene2206DoorSpikes::handleMessage);
		SetSpriteUpdate(NULL);
	}
}

AsScene2206Platform::AsScene2206Platform(NeverhoodEngine *vm, uint32 fileHash)
	: StaticSprite(vm, fileHash, 50) {

	SetUpdateHandler(&AsScene2206Platform::update);
	SetMessageHandler(&AsScene2206Platform::handleMessage);
	SetSpriteUpdate(NULL);
}

void AsScene2206Platform::update() {
	handleSpriteUpdate();
	updatePosition();
}

uint32 AsScene2206Platform::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x4803:
		_yDelta = 0;
		SetMessageHandler(NULL);
		SetSpriteUpdate(&AsScene2206Platform::suMoveDown);
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene2206Platform::suMoveDown() {
	_yDelta++;
	_y += _yDelta;
}

SsScene2206TestTube::SsScene2206TestTube(NeverhoodEngine *vm, Scene *parentScene, int surfacePriority, uint32 fileHash)
	: StaticSprite(vm, fileHash, surfacePriority), _parentScene(parentScene) {

	if (getGlobalVar(V_HAS_TEST_TUBE)) {
		setVisible(false);
		SetMessageHandler(NULL);
	} else
		SetMessageHandler(&SsScene2206TestTube::handleMessage);
	_collisionBoundsOffset = _drawOffset;
	updateBounds();
}

uint32 SsScene2206TestTube::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		sendMessage(_parentScene, 0x4826, 0);
		messageResult = 1;
		break;
	case NM_KLAYMEN_USE_OBJECT:
		setGlobalVar(V_HAS_TEST_TUBE, 1);
		setVisible(false);
		SetMessageHandler(NULL);
		break;
	default:
		break;
	}
	return messageResult;
}

AsScene2207Elevator::AsScene2207Elevator(NeverhoodEngine *vm, Scene *parentScene)
	: AnimatedSprite(vm, 900), _parentScene(parentScene), _pointIndex(0), _destPointIndex(0), _destPointIndexDelta(0) {

	NPoint pt;

	_dataResource.load(0x00524846);
	_pointArray = _dataResource.getPointArray(0x005B02B7);
	pt = _dataResource.getPoint(0x403A82B1);
	_x = pt.x;
	_y = pt.y;
	createSurface(1100, 129, 103);
	startAnimation(getGlobalVar(V_LIGHTS_ON) ? 0xC858CC19 : 0x294B3377, 0, 0);
	_newStickFrameIndex = 0;
	SetUpdateHandler(&AsScene2207Elevator::update);
	SetMessageHandler(&AsScene2207Elevator::handleMessage);
	SetSpriteUpdate(&AsScene2207Elevator::suSetPosition);
}

AsScene2207Elevator::~AsScene2207Elevator() {
	_vm->_soundMan->deleteSoundGroup(0x02700413);
}

void AsScene2207Elevator::update() {

	if (_destPointIndex + _destPointIndexDelta > _pointIndex) {
		_pointIndex++;
		startAnimation(getGlobalVar(V_LIGHTS_ON) ? 0xC858CC19 : 0x294B3377, _pointIndex, _pointIndex);
		_newStickFrameIndex = _pointIndex;
		if (_destPointIndex + _destPointIndexDelta == _pointIndex) {
			if (_destPointIndexDelta != 0)
				_destPointIndexDelta = 0;
			else {
				_vm->_soundMan->deleteSound(0xD3B02847);
				playSound(0, 0x53B8284A);
			}
		}
	}

	if (_destPointIndex + _destPointIndexDelta < _pointIndex) {
		_pointIndex--;
		if (_pointIndex == 0)
			sendMessage(_parentScene, 0x2003, 0);
		startAnimation(getGlobalVar(V_LIGHTS_ON) ? 0xC858CC19 : 0x294B3377, _pointIndex, _pointIndex);
		_newStickFrameIndex = _pointIndex;
		if (_destPointIndex + _destPointIndexDelta == _pointIndex) {
			if (_destPointIndexDelta != 0)
				_destPointIndexDelta = 0;
			else {
				_vm->_soundMan->deleteSound(0xD3B02847);
				playSound(0, 0x53B8284A);
			}
		}
	}

	if (_pointIndex > 20 && _surface->getPriority() != 900)
		sendMessage(_parentScene, NM_POSITION_CHANGE, 900);
	else if (_pointIndex < 20 && _surface->getPriority() != 1100)
		sendMessage(_parentScene, NM_POSITION_CHANGE, 1100);

	AnimatedSprite::update();

	if (_destPointIndex + _destPointIndexDelta == _pointIndex && _isMoving) {
		sendMessage(_parentScene, 0x2004, 0);
		_isMoving = false;
	}

}

void AsScene2207Elevator::suSetPosition() {
	_x = (*_pointArray)[_pointIndex].x;
	_y = (*_pointArray)[_pointIndex].y - 60;
	updateBounds();
}

uint32 AsScene2207Elevator::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_UPDATE:
		moveToY(param.asInteger());
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene2207Elevator::moveToY(int16 y) {
	int16 minDistance = 480;

	if (!_pointArray || _pointArray->size() == 0)
		return;

	for (uint i = 0; i < _pointArray->size(); i++) {
		int16 distance = ABS(y - (*_pointArray)[i].y);
		if (distance < minDistance) {
			minDistance = distance;
			_destPointIndex = i;
		}
	}

	if (_destPointIndex != _pointIndex) {
		if (_destPointIndex == 0 || _destPointIndex == (int)_pointArray->size() - 1)
			_destPointIndexDelta = 0;
		else if (_destPointIndex < _pointIndex)
			_destPointIndexDelta = -2;
		else
			_destPointIndexDelta = 2;
		_vm->_soundMan->addSound(0x02700413, 0xD3B02847);
		_vm->_soundMan->playSoundLooping(0xD3B02847);
	}

	_isMoving = true;

}

AsScene2207Lever::AsScene2207Lever(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y, int doDeltaX)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene) {

	_x = x;
	_y = y;
	createSurface(1010, 71, 73);
	setDoDeltaX(doDeltaX);
	startAnimation(0x80880090, 0, -1);
	_newStickFrameIndex = 0;
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2207Lever::handleMessage);
}

uint32 AsScene2207Lever::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		sendMessage(_parentScene, 0x4826, 0);
		messageResult = 1;
		break;
	case NM_ANIMATION_STOP:
		gotoNextState();
		stopAnimation();
		break;
	case NM_KLAYMEN_RAISE_LEVER:
		stLeverUp();
		break;
	case NM_KLAYMEN_LOWER_LEVER:
		stLeverDown();
		break;
	case NM_MOVE_TO_BACK:
		sendMessage(_parentScene, NM_PRIORITY_CHANGE, 990);
		break;
	case NM_MOVE_TO_FRONT:
		sendMessage(_parentScene, NM_PRIORITY_CHANGE, 1010);
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene2207Lever::stLeverDown() {
	startAnimation(0x80880090, 1, -1);
	playSound(0, 0x40581882);
	FinalizeState(&AsScene2207Lever::stLeverDownEvent);
}

void AsScene2207Lever::stLeverDownEvent() {
	sendMessage(_parentScene, NM_KLAYMEN_LOWER_LEVER, 0);
}

void AsScene2207Lever::stLeverUp() {
	startAnimation(0x80880090, 6, -1);
	_playBackwards = true;
	playSound(0, 0x40581882);
	FinalizeState(&AsScene2207Lever::stLeverUpEvent);
}

void AsScene2207Lever::stLeverUpEvent() {
	sendMessage(_parentScene, NM_KLAYMEN_RAISE_LEVER, 0);
}

AsScene2207WallRobotAnimation::AsScene2207WallRobotAnimation(NeverhoodEngine *vm, Scene *parentScene)
	: AnimatedSprite(vm, 1200), _idle(true) {

	_x = 309;
	_y = 320;
	createSurface1(0xCCFD6090, 100);
	startAnimation(0xCCFD6090, 0, -1);
	_newStickFrameIndex = 0;
	loadSound(1, 0x40330872);
	loadSound(2, 0x72A2914A);
	loadSound(3, 0xD4226080);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2207WallRobotAnimation::handleMessage);
}

AsScene2207WallRobotAnimation::~AsScene2207WallRobotAnimation() {
	_vm->_soundMan->deleteSoundGroup(0x80D00820);
}

uint32 AsScene2207WallRobotAnimation::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (!_idle) {
			if (param.asInteger() == 0x3423093) {
				_vm->_soundMan->addSound(0x80D00820, 0x12121943);
				_vm->_soundMan->playSoundLooping(0x12121943);
			} else if (param.asInteger() == 0x834AB011) {
				stopSound(0);
				stopSound(1);
				stopSound(2);
				stopSound(3);
				_vm->_soundMan->deleteSound(0x12121943);
			} else if (param.asInteger() == 0x3A980501)
				playSound(1);
			else if (param.asInteger() == 0x2A2AD498)
				playSound(2);
			else if (param.asInteger() == 0xC4980008)
				playSound(3);
			else if (param.asInteger() == 0x06B84228)
				playSound(0, 0xE0702146);
		}
		break;
	case NM_KLAYMEN_STOP_CLIMBING:
		stStartAnimation();
		break;
	case NM_CAR_MOVE_TO_PREV_POINT:
		stStopAnimation();
		break;
	case NM_ANIMATION_STOP:
		gotoNextState();
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene2207WallRobotAnimation::stStartAnimation() {
	if (!_idle) {
		NextState(NULL);
	} else {
		startAnimation(0xCCFD6090, 0, -1);
		_idle = false;
		setVisible(true);
	}
}

void AsScene2207WallRobotAnimation::stStopAnimation() {
	NextState(&AsScene2207WallRobotAnimation::cbStopAnimation);
}

void AsScene2207WallRobotAnimation::cbStopAnimation() {
	stopAnimation();
	stopSound(0);
	stopSound(1);
	stopSound(2);
	stopSound(3);
	_vm->_soundMan->deleteSound(0x12121943);
	_idle = true;
	setVisible(false);
}

AsScene2207WallCannonAnimation::AsScene2207WallCannonAnimation(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1200), _idle(true) {

	_x = 309;
	_y = 320;
	createSurface1(0x8CAA0099, 100);
	startAnimation(0x8CAA0099, 0, -1);
	_newStickFrameIndex = 0;
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene2207WallCannonAnimation::handleMessage);
}

uint32 AsScene2207WallCannonAnimation::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_KLAYMEN_STOP_CLIMBING:
		stStartAnimation();
		break;
	case NM_CAR_MOVE_TO_PREV_POINT:
		stStopAnimation();
		break;
	case NM_ANIMATION_STOP:
		gotoNextState();
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene2207WallCannonAnimation::stStartAnimation() {
	if (!_idle) {
		NextState(NULL);
	} else {
		setVisible(true);
		startAnimation(0x8CAA0099, 0, -1);
		_idle = false;
	}
}

void AsScene2207WallCannonAnimation::stStopAnimation() {
	NextState(&AsScene2207WallCannonAnimation::cbStopAnimation);
}

void AsScene2207WallCannonAnimation::cbStopAnimation() {
	stopAnimation();
	setVisible(false);
	_idle = true;
}

SsScene2207Symbol::SsScene2207Symbol(NeverhoodEngine *vm, uint32 fileHash, int index)
	: StaticSprite(vm, fileHash, 100) {

	_x = 330;
	_y = 246 + index * 50;
	updatePosition();
}

KmScene2201::KmScene2201(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y, NRect *clipRects, int clipRectsCount)
	: Klaymen(vm, parentScene, x, y) {

	_surface->setClipRects(clipRects, clipRectsCount);
	_dataResource.load(0x04104242);
}

uint32 KmScene2201::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case NM_KLAYMEN_PICKUP:
		GotoState(&Klaymen::stPickUpGeneric);
		break;
	case NM_KLAYMEN_PRESS_BUTTON:
		if (param.asInteger() == 0)
			GotoState(&Klaymen::stPressButtonSide);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x4818:
		startWalkToX(_dataResource.getPoint(param.asInteger()).x, false);
		break;
	case 0x481B:
		if (param.asPoint().y != 0)
			startWalkToXDistance(param.asPoint().y, param.asPoint().x);
		else
			startWalkToAttachedSpriteXDistance(param.asPoint().x);
		break;
	case NM_KLAYMEN_TURN_TO_USE:
		GotoState(&Klaymen::stTurnToUse);
		break;
	case NM_KLAYMEN_RETURN_FROM_USE:
		GotoState(&Klaymen::stReturnFromUse);
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x482E:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stWalkToFrontNoStep);
		else
			GotoState(&Klaymen::stWalkToFront);
		break;
	case 0x482F:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stTurnToFront);
		else
			GotoState(&Klaymen::stTurnToBack);
		break;
	case 0x483F:
		startSpecialWalkRight(param.asInteger());
		break;
	case 0x4840:
		startSpecialWalkLeft(param.asInteger());
		break;
	default:
		break;
	}
	return 0;
}

KmScene2203::KmScene2203(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

uint32 KmScene2203::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case NM_KLAYMEN_PICKUP:
		if (param.asInteger() == 2)
			GotoState(&Klaymen::stPickUpNeedle);
		else if (param.asInteger() == 1)
			GotoState(&Klaymen::stPickUpTube);
		else
			GotoState(&Klaymen::stPickUpGeneric);
		break;
	case NM_KLAYMEN_PRESS_BUTTON:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stPressButton);
		else if (param.asInteger() == 2)
			GotoState(&Klaymen::stPressFloorButton);
		else
			GotoState(&Klaymen::stPressButtonSide);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x4818:
		startWalkToX(_dataResource.getPoint(param.asInteger()).x, false);
		break;
	case 0x4819:
		GotoState(&KmScene2203::stClayDoorOpen);
		break;
	case NM_KLAYMEN_INSERT_DISK:
		GotoState(&Klaymen::stInsertDisk);
		break;
	case 0x481B:
		if (param.asPoint().y != 0)
			startWalkToXDistance(param.asPoint().y, param.asPoint().x);
		else
			startWalkToAttachedSpriteXDistance(param.asPoint().x);
		break;
	case NM_KLAYMEN_TURN_TO_USE:
		GotoState(&Klaymen::stTurnToUse);
		break;
	case NM_KLAYMEN_RETURN_FROM_USE:
		GotoState(&Klaymen::stReturnFromUse);
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x483F:
		startSpecialWalkRight(param.asInteger());
		break;
	case 0x4840:
		startSpecialWalkLeft(param.asInteger());
		break;
	default:
		break;
	}
	return 0;
}

void KmScene2203::stClayDoorOpen() {
	if (!stStartAction(AnimationCallback(&KmScene2203::stClayDoorOpen))) {
		_busyStatus = 2;
		_acceptInput = false;
		startAnimation(0x5CCCB330, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&KmScene2203::hmClayDoorOpen);
		SetSpriteUpdate(&Klaymen::suUpdateDestX);
	}
}

uint32 KmScene2203::hmClayDoorOpen(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmLowLevelAnimation(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x040D4186) {
			sendMessage(_attachedSprite, NM_KLAYMEN_OPEN_DOOR, 0);
		}
		break;
	default:
		break;
	}
	return messageResult;
}

KmScene2205::KmScene2205(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

void KmScene2205::xUpdate() {
	setGlobalVar(V_KLAYMEN_FRAMEINDEX, _currFrameIndex);
}

uint32 KmScene2205::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_destX = param.asInteger();
			GotoState(&Klaymen::stStartWalkingResume);
		} else
			GotoState(&Klaymen::stPeekWall);
		break;
	case NM_KLAYMEN_PRESS_BUTTON:
		if (param.asInteger() == 0)
			GotoState(&Klaymen::stPressButtonSide);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x4818:
		startWalkToX(_dataResource.getPoint(param.asInteger()).x, false);
		break;
	case 0x483F:
		startSpecialWalkRight(param.asInteger());
		break;
	case 0x4840:
		startSpecialWalkLeft(param.asInteger());
		break;
	default:
		break;
	}
	return 0;
}

KmScene2206::KmScene2206(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	_walkResumeFrameIncr = 1;
	_vm->_soundMan->addSound(0x80101800, 0xD3B02847);
}

KmScene2206::~KmScene2206() {
	_vm->_soundMan->deleteSoundGroup(0x80101800);
}

void KmScene2206::xUpdate() {
	setGlobalVar(V_KLAYMEN_FRAMEINDEX, _currFrameIndex);
}

uint32 KmScene2206::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4803:
		GotoState(&KmScene2206::stRidePlatformDown);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_destX = param.asInteger();
			GotoState(&Klaymen::stStartWalkingResume);
		} else
			GotoState(&Klaymen::stPeekWall);
		break;
	case NM_KLAYMEN_PICKUP:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stPickUpTube);
		else
			GotoState(&Klaymen::stPickUpGeneric);
		break;
	case NM_KLAYMEN_PRESS_BUTTON:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stPressButton);
		else if (param.asInteger() == 2)
			GotoState(&Klaymen::stPressFloorButton);
		else
			GotoState(&Klaymen::stPressButtonSide);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x481B:
		if (param.asPoint().y != 0)
			startWalkToXDistance(param.asPoint().y, param.asPoint().x);
		else
			startWalkToAttachedSpriteXDistance(param.asPoint().x);
		break;
	case 0x481F:
		if (param.asInteger() == 0)
			GotoState(&Klaymen::stWonderAboutHalf);
		else if (param.asInteger() == 1)
			GotoState(&Klaymen::stWonderAboutAfter);
		else if (param.asInteger() == 3)
			GotoState(&Klaymen::stTurnToUseHalf);
		else if (param.asInteger() == 4)
			GotoState(&Klaymen::stTurnAwayFromUse);
		else
			GotoState(&Klaymen::stWonderAbout);
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x482E:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stWalkToFrontNoStep);
		else
			GotoState(&Klaymen::stWalkToFront);
		break;
	case 0x482F:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stTurnToFront);
		else
			GotoState(&Klaymen::stTurnToBack);
		break;
	case 0x4837:
		stopWalking();
		break;
	case 0x483F:
		startSpecialWalkRight(param.asInteger());
		break;
	case 0x4840:
		startSpecialWalkLeft(param.asInteger());
		break;
	default:
		break;
	}
	return 0;
}

void KmScene2206::suRidePlatformDown() {
	_platformDeltaY++;
	_y += _platformDeltaY;
	if (_y > 600)
		sendMessage(this, NM_SCENE_LEAVE, 0);
}

void KmScene2206::stRidePlatformDown() {
	if (!stStartActionFromIdle(AnimationCallback(&KmScene2206::stRidePlatformDown))) {
		_busyStatus = 1;
		sendMessage(_parentScene, 0x4803, 0);
		_acceptInput = false;
		_platformDeltaY = 0;
		startAnimation(0x5420E254, 0, -1);
		SetUpdateHandler(&Klaymen::update);
		SetMessageHandler(&Klaymen::hmLowLevel);
		SetSpriteUpdate(&KmScene2206::suRidePlatformDown);
		_vm->_soundMan->playSoundLooping(0xD3B02847);
	}
}

KmScene2207::KmScene2207(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

uint32 KmScene2207::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x2001:
		GotoState(&Klaymen::stRidePlatform);
		break;
	case NM_KLAYMEN_CLIMB_LADDER:
		suRidePlatform();
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x480D:
		GotoState(&Klaymen::stInteractLever);
		break;
	case NM_KLAYMEN_PICKUP:
		GotoState(&Klaymen::stPickUpGeneric);
		break;
	case NM_KLAYMEN_PRESS_BUTTON:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stPressButton);
		else if (param.asInteger() == 2)
			GotoState(&Klaymen::stPressFloorButton);
		else
			GotoState(&Klaymen::stPressButtonSide);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x481B:
		if (param.asPoint().y != 0)
			startWalkToXDistance(param.asPoint().y, param.asPoint().x);
		else
			startWalkToAttachedSpriteXDistance(param.asPoint().x);
		break;
	case NM_KLAYMEN_RELEASE_LEVER:
		GotoState(&Klaymen::stReleaseLever);
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x483F:
		startSpecialWalkRight(param.asInteger());
		break;
	case 0x4840:
		startSpecialWalkLeft(param.asInteger());
		break;
	default:
		break;
	}
	return 0;
}

KmScene2242::KmScene2242(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

void KmScene2242::xUpdate() {
	setGlobalVar(V_KLAYMEN_FRAMEINDEX, _currFrameIndex);
}

uint32 KmScene2242::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_destX = param.asInteger();
			GotoState(&Klaymen::stStartWalkingResume);
		} else
			GotoState(&Klaymen::stPeekWall);
		break;
	case NM_KLAYMEN_PICKUP:
		if (param.asInteger() == 2)
			GotoState(&Klaymen::stPickUpNeedle);
		else if (param.asInteger() == 1)
			GotoState(&Klaymen::stPickUpTube);
		else
			GotoState(&Klaymen::stPickUpGeneric);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x481B:
		if (param.asPoint().y != 0)
			startWalkToXDistance(param.asPoint().y, param.asPoint().x);
		else
			startWalkToAttachedSpriteXDistance(param.asPoint().x);
		break;
	case 0x481F:
		if (param.asInteger() == 0)
			GotoState(&Klaymen::stWonderAboutHalf);
		else if (param.asInteger() == 1)
			GotoState(&Klaymen::stWonderAboutAfter);
		else if (param.asInteger() == 3)
			GotoState(&Klaymen::stTurnToUseHalf);
		else if (param.asInteger() == 4)
			GotoState(&Klaymen::stTurnAwayFromUse);
		else
			GotoState(&Klaymen::stWonderAbout);
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x4837:
		stopWalking();
		break;
	default:
		break;
	}
	return 0;
}

KmHallOfRecords::KmHallOfRecords(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {
	// Empty
}

void KmHallOfRecords::xUpdate() {
	setGlobalVar(V_KLAYMEN_FRAMEINDEX, _currFrameIndex);
}

uint32 KmHallOfRecords::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_destX = param.asInteger();
			GotoState(&Klaymen::stStartWalkingResume);
		} else
			GotoState(&Klaymen::stPeekWall);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x481F:
		if (param.asInteger() == 0)
			GotoState(&Klaymen::stWonderAboutHalf);
		else if (param.asInteger() == 1)
			GotoState(&Klaymen::stWonderAboutAfter);
		else if (param.asInteger() == 3)
			GotoState(&Klaymen::stTurnToUseHalf);
		else if (param.asInteger() == 4)
			GotoState(&Klaymen::stTurnAwayFromUse);
		else
			GotoState(&Klaymen::stWonderAbout);
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x4837:
		stopWalking();
		break;
	default:
		break;
	}
	return 0;
}

KmScene2247::KmScene2247(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

void KmScene2247::xUpdate() {
	setGlobalVar(V_KLAYMEN_FRAMEINDEX, _currFrameIndex);
}

uint32 KmScene2247::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case 0x4804:
		if (param.asInteger() != 0) {
			_destX = param.asInteger();
			GotoState(&Klaymen::stStartWalkingResume);
		} else
			GotoState(&Klaymen::stPeekWall);
		break;
	case 0x4817:
		setDoDeltaX(param.asInteger());
		gotoNextStateExt();
		break;
	case 0x481F:
		if (param.asInteger() == 0)
			GotoState(&Klaymen::stWonderAboutHalf);
		else if (param.asInteger() == 1)
			GotoState(&Klaymen::stWonderAboutAfter);
		else if (param.asInteger() == 3)
			GotoState(&Klaymen::stTurnToUseHalf);
		else if (param.asInteger() == 4)
			GotoState(&Klaymen::stTurnAwayFromUse);
		else
			GotoState(&Klaymen::stWonderAbout);
		break;
	case 0x482D:
		setDoDeltaX(_x > (int16)param.asInteger() ? 1 : 0);
		gotoNextStateExt();
		break;
	case 0x4837:
		stopWalking();
		break;
	default:
		break;
	}
	return 0;
}

} // End of namespace Neverhood
