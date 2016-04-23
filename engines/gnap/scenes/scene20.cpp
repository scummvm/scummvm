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
#include "gnap/scenes/scene20.h"

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

Scene20::Scene20(GnapEngine *vm) : Scene(vm) {
	_s20_stonerGuyCtr = 3;
	_s20_stonerGuyShowingJoint = false;
	_s20_groceryStoreGuyCtr = 0;
}

int Scene20::init() {
	return 0x186;
}

void Scene20::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSGroceryStoreHat, 114, 441, 174, 486, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 0, 7);
	_vm->setHotspot(kHSExitParkingLot, 0, 300, 15, 600, SF_EXIT_L_CURSOR | SF_WALKABLE, 0, 7);
	_vm->setHotspot(kHSStonerGuy, 276, 290, 386, 450, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 8);
	_vm->setHotspot(kHSGroceryStoreGuy, 123, 282, 258, 462, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 7);
	_vm->setHotspot(kHSExitInsideGrubCity, 519, 250, 581, 413, SF_EXIT_L_CURSOR, 8, 7);
	_vm->setHotspot(kHSExitOutsideCircusWorld, 660, 222, 798, 442, SF_EXIT_NE_CURSOR, 9, 6);
	_vm->setHotspot(kHSExitOutsideToyStore, 785, 350, 800, 600, SF_EXIT_R_CURSOR, 11, 8);
	_vm->setHotspot(kHSExitPhone, 250, 585, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 10);
	_vm->setHotspot(kHSWalkArea1, 0, 0, 800, 468);
	_vm->setHotspot(kHSWalkArea2, 605, 0, 800, 600);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_vm->_hotspotsCount = 12;
}

void Scene20::updateAnimationsCb() {
	if (_vm->_gameSys->getAnimationStatus(2) == 2) {
		switch (_s20_nextStonerGuySequenceId) {
		case 0x16B:
			if (!_vm->_timers[4]) {
				_s20_stonerGuyShowingJoint = false;
				_vm->_gameSys->insertSequence(0x16B, 21, _s20_currStonerGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
				_s20_currStonerGuySequenceId = 0x16B;
				_s20_nextStonerGuySequenceId = -1;
			}
			break;
		case 0x16A:
			// Grab joint
			_vm->playGnapPullOutDevice(4, 4);
			_vm->playGnapUseDevice(0, 0);
			_vm->_gameSys->setAnimation(0x16A, 21, 0);
			_vm->_gameSys->insertSequence(0x16A, 21, _s20_currStonerGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
			_s20_currStonerGuySequenceId = 0x16A;
			_s20_nextStonerGuySequenceId = -1;
			_vm->invAdd(kItemJoint);
			_vm->setFlag(kGFJointTaken);
			_s20_stonerGuyShowingJoint = false;
			_vm->_gnapActionStatus = kASGrabJointDone;
			break;
		case 0x16E:
			_vm->_gameSys->setAnimation(0x16E, 21, 2);
			_vm->_gameSys->insertSequence(0x16E, 21, _s20_currStonerGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
			_s20_currStonerGuySequenceId = 0x16E;
			_s20_nextStonerGuySequenceId = -1;
			_s20_nextGroceryStoreGuySequenceId = 0x175;
			break;
		case 0x16D:
			_vm->_gameSys->setAnimation(_s20_nextStonerGuySequenceId, 21, 2);
			_vm->_gameSys->setAnimation(_s20_nextStonerGuySequenceId, 21, 0);
			_vm->_gameSys->insertSequence(_s20_nextStonerGuySequenceId, 21, _s20_currStonerGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
			_s20_currStonerGuySequenceId = _s20_nextStonerGuySequenceId;
			_s20_nextStonerGuySequenceId = -1;
			_vm->_gnapActionStatus = kASActionDone;
			break;
		case 0x16F:
			_vm->_gameSys->setAnimation(_s20_nextStonerGuySequenceId, 21, 2);
			_vm->_gameSys->setAnimation(0x17A, 20, 3);
			_vm->_gameSys->insertSequence(_s20_nextStonerGuySequenceId, 21, _s20_currStonerGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x17A, 20, _s20_currGroceryStoreGuySequenceId, 20, kSeqSyncWait, 0, 0, 0);
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
			_vm->_gameSys->insertSequence(_s20_nextStonerGuySequenceId, 21, _s20_currStonerGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x17C, 20, _s20_currGroceryStoreGuySequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(0x17C, 20, 3);
			_vm->_gameSys->setAnimation(_s20_nextStonerGuySequenceId, 21, 2);
			_s20_currGroceryStoreGuySequenceId = 0x17C;
			_s20_nextGroceryStoreGuySequenceId = -1;
			_s20_currStonerGuySequenceId = _s20_nextStonerGuySequenceId;
			_s20_nextStonerGuySequenceId = -1;
			break;
		default:
			_s20_nextStonerGuySequenceId = 0x16C;
			_vm->_gameSys->setAnimation(0x16C, 21, 2);
			_vm->_gameSys->insertSequence(_s20_nextStonerGuySequenceId, 21, _s20_currStonerGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
			_s20_currStonerGuySequenceId = _s20_nextStonerGuySequenceId;
			_s20_nextStonerGuySequenceId = -1;
			break;
		}
	}
}

void Scene20::stopSounds() {
	_vm->stopSound(0x18E);
	_vm->stopSound(0x18F);
	_vm->stopSound(0x190);
	_vm->stopSound(0x191);
	_vm->stopSound(0x194);
	_vm->stopSound(0x195);
	_vm->stopSound(0x192);
	_vm->stopSound(0x193);
	_vm->stopSound(0x196);
	_vm->stopSound(0x197);
	_vm->stopSound(0x198);
	_vm->stopSound(0x199);
	_vm->stopSound(0x19A);
}

void Scene20::run() {
	_vm->playSound(0x10940, true);
	_vm->startSoundTimerA(8);

	_s20_stonerGuyShowingJoint = false;
	_vm->_timers[7] = _vm->getRandom(100) + 100;

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
	_vm->_gameSys->setAnimation(_s20_currStonerGuySequenceId, 21, 2);
	_vm->_gameSys->insertSequence(_s20_currStonerGuySequenceId, 21, 0, 0, kSeqNone, 0, 0, 0);
	
	_vm->_timers[6] = _vm->getRandom(20) + 30;
	
	_s20_currGroceryStoreGuySequenceId = 0x17C;
	_s20_nextGroceryStoreGuySequenceId = -1;
	_vm->_gameSys->setAnimation(0x17C, 20, 3);	
	_vm->_gameSys->insertSequence(0x17C, 20, 0, 0, kSeqNone, 0, 0, 0);
	
	_vm->_timers[5] = _vm->getRandom(50) + 130;
	if (_vm->isFlag(kGFGroceryStoreHatTaken))
		_vm->_gameSys->insertSequence(0x17F, 20, 0, 0, kSeqNone, 0, 0, 0);
	else
		_vm->_gameSys->insertSequence(0x174, 20, 0, 0, kSeqNone, 0, 0, 0);
	
	_vm->queueInsertDeviceIcon();
	
	if (_vm->isFlag(kGFSceneFlag1)) {
		_vm->clearFlag(kGFSceneFlag1);
		_vm->endSceneInit();
		_vm->_gameSys->setAnimation(0x182, 140, 0);
		_vm->_gameSys->insertSequence(0x182, 140, 0, 0, kSeqNone, 0, 0, 0);
		while (_vm->_gameSys->getAnimationStatus(0) != 2) {
			// checkGameAppStatus();
			_vm->gameUpdateTick();
		}
		_vm->initGnapPos(11, 8, kDirBottomLeft);
		_vm->initPlatypusPos(11, 9, kDirUnk4);
		_vm->gnapWalkTo(5, 8, -1, 0x107BA, 1);
		_vm->platypusWalkTo(6, 9, -1, 0x107C2, 1);
	} else {
		switch (_vm->_prevSceneNum) {
		case 17:
			_vm->initGnapPos(5, 11, kDirBottomRight);
			_vm->initPlatypusPos(6, 11, kDirNone);
			_vm->endSceneInit();
			_vm->gnapWalkTo(5, 8, -1, 0x107B9, 1);
			_vm->platypusWalkTo(6, 9, -1, 0x107C2, 1);
			break;
		case 18:
			_vm->initGnapPos(11, 8, kDirBottomLeft);
			_vm->initPlatypusPos(11, 9, kDirUnk4);
			_vm->endSceneInit();
			_vm->gnapWalkTo(5, 8, -1, 0x107BA, 1);
			_vm->platypusWalkTo(6, 9, -1, 0x107C2, 1);
			break;
		case 21:
			_vm->initGnapPos(-1, 8, kDirBottomLeft);
			_vm->initPlatypusPos(-1, 9, kDirUnk4);
			_vm->endSceneInit();
			_vm->gnapWalkTo(3, 8, -1, 0x107B9, 1);
			_vm->platypusWalkTo(3, 9, -1, 0x107C2, 1);
			break;
		case 22:
			_vm->initGnapPos(7, 6, kDirBottomRight);
			_vm->initPlatypusPos(8, 6, kDirNone);
			_vm->endSceneInit();
			_vm->gnapWalkTo(8, 8, -1, 0x107B9, 1);
			_vm->platypusWalkTo(9, 9, -1, 0x107C2, 1);
			break;
		default:
			_vm->initGnapPos(8, 6, kDirBottomLeft);
			_vm->initPlatypusPos(9, 6, kDirUnk4);
			_vm->endSceneInit();
			_vm->_hotspots[kHSWalkArea2]._flags |= SF_WALKABLE;
			_vm->gnapWalkTo(8, 8, -1, 0x107BA, 1);
			_vm->platypusWalkTo(9, 9, -1, 0x107C2, 1);
			_vm->_hotspots[kHSWalkArea2]._flags &= ~SF_WALKABLE;
			break;
		}
	}
	
	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
	
		_vm->testWalk(0, 0, -1, -1, -1, -1);
		_vm->testWalk(0, 1, 7, 9, 8, 9);
	
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
						_vm->gnapKissPlatypus(20);
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

		case kHSExitParkingLot:
			if (_vm->_gnapActionStatus < 0) {
				if (_s20_stonerGuyShowingJoint)
					_vm->_timers[4] = 0;
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 21;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitParkingLot].x, _vm->_hotspotsWalkPos[kHSExitParkingLot].y, 0, 0x107AF, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitParkingLot].x, _vm->_hotspotsWalkPos[kHSExitParkingLot].y + 1, -1, 0x107CF, 1);
				_vm->_platypusFacing = kDirUnk4;
			}
			break;
		
		case kHSExitPhone:
			if (_vm->_gnapActionStatus < 0) {
				if (_s20_stonerGuyShowingJoint)
					_vm->_timers[4] = 0;
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 17;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitPhone].x, _vm->_hotspotsWalkPos[kHSExitPhone].y, 0, 0x107AE, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitPhone].x + 1, _vm->_hotspotsWalkPos[kHSExitPhone].y, -1, 0x107C2, 1);
			}
			break;
		
		case kHSExitOutsideToyStore:
			if (_vm->_gnapActionStatus < 0) {
				if (_s20_stonerGuyShowingJoint)
					_vm->_timers[4] = 0;
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 18;
				_vm->_hotspots[kHSWalkArea2]._flags |= SF_WALKABLE;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitOutsideToyStore].x, _vm->_hotspotsWalkPos[kHSExitOutsideToyStore].y, 0, 0x107AB, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitOutsideToyStore].x, _vm->_hotspotsWalkPos[kHSExitOutsideToyStore].y + 1, -1, 0x107CD, 1);
				_vm->_hotspots[kHSWalkArea2]._flags &= ~SF_WALKABLE;
			}
			break;
		
		case kHSExitInsideGrubCity:
			if (_vm->_gnapActionStatus < 0) {
				if (_s20_stonerGuyShowingJoint)
					_vm->_timers[4] = 0;
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 22;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitInsideGrubCity].x, _vm->_hotspotsWalkPos[kHSExitInsideGrubCity].y - 1, 0, 0x107BB, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitInsideGrubCity].x + 1, _vm->_hotspotsWalkPos[kHSExitInsideGrubCity].y, -1, 0x107C2, 1);
				_vm->_platypusFacing = kDirUnk4;
			}
			break;
		
		case kHSExitOutsideCircusWorld:
			if (_vm->_gnapActionStatus < 0) {
				if (_s20_stonerGuyShowingJoint)
					_vm->_timers[4] = 0;
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 24;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitOutsideCircusWorld].x, _vm->_hotspotsWalkPos[kHSExitOutsideCircusWorld].y, 0, 0x107BB, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitOutsideCircusWorld].x + 1, _vm->_hotspotsWalkPos[kHSExitOutsideCircusWorld].y, -1, 0x107C2, 1);
			}
			break;
		
		case kHSStonerGuy:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHSStonerGuy].x, _vm->_hotspotsWalkPos[kHSStonerGuy].y, 5, 4);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapMoan2(5, 4);
						break;
					case GRAB_CURSOR:
						_vm->_gnapIdleFacing = kDirUpRight;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSStonerGuy].x, _vm->_hotspotsWalkPos[kHSStonerGuy].y, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
						if (_s20_stonerGuyShowingJoint)
							_vm->_gnapActionStatus = kASGrabJoint;
						else
							_vm->playGnapImpossible(0, 0);
						break;
					case TALK_CURSOR:
						_vm->_gnapIdleFacing = kDirUpRight;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSStonerGuy].x, _vm->_hotspotsWalkPos[kHSStonerGuy].y, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						if (_vm->isFlag(kGFJointTaken))
							_vm->_gnapActionStatus = kASTalkStonerGuyNoJoint;
						else
							_vm->_gnapActionStatus = kASTalkStonerGuyHasJoint;
						break;
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;
		
		case kHSGroceryStoreGuy:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHSGroceryStoreGuy].x, _vm->_hotspotsWalkPos[kHSGroceryStoreGuy].y, 2, 4);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(2, 3);
						break;
					case GRAB_CURSOR:
						_s20_stonerGuyShowingJoint = false;
						_vm->_gnapIdleFacing = kDirUpLeft;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSGroceryStoreGuy].x, _vm->_hotspotsWalkPos[kHSGroceryStoreGuy].y, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
						_vm->_gnapActionStatus = kASGrabGroceryStoreGuy;
						break;
					case TALK_CURSOR:
						_vm->_gnapIdleFacing = kDirUpLeft;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSGroceryStoreGuy].x, _vm->_hotspotsWalkPos[kHSGroceryStoreGuy].y, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_vm->_gnapActionStatus = kASTalkGroceryStoreGuy;
						break;
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;
		
		case kHSGroceryStoreHat:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemCowboyHat) {
					_vm->_gnapIdleFacing = kDirUpRight;
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSGroceryStoreHat].x, _vm->_hotspotsWalkPos[kHSGroceryStoreHat].y, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
					_vm->_gnapActionStatus = kASSwitchGroceryStoreHat;
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHSGroceryStoreHat].x, _vm->_hotspotsWalkPos[kHSGroceryStoreHat].y, 1, 6);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(1, 6);
						break;
					case GRAB_CURSOR:
						_s20_stonerGuyShowingJoint = false;
						_vm->_gnapIdleFacing = kDirUpLeft;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSGroceryStoreGuy].x, _vm->_hotspotsWalkPos[kHSGroceryStoreGuy].y, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
						_vm->_gnapActionStatus = kASGrabGroceryStoreHat;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
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

		if (!_vm->isSoundPlaying(0x10940))
			_vm->playSound(0x10940, true);
	
		if (!_vm->_isLeavingScene) {
			if (_vm->_platypusActionStatus < 0) {
				_vm->_hotspots[kHSWalkArea1]._y2 += 48;
				_vm->updatePlatypusIdleSequence();
				_vm->_hotspots[kHSWalkArea1]._y2 -= 48;
			}
			if (_vm->_gnapActionStatus < 0)
				_vm->updateGnapIdleSequence();
			if (_vm->_gnapActionStatus < 0 && !_vm->_timers[5] && _s20_nextGroceryStoreGuySequenceId == -1) {
				_vm->_timers[5] = _vm->getRandom(50) + 130;
				if (_vm->getRandom(4) != 0)
					_s20_nextGroceryStoreGuySequenceId = 0x17C;
				else
					_s20_nextGroceryStoreGuySequenceId = 0x17A;
			}
			if (!_vm->_timers[7]) {
				_vm->_timers[7] = _vm->getRandom(100) + 100;
				if (_vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0) {
					switch (_vm->getRandom(3)) {
					case 0:
						_vm->_gameSys->insertSequence(0x183, 253, 0, 0, kSeqNone, 0, 0, 0);
						break;
					case 1:
						_vm->_gameSys->insertSequence(0x184, 253, 0, 0, kSeqNone, 0, 0, 0);
						break;
					case 2:
						_vm->_gameSys->insertSequence(0x185, 253, 0, 0, kSeqNone, 0, 0, 0);
						break;
					}
				}
			}
			_vm->playSoundA();
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

void Scene20::updateAnimations() {
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kASLeaveScene:
			_vm->_sceneDone = true;
			break;
		case kASTalkStonerGuyNoJoint:
			_vm->_gameSys->setAnimation(0x170, 21, 2);
			_vm->_gameSys->setAnimation(0x17B, 20, 3);
			_vm->_gameSys->insertSequence(0x17B, 20, _s20_currGroceryStoreGuySequenceId, 20, kSeqSyncExists, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x170, 21, _s20_currStonerGuySequenceId, 21, kSeqSyncExists, 0, 0, 0);
			_vm->stopSound(0x1A1);
			stopSounds();
			_s20_currGroceryStoreGuySequenceId = 0x17B;
			_s20_currStonerGuySequenceId = 0x170;
			_s20_nextGroceryStoreGuySequenceId = -1;
			_s20_nextStonerGuySequenceId = 0x16E;
			_vm->_timers[5] = 100;
			_vm->_timers[6] = 100;
			break;
		case kASTalkStonerGuyHasJoint:
			_vm->_gameSys->setAnimation(0x168, 21, 2);
			_vm->_gameSys->setAnimation(379, 20, 3);
			_vm->_gameSys->insertSequence(0x17B, 20, _s20_currGroceryStoreGuySequenceId, 20, kSeqSyncExists, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x170, 21, _s20_currStonerGuySequenceId, 21, kSeqSyncExists, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x168, 21, 0x170, 21, kSeqSyncWait, 0, 0, 0);
			_vm->stopSound(0x1A1);
			stopSounds();
			_s20_currGroceryStoreGuySequenceId = 0x17B;
			_s20_currStonerGuySequenceId = 0x168;
			_s20_nextGroceryStoreGuySequenceId = -1;
			_s20_nextStonerGuySequenceId = 0x16B;
			_vm->_timers[5] = 200;
			_vm->_timers[6] = 200;
			_vm->_timers[4] = 100;
			_s20_stonerGuyShowingJoint = true;
			_vm->_gnapActionStatus = -1;
			break;
		case kASGrabJoint:
			_s20_nextStonerGuySequenceId = 0x16A;
			break;
		case kASActionDone:
			_vm->_gnapActionStatus = -1;
			break;
		case kASTalkGroceryStoreGuy:
			_vm->_gameSys->setAnimation(0x170, 21, 2);
			_vm->_gameSys->setAnimation(0x17B, 20, 3);
			_vm->_gameSys->insertSequence(0x17B, 20, _s20_currGroceryStoreGuySequenceId, 20, kSeqSyncExists, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x170, 21, _s20_currStonerGuySequenceId, 21, kSeqSyncExists, 0, 0, 0);
			_vm->stopSound(0x1A1);
			stopSounds();
			_s20_currGroceryStoreGuySequenceId = 0x17B;
			_s20_currStonerGuySequenceId = 0x170;
			_s20_groceryStoreGuyCtr = (_s20_groceryStoreGuyCtr + 1) % 2;
			if (_s20_groceryStoreGuyCtr != 0)
				_s20_nextGroceryStoreGuySequenceId = 0x176;
			else
				_s20_nextGroceryStoreGuySequenceId = 0x177;
			_vm->_timers[5] = 100;
			_vm->_timers[6] = 100;
			break;
		case kASGrabGroceryStoreGuy:
			_vm->_gameSys->setAnimation(0x170, 21, 2);
			_vm->_gameSys->setAnimation(0x17B, 20, 3);
			_vm->_gameSys->insertSequence(0x170, 21, _s20_currStonerGuySequenceId, 21, kSeqSyncExists, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x17B, 20, _s20_currGroceryStoreGuySequenceId, 20, kSeqSyncExists, 0, 0, 0);
			_vm->stopSound(0x1A1);
			stopSounds();
			_s20_currGroceryStoreGuySequenceId = 0x17B;
			_s20_currStonerGuySequenceId = 0x170;
			_vm->_timers[5] = 120;
			_vm->_timers[6] = 120;
			_s20_nextGroceryStoreGuySequenceId = 0x178;
			break;
		case kASGrabGroceryStoreHat:
			_vm->_gameSys->setAnimation(0x170, 21, 2);
			_vm->_gameSys->setAnimation(0x17B, 20, 3);
			_vm->_gameSys->insertSequence(0x17B, 20, _s20_currGroceryStoreGuySequenceId, 20, kSeqSyncExists, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x170, 21, _s20_currStonerGuySequenceId, 21, kSeqSyncExists, 0, 0, 0);
			_vm->stopSound(0x1A1);
			stopSounds();
			_s20_currGroceryStoreGuySequenceId = 0x17B;
			_s20_currStonerGuySequenceId = 0x170;
			_s20_nextGroceryStoreGuySequenceId = 0x179;
			break;
		case kASSwitchGroceryStoreHat:
			_vm->setGrabCursorSprite(-1);
			_vm->_gameSys->setAnimation(0x180, _vm->_gnapId, 0);
			_vm->_gameSys->insertSequence(0x180, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceId = 0x180;
			_vm->_gnapSequenceDatNum = 0;
			_vm->invRemove(kItemCowboyHat);
			_vm->invAdd(kItemGroceryStoreHat);
			_vm->_gnapActionStatus = kASSwitchGroceryStoreHatDone;
			break;
		case kASSwitchGroceryStoreHatDone:
			_vm->_gameSys->insertSequence(0x17F, 20, 372, 20, kSeqSyncWait, 0, 0, 0);
			_vm->setFlag(kGFGroceryStoreHatTaken);
			_vm->hideCursor();
			_vm->setGrabCursorSprite(-1);
			_vm->addFullScreenSprite(0x12C, 255);
			_vm->_gameSys->setAnimation(0x181, 256, 0);
			_vm->_gameSys->insertSequence(0x181, 256, 0, 0, kSeqNone, 0, 0, 0);
			while (_vm->_gameSys->getAnimationStatus(0) != 2)
				_vm->gameUpdateTick();
			_vm->removeFullScreenSprite();
			_vm->showCursor();
			_vm->setGrabCursorSprite(kItemGroceryStoreHat);
			_vm->_gnapIdleFacing = kDirBottomRight;
			_vm->gnapWalkTo(3, 8, -1, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
			_vm->_gnapActionStatus = -1;
			break;
		case kASGrabJointDone:
			_vm->setGrabCursorSprite(kItemJoint);
			_vm->_gnapActionStatus = -1;
			break;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(3) == 2) {
		switch (_s20_nextGroceryStoreGuySequenceId) {
		case 0x176:
		case 0x177:
			_vm->_gameSys->setAnimation(_s20_nextGroceryStoreGuySequenceId, 20, 3);
			_vm->_gameSys->insertSequence(_s20_nextGroceryStoreGuySequenceId, 20, _s20_currGroceryStoreGuySequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_s20_currGroceryStoreGuySequenceId = _s20_nextGroceryStoreGuySequenceId;
			_s20_nextGroceryStoreGuySequenceId = -1;
			_s20_nextStonerGuySequenceId = 0x16D;
			break;
		case 0x178:
			_vm->_gameSys->setAnimation(_s20_nextGroceryStoreGuySequenceId, 20, 3);
			_vm->_gameSys->setAnimation(0x17D, _vm->_gnapId, 0);
			_vm->_gameSys->insertSequence(_s20_nextGroceryStoreGuySequenceId, 20, _s20_currGroceryStoreGuySequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x17D, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceId = 0x17D;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapActionStatus = kASActionDone;
			_vm->_gameSys->setAnimation(0x16D, 21, 2);
			_vm->_gameSys->insertSequence(0x16D, 21, _s20_currStonerGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
			_s20_currStonerGuySequenceId = 0x16D;
			_s20_currGroceryStoreGuySequenceId = 0x178;
			_s20_nextGroceryStoreGuySequenceId = -1;
			_s20_nextStonerGuySequenceId = -1;
			break;
		case 0x179:
			_vm->_gameSys->setAnimation(_s20_nextGroceryStoreGuySequenceId, 20, 3);
			_vm->_gameSys->setAnimation(0x16D, 21, 0);
			_vm->_gameSys->insertSequence(_s20_nextGroceryStoreGuySequenceId, 20, _s20_currGroceryStoreGuySequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x17E, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceId = 0x17E;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapActionStatus = kASActionDone;
			_vm->_gameSys->setAnimation(0x16D, 21, 2);
			_vm->_gameSys->insertSequence(0x16D, 21, _s20_currStonerGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
			_s20_currStonerGuySequenceId = 0x16D;
			_s20_currGroceryStoreGuySequenceId = 377;
			_s20_nextGroceryStoreGuySequenceId = -1;
			_s20_nextStonerGuySequenceId = -1;
			_vm->gnapWalkTo(4, 8, -1, 0x107BB, 1);
			break;
		case 0x17C:
			_vm->_gameSys->setAnimation(0, 0, 3);
			_s20_nextStonerGuySequenceId = 0x171;
			break;
		case 0x17A:
			_vm->_gameSys->setAnimation(0, 0, 3);
			_s20_nextStonerGuySequenceId = 0x16F;
			break;
		case 0x175:
			_vm->_gameSys->setAnimation(0x175, 20, 0);
			_vm->_gameSys->setAnimation(0x175, 20, 3);
			_vm->_gameSys->insertSequence(0x175, 20, _s20_currGroceryStoreGuySequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_s20_currGroceryStoreGuySequenceId = 0x175;
			_s20_nextGroceryStoreGuySequenceId = -1;
			_vm->_gnapActionStatus = kASActionDone;
			break;
		default:
			if (_s20_nextGroceryStoreGuySequenceId != -1) {
				_vm->_gameSys->setAnimation(_s20_nextGroceryStoreGuySequenceId, 20, 3);
				_vm->_gameSys->insertSequence(_s20_nextGroceryStoreGuySequenceId, 20, _s20_currGroceryStoreGuySequenceId, 20, kSeqSyncWait, 0, 0, 0);
				_s20_currGroceryStoreGuySequenceId = _s20_nextGroceryStoreGuySequenceId;
				_s20_nextGroceryStoreGuySequenceId = -1;
			}
			break;
		}
	}

	updateAnimationsCb();
}

} // End of namespace Gnap
