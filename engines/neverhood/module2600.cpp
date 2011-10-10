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

#include "neverhood/module2600.h"

namespace Neverhood {

Module2600::Module2600(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {
	
	if (which < 0) {
		createScene(_vm->gameState().sceneNum, -1);
	} else if (which == 1) {
		createScene(4, 1);
	} else {
		createScene(0, 1);
	}

	// TODO Sound1ChList_addSoundResources(0x40271018, dword_4B87E8, true);
	// TODO Sound1ChList_setSoundValuesMulti(dword_4B87E8, true, 50, 600, 5, 150);
	// TODO Sound1ChList_sub_407C70(0x40271018, 0x41861371, 0x43A2507F);

}

Module2600::~Module2600() {
	// TODO Sound1ChList_sub_407A50(0x40271018);
}

void Module2600::createScene(int sceneNum, int which) {
	debug("Module2600::createScene(%d, %d)", sceneNum, which);
	_vm->gameState().sceneNum = sceneNum;
	switch (_vm->gameState().sceneNum) {
	case 0:
		createNavigationScene(0x004B8608, which);
		break;
	case 1:
		createNavigationScene(0x004B8638, which);
		break;
	case 2:
		createNavigationScene(0x004B86C8, which);
		break;
	case 3:
		if (getGlobalVar(0x0A310817)) {
			createNavigationScene(0x004B8758, which);
		} else {
			createNavigationScene(0x004B86F8, which);
		}
		break;
	case 4:
		createNavigationScene(0x004B87B8, which);
		break;
	case 6:
		createNavigationScene(0x004B8698, which);
		break;
	case 7:
		// TODO Sound1ChList_sub_407A50(0x40271018);
		createSmackerScene(0x30090001, true, true, false);
		break;
	case 8:
//TODO		_childObject = new Scene2609(_vm, this, which);
		break;
	case 1002:
		if (getGlobalVar(0x40040831) == 1) {
			createSmackerScene(0x018C0404, true, true, false);
		} else if (getGlobalVar(0x40040831) == 2) {
			createSmackerScene(0x018C0407, true, true, false);
		} else {
			createSmackerScene(0x818C0405, true, true, false);
		}
		if (getGlobalVar(0x40040831) >= 2) {
			setGlobalVar(0x40040831, 0);
		} else {
			incGlobalVar(0x40040831, +1);
		}
		break;
	case 1003:
		createSmackerScene(0x001C0007, true, true, false);
		break;
	case 1006:
		if (getGlobalVar(0x4E0BE910)) {
			createSmackerScene(0x049A1181, true, true, false);
		} else {
			createSmackerScene(0x04981181, true, true, false);
		}
		break;
	case 1008:
		if (getGlobalVar(0x4E0BE910)) {
			createSmackerScene(0x42B80941, true, true, false);
		} else {
			createSmackerScene(0x42980941, true, true, false);
		}
		break;
	}
	SetUpdateHandler(&Module2600::updateScene);
	_childObject->handleUpdate();
}

void Module2600::updateScene() {
	if (!updateChild()) {
		switch (_vm->gameState().sceneNum) {
		case 0:
			if (_moduleResult == 1) {
				createScene(1, 3);
			} else {
				leaveModule(0);
			}
			break;
		case 1:
			if (_moduleResult == 0) {
				createScene(6, 0);
			} else if (_moduleResult == 1) {
				createScene(0, 0);
			} else if (_moduleResult == 2) {
				createScene(2, 1);
			} else if (_moduleResult == 3) {
				createScene(3, 0);
			}
			break;
		case 2:
			if (_moduleResult == 0) {
				createScene(1, 0);
			} else if (_moduleResult == 1) {
				createScene(1002, -1);
			}
			break;
		case 3:
			if (_moduleResult == 0) {
				if (getGlobalVar(0x0A310817)) {
					createScene(4, 0);
				} else {
					createScene(1003, -1);
				}
			} else if (_moduleResult == 2) {
				createScene(1, 1);
			} else if (_moduleResult == 3) {
				if (getGlobalVar(0x0A310817)) {
					createScene(4, 0);
				} else {
					setGlobalVar(0x0A310817, 1);
					createScene(7, -1);
				}
			}
			break;
		case 4:
			if (_moduleResult == 0) {
				leaveModule(1);
			} else {
				createScene(3, 1);
			}
			break;
		case 6:
			if (_moduleResult == 0) {
				createScene(1006, -1);
			} else if (_moduleResult == 1) {
				createScene(1, 2);
			}
			break;
		case 7:
			leaveModule(0);
			break;
		case 8:
			createScene(1008, -1);
			break;
		case 1002:
			createScene(2, 1);
			break;
		case 1003:
			createScene(3, 0);
			break;
		case 1006:
			createScene(8, -1);
			break;
		case 1008:
			createScene(6, 0);
			break;
		}
	}
}
			
} // End of namespace Neverhood
