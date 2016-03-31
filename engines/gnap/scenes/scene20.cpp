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
	kHSPlatypus					= 0,
	kHSGroceryStoreHat			= 1,
	kHSExitParkingLot			= 2,
	kHSStonerGuy				= 3,
	kHSGroceryStoreGuy			= 4,
	kHSDevice					= 5,
	kHSExitInsideGrubCity		= 6,
	kHSExitOutsideCircusWorld	= 7,
	kHSExitOutsideToyStore		= 8,
	kHSExitPhone				= 9,
	kHSWalkArea1				= 10,
	kHSWalkArea2				= 11
};

enum {
	kASLeaveScene					= 0,
	kASTalkStonerGuyNoJoint			= 2,
	kASTalkStonerGuyHasJoint		= 3,
	kASGrabJoint					= 4,
	kASActionDone					= 5,
	kASTalkGroceryStoreGuy			= 6,
	kASGrabGroceryStoreGuy			= 9,
	kASGrabGroceryStoreHat			= 10,
	kASSwitchGroceryStoreHat		= 11,
	kASSwitchGroceryStoreHatDone	= 12,
	kASGrabJointDone				= 13
};

int GnapEngine::scene20_init() {
	return 0x186;
}

void GnapEngine::scene20_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSGroceryStoreHat, 114, 441, 174, 486, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 0, 7);
	setHotspot(kHSExitParkingLot, 0, 300, 15, 600, SF_EXIT_L_CURSOR | SF_WALKABLE, 0, 7);
	setHotspot(kHSStonerGuy, 276, 290, 386, 450, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 8);
	setHotspot(kHSGroceryStoreGuy, 123, 282, 258, 462, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 7);
	setHotspot(kHSExitInsideGrubCity, 519, 250, 581, 413, SF_EXIT_L_CURSOR, 8, 7);
	setHotspot(kHSExitOutsideCircusWorld, 660, 222, 798, 442, SF_EXIT_NE_CURSOR, 9, 6);
	setHotspot(kHSExitOutsideToyStore, 785, 350, 800, 600, SF_EXIT_R_CURSOR, 11, 8);
	setHotspot(kHSExitPhone, 250, 585, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 10);
	setHotspot(kHSWalkArea1, 0, 0, 800, 468);
	setHotspot(kHSWalkArea2, 605, 0, 800, 600);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_hotspotsCount = 12;
}

void GnapEngine::scene20_updateAnimationsCb() {

	if (_gameSys->getAnimationStatus(2) == 2) {
		switch (_s20_nextStonerGuySequenceId) {
		case 0x16B:
			if (!_timers[4]) {
				_s20_stonerGuyShowingJoint = false;
				_gameSys->insertSequence(0x16B, 21, _s20_currStonerGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
				_s20_currStonerGuySequenceId = 0x16B;
				_s20_nextStonerGuySequenceId = -1;
			}
			break;
		case 0x16A:
			// Grab joint
			playGnapPullOutDevice(4, 4);
			playGnapUseDevice(0, 0);
			_gameSys->setAnimation(0x16A, 21, 0);
			_gameSys->insertSequence(0x16A, 21, _s20_currStonerGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
			_s20_currStonerGuySequenceId = 0x16A;
			_s20_nextStonerGuySequenceId = -1;
			invAdd(kItemJoint);
			setFlag(17);
			_s20_stonerGuyShowingJoint = false;
			_gnapActionStatus = kASGrabJointDone;
			break;
		case 0x16E:
			_gameSys->setAnimation(0x16E, 21, 2);
			_gameSys->insertSequence(0x16E, 21, _s20_currStonerGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
			_s20_currStonerGuySequenceId = 0x16E;
			_s20_nextStonerGuySequenceId = -1;
			_s20_nextGroceryStoreGuySequenceId = 0x175;
			break;
		case 0x16D:
			_gameSys->setAnimation(_s20_nextStonerGuySequenceId, 21, 2);
			_gameSys->setAnimation(_s20_nextStonerGuySequenceId, 21, 0);
			_gameSys->insertSequence(_s20_nextStonerGuySequenceId, 21, _s20_currStonerGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
			_s20_currStonerGuySequenceId = _s20_nextStonerGuySequenceId;
			_s20_nextStonerGuySequenceId = -1;
			_gnapActionStatus = kASActionDone;
			break;
		case 0x16F:
			_gameSys->setAnimation(_s20_nextStonerGuySequenceId, 21, 2);
			_gameSys->setAnimation(0x17A, 20, 3);
			_gameSys->insertSequence(_s20_nextStonerGuySequenceId, 21, _s20_currStonerGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0x17A, 20, _s20_currGroceryStoreGuySequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_s20_currGroceryStoreGuySequenceId = 0x17A;
			_s20_nextGroceryStoreGuySequenceId = -1;
			_s20_currStonerGuySequenceId = _s20_nextStonerGuySequenceId;
			_s20_nextStonerGuySequenceId = -1;
			break;
		case 0x171:
			_s20_stonerGuyCtr = (_s20_stonerGuyCtr + 1) % 3;
			switch (_s20_stonerGuyCtr) {
			case 1:
				_s20_nextStonerGuySequenceId = 0x171;
				break;
			case 2:
				_s20_nextStonerGuySequenceId = 0x172;
				break;
			case 3:
				_s20_nextStonerGuySequenceId = 0x173;
				break;
			default:
				_s20_nextStonerGuySequenceId = 0x171;
				break;
			}
			_gameSys->insertSequence(_s20_nextStonerGuySequenceId, 21, _s20_currStonerGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0x17C, 20, _s20_currGroceryStoreGuySequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_gameSys->setAnimation(0x17C, 20, 3);
			_gameSys->setAnimation(_s20_nextStonerGuySequenceId, 21, 2);
			_s20_currGroceryStoreGuySequenceId = 0x17C;
			_s20_nextGroceryStoreGuySequenceId = -1;
			_s20_currStonerGuySequenceId = _s20_nextStonerGuySequenceId;
			_s20_nextStonerGuySequenceId = -1;
			break;
		default:
			_s20_nextStonerGuySequenceId = 0x16C;
			_gameSys->setAnimation(0x16C, 21, 2);
			_gameSys->insertSequence(_s20_nextStonerGuySequenceId, 21, _s20_currStonerGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
			_s20_currStonerGuySequenceId = _s20_nextStonerGuySequenceId;
			_s20_nextStonerGuySequenceId = -1;
			break;
		}
	}

}

void GnapEngine::scene20_stopSounds() {
	stopSound(0x18E);
	stopSound(0x18F);
	stopSound(0x190);
	stopSound(0x191);
	stopSound(0x194);
	stopSound(0x195);
	stopSound(0x192);
	stopSound(0x193);
	stopSound(0x196);
	stopSound(0x197);
	stopSound(0x198);
	stopSound(0x199);
	stopSound(0x19A);
}

void GnapEngine::scene20_run() {
	
	playSound(0x10940, 1);

	startSoundTimerA(8);

	_s20_stonerGuyShowingJoint = false;
	_timers[7] = getRandom(100) + 100;

	_s20_stonerGuyCtr = (_s20_stonerGuyCtr + 1) % 3;
	switch (_s20_stonerGuyCtr) {
	case 1:
		_s20_currStonerGuySequenceId = 0x171;
		break;
	case 2:
		_s20_currStonerGuySequenceId = 0x172;
		break;
	case 3:
		_s20_currStonerGuySequenceId = 0x173;
		break;
	}
	
	_s20_nextStonerGuySequenceId = -1;
	_gameSys->setAnimation(_s20_currStonerGuySequenceId, 21, 2);
	_gameSys->insertSequence(_s20_currStonerGuySequenceId, 21, 0, 0, kSeqNone, 0, 0, 0);
	
	_timers[6] = getRandom(20) + 30;
	
	_s20_currGroceryStoreGuySequenceId = 0x17C;
	_s20_nextGroceryStoreGuySequenceId = -1;
	_gameSys->setAnimation(0x17C, 20, 3);	
	_gameSys->insertSequence(0x17C, 20, 0, 0, kSeqNone, 0, 0, 0);
	
	_timers[5] = getRandom(50) + 130;
	if (isFlag(19))
		_gameSys->insertSequence(0x17F, 20, 0, 0, kSeqNone, 0, 0, 0);
	else
		_gameSys->insertSequence(0x174, 20, 0, 0, kSeqNone, 0, 0, 0);
	
	queueInsertDeviceIcon();
	
	if (isFlag(11)) {
		clearFlag(11);
		endSceneInit();
		_gameSys->setAnimation(0x182, 140, 0);
		_gameSys->insertSequence(0x182, 140, 0, 0, kSeqNone, 0, 0, 0);
		while (_gameSys->getAnimationStatus(0) != 2) {
			// checkGameAppStatus();
			gameUpdateTick();
		}
		initGnapPos(11, 8, kDirBottomLeft);
		initBeaverPos(11, 9, 4);
		gnapWalkTo(5, 8, -1, 0x107BA, 1);
		platypusWalkTo(6, 9, -1, 0x107C2, 1);
	} else {
		switch (_prevSceneNum) {
		case 17:
			initGnapPos(5, 11, kDirBottomRight);
			initBeaverPos(6, 11, 0);
			endSceneInit();
			gnapWalkTo(5, 8, -1, 0x107B9, 1);
			platypusWalkTo(6, 9, -1, 0x107C2, 1);
			break;
		case 18:
			initGnapPos(11, 8, kDirBottomLeft);
			initBeaverPos(11, 9, 4);
			endSceneInit();
			gnapWalkTo(5, 8, -1, 0x107BA, 1);
			platypusWalkTo(6, 9, -1, 0x107C2, 1);
			break;
		case 21:
			initGnapPos(-1, 8, kDirBottomLeft);
			initBeaverPos(-1, 9, 4);
			endSceneInit();
			gnapWalkTo(3, 8, -1, 0x107B9, 1);
			platypusWalkTo(3, 9, -1, 0x107C2, 1);
			break;
		case 22:
			initGnapPos(7, 6, kDirBottomRight);
			initBeaverPos(8, 6, 0);
			endSceneInit();
			gnapWalkTo(8, 8, -1, 0x107B9, 1);
			platypusWalkTo(9, 9, -1, 0x107C2, 1);
			break;
		default:
			initGnapPos(8, 6, kDirBottomLeft);
			initBeaverPos(9, 6, 4);
			endSceneInit();
			_hotspots[kHSWalkArea2]._flags |= SF_WALKABLE;
			gnapWalkTo(8, 8, -1, 0x107BA, 1);
			platypusWalkTo(9, 9, -1, 0x107C2, 1);
			_hotspots[kHSWalkArea2]._flags &= ~SF_WALKABLE;
			break;
		}
	}
	
	while (!_sceneDone) {
	
		updateMouseCursor();
		updateCursorByHotspot();
	
		testWalk(0, 0, -1, -1, -1, -1);
		testWalk(0, 1, 7, 9, 8, 9);
	
		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);
	
		switch (_sceneClickedHotspot) {

		case kHSDevice:
			if (_gnapActionStatus < 0) {
				runMenu();
				scene20_updateHotspots();
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
						gnapKissPlatypus(20);
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

		case kHSExitParkingLot:
			if (_gnapActionStatus < 0) {
				if (_s20_stonerGuyShowingJoint)
					_timers[4] = 0;
				_isLeavingScene = 1;
				_newSceneNum = 21;
				gnapWalkTo(_hotspotsWalkPos[kHSExitParkingLot].x, _hotspotsWalkPos[kHSExitParkingLot].y, 0, 0x107AF, 1);
				_gnapActionStatus = kASLeaveScene;
				platypusWalkTo(_hotspotsWalkPos[kHSExitParkingLot].x, _hotspotsWalkPos[kHSExitParkingLot].y + 1, -1, 0x107CF, 1);
				_beaverFacing = 4;
			}
			break;
		
		case kHSExitPhone:
			if (_gnapActionStatus < 0) {
				if (_s20_stonerGuyShowingJoint)
					_timers[4] = 0;
				_isLeavingScene = 1;
				_newSceneNum = 17;
				gnapWalkTo(_hotspotsWalkPos[kHSExitPhone].x, _hotspotsWalkPos[kHSExitPhone].y, 0, 0x107AE, 1);
				_gnapActionStatus = kASLeaveScene;
				platypusWalkTo(_hotspotsWalkPos[kHSExitPhone].x + 1, _hotspotsWalkPos[kHSExitPhone].y, -1, 0x107C2, 1);
			}
			break;
		
		case kHSExitOutsideToyStore:
			if (_gnapActionStatus < 0) {
				if (_s20_stonerGuyShowingJoint)
					_timers[4] = 0;
				_isLeavingScene = 1;
				_newSceneNum = 18;
				_hotspots[kHSWalkArea2]._flags |= SF_WALKABLE;
				gnapWalkTo(_hotspotsWalkPos[kHSExitOutsideToyStore].x, _hotspotsWalkPos[kHSExitOutsideToyStore].y, 0, 0x107AB, 1);
				_gnapActionStatus = kASLeaveScene;
				platypusWalkTo(_hotspotsWalkPos[kHSExitOutsideToyStore].x, _hotspotsWalkPos[kHSExitOutsideToyStore].y + 1, -1, 0x107CD, 1);
				_hotspots[kHSWalkArea2]._flags &= ~SF_WALKABLE;
			}
			break;
		
		case kHSExitInsideGrubCity:
			if (_gnapActionStatus < 0) {
				if (_s20_stonerGuyShowingJoint)
					_timers[4] = 0;
				_isLeavingScene = 1;
				_newSceneNum = 22;
				gnapWalkTo(_hotspotsWalkPos[kHSExitInsideGrubCity].x, _hotspotsWalkPos[kHSExitInsideGrubCity].y - 1, 0, 0x107BB, 1);
				_gnapActionStatus = kASLeaveScene;
				platypusWalkTo(_hotspotsWalkPos[kHSExitInsideGrubCity].x + 1, _hotspotsWalkPos[kHSExitInsideGrubCity].y, -1, 0x107C2, 1);
				_beaverFacing = 4;
			}
			break;
		
		case kHSExitOutsideCircusWorld:
			if (_gnapActionStatus < 0) {
				if (_s20_stonerGuyShowingJoint)
					_timers[4] = 0;
				_isLeavingScene = 1;
				_newSceneNum = 24;
				gnapWalkTo(_hotspotsWalkPos[kHSExitOutsideCircusWorld].x, _hotspotsWalkPos[kHSExitOutsideCircusWorld].y, 0, 0x107BB, 1);
				_gnapActionStatus = kASLeaveScene;
				platypusWalkTo(_hotspotsWalkPos[kHSExitOutsideCircusWorld].x + 1, _hotspotsWalkPos[kHSExitOutsideCircusWorld].y, -1, 0x107C2, 1);
			}
			break;
		
		case kHSStonerGuy:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[kHSStonerGuy].x, _hotspotsWalkPos[kHSStonerGuy].y, 5, 4);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapMoan2(5, 4);
						break;
					case GRAB_CURSOR:
						_gnapIdleFacing = kDirUpRight;
						gnapWalkTo(_hotspotsWalkPos[kHSStonerGuy].x, _hotspotsWalkPos[kHSStonerGuy].y, 0, getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
						if (_s20_stonerGuyShowingJoint)
							_gnapActionStatus = kASGrabJoint;
						else
							playGnapImpossible(0, 0);
						break;
					case TALK_CURSOR:
						_gnapIdleFacing = kDirUpRight;
						gnapWalkTo(_hotspotsWalkPos[kHSStonerGuy].x, _hotspotsWalkPos[kHSStonerGuy].y, 0, getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						if (isFlag(17))
							_gnapActionStatus = kASTalkStonerGuyNoJoint;
						else
							_gnapActionStatus = kASTalkStonerGuyHasJoint;
						break;
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;
		
		case kHSGroceryStoreGuy:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[kHSGroceryStoreGuy].x, _hotspotsWalkPos[kHSGroceryStoreGuy].y, 2, 4);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(2, 3);
						break;
					case GRAB_CURSOR:
						_s20_stonerGuyShowingJoint = false;
						_gnapIdleFacing = kDirUpLeft;
						gnapWalkTo(_hotspotsWalkPos[kHSGroceryStoreGuy].x, _hotspotsWalkPos[kHSGroceryStoreGuy].y, 0, getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
						_gnapActionStatus = kASGrabGroceryStoreGuy;
						break;
					case TALK_CURSOR:
						_gnapIdleFacing = kDirUpLeft;
						gnapWalkTo(_hotspotsWalkPos[kHSGroceryStoreGuy].x, _hotspotsWalkPos[kHSGroceryStoreGuy].y, 0, getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_gnapActionStatus = kASTalkGroceryStoreGuy;
						break;
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;
		
		case kHSGroceryStoreHat:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex == kItemCowboyHat) {
					_gnapIdleFacing = kDirUpRight;
					gnapWalkTo(_hotspotsWalkPos[kHSGroceryStoreHat].x, _hotspotsWalkPos[kHSGroceryStoreHat].y, 0, getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
					_gnapActionStatus = kASSwitchGroceryStoreHat;
				} else if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[kHSGroceryStoreHat].x, _hotspotsWalkPos[kHSGroceryStoreHat].y, 1, 6);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(1, 6);
						break;
					case GRAB_CURSOR:
						_s20_stonerGuyShowingJoint = false;
						_gnapIdleFacing = kDirUpLeft;
						gnapWalkTo(_hotspotsWalkPos[kHSGroceryStoreGuy].x, _hotspotsWalkPos[kHSGroceryStoreGuy].y, 0, getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
						_gnapActionStatus = kASGrabGroceryStoreHat;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
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

		scene20_updateAnimations();

		if (!isSoundPlaying(0x10940))
			playSound(0x10940, 1);
	
		if (!_isLeavingScene) {
			if (_beaverActionStatus < 0) {
				_hotspots[kHSWalkArea1]._y2 += 48;
				updateBeaverIdleSequence();
				_hotspots[kHSWalkArea1]._y2 -= 48;
			}
			if (_gnapActionStatus < 0)
				updateGnapIdleSequence();
			if (_gnapActionStatus < 0 && !_timers[5] && _s20_nextGroceryStoreGuySequenceId == -1) {
				_timers[5] = getRandom(50) + 130;
				if (getRandom(4) != 0)
					_s20_nextGroceryStoreGuySequenceId = 0x17C;
				else
					_s20_nextGroceryStoreGuySequenceId = 0x17A;
			}
			if (!_timers[7]) {
				_timers[7] = getRandom(100) + 100;
				if (_gnapActionStatus < 0 && _beaverActionStatus < 0) {
					switch (getRandom(3)) {
					case 0:
						_gameSys->insertSequence(0x183, 253, 0, 0, kSeqNone, 0, 0, 0);
						break;
					case 1:
						_gameSys->insertSequence(0x184, 253, 0, 0, kSeqNone, 0, 0, 0);
						break;
					case 2:
						_gameSys->insertSequence(0x185, 253, 0, 0, kSeqNone, 0, 0, 0);
						break;
					}
				}
			}
			playSoundA();
		}
	
		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene20_updateHotspots();
		}
		
		gameUpdateTick();
	
	}
	
}

void GnapEngine::scene20_updateAnimations() {

	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		switch (_gnapActionStatus) {
		case kASLeaveScene:
			_sceneDone = true;
			break;
		case kASTalkStonerGuyNoJoint:
			_gameSys->setAnimation(0x170, 21, 2);
			_gameSys->setAnimation(0x17B, 20, 3);
			_gameSys->insertSequence(0x17B, 20, _s20_currGroceryStoreGuySequenceId, 20, kSeqSyncExists, 0, 0, 0);
			_gameSys->insertSequence(0x170, 21, _s20_currStonerGuySequenceId, 21, kSeqSyncExists, 0, 0, 0);
			stopSound(0x1A1);
			scene20_stopSounds();
			_s20_currGroceryStoreGuySequenceId = 0x17B;
			_s20_currStonerGuySequenceId = 0x170;
			_s20_nextGroceryStoreGuySequenceId = -1;
			_s20_nextStonerGuySequenceId = 0x16E;
			_timers[5] = 100;
			_timers[6] = 100;
			break;
		case kASTalkStonerGuyHasJoint:
			_gameSys->setAnimation(0x168, 21, 2);
			_gameSys->setAnimation(379, 20, 3);
			_gameSys->insertSequence(0x17B, 20, _s20_currGroceryStoreGuySequenceId, 20, kSeqSyncExists, 0, 0, 0);
			_gameSys->insertSequence(0x170, 21, _s20_currStonerGuySequenceId, 21, kSeqSyncExists, 0, 0, 0);
			_gameSys->insertSequence(0x168, 21, 0x170, 21, kSeqSyncWait, 0, 0, 0);
			stopSound(0x1A1);
			scene20_stopSounds();
			_s20_currGroceryStoreGuySequenceId = 0x17B;
			_s20_currStonerGuySequenceId = 0x168;
			_s20_nextGroceryStoreGuySequenceId = -1;
			_s20_nextStonerGuySequenceId = 0x16B;
			_timers[5] = 200;
			_timers[6] = 200;
			_timers[4] = 100;
			_s20_stonerGuyShowingJoint = true;
			_gnapActionStatus = -1;
			break;
		case kASGrabJoint:
			_s20_nextStonerGuySequenceId = 0x16A;
			break;
		case kASActionDone:
			_gnapActionStatus = -1;
			break;
		case kASTalkGroceryStoreGuy:
			_gameSys->setAnimation(0x170, 21, 2);
			_gameSys->setAnimation(0x17B, 20, 3);
			_gameSys->insertSequence(0x17B, 20, _s20_currGroceryStoreGuySequenceId, 20, kSeqSyncExists, 0, 0, 0);
			_gameSys->insertSequence(0x170, 21, _s20_currStonerGuySequenceId, 21, kSeqSyncExists, 0, 0, 0);
			stopSound(0x1A1);
			scene20_stopSounds();
			_s20_currGroceryStoreGuySequenceId = 0x17B;
			_s20_currStonerGuySequenceId = 0x170;
			_s20_groceryStoreGuyCtr = (_s20_groceryStoreGuyCtr + 1) % 2;
			if (_s20_groceryStoreGuyCtr != 0)
				_s20_nextGroceryStoreGuySequenceId = 0x176;
			else
				_s20_nextGroceryStoreGuySequenceId = 0x177;
			_timers[5] = 100;
			_timers[6] = 100;
			break;
		case kASGrabGroceryStoreGuy:
			_gameSys->setAnimation(0x170, 21, 2);
			_gameSys->setAnimation(0x17B, 20, 3);
			_gameSys->insertSequence(0x170, 21, _s20_currStonerGuySequenceId, 21, kSeqSyncExists, 0, 0, 0);
			_gameSys->insertSequence(0x17B, 20, _s20_currGroceryStoreGuySequenceId, 20, kSeqSyncExists, 0, 0, 0);
			stopSound(0x1A1);
			scene20_stopSounds();
			_s20_currGroceryStoreGuySequenceId = 0x17B;
			_s20_currStonerGuySequenceId = 0x170;
			_timers[5] = 120;
			_timers[6] = 120;
			_s20_nextGroceryStoreGuySequenceId = 0x178;
			break;
		case kASGrabGroceryStoreHat:
			_gameSys->setAnimation(0x170, 21, 2);
			_gameSys->setAnimation(0x17B, 20, 3);
			_gameSys->insertSequence(0x17B, 20, _s20_currGroceryStoreGuySequenceId, 20, kSeqSyncExists, 0, 0, 0);
			_gameSys->insertSequence(0x170, 21, _s20_currStonerGuySequenceId, 21, kSeqSyncExists, 0, 0, 0);
			stopSound(0x1A1);
			scene20_stopSounds();
			_s20_currGroceryStoreGuySequenceId = 0x17B;
			_s20_currStonerGuySequenceId = 0x170;
			_s20_nextGroceryStoreGuySequenceId = 0x179;
			break;
		case kASSwitchGroceryStoreHat:
			setGrabCursorSprite(-1);
			_gameSys->setAnimation(0x180, _gnapId, 0);
			_gameSys->insertSequence(0x180, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gnapSequenceId = 0x180;
			_gnapSequenceDatNum = 0;
			invRemove(kItemCowboyHat);
			invAdd(kItemGroceryStoreHat);
			_gnapActionStatus = kASSwitchGroceryStoreHatDone;
			break;
		case kASSwitchGroceryStoreHatDone:
			_gameSys->insertSequence(0x17F, 20, 372, 20, kSeqSyncWait, 0, 0, 0);
			setFlag(19);
			hideCursor();
			setGrabCursorSprite(-1);
			addFullScreenSprite(0x12C, 255);
			_gameSys->setAnimation(0x181, 256, 0);
			_gameSys->insertSequence(0x181, 256, 0, 0, kSeqNone, 0, 0, 0);
			while (_gameSys->getAnimationStatus(0) != 2)
				gameUpdateTick();
			removeFullScreenSprite();
			showCursor();
			setGrabCursorSprite(kItemGroceryStoreHat);
			_gnapIdleFacing = kDirBottomRight;
			gnapWalkTo(3, 8, -1, getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
			_gnapActionStatus = -1;
			break;
		case kASGrabJointDone:
			setGrabCursorSprite(kItemJoint);
			_gnapActionStatus = -1;
			break;
		}
	}
	
	if (_gameSys->getAnimationStatus(3) == 2) {
		switch (_s20_nextGroceryStoreGuySequenceId) {
		case 0x176:
		case 0x177:
			_gameSys->setAnimation(_s20_nextGroceryStoreGuySequenceId, 20, 3);
			_gameSys->insertSequence(_s20_nextGroceryStoreGuySequenceId, 20, _s20_currGroceryStoreGuySequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_s20_currGroceryStoreGuySequenceId = _s20_nextGroceryStoreGuySequenceId;
			_s20_nextGroceryStoreGuySequenceId = -1;
			_s20_nextStonerGuySequenceId = 0x16D;
			break;
		case 0x178:
			_gameSys->setAnimation(_s20_nextGroceryStoreGuySequenceId, 20, 3);
			_gameSys->setAnimation(0x17D, _gnapId, 0);
			_gameSys->insertSequence(_s20_nextGroceryStoreGuySequenceId, 20, _s20_currGroceryStoreGuySequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0x17D, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gnapSequenceId = 0x17D;
			_gnapSequenceDatNum = 0;
			_gnapActionStatus = kASActionDone;
			_gameSys->setAnimation(0x16D, 21, 2);
			_gameSys->insertSequence(0x16D, 21, _s20_currStonerGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
			_s20_currStonerGuySequenceId = 0x16D;
			_s20_currGroceryStoreGuySequenceId = 0x178;
			_s20_nextGroceryStoreGuySequenceId = -1;
			_s20_nextStonerGuySequenceId = -1;
			break;
		case 0x179:
			_gameSys->setAnimation(_s20_nextGroceryStoreGuySequenceId, 20, 3);
			_gameSys->setAnimation(0x16D, 21, 0);
			_gameSys->insertSequence(_s20_nextGroceryStoreGuySequenceId, 20, _s20_currGroceryStoreGuySequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0x17E, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gnapSequenceId = 0x17E;
			_gnapSequenceDatNum = 0;
			_gnapActionStatus = kASActionDone;
			_gameSys->setAnimation(0x16D, 21, 2);
			_gameSys->insertSequence(0x16D, 21, _s20_currStonerGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
			_s20_currStonerGuySequenceId = 0x16D;
			_s20_currGroceryStoreGuySequenceId = 377;
			_s20_nextGroceryStoreGuySequenceId = -1;
			_s20_nextStonerGuySequenceId = -1;
			gnapWalkTo(4, 8, -1, 0x107BB, 1);
			break;
		case 0x17C:
			_gameSys->setAnimation(0, 0, 3);
			_s20_nextStonerGuySequenceId = 0x171;
			break;
		case 0x17A:
			_gameSys->setAnimation(0, 0, 3);
			_s20_nextStonerGuySequenceId = 0x16F;
			break;
		case 0x175:
			_gameSys->setAnimation(0x175, 20, 0);
			_gameSys->setAnimation(0x175, 20, 3);
			_gameSys->insertSequence(0x175, 20, _s20_currGroceryStoreGuySequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_s20_currGroceryStoreGuySequenceId = 0x175;
			_s20_nextGroceryStoreGuySequenceId = -1;
			_gnapActionStatus = kASActionDone;
			break;
		default:
			if (_s20_nextGroceryStoreGuySequenceId != -1) {
				_gameSys->setAnimation(_s20_nextGroceryStoreGuySequenceId, 20, 3);
				_gameSys->insertSequence(_s20_nextGroceryStoreGuySequenceId, 20, _s20_currGroceryStoreGuySequenceId, 20, kSeqSyncWait, 0, 0, 0);
				_s20_currGroceryStoreGuySequenceId = _s20_nextGroceryStoreGuySequenceId;
				_s20_nextGroceryStoreGuySequenceId = -1;
			}
			break;
		}
	}
	
	scene20_updateAnimationsCb();

}

} // End of namespace Gnap
