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

#include "neverhood/diskplayerscene.h"
#include "neverhood/menumodule.h"
#include "neverhood/navigationscene.h"
#include "neverhood/modules/module1800.h"

namespace Neverhood {

static const uint32 kModule1800SoundList[] = {
	0x16805548,
	0x16805048,
	0xD0E14441,
	0x90E090C2,
	0x90E1D0C2,
	0x90E2D0C2,
	0
};

Module1800::Module1800(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {

	_vm->_soundMan->addSoundList(0x04A14718, kModule1800SoundList);
	_vm->_soundMan->setSoundListParams(kModule1800SoundList, true, 50, 600, 10, 150);
	_vm->_soundMan->playTwoSounds(0x04A14718, 0x8A382B55, 0x0C242F1D, 0);

	if (which < 0)
		createScene(_vm->gameState().sceneNum, -1);
	else if (which == 2)
		createScene(5, 0);
	else if (which == 3)
		createScene(0, 0);
	else
		createScene(3, 1);

}

Module1800::~Module1800() {
	_vm->_soundMan->deleteGroup(0x04A14718);
}

void Module1800::createScene(int sceneNum, int which) {
	static const byte kNavigationTypes00[] = {1, 0, 2, 0};
	static const byte kNavigationTypes01[] = {5};
	debug(1, "Module1800::createScene(%d, %d)", sceneNum, which);
	_sceneNum = sceneNum;
	switch (_sceneNum) {
	case 0:
		_vm->gameState().sceneNum = 0;
		createNavigationScene(0x004AFD38, which, kNavigationTypes00);
		break;
	case 1:
		_vm->gameState().sceneNum = 1;
		createNavigationScene(0x004AFD98, which, kNavigationTypes01);
		break;
	case 2:
		_vm->gameState().sceneNum = 2;
		createSmackerScene(0x006C0085, true, true, false);
		break;
	case 3:
		_vm->gameState().sceneNum = 3;
		createNavigationScene(0x004AFDB0, which);
		break;
	case 4:
		_vm->gameState().sceneNum = 4;
		createNavigationScene(0x004AFDE0, which);
		break;
	case 5:
		_vm->gameState().sceneNum = 5;
		createNavigationScene(0x004AFE40, which);
		break;
	case 6:
		_vm->gameState().sceneNum = 6;
		_vm->_soundMan->deleteGroup(0x04A14718);
		createSmackerScene(0x08D84010, true, true, false);
		break;
	case 7:
		_vm->gameState().sceneNum = 7;
		_vm->_soundMan->setSoundListParams(kModule1800SoundList, false, 0, 0, 0, 0);
		createSmackerScene(0x0168B121, true, true, false);
		break;
	case 8:
		_vm->gameState().sceneNum = 8;
		_childObject = new CreditsScene(_vm, this, false);
		break;
	case 1009:
		_vm->gameState().sceneNum = 3;
		// NOTE: Newly introduced sceneNum 1009 (was duplicate 3 with own update handler)
		createSmackerScene(0x0A840C01, true, true, false);
		break;
	default:
		break;
	}
	SetUpdateHandler(&Module1800::updateScene);
	_childObject->handleUpdate();
}

void Module1800::updateScene() {
	if (!updateChild()) {
		switch (_sceneNum) {
		case 0:
			if (_moduleResult == 1)
				createScene(4, 0);
			else if (_moduleResult == 2)
				createScene(1, -1);
			else if (_moduleResult == 3)
				createScene(3, 0);
			break;
		case 1:
			if (_navigationAreaType == 3)
				createScene(7, -1);
			else
				createScene(2, -1);
			break;
		case 2:
			createScene(0, 2);
			break;
		case 3:
			if (_moduleResult == 0)
				createScene(1009, -1);
			else if (_moduleResult == 1)
				createScene(0, 1);
			break;
		case 4:
			if (_moduleResult == 0)
				createScene(6, -1);
			else if (_moduleResult == 1)
				createScene(5, 0);
			else if (_moduleResult == 2)
				createScene(0, 3);
			else if (_moduleResult == 3)
				createScene(4, 3);
			break;
		case 5:
			if (_moduleResult == 0)
				leaveModule(2);
			else if (_moduleResult == 1)
				createScene(4, 3);
			break;
		case 6:
			createScene(8, -1);
			break;
		case 7:
			leaveModule(3);
			break;
		case 8:
			// NOTE: After Klaymen jumped into the hole and died...
			leaveModule(1);
			break;
		case 1009:
			leaveModule(0);
			break;
		default:
			break;
		}
	} else {
		switch (_sceneNum) {
		case 0:
			if (navigationScene()->isWalkingForward() && navigationScene()->getNavigationIndex() == 2)
				_vm->_soundMan->setTwoSoundsPlayFlag(false);
			break;
		default:
			break;
		}
	}
}

} // End of namespace Neverhood
