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

#include "neverhood/module1500.h"

namespace Neverhood {

Module1500::Module1500(NeverhoodEngine *vm, Module *parentModule, int which, bool flag)
	: Module(vm, parentModule), _flag(flag) {
	
	debug("Create Module1500(%d)", which);

	if (which < 0) {
		switch (_vm->gameState().sceneNum) {
		case 1:
			createScene1502();
			break;
		case 2:
			createScene1503();
			break;
		case 3:
			createScene1504();
			break;
		default:
			createScene1501();			
		}
	} else {
		createScene1504();
	}

}

void Module1500::update() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		switch (_vm->gameState().sceneNum) {
		case 0:
			createScene1502();
			break;
		case 1:
			if (_flag) {
				createScene1503();
			} else {
				sendMessage(_parentModule, 0x1009, 0);
			}
			break;
		case 3:
			createScene1501();
			break;
		default:
			sendMessage(_parentModule, 0x1009, 0);
			break;
		}
	}
}

void Module1500::createScene1501() {
	debug("createScene1501");
	_vm->gameState().sceneNum = 0;
	_childObject = new Scene1501(_vm, this, 0x8420221D, 0xA61024C4, 150, 48);
	SetUpdateHandler(&Module1500::update);
}
			
void Module1500::createScene1502() {
	debug("createScene1502");
	_vm->gameState().sceneNum = 1;
	_childObject = new Scene1501(_vm, this, 0x30050A0A, 0x58B45E58, 110, 48);
	SetUpdateHandler(&Module1500::update);
}

void Module1500::createScene1503() {
	sendMessage(_parentModule, 0x0800, 0);
	_vm->gameState().sceneNum = 2;
	createSmackerScene(0x001A0005, true, true, true);
	SetUpdateHandler(&Module1500::update);
}

void Module1500::createScene1504() {
	_vm->gameState().sceneNum = 3;
	_childObject = new Scene1501(_vm, this, 0x0CA04202, 0, 110, 48);
	SetUpdateHandler(&Module1500::update);
}

// Scene1501

Scene1501::Scene1501(NeverhoodEngine *vm, Module *parentModule, uint32 backgroundFileHash, uint32 soundFileHash, int countdown2, int countdown3)
	: Scene(vm, parentModule, true), _soundResource(vm), 
	_countdown3(countdown3), _countdown2(countdown2), _countdown1(0), _flag(false) {

	debug("Create Scene1501(%08X, %08X, %d, %d)", backgroundFileHash, soundFileHash, countdown2, countdown3);
	
	Palette2 *palette2;

	SetUpdateHandler(&Scene1501::update);
	SetMessageHandler(&Scene1501::handleMessage);
	
	_surfaceFlag = true;

	setBackground(backgroundFileHash);

	palette2 = new Palette2(_vm);
	palette2->usePalette();
	_palette = palette2; 
	addEntity(_palette);
	palette2->addPalette(backgroundFileHash, 0, 256, 0);
	palette2->startFadeToPalette(12);

	/*	
	if (soundFileHash != 0) {
		_soundResource.set(soundFileHash);
		_soundResource.load();
		_soundResource.play();
	}
	*/

}

void Scene1501::update() {

	Scene::update();

	// TODO: Since these countdowns are used a lot, maybe these can be wrapped in a class/struct
	// so the code gets a little cleaner.

	if (_countdown1 != 0) {
		_countdown1--;
		if (_countdown1 == 0) {
			_vm->_screen->clear();
			sendMessage(_parentModule, 0x1009, 0);
		}
	} else if ((_countdown2 != 0 && (--_countdown2 == 0)) /*|| !_soundResource.isPlaying()*/) {
		_countdown1 = 12;
		_palette->startFadeToBlack(11);
	}

	if (_countdown3 != 0)
		_countdown3--;

	if (_countdown3 == 0 && _flag && _countdown1 == 0) {
		_countdown1 = 12;
		_palette->startFadeToBlack(11);
	}
	
}

uint32 Scene1501::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Scene::handleMessage(messageNum, param, sender);
	if (messageNum == 0x0009) {
		_flag = true;
	}
	return messageResult;
}

} // End of namespace Neverhood
