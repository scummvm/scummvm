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

#include "neverhood/module2300.h"

namespace Neverhood {

Module2300::Module2300(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule), _volume(0) {
	
	// TODO Sound1ChList_addSoundResources(0x1A214010, dword_4B6938, true);
	// TODO Sound1ChList_setSoundValuesMulti(dword_4B6938, true, 50, 600, 10, 10, 150);

	_flag = getGlobalVar(0x10938830) == 0;
	
	if (_flag) {
		// TODO Sound1ChList_setVolume(0x90F0D1C3, 0);
		// TODO Sound1ChList_playLooping(0x90F0D1C3);
	} else {
		// TODO Sound1ChList_setSoundValues(0x90F0D1C3, false, 0, 0, 0, 0);
	}

	// TODO Sound1ChList_sub_407C70(0x1A214010, 0x48498E46, 0x50399F64, 0);
	// TODO Sound1ChList_sub_407C70(0x1A214010, 0x41861371, 0x43A2507F, 0);

	if (which < 0) {
		createScene(_vm->gameState().sceneNum, -1);
	} else if (which == 1) {
		createScene(2, 0);
	} else if (which == 2) {
		createScene(3, 0);
	} else if (which == 3) {
		createScene(4, -1);
	} else if (which == 4) {
		createScene(1, 3);
	} else {
		createScene(0, 1);
	}

}

Module2300::~Module2300() {
	// TODO Sound1ChList_sub_407A50(0x1A214010);
}

void Module2300::createScene(int sceneNum, int which) {
	debug("Module2300::createScene(%d, %d)", sceneNum, which);
	_vm->gameState().sceneNum = sceneNum;
	switch (_vm->gameState().sceneNum) {
	case 0:
		createNavigationScene(0x004B67B8, which);
		break;
	case 1:
		createNavigationScene(0x004B67E8, which);
		if (_flag) {
			_volume = 15;
			// TODO Sound1ChList_setVolume(0x90F0D1C3, 15);
		}
		break;
	case 2:
		createNavigationScene(0x004B6878, which);
		break;
	case 3:
		if (getGlobalVar(0x10938830)) {
			createNavigationScene(0x004B68F0, which);
		} else {
			// TODO Sound1ChList_setVolume(0x90F0D1C3, _volume);
			createNavigationScene(0x004B68A8, which);
			if (_flag) {
				_volume = 87;
				// TODO Sound1ChList_setVolume(0x90F0D1C3, 87);
			}
		}
		break;
	case 4:
		// TODO Sound1ChList_sub_4080B0(true);
		createSmackerScene(0x20080A0B, true, true, false);
		break;
	}
	SetUpdateHandler(&Module2300::updateScene);
	_childObject->handleUpdate();
}

void Module2300::updateScene() {
	if (!updateChild()) {
		switch (_vm->gameState().sceneNum) {
		case 0:
			if (_moduleResult == 1) {
				createScene(1, 4);
			} else {
				leaveModule(0);
			}
			break;
		case 1:
			if (_moduleResult == 1) {
				createScene(0, 0);
			} else if (_moduleResult == 2) {
				createScene(2, 1);
			} else if (_moduleResult == 3) {
				createScene(1, 3);
			} else if (_moduleResult == 4) {
				createScene(3, 1);
			} else if (_moduleResult == 5) {
				leaveModule(3);
			} else {
				leaveModule(4);
			}
			break;
		case 2:
			if (_moduleResult == 1) {
				leaveModule(3);
			} else {
				createScene(1, 5);
			}
			break;
		case 3:
			if (_moduleResult == 1) {
				leaveModule(2);
			} else {
				createScene(1, 1);
			}
			break;
		case 4:
			// TODO Sound1ChList_sub_4080B0(false);
			createScene(1, 2);
			break;
		}
	} else {
		switch (_vm->gameState().sceneNum) {
		case 1:
#if 0 // TODO
			NavigationScene *navigationScene = (NavigationScene*)_childObject; 
			if (_flag && navigationScene->getSoundFlag1() && navigationScene->getNavigationIndex() == 4 && 
				navigationScene->getSmackerPlayer() && navigationScene->getSmackerPlayer()->getFrameNumber() % 2) {
				_volume++;
				Sound1ChList_setVolume(0x90F0D1C3, _volume);
			}
#endif
#if 0 // TODO
			if (navigationScene->getSoundFlag1() && navigationScene->getNavigationIndex() == 0 && 
				navigationScene->getSmackerPlayer() && navigationScene->getSmackerPlayer()->getFrameNumber() == 50) {
				Sound1ChList_sub_407C70(0x1A214010, 0x48498E46, 0x50399F64);
				Sound1ChList_setVolume(0x48498E46, 70);
				Sound1ChList_setVolume(0x50399F64, 70);
			}
#endif
			break;
		case 3:
#if 0 // TODO
			NavigationScene *navigationScene = (NavigationScene*)_childObject; 
			if (_flag && navigationScene->getSoundFlag1() && navigationScene->getSmackerPlayer() && 
				navigationScene->getSmackerPlayer()->getFrameNumber() % 2) {
				_volume--;
				Sound1ChList_setVolume(0x90F0D1C3, _volume);
			}
#endif
			break;
		}
	}
}
			
} // End of namespace Neverhood
