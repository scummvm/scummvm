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

#include "neverhood/module3000.h"
#include "neverhood/gamemodule.h"
#include "neverhood/navigationscene.h"

namespace Neverhood {

Module3000::Module3000(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule), _soundVolume(0) {
	
	// TODO Sound1ChList_addSoundResources(0x81293110, dword_4B7FC8, true);
	// TODO Sound1ChList_setSoundValuesMulti(dword_4B7FC8, 1, 50, 600, 5, 150);
	// TODO Sound1ChList_setSoundValues(0x90F0D1C3, false, 20000, 30000, 20000, 30000);
	// TODO Sound1ChList_sub_407C70(0x81293110, 0x48498E46, 0x50399F64, 0);
	// TODO Sound1ChList_sub_407C70(0x81293110, 0x40030A51, 0xC862CA15, 0);
	// TODO Sound1ChList_sub_407C70(0x81293110, 0x41861371, 0x43A2507F, 0);

	_flag = getGlobalVar(0x10938830) != 0;

	if (_flag) {
		// TODO Sound1ChList_setVolume(0x90F0D1C3, 0);
		// TODO Sound1ChList_playLooping(0x90F0D1C3);
	}

	if (which < 0) {
		createScene(_vm->gameState().sceneNum, -1);
	} else if (which == 0) {
		createScene(1, 0);
	} else if (which == 1) {
		createScene(4, 2);
	} else if (which == 2) {
		createScene(4, 1);
	} else if (which == 3) {
		createScene(5, 1);
	}

}

Module3000::~Module3000() {
	// TODO Sound1ChList_sub_407A50(0x81293110);
}

void Module3000::createScene(int sceneNum, int which) {
	static const byte kNavigationTypes05[] = {3, 0};
	static const byte kNavigationTypes06[] = {5};
	debug("Module3000::createScene(%d, %d)", sceneNum, which);
	_vm->gameState().sceneNum = sceneNum;
	switch (_vm->gameState().sceneNum) {
	case 1:
		if (!getGlobalVar(0x01BA1A52)) {
			createNavigationScene(0x004B7C80, which);
		} else if (getGlobalVar(0x10938830)) {
			createNavigationScene(0x004B7CE0, which);
		} else {
			createNavigationScene(0x004B7CB0, which);
		}
		break;
	case 2:
		// TODO Sound1ChList_sub_407C70(0x81293110, 0x40030A51, 0xC862CA15, 0);
		if (_flag) {
			_soundVolume = 90;
			// TODO Sound1ChList_setVolume(0x90F0D1C3, 90);
		}
		if (getGlobalVar(0x10938830)) {
			createNavigationScene(0x004B7D58, which);
		} else {
			createNavigationScene(0x004B7D10, which);
		}
		break;
	case 3:
		if (getGlobalVar(0x09221A62))
			createNavigationScene(0x004B7E60, which);
		else if (getGlobalVar(0x10938830))
			createNavigationScene(0x004B7DA0, which);
		else
			createNavigationScene(0x004B7E00, which);
		break;
	case 4:
		if (getGlobalVar(0x09221A62))
			createNavigationScene(0x004B7F20, which);
		else
			createNavigationScene(0x004B7EC0, which);
		break;
	case 5:
		createNavigationScene(0x004B7F80, which, kNavigationTypes05);
		break;
	case 6:
		createNavigationScene(0x004B7FB0, which, kNavigationTypes06);
		break;
	case 7:
		// TODO Sound1ChList_setSoundValuesMulti(dword_4B7FC8, 0, 0, 0, 0, 0);
		if (!getSubVar(0x40050052, 0x089809C2)) {
			setSubVar(0x40050052, 0x089809C2, 1);
			createSmackerScene(0x90022001, true, true, false);
		} else
			createSmackerScene(0x98022001, true, true, false);
		break;
	case 8:
		_childObject = new Scene3009(_vm, this, which);
		break;
	case 9:
		_childObject = new Scene3010(_vm, this, 0);
		break;
	case 10:
		_childObject = new Scene3011(_vm, this, 0);
		break;
	case 11:
		// TODO Sound1ChList_setSoundValuesMulti(dword_4B7FC8, 0, 0, 0, 0, 0);
		if (!getSubVar(0x40050052, 0x10130993)) {
			setSubVar(0x40050052, 0x10130993, 1);
			createSmackerScene(0x31093019, true, true, false);
		} else
			createSmackerScene(0x20093019, true, true, false);
		break;
	case 12:
		_childObject = new Scene3010(_vm, this, 1);
		break;
	// NOTE: Newly introduced sceneNums
	case 1001:
		if (!getGlobalVar(0x01BA1A52))
			if (getGlobalVar(0x10938830))
				createSmackerScene(0x00940021, true, true, false);
			else
				createSmackerScene(0x01140021, true, true, false);
		else
			if (getGlobalVar(0x10938830))
				createSmackerScene(0x001011B1, true, true, false);
			else
				createSmackerScene(0x001021B1, true, true, false);
		setGlobalVar(0x01BA1A52, getGlobalVar(0x01BA1A52) ? 0 : 1);
		break;
	case 1006:
		createSmackerScene(0x080810C5, true, true, false);
		break;
	case 1008:
		createSmackerScene(getGlobalVar(0xF0402B0A), true, true, false);
		break;
	}
	SetUpdateHandler(&Module3000::updateScene);
	_childObject->handleUpdate();
}

void Module3000::updateScene() {
	if (!updateChild()) {
		switch (_vm->gameState().sceneNum) {
		case 1:
			if (!getGlobalVar(0x01BA1A52)) {
				if (_moduleResult == 0)
					createScene(9, -1);
				else if (_moduleResult == 1)
					leaveModule(0);
			} else {
				if (_moduleResult == 0)
					if (_navigationAreaType == 2)
						createScene(2, 0);
					else
						createScene(1001, -1);
				else if (_moduleResult == 1)
					leaveModule(0);
			}
			break;
		case 2:
			// TODO Sound1ChList_sub_407C70(0x81293110, 0x41861371, 0x43A2507F, 0);
			if (_flag) {
				_soundVolume = 0;
				// TODO Sound1ChList_setVolume(0x90F0D1C3, 0);
			}
			if (_moduleResult == 0) {
				createScene(3, 0);
			} else if (_moduleResult == 1) {
				setGlobalVar(0x01BA1A52, 0);
				createScene(1, 1);
			}
			break;
		case 3:
			if (_moduleResult == 1)
				createScene(4, 0);
			else if (_moduleResult == 3)
				createScene(10, -1);
			else if (getGlobalVar(0x09221A62))
				createScene(5, 0);
			else
				createScene(2, 1);
			break;
		case 4:
			if (_moduleResult == 0)
				leaveModule(1);
			else if (_moduleResult == 1)
				createScene(7, -1);
			else if (_moduleResult == 2)
				createScene(3, 3);
			break;
		case 5:
			if (_moduleResult == 0)
				createScene(6, 0);
			else if (_moduleResult == 1)
				createScene(3, 0);
			break;
		case 6:
			if (_navigationAreaType == 4)
				createScene(11, -1);
			else
				createScene(1006, -1);
			break;
		case 7:
			createScene(8, -1);
			break;
		case 8:
			_flag = getGlobalVar(0x10938830); // CHECKME
			if (_moduleResult != 1) {
				// TODO: Sound1ChList_setSoundValuesMulti(dword_4B7FC8, true, 0, 0, 0, 0):
				createScene(4, 1);
			} else if (getGlobalVar(0xF0402B0A)) {
				createScene(1008, -1);
			} else {
				// TODO: Sound1ChList_setSoundValuesMulti(dword_4B7FC8, true, 0, 0, 0, 0);
				createScene(4, 1);
			}
			break;
		case 9:
			if (_moduleResult == 0 || _moduleResult == 2)
				createScene(1, 0);
			else if (_moduleResult == 1)
				createScene(1001, -1);
			break;
		case 10:
			createScene(3, 3);
			break;
		case 11:
			leaveModule(3);
			break;
		case 12:
			createScene(1, 0);
			break;
		case 1001:
			if (getGlobalVar(0x01BA1A52))
				createScene(1, 0);
			else
				createScene(12, -1);
			break;
		case 1006:
			createScene(5, 0);
			break;
		case 1008:
			createScene(8, -1);
			break;
		}
	} else {
		switch (_vm->gameState().sceneNum) {
		case 1:
#if 0 // ALL TODO
			if (navigationScene()->getSoundFlag1()) {
				uint32 frameNumber = navigationScene()->getFrameNumber();
				int navigationIndex = navigationScene()->getIndex();
				if (navigationIndex == 1) {
					if (frameNumber == 0) {
						// TODO Sound1ChList_sub_407C70(0x81293110, 0x48498E46, 0x50399F64, 0);
						// TODO Sound1ChList_setVolume(0x48498E46, 70);
						// TODO Sound1ChList_setVolume(0x50399F64, 70);
					} else if (frameNumber == 100) {
						// TODO Sound1ChList_sub_407C70(0x81293110, 0x41861371, 0x43A2507F, 0);
					}
				} else if (navigationIndex == 0) {
					if (frameNumber == 0) {
						// TODO Sound1ChList_sub_407C70(0x81293110, 0x48498E46, 0x50399F64, 0);
						// TODO Sound1ChList_setVolume(0x48498E46, 70);
						// TODO Sound1ChList_setVolume(0x50399F64, 70);
					} else if (frameNumber == 10) {
						// TODO Sound1ChList_sub_407C70(0x81293110, 0x40030A51, 0xC862CA15, 0);
					}
					if (_flag && _soundVolume < 90 && frameNumber % 2) {
						if (frameNumber == 0)
							_soundVolume = 40;
						else
							_soundVolume++;
						// TODO Sound1ChList_setVolume(0x90F0D1C3, _soundVolume);
					}
				}
			}
#endif
			break;
		case 2:
#if 0 // ALL TODO
			if (navigationScene()->getSoundFlag1()) {
				uint32 frameNumber = navigationScene()->getFrameNumber();
				int navigationIndex = navigationScene()->getIndex();
				if (_flag && _soundVolume > 1 && frameNumber % 2) {
					_soundVolume--;
					// TODO Sound1ChList_setVolume(0x90F0D1C3, _soundVolume);
				}
				if (navigationIndex == 0) {
					if (frameNumber == 35) {
						// TODO Sound1ChList_sub_407C70(0x81293110, 0x41861371, 0x43A2507F, 0);
					}
				} else if (navigationIndex == 1) {
					if (frameNumber == 55) {
						// TODO Sound1ChList_sub_407C70(0x81293110, 0x48498E46, 0x50399F64, 0);
						// TODO Sound1ChList_setVolume(0x48498E46, 70);
						// TODO Sound1ChList_setVolume(0x50399F64, 70);
					}
				}
			}
#endif
			break;
		case 3:
#if 0 // ALL TODO
			if (navigationScene()->getSoundFlag1()) {
				uint32 frameNumber = navigationScene()->getFrameNumber();
				int navigationIndex = navigationScene()->getIndex();
				if (navigationIndex == 2) {
					if (frameNumber == 40) {
						// TODO Sound1ChList_sub_407C70(0x81293110, 0x40030A51, 0xC862CA15, 0);
					}
					if (_flag && _soundVolume < 90 && frameNumber % 2) {
						if (frameNumber == 0)
							_soundVolume = 40;
						else
							_soundVolume++;
						// TODO Sound1ChList_setVolume(0x90F0D1C3, _soundVolume);
					}
				}
			}
#endif
			break;
		case 5:
#if 0 // ALL TODO
			if (navigationScene()->getSoundFlag1() && navigationScene()->getIndex() == 0) {
				// TODO Sound1ChList_sub_4080B0(false);
			}
#endif
			break;
		}
	}
}

// Scene3009

static const uint32 kScene3009SmackerFileHashes[] = {
	0x1010000D,
	0x340A0049,
	0x340A0049,
	0x0282081D,
	0x0082080D,
	0x0882080D,
	0x0882080D,
	0x0282081D,
	0x004B000B,
	0x014B000B,
	0x044B000B,
	0x0282081D,
	0x0282081D,
	0x0282081D,
	0x340A0049
};

static const uint32 kScene3009CannonLocationFileHashes[] = {
	0x00000000,
	0x8004001B,
	0x0004001A,
	0x1048404B,
	0x50200109,
	0x12032109,
	0x10201109,
	0x000A2030,
	0x000A0028,
	0x000A0028,
	0x000A0028,
	0x040A1069,
	0x040A1069,
	0x040A1069,
	0x240A1101
};

static const uint32 kSsScene3009SymbolEdgesFileHashes[] = {
	0x618827A0,
	0xB1A92322
};

static const uint32 kSsScene3009TargetLineFileHashes[] = {
	0x4011018C,
	0x15086623
};

static const NPoint kAsScene3009SymbolPoints[] = {
	{289, 338},
	{285, 375},
	{284, 419},
	{456, 372},
	{498, 372},
	{541, 372}
};

static const uint32 kAsScene3009SymbolFileHashes[] = {
	0x24542582,
	0x1CD61D96
};

static const uint32 kSsScene3009SymbolArrowFileHashes1[] = {
	0x24016060,  
	0x21216221,
	0x486160A0,
	0x42216422,
	0x90A16120,
	0x84216824,
	0x08017029,
	0x08217029,
	0x10014032,
	0x10214032,
	0x20012004,
	0x20212004
};

static const uint32 kSsScene3009SymbolArrowFileHashes2[] = {
	0x40092024, 
	0x01636002,
	0x8071E028,
	0x02A56064,
	0x00806031,
	0x052960A8,
	0x0A116130,
	0x0A316130,
	0x14216200,
	0x14016200,
	0x28416460,
	0x28616460
};

SsScene3009FireCannonButton::SsScene3009FireCannonButton(NeverhoodEngine *vm, Scene3009 *parentScene)
	: StaticSprite(vm, 1400), _soundResource(vm), _parentScene(parentScene),
	_flag1(false) {
	
	_spriteResource.load2(0x120B24B0);
	createSurface(400, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_deltaRect.x = 0;
	_deltaRect.y = 0;
	_deltaRect.width = _spriteResource.getDimensions().width;
	_deltaRect.height = _spriteResource.getDimensions().height;
	setVisible(false);
	processDelta();
	_needRefresh = true;
	SetUpdateHandler(&SsScene3009FireCannonButton::update);
	SetMessageHandler(&SsScene3009FireCannonButton::handleMessage);
	_soundResource.load(0x3901B44F);
}

void SsScene3009FireCannonButton::update() {
	StaticSprite::update();
	if (_flag1 && !_soundResource.isPlaying()) {
		sendMessage(_parentScene, 0x2000, 0);
		setVisible(false);
	}
}

uint32 SsScene3009FireCannonButton::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (!_flag1 && !_parentScene->sub462E90()) {
			_flag1 = true;
			setVisible(true);
			_soundResource.play();
		}
		messageResult = 1;
		break;
	}
	return messageResult;
}

SsScene3009SymbolEdges::SsScene3009SymbolEdges(NeverhoodEngine *vm, int index)
	: StaticSprite(vm, 1400), _blinkCountdown(0) {

	_spriteResource.load2(kSsScene3009SymbolEdgesFileHashes[index]);
	createSurface(600, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_needRefresh = true;
	if (getGlobalVar(0x0C0288F4)) {
		hide();
	} else {
		startBlinking();
	}
	SetUpdateHandler(&SsScene3009SymbolEdges::update);
}

void SsScene3009SymbolEdges::update() {
	if (_blinkCountdown != 0 && (--_blinkCountdown == 0)) {
		if (_blinkToggle) {
			setVisible(true);
		} else {
			setVisible(false);
		}
		StaticSprite::update();
		_blinkCountdown = 3;
		_blinkToggle = !_blinkToggle;
	}
}

void SsScene3009SymbolEdges::show() {
	setVisible(true);
	StaticSprite::update();
	_blinkCountdown = 0;
}

void SsScene3009SymbolEdges::hide() {
	setVisible(false);
	StaticSprite::update();
	_blinkCountdown = 0;
}

void SsScene3009SymbolEdges::startBlinking() {
	setVisible(true);
	StaticSprite::update();
	_blinkCountdown = 3;
	_blinkToggle = true;
}

SsScene3009TargetLine::SsScene3009TargetLine(NeverhoodEngine *vm, int index)
	: StaticSprite(vm, 1400) {

	_spriteResource.load2(kSsScene3009TargetLineFileHashes[index]);
	createSurface(600, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	setVisible(false);
	_needRefresh = true;
}

void SsScene3009TargetLine::show() {
	setVisible(true);
	StaticSprite::update();
}

SsScene3009SymbolArrow::SsScene3009SymbolArrow(NeverhoodEngine *vm, Sprite *asSymbol, int index)
	: StaticSprite(vm, 1400), _soundResource(vm), _asSymbol(asSymbol), 
	_index(index), _enabled(true), _countdown(0) {

	_incrDecr = _index % 2;

	_spriteResource.load2(kSsScene3009SymbolArrowFileHashes2[_index]);
	createSurface(1200, 33, 31);
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = 33;
	_drawRect.height = 31;
	_deltaRect = _drawRect;
	processDelta();
	_needRefresh = true;
	SetUpdateHandler(&SsScene3009SymbolArrow::update);
	SetMessageHandler(&SsScene3009SymbolArrow::handleMessage);
	_soundResource.load(0x2C852206);
}

void SsScene3009SymbolArrow::hide() {
	_enabled = false;
	setVisible(false);
}

void SsScene3009SymbolArrow::update() {
	StaticSprite::update();
	if (_countdown != 0 && (--_countdown == 0)) {
		_spriteResource.load2(kSsScene3009SymbolArrowFileHashes2[_index]);
		_needRefresh = true;
		_drawRect.x = 0;
		_drawRect.y = 0;
		_drawRect.width = _spriteResource.getDimensions().width;
		_drawRect.height = _spriteResource.getDimensions().height;
	}
}

uint32 SsScene3009SymbolArrow::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_enabled && _countdown == 0) {
			_countdown = 2;
			_spriteResource.load2(kSsScene3009SymbolArrowFileHashes1[_index]);
			_needRefresh = true;
			_drawRect.x = 0;
			_drawRect.y = 0;
			_drawRect.width = _spriteResource.getDimensions().width;
			_drawRect.height = _spriteResource.getDimensions().height;
			_soundResource.play();
			sendMessage(_asSymbol, 0x2005, _incrDecr);
		}
		messageResult = 1;
		break;
	}
	return messageResult;
}

AsScene3009VerticalIndicator::AsScene3009VerticalIndicator(NeverhoodEngine *vm, Scene3009 *parentScene, int index)
	: AnimatedSprite(vm, 1000), _parentScene(parentScene), _enabled(false) {

	_x = 300;
	_y = getGlobalVar(0x000809C2) ? 52 : 266;
	createSurface1(0xC2463913, 1200);
	_needRefresh = true;
	updatePosition();
	setVisible(false);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene3009VerticalIndicator::handleMessage);
}

void AsScene3009VerticalIndicator::show() {
	startAnimation(0xC2463913, 0, -1);
	setVisible(true);
	updatePosition();
	_enabled = true;
}

uint32 AsScene3009VerticalIndicator::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_enabled) {
			sendMessage(_parentScene, 0x2002, 0);
		}
		messageResult = 1;
		break;
	}
	return messageResult;
}

AsScene3009HorizontalIndicator::AsScene3009HorizontalIndicator(NeverhoodEngine *vm, Scene3009 *parentScene, uint32 varValue)
	: AnimatedSprite(vm, 1000), _parentScene(parentScene), _enabled(false) {
	
	_x = getGlobalVar(0x9040018A) ? 533 : 92;
	_y = 150;
	createSurface1(0xC0C12954, 1200);
	_needRefresh = true;
	updatePosition();
	setVisible(false);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene3009HorizontalIndicator::handleMessage);
	if (varValue == 8 || varValue == 9 || varValue == 10) {
		SetSpriteCallback(&AsScene3009HorizontalIndicator::suMoveRight);
		_x = 280;
	}
}

uint32 AsScene3009HorizontalIndicator::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_enabled) {
			sendMessage(_parentScene, 0x2004, 0);
		}
		messageResult = 1;
		break;
	}
	return messageResult;
}

void AsScene3009HorizontalIndicator::suMoveLeft() {
	_x -= 6;
	if (_x < 92) {
		SetSpriteCallback(NULL);
		_x = 92;
	}
}

void AsScene3009HorizontalIndicator::suMoveRight() {
	_x += 6;
	if (_x > 533) {
		SetSpriteCallback(NULL);
		_x = 533;
	}
}

void AsScene3009HorizontalIndicator::show() {
	startAnimation(0xC0C12954, 0, -1);
	setVisible(true);
	updatePosition();
	_enabled = true;
}

void AsScene3009HorizontalIndicator::stMoveLeft() {
	_x = 533;
	SetSpriteCallback(&AsScene3009HorizontalIndicator::suMoveLeft);
}

void AsScene3009HorizontalIndicator::stMoveRight() {
	_x = 330;
	SetSpriteCallback(&AsScene3009HorizontalIndicator::suMoveRight);
}

AsScene3009Symbol::AsScene3009Symbol(NeverhoodEngine *vm, Scene3009 *parentScene, int index)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _index(index) {

	_symbolIndex = getSubVar(0x00000914, _index);
	
	_x = kAsScene3009SymbolPoints[_index].x;
	_y = kAsScene3009SymbolPoints[_index].y;
	createSurface1(kAsScene3009SymbolFileHashes[_index / 3], 1200);
	startAnimation(kAsScene3009SymbolFileHashes[_index / 3], _symbolIndex, -1);
	_newStickFrameIndex = _symbolIndex;
	_needRefresh = true;
	updatePosition();
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene3009Symbol::handleMessage);
	_ssArrowPrev = _parentScene->insertSprite<SsScene3009SymbolArrow>(this, _index * 2 + 0);
	_vm->_collisionMan->addSprite(_ssArrowPrev);
	_ssArrowNext = _parentScene->insertSprite<SsScene3009SymbolArrow>(this, _index * 2 + 1);
	_vm->_collisionMan->addSprite(_ssArrowNext);
}

uint32 AsScene3009Symbol::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2005:
		if (param.asInteger()) {
			if (_symbolIndex == 11)
				_symbolIndex = 0;
			else
				_symbolIndex++;
		} else {
			if (_symbolIndex == 0)
				_symbolIndex = 11;
			else
				_symbolIndex--;
		}
		startAnimation(kAsScene3009SymbolFileHashes[_index / 3], _symbolIndex, -1);
		_newStickFrameIndex = _symbolIndex;
		setSubVar(0x00000914, _index, _symbolIndex);
		if (_index / 3 == 0) {
			sendMessage(_parentScene, 0x2001, 0);
		} else {
			sendMessage(_parentScene, 0x2003, 0);
		}
		messageResult = 1;
		break;
	}
	return messageResult;
}

void AsScene3009Symbol::hide() {
	_ssArrowPrev->hide();
	_ssArrowNext->hide();
}

Scene3009::Scene3009(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _keepVideo(false), _flag2(false), 
	/*_flag3(false), */_flag4(false), _lockSymbolsPart1Countdown(1), _lockSymbolsPart2Countdown(1) {

	_cannonLocation = getGlobalVar(0x20580A86);
	debug("_cannonLocation = %d", _cannonLocation);
	
	_vm->gameModule()->initScene3009Vars();
	
	setGlobalVar(0xF0402B0A, 0);
	_surfaceFlag = true;
	
	_vm->_screen->clear();
	
	setBackground(0xD000420C);
	setPalette(0xD000420C);
	insertMouse435(0x04208D08, 20, 620);

	_ssFireCannonButton = insertSprite<SsScene3009FireCannonButton>(this);
	_vm->_collisionMan->addSprite(_ssFireCannonButton);

	_asVerticalIndicator = insertSprite<AsScene3009VerticalIndicator>(this, _cannonLocation);
	_vm->_collisionMan->addSprite(_asVerticalIndicator);

	_asHorizontalIndicator = insertSprite<AsScene3009HorizontalIndicator>(this, _cannonLocation);
	_vm->_collisionMan->addSprite(_asHorizontalIndicator);

	if (_cannonLocation != 0 && _cannonLocation != 8 && _cannonLocation != 9 && _cannonLocation != 10) {
		_keepVideo = true;
	} else {
		_keepVideo = false;
		if (_cannonLocation != 0) {
			_asHorizontalIndicator->stMoveRight();
			_flag4 = true;
		}
	}

	_smackerPlayer = addSmackerPlayer(new SmackerPlayer(_vm, this, kScene3009SmackerFileHashes[_cannonLocation], false, _keepVideo));
	_smackerPlayer->setDrawPos(89, 37);
	_palette->usePalette(); // Use it again since the SmackerPlayer overrides the usage

	insertStaticSprite(0x8540252C, 400);

	for (int i = 0; i < 2; i++) {
		_ssSymbolEdges[i] = insertSprite<SsScene3009SymbolEdges>(i);
		_ssTargetLines[i] = insertSprite<SsScene3009TargetLine>(i);
	}

	for (int i = 0; i < 6; i++) {
		_asSymbols[i] = insertSprite<AsScene3009Symbol>(this, i);
		if (i < 3)
			_correctSymbols[i] = getSubVar(0x00504B86, i);
		else
			_correctSymbols[i] = getSubVar(0x0A4C0A9A, i - 3);
	}

	SetMessageHandler(&Scene3009::handleMessage);
	SetUpdateHandler(&Scene3009::update);

	// DEBUG: Set the correct code
	for (int i = 0; i < 6; i++)
		setSubVar(0x00000914, i, _correctSymbols[i]);
	sendMessage(this, 0x2003, 0);
	//setGlobalVar(0x610210B7, 1);   

}

void Scene3009::update() {
	Scene::update();
	
	if (!_keepVideo && _smackerPlayer->getFrameNumber() + 1 == _smackerPlayer->getFrameCount() && _cannonLocation <= 14) {
		switch (_cannonLocation) {
		case 0:
		case 14:
			_smackerPlayer->open(0x340A0049, true);
			_palette->usePalette();
			_keepVideo = true;
			break;
		case 8:
			_smackerPlayer->open(0x0082080D, true);
			_palette->usePalette();
			_keepVideo = true;
			_flag4 = false;
			break;
		case 9:
			_smackerPlayer->open(0x0282080D, true);
			_palette->usePalette();
			_keepVideo = true;
			_flag4 = false;
			break;
		case 10:
			_smackerPlayer->open(0x0882080D, true);
			_palette->usePalette();
			_keepVideo = true;
			_flag4 = false;
			break;
		case 11:
		case 12:
		case 13:
			if (_flag2) {
				if (_cannonLocation == 11)
					_smackerPlayer->open(0x110A000F, false);
				else if (_cannonLocation == 12)				
					_smackerPlayer->open(0x500B004F, false);
				else if (_cannonLocation == 13)				
					_smackerPlayer->open(0x100B010E, false);
				_palette->usePalette();
				_flag2 = false;
				_asHorizontalIndicator->stMoveLeft();
			} else {
				playExtVideo();
			}
			break;
		}
	}

	if (_lockSymbolsPart1Countdown != 0 && (--_lockSymbolsPart1Countdown == 0) && isSymbolsPart1Solved()) {
		for (int i = 0; i < 3; i++)
			_asSymbols[i]->hide();
		if (!getGlobalVar(0x0C0288F4) || getGlobalVar(0x000809C2) || getGlobalVar(0x9040018A)) {
			_ssSymbolEdges[0]->show();
			_ssTargetLines[0]->show();
			_asVerticalIndicator->show();
		}
	}

	if (_lockSymbolsPart2Countdown != 0 && (--_lockSymbolsPart2Countdown == 0) && isSymbolsPart2Solved()) {
		for (int i = 3; i < 6; i++)
			_asSymbols[i]->hide();
		if (!getGlobalVar(0x0C0288F4) || getGlobalVar(0x000809C2) || getGlobalVar(0x9040018A)) {
			_ssSymbolEdges[1]->show();
			_ssTargetLines[1]->show();
			_asHorizontalIndicator->show();
		}
	}

}

uint32 Scene3009::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		// TODO: Debug stuff
		if ((param.asPoint().x <= 20 || param.asPoint().x >= 620) && !getGlobalVar(0x000809C2)) {
			setGlobalVar(0x20580A86, 0);
			leaveScene(0);
		}
		break;
	case 0x000D:
		// TODO: Debug stuff
		break;
	case 0x2000:
		if (!getGlobalVar(0x000809C2)) {
			if (!getGlobalVar(0x10938830)) {
				_cannonLocation = 1;
				setGlobalVar(0x10938830, 1);
			} else {
				_cannonLocation = 2;
			}
		} else if (!getGlobalVar(0x9040018A)) {
			_cannonLocation = 3;
		} else if (!getGlobalVar(0x610210B7)) {
			_cannonLocation = 4;
		} else if (!getGlobalVar(0x0C0288F4)) {
			setGlobalVar(0x0C0288F4, 1);
			_cannonLocation = 5;
		} else {
			_cannonLocation = 6;
		}
		playExtVideo();
		break;
	case 0x2001:
		_lockSymbolsPart1Countdown = 24;
		break;
	case 0x2002:
		if (!getGlobalVar(0x9040018A) && !_flag4) {
			if (getGlobalVar(0x000809C2)) {
				_cannonLocation = 14;
				setGlobalVar(0x000809C2, 0);
			} else {
				_cannonLocation = 7;
				setGlobalVar(0x000809C2, 1);
			}
			playExtVideo();
		}
		break;
	case 0x2003:
		_lockSymbolsPart2Countdown = 24;
		break;
	case 0x2004:
		if (getGlobalVar(0x000809C2)) {
			if (!getGlobalVar(0x9040018A)) {
				if (!getGlobalVar(0x610210B7)) {
					_cannonLocation = 8;
				} else if (!getGlobalVar(0x0C0288F4)) {
					_cannonLocation = 9;
				} else {
					_cannonLocation = 10;
				}
				setGlobalVar(0x9040018A, 1);
				_flag4 = true;
				playExtVideo();
			} else {
				if (!getGlobalVar(0x610210B7)) {
					_cannonLocation = 11;
					_smackerPlayer->open(0x108A000F, false);
				} else if (!getGlobalVar(0x0C0288F4)) {
					_cannonLocation = 12;
					_smackerPlayer->open(0x500B002F, false);
				} else {
					_cannonLocation = 13;
					_smackerPlayer->open(0x100B008E, false);
				}
				_palette->usePalette();
				_flag2 = true;
				_flag4 = true;
				_keepVideo = false;
				setGlobalVar(0x9040018A, 0);
			}
		}
		break;
	}
	return 0;
}

void Scene3009::playExtVideo() {
	setGlobalVar(0x20580A86, _cannonLocation);
	setGlobalVar(0xF0402B0A, kScene3009CannonLocationFileHashes[_cannonLocation]);
	leaveScene(1);
}

bool Scene3009::isSymbolsPart1Solved() {
	for (int i = 0; i < 3; i++)
		if (_correctSymbols[i] != getSubVar(0x00000914, i))
			return false;
	return true;
}

bool Scene3009::isSymbolsPart2Solved() {
	for (int i = 3; i < 6; i++)
		if (_correctSymbols[i] != getSubVar(0x00000914, i))
			return false;
	return true;
}

bool Scene3009::sub462E90() {
	return _flag4;
}

// Scene3010

static const uint32 kScene3010ButtonNameHashes[] = {
	0x304008D2,
	0x40119852,
	0x01180951
};

static const uint32 kScene3010DeadBoltButtonFileHashes1[] = {
	0x301024C2,
	0x20280580,
	0x30200452
};

static const uint32 kScene3010DeadBoltButtonFileHashes2[] = {
	0x50C025A8,
	0x1020A0A0,
	0x5000A7E8
};

static const NPoint kAsScene3010DeadBoltPoints[] = {
	{550, 307},
	{564, 415},
	{560, 514}
};

static const uint32 kAsScene3010DeadBoltFileHashes2[] = {
	0x181A0042,
	0x580A08F2,
	0x18420076
};

static const uint32 kAsScene3010DeadBoltFileHashes1[] = {
	0x300E105A,
	0x804E0052,
	0x040E485A
};

SsScene3010DeadBoltButton::SsScene3010DeadBoltButton(NeverhoodEngine *vm, Scene *parentScene, int buttonIndex, int initCountdown, bool initDisabled)
	: StaticSprite(vm, 900), _parentScene(parentScene), _soundResource1(vm),
	_soundResource2(vm), _soundResource3(vm), _buttonLocked(false), _countdown1(0), 
	_countdown2(0), _buttonIndex(buttonIndex) {

	NDimensions dimensions1, dimensions2;
	 
	_buttonEnabled = getSubVar(0x14800353, kScene3010ButtonNameHashes[_buttonIndex]) != 0;
	_spriteResource.load2(kScene3010DeadBoltButtonFileHashes1[_buttonIndex]);
	dimensions1 = _spriteResource.getDimensions();
	_spriteResource.load2(kScene3010DeadBoltButtonFileHashes2[_buttonIndex]);
	dimensions2 = _spriteResource.getDimensions();
	createSurface(400, 
		MAX(dimensions1.width, dimensions2.width),
		MAX(dimensions1.height, dimensions2.height));
	setSprite(kScene3010DeadBoltButtonFileHashes2[_buttonIndex]);
	if (initDisabled) {
		disableButton();
	} else if (_buttonEnabled) {
		_countdown1 = initCountdown * 12 + 1;
	}
	_soundResource1.load(0xF4217243);
	_soundResource2.load(0x44049000);
	_soundResource3.load(0x6408107E);
	SetUpdateHandler(&SsScene3010DeadBoltButton::update);
	SetMessageHandler(&SsScene3010DeadBoltButton::handleMessage);
}

void SsScene3010DeadBoltButton::update() {

	if (_countdown1 != 0 && (--_countdown1 == 0)) {
		_soundResource1.play();
		setVisible(false);
		setSprite(kScene3010DeadBoltButtonFileHashes1[_buttonIndex]);
	}

	if (_countdown2 != 0 && (--_countdown2 == 0)) {
		setVisible(true);
		setSprite(kScene3010DeadBoltButtonFileHashes2[_buttonIndex]);
	}

}

uint32 SsScene3010DeadBoltButton::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (!_buttonLocked && _countdown1 == 0) {
			if (_buttonEnabled) {
				_soundResource2.play();
				_soundResource3.play();
				setVisible(true);
				_buttonLocked = true;
				sendMessage(_parentScene, 0x2000, _buttonIndex);
			} else {
				sendMessage(_parentScene, 0x2002, _buttonIndex);
			}
			_needRefresh = true;
			StaticSprite::update();
		}
		messageResult = 1;
		break;
	}
	return messageResult;
}

void SsScene3010DeadBoltButton::disableButton() {
	_buttonLocked = true;
	setSprite(kScene3010DeadBoltButtonFileHashes1[_buttonIndex]);
	setVisible(true);
}

void SsScene3010DeadBoltButton::setSprite(uint32 fileHash) {
	_spriteResource.load(fileHash);
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_deltaRect = _drawRect;
	processDelta();
	_needRefresh = true;
	StaticSprite::update();
}

void SsScene3010DeadBoltButton::setCountdown(int count) {
	_countdown2 = count * 18 + 1;
}

AsScene3010DeadBolt::AsScene3010DeadBolt(NeverhoodEngine *vm, Scene *parentScene, int boltIndex, bool initUnlocked)
	: AnimatedSprite(vm, 1100), _soundResource1(vm), _soundResource2(vm), _soundResource3(vm),
	_parentScene(parentScene), _boltIndex(boltIndex), _soundToggle(true), _unlocked(false), _locked(false),
	_countdown(0) {

	_x = kAsScene3010DeadBoltPoints[_boltIndex].x;
	_y = kAsScene3010DeadBoltPoints[_boltIndex].y;

	if (getSubVar(0x14800353, kScene3010ButtonNameHashes[_boltIndex])) {
		createSurface1(kAsScene3010DeadBoltFileHashes1[_boltIndex], 1200);
		startAnimation(kAsScene3010DeadBoltFileHashes1[_boltIndex], 0, -1);
		_soundResource1.load(0x46005BC4);
	} else {
		createSurface1(kAsScene3010DeadBoltFileHashes2[_boltIndex], 1200);
		startAnimation(kAsScene3010DeadBoltFileHashes2[_boltIndex], 0, -1);
		_soundResource1.load(0x420073DC);
		_soundResource2.load(0x420073DC);
	}
	
	setVisible(false);
	stIdle();
	if (initUnlocked)
		unlock(true);

	_needRefresh = true;
	AnimatedSprite::updatePosition();
	
}

void AsScene3010DeadBolt::update() {
	updateAnim();
	updatePosition();
	if (_countdown != 0 && (--_countdown == 0)) {
		stDisabled();
	}
}

uint32 AsScene3010DeadBolt::hmAnimation(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x3002:
		gotoNextState();
		break;
	}
	return messageResult;
}

void AsScene3010DeadBolt::stIdle() {
	stopAnimation();
	SetUpdateHandler(&AsScene3010DeadBolt::update);
	SetMessageHandler(&Sprite::handleMessage);
	_locked = false;
}

void AsScene3010DeadBolt::unlock(bool skipAnim) {
	if (!_unlocked) {
		setVisible(true);
		if (skipAnim) {
			startAnimation(kAsScene3010DeadBoltFileHashes1[_boltIndex], -1, 0);
			_newStickFrameIndex = -2;
		} else {
			startAnimation(kAsScene3010DeadBoltFileHashes1[_boltIndex], 0, -1);
			SetMessageHandler(&AsScene3010DeadBolt::hmAnimation);
			FinalizeState(&AsScene3010DeadBolt::stIdleMessage);
			NextState(&AsScene3010DeadBolt::stIdle);
			_soundResource1.play();
		}
		_unlocked = true;
		_soundResource3.load(0x4010C345);
	}
}

void AsScene3010DeadBolt::stIdleMessage() {
	stopAnimation();
	SetMessageHandler(&Sprite::handleMessage);
	sendMessage(_parentScene, 0x2001, _boltIndex);
}

void AsScene3010DeadBolt::lock() {
	if (!_locked) {
		_locked = true;
		setVisible(true);
		startAnimation(kAsScene3010DeadBoltFileHashes2[_boltIndex], 0, -1);
		SetMessageHandler(&AsScene3010DeadBolt::hmAnimation);
		FinalizeState(&AsScene3010DeadBolt::stDisabledMessage);
		NextState(&AsScene3010DeadBolt::stIdle);
		if (_soundToggle) {
			_soundResource1.play();
		} else {
			_soundResource2.play();
		}
		_soundToggle = !_soundToggle;
	}
}

void AsScene3010DeadBolt::setCountdown(int count) {
	_countdown = count * 18 + 1;
}

void AsScene3010DeadBolt::stDisabled() {
	setVisible(true);
	startAnimation(kAsScene3010DeadBoltFileHashes1[_boltIndex], 0, -1);
	SetMessageHandler(&AsScene3010DeadBolt::hmAnimation);
	FinalizeState(&AsScene3010DeadBolt::stDisabledMessage);
	NextState(&AsScene3010DeadBolt::stIdle);
	_playBackwards = true;
	_soundResource3.play();
}

void AsScene3010DeadBolt::stDisabledMessage() {
	setVisible(false);
	sendMessage(_parentScene, 0x2003, _boltIndex);
}

Scene3010::Scene3010(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _soundResource(vm), _countdown(0),
	_doorUnlocked(false), _checkUnlocked(false) {
	
	int initCountdown = 0;

	// DEBUG: Enable all buttons
	setSubVar(0x14800353, kScene3010ButtonNameHashes[0], 1);
	setSubVar(0x14800353, kScene3010ButtonNameHashes[1], 1);
	setSubVar(0x14800353, kScene3010ButtonNameHashes[2], 1);

	_surfaceFlag = true;

	setBackground(0x80802626);
	setPalette(0x80802626);

	for (int i = 0; i < 3; i++) {
		_asDeadBolts[i] = insertSprite<AsScene3010DeadBolt>(this, i, which == 1);//CHECKME
		_ssDeadBoltButtons[i] = insertSprite<SsScene3010DeadBoltButton>(this, i, initCountdown, which == 1);//CHECKME
		_vm->_collisionMan->addSprite(_ssDeadBoltButtons[i]);
		if (getSubVar(0x14800353, kScene3010ButtonNameHashes[i]))
			initCountdown++;
		_boltUnlocking[i] = false;
		_boltUnlocked[i] = false;
	}

	if (which == 0) {
		insertMouse435(0x02622800, 20, 620);
	}

	_soundResource.load(0x68E25540);

	SetMessageHandler(&Scene3010::handleMessage);
	SetUpdateHandler(&Scene3010::update);

	if (which == 1) {
		_checkUnlocked = true;
		for (int i = 0; i < 3; i++) {
			_boltUnlocked[i] = true;
			_ssDeadBoltButtons[i]->setCountdown(i + 1);
			_asDeadBolts[i]->setCountdown(i + 1);
		}
	}

}

void Scene3010::update() {
	Scene::update();
	if (_checkUnlocked && !_boltUnlocked[0] && !_boltUnlocked[1] && !_boltUnlocked[2]) {
		_countdown = 24;
		_checkUnlocked = false;
	}
	if (_countdown != 0 && (--_countdown == 0)) {
		leaveScene(_doorUnlocked ? 1 : 0);
	}
}

uint32 Scene3010::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		// TODO: Debug stuff
		if ((param.asPoint().x <= 20 || param.asPoint().x >= 620) && _countdown == 0 && !_checkUnlocked) {
			if (!_boltUnlocking[0] && !_boltUnlocking[1] && !_boltUnlocking[2]) {
				showMouse(false);
				if (!_boltUnlocked[0] && !_boltUnlocked[1] && !_boltUnlocked[2]) {
					_countdown = 1;
				} else {
					_checkUnlocked = true;
					for (int i = 0; i < 3; i++) {
						_ssDeadBoltButtons[i]->setCountdown(i);
						if (_boltUnlocked[i]) {
							_asDeadBolts[i]->setCountdown(i);
						}
					}
				}
			}
		}
		break;
	case 0x000D:
		// TODO: Debug stuff
		break;
	case 0x2000:
		if (!_boltUnlocked[param.asInteger()] && !_checkUnlocked && _countdown == 0) {
			_asDeadBolts[param.asInteger()]->unlock(false);
			_boltUnlocking[param.asInteger()] = true;
		}
		break;
	case 0x2001:
		_boltUnlocked[param.asInteger()] = true;
		_boltUnlocking[param.asInteger()] = false;
		if (_boltUnlocked[0] && _boltUnlocked[1] && _boltUnlocked[2]) {
			if (!getGlobalVar(0x00040153)) {
				setGlobalVar(0x00040153, 1);
				_soundResource.play();
				_countdown = 60;
			} else {
				_countdown = 48;
			}
			_doorUnlocked = true;
		}
		break;
	case 0x2002:
		if (!_checkUnlocked && _countdown == 0) {
			_asDeadBolts[param.asInteger()]->lock();
		}
		break;
	case 0x2003:
		_boltUnlocked[param.asInteger()] = false;
		break;
	}
	return 0;
}

// Scene3011

static const uint32 kAsScene3011SymbolFileHashes[] = {
	0x00C88050,
	0x01488050,
	0x02488050,
	0x04488050,
	0x08488050,
	0x10488050,
	0x20488050,
	0x40488050,
	0x80488050,
	0x00488051,
	0x00488052,
	0x00488054,
	0x008B0000,
	0x008D0000,
	0x00810000,
	0x00990000,
	0x00A90000,
	0x00C90000,
	0x00090000,
	0x01890000,
	0x02890000,
	0x04890000,
	0x08890000,
	0x10890000
};

SsScene3011Button::SsScene3011Button(NeverhoodEngine *vm, Scene *parentScene, bool flag)
	: StaticSprite(vm, 1400), _parentScene(parentScene), _soundResource(vm),
	_countdown(0) {
	
	if (flag) {
		_spriteResource.load2(0x11282020);
	} else {
		_spriteResource.load2(0x994D0433);
	}
	_soundResource.load(0x44061000);
	createSurface(400, _spriteResource.getDimensions().width, _spriteResource.getDimensions().height);
	_x = _spriteResource.getPosition().x;
	_y = _spriteResource.getPosition().y;
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = _spriteResource.getDimensions().width;
	_drawRect.height = _spriteResource.getDimensions().height;
	_deltaRect = _drawRect;
	setVisible(false);
	processDelta();
	_needRefresh = true;
	SetUpdateHandler(&SsScene3011Button::update);
	SetMessageHandler(&SsScene3011Button::handleMessage);
}

void SsScene3011Button::update() {
	StaticSprite::update();
	if (_countdown != 0 && (--_countdown == 0)) {
		setVisible(false);
	}
}

uint32 SsScene3011Button::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = 0;
	StaticSprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_countdown == 0) {
			setVisible(true);
			_countdown = 4;
			sendMessage(_parentScene, 0x2000, 0);
			_soundResource.play();
		}
		messageResult = 1;
		break;
	}
	return messageResult;
}

AsScene3011Symbol::AsScene3011Symbol(NeverhoodEngine *vm, int index, bool flag)
	: AnimatedSprite(vm, 1000), _soundResource1(vm), _soundResource2(vm),
	_index(index), _flag1(flag), _flag2(false) {

	if (flag) {
		_x = 310;
		_y = 200;
		createSurface1(kAsScene3011SymbolFileHashes[_index], 1200);
		_soundResource1.load(0x6052C60F);
		_soundResource2.load(0x6890433B);
	} else {
		_index = 12;
		_x = index * 39 + 96;
		_y = 225;
		createSurface(1200, 41, 48);
		_soundResource1.load(0x64428609);
		_soundResource2.load(0x7080023B);
	}
	setVisible(false);
	_needRefresh = true;
	SetUpdateHandler(&AnimatedSprite::update);
}

void AsScene3011Symbol::show(bool flag) {
	_flag2 = flag;
	startAnimation(kAsScene3011SymbolFileHashes[_index], 0, -1);
	setVisible(true);
	if (flag) {
		_soundResource2.play();
	} else {
		_soundResource1.play();
	}
}

void AsScene3011Symbol::hide() {
	stopAnimation();
	setVisible(false);
}

void AsScene3011Symbol::stopSound() {
	if (_flag2) {
		_soundResource2.stop();
	} else {
		_soundResource2.stop();
	}
}

void AsScene3011Symbol::change(int index, bool flag) {
	_index = index;
	_flag2 = flag;
	startAnimation(kAsScene3011SymbolFileHashes[_index], 0, -1);
	setVisible(true);
	if (flag) {
		_soundResource2.play();
	} else {
		_soundResource1.play();
	}
}

Scene3011::Scene3011(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule, true), _updateStatus(0), _buttonClicked(false), _index2(0) {

	// TODO _vm->gameModule()->initScene3011Vars();
	_index1 = getGlobalVar(0x2414C2F2);

	_surfaceFlag = true;
	SetMessageHandler(&Scene3011::handleMessage);
	SetUpdateHandler(&Scene3011::update);
	
	setBackground(0x92124A04);
	setPalette(0xA4070114);
	addEntity(_palette);

	insertMouse435(0x24A00929, 20, 620);

	for (int i = 0; i < 12; i++)
		_asSymbols[i] = insertSprite<AsScene3011Symbol>(i, true);

	_ssButton = insertSprite<SsScene3011Button>(this, true);
	_vm->_collisionMan->addSprite(_ssButton);
	
}

void Scene3011::update() {
	Scene::update();
	
	if (_countdown != 0 && (--_countdown == 0)) {
		switch (_updateStatus) {
		case 0:
			if (_buttonClicked) {
				if (_index1 == _index2) {
					do {
						_index3 = _vm->_rnd->getRandomNumber(12 - 1);
					} while (_index1 == _index3);
					_asSymbols[getSubVar(0x04909A50, _index3)]->show(true);
				} else {
					_asSymbols[getSubVar(0x04909A50, _index2)]->show(false);
				}
				_updateStatus = 1;
				_countdown = 24;
				fadeIn();
				_buttonClicked = false;
			}
			break;
		case 1:
			_updateStatus = 2;
			_countdown = 24;
			break;
		case 2:
			fadeOut();
			_updateStatus = 3;
			_countdown = 24;
			break;
		case 3:
			_updateStatus = 0;
			_countdown = 1;
			if (_index1 == _index2) {
				_asSymbols[getSubVar(0x04909A50, _index3)]->hide();
			} else {
				_asSymbols[getSubVar(0x04909A50, _index2)]->hide();
			}
			_index2++;
			if (_index2 >= 12)
				_index2 = 0;
			break;
		}
	}
}

uint32 Scene3011::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x0001:
		if (param.asPoint().x <= 20 || param.asPoint().x >= 620) {
			leaveScene(0);
		}
		break;
	case 0x2000:
		_buttonClicked = true;
		if (_countdown == 0)
			_countdown = 1;
		break;
	}
	return 0;
}

void Scene3011::fadeIn() {
	_palette->addBasePalette(0x92124A04, 0, 256, 0);
	_palette->startFadeToPalette(24);
}

void Scene3011::fadeOut() {
	_palette->addBasePalette(0xA4070114, 0, 256, 0);
	_palette->startFadeToPalette(24);
}

} // End of namespace Neverhood
