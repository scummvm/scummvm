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

#include "neverhood/modules/module2000.h"
#include "neverhood/modules/module2000_sprites.h"

namespace Neverhood {

Module2000::Module2000(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {

	if (which < 0)
		createScene(_vm->gameState().sceneNum, -1);
	else if (which == 0)
		createScene(0, 1);
	else if (which == 1)
		createScene(0, 3);

}

Module2000::~Module2000() {
	_vm->_soundMan->deleteGroup(0x81293110);
}

void Module2000::createScene(int sceneNum, int which) {
	debug(1, "Module2000::createScene(%d, %d)", sceneNum, which);
	_sceneNum = sceneNum;
	switch (_sceneNum) {
	case 0:
		_vm->gameState().sceneNum = 0;
		_childObject = new Scene2001(_vm, this, which);
		break;
	case 1:
		_vm->gameState().sceneNum = 1;
		createNavigationScene(getGlobalVar(V_WORLDS_JOINED) ? 0x004B7B48 : 0x004B7B00, which);
		break;
	case 2:
		_vm->gameState().sceneNum = 2;
		setGlobalVar(V_WORLDS_JOINED, 1);
		setSubVar(V_TELEPORTER_DEST_AVAILABLE, 1, 1);
		createSmackerScene(0x204B2031, true, true, false);
		break;
	default:
		break;
	}
	SetUpdateHandler(&Module2000::updateScene);
	_childObject->handleUpdate();
}

void Module2000::updateScene() {
	if (!updateChild()) {
		switch (_sceneNum) {
		case 0:
			if (_moduleResult == 1)
				leaveModule(0);
			else
				createScene(1, 0);
			break;
		case 1:
			if (_moduleResult == 0) {
				if (getGlobalVar(V_WORLDS_JOINED))
					createScene(1, 0);
				else
					createScene(2, -1);
			} else if (_moduleResult == 1)
				createScene(1, 1);
			else if (_moduleResult == 2)
				createScene(0, 0);
			break;
		case 2:
			createScene(1, 0);
			break;
		default:
			break;
		}
	}
}

// Scene2001

Scene2001::Scene2001(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule) {

	Sprite *tempSprite;

	SetMessageHandler(&Scene2001::handleMessage);

	setBackground(0xA6417244);
	setPalette(0xA6417244);
	insertScreenMouse(0x17240A6C);

	tempSprite = insertStaticSprite(0x0D641724, 1100);

	if (which < 0) {
		// Restoring game
		insertKlaymen<KmScene2001>(300, 345);
		setMessageList(0x004B3538);
		sendMessage(this, 0x2000, 0);
	} else if (which == 1) {
		// Klaymen teleporting in
		insertKlaymen<KmScene2001>(116, 345);
		sendMessage(_klaymen, 0x2000, 1);
		setMessageList(0x004B3540, false);
		sendMessage(this, 0x2000, 1);
	} else if (which == 2) {
		// Klaymen teleporting out
		insertKlaymen<KmScene2001>(116, 345);
		sendMessage(_klaymen, 0x2000, 1);
		setMessageList(0x004B35F0, false);
		sendMessage(this, 0x2000, 1);
	} else if (which == 3) {
		// Klaymen returning from teleporter console
		insertKlaymen<KmScene2001>(116, 345);
		sendMessage(_klaymen, 0x2000, 1);
		setMessageList(0x004B3550, false);
		sendMessage(this, 0x2000, 1);
	} else {
		// Klaymen standing around
		insertKlaymen<KmScene2001>(390, 345);
		setMessageList(0x004B3530);
		sendMessage(this, 0x2000, 0);
		_klaymen->setDoDeltaX(1);
	}

	_klaymen->setClipRect(tempSprite->getDrawRect().x, 0, 640, 480);

}

uint32 Scene2001::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_UPDATE:
		if (param.asInteger()) {
			setRectList(0x004B3680);
			_klaymen->setKlaymenIdleTable3();
		} else {
			setRectList(0x004B3670);
			_klaymen->setKlaymenIdleTable1();
		}
		break;
	default:
		break;
	}
	return 0;
}

} // End of namespace Neverhood
