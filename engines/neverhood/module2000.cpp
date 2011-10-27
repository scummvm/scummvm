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

#include "neverhood/module2000.h"
#include "neverhood/gamemodule.h"
#include "neverhood/navigationscene.h"

namespace Neverhood {

Module2000::Module2000(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {
	
	if (which < 0) {
		createScene(_vm->gameState().sceneNum, -1);
	} else if (which == 0) {
		createScene(0, 3);
	} else if (which == 1) {
		createScene(0, 1);
	}

}

Module2000::~Module2000() {
	// TODO Sound1ChList_sub_407A50(0x81293110);
}

void Module2000::createScene(int sceneNum, int which) {
	debug("Module2000::createScene(%d, %d)", sceneNum, which);
	_vm->gameState().sceneNum = sceneNum;
	switch (_vm->gameState().sceneNum) {
	case 0:
		_childObject = new Scene2001(_vm, this, which);
		break;
	case 1:
		createNavigationScene(getGlobalVar(0x98109F12) ? 0x004B7B48 : 0x004B7B00, which);
		break;
	case 2:
		setGlobalVar(0x98109F12, 1);
		setSubVar(0x2C145A98, 1, 1);
		createSmackerScene(0x204B2031, true, true, false);
		break;
	}
	SetUpdateHandler(&Module2000::updateScene);
	_childObject->handleUpdate();
}

void Module2000::updateScene() {
	if (!updateChild()) {
		switch (_vm->gameState().sceneNum) {
		case 0:
			if (_moduleResult == 1) {
				leaveModule(0);
			} else {
				createScene(1, 0);
			}
			break;
		case 1:
			if (_moduleResult == 0) {
				if (getGlobalVar(0x98109F12)) {
					createScene(1, 0);
				} else {
					createScene(2, -1);
				}
			} else if (_moduleResult == 1) {
				createScene(1, 1);
			} else if (_moduleResult == 2) {
				createScene(0, 0);
			}
			break;
		case 2:
			createScene(1, 0);
			break;
		}
	}
}

// Scene2001

Scene2001::Scene2001(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true) {

	_surfaceFlag = true;
	SetMessageHandler(&Scene2001::handleMessage);

	setBackground(0xA6417244);
	setPalette(0xA6417244);
	insertMouse433(0x17240A6C);

	_class401 = insertStaticSprite(0x0D641724, 1100);

	if (which < 0) {
		insertKlayman<KmScene2001>(300, 345);
		setMessageList(0x004B3538);
		sendMessage(this, 0x2000, 0);
	} else if (which == 1) {
		insertKlayman<KmScene2001>(116, 345);
		sendMessage(_klayman, 0x2000, 1);
		setMessageList(0x004B3540, false);
		sendMessage(this, 0x2000, 1);
	} else if (which == 2) {
		insertKlayman<KmScene2001>(116, 345);
		sendMessage(_klayman, 0x2000, 1);
		setMessageList(0x004B35F0, false);
		sendMessage(this, 0x2000, 1);
	} else if (which == 3) {
		insertKlayman<KmScene2001>(116, 345);
		sendMessage(_klayman, 0x2000, 1);
		setMessageList(0x004B3550, false);
		sendMessage(this, 0x2000, 1);
	} else {
		insertKlayman<KmScene2001>(390, 345);
		setMessageList(0x004B3530);
		sendMessage(this, 0x2000, 0);
		_klayman->setDoDeltaX(1);
	}
	
	_klayman->setClipRect(_class401->getDrawRect().x, 0, 640, 480);
	
}

uint32 Scene2001::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2000:
		if (param.asInteger()) {
			setRectList(0x004B3680);
			_klayman->setKlaymanIdleTable3();
		} else {
			setRectList(0x004B3670);
			_klayman->setKlaymanIdleTable1();
		}
	}	
	return 0;
}

} // End of namespace Neverhood
