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

#include "neverhood/navigationscene.h"
#include "neverhood/mouse.h"

namespace Neverhood {

enum AreaType {
	kAreaCanMoveForward = 0,
	kAreaCannotMoveForward = 1
};

NavigationScene::NavigationScene(NeverhoodEngine *vm, Module *parentModule, uint32 navigationListId, int navigationIndex, const byte *itemsTypes)
	: Scene(vm, parentModule), _itemsTypes(itemsTypes), _navigationIndex(navigationIndex), _smackerDone(false),
	_isWalkingForward(false), _isTurning(false), _smackerFileHash(0), _interactive(true), _leaveSceneAfter(false) {

	_navigationList = _vm->_staticData->getNavigationList(navigationListId);
	_navigationListId = navigationListId;

	if (_navigationIndex < 0) {
		_navigationIndex = (int)getGlobalVar(V_NAVIGATION_INDEX);
		if (_navigationIndex >= (int)_navigationList->size())
			_navigationIndex = 0;
	}
	setGlobalVar(V_NAVIGATION_INDEX, _navigationIndex);

	SetUpdateHandler(&NavigationScene::update);
	SetMessageHandler(&NavigationScene::handleMessage);

	_smackerPlayer = addSmackerPlayer(new SmackerPlayer(_vm, this, (*_navigationList)[_navigationIndex].fileHash, true, true));

	createMouseCursor();

	_vm->_screen->clear();
	_vm->_screen->setSmackerDecoder(_smackerPlayer->getSmackerDecoder());

	sendMessage(_parentModule, 0x100A, _navigationIndex);
}

NavigationScene::~NavigationScene() {
	_vm->_soundMan->setTwoSoundsPlayFlag(false);
	_vm->_soundMan->setSoundThreePlayFlag(false);
}

int NavigationScene::getNavigationAreaType() {
	NPoint mousePos;
	mousePos.x = _mouseCursor->getX();
	mousePos.y = _mouseCursor->getY();
	return sendPointMessage(_mouseCursor, 0x2064, mousePos);
}

void NavigationScene::update() {
	if (_smackerFileHash != 0) {
		showMouse(false);
		_smackerPlayer->open(_smackerFileHash, false);
		_vm->_screen->clear();
		_vm->_screen->setSmackerDecoder(_smackerPlayer->getSmackerDecoder());
		_smackerDone = false;
		/*
		if (!_interactive)
			_smackerDone = true;
		*/
		_smackerFileHash = 0;
	} else if (_smackerDone) {
		if (_leaveSceneAfter) {
			_vm->_screen->setSmackerDecoder(NULL);
			sendMessage(_parentModule, 0x1009, _navigationIndex);
		} else {
			const NavigationItem &navigationItem = (*_navigationList)[_navigationIndex];
			createMouseCursor();
			showMouse(true);
			_isTurning = false;
			_isWalkingForward = false;
			_interactive = true;
			_vm->_soundMan->setTwoSoundsPlayFlag(false);
			_vm->_soundMan->setSoundThreePlayFlag(false);
			_smackerDone = false;
			_smackerPlayer->open(navigationItem.fileHash, true);
			_vm->_screen->clear();
			_vm->_screen->setSmackerDecoder(_smackerPlayer->getSmackerDecoder());
			sendMessage(_parentModule, 0x100A, _navigationIndex);
		}
	}
	Scene::update();
}

uint32 NavigationScene::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	switch (messageNum) {
	case NM_MOUSE_MOVE:
		if (_interactive)
			sendMessage(_mouseCursor, 0x4002, param);
		break;
	case NM_MOUSE_CLICK:
		if (_interactive)
			handleNavigation(param.asPoint());
		break;
	case NM_KEYPRESS_SPACE:
		if (!_interactive)
			_smackerDone = true;
		break;
	case NM_ANIMATION_STOP:
		_smackerDone = true;
		break;
	default:
		break;
	}
	return 0;
}

void NavigationScene::createMouseCursor() {
	const NavigationItem &navigationItem = (*_navigationList)[_navigationIndex];
	uint32 mouseCursorFileHash;
	int areaType;

	if (_mouseCursor)
		deleteSprite((Sprite**)&_mouseCursor);

	mouseCursorFileHash = navigationItem.mouseCursorFileHash;
	if (mouseCursorFileHash == 0)
		mouseCursorFileHash = 0x63A40028;

	if (_itemsTypes)
		areaType = _itemsTypes[_navigationIndex];
	else if (navigationItem.middleSmackerFileHash != 0 || navigationItem.middleFlag)
		areaType = kAreaCanMoveForward;
	else
		areaType = kAreaCannotMoveForward;

	insertNavigationMouse(mouseCursorFileHash, areaType);
	sendPointMessage(_mouseCursor, 0x4002, _vm->getMousePos());
}

void NavigationScene::handleNavigation(const NPoint &mousePos) {
	const NavigationItem &navigationItem = (*_navigationList)[_navigationIndex];
	bool oldIsWalkingForward = _isWalkingForward;
	bool oldIsTurning = _isTurning;
	uint32 direction = sendPointMessage(_mouseCursor, 0x2064, mousePos);

	switch (direction) {
	case 0:
		if (navigationItem.leftSmackerFileHash != 0) {
			_smackerFileHash = navigationItem.leftSmackerFileHash;
			_interactive = false;
			_isWalkingForward = false;
			_isTurning = true;
			do {
				_navigationIndex--;
				if (_navigationIndex < 0)
					_navigationIndex = _navigationList->size() - 1;
			} while (!(*_navigationList)[_navigationIndex].interactive);
			setGlobalVar(V_NAVIGATION_INDEX, _navigationIndex);
		} else {
			_vm->_screen->setSmackerDecoder(NULL);
			sendMessage(_parentModule, 0x1009, _navigationIndex);
		}
		break;
	case 1:
		if (navigationItem.rightSmackerFileHash != 0) {
			_smackerFileHash = navigationItem.rightSmackerFileHash;
			_interactive = false;
			_isWalkingForward = false;
			_isTurning = true;
			do {
				_navigationIndex++;
				if (_navigationIndex >= (int)_navigationList->size())
					_navigationIndex = 0;
			} while (!(*_navigationList)[_navigationIndex].interactive);
			setGlobalVar(V_NAVIGATION_INDEX, _navigationIndex);
		} else {
			_vm->_screen->setSmackerDecoder(NULL);
			sendMessage(_parentModule, 0x1009, _navigationIndex);
		}
		break;
	case 2:
	case 3:
	case 4:
		if (navigationItem.middleFlag) {
			_vm->_screen->setSmackerDecoder(NULL);
			sendMessage(_parentModule, 0x1009, _navigationIndex);
		} else if (navigationItem.middleSmackerFileHash != 0) {
			_smackerFileHash = navigationItem.middleSmackerFileHash;
			_interactive = false;
			_isWalkingForward = true;
			_isTurning = false;
			_leaveSceneAfter = true;
		}
		break;
	default:
		break;
	}

	if (oldIsTurning != _isTurning)
		_vm->_soundMan->setSoundThreePlayFlag(_isTurning);

	if (oldIsWalkingForward != _isWalkingForward)
		_vm->_soundMan->setTwoSoundsPlayFlag(_isWalkingForward);
}

} // End of namespace Neverhood
