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

#include "neverhood/diskplayerscene.h"
#include "neverhood/gamemodule.h"
#include "neverhood/menumodule.h"
#include "neverhood/smackerplayer.h"
#include "neverhood/modules/module1000_sprites.h"
#include "neverhood/modules/module1200_sprites.h"
#include "neverhood/modules/module1300.h"
#include "neverhood/modules/module1300_sprites.h"
#include "neverhood/modules/module1400_sprites.h"
#include "neverhood/modules/module2200_sprites.h"

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
	debug(1, "Module1300::createScene(%d, %d)", sceneNum, which);
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
	default:
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
		default:
			break;
		}
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

	sendEntityMessage(_klaymen, NM_CAR_MOVE_TO_PREV_POINT, _asVenusFlyTrap);

}

uint32 Scene1302::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = 0;
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x4A845A00) {
			sendEntityMessage(_klaymen, 0x1014, _asRing1);
		} else if (param.asInteger() == 0x43807801) {
			if (!getGlobalVar(V_FLYTRAP_RING_BRIDGE)) {
				sendEntityMessage(_klaymen, 0x1014, _asRing2);
				if (_asVenusFlyTrap->getX() - 10 < 218 + 32 && _asVenusFlyTrap->getX() + 10 > 218 + 32)
					setMessageList(0x004B0940);
				else
					setMessageList(0x004B0938);
			} else
				setMessageList(0x004B0950);
			messageResult = 1;
		} else if (param.asInteger() == 0x46C26A01) {
			sendEntityMessage(_klaymen, 0x1014, _asRing3);
		} else if (param.asInteger() == 0x468C7B11) {
			if (!getGlobalVar(V_FLYTRAP_RING_FENCE)) {
				sendEntityMessage(_klaymen, 0x1014, _asRing4);
				if (_asVenusFlyTrap->getX() - 10 < 218 + 32 + 32 + 32 && _asVenusFlyTrap->getX() + 10 > 218 + 32 + 32 + 32)
					setMessageList(0x004B0940);
				else
					setMessageList(0x004B0938);
			} else
				setMessageList(0x004B0950);
			messageResult = 1;
		} else if (param.asInteger() == 0x42845B19) {
			sendEntityMessage(_klaymen, 0x1014, _asRing5);
		} else if (param.asInteger() == 0x430A6060) {
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
	case NM_ANIMATION_UPDATE:
		if (_klaymen->getY() > 360) {
			sendEntityMessage(_klaymen, 0x1014, _asVenusFlyTrap);
			setMessageList2(0x004B08F0);
		} else
			setMessageList2(0x004B0920);
		break;
	case NM_POSITION_CHANGE:
		if (_klaymen->getX() > 545)
			leaveScene(1);
		break;
	case 0x2032:
		_sprite2->setVisible(true);
		break;
	case NM_KLAYMEN_USE_OBJECT:
		sendMessage(_parentModule, 0x1024, 2);
		if (sender == _asRing1)
			playSound(0, 0x665198C0);
		else if (sender == _asRing2) {
			sendMessage(_asBridge, NM_KLAYMEN_OPEN_DOOR, 0);
			setGlobalVar(V_FLYTRAP_RING_BRIDGE, 1);
		} else if (sender == _asRing3) {
			playSound(0, 0xE2D389C0);
		} else if (sender == _asRing4) {
			sendMessage(_ssFence, NM_KLAYMEN_OPEN_DOOR, 0);
			setGlobalVar(V_FLYTRAP_RING_FENCE, 1);
		} else if (sender == _asRing5)
			playSound(0, 0x40428A09);
		break;
	case NM_KLAYMEN_RAISE_LEVER:
		if (sender == _asRing2) {
			sendMessage(_asBridge, NM_KLAYMEN_CLOSE_DOOR, 0);
			setGlobalVar(V_FLYTRAP_RING_BRIDGE, 0);
			_sprite2->setVisible(false);
		} else if (sender == _asRing4) {
			sendMessage(_ssFence, NM_KLAYMEN_CLOSE_DOOR, 0);
			setGlobalVar(V_FLYTRAP_RING_FENCE, 0);
		} else if (sender == _asVenusFlyTrap) {
			if (getGlobalVar(V_FLYTRAP_RING_BRIDGE))
				sendMessage(_asRing2, NM_KLAYMEN_RAISE_LEVER, 0);
			else
				sendMessage(_asRing4, NM_KLAYMEN_RAISE_LEVER, 0);
		}
		break;
	case NM_KLAYMEN_LOWER_LEVER:
		if (sender == _asRing2) {
			playSound(0, 0x60755842);
			sendMessage(_asBridge, NM_KLAYMEN_OPEN_DOOR, 0);
			setGlobalVar(V_FLYTRAP_RING_BRIDGE, 1);
		} else if (sender == _asRing4) {
			playSound(0, 0x60755842);
			sendMessage(_ssFence, NM_KLAYMEN_OPEN_DOOR, 0);
			setGlobalVar(V_FLYTRAP_RING_FENCE, 1);
		}
		break;
	case NM_MOVE_TO_BACK:
		sendMessage(_asVenusFlyTrap, NM_MOVE_TO_FRONT, 0);
		break;
	case NM_MOVE_TO_FRONT:
		sendMessage(_asVenusFlyTrap, NM_MOVE_TO_BACK, 0);
		break;
	case 0x8000:
		setSpriteSurfacePriority(_class595, 995);
		break;
	case 0x8001:
		setSpriteSurfacePriority(_class595, 1015);
		break;
	default:
		break;
	}
	return messageResult;
}

Scene1303::Scene1303(NeverhoodEngine *vm, Module *parentModule)
	: Scene(vm, parentModule), _asBalloon(NULL) {

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
	case NM_ANIMATION_UPDATE:
		setGlobalVar(V_BALLOON_POPPED, 1);
		sendMessage(_asBalloon, 0x2000, 0);
		break;
	case 0x4826:
		if (sender == _asBalloon && getGlobalVar(V_HAS_NEEDLE))
			setMessageList(0x004AF9B8);
		break;
	default:
		break;
	}
	return 0;
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
	case NM_ANIMATION_START:
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
	default:
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

Scene1306::Scene1306(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule), _asKey(nullptr) {

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
		// Restoring game
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
		sendMessage(_asElevator, NM_KLAYMEN_OPEN_DOOR, 0);
	}
}

Scene1306::~Scene1306() {
	setGlobalVar(V_KLAYMEN_IS_DELTA_X, _klaymen->isDoDeltaX() ? 1 : 0);
}

uint32 Scene1306::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
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
	case NM_ANIMATION_UPDATE:
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
			sendMessage(_asElevator, NM_KLAYMEN_OPEN_DOOR, 0);
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
	case NM_MOVE_TO_BACK:
		setSurfacePriority(_asElevator->getSurface(), 1100);
		setSurfacePriority(_asElevatorDoor->getSurface(), 1090);
		setSurfacePriority(_sprite1->getSurface(), 1080);
		break;
	case NM_MOVE_TO_FRONT:
		setSurfacePriority(_asElevator->getSurface(), 100);
		setSurfacePriority(_asElevatorDoor->getSurface(), 90);
		setSurfacePriority(_sprite1->getSurface(), 80);
		sendMessage(this, 0x2000, 0);
		addCollisionSprite(_asTape);
		break;
	default:
		break;
	}
	return 0;
}

uint32 Scene1306::handleMessage416EB0(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_KLAYMEN_OPEN_DOOR:
		setMessageList(0x004AFBD0);
		SetMessageHandler(&Scene1306::handleMessage);
		break;
	case NM_KLAYMEN_CLOSE_DOOR:
		leaveScene(1);
		break;
	case NM_MOVE_TO_BACK:
		setSurfacePriority(_asElevator->getSurface(), 1100);
		setSurfacePriority(_asElevatorDoor->getSurface(), 1090);
		setSurfacePriority(_sprite1->getSurface(), 1080);
		break;
	case NM_MOVE_TO_FRONT:
		setSurfacePriority(_asElevator->getSurface(), 100);
		setSurfacePriority(_asElevatorDoor->getSurface(), 90);
		setSurfacePriority(_sprite1->getSurface(), 80);
		sendMessage(this, 0x2000, 0);
		addCollisionSprite(_asTape);
		break;
	default:
		break;
	}
	return 0;
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
	case NM_MOUSE_CLICK:
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
	case NM_POSITION_CHANGE:
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
	default:
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
	_sprite5 = nullptr;

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
		sendMessage(_asJaggyDoor, NM_KLAYMEN_OPEN_DOOR, 0);
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
	case NM_ANIMATION_START:
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
			sendMessage(_asJaggyDoor, NM_KLAYMEN_CLOSE_DOOR, 0);
			_sprite1->setVisible(false);
			_klaymen->setVisible(false);
		}
		break;
	case NM_PRIORITY_CHANGE:
		if (sender == _asProjector) {
			if (param.asInteger() >= 1000)
				setSurfacePriority(_sprite3->getSurface(), 1100);
			else
				setSurfacePriority(_sprite3->getSurface(), 995);
		}
		break;
	case NM_ANIMATION_UPDATE:
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
	case NM_KLAYMEN_RAISE_LEVER:
		sendMessage(_asLightWallSymbols, 0x2003, 0);
		break;
	case NM_KLAYMEN_LOWER_LEVER:
		sendMessage(_asLightWallSymbols, NM_POSITION_CHANGE, 0);
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
	default:
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
		_vm->_screen->setSmackerDecoder(_smackerPlayer->getSmackerDecoder());
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
		_vm->_screen->setSmackerDecoder(_smackerPlayer->getSmackerDecoder());
		_smackerFileHash = 0;
	}

	Scene::update();

}

uint32 Scene1317::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_STOP:
		stChooseKing();
		break;
	default:
		break;
	}
	return messageResult;
}

uint32 Scene1317::hmChooseKing(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_MOUSE_CLICK:
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
	default:
		break;
	}
	return messageResult;
}

uint32 Scene1317::hmHoborgAsKing(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_STOP:
		stEndMovie();
		break;
	default:
		break;
	}
	return messageResult;
}

uint32 Scene1317::hmEndMovie(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_STOP:
		leaveScene(0);
		break;
	default:
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
