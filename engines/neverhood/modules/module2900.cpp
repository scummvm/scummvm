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

#include "neverhood/modules/module2900.h"
#include "neverhood/modules/module2900_sprites.h"
#include "neverhood/modules/module1100.h"
#include "neverhood/modules/module1300.h"
#include "neverhood/modules/module1700.h"
#include "neverhood/modules/module2000.h"
#include "neverhood/modules/module2100.h"
#include "neverhood/modules/module2800.h"

namespace Neverhood {

Module2900::Module2900(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {

	_teleporterModuleResult = -1;

	if (which >= 0)
		setGlobalVar(V_TELEPORTER_WHICH, which);

	createScene(0, 0);
}

void Module2900::createScene(int sceneNum, int which) {
	debug(1, "Module2900::createScene(%d, %d)", sceneNum, which);
	_sceneNum = sceneNum;
	switch (_sceneNum) {
	case 0:
		_vm->gameState().sceneNum = 0;
		_childObject = new Scene2901(_vm, this, getGlobalVar(V_TELEPORTER_WHICH));
		break;
	case 1:
		_vm->gameState().sceneNum = 0;
		_childObject = new Scene2805(_vm, this, which);
		break;
	case 2:
		_vm->gameState().sceneNum = 0;
		_childObject = new Scene2101(_vm, this, which);
		break;
	case 3:
		_vm->gameState().sceneNum = 0;
		_childObject = new Scene1306(_vm, this, which);
		break;
	case 4:
		_vm->gameState().sceneNum = 0;
		_childObject = new Scene1705(_vm, this, which);
		break;
	case 5:
		_vm->gameState().sceneNum = 0;
		_childObject = new Scene1109(_vm, this, which);
		break;
	case 6:
		_vm->gameState().sceneNum = 0;
		_childObject = new Scene2001(_vm, this, which);
		break;
	default:
		break;
	}
	SetUpdateHandler(&Module2900::updateScene);
	_childObject->handleUpdate();
}

void Module2900::updateScene() {
	if (!updateChild()) {
		switch (_sceneNum) {
		case 0:
			if (_moduleResult == (uint32)-1) {
				leaveModule((uint32)-1);
			} else {
				_teleporterModuleResult = _moduleResult;
				switch (getGlobalVar(V_TELEPORTER_WHICH)) {
				case 0:
					createScene(3, 4);
					break;
				case 1:
					createScene(2, 2);
					break;
				case 2:
					createScene(5, 2);
					break;
				case 3:
					createScene(4, 2);
					break;
				case 4:
					createScene(6, 2);
					break;
				case 5:
					createScene(1, 2);
					break;
				default:
					leaveModule(_moduleResult);
					break;
				}
			}
			break;
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
			leaveModule(_teleporterModuleResult);
			break;
		default:
			break;
		}
	}
}

static const uint32 kScene2901FileHashes1[] = {
	0x023023B4,
	0x36204507,
	0x046CF08E,
	0x9313A237,
	0xA651F246,
	0x02108034
};

static const uint32 kScene2901FileHashes2[] = {
	0x023B002B,
	0x0450336A,
	0xCF08A04E,
	0x3A233939,
	0x1F242A6D,
	0x08030029
};

Scene2901::Scene2901(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule), _currLocationButtonNum(which), _selectedButtonNum(which),
	_currWhirlButtonNum(0), _prevWhirlButtonNum(0), _countdown1(1), _skipCountdown(0), _blinkOn(0) {

	_isButton2Broken = getGlobalVar(V_ENTRANCE_OPEN) != 0;

	setSubVar(V_TELEPORTER_DEST_AVAILABLE, which, 1);
	setSubVar(V_TELEPORTER_DEST_AVAILABLE, 5, 1);
	setSubVar(V_TELEPORTER_DEST_AVAILABLE, 4, 1);

	if (_currLocationButtonNum == 3)
		setSubVar(V_TELEPORTER_DEST_AVAILABLE, 2, 1);

	setBackground(kScene2901FileHashes1[_currLocationButtonNum]);
	setPalette(kScene2901FileHashes1[_currLocationButtonNum]);

	for (uint i = 0; i < 6; ++i) {
		if (i != 2 || !_isButton2Broken) {
			_ssLocationButtons[i] = insertSprite<SsScene2901LocationButton>(this, _currLocationButtonNum, i);
			addCollisionSprite(_ssLocationButtons[i]);
			_ssLocationButtonLights[i] = insertSprite<SsScene2901LocationButtonLight>(_currLocationButtonNum, i);
		}
	}

	if (_isButton2Broken)
		insertSprite<SsScene2901BrokenButton>(_currLocationButtonNum);

	_ssBigButton = insertSprite<SsScene2901BigButton>(this, _currLocationButtonNum);
	addCollisionSprite(_ssBigButton);

	insertPuzzleMouse(kScene2901FileHashes2[_currLocationButtonNum], 20, 620);

	SetUpdateHandler(&Scene2901::update);
	SetMessageHandler(&Scene2901::handleMessage);

}

void Scene2901::update() {
	Scene::update();
	if (_countdown1 != 0 && (--_countdown1) == 0) {
		if (_currLocationButtonNum == _selectedButtonNum) {
			_ssLocationButtonLights[_currWhirlButtonNum]->hide();
			++_currWhirlButtonNum;
			while (!getSubVar(V_TELEPORTER_DEST_AVAILABLE, _currWhirlButtonNum) || (_currWhirlButtonNum == 2 && _isButton2Broken) || _currLocationButtonNum == _currWhirlButtonNum) {
				++_currWhirlButtonNum;
				if (_currWhirlButtonNum >= 6)
					_currWhirlButtonNum = 0;
			}
			if (_currWhirlButtonNum != _prevWhirlButtonNum || _skipCountdown == 0) {
				_ssLocationButtonLights[_currWhirlButtonNum]->show();
				_skipCountdown = 4;
			}
			_countdown1 = 2;
			--_skipCountdown;
			_prevWhirlButtonNum = _currWhirlButtonNum;
		} else if (_blinkOn) {
			_blinkOn = false;
			_ssLocationButtonLights[_selectedButtonNum]->hide();
			_countdown1 = 16;
		} else {
			_blinkOn = true;
			_ssLocationButtonLights[_selectedButtonNum]->show();
			_countdown1 = 4;
		}
	}
}

uint32 Scene2901::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_MOUSE_CLICK:
		if (param.asPoint().x <= 20 || param.asPoint().x >= 620)
			leaveScene((uint32)-1);
		break;
	case NM_ANIMATION_UPDATE:
		if (_currLocationButtonNum != _selectedButtonNum)
			leaveScene(_selectedButtonNum);
		break;
	case 0x2001:
		if (_currLocationButtonNum == _selectedButtonNum)
			_selectedButtonNum = _currWhirlButtonNum;
		_ssLocationButtonLights[_selectedButtonNum]->hide();
		_selectedButtonNum = param.asInteger();
		if (!getSubVar(V_TELEPORTER_DEST_AVAILABLE, _selectedButtonNum))
			_selectedButtonNum = _currLocationButtonNum;
		break;
	default:
		break;
	}
	return 0;
}

} // End of namespace Neverhood
