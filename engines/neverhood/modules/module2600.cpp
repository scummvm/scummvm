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

#include "neverhood/modules/module2600.h"
#include "neverhood/modules/module2600_sprites.h"

namespace Neverhood {

static const uint32 kModule2600SoundList[] = {
	0xB288D450,
	0x90804450,
	0x99801500,
	0xB288D455,
	0x93825040,
	0
};

Module2600::Module2600(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {

	if (which < 0)
		createScene(_vm->gameState().sceneNum, -1);
	else if (which == 1)
		createScene(4, 1);
	else
		createScene(0, 1);

	_vm->_soundMan->addSoundList(0x40271018, kModule2600SoundList);
	_vm->_soundMan->setSoundListParams(kModule2600SoundList, true, 50, 600, 5, 150);
	_vm->_soundMan->playTwoSounds(0x40271018, 0x41861371, 0x43A2507F, 0);

}

Module2600::~Module2600() {
	_vm->_soundMan->deleteGroup(0x40271018);
}

void Module2600::createScene(int sceneNum, int which) {
	debug(1, "Module2600::createScene(%d, %d)", sceneNum, which);
	_sceneNum = sceneNum;
	switch (_sceneNum) {
	case 0:
		_vm->gameState().sceneNum = 0;
		createNavigationScene(0x004B8608, which);
		break;
	case 1:
		_vm->gameState().sceneNum = 1;
		createNavigationScene(0x004B8638, which);
		break;
	case 2:
		_vm->gameState().sceneNum = 2;
		createNavigationScene(0x004B86C8, which);
		break;
	case 3:
		_vm->gameState().sceneNum = 3;
		if (getGlobalVar(V_CREATURE_ANGRY))
			createNavigationScene(0x004B8758, which);
		else
			createNavigationScene(0x004B86F8, which);
		break;
	case 4:
		_vm->gameState().sceneNum = 4;
		createNavigationScene(0x004B87B8, which);
		break;
	case 6:
		_vm->gameState().sceneNum = 6;
		createNavigationScene(0x004B8698, which);
		break;
	case 7:
		_vm->gameState().sceneNum = 7;
		_vm->_soundMan->deleteGroup(0x40271018);
		createSmackerScene(0x30090001, true, true, false);
		break;
	case 8:
		_vm->gameState().sceneNum = 8;
		_childObject = new Scene2609(_vm, this, which);
		break;
	case 1002:
		_vm->gameState().sceneNum = 2;
		if (getGlobalVar(V_FRUIT_COUNTING_INDEX) == 1)
			createSmackerScene(0x018C0404, true, true, false);
		else if (getGlobalVar(V_FRUIT_COUNTING_INDEX) == 2)
			createSmackerScene(0x018C0407, true, true, false);
		else
			createSmackerScene(0x818C0405, true, true, false);
		if (getGlobalVar(V_FRUIT_COUNTING_INDEX) >= 2)
			setGlobalVar(V_FRUIT_COUNTING_INDEX, 0);
		else
			incGlobalVar(V_FRUIT_COUNTING_INDEX, +1);
		break;
	case 1003:
		_vm->gameState().sceneNum = 3;
		createSmackerScene(0x001C0007, true, true, false);
		break;
	case 1006:
		_vm->gameState().sceneNum = 6;
		if (getGlobalVar(V_WATER_RUNNING))
			createSmackerScene(0x049A1181, true, true, false);
		else
			createSmackerScene(0x04981181, true, true, false);
		break;
	case 1008:
		_vm->gameState().sceneNum = 8;
		if (getGlobalVar(V_WATER_RUNNING))
			createSmackerScene(0x42B80941, true, true, false);
		else
			createSmackerScene(0x42980941, true, true, false);
		break;
	case 9999:
		createDemoScene();
		break;
	default:
		break;
	}
	SetUpdateHandler(&Module2600::updateScene);
	_childObject->handleUpdate();
}

void Module2600::updateScene() {
	if (!updateChild()) {
		switch (_sceneNum) {
		case 0:
			if (_moduleResult == 1)
				createScene(1, 3);
			else
				leaveModule(0);
			break;
		case 1:
			if (_moduleResult == 0)
				createScene(6, 0);
			else if (_moduleResult == 1)
				createScene(0, 0);
			else if (_moduleResult == 2)
				createScene(2, 1);
			else if (_moduleResult == 3)
				createScene(3, 0);
			break;
		case 2:
			if (_moduleResult == 0)
				createScene(1, 0);
			else if (_moduleResult == 1) {
				if (_vm->isDemo())
					createScene(9999, -1);
				else
					createScene(1002, -1);
			}
			break;
		case 3:
			if (_moduleResult == 0) {
				if (getGlobalVar(V_CREATURE_ANGRY))
					createScene(4, 0);
				else
					createScene(1003, -1);
			} else if (_moduleResult == 2)
				createScene(1, 1);
			else if (_moduleResult == 3) {
				if (getGlobalVar(V_CREATURE_ANGRY))
					createScene(4, 0);
				else {
					setGlobalVar(V_CREATURE_ANGRY, 1);
					createScene(7, -1);
				}
			}
			break;
		case 4:
			if (_moduleResult == 0)
				leaveModule(1);
			else
				createScene(3, 1);
			break;
		case 6:
			if (_moduleResult == 0) {
				if (_vm->isDemo())
					createScene(9999, -1);
				else
					createScene(1006, -1);
			}
			else if (_moduleResult == 1)
				createScene(1, 2);
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
		case 9999:
			createScene(_vm->gameState().sceneNum, -1);
			break;
		default:
			break;
		}
	}
}

Scene2609::Scene2609(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule), _isBusy(false) {

	SetUpdateHandler(&Scene::update);
	SetMessageHandler(&Scene2609::handleMessage);

	setBackground(0x51409A16);
	setPalette(0x51409A16);
	_asWater = insertSprite<AsScene2609Water>();
	_ssButton = insertSprite<SsScene2609Button>(this);
	addCollisionSprite(_ssButton);
	insertPuzzleMouse(0x09A1251C, 20, 620);
	insertStaticSprite(0x02138002, 1200);
	insertStaticSprite(0x825E2827, 1200);
}

uint32 Scene2609::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_MOUSE_CLICK:
		if ((param.asPoint().x <= 20 || param.asPoint().x >= 620) && !_isBusy)
			leaveScene(0);
		break;
	case NM_ANIMATION_UPDATE:
		_isBusy = true;
		break;
	case 0x2001:
		_isBusy = false;
		sendMessage(_asWater, 0x2001, 0);
		break;
	case NM_POSITION_CHANGE:
		_isBusy = false;
		sendMessage(_asWater, NM_POSITION_CHANGE, 0);
		break;
	default:
		break;
	}
	return 0;
}

} // End of namespace Neverhood
