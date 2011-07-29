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

#include "neverhood/module1800.h"
#include "neverhood/navigationscene.h"

namespace Neverhood {

Module1800::Module1800(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {
	
	debug("Create Module1800(%d)", which);

	// TODO Sound1ChList_addSoundResources(0x04A14718, dword_4AFE70);
	// TODO Sound1ChList_setSoundValuesMulti(dword_4AFE70, 1, 50, 600, 10, 150);
	// TODO Sound1ChList_sub_407C70(0x04A14718, 0x8A382B55, 0x0C242F1D, 0);

	if (which < 0) {
		switch (_vm->gameState().sceneNum) {
		case 0:
			createScene1801(-1);
			break;
		case 1:
			createScene1802(-1);
			break;
		case 2:
			createScene1803(-1);
			break;
		default:
		case 3:
			createScene1804(-1);
			break;
		case 4:
			createScene1805(-1);
			break;
		case 5:
			createScene1806(-1);
			break;
		case 6:
			createScene1807(-1);
			break;
		case 7:
			createScene1808(-1);
			break;
		case 8:
			createScene1809(-1);
			break;
		}
	} else if (which == 2) {
		createScene1806(0);
	} else if (which == 3) {
		createScene1801(0);
	} else {
		createScene1804(1);
	} 

}

Module1800::~Module1800() {
	// TODO Sound1ChList_sub_407A50(0x04A14718);
}

void Module1800::createScene1801(int which) {
	static const byte kNavigationTypes[] = {1, 0, 2, 0};
	_vm->gameState().sceneNum = 0;
	createNavigationScene(0x004AFD38, which, kNavigationTypes);
	SetUpdateHandler(&Module1800::updateScene1801);
}
			
void Module1800::createScene1802(int which) {
	static const byte kNavigationTypes[] = {5};
	_vm->gameState().sceneNum = 1;
	createNavigationScene(0x004AFD98, which, kNavigationTypes);
	SetUpdateHandler(&Module1800::updateScene1802);
}

void Module1800::createScene1803(int which) {
	_vm->gameState().sceneNum = 2;
	createSmackerScene(0x006C0085, true, true, false);
	SetUpdateHandler(&Module1800::updateScene1803);
}
			
void Module1800::createScene1804(int which) {
	_vm->gameState().sceneNum = 3;
	createNavigationScene(0x004AFDB0, which);
	SetUpdateHandler(&Module1800::updateScene1804);
}

void Module1800::createScene1804b(int which) {
	_vm->gameState().sceneNum = 3;
	createSmackerScene(0x0A840C01, true, true, false);
	SetUpdateHandler(&Module1800::updateScene1803);
}
			
void Module1800::createScene1805(int which) {
	_vm->gameState().sceneNum = 4;
	createNavigationScene(0x004AFDE0, which);
	SetUpdateHandler(&Module1800::updateScene1805);
}

void Module1800::createScene1806(int which) {
	_vm->gameState().sceneNum = 5;
	createNavigationScene(0x004AFE40, which);
	SetUpdateHandler(&Module1800::updateScene1806);
}

void Module1800::createScene1807(int which) {
	_vm->gameState().sceneNum = 6;
	createSmackerScene(0x08D84010, true, true, false);
	SetUpdateHandler(&Module1800::updateScene1803);
	// TODO Sound1ChList_sub_407A50(0x04A14718);
}
			
void Module1800::createScene1808(int which) {
	_vm->gameState().sceneNum = 7;
	// TODO Sound1ChList_setSoundValuesMulti(dword_4AFE70, 0, 0, 0, 0, 0);
	createSmackerScene(0x0168B121, true, true, false);
	SetUpdateHandler(&Module1800::updateScene1803);
}
			
void Module1800::createScene1809(int which) {
#if 0 // TODO
	_vm->gameState().sceneNum = 8;
	_childObject = new CreditsScene(_vm, this, 0);
	SetUpdateHandler(&Module1800::updateScene1809);
#endif	
}

void Module1800::updateScene1801() {
	_childObject->handleUpdate();
#if 0 // TODO
	NavigationScene *navigationScene = (NavigationScene*)_childObject;
	if (navigationScene->soundFlag1 && navigationScene->index == 2) {
		// TODO Sound1ChList_sub_4080B0(false);
	}
#endif	
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 1) {
			createScene1805(0);
			_childObject->handleUpdate();
		} else if (_field20 == 2) {
			createScene1802(-1);
			_childObject->handleUpdate();
		} else if (_field20 == 3) {
			createScene1804(0);
			_childObject->handleUpdate();
		}
	}
}

void Module1800::updateScene1802() {
	_childObject->handleUpdate();
	if (_done) {
		int areaType = navigationScene()->getNavigationAreaType();
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (areaType == 3) {
			createScene1808(-1);
		} else {
			createScene1803(-1);
		}
		_childObject->handleUpdate();
	}
}

void Module1800::updateScene1803() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 2) {
			createScene1801(2);
			_childObject->handleUpdate();
		} else if (_field20 == 3) {
			_parentModule->sendMessage(0x1009, 0, this);
		} else if (_field20 == 6) {
			createScene1809(-1);
			_childObject->handleUpdate();
		} else if (_field20 == 7) {
			_parentModule->sendMessage(0x1009, 3, this);
		}
	}
}

void Module1800::updateScene1804() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 0) {
			createScene1804b(-1);
			_childObject->handleUpdate();
		} else if (_field20 == 1) {
			createScene1801(1);
			_childObject->handleUpdate();
		}
	}
}

void Module1800::updateScene1805() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 0) {
			createScene1807(-1);
			_childObject->handleUpdate();
		} else if (_field20 == 1) {
			createScene1806(0);
			_childObject->handleUpdate();
		} else if (_field20 == 2) {
			createScene1801(3);
			_childObject->handleUpdate();
		} else if (_field20 == 3) {
			createScene1805(3);
			_childObject->handleUpdate();
		}
	}
}

void Module1800::updateScene1806() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		if (_field20 == 0) {
			_parentModule->sendMessage(0x1009, 2, this);
		} else if (_field20 == 1) {
			createScene1805(3);
			_childObject->handleUpdate();
		}
	}
	if (_field24 >= 0) {
		if (_field24 == 1) {
			// TODO _resourceTable.setResourceList(ex_sub_42EDA0(0), true);
		}
		_field24 = -1;
	}
}

void Module1800::updateScene1809() {
	_childObject->handleUpdate();
	if (_done) {
		_done = false;
		delete _childObject;
		_childObject = NULL;
		_parentModule->sendMessage(0x1009, 1, this);
		// TODO GameState stuff
	}
}
} // End of namespace Neverhood
