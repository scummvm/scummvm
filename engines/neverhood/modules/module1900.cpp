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

#include "neverhood/modules/module1900.h"
#include "neverhood/modules/module1900_sprites.h"

namespace Neverhood {

static const uint32 kModule1900SoundList[] = {
	0xB4005E60,
	0x91835066,
	0x90E14440,
	0
};

Module1900::Module1900(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {

	// NOTE: The original has a Scene1908 here as well but it's not used here but in another module...

	if (which < 0)
		createScene(_vm->gameState().sceneNum, -1);
	else
		createScene(0, 0);

	_vm->_soundMan->addSoundList(0x04E1C09C, kModule1900SoundList);
	_vm->_soundMan->setSoundListParams(kModule1900SoundList, true, 50, 600, 5, 150);

}

Module1900::~Module1900() {
	_vm->_soundMan->deleteGroup(0x04E1C09C);
}

void Module1900::createScene(int sceneNum, int which) {
	debug(1, "Module1900::createScene(%d, %d)", sceneNum, which);
	_sceneNum = sceneNum;
	switch (_sceneNum) {
	case 0:
		_vm->gameState().sceneNum = 0;
		_childObject = new Scene1901(_vm, this, which);
		break;
	case 6:
		_vm->gameState().sceneNum = 6;
		_childObject = new Scene1907(_vm, this);
		break;
	default:
		break;
	}
	SetUpdateHandler(&Module1900::updateScene);
	_childObject->handleUpdate();
}

void Module1900::updateScene() {
	if (!updateChild()) {
		switch (_sceneNum) {
		case 0:
			if (_moduleResult == 1)
				createScene(6, 0);
			else
				leaveModule(0);
			break;
		case 6:
			createScene(0, 1);
			break;
		default:
			break;
		}
	}
}

Scene1901::Scene1901(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule) {

	Sprite *tempSprite;

	setRectList(0x004B34C8);

	setBackground(0x01303227);
	setPalette(0x01303227);
	insertScreenMouse(0x0322301B);

	insertStaticSprite(0x42213133, 1100);

	if (!getGlobalVar(V_STAIRS_PUZZLE_SOLVED))
		insertStaticSprite(0x40A40168, 100);
	else if (getGlobalVar(V_STAIRS_DOWN)) {
		insertStaticSprite(0x124404C4, 100);
		setGlobalVar(V_STAIRS_DOWN_ONCE, 1);
	} else
		insertStaticSprite(0x02840064, 100);

	if (which < 0) {
		// Restoring game
		insertKlaymen<KmScene1901>(120, 380);
		setMessageList(0x004B3408);
	} else if (which == 1) {
		// Klaymen returning from the puzzle
		insertKlaymen<KmScene1901>(372, 380);
		setMessageList(0x004B3410);
	} else {
		// Klaymen entering from the left
		insertKlaymen<KmScene1901>(0, 380);
		setMessageList(0x004B3400);
	}

	tempSprite = insertStaticSprite(0x4830A402, 1100);
	_klaymen->setClipRect(tempSprite->getDrawRect().x, 0, 640, 480);

}

Scene1907::Scene1907(NeverhoodEngine *vm, Module *parentModule)
	: Scene(vm, parentModule), _currMovingSymbolIndex(0), _pluggedInCount(0),
	_moveDownCountdown(0), _moveUpCountdown(0), _countdown3(0), _hasPlugInFailed(false) {

	setBackground(0x20628E05);
	setPalette(0x20628E05);

	for (int i = 0; i < 9; i++)
		_positionFree[i] = true;

	for (int i = 0; i < 9; i++) {
		_asSymbols[i] = insertSprite<AsScene1907Symbol>(this, i, getRandomPositionIndex());
		addCollisionSprite(_asSymbols[i]);
	}

	_ssUpDownButton = insertSprite<SsScene1907UpDownButton>(this, _asSymbols[8]);
	addCollisionSprite(_ssUpDownButton);

	_asWaterHint = insertSprite<AsScene1907WaterHint>();

	insertPuzzleMouse(0x28E0120E, 20, 620);

	SetMessageHandler(&Scene1907::handleMessage);
	SetUpdateHandler(&Scene1907::update);

	if (getGlobalVar(V_STAIRS_PUZZLE_SOLVED))
		_pluggedInCount = 9;

	loadSound(0, 0x72004A10);
	loadSound(1, 0x22082A12);
	loadSound(2, 0x21100A10);
	loadSound(3, 0x68E25540);

}

void Scene1907::update() {
	Scene::update();

	if (_hasPlugInFailed) {
		int fallOffDelay = 0;
		_hasPlugInFailed = false;
		for (int i = 0; i < 9; i++) {
			AsScene1907Symbol *asSymbol = _asSymbols[8 - i];
			if (asSymbol->isPluggedIn()) {
				asSymbol->fallOff(getRandomPositionIndex(), fallOffDelay);
				fallOffDelay += _vm->_rnd->getRandomNumber(10 - 1) + 4;
			}
		}
	}

	if (_moveDownCountdown != 0 && (--_moveDownCountdown == 0)) {
		_asSymbols[_currMovingSymbolIndex]->moveDown();
		if (_currMovingSymbolIndex > 0) {
			_moveDownCountdown = 2;
			_currMovingSymbolIndex--;
		}
	}

	if (_moveUpCountdown != 0 && (--_moveUpCountdown == 0)) {
		_moveDownCountdown = 0;
		for (int i = 0; i < 9; i++)
			_asSymbols[i]->moveUp();
	}

	if (_countdown3 != 0 && (--_countdown3 == 0)) {
		_asWaterHint->show();
		_moveUpCountdown = 4;
	}

}

uint32 Scene1907::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_MOUSE_CLICK:
		if ((param.asPoint().x <= 20 || param.asPoint().x >= 620) &&
			!_hasPlugInFailed && _moveDownCountdown == 0 && _moveUpCountdown == 0 && _countdown3 == 0) {
			leaveScene(0);
		}
		break;
	case NM_ANIMATION_UPDATE:
		if (getGlobalVar(V_STAIRS_DOWN)) {
			playSound(0);
			for (int i = 0; i < 9; i++)
				_asSymbols[i]->moveUp();
			_ssUpDownButton->setToUpPosition();
			setGlobalVar(V_STAIRS_DOWN, 0);
		} else {
			if (!getGlobalVar(V_WALL_BROKEN)) {
				playSound(2);
				_countdown3 = 5;
			} else {
				playSound(1);
				_ssUpDownButton->setToDownPosition();
				setGlobalVar(V_STAIRS_DOWN, 1);
			}
			_moveDownCountdown = 1;
			_currMovingSymbolIndex = 8;
		}
		break;
	case 0x2001:
		playSound(3);
		setGlobalVar(V_STAIRS_PUZZLE_SOLVED, 1);
		break;
	default:
		break;
	}
	return 0;
}

void Scene1907::plugInFailed() {
	_pluggedInCount = 0;
	_hasPlugInFailed = true;
}

int Scene1907::getRandomPositionIndex() {
	bool found = false;
	int index = 0;
	// Check if any position is free
	for (int i = 0; i < 9; i++)
		if (_positionFree[i])
			found = true;
	if (found) {
		// Get a random free position
		found = false;
		while (!found) {
			index = _vm->_rnd->getRandomNumber(9 - 1);
			if (_positionFree[index])
				found = true;
		}
	}
	return index;
}

} // End of namespace Neverhood
