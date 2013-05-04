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

#include "neverhood/modules/module3000.h"
#include "neverhood/gamemodule.h"
#include "neverhood/navigationscene.h"

namespace Neverhood {

static const uint32 kModule3000SoundList[] = {
	0x92025040,
	0x90035066,
	0x90815450,
	0x99801500,
	0x90E14440,
	0x16805048,
	0x90F0D1C3,
	0
};

Module3000::Module3000(NeverhoodEngine *vm, Module *parentModule, int which)
	: Module(vm, parentModule), _soundVolume(0) {
	
	_vm->_soundMan->addSoundList(0x81293110, kModule3000SoundList);
	_vm->_soundMan->setSoundListParams(kModule3000SoundList, true, 50, 600, 5, 150);
	_vm->_soundMan->setSoundParams(0x90F0D1C3, false, 20000, 30000, 20000, 30000);
	_vm->_soundMan->playTwoSounds(0x81293110, 0x48498E46, 0x50399F64, 0);
	_vm->_soundMan->playTwoSounds(0x81293110, 0x40030A51, 0xC862CA15, 0);
	_vm->_soundMan->playTwoSounds(0x81293110, 0x41861371, 0x43A2507F, 0);

	_isWallBroken = getGlobalVar(V_WALL_BROKEN) != 0;

	if (_isWallBroken) {
		_vm->_soundMan->setSoundVolume(0x90F0D1C3, 0);
		_vm->_soundMan->playSoundLooping(0x90F0D1C3);
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
	_vm->_soundMan->deleteGroup(0x81293110);
}

void Module3000::createScene(int sceneNum, int which) {
	static const byte kNavigationTypes05[] = {3, 0};
	static const byte kNavigationTypes06[] = {5};
	debug("Module3000::createScene(%d, %d)", sceneNum, which);
	_vm->gameState().sceneNum = sceneNum;
	switch (_vm->gameState().sceneNum) {
	case 1:
		if (!getGlobalVar(V_BOLT_DOOR_OPEN)) {
			createNavigationScene(0x004B7C80, which);
		} else if (getGlobalVar(V_WALL_BROKEN)) {
			createNavigationScene(0x004B7CE0, which);
		} else {
			createNavigationScene(0x004B7CB0, which);
		}
		break;
	case 2:
		_vm->_soundMan->playTwoSounds(0x81293110, 0x40030A51, 0xC862CA15, 0);
		if (_isWallBroken) {
			_soundVolume = 90;
			_vm->_soundMan->setSoundVolume(0x90F0D1C3, 90);
		}
		if (getGlobalVar(V_WALL_BROKEN)) {
			createNavigationScene(0x004B7D58, which);
		} else {
			createNavigationScene(0x004B7D10, which);
		}
		break;
	case 3:
		if (getGlobalVar(V_STAIRS_DOWN))
			createNavigationScene(0x004B7E60, which);
		else if (getGlobalVar(V_WALL_BROKEN))
			createNavigationScene(0x004B7DA0, which);
		else
			createNavigationScene(0x004B7E00, which);
		break;
	case 4:
		if (getGlobalVar(V_STAIRS_DOWN))
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
		_vm->_soundMan->setSoundListParams(kModule3000SoundList, false, 0, 0, 0, 0);
		if (!getSubVar(VA_IS_PUZZLE_INIT, 0x089809C2)) {
			setSubVar(VA_IS_PUZZLE_INIT, 0x089809C2, 1);
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
		_vm->_soundMan->setSoundListParams(kModule3000SoundList, false, 0, 0, 0, 0);
		if (!getSubVar(VA_IS_PUZZLE_INIT, 0x10130993)) {
			setSubVar(VA_IS_PUZZLE_INIT, 0x10130993, 1);
			createSmackerScene(0x31093019, true, true, false);
		} else
			createSmackerScene(0x20093019, true, true, false);
		break;
	case 12:
		_childObject = new Scene3010(_vm, this, 1);
		break;
	// NOTE: Newly introduced sceneNums
	case 1001:
		if (!getGlobalVar(V_BOLT_DOOR_OPEN))
			if (getGlobalVar(V_WALL_BROKEN))
				createSmackerScene(0x00940021, true, true, false);
			else
				createSmackerScene(0x01140021, true, true, false);
		else
			if (getGlobalVar(V_WALL_BROKEN))
				createSmackerScene(0x001011B1, true, true, false);
			else
				createSmackerScene(0x001021B1, true, true, false);
		setGlobalVar(V_BOLT_DOOR_OPEN, getGlobalVar(V_BOLT_DOOR_OPEN) ? 0 : 1);
		break;
	case 1006:
		createSmackerScene(0x080810C5, true, true, false);
		break;
	case 1008:
		createSmackerScene(getGlobalVar(V_CANNON_SMACKER_NAME), true, true, false);
		break;
	}
	SetUpdateHandler(&Module3000::updateScene);
	_childObject->handleUpdate();
}

void Module3000::updateScene() {
	if (!updateChild()) {
		switch (_vm->gameState().sceneNum) {
		case 1:
			if (!getGlobalVar(V_BOLT_DOOR_OPEN)) {
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
			_vm->_soundMan->playTwoSounds(0x81293110, 0x41861371, 0x43A2507F, 0);
			if (_isWallBroken) {
				_soundVolume = 0;
				_vm->_soundMan->setSoundVolume(0x90F0D1C3, 0);
			}
			if (_moduleResult == 0) {
				createScene(3, 0);
			} else if (_moduleResult == 1) {
				setGlobalVar(V_BOLT_DOOR_OPEN, 0);
				createScene(1, 1);
			}
			break;
		case 3:
			if (_moduleResult == 1)
				createScene(4, 0);
			else if (_moduleResult == 3)
				createScene(10, -1);
			else if (getGlobalVar(V_STAIRS_DOWN))
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
			_isWallBroken = getGlobalVar(V_WALL_BROKEN) != 0;
			if (_moduleResult != 1) {
				_vm->_soundMan->setSoundListParams(kModule3000SoundList, true, 0, 0, 0, 0);
				createScene(4, 1);
			} else if (getGlobalVar(V_CANNON_SMACKER_NAME)) {
				createScene(1008, -1);
			} else {
				_vm->_soundMan->setSoundListParams(kModule3000SoundList, true, 0, 0, 0, 0);
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
			if (getGlobalVar(V_BOLT_DOOR_OPEN))
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
			if (navigationScene()->isWalkingForward()) {
				uint32 frameNumber = navigationScene()->getFrameNumber();
				int navigationIndex = navigationScene()->getNavigationIndex();
				if (navigationIndex == 1) {
					if (frameNumber == 0) {
						_vm->_soundMan->playTwoSounds(0x81293110, 0x48498E46, 0x50399F64, 0);
						_vm->_soundMan->setSoundVolume(0x48498E46, 70);
						_vm->_soundMan->setSoundVolume(0x50399F64, 70);
					} else if (frameNumber == 100) {
						_vm->_soundMan->playTwoSounds(0x81293110, 0x41861371, 0x43A2507F, 0);
					}
				} else if (navigationIndex == 0) {
					if (frameNumber == 0) {
						_vm->_soundMan->playTwoSounds(0x81293110, 0x48498E46, 0x50399F64, 0);
						_vm->_soundMan->setSoundVolume(0x48498E46, 70);
						_vm->_soundMan->setSoundVolume(0x50399F64, 70);
					} else if (frameNumber == 10) {
						_vm->_soundMan->playTwoSounds(0x81293110, 0x40030A51, 0xC862CA15, 0);
					}
					if (_isWallBroken && _soundVolume < 90 && frameNumber % 2) {
						if (frameNumber == 0)
							_soundVolume = 40;
						else
							_soundVolume++;
						_vm->_soundMan->setSoundVolume(0x90F0D1C3, _soundVolume);
					}
				}
			}
			break;
		case 2:
			if (navigationScene()->isWalkingForward()) {
				uint32 frameNumber = navigationScene()->getFrameNumber();
				int navigationIndex = navigationScene()->getNavigationIndex();
				if (_isWallBroken && _soundVolume > 1 && frameNumber % 2) {
					_soundVolume--;
					_vm->_soundMan->setSoundVolume(0x90F0D1C3, _soundVolume);
				}
				if (navigationIndex == 0) {
					if (frameNumber == 35) {
						_vm->_soundMan->playTwoSounds(0x81293110, 0x41861371, 0x43A2507F, 0);
					}
				} else if (navigationIndex == 1) {
					if (frameNumber == 55) {
						_vm->_soundMan->playTwoSounds(0x81293110, 0x48498E46, 0x50399F64, 0);
						_vm->_soundMan->setSoundVolume(0x48498E46, 70);
						_vm->_soundMan->setSoundVolume(0x50399F64, 70);
					}
				}
			}
			break;
		case 3:
			if (navigationScene()->isWalkingForward()) {
				uint32 frameNumber = navigationScene()->getFrameNumber();
				int navigationIndex = navigationScene()->getNavigationIndex();
				if (navigationIndex == 2) {
					if (frameNumber == 40) {
						_vm->_soundMan->playTwoSounds(0x81293110, 0x40030A51, 0xC862CA15, 0);
					}
					if (_isWallBroken && _soundVolume < 90 && frameNumber % 2) {
						if (frameNumber == 0)
							_soundVolume = 40;
						else
							_soundVolume++;
						_vm->_soundMan->setSoundVolume(0x90F0D1C3, _soundVolume);
					}
				}
			}
			break;
		case 5:
			if (navigationScene()->isWalkingForward() && navigationScene()->getNavigationIndex() == 0) {
				_vm->_soundMan->setTwoSoundsPlayFlag(false);
			}
			break;
		}
	}
}

// Scene3009

enum {
	kCTSNull				= 0,
	kCTSBreakWall			= 1,
	kCTSWall				= 2,
	kCTSEmptyness			= 3,
	kCTSFireRobotNoTarget	= 4,
	kCTSFireRobotIsTarget	= 5,
	kCTSFireNoRobot			= 6,
	kCTSRaiseCannon			= 7,
	kCTSRightRobotNoTarget	= 8,
	kCTSRightRobotIsTarget	= 9,
	kCTSRightNoRobot		= 10,
	kCTSLeftRobotNoTarget	= 11,
	kCTSLeftRobotIsTarget	= 12,
	kCTSLeftNoRobot			= 13,
	kCTSLowerCannon			= 14,
	kCTSCount				= 14
};

static const uint32 kScene3009CannonScopeVideos[] = {
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

static const uint32 kScene3009CannonActionVideos[] = {
	0x00000000,
	0x8004001B,	// 1 Fire cannon at wall, it breaks (lowered)
	0x0004001A,	// 2 Fire cannon at wall, nothing happens (lowered)
	0x1048404B,	// 3 Fire cannon at emptyness (raised)
	0x50200109,	// 4 Fire cannon, robot missed (raised)
	0x12032109,	// 5 Fire cannon, robot hit (raised)
	0x10201109,	// 6 Fire cannon, no robot (raised)
	0x000A2030,	// 7 Raise the cannon
	0x000A0028,	// 8
	0x000A0028,	// 9
	0x000A0028,	// 10
	0x040A1069,	// 11
	0x040A1069,	// 12
	0x040A1069,	// 13
	0x240A1101	// 14 Lower the cannon
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
	: StaticSprite(vm, 1400), _parentScene(parentScene), _isClicked(false) {
	
	loadSprite(0x120B24B0, kSLFDefDrawOffset | kSLFDefPosition | kSLFDefCollisionBoundsOffset, 400);
	setVisible(false);
	SetUpdateHandler(&SsScene3009FireCannonButton::update);
	SetMessageHandler(&SsScene3009FireCannonButton::handleMessage);
	loadSound(0, 0x3901B44F);
}

void SsScene3009FireCannonButton::update() {
	updatePosition();
	if (_isClicked && !isSoundPlaying(0)) {
		sendMessage(_parentScene, 0x2000, 0);
		setVisible(false);
	}
}

uint32 SsScene3009FireCannonButton::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (!_isClicked && !_parentScene->isTurning()) {
			_isClicked = true;
			setVisible(true);
			playSound(0);
		}
		messageResult = 1;
		break;
	}
	return messageResult;
}

SsScene3009SymbolEdges::SsScene3009SymbolEdges(NeverhoodEngine *vm, int index)
	: StaticSprite(vm, 1400), _blinkCountdown(0) {

	loadSprite(kSsScene3009SymbolEdgesFileHashes[index], kSLFDefDrawOffset | kSLFDefPosition, 600);
	if (getGlobalVar(V_ROBOT_HIT))
		hide();
	else
		startBlinking();
	SetUpdateHandler(&SsScene3009SymbolEdges::update);
}

void SsScene3009SymbolEdges::update() {
	if (_blinkCountdown != 0 && (--_blinkCountdown == 0)) {
		if (_blinkToggle) {
			setVisible(true);
		} else {
			setVisible(false);
		}
		updatePosition();
		_blinkCountdown = 3;
		_blinkToggle = !_blinkToggle;
	}
}

void SsScene3009SymbolEdges::show() {
	setVisible(true);
	updatePosition();
	_blinkCountdown = 0;
}

void SsScene3009SymbolEdges::hide() {
	setVisible(false);
	updatePosition();
	_blinkCountdown = 0;
}

void SsScene3009SymbolEdges::startBlinking() {
	setVisible(true);
	updatePosition();
	_blinkCountdown = 3;
	_blinkToggle = true;
}

SsScene3009TargetLine::SsScene3009TargetLine(NeverhoodEngine *vm, int index)
	: StaticSprite(vm, 1400) {

	loadSprite(kSsScene3009TargetLineFileHashes[index], kSLFDefDrawOffset | kSLFDefPosition, 600);
	setVisible(false);
}

void SsScene3009TargetLine::show() {
	setVisible(true);
	updatePosition();
}

SsScene3009SymbolArrow::SsScene3009SymbolArrow(NeverhoodEngine *vm, Sprite *asSymbol, int index)
	: StaticSprite(vm, 1400), _asSymbol(asSymbol), _index(index), _enabled(true), _countdown(0) {

	_incrDecr = _index % 2;

	createSurface(1200, 33, 31);
	loadSprite(kSsScene3009SymbolArrowFileHashes2[_index], kSLFDefPosition);
	_drawOffset.set(0, 0, 33, 31);
	_collisionBoundsOffset = _drawOffset;
	updateBounds();
	_needRefresh = true;

	SetUpdateHandler(&SsScene3009SymbolArrow::update);
	SetMessageHandler(&SsScene3009SymbolArrow::handleMessage);
	loadSound(0, 0x2C852206);
}

void SsScene3009SymbolArrow::hide() {
	_enabled = false;
	setVisible(false);
}

void SsScene3009SymbolArrow::update() {
	updatePosition();
	if (_countdown != 0 && (--_countdown == 0)) {
		loadSprite(kSsScene3009SymbolArrowFileHashes2[_index], kSLFDefDrawOffset);
	}
}

uint32 SsScene3009SymbolArrow::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x1011:
		if (_enabled && _countdown == 0) {
			_countdown = 2;
			loadSprite(kSsScene3009SymbolArrowFileHashes1[_index], kSLFDefDrawOffset);
			playSound(0);
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
	_y = getGlobalVar(V_CANNON_RAISED) ? 52 : 266;
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

AsScene3009HorizontalIndicator::AsScene3009HorizontalIndicator(NeverhoodEngine *vm, Scene3009 *parentScene, uint32 cannonTargetStatus)
	: AnimatedSprite(vm, 1000), _parentScene(parentScene), _enabled(false) {
	
	_x = getGlobalVar(V_CANNON_TURNED) ? 533 : 92;
	_y = 150;
	createSurface1(0xC0C12954, 1200);
	_needRefresh = true;
	updatePosition();
	setVisible(false);
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene3009HorizontalIndicator::handleMessage);
	if (cannonTargetStatus == kCTSRightRobotNoTarget || cannonTargetStatus == kCTSRightRobotIsTarget || cannonTargetStatus == kCTSRightNoRobot) {
		SetSpriteUpdate(&AsScene3009HorizontalIndicator::suMoveRight);
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
		SetSpriteUpdate(NULL);
		_x = 92;
	}
}

void AsScene3009HorizontalIndicator::suMoveRight() {
	_x += 6;
	if (_x > 533) {
		SetSpriteUpdate(NULL);
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
	SetSpriteUpdate(&AsScene3009HorizontalIndicator::suMoveLeft);
}

void AsScene3009HorizontalIndicator::stMoveRight() {
	_x = 330;
	SetSpriteUpdate(&AsScene3009HorizontalIndicator::suMoveRight);
}

AsScene3009Symbol::AsScene3009Symbol(NeverhoodEngine *vm, Scene3009 *parentScene, int symbolPosition)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _symbolPosition(symbolPosition) {

	_symbolIndex = getSubVar(VA_CURR_CANNON_SYMBOLS, _symbolPosition);
	
	_x = kAsScene3009SymbolPoints[_symbolPosition].x;
	_y = kAsScene3009SymbolPoints[_symbolPosition].y;
	createSurface1(kAsScene3009SymbolFileHashes[_symbolPosition / 3], 1200);
	startAnimation(kAsScene3009SymbolFileHashes[_symbolPosition / 3], _symbolIndex, -1);
	_newStickFrameIndex = _symbolIndex;
	_needRefresh = true;
	updatePosition();
	SetUpdateHandler(&AnimatedSprite::update);
	SetMessageHandler(&AsScene3009Symbol::handleMessage);
	_ssArrowPrev = _parentScene->insertSprite<SsScene3009SymbolArrow>(this, _symbolPosition * 2 + 0);
	_parentScene->addCollisionSprite(_ssArrowPrev);
	_ssArrowNext = _parentScene->insertSprite<SsScene3009SymbolArrow>(this, _symbolPosition * 2 + 1);
	_parentScene->addCollisionSprite(_ssArrowNext);
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
		startAnimation(kAsScene3009SymbolFileHashes[_symbolPosition / 3], _symbolIndex, -1);
		_newStickFrameIndex = _symbolIndex;
		setSubVar(VA_CURR_CANNON_SYMBOLS, _symbolPosition, _symbolIndex);
		if (_symbolPosition / 3 == 0) {
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
	: Scene(vm, parentModule), _keepVideo(false), _moveCannonLeftFirst(false), 
	_isTurning(false), _lockSymbolsPart1Countdown(1), _lockSymbolsPart2Countdown(1) {

	_cannonTargetStatus = getGlobalVar(V_CANNON_TARGET_STATUS);
	
	_vm->gameModule()->initCannonSymbolsPuzzle();
	
	setGlobalVar(V_CANNON_SMACKER_NAME, 0);
	
	_vm->_screen->clear();
	
	setBackground(0xD000420C);
	setPalette(0xD000420C);
	insertPuzzleMouse(0x04208D08, 20, 620);

	_ssFireCannonButton = insertSprite<SsScene3009FireCannonButton>(this);
	addCollisionSprite(_ssFireCannonButton);

	_asVerticalIndicator = insertSprite<AsScene3009VerticalIndicator>(this, _cannonTargetStatus);
	addCollisionSprite(_asVerticalIndicator);

	_asHorizontalIndicator = insertSprite<AsScene3009HorizontalIndicator>(this, _cannonTargetStatus);
	addCollisionSprite(_asHorizontalIndicator);

	if (_cannonTargetStatus != kCTSNull && _cannonTargetStatus != kCTSRightRobotNoTarget && _cannonTargetStatus != kCTSRightRobotIsTarget && _cannonTargetStatus != kCTSRightNoRobot) {
		_keepVideo = true;
	} else {
		_keepVideo = false;
		if (_cannonTargetStatus != kCTSNull) {
			_asHorizontalIndicator->stMoveRight();
			_isTurning = true;
		}
	}

	_smackerPlayer = addSmackerPlayer(new SmackerPlayer(_vm, this, kScene3009CannonScopeVideos[_cannonTargetStatus], false, _keepVideo));
	_smackerPlayer->setDrawPos(89, 37);
	_palette->usePalette(); // Use it again since the SmackerPlayer overrides the usage

	insertStaticSprite(0x8540252C, 400);

	for (int i = 0; i < 2; i++) {
		_ssSymbolEdges[i] = insertSprite<SsScene3009SymbolEdges>(i);
		_ssTargetLines[i] = insertSprite<SsScene3009TargetLine>(i);
	}

	for (int symbolPosition = 0; symbolPosition < 6; symbolPosition++) {
		_asSymbols[symbolPosition] = insertSprite<AsScene3009Symbol>(this, symbolPosition);
		if (symbolPosition < 3)
			_correctSymbols[symbolPosition] = getSubVar(VA_GOOD_CANNON_SYMBOLS_1, symbolPosition);
		else
			_correctSymbols[symbolPosition] = getSubVar(VA_GOOD_CANNON_SYMBOLS_2, symbolPosition - 3);
	}

	SetMessageHandler(&Scene3009::handleMessage);
	SetUpdateHandler(&Scene3009::update);

	// DEBUG Enable to set the correct code
#if 0	
	for (int i = 0; i < 6; i++)
		setSubVar(VA_CURR_CANNON_SYMBOLS, i, _correctSymbols[i]);
	sendMessage(this, 0x2003, 0);
#endif   

}

void Scene3009::update() {
	Scene::update();
	
	if (!_keepVideo && _smackerPlayer->isDone() && _cannonTargetStatus <= kCTSCount) {
		switch (_cannonTargetStatus) {
		case kCTSNull:
		case kCTSLowerCannon:
			_smackerPlayer->open(0x340A0049, true);
			_palette->usePalette();
			_keepVideo = true;
			break;
		case kCTSRightRobotNoTarget:
			_smackerPlayer->open(0x0082080D, true);
			_palette->usePalette();
			_keepVideo = true;
			_isTurning = false;
			break;
		case kCTSRightRobotIsTarget:
			_smackerPlayer->open(0x0282080D, true);
			_palette->usePalette();
			_keepVideo = true;
			_isTurning = false;
			break;
		case kCTSRightNoRobot:
			_smackerPlayer->open(0x0882080D, true);
			_palette->usePalette();
			_keepVideo = true;
			_isTurning = false;
			break;
		case kCTSLeftRobotNoTarget:
		case kCTSLeftRobotIsTarget:
		case kCTSLeftNoRobot:
			if (_moveCannonLeftFirst) {
				if (_cannonTargetStatus == kCTSLeftRobotNoTarget)
					_smackerPlayer->open(0x110A000F, false);
				else if (_cannonTargetStatus == kCTSLeftRobotIsTarget)				
					_smackerPlayer->open(0x500B004F, false);
				else if (_cannonTargetStatus == kCTSLeftNoRobot)				
					_smackerPlayer->open(0x100B010E, false);
				_palette->usePalette();
				_moveCannonLeftFirst = false;
				_asHorizontalIndicator->stMoveLeft();
			} else {
				playActionVideo();
			}
			break;
		}
	}

	if (_lockSymbolsPart1Countdown != 0 && (--_lockSymbolsPart1Countdown == 0) && isSymbolsPart1Solved()) {
		for (int i = 0; i < 3; i++)
			_asSymbols[i]->hide();
		if (!getGlobalVar(V_ROBOT_HIT) || getGlobalVar(V_CANNON_RAISED) || getGlobalVar(V_CANNON_TURNED)) {
			_ssSymbolEdges[0]->show();
			_ssTargetLines[0]->show();
			_asVerticalIndicator->show();
		}
	}

	if (_lockSymbolsPart2Countdown != 0 && (--_lockSymbolsPart2Countdown == 0) && isSymbolsPart2Solved()) {
		for (int i = 3; i < 6; i++)
			_asSymbols[i]->hide();
		if (!getGlobalVar(V_ROBOT_HIT) || getGlobalVar(V_CANNON_RAISED) || getGlobalVar(V_CANNON_TURNED)) {
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
		if ((param.asPoint().x <= 20 || param.asPoint().x >= 620) && !getGlobalVar(V_CANNON_RAISED)) {
			setGlobalVar(V_CANNON_TARGET_STATUS, 0);
			leaveScene(0);
		}
		break;
	case 0x2000:
		if (!getGlobalVar(V_CANNON_RAISED)) {
			if (!getGlobalVar(V_WALL_BROKEN)) {
				_cannonTargetStatus = kCTSBreakWall;
				setGlobalVar(V_WALL_BROKEN, 1);
			} else {
				_cannonTargetStatus = kCTSWall;
			}
		} else if (!getGlobalVar(V_CANNON_TURNED)) {
			_cannonTargetStatus = kCTSEmptyness;
		} else if (!getGlobalVar(V_ROBOT_TARGET)) {
			_cannonTargetStatus = kCTSFireRobotNoTarget;
		} else if (!getGlobalVar(V_ROBOT_HIT)) {
			setGlobalVar(V_ROBOT_HIT, 1);
			_cannonTargetStatus = kCTSFireRobotIsTarget;
		} else {
			_cannonTargetStatus = kCTSFireNoRobot;
		}
		playActionVideo();
		break;
	case 0x2001:
		_lockSymbolsPart1Countdown = 24;
		break;
	case 0x2002:
		// Raise/lower the cannon
		if (!getGlobalVar(V_CANNON_TURNED) && !_isTurning) {
			if (getGlobalVar(V_CANNON_RAISED)) {
				_cannonTargetStatus = kCTSLowerCannon;
				setGlobalVar(V_CANNON_RAISED, 0);
			} else {
				_cannonTargetStatus = kCTSRaiseCannon;
				setGlobalVar(V_CANNON_RAISED, 1);
			}
			playActionVideo();
		}
		break;
	case 0x2003:
		_lockSymbolsPart2Countdown = 24;
		break;
	case 0x2004:
		// Turn the cannon if it's raised
		if (getGlobalVar(V_CANNON_RAISED)) {
			if (!getGlobalVar(V_CANNON_TURNED)) {
				// Cannon is at the left position
				if (!getGlobalVar(V_ROBOT_TARGET)) {
					_cannonTargetStatus = kCTSRightRobotNoTarget;
				} else if (!getGlobalVar(V_ROBOT_HIT)) {
					_cannonTargetStatus = kCTSRightRobotIsTarget;
				} else {
					_cannonTargetStatus = kCTSRightNoRobot;
				}
				setGlobalVar(V_CANNON_TURNED, 1);
				_isTurning = true;
				playActionVideo();
			} else {
				// Cannon is at the right position
				if (!getGlobalVar(V_ROBOT_TARGET)) {
					_cannonTargetStatus = kCTSLeftRobotNoTarget;
					_smackerPlayer->open(0x108A000F, false);
				} else if (!getGlobalVar(V_ROBOT_HIT)) {
					_cannonTargetStatus = kCTSLeftRobotIsTarget;
					_smackerPlayer->open(0x500B002F, false);
				} else {
					_cannonTargetStatus = kCTSLeftNoRobot;
					_smackerPlayer->open(0x100B008E, false);
				}
				_palette->usePalette();
				_moveCannonLeftFirst = true;
				_isTurning = true;
				_keepVideo = false;
				setGlobalVar(V_CANNON_TURNED, 0);
			}
		}
		break;
	}
	return 0;
}

void Scene3009::playActionVideo() {
	setGlobalVar(V_CANNON_TARGET_STATUS, _cannonTargetStatus);
	setGlobalVar(V_CANNON_SMACKER_NAME, kScene3009CannonActionVideos[_cannonTargetStatus]);
	leaveScene(1);
}

bool Scene3009::isSymbolsPart1Solved() {
	for (int i = 0; i < 3; i++)
		if (_correctSymbols[i] != getSubVar(VA_CURR_CANNON_SYMBOLS, i))
			return false;
	return true;
}

bool Scene3009::isSymbolsPart2Solved() {
	for (int i = 3; i < 6; i++)
		if (_correctSymbols[i] != getSubVar(VA_CURR_CANNON_SYMBOLS, i))
			return false;
	return true;
}

bool Scene3009::isTurning() {
	return _isTurning;
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
	: StaticSprite(vm, 900), _parentScene(parentScene), _buttonLocked(false), _countdown1(0), _countdown2(0), _buttonIndex(buttonIndex) {

	_buttonEnabled = getSubVar(VA_LOCKS_DISABLED, kScene3010ButtonNameHashes[_buttonIndex]) != 0;
	createSurface(400, 88, 95);
	setSprite(kScene3010DeadBoltButtonFileHashes2[_buttonIndex]);
	if (initDisabled)
		disableButton();
	else if (_buttonEnabled)
		_countdown1 = initCountdown * 12 + 1;
	loadSound(0, 0xF4217243);
	loadSound(1, 0x44049000);
	loadSound(2, 0x6408107E);
	SetUpdateHandler(&SsScene3010DeadBoltButton::update);
	SetMessageHandler(&SsScene3010DeadBoltButton::handleMessage);
}

void SsScene3010DeadBoltButton::update() {

	if (_countdown1 != 0 && (--_countdown1 == 0)) {
		playSound(0);
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
				playSound(1);
				playSound(2);
				setVisible(true);
				_buttonLocked = true;
				sendMessage(_parentScene, 0x2000, _buttonIndex);
			} else {
				sendMessage(_parentScene, 0x2002, _buttonIndex);
			}
			_needRefresh = true;
			updatePosition();
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
	loadSprite(fileHash, kSLFDefDrawOffset | kSLFDefPosition | kSLFDefCollisionBoundsOffset);
}

void SsScene3010DeadBoltButton::setCountdown(int count) {
	_countdown2 = count * 18 + 1;
}

AsScene3010DeadBolt::AsScene3010DeadBolt(NeverhoodEngine *vm, Scene *parentScene, int boltIndex, bool initUnlocked)
	: AnimatedSprite(vm, 1100), _parentScene(parentScene), _boltIndex(boltIndex), _soundToggle(true),
	_unlocked(false), _locked(false), _countdown(0) {

	_x = kAsScene3010DeadBoltPoints[_boltIndex].x;
	_y = kAsScene3010DeadBoltPoints[_boltIndex].y;

	if (getSubVar(VA_LOCKS_DISABLED, kScene3010ButtonNameHashes[_boltIndex])) {
		createSurface1(kAsScene3010DeadBoltFileHashes1[_boltIndex], 1200);
		startAnimation(kAsScene3010DeadBoltFileHashes1[_boltIndex], 0, -1);
		loadSound(0, 0x46005BC4);
	} else {
		createSurface1(kAsScene3010DeadBoltFileHashes2[_boltIndex], 1200);
		startAnimation(kAsScene3010DeadBoltFileHashes2[_boltIndex], 0, -1);
		loadSound(0, 0x420073DC);
		loadSound(1, 0x420073DC);
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
			_newStickFrameIndex = STICK_LAST_FRAME;
		} else {
			startAnimation(kAsScene3010DeadBoltFileHashes1[_boltIndex], 0, -1);
			SetMessageHandler(&AsScene3010DeadBolt::hmAnimation);
			FinalizeState(&AsScene3010DeadBolt::stIdleMessage);
			NextState(&AsScene3010DeadBolt::stIdle);
			playSound(0);
		}
		_unlocked = true;
		loadSound(2, 0x4010C345);
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
			playSound(0);
		} else {
			playSound(1);
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
	playSound(2);
}

void AsScene3010DeadBolt::stDisabledMessage() {
	setVisible(false);
	sendMessage(_parentScene, 0x2003, _boltIndex);
}

Scene3010::Scene3010(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule), _countdown(0), _doorUnlocked(false), _checkUnlocked(false) {
	
	int initCountdown = 0;

	// DEBUG Enable to activate all buttons
#if 0	
	setSubVar(VA_LOCKS_DISABLED, kScene3010ButtonNameHashes[0], 1);
	setSubVar(VA_LOCKS_DISABLED, kScene3010ButtonNameHashes[1], 1);
	setSubVar(VA_LOCKS_DISABLED, kScene3010ButtonNameHashes[2], 1);
#endif	

	setBackground(0x80802626);
	setPalette(0x80802626);

	for (int i = 0; i < 3; i++) {
		_asDeadBolts[i] = insertSprite<AsScene3010DeadBolt>(this, i, which == 1);//CHECKME
		_ssDeadBoltButtons[i] = insertSprite<SsScene3010DeadBoltButton>(this, i, initCountdown, which == 1);//CHECKME
		addCollisionSprite(_ssDeadBoltButtons[i]);
		if (getSubVar(VA_LOCKS_DISABLED, kScene3010ButtonNameHashes[i]))
			initCountdown++;
		_boltUnlocking[i] = false;
		_boltUnlocked[i] = false;
	}

	if (which == 0) {
		insertPuzzleMouse(0x02622800, 20, 620);
	}

	loadSound(0, 0x68E25540);

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
			if (!getGlobalVar(V_BOLT_DOOR_UNLOCKED)) {
				setGlobalVar(V_BOLT_DOOR_UNLOCKED, 1);
				playSound(0);
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
	: StaticSprite(vm, 1400), _parentScene(parentScene), _countdown(0) {
	
	loadSprite(flag ? 0x11282020 : 0x994D0433, kSLFDefDrawOffset | kSLFDefPosition | kSLFDefCollisionBoundsOffset, 400);
	setVisible(false);
	loadSound(0, 0x44061000);
	SetUpdateHandler(&SsScene3011Button::update);
	SetMessageHandler(&SsScene3011Button::handleMessage);
}

void SsScene3011Button::update() {
	updatePosition();
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
			playSound(0);
		}
		messageResult = 1;
		break;
	}
	return messageResult;
}

AsScene3011Symbol::AsScene3011Symbol(NeverhoodEngine *vm, int symbolIndex, bool largeSymbol)
	: AnimatedSprite(vm, 1000), _symbolIndex(symbolIndex), _largeSymbol(largeSymbol), _isNoisy(false) {

	if (_largeSymbol) {
		_x = 310;
		_y = 200;
		createSurface1(kAsScene3011SymbolFileHashes[_symbolIndex], 1200);
		loadSound(0, 0x6052C60F);
		loadSound(1, 0x6890433B);
	} else {
		_symbolIndex = 12;
		_x = symbolIndex * 39 + 96;
		_y = 225;
		createSurface(1200, 41, 48);
		loadSound(0, 0x64428609);
		loadSound(1, 0x7080023B);
	}
	setVisible(false);
	_needRefresh = true;
	SetUpdateHandler(&AnimatedSprite::update);
}

void AsScene3011Symbol::show(bool isNoisy) {
	_isNoisy = isNoisy;
	startAnimation(kAsScene3011SymbolFileHashes[_symbolIndex], 0, -1);
	setVisible(true);
	if (_isNoisy) {
		playSound(1);
	} else {
		playSound(0);
	}
}

void AsScene3011Symbol::hide() {
	stopAnimation();
	setVisible(false);
}

void AsScene3011Symbol::stopSymbolSound() {
	if (_isNoisy) {
		stopSound(1);
	} else {
		stopSound(0);
	}
}

void AsScene3011Symbol::change(int symbolIndex, bool isNoisy) {
	_symbolIndex = symbolIndex;
	_isNoisy = isNoisy;
	startAnimation(kAsScene3011SymbolFileHashes[_symbolIndex], 0, -1);
	setVisible(true);
	if (_isNoisy) {
		playSound(1);
	} else {
		playSound(0);
	}
}

Scene3011::Scene3011(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule), _updateStatus(0), _buttonClicked(false), _currentSymbolIndex(0), _countdown(0) {

	_vm->gameModule()->initCodeSymbolsPuzzle();
	_noisySymbolIndex = getGlobalVar(V_NOISY_SYMBOL_INDEX);

	SetMessageHandler(&Scene3011::handleMessage);
	SetUpdateHandler(&Scene3011::update);
	
	setBackground(0x92124A04);
	setPalette(0xA4070114);
	addEntity(_palette);

	insertPuzzleMouse(0x24A00929, 20, 620);

	for (int symbolIndex = 0; symbolIndex < 12; symbolIndex++)
		_asSymbols[symbolIndex] = insertSprite<AsScene3011Symbol>(symbolIndex, true);

	_ssButton = insertSprite<SsScene3011Button>(this, true);
	addCollisionSprite(_ssButton);
	
}

void Scene3011::update() {
	Scene::update();
	
	if (_countdown != 0 && (--_countdown == 0)) {
		switch (_updateStatus) {
		case 0:
			if (_buttonClicked) {
				if (_noisySymbolIndex == _currentSymbolIndex) {
					do {
						_noisyRandomSymbolIndex = _vm->_rnd->getRandomNumber(12 - 1);
					} while (_noisySymbolIndex == _noisyRandomSymbolIndex);
					_asSymbols[getSubVar(VA_CODE_SYMBOLS, _noisyRandomSymbolIndex)]->show(true);
				} else {
					_asSymbols[getSubVar(VA_CODE_SYMBOLS, _currentSymbolIndex)]->show(false);
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
			if (_noisySymbolIndex == _currentSymbolIndex) {
				_asSymbols[getSubVar(VA_CODE_SYMBOLS, _noisyRandomSymbolIndex)]->hide();
			} else {
				_asSymbols[getSubVar(VA_CODE_SYMBOLS, _currentSymbolIndex)]->hide();
			}
			_currentSymbolIndex++;
			if (_currentSymbolIndex >= 12)
				_currentSymbolIndex = 0;
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
