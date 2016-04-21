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

#include "gnap/gnap.h"
#include "gnap/gamesys.h"
#include "gnap/resource.h"

#include "gnap/scenes/scene12.h"

namespace Gnap {

enum {
	kHSPlatypus		= 0,
	kHSExitRight	= 1,
	kHSToothGuy		= 2,
	kHSBarkeeper	= 3,
	kHSBeardGuy		= 4,
	kHSJukebox		= 5,
	kHSWalkArea1	= 6,
	kHSDevice		= 7,
	kHSWalkArea2	= 8,
	kHSWalkArea3	= 9,
	kHSWalkArea4	= 10
};

enum {
	kASLeaveScene					= 0,
	kASQuarterToToothGuyDone		= 1,
	kASTalkToothGuy					= 2,
	kASGrabToothGuy					= 4,
	kASShowItemToToothGuy			= 5,
	kASQuarterWithHoleToToothGuy	= 6,
	kASQuarterToToothGuy			= 7,
	kASTalkBeardGuy					= 8,
	kASLookBeardGuy					= 9,
	kASGrabBeardGuy					= 10,
	kASShowItemToBeardGuy			= 11,
	kASTalkBarkeeper				= 12,
	kASLookBarkeeper				= 13,
	kASShowItemToBarkeeper			= 15,
	kASQuarterWithBarkeeper			= 16,
	kASPlatWithBarkeeper			= 17,
	kASPlatWithToothGuy				= 18,
	kASPlatWithBeardGuy				= 19
};

Scene12::Scene12(GnapEngine *vm) : Scene(vm) {
	_s12_nextBeardGuySequenceId = -1;
	_s12_currBeardGuySequenceId = -1;
	_s12_nextToothGuySequenceId = -1;
	_s12_currToothGuySequenceId = -1;
	_s12_nextBarkeeperSequenceId = -1;
	_s12_currBarkeeperSequenceId = -1;
}

int Scene12::init() {
	return 0x209;
}

void Scene12::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSExitRight, 790, 360, 799, 599, SF_EXIT_R_CURSOR);
	_vm->setHotspot(kHSToothGuy, 80, 180, 160, 380, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSBarkeeper, 490, 175, 580, 238, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSBeardGuy, 620, 215, 720, 350, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSJukebox, 300, 170, 410, 355, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSWalkArea1, 0, 0, 260, 460);
	_vm->setHotspot(kHSWalkArea2, 0, 0, 380, 410);
	_vm->setHotspot(kHSWalkArea3, 0, 0, 799, 395);
	_vm->setHotspot(kHSWalkArea4, 585, 0, 799, 455);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_vm->_hotspotsCount = 11;
}

void Scene12::run() {
	int v18 = 1;

	_vm->queueInsertDeviceIcon();

	_vm->_gameSys->insertSequence(0x207, 256, 0, 0, kSeqNone, 0, 0, 0);
	_vm->_gameSys->insertSequence(0x200, 50, 0, 0, kSeqNone, 0, 0, 0);
	
	_s12_currToothGuySequenceId = 0x200;
	_s12_nextToothGuySequenceId = -1;
	
	_vm->_gameSys->setAnimation(0x200, 50, 2);
	_vm->_gameSys->insertSequence(0x202, 50, 0, 0, kSeqNone, 0, 0, 0);
	
	_s12_currBeardGuySequenceId = 0x202;
	_s12_nextBeardGuySequenceId = -1;
	
	_vm->_gameSys->setAnimation(0x202, 50, 4);
	_vm->_gameSys->insertSequence(0x203, 50, 0, 0, kSeqNone, 0, 0, 0);
	
	_s12_currBarkeeperSequenceId = 0x203;
	_s12_nextBarkeeperSequenceId = -1;
	
	_vm->_gameSys->setAnimation(0x203, 50, 3);
	
	_vm->_timers[4] = 30;
	_vm->_timers[6] = _vm->getRandom(30) + 20;
	_vm->_timers[5] = _vm->getRandom(30) + 20;
	_vm->_timers[7] = _vm->getRandom(100) + 100;
	
	if (_vm->_prevSceneNum == 15) {
		_vm->initGnapPos(5, 6, kDirBottomRight);
		_vm->initPlatypusPos(3, 7, kDirNone);
		_vm->endSceneInit();
	} else {
		_vm->initGnapPos(11, 8, kDirBottomLeft);
		_vm->initPlatypusPos(12, 8, kDirUnk4);
		_vm->endSceneInit();
		_vm->gnapWalkTo(8, 8, -1, 0x107BA, 1);
		_vm->platypusWalkTo(9, 8, -1, 0x107D2, 1);
	}

	while (!_vm->_sceneDone) {
		_vm->testWalk(0, 0, -1, -1, -1, -1);
	
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
		_vm->updateGrabCursorSprite(0, 0);
	
		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);
	
		switch (_vm->_sceneClickedHotspot) {
		case kHSDevice:
			if (_vm->_gnapActionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHSPlatypus:
			switch (_vm->_verbCursor) {
			case LOOK_CURSOR:
				_vm->playGnapMoan1(_vm->_platX, _vm->_platY);
				break;
			case GRAB_CURSOR:
				_vm->gnapKissPlatypus(0);
				break;
			case TALK_CURSOR:
				_vm->playGnapBrainPulsating(_vm->_platX, _vm->_platY);
				_vm->playPlatypusSequence(_vm->getPlatypusSequenceId());
				break;
			case PLAT_CURSOR:
				break;
			}
			break;

		case kHSExitRight:
			_vm->_isLeavingScene = true;
			_vm->gnapWalkTo(10, -1, 0, 0x107AB, 1);
			_vm->_gnapActionStatus = kASLeaveScene;
			_vm->platypusWalkTo(10, -1, -1, -1, 1);
			_vm->_newSceneNum = 11;
			break;

		case kHSToothGuy:
			if (_vm->_grabCursorSpriteIndex == kItemQuarter) {
				_vm->_largeSprite = _vm->_gameSys->createSurface(0x141);
				_vm->gnapWalkTo(3, 7, 0, 0x107BC, 9);
				_vm->_gnapIdleFacing = kDirUpLeft;
				_vm->_gnapActionStatus = kASQuarterToToothGuy;
				_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, 2, 0);
				_vm->setGrabCursorSprite(-1);
			} else if (_vm->_grabCursorSpriteIndex == kItemQuarterWithHole) {
				_vm->gnapWalkTo(3, 7, 0, 0x107BC, 9);
				_vm->_gnapIdleFacing = kDirUpLeft;
				_vm->_gnapActionStatus = kASQuarterWithHoleToToothGuy;
				_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, 2, 0);
			} else if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->gnapWalkTo(3, 7, 0, 0x107BC, 9);
				_vm->_gnapIdleFacing = kDirUpLeft;
				_vm->_gnapActionStatus = kASShowItemToToothGuy;
				_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, 2, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->playGnapScratchingHead(1, 2);
					break;
				case GRAB_CURSOR:
					_vm->gnapWalkTo(3, 7, 0, 0x107BC, 1);
					_vm->_gnapActionStatus = kASGrabToothGuy;
					break;
				case TALK_CURSOR:
					_vm->_gnapIdleFacing = kDirUpLeft;
					_vm->gnapWalkTo(3, 7, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
					_vm->_gnapActionStatus = kASTalkToothGuy;
					break;
				case PLAT_CURSOR:
					_vm->gnapUseDeviceOnPlatypuss();
					_vm->platypusWalkTo(3, 7, 1, 0x107D2, 1);
					_vm->_platypusActionStatus = kASPlatWithToothGuy;
					_vm->_platypusFacing = kDirUnk4;
					_vm->playGnapIdle(2, 7);
					break;
				}
			}
			break;

		case kHSBarkeeper:
			if (_vm->_grabCursorSpriteIndex == kItemQuarter || _vm->_grabCursorSpriteIndex == kItemQuarterWithHole) {
				_vm->gnapWalkTo(6, 6, 0, 0x107BB, 9);
				_vm->_gnapIdleFacing = kDirUpRight;
				_vm->_gnapActionStatus = kASQuarterWithBarkeeper;
				_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, 7, 0);
			} else if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->gnapWalkTo(6, 6, 0, 0x107BB, 9);
				_vm->_gnapIdleFacing = kDirUpRight;
				_vm->_gnapActionStatus = kASShowItemToBarkeeper;
				_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, 7, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->gnapWalkTo(6, 6, 0, 0x107BB, 1);
					_vm->_gnapIdleFacing = kDirUpRight;
					_vm->_gnapActionStatus = kASLookBarkeeper;
					break;
				case GRAB_CURSOR:
					_vm->playGnapImpossible(0, 0);
					break;
				case TALK_CURSOR:
					_vm->_gnapIdleFacing = kDirUpRight;
					_vm->gnapWalkTo(6, 6, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
					_vm->_gnapActionStatus = kASTalkBarkeeper;
					break;
				case PLAT_CURSOR:
					_vm->playGnapPullOutDevice(_vm->_platX, _vm->_platY);
					_vm->_gameSys->setAnimation(makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, 0);
					_vm->_gnapActionStatus = kASPlatWithBarkeeper;
					break;
				}
			}
			break;

		case kHSBeardGuy:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->gnapWalkTo(7, 6, 0, 0x107BB, 9);
				_vm->_gnapIdleFacing = kDirUpRight;
				_vm->_gnapActionStatus = kASShowItemToBeardGuy;
				_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, 8, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->gnapWalkTo(7, 6, 0, 0x107BB, 1);
					_vm->_gnapIdleFacing = kDirUpRight;
					_vm->_gnapActionStatus = kASLookBeardGuy;
					break;
				case GRAB_CURSOR:
					// NOTE Bug in the original. It has 9 as flags which seems wrong here.
					_vm->gnapWalkTo(7, 6, 0, 0x107BB, 1);
					_vm->_gnapIdleFacing = kDirUpRight;
					_vm->_gnapActionStatus = kASGrabBeardGuy;
					break;
				case TALK_CURSOR:
					_vm->_gnapIdleFacing = kDirUpRight;
					_vm->gnapWalkTo(7, 6, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
					_vm->_gnapActionStatus = kASTalkBeardGuy;
					break;
				case PLAT_CURSOR:
					_vm->gnapUseDeviceOnPlatypuss();
					_vm->platypusWalkTo(7, 6, 1, 0x107C2, 1);
					_vm->_platypusActionStatus = kASPlatWithBeardGuy;
					_vm->_platypusFacing = kDirNone;
					_vm->playGnapIdle(7, 6);
					break;
				}
			}
			break;

		case kHSJukebox:
			_vm->_newSceneNum = 15;
			_vm->_isLeavingScene = true;
			_vm->gnapWalkTo(5, 6, 0, 0x107BC, 1);
			_vm->_gnapActionStatus = kASLeaveScene;
			break;

		case kHSWalkArea1:
		case kHSWalkArea2:
		case kHSWalkArea3:
		case kHSWalkArea4:
			if (_vm->_gnapActionStatus < 0)
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			break;

		default:
			if (_vm->_mouseClickState._left && _vm->_gnapActionStatus < 0) {
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				_vm->_mouseClickState._left = false;
			}
			break;

		}
	
		updateAnimations();
	
		if (!_vm->_isLeavingScene) {
			_vm->updatePlatypusIdleSequence();
			_vm->updateGnapIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = 15;
				if (_s12_nextToothGuySequenceId == -1) {
					if (v18 == 0 && _s12_currBeardGuySequenceId == 0x202 && _s12_currBarkeeperSequenceId == 0x203 && _vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0) {
						if (_vm->getRandom(2) != 0)
							_s12_nextToothGuySequenceId = 0x1EC;
						else
							_s12_nextToothGuySequenceId = 0x204;
					} else if (_s12_currToothGuySequenceId != 0x200)
						_s12_nextToothGuySequenceId = 0x200;
					v18 = (v18 + 1) % 15;
				}
			}
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(30) + 20;
				if (_s12_nextBarkeeperSequenceId == -1 && _vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0) {
					if (v18 == 0 && _s12_currToothGuySequenceId == 0x200 && _s12_currBeardGuySequenceId == 0x202 && _vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0) {
						if (_vm->getRandom(2) != 0)
							_s12_nextBarkeeperSequenceId = 0x208;
						else
							_s12_nextBarkeeperSequenceId = 0x1FB;
					} else
						_s12_nextBarkeeperSequenceId = 0x203;
					v18 = (v18 + 1) % 15;
				}
			}
			if (!_vm->_timers[6]) {
				_vm->_timers[6] = _vm->getRandom(30) + 15;
				if (_s12_nextBeardGuySequenceId == -1 && _vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0) {
					if (v18 == 0 && _s12_currToothGuySequenceId == 0x200 && _s12_currBarkeeperSequenceId == 0x203 && _vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0)
						_s12_nextBeardGuySequenceId = 0x1F2;
					else
						_s12_nextBeardGuySequenceId = 0x202;
					v18 = (v18 + 1) % 15;
				}
			}
			if (!_vm->_timers[7]) {
				_vm->_timers[7] = _vm->getRandom(100) + 100;
				int _gnapRandomValue = _vm->getRandom(3);
				switch (_gnapRandomValue) {
				case 0:
					_vm->_gameSys->insertSequence(0x8A5 | 0x10000, 179, 0, 0, kSeqNone, 0, 0, 0);
					break;
				case 1:
					_vm->_gameSys->insertSequence(0x8A7 | 0x10000, 179, 0, 0, kSeqNone, 0, 0, 0);
					break;
				case 2:
					_vm->_gameSys->insertSequence(0x8A6 | 0x10000, 179, 0, 0, kSeqNone, 0, 0, 0);
					break;
				}
			}
		}
	
		_vm->checkGameKeys();
	
		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[4] = 30;
			_vm->_timers[5] = _vm->getRandom(30) + 20;
			_vm->_timers[6] = _vm->getRandom(30) + 20;
		}
		
		_vm->gameUpdateTick();
	}
}

void Scene12::updateAnimations() {
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kASLeaveScene:
			_vm->_sceneDone = true;
			break;
		case kASTalkToothGuy:
			if (_vm->isKeyStatus1(Common::KEYCODE_j)) {
				// Easter egg
				_vm->clearKeyStatus1(Common::KEYCODE_j);
				_s12_nextToothGuySequenceId = 0x206;
			} else {
				_s12_nextToothGuySequenceId = 0x1EE;
			}
			break;
		case 3:
			break;
		case kASGrabToothGuy:
			if (_vm->isKeyStatus1(Common::KEYCODE_j)) {
				_vm->clearKeyStatus1(Common::KEYCODE_j);
				_s12_nextToothGuySequenceId = 0x206;
			} else {
				_s12_nextToothGuySequenceId = 0x1EF;
			}
			break;
		case kASShowItemToToothGuy:
			if (_vm->isKeyStatus1(Common::KEYCODE_j)) {
				_vm->clearKeyStatus1(Common::KEYCODE_j);
				_s12_nextToothGuySequenceId = 0x206;
			} else {
				_s12_nextToothGuySequenceId = 0x1ED;
			}
			break;
		case kASQuarterWithHoleToToothGuy:
			if (_vm->isKeyStatus1(Common::KEYCODE_j)) {
				_vm->clearKeyStatus1(Common::KEYCODE_j);
				_s12_nextToothGuySequenceId = 0x206;
			} else {
				_s12_nextToothGuySequenceId = 0x1EA;
			}
			break;
		case kASQuarterToToothGuy:
			if (_vm->isKeyStatus1(Common::KEYCODE_j)) {
				_vm->clearKeyStatus1(Common::KEYCODE_j);
				_s12_nextToothGuySequenceId = 0x206;
			} else {
				_s12_nextToothGuySequenceId = 0x1E9;
			}
			break;
		case kASQuarterToToothGuyDone:
			_vm->_gnapActionStatus = -1;
			_vm->showCursor();
			_vm->_gameSys->removeSpriteDrawItem(_vm->_largeSprite, 300);
			_vm->deleteSurface(&_vm->_largeSprite);
			_vm->setGrabCursorSprite(kItemQuarterWithHole);
			break;
		case kASTalkBeardGuy:
			_s12_nextBeardGuySequenceId = 0x1F4;
			break;
		case kASLookBeardGuy:
			_s12_nextBeardGuySequenceId = 0x1F3;
			break;
		case kASGrabBeardGuy:
			_s12_nextBeardGuySequenceId = 0x1F1;
			break;
		case kASShowItemToBeardGuy:
			_s12_nextBeardGuySequenceId = 0x1F0;
			break;
		case kASTalkBarkeeper:
			if (_vm->getRandom(2) != 0)
				_s12_nextBarkeeperSequenceId = 0x1FD;
			else
				_s12_nextBarkeeperSequenceId = 0x1FF;
			break;
		case kASLookBarkeeper:
			_s12_nextBarkeeperSequenceId = 0x1F8;
			break;
		case 14:
			_s12_nextBarkeeperSequenceId = 0x1F6;
			break;
		case kASShowItemToBarkeeper:
			_s12_nextBarkeeperSequenceId = 0x1F5;
			break;
		case kASQuarterWithBarkeeper:
			_s12_nextBarkeeperSequenceId = 0x1FA;
			break;
		case kASPlatWithBarkeeper:
			_s12_nextBarkeeperSequenceId = 0x1F9;
			break;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(1) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 1);
		switch (_vm->_platypusActionStatus) {
		case kASPlatWithToothGuy:
			_s12_nextToothGuySequenceId = 0x1EB;
			break;
		case kASPlatWithBeardGuy:
			_s12_nextBeardGuySequenceId = 0x1F3;
			break;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(2) == 2) {
		if (_s12_currToothGuySequenceId == 0x1E9) {
			_vm->_gameSys->setAnimation(0, 0, 2);
			_vm->hideCursor();
			_vm->_gameSys->setAnimation(0x10843, 301, 0);
			_vm->_gnapActionStatus = kASQuarterToToothGuyDone;
			_vm->_gameSys->insertSpriteDrawItem(_vm->_largeSprite, 0, 0, 300);
			_vm->_gameSys->insertSequence(0x10843, 301, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x107B7, _vm->_gnapId, 0x10843, 301,
				kSeqSyncWait, 0, 75 * _vm->_gnapX - _vm->_gnapGridX, 48 * _vm->_gnapY - _vm->_gnapGridY);
			_vm->_gnapSequenceId = 0x7B7;
			_vm->_gnapSequenceDatNum = 1;
			_vm->setFlag(kGFTwigTaken);
			_vm->invAdd(kItemQuarterWithHole);
			_vm->invRemove(kItemQuarter);
		}
		if (_s12_nextToothGuySequenceId == 0x1EF) {
			_vm->_gameSys->setAnimation(_s12_nextToothGuySequenceId, 50, 2);
			_vm->_gameSys->insertSequence(_s12_nextToothGuySequenceId, 50, _s12_currToothGuySequenceId, 50, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x205, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_s12_currToothGuySequenceId = _s12_nextToothGuySequenceId;
			_s12_nextToothGuySequenceId = -1;
			_vm->_gnapSequenceId = 0x205;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_timers[4] = 40;
			_vm->_timers[2] = _vm->getRandom(20) + 70;
			_vm->_timers[3] = _vm->getRandom(50) + 200;
			if (_vm->_gnapActionStatus == kASGrabToothGuy)
				_vm->_gnapActionStatus = -1;
		} else if (_s12_nextToothGuySequenceId != -1) {
			_vm->_gameSys->insertSequence(_s12_nextToothGuySequenceId, 50, _s12_currToothGuySequenceId, 50, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(_s12_nextToothGuySequenceId, 50, 2);
			_s12_currToothGuySequenceId = _s12_nextToothGuySequenceId;
			_s12_nextToothGuySequenceId = -1;
			_vm->_timers[4] = 50;
			if (_vm->_gnapActionStatus >= kASTalkToothGuy && _vm->_gnapActionStatus <= kASQuarterToToothGuy && _s12_currToothGuySequenceId != 0x1E9 &&
				_s12_currToothGuySequenceId != 0x1EC && _s12_currToothGuySequenceId != 0x200)
				_vm->_gnapActionStatus = -1;
			if (_vm->_platypusActionStatus == kASPlatWithToothGuy)
				_vm->_platypusActionStatus = -1;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(3) == 2) {
		if (_vm->_gnapActionStatus == kASPlatWithBarkeeper && _s12_currBarkeeperSequenceId == 0x1F9) {
			_vm->_gnapActionStatus = -1;
			_vm->playGnapIdle(7, 6);
			_vm->_timers[5] = 0;
		}
		if (_s12_nextBarkeeperSequenceId != -1) {
			_vm->_gameSys->insertSequence(_s12_nextBarkeeperSequenceId, 50, _s12_currBarkeeperSequenceId, 50, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(_s12_nextBarkeeperSequenceId, 50, 3);
			_s12_currBarkeeperSequenceId = _s12_nextBarkeeperSequenceId;
			_s12_nextBarkeeperSequenceId = -1;
			_vm->_timers[5] = _vm->getRandom(30) + 20;
			if (_vm->_gnapActionStatus >= kASTalkBarkeeper && _vm->_gnapActionStatus <= kASQuarterWithBarkeeper && _s12_currBarkeeperSequenceId != 0x203 &&
				_s12_currBarkeeperSequenceId != 0x1FB && _s12_currBarkeeperSequenceId != 0x208)
				_vm->_gnapActionStatus = -1;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(4) == 2 && _s12_nextBeardGuySequenceId != -1) {
		_vm->_gameSys->insertSequence(_s12_nextBeardGuySequenceId, 50, _s12_currBeardGuySequenceId, 50, kSeqSyncWait, 0, 0, 0);
		_vm->_gameSys->setAnimation(_s12_nextBeardGuySequenceId, 50, 4);
		_s12_currBeardGuySequenceId = _s12_nextBeardGuySequenceId;
		_s12_nextBeardGuySequenceId = -1;
		_vm->_timers[6] = _vm->getRandom(30) + 20;
		if (_vm->_gnapActionStatus >= kASTalkBeardGuy && _vm->_gnapActionStatus <= kASShowItemToBeardGuy && _s12_currBeardGuySequenceId != 0x202 && _s12_currBeardGuySequenceId != 0x1F2)
			_vm->_gnapActionStatus = -1;
		if (_vm->_platypusActionStatus == kASPlatWithBeardGuy)
			_vm->_platypusActionStatus = -1;
	}
}

} // End of namespace Gnap
