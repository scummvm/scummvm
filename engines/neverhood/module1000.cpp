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

#include "neverhood/module1000.h"

namespace Neverhood {

Module1000::Module1000(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {
	
	debug("Create Module1000(%d)", which);

	_musicFileHash = getGlobalVar(0xD0A14D10) ? 0x81106480 : 0x00103144;		

	// TODO Music18hList_add(0x03294419, 0x061880C6);
	// TODO Music18hList_add(0x03294419, _musicFileHash);

	if (which < 0) {
		createScene(_vm->gameState().sceneNum, -1);
	} else if (which == 0) {
		createScene(0, 0);
	} else if (which == 1) {
		createScene(1, 1);
	}

}

Module1000::~Module1000() {
	// TODO Music18hList_deleteGroup(0x03294419);
}

void Module1000::createScene(int sceneNum, int which) {
	debug("Module1000::createScene(%d, %d)", sceneNum, which);
	_vm->gameState().sceneNum = sceneNum;
	switch (_vm->gameState().sceneNum) {
	case 0:
		// TODO Music18hList_play(0x061880C6, 0, 0, 1);
		_childObject = new Scene1001(_vm, this, which);
		break;
	case 1:
		// TODO Music18hList_play(0x061880C6, 0, 0, 1);
		_childObject = new Scene1002(_vm, this, which);
		break;
	case 2:
		// TODO Music18hList_play(0x061880C6, 0, 0);
		_childObject = new Class152(_vm, this, 0xC084110C, 0x41108C00);
		break;
	case 3:
		// TODO Music18hList_stop(0x061880C6, 0, 2);
		_childObject = new Scene1004(_vm, this, which);
		break;
	case 4:
		// TODO Music18hList_stop(0x061880C6, 0, 0);
		// TODO Music18hList_play(_musicFileHash, 0, 0, 1);
		_childObject = new Scene1005(_vm, this, which);
		break;
	}
	SetUpdateHandler(&Module1000::updateScene);
	_childObject->handleUpdate();
}

void Module1000::updateScene() {
	if (!updateChild()) {
		switch (_vm->gameState().sceneNum) {
		case 0:
			if (_moduleResult == 2)
				createScene(2, 0);
			else
				createScene(1, 0);
			break;
		case 1:
			if (_moduleResult == 1)
				leaveModule(0);
			else if (_moduleResult == 2)
				createScene(3, 0);
			else
				createScene(0, 1);
			break;
		case 2:
			createScene(0, 2);
			break;
		case 3:
			if (_moduleResult == 1)
				createScene(4, 0);
			else
				createScene(1, 2);
			break;
		case 4:
			// TODO Music18hList_stop(_musicFileHash, 0, 1);
			createScene(3, 1);
			break;
		}
	}
}

// Scene1001			

AsScene1001Door::AsScene1001Door(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1100), _soundResource1(vm), _soundResource2(vm) {
	
	createSurface(800, 137, 242);
	_x = 726;
	_y = 440;
	stShowIdleDoor();
	_soundResource2.load(0xED403E03);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1001Door::handleMessage);
}

uint32 AsScene1001Door::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2000:
		hammerHitsDoor();
		break;
	case 0x3002:
		gotoNextState();
		break;
	}
	return 0;
}

void AsScene1001Door::hammerHitsDoor() {
	switch (getGlobalVar(0x52371C95)) {
	case 0:
	case 1:
		_soundResource1.play(0x65482F03);
		startAnimation(0x624C0498, 1, 3);
		NextState(&AsScene1001Door::stShowIdleDoor);		
		break;
	case 2:
		_soundResource2.play();
		startAnimation(0x624C0498, 6, 6);
		NextState(&AsScene1001Door::stBustedDoorMove);		
		break;
	default:
		// Nothing
		break;		
	}
	incGlobalVar(0x52371C95, 1);
}

void AsScene1001Door::stShowIdleDoor() {
	switch (getGlobalVar(0x52371C95)) {
	case 1:
		startAnimation(0x624C0498, 4, -1);
		_newStickFrameIndex = 4;
		break;
	case 2:
		startAnimation(0x624C0498, 1, -1);
		_newStickFrameIndex = 1;
		break;
	case 3:
		stopAnimation();
		setVisible(false);
		break;
	default:
		startAnimation(0x624C0498, 0, -1);
		_newStickFrameIndex = 0;
		break;
	}
}

void AsScene1001Door::stBustedDoorMove() {
	setGlobalVar(0xD217189D, 1);
	startAnimation(0x624C0498, 6, 6);
	NextState(&AsScene1001Door::stBustedDoorGone);
	_x = 30;
}

void AsScene1001Door::stBustedDoorGone() {
	_soundResource1.play();
	stopAnimation();
	setVisible(false);	
}
	
AsScene1001Hammer::AsScene1001Hammer(NeverhoodEngine *vm, Sprite *asDoor)
	: AnimatedSprite(vm, 1100), _soundResource(vm), _asDoor(asDoor) {

	_x = 547;
	_y = 206;
	createSurface(900, 177, 192);
	startAnimation(0x022C90D4, -1, -1);
	_newStickFrameIndex = -2;
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1001Hammer::handleMessage);
}

uint32 AsScene1001Hammer::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x00352100) {
			sendMessage(_asDoor, 0x2000, 0);
		} else if (param.asInteger() == 0x0A1A0109) {
			_soundResource.play(0x66410886);
		}
		break;
	case 0x2000:
		startAnimation(0x022C90D4, 1, -1);
		_soundResource.play(0xE741020A);
		_newStickFrameIndex = -2;
		break;
	}
	return 0;
}

AsScene1001Window::AsScene1001Window(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1200), _soundResource(vm) {

	_x = 320;
	_y = 240;
	createSurface(100, 66, 129);
	startAnimation(0xC68C2299, 0, -1);
	_newStickFrameIndex = 0;
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1001Window::handleMessage);
}

uint32 AsScene1001Window::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x0E0A1410) {
			_soundResource.play(0x60803F10);
		}
		break;
	case 0x2001:
		startAnimation(0xC68C2299, 0, -1);
		break;
	case 0x3002:
		SetMessageHandler(NULL);
		setGlobalVar(0x03C698DA, 1);
		setVisible(false);
		break;
	}
	return 0;
}

AsScene1001Lever::AsScene1001Lever(NeverhoodEngine *vm, Scene *parentScene, int16 x, int16 y, int deltaXType)
	: AnimatedSprite(vm, 1100), _soundResource(vm), _parentScene(parentScene) {
	
	createSurface(1010, 71, 73);
	setDoDeltaX(deltaXType);
	startAnimation(0x04A98C36, 0, -1);
	_newStickFrameIndex = 0;
	_x = x;
	_y = y;
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1001Lever::handleMessage);
}

uint32 AsScene1001Lever::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x00C0C444) {
			sendMessage(_parentScene, 0x480F, 0);
		} else if (param.asInteger() == 0xC41A02C0) {
			_soundResource.play(0x40581882);
		}
		break;
	case 0x1011:
		sendMessage(_parentScene, 0x4826, 0);
		messageResult = 1;
		break;
	case 0x3002:
		startAnimation(0x04A98C36, 0, -1);
		_newStickFrameIndex = 0;
		break;
	case 0x480F:
		startAnimation(0x04A98C36, 0, -1);
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
	
SsCommonButtonSprite::SsCommonButtonSprite(NeverhoodEngine *vm, Scene *parentScene, uint32 fileHash, int surfacePriority, uint32 soundFileHash)
	: StaticSprite(vm, fileHash, surfacePriority), _parentScene(parentScene), _soundResource(vm), _countdown(0) {

	_priority = 1100;
	_soundFileHash = soundFileHash ? soundFileHash : 0x44141000; 
	setVisible(false);
	SetUpdateHandler(&SsCommonButtonSprite::update);
	SetMessageHandler(&SsCommonButtonSprite::handleMessage);
}
	
void SsCommonButtonSprite::update() {
	if (_countdown != 0 && (--_countdown) == 0) {
		setVisible(false);
	}
}
	
uint32 SsCommonButtonSprite::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x480B:
		sendMessage(_parentScene, 0x480B, 0);
		setVisible(true);
		_countdown = 8;
		_soundResource.play(_soundFileHash);
		break;
	}
	return messageResult;
}
		
Scene1001::Scene1001(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true) {

	_name = "Scene1001";

	Sprite *tempSprite;

	SetMessageHandler(&Scene1001::handleMessage);
	
	setHitRects(0x004B4860);
	_surfaceFlag = false;
	setBackground(0x4086520E);
	setPalette(0x4086520E);
	insertMouse433(0x6520A400);
	
	if (which < 0) {
		setRectList(0x004B49F0);
		insertKlayman<KmScene1001>(200, 433);
		setMessageList(0x004B4888);
	} else if (which == 1) {
		setRectList(0x004B49F0);
		insertKlayman<KmScene1001>(640, 433);
		setMessageList(0x004B4898);
	} else if (which == 2) {
		setRectList(0x004B49F0);
		if (getGlobalVar(0xC0418A02)) {
			insertKlayman<KmScene1001>(390, 433);
			_klayman->setDoDeltaX(1);
		} else {
			insertKlayman<KmScene1001>(300, 433);
		}
		setMessageList(0x004B4970);
	} else {
		setRectList(0x004B4A00);
		insertKlayman<KmScene1001>(200, 433);
		setMessageList(0x004B4890);
	}

	tempSprite = insertStaticSprite(0x2080A3A8, 1300);

	_klayman->setClipRect(0, 0, tempSprite->getDrawRect().x2(), 480);
	
	if (getGlobalVar(0xD217189D) == 0) {
		_asDoor = insertSprite<AsScene1001Door>();
		_asDoor->setClipRect(0, 0, tempSprite->getDrawRect().x2(), 480);
	} else {
		_asDoor = NULL;
	}

	_asLever = insertSprite<AsScene1001Lever>(this, 150, 433, 1);

	insertStaticSprite(0x809861A6, 950);
	insertStaticSprite(0x89C03848, 1100);

	_ssButton = insertSprite<SsCommonButtonSprite>(this, 0x15288120, 100, 0);

	if (getGlobalVar(0x03C698DA) == 0) {
		tempSprite = insertStaticSprite(0x8C066150, 200);
		_asWindow = insertSprite<AsScene1001Window>();
		_asWindow->setClipRect(tempSprite->getDrawRect());
	} else {
		_asWindow = NULL;
	}

	_asHammer = insertSprite<AsScene1001Hammer>(_asDoor);

}

Scene1001::~Scene1001() {
	setGlobalVar(0xC0418A02, _klayman->isDoDeltaX());
}

uint32 Scene1001::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	debug("Scene1001::handleMessage(%04X)", messageNum);
	uint32 messageResult = 0;
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		if (param.asPoint().x == 0 && getGlobalVar(0xA4014072)) {
			leaveScene(0);
		}
		break;
	case 0x000D:
		if (param.asInteger() == 0x188B2105) {
			leaveScene(0);
			messageResult = 1;
		}
		break;
	case 0x100D:
		if (param.asInteger() == 0x00342624) {
			sendEntityMessage(_klayman, 0x1014, _asLever);
			setMessageList2(0x004B4910);
			messageResult = 1;
		} else if (param.asInteger() == 0x21E64A00) {
			if (getGlobalVar(0xD217189D)) {
				setMessageList(0x004B48A8);
			} else {
				setMessageList(0x004B48C8);
			}
			messageResult = 1;
		} else if (param.asInteger() == 0x040424D0) {
			sendEntityMessage(_klayman, 0x1014, _ssButton);
		} else if (param.asInteger() == 0x80006358) {
			if (getGlobalVar(0x03C698DA)) {
				setMessageList(0x004B4938);
			} else {
				setMessageList(0x004B4960);
			}
		}
		break;
	case 0x2002:
		setRectList(0x004B49F0);
		break;
	case 0x480B:
		if (_asWindow) {
			sendMessage(_asWindow, 0x2001, 0);
		}
		break;
	case 0x480F:
		if (_asHammer) {
			sendMessage(_asHammer, 0x2000, 0);
		}
		break;
	}
	return messageResult;
}

// Scene1002

SsScene1002LadderArch::SsScene1002LadderArch(NeverhoodEngine *vm, Scene *parentScene)
	: StaticSprite(vm, 0x152C1313, 1015), _parentScene(parentScene) {

	SetMessageHandler(&SsScene1002LadderArch::handleMessage);
}

uint32 SsScene1002LadderArch::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x482A:
		sendMessage(_parentScene, 0x1022, 995);
		break;
	case 0x482B:
		sendMessage(_parentScene, 0x1022, 1015);
		break;
	}
	return messageResult;
}

Class599::Class599(NeverhoodEngine *vm, Scene *parentScene)
	: StaticSprite(vm, 0x316C4BB4, 1015), _parentScene(parentScene) {

	SetMessageHandler(&Class599::handleMessage);
}

uint32 Class599::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x482A:
		sendMessage(_parentScene, 0x1022, 995);
		break;
	case 0x482B:
		sendMessage(_parentScene, 0x1022, 1015);
		break;
	}
	return messageResult;
}

AsScene1002Ring::AsScene1002Ring(NeverhoodEngine *vm, Scene *parentScene, bool flag1, int16 x, int16 y, int16 clipY1, bool flag2)
	: AnimatedSprite(vm, 1100), _soundResource(vm), _parentScene(parentScene), _flag1(flag1) {

	SetUpdateHandler(&AsScene1002Ring::update);
	
	if (flag1) {
		createSurface(990, 68, 314);
		if (flag2) {
			startAnimation(0x04103090, 0, -1);
			SetMessageHandler(&AsScene1002Ring::hmRingHangingLow);
		} else {
			startAnimation(0xA85C4011, _vm->_rnd->getRandomNumber(15), -1);
			SetMessageHandler(&AsScene1002Ring::hmRingIdle);
		}
	} else {
		createSurface(990, 68, 138);
		startAnimation(0xA85C4011, _vm->_rnd->getRandomNumber(15), -1);
		SetMessageHandler(&AsScene1002Ring::hmRingIdle);
	}

	setClipRect(0, clipY1, 640, 480);

	_x = x;
	_y = y;

	setDoDeltaX(_vm->_rnd->getRandomNumber(1));

}

void AsScene1002Ring::update() {
	AnimatedSprite::updateAnim();
	AnimatedSprite::updatePosition();
}

uint32 AsScene1002Ring::hmRingIdle(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x4806:
		setDoDeltaX(((Sprite*)sender)->isDoDeltaX() ? 1 : 0);
		sendMessage(_parentScene, 0x4806, 0);
		SetMessageHandler(&AsScene1002Ring::hmRingPulled1);
		if (_flag1) {
			startAnimation(0x87502558, 0, -1);
		} else {
			startAnimation(0x80DD4010, 0, -1);
		}
		break;
	case 0x480F:
		setDoDeltaX(((Sprite*)sender)->isDoDeltaX() ? 1 : 0);
		sendMessage(_parentScene, 0x480F, 0);
		SetMessageHandler(&AsScene1002Ring::hmRingPulled2);
		startAnimation(0x861A2020, 0, -1);
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

uint32 AsScene1002Ring::hmRingPulled1(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		if (_flag1) {
			startAnimation(0x78D0A812, 0, -1);
			SetMessageHandler(&AsScene1002Ring::hmRingHangingLow);
		} else {
			startAnimation(0xB85D2A10, 0, -1);
			SetMessageHandler(&AsScene1002Ring::hmRingHangingLow);
		}
		break;
	case 0x4807:
		sendMessage(_parentScene, 0x4807, 0);
		setDoDeltaX(_vm->_rnd->getRandomNumber(1));
		startAnimation(0x8258A030, 0, -1);
		SetMessageHandler(&AsScene1002Ring::hmRingReleased);
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

uint32 AsScene1002Ring::hmRingPulled2(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		startAnimation(0x04103090, 0, -1);
		SetMessageHandler(&AsScene1002Ring::hmRingHangingLow);
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

uint32 AsScene1002Ring::hmRingHangingLow(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x4807:
		sendMessage(_parentScene, 0x4807, 0);
		setDoDeltaX(_vm->_rnd->getRandomNumber(1));
		startAnimation(0x8258A030, 0, -1);
		SetMessageHandler(&AsScene1002Ring::hmRingReleased);
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

uint32 AsScene1002Ring::hmRingReleased(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = hmRingIdle(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x05410F72) {
			_soundResource.play(0x21EE40A9);
		}
		break;
	case 0x3002:
		startAnimation(0xA85C4011, 0, -1);
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

AsScene1002Door::AsScene1002Door(NeverhoodEngine *vm, NRect &clipRect)
	: StaticSprite(vm, 1200) {
	
	_spriteResource.load2(0x1052370F);
	createSurface(800, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	setClipRect(clipRect);

	_x = 526;
	
	if (getGlobalVar(0x8306F218)) {
		_y = 49; 
	} else {
		_y = 239; 
	}

	_surface->getDrawRect().x = 0;
	_surface->getDrawRect().y = 0;
	_surface->getDrawRect().width = _spriteResource.getDimensions().width;
	_surface->getDrawRect().height = _spriteResource.getDimensions().height;

	_needRefresh = true;
	
	SetUpdateHandler(&AsScene1002Door::update);
	SetMessageHandler(&AsScene1002Door::handleMessage);
	SetSpriteUpdate(NULL);
	StaticSprite::update();
	
}

void AsScene1002Door::update() {
	handleSpriteUpdate();
	StaticSprite::update();
}

uint32 AsScene1002Door::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x4808:
		setGlobalVar(0x8306F218, 1);
		SetSpriteUpdate(&AsScene1002Door::suOpenDoor);
		break;
	case 0x4809:
		setGlobalVar(0x8306F218, 0);
		SetSpriteUpdate(&AsScene1002Door::suCloseDoor);
		break;
	}
	return messageResult;
}

void AsScene1002Door::suOpenDoor() {
	if (_y > 49) {
		_y -= 8;
		if (_y < 49) {
			SetSpriteUpdate(NULL);
			_y = 49;
		}
		_needRefresh = true;
	}
}

void AsScene1002Door::suCloseDoor() {
	if (_y < 239) {
		_y += 8;
		if (_y > 239) {
			SetSpriteUpdate(NULL);
			_y = 239;
		}
		_needRefresh = true;
	}
}

AsScene1002BoxingGloveHitEffect::AsScene1002BoxingGloveHitEffect(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1400) {

	createSurface(1025, 88, 165);
	setVisible(false);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1002BoxingGloveHitEffect::handleMessage);	
}

uint32 AsScene1002BoxingGloveHitEffect::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2004:
		_x = ((Sprite*)sender)->getX() - 98;
		_y = ((Sprite*)sender)->getY() - 111;
		startAnimation(0x0422255A, 0, -1);
		setVisible(true);
		break;
	case 0x3002:
		stopAnimation();
		setVisible(false);
		break;
	}
	return messageResult;
}

AsScene1002DoorSpy::AsScene1002DoorSpy(NeverhoodEngine *vm, NRect &clipRect, Scene *parentScene, Sprite *asDoor, Sprite *asScene1002BoxingGloveHitEffect)
	: AnimatedSprite(vm, 1300), _clipRect(clipRect), _parentScene(parentScene), _asDoor(asDoor), _asBoxingGloveHitEffect(asScene1002BoxingGloveHitEffect),
	_soundResource(vm) {

	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1002DoorSpy::handleMessage);
	SetSpriteUpdate(&AsScene1002DoorSpy::suDoorSpy);
	createSurface(800, 136, 147);
	setClipRect(clipRect);
	suDoorSpy();
	_soundResource.load(0xC0C40298);
	startAnimation(0x586C1D48, 0, 0);
}

uint32 AsScene1002DoorSpy::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0xA61CA1C2) {
			sendMessage(_asBoxingGloveHitEffect, 0x2004, 0);
		} else if (param.asInteger() == 0x14CE0620) {
			_soundResource.play();
		}
		break;
	case 0x2003:
		stDoorSpyBoxingGlove();
		break;
	}
	return messageResult;
}

uint32 AsScene1002DoorSpy::hmDoorSpyAnimation(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		gotoNextState();
		break;
	}
	return messageResult;
}

void AsScene1002DoorSpy::suDoorSpy() {
	_x = _asDoor->getX() + 34;
	_y = _asDoor->getY() + 175;
}

void AsScene1002DoorSpy::stDoorSpyIdle() {
	setClipRect(_clipRect);
	_parentScene->setSurfacePriority(getSurface(), 800);
	startAnimation(0x586C1D48, 0, 0);
	SetMessageHandler(&AsScene1002DoorSpy::handleMessage);
}

void AsScene1002DoorSpy::stDoorSpyBoxingGlove() {
	setClipRect(0, 0, 640, 480);
	_parentScene->setSurfacePriority(getSurface(), 1200);
	startAnimation(0x586C1D48, 1, -1);
	SetMessageHandler(&AsScene1002DoorSpy::hmDoorSpyAnimation);
	NextState(&AsScene1002DoorSpy::stDoorSpyIdle);
}

Class426::Class426(NeverhoodEngine *vm, Scene *parentScene, uint32 fileHash1, uint32 fileHash2, int surfacePriority, uint32 soundFileHash) 
	: StaticSprite(vm, 1100), _parentScene(parentScene), _soundResource(vm), _status(0) {

	_soundFileHash = soundFileHash != 0 ? soundFileHash : 0x44141000;

	_fileHashes[0] = fileHash1;
	_fileHashes[1] = fileHash2;
	
	_spriteResource.load2(fileHash1);
	createSurface(surfacePriority, 40, 40);
	
	_surface->getDrawRect().x = 0;
	_surface->getDrawRect().y = 0;
	_surface->getDrawRect().width = _spriteResource.getDimensions().width;
	_surface->getDrawRect().height = _spriteResource.getDimensions().height;
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;

	setVisible(false);
	_needRefresh = true;
	
	SetUpdateHandler(&Class426::update);
	SetMessageHandler(&Class426::handleMessage);

}

void Class426::setFileHashes(uint32 fileHash1, uint32 fileHash2) {
	_fileHashes[0] = fileHash1;
	_fileHashes[1] = fileHash2;
	if (_status == 2) {
		_spriteResource.load2(fileHash2);
		_surface->getDrawRect().x = 0;
		_surface->getDrawRect().y = 0;
		_surface->getDrawRect().width = _spriteResource.getDimensions().width;
		_surface->getDrawRect().height = _spriteResource.getDimensions().height;
		_x = _spriteResource.getPosition().x;
		_y = _spriteResource.getPosition().y;
		_needRefresh = true;
		StaticSprite::update();
	} else {
		_spriteResource.load2(fileHash1);
		_surface->getDrawRect().x = 0;
		_surface->getDrawRect().y = 0;
		_surface->getDrawRect().width = _spriteResource.getDimensions().width;
		_surface->getDrawRect().height = _spriteResource.getDimensions().height;
		_x = _spriteResource.getPosition().x;
		_y = _spriteResource.getPosition().y;
		_needRefresh = true;
		StaticSprite::update();
	}
}

void Class426::update() {
	if (_countdown != 0 && (--_countdown) == 0) {
		if (_status == 1) {
			_status = 2;
			_spriteResource.load2(_fileHashes[1]);
			_surface->getDrawRect().x = 0;
			_surface->getDrawRect().y = 0;
			_surface->getDrawRect().width = _spriteResource.getDimensions().width;
			_surface->getDrawRect().height = _spriteResource.getDimensions().height;
			_x = _spriteResource.getPosition().x;
			_y = _spriteResource.getPosition().y;
			_needRefresh = true;
			StaticSprite::update();
			_countdown = 4;
		} else if (_status == 2) {
			_status = 3;
			_spriteResource.load2(_fileHashes[0]);
			_surface->getDrawRect().x = 0;
			_surface->getDrawRect().y = 0;
			_surface->getDrawRect().width = _spriteResource.getDimensions().width;
			_surface->getDrawRect().height = _spriteResource.getDimensions().height;
			_x = _spriteResource.getPosition().x;
			_y = _spriteResource.getPosition().y;
			_needRefresh = true;
			StaticSprite::update();
			_countdown = 4;
		} else if (_status == 3) {
			_status = 0;
			setVisible(false);
		}
	}
}

uint32 Class426::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x480B:
		sendMessage(_parentScene, 0x480B, 0);
		_status = 1;
		_countdown = 4;
		setVisible(true);
		_soundResource.play(_soundFileHash);
		break;
	}
	return messageResult;
}

AsScene1002VenusFlyTrap::AsScene1002VenusFlyTrap(NeverhoodEngine *vm, Scene *parentScene, Sprite *klayman, bool flag)
	: AnimatedSprite(vm, 1100), _soundResource(vm), _parentScene(parentScene), _klayman(klayman),
	_flag(flag), _countdown(0) {

	createSurface(995, 175, 195);

	SetUpdateHandler(&AsScene1002VenusFlyTrap::update);
	SetMessageHandler(&AsScene1002VenusFlyTrap::handleMessage);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);

	if (!_flag) {
		if (getGlobalVar(0x8306F218)) {
			setDoDeltaX(1);
			_x = 366;
			_y = 435;
			stRingGrabbed();
		} else {
			_x = 174 + getGlobalVar(0x1B144052) * 32;
			_y = 435;
			stIdle();
		}
	} else {
		_x = 186 + getGlobalVar(0x86341E88) * 32;
		_y = 364;
		if (getGlobalVar(0x13206309) || getGlobalVar(0x80101B1E)) {
			stRingGrabbed();
		} else {
			stIdle();
		} 
	}
	
	_flags = 4;
}

void AsScene1002VenusFlyTrap::update() {
	if (_countdown != 0 && (--_countdown == 0)) {
		gotoNextState();
	}
	AnimatedSprite::update();
}

void AsScene1002VenusFlyTrap::upIdle() {
	if (_countdown == 0 && _klayman->getX() - 20 > _x) {
		setDoDeltaX(1);
	} else if (_klayman->getX() + 20 < _x) {
		setDoDeltaX(0);
	}
	update();
}

uint32 AsScene1002VenusFlyTrap::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x000890C4) {
			_soundResource.play(0xC21190D8);
		} else if (param.asInteger() == 0x522200A0) {
			_soundResource.play(0x931080C8);
		}
		break;
	case 0x1011:
		if (_flag) {
			if (_x >= 154 && _x <= 346) {
				sendMessage(_parentScene, 0x2000, 0);
				messageResult = 1;
			}
		} else {
			if (_x >= 174 && _x <= 430) {
				sendMessage(_parentScene, 0x2000, 0);
				messageResult = 1;
			}
		}
		break;
	case 0x480B:
		setDoDeltaX(param.asInteger() != 0 ? 1 : 0);
		if (!_flag) {
			if (getGlobalVar(0x8306F218)) {
				stRelease();
			} else {
				stWalk();
			}
		} else {
			if (getGlobalVar(0x13206309) || getGlobalVar(0x80101B1E)) {
				stRelease();
			} else {
				stWalk();
			}
		}
		break;
	case 0x480C:
		if (_flag) {
			if (_x >= 154 && _x <= 346)
				messageResult = 1;
			else				
				messageResult = 0;
		} else {
			if (_x >= 174 && _x <= 430)
				messageResult = 1;
			else				
				messageResult = 0;
		}
		break;
	case 0x480E:
		if (param.asInteger() == 1) {
			stGrabRing();
		}
		break;
	case 0x4810:
		swallowKlayman();
		break;
	case 0x482A:
		sendMessage(_parentScene, 0x1022, 995);
		break;
	case 0x482B:
		sendMessage(_parentScene, 0x1022, 1015);
		break;
	}
	return messageResult;
}

uint32 AsScene1002VenusFlyTrap::hmAnimationSimple(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		gotoNextState();
		break;
	}
	return messageResult;
}

uint32 AsScene1002VenusFlyTrap::hmAnimationExt(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x000890C4) {
			_soundResource.play(0xC21190D8);
		} else if (param.asInteger() == 0x41881801) {
			if (_flag) {
				if (_x > 330) {
					sendMessage(_klayman, 0x4811, 2);
				} else if (_x > 265) {
					// FIXME: This check is the same as the one below.
					// Perhaps the third parameter should be 1 here?
					sendMessage(_klayman, 0x4811, 0);
				} else {
					sendMessage(_klayman, 0x4811, 0);
				}
			} else {
				sendMessage(_klayman, 0x4811, 0);
			}
		} else if (param.asInteger() == 0x522200A0) {
			_soundResource.play(0x931080C8);
		}
		break;
	case 0x3002:
		gotoNextState();
		break;
	case 0x482A:
		sendMessage(_parentScene, 0x1022, 995);
		break;
	case 0x482B:
		sendMessage(_parentScene, 0x1022, 1015);
		break;
	}
	return messageResult;
}

void AsScene1002VenusFlyTrap::stWalkBack() {
	setDoDeltaX(2);
	startAnimation(0xC4080034, 0, -1);
	SetUpdateHandler(&AsScene1002VenusFlyTrap::update);
	SetMessageHandler(&AsScene1002VenusFlyTrap::hmAnimationExt);
	NextState(&AsScene1002VenusFlyTrap::stIdle);
}

void AsScene1002VenusFlyTrap::stWalk() {
	startAnimation(0xC4080034, 0, -1);
	SetUpdateHandler(&AsScene1002VenusFlyTrap::update);
	SetMessageHandler(&AsScene1002VenusFlyTrap::hmAnimationSimple);
	NextState(&AsScene1002VenusFlyTrap::stIdle);
}

void AsScene1002VenusFlyTrap::stRelease() {
	sendMessage(_parentScene, 0x4807, 0);
	startAnimation(0x82292851, 0, -1);
	SetUpdateHandler(&AsScene1002VenusFlyTrap::update);
	SetMessageHandler(&AsScene1002VenusFlyTrap::hmAnimationSimple);
	NextState(&AsScene1002VenusFlyTrap::stIdle);
}

void AsScene1002VenusFlyTrap::stGrabRing() {
	setDoDeltaX(1);
	startAnimation(0x86A82A11, 0, -1);
	SetUpdateHandler(&AsScene1002VenusFlyTrap::update);
	SetMessageHandler(&AsScene1002VenusFlyTrap::hmAnimationSimple);
	NextState(&AsScene1002VenusFlyTrap::stRingGrabbed);
}

void AsScene1002VenusFlyTrap::stRingGrabbed() {
	startAnimation(0xB5A86034, 0, -1);
	SetUpdateHandler(&AsScene1002VenusFlyTrap::update);
	SetMessageHandler(&AsScene1002VenusFlyTrap::handleMessage);
}

void AsScene1002VenusFlyTrap::stKlaymanInside() {
	startAnimation(0x31303094, 0, -1);
	SetUpdateHandler(&AsScene1002VenusFlyTrap::update);
	SetMessageHandler(NULL);
	NextState(&AsScene1002VenusFlyTrap::stKlaymanInsideMoving);
	_countdown = 24;
}

void AsScene1002VenusFlyTrap::stIdle() {
	startAnimation(0xC8204250, 0, -1);
	SetUpdateHandler(&AsScene1002VenusFlyTrap::upIdle);
	SetMessageHandler(&AsScene1002VenusFlyTrap::handleMessage);
	if (_flag) {
		if (_x >= 154 && _x <= 346) {
			setGlobalVar(0x86341E88, (_x - 186) / 32);
		} else {
			NextState(&AsScene1002VenusFlyTrap::stWalkBack);
			_countdown = 12;
		}
	} else {
		if (_x >= 174 && _x <= 430) {
			setGlobalVar(0x1B144052, (_x - 174) / 32);
		} else {
			NextState(&AsScene1002VenusFlyTrap::stWalkBack);
			_countdown = 12;
		}
	}
}

void AsScene1002VenusFlyTrap::stKlaymanInsideMoving() {
	startAnimation(0x152920C4, 0, -1);
	SetUpdateHandler(&AsScene1002VenusFlyTrap::update);
	SetMessageHandler(&AsScene1002VenusFlyTrap::hmAnimationExt);
	NextState(&AsScene1002VenusFlyTrap::stSpitOutKlayman);
}

void AsScene1002VenusFlyTrap::stSpitOutKlayman() {
	startAnimation(0x84001117, 0, -1);
	SetUpdateHandler(&AsScene1002VenusFlyTrap::update);
	SetMessageHandler(&AsScene1002VenusFlyTrap::hmAnimationExt);
	NextState(&AsScene1002VenusFlyTrap::stIdle);
}

void AsScene1002VenusFlyTrap::swallowKlayman() {
	if (_x - 15 < _klayman->getX() && _x + 15 > _klayman->getX()) {
		if (_flag) {
			setDoDeltaX(_x > 265 && _x < 330 ? 1 : 0);
		} else {
			setDoDeltaX(_x > 320 ? 1 : 0);
		}
		sendMessage(_klayman, 0x2001, 0);
		startAnimation(0x8C2C80D4, 0, -1);
		SetUpdateHandler(&AsScene1002VenusFlyTrap::update);
		SetMessageHandler(&AsScene1002VenusFlyTrap::hmAnimationExt);
		NextState(&AsScene1002VenusFlyTrap::stKlaymanInside);
	}
}

AsScene1002OutsideDoorBackground::AsScene1002OutsideDoorBackground(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1200), _countdown(0) {

	createSurface(850, 186, 212);
	_x = 320;
	_y = 240;
	if (getGlobalVar(0x8306F218)) {
		startAnimation(0x004A4495, -1, -1);
		_newStickFrameIndex = -2;
	} else {
		setVisible(false);
	}
	SetUpdateHandler(&AsScene1002OutsideDoorBackground::update);
	SetMessageHandler(&AsScene1002OutsideDoorBackground::handleMessage);	
}

void AsScene1002OutsideDoorBackground::update() {
	if (_countdown != 0 && (--_countdown == 0)) {
		if (_isDoorClosed) {
			stCloseDoor();
		} else {
			stOpenDoor();
		}
	}
	AnimatedSprite::update();
}

uint32 AsScene1002OutsideDoorBackground::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageResult) {
	case 0x4808:
		_isDoorClosed = false;
		_countdown = 2;
		break;
	case 0x4809:
		_isDoorClosed = true;
		_countdown = 2;
		break;
	}
	return messageResult;
}

uint32 AsScene1002OutsideDoorBackground::hmAnimation(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = handleMessage(messageNum, param, sender);
	switch (messageResult) {
	case 0x3002:
		gotoNextState();
		break;
	}
	return messageResult;
}

void AsScene1002OutsideDoorBackground::stOpenDoor() {
	startAnimation(0x004A4495, 0, -1);
	SetMessageHandler(&AsScene1002OutsideDoorBackground::handleMessage);
	_newStickFrameIndex = -2;
	setVisible(true);
}

void AsScene1002OutsideDoorBackground::stCloseDoor() {
	startAnimation(0x004A4495, -1, -1);
	_playBackwards = true;
	SetMessageHandler(&AsScene1002OutsideDoorBackground::hmAnimation);
	NextState(&AsScene1002OutsideDoorBackground::stDoorClosed);
	setVisible(true);
}

void AsScene1002OutsideDoorBackground::stDoorClosed() {
	setVisible(false);
	stopAnimation();
}

AsScene1002KlaymanLadderHands::AsScene1002KlaymanLadderHands(NeverhoodEngine *vm, Klayman *klayman)
	: AnimatedSprite(vm, 1200), _klayman(klayman) {
	
	createSurface(1200, 40, 163);
	SetUpdateHandler(&AsScene1002KlaymanLadderHands::update);
	SetMessageHandler(&Sprite::handleMessage);
	setVisible(false);
}

void AsScene1002KlaymanLadderHands::update() {
	if (_klayman->getCurrAnimFileHash() == 0x3A292504) {
		startAnimation(0xBA280522, _klayman->getFrameIndex(), -1);
		_newStickFrameIndex = _klayman->getFrameIndex();
		setVisible(true);
		_x = _klayman->getX(); 
		_y = _klayman->getY(); 
		setDoDeltaX(_klayman->isDoDeltaX() ? 1 : 0);
	} else if (_klayman->getCurrAnimFileHash() == 0x122D1505) {
		startAnimation(0x1319150C, _klayman->getFrameIndex(), -1);
		_newStickFrameIndex = _klayman->getFrameIndex();
		setVisible(true);
		_x = _klayman->getX(); 
		_y = _klayman->getY(); 
		setDoDeltaX(_klayman->isDoDeltaX() ? 1 : 0);
	} else {
		setVisible(false);
	}
	AnimatedSprite::update();
}

AsScene1002KlaymanPeekHand::AsScene1002KlaymanPeekHand(NeverhoodEngine *vm, Scene *parentScene, Klayman *klayman)
	: AnimatedSprite(vm, 1200), _parentScene(parentScene), _klayman(klayman),
	_isClipRectSaved(false) {
	
	SetUpdateHandler(&AsScene1002KlaymanPeekHand::update);
	SetMessageHandler(&AsScene1002KlaymanPeekHand::handleMessage);
	createSurface(1000, 33, 41);
	setVisible(false);
}

void AsScene1002KlaymanPeekHand::update() {
	if (_klayman->getCurrAnimFileHash() == 0xAC20C012 && _klayman->getFrameIndex() < 50) {
		startAnimation(0x9820C913, _klayman->getFrameIndex(), -1);
		_newStickFrameIndex = _klayman->getFrameIndex();
		setVisible(true);
		_x = _klayman->getX();
		_y = _klayman->getY();
		setDoDeltaX(_klayman->isDoDeltaX() ? 1 : 0);
	} else {
		setVisible(false);
	}
	AnimatedSprite::update();
}

uint32 AsScene1002KlaymanPeekHand::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x4AB28209) {
			sendMessage(_parentScene, 0x1022, 1200);
			_isClipRectSaved = true;
			_savedClipRect = _surface->getClipRect();
			setClipRect(0, 0, 640, 480);
		} else if (param.asInteger() == 0x88001184) {
			sendMessage(_parentScene, 0x1022, 1000);
			if (_isClipRectSaved)
				setClipRect(_savedClipRect);
		}
		break;
	}
	return messageResult;
}

Scene1002::Scene1002(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _soundResource1(vm), _soundResource2(vm), _soundResource3(vm),
	_flag1B4(false), _flag1BE(false) {

	NRect tempClipRect;
	Sprite *tempSprite;

	SetUpdateHandler(&Scene1002::update);
	SetMessageHandler(&Scene1002::handleMessage);

	setHitRects(0x004B4138);

	_surfaceFlag = true;

	setBackground(0x12C23307);
	setPalette(0x12C23307);

	_flag = false;

	insertStaticSprite(0x06149428, 1100);
	insertStaticSprite(0x312C8774, 1100);

	_ssLadderArch = insertSprite<SsScene1002LadderArch>(this);
	_ssLadderArchPart1 = insertStaticSprite(0x060000A0, 1200);
	_ssLadderArchPart2 = insertStaticSprite(0xB2A423B0, 1100);
	_ssLadderArchPart3 = insertStaticSprite(0x316E0772, 1100);

	_class599 = insertSprite<Class599>(this);

	if (which < 0) {
		if (_vm->_gameState.field2 == 0) {
			insertKlayman<KmScene1002>(90, 226, _class599, _ssLadderArch);
			_asKlaymanLadderHands = insertSprite<AsScene1002KlaymanLadderHands>(_klayman);
			setMessageList(0x004B4270);
			_klayman->setClipRect(31, 0, _ssLadderArchPart2->getDrawRect().x2(), _ssLadderArchPart3->getDrawRect().y2());
			_asKlaymanLadderHands->getSurface()->getClipRect() = _klayman->getSurface()->getClipRect();
			_klayman->setRepl(64, 0);
		} else {
			insertKlayman<KmScene1002>(379, 435, _class599, _ssLadderArch);
			_asKlaymanLadderHands = insertSprite<AsScene1002KlaymanLadderHands>(_klayman);
			setMessageList(0x004B4270);
			_klayman->setClipRect(_ssLadderArch->getDrawRect().x, 0, _ssLadderArchPart2->getDrawRect().x2(), _ssLadderArchPart1->getDrawRect().y2());
			_asKlaymanLadderHands->setClipRect(_klayman->getClipRect());
		}
	} else if (which == 1) {
		insertKlayman<KmScene1002>(650, 435, _class599, _ssLadderArch);
		_asKlaymanLadderHands = insertSprite<AsScene1002KlaymanLadderHands>(_klayman);
		setMessageList(0x004B4478);
		_klayman->setClipRect(_ssLadderArch->getDrawRect().x, 0, _ssLadderArchPart2->getDrawRect().x2(), _ssLadderArchPart1->getDrawRect().y2());
		_asKlaymanLadderHands->setClipRect(_klayman->getClipRect());
		_vm->_gameState.field2 = 1;
	} else if (which == 2) {
		insertKlayman<KmScene1002>(68, 645, _class599, _ssLadderArch);
		_asKlaymanLadderHands = insertSprite<AsScene1002KlaymanLadderHands>(_klayman);
		setMessageList(0x004B4298);
		_klayman->setClipRect(_ssLadderArch->getDrawRect().x, 0, _ssLadderArchPart2->getDrawRect().x2(), _ssLadderArchPart1->getDrawRect().y2());
		_asKlaymanLadderHands->setClipRect(_klayman->getClipRect());
		_vm->_gameState.field2 = 1;
		sendMessage(_klayman, 0x4820, 0);
	} else {
		insertKlayman<KmScene1002>(90, 226, _class599, _ssLadderArch);
		_asKlaymanLadderHands = insertSprite<AsScene1002KlaymanLadderHands>(_klayman);
		setMessageList(0x004B4470);
		_klayman->setClipRect(31, 0, _ssLadderArchPart2->getDrawRect().x2(), _ssLadderArchPart3->getDrawRect().y2());
		_asKlaymanLadderHands->setClipRect(_klayman->getClipRect());
		_asKlaymanPeekHand = insertSprite<AsScene1002KlaymanPeekHand>(this, _klayman);
		_asKlaymanPeekHand->setClipRect(_klayman->getClipRect());
		_klayman->setRepl(64, 0);
		_vm->_gameState.field2 = 0;
	}

	insertMouse433(0x23303124);

	tempSprite = insertStaticSprite(0xB3242310, 825);
	tempClipRect.set(tempSprite->getDrawRect().x, tempSprite->getDrawRect().y,
		_ssLadderArchPart2->getDrawRect().x2(), _ssLadderArchPart2->getDrawRect().y2());

	_asRing1 = insertSprite<AsScene1002Ring>(this, false, 258, 191, _class599->getDrawRect().y, false);
	_asRing2 = insertSprite<AsScene1002Ring>(this, false, 297, 189, _class599->getDrawRect().y, false);
	_asRing3 = insertSprite<AsScene1002Ring>(this, true, 370, 201, _class599->getDrawRect().y, getGlobalVar(0x8306F218) != 0);
	_asRing4 = insertSprite<AsScene1002Ring>(this, false, 334, 191, _class599->getDrawRect().y, false);
	_asRing5 = insertSprite<AsScene1002Ring>(this, false, 425, 184, _class599->getDrawRect().y, false);

	_asDoor = insertSprite<AsScene1002Door>(tempClipRect);
	tempSprite = insertSprite<AsScene1002BoxingGloveHitEffect>();
	_asDoorSpy = insertSprite<AsScene1002DoorSpy>(tempClipRect, this, _asDoor, tempSprite);
	_class426 = insertSprite<Class426>(this, 0x00412692, 0x140B60BE, 800, 0);
	_asVenusFlyTrap = insertSprite<AsScene1002VenusFlyTrap>(this, _klayman, false);
	_vm->_collisionMan->addSprite(_asVenusFlyTrap);

	sendEntityMessage(_klayman, 0x2007, _asVenusFlyTrap);

	_asOutsideDoorBackground = insertSprite<AsScene1002OutsideDoorBackground>();
								  
	setRectList(0x004B43A0);

	_soundResource2.load(0x60755842);
	_soundResource3.load(0x616D5821);

}

Scene1002::~Scene1002() {
}

void Scene1002::update() {
	Scene::update();
	if (!_flag1B4 && _klayman->getY() > 230) {
		_klayman->setClipRect(_ssLadderArch->getDrawRect().x, 0, _ssLadderArchPart2->getDrawRect().x2(), _ssLadderArchPart1->getDrawRect().y2());
		_asKlaymanLadderHands->setClipRect(_klayman->getClipRect());
		deleteSprite(&_ssLadderArchPart3);
		_klayman->clearRepl();
		_flag1B4 = true;
		_vm->_gameState.field2 = 1;
	}

	if (_flag1BE && _klayman->getY() > 422) {
		sendMessage(_parentModule, 0x1024, 1);
		_flag1BE = false;
	}
	
}

uint32 Scene1002::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = 0;
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		// Debug stuff (original)
		if (param.asPoint().x == 0 && getGlobalVar(0xA4014072)) {
			setGlobalVar(0x8306F218, 1);
			setGlobalVar(0x1B144052, 3);
			leaveScene(1);
		}
		break;
	case 0x000D:
		// Debug stuff (original)
		if (param.asInteger() == 0x48848178) {
			setGlobalVar(0x8306F218, 1);
			setGlobalVar(0x1B144052, 3);
			leaveScene(1);
		}
		messageResult = 1;
		break;
	case 0x100D:
		if (param.asInteger() == 0xE6EE60E1) {
			if (getGlobalVar(0x8306F218)) {
				setMessageList(0x004B4428);
			} else {
				setMessageList(0x004B4448);
			}
			messageResult = 1;
		} else if (param.asInteger() == 0x4A845A00) {
			sendEntityMessage(_klayman, 0x1014, _asRing1);
		} else if (param.asInteger() == 0x43807801) {
			sendEntityMessage(_klayman, 0x1014, _asRing2);
		} else if (param.asInteger() == 0x46C26A01) {
			if (getGlobalVar(0x8306F218)) {
				setMessageList(0x004B44B8);
			} else {
				sendEntityMessage(_klayman, 0x1014, _asRing3);
				if (_asVenusFlyTrap->getX() - 10 < 366 && _asVenusFlyTrap->getX() + 10 > 366) {
					setGlobalVar(0x2B514304, 1);
					setMessageList(0x004B44A8);
				} else {
					setMessageList(0x004B44A0);
				}
			}
			messageResult = 1;
		} else if (param.asInteger() == 0x468C7B11) {
			sendEntityMessage(_klayman, 0x1014, _asRing4);
		} else if (param.asInteger() == 0x42845B19) {
			sendEntityMessage(_klayman, 0x1014, _asRing5);
		} else if (param.asInteger() == 0xC0A07458) {
			sendEntityMessage(_klayman, 0x1014, _class426);
		}
		break;
	case 0x1024:
		sendMessage(_parentModule, 0x1024, param.asInteger());
		break;
	case 0x2000:
		if (_flag) {
			setMessageList2(0x004B43D0);
		} else {
			if (_klayman->getY() > 420) {
				sendEntityMessage(_klayman, 0x1014, _asVenusFlyTrap);
				setMessageList2(0x004B4480);
			} else if (_klayman->getY() > 227) {
				setMessageList2(0x004B41E0);
			} else {
				setMessageList2(0x004B4148);
			}
		}
		break;
	case 0x2002:
		_messageList = NULL;
		break;										
	case 0x2005:
		_flag = true;
		setRectList(0x004B4418);
		break;										
	case 0x2006:
		_flag = false;
		setRectList(0x004B43A0);
		break;
	case 0x4806:
		sendMessage(_parentModule, 0x1024, 2);
		_flag1BE = true;
		if (sender == _asRing1) {
			setGlobalVar(0x4DE80AC0, 0);
			_soundResource1.play(0x665198C0);
		} else if (sender == _asRing2) {
			setGlobalVar(0x4DE80AC0, 0);
			_soundResource1.play(0xE2D389C0);
		} else if (sender == _asRing3) {
			setGlobalVar(0x4DE80AC0, 0);
			_soundResource2.play();
			sendMessage(_asDoor, 0x4808, 0);
			sendMessage(_asOutsideDoorBackground, 0x4808, 0);
		} else if (sender == _asRing4) {
			setGlobalVar(0x4DE80AC0, 0);
			_soundResource1.play(0xE0558848);
		} else if (sender == _asRing5) {
			setGlobalVar(0x4DE80AC0, 1);
			_soundResource1.play(0x44014282);
		}
		break;
	case 0x4807:
		if (sender == _asRing3) {
			_soundResource3.play();
			sendMessage(_asDoor, 0x4809, 0);
			sendMessage(_asOutsideDoorBackground, 0x4809, 0);
		} else if (sender == _asVenusFlyTrap) {
			if (getGlobalVar(0x8306F218)) {
				sendMessage(_asRing3, 0x4807, 0);
			}
		}	
		break;
	case 0x480B:
		sendEntityMessage(_klayman, 0x1014, _asDoorSpy);
		break;				
	case 0x480F:
		setGlobalVar(0x4DE80AC0, 0);
		_soundResource2.play();
		sendMessage(_asDoor, 0x4808, 0);
		sendMessage(_asOutsideDoorBackground, 0x4808, 0);
		break;				
	}	
	return messageResult;
}

// Class152

Class152::Class152(NeverhoodEngine *vm, Module *parentModule, uint32 backgroundFileHash, uint32 cursorFileHash)
	: Scene(vm, parentModule, true) {

	_surfaceFlag = false;

	SetMessageHandler(&Class152::handleMessage);
	
	setBackground(backgroundFileHash);
	setPalette(backgroundFileHash);
	insertMouse435(cursorFileHash, 20, 620);
}

uint32 Class152::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		if (param.asPoint().x <= 20 || param.asPoint().x >= 620)
			leaveScene(0);
		break;
	}
	return 0;
}

// Scene1004

AsScene1004TrashCan::AsScene1004TrashCan(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1100), _soundResource(vm) {

	_x = 330;
	_y = 327;
	createSurface(800, 56, 50);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1004TrashCan::handleMessage);
	setVisible(false);
}

uint32 AsScene1004TrashCan::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x225A8587) {
			_soundResource.play(0x109AFC4C);
		}
		break;
	case 0x2002:
		startAnimation(0xEB312C11, 0, -1);
		setVisible(true);
		break;
	case 0x3002:
		stopAnimation();
		setVisible(false);
		break;
	}
	return 0;
}

Scene1004::Scene1004(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _paletteAreaStatus(-1) {

	Sprite *tempSprite;
	
	_surfaceFlag = true;
	
	SetUpdateHandler(&Scene1004::update);
	SetMessageHandler(&Scene1004::handleMessage);

	setBackground(0x50C03005);

	if (getGlobalVar(0x0D0A14D10)) {
		setPalette(0xA30BA329);
		_palette->addBasePalette(0xA30BA329, 0, 256, 0);
	} else {
		setPalette(0x50C03005);
		_palette->addBasePalette(0x50C03005, 0, 256, 0);
	}
	addEntity(_palette);

	insertMouse433(0x03001504);

	if (which < 0) {
		setRectList(0x004B7C70);
		insertKlayman<KmScene1004>(330, 327);
		setMessageList(0x004B7C18);
	} else if (which == 1) {
		setRectList(0x004B7C70);
		insertKlayman<KmScene1004>(330, 327);
		setMessageList(0x004B7C08);
	} else {
		loadDataResource(0x01900A04);
		insertKlayman<KmScene1004>(_dataResource.getPoint(0x80052A29).x, 27);
		setMessageList(0x004B7BF0);
	}
	
	updatePaletteArea();
	
	_asKlaymanLadderHands = insertSprite<AsScene1002KlaymanLadderHands>(_klayman);

	insertStaticSprite(0x800034A0, 1100);
	insertStaticSprite(0x64402020, 1100);
	insertStaticSprite(0x3060222E, 1300);
	tempSprite = insertStaticSprite(0x0E002004, 1300);
	
	_klayman->setClipRect(0, tempSprite->getDrawRect().y, 640, 480);
	_asKlaymanLadderHands->setClipRect(_klayman->getClipRect());

	_asTrashCan = insertSprite<AsScene1004TrashCan>();

}

void Scene1004::update() {
	Scene::update();
	updatePaletteArea();
}

uint32 Scene1004::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = 0;
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x926500A1) {
			setMessageList(0x004B7C20);
			messageResult = 1;
		}
		break;
	case 0x2000:
		loadDataResource(0x01900A04);
		break;
	case 0x2001:
		setRectList(0x004B7C70);
		break;
	case 0x2002:
		sendMessage(_asTrashCan, 0x2002, 0);
		break;
	}
	return messageResult;
} 

void Scene1004::updatePaletteArea() {
	if (_klayman->getY() < 150) {
		if (_paletteAreaStatus != 0) {
			_paletteAreaStatus = 0;
			_palette->addBasePalette(0x406B0D10, 0, 64, 0);
			_palette->startFadeToPalette(12);
		}
	} else {
		if (_paletteAreaStatus != 1) {
			_paletteAreaStatus = 1;
			_palette->addBasePalette(0x24332243, 0, 64, 0);
			_palette->startFadeToPalette(12);
		}
	}
}

// Scene1005

Scene1005::Scene1005(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true) {

	SetMessageHandler(&Scene1005::handleMessage);

	_surfaceFlag = true;
	
	if (getGlobalVar(0xD0A14D10)) {
		setBackground(0x2800E011);
		setPalette(0x2800E011);
		insertStaticSprite(0x492D5AD7, 100);
		insertMouse435(0x0E015288, 20, 620);
	} else {
		setBackground(0x8870A546);
		setPalette(0x8870A546);
		insertStaticSprite(0x40D1E0A9, 100);
		insertStaticSprite(0x149C00A6, 100);
		insertMouse435(0x0A54288F, 20, 620);
	}

	drawTextToBackground();
	
}

uint32 Scene1005::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		if (param.asPoint().x <= 20 || param.asPoint().x >= 620) {
			leaveScene(0);			
		}
		break;
	}
	return 0;
}

void Scene1005::drawTextToBackground() {
	TextResource textResource(_vm);
	const char *textStart, *textEnd;
	int16 y = 36;
	uint32 textIndex = getTextIndex();
	FontSurface *fontSurface = createFontSurface();
	textResource.load(0x80283101);
	textStart = textResource.getString(textIndex, textEnd);
	while (textStart < textEnd) {
		fontSurface->drawString(_background->getSurface(), 188, y, (const byte*)textStart);
		y += 36;
		textStart += strlen(textStart) + 1;
	}
	delete fontSurface;
}

FontSurface *Scene1005::createFontSurface() {
	FontSurface *fontSurface;
	DataResource fontData(_vm);
	SpriteResource fontSprite(_vm);
	fontData.load(calcHash("asRecFont"));
	uint16 numRows = fontData.getPoint(calcHash("meNumRows")).x;
	uint16 firstChar = fontData.getPoint(calcHash("meFirstChar")).x;
	uint16 charWidth = fontData.getPoint(calcHash("meCharWidth")).x;
	uint16 charHeight = fontData.getPoint(calcHash("meCharHeight")).x;
	NPointArray *tracking = fontData.getPointArray(calcHash("meTracking"));
	fontSurface = new FontSurface(_vm, tracking, numRows, firstChar, charWidth, charHeight);	
	if (getGlobalVar(0xD0A14D10)) {
		fontSprite.load2(0x283CE401);
	} else {
		fontSprite.load2(0xC6604282);
	}
	fontSurface->drawSpriteResourceEx(fontSprite, false, false, 0, 0);
	return fontSurface;
}

uint32 Scene1005::getTextIndex() {
	uint32 textIndex;
	textIndex = getTextIndex1();
	if (getGlobalVar(0xD0A14D10)) {
		textIndex = getTextIndex2();
	}
	if (getGlobalVar(0x8440001F) && getGlobalVar(0x01830201) == textIndex) {
		textIndex = getTextIndex3();
	} else {
		setGlobalVar(0x8440001F, 1);
		setGlobalVar(0x01830201, textIndex);
	}
	return textIndex;
}

uint32 Scene1005::getTextIndex1() {
	uint32 textIndex;
	if (getGlobalVar(0x98109F12)) {
		if (!getGlobalVar(0x2090590C))
			textIndex = 18;
		else if (!getGlobalVar(0x610210B7))
			textIndex = 19;
		else if (getGlobalVar(0x0C0288F4)) {
			if (!getGlobalVar(0xD0A14D10))
				textIndex = 23;
			else if (!getSubVar(0x0090EA95, 0) && !getSubVar(0x08D0AB11, 0))
				textIndex = 24;
			else if (!getGlobalVar(0xC0780812))			
				textIndex = 26;
			else if (!getSubVar(0x0090EA95, 1) && !getSubVar(0x08D0AB11, 1))
				textIndex = 27;
			else if (!getGlobalVar(0xC0780812)) 
				textIndex = 28;
			else				
				textIndex = 29;
		} else if (!getGlobalVar(0xE7498218))
			textIndex = 20;
		else if (!getGlobalVar(0x081890D14))
			textIndex = 21;
		else			
			textIndex = 22;
	} else if (getGlobalVar(0x00040153)) {
		if (!getGlobalVar(0x10938830))
			textIndex = 12;
		else if (!getGlobalVar(0x2050861A))
			textIndex = 13;
		else if (!getGlobalVar(0x4DE80AC0))
			textIndex = 50;
		else if (!getGlobalVar(0x89C669AA))
			textIndex = 14;
		else if (!getGlobalVar(0x1C1B8A9A))
			textIndex = 15;
		else if (!getGlobalVar(0xCB45DE03))
			textIndex = 16;
		else 
			textIndex = 17;
	} else if (!getGlobalVar(0x2B514304)) {
		textIndex = 0;
	} else if (getGlobalVar(0x0A18CA33)) {
		if (!getGlobalVar(0x404290D5))
			textIndex = 4;
		else if (!getGlobalVar(0x45080C38))
			textIndex = 5;
		else if (!getSubVar(0x14800353, 0x40119852))
			textIndex = 6;
		else if (!getGlobalVar(0x4E0BE910))
			textIndex = 7;
		else if (!getGlobalVar(0x86615030))
			textIndex = 8;
		else if (!getSubVar(0x14800353, 0x304008D2))
			textIndex = 9;
		else if (!getSubVar(0x14800353, 0x01180951))
			textIndex = 10;
		else 
			textIndex = 11;
	} else if (!getGlobalVar(0x0A310817)) {
		textIndex = 1;
	} else if (getGlobalVar(0x000CF819)) {
		textIndex = 3;
	} else {
		textIndex = 2;
	}
	return textIndex;
}

uint32 Scene1005::getTextIndex2() {
	uint32 textIndex = getGlobalVar(0x29408F00);
	if (textIndex + 1 >= 10) {
		setGlobalVar(0x29408F00, 0);
		textIndex = 0;
	} else {
		setGlobalVar(0x29408F00, textIndex + 1);
	}
	return textIndex + 40;
}

uint32 Scene1005::getTextIndex3() {
	uint32 textIndex = getGlobalVar(0x8A140C21);
	if (textIndex + 1 >= 10) {
		setGlobalVar(0x8A140C21, 0);
		textIndex = 0;
	} else {
		setGlobalVar(0x8A140C21, textIndex + 1);
	}
	return textIndex + 30;
}

} // End of namespace Neverhood
