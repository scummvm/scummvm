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
#include "gnap/scenes/group4.h"

namespace Gnap {

enum {
	kHS40Platypus			= 0,
	kHS40ExitCave			= 1,
	kHS40ExitToyStand		= 2,
	kHS40ExitBBQ			= 3,
	kHS40ExitUfo			= 4,
	kHS40ExitKissinBooth	= 5,
	kHS40ExitDancefloor	= 6,
	kHS40ExitShoe			= 7,
	kHS40Device			= 8
};

Scene40::Scene40(GnapEngine *vm) : Scene(vm) {
}

int Scene40::init() {
	GameSys gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 1);
	return _vm->isFlag(kGFUnk23) ? 0x01 : 0x00;
}

void Scene40::updateHotspots() {
	_vm->setHotspot(kHS40Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_DISABLED | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS40ExitCave, 169, 510, 264, 600, SF_EXIT_D_CURSOR, 0, 8);
	_vm->setHotspot(kHS40ExitToyStand, 238, 297, 328, 376, SF_EXIT_L_CURSOR, 0, 8);
	_vm->setHotspot(kHS40ExitBBQ, 328, 220, 401, 306, SF_EXIT_L_CURSOR, 0, 8);
	_vm->setHotspot(kHS40ExitUfo, 421, 215, 501, 282, SF_EXIT_U_CURSOR, 0, 8);
	_vm->setHotspot(kHS40ExitKissinBooth, 476, 284, 556, 345, SF_EXIT_R_CURSOR, 0, 8);
	_vm->setHotspot(kHS40ExitDancefloor, 317, 455, 445, 600, SF_EXIT_D_CURSOR, 0, 8);
	_vm->setHotspot(kHS40ExitShoe, 455, 346, 549, 417, SF_EXIT_D_CURSOR, 0, 8);
	_vm->setDeviceHotspot(kHS40Device, -1, -1, -1, -1);
	_vm->_hotspotsCount = 9;
}

void Scene40::run() {
	_vm->queueInsertDeviceIcon();
	_vm->endSceneInit();

	while (!_vm->_sceneDone) {
		if (!_vm->isSoundPlaying(0x1094B))
			_vm->playSound(0x1094B, true);

		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
	
		_vm->testWalk(0, 0, -1, -1, -1, -1);

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);
	
		switch (_vm->_sceneClickedHotspot) {
		case kHS40Device:
			_vm->runMenu();
			updateHotspots();
			break;
			
		case kHS40Platypus:
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
	
		case kHS40ExitCave:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_newSceneNum = 39;
				_vm->_sceneDone = true;
			}
			break;
	
		case kHS40ExitToyStand:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_newSceneNum = 41;
				_vm->_sceneDone = true;
			}
			break;
	
		case kHS40ExitBBQ:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_newSceneNum = 42;
				_vm->_sceneDone = true;
			}
			break;
	
		case kHS40ExitUfo:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_newSceneNum = 43;
				_vm->_sceneDone = true;
			}
			break;
	
		case kHS40ExitKissinBooth:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_newSceneNum = 44;
				_vm->_sceneDone = true;
			}
			break;
	
		case kHS40ExitDancefloor:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_newSceneNum = 45;
				_vm->_sceneDone = true;
			}
			break;
	
		case kHS40ExitShoe:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_newSceneNum = 46;
				_vm->_sceneDone = true;
			}
			break;
	
		default:
			if (_vm->_mouseClickState._left && _vm->_gnapActionStatus < 0)
				_vm->_mouseClickState._left = false;
			break;
		
		}
	
		updateAnimations();
		_vm->checkGameKeys();
	
		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
		}
		
		_vm->gameUpdateTick();
	}
}

void Scene40::updateAnimations() {
	GameSys gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		if (_vm->_gnapActionStatus)
			_vm->_gnapActionStatus = -1;
		else
			_vm->_sceneDone = true;
	}
}

/*****************************************************************************/

enum {
	kHS41Platypus		= 0,
	kHS41ExitCave		= 1,
	kHS41Exit			= 2,
	kHS41ExitBBQ		= 3,
	kHS41ToyVendor	= 4,
	kHS41Kid			= 5,
	kHS41ToyUfo		= 6,
	kHS41Device		= 7,
	kHS41WalkArea1	= 8
};

enum {
	kHS41UfoExitLeft	= 1,
	kHS41UfoExitRight	= 2,
	kHS41UfoDevice	= 3,
	kHS41UfoWalkArea1	= 4
};

enum {
	kAS41LeaveScene				= 0,
	kAS41UseQuarterWithToyVendor	= 1,
	kAS41TalkToyVendor			= 2,
	kAS41UseGumWithToyUfo			= 3,
	kAS41UseChickenBucketWithKid	= 4,
	kAS41GrabKid					= 5,
	kAS41GiveBackToyUfo			= 6,
	kAS41ToyUfoLeaveScene			= 7,
	kAS41ToyUfoRefresh			= 8,
	kAS41UfoGumAttached			= 9
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
		_vm->setHotspot(kHS41Platypus, 0, 0, 0, 0, SF_DISABLED);
		_vm->setHotspot(kHS41UfoExitLeft, 0, 0, 10, 500, SF_EXIT_L_CURSOR | SF_DISABLED);
		_vm->setHotspot(kHS41UfoExitRight, 790, 0, 799, 500, SF_EXIT_R_CURSOR);
		_vm->setHotspot(kHS41UfoWalkArea1, 0, 0, 800, 470, SF_DISABLED);
		_vm->setDeviceHotspot(kHS41UfoDevice, -1, -1, -1, -1);
		_vm->_hotspotsCount = 5;
	} else {
		_vm->setHotspot(kHS41Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHS41ExitCave, 150, 590, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
		_vm->setHotspot(kHS41Exit, 0, 100, 10, 599, SF_EXIT_L_CURSOR | SF_DISABLED, 0, 8);
		_vm->setHotspot(kHS41ExitBBQ, 790, 100, 799, 599, SF_EXIT_R_CURSOR | SF_WALKABLE, 10, 8);
		_vm->setHotspot(kHS41ToyVendor, 320, 150, 430, 310, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHS41Kid, 615, 340, 710, 460, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHS41ToyUfo, 0, 0, 0, 0, SF_GRAB_CURSOR);
		_vm->setHotspot(kHS41WalkArea1, 0, 0, 800, 470);
		_vm->setDeviceHotspot(kHS41Device, -1, -1, -1, -1);
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
			_vm->_gnapActionStatus = kAS41GiveBackToyUfo;
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
			_vm->_hotspots[kHS41ToyUfo]._x1 = _vm->_toyUfoX - 25;
			_vm->_hotspots[kHS41ToyUfo]._y1 = _vm->_toyUfoY - 20;
			_vm->_hotspots[kHS41ToyUfo]._x2 = _vm->_toyUfoX + 25;
			_vm->_hotspots[kHS41ToyUfo]._y2 = _vm->_toyUfoY + 20;
		}
	
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
	
		_vm->testWalk(0, 0, -1, -1, -1, -1);
	
		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);
	
		if (_vm->isFlag(kGFGnapControlsToyUFO)) {
			switch (_vm->_sceneClickedHotspot) {
			case kHS41UfoExitLeft:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = kAS41ToyUfoLeaveScene;
					_vm->_newSceneNum = 45;
					_vm->toyUfoFlyTo(-35, -1, -35, 799, 0, 300, 2);
				}
				break;
	
			case kHS41UfoExitRight:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = kAS41ToyUfoLeaveScene;
					_vm->_newSceneNum = 42;
					_vm->toyUfoFlyTo(835, -1, 0, 835, 0, 300, 2);
				}
				break;
	
			case kHS41UfoDevice:
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(100) + 100;
				_vm->_timers[5] = _vm->getRandom(30) + 20;
				break;
			}
		} else {
			switch (_vm->_sceneClickedHotspot) {
			case kHS41Device:
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(100) + 100;
				_vm->_timers[5] = _vm->getRandom(30) + 20;
				break;
	
			case kHS41Platypus:
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
	
			case kHS41ExitCave:
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS41ExitCave].x, _vm->_hotspotsWalkPos[kHS41ExitCave].y, 0, 0x107AE, 1);
				_vm->_gnapActionStatus = kAS41LeaveScene;
				_vm->_newSceneNum = 40;
				break;
	
			case kHS41Exit:
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS41Exit].x, -1, 0, 0x107AF, 1);
				_vm->_gnapActionStatus = kAS41LeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS41Exit].x, -1, -1, 0x107CF, 1);
				_vm->_newSceneNum = 45;
				break;
	
			case kHS41ExitBBQ:
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS41ExitBBQ].x, -1, 0, 0x107AB, 1);
				_vm->_gnapActionStatus = kAS41LeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS41ExitBBQ].x, -1, -1, 0x107CD, 1);
				_vm->_newSceneNum = 42;
				break;
	
			case kHS41ToyVendor:
				if (_vm->_grabCursorSpriteIndex == kItemDiceQuarterHole) {
					_vm->_gnapActionStatus = kAS41UseQuarterWithToyVendor;
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
						_vm->_gnapActionStatus = kAS41TalkToyVendor;
						break;
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
				break;
	
			case kHS41Kid:
				if (_vm->_grabCursorSpriteIndex == kItemChickenBucket) {
					_vm->gnapWalkTo(7, 7, 0, 0x107BB, 1);
					_vm->_gnapIdleFacing = kDirUpRight;
					_vm->_gnapActionStatus = kAS41UseChickenBucketWithKid;
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
						_vm->_gnapActionStatus = kAS41GrabKid;
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
	
			case kHS41ToyUfo:
				if (_vm->_grabCursorSpriteIndex == kItemGum) {
					_vm->playGnapPullOutDevice(9, 0);
					gameSys.setAnimation(makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, 0);
					_vm->_gnapActionStatus = kAS41UseGumWithToyUfo;
				}
				break;
	
			case kHS41WalkArea1:
				if (_vm->_gnapActionStatus < 0)
					_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				break;
			}
		}
	
		if (_vm->isFlag(kGFGnapControlsToyUFO)) {
			if (!_vm->_timers[9] && _vm->_gnapActionStatus < 0) {
				_vm->_gnapActionStatus = kAS41GiveBackToyUfo;
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
				_vm->_toyUfoActionStatus = kAS41ToyUfoRefresh;
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
		case kAS41LeaveScene:
			gameSys.setAnimation(0, 0, 0);
			_vm->_sceneDone = true;
			_vm->_gnapActionStatus = -1;
			break;
		case kAS41UseQuarterWithToyVendor:
			gameSys.setAnimation(0, 0, 0);
			_nextToyVendorSequenceId = 0x114;
			_vm->_gnapActionStatus = -1;
			break;
		case kAS41TalkToyVendor:
			gameSys.setAnimation(0, 0, 0);
			_nextToyVendorSequenceId = 0x116;
			_vm->_gnapActionStatus = -1;
			break;
		case kAS41UseGumWithToyUfo:
			gameSys.setAnimation(0, 0, 0);
			_vm->playGnapUseDevice(9, 0);
			_vm->_gnapActionStatus = -1;
			_vm->setGrabCursorSprite(-1);
			_vm->invRemove(kItemGum);
			_vm->_toyUfoActionStatus = kAS41UfoGumAttached;
			break;
		case kAS41UseChickenBucketWithKid:
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
		case kAS41GrabKid:
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
		case kAS41GiveBackToyUfo:
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
		case kAS41ToyUfoLeaveScene:
			_vm->_sceneDone = true;
			break;
		case kAS41UfoGumAttached:
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
			_vm->_toyUfoActionStatus = kAS41ToyUfoRefresh;
			_vm->toyUfoFlyTo(_vm->getRandom(300) + 500, _vm->getRandom(225) + 75, 0, 799, 0, 300, 2);
		}
	}
}

/*****************************************************************************/

enum {
	kHS42Platypus		= 0,
	kHS42ExitUfoParty	= 1,
	kHS42ExitToyStand	= 2,
	kHS42ExitUfo		= 3,
	kHS42BBQVendor	= 4,
	kHS42ChickenLeg	= 5,
	kHS42Device		= 6,
	kHS42WalkArea1	= 7,
	kHS42WalkArea2	= 8
};

enum {
	kHS42UfoExitLeft	= 1,
	kHS42UfoExitRight	= 2,
	kHS42UfoHotSauce	= 3,
	kHS42UfoDevice	= 4
};

enum {
	kAS42LeaveScene					= 0,
	kAS42TalkBBQVendor				= 1,
	kAS42UseQuarterWithBBQVendor		= 2,
	kAS42UseQuarterWithBBQVendorDone	= 3,
	kAS42GrabChickenLeg				= 4,
	kAS42ToyUfoLeaveScene				= 5,
	kAS42ToyUfoRefresh				= 6,
	kAS42ToyUfoPickUpHotSauce			= 7
};

Scene42::Scene42(GnapEngine *vm) : Scene(vm) {
	_currBBQVendorSequenceId = -1;
	_nextBBQVendorSequenceId = -1;
}

int Scene42::init() {
	GameSys gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 1);
	gameSys.setAnimation(0, 0, 2);
	if (_vm->isFlag(kGFPictureTaken) || (_vm->isFlag(kGFUnk18) && _vm->isFlag(kGFUnk23)))
		return 0x153;
	return 0x152;
}

void Scene42::updateHotspots() {
	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->setHotspot(kHS42Platypus, 0, 0, 0, 0, SF_DISABLED);
		_vm->setHotspot(kHS42UfoExitLeft, 0, 0, 10, 599, SF_EXIT_L_CURSOR);
		_vm->setHotspot(kHS42UfoExitRight, 790, 0, 799, 599, SF_EXIT_R_CURSOR);
		_vm->setHotspot(kHS42UfoHotSauce, 335, 110, 440, 175, SF_DISABLED);
		_vm->setDeviceHotspot(kHS42UfoDevice, -1, 534, -1, 599);
		if ((_vm->isFlag(kGFPictureTaken) || _vm->isFlag(kGFUnk18)) && _vm->isFlag(kGFUnk23) && !_vm->isFlag(kGFUnk24))
			_vm->_hotspots[kHS42UfoHotSauce]._flags = SF_GRAB_CURSOR;
		_vm->_hotspotsCount = 5;
	} else {
		_vm->setHotspot(kHS42Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHS42ExitUfoParty, 150, 585, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
		_vm->setHotspot(kHS42ExitToyStand, 0, 100, 10, 599, SF_EXIT_L_CURSOR, 0, 8);
		_vm->setHotspot(kHS42ExitUfo, 790, 100, 799, 599, SF_EXIT_R_CURSOR, 10, 8);
		_vm->setHotspot(kHS42BBQVendor, 410, 200, 520, 365, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 4, 8);
		_vm->setHotspot(kHS42ChickenLeg, 530, 340, 620, 430, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 8, 7);
		_vm->setHotspot(kHS42WalkArea1, 0, 0, 800, 445);
		_vm->setHotspot(kHS42WalkArea2, 240, 0, 550, 495);
		_vm->setDeviceHotspot(kHS42Device, -1, -1, -1, -1);
		_vm->_hotspotsCount = 9;
	}
}

void Scene42::run() {
	GameSys gameSys = *_vm->_gameSys;

	_vm->queueInsertDeviceIcon();

	_currBBQVendorSequenceId = 0x14A;
	_nextBBQVendorSequenceId = -1;

	gameSys.setAnimation(0x14A, 1, 2);
	gameSys.insertSequence(_currBBQVendorSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->_toyUfoId = 0;
		_vm->_toyUfoActionStatus = -1;
		if (_vm->_prevSceneNum == 43 && _vm->isFlag(kGFUnk18)) {
			_vm->_toyUfoSequenceId = 0x872;
			_vm->_toyUfoNextSequenceId = _vm->toyUfoGetSequenceId();
			gameSys.insertSequence(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_toyUfoX = 317;
			_vm->_toyUfoY = 61;
			_vm->toyUfoSetStatus(kGFJointTaken);
			_vm->setFlag(kGFPictureTaken);
			_vm->_timers[9] = 600;
		} else {
			_vm->_toyUfoSequenceId = _vm->toyUfoGetSequenceId();
			_vm->_toyUfoNextSequenceId = _vm->_toyUfoSequenceId;
			if (_vm->_prevSceneNum == 41)
				_vm->_toyUfoX = 30;
			else
				_vm->_toyUfoX = 770;
			gameSys.insertSequence(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 0, 0, kSeqNone, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
		}
		gameSys.setAnimation(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 3);
		_vm->endSceneInit();
		if (_vm->_toyUfoSequenceId == 0x872)
			_vm->setGrabCursorSprite(-1);
	} else if (_vm->_prevSceneNum == 41) {
		_vm->initGnapPos(-1, 8, kDirUpRight);
		_vm->initPlatypusPos(-1, 9, kDirUpLeft);
		_vm->endSceneInit();
		_vm->gnapWalkTo(2, 8, -1, 0x107B9, 1);
		_vm->platypusWalkTo(1, 8, -1, 0x107C2, 1);
	} else if (_vm->_prevSceneNum == 43) {
		_vm->initGnapPos(11, 8, kDirUpRight);
		_vm->initPlatypusPos(11, 9, kDirUpLeft);
		_vm->endSceneInit();
		_vm->gnapWalkTo(8, 8, -1, 0x107BA, 1);
		_vm->platypusWalkTo(9, 8, -1, 0x107D2, 1);
	} else {
		_vm->initGnapPos(5, 11, kDirUpRight);
		_vm->initPlatypusPos(6, 11, kDirUpLeft);
		_vm->endSceneInit();
		_vm->gnapWalkTo(5, 8, -1, 0x107BA, 1);
		_vm->platypusWalkTo(6, 8, -1, 0x107C2, 1);
	}

	while (!_vm->_sceneDone) {
		if (!_vm->isSoundPlaying(0x1094B))
			_vm->playSound(0x1094B, true);

		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->testWalk(0, 0, -1, -1, -1, -1);

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		if (_vm->isFlag(kGFGnapControlsToyUFO)) {
			switch (_vm->_sceneClickedHotspot) {
			case kHS42UfoExitLeft:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = kAS42ToyUfoLeaveScene;
					_vm->_newSceneNum = 41;
					_vm->toyUfoFlyTo(-35, -1, -35, 799, 0, 300, 3);
				}
				break;

			case kHS42UfoExitRight:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = kAS42ToyUfoLeaveScene;
					_vm->_newSceneNum = 43;
					_vm->toyUfoFlyTo(835, -1, 0, 835, 0, 300, 3);
				}
				break;

			case kHS42UfoHotSauce:
				if (_vm->isFlag(kGFJointTaken)) {
					_vm->_toyUfoActionStatus = kAS42ToyUfoPickUpHotSauce;
					_vm->toyUfoFlyTo(384, 77, 0, 799, 0, 300, 3);
					_vm->_timers[9] = 600;
				} else {
					_vm->_toyUfoActionStatus = kAS42ToyUfoRefresh;
					_vm->toyUfoFlyTo(-1, -1, 0, 799, 0, 300, 3);
				}
				break;

			case kHS42UfoDevice:
				_vm->runMenu();
				updateHotspots();
				break;
			}
		} else {
			switch (_vm->_sceneClickedHotspot) {
			case kHS42Device:
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(20) + 30;
				break;

			case kHS42Platypus:
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

			case kHS42ExitUfoParty:
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_gnapX, _vm->_hotspotsWalkPos[kHS42ExitUfoParty].y, 0, 0x107AE, 1);
				_vm->_gnapActionStatus = kAS42LeaveScene;
				_vm->platypusWalkTo(_vm->_platX, _vm->_hotspotsWalkPos[kHS42ExitUfoParty].y, -1, 0x107C7, 1);
				_vm->_newSceneNum = 40;
				break;

			case kHS42ExitToyStand:
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS42ExitToyStand].x, _vm->_gnapY, 0, 0x107AF, 1);
				_vm->_gnapActionStatus = kAS42LeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS42ExitToyStand].x, _vm->_hotspotsWalkPos[kHS42ExitToyStand].y, -1, 0x107CF, 1);
				_vm->_newSceneNum = 41;
				break;

			case kHS42ExitUfo:
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS42ExitUfo].x, _vm->_gnapY, 0, 0x107AB, 1);
				_vm->_gnapActionStatus = kAS42LeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS42ExitUfo].x, _vm->_hotspotsWalkPos[kHS42ExitUfo].y, -1, 0x107CD, 1);
				_vm->_newSceneNum = 43;
				break;

			case kHS42BBQVendor:
				if (_vm->_grabCursorSpriteIndex == kItemDiceQuarterHole) {
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS42BBQVendor].x, _vm->_hotspotsWalkPos[kHS42BBQVendor].y, 0, 0x107BB, 1);
					_vm->_gnapActionStatus = kAS42UseQuarterWithBBQVendor;
					if (_vm->_platY < 9)
						_vm->platypusWalkTo(_vm->_platX, 9, -1, -1, 1);
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHS42BBQVendor].x, _vm->_hotspotsWalkPos[kHS42BBQVendor].y, _vm->_hotspotsWalkPos[kHS42BBQVendor].x + 1, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(_vm->_hotspotsWalkPos[kHS42BBQVendor].x - 1, 0);
						break;
					case TALK_CURSOR:
						_vm->_gnapIdleFacing = kDirUpRight;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS42BBQVendor].x, _vm->_hotspotsWalkPos[kHS42BBQVendor].y, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_vm->_gnapActionStatus = kAS42TalkBBQVendor;
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
				break;

			case kHS42ChickenLeg:
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHS42ChickenLeg].x, _vm->_hotspotsWalkPos[kHS42ChickenLeg].y, _vm->_hotspotsWalkPos[kHS42ChickenLeg].x - 1, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(_vm->_hotspotsWalkPos[kHS42ChickenLeg].x - 1, 0);
						break;
					case GRAB_CURSOR:
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS42ChickenLeg].x, _vm->_hotspotsWalkPos[kHS42ChickenLeg].y, 0, 0x107BC, 1);
						_vm->_gnapActionStatus = kAS42GrabChickenLeg;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
				break;

			case kHS42WalkArea1:
			case kHS42WalkArea2:
				if (_vm->_gnapActionStatus < 0)
					_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				break;

			}
		}

		if (_vm->_mouseClickState._left && _vm->_gnapActionStatus < 0) {
			_vm->_mouseClickState._left = false;
			if (_vm->isFlag(kGFGnapControlsToyUFO)) {
				_vm->_toyUfoActionStatus = kAS42ToyUfoRefresh;
				_vm->toyUfoFlyTo(-1, -1, 0, 799, 0, 300, 3);
			} else {
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			}
		}

		updateAnimations();

		_vm->toyUfoCheckTimer();

		if (!_vm->_isLeavingScene) {
			if (_vm->_platypusActionStatus < 0 && !_vm->isFlag(kGFGnapControlsToyUFO))
				_vm->updatePlatypusIdleSequence();
			if (_vm->_gnapActionStatus < 0 && !_vm->isFlag(kGFGnapControlsToyUFO))
				_vm->updateGnapIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(20) + 30;
				if (_vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0 && _nextBBQVendorSequenceId == -1) {
					switch (_vm->getRandom(8)) {
					case 0:
						_nextBBQVendorSequenceId = 0x14C;
						break;
					case 1:
					case 2:
						_nextBBQVendorSequenceId = 0x149;
						break;
					case 3:
					case 4:
					case 5:
					case 6:
						_nextBBQVendorSequenceId = 0x14D;
						break;
					case 7:
						_nextBBQVendorSequenceId = 0x14A;
						break;
					}
					if (_nextBBQVendorSequenceId == _currBBQVendorSequenceId && _nextBBQVendorSequenceId != 0x14D)
						_nextBBQVendorSequenceId = -1;
				}
			}
		}

		_vm->checkGameKeys();

		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[4] = _vm->getRandom(20) + 30;
		}

		_vm->gameUpdateTick();
	}
}

void Scene42::updateAnimations() {
	GameSys gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(0) == 2) {
		switch (_vm->_gnapActionStatus) {
		case kAS42LeaveScene:
			gameSys.setAnimation(0, 0, 0);
			_vm->_gnapActionStatus = -1;
			_vm->_sceneDone = true;
			break;
		case kAS42TalkBBQVendor:
			gameSys.setAnimation(0, 0, 0);
			_vm->_gnapActionStatus = -1;
			_nextBBQVendorSequenceId = 0x14B;
			break;
		case kAS42UseQuarterWithBBQVendor:
		case kAS42GrabChickenLeg:
			if (gameSys.getAnimationStatus(2) == 2) {
				int sequenceId;
				if (_vm->_gnapActionStatus == kAS42UseQuarterWithBBQVendor) {
					_vm->invRemove(kItemDiceQuarterHole);
					_vm->invAdd(kItemChickenBucket);
					_vm->setGrabCursorSprite(-1);
					sequenceId = 0x150;
					_nextBBQVendorSequenceId = 0x148;
				} else if (_vm->isFlag(kGFUnk27)) {
					if (_vm->isFlag(kGFUnk28)) {
						sequenceId = 0x7B7;
						_nextBBQVendorSequenceId = 0x145;
					} else {
						_vm->setFlag(kGFUnk28);
						sequenceId = 0x14F;
						_nextBBQVendorSequenceId = 0x147;
					}
				} else {
					_vm->setFlag(kGFUnk27);
					sequenceId = 0x14E;
					_nextBBQVendorSequenceId = 0x146;
				}
				if (sequenceId == 0x7B7) {
					gameSys.insertSequence(0x107B7, _vm->_gnapId,
						makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId,
						kSeqSyncWait, _vm->getSequenceTotalDuration(_nextBBQVendorSequenceId),
						75 * _vm->_gnapX - _vm->_gnapGridX, 48 * _vm->_gnapY - _vm->_gnapGridY);
					_vm->_gnapSequenceDatNum = 1;
				} else {
					gameSys.insertSequence(sequenceId, _vm->_gnapId,
						makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId,
						kSeqSyncWait, 0, 0, 0);
					_vm->_gnapSequenceDatNum = 0;
				}
				_vm->_gnapSequenceId = sequenceId;
				gameSys.setAnimation(sequenceId | (_vm->_gnapSequenceDatNum << 16), _vm->_gnapId, 0);
				if (_vm->_gnapActionStatus == kAS42UseQuarterWithBBQVendor)
					_vm->_gnapActionStatus = kAS42UseQuarterWithBBQVendorDone;
				else
					_vm->_gnapActionStatus = -1;
				gameSys.insertSequence(_nextBBQVendorSequenceId, 1, _currBBQVendorSequenceId, 1, kSeqSyncWait, 0, 0, 0);
				gameSys.setAnimation(_nextBBQVendorSequenceId, 1, 2);
				_currBBQVendorSequenceId = _nextBBQVendorSequenceId;
				if (_nextBBQVendorSequenceId == 0x145)
					_nextBBQVendorSequenceId = 0x14A;
				else
					_nextBBQVendorSequenceId = -1;
				_vm->_timers[4] = _vm->getRandom(20) + 30;
				_vm->_timers[2] = _vm->getRandom(30) + 20;
				_vm->_timers[3] = _vm->getRandom(50) + 200;
			}
			break;
		case kAS42UseQuarterWithBBQVendorDone:
			gameSys.setAnimation(0, 0, 0);
			_vm->setGrabCursorSprite(kItemChickenBucket);
			_vm->_gnapActionStatus = -1;
			break;
		default:
			gameSys.setAnimation(0, 0, 0);
			_vm->_gnapActionStatus = -1;
			break;
		}
	}

	if (gameSys.getAnimationStatus(2) == 2 && _nextBBQVendorSequenceId != -1) {
		gameSys.insertSequence(_nextBBQVendorSequenceId, 1, _currBBQVendorSequenceId, 1, kSeqSyncWait, 0, 0, 0);
		gameSys.setAnimation(_nextBBQVendorSequenceId, 1, 2);
		_currBBQVendorSequenceId = _nextBBQVendorSequenceId;
		_nextBBQVendorSequenceId = -1;
		_vm->_timers[4] = _vm->getRandom(20) + 30;
	}

	if (gameSys.getAnimationStatus(3) == 2) {
		switch (_vm->_toyUfoActionStatus) {
		case kAS42ToyUfoLeaveScene:
			_vm->_sceneDone = true;
			break;
		case kAS42ToyUfoPickUpHotSauce:
			gameSys.insertSequence(0x10870, _vm->_toyUfoId, _vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, kSeqSyncWait, 0, 0, 0);
			_vm->setFlag(kGFUnk24);
			updateHotspots();
			_vm->toyUfoSetStatus(kGFGroceryStoreHatTaken);
			_vm->_toyUfoSequenceId = 0x870;
			gameSys.setAnimation(0x10870, _vm->_toyUfoId, 3);
			_vm->_toyUfoActionStatus = -1;
			_vm->_toyUfoX = 0x181;
			_vm->_toyUfoY = 53;
			break;
		default:
			if (_vm->_toyUfoSequenceId == 0x872) {
				_vm->hideCursor();
				_vm->addFullScreenSprite(0x13E, 255);
				gameSys.setAnimation(0x151, 256, 0);
				gameSys.insertSequence(0x151, 256, 0, 0, kSeqNone, 0, 0, 0);
				while (gameSys.getAnimationStatus(0) != 2)
					_vm->gameUpdateTick();
				_vm->removeFullScreenSprite();
				_vm->showCursor();
			}
			_vm->_toyUfoNextSequenceId = _vm->toyUfoGetSequenceId();
			gameSys.setAnimation(_vm->_toyUfoNextSequenceId | 0x10000, (_vm->_toyUfoId + 1) % 10, 3);
			gameSys.insertSequence(_vm->_toyUfoNextSequenceId | 0x10000, (_vm->_toyUfoId + 1) % 10,
				_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId,
				kSeqSyncWait, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
			_vm->_toyUfoSequenceId = _vm->_toyUfoNextSequenceId;
			_vm->_toyUfoId = (_vm->_toyUfoId + 1) % 10;
			break;
		}
		_vm->_toyUfoActionStatus = -1;
	}
}

/*****************************************************************************/

enum {
	kHS43Platypus			= 0,
	kHS43Device			= 1,
	kHS43ExitUfoParty		= 2,
	kHS43ExitBBQ			= 3,
	kHS43ExitKissinBooth	= 4,
	kHS43TwoHeadedGuy		= 5,
	kHS43Key				= 6,
	kHS43Ufo				= 7,
	kHS43WalkArea1		= 8,
	kHS43WalkArea2		= 9
};

enum {
	kHS43UfoExitLeft	= 1,
	kHS43UfoExitRight	= 2,
	kHS43UfoKey		= 3,
	kHS43UfoBucket	= 4,
	kHS43UfoDevice	= 5
};

Scene43::Scene43(GnapEngine *vm) : Scene(vm) {
	_currTwoHeadedGuySequenceId = -1;
	_nextTwoHeadedGuySequenceId = -1;
}

int Scene43::init() {
	GameSys gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 1);
	gameSys.setAnimation(0, 0, 2);
	return 0x13F;
}

void Scene43::updateHotspots() {
	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->setHotspot(kHS43Platypus, 0, 0, 0, 0, SF_DISABLED);
		_vm->setHotspot(kHS43UfoExitLeft, 0, 0, 10, 599, SF_EXIT_L_CURSOR);
		_vm->setHotspot(kHS43UfoExitRight, 790, 0, 799, 599, SF_EXIT_R_CURSOR);
		_vm->setHotspot(kHS43UfoKey, 140, 170, 185, 260, SF_GRAB_CURSOR);
		_vm->setHotspot(kHS43UfoBucket, 475, 290, 545, 365, SF_DISABLED);
		_vm->setDeviceHotspot(kHS43UfoDevice, -1, 534, -1, 599);
		if (_vm->isFlag(kGFGroceryStoreHatTaken))
			_vm->_hotspots[kHS43UfoBucket]._flags = SF_GRAB_CURSOR;
		// NOTE Bug in the original. Key hotspot wasn't disabled.
		if (_vm->isFlag(kGFUnk14))
			_vm->_hotspots[kHS43UfoKey]._flags = SF_DISABLED;
		_vm->_hotspotsCount = 6;
	} else {
		_vm->setHotspot(kHS43Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHS43ExitUfoParty, 150, 580, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
		_vm->setHotspot(kHS43ExitBBQ, 0, 100, 10, 599, SF_EXIT_L_CURSOR, 0, 8);
		_vm->setHotspot(kHS43ExitKissinBooth, 790, 100, 799, 599, SF_EXIT_R_CURSOR, 10, 8);
		_vm->setHotspot(kHS43TwoHeadedGuy, 470, 240, 700, 470, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHS43Key, 140, 170, 185, 260, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHS43Ufo, 110, 0, 690, 350, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHS43WalkArea1, 0, 0, 800, 445);
		_vm->setHotspot(kHS43WalkArea2, 465, 0, 800, 493);
		_vm->setDeviceHotspot(kHS43Device, -1, -1, -1, -1);
        if (_vm->isFlag(kGFUnk14))
			_vm->_hotspots[kHS43Key]._flags = SF_DISABLED;
		_vm->_hotspotsCount = 10;
	}
}

void Scene43::run() {
	GameSys gameSys = *_vm->_gameSys;

	_vm->queueInsertDeviceIcon();

	if (!_vm->isFlag(kGFUnk14))
		gameSys.insertSequence(0x1086F, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	_currTwoHeadedGuySequenceId = 0x13C;
	_nextTwoHeadedGuySequenceId = -1;
	
	gameSys.setAnimation(0x13C, 1, 2);
	gameSys.insertSequence(_currTwoHeadedGuySequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->_toyUfoId = 0;
		_vm->_toyUfoActionStatus = -1;
		_vm->_toyUfoSequenceId = _vm->toyUfoGetSequenceId();
		_vm->_toyUfoNextSequenceId = _vm->_toyUfoSequenceId;
		if (_vm->_prevSceneNum == 42)
			_vm->_toyUfoX = 30;
		else
			_vm->_toyUfoX = 770;
		gameSys.setAnimation(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 3);
		gameSys.insertSequence(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 0, 0, kSeqNone, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
		_vm->endSceneInit();
	} else {
		switch (_vm->_prevSceneNum) {
		case 42:
			_vm->initGnapPos(-1, 8, kDirUpRight);
			_vm->initPlatypusPos(-1, 9, kDirUpLeft);
			_vm->endSceneInit();
			_vm->gnapWalkTo(2, 8, -1, 0x107B9, 1);
			_vm->platypusWalkTo(1, 8, -1, 0x107C2, 1);
			break;
		case 44:
			_vm->initGnapPos(11, 8, kDirUpRight);
			_vm->initPlatypusPos(11, 9, kDirUpLeft);
			_vm->endSceneInit();
			_vm->gnapWalkTo(8, 8, -1, 0x107BA, 1);
			_vm->platypusWalkTo(9, 8, -1, 0x107D2, 1);
			break;
		case 54:
			_vm->initGnapPos(4, 7, kDirBottomLeft);
			_vm->initPlatypusPos(11, 8, kDirUpLeft);
			_vm->endSceneInit();
			_vm->platypusWalkTo(9, 8, -1, 0x107D2, 1);
			break;
		default:
			_vm->initGnapPos(5, 11, kDirUpRight);
			_vm->initPlatypusPos(6, 11, kDirUpLeft);
			_vm->endSceneInit();
			_vm->gnapWalkTo(5, 8, -1, 0x107BA, 1);
			_vm->platypusWalkTo(6, 8, -1, 0x107C2, 1);
			break;
		}
	}
	
	while (!_vm->_sceneDone) {
		if (!_vm->isSoundPlaying(0x1094B))
			_vm->playSound(0x1094B, true);
	
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
	
		_vm->testWalk(0, 0, -1, -1, -1, -1);
	
		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);
	
		if (_vm->isFlag(kGFGnapControlsToyUFO)) {
			switch (_vm->_sceneClickedHotspot) {
			case kHS43UfoDevice:
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(100) + 100;
				break;
			
			case kHS43UfoExitLeft:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = 4;
					_vm->_newSceneNum = 42;
					_vm->toyUfoFlyTo(-35, -1, -35, 799, 0, 300, 3);
				}
				break;
			
			case kHS43UfoExitRight:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = 4;
					_vm->_newSceneNum = 44;
					_vm->toyUfoFlyTo(835, -1, 0, 835, 0, 300, 3);
				}
				break;
			
			case kHS43UfoKey:
				if (_vm->isFlag(kGFJointTaken)) {
					_vm->_toyUfoActionStatus = 6;
					_vm->toyUfoFlyTo(163, 145, 0, 799, 0, 300, 3);
				} else {
					_vm->_toyUfoActionStatus = 5;
					_vm->toyUfoFlyTo(-1, -1, 0, 799, 0, 300, 3);
				}
				break;
			
			case kHS43UfoBucket:
				_vm->_toyUfoActionStatus = 7;
				_vm->toyUfoFlyTo(497, 143, 0, 799, 0, 300, 3);
				_vm->_timers[9] = 600;
				break;
			}
		} else {
			switch (_vm->_sceneClickedHotspot) {
			case kHS43Device:
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(100) + 100;
				break;

			case kHS43Platypus:
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

			case kHS43ExitUfoParty:
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS43ExitUfoParty].x, _vm->_hotspotsWalkPos[kHS43ExitUfoParty].y, 0, 0x107AE, 1);
				_vm->_gnapActionStatus = 0;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS43ExitUfoParty].x, _vm->_hotspotsWalkPos[kHS43ExitUfoParty].y, -1, 0x107C7, 1);
				_vm->_newSceneNum = 40;
				break;

			case kHS43ExitBBQ:
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS43ExitBBQ].x, _vm->_gnapY, 0, 0x107AF, 1);
				_vm->_gnapActionStatus = 0;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS43ExitBBQ].x, _vm->_hotspotsWalkPos[kHS43ExitBBQ].y, -1, 0x107CF, 1);
				_vm->_newSceneNum = 42;
				break;

			case kHS43ExitKissinBooth:
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS43ExitKissinBooth].x, _vm->_gnapY, 0, 0x107AB, 1);
				_vm->_gnapActionStatus = 0;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS43ExitKissinBooth].x, _vm->_hotspotsWalkPos[kHS43ExitKissinBooth].y, -1, 0x107CD, 1);
				_vm->_newSceneNum = 44;
				break;

			case kHS43TwoHeadedGuy:
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(6, 8, 7, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(7, 0);
						break;
					case TALK_CURSOR:
							_vm->_gnapIdleFacing = kDirUpRight;
							_vm->gnapWalkTo(5, 8, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
							_vm->_gnapActionStatus = 2;
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
				break;

			case kHS43Key:
			case kHS43Ufo:
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(3, 7, 2, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(0, 0);
						break;
					case GRAB_CURSOR:
						_vm->_gnapIdleFacing = kDirUpRight;
						_vm->gnapWalkTo(3, 7, 0, 67515, 1);
						_vm->_gnapActionStatus = 1;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
				break;

			case kHS43WalkArea1:
			case kHS43WalkArea2:
				if (_vm->_gnapActionStatus < 0)
					_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				break;
			}
		}
	
		if (_vm->_mouseClickState._left && _vm->_gnapActionStatus < 0) {
			_vm->_mouseClickState._left = false;
			if (_vm->isFlag(kGFGnapControlsToyUFO) && (_vm->_toyUfoActionStatus == 5 || _vm->_toyUfoActionStatus == -1)) {
				_vm->_toyUfoActionStatus = 5;
				_vm->toyUfoFlyTo(-1, -1, 0, 799, 0, 300, 3);
			} else {
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			}
		}
	
		updateAnimations();
	
		_vm->toyUfoCheckTimer();
	
		if (!_vm->_isLeavingScene) {
			if (_vm->_platypusActionStatus < 0 && !_vm->isFlag(kGFGnapControlsToyUFO))
				_vm->updatePlatypusIdleSequence();
			if (_vm->_gnapActionStatus < 0 && !_vm->isFlag(kGFGnapControlsToyUFO))
				_vm->updateGnapIdleSequence();
			if (!_vm->_timers[4] && (!_vm->isFlag(kGFGnapControlsToyUFO) || !_vm->isFlag(kGFGroceryStoreHatTaken))) {
				_vm->_timers[4] = _vm->getRandom(100) + 100;
				if (_vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0 && _nextTwoHeadedGuySequenceId == -1) {
					switch (_vm->getRandom(5)) {
					case 0:
						_nextTwoHeadedGuySequenceId = 0x13C;
						break;
					case 1:
						_nextTwoHeadedGuySequenceId = 0x134;
						break;
					case 2:
						_nextTwoHeadedGuySequenceId = 0x135;
						break;
					case 3:
						_nextTwoHeadedGuySequenceId = 0x136;
						break;
					case 4:
						_nextTwoHeadedGuySequenceId = 0x13A;
						break;
					}
					if (_nextTwoHeadedGuySequenceId == _currTwoHeadedGuySequenceId)
						_nextTwoHeadedGuySequenceId = -1;
				}
			}
		}
	
		_vm->checkGameKeys();
	
		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[4] = _vm->getRandom(100) + 100;
		}
		
		_vm->gameUpdateTick();
	}
	
	if (_vm->_newSceneNum == 54)
		_vm->clearFlag(kGFGnapControlsToyUFO);
}

void Scene43::updateAnimations() {
	GameSys gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(0) == 2) {
		switch (_vm->_gnapActionStatus) {
		case 0:
			gameSys.setAnimation(0, 0, 0);
			_vm->_sceneDone = true;
			break;

		case 1:
			if (gameSys.getAnimationStatus(2) == 2) {
				_vm->_timers[2] = _vm->getRandom(30) + 20;
				_vm->_timers[3] = _vm->getRandom(50) + 200;
				gameSys.insertSequence(0x13D, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
				_vm->_gnapSequenceId = 0x13D;
				_vm->_gnapSequenceDatNum = 0;
				gameSys.setAnimation(0x13D, _vm->_gnapId, 0);
				_nextTwoHeadedGuySequenceId = 0x13B;
				gameSys.insertSequence(0x13B, 1, _currTwoHeadedGuySequenceId, 1, kSeqSyncWait, 0, 0, 0);
				gameSys.setAnimation(_nextTwoHeadedGuySequenceId, 1, 2);
				_currTwoHeadedGuySequenceId = _nextTwoHeadedGuySequenceId;
				_nextTwoHeadedGuySequenceId = -1;
				_vm->_timers[4] = _vm->getRandom(100) + 100;
				_vm->_gnapActionStatus = -1;
			}
			break;

		default:
			gameSys.setAnimation(0, 0, 0);
			_vm->_gnapActionStatus = -1;
			break;
		}
	}
	
	if (gameSys.getAnimationStatus(2) == 2) {
		if (_currTwoHeadedGuySequenceId == 0x13A) {
			if (_vm->isFlag(kGFGroceryStoreHatTaken)) {
				_nextTwoHeadedGuySequenceId = 0x13E;
				_vm->stopSound(0x108F6);
			} else if (_vm->getRandom(2) != 0) {
				_nextTwoHeadedGuySequenceId = 0x137;
			} else {
				_nextTwoHeadedGuySequenceId = 0x138;
			}
		} else if (_currTwoHeadedGuySequenceId == 0x13E) {
			_vm->_sceneDone = true;
			_vm->_newSceneNum = 54;
		}
		if (_nextTwoHeadedGuySequenceId != -1) {
			gameSys.insertSequence(_nextTwoHeadedGuySequenceId, 1, _currTwoHeadedGuySequenceId, 1, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(_nextTwoHeadedGuySequenceId, 1, 2);
			_currTwoHeadedGuySequenceId = _nextTwoHeadedGuySequenceId;
			_nextTwoHeadedGuySequenceId = -1;
			_vm->_timers[4] = _vm->getRandom(100) + 100;
		}
	}
	
	if (gameSys.getAnimationStatus(3) == 2) {
		switch (_vm->_toyUfoActionStatus) {
		case 4:
			_vm->_sceneDone = true;
			_vm->_toyUfoActionStatus = -1;
			break;
		case 6:
			gameSys.insertSequence(0x10871, _vm->_toyUfoId, _vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, kSeqSyncWait, 0, 0, 0);
			gameSys.removeSequence(0x1086F, 1, true);
			_vm->setFlag(kGFUnk14);
			updateHotspots();
			_vm->toyUfoSetStatus(kGFUnk18);
			_vm->_toyUfoSequenceId = 0x871;
			gameSys.setAnimation(0x10871, _vm->_toyUfoId, 3);
			_vm->_toyUfoActionStatus = -1;
			_vm->_toyUfoX = 96;
			_vm->_toyUfoY = 131;
			break;
		case 7:
			gameSys.insertSequence(0x10874, _vm->_toyUfoId, _vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, kSeqSyncWait, 0, 0, 0);
			_vm->_toyUfoSequenceId = 0x874;
			gameSys.setAnimation(0x10874, _vm->_toyUfoId, 3);
			_vm->_toyUfoActionStatus = 8;
			_vm->setFlag(kGFJointTaken);
			_vm->_gnapActionStatus = 3;
			break;
		case 8:
			_nextTwoHeadedGuySequenceId = 0x13A;
			_vm->_toyUfoX = 514;
			_vm->_toyUfoY = 125;
			_vm->toyUfoFlyTo(835, 125, 0, 835, 0, 300, 3);
			_vm->_toyUfoActionStatus = 9;
			break;
		case 9:
			// Nothing
			break;
		default:
			_vm->_toyUfoNextSequenceId = _vm->toyUfoGetSequenceId();
			gameSys.insertSequence(_vm->_toyUfoNextSequenceId | 0x10000, _vm->_toyUfoId + 1,
				_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId,
				kSeqSyncWait, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
			_vm->_toyUfoSequenceId = _vm->_toyUfoNextSequenceId;
			++_vm->_toyUfoId;
			gameSys.setAnimation(_vm->_toyUfoNextSequenceId | 0x10000, _vm->_toyUfoId, 3);
			_vm->_toyUfoActionStatus = -1;
			break;
		}
	}
}

/*****************************************************************************/

enum {
	kHS44Platypus		= 0,
	kHS44ExitUfoParty	= 1,
	kHS44ExitUfo		= 2,
	kHS44ExitShow		= 3,
	kHS44KissingLady	= 4,
	kHS44Spring		= 5,
	kHS44SpringGuy	= 6,
	kHS44Device		= 7,
	kHS44WalkArea1	= 8,
	kHS44WalkArea2	= 9
};

enum {
	kHS44UfoExitLeft	= 1,
	kHS44UfoExitRight	= 2,
	kHS44UfoDevice	= 3
};

Scene44::Scene44(GnapEngine *vm) : Scene(vm) {
	_nextSpringGuySequenceId = -1;
	_nextKissingLadySequenceId = -1;
	_currSpringGuySequenceId = -1;
	_currKissingLadySequenceId = -1;
}

int Scene44::init() {
	GameSys gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 1);
	gameSys.setAnimation(0, 0, 2);
	gameSys.setAnimation(0, 0, 3);
	return 0xFF;
}

void Scene44::updateHotspots() {
	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->setHotspot(kHS44Platypus, 0, 0, 0, 0, SF_DISABLED);
		_vm->setHotspot(kHS44UfoExitLeft, 0, 0, 10, 599, SF_EXIT_L_CURSOR);
		_vm->setHotspot(kHS44UfoExitRight, 790, 0, 799, 599, SF_EXIT_R_CURSOR);
		_vm->setDeviceHotspot(kHS44UfoDevice, -1, 534, -1, 599);
		_vm->_hotspotsCount = 4;
	} else {
		_vm->setHotspot(kHS44Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHS44ExitUfoParty, 150, 580, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
		_vm->setHotspot(kHS44ExitUfo, 0, 100, 10, 599, SF_EXIT_L_CURSOR, 0, 8);
		_vm->setHotspot(kHS44ExitShow, 790, 100, 799, 599, SF_EXIT_R_CURSOR, 10, 8);
		_vm->setHotspot(kHS44KissingLady, 300, 160, 400, 315, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 5, 7);
		_vm->setHotspot(kHS44Spring, 580, 310, 635, 375, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 7, 8);
		_vm->setHotspot(kHS44SpringGuy, 610, 375, 690, 515, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 7, 8);
		_vm->setHotspot(kHS44WalkArea1, 0, 0, 800, 445);
		_vm->setHotspot(kHS44WalkArea2, 617, 0, 800, 600);
		_vm->setDeviceHotspot(kHS44Device, -1, -1, -1, -1);
		if (_vm->isFlag(kGFUnk13))
			_vm->_hotspots[kHS44KissingLady]._flags = SF_DISABLED;
		if (_vm->isFlag(kGFSpringTaken))
			_vm->_hotspots[kHS44Spring]._flags = SF_DISABLED;
		_vm->_hotspotsCount = 10;
	}
}

void Scene44::run() {
	GameSys gameSys = *_vm->_gameSys;

	_vm->queueInsertDeviceIcon();

	gameSys.insertSequence(0xF7, 0, 0, 0, kSeqLoop, 0, 0, 0);
	gameSys.insertSequence(0xFC, 256, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->isFlag(kGFSpringTaken))
		_currSpringGuySequenceId = 0xF8;
	else
		_currSpringGuySequenceId = 0xF9;
	
	_nextSpringGuySequenceId = -1;
	gameSys.setAnimation(_currSpringGuySequenceId, 1, 4);
	gameSys.insertSequence(_currSpringGuySequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->isFlag(kGFUnk13)) {
		if (_vm->_prevSceneNum != 50 || _vm->_sceneSavegameLoaded) {
			_currKissingLadySequenceId = 0xF6;
			_nextKissingLadySequenceId = -1;
		} else {
			_vm->setGrabCursorSprite(kItemGum);
			_currKissingLadySequenceId = 0xF5;
			_nextKissingLadySequenceId = 0xF6;
			gameSys.setAnimation(0xF5, 1, 2);
		}
	} else {
		_currKissingLadySequenceId = 0xEC;
		_nextKissingLadySequenceId = -1;
		gameSys.setAnimation(0xEC, 1, 2);
	}
	
	gameSys.insertSequence(_currKissingLadySequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->_toyUfoId = 0;
		_vm->_toyUfoActionStatus = -1;
		_vm->_toyUfoSequenceId = _vm->toyUfoGetSequenceId();
		_vm->_toyUfoNextSequenceId = _vm->_toyUfoSequenceId;
		if (_vm->_prevSceneNum == 43)
			_vm->_toyUfoX = 30;
		else
			_vm->_toyUfoX = 770;
		gameSys.setAnimation(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 3);
		gameSys.insertSequence(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 0, 0, kSeqNone, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
		_vm->endSceneInit();
	} else {
		switch (_vm->_prevSceneNum) {
		case 43:
			_vm->initGnapPos(-1, 8, kDirUpRight);
			_vm->initPlatypusPos(-1, 7, kDirUpLeft);
			_vm->endSceneInit();
			_vm->gnapWalkTo(2, 8, -1, 0x107B9, 1);
			_vm->platypusWalkTo(1, 8, -1, 0x107C2, 1);
			break;
		case 46:
			_vm->initGnapPos(11, 8, kDirUpRight);
			_vm->initPlatypusPos(11, 8, kDirUpLeft);
			_vm->endSceneInit();
			_vm->gnapWalkTo(6, 8, -1, 0x107BA, 1);
			_vm->platypusWalkTo(7, 8, -1, 0x107D2, 1);
			break;
		case 50:
			_vm->initGnapPos(4, 8, kDirBottomRight);
			if (_vm->_sceneSavegameLoaded) {
				_vm->initPlatypusPos(_vm->_hotspotsWalkPos[4].x, _vm->_hotspotsWalkPos[4].y, kDirUnk4);
			} else if (!_vm->isFlag(kGFUnk13)) {
				_vm->_timers[0] = 50;
				_vm->_timers[1] = 20;
				_vm->_platX = 5;
				_vm->_platY = 8;
				_vm->_platypusSequenceId = 0xFD;
				_vm->_platypusFacing = kDirNone;
				_vm->_platypusId = 160;
				_vm->_platypusSequenceDatNum = 0;
				gameSys.insertSequence(0xFD, 160, 0, 0, kSeqNone, 0, 0, 0);
			}
			_vm->endSceneInit();
			break;
		default:
			_vm->initGnapPos(5, 11, kDirUpRight);
			_vm->initPlatypusPos(6, 11, kDirUpLeft);
			_vm->endSceneInit();
			_vm->platypusWalkTo(6, 8, -1, 0x107C2, 1);
			_vm->gnapWalkTo(5, 8, -1, 0x107BA, 1);
			break;
		}
	}
	
	while (!_vm->_sceneDone) {
		if (!_vm->isSoundPlaying(0x1094B))
			_vm->playSound(0x1094B, true);
	
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
	
		_vm->testWalk(0, 0, -1, -1, -1, -1);
	
		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);
	
		if (_vm->isFlag(kGFGnapControlsToyUFO)) {
			switch (_vm->_sceneClickedHotspot) {
			case kHS44UfoExitLeft:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = 6;
					_vm->_newSceneNum = 43;
					_vm->toyUfoFlyTo(-35, -1, -35, 799, 0, 300, 3);
				}
				break;

			case kHS44UfoExitRight:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = 6;
					_vm->_newSceneNum = 46;
					_vm->toyUfoFlyTo(835, -1, 0, 835, 0, 300, 3);
				}
				break;

			case kHS44UfoDevice:
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(20) + 20;
				break;
			}
		} else if (_vm->_sceneClickedHotspot <= 9) {
			switch (_vm->_sceneClickedHotspot) {
			case kHS44Device:
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(20) + 20;
				break;
			
			case kHS44Platypus:
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

			case kHS44ExitUfoParty:
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS44ExitUfoParty].x, _vm->_hotspotsWalkPos[kHS44ExitUfoParty].y, 0, 0x107AE, 1);
				_vm->_gnapActionStatus = 0;
				_vm->_newSceneNum = 40;
				break;
			
			case kHS44ExitUfo:
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS44ExitUfo].x, _vm->_gnapY, 0, 0x107AF, 1);
				_vm->_gnapActionStatus = 0;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS44ExitUfo].x, _vm->_platY, -1, 0x107CF, 1);
				_vm->_newSceneNum = 43;
				break;
			
			case kHS44ExitShow:
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS44ExitShow].x, _vm->_hotspotsWalkPos[kHS44ExitShow].y, 0, 0x107AB, 1);
				_vm->_gnapActionStatus = 0;
				_vm->_newSceneNum = 46;
				break;
			
			case kHS44KissingLady:
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->_gnapActionStatus = 2;
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS44KissingLady].x, _vm->_hotspotsWalkPos[kHS44KissingLady].y, 0, -1, 9);
					_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, _vm->_hotspotsWalkPos[kHS44KissingLady].x - 1, _vm->_hotspotsWalkPos[kHS44KissingLady].y);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(4, 3);
						break;
					case GRAB_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					case TALK_CURSOR:
						_vm->_gnapIdleFacing = kDirUpLeft;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS44KissingLady].x, _vm->_hotspotsWalkPos[kHS44KissingLady].y, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_vm->_gnapActionStatus = 1;
						break;
					case PLAT_CURSOR:
						_vm->gnapUseDeviceOnPlatypuss();
						_vm->platypusWalkTo(6, 7, 1, 0x107D2, 1);
						if (_vm->_gnapX == 7 && _vm->_gnapY == 7)
							_vm->gnapWalkStep();
						_vm->playGnapIdle(5, 7);
						_vm->_platypusActionStatus = 4;
						break;
					}
				}
				break;
			
			case kHS44Spring:
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHS44Spring].x, _vm->_hotspotsWalkPos[kHS44Spring].y, 8, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(8, 7);
						break;
					case GRAB_CURSOR:
						_vm->playGnapPullOutDevice(8, 0);
						_vm->playGnapUseDevice(8, 0);
						_nextSpringGuySequenceId = 0xFB;
						_vm->invAdd(kItemSpring);
						_vm->setFlag(kGFSpringTaken);
						updateHotspots();
						break;
					case TALK_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
				break;
			
			case kHS44SpringGuy:
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHS44SpringGuy].x, _vm->_hotspotsWalkPos[kHS44SpringGuy].y, 8, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						if (_vm->isFlag(kGFSpringTaken))
							_vm->playGnapMoan1(8, 7);
						else
							_vm->playGnapScratchingHead(8, 7);
						break;
					case TALK_CURSOR:
						_vm->_gnapIdleFacing = kDirUpRight;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS44SpringGuy].x, _vm->_hotspotsWalkPos[kHS44SpringGuy].y, -1, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
				break;
			
			case kHS44WalkArea1:
			case kHS44WalkArea2:
				if (_vm->_gnapActionStatus < 0)
					_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				break;
			
			}
		}
	
		if (_vm->_mouseClickState._left && _vm->_gnapActionStatus < 0) {
			_vm->_mouseClickState._left = false;
			if (_vm->isFlag(kGFGnapControlsToyUFO)) {
				_vm->_toyUfoActionStatus = 7;
				_vm->toyUfoFlyTo(-1, -1, 0, 799, 0, 300, 3);
			} else {
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			}
		}
	
		updateAnimations();
		_vm->toyUfoCheckTimer();
	
		if (!_vm->_isLeavingScene) {
			if (_vm->_platypusActionStatus < 0 && !_vm->isFlag(kGFGnapControlsToyUFO) && _currKissingLadySequenceId != 0xF5)
				_vm->updatePlatypusIdleSequence();
			if (_vm->_gnapActionStatus < 0 && !_vm->isFlag(kGFGnapControlsToyUFO))
				_vm->updateGnapIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(20) + 20;
				if (_vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0 && _nextKissingLadySequenceId == -1) {
					switch (_vm->getRandom(20)) {
					case 0:
						_nextKissingLadySequenceId = 0xED;
						break;
					case 1:
						_nextKissingLadySequenceId = 0xEE;
						break;
					case 2:
						_nextKissingLadySequenceId = 0xF0;
						break;
					case 3:
						_nextKissingLadySequenceId = 0xF3;
						break;
					case 4:
						_nextKissingLadySequenceId = 0xF4;
						break;
					default:
						_nextKissingLadySequenceId = 0xEC;
						break;
					}
					if (_nextKissingLadySequenceId != 0xEC && _nextKissingLadySequenceId == _currKissingLadySequenceId)
						_nextKissingLadySequenceId = -1;
				}
			}
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(20) + 20;
				if (_vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0 && _nextSpringGuySequenceId == -1) {
					if (_vm->getRandom(5) != 0) {
						if (!_vm->isFlag(kGFSpringTaken))
							_nextSpringGuySequenceId = 0xF9;
					} else {
						if (_vm->isFlag(kGFSpringTaken))
							_nextSpringGuySequenceId = 0xF8;
						else
							_nextSpringGuySequenceId = 0xFA;
					}
				}
			}
		}
	
		_vm->checkGameKeys();
	
		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[4] = _vm->getRandom(20) + 20;
		}
		
		_vm->gameUpdateTick();	
	}
}

void Scene44::updateAnimations() {
	GameSys gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(0) == 2) 	{
		gameSys.setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case 0:
			_vm->_sceneDone = true;
			break;
		case 1:
			_nextKissingLadySequenceId = 0xEF;
			break;
		case 2:
			_nextKissingLadySequenceId = 0xF2;
			break;
		}
		_vm->_gnapActionStatus = -1;
	}
	
	if (gameSys.getAnimationStatus(1) == 2) {
		gameSys.setAnimation(0, 0, 1);
		switch (_vm->_platypusActionStatus) {
		case 4:
			if (gameSys.getAnimationStatus(2) == 2) {
				gameSys.insertSequence(0xFE, _vm->_platypusId, _vm->_platypusSequenceId | (_vm->_platypusSequenceDatNum << 16), _vm->_platypusId, kSeqSyncWait, 0, 0, 0);
				_vm->_platypusSequenceId = 0xFE;
				_vm->_platypusSequenceDatNum = 0;
				gameSys.setAnimation(0xFE, _vm->_platypusId, 1);
				gameSys.removeSequence(_currKissingLadySequenceId, 1, true);
				_vm->_platypusActionStatus = 5;
			}
			break;
		case 5:
			_vm->_sceneDone = true;
			_vm->_newSceneNum = 50;
			break;
		default:
			_vm->_platypusActionStatus = -1;
			break;
		}
	}
	
	if (gameSys.getAnimationStatus(2) == 2) {
		if (_nextKissingLadySequenceId == 0xF6) {
			gameSys.insertSequence(_nextKissingLadySequenceId, 1, _currKissingLadySequenceId, 1, kSeqSyncWait, 0, 0, 0);
			_vm->initPlatypusPos(5, 8, kDirNone);
			_currKissingLadySequenceId = _nextKissingLadySequenceId;
			_nextKissingLadySequenceId = -1;
			gameSys.setAnimation(0, 0, 2);
		} else if (_nextKissingLadySequenceId != -1) {
			gameSys.insertSequence(_nextKissingLadySequenceId, 1, _currKissingLadySequenceId, 1, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(_nextKissingLadySequenceId, 1, 2);
			_currKissingLadySequenceId = _nextKissingLadySequenceId;
			_nextKissingLadySequenceId = -1;
			_vm->_timers[4] = _vm->getRandom(20) + 20;
		}
	}
	
	if (gameSys.getAnimationStatus(4) == 2) {
		if (_currSpringGuySequenceId == 0xFB) {
			_vm->setGrabCursorSprite(kItemSpring);
			_nextSpringGuySequenceId = 0xF8;
		}
		if (_nextSpringGuySequenceId != -1) {
			gameSys.insertSequence(_nextSpringGuySequenceId, 1, _currSpringGuySequenceId, 1, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(_nextSpringGuySequenceId, 1, 4);
			_currSpringGuySequenceId = _nextSpringGuySequenceId;
			_nextSpringGuySequenceId = -1;
			_vm->_timers[5] = _vm->getRandom(20) + 20;
		}
	}
	
	if (gameSys.getAnimationStatus(3) == 2) {
		switch (_vm->_toyUfoActionStatus) {
		case 6:
			_vm->_sceneDone = true;
			break;
		default:
			_vm->_toyUfoNextSequenceId = _vm->toyUfoGetSequenceId();
			gameSys.insertSequence(_vm->_toyUfoNextSequenceId | 0x10000, _vm->_toyUfoId + 1,
				_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId,
				kSeqSyncWait, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
			_vm->_toyUfoSequenceId = _vm->_toyUfoNextSequenceId;
			++_vm->_toyUfoId;
			gameSys.setAnimation(_vm->_toyUfoNextSequenceId | 0x10000, _vm->_toyUfoId, 3);
			break;
		}
		_vm->_toyUfoActionStatus = -1;
	}
}

/*****************************************************************************/

enum {
	kHS45Platypus			= 0,
	kHS45ExitUfoParty		= 1,
	kHS45ExitShoe			= 2,
	kHS45ExitRight		= 3,
	kHS45ExitDiscoBall	= 4,
	kHS45DiscoBall		= 5,
	kHS45Device			= 6,
	kHS45WalkArea1		= 7
};

enum {
	kHS45UfoExitLeft	= 1,
	kHS45UfoExitRight	= 2,
	kHS45UfoDevice	= 3
};

Scene45::Scene45(GnapEngine *vm) : Scene(vm) {
	_currDancerSequenceId = -1;
}

int Scene45::init() {
	GameSys gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 1);
	gameSys.setAnimation(0, 0, 2);
	gameSys.setAnimation(0, 0, 3);
	gameSys.setAnimation(0, 0, 4);
	gameSys.setAnimation(0, 0, 5);
	return _vm->isFlag(kGFUnk23) ? 0xA2 : 0xA1;
}

void Scene45::updateHotspots() {
	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->setHotspot(kHS45Platypus, 0, 0, 0, 0, SF_DISABLED);
		_vm->setHotspot(kHS45UfoExitLeft, 0, 0, 10, 599, SF_EXIT_L_CURSOR);
		_vm->setHotspot(kHS45UfoExitRight, 794, 0, 799, 599, SF_EXIT_R_CURSOR | SF_DISABLED);
		_vm->setDeviceHotspot(kHS45UfoDevice, -1, 534, -1, 599);
		_vm->_hotspotsCount = 4;
	} else {
		_vm->setHotspot(kHS45Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHS45ExitUfoParty, 150, 580, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
		_vm->setHotspot(kHS45ExitShoe, 0, 100, 10, 599, SF_EXIT_L_CURSOR, 0, 8);
		_vm->setHotspot(kHS45ExitRight, 794, 100, 799, 599, SF_EXIT_R_CURSOR | SF_DISABLED, 10, 8);
		_vm->setHotspot(kHS45ExitDiscoBall, 200, 0, 600, 10, SF_DISABLED);
		_vm->setHotspot(kHS45DiscoBall, 370, 10, 470, 125, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 7);
		_vm->setHotspot(kHS45WalkArea1, 0, 0, 800, 472);
		_vm->setDeviceHotspot(kHS45Device, -1, -1, -1, -1);
		if (_vm->isFlag(kGFUnk22)) {
			_vm->_hotspots[kHS45Platypus]._flags = SF_DISABLED;
			_vm->_hotspots[kHS45ExitUfoParty]._flags = SF_DISABLED;
			_vm->_hotspots[kHS45ExitShoe]._flags = SF_DISABLED;
			_vm->_hotspots[kHS45ExitRight]._flags = SF_DISABLED;
			_vm->_hotspots[kHS45ExitDiscoBall]._flags = SF_EXIT_U_CURSOR;
		}
		if (_vm->isFlag(kGFUnk23) || _vm->isFlag(kGFUnk22))
			_vm->_hotspots[kHS45DiscoBall]._flags = SF_DISABLED;
		_vm->_hotspotsCount = 8;
	}
}

void Scene45::run() {
	GameSys gameSys = *_vm->_gameSys;

	if (!_vm->isSoundPlaying(0x1094A))
		_vm->playSound(0x1094A, true);
	
	_vm->queueInsertDeviceIcon();

	gameSys.insertSequence(0x96, 1, 0, 0, kSeqNone, 0, 0, 0);
	gameSys.setAnimation(0x96, 1, 3);
	gameSys.insertSequence(0x99, 1, 0, 0, kSeqNone, 0, 0, 0);
	gameSys.setAnimation(0x99, 1, 4);
	_currDancerSequenceId = 0x8F;
	gameSys.setAnimation(_currDancerSequenceId, 1, 2);
	gameSys.insertSequence(_currDancerSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->_toyUfoId = 0;
		_vm->_toyUfoActionStatus = -1;
		_vm->_toyUfoSequenceId = _vm->toyUfoGetSequenceId();
		_vm->_toyUfoNextSequenceId = _vm->_toyUfoSequenceId;
		if (_vm->_prevSceneNum == 46)
			_vm->_toyUfoX = 30;
		else
			_vm->_toyUfoX = 770;
		gameSys.setAnimation(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 5);
		gameSys.insertSequence(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 0, 0, kSeqNone, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
		_vm->endSceneInit();
	} else if (_vm->isFlag(kGFUnk22)) {
		_vm->_gnapSequenceId = 0x9E;
		_vm->_gnapSequenceDatNum = 0;
		_vm->_gnapId = 1;
		gameSys.setAnimation(0x9E, 1, 0);
		_vm->_gnapActionStatus = 1;
		gameSys.insertSequence(_vm->_gnapSequenceId, _vm->_gnapId, 0, 0, kSeqNone, 0, 0, 0);
		_vm->initPlatypusPos(4, 8, kDirNone);
		_vm->endSceneInit();
	} else if (_vm->_prevSceneNum == 46) {
		_vm->initGnapPos(-1, 8, kDirUpRight);
		_vm->initPlatypusPos(-1, 9, kDirUpLeft);
		_vm->endSceneInit();
		_vm->platypusWalkTo(4, 8, -1, 0x107C2, 1);
		_vm->gnapWalkTo(2, 7, -1, 0x107B9, 1);
	} else if (_vm->_prevSceneNum == 41) {
		_vm->initGnapPos(11, 8, kDirUpRight);
		_vm->initPlatypusPos(11, 9, kDirUpLeft);
		_vm->endSceneInit();
		_vm->platypusWalkTo(4, 8, -1, 0x107D2, 1);
		_vm->gnapWalkTo(10, 9, -1, 0x107BA, 1);
	} else {
		_vm->initGnapPos(2, 11, kDirUpRight);
		_vm->initPlatypusPos(6, 11, kDirUpLeft);
		_vm->endSceneInit();
		_vm->platypusWalkTo(4, 8, -1, 0x107C2, 1);
		_vm->gnapWalkTo(2, 7, -1, 0x107B9, 1);
	}
	
	if (!_vm->isFlag(kGFUnk21) && !_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->setFlag(kGFUnk21);
		_vm->setGrabCursorSprite(-1);
		gameSys.setAnimation(0x9D, _vm->_gnapId, 0);
		gameSys.insertSequence(0x9D, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
		while (gameSys.getAnimationStatus(0) != 2) {
			_vm->gameUpdateTick();
			if (gameSys.getAnimationStatus(2) == 2) {
				gameSys.setAnimation(0, 0, 2);
				int newSeqId = _vm->getRandom(7) + 0x8F;
				gameSys.insertSequence(newSeqId, 1, _currDancerSequenceId, 1, kSeqSyncWait, 0, 0, 0);
				gameSys.setAnimation(newSeqId, 1, 2);
				_currDancerSequenceId = newSeqId;
			}
			if (gameSys.getAnimationStatus(3) == 2 && gameSys.getAnimationStatus(4) == 2) {
				gameSys.insertSequence(0x96, 1, 0x96, 1, kSeqSyncWait, 0, 0, 0);
				gameSys.setAnimation(0x96, 1, 3);
				gameSys.insertSequence(0x99, 1, 0x99, 1, kSeqSyncWait, 0, 0, 0);
				gameSys.setAnimation(0x99, 1, 4);
			}
		}
		_vm->_gnapSequenceId = 0x9D;
		_vm->_gnapSequenceDatNum = 0;
		_vm->hideCursor();
		_vm->addFullScreenSprite(0x8A, 255);
		gameSys.setAnimation(0xA0, 256, 0);
		gameSys.insertSequence(0xA0, 256, 0, 0, kSeqNone, 0, 0, 0);
		while (gameSys.getAnimationStatus(0) != 2)
			_vm->gameUpdateTick();
		gameSys.setAnimation(0x107BD, _vm->_gnapId, 0);
		gameSys.insertSequence(0x107BD, _vm->_gnapId,
			makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId,
			kSeqSyncWait, 0, 75 * _vm->_gnapX - _vm->_gnapGridX, 48 * _vm->_gnapY - _vm->_gnapGridY);
		_vm->removeFullScreenSprite();
		_vm->showCursor();
		_vm->_gnapSequenceId = 0x7BD;
		_vm->_gnapSequenceDatNum = 1;
	}
	
	_vm->playPlatypusSequence(0x9A);
	gameSys.setAnimation(_vm->_platypusSequenceId, _vm->_platypusId, 1);
	
	while (!_vm->_sceneDone) {
		if (!_vm->isSoundPlaying(0x1094A))
			_vm->playSound(0x1094A, true);
	
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
	
		_vm->testWalk(0, 0, -1, -1, -1, -1);
	
		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);
	
		if (_vm->isFlag(kGFGnapControlsToyUFO)) {
			switch (_vm->_sceneClickedHotspot) {
			case kHS45UfoExitLeft:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = 2;
					_vm->_newSceneNum = 46;
					_vm->toyUfoFlyTo(-35, -1, -35, 799, 0, 300, 5);
				}
				break;

			case kHS45UfoExitRight:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = 2;
					_vm->_newSceneNum = 41;
					_vm->toyUfoFlyTo(835, -1, 0, 835, 0, 300, 5);
				}
				break;

			case kHS45UfoDevice:
				_vm->runMenu();
				updateHotspots();
				break;
			}
		} else {
			switch (_vm->_sceneClickedHotspot) {
			case kHS45Device:
				_vm->runMenu();
				updateHotspots();
				break;
			
			case kHS45Platypus:
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
							_vm->playPlatypusSequence(0x9A);
							gameSys.setAnimation(_vm->_platypusSequenceId, _vm->_platypusId, 1);
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

			case kHS45ExitUfoParty:
				if (_vm->_gnapActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->gnapWalkTo(_vm->_gnapX, _vm->_hotspotsWalkPos[kHS45ExitUfoParty].y, 0, 0x107AE, 1);
					_vm->_gnapActionStatus = 0;
					_vm->_newSceneNum = 40;
				}
				break;
			
			case kHS45ExitShoe:
				if (_vm->_gnapActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS45ExitShoe].x, _vm->_gnapY, 0, 0x107AF, 1);
					_vm->_gnapActionStatus = 0;
					_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS45ExitShoe].x, _vm->_platY, -1, 0x107CF, 1);
					_vm->_newSceneNum = 46;
				}
				break;
			
			case kHS45ExitRight:
				if (_vm->_gnapActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS45ExitRight].x, _vm->_gnapY, 0, 0x107AB, 1);
					_vm->_gnapActionStatus = 0;
					_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS45ExitRight].x, _vm->_platY, -1, 0x107CD, 1);
					_vm->_newSceneNum = 41;
				}
				break;

			case kHS45ExitDiscoBall:
				_vm->clearFlag(kGFUnk22);
				_vm->setFlag(kGFUnk23);
				_vm->_sceneDone = true;
				_vm->_newSceneNum = 54;
				break;

			case kHS45DiscoBall:
				if (_vm->_gnapActionStatus < 0) {
					if (_vm->_grabCursorSpriteIndex == kItemSpring) {
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS45DiscoBall].x, _vm->_hotspotsWalkPos[kHS45DiscoBall].y, 0, 0x9F, 5);
						_vm->_gnapActionStatus = 1;
						_vm->setGrabCursorSprite(-1);
						_vm->invRemove(kItemSpring);
					} else if (_vm->_grabCursorSpriteIndex >= 0) {
						_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, 5, 0);
					} else {
						switch (_vm->_verbCursor) {
						case LOOK_CURSOR:
							_vm->playGnapScratchingHead(5, 0);
							break;
						case GRAB_CURSOR:
						case TALK_CURSOR:
						case PLAT_CURSOR:
							_vm->playGnapImpossible(0, 0);
							break;
						}
					}
				}
				break;
			
			case kHS45WalkArea1:
				if (_vm->_gnapActionStatus < 0)
					_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				break;
			}
		}

		if (_vm->_mouseClickState._left && _vm->_gnapActionStatus < 0) {
			_vm->_mouseClickState._left = false;
			if (_vm->isFlag(kGFGnapControlsToyUFO)) {
				_vm->_toyUfoActionStatus = 3;
				_vm->toyUfoFlyTo(-1, -1, 0, 799, 0, 300, 5);
			} else {
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			}
		}
	
		updateAnimations();
		_vm->toyUfoCheckTimer();
	
		if (!_vm->_isLeavingScene && _vm->_gnapActionStatus < 0 && !_vm->isFlag(kGFGnapControlsToyUFO))
			_vm->updateGnapIdleSequence();
	
		_vm->checkGameKeys();
	
		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
		}
	
		_vm->gameUpdateTick();
	}

	_vm->_sceneWaiting = false;
}

void Scene45::updateAnimations() {
	GameSys gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case 0:
			_vm->_sceneDone = true;
			break;
		case 1:
			_vm->_sceneWaiting = true;
			_vm->setFlag(kGFUnk22);
			updateHotspots();
			gameSys.insertSequence(0x9E, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceId = 0x9E;
			_vm->_gnapSequenceDatNum = 0;
			gameSys.setAnimation(0x9E, _vm->_gnapId, 0);
			break;
		default:
			_vm->_gnapActionStatus = -1;
			break;
		}
	}
	
	if (gameSys.getAnimationStatus(1) == 2) {
		gameSys.setAnimation(0, 0, 1);
		if (_vm->getRandom(2) != 0)
			_vm->playPlatypusSequence(0x9B);
		else
			_vm->playPlatypusSequence(0x9C);
		gameSys.setAnimation(_vm->_platypusSequenceId, _vm->_platypusId, 1);
	}
	
	if (gameSys.getAnimationStatus(2) == 2) {
		gameSys.setAnimation(0, 0, 2);
		int newSeqId = _vm->getRandom(7) + 0x8F;
		gameSys.insertSequence(newSeqId, 1, _currDancerSequenceId, 1, kSeqSyncWait, 0, 0, 0);
		gameSys.setAnimation(newSeqId, 1, 2);
		_currDancerSequenceId = newSeqId;
	}
	
	if (gameSys.getAnimationStatus(3) == 2 && gameSys.getAnimationStatus(4) == 2) {
		gameSys.insertSequence(0x96, 1, 0x96, 1, kSeqSyncWait, 0, 0, 0);
		gameSys.setAnimation(0x96, 1, 3);
		gameSys.insertSequence(0x99, 1, 0x99, 1, kSeqSyncWait, 0, 0, 0);
		gameSys.setAnimation(0x99, 1, 4);
	}
	
	if (gameSys.getAnimationStatus(5) == 2) {
		switch (_vm->_toyUfoActionStatus) {
		case 2:
			_vm->_sceneDone = true;
			break;
		default:
			_vm->_toyUfoNextSequenceId = _vm->toyUfoGetSequenceId();
			gameSys.insertSequence(_vm->_toyUfoNextSequenceId | 0x10000, _vm->_toyUfoId + 1,
				_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId,
				kSeqSyncWait, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
			_vm->_toyUfoSequenceId = _vm->_toyUfoNextSequenceId;
			++_vm->_toyUfoId;
			gameSys.setAnimation(_vm->_toyUfoNextSequenceId | 0x10000, _vm->_toyUfoId, 5);
			break;
		}
		_vm->_toyUfoActionStatus = -1;
	}
}

/*****************************************************************************/

enum {
	kHS46Platypus			= 0,
	kHS46ExitUfoParty		= 1,
	kHS46ExitKissinBooth	= 2,
	kHS46ExitDisco		= 3,
	kHS46SackGuy			= 4,
	kHS46ItchyGuy			= 5,
	kHS46Device			= 6,
	kHS46WalkArea1		= 7
};

enum {
	kHS46UfoExitLeft	= 1,
	kHS46UfoExitRight	= 2,
	kHS46UfoDevice	= 3
};

Scene46::Scene46(GnapEngine *vm) : Scene(vm) {
	_currSackGuySequenceId = -1;
	_nextItchyGuySequenceId = -1;
	_nextSackGuySequenceId = -1;
	_currItchyGuySequenceId = -1;
}

int Scene46::init() {
	GameSys gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 1);
	gameSys.setAnimation(0, 0, 2);
	gameSys.setAnimation(0, 0, 3);
	gameSys.setAnimation(0, 0, 4);
	return 0x4E;
}

void Scene46::updateHotspots() {
	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->setHotspot(kHS46Platypus, 0, 0, 0, 0, SF_DISABLED);
		_vm->setHotspot(kHS46UfoExitLeft, 0, 0, 10, 599, SF_EXIT_L_CURSOR);
		_vm->setHotspot(kHS46UfoExitRight, 790, 0, 799, 599, SF_EXIT_R_CURSOR);
		_vm->setDeviceHotspot(kHS46UfoDevice, -1, 534, -1, 599);
		_vm->_hotspotsCount = 4;
	} else {
		_vm->setHotspot(kHS46Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHS46ExitUfoParty, 150, 580, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
		_vm->setHotspot(kHS46ExitKissinBooth, 0, 100, 10, 599, SF_EXIT_L_CURSOR, 0, 8);
		_vm->setHotspot(kHS46ExitDisco, 790, 100, 799, 599, SF_EXIT_R_CURSOR, 10, 8);
		_vm->setHotspot(kHS46SackGuy, 180, 370, 235, 490, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 8);
		_vm->setHotspot(kHS46ItchyGuy, 535, 210, 650, 480, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 8);
		_vm->setHotspot(kHS46WalkArea1, 0, 0, 800, 485);
		_vm->setDeviceHotspot(kHS46Device, -1, -1, -1, -1);
		_vm->_hotspotsCount = 8;
	}
}

void Scene46::run() {
	GameSys gameSys = *_vm->_gameSys;

	_vm->queueInsertDeviceIcon();
	gameSys.insertSequence(0x4D, 0, 0, 0, kSeqLoop, 0, 0, 0);
	
	_currSackGuySequenceId = 0x4B;
	_nextSackGuySequenceId = -1;
	gameSys.setAnimation(0x4B, 1, 3);
	gameSys.insertSequence(_currSackGuySequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	_currItchyGuySequenceId = 0x47;
	_nextItchyGuySequenceId = -1;
	gameSys.setAnimation(0x47, 1, 4);
	gameSys.insertSequence(_currItchyGuySequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->_toyUfoId = 0;
		_vm->_toyUfoActionStatus = -1;
		_vm->_toyUfoSequenceId = _vm->toyUfoGetSequenceId();
		_vm->_toyUfoNextSequenceId = _vm->_toyUfoSequenceId;
		if (_vm->_prevSceneNum == 44)
			_vm->_toyUfoX = 30;
		else
			_vm->_toyUfoX = 770;
		gameSys.setAnimation(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 2);
		gameSys.insertSequence(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 0, 0, kSeqNone, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
		_vm->endSceneInit();
	} else if (_vm->_prevSceneNum == 44) {
		_vm->initGnapPos(-1, 8, kDirUpRight);
		_vm->initPlatypusPos(-1, 8, kDirUpLeft);
		_vm->endSceneInit();
		_vm->platypusWalkTo(1, 8, -1, 0x107C2, 1);
		_vm->gnapWalkTo(2, 8, -1, 0x107B9, 1);
	} else if (_vm->_prevSceneNum == 45) {
		_vm->initGnapPos(11, 8, kDirUpRight);
		_vm->initPlatypusPos(12, 8, kDirUpLeft);
		_vm->endSceneInit();
		_vm->gnapWalkTo(8, 8, -1, 0x107BA, 1);
		_vm->platypusWalkTo(9, 8, -1, 0x107D2, 1);
	} else {
		_vm->initGnapPos(5, 11, kDirUpRight);
		_vm->initPlatypusPos(6, 11, kDirUpLeft);
		_vm->endSceneInit();
		_vm->platypusWalkTo(5, 8, -1, 0x107C2, 1);
		_vm->gnapWalkTo(6, 8, -1, 0x107BA, 1);
	}

	_vm->_timers[4] = _vm->getRandom(50) + 80;
	_vm->_timers[5] = _vm->getRandom(50) + 80;

	while (!_vm->_sceneDone) {
		if (!_vm->isSoundPlaying(0x1094B))
			_vm->playSound(0x1094B, true);
	
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
	
		_vm->testWalk(0, 0, -1, -1, -1, -1);
	
		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);
	
		if (_vm->isFlag(kGFGnapControlsToyUFO)) {
			switch (_vm->_sceneClickedHotspot) {
			case kHS46UfoExitLeft:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = 3;
					_vm->_newSceneNum = 44;
					_vm->toyUfoFlyTo(-35, -1, -35, 799, 0, 300, 2);
				}
				break;

			case kHS46UfoExitRight:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = 3;
					_vm->_newSceneNum = 45;
					_vm->toyUfoFlyTo(835, -1, 0, 835, 0, 300, 2);
				}
				break;

			case kHS46UfoDevice:
				_vm->runMenu();
				updateHotspots();
				break;
			}
		} else {
			switch (_vm->_sceneClickedHotspot) {
			case kHS46Device:
				_vm->runMenu();
				updateHotspots();
				break;

			case kHS46Platypus:
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

			case kHS46SackGuy:
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHS46SackGuy].x, _vm->_hotspotsWalkPos[kHS46SackGuy].y, 2, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapMoan1(_vm->_hotspotsWalkPos[kHS46SackGuy].x + 1, 0);
						break;
					case TALK_CURSOR:
						_vm->_gnapIdleFacing = kDirUpLeft;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS46SackGuy].x, _vm->_hotspotsWalkPos[kHS46SackGuy].y, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_vm->_gnapActionStatus = 2;
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
				break;

			case kHS46ItchyGuy:
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHS46ItchyGuy].x, _vm->_hotspotsWalkPos[kHS46ItchyGuy].y, 7, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapMoan1(_vm->_hotspotsWalkPos[kHS46ItchyGuy].x - 1, 0);
						break;
					case TALK_CURSOR:
						_vm->_gnapIdleFacing = kDirUpRight;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS46ItchyGuy].x, _vm->_hotspotsWalkPos[kHS46ItchyGuy].y, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_vm->_gnapActionStatus = 1;
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
				break;

			case kHS46ExitUfoParty:
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_gnapX, _vm->_hotspotsWalkPos[kHS46ExitUfoParty].y, 0, 0x107AE, 1);
				_vm->_gnapActionStatus = 0;
				_vm->_newSceneNum = 40;
				break;

			case kHS46ExitKissinBooth:
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS46ExitKissinBooth].x, _vm->_gnapY, 0, 0x107AF, 1);
				_vm->_gnapActionStatus = 0;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS46ExitKissinBooth].x, _vm->_platY, -1, 0x107CF, 1);
				_vm->_newSceneNum = 44;
				break;

			case kHS46ExitDisco:
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS46ExitDisco].x, _vm->_gnapY, 0, 0x107AB, 1);
				_vm->_gnapActionStatus = 0;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS46ExitDisco].x, _vm->_platY, -1, 0x107CD, 1);
				_vm->_newSceneNum = 45;
				break;

			case kHS46WalkArea1:
				if (_vm->_gnapActionStatus < 0)
					_vm->gnapWalkTo(-1, -1, -1, -1, 1);
					break;
			}
		}
	
		if (_vm->_mouseClickState._left && _vm->_gnapActionStatus < 0) {
			_vm->_mouseClickState._left = false;
			if (_vm->isFlag(kGFGnapControlsToyUFO)) {
				_vm->_toyUfoActionStatus = 4;
				_vm->toyUfoFlyTo(-1, -1, 0, 799, 0, 300, 2);
			} else {
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			}
		}
	
		updateAnimations();
		_vm->toyUfoCheckTimer();
	
		if (!_vm->_isLeavingScene) {
			if (_vm->_platypusActionStatus < 0 && !_vm->isFlag(kGFGnapControlsToyUFO))
				_vm->updatePlatypusIdleSequence();
			if (_vm->_gnapActionStatus < 0 && !_vm->isFlag(kGFGnapControlsToyUFO))
				_vm->updateGnapIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(50) + 80;
				if (_vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0 && _nextItchyGuySequenceId == -1) {
					if (_vm->getRandom(2) != 0)
						_nextItchyGuySequenceId = 0x49;
					else
						_nextItchyGuySequenceId = 0x48;
				}
			}
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(50) + 80;
				if (_vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0 && _nextSackGuySequenceId == -1)
					_nextSackGuySequenceId = 0x4C;
			}
		}
	
		_vm->checkGameKeys();
	
		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
		}
		
		_vm->gameUpdateTick();
	}
}

void Scene46::updateAnimations() {
	GameSys gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case 0:
			_vm->_sceneDone = true;
			break;
		case 1:
			_nextItchyGuySequenceId = 0x46;
			break;
		case 2:
			_nextSackGuySequenceId = 0x4A;
			break;
		}
		_vm->_gnapActionStatus = -1;
	}
	
	if (gameSys.getAnimationStatus(3) == 2 && _nextSackGuySequenceId != -1) {
		gameSys.insertSequence(_nextSackGuySequenceId, 1, _currSackGuySequenceId, 1, kSeqSyncWait, 0, 0, 0);
		gameSys.setAnimation(_nextSackGuySequenceId, 1, 3);
		_currSackGuySequenceId = _nextSackGuySequenceId;
		_nextSackGuySequenceId = -1;
		_vm->_timers[5] = _vm->getRandom(50) + 80;
	}
	
	if (gameSys.getAnimationStatus(4) == 2 && _nextItchyGuySequenceId != -1) {
		gameSys.insertSequence(_nextItchyGuySequenceId, 1, _currItchyGuySequenceId, 1, kSeqSyncWait, 0, 0, 0);
		gameSys.setAnimation(_nextItchyGuySequenceId, 1, 4);
		_currItchyGuySequenceId = _nextItchyGuySequenceId;
		_nextItchyGuySequenceId = -1;
		_vm->_timers[4] = _vm->getRandom(50) + 80;
	}
	
	if (gameSys.getAnimationStatus(2) == 2) {
		switch (_vm->_toyUfoActionStatus) {
		case 3:
			_vm->_sceneDone = true;
			break;
		default:
			_vm->_toyUfoNextSequenceId = _vm->toyUfoGetSequenceId();
			gameSys.insertSequence(_vm->_toyUfoNextSequenceId | 0x10000, _vm->_toyUfoId + 1,
				_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId,
				kSeqSyncWait, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
			_vm->_toyUfoSequenceId = _vm->_toyUfoNextSequenceId;
			++_vm->_toyUfoId;
			gameSys.setAnimation(_vm->_toyUfoNextSequenceId | 0x10000, _vm->_toyUfoId, 2);
			break;
		}
		_vm->_toyUfoActionStatus = -1;
	}
}

/*****************************************************************************/

static const ObstacleDef kObstacleDefs[] = {
	{0xB4, 15}, {0xCB, 14}, {0xCD, 13}, {0xCF, 15}, {0xBA, 14},
	{0xCD, 13}, {0xCF, 12}, {0xCB, 15}, {0xBD, 13}, {0xCF, 12},
	{0xCD, 11}, {0xCB, 15}, {0xB7, 12}, {0xCD, 11}, {0xCB, 10},
	{0xCF, 15}, {0xCF, 14}, {0xBD, 13}, {0xCF, 12}, {0xCD, 11},
	{0xCB, 15}, {0xCB, 13}, {0xB4, 12}, {0xCB, 11}, {0xCD, 10},
	{0xCF, 15}, {0xCD, 12}, {0xBA, 12}, {0xCD, 12}, {0xCF, 12},
	{0xCB, 15}, {0xCB,  9}, {0xCD,  9}, {0xCF,  9}, {0xCD,  9},
	{0xCB,  9}, {0xCD,  9}, {0xCF,  5}, {0xBD, 13}, {0xCF,  8},
	{0xCB,  8}, {0xCD, 15}, {0xB4,  1}, {0xBD,  7}, {0xCF,  7},
	{0xCD,  7}, {0xCB,  7}, {0xCD,  7}, {0xCF, 15}, {0xCF, 15}
};

Scene49::Scene49(GnapEngine *vm) : Scene(vm) {
	_scoreBarFlash = false;
	_scoreBarPos = -1;
	_scoreLevel = -1;
	_obstacleIndex = -1;
	_truckSequenceId = -1;
	_truckId = -1;
	_truckLaneNum = -1;

	for (int i = 0; i < 5; i++) {
		_obstacles[i]._currSequenceId = -1;
		_obstacles[i]._closerSequenceId = -1;
		_obstacles[i]._passedSequenceId = -1;
		_obstacles[i]._splashSequenceId = -1;
		_obstacles[i]._collisionSequenceId = -1;
		_obstacles[i]._prevId = -1;
		_obstacles[i]._currId = -1;
		_obstacles[i]._laneNum = -1;
	}
}

int Scene49::init() {
	GameSys gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 1);
	for (int i = 0; i < 5; ++i)
		gameSys.setAnimation(0, 0, i + 2);
	_vm->_timers[2] = 0;
	_vm->_timers[0] = 0;
	_vm->_timers[1] = 0;
	_vm->clearKeyStatus1(28);
	_vm->clearKeyStatus1(54);
	_vm->clearKeyStatus1(52);
	return 0xD5;
}

void Scene49::updateHotspots() {
	_vm->_hotspotsCount = 0;
}

void Scene49::checkObstacles() {
	if (_vm->_timers[2] == 0) {
		if (_vm->_timers[3] == 0) {
			for (int i = 0; i < 5; ++i)
				clearObstacle(i);
		}

		for (int j = 0; j < 5; ++j) {
			if (_obstacles[j]._currSequenceId == 0) {
				_vm->_timers[3] = 35;
				_obstacles[j]._currSequenceId = kObstacleDefs[_obstacleIndex]._sequenceId;
				switch (_obstacles[j]._currSequenceId) {
				case 0xB4:
					_obstacles[j]._laneNum = 1;
					_obstacles[j]._closerSequenceId = 180;
					_obstacles[j]._passedSequenceId = 181;
					_obstacles[j]._splashSequenceId = 182;
					_obstacles[j]._collisionSequenceId = 192;
					break;
				case 0xB7:
					_obstacles[j]._laneNum = 2;
					_obstacles[j]._closerSequenceId = 183;
					_obstacles[j]._passedSequenceId = 184;
					_obstacles[j]._splashSequenceId = 185;
					_obstacles[j]._collisionSequenceId = 193;
					break;
				case 0xBD:
					_obstacles[j]._laneNum = 3;
					_obstacles[j]._closerSequenceId = 189;
					_obstacles[j]._passedSequenceId = 190;
					_obstacles[j]._splashSequenceId = 191;
					_obstacles[j]._collisionSequenceId = 195;
					break;
				case 0xBA:
					_obstacles[j]._laneNum = 2;
					_obstacles[j]._closerSequenceId = 186;
					_obstacles[j]._passedSequenceId = 187;
					_obstacles[j]._splashSequenceId = 188;
					_obstacles[j]._collisionSequenceId = 194;
					break;
				case 0xCB:
					_obstacles[j]._laneNum = 1;
					_obstacles[j]._closerSequenceId = 203;
					_obstacles[j]._passedSequenceId = 204;
					_obstacles[j]._splashSequenceId = 0;
					_obstacles[j]._collisionSequenceId = 209;
					break;
				case 0xCD:
					_obstacles[j]._laneNum = 2;
					_obstacles[j]._closerSequenceId = 205;
					_obstacles[j]._passedSequenceId = 206;
					_obstacles[j]._splashSequenceId = 0;
					_obstacles[j]._collisionSequenceId = 210;
					break;
				case 0xCF:
					_obstacles[j]._laneNum = 3;
					_obstacles[j]._closerSequenceId = 207;
					_obstacles[j]._passedSequenceId = 208;
					_obstacles[j]._splashSequenceId = 0;
					_obstacles[j]._collisionSequenceId = 211;
					break;
				}
				_obstacles[j]._prevId = _truckId;
				_obstacles[j]._currId = _obstacles[j]._prevId;
				_vm->_gameSys->setAnimation(_obstacles[j]._currSequenceId, _obstacles[j]._currId, j + 2);
				_vm->_gameSys->insertSequence(_obstacles[j]._currSequenceId, _obstacles[j]._currId, 0, 0, kSeqNone, 0, 0, -50);
				_vm->_timers[2] = kObstacleDefs[_obstacleIndex]._ticks;
				++_obstacleIndex;
				if (_obstacleIndex == 50)
					_obstacleIndex = 0;
				break;
			}
		}
	}
}

void Scene49::updateObstacle(int id) {
	GameSys gameSys = *_vm->_gameSys;
	Scene49Obstacle &obstacle = _obstacles[id];

	obstacle._currId = obstacle._prevId;

	switch (obstacle._laneNum) {
	case 1:
		obstacle._prevId = _truckId + 1;
		break;
	case 2:
		if (_truckLaneNum != 2 && _truckLaneNum != 3)
			obstacle._prevId = _truckId - 1;
		else
			obstacle._prevId = _truckId + 1;
		break;
	case 3:
		if (_truckLaneNum != 1 && _truckLaneNum != 2)
			obstacle._prevId = _truckId;
		else
			obstacle._prevId = _truckId - 1;
		break;
	}

	if (obstacle._currSequenceId == obstacle._closerSequenceId) {
		if (_truckLaneNum == obstacle._laneNum) {
			if (obstacle._splashSequenceId) {
				gameSys.setAnimation(obstacle._collisionSequenceId, obstacle._prevId, id + 2);
				gameSys.insertSequence(obstacle._collisionSequenceId, obstacle._prevId,
					obstacle._currSequenceId, obstacle._currId,
					kSeqSyncWait, 0, 0, -50);
				obstacle._currSequenceId = obstacle._collisionSequenceId;
				_vm->playSound(224, false);
				increaseScore(30);
			} else if ((obstacle._laneNum == 1 && _truckSequenceId == 0xB0) ||
				(obstacle._laneNum == 2 && (_truckSequenceId == 0xB1 || _truckSequenceId == 0xB2)) ||
				(obstacle._laneNum == 3 && _truckSequenceId == 0xB3)) {
				gameSys.setAnimation(obstacle._passedSequenceId, obstacle._prevId, id + 2);
				gameSys.insertSequence(obstacle._passedSequenceId, obstacle._prevId,
					obstacle._currSequenceId, obstacle._currId,
					kSeqSyncWait, 0, 0, -50);
				obstacle._currSequenceId = obstacle._passedSequenceId;
			} else {
				gameSys.setAnimation(obstacle._collisionSequenceId, 256, 0);
				gameSys.setAnimation(obstacle._passedSequenceId, obstacle._prevId, id + 2);
				gameSys.insertSequence(obstacle._passedSequenceId, obstacle._prevId,
					obstacle._currSequenceId, obstacle._currId,
					kSeqSyncWait, 0, 0, -50);
				gameSys.insertSequence(obstacle._collisionSequenceId, 256,
					_truckSequenceId, _truckId,
					kSeqSyncExists, 0, 0, -50);
				_truckSequenceId = obstacle._collisionSequenceId;
				_truckId = 256;
				obstacle._currSequenceId = obstacle._passedSequenceId;
				_vm->playSound(225, false);
				decreaseScore(30);
			}
		} else {
			gameSys.setAnimation(obstacle._passedSequenceId, obstacle._prevId, id + 2);
			gameSys.insertSequence(obstacle._passedSequenceId, obstacle._prevId,
				obstacle._currSequenceId, obstacle._currId,
				kSeqSyncWait, 0, 0, -50);
			obstacle._currSequenceId = obstacle._passedSequenceId;
		}
	} else if (obstacle._currSequenceId == obstacle._passedSequenceId) {
		if (_truckLaneNum == obstacle._laneNum) {
			if (obstacle._splashSequenceId) {
				gameSys.setAnimation(obstacle._collisionSequenceId, obstacle._prevId, id + 2);
				gameSys.insertSequence(obstacle._collisionSequenceId, obstacle._prevId,
					obstacle._currSequenceId, obstacle._currId,
					kSeqSyncWait, 0, 0, -50);
				obstacle._currSequenceId = obstacle._collisionSequenceId;
				_vm->playSound(224, false);
				increaseScore(30);
			}
		} else if (obstacle._splashSequenceId) {
			gameSys.setAnimation(obstacle._splashSequenceId, obstacle._prevId, id + 2);
			gameSys.insertSequence(obstacle._splashSequenceId, obstacle._prevId,
				obstacle._currSequenceId, obstacle._currId,
				kSeqSyncWait, 0, 0, -50);
			obstacle._currSequenceId = obstacle._splashSequenceId;
		}
	} else {
		gameSys.setAnimation(0, 0, id + 2);
		clearObstacle(id);
	}
}

void Scene49::increaseScore(int amount) {
	if (_scoreBarPos + amount <= 556) {
		_scoreBarPos += amount;
		_vm->_gameSys->fillSurface(0, _scoreBarPos, 508, amount, 22, 255, 0, 0);
	}
	_scoreLevel = _scoreBarPos + amount >= 556;
}

void Scene49::decreaseScore(int amount) {
	if (_scoreBarPos >= 226 && _scoreLevel == 0) {
		if (_scoreBarFlash)
			refreshScoreBar();
		_vm->_gameSys->fillSurface(0, _scoreBarPos, 508, amount, 22, 89, 0, 5);
		_scoreBarPos -= amount;
		_scoreLevel = 0;
	}
}

void Scene49::refreshScoreBar() {
	if (_scoreBarFlash)
		_vm->_gameSys->fillSurface(0, 226, 508, 330, 22, 255, 0, 0);
	else
		_vm->_gameSys->fillSurface(0, 226, 508, 330, 22, 89, 0, 5);
	_scoreBarFlash = !_scoreBarFlash;
}

void Scene49::clearObstacle(int index) {
	_obstacles[index]._currSequenceId = 0;
	_obstacles[index]._closerSequenceId = 0;
	_obstacles[index]._passedSequenceId = 0;
	_obstacles[index]._splashSequenceId = 0;
	_obstacles[index]._collisionSequenceId = 0;
	_obstacles[index]._prevId = 0;
	_obstacles[index]._currId = 0;
	_obstacles[index]._laneNum = 0;
}

void Scene49::run() {
	GameSys gameSys = *_vm->_gameSys;

	bool animToggle6 = false;
	bool animToggle5 = false;
	bool animToggle4 = false;
	bool animToggle3 = false;
	bool streetAnimToggle = false;
	bool bgAnimToggle = false;

	_vm->playSound(0xE2, true);
	_vm->setSoundVolume(0xE2, 75);

	_vm->hideCursor();
	_vm->setGrabCursorSprite(-1);

	_scoreBarPos = 196;
	_scoreLevel = 0;
	_scoreBarFlash = false;

	switch (_vm->getRandom(3)) {
	case 0:
		_truckSequenceId = 0xAD;
		_truckLaneNum = 1;
		break;
	case 1:
		_truckSequenceId = 0xAE;
		_truckLaneNum = 2;
		break;
	case 2:
		_truckSequenceId = 0xAF;
		_truckLaneNum = 3;
		break;
	}

	int bgWidth1 = gameSys.getSpriteWidthById(0x5E);
	int bgX1 = 600;

	int bgWidth2 = gameSys.getSpriteWidthById(0x5F);
	int bgX2 = 400;

	int bgWidth3 = gameSys.getSpriteWidthById(4);
	int bgX3 = 700;

	int bgWidth4 = gameSys.getSpriteWidthById(5);
	int bgX4 = 500;

	int bgWidth5 = gameSys.getSpriteWidthById(6);
	int bgX5 = 300;

	int bgWidth6 = gameSys.getSpriteWidthById(7);
	int bgX6 = 100;

	gameSys.setAnimation(0xC8, 251, 1);
	gameSys.setAnimation(_truckSequenceId, 256, 0);
	gameSys.insertSequence(0xC9, 256, 0, 0, kSeqNone, 0, 600, 85);
	gameSys.insertSequence(0xCA, 257, 0, 0, kSeqNone, 0, 400, 100);
	gameSys.insertSequence(0xC4, 256, 0, 0, kSeqNone, 0, 700, 140);
	gameSys.insertSequence(0xC5, 257, 0, 0, kSeqNone, 0, 500, 160);
	gameSys.insertSequence(0xC6, 258, 0, 0, kSeqNone, 0, 300, 140);
	gameSys.insertSequence(0xC7, 259, 0, 0, kSeqNone, 0, 100, 140);
	gameSys.insertSequence(0xC8, 251, 0, 0, kSeqNone, 0, 0, -50);
	gameSys.insertSequence(_truckSequenceId, 256, 0, 0, kSeqNone, 0, 0, -50);

	_vm->_timers[0] = 2;

	for (int i = 0; i < 5; ++i)
		clearObstacle(i);

	_obstacleIndex = 0;

	_vm->_timers[2] = _vm->getRandom(20) + 10;

	_truckId = 256;
	_vm->_timers[3] = 35;
	
	while (!_vm->_sceneDone) {
		if (_vm->_timers[0] == 0) {
			// Update background animations (clouds etc.)
			--bgX1;
			bgX2 -= 2;
			bgX3 -= 5;
			--bgX4;
			--bgX5;
			--bgX6;
			if (bgX1 <= -bgWidth1)
				bgX1 = 799;
			if (bgX2 <= -bgWidth2)
				bgX2 = 799;
			if (bgX3 <= -bgWidth3)
				bgX3 = 799;
			if (bgX4 <= -bgWidth4)
				bgX4 = 799;
			if (bgX5 <= -bgWidth5)
				bgX5 = 799;
			if (bgX6 <= -bgWidth6)
				bgX6 = 799;
			bgAnimToggle = !bgAnimToggle;
			gameSys.insertSequence(0xC9, (bgAnimToggle ? 1 : 0) + 256, 0xC9, (bgAnimToggle ? 0 : 1) + 256, kSeqSyncWait, 0, bgX1, 85);
			gameSys.insertSequence(0xCA, (bgAnimToggle ? 1 : 0) + 257, 0xCA, (bgAnimToggle ? 0 : 1) + 257, kSeqSyncWait, 0, bgX2, 100);
			gameSys.insertSequence(0xC4, (bgAnimToggle ? 1 : 0) + 256, 0xC4, (bgAnimToggle ? 0 : 1) + 256, kSeqSyncWait, 0, bgX3, 140);
			gameSys.insertSequence(0xC5, (bgAnimToggle ? 1 : 0) + 257, 0xC5, (bgAnimToggle ? 0 : 1) + 257, kSeqSyncWait, 0, bgX4, 160);
			gameSys.insertSequence(0xC6, (bgAnimToggle ? 1 : 0) + 258, 0xC6, (bgAnimToggle ? 0 : 1) + 258, kSeqSyncWait, 0, bgX5, 140);
			gameSys.insertSequence(0xC7, (bgAnimToggle ? 1 : 0) + 259, 0xC7, (bgAnimToggle ? 0 : 1) + 259, kSeqSyncWait, 0, bgX6, 140);
			_vm->_timers[0] = 2;
		}

		if (gameSys.getAnimationStatus(1) == 2) {
			streetAnimToggle = !streetAnimToggle;
			gameSys.setAnimation(0xC8, (streetAnimToggle ? 1 : 0) + 251, 1);
			gameSys.insertSequence(0xC8, (streetAnimToggle ? 1 : 0) + 251, 200, (streetAnimToggle ? 0 : 1) + 251, kSeqSyncWait, 0, 0, -50);
		}

		checkObstacles();

		if (gameSys.getAnimationStatus(0) == 2) {
			switch (_truckSequenceId) {
			case 0xB1:
				_truckLaneNum = 1;
				break;
			case 0xB0:
			case 0xB3:
				_truckLaneNum = 2;
				break;
			case 0xB2:
				_truckLaneNum = 3;
				break;
			}
			animToggle3 = !animToggle3;
			if (_truckLaneNum == 1) {
				gameSys.setAnimation(0xAD, (animToggle3 ? 1 : 0) + 256, 0);
				gameSys.insertSequence(0xAD, (animToggle3 ? 1 : 0) + 256, _truckSequenceId, _truckId, kSeqSyncWait, 0, 0, -50);
				_truckSequenceId = 0xAD;
			} else if (_truckLaneNum == 2) {
				gameSys.setAnimation(0xAE, (animToggle3 ? 1 : 0) + 256, 0);
				gameSys.insertSequence(0xAE, (animToggle3 ? 1 : 0) + 256, _truckSequenceId, _truckId, kSeqSyncWait, 0, 0, -50);
				_truckSequenceId = 0xAE;
			} else {
				gameSys.setAnimation(0xAF, (animToggle3 ? 1 : 0) + 256, 0);
				gameSys.insertSequence(0xAF, (animToggle3 ? 1 : 0) + 256, _truckSequenceId, _truckId, kSeqSyncWait, 0, 0, -50);
				_truckSequenceId = 0xAF;
			}
			_truckId = (animToggle3 ? 1 : 0) + 256;
			if (_scoreLevel == 1) {
				if (!gameSys.isSequenceActive(0xD4, 266)) {
					gameSys.setAnimation(0xD4, 266, 8);
					gameSys.insertSequence(0xD4, 266, 0, 0, kSeqNone, 0, 0, -50);
				}
				++_scoreLevel;
				_vm->_timers[1] = 2;
				animToggle4 = false;
				animToggle5 = false;
				animToggle6 = false;
				_scoreBarFlash = false;
			}
		}

		if (_scoreLevel != 0 && !_vm->_timers[1]) {
			refreshScoreBar();
			_vm->_timers[1] = 8;
			if (animToggle6) {
				if (animToggle5) {
					if (animToggle4 && !gameSys.isSequenceActive(212, 266))
						gameSys.insertSequence(212, 266, 0, 0, kSeqNone, 0, 0, -50);
					animToggle4 = !animToggle4;
				}
				animToggle5 = !animToggle5;
			}
			animToggle6 = !animToggle6;
		}

		updateAnimations();

		if (clearKeyStatus()) {
			_vm->_sceneDone = true;
			_vm->_newSceneNum = 2;
			_vm->_newCursorValue = 1;
		}

		if (_vm->isKeyStatus1(Common::KEYCODE_RIGHT)) {
			// Steer right
			if (_truckSequenceId == 0xB3)
				_truckLaneNum = 2;
			if (_truckSequenceId == 0xB1)
				_truckLaneNum = 1;
			if (_truckLaneNum != 3 && _truckLaneNum != 2) {
				if (_scoreLevel) {
					_vm->_sceneDone = true;
					_vm->_newSceneNum = 47;
				}
			} else {
				int steerSequenceId = (_truckLaneNum == 3) ? 0xB3 : 0xB1;
				if (_truckSequenceId == 0xAE || _truckSequenceId == 0xAF) {
					gameSys.setAnimation(steerSequenceId, 256, 0);
					gameSys.insertSequence(steerSequenceId, 256, _truckSequenceId, _truckId, kSeqSyncExists, 0, 0, -50);
					_truckSequenceId = steerSequenceId;
					_truckId = 256;
				}
			}
			_vm->clearKeyStatus1(Common::KEYCODE_RIGHT);
		}

		if (_vm->isKeyStatus1(Common::KEYCODE_LEFT)) {
			// Steer left
			if (_truckSequenceId == 0xB0)
				_truckLaneNum = 2;
			if (_truckSequenceId == 0xB2)
				_truckLaneNum = 3;
			if (_truckLaneNum == 1 || _truckLaneNum == 2) {
				int steerSequenceId = (_truckLaneNum == 1) ? 0xB0 : 0xB2;
				if (_truckSequenceId == 0xAD || _truckSequenceId == 0xAE) {
					gameSys.setAnimation(steerSequenceId, 256, 0);
					gameSys.insertSequence(steerSequenceId, 256, _truckSequenceId, _truckId, kSeqSyncExists, 0, 0, -50);
					_truckSequenceId = steerSequenceId;
					_truckId = 256;
				}
			}
			_vm->clearKeyStatus1(Common::KEYCODE_LEFT);
		}
		_vm->gameUpdateTick();
	}
	_vm->stopSound(0xE2);  
}

void Scene49::updateAnimations() {
	GameSys gameSys = *_vm->_gameSys;

	for (int i = 0; i < 5; ++i) {
		if (gameSys.getAnimationStatus(i + 2) == 2) {
			if (_obstacles[i]._currSequenceId)
				updateObstacle(i);
		}
	}

	if (gameSys.getAnimationStatus(8) == 2) {
		_vm->_sceneDone = true;
		_vm->_newSceneNum = 47;
	}
}

} // End of namespace Gnap
