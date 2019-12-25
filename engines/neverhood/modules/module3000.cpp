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
#include "neverhood/modules/module3000.h"
#include "neverhood/modules/module3000_sprites.h"

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
	: Module(vm, parentModule), _waterfallSoundVolume(0) {

	_vm->_soundMan->addSoundList(0x81293110, kModule3000SoundList);
	_vm->_soundMan->setSoundListParams(kModule3000SoundList, true, 50, 600, 5, 150);
	_vm->_soundMan->setSoundParams(0x90F0D1C3, false, 20000, 30000, 20000, 30000);
	_vm->_soundMan->playTwoSounds(0x81293110, 0x48498E46, 0x50399F64, 0);
	_vm->_soundMan->playTwoSounds(0x81293110, 0x40030A51, 0xC862CA15, 0);
	_vm->_soundMan->playTwoSounds(0x81293110, 0x41861371, 0x43A2507F, 0);

	_isWaterfallRunning = getGlobalVar(V_WALL_BROKEN) != 1;

	if (_isWaterfallRunning) {
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
	static const byte kNavigationTypes05[] = {2, 0};
	static const byte kNavigationTypes06[] = {5};
	debug(1, "Module3000::createScene(%d, %d)", sceneNum, which);
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
		if (_isWaterfallRunning) {
			_waterfallSoundVolume = 90;
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
	default:
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
			if (_isWaterfallRunning) {
				_waterfallSoundVolume = 0;
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
			_isWaterfallRunning = getGlobalVar(V_WALL_BROKEN) != 1;
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
		default:
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
					if (_isWaterfallRunning && _waterfallSoundVolume < 90 && frameNumber % 2) {
						if (frameNumber == 0)
							_waterfallSoundVolume = 40;
						else
							_waterfallSoundVolume++;
						_vm->_soundMan->setSoundVolume(0x90F0D1C3, _waterfallSoundVolume);
					}
				}
			}
			break;
		case 2:
			if (navigationScene()->isWalkingForward()) {
				uint32 frameNumber = navigationScene()->getFrameNumber();
				int navigationIndex = navigationScene()->getNavigationIndex();
				if (_isWaterfallRunning && _waterfallSoundVolume > 1 && frameNumber % 2) {
					_waterfallSoundVolume--;
					_vm->_soundMan->setSoundVolume(0x90F0D1C3, _waterfallSoundVolume);
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
					if (_isWaterfallRunning && _waterfallSoundVolume < 90 && frameNumber % 2) {
						if (frameNumber == 0)
							_waterfallSoundVolume = 40;
						else
							_waterfallSoundVolume++;
						_vm->_soundMan->setSoundVolume(0x90F0D1C3, _waterfallSoundVolume);
					}
				}
			}
			break;
		case 5:
			if (navigationScene()->isWalkingForward() && navigationScene()->getNavigationIndex() == 0) {
				_vm->_soundMan->setTwoSoundsPlayFlag(false);
			}
			break;
		default:
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

	_cannonSmackerPlayer = addSmackerPlayer(new SmackerPlayer(_vm, this, kScene3009CannonScopeVideos[_cannonTargetStatus], false, _keepVideo));
	_cannonSmackerPlayer->setDrawPos(89, 37);
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
}

Scene3009::~Scene3009() {
}

void Scene3009::openSmacker(uint32 fileHash, bool keepLastFrame) {
	_cannonSmackerPlayer->open(fileHash, keepLastFrame);
	//_vm->_screen->setSmackerDecoder(_cannonSmackerPlayer->getSmackerDecoder());
	_palette->usePalette();
}

void Scene3009::update() {
	Scene::update();

	if (!_keepVideo && _cannonSmackerPlayer->isDone() && _cannonTargetStatus <= kCTSCount) {
		switch (_cannonTargetStatus) {
		case kCTSNull:
		case kCTSLowerCannon:
			openSmacker(0x340A0049, true);
			_keepVideo = true;
			break;
		case kCTSRightRobotNoTarget:
			openSmacker(0x0082080D, true);
			_keepVideo = true;
			_isTurning = false;
			break;
		case kCTSRightRobotIsTarget:
			openSmacker(0x0282080D, true);
			_keepVideo = true;
			_isTurning = false;
			break;
		case kCTSRightNoRobot:
			openSmacker(0x0882080D, true);
			_keepVideo = true;
			_isTurning = false;
			break;
		case kCTSLeftRobotNoTarget:
		case kCTSLeftRobotIsTarget:
		case kCTSLeftNoRobot:
			if (_moveCannonLeftFirst) {
				if (_cannonTargetStatus == kCTSLeftRobotNoTarget)
					openSmacker(0x110A000F, false);
				else if (_cannonTargetStatus == kCTSLeftRobotIsTarget)
					openSmacker(0x500B004F, false);
				else if (_cannonTargetStatus == kCTSLeftNoRobot)
					openSmacker(0x100B010E, false);
				_moveCannonLeftFirst = false;
				_asHorizontalIndicator->stMoveLeft();
			} else {
				playActionVideo();
			}
			break;
		default:
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
	case NM_MOUSE_CLICK:
		if ((param.asPoint().x <= 20 || param.asPoint().x >= 620) && !getGlobalVar(V_CANNON_RAISED)) {
			setGlobalVar(V_CANNON_TARGET_STATUS, 0);
			leaveScene(0);
		}
		break;
	case NM_ANIMATION_UPDATE:
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
	case NM_POSITION_CHANGE:
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
					openSmacker(0x108A000F, false);
				} else if (!getGlobalVar(V_ROBOT_HIT)) {
					_cannonTargetStatus = kCTSLeftRobotIsTarget;
					openSmacker(0x500B002F, false);
				} else {
					_cannonTargetStatus = kCTSLeftNoRobot;
					openSmacker(0x100B008E, false);
				}
				_moveCannonLeftFirst = true;
				_isTurning = true;
				_keepVideo = false;
				setGlobalVar(V_CANNON_TURNED, 0);
			}
		}
		break;
	default:
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

Scene3010::Scene3010(NeverhoodEngine *vm, Module *parentModule, int which)
	: Scene(vm, parentModule), _countdown(0), _doorUnlocked(false), _checkUnlocked(false) {

	int initCountdown = 0;

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
	case NM_MOUSE_CLICK:
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
	case NM_ANIMATION_UPDATE:
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
	case NM_POSITION_CHANGE:
		if (!_checkUnlocked && _countdown == 0) {
			_asDeadBolts[param.asInteger()]->lock();
		}
		break;
	case 0x2003:
		_boltUnlocked[param.asInteger()] = false;
		break;
	default:
		break;
	}
	return 0;
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
		default:
			break;
		}
	}
}

uint32 Scene3011::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	Scene::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case NM_MOUSE_CLICK:
		if (param.asPoint().x <= 20 || param.asPoint().x >= 620) {
			leaveScene(0);
		}
		break;
	case NM_ANIMATION_UPDATE:
		_buttonClicked = true;
		if (_countdown == 0)
			_countdown = 1;
		break;
	default:
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
