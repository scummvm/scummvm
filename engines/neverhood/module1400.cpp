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

#include "neverhood/module1400.h"
#include "neverhood/module1000.h"
#include "neverhood/module2100.h"
#include "neverhood/module2200.h"
#include "neverhood/diskplayerscene.h"
#include "neverhood/gamemodule.h"

namespace Neverhood {

Module1400::Module1400(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {
	
	_vm->_soundMan->addMusic(0x00AD0012, 0x06333232);
	_vm->_soundMan->addMusic(0x00AD0012, 0x624A220E);

	if (which < 0) {
		createScene(_vm->gameState().sceneNum, -1);
	} else {
		createScene(0, 0);
	}

}

Module1400::~Module1400() {
	_vm->_soundMan->deleteMusicGroup(0x00AD0012);
}

void Module1400::createScene(int sceneNum, int which) {
	debug("Module1400::createScene(%d, %d)", sceneNum, which);
	_vm->gameState().sceneNum = sceneNum;
	switch (_vm->gameState().sceneNum) {
	case 0:
		_vm->_soundMan->startMusic(0x06333232, 0, 2);
		_childObject = new Scene1401(_vm, this, which);
		break;
	case 1:
		_vm->_soundMan->stopMusic(0x06333232, 0, 2);
		_vm->_soundMan->stopMusic(0x624A220E, 0, 2);
		_childObject = new Scene1402(_vm, this, which);
		break;
	case 2:
		_vm->_soundMan->stopMusic(0x06333232, 0, 2);
		_vm->_soundMan->startMusic(0x624A220E, 0, 2);
		_childObject = new Scene1403(_vm, this, which);
		break;
	case 3:
		_vm->_soundMan->startMusic(0x06333232, 0, 2);
		_childObject = new Scene1404(_vm, this, which);
		break;
	case 4:
		_vm->_soundMan->startMusic(0x06333232, 0, 2);
		_childObject = new Scene1405(_vm, this, which);
		break;
	case 5:
		_vm->_soundMan->stopMusic(0x06333232, 0, 2);
		_childObject = new DiskplayerScene(_vm, this, 2);
		break;
	case 6:
		_vm->_soundMan->stopMusic(0x06333232, 0, 2);
		_childObject = new Scene1407(_vm, this, which);
		break;
	}
	SetUpdateHandler(&Module1400::updateScene);
	_childObject->handleUpdate();
}

void Module1400::updateScene() {
	if (!updateChild()) {
		switch (_vm->gameState().sceneNum) {
		case 0:
			if (_moduleResult == 1) {
				createScene(1, 0);
			} else if (_moduleResult == 2) {
				createScene(3, 0);
			} else {
				leaveModule(0);
			}
			break;
		case 1:
			if (_moduleResult == 1) {
				createScene(2, 0);
			} else if (_moduleResult == 2) {
				createScene(6, -1);
			} else {
				createScene(0, 1);
			}
			break;
		case 2:
			createScene(1, 1);
			break;
		case 3:
			if (_moduleResult == 1) {
				createScene(4, 0);
			} else if (_moduleResult == 2) {
				createScene(5, -1);
			} else {
				createScene(0, 2);
			}
			break;
		case 4:
			createScene(3, 1);
			break;
		case 5:
			createScene(3, 2);
			break;
		case 6:
			createScene(1, 2);
			break;
		}
	}
}

// Scene1401

AsScene1401Pipe::AsScene1401Pipe(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1100), _countdown1(0), _countdown2(0) {
	
	createSurface(900, 152, 147);
	_x = 454;
	_y = 217;
	SetMessageHandler(&AsScene1401Pipe::handleMessage);
	SetUpdateHandler(&AsScene1401Pipe::update);
	startAnimation(0x4C210500, 0, -1);
}

AsScene1401Pipe::~AsScene1401Pipe() {
	_vm->_soundMan->deleteSoundGroup(0x01104C08);
}

void AsScene1401Pipe::update() {
	AnimatedSprite::update();
	if (_countdown1 != 0 && (--_countdown1 == 0)) {
		stDoneSucking();
	}
	if (_countdown2 != 0 && (--_countdown2 == 0)) {
		_vm->_soundMan->addSound(0x01104C08, 0x4A116437);
		_vm->_soundMan->playSoundLooping(0x4A116437);
	}
}

void AsScene1401Pipe::upSuckInProjector() {
	AnimatedSprite::update();
	if (_countdown1 != 0) {
		_countdown1--;
	}
}

uint32 AsScene1401Pipe::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x0A8A1490) {
			playSound(1, 0x6AB6666F);
		}
		break;
	case 0x2000:
		_countdown1 = 70;
		_countdown2 = 8;
		stStartSucking();
		break;		
	case 0x483A:
		stSuckInProjector();
		break;		
	}
	return messageResult;
}

uint32 AsScene1401Pipe::hmSuckInProjector(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		if (_countdown1 != 0) {
			stStartSucking();
		} else {
			stDoneSucking();
		}
		SetMessageHandler(&AsScene1401Pipe::handleMessage);
		SetUpdateHandler(&AsScene1401Pipe::update);
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
	SetMessageHandler(&AsScene1401Pipe::hmSuckInProjector);
	SetUpdateHandler(&AsScene1401Pipe::upSuckInProjector);
}

AsScene1401Mouse::AsScene1401Mouse(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1100) {
	
	createSurface(100, 71, 41);
	_x = 478;
	_y = 433;
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1401Mouse::handleMessage);
	startAnimation(0xA282C472, 0, -1);
}

uint32 AsScene1401Mouse::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x66382026) {
			playSound(0, 0x0CD84468);
		} else if (param.asInteger() == 0x6E28061C) {
			playSound(0, 0x78C8402C);
		} else if (param.asInteger() == 0x462F0410) {
			playSound(0, 0x60984E28);
		}
		break;
	case 0x4839:
		stSuckedIn();
		break;		
	}
	return messageResult;
}

void AsScene1401Mouse::suSuckedIn() {
	AnimatedSprite::updateDeltaXY();
	if (_rect.y1 <= 150) {
		playSound(0, 0x0E32247F);
		stopAnimation();
		SetSpriteUpdate(NULL);
		SetMessageHandler(NULL);
		setVisible(false);
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
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1401Cheese::handleMessage);
	startAnimation(0x461A1490, 0, -1);
}

uint32 AsScene1401Cheese::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x4839:
		stSuckedIn();
		break;		
	}
	return messageResult;
}

void AsScene1401Cheese::suSuckedIn() {
	AnimatedSprite::updateDeltaXY();
	if (_rect.y1 <= 150) {
		playSound(0, 0x18020439);
		stopAnimation();
		SetSpriteUpdate(NULL);
		SetMessageHandler(NULL);
		setVisible(false);
	}
}

void AsScene1401Cheese::stSuckedIn() {
	startAnimation(0x103B8020, 0, -1);
	SetSpriteUpdate(&AsScene1401Cheese::suSuckedIn);
}

AsScene1401BackDoor::AsScene1401BackDoor(NeverhoodEngine *vm, Sprite *klayman, bool isOpen)
	: AnimatedSprite(vm, 1100), _klayman(klayman), _countdown(0) {

	_x = 320;
	_y = 240;
	createSurface1(0x04551900, 100);
	SetUpdateHandler(&AsScene1401BackDoor::update);
	SetMessageHandler(&AsScene1401BackDoor::handleMessage);
	_newStickFrameIndex = STICK_LAST_FRAME;
	if (isOpen) {
		_isOpen = true;
		startAnimation(0x04551900, -1,- 1);
		_countdown = 48;
	} else {
		_isOpen = false;
		stopAnimation();
		setVisible(false);
	}
}

void AsScene1401BackDoor::update() {
	if (_countdown != 0 && (--_countdown == 0)) {
		stCloseDoor();
	}
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
	case 0x3002:
		gotoNextState();
		break;
	case 0x4808:
		_countdown = 168;
		if (_isOpen)
			stOpenDoor();
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

AsCommonProjector::AsCommonProjector(NeverhoodEngine *vm, Scene *parentScene, Sprite *klayman, Sprite *asPipe)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _klayman(klayman), _asPipe(asPipe) {

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
	case 0x4807:
		setGlobalVar(V_PROJECTOR_SLOT, (_x - _asProjectorItem->point.x) / 108);
		if ((int8)getGlobalVar(V_PROJECTOR_SLOT) == _asProjectorItem->lockSlotIndex) {
			stStartLockedInSlot();
		} else {
			stIdle();
		}
		break;
	case 0x480B:
		if (param.asInteger() != 1) {
			if ((int8)getGlobalVar(V_PROJECTOR_SLOT) < _asProjectorItem->maxSlotCount) {
				incGlobalVar(V_PROJECTOR_SLOT, 1);
			}
		} else if (getGlobalVar(V_PROJECTOR_SLOT) > 0) {
			incGlobalVar(V_PROJECTOR_SLOT, -1);
		}
		stMoving();
		break;
	case 0x480C:
		// Check if the projector can be moved
		if (param.asInteger() != 1) {
			messageResult = (int8)getGlobalVar(V_PROJECTOR_SLOT) < _asProjectorItem->maxSlotCount ? 1 : 0;
		} else {
			messageResult = getGlobalVar(V_PROJECTOR_SLOT) > 0 ? 1 : 0;
		}
		break;
	case 0x482A:
		sendMessage(_parentScene, 0x1022, 990);
		break;
	case 0x482B:
		sendMessage(_parentScene, 0x1022, 1010);
		break;
	case 0x4839:
		stStartSuckedIn();
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
		} else {
			sendMessage(_parentScene, 0x4826, 0);
		}
		messageResult = 1;
		break;
	case 0x4807:
		sendMessage(_parentScene, 0x4807, 0);
		stStopProjecting();
		break;
	case 0x480B:
		if (param.asInteger() != 1) {
			if ((int8)getGlobalVar(V_PROJECTOR_SLOT) < _asProjectorItem->maxSlotCount) {
				incGlobalVar(V_PROJECTOR_SLOT, 1);
			}
		} else if (getGlobalVar(V_PROJECTOR_SLOT) > 0) {
			incGlobalVar(V_PROJECTOR_SLOT, -1);
		}
		stTurnToFront();
		break;
	case 0x480C:
		// Check if the projector can be moved
		if (param.asInteger() != 1) {
			messageResult = (int8)getGlobalVar(V_PROJECTOR_SLOT) < _asProjectorItem->maxSlotCount ? 1 : 0;
		} else {
			messageResult = getGlobalVar(V_PROJECTOR_SLOT) > 0 ? 1 : 0;
		}
		break;
	case 0x480F:
		stStartProjecting();
		break;
	case 0x482A:
		sendMessage(_parentScene, 0x1022, 990);
		break;
	case 0x482B:
		sendMessage(_parentScene, 0x1022, 1010);
		break;
	}
	return messageResult;
}

uint32 AsCommonProjector::hmAnimation(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		gotoNextState();
		break;
	}
	return messageResult;
}

void AsCommonProjector::suMoving() {
	if (_x <= _klayman->getX())
		_x = _klayman->getX() - 100;
	else
		_x = _klayman->getX() + 100;
	moveProjector();
	if (_beforeMoveX == _x) {
		if (getGlobalVar(V_PROJECTOR_SLOT) == 0 && _asProjectorItem->leftBorderLeaves != 0) {
			sendMessage(_parentScene, 0x1019, 0);
			incGlobalVar(V_PROJECTOR_LOCATION, -1);
			setGlobalVar(V_PROJECTOR_SLOT, kAsCommonProjectorItems[getGlobalVar(V_PROJECTOR_LOCATION)].maxSlotCount);
		} else if ((int8)getGlobalVar(V_PROJECTOR_SLOT) == _asProjectorItem->maxSlotCount && _asProjectorItem->rightBorderLeaves != 0) {
			sendMessage(_parentScene, 0x1019, 1);
			incGlobalVar(V_PROJECTOR_LOCATION, +1);
			setGlobalVar(V_PROJECTOR_SLOT, 0);
		}
	}
	Sprite::processDelta();
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

	if (_lockedInSlot && !nowLockedInSlot) {
		_lockedInSlot = false;
	} else if (!_lockedInSlot && nowLockedInSlot) {
		playSound(1, 0x5440E474);
		_lockedInSlot = true;
	}
	
}

void AsCommonProjector::stSuckedIn() {
	AnimatedSprite::updateDeltaXY();
	if (_rect.y1 <= 150) {
		sendMessage(_asPipe, 0x483A, 0);
		stopAnimation();
		SetMessageHandler(&Sprite::handleMessage);
		SetSpriteUpdate(NULL);
		setVisible(false);
	}
}

void AsCommonProjector::stIdle() {
	SetSpriteUpdate(NULL);
	SetMessageHandler(&AsCommonProjector::handleMessage);
	startAnimation(0x10E3042B, 0, -1);
}

void AsCommonProjector::stMoving() {
	_beforeMoveX = getGlobalVar(V_PROJECTOR_SLOT) * 108 + _asProjectorItem->point.x;
	startAnimation(0x14A10137, 0, -1);
	SetSpriteUpdate(&AsCommonProjector::suMoving);
	SetMessageHandler(&AsCommonProjector::handleMessage);
	playSound(1, 0xEC008474);
}

void AsCommonProjector::stStartLockedInSlot() {
	SetSpriteUpdate(NULL);
	SetMessageHandler(&AsCommonProjector::hmAnimation);
	startAnimation(0x80C32213, 0, -1);
	NextState(&AsCommonProjector::stStayLockedInSlot);
}

void AsCommonProjector::stStayLockedInSlot() {
	SetSpriteUpdate(NULL);
	SetMessageHandler(&AsCommonProjector::hmLockedInSlot);
	startAnimation(0xD23B207F, 0, -1);
}

void AsCommonProjector::stStartProjecting() {
	startAnimation(0x50A80517, 0, -1);
	SetMessageHandler(&AsCommonProjector::hmAnimation);
	SetSpriteUpdate(NULL);
	NextState(&AsCommonProjector::stLockedInSlot);
	setGlobalVar(V_PROJECTOR_ACTIVE, 1);
	playSound(0, 0xCC4A8456);
	_vm->_soundMan->addSound(0x05331081, 0xCE428854);
	_vm->_soundMan->playSoundLooping(0xCE428854);
}

void AsCommonProjector::stLockedInSlot() {
	sendMessage(_parentScene, 0x480F, 0);
	startAnimation(0xD833207F, 0, -1);
	SetSpriteUpdate(NULL);
	SetMessageHandler(&AsCommonProjector::hmLockedInSlot);
}

void AsCommonProjector::stStopProjecting() {
	startAnimation(0x50A94417, 0, -1);
	SetSpriteUpdate(NULL);
	SetMessageHandler(&AsCommonProjector::hmAnimation);
	NextState(&AsCommonProjector::stStayLockedInSlot);
	setGlobalVar(V_PROJECTOR_ACTIVE, 0);
	playSound(0, 0xCC4A8456);
	_vm->_soundMan->deleteSound(0xCE428854);
}

void AsCommonProjector::stTurnToFront() {
	_beforeMoveX = getGlobalVar(V_PROJECTOR_SLOT) * 108 + _asProjectorItem->point.x;
	startAnimation(0x22CB4A33, 0, -1);
	SetSpriteUpdate(&AsCommonProjector::suMoving);
	SetMessageHandler(&AsCommonProjector::hmAnimation);
	NextState(&AsCommonProjector::stMoving);
}

void AsCommonProjector::stStartSuckedIn() {
	setGlobalVar(V_PROJECTOR_LOCATION, 4);
	setGlobalVar(V_PROJECTOR_SLOT, 0);
	SetSpriteUpdate(&AsCommonProjector::stSuckedIn);
	SetMessageHandler(&Sprite::handleMessage);
	startAnimation(0x708D4712, 0, -1);
	playSound(2);
}

Scene1401::Scene1401(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _projectorBorderFlag(false), _ssFloorButton(NULL), _asProjector(NULL), 
	_asPipe(NULL), _asMouse(NULL), _asCheese(NULL), _asBackDoor(NULL), 
	_sprite1(NULL), _sprite2(NULL), _sprite3(NULL), _ssButton(NULL) {

	SetMessageHandler(&Scene1401::handleMessage);
	SetUpdateHandler(&Scene1401::update);
	setRectList(0x004B6758);

	setBackground(0x08221FA5);
	setPalette(0x08221FA5);
	insertMouse433(0x21FA108A);
	
	_ssFloorButton = insertSprite<SsCommonFloorButton>(this, 0x980F3124, 0x12192892, 100, 0);
	_asPipe = insertSprite<AsScene1401Pipe>();

	if (!getGlobalVar(V_MOUSE_SUCKED_IN)) {
		_asMouse = insertSprite<AsScene1401Mouse>();
		_asCheese = insertSprite<AsScene1401Cheese>();
	}

	_sprite3 = insertStaticSprite(0xA82BA811, 1100);
	insertStaticSprite(0x0A116C60, 1100);
	_ssButton = insertSprite<SsCommonButtonSprite>(this, 0xB84B1100, 100, 0);
	_sprite1 = insertStaticSprite(0x38EA100C, 1005);
	_sprite2 = insertStaticSprite(0x98D0223C, 1200);
	_sprite2->setVisible(false);

	if (which < 0) {
		insertKlayman<KmScene1401>(380, 447);
		setMessageList(0x004B65C8);
		_sprite1->setVisible(false);
	} else if (which == 1) {
		insertKlayman<KmScene1401>(0, 447);
		setMessageList(0x004B65D0);
		_sprite1->setVisible(false);
	} else if (which == 2) {
		insertKlayman<KmScene1401>(660, 447);
		setMessageList(0x004B65D8);
		_sprite1->setVisible(false);
	} else {
		insertKlayman<KmScene1401>(290, 413);
		setMessageList(0x004B65E8);
		_sprite1->setVisible(false);
	}

	if (getGlobalVar(V_PROJECTOR_LOCATION) == 2) {
		_asProjector = insertSprite<AsCommonProjector>(this, _klayman, _asPipe);
		_vm->_collisionMan->addSprite(_asProjector);
		if (getGlobalVar(V_PROJECTOR_SLOT) == 6) {
			sendEntityMessage(_klayman, 0x1014, _asProjector);
			_klayman->setX(_asProjector->getX() + 100);
			_klayman->processDelta();
			setMessageList(0x004B6670);
		} else if (getGlobalVar(V_PROJECTOR_SLOT) == 0) {
			sendEntityMessage(_klayman, 0x1014, _asProjector);
			_klayman->setX(_asProjector->getX() - 100);
			_klayman->processDelta();
			setMessageList(0x004B6670);
		}
		_asProjector->setClipRect(_sprite3->getDrawRect().x, _sprite2->getDrawRect().y, 640, 480);
	}
	
	_klayman->setClipRect(_sprite3->getDrawRect().x, 0, 640, 480);

	if (which == 0 && _asProjector) {
		sendMessage(_asProjector, 0x482B, 0);
	}

	_asBackDoor = insertSprite<AsScene1401BackDoor>(_klayman, which == 1);

}

void Scene1401::update() {
	Scene::update();
	if (_asProjector && !_projectorBorderFlag && _asProjector->getY() < 360) {
		_sprite2->setVisible(true);
		_projectorBorderFlag = true;
	} else {
		_sprite2->setVisible(false);
	}
}

uint32 Scene1401::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x02144CB1) {
			sendEntityMessage(_klayman, 0x1014, _ssFloorButton);
		} else if (param.asInteger() == 0x402064D8) {
			sendEntityMessage(_klayman, 0x1014, _ssButton);
		} else if (param.asInteger() == 0x01C66840) {
			if (sendMessage(_asBackDoor, 0x2001, 0) != 0) {
				setMessageList(0x004B6690);
			} else {
				setMessageList(0x004B66B0);
			}
		}
		break;
	case 0x1019:
		if (param.asInteger() != 0) {
			leaveScene(2);
		} else {
			leaveScene(1);
		}			
		break;
	case 0x480B:
		if (sender == _ssFloorButton) {
			sendMessage(_asPipe, 0x2000, 0);
			if (!getGlobalVar(V_MOUSE_SUCKED_IN)) {
				sendMessage(_asMouse, 0x4839, 0);
				sendMessage(_asCheese, 0x4839, 0);
				setGlobalVar(V_MOUSE_SUCKED_IN, 1);
			}
			if (_asProjector && _asProjector->getX() > 404 && _asProjector->getX() < 504)
				sendMessage(_asProjector , 0x4839, 0);
		} else if (sender == _ssButton) {
			sendMessage(_ssButton, 0x4808, 0);
		}
		break;
	case 0x4826:
		if (sender == _asProjector) {
			if (sendMessage(_asProjector, 0x480C, _klayman->getX() > _asProjector->getX() ? 1 : 0) != 0) {
				sendEntityMessage(_klayman, 0x1014, _asProjector);
				setMessageList2(0x004B6658);
			} else {
				setMessageList2(0x004B65F0);
			}
		}						
		break;
	case 0x482A:
		_sprite1->setVisible(true);
		if (_asProjector) {
			sendMessage(_asProjector, 0x482B, 0);
		}
		break;
	case 0x482B:
		_sprite1->setVisible(false);
		if (_asProjector) {
			sendMessage(_asProjector, 0x482A, 0);
		}
		break;
	}
	return 0;
}

// Scene1402

SsScene1402BridgePart::SsScene1402BridgePart(NeverhoodEngine *vm, uint32 fileHash, int surfacePriority)
	: StaticSprite(vm, fileHash, surfacePriority) {
	
	SetFilterY(&Sprite::defFilterY);
	SetUpdateHandler(&StaticSprite::update);
	
}

AsScene1402PuzzleBox::AsScene1402PuzzleBox(NeverhoodEngine *vm, Scene *parentScene, int which)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene) {

	createSurface(900, 152, 147);

	SetFilterY(&Sprite::defFilterY);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1402PuzzleBox::handleMessage);
	_x = 279;
	_y = 270;
	if (which == 2) {
		// Puzzle box after the puzzle was solved
		startAnimation(0x20060259, 0, -1);
		NextState(&AsScene1402PuzzleBox::stMoveDownSolvedDone);
		playSound(0, 0x419014AC);
		loadSound(1, 0x61901C29);
	} else if (which == 1) {
		// Puzzle box appears
		startAnimation(0x210A0213, 0, -1);
		NextState(&AsScene1402PuzzleBox::stMoveUpDone);
		playSound(0, 0x41809C6C);
	} else {
		// Puzzle box is here
		startAnimation(0x20060259, 0, -1);
		loadSound(1, 0x61901C29);
		_newStickFrameIndex = STICK_LAST_FRAME;
	}
}

uint32 AsScene1402PuzzleBox::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2002:
		playSound(1);
		startAnimation(0x20060259, -1, -1);
		_playBackwards = true;
		NextState(&AsScene1402PuzzleBox::stMoveDownDone);
		break;
	case 0x3002:
		gotoNextState();
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

Scene1402::Scene1402(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _isShaking(false), _asPuzzleBox(NULL), _asProjector(NULL) {

	SetMessageHandler(&Scene1402::handleMessage);

	setBackground(0x231482F0);
	setBackgroundY(-10);
	// TODO g_screen->field_26 = 0;
	setPalette(0x231482F0);
	_palette->addPalette(0x91D3A391, 0, 64, 0);
	insertMouse433(0x482F4239);

	_ssBridgePart1 = insertSprite<SsScene1402BridgePart>(0x15402D64, 1100);
	_ssBridgePart2 = insertSprite<SsScene1402BridgePart>(0x10A02120, 1100);
	_ssBridgePart3 = insertSprite<SsScene1402BridgePart>(0x60882BE0, 1100);

	if (getGlobalVar(V_MOUSE_PUZZLE_SOLVED))
		setRectList(0x004B0C48);
	else
		setRectList(0x004B0C98);

	if (which < 0) {
		insertKlayman<KmScene1402>(377, 391);
		setMessageList(0x004B0B48);
		if (!getGlobalVar(V_MOUSE_PUZZLE_SOLVED)) {
			_asPuzzleBox = insertSprite<AsScene1402PuzzleBox>(this, 0);
		}
	} else if (which == 1) {
		insertKlayman<KmScene1402>(42, 391);
		setMessageList(0x004B0B50);
	} else if (which == 2) {
		insertKlayman<KmScene1402>(377, 391);
		setMessageList(0x004B0B60);
		_klayman->setDoDeltaX(1);
		if (getGlobalVar(V_MOUSE_PUZZLE_SOLVED)) {
			_asPuzzleBox = insertSprite<AsScene1402PuzzleBox>(this, 1);
			clearRectList();
			showMouse(false);
			startShaking();
		} else {
			_asPuzzleBox = insertSprite<AsScene1402PuzzleBox>(this, 0);
		}
	} else {
		insertKlayman<KmScene1402>(513, 391);
		setMessageList(0x004B0B58);
		if (!getGlobalVar(V_MOUSE_PUZZLE_SOLVED)) {
			_asPuzzleBox = insertSprite<AsScene1402PuzzleBox>(this, 2);
			startShaking();
		}
	}

	if (_asPuzzleBox)
		_asPuzzleBox->setClipRect(0, 0, 640, _ssBridgePart3->getDrawRect().y2());

	if (getGlobalVar(V_PROJECTOR_LOCATION) == 1) {
		_asProjector = insertSprite<AsCommonProjector>(this, _klayman, (Sprite*)NULL);
		_vm->_collisionMan->addSprite(_asProjector);
		if (getGlobalVar(V_PROJECTOR_SLOT) == 4) {
			sendEntityMessage(_klayman, 0x1014, _asProjector);
			_klayman->setX(_asProjector->getX() + 100);
			_klayman->processDelta();
			setMessageList(0x004B0BD0);
		} else if (getGlobalVar(V_PROJECTOR_SLOT) == 0) {
			sendEntityMessage(_klayman, 0x1014, _asProjector);
			_klayman->setX(_asProjector->getX() - 100);
			_klayman->processDelta();
			setMessageList(0x004B0BD0);
		}
		_asProjector->setClipRect(_ssBridgePart1->getDrawRect().x, 0, _ssBridgePart2->getDrawRect().x, _ssBridgePart3->getDrawRect().y2());
	}

	_klayman->setClipRect(_ssBridgePart1->getDrawRect().x, 0, _ssBridgePart2->getDrawRect().x2(), _ssBridgePart3->getDrawRect().y2());
	
}

void Scene1402::upShaking() {
	if (_isShaking) {
		setBackgroundY(_vm->_rnd->getRandomNumber(10 - 1) - 10);
		// TODO g_screen->field_26 = -10 - _background->getDrawRect().y;
	} else {
		setBackgroundY(-10);
		// TODO g_screen->field_26 = 0;
		SetUpdateHandler(&Scene::update);
	}
	Scene::update();
	if (_asPuzzleBox)
		_asPuzzleBox->setClipRect(0, 0, 640, _ssBridgePart3->getDrawRect().y2());
	_klayman->setClipRect(_ssBridgePart1->getDrawRect().x, 0, _ssBridgePart2->getDrawRect().x2(), _ssBridgePart3->getDrawRect().y2());
}

uint32 Scene1402::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x00F43389) {
			if (getGlobalVar(V_MOUSE_PUZZLE_SOLVED)) {
				leaveScene(0);
			} else {
				clearRectList();
				_klayman->setVisible(false);
				showMouse(false);
				sendMessage(_asPuzzleBox, 0x2002, 0);
				startShaking();
			}
		}
		break;
	case 0x1019:
		if (param.asInteger()) {
			leaveScene(0);
		} else {
			leaveScene(1);
		}
		break;
	case 0x2000:
		stopShaking();
		showMouse(true);
		setRectList(0x004B0C48);
		break;
	case 0x2001:
		stopShaking();
		leaveScene(0);
		break;
	case 0x2003:
		stopShaking();
		break;
	case 0x4826:
		if (sender == _asProjector) {
			if (sendMessage(_asProjector, 0x408C, _klayman->getX() > _asProjector->getX() ? 1 : 0) != 0) {
				sendEntityMessage(_klayman, 0x1014, _asProjector);
				setMessageList2(0x004B0BB8);
			} else {
				setMessageList2(0x004B0B68);
			}
		}
	}
	return 0;
}

void Scene1402::startShaking() {
	_isShaking = true;
	SetUpdateHandler(&Scene1402::upShaking);
}

void Scene1402::stopShaking() {
	_isShaking = false;
}

// Scene1407

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

	SetUpdateHandler(&AnimatedSprite::update);
	_x = 108;
	_y = 106;
	stIdleLookAtGoodHole();
}

void AsScene1407Mouse::suWalkTo() {
	int16 xdelta = _walkDestX - _x;
	if (xdelta > _deltaX)
		xdelta = _deltaX;
	else if (xdelta < -_deltaX)
		xdelta = -_deltaX;
	_deltaX = 0;		
	if (_walkDestX == _x) {
		sendMessage(this, 0x1019, 0);
	} else {
		_x += xdelta;
		processDelta();
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
	case 0x0001:
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
				if (mouseX < kScene1407MouseSections[_currSectionIndex].x1) {
					_walkDestX = kScene1407MouseSections[_currSectionIndex].x1;
				} else if (mouseX > kScene1407MouseSections[_currSectionIndex].x2) {
					_walkDestX = kScene1407MouseSections[_currSectionIndex].x2;
				} else {
					_walkDestX = mouseX;
				}
				stWalkToDest();
			}
		}
		break;
	case 0x1019:
		gotoNextState();
		break;
	case 0x2001:
		{
			// Reset the position
			// Find the nearest hole and go through it, and exit at the first hole
			int16 distance = 640;
			int matchIndex = 50;
			for (int index = 0; index < 50; index++) {
				if (kScene1407MouseHoles[index].sectionIndex == _currSectionIndex) {
					if (ABS(kScene1407MouseHoles[index].x - _x) < distance) {
						matchIndex = index;
						distance = ABS(kScene1407MouseHoles[index].x - _x);
					}
				}
			}
			if (matchIndex < 50) {
				_nextHoleIndex = 0;
				_walkDestX = kScene1407MouseHoles[matchIndex].x;
				stWalkToHole();
			}
		}
		break;
	}
	return messageResult;
}

void AsScene1407Mouse::stIdleLookAtGoodHole() {
	setDoDeltaX(kScene1407MouseHoles[kScene1407MouseSections[_currSectionIndex].goodHoleIndex].x < _x ? 1 : 0);
	startAnimation(0x72215194, 0, -1);
	SetSpriteUpdate(NULL);
	SetMessageHandler(&AsScene1407Mouse::handleMessage);
}

void AsScene1407Mouse::stWalkToDest() {
	if (_walkDestX != _x) {
		setDoDeltaX(_walkDestX < _x ? 1 : 0);
		startAnimation(0x22291510, 0, -1);
		SetSpriteUpdate(&AsScene1407Mouse::suWalkTo);
		SetMessageHandler(&AsScene1407Mouse::handleMessage);
		NextState(&AsScene1407Mouse::stIdleLookAtGoodHole);
	}
}

void AsScene1407Mouse::stWalkToHole() {
	setDoDeltaX(_walkDestX < _x ? 1 : 0);
	startAnimation(0x22291510, 0, -1);
	SetSpriteUpdate(&AsScene1407Mouse::suWalkTo);
	SetMessageHandler(&AsScene1407Mouse::handleMessage);
	NextState(&AsScene1407Mouse::stGoThroughHole);
}

void AsScene1407Mouse::stGoThroughHole() {
	startAnimation(0x72215194, 0, -1);
	SetSpriteUpdate(NULL);
	SetMessageHandler(NULL);
	SetUpdateHandler(&AsScene1407Mouse::upGoThroughHole);
	NextState(&AsScene1407Mouse::stArriveAtHole);
	setVisible(false);
	_countdown = 12;
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

Scene1407::Scene1407(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _puzzleSolvedCountdown(0),	_resetButtonCountdown(0) {

	SetMessageHandler(&Scene1407::handleMessage);
	SetUpdateHandler(&Scene1407::update);

	setBackground(0x00442225);
	setPalette(0x00442225);
	insertMouse435(0x4222100C, 20, 620);

	_asMouse = insertSprite<AsScene1407Mouse>(this);
	_ssResetButton = insertStaticSprite(0x12006600, 100);
	_ssResetButton->setVisible(false); 

}

void Scene1407::update() {
	Scene::update();
	if (_puzzleSolvedCountdown != 0 && (--_puzzleSolvedCountdown == 0)) {
		leaveScene(1);
	} else if (_resetButtonCountdown != 0 && (--_resetButtonCountdown == 0)) {
		_ssResetButton->setVisible(false);
	}
}

uint32 Scene1407::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		if (_puzzleSolvedCountdown == 0) {
			if (param.asPoint().x <= 20 || param.asPoint().x >= 620) {
				// Exit scene
				leaveScene(0);
			} else if (param.asPoint().x >= 75 && param.asPoint().x <= 104 &&
				param.asPoint().y >= 62 && param.asPoint().y <= 90) {
				// The reset button was clicked
				sendMessage(_asMouse, 0x2001, 0);
				_ssResetButton->setVisible(true);
				playSound(0, 0x44045000);
				_resetButtonCountdown = 12;
			} else {
				// Handle the mouse
				sendMessage(_asMouse, messageNum, param);
			}
		}
		break;
	case 0x2000:
		// The mouse got the cheese (nomnom)
		setGlobalVar(V_MOUSE_PUZZLE_SOLVED, 1);
		playSound(0, 0x68E25540);
		showMouse(false);
		_puzzleSolvedCountdown = 72;
		break;
	}
	return 0;
}

// Scene1403

Scene1403::Scene1403(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _asProjector(NULL), _flag(false) {
	
	SetMessageHandler(&Scene1403::handleMessage);
	
	setRectList(0x004B1FF8);

	setBackground(0x2110A234);
	setPalette(0x2110A234);
	insertMouse433(0x0A230219);

	_class401_1 = insertStaticSprite(0x01102A33, 100);
	_class401_1->setVisible(false);

	_class401_2 = insertStaticSprite(0x04442520, 995);
	
	_class401_3 = insertStaticSprite(0x08742271, 995);

	_asTape1 = insertSprite<AsScene1201Tape>(this, 12, 1100, 201, 468, 0x9148A011);
	_vm->_collisionMan->addSprite(_asTape1);
	_asTape1->setRepl(64, 0);

	_asTape2 = insertSprite<AsScene1201Tape>(this, 16, 1100, 498, 468, 0x9048A093);
	_vm->_collisionMan->addSprite(_asTape2);
	_asTape2->setRepl(64, 0);

	if (which < 0) {
		insertKlayman<KmScene1402>(380, 463);
		setMessageList(0x004B1F18);
	} else {
		insertKlayman<KmScene1402>(640, 463);
		setMessageList(0x004B1F20);
	}
	_klayman->setRepl(64, 0);

	if (getGlobalVar(V_PROJECTOR_LOCATION) == 4) {
		_asProjector = insertSprite<AsCommonProjector>(this, _klayman, (Sprite*)NULL);
		_vm->_collisionMan->addSprite(_asProjector);
		if (getGlobalVar(V_PROJECTOR_SLOT) == 4) {
			sendEntityMessage(_klayman, 0x1014, _asProjector);
			_klayman->setX(_asProjector->getX() + 100);
			_klayman->processDelta();
			setMessageList(0x004B1F70);
		}
		_asProjector->setClipRect(0, 0, 640, _class401_2->getDrawRect().y2());
		_asProjector->setRepl(64, 0);
	}

}

uint32 Scene1403::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x88C11390) {
			setRectList(0x004B2008);
			_flag = true;
		} else if (param.asInteger() == 0x08821382) {
			sendEntityMessage(_klayman, 0x1014, _asProjector);
			setRectList(0x004B1FF8);
			_flag = false;
		}
		break;
	case 0x1019:
		leaveScene(0);
		break;
	case 0x1022:
		if (sender == _asProjector) {
			if (param.asInteger() >= 1000) {
				setSurfacePriority(_class401_3->getSurface(), 1100);
			} else {
				setSurfacePriority(_class401_3->getSurface(), 995);
			}
		}
		break;
	case 0x4807:
		_class401_1->setVisible(false);
		break;
	case 0x480F:
		_class401_1->setVisible(true);
		break;
	case 0x4826:
		if (sender == _asProjector) {
			if (_flag) {
				setMessageList2(0x004B1FA8);
			} else if (param.asInteger() == 1) {
				sendEntityMessage(_klayman, 0x1014, _asProjector);
				setMessageList2(0x004B1F88);
			} else if (sendMessage(_asProjector, 0x480C, _klayman->getX() > _asProjector->getX() ? 1 : 0) != 0) {
				sendEntityMessage(_klayman, 0x1014, _asProjector);
				setMessageList2(0x004B1F58);
			} else {
				setMessageList2(0x004B1F28);
			}
		} else if (sender == _asTape1 || sender == _asTape2) {
			if (_flag) {
				setMessageList2(0x004B1FA8);
			} else if (_messageListStatus != 2) {
				sendEntityMessage(_klayman, 0x1014, sender);
				setMessageList2(0x004B1FB8);
			}
		}
		break;
	}
	return 0;
}

// Scene1404

Scene1404::Scene1404(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _asProjector(NULL), _asKey(NULL) {
	
	if (getGlobalVar(V_HAS_FINAL_KEY) && getGlobalVar(V_KEY3_LOCATION) == 0)
		setGlobalVar(V_KEY3_LOCATION, 5);
	
	SetMessageHandler(&Scene1404::handleMessage);

	setRectList(0x004B8D80);

	setBackground(0xAC0B006F);
	setPalette(0xAC0B006F);
	_palette->addPalette(0x00801510, 0, 65, 0);
	insertMouse433(0xB006BAC8);

	if (getGlobalVar(V_KEY3_LOCATION) == 5) {
		_asKey = insertSprite<AsCommonKey>(this, 2, 1100, 267, 411);
		_vm->_collisionMan->addSprite(_asKey);
	}

	_sprite1 = insertStaticSprite(0x1900A1F8, 1100);

	_asTape = insertSprite<AsScene1201Tape>(this, 14, 1100, 281, 411, 0x9148A011);
	_vm->_collisionMan->addSprite(_asTape);

	if (which < 0) {
		insertKlayman<KmScene1404>(376, 406);
		setMessageList(0x004B8C28);
	} else if (which == 1) {
		insertKlayman<KmScene1404>(376, 406);
		setMessageList(0x004B8C30);
	} else if (which == 2) {
		if (getGlobalVar(V_KLAYMAN_IS_DELTA_X)) {
			insertKlayman<KmScene1404>(347, 406);
			_klayman->setDoDeltaX(1);
		} else {
			insertKlayman<KmScene1404>(187, 406);
		}
		setMessageList(0x004B8D28);
	} else {
		insertKlayman<KmScene1404>(30, 406);
		setMessageList(0x004B8C38);
	}

	if (getGlobalVar(V_PROJECTOR_LOCATION) == 3) {
		_asProjector = insertSprite<AsCommonProjector>(this, _klayman, (Sprite*)NULL);
		_vm->_collisionMan->addSprite(_asProjector);
		if (getGlobalVar(V_PROJECTOR_SLOT) == 0) {
			sendEntityMessage(_klayman, 0x1014, _asProjector);
			_klayman->setX(_asProjector->getX() - 100);
			_klayman->processDelta();
			setMessageList(0x004B8CB8);
		}
		_asProjector->setClipRect(_sprite1->getDrawRect().x, 0, 640, 480);
	}

	_klayman->setClipRect(_sprite1->getDrawRect().x, 0, 640, 480);

}

Scene1404::~Scene1404() {
	setGlobalVar(V_KLAYMAN_IS_DELTA_X, _klayman->isDoDeltaX() ? 1 : 0);
}

uint32 Scene1404::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x410650C2) {
			if (_asProjector && _asProjector->getX() == 220) {
				setMessageList(0x004B8C40);
			} else {
				setMessageList(0x004B8CE8);
			}
		}
		break;
	case 0x1019:
		leaveScene(0);
		break;
	case 0x4826:
		if (sender == _asProjector) {
			if (sendMessage(_asProjector, 0x480C, _klayman->getX() > _asProjector->getX() ? 1 : 0) != 0) {
				sendEntityMessage(_klayman, 0x1014, _asProjector);
				setMessageList2(0x004B8CA0);
			} else {
				setMessageList2(0x004B8C40);
			}
		} else if (sender == _asTape && _messageListStatus != 2) {
			sendEntityMessage(_klayman, 0x1014, _asTape);
			setMessageList(0x004B8CD0);
		} else if (sender == _asKey && _messageListStatus != 2) {
			sendEntityMessage(_klayman, 0x1014, _asKey);
			setMessageList(0x004B8D18);
		}
		break;
	}
	return 0;
}

// Scene1405

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
	SetUpdateHandler(&AsScene1405Tile::update);
	SetMessageHandler(&AsScene1405Tile::handleMessage);
	
	debug("getSubVar(VA_TILE_SYMBOLS, _tileIndex) = %d", getSubVar(VA_TILE_SYMBOLS, _tileIndex));
	
	startAnimation(0x844B805C, getSubVar(VA_TILE_SYMBOLS, _tileIndex), -1);
	_newStickFrameIndex = (int16)getSubVar(VA_TILE_SYMBOLS, _tileIndex);
}

void AsScene1405Tile::update() {
	updateAnim();
	updatePosition();
	if (_countdown != 0 && (--_countdown == 0)) {
		show();
	}
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

void AsScene1405Tile::hide() {
	if (_isShowing) {
		_isShowing = false;
		playSound(0);
		setVisible(false);
	}
}

Scene1405::Scene1405(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _selectFirstTile(true), _tilesLeft(48), _countdown(0) {

	_vm->gameModule()->initMemoryPuzzle();
	
	setBackground(0x0C0C007D);
	setPalette(0x0C0C007D);
	insertMouse435(0xC00790C8, 20, 620);
	
	for (uint32 tileIndex = 0; tileIndex < 48; tileIndex++) {
		_tiles[tileIndex] = insertSprite<AsScene1405Tile>(this, tileIndex);
		_vm->_collisionMan->addSprite(_tiles[tileIndex]);
		if (getSubVar(VA_IS_TILE_MATCH, tileIndex))
			_tilesLeft--;
	}
	
	loadSound(0, 0x68E25540);
	
	SetMessageHandler(&Scene1405::handleMessage);
	SetUpdateHandler(&Scene1405::update);

}

void Scene1405::update() {
	Scene::update();
	if (_countdown != 0 && (--_countdown == 0)) {
		_tilesLeft = 48;
		_tiles[_firstTileIndex]->hide();
		_tiles[_secondTileIndex]->hide();
		for (uint32 i = 0; i < 48; i++) {
			if (getSubVar(VA_IS_TILE_MATCH, i)) {
				_tiles[i]->hide();
				setSubVar(VA_IS_TILE_MATCH, i, 0);
			}
		}
	}
}

uint32 Scene1405::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		if (param.asPoint().x <= 20 || param.asPoint().x >= 620) {
			leaveScene(0);
		}
		break;
	case 0x2000:
		if (_selectFirstTile) {
			_firstTileIndex = param.asInteger();
			_selectFirstTile = false;
		} else {
			_secondTileIndex = param.asInteger();
			if (_firstTileIndex != _secondTileIndex) {
				_selectFirstTile = true;
				if (getSubVar(VA_TILE_SYMBOLS, _secondTileIndex) == getSubVar(VA_TILE_SYMBOLS, _firstTileIndex)) {
					setSubVar(VA_IS_TILE_MATCH, _firstTileIndex, 1);
					setSubVar(VA_IS_TILE_MATCH, _secondTileIndex, 1);
					_tilesLeft -= 2;
					if (_tilesLeft == 0) {
						playSound(0);
					}
				} else {
					_countdown = 10;
				}
			}
		}
		break;
	}
	return 0;
}

} // End of namespace Neverhood
