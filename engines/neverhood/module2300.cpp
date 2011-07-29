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
	
	debug("Create Module2300(%d)", which);

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
		switch (_vm->gameState().sceneNum) {
		case 0:
			createScene2301(-1);
			break;
		case 1:
			createScene2302(-1);
			break;
		case 2:
			createScene2303(-1);
			break;
		case 3:
			createScene2304(-1);
			break;
		case 4:
			createScene2305(-1);
			break;
		}
	} else if (which == 1) {
		createScene2303(0);
	} else if (which == 2) {
		createScene2304(0);
	} else if (which == 3) {
		createScene2305(-1);
	} else if (which == 4) {
		createScene2302(3);
	} else {
		createScene2301(1);
	}

}

Module2300::~Module2300() {
	// TODO Sound1ChList_sub_407A50(0x1A214010);
}

void Module2300::createScene2301(int which) {
	_vm->gameState().sceneNum = 0;
	createNavigationScene(0x004B67B8, which);
	SetUpdateHandler(&Module2300::updateScene2301);
}
			
void Module2300::createScene2302(int which) {
	_vm->gameState().sceneNum = 1;
	createNavigationScene(0x004B67E8, which);
	SetUpdateHandler(&Module2300::updateScene2302);
	if (_flag) {
		_volume = 15;
		// TODO Sound1ChList_setVolume(0x90F0D1C3, 15);
	}
}

void Module2300::createScene2303(int which) {
	_vm->gameState().sceneNum = 2;
	createNavigationScene(0x004B6878, which);
	SetUpdateHandler(&Module2300::updateScene2303);
}

void Module2300::createScene2304(int which) {
	_vm->gameState().sceneNum = 3;
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
	SetUpdateHandler(&Module2300::updateScene2304);
}

void Module2300::createScene2305(int which) {
	_vm->gameState().sceneNum = 4;
	// TODO Sound1ChList_sub_4080B0(true);
	createSmackerScene(0x20080A0B, true, true, false);
	SetUpdateHandler(&Module2300::updateScene2305);
}

void Module2300::updateScene2301() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2302(4);
			_childObject->handleUpdate();
		} else {
			_parentModule->sendMessage(0x1009, 0, this);
		}
	}
	if (_field24 >= 0) {
		if (_field24 == 0) {
			_parentModule->sendMessage(0x100A, 0, this);
		}
		_field24 = -1;
	}
	if (_field26 >= 0) {
		_parentModule->sendMessage(0x1023, 0, this);
		_field26 = -1;
	}
}

void Module2300::updateScene2302() {
	_childObject->handleUpdate();
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
	if (_done) {
		debug("SCENE 2302 DONE; _field20 = %d", _field20);
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene2301(0);
			_childObject->handleUpdate();
		} else if (_field20 == 2) {
			createScene2303(1);
			_childObject->handleUpdate();
		} else if (_field20 == 3) {
			createScene2302(3);
			_childObject->handleUpdate();
		} else if (_field20 == 4) {
			createScene2304(1);
			_childObject->handleUpdate();
		} else if (_field20 == 5) {
			_parentModule->sendMessage(0x1009, 3, this);
		} else {
			_parentModule->sendMessage(0x1009, 4, this);
		}
	}
}
			
void Module2300::updateScene2303() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			_parentModule->sendMessage(0x1009, 3, this);
		} else {
			createScene2302(5);
			_childObject->handleUpdate();
		}
	}
	if (_field24 >= 0) {
		if (_field24 == 1) {
			// TODO _resourceTable1.setResourceList(ex_sub_479D00(0), true);
			// TODO _resourceTable2.loadResources();
		}
		_field24 = -1;
	}
}
			
void Module2300::updateScene2304() {
	_childObject->handleUpdate();
#if 0 // TODO
	NavigationScene *navigationScene = (NavigationScene*)_childObject; 
	if (_flag && navigationScene->getSoundFlag1() && navigationScene->getSmackerPlayer() && 
		navigationScene->getSmackerPlayer()->getFrameNumber() % 2) {
		_volume--;
		Sound1ChList_setVolume(0x90F0D1C3, _volume);
	}
#endif
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			_parentModule->sendMessage(0x1009, 2, this);
		} else {
			createScene2302(1);
			_childObject->handleUpdate();
		}
	}
	if (_field24 >= 0) {
		if (_field24 == 0) {
			_parentModule->sendMessage(0x100A, 2, this);
		}
		_field24 = -1;
	}
	if (_field26 >= 0) {
		if (_field26 == 1) {
			_parentModule->sendMessage(0x1023, 2, this);
		}
		_field26 = -1;
	}
}
			
void Module2300::updateScene2305() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		// TODO Sound1ChList_sub_4080B0(false);
		createScene2302(2);
		_childObject->handleUpdate();
	}
}
			
} // End of namespace Neverhood
