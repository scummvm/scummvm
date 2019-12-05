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

#include "neverhood/modules/module1500.h"

namespace Neverhood {

Module1500::Module1500(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {

	if (which < 0)
		createScene(_vm->gameState().sceneNum, -1);
	else
		createScene(3, -1);

}

void Module1500::createScene(int sceneNum, int which) {
	debug(1, "Module1500::createScene(%d, %d)", sceneNum, which);
	_sceneNum = sceneNum;
	switch (_sceneNum) {
	case 0:
		_vm->gameState().sceneNum = 0;
		_childObject = new Scene1501(_vm, this, 0x8420221D, 0xA61024C4, 150, 48);
		break;
	case 1:
		_vm->gameState().sceneNum = 1;
		_childObject = new Scene1501(_vm, this, 0x30050A0A, 0x58B45E58, 110, 48);
		break;
	case 2:
		_vm->gameState().sceneNum = 2;
		sendMessage(_parentModule, 0x0800, 0);
		createSmackerScene(0x001A0005, true, true, true);
		break;
	case 3:
		_vm->gameState().sceneNum = 3;
		_childObject = new Scene1501(_vm, this, 0x0CA04202, 0, 110, 48);
		break;
	default:
		break;
	}
	SetUpdateHandler(&Module1500::updateScene);
	_childObject->handleUpdate();
}

void Module1500::updateScene() {
	if (!updateChild()) {
		switch (_sceneNum) {
		case 0:
			createScene(1, -1);
			break;
		case 1:
			createScene(2, -1);
			break;
		case 3:
			createScene(0, -1);
			break;
		default:
			leaveModule(0);
			break;
		}
	}
}

// Scene1501

Scene1501::Scene1501(NeverhoodEngine *vm, Module *parentModule, uint32 backgroundFileHash, uint32 soundFileHash, int countdown2, int countdown3)
	: Scene(vm, parentModule), _countdown3(countdown3), _countdown2(countdown2), _countdown1(0), _skip(false) {

	SetUpdateHandler(&Scene1501::update);
	SetMessageHandler(&Scene1501::handleMessage);

	setBackground(backgroundFileHash);
	setPalette();
	addEntity(_palette);
	_palette->addBasePalette(backgroundFileHash, 0, 256, 0);
	_palette->startFadeToPalette(12);

	if (soundFileHash != 0)
		playSound(0, soundFileHash);

}

void Scene1501::update() {
	Scene::update();
	if (_countdown1 != 0) {
		_countdown1--;
		if (_countdown1 == 0 || _skip) {
			_vm->_screen->clear();
			leaveScene(0);
		}
	} else if ((_countdown2 != 0 && (--_countdown2 == 0)) || (_countdown2 == 0 && !isSoundPlaying(0)) || _skip) {
		_countdown1 = 12;
		_palette->startFadeToBlack(11);
	}

	if (_countdown3 != 0)
		_countdown3--;

	if (_countdown3 == 0 && _skip && _countdown1 == 0) {
		_countdown1 = 12;
		_palette->startFadeToBlack(11);
	}

}

uint32 Scene1501::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_KEYPRESS_SPACE:
		_skip = true;
		break;
	default:
		break;
	}
	return messageResult;
}

} // End of namespace Neverhood
