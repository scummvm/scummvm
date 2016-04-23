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
#include "gnap/scenes/scene22.h"

namespace Gnap {

enum {
	kHSPlatypus				= 0,
	kHSExitOutsideGrubCity	= 1,
	kHSExitBackGrubCity		= 2,
	kHSCashier				= 3,
	kHSDevice				= 4,
	kHSWalkArea1			= 5,
	kHSWalkArea2			= 6
};

enum {
	kASLeaveScene				= 0,
	kASTalkCashier				= 1
};

Scene22::Scene22(GnapEngine *vm) : Scene(vm) {
	_s22_caughtBefore = false;
	_s22_cashierCtr = 3;
}

int Scene22::init() {
	return 0x5E;
}

void Scene22::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSExitOutsideGrubCity, 0, 180, 184, 472, SF_EXIT_L_CURSOR, 3, 6);
	_vm->setHotspot(kHSExitBackGrubCity, 785, 405, 800, 585, SF_EXIT_R_CURSOR, 11, 9);
	_vm->setHotspot(kHSCashier, 578, 230, 660, 376, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 8);
	_vm->setHotspot(kHSWalkArea1, 553, 0, 800, 542);
	_vm->setHotspot(kHSWalkArea2, 0, 0, 552, 488);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_vm->_hotspotsCount = 7;
}

void Scene22::run() {
	_vm->_gameSys->insertSequence(0x5D, 254, 0, 0, kSeqNone, 0, 0, 0);
	
	_s22_currCashierSequenceId = 0x59;
	_s22_nextCashierSequenceId = -1;
	
	_vm->_gameSys->setAnimation(0x59, 1, 3);
	_vm->_gameSys->insertSequence(_s22_currCashierSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	_vm->_timers[6] = _vm->getRandom(30) + 20;
	
	_vm->queueInsertDeviceIcon();
	
	if (_vm->_prevSceneNum == 20) {
		_vm->initGnapPos(2, 8, kDirBottomRight);
		_vm->initPlatypusPos(1, 8, kDirNone);
		_vm->endSceneInit();
	} else {
		_vm->initGnapPos(11, _vm->_hotspotsWalkPos[kHSExitBackGrubCity].y, kDirBottomRight);
		_vm->initPlatypusPos(11, _vm->_hotspotsWalkPos[kHSExitBackGrubCity].y + 1, kDirNone);
		_vm->endSceneInit();
		_vm->gnapWalkTo(8, 8, -1, 0x107B9, 1);
		_vm->platypusWalkTo(9, 8, -1, 0x107C2, 1);
	}
	
	if (_vm->isFlag(kGFSceneFlag1)) {
		int storeDetectiveSeqId;
		_vm->setGrabCursorSprite(-1);
		_vm->invRemove(kItemCereals);
		if (_s22_caughtBefore) {
			switch (_vm->getRandom(3)) {
			case 0:
				storeDetectiveSeqId = 0x55;
				break;
			case 1:
				storeDetectiveSeqId = 0x56;
				break;
			default:
				storeDetectiveSeqId = 0x57;
				break;
			}
		} else {
			_s22_caughtBefore = true;
			storeDetectiveSeqId = 0x54;
		}
		_vm->_gameSys->waitForUpdate();
		_vm->_gameSys->requestClear1();
		_vm->_gameSys->drawSpriteToBackground(0, 0, 0x44);
		_vm->_gameSys->setAnimation(storeDetectiveSeqId, 256, 4);
		_vm->_gameSys->insertSequence(storeDetectiveSeqId, 256, 0, 0, kSeqNone, 0, 0, 0);
		while (_vm->_gameSys->getAnimationStatus(4) != 2) {
			_vm->gameUpdateTick();
		}
		_vm->_sceneDone = true;
		_vm->_newSceneNum = 20;
		_s22_caughtBefore = true;
	}
	
	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
	
		_vm->testWalk(0, 0, -1, -1, -1, -1);
	
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
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemJoint) {
					_vm->gnapUseJointOnPlatypus();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapImpossible(0, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(_vm->_platX, _vm->_platY);
						break;
					case GRAB_CURSOR:
						_vm->gnapKissPlatypus(0);
						break;
					case TALK_CURSOR:
						_vm->playGnapBrainPulsating(_vm->_platX, _vm->_platY);
						_vm->playPlatypusSequence(_vm->getPlatypusSequenceId());
						break;
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSExitOutsideGrubCity:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 20;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitOutsideGrubCity].x, _vm->_hotspotsWalkPos[kHSExitOutsideGrubCity].y, 0, 0x107AF, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitOutsideGrubCity].x, _vm->_hotspotsWalkPos[kHSExitOutsideGrubCity].y + 1, -1, 0x107C2, 1);
			}
			break;

		case kHSExitBackGrubCity:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 23;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitBackGrubCity].x, _vm->_hotspotsWalkPos[kHSExitBackGrubCity].y, 0, 0x107AB, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitBackGrubCity].x, _vm->_hotspotsWalkPos[kHSExitBackGrubCity].y + 1, -1, 0x107C2, 1);
			}
			break;
		
		case kHSCashier:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHSCashier].x, _vm->_hotspotsWalkPos[kHSCashier].y, 8, 4);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(8, 4);
						break;
					case GRAB_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					case TALK_CURSOR:
						_vm->_gnapIdleFacing = kDirUpRight;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSCashier].x, _vm->_hotspotsWalkPos[kHSCashier].y,
							0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_vm->_gnapActionStatus = kASTalkCashier;
						break;
					case PLAT_CURSOR:
						_vm->gnapUseDeviceOnPlatypuss();
						break;
					}
				}
			}
			break;
		
		case kHSWalkArea1:
		case kHSWalkArea2:
			_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			break;
		
		default:
			if (_vm->_mouseClickState._left) {
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				_vm->_mouseClickState._left = false;
			}
			break;
		}
	
		updateAnimations();
	
		if (!_vm->_isLeavingScene) {
			_vm->updatePlatypusIdleSequence();
			_vm->updateGnapIdleSequence();
			if (!_vm->_timers[6] && _s22_nextCashierSequenceId == -1) {
				_vm->_timers[6] = _vm->getRandom(30) + 20;
				if (_vm->getRandom(8) != 0) {
					_s22_nextCashierSequenceId = 0x59;
				} else {
					_s22_cashierCtr = (_s22_cashierCtr + 1) % 3;
					switch (_s22_cashierCtr) {
					case 1:
						_s22_nextCashierSequenceId = 0x58;
						break;
					case 2:
						_s22_nextCashierSequenceId = 0x5A;
						break;
					case 3:
						_s22_nextCashierSequenceId = 0x5B;
						break;
					default:
						_s22_nextCashierSequenceId = 0x58;
						break;
					}
				}
			}
		}
	
		_vm->checkGameKeys();
	
		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[2] = _vm->getRandom(30) + 20;
			_vm->_timers[3] = 400;
			_vm->_timers[1] = _vm->getRandom(20) + 30;
			_vm->_timers[0] = _vm->getRandom(75) + 75;
		}
		
		_vm->gameUpdateTick();
	}
}

void Scene22::updateAnimations() {
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kASLeaveScene:
			_vm->_sceneDone = true;
			break;
		case kASTalkCashier:
			_s22_nextCashierSequenceId = 0x5C;
			break;
		}
		_vm->_gnapActionStatus = -1;
	}
	
	if (_vm->_gameSys->getAnimationStatus(3) == 2 && _s22_nextCashierSequenceId != -1) {
		_vm->_gameSys->setAnimation(_s22_nextCashierSequenceId, 1, 3);
		_vm->_gameSys->insertSequence(_s22_nextCashierSequenceId, 1, _s22_currCashierSequenceId, 1, kSeqSyncWait, 0, 0, 0);
		_s22_currCashierSequenceId = _s22_nextCashierSequenceId;
		_s22_nextCashierSequenceId = -1;
	}
}

} // End of namespace Gnap
