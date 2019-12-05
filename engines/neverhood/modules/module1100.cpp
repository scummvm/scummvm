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

#include "neverhood/gamemodule.h"
#include "neverhood/navigationscene.h"
#include "neverhood/modules/module1100.h"
#include "neverhood/modules/module1100_sprites.h"

namespace Neverhood {

static const uint32 kModule1100SoundList[] = {
	0x90805C50,
	0xB288D450,
	0x98C05840,
	0x98A01500,
	0xB4005E50,
	0x92025040,
	0x90035066,
	0x74E01054,
	0
};

Module1100::Module1100(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {

	if (which < 0) {
		createScene(_vm->gameState().sceneNum, -1);
	} else if (which == 1) {
		createScene(8, 1);
	} else {
		createScene(8, 3);
	}

	_vm->_soundMan->addSoundList(0x0002C818, kModule1100SoundList);
	_vm->_soundMan->setSoundListParams(kModule1100SoundList, true, 50, 600, 20, 250);
	_vm->_soundMan->setSoundParams(0x74E01054, false, 100, 200, 10, 20);
	_vm->_soundMan->setSoundVolume(0x74E01054, 60);
	_vm->_soundMan->playTwoSounds(0x0002C818, 0x41861371, 0x43A2507F, 0);

}

Module1100::~Module1100() {
	_vm->_soundMan->deleteGroup(0x0002C818);
}

void Module1100::createScene(int sceneNum, int which) {
	static const uint32 kSmackerFileHashList06[] = {0x10880805, 0x1088081D, 0};
	static const uint32 kSmackerFileHashList07[] = {0x00290321, 0x01881000, 0};
	static const byte kNavigationTypes02[] = {1, 0, 4, 1};
	debug(1, "Module1100::createScene(%d, %d)", sceneNum, which);
	_sceneNum = sceneNum;
	switch (_sceneNum) {
	case 0:
		_vm->gameState().sceneNum = 0;
		_countdown = 65;
		createNavigationScene(0x004B8430, which);
		break;
	case 1:
		_vm->gameState().sceneNum = 1;
		_countdown = 50;
		createNavigationScene(0x004B8460, which);
		break;
	case 2:
		_vm->gameState().sceneNum = 2;
		if (getGlobalVar(V_ROBOT_TARGET)) {
			createNavigationScene(0x004B84F0, which, kNavigationTypes02);
		} else {
			createNavigationScene(0x004B8490, which, kNavigationTypes02);
		}
		break;
	case 3:
		_vm->gameState().sceneNum = 3;
		if (getGlobalVar(V_ROBOT_TARGET)) {
			createNavigationScene(0x004B8580, which);
		} else {
			createNavigationScene(0x004B8550, which);
		}
		break;
	case 4:
		_vm->gameState().sceneNum = 4;
		_childObject = new Scene1105(_vm, this);
		break;
	case 5:
		_vm->gameState().sceneNum = 5;
		if (getGlobalVar(V_ROBOT_TARGET))
			createSmackerScene(0x04180001, true, false, false);
		else
			createSmackerScene(0x04180007, true, false, false);
		break;
	case 6:
		_vm->gameState().sceneNum = 6;
		_vm->_soundMan->deleteSoundGroup(0x0002C818);
		createSmackerScene(kSmackerFileHashList06, true, true, false);
		break;
	case 7:
		_vm->gameState().sceneNum = 7;
		_vm->_soundMan->setSoundParams(0x74E01054, false, 0, 0, 0, 0);
		createSmackerScene(kSmackerFileHashList07, true, true, false);
		break;
	case 8:
		_vm->gameState().sceneNum = 8;
		_childObject = new Scene1109(_vm, this, which);
		break;
	case 1002:
		_vm->gameState().sceneNum = 2;
		_countdown = 40;
		_vm->_soundMan->setTwoSoundsPlayFlag(true);
		createSmackerScene(0x00012211, true, true, false);
		break;
	default:
		break;
	}
	SetUpdateHandler(&Module1100::updateScene);
	_childObject->handleUpdate();
}

void Module1100::updateScene() {
	if (!updateChild()) {
		switch (_sceneNum) {
		case 0:
			_countdown = 0;
			_vm->_soundMan->setSoundVolume(0x48498E46, 65);
			_vm->_soundMan->setSoundVolume(0x50399F64, 65);
			if (_moduleResult == 0) {
				_vm->_soundMan->playTwoSounds(0x0002C818, 0x48498E46, 0x50399F64, 0);
				createScene(1, 0);
			} else if (_moduleResult == 1) {
				/* NOTE This fixes a bug in the original where the "tunnel" footstep
					sounds are played instead of the correct footsteps. */
				_vm->_soundMan->playTwoSounds(0x0002C818, 0x41861371, 0x43A2507F, 0);
				createScene(8, 0);
			}
			break;
		case 1:
			_countdown = 0;
			_vm->_soundMan->playTwoSounds(0x0002C818, 0x41861371, 0x43A2507F, 0);
			if (getGlobalVar(V_ROBOT_HIT)) {
				if (_moduleResult == 0)
					createScene(6, -1);
				else if (_moduleResult == 1)
					createScene(0, 1);
			} else {
				if (_moduleResult == 0)
					createScene(2, 0);
				else if (_moduleResult == 1)
					createScene(0, 1);
			}
			break;
		case 2:
			_vm->_soundMan->setSoundParams(0x74E01054, false, 0, 0, 0, 0);
			if (_navigationAreaType == 3)
				createScene(7, -1);
			else if (_moduleResult == 1)
				createScene(3, 0);
			else if (_moduleResult == 2)
				createScene(1002, -1);
			break;
		case 3:
			if (_moduleResult == 0)
				createScene(4, 0);
			else if (_moduleResult == 1)
				createScene(2, 3);
			break;
		case 4:
			if (_moduleResult == 0)
				createScene(3, 0);
			else if (_moduleResult == 1)
				createScene(5, -1);
			break;
		case 5:
			_vm->_soundMan->setTwoSoundsPlayFlag(false);
			if (getGlobalVar(V_ROBOT_TARGET))
				createScene(3, 0);
			else
				createScene(4, 0);
			break;
		case 6:
			_vm->_soundMan->setTwoSoundsPlayFlag(false);
			leaveModule(1);
			break;
		case 7:
			_vm->_soundMan->setTwoSoundsPlayFlag(false);
			createScene(2, 2);
			break;
		case 8:
			if (_moduleResult == 0)
				createScene(0, 0);
			else if (_moduleResult == 1)
				leaveModule(0);
			break;
		case 1002:
			_vm->_soundMan->setTwoSoundsPlayFlag(false);
			_countdown = 0;
			_vm->_soundMan->playTwoSounds(0x0002C818, 0x48498E46, 0x50399F64, 0);
			createScene(1, 1);
			break;
		default:
			break;
		}
	} else {
		switch (_sceneNum) {
		case 0:
			if (navigationScene()->isWalkingForward() && _countdown != 0 && (--_countdown == 0)) {
				_vm->_soundMan->playTwoSounds(0x0002C818, 0x48498E46, 0x50399F64, 0);
				_vm->_soundMan->setSoundVolume(0x48498E46, 65);
				_vm->_soundMan->setSoundVolume(0x50399F64, 65);
			}
			break;
		case 1:
			if (navigationScene()->isWalkingForward() && _countdown != 0 && (--_countdown == 0))
				_vm->_soundMan->playTwoSounds(0x0002C818, 0x41861371, 0x43A2507F, 0);
			break;
		case 2:
			_vm->_soundMan->setSoundParams(0x74E01054, !navigationScene()->isWalkingForward(), 0, 0, 0, 0);
			break;
		case 5:
		case 6:
		case 7:
		case 1002:
			if (_countdown != 0 && (--_countdown == 0)) {
				_vm->_soundMan->playTwoSounds(0x0002C818, 0x48498E46, 0x50399F64, 0);
				_vm->_soundMan->setSoundVolume(0x48498E46, 65);
				_vm->_soundMan->setSoundVolume(0x50399F64, 65);
			}
			break;
		default:
			break;
		}
	}
}

static const uint32 kScene1105BackgroundFileHashes[] = {
	0x20018662,
	0x20014202,
	0x20012202,
	0x20010002 // CHECKME: This used ??
};

static const uint32 kScene1105FileHashes[] = {
	0x00028006,
	0x0100A425,
	0x63090415,
	0x082100C4,
	0x0068C607,
	0x00018344,
	0x442090E4,
	0x0400E004,
	0x5020A054,
	0xB14A891E
};

Scene1105::Scene1105(NeverhoodEngine *vm, Module *parentModule)
	: Scene(vm, parentModule), _countdown(0), _isPanelOpen(false), _isActionButtonClicked(false), _doMoveTeddy(false),
	_isClosePanelDone(false), _leaveResult(0), _backgroundIndex(0) {

	Sprite *ssOpenButton;

	_vm->gameModule()->initMemoryPuzzle();

	SetUpdateHandler(&Scene1105::update);
	SetMessageHandler(&Scene1105::handleMessage);

	setBackground(0x20010002);
	setPalette(0x20010002);

	_asTeddyBear = insertSprite<AsScene1105TeddyBear>(this);
	ssOpenButton = insertSprite<SsScene1105OpenButton>(this);
	addCollisionSprite(ssOpenButton);
	insertPuzzleMouse(0x10006208, 20, 620);

	loadSound(0, 0x48442057);
	loadSound(1, 0xC025014F);
	loadSound(2, 0x68E25540);

}

uint32 Scene1105::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = 0;
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_MOUSE_CLICK:
		if (param.asPoint().x <= 20 || param.asPoint().x >= 620) {
			if (!_isActionButtonClicked && _backgroundIndex == 0) {
				if (_isPanelOpen) {
					_isPanelOpen = false;
					_backgroundIndex = 15;
					SetUpdateHandler(&Scene1105::upClosePanel);
				} else
					_isClosePanelDone = true;
				_leaveResult = 0;
			}
		}
		break;
	case 0x2001:
		showMouse(false);
		_backgroundIndex = 24;
		SetUpdateHandler(&Scene1105::upOpenPanel);
		break;
	case 0x2003:
		_backgroundIndex = 24;
		_leaveResult = 1;
		SetUpdateHandler(&Scene1105::upClosePanel);
		break;
	case NM_KLAYMEN_RAISE_LEVER:
		if (sender == _ssActionButton) {
			if (getSubVar(VA_GOOD_DICE_NUMBERS, 0) == getSubVar(VA_CURR_DICE_NUMBERS, 0) &&
				getSubVar(VA_GOOD_DICE_NUMBERS, 1) == getSubVar(VA_CURR_DICE_NUMBERS, 1) &&
				getSubVar(VA_GOOD_DICE_NUMBERS, 2) == getSubVar(VA_CURR_DICE_NUMBERS, 2)) {
				setGlobalVar(V_ROBOT_TARGET, 1);
				playSound(2);
				_doMoveTeddy = true;
			} else {
				sendMessage(_asTeddyBear, NM_POSITION_CHANGE, 0);
			}
			showMouse(false);
			_isActionButtonClicked = true;
		}
		break;
	case 0x4826:
		if (_isPanelOpen) {
			if (sender == _ssActionButton) {
				sendMessage(_ssActionButton, 0x480B, 0);
				_isPanelOpen = false;
			} else if (!getGlobalVar(V_ROBOT_TARGET)) {
				if (sender == _ssSymbol1UpButton) {
					if (getSubVar(VA_CURR_DICE_NUMBERS, 0) < 9) {
						incSubVar(VA_CURR_DICE_NUMBERS, 0, +1);
						sendMessage(_ssSymbol1UpButton, 0x480B, 0);
						sendMessage(_ssSymbolDice[0], 0x2000, 0);
					}
				} else if (sender == _ssSymbol1DownButton) {
					if (getSubVar(VA_CURR_DICE_NUMBERS, 0) > 1) {
						incSubVar(VA_CURR_DICE_NUMBERS, 0, -1);
						sendMessage(_ssSymbol1DownButton, 0x480B, 0);
						sendMessage(_ssSymbolDice[0], 0x2000, 0);
					}
				} else if (sender == _ssSymbol2UpButton) {
					if (getSubVar(VA_CURR_DICE_NUMBERS, 1) < 9) {
						incSubVar(VA_CURR_DICE_NUMBERS, 1, +1);
						sendMessage(_ssSymbol2UpButton, 0x480B, 0);
						sendMessage(_ssSymbolDice[1], 0x2000, 0);
					}
				} else if (sender == _ssSymbol2DownButton) {
					if (getSubVar(VA_CURR_DICE_NUMBERS, 1) > 1) {
						incSubVar(VA_CURR_DICE_NUMBERS, 1, -1);
						sendMessage(_ssSymbol2DownButton, 0x480B, 0);
						sendMessage(_ssSymbolDice[1], 0x2000, 0);
					}
				} else if (sender == _ssSymbol3UpButton) {
					if (getSubVar(VA_CURR_DICE_NUMBERS, 2) < 9) {
						incSubVar(VA_CURR_DICE_NUMBERS, 2, +1);
						sendMessage(_ssSymbol3UpButton, 0x480B, 0);
						sendMessage(_ssSymbolDice[2], 0x2000, 0);
					}
				} else if (sender == _ssSymbol3DownButton) {
					if (getSubVar(VA_CURR_DICE_NUMBERS, 2) > 1) {
						incSubVar(VA_CURR_DICE_NUMBERS, 2, -1);
						sendMessage(_ssSymbol3DownButton, 0x480B, 0);
						sendMessage(_ssSymbolDice[2], 0x2000, 0);
					}
				}
			}
		}
		break;
	default:
		break;
	}
	return messageResult;
}

void Scene1105::createObjects() {
	_ssSymbols[0] = insertSprite<SsScene1105Symbol>(kScene1105FileHashes[getSubVar(VA_DICE_MEMORY_SYMBOLS, 0)], 161, 304);
	_ssSymbols[1] = insertSprite<SsScene1105Symbol>(kScene1105FileHashes[getSubVar(VA_DICE_MEMORY_SYMBOLS, 1)], 294, 304);
	_ssSymbols[2] = insertSprite<SsScene1105Symbol>(kScene1105FileHashes[getSubVar(VA_DICE_MEMORY_SYMBOLS, 2)], 440, 304);

	_ssSymbolDice[0] = insertSprite<SsScene1105SymbolDie>(0, 206, 304);
	_ssSymbolDice[1] = insertSprite<SsScene1105SymbolDie>(1, 339, 304);
	_ssSymbolDice[2] = insertSprite<SsScene1105SymbolDie>(2, 485, 304);

	_ssSymbol1UpButton = insertSprite<SsScene1105Button>(this, 0x08002860, NRect::make(146, 362, 192, 403));
	addCollisionSprite(_ssSymbol1UpButton);
	_ssSymbol1DownButton = insertSprite<SsScene1105Button>(this, 0x42012460, NRect::make(147, 404, 191, 442));
	addCollisionSprite(_ssSymbol1DownButton);
	_ssSymbol2UpButton = insertSprite<SsScene1105Button>(this, 0x100030A0, NRect::make(308, 361, 355, 402));
	addCollisionSprite(_ssSymbol2UpButton);
	_ssSymbol2DownButton = insertSprite<SsScene1105Button>(this, 0x840228A0, NRect::make(306, 406, 352, 445));
	addCollisionSprite(_ssSymbol2DownButton);
	_ssSymbol3UpButton = insertSprite<SsScene1105Button>(this, 0x20000120, NRect::make(476, 358, 509, 394));
	addCollisionSprite(_ssSymbol3UpButton);
	_ssSymbol3DownButton = insertSprite<SsScene1105Button>(this, 0x08043121, NRect::make(463, 401, 508, 438));
	addCollisionSprite(_ssSymbol3DownButton);
	_ssActionButton = insertSprite<SsScene1105Button>(this, 0x8248AD35, NRect::make(280, 170, 354, 245));
	addCollisionSprite(_ssActionButton);

	_isPanelOpen = true;

	_asTeddyBear->show();

	insertPuzzleMouse(0x18666208, 20, 620);

}

void Scene1105::upOpenPanel() {
	Scene::update();
	if (_backgroundIndex != 0) {
		_backgroundIndex--;
		if (_backgroundIndex < 6 && _backgroundIndex % 2 == 0) {
			uint32 backgroundFileHash = kScene1105BackgroundFileHashes[_backgroundIndex / 2];
			changeBackground(backgroundFileHash);
			_palette->addPalette(backgroundFileHash, 0, 256, 0);
		}
		if (_backgroundIndex == 10)
			playSound(0);
		if (_backgroundIndex == 0) {
			SetUpdateHandler(&Scene1105::update);
			_countdown = 2;
		}
	}
}

void Scene1105::upClosePanel() {
	Scene::update();
	if (_backgroundIndex != 0) {
		_backgroundIndex--;
		if (_backgroundIndex == 14) {
			showMouse(false);
			_ssSymbols[0]->hide();
			_ssSymbols[1]->hide();
			_ssSymbols[2]->hide();
			_ssSymbolDice[0]->hide();
			_ssSymbolDice[1]->hide();
			_ssSymbolDice[2]->hide();
		}
		if (_backgroundIndex < 6 && _backgroundIndex % 2 == 0) {
			uint32 backgroundFileHash = kScene1105BackgroundFileHashes[3 - _backgroundIndex / 2]; // CHECKME
			if (_backgroundIndex == 4) {
				playSound(1);
				_asTeddyBear->hide();
			}
			changeBackground(backgroundFileHash);
			_palette->addPalette(backgroundFileHash, 0, 256, 0);
		}
		if (_backgroundIndex == 0) {
			SetUpdateHandler(&Scene1105::update);
			_isClosePanelDone = true;
		}
	}
}

void Scene1105::update() {
	Scene::update();
	if (_countdown != 0 && (--_countdown == 0))
		createObjects();
	if (_isClosePanelDone && !isSoundPlaying(1))
		leaveScene(_leaveResult);
	if (_doMoveTeddy && !isSoundPlaying(2)) {
		sendMessage(_asTeddyBear, NM_POSITION_CHANGE, 0);
		_doMoveTeddy = false;
	}
}

Scene1109::Scene1109(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule) {

	SetMessageHandler(&Scene1109::handleMessage);

	setBackground(0x8449E02F);
	setPalette(0x8449E02F);
	insertScreenMouse(0x9E02B84C);

	_sprite1 = insertStaticSprite(0x600CEF01, 1100);

	if (which < 0) {
		// Restoring game
		insertKlaymen<KmScene1109>(140, 436);
		setMessageList(0x004B6260);
		sendMessage(this, 0x2000, 0);
	} else if (which == 1) {
		// Klaymen teleporting in
		insertKlaymen<KmScene1109>(450, 436);
		sendMessage(_klaymen, 0x2000, 1);
		setMessageList(0x004B6268, false);
		sendMessage(this, 0x2000, 1);
	} else if (which == 2) {
		// Klaymen teleporting out
		insertKlaymen<KmScene1109>(450, 436);
		sendMessage(_klaymen, 0x2000, 1);
		setMessageList(0x004B6318, false);
		sendMessage(this, 0x2000, 1);
	} else if (which == 3) {
		// Klaymen returning from teleporter console
		insertKlaymen<KmScene1109>(450, 436);
		sendMessage(_klaymen, 0x2000, 1);
		setMessageList(0x004B6278, false);
		sendMessage(this, 0x2000, 1);
	} else {
		// Klaymen entering from the left
		insertKlaymen<KmScene1109>(0, 436);
		setMessageList(0x004B6258);
		sendMessage(this, 0x2000, 0);
	}

	_klaymen->setClipRect(0, 0, _sprite1->getDrawRect().x2(), 480);

}

uint32 Scene1109::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_ANIMATION_UPDATE:
		if (param.asInteger()) {
			setRectList(0x004B63A8);
			_klaymen->setKlaymenIdleTable3();
		} else {
			setRectList(0x004B6398);
			_klaymen->setKlaymenIdleTable1();
		}
		break;
	default:
		break;
	}
	return 0;
}

} // End of namespace Neverhood
