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

#include "neverhood/modules/module1300.h"
#include "neverhood/modules/module1000.h"
#include "neverhood/modules/module1200.h"
#include "neverhood/modules/module1400.h"
#include "neverhood/modules/module2200.h"
#include "neverhood/gamemodule.h"
#include "neverhood/diskplayerscene.h"
#include "neverhood/menumodule.h"
#include "neverhood/navigationscene.h"
#include "neverhood/smackerscene.h"

namespace Neverhood {

static const uint32 kModule1300SoundList[] = {
	0x16805648,
	0x16805C48,
	0xB4964448,
	0x96A05481,
	0xD0E14441,
	0x90815450,
	0
};

Module1300::Module1300(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {
	
	_vm->_soundMan->addMusic(0x61C090, 0x00203197);
	_vm->_soundMan->addSoundList(0x61C090, kModule1300SoundList);
	_vm->_soundMan->setSoundListParams(kModule1300SoundList, false, 50, 600, 20, 150);
	_vm->_soundMan->playTwoSounds(0x61C090, 0x48498E46, 0x50399F64, 0);
	_vm->_soundMan->setSoundVolume(0x48498E46, 70);
	_vm->_soundMan->setSoundVolume(0x50399F64, 70);

	if (which < 0) {
		if (_vm->gameState().sceneNum >= 1 && _vm->gameState().sceneNum <= 17)
			createScene(_vm->gameState().sceneNum, -1);
		else			
			createScene(11, 0);
	} else {
		switch (which) {
		case 0:
			createScene(11, 0);
			break;
		case 1:
			createScene(13, 0);
			break;
		case 2:
			createScene(14, 0);
			break;
		case 3:
			createScene(15, 0);
			break;
		case 4:
			createScene(7, 0);
			break;
		case 5:
			createScene(5, 1);
			break;
		case 6:
			createScene(5, 5);
			break;
		case 7:
			createScene(3, 0);
			break;
		case 8:
			createScene(1, 0);
			break;
		case 9:
			createScene(2, 0);
			break;
		case 10:
			createScene(6, 0);
			break;
		case 11:
			createScene(4, 0);
			break;
		default:
			createScene(12, 0);
			break;
		}
	}
	
}

Module1300::~Module1300() {
	_vm->_soundMan->deleteGroup(0x61C090);
}

void Module1300::createScene(int sceneNum, int which) {
	debug("Module1300::createScene(%d, %d)", sceneNum, which);
	_sceneNum = sceneNum;
	switch (_sceneNum) {
	case 1:
		_vm->gameState().sceneNum = 1;
		_vm->_soundMan->setSoundListParams(kModule1300SoundList, false, 0, 0, 0, 0);
		_vm->_soundMan->startMusic(0x00203197, 0, 2);
		_childObject = new Scene1302(_vm, this, which);
		break;
	case 2:
		_vm->gameState().sceneNum = 2;
		_vm->_soundMan->setSoundListParams(kModule1300SoundList, false, 0, 0, 0, 0);
		_vm->_soundMan->stopMusic(0x00203197, 0, 2);
		_childObject = new Scene1303(_vm, this);
		break;
	case 3:
		_vm->gameState().sceneNum = 3;
		_vm->_soundMan->setSoundListParams(kModule1300SoundList, false, 0, 0, 0, 0);
		_vm->_soundMan->stopMusic(0x00203197, 0, 2);
		_childObject = new Scene1304(_vm, this, which);
		break;
	case 4:
		_vm->gameState().sceneNum = 4;
		_vm->_soundMan->setSoundListParams(kModule1300SoundList, false, 0, 0, 0, 0);
		_vm->_soundMan->startMusic(0x00203197, 0, 2);
		_childObject = new Scene1305(_vm, this, which);
		break;
	case 5:
		_vm->gameState().sceneNum = 5;
		_vm->_soundMan->setSoundListParams(kModule1300SoundList, false, 0, 0, 0, 0);
		_vm->_soundMan->startMusic(0x00203197, 0, 2);
		_childObject = new Scene1306(_vm, this, which);
		break;
	case 6:
		_vm->gameState().sceneNum = 6;
		_vm->_soundMan->setSoundListParams(kModule1300SoundList, false, 0, 0, 0, 0);
		_vm->_soundMan->startMusic(0x00203197, 0, 2);
		_childObject = new Scene1307(_vm, this);
		break;
	case 7:
		_vm->gameState().sceneNum = 7;
		_vm->_soundMan->setSoundListParams(kModule1300SoundList, false, 0, 0, 0, 0);
		_vm->_soundMan->startMusic(0x00203197, 0, 2);
		_childObject = new Scene1308(_vm, this, which);
		break;
	case 8:
		_vm->gameState().sceneNum = 8;
		_vm->_soundMan->setSoundListParams(kModule1300SoundList, false, 0, 0, 0, 0);
		_vm->_soundMan->stopMusic(0x00203197, 0, 2);
		_childObject = new DiskplayerScene(_vm, this, 1);
		break;
	case 9:
		_vm->gameState().sceneNum = 9;
		_vm->_soundMan->setSoundListParams(kModule1300SoundList, false, 0, 0, 0, 0);
		_vm->_soundMan->stopMusic(0x00203197, 0, 2);
		createSmackerScene(0x20082818, true, true, false);
		break;
	case 10:
		_vm->gameState().sceneNum = 10;
		_vm->_soundMan->setSoundListParams(kModule1300SoundList, false, 0, 0, 0, 0);
		_vm->_soundMan->stopMusic(0x00203197, 0, 2);
		createSmackerScene(0x20082828, true, true, false);
		break;
	case 11:
		_vm->gameState().sceneNum = 11;
		_vm->_soundMan->setSoundListParams(kModule1300SoundList, true, 0, 0, 0, 0);
		_vm->_soundMan->stopMusic(0x00203197, 0, 2);
		createNavigationScene(0x004B27A8, which);
		break;
	case 12:
		_vm->gameState().sceneNum = 12;
		_vm->_soundMan->setSoundListParams(kModule1300SoundList, true, 0, 0, 0, 0);
		_vm->_soundMan->stopMusic(0x00203197, 0, 2);
		createNavigationScene(0x004B2718, which);
		break;
	case 13:
		_vm->gameState().sceneNum = 13;
		_vm->_soundMan->setSoundListParams(kModule1300SoundList, true, 0, 0, 0, 0);
		_vm->_soundMan->stopMusic(0x00203197, 0, 2);
		createNavigationScene(0x004B27D8, which);
		break;
	case 14:
		_vm->gameState().sceneNum = 14;
		_vm->_soundMan->setSoundListParams(kModule1300SoundList, true, 0, 0, 0, 0);
		_vm->_soundMan->stopMusic(0x00203197, 0, 2);
		createNavigationScene(0x004B2808, which);
		break;
	case 15:
		_vm->gameState().sceneNum = 15;
		_vm->_soundMan->setSoundListParams(kModule1300SoundList, true, 0, 0, 0, 0);
		_vm->_soundMan->stopMusic(0x00203197, 0, 2);
		createNavigationScene(0x004B2838, which);
		break;
	case 16:
		_vm->gameState().sceneNum = 16;
		_vm->_soundMan->setSoundListParams(kModule1300SoundList, false, 0, 0, 0, 0);
		_vm->_soundMan->stopMusic(0x00203197, 0, 2);
		_childObject = new Scene1317(_vm, this);
		break;
	case 17:
		_vm->gameState().sceneNum = 17;
		_vm->_soundMan->setSoundListParams(kModule1300SoundList, false, 0, 0, 0, 0);
		_vm->_soundMan->stopMusic(0x00203197, 0, 2);
		_childObject = new CreditsScene(_vm, this, false);
		break;
	}
	SetUpdateHandler(&Module1300::updateScene);
	_childObject->handleUpdate();
}
			
void Module1300::updateScene() {
	if (!updateChild()) {
		switch (_sceneNum) {
		case 1:
			if (_moduleResult == 1)
				createScene(4, 0);
			else
				createScene(7, 1);
			break;
		case 2:
			createScene(5, 3);
			break;
		case 3:
			createScene(15, 0);
			break;
		case 4:
			createScene(16, -1);
			break;
		case 5:
			if (_moduleResult == 2)
				createScene(8, 0);
			else if (_moduleResult == 3)
				createScene(2, 0);
			else if (_moduleResult == 0)
				leaveModule(0);
			else if (_moduleResult == 1)
				createScene(10, -1);
			break;
		case 6:
			createScene(7, 2);
			break;
		case 7:
			if (_moduleResult == 0)
				createScene(13, 0);
			else if (_moduleResult == 1)
				createScene(1, 0);
			else if (_moduleResult == 2)
				createScene(6, 0);
			break;
		case 8:
			createScene(5, 2);
			break;
		case 9:
			createScene(5, 0);
			break;
		case 10:
			createScene(14, 0);
			break;
		case 11:
			if (_moduleResult == 0)
				createScene(12, 0);
			else if (_moduleResult == 1)
				createScene(11, 1);
			break;
		case 12:
			if (_moduleResult == 0) 
				createScene(14, 1);
			else if (_moduleResult == 1)
				createScene(15, 1);
			else if (_moduleResult == 3)
				createScene(11, 1);
			else if (_moduleResult == 5)
				createScene(13, 1);
			break;
		case 13:
			if (_moduleResult == 0)
				createScene(12, 2);
			else if (_moduleResult == 1)
				createScene(7, 0);
			break;
		case 14:
			if (_moduleResult == 0)
				createScene(12, 3);
			else if (_moduleResult == 1)
				createScene(9, -1);
			break;
		case 15:
			if (_moduleResult == 0)
				createScene(12, 4);
			else if (_moduleResult == 1)
				createScene(3, 0);
			break;
		case 16:
			createScene(17, -1);
			break;
		case 17:
			leaveModule(1);
			break;
		}
	}
}

AsScene1302Bridge::AsScene1302Bridge(NeverhoodEngine *vm, Scene *parentScene)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene) {

	_x = 320;
	_y = 240;
	createSurface1(0x88148150, 500);
	if (!getGlobalVar(V_FLYTRAP_RING_BRIDGE)) {
		startAnimation(0x88148150, 0, -1);
		_newStickFrameIndex = 0;
	} else {
		startAnimation(0x88148150, -1, -1);
		_newStickFrameIndex = STICK_LAST_FRAME;
	}
	loadSound(0, 0x68895082);
	loadSound(1, 0x689BD0C1);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1302Bridge::handleMessage);
}

uint32 AsScene1302Bridge::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		gotoNextState();
		break;
	case 0x4808:
		stLowerBridge();
		break;
	case 0x4809:
		stRaiseBridge();
		break;
	}
	return messageResult;
}

void AsScene1302Bridge::stLowerBridge() {
	startAnimation(0x88148150, 0, -1);
	playSound(1);
	NextState(&AsScene1302Bridge::cbLowerBridgeEvent);
}

void AsScene1302Bridge::stRaiseBridge() {
	startAnimation(0x88148150, 7, -1);
	_playBackwards = true;
	_newStickFrameIndex = 0;
	playSound(0);
}

void AsScene1302Bridge::cbLowerBridgeEvent() {
	sendMessage(_parentScene, 0x2032, 0);
	startAnimation(0x88148150, -1, -1);
	_newStickFrameIndex = STICK_LAST_FRAME;
}

SsScene1302Fence::SsScene1302Fence(NeverhoodEngine *vm)
	: StaticSprite(vm, 0x11122122, 200) {
	
	_firstY = _y;
	if (getGlobalVar(V_FLYTRAP_RING_FENCE))
		_y += 152;
	loadSound(0, 0x7A00400C);
	loadSound(1, 0x78184098);
	SetUpdateHandler(&SsScene1302Fence::update);
	SetMessageHandler(&SsScene1302Fence::handleMessage);
	SetSpriteUpdate(NULL);
}

void SsScene1302Fence::update() {
	handleSpriteUpdate();
	updatePosition();
}

uint32 SsScene1302Fence::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x4808:
		playSound(0);
		SetMessageHandler(NULL);
		SetSpriteUpdate(&SsScene1302Fence::suMoveDown);
		break;
	case 0x4809:
		playSound(1);
		SetMessageHandler(NULL);
		SetSpriteUpdate(&SsScene1302Fence::suMoveUp);
		break;
	}
	return messageResult;
}

void SsScene1302Fence::suMoveDown() {
	if (_y < _firstY + 152)
		_y += 8;
	else {
		SetMessageHandler(&SsScene1302Fence::handleMessage);
		SetSpriteUpdate(NULL);
	}
}

void SsScene1302Fence::suMoveUp() {
	if (_y > _firstY)
		_y -= 8;
	else {
		SetMessageHandler(&SsScene1302Fence::handleMessage);
		SetSpriteUpdate(NULL);
	}
}

Scene1302::Scene1302(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule) {

	SetMessageHandler(&Scene1302::handleMessage);

	setHitRects(0x004B0858);
	setRectList(0x004B0A38);
	setBackground(0x420643C4);
	setPalette(0x420643C4);
	insertScreenMouse(0x643C0428);

	_class595 = insertStaticSprite(0xB0420130, 1015);
	_sprite1 = insertStaticSprite(0x942FC224, 300);
	_sprite2 = insertStaticSprite(0x70430830, 1200);
	_sprite2->setVisible(false);
	_sprite3 = insertStaticSprite(0x16E01E20, 1100);
	_asRing1 = insertSprite<AsScene1002Ring>(this, false, 218, 122, _class595->getDrawRect().y, false);
	_asRing2 = insertSprite<AsScene1002Ring>(this, true, 218 + 32, 132, _class595->getDrawRect().y, getGlobalVar(V_FLYTRAP_RING_BRIDGE));
	_asRing3 = insertSprite<AsScene1002Ring>(this, false, 218 + 32 + 32, 122, _class595->getDrawRect().y, false);
	_asRing4 = insertSprite<AsScene1002Ring>(this, true, 218 + 32 + 32 + 32, 132, _class595->getDrawRect().y, getGlobalVar(V_FLYTRAP_RING_FENCE));
	_asRing5 = insertSprite<AsScene1002Ring>(this, false, 218 + 32 + 32 + 32 + 32, 115, _class595->getDrawRect().y, false);
	_asBridge = insertSprite<AsScene1302Bridge>(this);
	_ssFence = insertSprite<SsScene1302Fence>();
	_ssFence->setClipRect(0, 0, 640, _sprite1->getDrawRect().y2());

	if (which < 0) {
		// Restoring game
		insertKlaymen<KmScene1002>(380, 364);
		setMessageList(0x004B0868);
	} else {
		// Klaymen entering from back
		insertKlaymen<KmScene1002>(293, 330);
		setMessageList(0x004B0870);
	}

	_klaymen->setClipRect(0, 0, _sprite3->getDrawRect().x2(), 480);

	_asVenusFlyTrap = insertSprite<AsScene1002VenusFlyTrap>(this, _klaymen, true);
	addCollisionSprite(_asVenusFlyTrap);

	sendEntityMessage(_klaymen, 0x2007, _asVenusFlyTrap);
	
}

uint32 Scene1302::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = 0;
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x4A845A00)
			sendEntityMessage(_klaymen, 0x1014, _asRing1);
		else if (param.asInteger() == 0x43807801) {
			if (!getGlobalVar(V_FLYTRAP_RING_BRIDGE)) {
				sendEntityMessage(_klaymen, 0x1014, _asRing2);
				if (_asVenusFlyTrap->getX() - 10 < 218 + 32 && _asVenusFlyTrap->getX() + 10 > 218 + 32)
					setMessageList(0x004B0940);
				else
					setMessageList(0x004B0938);
			} else
				setMessageList(0x004B0950);
			messageResult = 1;
		} else if (param.asInteger() == 0x46C26A01)
			sendEntityMessage(_klaymen, 0x1014, _asRing3);
		else if (param.asInteger() == 0x468C7B11) {
			if (!getGlobalVar(V_FLYTRAP_RING_FENCE)) {
				sendEntityMessage(_klaymen, 0x1014, _asRing4);
				if (_asVenusFlyTrap->getX() - 10 < 218 + 32 + 32 + 32 && _asVenusFlyTrap->getX() + 10 > 218 + 32 + 32 + 32)
					setMessageList(0x004B0940);
				else
					setMessageList(0x004B0938);
			} else
				setMessageList(0x004B0950);
			messageResult = 1;
		} else if (param.asInteger() == 0x42845B19)
			sendEntityMessage(_klaymen, 0x1014, _asRing5);
		else if (param.asInteger() == 0x430A6060) {
			if (getGlobalVar(V_FLYTRAP_RING_BRIDGE))
				setMessageList2(0x004B0910);
			else
				cancelMessageList();
		} else if (param.asInteger() == 0x012E2070) {
			if (getGlobalVar(V_FLYTRAP_RING_BRIDGE))
				setMessageList2(0x004B0968);
			else
				cancelMessageList();
		} else if (param.asInteger() == 0x11C40840) {
			if (_asVenusFlyTrap->getX() >= 260 && _asVenusFlyTrap->getX() <= 342)
				setMessageList(0x004B0878);
			else
				setMessageList(0x004B0978);
		}
		break;
	case 0x2000:
		if (_klaymen->getY() > 360) {
			sendEntityMessage(_klaymen, 0x1014, _asVenusFlyTrap);
			setMessageList2(0x004B08F0);	
		} else
			setMessageList2(0x004B0920);
		break;
	case 0x2002:
		if (_klaymen->getX() > 545)
			leaveScene(1);
		break;
	case 0x2032:
		_sprite2->setVisible(true);
		break;
	case 0x4806:
		sendMessage(_parentModule, 0x1024, 2);
		if (sender == _asRing1)
			playSound(0, 0x665198C0);
		else if (sender == _asRing2) {
			sendMessage(_asBridge, 0x4808, 0);
			setGlobalVar(V_FLYTRAP_RING_BRIDGE, 1);
		} else if (sender == _asRing3)
			playSound(0, 0xE2D389C0);
		else if (sender == _asRing4) {
			sendMessage(_ssFence, 0x4808, 0);
			setGlobalVar(V_FLYTRAP_RING_FENCE, 1);
		} else if (sender == _asRing5)
			playSound(0, 0x40428A09);
		break;
	case 0x4807:
		if (sender == _asRing2) {
			sendMessage(_asBridge, 0x4809, 0);
			setGlobalVar(V_FLYTRAP_RING_BRIDGE, 0);
			_sprite2->setVisible(false);
		} else if (sender == _asRing4) {
			sendMessage(_ssFence, 0x4809, 0);
			setGlobalVar(V_FLYTRAP_RING_FENCE, 0);
		} else if (sender == _asVenusFlyTrap) {
			if (getGlobalVar(V_FLYTRAP_RING_BRIDGE))
				sendMessage(_asRing2, 0x4807, 0);
			else
				sendMessage(_asRing4, 0x4807, 0);
		}
		break;
	case 0x480F:
		if (sender == _asRing2) {
			playSound(0, 0x60755842);
			sendMessage(_asBridge, 0x4808, 0);
			setGlobalVar(V_FLYTRAP_RING_BRIDGE, 1);
		} else if (sender == _asRing4) {
			playSound(0, 0x60755842);
			sendMessage(_ssFence, 0x4808, 0);
			setGlobalVar(V_FLYTRAP_RING_FENCE, 1);
		}
		break;
	case 0x482A:
		sendMessage(_asVenusFlyTrap, 0x482B, 0);
		break;
	case 0x482B:
		sendMessage(_asVenusFlyTrap, 0x482A, 0);
		break;
	case 0x8000:
		setSpriteSurfacePriority(_class595, 995);
		break;
	case 0x8001:
		setSpriteSurfacePriority(_class595, 1015);
		break;
	}
	return messageResult;
}

AsScene1303Balloon::AsScene1303Balloon(NeverhoodEngine *vm, Scene *parentScene)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene) {
	
	createSurface(200, 128, 315);
	_x = 289;
	_y = 390;
	startAnimation(0x800278D2, 0, -1);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1303Balloon::handleMessage);
	SetSpriteUpdate(&AnimatedSprite::updateDeltaXY);
}

uint32 AsScene1303Balloon::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		sendMessage(_parentScene, 0x4826, 0);
		messageResult = 1;
		break;
	case 0x2000:
		stPopBalloon();
		break;
	}
	return messageResult;
}

uint32 AsScene1303Balloon::hmBalloonPopped(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x020B0003)
			playSound(0, 0x742B0055);
		break;
	case 0x3002:
		playSound(0, 0x470007EE);
		stopAnimation();
		setVisible(false);
		SetMessageHandler(NULL);
		break;
	}
	return messageResult;
}

void AsScene1303Balloon::stPopBalloon() {
	startAnimation(0xAC004CD0, 0, -1);
	SetMessageHandler(&AsScene1303Balloon::hmBalloonPopped);
}

Scene1303::Scene1303(NeverhoodEngine *vm, Module *parentModule)
	: Scene(vm, parentModule) {

	SetMessageHandler(&Scene1303::handleMessage);

	setRectList(0x004AF9E8);
	setBackground(0x01581A9C);
	setPalette(0x01581A9C);
	insertScreenMouse(0x81A9801D);

	if (!getGlobalVar(V_BALLOON_POPPED)) {
		_asBalloon = insertSprite<AsScene1303Balloon>(this);
		addCollisionSprite(_asBalloon);
	}
	
	_sprite1 = insertStaticSprite(0xA014216B, 1100);

	insertKlaymen<KmScene1303>(207, 332);
	setMessageList(0x004AF9A0);

	_klaymen->setClipRect(_sprite1->getDrawRect().x, 0, 640, 480);

}

uint32 Scene1303::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2000:
		setGlobalVar(V_BALLOON_POPPED, 1);
		sendMessage(_asBalloon, 0x2000, 0);
		break;
	case 0x4826:
		if (sender == _asBalloon && getGlobalVar(V_HAS_NEEDLE))
			setMessageList(0x004AF9B8);
		break;
	}
	return 0;
}

AsScene1304Needle::AsScene1304Needle(NeverhoodEngine *vm, Scene *parentScene, int surfacePriority, int16 x, int16 y)
	: AnimatedSprite(vm, 0x548E9411, surfacePriority, x, y), _parentScene(parentScene) {

	// NOTE: Skipped check if Klaymen already has the needle since that's done in the scene itself	
	SetMessageHandler(&AsScene1304Needle::handleMessage);
}

uint32 AsScene1304Needle::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		sendMessage(_parentScene, 0x4826, 0);
		messageResult = 1;
		break;
	case 0x4806:
		setGlobalVar(V_HAS_NEEDLE, 1);
		setVisible(false);
		SetMessageHandler(NULL);
		break;
	}
	return messageResult;
}

Scene1304::Scene1304(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule), _asNeedle(NULL) {
	
	SetMessageHandler(&Scene1304::handleMessage);

	setRectList(0x004B91A8);
	setBackground(0x062C0214);
	setPalette(0x062C0214);
	insertScreenMouse(0xC021006A);
	
	if (getGlobalVar(V_BALLOON_POPPED)) {
		_asKey = insertSprite<AsCommonKey>(this, 0, 1100, 278, 347);
		addCollisionSprite(_asKey);
	} else {
		_asKey = insertSprite<AnimatedSprite>(0x80106018, 100, 279, 48);
	}

	if (!getGlobalVar(V_HAS_NEEDLE)) {
		_asNeedle = insertSprite<AsScene1304Needle>(this, 1100, 278, 347);
		addCollisionSprite(_asNeedle);
	}

	_sprite1 = insertStaticSprite(0x0562E621, 1100);
	insertStaticSprite(0x012AE033, 1100);
	insertStaticSprite(0x090AF033, 1100);

	if (which < 0) {
		// Restoring game
		insertKlaymen<KmScene1304>(217, 347);
		setMessageList(0x004B90E8);
	} else {
		// Klaymen entering from the left
		insertKlaymen<KmScene1304>(100, 347);
		setMessageList(0x004B90F0);
	}

	_klaymen->setClipRect(_sprite1->getDrawRect().x, 0, 640, 480);

}

uint32 Scene1304::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x415634A4) {
			if (getGlobalVar(V_BALLOON_POPPED))
				cancelMessageList();
			else
				setMessageList(0x004B9158);
		}
		break;
	case 0x4826:
		if (sender == _asNeedle) {
			sendEntityMessage(_klaymen, 0x1014, _asNeedle);
			setMessageList(0x004B9130);
		} else if (sender == _asKey) {
			sendEntityMessage(_klaymen, 0x1014, _asKey);
			setMessageList(0x004B9140);
		}
		break;
	}
	return 0;
}

Scene1305::Scene1305(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule) {

	SetMessageHandler(&Scene1305::handleMessage);

	setRectList(0x004B6E98);
	setBackground(0x28801B64);
	setPalette(0x28801B64);
	insertScreenMouse(0x01B60280);

	if (which < 0) {
		// Restoring game
		insertKlaymen<KmScene1305>(212, 441);
		setMessageList(0x004B6E40);
	} else {
		// Klaymen enters falling
		insertKlaymen<KmScene1305>(212, 441);
		setMessageList(0x004B6E48);
	}
	
}

uint32 Scene1305::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	return Scene::handleMessage(messageNum, param, sender);
}

AsScene1306Elevator::AsScene1306Elevator(NeverhoodEngine *vm, Scene *parentScene, AnimatedSprite *asElevatorDoor)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _asElevatorDoor(asElevatorDoor), _isUp(false), _isDown(true),
	_countdown(0) {

	_x = 320;
	_y = 240;
	createSurface1(0x043B0270, 100);
	startAnimation(0x043B0270, 0, -1);
	_newStickFrameIndex = 0;
	loadSound(0, 0x1C100E83);
	loadSound(1, 0x1C08CEC5);
	loadSound(2, 0x5D011E87);
	SetMessageHandler(&AsScene1306Elevator::handleMessage);
}

void AsScene1306Elevator::update() {
	if (_isUp && _countdown != 0 && (--_countdown == 0))
		stGoingDown();
	AnimatedSprite::update();
	if (_currFrameIndex == 7) {
		playSound(1);
		_asElevatorDoor->setVisible(false);
	}
}

void AsScene1306Elevator::upGoingDown() {
	AnimatedSprite::update();
	if (_currFrameIndex == 5)
		_asElevatorDoor->setVisible(true);
}

uint32 AsScene1306Elevator::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2001:
		if (_isUp)
			_countdown = 144;
		messageResult = _isUp ? 1 : 0;
		break;
	case 0x3002:
		gotoNextState();
		break;
	case 0x4808:
		if (_isDown)
			stGoingUp();
		break;
	}
	return messageResult;
}

void AsScene1306Elevator::stGoingUp() {
	setVisible(true);
	_isDown = false;
	startAnimation(0x043B0270, 0, -1);
	playSound(0);
	SetUpdateHandler(&AsScene1306Elevator::update);
	NextState(&AsScene1306Elevator::cbGoingUpEvent);
}

void AsScene1306Elevator::cbGoingUpEvent() {
	sendMessage(_parentScene, 0x4808, 0);
	_isUp = true;
	_countdown = 144;
	stopAnimation();
	setVisible(false);
	SetUpdateHandler(&AsScene1306Elevator::update);
}

void AsScene1306Elevator::stGoingDown() {
	_isUp = false;
	setVisible(true);
	startAnimation(0x043B0270, -1, -1);
	_playBackwards = true;
	playSound(1);
	SetUpdateHandler(&AsScene1306Elevator::upGoingDown);
	NextState(&AsScene1306Elevator::cbGoingDownEvent);
}

void AsScene1306Elevator::cbGoingDownEvent() {
	_isDown = true;
	sendMessage(_parentScene, 0x4809, 0);
	stopAnimation();
	SetUpdateHandler(&AsScene1306Elevator::update);
}

Scene1306::Scene1306(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule) {
	
	if (getGlobalVar(V_HAS_FINAL_KEY) && getGlobalVar(V_KEY3_LOCATION) == 0)
		setGlobalVar(V_KEY3_LOCATION, 4);
	
	SetMessageHandler(&Scene1306::handleMessage);

	setBackground(0x05303114);
	setPalette(0x05303114);
	insertScreenMouse(0x0311005B);

	if (getGlobalVar(V_KEY3_LOCATION) == 4) {
		_asKey = insertSprite<AsCommonKey>(this, 2, 1100, 435, 445);
		addCollisionSprite(_asKey);
	}

	_ssButton = insertSprite<SsCommonButtonSprite>(this, 0x404A36A0, 100, 0x440C1000);
	_asTape = insertSprite<AsScene1201Tape>(this, 19, 1100, 359, 445, 0x9148A011);
	_asElevatorDoor = insertSprite<AnimatedSprite>(0x043B0270, 90, 320, 240);
	_asElevatorDoor->startAnimation(0x043B0270, 6, -1);
	_asElevatorDoor->setNewHashListIndex(6);
	_asElevator = insertSprite<AsScene1306Elevator>(this, _asElevatorDoor);
	_sprite1 = insertStaticSprite(0x036A1EE0, 80);
	insertStaticSprite(0x00042313, 1100);

	if (which < 0) {
		// Resoring game
		insertKlaymen<KmScene1306>(380, 440);
		setMessageList(0x004AFAD0);
		sendMessage(this, 0x2000, 0);
		addCollisionSprite(_asTape);
	} else if (which == 1) {
		// Klaymen teleporting in
		insertKlaymen<KmScene1306>(136, 440);
		sendMessage(_klaymen, 0x2000, 1);
		setMessageList(0x004AFAF0);
		sendMessage(this, 0x2000, 1);
		addCollisionSprite(_asTape);
	} else if (which == 2) {
		// Klaymen returning from diskplayer
		if (getGlobalVar(V_KLAYMEN_IS_DELTA_X)) {
			insertKlaymen<KmScene1306>(515, 440);
			_klaymen->setDoDeltaX(1);
		} else {
			insertKlaymen<KmScene1306>(355, 440);
		}
		setMessageList(0x004AFBC8);
		sendMessage(this, 0x2000, 0);
		addCollisionSprite(_asTape);
	} else if (which == 3) {
		// Klaymen returning from window
		insertKlaymen<KmScene1306>(534, 440);
		setMessageList(0x004AFC30);
		sendMessage(this, 0x2000, 0);
		addCollisionSprite(_asTape);
	} else if (which == 4) {
		// Klaymen teleporting out
		insertKlaymen<KmScene1306>(136, 440);
		sendMessage(_klaymen, 0x2000, 1);
		setMessageList(0x004AFC38);
		sendMessage(this, 0x2000, 1);
		addCollisionSprite(_asTape);
	} else if (which == 5) {
		// Klaymen returning from teleporter
		insertKlaymen<KmScene1306>(136, 440);
		sendMessage(_klaymen, 0x2000, 1);
		setMessageList(0x004AFB00);
		sendMessage(this, 0x2000, 1);
		addCollisionSprite(_asTape);
	} else {
		// Klaymen coming up in elevator
		insertKlaymen<KmScene1306>(286, 408);
		setSurfacePriority(_asElevator->getSurface(), 1100);
		setSurfacePriority(_asElevatorDoor->getSurface(), 1090);
		setSurfacePriority(_sprite1->getSurface(), 1080);
		sendMessage(this, 0x2000, 0);
		SetMessageHandler(&Scene1306::handleMessage416EB0);
		clearRectList();
		sendMessage(_asElevator, 0x4808, 0);
	}

}
	
Scene1306::~Scene1306() {
	setGlobalVar(V_KLAYMEN_IS_DELTA_X, _klaymen->isDoDeltaX() ? 1 : 0);
}

uint32 Scene1306::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x402064D8)
			sendEntityMessage(_klaymen, 0x1014, _ssButton);
		else if (param.asInteger() == 0x01C66840) {
			if (sendMessage(_asElevator, 0x2001, 0) != 0)
				setMessageList(0x004AFBD8);
			else
				setMessageList(0x004AFAE0);
		} else if (param.asInteger() == 0x8E646E00) {
			setMessageList(0x004AFAD8);
			clearRectList();
			SetMessageHandler(&Scene1306::handleMessage416EB0);
		}
		break;
	case 0x2000:
		if (param.asInteger() != 0) {
			setRectList(0x004AFD28);
			_klaymen->setKlaymenIdleTable3();
		} else {
			setRectList(0x004AFD18);
			_klaymen->setKlaymenIdleTable1();
		}
		break;
	case 0x480B:
		if (sender == _ssButton)
			sendMessage(_asElevator, 0x4808, 0);
		break;
	case 0x4826:
		if (sender == _asKey) {
			if (_klaymen->getX() >= 249) {
				sendEntityMessage(_klaymen, 0x1014, _asKey);
				setMessageList(0x004AFC58);
			}
		} else if (sender == _asTape) {
			if (_klaymen->getX() >= 249) {
				sendEntityMessage(_klaymen, 0x1014, _asTape);
				setMessageList(0x004AFC68);
			}
		}
		break;
	case 0x482A:
		setSurfacePriority(_asElevator->getSurface(), 1100);
		setSurfacePriority(_asElevatorDoor->getSurface(), 1090);
		setSurfacePriority(_sprite1->getSurface(), 1080);
		break;
	case 0x482B:
		setSurfacePriority(_asElevator->getSurface(), 100);
		setSurfacePriority(_asElevatorDoor->getSurface(), 90);
		setSurfacePriority(_sprite1->getSurface(), 80);
		sendMessage(this, 0x2000, 0);
		addCollisionSprite(_asTape);
		break;
	}
	return 0;
}

uint32 Scene1306::handleMessage416EB0(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x4808:
		setMessageList(0x004AFBD0);
		SetMessageHandler(&Scene1306::handleMessage);
		break;
	case 0x4809:
		leaveScene(1);
		break;
	case 0x482A:
		setSurfacePriority(_asElevator->getSurface(), 1100);
		setSurfacePriority(_asElevatorDoor->getSurface(), 1090);
		setSurfacePriority(_sprite1->getSurface(), 1080);
		break;
	case 0x482B:
		setSurfacePriority(_asElevator->getSurface(), 100);
		setSurfacePriority(_asElevatorDoor->getSurface(), 90);
		setSurfacePriority(_sprite1->getSurface(), 80);
		sendMessage(this, 0x2000, 0);
		addCollisionSprite(_asTape);
		break;
	}
	return 0;
}

static const uint32 kAsScene1307KeyResourceList1[] = {
	0x0438069C, 0x45B0023C, 0x05700217
};

static const uint32 kAsScene1307KeyResourceList2[] = {
	0x04441334, 0x061433F0, 0x06019390
};

static const uint32 kAsScene1307KeyResourceList3[] = {
	0x11A80030, 0x178812B1, 0x1488121C
};

static const uint32 *kAsScene1307KeyResourceLists[] = {
	kAsScene1307KeyResourceList1,
	kAsScene1307KeyResourceList2,
	kAsScene1307KeyResourceList3
};

static const int kAsScene1307KeySurfacePriorities[] = {
	700, 500, 300, 100
};

const uint kAsScene1307KeyPointsCount = 12;

static const NPoint kAsScene1307KeyPoints[] = {
	{-2,  0}, {-5,  0}, { 5,  0},
	{12,  0}, {17,  0}, {25,  0},
	{16, -2}, {10, -6}, { 0, -7},
	{-7, -3}, {-3,  4}, { 2,  2}
};

const uint kAsScene1307KeyFrameIndicesCount = 20;

static const int16 kAsScene1307KeyFrameIndices[] = {
	 1,  4,  8, 11, 15, 16, 17, 17, 17, 16, 
	15, 14, 12, 10,  9,  7,  5,  3,  2,  1
};

const int kAsScene1307KeyDivValue = 200;
const int16 kAsScene1307KeyXDelta = 70;
const int16 kAsScene1307KeyYDelta = -12;

AsScene1307Key::AsScene1307Key(NeverhoodEngine *vm, Scene *parentScene, uint keyIndex, NRect *clipRects)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _keyIndex(keyIndex), _clipRects(clipRects),
	_isClickable(true) {
	
	NPoint pt;
	const uint32 *fileHashes = kAsScene1307KeyResourceLists[_keyIndex]; 
	
	_dataResource.load(0x22102142);
	_pointList = _dataResource.getPointArray(0xAC849240);
	pt = (*_pointList)[getSubVar(VA_CURR_KEY_SLOT_NUMBERS, _keyIndex)];
	_x = pt.x;
	_y = pt.y;
	createSurface(kAsScene1307KeySurfacePriorities[getSubVar(VA_CURR_KEY_SLOT_NUMBERS, _keyIndex) % 4], 190, 148);
	startAnimation(fileHashes[0], 0, -1);
	loadSound(0, 0xDC4A1280);
	loadSound(1, 0xCC021233);
	loadSound(2, 0xC4C23844);
	loadSound(3, 0xC4523208);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1307Key::handleMessage);
}

uint32 AsScene1307Key::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_isClickable) {
			sendMessage(_parentScene, 0x4826, 0);
			stRemoveKey();
			messageResult = 1;
		}
		break;
	case 0x2000:
		_isClickable = param.asInteger() != 0;
		break;
	case 0x2001:
		setSubVar(VA_CURR_KEY_SLOT_NUMBERS, _keyIndex, param.asInteger());
		stMoveKey();
		break;
	case 0x2003:
		playSound(3);
		stUnlock();
		break;
	case 0x2004:
		playSound(2);
		stInsert();
		break;
	}
	return messageResult;
}

void AsScene1307Key::suRemoveKey() {
	if (_pointIndex < kAsScene1307KeyPointsCount) {
		_x += kAsScene1307KeyPoints[_pointIndex].x;
		_y += kAsScene1307KeyPoints[_pointIndex].y;
		updateBounds();
		_pointIndex++;
	} else {
		SetSpriteUpdate(NULL);
	}
}

void AsScene1307Key::suInsertKey() {
	if (_pointIndex < kAsScene1307KeyPointsCount) {
		_x -= kAsScene1307KeyPoints[kAsScene1307KeyPointsCount - _pointIndex - 1].x;
		_y -= kAsScene1307KeyPoints[kAsScene1307KeyPointsCount - _pointIndex - 1].y;
		updateBounds();
		_pointIndex++;
		if (_pointIndex == 7)
			playSound(0);
	} else {
		SetSpriteUpdate(NULL);
		sendMessage(_parentScene, 0x2002, 0);
	}
}

void AsScene1307Key::suMoveKey() {
	if (_pointIndex < kAsScene1307KeyFrameIndicesCount) {
		_frameIndex += kAsScene1307KeyFrameIndices[_pointIndex];
		_x = _prevX + (_deltaX * _frameIndex) / kAsScene1307KeyDivValue;
		_y = _prevY + (_deltaY * _frameIndex) / kAsScene1307KeyDivValue;
		updateBounds();
		_pointIndex++;
	} else {
		NPoint pt = (*_pointList)[getSubVar(VA_CURR_KEY_SLOT_NUMBERS, _keyIndex)];
		_x = pt.x + kAsScene1307KeyXDelta;
		_y = pt.y + kAsScene1307KeyYDelta;
		stInsertKey();
	}
}

void AsScene1307Key::stRemoveKey() {
	const uint32 *fileHashes = kAsScene1307KeyResourceLists[_keyIndex]; 
	_pointIndex = 0;
	startAnimation(fileHashes[0], 0, -1);
	playSound(1);
	SetSpriteUpdate(&AsScene1307Key::suRemoveKey);
}

void AsScene1307Key::stInsertKey() {
	_pointIndex = 0;
	sendMessage(_parentScene, 0x1022, kAsScene1307KeySurfacePriorities[getSubVar(VA_CURR_KEY_SLOT_NUMBERS, _keyIndex) % 4]);
	setClipRect(_clipRects[getSubVar(VA_CURR_KEY_SLOT_NUMBERS, _keyIndex) % 4]);
	_newStickFrameIndex = STICK_LAST_FRAME;
	SetSpriteUpdate(&AsScene1307Key::suInsertKey);
}

void AsScene1307Key::stMoveKey() {
	NPoint pt = (*_pointList)[getSubVar(VA_CURR_KEY_SLOT_NUMBERS, _keyIndex)];
	int16 newX = pt.x + kAsScene1307KeyXDelta;
	int16 newY = pt.y + kAsScene1307KeyYDelta;
	sendMessage(_parentScene, 0x1022, 1000);
	setClipRect(0, 0, 640, 480);
	_prevX = _x;
	_prevY = _y;
	if (newX == _x && newY == _y) {
		stInsertKey();
	} else {
		const uint32 *fileHashes = kAsScene1307KeyResourceLists[_keyIndex]; 
		_pointIndex = 0;
		_frameIndex = 0;
		_deltaX = newX - _x;
		_deltaY = newY - _y;
		startAnimation(fileHashes[0], 0, -1);
		SetSpriteUpdate(&AsScene1307Key::suMoveKey);
	}
}

void AsScene1307Key::stUnlock() {
	const uint32 *fileHashes = kAsScene1307KeyResourceLists[_keyIndex]; 
	startAnimation(fileHashes[1], 0, -1);
	_newStickFrameIndex = STICK_LAST_FRAME;
}

void AsScene1307Key::stInsert() {
	const uint32 *fileHashes = kAsScene1307KeyResourceLists[_keyIndex]; 
	startAnimation(fileHashes[2], 0, -1);
	_newStickFrameIndex = STICK_LAST_FRAME;
}

Scene1307::Scene1307(NeverhoodEngine *vm, Module *parentModule)
	: Scene(vm, parentModule), _countdown(0), _asCurrKey(NULL),
	_isInsertingKey(false), _doLeaveScene(false), _isPuzzleSolved(false) {

	Sprite *tempSprite;
	
	_vm->gameModule()->initKeySlotsPuzzle();
	
	_dataResource.load(0x22102142);
	_keyHolePoints = _dataResource.getPointArray(0xAC849240);

	for (uint i = 0; i < 16; i++) {
		NPoint pt = (*_keyHolePoints)[i];
		_keyHoleRects[i].x1 = pt.x - 15;
		_keyHoleRects[i].y1 = pt.y - 15;
		_keyHoleRects[i].x2 = pt.x + 15;
		_keyHoleRects[i].y2 = pt.y + 15;
	}

	SetMessageHandler(&Scene1307::handleMessage);
	SetUpdateHandler(&Scene1307::update);

	setBackground(0xA8006200);
	setPalette(0xA8006200);
	addEntity(_palette);
	insertPuzzleMouse(0x06204A88, 20, 620);

	tempSprite = insertStaticSprite(0x00A3621C, 800);
	_clipRects[0].set(tempSprite->getDrawRect().x, 0, 640, 480);
	tempSprite = insertStaticSprite(0x00A3641C, 600);
	_clipRects[1].set(tempSprite->getDrawRect().x, 0, 640, 480);
	tempSprite = insertStaticSprite(0x00A3681C, 400);
	_clipRects[2].set(tempSprite->getDrawRect().x, 0, 640, 480);
	tempSprite = insertStaticSprite(0x00A3701C, 200);
	_clipRects[3].set(tempSprite->getDrawRect().x, 0, 640, 480);

	for (uint keyIndex = 0; keyIndex < 3; keyIndex++) {
		if (getSubVar(VA_IS_KEY_INSERTED, keyIndex)) {
			_asKeys[keyIndex] = insertSprite<AsScene1307Key>(this, keyIndex, _clipRects);
			addCollisionSprite(_asKeys[keyIndex]);
		} else {
			_asKeys[keyIndex] = NULL;
		}
	}

	loadSound(0, 0x68E25540);

}

void Scene1307::update() {
	Scene::update();
	if (_countdown && (--_countdown == 0))
		_doLeaveScene = true;
	else if (_countdown == 20)
		_palette->startFadeToWhite(40);
	if (_doLeaveScene && !isSoundPlaying(0)) {
		leaveScene(1);
		setGlobalVar(V_KEYDOOR_UNLOCKED, 1);
	} 
}

uint32 Scene1307::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = 0;
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		if (!_isPuzzleSolved) {
			if (param.asPoint().x > 20 && param.asPoint().x < 620) {
				if (_asCurrKey && !_isInsertingKey) {
					int16 mouseX = param.asPoint().x;
					int16 mouseY = param.asPoint().y;
					uint clickedKeyHoleIndex;
					for (clickedKeyHoleIndex = 0; clickedKeyHoleIndex < 16; clickedKeyHoleIndex++) {
						if (mouseX >= _keyHoleRects[clickedKeyHoleIndex].x1 && mouseX <= _keyHoleRects[clickedKeyHoleIndex].x2 && 
							mouseY >= _keyHoleRects[clickedKeyHoleIndex].y1 && mouseY <= _keyHoleRects[clickedKeyHoleIndex].y2)
							break;
					}
					if (clickedKeyHoleIndex < 16) {
						// Check if the clicked keyhole is already occupied with a key
						bool occupied = false;
						for (uint keyIndex = 0; keyIndex < 3 && !occupied; keyIndex++) {
							if (getSubVar(VA_IS_KEY_INSERTED, keyIndex) && _asKeys[keyIndex] != _asCurrKey) {
								if (getSubVar(VA_CURR_KEY_SLOT_NUMBERS, keyIndex) == clickedKeyHoleIndex)
									occupied = true;
							}
						}
						if (!occupied) {
							// If the keyhole is free, insert the current key
							sendMessage(_asCurrKey, 0x2001, clickedKeyHoleIndex);
							_isInsertingKey = true;
							_mouseClicked = false;
						}
					}
				}
			} else if (_countdown == 0 && !_asCurrKey && !_isInsertingKey)
				leaveScene(0);
		}
		break;
	case 0x2002:
		// Check if all keys are in the correct keyholes
		if (getSubVar(VA_IS_KEY_INSERTED, 0) && getSubVar(VA_CURR_KEY_SLOT_NUMBERS, 0) == getSubVar(VA_GOOD_KEY_SLOT_NUMBERS, 0) &&
			getSubVar(VA_IS_KEY_INSERTED, 1) && getSubVar(VA_CURR_KEY_SLOT_NUMBERS, 1) == getSubVar(VA_GOOD_KEY_SLOT_NUMBERS, 1) &&
			getSubVar(VA_IS_KEY_INSERTED, 2) && getSubVar(VA_CURR_KEY_SLOT_NUMBERS, 2) == getSubVar(VA_GOOD_KEY_SLOT_NUMBERS, 2)) {
			// Play unlock animations for all keys
			for (uint keyIndex = 0; keyIndex < 3; keyIndex++) {
				if (_asKeys[keyIndex])
					sendMessage(_asKeys[keyIndex], 0x2003, 1);
			}
			playSound(0);
			_isPuzzleSolved = true;
			_countdown = 47;
		} else {
			for (uint keyIndex = 0; keyIndex < 3; keyIndex++)
				if (getSubVar(VA_IS_KEY_INSERTED, keyIndex) && _asKeys[keyIndex])
					sendMessage(_asKeys[keyIndex], 0x2000, 1);
			sendMessage(_asCurrKey, 0x2004, 1);
		}
		_asCurrKey = NULL;
		_isInsertingKey = false;
		break;
	case 0x4826:
		_asCurrKey = (Sprite*)sender;
		for (uint keyIndex = 0; keyIndex < 3; keyIndex++)
			if (getSubVar(VA_IS_KEY_INSERTED, keyIndex) && _asKeys[keyIndex])
				sendMessage(_asKeys[keyIndex], 0x2000, 0);
		break;
	}
	return messageResult;
}

static const uint32 kScene1308NumberFileHashes[] = {
	0x08006320, 0x10006320, 0x20006320,
	0x40006320, 0x80006320, 0x00006321,
	0x00006322, 0x00006324, 0x00006328,
	0x08306320, 0x10306320, 0x20306320,
	0x40306320, 0x80306320, 0x00306321,
	0x00306322
};

AsScene1308JaggyDoor::AsScene1308JaggyDoor(NeverhoodEngine *vm, Scene *parentScene)
	: AnimatedSprite(vm, 0xBA0AE050, 1100, 320, 240), _parentScene(parentScene) {
	
	setVisible(false);
	stopAnimation();
	SetMessageHandler(&AsScene1308JaggyDoor::handleMessage);
}

uint32 AsScene1308JaggyDoor::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		gotoNextState();
		break;
	case 0x4808:
		stOpenDoor();
		break;
	case 0x4809:
		stCloseDoor();
		break;
	}
	return messageResult;
}

void AsScene1308JaggyDoor::stOpenDoor() {
	startAnimation(0xBA0AE050, 0, -1);
	setVisible(true);
	playSound(0, calcHash("fxDoorOpen38"));
	NextState(&AsScene1308JaggyDoor::stOpenDoorDone);
}

void AsScene1308JaggyDoor::stOpenDoorDone() {
	sendMessage(_parentScene, 0x2000, 0);
	stopAnimation();
	setVisible(false);
}

void AsScene1308JaggyDoor::stCloseDoor() {
	startAnimation(0xBA0AE050, -1, -1);
	_playBackwards = true;
	setVisible(true);
	playSound(0, calcHash("fxDoorClose38"));
	NextState(&AsScene1308JaggyDoor::stCloseDoorDone);
}

void AsScene1308JaggyDoor::stCloseDoorDone() {
	sendMessage(_parentScene, 0x2001, 0);
	stopAnimation();
}

AsScene1308KeyboardDoor::AsScene1308KeyboardDoor(NeverhoodEngine *vm, Scene *parentScene)
	: AnimatedSprite(vm, 0xA08A0851, 1100, 320, 240), _parentScene(parentScene) {
	
	playSound(0, 0x51456049);
	SetMessageHandler(&AsScene1308KeyboardDoor::handleMessage);
	NextState(&AsScene1308KeyboardDoor::stFallingKeys);
}
 
uint32 AsScene1308KeyboardDoor::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		gotoNextState();
		break;
	}
	return messageResult;
}

void AsScene1308KeyboardDoor::stFallingKeys() {
	startAnimation(0x6238B191, 0, -1);
	_x = 580;
	_y = 383;
	NextState(&AsScene1308KeyboardDoor::stFallingKeysDone);
}

void AsScene1308KeyboardDoor::stFallingKeysDone() {
	sendMessage(_parentScene, 0x2004, 0);
	stopAnimation();
	setVisible(false);
}

AsScene1308LightWallSymbols::AsScene1308LightWallSymbols(NeverhoodEngine *vm, Scene *parentScene)
	: AnimatedSprite(vm, 0x80180A10, 100, 320, 240), _parentScene(parentScene) {
	
	setVisible(false);
	stopAnimation();
	Entity::_priority = 1200;
	SetMessageHandler(&AsScene1308LightWallSymbols::handleMessage);
}

uint32 AsScene1308LightWallSymbols::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2002:
		stFadeIn();
		break;
	case 0x2003:
		stFadeOut();
		break;
	case 0x3002:
		gotoNextState();
		break;
	}
	return messageResult;
}

void AsScene1308LightWallSymbols::stFadeIn() {
	startAnimation(0x80180A10, 0, -1);
	setVisible(true);
	_newStickFrameIndex = STICK_LAST_FRAME;
}

void AsScene1308LightWallSymbols::stFadeOut() {
	startAnimation(0x80180A10, -1, -1);
	_playBackwards = true;
	NextState(&AsScene1308LightWallSymbols::stFadeOutDone);
}

void AsScene1308LightWallSymbols::stFadeOutDone() {
	sendMessage(_parentScene, 0x2003, 0);
	stopAnimation();
	setVisible(false);
}

SsScene1308Number::SsScene1308Number(NeverhoodEngine *vm, uint32 fileHash, int index)
	: StaticSprite(vm, fileHash, 100) {
	
	setVisible(false);
	_x = _spriteResource.getPosition().x + index * 20;
	updatePosition();
}

AsScene1308Mouse::AsScene1308Mouse(NeverhoodEngine *vm)
	: AnimatedSprite(vm, 1100) {

	_x = 286;
	_y = 429;
	createSurface1(0xA282C472, 100);
	startAnimation(0xA282C472, 0, -1);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1308Mouse::handleMessage);
}

uint32 AsScene1308Mouse::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x66382026)
			playSound(0, 0x0CD84468);
		else if (param.asInteger() == 0x6E28061C)
			playSound(0, 0x78C8402C);
		else if (param.asInteger() == 0x462F0410)
			playSound(0, 0x60984E28);
		break;
	}
	return messageResult;
}

Scene1308::Scene1308(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule), _isProjecting(false), _asProjector(NULL) {
	
	_vm->gameModule()->initKeySlotsPuzzle();

	SetMessageHandler(&Scene1308::handleMessage);
	
	setBackground(0x41024202);
	setPalette(0x41024202);
	insertScreenMouse(0x24206418);

	_asTape = insertSprite<AsScene1201Tape>(this, 17, 1100, 502, 445, 0x9148A011);
	addCollisionSprite(_asTape);

	if (getGlobalVar(V_MOUSE_SUCKED_IN)) {
		insertSprite<AsScene1308Mouse>();
		insertSprite<AnimatedSprite>(0x461A1490, 200, 235, 429);
	}
	
	_sprite1 = insertStaticSprite(0x0A042060, 1100);
	_asJaggyDoor = insertSprite<AsScene1308JaggyDoor>(this);
	_asLightWallSymbols = insertSprite<AsScene1308LightWallSymbols>(this);
	_ssNumber1 = insertSprite<SsScene1308Number>(kScene1308NumberFileHashes[getSubVar(VA_GOOD_KEY_SLOT_NUMBERS, 1)], 0);
	_ssNumber2 = insertSprite<SsScene1308Number>(kScene1308NumberFileHashes[getSubVar(VA_GOOD_KEY_SLOT_NUMBERS, 0)], 1);
	_ssNumber3 = insertSprite<SsScene1308Number>(kScene1308NumberFileHashes[getSubVar(VA_GOOD_KEY_SLOT_NUMBERS, 2)], 2);
	_sprite2 = insertStaticSprite(0x40043120, 995);
	_sprite3 = insertStaticSprite(0x43003100, 995);
	_sprite4 = NULL;
	
	if (which < 0) {
		// Restoring game
		insertKlaymen<KmScene1308>(380, 440);
		setMessageList(0x004B57C0);
		if (getGlobalVar(V_KEYDOOR_UNLOCKED)) {
			_sprite4 = insertStaticSprite(0x0101A624, 1100);
			setRectList(0x004B5990);
		} else {
			_sprite5 = insertStaticSprite(0x080811A0, 100);
			setRectList(0x004B5980);
		}
	} else if (which == 1) {
		// Klaymen entering from the right
		insertKlaymen<KmScene1308>(640, 440);
		setMessageList(0x004B57C8);
		if (getGlobalVar(V_KEYDOOR_UNLOCKED)) {
			_sprite4 = insertStaticSprite(0x0101A624, 1100);
			setRectList(0x004B5990);
		} else {
			_sprite5 = insertStaticSprite(0x080811A0, 100);
			setRectList(0x004B5980);
		}
	} else if (which == 2) {
		// Klaymen returning from keyslots panel
		insertKlaymen<KmScene1308>(475, 440);
		setMessageList(0x004B58B0);
		if (getGlobalVar(V_KEYDOOR_UNLOCKED)) {
			_sprite5 = insertSprite<AsScene1308KeyboardDoor>(this);
			_sprite4 = insertStaticSprite(0x0101A624, 1100);
			_sprite4->setVisible(false);
		} else {
			_sprite5 = insertStaticSprite(0x080811A0, 100);
			setRectList(0x004B5980);
		}
	} else {
		// Klaymen entering from the left
		insertKlaymen<KmScene1308>(41, 440);
		setMessageList(0x004B57D0);
		sendMessage(_asJaggyDoor, 0x4808, 0);
		_sprite1->setVisible(false);
		if (getGlobalVar(V_KEYDOOR_UNLOCKED)) {
			_sprite4 = insertStaticSprite(0x0101A624, 1100);
			_klaymen->setVisible(false);
		} else {
			_sprite5 = insertStaticSprite(0x080811A0, 100);
			_klaymen->setVisible(false);
		}
	}

	if (_sprite4)
		_klaymen->setClipRect(_sprite1->getDrawRect().x, 0, _sprite4->getDrawRect().x2(), 480);
	else
		_klaymen->setClipRect(_sprite1->getDrawRect().x, 0, 640, 480);

	if (getGlobalVar(V_PROJECTOR_LOCATION) == 4) {
		_asProjector = insertSprite<AsCommonProjector>(this, _klaymen, (Sprite*)NULL);
		addCollisionSprite(_asProjector);
		_asProjector->setClipRect(0, 0, 640, _sprite2->getDrawRect().y2());
		_asProjector->setRepl(64, 0);
	}

}

uint32 Scene1308::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x100D:
		if (param.asInteger() == 0x88C11390) {
			setRectList(0x004B59A0);
			_isProjecting = true;
		} else if (param.asInteger() == 0x08821382) {
			sendEntityMessage(_klaymen, 0x1014, _asProjector);
			if (getGlobalVar(V_KEYDOOR_UNLOCKED))
				setRectList(0x004B5990);
			else
				setRectList(0x004B5980);
			_isProjecting = false;
		} else if (param.asInteger() == 0x4AC68808) {
			clearRectList();
			sendMessage(_asJaggyDoor, 0x4809, 0);
			_sprite1->setVisible(false);
			_klaymen->setVisible(false);
		}
		break;
	case 0x1022:
		if (sender == _asProjector) {
			if (param.asInteger() >= 1000)
				setSurfacePriority(_sprite3->getSurface(), 1100);
			else
				setSurfacePriority(_sprite3->getSurface(), 995);
		}
		break;
	case 0x2000:
		if (getGlobalVar(V_KEYDOOR_UNLOCKED))
			setRectList(0x004B5990);
		else
			setRectList(0x004B5980);
		setMessageList(0x004B57E8, false);
		_sprite1->setVisible(true);
		_klaymen->setVisible(true);
		break;
	case 0x2001:
		leaveScene(0);
		break;
	case 0x2003:
		_ssNumber1->setVisible(false);
		_ssNumber2->setVisible(false);
		_ssNumber3->setVisible(false);
		break;
	case 0x2004:
		_sprite4->setVisible(true);
		setRectList(0x004B5990);
		break;
	case 0x4807:
		sendMessage(_asLightWallSymbols, 0x2003, 0);
		break;
	case 0x480F:
		sendMessage(_asLightWallSymbols, 0x2002, 0);
		_ssNumber1->setVisible(true);
		_ssNumber2->setVisible(true);
		_ssNumber3->setVisible(true);
		break;
	case 0x4826:
		if (sender == _asProjector) {
			if (_isProjecting)
				setMessageList2(0x004B5868);
			else {
				if (param.asInteger() == 1) {
					sendEntityMessage(_klaymen, 0x1014, _asProjector);
					setMessageList2(0x004B5848);
				} else if (sendMessage(_asProjector, 0x480C, _klaymen->getX() <= _asProjector->getX() ? 0 : 1) != 0) {
					sendEntityMessage(_klaymen, 0x1014, _asProjector);
					setMessageList2(0x004B5830);
				} else
					setMessageList2(0x004B5800);
			}
		} else if (sender == _asTape) {
			if (_isProjecting)
				setMessageList2(0x004B5868);
			else if (_messageListStatus != 2) {
				sendEntityMessage(_klaymen, 0x1014, _asTape);
				setMessageList2(0x004B58E0);
			}
		}
		break;
	}
	return 0;
}

Scene1317::Scene1317(NeverhoodEngine *vm, Module *parentModule)
	: Scene(vm, parentModule) {
	
	SetMessageHandler(&Scene1317::handleMessage);
	_smackerPlayer = addSmackerPlayer(new SmackerPlayer(_vm, this, 0x08982841, true, false));
	_vm->_screen->setSmackerDecoder(_smackerPlayer->getSmackerDecoder());
	insertScreenMouse(0x08284011);
	showMouse(false);
	_smackerFileHash = 0;
	_keepLastSmackerFrame = false;
}

void Scene1317::update() {
	if (_smackerFileHash) {
		_smackerPlayer->open(_smackerFileHash, _keepLastSmackerFrame);
		_smackerFileHash = 0;
	}
	Scene::update();
}

void Scene1317::upChooseKing() {
	if (!_klaymenBlinks && _klaymenBlinkCountdown != 0 && (--_klaymenBlinkCountdown == 0))
		_klaymenBlinks = true;
		
	if (!_klaymenBlinks && _smackerPlayer->getFrameNumber() + 1 >= 2) {
		_smackerPlayer->rewind();
	} else if (_klaymenBlinks && _smackerPlayer->getFrameNumber() + 1 >= 6) {
		_smackerPlayer->rewind();
		_klaymenBlinks = false;
		_klaymenBlinkCountdown = _vm->_rnd->getRandomNumber(30 - 1) + 15;
	}

	if (!_klaymenBlinks && _decisionCountdown != 0 && (--_decisionCountdown == 0))
		stNoDecisionYet();
			
	if (_smackerFileHash) {
		_smackerPlayer->open(_smackerFileHash, _keepLastSmackerFrame);
		_smackerFileHash = 0;
	}

	Scene::update();
	
}

uint32 Scene1317::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		stChooseKing();
		break;
	}
	return messageResult;
}
	
uint32 Scene1317::hmChooseKing(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		if (param.asPoint().x >= 21 && param.asPoint().y >= 24 &&
			param.asPoint().x <= 261 && param.asPoint().y <= 280) {
			stHoborgAsKing();
		} else if (param.asPoint().x >= 313 && param.asPoint().y >= 184 &&
			param.asPoint().x <= 399 && param.asPoint().y <= 379) {
			stKlaymenAsKing();
		} else if (param.asPoint().x >= 347 && param.asPoint().y >= 380 &&
			param.asPoint().x <= 418 && param.asPoint().y <= 474) {
			stKlaymenAsKing();
		}
		break;
	}
	return messageResult;
}

uint32 Scene1317::hmHoborgAsKing(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		stEndMovie();
		break;
	}
	return messageResult;
}

uint32 Scene1317::hmEndMovie(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		leaveScene(0);
		break;
	}
	return messageResult;
}

void Scene1317::stChooseKing() {
	showMouse(true);
	_smackerFileHash = 0x10982841;
	_keepLastSmackerFrame = true;
	_decisionCountdown = 450;
	_klaymenBlinks = false;
	_klaymenBlinkCountdown = _vm->_rnd->getRandomNumber(30 - 1) + 15;
	SetMessageHandler(&Scene1317::hmChooseKing);
	SetUpdateHandler(&Scene1317::upChooseKing);
}

void Scene1317::stNoDecisionYet() {
	showMouse(false);
	_smackerFileHash = 0x20982841;
	_keepLastSmackerFrame = false;
	SetMessageHandler(&Scene1317::handleMessage);
	SetUpdateHandler(&Scene1317::update);
}

void Scene1317::stHoborgAsKing() {
	showMouse(false);
	_smackerFileHash = 0x40982841;
	_keepLastSmackerFrame = false;
	SetMessageHandler(&Scene1317::hmHoborgAsKing);
	SetUpdateHandler(&Scene1317::update);
}

void Scene1317::stKlaymenAsKing() {
	showMouse(false);
	_smackerFileHash = 0x80982841;
	_keepLastSmackerFrame = false;
	SetMessageHandler(&Scene1317::hmEndMovie);
	SetUpdateHandler(&Scene1317::update);
}

void Scene1317::stEndMovie() {
	showMouse(false);
	_smackerFileHash = 0x40800711;
	_keepLastSmackerFrame = false;
	SetMessageHandler(&Scene1317::hmEndMovie);
	SetUpdateHandler(&Scene1317::update);
}

} // End of namespace Neverhood
