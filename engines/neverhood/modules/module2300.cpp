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
#include "neverhood/modules/module2300.h"

namespace Neverhood {

static const uint32 kModule2300SoundList[] = {
	0x90805C50, 0x90804450, 0xB4005E60, 0x91835066,
	0x90E14440, 0x90F0D1C3, 0
};

Module2300::Module2300(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule), _waterfallSoundVolume(0) {

	_vm->_soundMan->addSoundList(0x1A214010, kModule2300SoundList);
	_vm->_soundMan->setSoundListParams(kModule2300SoundList, true, 50, 600, 10, 150);

	_isWaterfallRunning = getGlobalVar(V_WALL_BROKEN) != 1;

	if (_isWaterfallRunning) {
		_vm->_soundMan->setSoundVolume(0x90F0D1C3, 0);
		_vm->_soundMan->playSoundLooping(0x90F0D1C3);
	} else {
		_vm->_soundMan->setSoundParams(0x90F0D1C3, false, 0, 0, 0, 0);
	}

	_vm->_soundMan->playTwoSounds(0x1A214010, 0x48498E46, 0x50399F64, 0);
	_vm->_soundMan->playTwoSounds(0x1A214010, 0x41861371, 0x43A2507F, 0);

	if (which < 0)
		createScene(_vm->gameState().sceneNum, -1);
	else if (which == 1)
		createScene(2, 0);
	else if (which == 2)
		createScene(3, 0);
	else if (which == 3)
		createScene(4, -1);
	else if (which == 4)
		createScene(1, 3);
	else
		createScene(0, 1);

}

Module2300::~Module2300() {
	_vm->_soundMan->deleteGroup(0x1A214010);
}

void Module2300::createScene(int sceneNum, int which) {
	debug(1, "Module2300::createScene(%d, %d)", sceneNum, which);
	_sceneNum = sceneNum;
	switch (_sceneNum) {
	case 0:
		_vm->gameState().sceneNum = 0;
		createNavigationScene(0x004B67B8, which);
		break;
	case 1:
		_vm->gameState().sceneNum = 1;
		createNavigationScene(0x004B67E8, which);
		if (_isWaterfallRunning) {
			_waterfallSoundVolume = 15;
			_vm->_soundMan->setSoundVolume(0x90F0D1C3, 15);
		}
		break;
	case 2:
		_vm->gameState().sceneNum = 2;
		createNavigationScene(0x004B6878, which);
		break;
	case 3:
		_vm->gameState().sceneNum = 3;
		if (getGlobalVar(V_WALL_BROKEN))
			createNavigationScene(0x004B68F0, which);
		else {
			_vm->_soundMan->setSoundVolume(0x90F0D1C3, _waterfallSoundVolume);
			createNavigationScene(0x004B68A8, which);
			if (_isWaterfallRunning) {
				_waterfallSoundVolume = 87;
				_vm->_soundMan->setSoundVolume(0x90F0D1C3, 87);
			}
		}
		break;
	case 4:
		_vm->gameState().sceneNum = 4;
		_vm->_soundMan->setTwoSoundsPlayFlag(true);
		createSmackerScene(0x20080A0B, true, true, false);
		break;
	case 9999:
		createDemoScene();
		break;
	default:
		break;
	}
	SetUpdateHandler(&Module2300::updateScene);
	_childObject->handleUpdate();
}

void Module2300::updateScene() {
	if (!updateChild()) {
		switch (_sceneNum) {
		case 0:
			if (_moduleResult == 1)
				createScene(1, 4);
			else
				leaveModule(0);
			break;
		case 1:
			if (_moduleResult == 1)
				createScene(0, 0);
			else if (_vm->isDemo() && !(_vm->isBigDemo() && _moduleResult == 4))
				createScene(9999, 0);
			else if (_moduleResult == 2)
				createScene(2, 1);
			else if (_moduleResult == 3)
				createScene(1, 3);
			else if (_moduleResult == 4)
				createScene(3, 1);
			else if (_moduleResult == 5)
				leaveModule(3);
			else
				leaveModule(4);
			break;
		case 2:
			if (_moduleResult == 1)
				leaveModule(1);
			else
				createScene(1, 5);
			break;
		case 3:
			if (_moduleResult == 1)
				leaveModule(2);
			else
				createScene(1, 1);
			break;
		case 4:
			_vm->_soundMan->setTwoSoundsPlayFlag(false);
			createScene(1, 2);
			break;
		case 9999:
			createScene(1, -1);
			break;
		default:
			break;
		}
	} else {
		switch (_sceneNum) {
		case 1:
			if (_isWaterfallRunning && navigationScene()->isWalkingForward() && navigationScene()->getNavigationIndex() == 4 &&
				navigationScene()->getFrameNumber() % 2) {
				_waterfallSoundVolume++;
				_vm->_soundMan->setSoundVolume(0x90F0D1C3, _waterfallSoundVolume);
			}
			if (navigationScene()->isWalkingForward() && navigationScene()->getNavigationIndex() == 0 &&
				navigationScene()->getFrameNumber() == 50) {
				_vm->_soundMan->playTwoSounds(0x1A214010, 0x48498E46, 0x50399F64, 0);
				_vm->_soundMan->setSoundVolume(0x48498E46, 70);
				_vm->_soundMan->setSoundVolume(0x50399F64, 70);
			}
			break;
		case 3:
			if (_isWaterfallRunning && navigationScene()->isWalkingForward() && navigationScene()->getFrameNumber() % 2) {
				_waterfallSoundVolume--;
				_vm->_soundMan->setSoundVolume(0x90F0D1C3, _waterfallSoundVolume);
			}
			break;
		default:
			break;
		}
	}
}

} // End of namespace Neverhood
