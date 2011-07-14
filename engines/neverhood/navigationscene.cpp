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

#include "neverhood/navigationscene.h"
#include "neverhood/mouse.h"

namespace Neverhood {

NavigationScene::NavigationScene(NeverhoodEngine *vm, Module *parentModule, uint32 navigationListId, int navigationIndex, byte *itemsTypes)
	: Scene(vm, parentModule, true), _itemsTypes(itemsTypes), _navigationIndex(navigationIndex), _smackerDone(false),
	_soundFlag1(false), _soundFlag2(false), _smackerFileHash(0), _interactive(true), _done(false) {

	_navigationList = _vm->_staticData->getNavigationList(navigationListId);
	for (NavigationList::iterator it = _navigationList->begin(); it != _navigationList->end(); it++) {
		debug("%08X %08X %08X %08X %d %d %08X",	(*it).fileHash,	(*it).leftSmackerFileHash, (*it).rightSmackerFileHash,
		(*it).middleSmackerFileHash, (*it).interactive, (*it).middleFlag, (*it).mouseCursorFileHash);
	}

	if (_navigationIndex < 0) {
		_navigationIndex = (int)getGlobalVar(0x4200189E);
		if (_navigationIndex >= (int)_navigationList->size())
			_navigationIndex = 0; 
	}
	setGlobalVar(0x4200189E, _navigationIndex);
	
	SetUpdateHandler(&NavigationScene::update);
	SetMessageHandler(&NavigationScene::handleMessage);
	
	_smackerPlayer = new SmackerPlayer(_vm, this, (*_navigationList)[_navigationIndex].fileHash, true, true);	

	addEntity(_smackerPlayer);
	
	addSurface(_smackerPlayer->getSurface());

	createMouseCursor();

	_vm->_screen->clear();

	_parentModule->sendMessage(0x100A, _navigationIndex, this);

}

NavigationScene::~NavigationScene() {
	// TODO Sound1ChList_sub_4080B0(0);
	// TODO Sound1ChList_sub_408110(0);
}

byte NavigationScene::getNavigationAreaType() {
	return 0; // TODO
}

void NavigationScene::update() {
	if (_smackerFileHash != 0) {
		_mouseCursor->getSurface()->setVisible(false);
		_smackerPlayer->open(_smackerFileHash, false);
		_vm->_screen->clear();
		_smackerDone = false;
		_smackerFileHash = 0;
	} else if (_smackerDone) {
		if (_done) {
			_parentModule->sendMessage(0x1009, _navigationIndex, this);
		} else {
			const NavigationItem &navigationItem = (*_navigationList)[_navigationIndex];
			createMouseCursor();
			_mouseCursor->getSurface()->setVisible(true);
			_soundFlag2 = false;
			_soundFlag1 = false;
			_interactive = true;
			// TODO Sound1ChList_sub_4080B0(0);
			// TODO Sound1ChList_sub_408110(0);
			_smackerDone = false;
			_smackerPlayer->open(navigationItem.fileHash, true);
			_vm->_screen->clear();
			_parentModule->sendMessage(0x100A, _navigationIndex, this);
		}
	} 
	Scene::update();
}

uint32 NavigationScene::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	switch (messageNum) {
	case 0x0000:
		_mouseCursor->sendMessage(0x4002, param, this);
		break;
	case 0x0001:
		handleNavigation(param.asPoint());
		break;
	case 0x0009:
		if (!_interactive)
			_smackerDone = true;
		break;
	case 0x3002:
		_smackerDone = true;
		break;
	}
	return 0;
}

void NavigationScene::createMouseCursor() {

	const NavigationItem &navigationItem = (*_navigationList)[_navigationIndex];
	uint32 mouseCursorFileHash;
	int areaType;

	if (_mouseCursor) {
		deleteSprite(&_mouseCursor);
	}

	mouseCursorFileHash = navigationItem.mouseCursorFileHash;
	// TODO: Check the resource...
	if (mouseCursorFileHash == 0)
		mouseCursorFileHash = 0x63A40028;
		
	if (_itemsTypes) {
		areaType = _itemsTypes[_navigationIndex];
	} else if (navigationItem.middleSmackerFileHash != 0 || navigationItem.middleFlag) {
		areaType = 0;
	} else {
		areaType = 1;
	}

	_mouseCursor = addSprite(new NavigationMouse(_vm, mouseCursorFileHash, areaType));
	_mouseCursor->sendPointMessage(0x4002, _vm->getMousePos(), this);
	
}

void NavigationScene::handleNavigation(const NPoint &mousePos) {

	const NavigationItem &navigationItem = (*_navigationList)[_navigationIndex];
	bool oldSoundFlag1 = _soundFlag1;
	bool oldSoundFlag2 = _soundFlag2;
	uint32 direction = _mouseCursor->sendPointMessage(0x2064, mousePos, this);
	
	switch (direction) {
	// TODO: Merge cases 0 and 1?
	case 0:
		if (navigationItem.leftSmackerFileHash != 0) {
			_smackerFileHash = navigationItem.leftSmackerFileHash;
			_interactive = false;
			_soundFlag1 = false;
			_soundFlag2 = true;
			do {
				_navigationIndex--;
				if (_navigationIndex < 0)
					_navigationIndex = _navigationList->size() - 1;
			} while (!(*_navigationList)[_navigationIndex].interactive);
			setGlobalVar(0x4200189E, _navigationIndex);
		} else {
			_parentModule->sendMessage(0x1009, _navigationIndex, this);
		}
		break;
	case 1:
		if (navigationItem.rightSmackerFileHash != 0) {
			_smackerFileHash = navigationItem.rightSmackerFileHash;
			_interactive = false;
			_soundFlag1 = false;
			_soundFlag2 = true;
			do {
				_navigationIndex++;
				if (_navigationIndex >= (int)_navigationList->size())
					_navigationIndex = 0;
			} while (!(*_navigationList)[_navigationIndex].interactive);
			setGlobalVar(0x4200189E, _navigationIndex);
		} else {
			_parentModule->sendMessage(0x1009, _navigationIndex, this);
		}
		break;
	case 2:
	case 3:
	case 4:
		if (navigationItem.middleFlag) {
			_parentModule->sendMessage(0x1009, _navigationIndex, this);
		} else if (navigationItem.middleSmackerFileHash != 0) {
			_smackerFileHash = navigationItem.middleSmackerFileHash;
			_interactive = false;
			_soundFlag1 = true;
			_soundFlag2 = false;
			_done = true;
		}
		break;
	}
	
	if (oldSoundFlag2 != _soundFlag2) {
		// TODO Sound1ChList_sub_408110(_soundFlag2);
	}

	if (oldSoundFlag1 != _soundFlag1) {
		// TODO Sound1ChList_sub_4080B0(_soundFlag1);
	}

}

} // End of namespace Neverhood
