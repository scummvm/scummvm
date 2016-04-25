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
#include "gnap/scenes/scene41.h"

namespace Gnap {

enum {
	kHSPlatypus		= 0,
	kHSExitCave		= 1,
	kHSExit			= 2,
	kHSExitBBQ		= 3,
	kHSToyVendor	= 4,
	kHSKid			= 5,
	kHSToyUfo		= 6,
	kHSDevice		= 7,
	kHSWalkArea1	= 8
};

enum {
	kHSUfoExitLeft	= 1,
	kHSUfoExitRight	= 2,
	kHSUfoDevice	= 3,
	kHSUfoWalkArea1	= 4
};

enum {
	kASLeaveScene				= 0,
	kASUseQuarterWithToyVendor	= 1,
	kASTalkToyVendor			= 2,
	kASUseGumWithToyUfo			= 3,
	kASUseChickenBucketWithKid	= 4,
	kASGrabKid					= 5,
	kASGiveBackToyUfo			= 6,
	kASToyUfoLeaveScene			= 7,
	kASToyUfoRefresh			= 8,
	kASUfoGumAttached			= 9
};

Scene41::Scene41(GnapEngine *vm) : Scene(vm) {
	_currKidSequenceId = -1;
	_nextKidSequenceId = -1;
	_currToyVendorSequenceId = -1;
	_nextToyVendorSequenceId = -1;
}

int Scene41::init() {
	GameSys gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 1);
	gameSys.setAnimation(0, 0, 2);
	return 0x129;
}

void Scene41::updateHotspots() {
	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_DISABLED);
		_vm->setHotspot(kHSUfoExitLeft, 0, 0, 10, 500, SF_EXIT_L_CURSOR | SF_DISABLED);
		_vm->setHotspot(kHSUfoExitRight, 790, 0, 799, 500, SF_EXIT_R_CURSOR);
		_vm->setHotspot(kHSUfoWalkArea1, 0, 0, 800, 470, SF_DISABLED);
		_vm->setDeviceHotspot(kHSUfoDevice, -1, -1, -1, -1);
		_vm->_hotspotsCount = 5;
	} else {
		_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHSExitCave, 150, 590, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
		_vm->setHotspot(kHSExit, 0, 100, 10, 599, SF_EXIT_L_CURSOR | SF_DISABLED, 0, 8);
		_vm->setHotspot(kHSExitBBQ, 790, 100, 799, 599, SF_EXIT_R_CURSOR | SF_WALKABLE, 10, 8);
		_vm->setHotspot(kHSToyVendor, 320, 150, 430, 310, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHSKid, 615, 340, 710, 460, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHSToyUfo, 0, 0, 0, 0, SF_GRAB_CURSOR);
		_vm->setHotspot(kHSWalkArea1, 0, 0, 800, 470);
		_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
		_vm->_hotspotsCount = 9;
	}
}

void Scene41::run() {
	GameSys gameSys = *_vm->_gameSys;

	_vm->queueInsertDeviceIcon();

	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->_toyUfoX = 770;
		if (_vm->_toyUfoY < 0 || _vm->_toyUfoY > 300)
			_vm->_toyUfoY = 150;
		if (!_vm->_timers[9])
			_vm->_gnapActionStatus = kASGiveBackToyUfo;
	} else {
		if (!_vm->isFlag(kGFUnk16) && !_vm->isFlag(kGFJointTaken) && !_vm->isFlag(kGFUnk18) && !_vm->isFlag(kGFGroceryStoreHatTaken))
			_vm->toyUfoSetStatus(kGFUnk16);
		_vm->_toyUfoX = 600;
		_vm->_toyUfoY = 200;
	}

	_vm->_toyUfoId = 0;
	_vm->_toyUfoActionStatus = -1;
	_vm->_toyUfoSequenceId = _vm->toyUfoGetSequenceId();
	_vm->_toyUfoNextSequenceId = _vm->_toyUfoSequenceId;

	gameSys.setAnimation(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 2);
	gameSys.insertSequence(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 0, 0, kSeqNone, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
	gameSys.insertSequence(0x128, 0, 0, 0, kSeqLoop, 0, 0, 0);
	
	if (_vm->isFlag(kGFGnapControlsToyUFO))
		_currKidSequenceId = 0x11B;
	else
		_currKidSequenceId = 0x11D;
	
	_nextKidSequenceId = -1;
	
	gameSys.setAnimation(_currKidSequenceId, 1, 4);
	gameSys.insertSequence(_currKidSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	_currToyVendorSequenceId = 0x118;
	_nextToyVendorSequenceId = -1;
	
	gameSys.setAnimation(0x118, 1, 3);
	gameSys.insertSequence(_currToyVendorSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
	gameSys.insertSequence(0x127, 2, 0, 0, kSeqNone, 0, 0, 0);
	
	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->_gnapSequenceId = 0x120;
		_vm->_gnapSequenceDatNum = 0;
		_vm->_gnapIdleFacing = kDirUpRight;
		_vm->_gnapX = 7;
		_vm->_gnapY = 7;
		_vm->_gnapId = 140;
		gameSys.insertSequence(0x120, 140, 0, 0, kSeqNone, 0, 0, 0);
		gameSys.setAnimation(makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, 0);
		_vm->initPlatypusPos(8, 10, kDirBottomLeft);
		_vm->endSceneInit();
	} else if (_vm->_prevSceneNum == 45) {
		_vm->initGnapPos(-1, 8, kDirUpRight);
		_vm->initPlatypusPos(-2, 8, kDirUpLeft);
		_vm->endSceneInit();
		_vm->platypusWalkTo(1, 8, -1, 0x107C2, 1);
		_vm->gnapWalkTo(2, 8, -1, 0x107B9, 1);
	} else if (_vm->_prevSceneNum == 42) {
		_vm->initGnapPos(11, 8, kDirUpRight);
		_vm->initPlatypusPos(11, 9, kDirUpLeft);
		_vm->endSceneInit();
		_vm->gnapWalkTo(8, 8, -1, 0x107BA, 1);
		_vm->platypusWalkTo(9, 8, -1, 0x107D2, 1);
	} else {
		_vm->initGnapPos(5, 8, kDirBottomRight);
		_vm->initPlatypusPos(6, 8, kDirBottomLeft);
		_vm->endSceneInit();
	}

	_vm->_timers[4] = _vm->getRandom(100) + 100;
	_vm->_timers[5] = _vm->getRandom(30) + 20;

	while (!_vm->_sceneDone) {
		if (!_vm->isSoundPlaying(0x1094B))
			_vm->playSound(0x1094B, true);
	
		if (!_vm->isFlag(kGFGnapControlsToyUFO)) {
			_vm->_hotspots[kHSToyUfo]._x1 = _vm->_toyUfoX - 25;
			_vm->_hotspots[kHSToyUfo]._y1 = _vm->_toyUfoY - 20;
			_vm->_hotspots[kHSToyUfo]._x2 = _vm->_toyUfoX + 25;
			_vm->_hotspots[kHSToyUfo]._y2 = _vm->_toyUfoY + 20;
		}
	
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
	
		_vm->testWalk(0, 0, -1, -1, -1, -1);
	
		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);
	
		if (_vm->isFlag(kGFGnapControlsToyUFO)) {
			switch (_vm->_sceneClickedHotspot) {
			case kHSUfoExitLeft:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = kASToyUfoLeaveScene;
					_vm->_newSceneNum = 45;
					_vm->toyUfoFlyTo(-35, -1, -35, 799, 0, 300, 2);
				}
				break;
	
			case kHSUfoExitRight:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = kASToyUfoLeaveScene;
					_vm->_newSceneNum = 42;
					_vm->toyUfoFlyTo(835, -1, 0, 835, 0, 300, 2);
				}
				break;
	
			case kHSUfoDevice:
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(100) + 100;
				_vm->_timers[5] = _vm->getRandom(30) + 20;
				break;
			}
		} else {
			switch (_vm->_sceneClickedHotspot) {
			case kHSDevice:
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(100) + 100;
				_vm->_timers[5] = _vm->getRandom(30) + 20;
				break;
	
			case kHSPlatypus:
				if (_vm->_gnapActionStatus < 0) {
					if (_vm->_grabCursorSpriteIndex >= 0) {
						_vm->playGnapImpossible(_vm->_platX, _vm->_platY);
					} else {
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
							_vm->playGnapImpossible(_vm->_platX, _vm->_platY);
							break;
						}
					}
				}
				break;
	
			case kHSExitCave:
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitCave].x, _vm->_hotspotsWalkPos[kHSExitCave].y, 0, 0x107AE, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->_newSceneNum = 40;
				break;
	
			case kHSExit:
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExit].x, -1, 0, 0x107AF, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExit].x, -1, -1, 0x107CF, 1);
				_vm->_newSceneNum = 45;
				break;
	
			case kHSExitBBQ:
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitBBQ].x, -1, 0, 0x107AB, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitBBQ].x, -1, -1, 0x107CD, 1);
				_vm->_newSceneNum = 42;
				break;
	
			case kHSToyVendor:
				if (_vm->_grabCursorSpriteIndex == kItemDiceQuarterHole) {
					_vm->_gnapActionStatus = kASUseQuarterWithToyVendor;
					_vm->gnapWalkTo(4, 7, 0, 0x107BB, 9);
					_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, 5, 0);
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(4, 7, 5, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapMoan1(5, 0);
						break;
					case GRAB_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					case TALK_CURSOR:
						_vm->_gnapIdleFacing = kDirUpRight;
						_vm->gnapWalkTo(4, 7, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_vm->_gnapActionStatus = kASTalkToyVendor;
						break;
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
				break;
	
			case kHSKid:
				if (_vm->_grabCursorSpriteIndex == kItemChickenBucket) {
					_vm->gnapWalkTo(7, 7, 0, 0x107BB, 1);
					_vm->_gnapIdleFacing = kDirUpRight;
					_vm->_gnapActionStatus = kASUseChickenBucketWithKid;
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(7, 7, 8, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(9, 0);
						break;
					case GRAB_CURSOR:
						_vm->gnapWalkTo(7, 7, 0, 0x107BB, 1);
						_vm->_gnapIdleFacing = kDirUpRight;
						_vm->_gnapActionStatus = kASGrabKid;
						break;
					case TALK_CURSOR:
						_vm->_gnapIdleFacing = kDirUpRight;
						_vm->gnapWalkTo(7, 7, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						break;
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
				break;
	
			case kHSToyUfo:
				if (_vm->_grabCursorSpriteIndex == kItemGum) {
					_vm->playGnapPullOutDevice(9, 0);
					gameSys.setAnimation(makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, 0);
					_vm->_gnapActionStatus = kASUseGumWithToyUfo;
				}
				break;
	
			case kHSWalkArea1:
				if (_vm->_gnapActionStatus < 0)
					_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				break;
			}
		}
	
		if (_vm->isFlag(kGFGnapControlsToyUFO)) {
			if (!_vm->_timers[9] && _vm->_gnapActionStatus < 0) {
				_vm->_gnapActionStatus = kASGiveBackToyUfo;
				if (_vm->_gnapSequenceId == 0x121 || _vm->_gnapSequenceId == 0x122) {
					gameSys.insertSequence(0x123, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
					_vm->_gnapSequenceId = 0x123;
					_vm->_gnapSequenceDatNum = 0;
					gameSys.setAnimation(0x123, _vm->_gnapId, 0);
				}
			}
		}
	
		if (_vm->_mouseClickState._left && _vm->_gnapActionStatus < 0) {
			_vm->_mouseClickState._left = false;
			if (_vm->isFlag(kGFGnapControlsToyUFO)) {
				int sequenceId;
				if (_vm->_leftClickMouseX >= 400) {
					if (_vm->_gnapSequenceId == 0x11F || _vm->_gnapSequenceId == 0x120 || _vm->_gnapSequenceId == 0x123 || _vm->_gnapSequenceId == 0x126)
						sequenceId = 0x120;
					else if (_vm->_leftClickMouseX - _vm->_toyUfoX >= 400)
						sequenceId = 0x126;
					else
						sequenceId = 0x123;
				} else {
					if (_vm->_gnapSequenceId == 0x121 || _vm->_gnapSequenceId == 0x125 || _vm->_gnapSequenceId == 0x122)
						sequenceId = 0x122;
					else if (_vm->_toyUfoX - _vm->_leftClickMouseX >= 400)
						sequenceId = 0x125;
					else
						sequenceId = 0x121;
				}
				gameSys.insertSequence(sequenceId, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
				_vm->_gnapSequenceId = sequenceId;
				_vm->_gnapSequenceDatNum = 0;
				gameSys.setAnimation(sequenceId, _vm->_gnapId, 0);
				_vm->_toyUfoActionStatus = kASToyUfoRefresh;
				_vm->toyUfoFlyTo(-1, -1, 0, 799, 0, 300, 2);
			} else {
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			}
		}

		updateAnimations();
	
		if (!_vm->_isLeavingScene) {
			if (_vm->_platypusActionStatus < 0)
				_vm->updatePlatypusIdleSequence();
			if (_vm->_gnapActionStatus < 0 && !_vm->isFlag(kGFGnapControlsToyUFO))
				_vm->updateGnapIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(100) + 100;
				if (_vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0 && _vm->_toyUfoActionStatus == -1 && _nextToyVendorSequenceId == -1) {
					switch (_vm->getRandom(3)) {
					case 0:
						_nextToyVendorSequenceId = 0x113;
						break;
					case 1:
						_nextToyVendorSequenceId = 0x117;
						break;
					case 2:
						_nextToyVendorSequenceId = 0x119;
						break;
					}
					if (_nextToyVendorSequenceId == _currToyVendorSequenceId)
						_nextToyVendorSequenceId = -1;
				}
			}
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(30) + 20;
				if (_vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0 && _vm->_toyUfoActionStatus == -1 && _nextKidSequenceId == -1) {
					if (_vm->isFlag(kGFGnapControlsToyUFO))
						_nextKidSequenceId = 0x11B;
					else if (_vm->getRandom(3) != 0)
						_nextKidSequenceId = 0x11D;
					else
						_nextKidSequenceId = 0x11E;
				}
			}
		}
	
		_vm->checkGameKeys();
	
		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[4] = _vm->getRandom(100) + 100;
			_vm->_timers[5] = _vm->getRandom(30) + 20;
		}
		_vm->gameUpdateTick();
	}
}

void Scene41::updateAnimations() {
	GameSys gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(0) == 2) {
		switch (_vm->_gnapActionStatus) {
		case kASLeaveScene:
			gameSys.setAnimation(0, 0, 0);
			_vm->_sceneDone = true;
			_vm->_gnapActionStatus = -1;
			break;
		case kASUseQuarterWithToyVendor:
			gameSys.setAnimation(0, 0, 0);
			_nextToyVendorSequenceId = 0x114;
			_vm->_gnapActionStatus = -1;
			break;
		case kASTalkToyVendor:
			gameSys.setAnimation(0, 0, 0);
			_nextToyVendorSequenceId = 0x116;
			_vm->_gnapActionStatus = -1;
			break;
		case kASUseGumWithToyUfo:
			gameSys.setAnimation(0, 0, 0);
			_vm->playGnapUseDevice(9, 0);
			_vm->_gnapActionStatus = -1;
			_vm->setGrabCursorSprite(-1);
			_vm->invRemove(kItemGum);
			_vm->_toyUfoActionStatus = kASUfoGumAttached;
			break;
		case kASUseChickenBucketWithKid:
			if (gameSys.getAnimationStatus(4) == 2) {
				_vm->_timers[2] = _vm->getRandom(30) + 20;
				_vm->_timers[3] = _vm->getRandom(50) + 200;
				_vm->setGrabCursorSprite(-1);
				gameSys.insertSequence(0x11F, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
				_vm->_gnapSequenceId = 0x11F;
				_vm->_gnapSequenceDatNum = 0;
				gameSys.setAnimation(0x11F, _vm->_gnapId, 0);
				_nextKidSequenceId = 0x11A;
				gameSys.insertSequence(0x11A, 1, _currKidSequenceId, 1, kSeqSyncWait, 0, 0, 0);
				gameSys.setAnimation(_nextKidSequenceId, 1, 4);
				_currKidSequenceId = _nextKidSequenceId;
				_nextKidSequenceId = 0x11B;
				_vm->_timers[5] = _vm->getRandom(30) + 20;
				_vm->_gnapActionStatus = -1;
				_vm->setFlag(kGFGnapControlsToyUFO);
				updateHotspots();
				_vm->_timers[9] = 600;
			}
			break;
		case kASGrabKid:
			if (gameSys.getAnimationStatus(3) == 2 && gameSys.getAnimationStatus(4) == 2) {
				_vm->_timers[2] = _vm->getRandom(30) + 20;
				_vm->_timers[3] = _vm->getRandom(50) + 200;
				gameSys.insertSequence(0x110, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
				_vm->_gnapSequenceId = 0x110;
				_vm->_gnapSequenceDatNum = 0;
				gameSys.setAnimation(0x110, _vm->_gnapId, 0);
				_nextToyVendorSequenceId = 0x111;
				gameSys.insertSequence(0x111, 1, _currToyVendorSequenceId, 1, kSeqSyncWait, 0, 0, 0);
				gameSys.setAnimation(_nextToyVendorSequenceId, 1, 3);
				_currToyVendorSequenceId = _nextToyVendorSequenceId;
				_nextToyVendorSequenceId = -1;
				_vm->_timers[4] = _vm->getRandom(100) + 100;
				_nextKidSequenceId = 0x10F;
				gameSys.insertSequence(0x10F, 1, _currKidSequenceId, 1, kSeqSyncWait, 0, 0, 0);
				gameSys.setAnimation(_nextKidSequenceId, 1, 4);
				_currKidSequenceId = _nextKidSequenceId;
				_nextKidSequenceId = -1;
				_vm->_timers[5] = _vm->getRandom(30) + 20;
				_vm->_gnapActionStatus = -1;
			}
			break;
		case kASGiveBackToyUfo:
			if (gameSys.getAnimationStatus(3) == 2 && gameSys.getAnimationStatus(4) == 2) {
				_vm->_timers[2] = _vm->getRandom(30) + 20;
				_vm->_timers[3] = _vm->getRandom(50) + 200;
				gameSys.insertSequence(0x124, _vm->_gnapId,
					makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId,
					kSeqSyncWait, 0, 0, 0);
				_vm->_gnapSequenceId = 0x124;
				_vm->_gnapSequenceDatNum = 0;
				gameSys.setAnimation(0x124, _vm->_gnapId, 0);
				_nextToyVendorSequenceId = 0x112;
				gameSys.insertSequence(0x112, 1, _currToyVendorSequenceId, 1, kSeqSyncWait, 0, 0, 0);
				gameSys.setAnimation(_nextToyVendorSequenceId, 1, 3);
				_currToyVendorSequenceId = _nextToyVendorSequenceId;
				_nextToyVendorSequenceId = -1;
				_vm->_timers[4] = _vm->getRandom(100) + 100;
				_nextKidSequenceId = 0x11C;
				gameSys.insertSequence(0x11C, 1, _currKidSequenceId, 1, kSeqSyncWait, 0, 0, 0);
				gameSys.setAnimation(_nextKidSequenceId, 1, 4);
				_currKidSequenceId = _nextKidSequenceId;
				_nextKidSequenceId = -1;
				_vm->_timers[5] = _vm->getRandom(30) + 20;
				_vm->_gnapActionStatus = -1;
				_vm->clearFlag(kGFGnapControlsToyUFO);
				updateHotspots();
			}
			break;
		}
	}
	
	if (gameSys.getAnimationStatus(2) == 2) {
		switch (_vm->_toyUfoActionStatus) {
		case kASToyUfoLeaveScene:
			_vm->_sceneDone = true;
			break;
		case kASUfoGumAttached:
			_vm->_toyUfoNextSequenceId = 0x873;
			gameSys.insertSequence(0x10873, _vm->_toyUfoId, _vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId,
				kSeqSyncWait, 0, _vm->_toyUfoX - 365, _vm->_toyUfoY - 128);
			_vm->_toyUfoSequenceId = _vm->_toyUfoNextSequenceId;
			gameSys.setAnimation(_vm->_toyUfoNextSequenceId | 0x10000, _vm->_toyUfoId, 2);
			_vm->toyUfoSetStatus(kGFJointTaken);
			break;
		default:
			_vm->_toyUfoNextSequenceId = _vm->toyUfoGetSequenceId();
			gameSys.insertSequence(_vm->_toyUfoNextSequenceId | 0x10000, _vm->_toyUfoId + 1, _vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId,
				kSeqSyncWait, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
			_vm->_toyUfoSequenceId = _vm->_toyUfoNextSequenceId;
			++_vm->_toyUfoId;
			gameSys.setAnimation(_vm->_toyUfoNextSequenceId | 0x10000, _vm->_toyUfoId, 2);
			break;
		}
		_vm->_toyUfoActionStatus = -1;
	}
	
	if (gameSys.getAnimationStatus(3) == 2 && _nextToyVendorSequenceId != -1) {
		gameSys.insertSequence(_nextToyVendorSequenceId, 1, _currToyVendorSequenceId, 1, kSeqSyncWait, 0, 0, 0);
		gameSys.setAnimation(_nextToyVendorSequenceId, 1, 3);
		_currToyVendorSequenceId = _nextToyVendorSequenceId;
		_nextToyVendorSequenceId = -1;
		_vm->_timers[4] = _vm->getRandom(100) + 100;
	}
	
	if (gameSys.getAnimationStatus(4) == 2 && _nextKidSequenceId != -1) {
		gameSys.insertSequence(_nextKidSequenceId, 1, _currKidSequenceId, 1, kSeqSyncWait, 0, 0, 0);
		gameSys.setAnimation(_nextKidSequenceId, 1, 4);
		_currKidSequenceId = _nextKidSequenceId;
		_nextKidSequenceId = -1;
		_vm->_timers[5] = _vm->getRandom(30) + 20;
		if (_currKidSequenceId == 0x11E) {
			_vm->_toyUfoActionStatus = kASToyUfoRefresh;
			_vm->toyUfoFlyTo(_vm->getRandom(300) + 500, _vm->getRandom(225) + 75, 0, 799, 0, 300, 2);
		}
	}
}

} // End of namespace Gnap
