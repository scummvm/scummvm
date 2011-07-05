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

	_musicFileHash = _vm->getGlobalVar(0xD0A14D10) ? 0x81106480 : 0x00103144;		

	// TODO Music18hList_add(0x03294419, 0x061880C6);
	// TODO Music18hList_add(0x03294419, _musicFileHash);

	if (which < 0) {
		switch (_vm->gameState().sceneNum) {
		case 0:
			createScene1001(-1);
			break;
		case 1:
			createScene1002(-1);
			break;
		case 2:
			createScene1003(-1);
			break;
		case 3:
			createScene1004(-1);
			break;
		case 4:
			createScene1005(-1);
			break;
		}
	} else if (which == 0) {
		createScene1001(0);
	} else if (which == 1) {
		createScene1002(1);
	}

}

Module1000::~Module1000() {
	// TODO Music18hList_deleteGroup(0x03294419);
}

void Module1000::createScene1001(int which) {
	debug("createScene1501");
	_vm->gameState().sceneNum = 0;
	_childObject = new Scene1001(_vm, this, which);
	// TODO ResourceTable_multiLoad(&_resourceTable1, &_resourceTable2, &_resourceTable3);
	// TODO Music18hList_play(0x061880C6, 0, 0, 1);
	SetUpdateHandler(&Module1000::updateScene1001);
}
			
void Module1000::createScene1002(int which) {
}

void Module1000::createScene1003(int which) {
}

void Module1000::createScene1004(int which) {
}

void Module1000::createScene1005(int which) {
}

void Module1000::updateScene1001() {
}

void Module1000::updateScene1002() {
}
			
void Module1000::updateScene1003() {
}
			
void Module1000::updateScene1004() {
}
			
void Module1000::updateScene1005() {
}
			
// Scene1001			
				
Scene1001::Scene1001(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true) {
}

void Scene1001::update() {
}

uint32 Scene1001::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	return 0;
}

} // End of namespace Neverhood
