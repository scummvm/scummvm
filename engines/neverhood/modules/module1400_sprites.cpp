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

#include "neverhood/modules/module1400_sprites.h"
#include "neverhood/modules/module1400.h"

namespace Neverhood {

AsScene1401Pipe::AsScene1401Pipe(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1100), _countdown1(0), _countdown2(0) {

	createSurface(900, 152, 147);
	_x = 454;
	_y = 217;
	startAnimation(0x4C210500, 0, -1);
	SetUpdateHandler(&AsScene1401Pipe::update);
	SetMessageHandler(&AsScene1401Pipe::handleMessage);
}

AsScene1401Pipe::~AsScene1401Pipe() {
	_vm->_soundMan->deleteSoundGroup(0x01104C08);
}

void AsScene1401Pipe::update() {
	AnimatedSprite::update();
	if (_countdown1 != 0 && (--_countdown1 == 0))
		stDoneSucking();
	if (_countdown2 != 0 && (--_countdown2 == 0)) {
		_vm->_soundMan->addSound(0x01104C08, 0x4A116437);
		_vm->_soundMan->playSoundLooping(0x4A116437);
	}
}

void AsScene1401Pipe::upSuckInProjector() {
	AnimatedSprite::update();
	if (_countdown1 != 0)
		_countdown1--;
}

uint32 AsScene1401Pipe::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x0A8A1490)
			playSound(1, 0x6AB6666F);
		break;
	case NM_ANIMATION_UPDATE:
		_countdown1 = 70;
		_countdown2 = 8;
		stStartSucking();
		break;
	case 0x483A:
		stSuckInProjector();
		break;
	default:
		break;
	}
	return messageResult;
}

uint32 AsScene1401Pipe::hmSuckInProjector(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_STOP:
		if (_countdown1 != 0)
			stStartSucking();
		else
			stDoneSucking();
		SetMessageHandler(&AsScene1401Pipe::handleMessage);
		SetUpdateHandler(&AsScene1401Pipe::update);
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene1401Pipe::stStartSucking() {
	startAnimation(0x4C240100, 0, -1);
	playSound(0, 0x4A30063F);
}

void AsScene1401Pipe::stDoneSucking() {
	_vm->_soundMan->deleteSound(0x4A116437);
	playSound(0, 0x4A120435);
	startAnimation(0x4C210500, 0, -1);
}

void AsScene1401Pipe::stSuckInProjector() {
	startAnimation(0x6C210810, 0, -1);
	SetUpdateHandler(&AsScene1401Pipe::upSuckInProjector);
	SetMessageHandler(&AsScene1401Pipe::hmSuckInProjector);
}

AsScene1401Mouse::AsScene1401Mouse(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1100) {

	createSurface(100, 71, 41);
	_x = 478;
	_y = 433;
	startAnimation(0xA282C472, 0, -1);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1401Mouse::handleMessage);
}

uint32 AsScene1401Mouse::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x66382026)
			playSound(0, 0x0CD84468);
		else if (param.asInteger() == 0x6E28061C)
			playSound(0, 0x78C8402C);
		else if (param.asInteger() == 0x462F0410)
			playSound(0, 0x60984E28);
		break;
	case 0x4839:
		stSuckedIn();
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene1401Mouse::suSuckedIn() {
	AnimatedSprite::updateDeltaXY();
	if (_collisionBounds.y1 <= 150) {
		playSound(0, 0x0E32247F);
		stopAnimation();
		setVisible(false);
		SetMessageHandler(NULL);
		SetSpriteUpdate(NULL);
	}
}

void AsScene1401Mouse::stSuckedIn() {
	startAnimation(0x34880040, 0, -1);
	SetSpriteUpdate(&AsScene1401Mouse::suSuckedIn);
}

AsScene1401Cheese::AsScene1401Cheese(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1100) {

	createSurface(200, 152, 147);
	_x = 427;
	_y = 433;
	startAnimation(0x461A1490, 0, -1);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1401Cheese::handleMessage);
}

uint32 AsScene1401Cheese::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x4839:
		stSuckedIn();
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene1401Cheese::suSuckedIn() {
	AnimatedSprite::updateDeltaXY();
	if (_collisionBounds.y1 <= 150) {
		playSound(0, 0x18020439);
		stopAnimation();
		setVisible(false);
		SetMessageHandler(NULL);
		SetSpriteUpdate(NULL);
	}
}

void AsScene1401Cheese::stSuckedIn() {
	startAnimation(0x103B8020, 0, -1);
	SetSpriteUpdate(&AsScene1401Cheese::suSuckedIn);
}

AsScene1401BackDoor::AsScene1401BackDoor(NeverhoodEngine *vm, Sprite *klaymen, bool isOpen)
	: AnimatedSprite(vm, 1100), _klaymen(klaymen), _countdown(0), _isOpen(isOpen) {

	_x = 320;
	_y = 240;
	createSurface1(0x04551900, 100);
	if (isOpen) {
		startAnimation(0x04551900, -1, -1);
		_countdown = 48;
	} else {
		stopAnimation();
		setVisible(false);
	}
	_newStickFrameIndex = STICK_LAST_FRAME;
	SetUpdateHandler(&AsScene1401BackDoor::update);
	SetMessageHandler(&AsScene1401BackDoor::handleMessage);
}

void AsScene1401BackDoor::update() {
	if (_countdown != 0 && (--_countdown == 0))
		stCloseDoor();
	AnimatedSprite::update();
}


uint32 AsScene1401BackDoor::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2001:
		if (_isOpen)
			_countdown = 168;
		messageResult = _isOpen ? 1 : 0;
		break;
	case NM_ANIMATION_STOP:
		gotoNextState();
		break;
	case NM_KLAYMEN_OPEN_DOOR:
		_countdown = 168;
		if (!_isOpen)
			stOpenDoor();
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene1401BackDoor::stOpenDoor() {
	_isOpen = true;
	setVisible(true);
	startAnimation(0x04551900, 0, -1);
	_newStickFrameIndex = STICK_LAST_FRAME;
	playSound(0, calcHash("fxDoorOpen24"));
}

void AsScene1401BackDoor::stCloseDoor() {
	_isOpen = false;
	setVisible(true);
	startAnimation(0x04551900, -1, -1);
	playSound(0, calcHash("fxDoorClose24"));
	_playBackwards = true;
	NextState(&AsScene1401BackDoor::stCloseDoorDone);
}

void AsScene1401BackDoor::stCloseDoorDone() {
	stopAnimation();
	setVisible(false);
}

static const AsCommonProjectorItem kAsCommonProjectorItems[] = {
	{{154, 453}, 4,  2,  0, 0, 1},
	{{104, 391}, 4, -1, -1, 1, 1},
	{{ 22, 447}, 6, -1, -1, 1, 1},
	{{112, 406}, 2, -1, -1, 1, 0},
	{{262, 433}, 1,  1,  0, 0, 0}
};

AsCommonProjector::AsCommonProjector(NeverhoodEngine *vm, Scene *parentScene, Sprite *klaymen, Sprite *asPipe)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _klaymen(klaymen), _asPipe(asPipe) {

	_asProjectorItem = &kAsCommonProjectorItems[getGlobalVar(V_PROJECTOR_LOCATION)];
	createSurface(990, 101, 182);
	startAnimation(0x10E3042B, 0, -1);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsCommonProjector::handleMessage);
	_x = getGlobalVar(V_PROJECTOR_SLOT) * 108 + _asProjectorItem->point.x;
	_lockedInSlot = true;
	moveProjector();
	setDoDeltaX(1);
	if ((int8)getGlobalVar(V_PROJECTOR_SLOT) == _asProjectorItem->lockSlotIndex)
		stStayLockedInSlot();
	loadSound(2, 0xC8C2507C);
}

AsCommonProjector::~AsCommonProjector() {
	_vm->_soundMan->deleteSoundGroup(0x05331081);
}

uint32 AsCommonProjector::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		sendMessage(_parentScene, 0x4826, 0);
		messageResult = 1;
		break;
	case NM_KLAYMEN_RAISE_LEVER:
		setGlobalVar(V_PROJECTOR_SLOT, (_x - _asProjectorItem->point.x) / 108);
		if ((int8)getGlobalVar(V_PROJECTOR_SLOT) == _asProjectorItem->lockSlotIndex)
			stStartLockedInSlot();
		else
			stIdle();
		break;
	case 0x480B:
		if (param.asInteger() != 1) {
			if ((int8)getGlobalVar(V_PROJECTOR_SLOT) < _asProjectorItem->maxSlotCount)
				incGlobalVar(V_PROJECTOR_SLOT, 1);
		} else if (getGlobalVar(V_PROJECTOR_SLOT) > 0)
			incGlobalVar(V_PROJECTOR_SLOT, -1);
		stMoving();
		break;
	case 0x480C:
		// Check if the projector can be moved
		if (param.asInteger() != 1)
			messageResult = (int8)getGlobalVar(V_PROJECTOR_SLOT) < _asProjectorItem->maxSlotCount ? 1 : 0;
		else
			messageResult = getGlobalVar(V_PROJECTOR_SLOT) > 0 ? 1 : 0;
		break;
	case NM_MOVE_TO_BACK:
		sendMessage(_parentScene, NM_PRIORITY_CHANGE, 990);
		break;
	case NM_MOVE_TO_FRONT:
		sendMessage(_parentScene, NM_PRIORITY_CHANGE, 1010);
		break;
	case 0x4839:
		stStartSuckedIn();
		break;
	default:
		break;
	}
	return messageResult;
}

uint32 AsCommonProjector::hmLockedInSlot(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (param.asPoint().x - _x >= 17 && param.asPoint().x - _x <= 56 &&
			param.asPoint().y - _y >= -120 && param.asPoint().y - _y <= -82) {
			sendMessage(_parentScene, 0x4826, 1);
		} else
			sendMessage(_parentScene, 0x4826, 0);
		messageResult = 1;
		break;
	case NM_KLAYMEN_RAISE_LEVER:
		sendMessage(_parentScene, NM_KLAYMEN_RAISE_LEVER, 0);
		stStopProjecting();
		break;
	case 0x480B:
		if (param.asInteger() != 1) {
			if ((int8)getGlobalVar(V_PROJECTOR_SLOT) < _asProjectorItem->maxSlotCount)
				incGlobalVar(V_PROJECTOR_SLOT, 1);
		} else if (getGlobalVar(V_PROJECTOR_SLOT) > 0)
			incGlobalVar(V_PROJECTOR_SLOT, -1);
		stTurnToFront();
		break;
	case 0x480C:
		// Check if the projector can be moved
		if (param.asInteger() != 1)
			messageResult = (int8)getGlobalVar(V_PROJECTOR_SLOT) < _asProjectorItem->maxSlotCount ? 1 : 0;
		else
			messageResult = getGlobalVar(V_PROJECTOR_SLOT) > 0 ? 1 : 0;
		break;
	case NM_KLAYMEN_LOWER_LEVER:
		stStartProjecting();
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

uint32 AsCommonProjector::hmAnimation(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_STOP:
		gotoNextState();
		break;
	default:
		break;
	}
	return messageResult;
}

void AsCommonProjector::suMoving() {
	if (_x <= _klaymen->getX())
		_x = _klaymen->getX() - 100;
	else
		_x = _klaymen->getX() + 100;
	moveProjector();
	if (_beforeMoveX == _x) {
		if (getGlobalVar(V_PROJECTOR_SLOT) == 0 && _asProjectorItem->leftBorderLeaves != 0) {
			sendMessage(_parentScene, NM_SCENE_LEAVE, 0);
			incGlobalVar(V_PROJECTOR_LOCATION, -1);
			setGlobalVar(V_PROJECTOR_SLOT, kAsCommonProjectorItems[getGlobalVar(V_PROJECTOR_LOCATION)].maxSlotCount);
		} else if ((int8)getGlobalVar(V_PROJECTOR_SLOT) == _asProjectorItem->maxSlotCount && _asProjectorItem->rightBorderLeaves != 0) {
			sendMessage(_parentScene, NM_SCENE_LEAVE, 1);
			incGlobalVar(V_PROJECTOR_LOCATION, +1);
			setGlobalVar(V_PROJECTOR_SLOT, 0);
		}
	}
	Sprite::updateBounds();
}

void AsCommonProjector::moveProjector() {

	bool nowLockedInSlot = false;

	_y = _asProjectorItem->point.y;

	if (_asProjectorItem->index1 != -1) {
		int16 elX = _asProjectorItem->index1 * 108 + _asProjectorItem->point.x;
		if (elX - 20 < _x && elX + 20 > _x) {
			nowLockedInSlot = true;
			_y = _asProjectorItem->point.y + 10;
		}
	}

	if (_asProjectorItem->lockSlotIndex != -1) {
		int16 elX = _asProjectorItem->lockSlotIndex * 108 + _asProjectorItem->point.x;
		if (elX - 20 < _x && elX + 20 > _x) {
			nowLockedInSlot = true;
			_y = _asProjectorItem->point.y + 10;
		}
	}

	if (_lockedInSlot && !nowLockedInSlot)
		_lockedInSlot = false;
	else if (!_lockedInSlot && nowLockedInSlot) {
		playSound(1, 0x5440E474);
		_lockedInSlot = true;
	}

}

void AsCommonProjector::stSuckedIn() {
	AnimatedSprite::updateDeltaXY();
	if (_collisionBounds.y1 <= 150) {
		sendMessage(_asPipe, 0x483A, 0);
		stopAnimation();
		setVisible(false);
		SetMessageHandler(&Sprite::handleMessage);
		SetSpriteUpdate(NULL);
	}
}

void AsCommonProjector::stIdle() {
	startAnimation(0x10E3042B, 0, -1);
	SetMessageHandler(&AsCommonProjector::handleMessage);
	SetSpriteUpdate(NULL);
}

void AsCommonProjector::stMoving() {
	_beforeMoveX = getGlobalVar(V_PROJECTOR_SLOT) * 108 + _asProjectorItem->point.x;
	startAnimation(0x14A10137, 0, -1);
	playSound(1, 0xEC008474);
	SetMessageHandler(&AsCommonProjector::handleMessage);
	SetSpriteUpdate(&AsCommonProjector::suMoving);
}

void AsCommonProjector::stStartLockedInSlot() {
	startAnimation(0x80C32213, 0, -1);
	SetMessageHandler(&AsCommonProjector::hmAnimation);
	SetSpriteUpdate(NULL);
	NextState(&AsCommonProjector::stStayLockedInSlot);
}

void AsCommonProjector::stStayLockedInSlot() {
	startAnimation(0xD23B207F, 0, -1);
	SetMessageHandler(&AsCommonProjector::hmLockedInSlot);
	SetSpriteUpdate(NULL);
}

void AsCommonProjector::stStartProjecting() {
	startAnimation(0x50A80517, 0, -1);
	setGlobalVar(V_PROJECTOR_ACTIVE, 1);
	playSound(0, 0xCC4A8456);
	_vm->_soundMan->addSound(0x05331081, 0xCE428854);
	_vm->_soundMan->playSoundLooping(0xCE428854);
	SetMessageHandler(&AsCommonProjector::hmAnimation);
	SetSpriteUpdate(NULL);
	NextState(&AsCommonProjector::stLockedInSlot);
}

void AsCommonProjector::stLockedInSlot() {
	sendMessage(_parentScene, NM_KLAYMEN_LOWER_LEVER, 0);
	startAnimation(0xD833207F, 0, -1);
	SetMessageHandler(&AsCommonProjector::hmLockedInSlot);
	SetSpriteUpdate(NULL);
}

void AsCommonProjector::stStopProjecting() {
	startAnimation(0x50A94417, 0, -1);
	setGlobalVar(V_PROJECTOR_ACTIVE, 0);
	playSound(0, 0xCC4A8456);
	_vm->_soundMan->deleteSound(0xCE428854);
	SetMessageHandler(&AsCommonProjector::hmAnimation);
	SetSpriteUpdate(NULL);
	NextState(&AsCommonProjector::stStayLockedInSlot);
}

void AsCommonProjector::stTurnToFront() {
	_beforeMoveX = getGlobalVar(V_PROJECTOR_SLOT) * 108 + _asProjectorItem->point.x;
	startAnimation(0x22CB4A33, 0, -1);
	SetMessageHandler(&AsCommonProjector::hmAnimation);
	SetSpriteUpdate(&AsCommonProjector::suMoving);
	NextState(&AsCommonProjector::stMoving);
}

void AsCommonProjector::stStartSuckedIn() {
	setGlobalVar(V_PROJECTOR_LOCATION, 4);
	setGlobalVar(V_PROJECTOR_SLOT, 0);
	startAnimation(0x708D4712, 0, -1);
	playSound(2);
	SetMessageHandler(&Sprite::handleMessage);
	SetSpriteUpdate(&AsCommonProjector::stSuckedIn);
}

SsScene1402BridgePart::SsScene1402BridgePart(NeverhoodEngine *vm, uint32 fileHash, int surfacePriority)
	: StaticSprite(vm, fileHash, surfacePriority) {

	SetFilterY(&Sprite::defFilterY);
	SetUpdateHandler(&StaticSprite::updatePosition);
}

AsScene1402PuzzleBox::AsScene1402PuzzleBox(NeverhoodEngine *vm, Scene *parentScene, int status)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene) {

	createSurface(900, 347, 230);

	SetFilterY(&Sprite::defFilterY);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1402PuzzleBox::handleMessage);
	_x = 279;
	_y = 270;
	if (status == 2) {
		// Puzzle box after the puzzle was solved
		startAnimation(0x20060259, 0, -1);
		playSound(0, 0x419014AC);
		loadSound(1, 0x61901C29);
		NextState(&AsScene1402PuzzleBox::stMoveDownSolvedDone);
	} else if (status == 1) {
		// Puzzle box appears
		startAnimation(0x210A0213, 0, -1);
		playSound(0, 0x41809C6C);
		NextState(&AsScene1402PuzzleBox::stMoveUpDone);
	} else {
		// Puzzle box is here
		startAnimation(0x20060259, -1, -1);
		loadSound(1, 0x61901C29);
		_newStickFrameIndex = STICK_LAST_FRAME;
	}
}

uint32 AsScene1402PuzzleBox::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_POSITION_CHANGE:
		playSound(1);
		startAnimation(0x20060259, -1, -1);
		_playBackwards = true;
		NextState(&AsScene1402PuzzleBox::stMoveDownDone);
		break;
	case NM_ANIMATION_STOP:
		gotoNextState();
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene1402PuzzleBox::stMoveUpDone() {
	sendMessage(_parentScene, 0x2000, 0);
	stopAnimation();
	setVisible(false);
}

void AsScene1402PuzzleBox::stMoveDownDone() {
	sendMessage(_parentScene, 0x2001, 0);
	stopAnimation();
	setVisible(false);
}

void AsScene1402PuzzleBox::stMoveDownSolvedDone() {
	sendMessage(_parentScene, 0x2003, 0);
	stopAnimation();
}

static const int16 kScene1407MouseFloorY[] = {
	106, 150, 191, 230, 267, 308, 350, 395
};

static const struct {
	int16 x;
	int16 floorIndex;
	int16 sectionIndex;
	int16 nextHoleIndex;
} kScene1407MouseHoles[] = {
	{125, 0,  0,  7},
	{452, 7, 21,  0},
	{337, 4, 11,  4},
	{286, 6, 17,  6},
	{348, 6, 17, 39},
	{536, 6, 18, 42},
	{111, 1,  3, 18},
	{203, 1,  3, 38},
	{270, 1,  3,  9},
	{197, 5, 14,  3},
	{252, 5, 14, 35},
	{297, 5, 14,  7},
	{359, 5, 14,  8},
	{422, 4, 12, 26},
	{467, 4, 12,  2},
	{539, 4, 12, 40},
	{111, 5, 13, 17},
	{211, 0,  1, 20},
	{258, 0,  1, 11},
	{322, 0,  1, 16},
	{ 99, 6, 16, 31},
	{142, 6, 16, 27},
	{194, 6, 16, 12},
	{205, 2,  6, 45},
	{264, 2,  6, 10},
	{ 98, 4, 10,  2},
	{152, 4, 10, 37},
	{199, 4, 10, 13},
	{258, 4, 10, 16},
	{100, 7, 19, 43},
	{168, 7, 19, 23},
	{123, 3,  8, 14},
	{181, 3,  8, 39},
	{230, 3,  8, 28},
	{292, 3,  8, 22},
	{358, 3,  8, 36},
	{505, 3,  9, 44},
	{400, 2,  7, 34},
	{454, 2,  7, 32},
	{532, 2,  7, 46},
	{484, 5, 15, 25},
	{529, 5, 15, 30},
	{251, 7, 20, 48},
	{303, 7, 20, 21},
	{360, 7, 20, 33},
	{503, 0,  2,  5},
	{459, 1,  4, 19},
	{530, 1,  4, 42},
	{111, 2,  5, 47},
	{442, 6, 18,  1}
};

static const struct {
	int16 x1, x2;
	int16 goodHoleIndex;
} kScene1407MouseSections[] = {
	{100, 149,  0},
	{182, 351, 17},
	{430, 524, 45},
	{ 89, 293,  7},
	{407, 555, 47},
	{ 89, 132, 48},
	{178, 303, 23},
	{367, 551, 38},
	{105, 398, 31},
	{480, 537, 36},
	{ 84, 275, 27},
	{318, 359,  2},
	{402, 560, 15},
	{ 91, 132, 16},
	{179, 400, 10},
	{461, 552, 41},
	{ 86, 218, 21},
	{267, 376,  4},
	{420, 560, 49},
	{ 77, 188, 30},
	{237, 394, 44},
	{438, 515,  5}
};

AsScene1407Mouse::AsScene1407Mouse(NeverhoodEngine *vm, Scene *parentScene)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _currSectionIndex(0) {

	createSurface(100, 117, 45);
	_x = 108;
	_y = 106;
	stIdleLookAtGoodHole();
	SetUpdateHandler(&AnimatedSprite::update);
}

void AsScene1407Mouse::suWalkTo() {
	int16 xdelta = _walkDestX - _x;
	if (xdelta > _deltaX)
		xdelta = _deltaX;
	else if (xdelta < -_deltaX)
		xdelta = -_deltaX;
	_deltaX = 0;
	if (_walkDestX == _x)
		sendMessage(this, NM_SCENE_LEAVE, 0);
	else {
		_x += xdelta;
		updateBounds();
	}
}

void AsScene1407Mouse::upGoThroughHole() {
	if (_countdown != 0 && (--_countdown == 0)) {
		SetUpdateHandler(&AnimatedSprite::update);
		gotoNextState();
	}
	AnimatedSprite::update();
}

uint32 AsScene1407Mouse::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_MOUSE_CLICK:
		{
			int16 mouseX = param.asPoint().x;
			int16 mouseY = param.asPoint().y;
			int holeIndex;
			for (holeIndex = 0; holeIndex < 50; holeIndex++) {
				int16 holeX = kScene1407MouseHoles[holeIndex].x;
				int16 holeY = kScene1407MouseFloorY[kScene1407MouseHoles[holeIndex].floorIndex];
				if (mouseX >= holeX - 14 && mouseX <= holeX + 14 && mouseY >= holeY - 36 && mouseY <= holeY)
					break;
			}
			if (holeIndex < 50 && kScene1407MouseHoles[holeIndex].sectionIndex == _currSectionIndex) {
				_nextHoleIndex = kScene1407MouseHoles[holeIndex].nextHoleIndex;
				_walkDestX = kScene1407MouseHoles[holeIndex].x;
				stWalkToHole();
			} else {
				if (mouseX < kScene1407MouseSections[_currSectionIndex].x1)
					_walkDestX = kScene1407MouseSections[_currSectionIndex].x1;
				else if (mouseX > kScene1407MouseSections[_currSectionIndex].x2)
					_walkDestX = kScene1407MouseSections[_currSectionIndex].x2;
				else
					_walkDestX = mouseX;
				stWalkToDest();
			}
		}
		break;
	case NM_SCENE_LEAVE:
		gotoNextState();
		break;
	case 0x2001:
		{
			// Reset the position
			// Find the nearest hole and go through it, and exit at the first hole
			int16 distance = 640;
			int matchIndex = 50;
			for (int index = 0; index < 50; index++)
				if (kScene1407MouseHoles[index].sectionIndex == _currSectionIndex &&
					ABS(kScene1407MouseHoles[index].x - _x) < distance) {
					matchIndex = index;
					distance = ABS(kScene1407MouseHoles[index].x - _x);
				}
			if (matchIndex < 50) {
				_nextHoleIndex = 0;
				_walkDestX = kScene1407MouseHoles[matchIndex].x;
				stWalkToHole();
			}
		}
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene1407Mouse::stIdleLookAtGoodHole() {
	setDoDeltaX(kScene1407MouseHoles[kScene1407MouseSections[_currSectionIndex].goodHoleIndex].x < _x ? 1 : 0);
	startAnimation(0x72215194, 0, -1);
	SetMessageHandler(&AsScene1407Mouse::handleMessage);
	SetSpriteUpdate(NULL);
}

void AsScene1407Mouse::stWalkToDest() {
	if (_walkDestX != _x) {
		setDoDeltaX(_walkDestX < _x ? 1 : 0);
		startAnimation(0x22291510, 0, -1);
		SetMessageHandler(&AsScene1407Mouse::handleMessage);
		SetSpriteUpdate(&AsScene1407Mouse::suWalkTo);
		NextState(&AsScene1407Mouse::stIdleLookAtGoodHole);
	}
}

void AsScene1407Mouse::stWalkToHole() {
	setDoDeltaX(_walkDestX < _x ? 1 : 0);
	startAnimation(0x22291510, 0, -1);
	SetMessageHandler(&AsScene1407Mouse::handleMessage);
	SetSpriteUpdate(&AsScene1407Mouse::suWalkTo);
	NextState(&AsScene1407Mouse::stGoThroughHole);
}

void AsScene1407Mouse::stGoThroughHole() {
	startAnimation(0x72215194, 0, -1);
	setVisible(false);
	_countdown = 12;
	SetUpdateHandler(&AsScene1407Mouse::upGoThroughHole);
	SetMessageHandler(NULL);
	SetSpriteUpdate(NULL);
	NextState(&AsScene1407Mouse::stArriveAtHole);
}

void AsScene1407Mouse::stArriveAtHole() {
	_currSectionIndex = kScene1407MouseHoles[_nextHoleIndex].sectionIndex;
	_x = kScene1407MouseHoles[_nextHoleIndex].x;
	_y = kScene1407MouseFloorY[kScene1407MouseHoles[_nextHoleIndex].floorIndex];
	if (_nextHoleIndex == 1) {
		sendMessage(_parentScene, 0x2000, 0);
		_walkDestX = 512;
		stWalkToDest();
		setVisible(true);
	} else {
		_walkDestX = _x + 14;
		stWalkToDest();
		setVisible(true);
	}
}

static const NPoint kAsScene1405TileItemPositions[] = {
	{100,  80}, {162,  78}, {222,  76}, {292,  76},
	{356,  82}, {422,  84}, {488,  86}, {550,  90},
	{102, 134}, {164, 132}, {224, 136},	{294, 136},
	{360, 136},	{422, 138},	{484, 144},	{548, 146},
	{ 98, 196},	{160, 200},	{228, 200},	{294, 202},
	{360, 198},	{424, 200},	{482, 202},	{548, 206},
	{ 98, 260},	{160, 264},	{226, 260},	{296, 262},
	{358, 260},	{424, 262},	{486, 264},	{550, 266},
	{ 94, 322},	{160, 316},	{226, 316},	{296, 320},
	{358, 322},	{422, 324},	{488, 322},	{550, 322},
	{ 98, 380},	{160, 376},	{226, 376},	{294, 378},
	{356, 380},	{420, 380},	{490, 378},	{552, 376}
};

AsScene1405Tile::AsScene1405Tile(NeverhoodEngine *vm, Scene1405 *parentScene, uint32 tileIndex)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _tileIndex(tileIndex), _countdown(0), _isShowing(false) {

	loadSound(0, 0x05308101);
	setSoundPan(0, (tileIndex % 8 * 4 + 4) * 25 / 8);
	_x = kAsScene1405TileItemPositions[_tileIndex].x;
	_y = kAsScene1405TileItemPositions[_tileIndex].y;
	createSurface1(0x844B805C, 1100);
	setVisible(false);
	if (getSubVar(VA_IS_TILE_MATCH, _tileIndex))
		_countdown = _vm->_rnd->getRandomNumber(36 - 1) + 1;
	startAnimation(0x844B805C, getSubVar(VA_TILE_SYMBOLS, _tileIndex), -1);
	_newStickFrameIndex = (int16)getSubVar(VA_TILE_SYMBOLS, _tileIndex);
	SetUpdateHandler(&AsScene1405Tile::update);
	SetMessageHandler(&AsScene1405Tile::handleMessage);
}

void AsScene1405Tile::update() {
	updateAnim();
	updatePosition();
	if (_countdown != 0 && (--_countdown == 0))
		show();
}

uint32 AsScene1405Tile::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (getSubVar(VA_IS_TILE_MATCH, _tileIndex) == 0 && _parentScene->getCountdown() == 0) {
			show();
			sendMessage(_parentScene, 0x2000, _tileIndex);
		}
		messageResult = 1;
		break;
	default:
		break;
	}
	return messageResult;
}

void AsScene1405Tile::show() {
	if (!_isShowing) {
		_isShowing = true;
		playSound(0);
		setVisible(true);
	}
}

void AsScene1405Tile::hide(bool playClickSound) {
	if (_isShowing) {
		_isShowing = false;
		if (playClickSound)
			playSound(0);
		setVisible(false);
	}
}

KmScene1401::KmScene1401(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

uint32 KmScene1401::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case NM_KLAYMEN_MOVE_OBJECT:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stMoveObjectSkipTurnFaceObject);
		else
			GotoState(&Klaymen::stMoveObjectFaceObject);
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
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stTurnAwayFromUse);
		else if (param.asInteger() == 0)
			GotoState(&Klaymen::stTurnToUseHalf);
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
	default:
		break;
	}
	return 0;
}

KmScene1402::KmScene1402(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	SetFilterY(&Sprite::defFilterY);
}

uint32 KmScene1402::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case NM_KLAYMEN_MOVE_OBJECT:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stMoveObjectSkipTurnFaceObject);
		else
			GotoState(&Klaymen::stMoveObjectFaceObject);
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
	case NM_KLAYMEN_TURN_TO_USE:
		GotoState(&Klaymen::stTurnToUse);
		break;
	case NM_KLAYMEN_RETURN_FROM_USE:
		GotoState(&Klaymen::stReturnFromUse);
		break;
	default:
		break;
	}
	return 0;
}

static const KlaymenIdleTableItem klaymenIdleTable1403[] = {
	{1, kIdleSpinHead},
	{1, kIdleChest},
	{1, kIdleHeadOff},
};

KmScene1403::KmScene1403(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	setKlaymenIdleTable(klaymenIdleTable1403, ARRAYSIZE(klaymenIdleTable1403));
}

uint32 KmScene1403::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case NM_KLAYMEN_MOVE_OBJECT:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stMoveObjectSkipTurnFaceObject);
		else
			GotoState(&Klaymen::stMoveObjectFaceObject);
		break;
	case 0x480D:
		GotoState(&Klaymen::stUseLever);
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
	case NM_KLAYMEN_RELEASE_LEVER:
		GotoState(&Klaymen::stReleaseLever);
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

// KmScene1404

KmScene1404::KmScene1404(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y)
	: Klaymen(vm, parentScene, x, y) {

	// Empty
}

uint32 KmScene1404::xHandleMessage(int messageNum, const MessageParam &param) {
	switch (messageNum) {
	case 0x4001:
	case 0x4800:
		startWalkToX(param.asPoint().x, false);
		break;
	case NM_KLAYMEN_STAND_IDLE:
		GotoState(&Klaymen::stTryStandIdle);
		break;
	case NM_KLAYMEN_MOVE_OBJECT:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stMoveObjectSkipTurnFaceObject);
		else
			GotoState(&Klaymen::stMoveObjectFaceObject);
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
	case 0x481F:
		if (param.asInteger() == 1)
			GotoState(&Klaymen::stWonderAboutAfter);
		else if (param.asInteger() == 0)
			GotoState(&Klaymen::stWonderAboutHalf);
		else if (param.asInteger() == 4)
			GotoState(&Klaymen::stTurnAwayFromUse);
		else if (param.asInteger() == 3)
			GotoState(&Klaymen::stTurnToUseHalf);
		else
			GotoState(&Klaymen::stWonderAbout);
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

} // End of namespace Neverhood
