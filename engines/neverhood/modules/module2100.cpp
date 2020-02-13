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

#include "neverhood/modules/module1200_sprites.h"
#include "neverhood/modules/module2100.h"
#include "neverhood/modules/module2100_sprites.h"

namespace Neverhood {

Module2100::Module2100(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {

	_vm->_soundMan->addMusic(0x10A10C14, 0x11482B95);

	if (which < 0)
		createScene(_vm->gameState().sceneNum, -1);
	else if (which == 1)
		createScene(0, 0);
	else if (which == 2)
		createScene(0, 3);
	else
		createScene(0, 1);

}

Module2100::~Module2100() {
	_vm->_soundMan->deleteMusicGroup(0x10A10C14);
}

void Module2100::createScene(int sceneNum, int which) {
	debug(1, "Module2100::createScene(%d, %d)", sceneNum, which);
	_sceneNum = sceneNum;
	switch (_sceneNum) {
	case 0:
		_vm->gameState().sceneNum = 0;
		_vm->_soundMan->startMusic(0x11482B95, 0, 1);
		_childObject = new Scene2101(_vm, this, which);
		break;
	default:
		break;
	}
	SetUpdateHandler(&Module2100::updateScene);
	_childObject->handleUpdate();
}

void Module2100::updateScene() {
	if (!updateChild()) {
		switch (_sceneNum) {
		case 0:
			if (_moduleResult == 1) {
				setGlobalVar(V_DOOR_PASSED, 1);
				leaveModule(0);
			} else {
				leaveModule(1);
			}
			break;
		default:
			break;
		}
	}
}

Scene2101::Scene2101(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule) {

	Sprite *tempSprite;

	SetMessageHandler(&Scene2101::handleMessage);
	SetUpdateHandler(&Scene2101::update);

	setBackground(0x44242305);
	setPalette(0x44242305);
	insertScreenMouse(0x4230144A);

	insertStaticSprite(0x00502330, 1100);
	tempSprite = insertStaticSprite(0x78492010, 1100);
	_ssFloorButton = insertSprite<SsCommonFloorButton>(this, 0x72427010, 0x32423010, 200, 0);
	_asTape1 = insertSprite<AsScene1201Tape>(this, 18, 1100, 412, 443, 0x9148A011);
	addCollisionSprite(_asTape1);
	_asTape2 = insertSprite<AsScene1201Tape>(this, 11, 1100, 441, 443, 0x9048A093);
	addCollisionSprite(_asTape2);

	if (which < 0) {
		insertKlaymen<KmScene2101>(380, 438);
		setMessageList(0x004B8E48);
		sendMessage(this, 0x2000, 0);
		_asDoor = insertSprite<AsScene2101Door>(false);
		_doorStatus = 1;
		_countdown1 = 0;
	} else if (which == 1) {
		// Klaymen entering from the right
		insertKlaymen<KmScene2101>(640, 438);
		setMessageList(0x004B8E50);
		sendMessage(this, 0x2000, 0);
		_asDoor = insertSprite<AsScene2101Door>(true);
		_doorStatus = 2;
		_countdown1 = 48;
	} else if (which == 2) {
		// Klaymen teleporting out
		insertKlaymen<KmScene2101>(115, 438);
		sendMessage(_klaymen, 0x2000, 1);
		setMessageList(0x004B8F58);
		sendMessage(this, 0x2000, 1);
		_asDoor = insertSprite<AsScene2101Door>(false);
		_doorStatus = 1;
		_countdown1 = 0;
	} else if (which == 3) {
		// Klaymen returning from the teleporter console
		insertKlaymen<KmScene2101>(115, 438);
		sendMessage(_klaymen, 0x2000, 1);
		setMessageList(0x004B8EB0);
		sendMessage(this, 0x2000, 1);
		_asDoor = insertSprite<AsScene2101Door>(false);
		_doorStatus = 1;
		_countdown1 = 0;
	} else {
		// Klaymen teleporting in
		insertKlaymen<KmScene2101>(115, 438);
		sendMessage(_klaymen, 0x2000, 1);
		setMessageList(0x004B8EA0);
		sendMessage(this, 0x2000, 1);
		_asDoor = insertSprite<AsScene2101Door>(false);
		_doorStatus = 1;
		_countdown1 = 0;
	}

	_asHitByDoorEffect = insertSprite<AsScene2101HitByDoorEffect>(_klaymen);
	_klaymen->setClipRect(0, 0, tempSprite->getDrawRect().x2(), 480);

}

void Scene2101::update() {
	if (_countdown1 != 0) {
		if (_doorStatus == 2) {
			if (--_countdown1 == 0) {
				sendMessage(_asDoor, NM_KLAYMEN_CLOSE_DOOR, 0);
				_doorStatus = 1;
			}
		} else {
			if (_klaymen->getX() > 575)
				_canAcceptInput  = false;
			if (--_countdown1 == 0) {
				if (_klaymen->getX() < 480) {
					sendMessage(_asDoor, NM_KLAYMEN_CLOSE_DOOR, 0);
					_doorStatus = 1;
				} else if (_klaymen->getX() >= 480 && _klaymen->getX() <= 575) {
					_klaymen->setDoDeltaX(0);
					setMessageList2(0x004B8F48);
					sendMessage(_asDoor, NM_KLAYMEN_CLOSE_DOOR, 0);
					sendMessage(_asHitByDoorEffect, 0x2001, 0);
					_doorStatus = 1;
				}
			}
		}
	} else if (_doorStatus == 1 && _messageValue >= 0 && _klaymen->getX() > 470 && !isMessageList2(0x004B8F48))
		setMessageList2(0x004B8F50);
	Scene::update();
}

uint32 Scene2101::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_START:
		if (param.asInteger() == 0x02144CB1)
			sendEntityMessage(_klaymen, 0x1014, _ssFloorButton);
		else if (param.asInteger() == 0x21E64A00) {
			if (_doorStatus == 0)
				setMessageList(0x004B8E80);
			else
				setMessageList(0x004B8EC8);
		} else if (param.asInteger() == 0x41442820)
			cancelMessageList();
		break;
	case NM_ANIMATION_UPDATE:
		if (param.asInteger() != 0) {
			setRectList(0x004B9008);
			_klaymen->setKlaymenIdleTable3();
		} else {
			setRectList(0x004B8FF8);
			_klaymen->setKlaymenIdleTable1();
		}
		break;
	case 0x480B:
		if (sender == _ssFloorButton && _doorStatus == 1) {
			sendMessage(_asDoor, NM_KLAYMEN_OPEN_DOOR, 0);
			_doorStatus = 0;
			_countdown1 = 90;
		}
		break;
	case 0x4826:
		if (sender == _asTape1 || sender == _asTape2) {
			if (_klaymen->getX() >= 228 && _klaymen->getX() <= 500) {
				sendEntityMessage(_klaymen, 0x1014, sender);
				setMessageList(0x004B8F78);
			} else if (_klaymen->getX() < 228)
				setMessageList2(0x004B8F00);
		}
		break;
	default:
		break;
	}
	return 0;
}

} // End of namespace Neverhood
