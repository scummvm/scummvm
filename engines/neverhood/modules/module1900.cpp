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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "neverhood/modules/module1900.h"
#include "neverhood/gamemodule.h"

namespace Neverhood {

static const uint32 kModule1900SoundList[] = {
	0xB4005E60,
	0x91835066,
	0x90E14440,
	0
};

Module1900::Module1900(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {

	// NOTE: The original has a Scene1908 here as well but it's not used here but in another module...

	if (which < 0)
		createScene(_vm->gameState().sceneNum, -1);
	else
		createScene(0, 0);

	_vm->_soundMan->addSoundList(0x04E1C09C, kModule1900SoundList);
	_vm->_soundMan->setSoundListParams(kModule1900SoundList, true, 50, 600, 5, 150);

}

Module1900::~Module1900() {
	_vm->_soundMan->deleteGroup(0x04E1C09C);
}

void Module1900::createScene(int sceneNum, int which) {
	debug(1, "Module1900::createScene(%d, %d)", sceneNum, which);
	_sceneNum = sceneNum;
	switch (_sceneNum) {
	case 0:
		_vm->gameState().sceneNum = 0;
		_childObject = new Scene1901(_vm, this, which);
		break;
	case 6:
		_vm->gameState().sceneNum = 6;
		_childObject = new Scene1907(_vm, this);
		break;
	}
	SetUpdateHandler(&Module1900::updateScene);
	_childObject->handleUpdate();
}

void Module1900::updateScene() {
	if (!updateChild()) {
		switch (_sceneNum) {
		case 0:
			if (_moduleResult == 1)
				createScene(6, 0);
			else
				leaveModule(0);
			break;
		case 6:
			createScene(0, 1);
			break;
		}
	}
}

// Scene1901

Scene1901::Scene1901(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule) {

	Sprite *tempSprite;

	setRectList(0x004B34C8);

	setBackground(0x01303227);
	setPalette(0x01303227);
	insertScreenMouse(0x0322301B);

	insertStaticSprite(0x42213133, 1100);

	if (!getGlobalVar(V_STAIRS_PUZZLE_SOLVED))
		insertStaticSprite(0x40A40168, 100);
	else if (getGlobalVar(V_STAIRS_DOWN)) {
		insertStaticSprite(0x124404C4, 100);
		setGlobalVar(V_STAIRS_DOWN_ONCE, 1);
	} else
		insertStaticSprite(0x02840064, 100);

	if (which < 0) {
		// Restoring game
		insertKlaymen<KmScene1901>(120, 380);
		setMessageList(0x004B3408);
	} else if (which == 1) {
		// Klaymen returning from the puzzle
		insertKlaymen<KmScene1901>(372, 380);
		setMessageList(0x004B3410);
	} else {
		// Klaymen entering from the left
		insertKlaymen<KmScene1901>(0, 380);
		setMessageList(0x004B3400);
	}

	tempSprite = insertStaticSprite(0x4830A402, 1100);
	_klaymen->setClipRect(tempSprite->getDrawRect().x, 0, 640, 480);

}

static const NPoint kAsScene1907SymbolGroundPositions[] = {
	{160, 310}, { 90, 340}, {210, 335},
	{210, 380}, {310, 340}, {290, 400},
	{400, 375}, {370, 435}, {475, 415}
};

static const NPoint kAsScene1907SymbolPluggedInPositions[] = {
	{275, 125}, {244, 125}, {238, 131},
	{221, 135}, {199, 136}, {168, 149},
	{145, 152}, {123, 154}, {103, 157}
};

static const NPoint kAsScene1907SymbolGroundHitPositions[] = {
	{275, 299}, {244, 299}, {238, 305},
	{221, 309}, {199, 310}, {168, 323},
	{145, 326}, {123, 328}, {103, 331}
};

static const NPoint kAsScene1907SymbolPluggedInDownPositions[] = {
	{275, 136}, {244, 156}, {238, 183},
	{221, 207}, {199, 228}, {168, 262},
	{145, 285}, {123, 307}, {103, 331}
};

static const uint32 kAsScene1907SymbolFileHashes[] = {
	0x006A1034, 0x006A1010, 0x006A1814,
	0x006A1016, 0x006A0014, 0x002A1014,
	0x00EA1014, 0x206A1014, 0x046A1414
};

bool AsScene1907Symbol::_plugInFailed = false;
int AsScene1907Symbol::_plugInTryCount = 0;

AsScene1907Symbol::AsScene1907Symbol(NeverhoodEngine *vm, Scene1907 *parentScene, int elementIndex, int positionIndex)
	: AnimatedSprite(vm, 1000 - positionIndex), _parentScene(parentScene), _elementIndex(elementIndex), _isMoving(false) {

	_plugInFailed = false;
	_plugInTryCount = 0;

	if (getGlobalVar(V_STAIRS_PUZZLE_SOLVED)) {
		_isPluggedIn = true;
		_currPositionIndex = elementIndex;
		if (!getGlobalVar(V_STAIRS_DOWN)) {
			_x = kAsScene1907SymbolPluggedInPositions[_currPositionIndex].x;
			_y = kAsScene1907SymbolPluggedInPositions[_currPositionIndex].y;
		} else {
			_x = kAsScene1907SymbolPluggedInDownPositions[_currPositionIndex].x;
			_y = kAsScene1907SymbolPluggedInDownPositions[_currPositionIndex].y;
		}
		createSurface1(kAsScene1907SymbolFileHashes[_elementIndex], 1000 + _currPositionIndex);
		startAnimation(kAsScene1907SymbolFileHashes[_elementIndex], -1, -1);
		_newStickFrameIndex = STICK_LAST_FRAME;
	} else {
		_isPluggedIn = false;
		_currPositionIndex = positionIndex;
		loadSound(0, 0x74231924);
		loadSound(1, 0x36691914);
		loadSound(2, 0x5421D806);
		_parentScene->setPositionFree(_currPositionIndex, false);
		_x = kAsScene1907SymbolGroundPositions[_currPositionIndex].x;
		_y = kAsScene1907SymbolGroundPositions[_currPositionIndex].y;
		createSurface1(kAsScene1907SymbolFileHashes[_elementIndex], 1000 + _currPositionIndex);
		startAnimation(kAsScene1907SymbolFileHashes[_elementIndex], 0, -1);
		_newStickFrameIndex = 0;
	}
	_collisionBoundsOffset.set(0, 0, 80, 80);
	Sprite::updateBounds();
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1907Symbol::handleMessage);

}

void AsScene1907Symbol::update() {
	updateAnim();
	handleSpriteUpdate();
	updatePosition();
	if (_plugInFailed && _plugInTryCount == 0)
		_plugInFailed = false;
}

uint32 AsScene1907Symbol::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (!_isPluggedIn && !_plugInFailed) {
			tryToPlugIn();
			messageResult = 1;
		} else
			messageResult = 0;
		break;
	}
	return messageResult;
}

uint32 AsScene1907Symbol::hmTryToPlugIn(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		gotoNextState();
		break;
	}
	return messageResult;
}

void AsScene1907Symbol::suTryToPlugIn() {
	_currStep++;
	_x -= _deltaX;
	_y -= _deltaY;
	if (_currStep == 16) {
		_x -= _smallDeltaX;
		_y -= _smallDeltaY;
		SetSpriteUpdate(NULL);
	}
}

void AsScene1907Symbol::suFallOff() {
	if (_fallOffDelay != 0) {
		_fallOffDelay--;
	} else {
		_y += _yAccel;
		_yAccel += 8;
		if (_y >= kAsScene1907SymbolGroundHitPositions[_currPositionIndex].y) {
			_y = kAsScene1907SymbolGroundHitPositions[_currPositionIndex].y;
			stFallOffHitGround();
		}
	}
}

void AsScene1907Symbol::suFallOffHitGround() {

	if (_x == _someX - _xBreak)
		_x -= _smallDeltaX;
	else
		_x -= _deltaX;

	if (_y == kAsScene1907SymbolGroundHitPositions[_currPositionIndex].y) {
		_y -= _someY;
	}

	if (_currStep < 8) {
		_y -= _yAccel;
		_yAccel -= 4;
		if (_yAccel < 0)
			_yAccel = 0;
	} else if (_currStep < 15) {
		_y += _yAccel;
		_yAccel += 4;
	} else {
		_y = kAsScene1907SymbolGroundPositions[_newPositionIndex].y;
		cbFallOffHitGroundEvent();
	}

	_currStep++;
}

void AsScene1907Symbol::suMoveDown() {
	_y += _yIncr;
	if (_yIncr < 11)
		_yIncr++;
	if (_y >= kAsScene1907SymbolPluggedInDownPositions[_elementIndex].y) {
		_y = kAsScene1907SymbolPluggedInDownPositions[_elementIndex].y;
		_isMoving = false;
		SetSpriteUpdate(NULL);
	}
}

void AsScene1907Symbol::suMoveUp() {
	_y -= _yIncr;
	if (getGlobalVar(V_WALL_BROKEN)) {
		if (_y - (9 + (_elementIndex > 5 ? 31 : 0)) < kAsScene1907SymbolPluggedInPositions[_elementIndex].y)
			_yIncr--;
		else
			_yIncr++;
	} else
		_yIncr = 2;
	if (_yIncr > 9)
		_yIncr = 9;
	else if (_yIncr < 1)
		_yIncr = 1;
	if (_y < kAsScene1907SymbolPluggedInPositions[_elementIndex].y) {
		_y = kAsScene1907SymbolPluggedInPositions[_elementIndex].y;
		_isMoving = false;
		SetSpriteUpdate(NULL);
	}
}

void AsScene1907Symbol::tryToPlugIn() {
	_isPluggedIn = true;
	_plugInTryCount++;
	_newPositionIndex = _parentScene->getNextPosition();
	_parentScene->setPositionFree(_currPositionIndex, true);
	sendMessage(_parentScene, 0x1022, 1100 + _newPositionIndex);
	startAnimation(kAsScene1907SymbolFileHashes[_elementIndex], 0, -1);
	SetUpdateHandler(&AsScene1907Symbol::update);
	SetMessageHandler(&AsScene1907Symbol::hmTryToPlugIn);
	SetSpriteUpdate(&AsScene1907Symbol::suTryToPlugIn);
	_currStep = 0;
	_deltaX = (_x - kAsScene1907SymbolPluggedInPositions[_newPositionIndex].x) / 16;
	_smallDeltaX = _x - _deltaX * 16 - kAsScene1907SymbolPluggedInPositions[_newPositionIndex].x;
	_deltaY = (_y - kAsScene1907SymbolPluggedInPositions[_newPositionIndex].y) / 16;
	_smallDeltaY = _y - _deltaY * 16 - kAsScene1907SymbolPluggedInPositions[_newPositionIndex].y;
	if (_elementIndex == _newPositionIndex) {
		NextState(&AsScene1907Symbol::stPlugIn);
	} else {
		_plugInFailed = true;
		NextState(&AsScene1907Symbol::stPlugInFail);
	}
}

void AsScene1907Symbol::fallOff(int newPositionIndex, int fallOffDelay) {
	_isPluggedIn = false;
	_newPositionIndex = newPositionIndex;
	_fallOffDelay = fallOffDelay;
	_parentScene->setPositionFree(_newPositionIndex, false);
	_x = kAsScene1907SymbolPluggedInPositions[_currPositionIndex].x;
	_y = kAsScene1907SymbolPluggedInPositions[_currPositionIndex].y;
	_someX = _x;
	_someY = _y;
	startAnimation(kAsScene1907SymbolFileHashes[_elementIndex], -1, 0);
	_playBackwards = true;
	_newStickFrameIndex = STICK_LAST_FRAME;
	_currStep = 0;
	_yAccel = 1;
	SetUpdateHandler(&AsScene1907Symbol::update);
	SetMessageHandler(&AsScene1907Symbol::handleMessage);
	SetSpriteUpdate(&AsScene1907Symbol::suFallOff);
}

void AsScene1907Symbol::stFallOffHitGround() {
	playSound(1);
	sendMessage(_parentScene, 0x1022, 1000 + _newPositionIndex);
	Entity::_priority = 1000 - _newPositionIndex;
	_parentScene->removeCollisionSprite(this);
	_parentScene->addCollisionSprite(this);
	SetSpriteUpdate(&AsScene1907Symbol::suFallOffHitGround);
	NextState(&AsScene1907Symbol::cbFallOffHitGroundEvent);
	_newStickFrameIndex = 0;
	_currStep = 0;
	_yAccel = 30;
	_deltaX = (_x - kAsScene1907SymbolGroundPositions[_newPositionIndex].x) / 15;
	_xBreak = _deltaX * 15;
	_smallDeltaX = _x - kAsScene1907SymbolGroundPositions[_newPositionIndex].x - _xBreak;
	_someY = 0;
	if (kAsScene1907SymbolGroundHitPositions[_currPositionIndex].y > kAsScene1907SymbolGroundPositions[_newPositionIndex].y)
		_someY = kAsScene1907SymbolGroundHitPositions[_currPositionIndex].y - kAsScene1907SymbolGroundPositions[_newPositionIndex].y;
}

void AsScene1907Symbol::cbFallOffHitGroundEvent() {
	_currPositionIndex = _newPositionIndex;
	if (_plugInTryCount > 0)
		_plugInTryCount--;
	startAnimation(kAsScene1907SymbolFileHashes[_elementIndex], 0, -1);
	_newStickFrameIndex = 0;
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1907Symbol::handleMessage);
	SetSpriteUpdate(NULL);
	updateBounds();
	playSound(2);
}

void AsScene1907Symbol::stPlugIn() {
	playSound(0);
	_currPositionIndex = _newPositionIndex;
	stopAnimation();
	SetMessageHandler(&AsScene1907Symbol::handleMessage);
	SetSpriteUpdate(NULL);
	if (_elementIndex == 8)
		sendMessage(_parentScene, 0x2001, 0);
}

void AsScene1907Symbol::stPlugInFail() {
	_currPositionIndex = _newPositionIndex;
	stopAnimation();
	_parentScene->plugInFailed();
}

void AsScene1907Symbol::moveUp() {
	startAnimation(kAsScene1907SymbolFileHashes[_elementIndex], -1, -1);
	stopAnimation();
	SetMessageHandler(&AsScene1907Symbol::handleMessage);
	SetSpriteUpdate(&AsScene1907Symbol::suMoveUp);
	_yIncr = 1;
	_isMoving = true;
}

void AsScene1907Symbol::moveDown() {
	startAnimation(kAsScene1907SymbolFileHashes[_elementIndex], -1, -1);
	stopAnimation();
	SetMessageHandler(&AsScene1907Symbol::handleMessage);
	SetSpriteUpdate(&AsScene1907Symbol::suMoveDown);
	_yIncr = 4;
	_isMoving = true;
}

SsScene1907UpDownButton::SsScene1907UpDownButton(NeverhoodEngine *vm, Scene1907 *parentScene, AsScene1907Symbol *asScene1907Symbol)
	: StaticSprite(vm, 1400), _parentScene(parentScene), _asScene1907Symbol(asScene1907Symbol),
	_countdown1(0) {

	loadSprite(0x64516424, kSLFDefDrawOffset | kSLFDefPosition | kSLFDefCollisionBoundsOffset, 1400);
	setVisible(false);
	loadSound(0, 0x44061000);
	SetUpdateHandler(&SsScene1907UpDownButton::update);
	SetMessageHandler(&SsScene1907UpDownButton::handleMessage);
	if (getGlobalVar(V_STAIRS_PUZZLE_SOLVED)) {
		if (getGlobalVar(V_STAIRS_DOWN))
			setToDownPosition();
		else
			setToUpPosition();
	}
}

void SsScene1907UpDownButton::update() {
	updatePosition();
	if (_countdown1 != 0 && (--_countdown1 == 0)) {
		setVisible(false);
		sendMessage(_parentScene, 0x2000, 0);
	}
}

uint32 SsScene1907UpDownButton::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_countdown1 == 0 && !_asScene1907Symbol->isMoving() && getGlobalVar(V_STAIRS_PUZZLE_SOLVED)) {
			setVisible(true);
			_countdown1 = 4;
			updatePosition();
			playSound(0);
		}
		messageResult = 1;
	}
	return messageResult;
}

void SsScene1907UpDownButton::setToUpPosition() {
	_y = _spriteResource.getPosition().y;
	updateBounds();
	updatePosition();
}

void SsScene1907UpDownButton::setToDownPosition() {
	_y = _spriteResource.getPosition().y + 174;
	updateBounds();
	updatePosition();
}

AsScene1907WaterHint::AsScene1907WaterHint(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1400) {

	createSurface1(0x110A1061, 1500);
	_x = 320;
	_y = 240;
	startAnimation(0x110A1061, 0, -1);
	_newStickFrameIndex = 0;
	setVisible(false);
	_needRefresh = true;
	AnimatedSprite::updatePosition();
	SetUpdateHandler(&AsScene1907WaterHint::update);
	SetMessageHandler(&Sprite::handleMessage);
}

void AsScene1907WaterHint::update() {
	updateAnim();
	updatePosition();
}

uint32 AsScene1907WaterHint::hmShowing(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		gotoNextState();
		break;
	}
	return messageResult;
}

void AsScene1907WaterHint::show() {
	setVisible(true);
	startAnimation(0x110A1061, 0, -1);
	SetMessageHandler(&AsScene1907WaterHint::hmShowing);
	NextState(&AsScene1907WaterHint::hide);
}

void AsScene1907WaterHint::hide() {
	stopAnimation();
	setVisible(false);
	SetMessageHandler(&Sprite::handleMessage);
}

Scene1907::Scene1907(NeverhoodEngine *vm, Module *parentModule)
	: Scene(vm, parentModule), _currMovingSymbolIndex(0), _pluggedInCount(0),
	_moveDownCountdown(0), _moveUpCountdown(0), _countdown3(0), _hasPlugInFailed(false) {

	setBackground(0x20628E05);
	setPalette(0x20628E05);

	for (int i = 0; i < 9; i++)
		_positionFree[i] = true;

	for (int i = 0; i < 9; i++) {
		_asSymbols[i] = insertSprite<AsScene1907Symbol>(this, i, getRandomPositionIndex());
		addCollisionSprite(_asSymbols[i]);
	}

	_ssUpDownButton = insertSprite<SsScene1907UpDownButton>(this, _asSymbols[8]);
	addCollisionSprite(_ssUpDownButton);

	_asWaterHint = insertSprite<AsScene1907WaterHint>();

	insertPuzzleMouse(0x28E0120E, 20, 620);

	SetMessageHandler(&Scene1907::handleMessage);
	SetUpdateHandler(&Scene1907::update);

	if (getGlobalVar(V_STAIRS_PUZZLE_SOLVED))
		_pluggedInCount = 9;

	loadSound(0, 0x72004A10);
	loadSound(1, 0x22082A12);
	loadSound(2, 0x21100A10);
	loadSound(3, 0x68E25540);

}

void Scene1907::update() {
	Scene::update();

	if (_hasPlugInFailed) {
		int fallOffDelay = 0;
		_hasPlugInFailed = false;
		for (int i = 0; i < 9; i++) {
			AsScene1907Symbol *asSymbol = _asSymbols[8 - i];
			if (asSymbol->isPluggedIn()) {
				asSymbol->fallOff(getRandomPositionIndex(), fallOffDelay);
				fallOffDelay += _vm->_rnd->getRandomNumber(10 - 1) + 4;
			}
		}
	}

	if (_moveDownCountdown != 0 && (--_moveDownCountdown == 0)) {
		_asSymbols[_currMovingSymbolIndex]->moveDown();
		if (_currMovingSymbolIndex > 0) {
			_moveDownCountdown = 2;
			_currMovingSymbolIndex--;
		}
	}

	if (_moveUpCountdown != 0 && (--_moveUpCountdown == 0)) {
		_moveDownCountdown = 0;
		for (int i = 0; i < 9; i++)
			_asSymbols[i]->moveUp();
	}

	if (_countdown3 != 0 && (--_countdown3 == 0)) {
		_asWaterHint->show();
		_moveUpCountdown = 4;
	}

}

uint32 Scene1907::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		if ((param.asPoint().x <= 20 || param.asPoint().x >= 620) &&
			!_hasPlugInFailed && _moveDownCountdown == 0 && _moveUpCountdown == 0 && _countdown3 == 0) {
			leaveScene(0);
		}
		break;
	case 0x2000:
		if (getGlobalVar(V_STAIRS_DOWN)) {
			playSound(0);
			for (int i = 0; i < 9; i++)
				_asSymbols[i]->moveUp();
			_ssUpDownButton->setToUpPosition();
			setGlobalVar(V_STAIRS_DOWN, 0);
		} else {
			if (!getGlobalVar(V_WALL_BROKEN)) {
				playSound(2);
				_countdown3 = 5;
			} else {
				playSound(1);
				_ssUpDownButton->setToDownPosition();
				setGlobalVar(V_STAIRS_DOWN, 1);
			}
			_moveDownCountdown = 1;
			_currMovingSymbolIndex = 8;
		}
		break;
	case 0x2001:
		playSound(3);
		setGlobalVar(V_STAIRS_PUZZLE_SOLVED, 1);
		break;
	}
	return 0;
}

void Scene1907::plugInFailed() {
	_pluggedInCount = 0;
	_hasPlugInFailed = true;
}

int Scene1907::getRandomPositionIndex() {
	bool found = false;
	int index = 0;
	// Check if any position is free
	for (int i = 0; i < 9; i++)
		if (_positionFree[i])
			found = true;
	if (found) {
		// Get a random free position
		found = false;
		while (!found) {
			index = _vm->_rnd->getRandomNumber(9 - 1);
			if (_positionFree[index])
				found = true;
		}
	}
	return index;
}

} // End of namespace Neverhood
