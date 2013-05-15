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

#include "neverhood/modules/module2900.h"
#include "neverhood/gamemodule.h"
#include "neverhood/modules/module1100.h"
#include "neverhood/modules/module1300.h"
#include "neverhood/modules/module1700.h"
#include "neverhood/modules/module2000.h"
#include "neverhood/modules/module2100.h"
#include "neverhood/modules/module2800.h"

namespace Neverhood {

Module2900::Module2900(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {

	if (which >= 0)
		setGlobalVar(V_TELEPORTER_WHICH, which);
		
	createScene(0, 0);

}

void Module2900::createScene(int sceneNum, int which) {
	debug("Module2900::createScene(%d, %d)", sceneNum, which);
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

static const uint32 kSsScene2901LocationButtonFileHashes[] = {
	0x2311326A,
	0x212323AC,
	0x10098138,
	0x25213167,
	0x1119A363,
	0x94452612,
	0x39464212,
	0x01860450,
	0x53002104,
	0x58E68412,
	0x18600300,
	0xB650A890,
	0x2452A7C4,
	0xA0232748,
	0x08862B02,
	0x2491E648,
	0x0010EB46,
	0x214C8A11,
	0x16A31921,
	0x0AC33A00,
	0x238028AA,
	0x26737A21,
	0x063039A8,
	0x51286C60,
	0x464006B4,
	0x42242538,
	0x20716010,
	0x4A2000AE,
	0x225124A6,
	0x28E82E45,
	0x58652C04,
	0xC82210A4,
	0x62A84060,
	0xC0693CB4,
	0x22212C64,
	0x5034EA71
};

static const NPoint kSsScene2901LocationButtonPoints[] = {
	{525, 120}, {576, 149}, {587, 205},
	{538, 232}, {484, 205}, {479, 153}
};

static const uint32 kSsScene2901LocationButtonLightFileHashes1[] = {
	0x03136246,
	0x2106216E,
	0x4025A13A,
	0x21816927,
	0x110B2202,
	0xCC0522B2,
	0x3CC24258,
	0x59C600F0,
	0x534A2480,
	0x50E61019,
	0x34400150,
	0x225BA090,
	0xB059AFC4,
	0xE093A741,
	0x0086BF09,
	0x3281E760,
	0xA048AB42,
	0x20649C01,
	0x14611904,
	0x26E33850,
	0x23A52A68,
	0xA2733024,
	0x10203880,
	0x1B2DE860,
	0x0644A6EC,
	0x426E20BC,
	0x80292014,
	0x4360B02E,
	0x22742664,
	0x98682705,
	0x0925B82C,
	0x5C2918A4,
	0xD2284920,
	0x41083CA6,
	0x6824A864,
	0x50266B10
};

static const uint32 kSsScene2901LocationButtonLightFileHashes2[] = {
	0x43C46D4C,
	0x43C4AD4C,
	0x43C52D4C,
	0x43C62D4C,
	0x43C02D4C,
	0x43CC2D4C
};

static const uint32 kSsScene2901BrokenButtonFileHashes[] = {
	0x3081BD3A,
	0xD3443003,
	0x0786A320,
	0xE3A22029,
	0x61611814,
	0x425848E2
};

static const uint32 kSsScene2901BigButtonFileHashes[] = {
	0x010D7748,
	0x9D02019A,
	0x351A2F43,
	0x448138E5,
	0x02788CF0,
	0x71718024
};

SsScene2901LocationButton::SsScene2901LocationButton(NeverhoodEngine *vm, Scene *parentScene, int which, uint index)
	: StaticSprite(vm, 900), _parentScene(parentScene), _index(index), _countdown1(0) {

	const NPoint &pt = kSsScene2901LocationButtonPoints[_index];
	
	loadSprite(kSsScene2901LocationButtonFileHashes[which * 6 + index], kSLFDefDrawOffset | kSLFDefPosition, 800);
	_collisionBounds.set(pt.x - 25, pt.y - 25, pt.x + 25, pt.y + 25);
	setVisible(false);
	loadSound(0, 0x440430C0);
	SetUpdateHandler(&SsScene2901LocationButton::update);
	SetMessageHandler(&SsScene2901LocationButton::handleMessage);
}
	
void SsScene2901LocationButton::update() {
	updatePosition();
	if (_countdown1 != 0 && (--_countdown1) == 0) {
		setVisible(false);
	}
}

uint32 SsScene2901LocationButton::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_countdown1 == 0) {
			playSound(0);
			setVisible(true);
			_countdown1 = 4;
			sendMessage(_parentScene, 0x2001, _index);
		}
		messageResult = 1;
		break;
	}
	return messageResult;
}

SsScene2901LocationButtonLight::SsScene2901LocationButtonLight(NeverhoodEngine *vm, int which, uint index)
	: StaticSprite(vm, 900), _index(index) {
	
	loadSprite(kSsScene2901LocationButtonLightFileHashes1[which * 6 + index], kSLFDefDrawOffset | kSLFDefPosition, 900);
	setVisible(false);
	loadSound(0, kSsScene2901LocationButtonLightFileHashes2[_index]);
}

void SsScene2901LocationButtonLight::show() {
	playSound(0);
	setVisible(true);
	updatePosition();
}

void SsScene2901LocationButtonLight::hide() {
	setVisible(false);
	updatePosition();
}

SsScene2901BrokenButton::SsScene2901BrokenButton(NeverhoodEngine *vm, int which)
	: StaticSprite(vm, 900) {

	loadSprite(kSsScene2901BrokenButtonFileHashes[which], kSLFDefDrawOffset | kSLFDefPosition, 900);
}

SsScene2901BigButton::SsScene2901BigButton(NeverhoodEngine *vm, Scene *parentScene, int which)
	: StaticSprite(vm, 900), _parentScene(parentScene), _which(which), _countdown1(0) {

	loadSprite(kSsScene2901BigButtonFileHashes[which], kSLFDefDrawOffset | kSLFDefPosition, 400);	
	_collisionBounds.set(62, 94, 322, 350);
	setVisible(false);
	loadSound(0, 0xF3D420C8);
	SetUpdateHandler(&SsScene2901BigButton::update);
	SetMessageHandler(&SsScene2901BigButton::handleMessage);
}

void SsScene2901BigButton::update() {
	updatePosition();
	if (_countdown1 != 0 && (--_countdown1) == 0) {
		setVisible(false);
		sendMessage(_parentScene, 0x2000, 0);
	}
}
	
uint32 SsScene2901BigButton::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_countdown1 == 0) {
			playSound(0);
			setVisible(true);
			_countdown1 = 4;
		}
		messageResult = 1;
		break;
	}
	return messageResult;
}

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
	case 0x0001:
		if (param.asPoint().x <= 20 || param.asPoint().x >= 620)
			leaveScene((uint32)-1);
		break;
	case 0x2000:
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
	}
	return 0;
}

} // End of namespace Neverhood
