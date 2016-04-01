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

int GnapEngine::scene22_init() {
	return 0x5E;
}

void GnapEngine::scene22_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSExitOutsideGrubCity, 0, 180, 184, 472, SF_EXIT_L_CURSOR, 3, 6);
	setHotspot(kHSExitBackGrubCity, 785, 405, 800, 585, SF_EXIT_R_CURSOR, 11, 9);
	setHotspot(kHSCashier, 578, 230, 660, 376, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 8);
	setHotspot(kHSWalkArea1, 553, 0, 800, 542);
	setHotspot(kHSWalkArea2, 0, 0, 552, 488);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_hotspotsCount = 7;
}

void GnapEngine::scene22_run() {

	_gameSys->insertSequence(0x5D, 254, 0, 0, kSeqNone, 0, 0, 0);
	
	_s22_currCashierSequenceId = 0x59;
	_s22_nextCashierSequenceId = -1;
	
	_gameSys->setAnimation(0x59, 1, 3);
	_gameSys->insertSequence(_s22_currCashierSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	_timers[6] = getRandom(30) + 20;
	
	queueInsertDeviceIcon();
	
	if (_prevSceneNum == 20) {
		initGnapPos(2, 8, kDirBottomRight);
		initBeaverPos(1, 8, kDirNone);
		endSceneInit();
	} else {
		initGnapPos(11, _hotspotsWalkPos[kHSExitBackGrubCity].y, kDirBottomRight);
		initBeaverPos(11, _hotspotsWalkPos[kHSExitBackGrubCity].y + 1, kDirNone);
		endSceneInit();
		gnapWalkTo(8, 8, -1, 0x107B9, 1);
		platypusWalkTo(9, 8, -1, 0x107C2, 1);
	}
	
	if (isFlag(11)) {
		int storeDetectiveSeqId;
		setGrabCursorSprite(-1);
		invRemove(kItemCereals);
		if (_s22_caughtBefore) {
			switch (getRandom(3)) {
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
		_gameSys->waitForUpdate();
		_gameSys->requestClear1();
		_gameSys->drawSpriteToBackground(0, 0, 0x44);
		_gameSys->setAnimation(storeDetectiveSeqId, 256, 4);
		_gameSys->insertSequence(storeDetectiveSeqId, 256, 0, 0, kSeqNone, 0, 0, 0);
		while (_gameSys->getAnimationStatus(4) != 2) {
			gameUpdateTick();
		}
		_sceneDone = true;
		_newSceneNum = 20;
		_s22_caughtBefore = true;
	}
	
	while (!_sceneDone) {
		
		updateMouseCursor();
		updateCursorByHotspot();
	
		testWalk(0, 0, -1, -1, -1, -1);
	
		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);
	
		switch (_sceneClickedHotspot) {

		case kHSDevice:
			if (_gnapActionStatus < 0) {
				runMenu();
				scene22_updateHotspots();
			}
			break;

		case kHSPlatypus:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex == kItemJoint) {
					gnapUseJointOnPlatypus();
				} else if (_grabCursorSpriteIndex >= 0) {
					playGnapImpossible(0, 0);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(_platX, _platY);
						break;
					case GRAB_CURSOR:
						gnapKissPlatypus(0);
						break;
					case TALK_CURSOR:
						playGnapBrainPulsating(_platX, _platY);
						playBeaverSequence(getBeaverSequenceId(0, 0, 0) | 0x10000);
						break;
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSExitOutsideGrubCity:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = 1;
				_newSceneNum = 20;
				gnapWalkTo(_hotspotsWalkPos[kHSExitOutsideGrubCity].x, _hotspotsWalkPos[kHSExitOutsideGrubCity].y, 0, 0x107AF, 1);
				_gnapActionStatus = kASLeaveScene;
				platypusWalkTo(_hotspotsWalkPos[kHSExitOutsideGrubCity].x, _hotspotsWalkPos[kHSExitOutsideGrubCity].y + 1, -1, 0x107C2, 1);
			}
			break;

		case kHSExitBackGrubCity:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = 1;
				_newSceneNum = 23;
				gnapWalkTo(_hotspotsWalkPos[kHSExitBackGrubCity].x, _hotspotsWalkPos[kHSExitBackGrubCity].y, 0, 0x107AB, 1);
				_gnapActionStatus = kASLeaveScene;
				platypusWalkTo(_hotspotsWalkPos[kHSExitBackGrubCity].x, _hotspotsWalkPos[kHSExitBackGrubCity].y + 1, -1, 0x107C2, 1);
			}
			break;
		
		case kHSCashier:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[kHSCashier].x, _hotspotsWalkPos[kHSCashier].y, 8, 4);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(8, 4);
						break;
					case GRAB_CURSOR:
						playGnapImpossible(0, 0);
						break;
					case TALK_CURSOR:
						_gnapIdleFacing = kDirUpRight;
						gnapWalkTo(_hotspotsWalkPos[kHSCashier].x, _hotspotsWalkPos[kHSCashier].y,
							0, getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_gnapActionStatus = kASTalkCashier;
						break;
					case PLAT_CURSOR:
						gnapUseDeviceOnBeaver();
						break;
					}
				}
			}
			break;
		
		case kHSWalkArea1:
		case kHSWalkArea2:
			gnapWalkTo(-1, -1, -1, -1, 1);
			break;
		
		default:
			if (_mouseClickState._left) {
				gnapWalkTo(-1, -1, -1, -1, 1);
				_mouseClickState._left = false;
			}
			break;
		
		}
	
		scene22_updateAnimations();
	
		if (!_isLeavingScene) {
			updateBeaverIdleSequence();
			updateGnapIdleSequence();
			if (!_timers[6] && _s22_nextCashierSequenceId == -1) {
				_timers[6] = getRandom(30) + 20;
				if (getRandom(8) != 0) {
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
	
		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene22_updateHotspots();
			_timers[2] = getRandom(30) + 20;
			_timers[3] = 400;
			_timers[1] = getRandom(20) + 30;
			_timers[0] = getRandom(75) + 75;
		}
		
		gameUpdateTick();
	
	}

}

void GnapEngine::scene22_updateAnimations() {

	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		switch (_gnapActionStatus) {
		case kASLeaveScene:
			_sceneDone = true;
			break;
		case kASTalkCashier:
			_s22_nextCashierSequenceId = 0x5C;
			break;
		}
		_gnapActionStatus = -1;
	}
	
	if (_gameSys->getAnimationStatus(3) == 2 && _s22_nextCashierSequenceId != -1) {
		_gameSys->setAnimation(_s22_nextCashierSequenceId, 1, 3);
		_gameSys->insertSequence(_s22_nextCashierSequenceId, 1, _s22_currCashierSequenceId, 1, kSeqSyncWait, 0, 0, 0);
		_s22_currCashierSequenceId = _s22_nextCashierSequenceId;
		_s22_nextCashierSequenceId = -1;
	}

}

} // End of namespace Gnap
