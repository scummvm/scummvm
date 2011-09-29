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

#include "neverhood/module1900.h"
#include "neverhood/gamemodule.h"

namespace Neverhood {

Module1900::Module1900(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {

	// NOTE: The original has a Scene1908 here as well but it's not used here but in another module... 
	
	if (which < 0) {
		createScene(_vm->gameState().sceneNum, -1);
	} else {
		createScene(0, 0);
	}

	// TODO Sound1ChList_addSoundResources(0x04E1C09C, dword_4B8800, true);
	// TODO Sound1ChList_setSoundValuesMulti(dword_4B8800, true, 50, 600, 5, 150);

}

Module1900::~Module1900() {
	// TODO Sound1ChList_sub_407A50(0x04E1C09C);
}

void Module1900::createScene(int sceneNum, int which) {
	debug("Module1900::createScene(%d, %d)", sceneNum, which);
	_vm->gameState().sceneNum = sceneNum;
	switch (_vm->gameState().sceneNum) {
	case 0:
		_childObject = new Scene1901(_vm, this, which);
		break;
	case 6:
		_childObject = new Scene1907(_vm, this, which);
		break;
	}
	SetUpdateHandler(&Module1900::updateScene);
	_childObject->handleUpdate();
}

void Module1900::updateScene() {
	if (!updateChild()) {
		switch (_vm->gameState().sceneNum) {
		case 0:
			if (_moduleResult == 1) {
				createScene(6, 0);
			} else {
				leaveModule(0);
			}
			break;
		case 6:
			createScene(0, 1);
			break;
		}
	}
}

// Scene1901

Scene1901::Scene1901(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true) {
	
	Sprite *tempSprite;

	_surfaceFlag = true;
	
	setRectList(0x004B34C8);

	setBackground(0x01303227);
	setPalette(0x01303227);
	insertMouse433(0x0322301B);
	
	insertStaticSprite(0x42213133, 1100);
	
	if (!getGlobalVar(0xA9035F60)) {
		insertStaticSprite(0x40A40168, 100);
	} else if (getGlobalVar(0x09221A62)) {
		insertStaticSprite(0x124404C4, 100);
		setGlobalVar(0x2050861A, 1);
	} else {
		insertStaticSprite(0x02840064, 100);
	}

	if (which < 0) {
		insertKlayman<KmScene1901>(120, 380);
		setMessageList(0x004B3408);
	} else if (which == 1) {
		insertKlayman<KmScene1901>(372, 380);
		setMessageList(0x004B3410);
	} else {
		insertKlayman<KmScene1901>(0, 380);
		setMessageList(0x004B3400);
	}

	tempSprite = insertStaticSprite(0x4830A402, 1100);
	_klayman->setClipRect(tempSprite->getDrawRect().x, 0, 640, 480);

}

static const NPoint kAsScene1907SymbolGroundPositions[] = {
	{160, 310},
	{ 90, 340},
	{210, 335},
	{210, 380},
	{310, 340},
	{290, 400},
	{400, 375},
	{370, 435},
	{475, 415}
};

static const NPoint kAsScene1907SymbolPluggedInPositions[] = { 
	{275, 125},
	{244, 125},
	{238, 131},
	{221, 135},
	{199, 136},
	{168, 149},
	{145, 152},
	{123, 154},
	{103, 157}
};

static const NPoint kAsScene1907SymbolGroundHitPositions[] = {
	{275, 299}, 
	{244, 299}, 
	{238, 305}, 
	{221, 309},
	{199, 310},
	{168, 323}, 
	{145, 326}, 
	{123, 328}, 
	{103, 331} 
};

static const NPoint kAsScene1907SymbolPluggedInDownPositions[] = {
	{275, 136},
	{244, 156},
	{238, 183},
	{221, 207},
	{199, 228},
	{168, 262},
	{145, 285},
	{123, 307},
	{103, 331}
};

static const uint32 kAsScene1907SymbolFileHashes[] = {
	0x006A1034,
	0x006A1010,
	0x006A1814,
	0x006A1016,
	0x006A0014,
	0x002A1014,
	0x00EA1014,
	0x206A1014,
	0x046A1414
};

int AsScene1907Symbol::_symbolFlag1 = 0;
int AsScene1907Symbol::_symbolFlag2 = 0;

AsScene1907Symbol::AsScene1907Symbol(NeverhoodEngine *vm, Scene1907 *parentScene, int elementIndex, int positionIndex)
	: AnimatedSprite(vm, 1000 - positionIndex), _soundResource1(vm), _soundResource2(vm), _soundResource3(vm),
	_parentScene(parentScene), _elementIndex(elementIndex), _isMoving(false) {

	_symbolFlag1 = 0;
	_symbolFlag2 = 0;
	
	if (getGlobalVar(0xA9035F60)) {
		_isPluggedIn = true;
		_currPositionIndex = elementIndex;
		if (!getGlobalVar(0x09221A62)) {
			_x = kAsScene1907SymbolPluggedInPositions[_currPositionIndex].x;
			_y = kAsScene1907SymbolPluggedInPositions[_currPositionIndex].y;
		} else {
			_x = kAsScene1907SymbolPluggedInDownPositions[_currPositionIndex].x;
			_y = kAsScene1907SymbolPluggedInDownPositions[_currPositionIndex].y;
		}
		createSurface1(kAsScene1907SymbolFileHashes[_elementIndex], 1000 + _currPositionIndex);
		setFileHash(kAsScene1907SymbolFileHashes[_elementIndex], -1, -1);
		_newHashListIndex = -2;
	} else {
		_isPluggedIn = false;
		_currPositionIndex = positionIndex;
		_soundResource1.load(0x74231924);
		_soundResource2.load(0x36691914);
		_soundResource3.load(0x5421D806);
		_parentScene->setPositionFree(_currPositionIndex, false);
		_x = kAsScene1907SymbolGroundPositions[_currPositionIndex].x;
		_y = kAsScene1907SymbolGroundPositions[_currPositionIndex].y;
		createSurface1(kAsScene1907SymbolFileHashes[_elementIndex], 1000 + _currPositionIndex);
		setFileHash(kAsScene1907SymbolFileHashes[_elementIndex], 0, -1);
		_newHashListIndex = 0;
	}
	_deltaRect.set(0, 0, 80, 80);
	Sprite::processDelta();
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1907Symbol::handleMessage);

}

void AsScene1907Symbol::update() {
	AnimatedSprite::updateAnim();
	handleSpriteUpdate();
	AnimatedSprite::updatePosition();
	if (_symbolFlag1 && !_symbolFlag2)
		_symbolFlag1 = 0;
}

uint32 AsScene1907Symbol::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (!_isPluggedIn && !_symbolFlag1) {
			tryToPlugIn();
			messageResult = 1;
		} else {
			messageResult = 0;
		}
		break;
	}
	return messageResult;
}

uint32 AsScene1907Symbol::hmTryToPlugIn(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		removeCallbacks();
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
		SetSpriteCallback(NULL);
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
		SetSpriteCallback(NULL);
	}	
}

void AsScene1907Symbol::suMoveUp() {
	_y -= _yIncr;
	if (getGlobalVar(0x10938830)) {
		if (_y - (9 + (_elementIndex > 5 ? 31 : 0)) < kAsScene1907SymbolPluggedInPositions[_elementIndex].y) {
			_yIncr--;
		} else {
			_yIncr++;
		}
	} else {
		_yIncr = 2;
	}
	if (_yIncr > 9)
		_yIncr = 9;
	else if (_yIncr < 1)
		_yIncr = 1;
	if (_y < kAsScene1907SymbolPluggedInPositions[_elementIndex].y) {
		_y = kAsScene1907SymbolPluggedInPositions[_elementIndex].y;
		_isMoving = false;
		SetSpriteCallback(NULL);
	}
}

void AsScene1907Symbol::tryToPlugIn() {
	_isPluggedIn = true;
	_symbolFlag2++;
	_newPositionIndex = _parentScene->getNextPosition();
	_parentScene->setPositionFree(_currPositionIndex, true);
	sendMessage(_parentScene, 0x1022, 1100 + _newPositionIndex);
	setFileHash(kAsScene1907SymbolFileHashes[_elementIndex], 0, -1);
	SetUpdateHandler(&AsScene1907Symbol::update);
	SetMessageHandler(&AsScene1907Symbol::hmTryToPlugIn);
	SetSpriteCallback(&AsScene1907Symbol::suTryToPlugIn);
	_currStep = 0;
	_deltaX = (_x - kAsScene1907SymbolPluggedInPositions[_newPositionIndex].x) / 16;
	_smallDeltaX = _x - _deltaX * 16 - kAsScene1907SymbolPluggedInPositions[_newPositionIndex].x;
	_deltaY = (_y - kAsScene1907SymbolPluggedInPositions[_newPositionIndex].y) / 16;
	_smallDeltaY = _y - _deltaY * 16 - kAsScene1907SymbolPluggedInPositions[_newPositionIndex].y;
	if (_elementIndex == _newPositionIndex) {
		SetAnimationCallback3(&AsScene1907Symbol::stPlugIn);
	} else {
		_symbolFlag1 = 1;
		SetAnimationCallback3(&AsScene1907Symbol::stPlugInFail);
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
	setFileHash(kAsScene1907SymbolFileHashes[_elementIndex], -1, 0);
	_playBackwards = true;
	_newHashListIndex = -2;
	_currStep = 0;
	_yAccel = 1;
	SetUpdateHandler(&AsScene1907Symbol::update);
	SetMessageHandler(&AsScene1907Symbol::handleMessage);
	SetSpriteCallback(&AsScene1907Symbol::suFallOff);
}

void AsScene1907Symbol::stFallOffHitGround() {
	_soundResource2.play();
	sendMessage(_parentScene, 0x1022, 1000 + _newPositionIndex);
	// TODO: Meh...
	Entity::_priority = 1000 - _newPositionIndex;
	_vm->_collisionMan->removeSprite(this);
	_vm->_collisionMan->addSprite(this);
	SetSpriteCallback(&AsScene1907Symbol::suFallOffHitGround);
	SetAnimationCallback3(&AsScene1907Symbol::cbFallOffHitGroundEvent);
	_newHashListIndex = 0;
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
	if (_symbolFlag2)
		_symbolFlag2--;
	setFileHash(kAsScene1907SymbolFileHashes[_elementIndex], 0, -1);
	_newHashListIndex = 0;
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1907Symbol::handleMessage);
	SetSpriteCallback(NULL);
	processDelta();
	_soundResource3.play();
}

void AsScene1907Symbol::stPlugIn() {
	_soundResource1.play();
	_currPositionIndex = _newPositionIndex;
	setFileHash1();
	SetMessageHandler(&AsScene1907Symbol::handleMessage);
	SetSpriteCallback(NULL);
	if (_elementIndex == 8)
		sendMessage(_parentScene, 0x2001, 0);
}

void AsScene1907Symbol::stPlugInFail() {
	_currPositionIndex = _newPositionIndex;
	setFileHash1();
	_parentScene->plugInFailed();
}

void AsScene1907Symbol::moveUp() {
	setFileHash(kAsScene1907SymbolFileHashes[_elementIndex], -1, -1);//????
	setFileHash1();
	SetMessageHandler(&AsScene1907Symbol::handleMessage);
	SetSpriteCallback(&AsScene1907Symbol::suMoveUp);
	_yIncr = 1;
	_isMoving = true;
}

void AsScene1907Symbol::moveDown() {
	setFileHash(kAsScene1907SymbolFileHashes[_elementIndex], -1, -1);//????
	setFileHash1();
	SetMessageHandler(&AsScene1907Symbol::handleMessage);
	SetSpriteCallback(&AsScene1907Symbol::suMoveDown);
	_yIncr = 4;
	_isMoving = true;
}

SsScene1907UpDownButton::SsScene1907UpDownButton(NeverhoodEngine *vm, Scene1907 *parentScene, AsScene1907Symbol *AsScene1907Symbol)
	: StaticSprite(vm, 1400), _soundResource(vm), _parentScene(parentScene), _AsScene1907Symbol(AsScene1907Symbol),
	_countdown1(0) {
	
	_spriteResource.load2(0x64516424);
	createSurface(1400, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	setVisible(false);
	_drawRect.set(0, 0, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	_deltaRect = _drawRect;
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	processDelta();
	_needRefresh = true;
	_soundResource.load(0x44061000);
	SetUpdateHandler(&SsScene1907UpDownButton::update);
	SetMessageHandler(&SsScene1907UpDownButton::handleMessage);
	if (getGlobalVar(0xA9035F60)) {
		if (getGlobalVar(0x09221A62))
			setToDownPosition();
		else
			setToUpPosition();
	}
}

void SsScene1907UpDownButton::update() {
	StaticSprite::update();
	if (_countdown1 != 0 && (--_countdown1 == 0)) {
		setVisible(false);
		sendMessage(_parentScene, 0x2000, 0);
	}
}

uint32 SsScene1907UpDownButton::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_countdown1 == 0 && !_AsScene1907Symbol->isMoving() && getGlobalVar(0xA9035F60)) {
			setVisible(true);
			_countdown1 = 4;
			StaticSprite::update();
			_soundResource.play();
		}
		messageResult = 1;
	}
	return messageResult;
}

void SsScene1907UpDownButton::setToUpPosition() {
	_y = _spriteResource.getPosition().y;
	Sprite::processDelta();
	StaticSprite::update();
}

void SsScene1907UpDownButton::setToDownPosition() {
	_y = _spriteResource.getPosition().y + 174;
	Sprite::processDelta();
	StaticSprite::update();
}

AsScene1907WaterHint::AsScene1907WaterHint(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1400) {
	
	createSurface1(0x110A1061, 1500);
	_x = 320;
	_y = 240;
	setFileHash(0x110A1061, 0, -1);
	_newHashListIndex = 0;
	setVisible(false);
	_needRefresh = true;
	AnimatedSprite::updatePosition();
	SetUpdateHandler(&AsScene1907WaterHint::update);
	SetMessageHandler(&Sprite::handleMessage);
}

void AsScene1907WaterHint::update() {
	AnimatedSprite::updateAnim();
	AnimatedSprite::updatePosition();
}

uint32 AsScene1907WaterHint::handleMessage46BA20(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		removeCallbacks();
		break;
	}
	return messageResult;
}

void AsScene1907WaterHint::show() {
	setVisible(true);
	setFileHash(0x110A1061, 0, -1);
	SetMessageHandler(&AsScene1907WaterHint::handleMessage46BA20);
	SetAnimationCallback3(&AsScene1907WaterHint::hide);
}

void AsScene1907WaterHint::hide() {
	setFileHash1();
	setVisible(false);
	SetMessageHandler(&Sprite::handleMessage);
}

Scene1907::Scene1907(NeverhoodEngine *vm, Module *parentModule, int which)	
	: Scene(vm, parentModule, true), _soundResource1(vm), _soundResource2(vm),
	_soundResource3(vm), _soundResource4(vm), _currMovingSymbolIndex(0), _pluggedInCount(0), 
	_moveDownCountdown(0), _moveUpCountdown(0), _countdown3(0), _hasPlugInFailed(false) {
	
	_surfaceFlag = true;

	//setGlobalVar(0x10938830, 1);
	
	setBackground(0x20628E05);
	setPalette(0x20628E05);

	for (int i = 0; i < 9; i++)
		_positionFree[i] = true;

	for (int i = 0; i < 9; i++) {
		_asSymbols[i] = insertSprite<AsScene1907Symbol>(this, i, getRandomPositionIndex());
		_vm->_collisionMan->addSprite(_asSymbols[i]);
	}
	
	_ssUpDownButton = insertSprite<SsScene1907UpDownButton>(this, _asSymbols[8]);
	_vm->_collisionMan->addSprite(_ssUpDownButton);

	_asWaterHint = insertSprite<AsScene1907WaterHint>();
	
	insertMouse435(0x28E0120E, 20, 620);

	SetMessageHandler(&Scene1907::handleMessage);
	SetUpdateHandler(&Scene1907::update);

	if (getGlobalVar(0xA9035F60))
		_pluggedInCount = 9;
		
	_soundResource1.load(0x72004A10);
	_soundResource2.load(0x22082A12);
	_soundResource3.load(0x21100A10);
	_soundResource4.load(0x68E25540);

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
		// TODO DEBUG stuff
		if ((param.asPoint().x <= 20 || param.asPoint().x >= 620) &&
			!_hasPlugInFailed && _moveDownCountdown == 0 && _moveUpCountdown == 0 && _countdown3 == 0) {
			leaveScene(0);
		}
		break;
	// TODO Debug stuff
	case 0x2000:
		if (getGlobalVar(0x09221A62)) {
			_soundResource1.play();
			for (int i = 0; i < 9; i++)
				_asSymbols[i]->moveUp();
			_ssUpDownButton->setToUpPosition();
			setGlobalVar(0x09221A62, 0);
		} else {
			if (!getGlobalVar(0x10938830)) {
				_soundResource3.play();
				_countdown3 = 5;
			} else {
				_soundResource2.play();
				_ssUpDownButton->setToDownPosition();
				setGlobalVar(0x09221A62, 1);
			}
			_moveDownCountdown = 1;
			_currMovingSymbolIndex = 8;
		}
		break;
	case 0x2001:
		_soundResource4.play();
		setGlobalVar(0xA9035F60, 1);
		break;
	}	
	return 0;
}

void Scene1907::plugInFailed() {
	_pluggedInCount = 0;
	_hasPlugInFailed = true;
}

int Scene1907::getRandomPositionIndex() {
	bool flag = false;
	int index = 0;
	for (int i = 0; i < 9; i++) {
		if (_positionFree[i])
			flag = true;
	}
	if (flag) {
		flag = false;
		while (!flag) {
			index = _vm->_rnd->getRandomNumber(9 - 1);
			if (_positionFree[index])
				flag = true;
		}
	}
	return index;
}

} // End of namespace Neverhood
