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

#include "neverhood/module1100.h"
#include "neverhood/gamemodule.h"

namespace Neverhood {

Module1100::Module1100(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule) {
	
	if (which < 0) {
		createScene(_vm->gameState().sceneNum, -1);
	} else if (which == 1) {
		createScene(9, 1);
	} else {
		createScene(9, 3);
	}

	// TODO Sound1ChList_addSoundResources(0x2C818, dword_4B85B0, true);
	// TODO Sound1ChList_setSoundValuesMulti(dword_4B85B0, true, 50, 600, 20, 250);
	// TODO Sound1ChList_setSoundValues(0x74E01054, false, 100, 200, 10, 20);
	// TODO Sound1ChList_setVolume(0x74E01054, 60);
	// TODO Sound1ChList_sub_407C70(0x2C818, 0x41861371, 0x43A2507F);

}

Module1100::~Module1100() {
	// TODO Sound1ChList_sub_407A50(0x2C818);
}

void Module1100::createScene(int sceneNum, int which) {
	static const uint32 kSmackerFileHashList06[] = {0x10880805, 0x1088081D, 0};
	static const uint32 kSmackerFileHashList07[] = {0x00290321, 0x01881000, 0};
	debug("Module1100::createScene(%d, %d)", sceneNum, which);
	_vm->gameState().sceneNum = sceneNum;
	switch (_vm->gameState().sceneNum) {
	case 0:
		_countdown = 65;
		createNavigationScene(0x004B8430, which);
		break;
	case 1:
		_countdown = 50;
		createNavigationScene(0x004B8460, which);
		break;
	case 2:
		if (getGlobalVar(0x610210B7)) {
			createNavigationScene(0x004B84F0, which);
		} else {
			createNavigationScene(0x004B8490, which);
		}
		break;
	case 3:
		if (getGlobalVar(0x610210B7)) {
			createNavigationScene(0x004B8580, which);
		} else {
			createNavigationScene(0x004B8550, which);
		}
		break;
	case 4:
		_childObject = new Scene1105(_vm, this, which);
		break;
	case 5:
		if (getGlobalVar(0x610210B7))
			createSmackerScene(0x04180001, true, false, false);
		else
			createSmackerScene(0x04180007, true, false, false);
		break;
	case 6:
		// TODO Sound1ChList_sub_407AF0(0x2C818);
		createSmackerScene(kSmackerFileHashList06, true, true, false);
		break;
	case 7:
		// TODO Sound1ChList_setSoundValues(0x74E01054, false, 0, 0, 0, 0);
		createSmackerScene(kSmackerFileHashList07, true, true, false);
		break;
	case 8:
		_childObject = new Scene1109(_vm, this, which);
		break;
	case 1002:
		_countdown = 40;
		// TODO Sound1ChList_sub_4080B0(true);
		createSmackerScene(0x00012211, true, true, false);
		break;
	}
	SetUpdateHandler(&Module1100::updateScene);
	_childObject->handleUpdate();
}

void Module1100::updateScene() {
	if (!updateChild()) {
		switch (_vm->gameState().sceneNum) {
		case 0:
			_countdown = 0;
			// TODO Sound1ChList_sub_407C70(0x2C818, 0x48498E46, 0x50399F64);
			// TODO Sound1ChList_setVolume(0x48498E46, 65);
			// TODO Sound1ChList_setVolume(0x50399F64, 65);
			if (_moduleResult == 0) {
				createScene(1, 0);
			} else if (_moduleResult == 1) {
				createScene(8, 0);
			}
			break;
		case 1:
			// TODO Sound1ChList_sub_407C70(0x2C818, 0x41861371, 0x43A2507F);
			if (getGlobalVar(0x0C0288F4)) {
				if (_moduleResult == 0) {
					createScene(6, -1);
				} else if (_moduleResult == 1) {
					createScene(0, 1);
				}
			} else {
				if (_moduleResult == 0) {
					createScene(2, 0);
				} else if (_moduleResult == 1) {
					createScene(0, 1);
				}
			}
			break;
		case 2:
			// TODO Sound1ChList_setSoundValues(0x74E01054, false, 0, 0, 0, 0);
			if (_navigationAreaType == 3) {
				createScene(7, -1);
			} else if (_moduleResult == 1) {
				createScene(3, 0);
			} else if (_moduleResult == 2) {
				createScene(1002, -1);
			}
			break;
		case 3:
			if (_moduleResult == 0) {
				createScene(4, 0);
			} else if (_moduleResult == 1) {
				createScene(2, 3);
			}
			break;
		case 4:
			if (_moduleResult == 0) {
				createScene(3, 0);
			} else if (_moduleResult == 1) {
				createScene(5, -1);
			}
			break;
		case 5:
			if (getGlobalVar(0x610210B7)) {
				createScene(3, 0);
			} else {
				createScene(4, 0);
			}
			break;
		case 6:
			leaveModule(1);
			break;
		case 7:
			createScene(2, 2);
			break;
		case 8:
			if (_moduleResult == 0) {
				createScene(0, 0);
			} else if (_moduleResult == 1) {
				leaveModule(0);
			}
			break;
		case 1002:
			_countdown = 0;
			// TODO Sound1ChList_sub_407C70(0x2C818, 0x48498E46, 0x50399F64, 0);
			createScene(1, 1);
			break;
		}
	} else {
		switch (_vm->gameState().sceneNum) {
		case 0:
#if 0 // TODO		
			if (navigationScene()->soundFlag1 && _countdown != 0 && (--_countdown == 0)) {
				Sound1ChList_sub_407C70(0x2C818, 0x48498E46, 0x50399F64);
				Sound1ChList_setVolume(0x48498E46, 65);
				Sound1ChList_setVolume(0x50399F64, 65);
			}
#endif			
			break;
		case 1:
#if 0 // TODO		
			if (navigationScene()->soundFlag1 && _countdown != 0 && (--_countdown == 0)) {
				Sound1ChList_sub_407C70(0x2C818, 0x41861371, 0x43A2507F);
			}
#endif			
			break;
		case 2:
			// TODO Sound1ChList_setSoundValues(0x74E01054, !navigationScene()->soundFlag1, 0, 0, 0, 0);
			break;
		case 5:
		case 6:
		case 7:
		case 1002:
			if (_countdown != 0 && (--_countdown == 0)) {
				// TODO Sound1ChList_sub_407C70(0x2C818, 0x48498E46, 0x50399F64);
				// TODO Sound1ChList_setVolume(0x48498E46, 65);
				// TODO Sound1ChList_setVolume(0x50399F64, 65);
			}
			break;
		}
	}
}

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

static const uint32 kScene1105BackgroundFileHashes[] = {
	0x20018662,
	0x20014202,
	0x20012202,
	0x20010002 // CHECKME: This used ??
};

static const uint32 kSsScene1105SymbolDieFileHashes[] = {
	0,
	0x90898414,
	0x91098414,
	0x92098414,
	0x94098414,
	0x98098414,
	0x80098414,
	0xB0098414,
	0xD0098414,
	0x10098414
};

SsScene1105Button::SsScene1105Button(NeverhoodEngine *vm, Scene *parentScene, uint32 fileHash, NRect &rect)
	: StaticSprite(vm, fileHash, 200), _soundResource(vm), _parentScene(parentScene),
	_countdown(0) {
	
	_rect = rect;
	SetMessageHandler(&SsScene1105Button::handleMessage);
	SetUpdateHandler(&SsScene1105Button::update);
	setVisible(false);
}

void SsScene1105Button::update() {
	if (_countdown != 0 && (--_countdown == 0)) {
		sendMessage(_parentScene, 0x4807, 0);
		setVisible(false);
	}
}

uint32 SsScene1105Button::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_countdown == 0) {
			sendMessage(_parentScene, 0x4826, 0);
			messageResult = 1;
		}
		break;
	case 0x480B:
		_countdown = 8;
		setVisible(true);
		_soundResource.play(0x44141000);
		break;
	}
	return messageResult;
}

SsScene1105Symbol::SsScene1105Symbol(NeverhoodEngine *vm, uint32 fileHash, int16 x, int16 y)
	: StaticSprite(vm, fileHash, 200) {

	_x = x;
	_y = y;
	_drawRect.x = -(_spriteResource.getDimensions().width / 2);
	_drawRect.y = -(_spriteResource.getDimensions().height / 2);
	StaticSprite::update();
}

void SsScene1105Symbol::hide() {
	setVisible(false);
	_needRefresh = true;
	StaticSprite::update();
}

SsScene1105SymbolDie::SsScene1105SymbolDie(NeverhoodEngine *vm, uint index, int16 x, int16 y)
	: StaticSprite(vm, 1100), _index(index) {

	SetMessageHandler(&SsScene1105SymbolDie::handleMessage);
	_x = x;
	_y = y;
	createSurface(200, 50, 50);
	loadSymbolSprite();
}

uint32 SsScene1105SymbolDie::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2000:
		loadSymbolSprite();
		break;
	}
	return messageResult;
}

void SsScene1105SymbolDie::loadSymbolSprite() {
	load(kSsScene1105SymbolDieFileHashes[getSubVar(0x61084036, _index)], true, false);
	_drawRect.x = -(_spriteResource.getDimensions().width / 2);
	_drawRect.y = -(_spriteResource.getDimensions().height / 2);
	StaticSprite::update();
}

void SsScene1105SymbolDie::hide() {
	setVisible(false);
	_needRefresh = true;
	StaticSprite::update();
}

AsScene1105TeddyBear::AsScene1105TeddyBear(NeverhoodEngine *vm, Scene *parentScene)
	: AnimatedSprite(vm, 1100), _soundResource1(vm), _soundResource2(vm),
	_parentScene(parentScene) {
	
	// TODO createSurface3(100, dword_4AF4C0);
	createSurface(100, 640, 480); //TODO: Remeove once the line above is done
	_x = 320;
	_y = 240;
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene1105TeddyBear::handleMessage);
	setFileHash(0x65084002, 0, -1);
	_newHashListIndex = 0;
	setVisible(false);
	_needRefresh = true;
	updatePosition();
	_soundResource1.load(0xCE840261);
	_soundResource2.load(0xCCA41A62);
}

uint32 AsScene1105TeddyBear::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2002:
		if (getGlobalVar(0x610210B7)) {
			setFileHash(0x6B0C0432, 0, -1);
			_soundResource1.play();
		} else {
			setFileHash(0x65084002, 0, -1);
			_soundResource2.play();
		}
		break;
	case 0x3002:
		sendMessage(_parentScene, 0x2003, 0);
		stopAnimation();
		break;
	}
	return messageResult;
}

void AsScene1105TeddyBear::show() {
	setVisible(true);
	_needRefresh = true;
	updatePosition();
}

void AsScene1105TeddyBear::hide() {
	setVisible(false);
	_needRefresh = true;
	updatePosition();
}

SsScene1105OpenButton::SsScene1105OpenButton(NeverhoodEngine *vm, Scene *parentScene)
	: StaticSprite(vm, 900), _soundResource(vm), _parentScene(parentScene),
	_countdown(0), _flag1(false) {
	
	_spriteResource.load2(0x8228A46C);
	createSurface(400, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_deltaRect = _drawRect;
	_needRefresh = true;
	processDelta();
	setVisible(false);
	_soundResource.load(0x44045140);
	SetUpdateHandler(&SsScene1105OpenButton::update);
	SetMessageHandler(&SsScene1105OpenButton::handleMessage);
}

void SsScene1105OpenButton::update() {
	StaticSprite::update();
	if (_countdown != 0 && (--_countdown == 0)) {
		setVisible(false);
		sendMessage(_parentScene, 0x2001, 0);
	}
}

uint32 SsScene1105OpenButton::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = 0;
	Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_countdown == 0 && !_flag1) {
			_soundResource.play();
			setVisible(true);
			_flag1 = true;
			_countdown = 4;
		}
		messageResult = 1;
		break;
	}
	return messageResult;
}

Scene1105::Scene1105(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _soundResource1(vm), _soundResource2(vm),
	_soundResource3(vm), _countdown(0), _flag1(false), _flag2(false), _flag3(false),
	_flag4(false), _flag5(false), _backgroundIndex(0) {
	
	Sprite *ssOpenButton;
	
	_vm->gameModule()->initScene1405Vars();
	
	_surfaceFlag = true;
	SetUpdateHandler(&Scene1105::update);
	SetMessageHandler(&Scene1105::handleMessage);
	
	setBackground(0x20010002);
	setPalette(0x20010002);
	
	_asTeddyBear = insertSprite<AsScene1105TeddyBear>(this);
	ssOpenButton = insertSprite<SsScene1105OpenButton>(this);
	_vm->_collisionMan->addSprite(ssOpenButton);
	insertMouse435(0x10006208, 20, 620);
	
	_soundResource1.load(0x48442057);
	_soundResource2.load(0xC025014F);
	_soundResource3.load(0x68E25540);
	
}

uint32 Scene1105::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = 0;
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		// TODO Debug stuff
		if (param.asPoint().x <= 20 || param.asPoint().x >= 620) {
			if (!_flag2 && _backgroundIndex == 0) {
				if (_flag1) {
					_flag1 = false;
					_backgroundIndex = 15;
					SetUpdateHandler(&Scene1105::upClosePanel);
				} else
					_flag1 = true;
				_flag5 = false;
			}
		}
		break;
	// TODO Debug stuff
	case 0x2001:
		showMouse(false);
		_backgroundIndex = 24;
		SetUpdateHandler(&Scene1105::upOpenPanel);
		break;
	case 0x2003:
		_backgroundIndex = 24;
		_flag5 = true;
		SetUpdateHandler(&Scene1105::upClosePanel);
		break;
	case 0x4807:
		if (sender == _ssActionButton) {
			if (getSubVar(0x7500993A, 0) == getSubVar(0x61084036, 0) &&
				getSubVar(0x7500993A, 1) == getSubVar(0x61084036, 1) &&
				getSubVar(0x7500993A, 2) == getSubVar(0x61084036, 2)) {
				setGlobalVar(0x610210B7, 1);
				_soundResource3.play();
				_flag3 = true;
			} else {
				sendMessage(_asTeddyBear, 0x2002, 0);
			}
			showMouse(false);
			_flag2 = true;
		}
		break;
	case 0x4826:
		if (_flag1) {
			if (sender == _ssActionButton) {
				sendMessage(_ssActionButton, 0x480B, 0);
				_flag1 = false;
			} else if (!getGlobalVar(0x610210B7)) {
				if (sender == _ssSymbol1UpButton) {
					if (getSubVar(0x61084036, 0) < 9) {
						incSubVar(0x61084036, 0, +1);
						sendMessage(_ssSymbol1UpButton, 0x480B, 0);
						sendMessage(_ssSymbolDice[0], 0x2000, 0);
					}
				} else if (sender == _ssSymbol1DownButton) {
					if (getSubVar(0x61084036, 0) > 1) {
						incSubVar(0x61084036, 0, -1);
						sendMessage(_ssSymbol1DownButton, 0x480B, 0);
						sendMessage(_ssSymbolDice[0], 0x2000, 0);
					}
				} else if (sender == _ssSymbol2UpButton) {
					if (getSubVar(0x61084036, 1) < 9) {
						incSubVar(0x61084036, 1, +1);
						sendMessage(_ssSymbol2UpButton, 0x480B, 0);
						sendMessage(_ssSymbolDice[1], 0x2000, 0);
					}
				} else if (sender == _ssSymbol2DownButton) {
					if (getSubVar(0x61084036, 1) > 1) {
						incSubVar(0x61084036, 1, -1);
						sendMessage(_ssSymbol2DownButton, 0x480B, 0);
						sendMessage(_ssSymbolDice[1], 0x2000, 0);
					}
				} else if (sender == _ssSymbol3UpButton) {
					if (getSubVar(0x61084036, 2) < 9) {
						incSubVar(0x61084036, 2, +1);
						sendMessage(_ssSymbol3UpButton, 0x480B, 0);
						sendMessage(_ssSymbolDice[2], 0x2000, 0);
					}
				} else if (sender == _ssSymbol3DownButton) {
					if (getSubVar(0x61084036, 2) > 1) {
						incSubVar(0x61084036, 2, -1);
						sendMessage(_ssSymbol3DownButton, 0x480B, 0);
						sendMessage(_ssSymbolDice[2], 0x2000, 0);
					}
				}
			}
		}
		break;
	}
	return messageResult;
}

void Scene1105::createObjects() {
	_ssSymbols[0] = insertSprite<SsScene1105Symbol>(kScene1105FileHashes[getSubVar(0x13100631, 0)], 161, 304);
	_ssSymbols[1] = insertSprite<SsScene1105Symbol>(kScene1105FileHashes[getSubVar(0x13100631, 1)], 294, 304);
	_ssSymbols[2] = insertSprite<SsScene1105Symbol>(kScene1105FileHashes[getSubVar(0x13100631, 2)], 440, 304);

	_ssSymbolDice[0] = insertSprite<SsScene1105SymbolDie>(0, 206, 304);
	_ssSymbolDice[1] = insertSprite<SsScene1105SymbolDie>(1, 339, 304);
	_ssSymbolDice[2] = insertSprite<SsScene1105SymbolDie>(2, 485, 304);

	_ssSymbol1UpButton = insertSprite<SsScene1105Button>(this, 0x08002860, NRect(146, 362, 192, 403));
	_vm->_collisionMan->addSprite(_ssSymbol1UpButton);
	_ssSymbol1DownButton = insertSprite<SsScene1105Button>(this, 0x42012460, NRect(147, 404, 191, 442));
	_vm->_collisionMan->addSprite(_ssSymbol1DownButton);
	_ssSymbol2UpButton = insertSprite<SsScene1105Button>(this, 0x100030A0, NRect(308, 361, 355, 402));
	_vm->_collisionMan->addSprite(_ssSymbol2UpButton);
	_ssSymbol2DownButton = insertSprite<SsScene1105Button>(this, 0x840228A0, NRect(306, 406, 352, 445));
	_vm->_collisionMan->addSprite(_ssSymbol2DownButton);
	_ssSymbol3UpButton = insertSprite<SsScene1105Button>(this, 0x20000120, NRect(476, 358, 509, 394));
	_vm->_collisionMan->addSprite(_ssSymbol3UpButton);
	_ssSymbol3DownButton = insertSprite<SsScene1105Button>(this, 0x08043121, NRect(463, 401, 508, 438));
	_vm->_collisionMan->addSprite(_ssSymbol3DownButton);
	_ssActionButton = insertSprite<SsScene1105Button>(this, 0x8248AD35, NRect(280, 170, 354, 245));
	_vm->_collisionMan->addSprite(_ssActionButton);
	
	_flag1 = true;
	
	_asTeddyBear->show();

	// TODO: Find a nicer way
	deleteSprite((Sprite**)&_mouseCursor);
	insertMouse435(0x18666208, 20, 620);
	
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
		if (_backgroundIndex == 10) {
			_soundResource1.play();
		}
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
				_soundResource2.play();
				_asTeddyBear->hide();
			}
			changeBackground(backgroundFileHash);
			_palette->addPalette(backgroundFileHash, 0, 256, 0);
		}
		if (_backgroundIndex == 0) {
			SetUpdateHandler(&Scene1105::update);
			_flag4 = true;
		}
	}
}

void Scene1105::update() {

	// DEBUG: Show the correct code
	debug("(%d, %d) (%d, %d) (%d, %d)", 
		getSubVar(0x7500993A, 0), getSubVar(0x61084036, 0),
		getSubVar(0x7500993A, 1), getSubVar(0x61084036, 1),
		getSubVar(0x7500993A, 2), getSubVar(0x61084036, 2));

	Scene::update();
	if (_countdown != 0 && (--_countdown == 0)) {
		createObjects();
	}
	if (_flag4 && !_soundResource2.isPlaying()) {
		leaveScene(_flag5);
	}
	if (_flag3 && !_soundResource3.isPlaying()) {
		sendMessage(_asTeddyBear, 0x2002, 0);
		_flag3 = false;
	}
}

Scene1109::Scene1109(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, which) {
	
	_surfaceFlag = true;
	SetMessageHandler(&Scene1109::handleMessage);
	
	setBackground(0x8449E02F);
	setPalette(0x8449E02F);
	insertMouse433(0x9E02B84C);
	
	_sprite1 = insertStaticSprite(0x600CEF01, 1100);

	if (which < 0) {
		insertKlayman<KmScene1109>(140, 436);
		setMessageList(0x004B6260);
		sendMessage(this, 0x2000, 0);
	} else if (which == 1) {
		insertKlayman<KmScene1109>(450, 436);
		sendMessage(_klayman, 0x2000, 1);
		setMessageList(0x004B6268, false);
		sendMessage(this, 0x2000, 1);
	} else if (which == 2) {
		insertKlayman<KmScene1109>(450, 436);
		sendMessage(_klayman, 0x2000, 1);
		setMessageList(0x004B6318, false);
		sendMessage(this, 0x2000, 1);
	} else if (which == 3) {
		insertKlayman<KmScene1109>(450, 436);
		sendMessage(_klayman, 0x2000, 1);
		setMessageList(0x004B6278, false);
		sendMessage(this, 0x2000, 1);
	} else {
		insertKlayman<KmScene1109>(0, 436);
		setMessageList(0x004B6258);
		sendMessage(this, 0x2000, 0);
	}

	_klayman->setClipRect(0, 0, _sprite1->getDrawRect().x2(), 480);

}

uint32 Scene1109::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2000:
		if (param.asInteger()) {
			setRectList(0x004B63A8);
			_klayman->setKlaymanTable3();
		} else {
			setRectList(0x004B6398);
			_klayman->setKlaymanTable1();
		}
		break;
	}
	return 0;
}

} // End of namespace Neverhood
