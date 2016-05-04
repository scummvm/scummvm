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
#include "gnap/scenes/group2.h"

namespace Gnap {

Scene20::Scene20(GnapEngine *vm) : Scene(vm) {
	_stonerGuyCtr = 3;
	_stonerGuyShowingJoint = false;
	_groceryStoreGuyCtr = 0;
}

int Scene20::init() {
	return 0x186;
}

void Scene20::updateHotspots() {
	_vm->setHotspot(kHS20Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS20GroceryStoreHat, 114, 441, 174, 486, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 0, 7);
	_vm->setHotspot(kHS20ExitParkingLot, 0, 300, 15, 600, SF_EXIT_L_CURSOR | SF_WALKABLE, 0, 7);
	_vm->setHotspot(kHS20StonerGuy, 276, 290, 386, 450, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 8);
	_vm->setHotspot(kHS20GroceryStoreGuy, 123, 282, 258, 462, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 7);
	_vm->setHotspot(kHS20ExitInsideGrubCity, 519, 250, 581, 413, SF_EXIT_L_CURSOR, 8, 7);
	_vm->setHotspot(kHS20ExitOutsideCircusWorld, 660, 222, 798, 442, SF_EXIT_NE_CURSOR, 9, 6);
	_vm->setHotspot(kHS20ExitOutsideToyStore, 785, 350, 800, 600, SF_EXIT_R_CURSOR, 11, 8);
	_vm->setHotspot(kHS20ExitPhone, 250, 585, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 10);
	_vm->setHotspot(kHS20WalkArea1, 0, 0, 800, 468);
	_vm->setHotspot(kHS20WalkArea2, 605, 0, 800, 600);
	_vm->setDeviceHotspot(kHS20Device, -1, -1, -1, -1);
	_vm->_hotspotsCount = 12;
}

void Scene20::updateAnimationsCb() {
	GameSys& gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(2) == 2) {
		switch (_nextStonerGuySequenceId) {
		case 0x16B:
			if (!_vm->_timers[4]) {
				_stonerGuyShowingJoint = false;
				gameSys.insertSequence(0x16B, 21, _currStonerGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
				_currStonerGuySequenceId = 0x16B;
				_nextStonerGuySequenceId = -1;
			}
			break;
		case 0x16A:
			// Grab joint
			_vm->playGnapPullOutDevice(4, 4);
			_vm->playGnapUseDevice(0, 0);
			gameSys.setAnimation(0x16A, 21, 0);
			gameSys.insertSequence(0x16A, 21, _currStonerGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
			_currStonerGuySequenceId = 0x16A;
			_nextStonerGuySequenceId = -1;
			_vm->invAdd(kItemJoint);
			_vm->setFlag(kGFJointTaken);
			_stonerGuyShowingJoint = false;
			_vm->_gnap->_actionStatus = kAS20GrabJointDone;
			break;
		case 0x16E:
			gameSys.setAnimation(0x16E, 21, 2);
			gameSys.insertSequence(0x16E, 21, _currStonerGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
			_currStonerGuySequenceId = 0x16E;
			_nextStonerGuySequenceId = -1;
			_nextGroceryStoreGuySequenceId = 0x175;
			break;
		case 0x16D:
			gameSys.setAnimation(_nextStonerGuySequenceId, 21, 2);
			gameSys.setAnimation(_nextStonerGuySequenceId, 21, 0);
			gameSys.insertSequence(_nextStonerGuySequenceId, 21, _currStonerGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
			_currStonerGuySequenceId = _nextStonerGuySequenceId;
			_nextStonerGuySequenceId = -1;
			_vm->_gnap->_actionStatus = kAS20ActionDone;
			break;
		case 0x16F:
			gameSys.setAnimation(_nextStonerGuySequenceId, 21, 2);
			gameSys.setAnimation(0x17A, 20, 3);
			gameSys.insertSequence(_nextStonerGuySequenceId, 21, _currStonerGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0x17A, 20, _currGroceryStoreGuySequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_currGroceryStoreGuySequenceId = 0x17A;
			_nextGroceryStoreGuySequenceId = -1;
			_currStonerGuySequenceId = _nextStonerGuySequenceId;
			_nextStonerGuySequenceId = -1;
			break;
		case 0x171:
			_stonerGuyCtr = (_stonerGuyCtr + 1) % 3;
			switch (_stonerGuyCtr) {
			case 1:
				_nextStonerGuySequenceId = 0x171;
				break;
			case 2:
				_nextStonerGuySequenceId = 0x172;
				break;
			case 3:
				_nextStonerGuySequenceId = 0x173;
				break;
			default:
				_nextStonerGuySequenceId = 0x171;
				break;
			}
			gameSys.insertSequence(_nextStonerGuySequenceId, 21, _currStonerGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0x17C, 20, _currGroceryStoreGuySequenceId, 20, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(0x17C, 20, 3);
			gameSys.setAnimation(_nextStonerGuySequenceId, 21, 2);
			_currGroceryStoreGuySequenceId = 0x17C;
			_nextGroceryStoreGuySequenceId = -1;
			_currStonerGuySequenceId = _nextStonerGuySequenceId;
			_nextStonerGuySequenceId = -1;
			break;
		default:
			_nextStonerGuySequenceId = 0x16C;
			gameSys.setAnimation(0x16C, 21, 2);
			gameSys.insertSequence(_nextStonerGuySequenceId, 21, _currStonerGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
			_currStonerGuySequenceId = _nextStonerGuySequenceId;
			_nextStonerGuySequenceId = -1;
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
	GameSys& gameSys = *_vm->_gameSys;

	_vm->playSound(0x10940, true);
	_vm->startSoundTimerA(8);

	_stonerGuyShowingJoint = false;
	_vm->_timers[7] = _vm->getRandom(100) + 100;

	_stonerGuyCtr = (_stonerGuyCtr + 1) % 3;
	switch (_stonerGuyCtr) {
	case 1:
		_currStonerGuySequenceId = 0x171;
		break;
	case 2:
		_currStonerGuySequenceId = 0x172;
		break;
	case 3:
		_currStonerGuySequenceId = 0x173;
		break;
	}

	_nextStonerGuySequenceId = -1;
	gameSys.setAnimation(_currStonerGuySequenceId, 21, 2);
	gameSys.insertSequence(_currStonerGuySequenceId, 21, 0, 0, kSeqNone, 0, 0, 0);

	_vm->_timers[6] = _vm->getRandom(20) + 30;

	_currGroceryStoreGuySequenceId = 0x17C;
	_nextGroceryStoreGuySequenceId = -1;
	gameSys.setAnimation(0x17C, 20, 3);
	gameSys.insertSequence(0x17C, 20, 0, 0, kSeqNone, 0, 0, 0);

	_vm->_timers[5] = _vm->getRandom(50) + 130;
	if (_vm->isFlag(kGFGroceryStoreHatTaken))
		gameSys.insertSequence(0x17F, 20, 0, 0, kSeqNone, 0, 0, 0);
	else
		gameSys.insertSequence(0x174, 20, 0, 0, kSeqNone, 0, 0, 0);

	_vm->queueInsertDeviceIcon();

	if (_vm->isFlag(kGFSceneFlag1)) {
		_vm->clearFlag(kGFSceneFlag1);
		_vm->endSceneInit();
		gameSys.setAnimation(0x182, 140, 0);
		gameSys.insertSequence(0x182, 140, 0, 0, kSeqNone, 0, 0, 0);
		while (gameSys.getAnimationStatus(0) != 2) {
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
			_vm->_hotspots[kHS20WalkArea2]._flags |= SF_WALKABLE;
			_vm->gnapWalkTo(8, 8, -1, 0x107BA, 1);
			_vm->platypusWalkTo(9, 9, -1, 0x107C2, 1);
			_vm->_hotspots[kHS20WalkArea2]._flags &= ~SF_WALKABLE;
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
		case kHS20Device:
			if (_vm->_gnap->_actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS20Platypus:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemJoint) {
					_vm->gnapUseJointOnPlatypus();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapImpossible(0, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(_vm->_plat->_pos.x, _vm->_plat->_pos.y);
						break;
					case GRAB_CURSOR:
						_vm->gnapKissPlatypus(20);
						break;
					case TALK_CURSOR:
						_vm->playGnapBrainPulsating(_vm->_plat->_pos.x, _vm->_plat->_pos.y);
						_vm->playPlatypusSequence(_vm->getPlatypusSequenceId());
						break;
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS20ExitParkingLot:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_stonerGuyShowingJoint)
					_vm->_timers[4] = 0;
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 21;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS20ExitParkingLot].x, _vm->_hotspotsWalkPos[kHS20ExitParkingLot].y, 0, 0x107AF, 1);
				_vm->_gnap->_actionStatus = kAS20LeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS20ExitParkingLot].x, _vm->_hotspotsWalkPos[kHS20ExitParkingLot].y + 1, -1, 0x107CF, 1);
				_vm->_plat->_idleFacing = kDirUnk4;
			}
			break;

		case kHS20ExitPhone:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_stonerGuyShowingJoint)
					_vm->_timers[4] = 0;
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 17;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS20ExitPhone].x, _vm->_hotspotsWalkPos[kHS20ExitPhone].y, 0, 0x107AE, 1);
				_vm->_gnap->_actionStatus = kAS20LeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS20ExitPhone].x + 1, _vm->_hotspotsWalkPos[kHS20ExitPhone].y, -1, 0x107C2, 1);
			}
			break;

		case kHS20ExitOutsideToyStore:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_stonerGuyShowingJoint)
					_vm->_timers[4] = 0;
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 18;
				_vm->_hotspots[kHS20WalkArea2]._flags |= SF_WALKABLE;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS20ExitOutsideToyStore].x, _vm->_hotspotsWalkPos[kHS20ExitOutsideToyStore].y, 0, 0x107AB, 1);
				_vm->_gnap->_actionStatus = kAS20LeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS20ExitOutsideToyStore].x, _vm->_hotspotsWalkPos[kHS20ExitOutsideToyStore].y + 1, -1, 0x107CD, 1);
				_vm->_hotspots[kHS20WalkArea2]._flags &= ~SF_WALKABLE;
			}
			break;

		case kHS20ExitInsideGrubCity:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_stonerGuyShowingJoint)
					_vm->_timers[4] = 0;
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 22;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS20ExitInsideGrubCity].x, _vm->_hotspotsWalkPos[kHS20ExitInsideGrubCity].y - 1, 0, 0x107BB, 1);
				_vm->_gnap->_actionStatus = kAS20LeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS20ExitInsideGrubCity].x + 1, _vm->_hotspotsWalkPos[kHS20ExitInsideGrubCity].y, -1, 0x107C2, 1);
				_vm->_plat->_idleFacing = kDirUnk4;
			}
			break;

		case kHS20ExitOutsideCircusWorld:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_stonerGuyShowingJoint)
					_vm->_timers[4] = 0;
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 24;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS20ExitOutsideCircusWorld].x, _vm->_hotspotsWalkPos[kHS20ExitOutsideCircusWorld].y, 0, 0x107BB, 1);
				_vm->_gnap->_actionStatus = kAS20LeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS20ExitOutsideCircusWorld].x + 1, _vm->_hotspotsWalkPos[kHS20ExitOutsideCircusWorld].y, -1, 0x107C2, 1);
			}
			break;

		case kHS20StonerGuy:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHS20StonerGuy].x, _vm->_hotspotsWalkPos[kHS20StonerGuy].y, 5, 4);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapMoan2(5, 4);
						break;
					case GRAB_CURSOR:
						_vm->_gnap->_idleFacing = kDirUpRight;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS20StonerGuy].x, _vm->_hotspotsWalkPos[kHS20StonerGuy].y, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
						if (_stonerGuyShowingJoint)
							_vm->_gnap->_actionStatus = kAS20GrabJoint;
						else
							_vm->playGnapImpossible(0, 0);
						break;
					case TALK_CURSOR:
						_vm->_gnap->_idleFacing = kDirUpRight;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS20StonerGuy].x, _vm->_hotspotsWalkPos[kHS20StonerGuy].y, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						if (_vm->isFlag(kGFJointTaken))
							_vm->_gnap->_actionStatus = kAS20TalkStonerGuyNoJoint;
						else
							_vm->_gnap->_actionStatus = kAS20TalkStonerGuyHasJoint;
						break;
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS20GroceryStoreGuy:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHS20GroceryStoreGuy].x, _vm->_hotspotsWalkPos[kHS20GroceryStoreGuy].y, 2, 4);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(2, 3);
						break;
					case GRAB_CURSOR:
						_stonerGuyShowingJoint = false;
						_vm->_gnap->_idleFacing = kDirUpLeft;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS20GroceryStoreGuy].x, _vm->_hotspotsWalkPos[kHS20GroceryStoreGuy].y, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
						_vm->_gnap->_actionStatus = kAS20GrabGroceryStoreGuy;
						break;
					case TALK_CURSOR:
						_vm->_gnap->_idleFacing = kDirUpLeft;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS20GroceryStoreGuy].x, _vm->_hotspotsWalkPos[kHS20GroceryStoreGuy].y, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_vm->_gnap->_actionStatus = kAS20TalkGroceryStoreGuy;
						break;
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS20GroceryStoreHat:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemCowboyHat) {
					_vm->_gnap->_idleFacing = kDirUpRight;
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS20GroceryStoreHat].x, _vm->_hotspotsWalkPos[kHS20GroceryStoreHat].y, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
					_vm->_gnap->_actionStatus = kAS20SwitchGroceryStoreHat;
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHS20GroceryStoreHat].x, _vm->_hotspotsWalkPos[kHS20GroceryStoreHat].y, 1, 6);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(1, 6);
						break;
					case GRAB_CURSOR:
						_stonerGuyShowingJoint = false;
						_vm->_gnap->_idleFacing = kDirUpLeft;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS20GroceryStoreGuy].x, _vm->_hotspotsWalkPos[kHS20GroceryStoreGuy].y, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
						_vm->_gnap->_actionStatus = kAS20GrabGroceryStoreHat;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS20WalkArea1:
		case kHS20WalkArea2:
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
			if (_vm->_plat->_actionStatus < 0) {
				_vm->_hotspots[kHS20WalkArea1]._y2 += 48;
				_vm->updatePlatypusIdleSequence();
				_vm->_hotspots[kHS20WalkArea1]._y2 -= 48;
			}
			if (_vm->_gnap->_actionStatus < 0)
				_vm->updateGnapIdleSequence();
			if (_vm->_gnap->_actionStatus < 0 && !_vm->_timers[5] && _nextGroceryStoreGuySequenceId == -1) {
				_vm->_timers[5] = _vm->getRandom(50) + 130;
				if (_vm->getRandom(4) != 0)
					_nextGroceryStoreGuySequenceId = 0x17C;
				else
					_nextGroceryStoreGuySequenceId = 0x17A;
			}
			if (!_vm->_timers[7]) {
				_vm->_timers[7] = _vm->getRandom(100) + 100;
				if (_vm->_gnap->_actionStatus < 0 && _vm->_plat->_actionStatus < 0) {
					switch (_vm->getRandom(3)) {
					case 0:
						gameSys.insertSequence(0x183, 253, 0, 0, kSeqNone, 0, 0, 0);
						break;
					case 1:
						gameSys.insertSequence(0x184, 253, 0, 0, kSeqNone, 0, 0, 0);
						break;
					case 2:
						gameSys.insertSequence(0x185, 253, 0, 0, kSeqNone, 0, 0, 0);
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
	GameSys& gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (_vm->_gnap->_actionStatus) {
		case kAS20LeaveScene:
			_vm->_sceneDone = true;
			break;
		case kAS20TalkStonerGuyNoJoint:
			gameSys.setAnimation(0x170, 21, 2);
			gameSys.setAnimation(0x17B, 20, 3);
			gameSys.insertSequence(0x17B, 20, _currGroceryStoreGuySequenceId, 20, kSeqSyncExists, 0, 0, 0);
			gameSys.insertSequence(0x170, 21, _currStonerGuySequenceId, 21, kSeqSyncExists, 0, 0, 0);
			_vm->stopSound(0x1A1);
			stopSounds();
			_currGroceryStoreGuySequenceId = 0x17B;
			_currStonerGuySequenceId = 0x170;
			_nextGroceryStoreGuySequenceId = -1;
			_nextStonerGuySequenceId = 0x16E;
			_vm->_timers[5] = 100;
			_vm->_timers[6] = 100;
			break;
		case kAS20TalkStonerGuyHasJoint:
			gameSys.setAnimation(0x168, 21, 2);
			gameSys.setAnimation(379, 20, 3);
			gameSys.insertSequence(0x17B, 20, _currGroceryStoreGuySequenceId, 20, kSeqSyncExists, 0, 0, 0);
			gameSys.insertSequence(0x170, 21, _currStonerGuySequenceId, 21, kSeqSyncExists, 0, 0, 0);
			gameSys.insertSequence(0x168, 21, 0x170, 21, kSeqSyncWait, 0, 0, 0);
			_vm->stopSound(0x1A1);
			stopSounds();
			_currGroceryStoreGuySequenceId = 0x17B;
			_currStonerGuySequenceId = 0x168;
			_nextGroceryStoreGuySequenceId = -1;
			_nextStonerGuySequenceId = 0x16B;
			_vm->_timers[5] = 200;
			_vm->_timers[6] = 200;
			_vm->_timers[4] = 100;
			_stonerGuyShowingJoint = true;
			_vm->_gnap->_actionStatus = -1;
			break;
		case kAS20GrabJoint:
			_nextStonerGuySequenceId = 0x16A;
			break;
		case kAS20ActionDone:
			_vm->_gnap->_actionStatus = -1;
			break;
		case kAS20TalkGroceryStoreGuy:
			gameSys.setAnimation(0x170, 21, 2);
			gameSys.setAnimation(0x17B, 20, 3);
			gameSys.insertSequence(0x17B, 20, _currGroceryStoreGuySequenceId, 20, kSeqSyncExists, 0, 0, 0);
			gameSys.insertSequence(0x170, 21, _currStonerGuySequenceId, 21, kSeqSyncExists, 0, 0, 0);
			_vm->stopSound(0x1A1);
			stopSounds();
			_currGroceryStoreGuySequenceId = 0x17B;
			_currStonerGuySequenceId = 0x170;
			_groceryStoreGuyCtr = (_groceryStoreGuyCtr + 1) % 2;
			if (_groceryStoreGuyCtr != 0)
				_nextGroceryStoreGuySequenceId = 0x176;
			else
				_nextGroceryStoreGuySequenceId = 0x177;
			_vm->_timers[5] = 100;
			_vm->_timers[6] = 100;
			break;
		case kAS20GrabGroceryStoreGuy:
			gameSys.setAnimation(0x170, 21, 2);
			gameSys.setAnimation(0x17B, 20, 3);
			gameSys.insertSequence(0x170, 21, _currStonerGuySequenceId, 21, kSeqSyncExists, 0, 0, 0);
			gameSys.insertSequence(0x17B, 20, _currGroceryStoreGuySequenceId, 20, kSeqSyncExists, 0, 0, 0);
			_vm->stopSound(0x1A1);
			stopSounds();
			_currGroceryStoreGuySequenceId = 0x17B;
			_currStonerGuySequenceId = 0x170;
			_vm->_timers[5] = 120;
			_vm->_timers[6] = 120;
			_nextGroceryStoreGuySequenceId = 0x178;
			break;
		case kAS20GrabGroceryStoreHat:
			gameSys.setAnimation(0x170, 21, 2);
			gameSys.setAnimation(0x17B, 20, 3);
			gameSys.insertSequence(0x17B, 20, _currGroceryStoreGuySequenceId, 20, kSeqSyncExists, 0, 0, 0);
			gameSys.insertSequence(0x170, 21, _currStonerGuySequenceId, 21, kSeqSyncExists, 0, 0, 0);
			_vm->stopSound(0x1A1);
			stopSounds();
			_currGroceryStoreGuySequenceId = 0x17B;
			_currStonerGuySequenceId = 0x170;
			_nextGroceryStoreGuySequenceId = 0x179;
			break;
		case kAS20SwitchGroceryStoreHat:
			_vm->setGrabCursorSprite(-1);
			gameSys.setAnimation(0x180, _vm->_gnapId, 0);
			gameSys.insertSequence(0x180, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceId = 0x180;
			_vm->_gnapSequenceDatNum = 0;
			_vm->invRemove(kItemCowboyHat);
			_vm->invAdd(kItemGroceryStoreHat);
			_vm->_gnap->_actionStatus = kAS20SwitchGroceryStoreHatDone;
			break;
		case kAS20SwitchGroceryStoreHatDone:
			gameSys.insertSequence(0x17F, 20, 372, 20, kSeqSyncWait, 0, 0, 0);
			_vm->setFlag(kGFGroceryStoreHatTaken);
			_vm->hideCursor();
			_vm->setGrabCursorSprite(-1);
			_vm->addFullScreenSprite(0x12C, 255);
			gameSys.setAnimation(0x181, 256, 0);
			gameSys.insertSequence(0x181, 256, 0, 0, kSeqNone, 0, 0, 0);
			while (gameSys.getAnimationStatus(0) != 2)
				_vm->gameUpdateTick();
			_vm->removeFullScreenSprite();
			_vm->showCursor();
			_vm->setGrabCursorSprite(kItemGroceryStoreHat);
			_vm->_gnap->_idleFacing = kDirBottomRight;
			_vm->gnapWalkTo(3, 8, -1, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
			_vm->_gnap->_actionStatus = -1;
			break;
		case kAS20GrabJointDone:
			_vm->setGrabCursorSprite(kItemJoint);
			_vm->_gnap->_actionStatus = -1;
			break;
		}
	}

	if (gameSys.getAnimationStatus(3) == 2) {
		switch (_nextGroceryStoreGuySequenceId) {
		case 0x176:
		case 0x177:
			gameSys.setAnimation(_nextGroceryStoreGuySequenceId, 20, 3);
			gameSys.insertSequence(_nextGroceryStoreGuySequenceId, 20, _currGroceryStoreGuySequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_currGroceryStoreGuySequenceId = _nextGroceryStoreGuySequenceId;
			_nextGroceryStoreGuySequenceId = -1;
			_nextStonerGuySequenceId = 0x16D;
			break;
		case 0x178:
			gameSys.setAnimation(_nextGroceryStoreGuySequenceId, 20, 3);
			gameSys.setAnimation(0x17D, _vm->_gnapId, 0);
			gameSys.insertSequence(_nextGroceryStoreGuySequenceId, 20, _currGroceryStoreGuySequenceId, 20, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0x17D, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceId = 0x17D;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnap->_actionStatus = kAS20ActionDone;
			gameSys.setAnimation(0x16D, 21, 2);
			gameSys.insertSequence(0x16D, 21, _currStonerGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
			_currStonerGuySequenceId = 0x16D;
			_currGroceryStoreGuySequenceId = 0x178;
			_nextGroceryStoreGuySequenceId = -1;
			_nextStonerGuySequenceId = -1;
			break;
		case 0x179:
			gameSys.setAnimation(_nextGroceryStoreGuySequenceId, 20, 3);
			gameSys.setAnimation(0x16D, 21, 0);
			gameSys.insertSequence(_nextGroceryStoreGuySequenceId, 20, _currGroceryStoreGuySequenceId, 20, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0x17E, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceId = 0x17E;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnap->_actionStatus = kAS20ActionDone;
			gameSys.setAnimation(0x16D, 21, 2);
			gameSys.insertSequence(0x16D, 21, _currStonerGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
			_currStonerGuySequenceId = 0x16D;
			_currGroceryStoreGuySequenceId = 377;
			_nextGroceryStoreGuySequenceId = -1;
			_nextStonerGuySequenceId = -1;
			_vm->gnapWalkTo(4, 8, -1, 0x107BB, 1);
			break;
		case 0x17C:
			gameSys.setAnimation(0, 0, 3);
			_nextStonerGuySequenceId = 0x171;
			break;
		case 0x17A:
			gameSys.setAnimation(0, 0, 3);
			_nextStonerGuySequenceId = 0x16F;
			break;
		case 0x175:
			gameSys.setAnimation(0x175, 20, 0);
			gameSys.setAnimation(0x175, 20, 3);
			gameSys.insertSequence(0x175, 20, _currGroceryStoreGuySequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_currGroceryStoreGuySequenceId = 0x175;
			_nextGroceryStoreGuySequenceId = -1;
			_vm->_gnap->_actionStatus = kAS20ActionDone;
			break;
		default:
			if (_nextGroceryStoreGuySequenceId != -1) {
				gameSys.setAnimation(_nextGroceryStoreGuySequenceId, 20, 3);
				gameSys.insertSequence(_nextGroceryStoreGuySequenceId, 20, _currGroceryStoreGuySequenceId, 20, kSeqSyncWait, 0, 0, 0);
				_currGroceryStoreGuySequenceId = _nextGroceryStoreGuySequenceId;
				_nextGroceryStoreGuySequenceId = -1;
			}
			break;
		}
	}

	updateAnimationsCb();
}

/*****************************************************************************/

Scene21::Scene21(GnapEngine *vm) : Scene(vm) {
	_currOldLadySequenceId = -1;
	_nextOldLadySequenceId = -1;
}

int Scene21::init() {
	_vm->_gameSys->setAnimation(0, 0, 3);

	return _vm->isFlag(kGFTwigTaken) ? 0x94 : 0x93;
}

void Scene21::updateHotspots() {
	_vm->setHotspot(kHS21Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS21Banana, 94, 394, 146, 430, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 2, 6);
	_vm->setHotspot(kHS21OldLady, 402, 220, 528, 430, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 4, 7);
	_vm->setHotspot(kHS21ExitOutsideGrubCity, 522, 498, 800, 600, SF_EXIT_SE_CURSOR | SF_WALKABLE, 5, 10);
	_vm->setHotspot(kHS21WalkArea1, 0, 0, 800, 440);
	_vm->setHotspot(kHS21WalkArea2, 698, 0, 800, 600);
	_vm->setDeviceHotspot(kHS21Device, -1, -1, -1, -1);
	if (_vm->isFlag(kGFUnk04) || !_vm->isFlag(kGFTwigTaken))
		_vm->_hotspots[kHS21Banana]._flags = SF_WALKABLE | SF_DISABLED;
	if (_vm->isFlag(kGFTwigTaken))
		_vm->_hotspots[kHS21OldLady]._flags = SF_DISABLED;
	_vm->_hotspotsCount = 7;
}

void Scene21::run() {
	GameSys& gameSys = *_vm->_gameSys;

	_vm->playSound(0x10940, true);
	_vm->startSoundTimerA(6);
	_vm->_timers[5] = _vm->getRandom(100) + 100;
	_vm->queueInsertDeviceIcon();

	if (_vm->isFlag(kGFTwigTaken)) {
		if (_vm->isFlag(kGFKeysTaken)) {
			_vm->initGnapPos(5, 8, kDirBottomRight);
			_vm->initPlatypusPos(6, 8, kDirNone);
			gameSys.insertSequence(0x8E, 2, 0, 0, kSeqNone, 0, 0, 0);
			if (!_vm->isFlag(kGFUnk04))
				gameSys.insertSequence(0x8D, 59, 0, 0, kSeqNone, 0, 0, 0);
			_vm->endSceneInit();
			_vm->clearFlag(kGFKeysTaken);
		} else {
			_vm->initGnapPos(5, 11, kDirBottomRight);
			_vm->initPlatypusPos(6, 11, kDirNone);
			if (!_vm->isFlag(kGFUnk04))
				gameSys.insertSequence(0x8D, 59, 0, 0, kSeqNone, 0, 0, 0);
			_vm->endSceneInit();
			_vm->gnapWalkTo(5, 8, -1, 0x107B9, 1);
			_vm->platypusWalkTo(6, 8, -1, 0x107C2, 1);
		}
	} else {
		_vm->initGnapPos(5, 11, kDirBottomRight);
		_vm->initPlatypusPos(6, 11, kDirNone);
		_currOldLadySequenceId = 0x89;
		gameSys.setAnimation(0x89, 79, 3);
		gameSys.insertSequence(_currOldLadySequenceId, 79, 0, 0, kSeqNone, 0, 0, 0);
		_nextOldLadySequenceId = -1;
		_vm->_timers[4] = _vm->getRandom(30) + 50;
		_vm->endSceneInit();
		_vm->gnapWalkTo(5, 8, -1, 0x107B9, 1);
		_vm->platypusWalkTo(6, 8, -1, 0x107C2, 1);
	}

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS21Device:
			if (_vm->_gnap->_actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS21Platypus:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemJoint) {
					_vm->gnapUseJointOnPlatypus();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapImpossible(0, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(_vm->_plat->_pos.x, _vm->_plat->_pos.y);
						break;
					case GRAB_CURSOR:
						_vm->gnapKissPlatypus(0);
						break;
					case TALK_CURSOR:
						_vm->playGnapBrainPulsating(_vm->_plat->_pos.x, _vm->_plat->_pos.y);
						_vm->playPlatypusSequence(_vm->getPlatypusSequenceId());
						break;
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS21Banana:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, 2, 5);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(2, 5);
						break;
					case GRAB_CURSOR:
						_vm->gnapWalkTo(_vm->_gnap->_pos, 0, _vm->getGnapSequenceId(gskIdle, _vm->_hotspotsWalkPos[kHS21Banana].x, _vm->_hotspotsWalkPos[kHS21Banana].y) | 0x10000, 1);
						_vm->playGnapPullOutDevice(2, 5);
						_vm->playGnapUseDevice(0, 0);
						_vm->_gnap->_actionStatus = kAS21GrabBanana;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						break;
					}
				}
			}
			break;

		case kHS21OldLady:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemGroceryStoreHat) {
					_vm->_newSceneNum = 47;
					_vm->gnapWalkTo(4, 6, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
					_vm->_gnap->_actionStatus = kAS21UseHatWithOldLady;
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(4, 6, 7, 4);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(7, 4);
						break;
					case GRAB_CURSOR:
						_vm->_gnap->_idleFacing = kDirUpLeft;
						_vm->_hotspots[kHS21WalkArea1]._flags |= SF_WALKABLE;
						_vm->gnapWalkTo(7, 6, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
						_vm->_gnap->_actionStatus = kAS21GrabOldLady;
						_vm->_hotspots[kHS21WalkArea1]._flags &= ~SF_WALKABLE;
						break;
					case TALK_CURSOR:
						_vm->_gnap->_idleFacing = kDirUpRight;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS21OldLady].x, _vm->_hotspotsWalkPos[kHS21OldLady].y, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_vm->_gnap->_actionStatus = kAS21TalkOldLady;
						break;
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS21ExitOutsideGrubCity:
			if (_vm->_gnap->_actionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 20;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS21ExitOutsideGrubCity].x, _vm->_hotspotsWalkPos[kHS21ExitOutsideGrubCity].y, 0, 0x107B3, 1);
				_vm->_gnap->_actionStatus = kAS21LeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS21ExitOutsideGrubCity].x + 1, _vm->_hotspotsWalkPos[kHS21ExitOutsideGrubCity].y, -1, 0x107C2, 1);
			}
			break;

		case kHS21WalkArea1:
		case kHS21WalkArea2:
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
			_vm->updatePlatypusIdleSequence();
			_vm->updateGnapIdleSequence();
			if (!_vm->isFlag(kGFTwigTaken) && !_vm->_timers[4] && _nextOldLadySequenceId == -1 && _vm->_gnap->_actionStatus == -1) {
				_vm->_timers[4] = _vm->getRandom(30) + 50;
				switch (_vm->getRandom(5)) {
				case 0:
					_nextOldLadySequenceId = 0x88;
					break;
				case 1:
					_nextOldLadySequenceId = 0x8A;
					break;
				default:
					_nextOldLadySequenceId = 0x89;
					break;
				}
			}
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(100) + 100;
				gameSys.insertSequence(0x92, 255, 0, 0, kSeqNone, 0, 0, 0);
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

void Scene21::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (_vm->_gnap->_actionStatus) {
		case kAS21TalkOldLady:
			_nextOldLadySequenceId = 0x8B;
			_vm->_gnap->_actionStatus = -1;
			break;
		case kAS21GrabBanana:
			gameSys.setAnimation(0x8C, 59, 0);
			gameSys.insertSequence(0x8C, 59, 141, 59, kSeqSyncWait, 0, 0, 0);
			_vm->setFlag(kGFUnk04);
			_vm->invAdd(kItemBanana);
			updateHotspots();
			_vm->_gnap->_actionStatus = kAS21GrabBananaDone;
			break;
		case kAS21GrabBananaDone:
			_vm->setGrabCursorSprite(kItemBanana);
			_vm->_gnap->_actionStatus = -1;
			break;
		case kAS21GrabOldLady:
			_vm->_timers[4] = _vm->getRandom(30) + 50;
			_nextOldLadySequenceId = 0x87;
			break;
		case kAS21UseHatWithOldLady:
			gameSys.setAnimation(0x8F, _vm->_gnapId, 0);
			gameSys.insertSequence(0x8F, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapSequenceId = 0x8F;
			_vm->_gnap->_actionStatus = kAS21UseHatWithOldLadyDone;
			_vm->invAdd(kItemTickets);
			_vm->invRemove(kItemGroceryStoreHat);
			_vm->setGrabCursorSprite(-1);
			break;
		case kAS21UseHatWithOldLadyDone:
			_nextOldLadySequenceId = 0x91;
			break;
		case kAS21LeaveScene:
			_vm->_sceneDone = true;
			break;
		}
	}

	if (gameSys.getAnimationStatus(3) == 2 && _nextOldLadySequenceId != -1) {
		if (_nextOldLadySequenceId == 0x87) {
			gameSys.setAnimation(_nextOldLadySequenceId, 79, 3);
			gameSys.insertSequence(_nextOldLadySequenceId, 79, _currOldLadySequenceId, 79, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0x86, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceId = 0x86;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnap->_actionStatus = -1;
			_currOldLadySequenceId = _nextOldLadySequenceId;
			_nextOldLadySequenceId = -1;
		} else if (_nextOldLadySequenceId == 0x91) {
			gameSys.setAnimation(0x91, 79, 0);
			gameSys.insertSequence(_nextOldLadySequenceId, 79, _currOldLadySequenceId, 79, kSeqSyncWait, 0, 0, 0);
			_vm->_gnap->_actionStatus = kAS21LeaveScene;
			_currOldLadySequenceId = _nextOldLadySequenceId;
			_nextOldLadySequenceId = -1;
		} else {
			gameSys.setAnimation(_nextOldLadySequenceId, 79, 3);
			gameSys.insertSequence(_nextOldLadySequenceId, 79, _currOldLadySequenceId, 79, kSeqSyncWait, 0, 0, 0);
			_currOldLadySequenceId = _nextOldLadySequenceId;
			_nextOldLadySequenceId = -1;
		}
	}
}

/*****************************************************************************/

Scene22::Scene22(GnapEngine *vm) : Scene(vm) {
	_caughtBefore = false;
	_cashierCtr = 3;
}

int Scene22::init() {
	return 0x5E;
}

void Scene22::updateHotspots() {
	_vm->setHotspot(kHS22Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS22ExitOutsideGrubCity, 0, 180, 184, 472, SF_EXIT_L_CURSOR, 3, 6);
	_vm->setHotspot(kHS22ExitBackGrubCity, 785, 405, 800, 585, SF_EXIT_R_CURSOR, 11, 9);
	_vm->setHotspot(kHS22Cashier, 578, 230, 660, 376, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 8);
	_vm->setHotspot(kHS22WalkArea1, 553, 0, 800, 542);
	_vm->setHotspot(kHS22WalkArea2, 0, 0, 552, 488);
	_vm->setDeviceHotspot(kHS22Device, -1, -1, -1, -1);
	_vm->_hotspotsCount = 7;
}

void Scene22::run() {
	GameSys& gameSys = *_vm->_gameSys;

	gameSys.insertSequence(0x5D, 254, 0, 0, kSeqNone, 0, 0, 0);

	_currCashierSequenceId = 0x59;
	_nextCashierSequenceId = -1;

	gameSys.setAnimation(0x59, 1, 3);
	gameSys.insertSequence(_currCashierSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);

	_vm->_timers[6] = _vm->getRandom(30) + 20;

	_vm->queueInsertDeviceIcon();

	if (_vm->_prevSceneNum == 20) {
		_vm->initGnapPos(2, 8, kDirBottomRight);
		_vm->initPlatypusPos(1, 8, kDirNone);
		_vm->endSceneInit();
	} else {
		_vm->initGnapPos(11, _vm->_hotspotsWalkPos[kHS22ExitBackGrubCity].y, kDirBottomRight);
		_vm->initPlatypusPos(11, _vm->_hotspotsWalkPos[kHS22ExitBackGrubCity].y + 1, kDirNone);
		_vm->endSceneInit();
		_vm->gnapWalkTo(8, 8, -1, 0x107B9, 1);
		_vm->platypusWalkTo(9, 8, -1, 0x107C2, 1);
	}

	if (_vm->isFlag(kGFSceneFlag1)) {
		int storeDetectiveSeqId;
		_vm->setGrabCursorSprite(-1);
		_vm->invRemove(kItemCereals);
		if (_caughtBefore) {
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
			_caughtBefore = true;
			storeDetectiveSeqId = 0x54;
		}
		gameSys.waitForUpdate();
		gameSys.requestClear1();
		gameSys.drawSpriteToBackground(0, 0, 0x44);
		gameSys.setAnimation(storeDetectiveSeqId, 256, 4);
		gameSys.insertSequence(storeDetectiveSeqId, 256, 0, 0, kSeqNone, 0, 0, 0);
		while (gameSys.getAnimationStatus(4) != 2) {
			_vm->gameUpdateTick();
		}
		_vm->_sceneDone = true;
		_vm->_newSceneNum = 20;
		_caughtBefore = true;
	}

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->testWalk(0, 0, -1, -1, -1, -1);

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS22Device:
			if (_vm->_gnap->_actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS22Platypus:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemJoint) {
					_vm->gnapUseJointOnPlatypus();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapImpossible(0, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(_vm->_plat->_pos.x, _vm->_plat->_pos.y);
						break;
					case GRAB_CURSOR:
						_vm->gnapKissPlatypus(0);
						break;
					case TALK_CURSOR:
						_vm->playGnapBrainPulsating(_vm->_plat->_pos.x, _vm->_plat->_pos.y);
						_vm->playPlatypusSequence(_vm->getPlatypusSequenceId());
						break;
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS22ExitOutsideGrubCity:
			if (_vm->_gnap->_actionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 20;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS22ExitOutsideGrubCity].x, _vm->_hotspotsWalkPos[kHS22ExitOutsideGrubCity].y, 0, 0x107AF, 1);
				_vm->_gnap->_actionStatus = kAS22LeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS22ExitOutsideGrubCity].x, _vm->_hotspotsWalkPos[kHS22ExitOutsideGrubCity].y + 1, -1, 0x107C2, 1);
			}
			break;

		case kHS22ExitBackGrubCity:
			if (_vm->_gnap->_actionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 23;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS22ExitBackGrubCity].x, _vm->_hotspotsWalkPos[kHS22ExitBackGrubCity].y, 0, 0x107AB, 1);
				_vm->_gnap->_actionStatus = kAS22LeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS22ExitBackGrubCity].x, _vm->_hotspotsWalkPos[kHS22ExitBackGrubCity].y + 1, -1, 0x107C2, 1);
			}
			break;

		case kHS22Cashier:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHS22Cashier].x, _vm->_hotspotsWalkPos[kHS22Cashier].y, 8, 4);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(8, 4);
						break;
					case GRAB_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					case TALK_CURSOR:
						_vm->_gnap->_idleFacing = kDirUpRight;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS22Cashier].x, _vm->_hotspotsWalkPos[kHS22Cashier].y,
							0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_vm->_gnap->_actionStatus = kAS22TalkCashier;
						break;
					case PLAT_CURSOR:
						_vm->gnapUseDeviceOnPlatypus();
						break;
					}
				}
			}
			break;

		case kHS22WalkArea1:
		case kHS22WalkArea2:
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
			if (!_vm->_timers[6] && _nextCashierSequenceId == -1) {
				_vm->_timers[6] = _vm->getRandom(30) + 20;
				if (_vm->getRandom(8) != 0) {
					_nextCashierSequenceId = 0x59;
				} else {
					_cashierCtr = (_cashierCtr + 1) % 3;
					switch (_cashierCtr) {
					case 1:
						_nextCashierSequenceId = 0x58;
						break;
					case 2:
						_nextCashierSequenceId = 0x5A;
						break;
					case 3:
						_nextCashierSequenceId = 0x5B;
						break;
					default:
						_nextCashierSequenceId = 0x58;
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
	GameSys& gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (_vm->_gnap->_actionStatus) {
		case kAS22LeaveScene:
			_vm->_sceneDone = true;
			break;
		case kAS22TalkCashier:
			_nextCashierSequenceId = 0x5C;
			break;
		}
		_vm->_gnap->_actionStatus = -1;
	}

	if (gameSys.getAnimationStatus(3) == 2 && _nextCashierSequenceId != -1) {
		gameSys.setAnimation(_nextCashierSequenceId, 1, 3);
		gameSys.insertSequence(_nextCashierSequenceId, 1, _currCashierSequenceId, 1, kSeqSyncWait, 0, 0, 0);
		_currCashierSequenceId = _nextCashierSequenceId;
		_nextCashierSequenceId = -1;
	}
}

/*****************************************************************************/

Scene23::Scene23(GnapEngine *vm) : Scene(vm) {
	_currStoreClerkSequenceId = -1;
	_nextStoreClerkSequenceId = -1;
}

int Scene23::init() {
	return 0xC0;
}

void Scene23::updateHotspots() {
	_vm->setHotspot(kHS23Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS23ExitFrontGrubCity, 0, 250, 15, 550, SF_EXIT_L_CURSOR | SF_WALKABLE, 0, 7);
	_vm->setHotspot(kHS23Cereals, 366, 332, 414, 408, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 4, 7);
	_vm->setHotspot(kHS23WalkArea1, 0, 0, 340, 460);
	_vm->setHotspot(kHS23WalkArea2, 340, 0, 800, 501);
	_vm->setDeviceHotspot(kHS23Device, -1, -1, -1, -1);
	_vm->_hotspotsCount = 6;
}

void Scene23::run() {
	GameSys& gameSys = *_vm->_gameSys;

	_vm->_timers[4] = _vm->getRandom(100) + 200;
	_vm->_timers[5] = _vm->getRandom(100) + 200;

	_currStoreClerkSequenceId = 0xB4;
	_nextStoreClerkSequenceId = -1;

	gameSys.setAnimation(0xB4, 1, 4);
	gameSys.insertSequence(_currStoreClerkSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);

	_vm->queueInsertDeviceIcon();

	_vm->initGnapPos(-1, 7, kDirBottomRight);
	_vm->initPlatypusPos(-2, 7, kDirNone);
	gameSys.insertSequence(0xBD, 255, 0, 0, kSeqNone, 0, 0, 0);
	gameSys.insertSequence(0xBF, 2, 0, 0, kSeqNone, 0, 0, 0);
	_vm->endSceneInit();

	_vm->platypusWalkTo(1, 7, -1, 0x107C2, 1);

	if (_vm->isFlag(kGFUnk24)) {
		_vm->gnapWalkTo(2, 7, -1, 0x107B9, 1);
	} else {
		_vm->gnapWalkTo(2, 7, 0, 0x107B9, 1);
		while (gameSys.getAnimationStatus(0) != 2)
			_vm->gameUpdateTick();
		_vm->playSequences(0x48, 0xBA, 0xBB, 0xBC);
		_vm->setFlag(kGFUnk24);
	}

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->testWalk(0, 3, -1, -1, -1, -1);

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS23Device:
			if (_vm->_gnap->_actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS23Platypus:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemJoint) {
					_vm->gnapUseJointOnPlatypus();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapImpossible(0, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(_vm->_plat->_pos.x, _vm->_plat->_pos.y);
						break;
					case GRAB_CURSOR:
						_vm->gnapKissPlatypus(0);
						break;
					case TALK_CURSOR:
						_vm->playGnapBrainPulsating(_vm->_plat->_pos.x, _vm->_plat->_pos.y);
						_vm->playPlatypusSequence(_vm->getPlatypusSequenceId());
						break;
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS23Cereals:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHS23Cereals].x, _vm->_hotspotsWalkPos[kHS23Cereals].y, 5, 4);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						if (_vm->isFlag(kGFSceneFlag1))
							_vm->playGnapMoan2(0, 0);
						else {
							_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS23Cereals].x, _vm->_hotspotsWalkPos[kHS23Cereals].y,
								0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
							_vm->_gnap->_actionStatus = kAS23LookCereals;
						}
						break;
					case GRAB_CURSOR:
						if (_vm->isFlag(kGFSceneFlag1))
							_vm->playGnapImpossible(0, 0);
						else {
							_vm->_gnap->_idleFacing = kDirBottomRight;
							_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS23Cereals].x, _vm->_hotspotsWalkPos[kHS23Cereals].y,
								0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
							_vm->setFlag(kGFSceneFlag1);
							_vm->_gnap->_actionStatus = kAS23GrabCereals;
							_vm->invAdd(kItemCereals);
						}
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS23ExitFrontGrubCity:
			if (_vm->_gnap->_actionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 22;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS23ExitFrontGrubCity].x, _vm->_hotspotsWalkPos[kHS23ExitFrontGrubCity].y, 0, 0x107AF, 1);
				_vm->_gnap->_actionStatus = kAS23LeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS23ExitFrontGrubCity].x, _vm->_hotspotsWalkPos[kHS23ExitFrontGrubCity].y - 1, -1, 0x107C2, 1);
			}
			break;

		case kHS23WalkArea1:
		case kHS23WalkArea2:
			if (_vm->_gnap->_actionStatus < 0)
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
			if (!_vm->_timers[4] && _vm->_gnap->_actionStatus == -1) {
				_vm->_timers[4] = _vm->getRandom(100) + 200;
				switch (_vm->getRandom(4)) {
				case 0:
					gameSys.insertSequence(0xB7, 256, 0, 0, kSeqNone, 0, 0, 0);
					break;
				case 1:
					gameSys.insertSequence(0xB8, 256, 0, 0, kSeqNone, 0, 0, 0);
					break;
				case 2:
				case 3:
					gameSys.insertSequence(0xB9, 256, 0, 0, kSeqNone, 0, 0, 0);
					break;
				}
			}
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(100) + 200;
				switch (_vm->getRandom(3)) {
				case 0:
					_vm->playSound(0xCE, false);
					break;
				case 1:
					_vm->playSound(0xD0, false);
					break;
				case 2:
					_vm->playSound(0xCF, false);
					break;
				}
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

void Scene23::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (_vm->_gnap->_actionStatus) {
		case kAS23LookCereals:
			_vm->showFullScreenSprite(0x48);
			_vm->_gnap->_actionStatus = -1;
			break;
		case kAS23GrabCereals:
			gameSys.setAnimation(0xBE, _vm->_gnapId, 0);
			gameSys.insertSequence(0xBE, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			gameSys.requestRemoveSequence(0xBF, 2);
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapSequenceId = 0xBE;
			_vm->_gnap->_actionStatus = kAS23GrabCerealsDone;
			break;
		case kAS23GrabCerealsDone:
			_vm->setGrabCursorSprite(kItemCereals);
			_vm->_gnap->_actionStatus = -1;
			break;
		case kAS23LeaveScene:
			_vm->_sceneDone = true;
			break;
		}
	}

	if (gameSys.getAnimationStatus(4) == 2 && _nextStoreClerkSequenceId == -1) {
		switch (_vm->getRandom(8)) {
		case 0:
		case 1:
		case 2:
			_nextStoreClerkSequenceId = 0xB4;
			break;
		case 3:
		case 4:
		case 5:
			_nextStoreClerkSequenceId = 0xB5;
			break;
		default:
			_nextStoreClerkSequenceId = 0xB6;
			break;
		}
		gameSys.setAnimation(_nextStoreClerkSequenceId, 1, 4);
		gameSys.insertSequence(_nextStoreClerkSequenceId, 1, _currStoreClerkSequenceId, 1, kSeqSyncWait, 0, 0, 0);
		_currStoreClerkSequenceId = _nextStoreClerkSequenceId;
		_nextStoreClerkSequenceId = -1;
	}
}

/*****************************************************************************/

Scene24::Scene24(GnapEngine *vm) : Scene(vm) {
	_currWomanSequenceId = -1;
	_nextWomanSequenceId = -1;
	_boySequenceId = -1;
	_girlSequenceId = -1;
}

int Scene24::init() {
	return 0x3B;
}

void Scene24::updateHotspots() {
	_vm->setHotspot(kHS24Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS24ExitCircusWorld, 785, 128, 800, 600, SF_EXIT_R_CURSOR, 8, 7);
	_vm->setHotspot(kHS24ExitOutsideGrubCity, 0, 213, 91, 600, SF_EXIT_NW_CURSOR, 1, 8);
	_vm->setHotspot(kHS24WalkArea1, 0, 0, 0, 0);
	_vm->setHotspot(kHS24WalkArea2, 530, 0, 800, 600);
	_vm->setHotspot(kHS24WalkArea3, 0, 0, 800, 517);
	_vm->setDeviceHotspot(kHS24Device, -1, -1, -1, -1);
	_vm->_hotspotsCount = 7;
}

void Scene24::run() {
	GameSys& gameSys = *_vm->_gameSys;
	int counter = 0;

	_vm->playSound(0x10940, true);
	_vm->startSoundTimerA(9);

	_vm->_timers[7] = _vm->getRandom(100) + 100;

	gameSys.insertSequence(0x2F, 256, 0, 0, kSeqNone, 0, 0, 0);

	_vm->_timers[4] = _vm->getRandom(20) + 50;
	_vm->_timers[5] = _vm->getRandom(20) + 40;
	_vm->_timers[6] = _vm->getRandom(50) + 30;

	gameSys.insertSequence(0x36, 20, 0, 0, kSeqNone, 0, 0, 0);
	gameSys.insertSequence(0x30, 20, 0, 0, kSeqNone, 0, 0, 0);
	gameSys.insertSequence(0x35, 20, 0, 0, kSeqNone, 0, 0, 0);

	_currWomanSequenceId = 0x35;
	_girlSequenceId = 0x36;
	_boySequenceId = 0x30;

	if (_vm->_debugLevel == 4)
		_vm->startIdleTimer(8);

	_vm->queueInsertDeviceIcon();

	if (_vm->_prevSceneNum == 20) {
		_vm->initGnapPos(1, 8, kDirBottomRight);
		_vm->initPlatypusPos(2, 8, kDirNone);
		_vm->endSceneInit();
		_vm->gnapWalkTo(1, 9, -1, 0x107B9, 1);
		_vm->platypusWalkTo(2, 9, -1, 0x107C2, 1);
	} else {
		_vm->initGnapPos(8, 8, kDirBottomLeft);
		_vm->initPlatypusPos(8, 8, kDirUnk4);
		_vm->endSceneInit();
		_vm->gnapWalkTo(2, 8, -1, 0x107BA, 1);
		_vm->platypusWalkTo(3, 8, -1, 0x107C2, 1);
	}

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->testWalk(0, 0, -1, -1, -1, -1);

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {

		case kHS24Device:
			if (_vm->_gnap->_actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS24Platypus:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemJoint) {
					_vm->gnapUseJointOnPlatypus();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapImpossible(0, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(_vm->_plat->_pos.x, _vm->_plat->_pos.y);
						break;
					case GRAB_CURSOR:
						_vm->gnapKissPlatypus(0);
						break;
					case TALK_CURSOR:
						_vm->playGnapBrainPulsating(_vm->_plat->_pos.x, _vm->_plat->_pos.y);
						_vm->playPlatypusSequence(_vm->getPlatypusSequenceId());
						break;
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS24ExitCircusWorld:
			if (_vm->_gnap->_actionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 25;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS24ExitCircusWorld].x, _vm->_hotspotsWalkPos[kHS24ExitCircusWorld].y, 0, 0x107AB, 1);
				_vm->_gnap->_actionStatus = kAS24LeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS24ExitCircusWorld].x + 1, _vm->_hotspotsWalkPos[kHS24ExitCircusWorld].y, -1, 0x107C2, 1);
			}
			break;

		case kHS24ExitOutsideGrubCity:
			if (_vm->_gnap->_actionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 20;
				_vm->_gnap->_idleFacing = kDirUpRight;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS24ExitOutsideGrubCity].x, _vm->_hotspotsWalkPos[kHS24ExitOutsideGrubCity].y, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
				_vm->_gnap->_actionStatus = kAS24LeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS24ExitOutsideGrubCity].x + 1, _vm->_hotspotsWalkPos[kHS24ExitOutsideGrubCity].y, -1, 0x107C2, 1);
			}
			break;

		case kHS24WalkArea1:
		case kHS24WalkArea2:
		case kHS24WalkArea3:
			if (_vm->_gnap->_actionStatus == -1)
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
			_vm->updatePlatypusIdleSequence();
			_vm->updateGnapIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(20) + 50;
				gameSys.insertSequence(0x37, 20, _girlSequenceId, 20, kSeqSyncWait, 0, 0, 0);
				_girlSequenceId = 0x37;
			}
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(20) + 40;
				gameSys.insertSequence(0x31, 20, _boySequenceId, 20, kSeqSyncWait, 0, 0, 0);
				_boySequenceId = 0x31;
			}
			if (!_vm->_timers[6]) {
				_vm->_timers[6] = _vm->getRandom(50) + 30;
				counter = (counter + 1) % 3;
				switch (counter) {
				case 0:
					_nextWomanSequenceId = 0x32;
					break;
				case 1:
					_nextWomanSequenceId = 0x33;
					break;
				case 2:
					_nextWomanSequenceId = 0x34;
					break;
				}
				gameSys.insertSequence(_nextWomanSequenceId, 20, _currWomanSequenceId, 20, kSeqSyncWait, 0, 0, 0);
				_currWomanSequenceId = _nextWomanSequenceId;
			}
			if (!_vm->_timers[7]) {
				_vm->_timers[7] = _vm->getRandom(100) + 100;
				switch (_vm->getRandom(3)) {
				case 0:
					gameSys.insertSequence(0x38, 253, 0, 0, kSeqNone, 0, 0, 0);
					break;
				case 1:
					gameSys.insertSequence(0x39, 253, 0, 0, kSeqNone, 0, 0, 0);
					break;
				case 2:
					gameSys.insertSequence(0x3A, 253, 0, 0, kSeqNone, 0, 0, 0);
					break;
				}
			}
			if (_vm->_debugLevel == 4)
				_vm->updateIdleTimer();
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

void Scene24::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		if (_vm->_gnap->_actionStatus == kAS24LeaveScene)
			_vm->_sceneDone = true;
		_vm->_gnap->_actionStatus = -1;
	}
}

/*****************************************************************************/

Scene25::Scene25(GnapEngine *vm) : Scene(vm) {
	_currTicketVendorSequenceId = -1;
	_nextTicketVendorSequenceId = -1;
}

int Scene25::init() {
	return 0x62;
}

void Scene25::updateHotspots() {
	_vm->setHotspot(kHS25Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS25TicketVendor, 416, 94, 574, 324, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 5, 5);
	_vm->setHotspot(kHS25ExitOutsideCircusWorld, 0, 519, 205, 600, SF_EXIT_SW_CURSOR, 5, 10);
	_vm->setHotspot(kHS25ExitInsideCircusWorld, 321, 70, 388, 350, SF_EXIT_NE_CURSOR, 3, 6);
	_vm->setHotspot(kHS25Posters1, 0, 170, 106, 326, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 1, 7);
	_vm->setHotspot(kHS25Posters2, 146, 192, 254, 306, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 7);
	_vm->setHotspot(kHS25Posters3, 606, 162, 654, 368, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 7, 7);
	_vm->setHotspot(kHS25Posters4, 708, 114, 754, 490, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 7, 8);
	_vm->setHotspot(kHS25WalkArea1, 0, 0, 800, 439);
	_vm->setHotspot(kHS25WalkArea2, 585, 0, 800, 600);
	_vm->setDeviceHotspot(kHS25Device, -1, -1, -1, -1);
	_vm->_hotspotsCount = 11;
}

void Scene25::playAnims(int index) {
	if (index > 4)
		return;

	GameSys& gameSys = *_vm->_gameSys;

	_vm->hideCursor();
	_vm->setGrabCursorSprite(-1);
	switch (index) {
	case 1:
		_vm->_largeSprite = gameSys.createSurface(0x25);
		break;
	case 2:
		_vm->_largeSprite = gameSys.createSurface(0x26);
		break;
	case 3:
		_vm->_largeSprite = gameSys.createSurface(0x27);
		break;
	case 4:
		_vm->_largeSprite = gameSys.createSurface(0x28);
		break;
	}
	gameSys.insertSpriteDrawItem(_vm->_largeSprite, 0, 0, 300);
	_vm->delayTicksCursor(5);
	while (!_vm->_mouseClickState._left && !_vm->isKeyStatus1(Common::KEYCODE_ESCAPE) && !_vm->isKeyStatus1(Common::KEYCODE_SPACE) &&
		!_vm->isKeyStatus1(29)) {
		_vm->gameUpdateTick();
	}
	_vm->_mouseClickState._left = false;
	_vm->clearKeyStatus1(Common::KEYCODE_ESCAPE);
	_vm->clearKeyStatus1(29);
	_vm->clearKeyStatus1(Common::KEYCODE_SPACE);
	gameSys.removeSpriteDrawItem(_vm->_largeSprite, 300);
	_vm->delayTicksCursor(5);
	_vm->deleteSurface(&_vm->_largeSprite);
	_vm->showCursor();
}

void Scene25::run() {
	GameSys& gameSys = *_vm->_gameSys;

	_vm->playSound(0x10940, true);
	_vm->startSoundTimerA(5);

	_currTicketVendorSequenceId = 0x52;
	gameSys.setAnimation(0x52, 39, 3);
	gameSys.insertSequence(_currTicketVendorSequenceId, 39, 0, 0, kSeqNone, 0, 0, 0);

	_nextTicketVendorSequenceId = -1;
	_vm->_timers[4] = _vm->getRandom(20) + 20;

	_vm->queueInsertDeviceIcon();

	if (_vm->_prevSceneNum == 24) {
		_vm->initGnapPos(5, 11, kDirUpLeft);
		_vm->initPlatypusPos(6, 11, kDirUnk4);
		_vm->endSceneInit();
		_vm->gnapWalkTo(5, 7, -1, 0x107BA, 1);
		_vm->platypusWalkTo(6, 7, -1, 0x107C2, 1);
	} else {
		_vm->initGnapPos(5, 6, kDirBottomRight);
		_vm->initPlatypusPos(6, 6, kDirNone);
		_vm->endSceneInit();
		_vm->gnapWalkTo(5, 8, -1, 0x107B9, 1);
		_vm->platypusWalkTo(6, 8, -1, 0x107C2, 1);
	}

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS25Device:
			if (_vm->_gnap->_actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS25Platypus:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemJoint) {
					_vm->gnapUseJointOnPlatypus();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapImpossible(0, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(_vm->_plat->_pos.x, _vm->_plat->_pos.y);
						break;
					case GRAB_CURSOR:
						_vm->gnapKissPlatypus(0);
						break;
					case TALK_CURSOR:
						_vm->playGnapBrainPulsating(_vm->_plat->_pos.x, _vm->_plat->_pos.y);
						_vm->playPlatypusSequence(_vm->getPlatypusSequenceId());
						break;
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS25TicketVendor:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemTickets) {
					_vm->_gnap->_actionStatus = kAS25ShowTicketToVendor;
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS25TicketVendor].x, _vm->_hotspotsWalkPos[kHS25TicketVendor].y,
						0, _vm->getGnapSequenceId(gskIdle, 9, 4) | 0x10000, 1);
					_vm->playGnapPullOutDevice(0, 0);
					_vm->playGnapUseDevice(0, 0);
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHS25TicketVendor].x, _vm->_hotspotsWalkPos[kHS25TicketVendor].y, 6, 1);
					_nextTicketVendorSequenceId = 0x5B;
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(6, 1);
						_nextTicketVendorSequenceId = (_vm->getRandom(2) == 1) ? 0x59 : 0x56;
						break;
					case TALK_CURSOR:
						_vm->_gnap->_idleFacing = kDirUpRight;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS25TicketVendor].x, _vm->_hotspotsWalkPos[kHS25TicketVendor].y, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_vm->_gnap->_actionStatus = kAS25TalkTicketVendor;
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS25ExitOutsideCircusWorld:
			if (_vm->_gnap->_actionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 24;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS25ExitOutsideCircusWorld].x, _vm->_hotspotsWalkPos[kHS25ExitOutsideCircusWorld].y, 0, 0x107B4, 1);
				_vm->_gnap->_actionStatus = kAS25LeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS25ExitOutsideCircusWorld].x + 1, _vm->_hotspotsWalkPos[kHS25ExitOutsideCircusWorld].y, -1, 0x107C2, 1);
			}
			break;

		case kHS25ExitInsideCircusWorld:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_vm->isFlag(kGFNeedleTaken)) {
					_vm->_isLeavingScene = true;
					_vm->_newSceneNum = 26;
					_vm->_hotspots[kHS25WalkArea1]._flags |= SF_WALKABLE;
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS25ExitInsideCircusWorld].x, _vm->_hotspotsWalkPos[kHS25ExitInsideCircusWorld].y, 0, 0x107B1, 1);
					_vm->_gnap->_actionStatus = kAS25LeaveScene;
					_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS25ExitInsideCircusWorld].x + 1, _vm->_hotspotsWalkPos[kHS25ExitInsideCircusWorld].y, -1, 0x107C2, 1);
					_vm->_hotspots[kHS25WalkArea1]._flags &= ~SF_WALKABLE;
				} else {
					_vm->_hotspots[kHS25WalkArea1]._flags |= SF_WALKABLE;
					_vm->gnapWalkTo(4, 5, 0, 0x107BB, 1);
					_vm->_gnap->_actionStatus = kAS25EnterCircusWihoutTicket;
					_vm->_hotspots[kHS25WalkArea1]._flags &= ~SF_WALKABLE;
				}
			}
			break;

		case kHS25Posters1:
		case kHS25Posters2:
		case kHS25Posters3:
		case kHS25Posters4:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapImpossible(0, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].x, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].y, -1, -1, 1);
						if (_vm->_sceneClickedHotspot == 5 || _vm->_sceneClickedHotspot == 6)
							_vm->_gnap->_idleFacing = kDirUpLeft;
						else if (_vm->_sceneClickedHotspot == 8)
							_vm->_gnap->_idleFacing = kDirBottomRight;
						else
							_vm->_gnap->_idleFacing = kDirUpRight;
						_vm->playGnapIdle(0, 0);
						playAnims(8 - _vm->_sceneClickedHotspot + 1);
						break;
					case GRAB_CURSOR:
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapMoan2(0, 0);
						break;
					}
				}
			}
			break;

		case kHS25WalkArea1:
		case kHS25WalkArea2:
			if (_vm->_gnap->_actionStatus < 0)
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
			if (!_vm->_timers[4] && _nextTicketVendorSequenceId == -1 && _vm->_gnap->_actionStatus == -1) {
				_vm->_timers[4] = _vm->getRandom(20) + 20;
				switch (_vm->getRandom(13)) {
				case 0:
					_nextTicketVendorSequenceId = 0x54;
					break;
				case 1:
					_nextTicketVendorSequenceId = 0x58;
					break;
				case 2:
					_nextTicketVendorSequenceId = 0x55;
					break;
				case 3:
					_nextTicketVendorSequenceId = 0x5A;
					break;
				case 4:
				case 5:
				case 6:
				case 7:
					_nextTicketVendorSequenceId = 0x5B;
					break;
				case 8:
				case 9:
				case 10:
				case 11:
					_nextTicketVendorSequenceId = 0x5C;
					break;
				case 12:
					_nextTicketVendorSequenceId = 0x5D;
					break;
				default:
					_nextTicketVendorSequenceId = 0x52;
					break;
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

void Scene25::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (_vm->_gnap->_actionStatus) {
		case kAS25TalkTicketVendor:
			_nextTicketVendorSequenceId = (_vm->getRandom(2) == 1) ? 0x57 : 0x5F;
			_vm->_gnap->_actionStatus = -1;
			break;
		case kAS25EnterCircusWihoutTicket:
			_nextTicketVendorSequenceId = 0x5E;
			gameSys.setAnimation(0x5E, 39, 0);
			gameSys.setAnimation(_nextTicketVendorSequenceId, 39, 3);
			gameSys.insertSequence(_nextTicketVendorSequenceId, 39, _currTicketVendorSequenceId, 39, kSeqSyncExists, 0, 0, 0);
			gameSys.insertSequence(0x60, 2, 0, 0, kSeqNone, 0, 0, 0);
			_currTicketVendorSequenceId = _nextTicketVendorSequenceId;
			_nextTicketVendorSequenceId = -1;
			_vm->_hotspots[kHS25WalkArea1]._flags |= SF_WALKABLE;
			_vm->playGnapIdle(0, 0);
			_vm->gnapWalkTo(_vm->_hotspotsWalkPos[3].x, _vm->_hotspotsWalkPos[3].y, -1, 0x107BB, 1);
			_vm->_hotspots[kHS25WalkArea1]._flags &= ~SF_WALKABLE;
			_vm->_gnap->_actionStatus = kAS25EnterCircusWihoutTicketDone;
			break;
		case kAS25EnterCircusWihoutTicketDone:
			_vm->_gnap->_actionStatus = -1;
			break;
		case kAS25ShowTicketToVendor:
			_vm->setGrabCursorSprite(-1);
			_vm->invRemove(kItemTickets);
			_vm->setFlag(kGFNeedleTaken);
			gameSys.setAnimation(0x61, 40, 0);
			gameSys.insertSequence(0x61, 40, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_gnap->_actionStatus = kAS25ShowTicketToVendorDone;
			break;
		case kAS25ShowTicketToVendorDone:
			_nextTicketVendorSequenceId = 0x53;
			break;
		case kAS25LeaveScene:
			_vm->_sceneDone = true;
			break;
		}
	}

	if (gameSys.getAnimationStatus(3) == 2) {
		if (_nextTicketVendorSequenceId == 0x53) {
			gameSys.insertSequence(_nextTicketVendorSequenceId, 39, _currTicketVendorSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			_currTicketVendorSequenceId = _nextTicketVendorSequenceId;
			_nextTicketVendorSequenceId = -1;
			_vm->_gnap->_actionStatus = -1;
		} else if (_nextTicketVendorSequenceId != -1) {
			gameSys.setAnimation(_nextTicketVendorSequenceId, 39, 3);
			gameSys.insertSequence(_nextTicketVendorSequenceId, 39, _currTicketVendorSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			_currTicketVendorSequenceId = _nextTicketVendorSequenceId;
			_nextTicketVendorSequenceId = -1;
		}
	}
}

/*****************************************************************************/

Scene26::Scene26(GnapEngine *vm) : Scene(vm) {
	_currKidSequenceId = -1;
	_nextKidSequenceId = -1;
}

int Scene26::init() {
	return _vm->isFlag(kGFUnk23) ? 0x61 : 0x60;
}

void Scene26::updateHotspots() {
	_vm->setHotspot(kHS26Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS26ExitOutsideCircusWorld, 0, 590, 300, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 1, 10);
	_vm->setHotspot(kHS26ExitOutsideClown, 200, 265, 265, 350, SF_EXIT_U_CURSOR, 3, 8);
	_vm->setHotspot(kHS26ExitArcade, 0, 295, 150, 400, SF_EXIT_NW_CURSOR, 2, 8);
	_vm->setHotspot(kHS26ExitElephant, 270, 290, 485, 375, SF_EXIT_U_CURSOR, 5, 8);
	_vm->setHotspot(kHS26ExitBeerStand, 530, 290, 620, 350, SF_EXIT_NE_CURSOR, 5, 8);
	_vm->setHotspot(kHS26WalkArea1, 0, 0, 800, 500);
	_vm->setHotspot(kHS26WalkArea2, 281, 0, 800, 600);
	_vm->setDeviceHotspot(kHS26Device, -1, -1, -1, -1);
	_vm->_hotspotsCount = 9;
}

void Scene26::run() {
	GameSys& gameSys = *_vm->_gameSys;

	_vm->startSoundTimerB(7);
	_vm->playSound(0x1093B, true);

	_currKidSequenceId = 0x5B;
	_nextKidSequenceId = -1;
	gameSys.setAnimation(0x5B, 160, 3);
	gameSys.insertSequence(_currKidSequenceId, 160, 0, 0, kSeqNone, 0, 0, 0);

	_vm->_timers[5] = _vm->getRandom(20) + 50;
	_vm->_timers[4] = _vm->getRandom(20) + 50;
	_vm->_timers[6] = _vm->getRandom(50) + 100;

	_vm->queueInsertDeviceIcon();

	gameSys.insertSequence(0x58, 40, 0, 0, kSeqLoop, 0, 0, 0);
	gameSys.insertSequence(0x5C, 40, 0, 0, kSeqLoop, 0, 0, 0);
	gameSys.insertSequence(0x5D, 40, 0, 0, kSeqLoop, 0, 0, 0);
	gameSys.insertSequence(0x5E, 40, 0, 0, kSeqLoop, 0, 0, 0);

	if (_vm->_prevSceneNum == 25) {
		_vm->initGnapPos(-1, 8, kDirBottomRight);
		_vm->initPlatypusPos(-2, 8, kDirNone);
		_vm->endSceneInit();
		_vm->gnapWalkTo(2, 8, -1, 0x107B9, 1);
		_vm->platypusWalkTo(1, 8, -1, 0x107C2, 1);
	} else {
		_vm->initGnapPos(2, 8, kDirBottomRight);
		_vm->initPlatypusPos(3, 8, kDirNone);
		_vm->endSceneInit();
	}

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS26Device:
			if (_vm->_gnap->_actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS26Platypus:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemJoint) {
					_vm->gnapUseJointOnPlatypus();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapImpossible(0, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapMoan1(_vm->_plat->_pos.x, _vm->_plat->_pos.y);
						break;
					case GRAB_CURSOR:
						_vm->gnapKissPlatypus(0);
						break;
					case TALK_CURSOR:
						_vm->playGnapBrainPulsating(_vm->_plat->_pos.x, _vm->_plat->_pos.y);
						_vm->playPlatypusSequence(_vm->getPlatypusSequenceId());
						break;
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS26ExitOutsideCircusWorld:
			if (_vm->_gnap->_actionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 25;
				_vm->gnapWalkTo(-1, _vm->_hotspotsWalkPos[kHS26ExitOutsideCircusWorld].y, 0, 0x107AE, 1);
				_vm->_gnap->_actionStatus = kAS26LeaveScene;
			}
			break;

		case kHS26ExitOutsideClown:
			if (_vm->_gnap->_actionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 27;
				_vm->gnapWalkTo(-1, _vm->_hotspotsWalkPos[kHS26ExitOutsideClown].y, 0, 0x107BC, 1);
				_vm->_gnap->_actionStatus = kAS26LeaveScene;
			}
			break;

		case kHS26ExitArcade:
			if (_vm->_gnap->_actionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 29;
				_vm->gnapWalkTo(-1, _vm->_hotspotsWalkPos[kHS26ExitArcade].y, 0, 0x107BC, 1);
				_vm->_gnap->_actionStatus = kAS26LeaveScene;
			}
			break;

		case kHS26ExitElephant:
			if (_vm->_gnap->_actionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 30;
				_vm->gnapWalkTo(-1, _vm->_hotspotsWalkPos[4].y, 0, 0x107BC, 1);
				_vm->_gnap->_actionStatus = kAS26LeaveScene;
			}
			break;

		case kHS26ExitBeerStand:
			if (_vm->_gnap->_actionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 31;
				_vm->gnapWalkTo(-1, _vm->_hotspotsWalkPos[kHS26ExitBeerStand].y, 0, 0x107BB, 1);
				_vm->_gnap->_actionStatus = kAS26LeaveScene;
			}
			break;

		case kHS26WalkArea1:
		case kHS26WalkArea2:
			if (_vm->_gnap->_actionStatus < 0)
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

		if (!_vm->isSoundPlaying(0x1093B))
			_vm->playSound(0x1093B, true);

		if (!_vm->_isLeavingScene) {
			_vm->updatePlatypusIdleSequence();
			_vm->updateGnapIdleSequence();
			if (!_vm->_timers[5] && _nextKidSequenceId == -1) {
				_vm->_timers[5] = _vm->getRandom(20) + 50;
				if (_vm->getRandom(5) != 0)
					_nextKidSequenceId = 0x5B;
				else
					_nextKidSequenceId = 0x5A;
			}
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(20) + 130;
				gameSys.insertSequence(0x59, 40, 0, 0, kSeqNone, 0, 0, 0);
			}
			if (!_vm->_timers[6]) {
				_vm->_timers[6] = _vm->getRandom(50) + 100;
				gameSys.insertSequence(0x5F, 40, 0, 0, kSeqNone, 0, 0, 0);
			}
			_vm->playSoundB();
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

void Scene26::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		if (_vm->_gnap->_actionStatus == kAS26LeaveScene)
			_vm->_sceneDone = true;
		_vm->_gnap->_actionStatus = -1;
	}

	if (gameSys.getAnimationStatus(3) == 2 && _nextKidSequenceId != -1) {
		gameSys.insertSequence(_nextKidSequenceId, 160, _currKidSequenceId, 160, kSeqSyncWait, 0, 0, 0);
		gameSys.setAnimation(_nextKidSequenceId, 160, 3);
		_currKidSequenceId = _nextKidSequenceId;
		_nextKidSequenceId = -1;
	}
}

/*****************************************************************************/

Scene27::Scene27(GnapEngine *vm) : Scene(vm) {
	_nextJanitorSequenceId = -1;
	_currJanitorSequenceId = -1;
}

int Scene27::init() {
	return 0xD5;
}

void Scene27::updateHotspots() {
	_vm->setHotspot(kHS27Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS27Janitor, 488, 204, 664, 450, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 8, 8);
	_vm->setHotspot(kHS27Bucket, 129, 406, 186, 453, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 6);
	_vm->setHotspot(kHS27ExitCircus, 200, 585, 700, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
	_vm->setHotspot(kHS27ExitArcade, 0, 0, 15, 600, SF_EXIT_L_CURSOR, 0, 6);
	_vm->setHotspot(kHS27ExitBeerStand, 785, 0, 800, 600, SF_EXIT_R_CURSOR, 11, 7);
	_vm->setHotspot(kHS27ExitClown, 340, 240, 460, 420, SF_EXIT_U_CURSOR, 6, 8);
	_vm->setHotspot(kHS27WalkArea1, 0, 0, 800, 507);
	_vm->setDeviceHotspot(kHS27Device, -1, -1, -1, -1);
	if (_vm->isFlag(kGFUnk13))
		_vm->_hotspots[kHS27Bucket]._flags = SF_DISABLED;
	_vm->_hotspotsCount = 9;
}

void Scene27::run() {
	GameSys& gameSys = *_vm->_gameSys;

	_vm->playSound(0x1093B, true);
	_vm->startSoundTimerB(4);
	_vm->_timers[7] = _vm->getRandom(100) + 300;
	_vm->queueInsertDeviceIcon();

	if (!_vm->isFlag(kGFUnk13))
		gameSys.insertSequence(0xD3, 39, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->_debugLevel == 4)
		_vm->startIdleTimer(6);

	gameSys.insertSequence(0xCB, 39, 0, 0, kSeqNone, 0, 0, 0);

	_currJanitorSequenceId = 0xCB;
	_nextJanitorSequenceId = -1;

	gameSys.setAnimation(0xCB, 39, 3);
	_vm->_timers[5] = _vm->getRandom(20) + 60;

	switch (_vm->_prevSceneNum) {
	case 26:
		_vm->initGnapPos(7, 12, kDirBottomRight);
		_vm->initPlatypusPos(6, 12, kDirNone);
		_vm->endSceneInit();
		_vm->gnapWalkTo(7, 8, -1, 0x107B9, 1);
		_vm->platypusWalkTo(6, 8, -1, 0x107C2, 1);
		break;
	case 29:
		_vm->initGnapPos(-1, 8, kDirBottomRight);
		_vm->initPlatypusPos(-1, 9, kDirNone);
		_vm->endSceneInit();
		_vm->gnapWalkTo(3, 8, -1, 0x107B9, 1);
		_vm->platypusWalkTo(3, 9, -1, 0x107C2, 1);
		break;
	case 31:
		_vm->initGnapPos(12, 8, kDirBottomLeft);
		_vm->initPlatypusPos(12, 9, kDirUnk4);
		_vm->endSceneInit();
		_vm->gnapWalkTo(8, 8, -1, 0x107BA, 1);
		_vm->platypusWalkTo(8, 9, -1, 0x107C2, 1);
		break;
	default:
		_vm->initGnapPos(6, 8, kDirBottomRight);
		_vm->initPlatypusPos(5, 9, kDirNone);
		_vm->endSceneInit();
		break;
	}

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
		_vm->_sceneClickedHotspot = -1;
		if (_vm->_gnap->_actionStatus < 0)
			_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS27Device:
			if (_vm->_gnap->_actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS27Platypus:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemJoint) {
					_vm->gnapUseJointOnPlatypus();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapImpossible(0, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapMoan1(_vm->_plat->_pos.x, _vm->_plat->_pos.y);
						break;
					case GRAB_CURSOR:
						_vm->gnapKissPlatypus(0);
						break;
					case TALK_CURSOR:
						_vm->playGnapBrainPulsating(_vm->_plat->_pos.x, _vm->_plat->_pos.y);
						_vm->playPlatypusSequence(_vm->getPlatypusSequenceId());
						break;
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS27Janitor:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemPicture) {
					_vm->_gnap->_idleFacing = kDirUpLeft;
					if (_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS27Janitor].x, _vm->_hotspotsWalkPos[kHS27Janitor].y, 0, 0x107BC, 1))
						_vm->_gnap->_actionStatus = kAS27ShowPictureToJanitor;
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHS27Janitor].x, _vm->_hotspotsWalkPos[kHS27Janitor].y, 7, 3);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(6, 3);
						break;
					case TALK_CURSOR:
						_vm->_gnap->_idleFacing = kDirUpLeft;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS27Janitor].x, _vm->_hotspotsWalkPos[kHS27Janitor].y, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_vm->_gnap->_actionStatus = kAS27TalkJanitor;
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS27Bucket:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, 3, 3);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(3, 3);
						break;
					case GRAB_CURSOR:
						_vm->_gnap->_idleFacing = kDirUpLeft;
						_vm->gnapWalkTo(_vm->_gnap->_pos, 0, _vm->getGnapSequenceId(gskIdle, _vm->_hotspotsWalkPos[kHS27Bucket].x, _vm->_hotspotsWalkPos[kHS27Bucket].y) | 0x10000, 1);
						_vm->_gnap->_actionStatus = kAS27GrabBucket;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS27ExitCircus:
			if (_vm->_gnap->_actionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 26;
				_vm->gnapWalkTo(-1, _vm->_hotspotsWalkPos[kHS27ExitCircus].y, 0, 0x107AE, 1);
				_vm->_gnap->_actionStatus = kAS27LeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS27ExitCircus].x + 1, _vm->_hotspotsWalkPos[kHS27ExitCircus].y, -1, 0x107C7, 1);
			}
			break;

		case kHS27ExitArcade:
			if (_vm->_gnap->_actionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 29;
				_vm->gnapWalkTo(-1, _vm->_hotspotsWalkPos[kHS27ExitArcade].y, 0, 0x107AF, 1);
				_vm->_gnap->_actionStatus = kAS27LeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS27ExitArcade].x, _vm->_hotspotsWalkPos[kHS27ExitArcade].y + 1, -1, 0x107CF, 1);
			}
			break;

		case kHS27ExitBeerStand:
			if (_vm->_gnap->_actionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 31;
				_vm->gnapWalkTo(-1, _vm->_hotspotsWalkPos[kHS27ExitBeerStand].y, 0, 0x107AB, 1);
				_vm->_gnap->_actionStatus = kAS27LeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS27ExitBeerStand].x, _vm->_hotspotsWalkPos[kHS27ExitBeerStand].y + 1, -1, 0x107CD, 1);
			}
			break;

		case kHS27ExitClown:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_vm->isFlag(kGFPlatypus)) {
					_vm->_isLeavingScene = true;
					_vm->_newSceneNum = 28;
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS27ExitClown].x, _vm->_hotspotsWalkPos[kHS27ExitClown].y, 0, 0x107AD, 1);
					_vm->_gnap->_actionStatus = kAS27LeaveScene;
					_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS27ExitClown].x + 1, _vm->_hotspotsWalkPos[kHS27ExitClown].y, -1, 0x107C4, 1);
				} else {
					_vm->_hotspots[kHS27WalkArea1]._flags |= SF_WALKABLE;
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS27ExitClown].x, 7, 0, 0x107BC, 1);
					_vm->_hotspots[kHS27WalkArea1]._flags &= SF_WALKABLE;
					_vm->_gnap->_actionStatus = kAS27TryEnterClownTent;
				}
			}
			break;

		case kHS27WalkArea1:
			if (_vm->_gnap->_actionStatus < 0)
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			break;

		default:
			if (_vm->_mouseClickState._left && _vm->_gnap->_actionStatus < 0) {
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				_vm->_mouseClickState._left = false;
			}
			break;

		}

		updateAnimations();

		if (!_vm->isSoundPlaying(0x1093B))
			_vm->playSound(0x1093B, true);

		if (!_vm->_isLeavingScene) {
			_vm->updatePlatypusIdleSequence();
			if (_vm->_gnap->_actionStatus < 0)
				_vm->updateGnapIdleSequence();
			if (_vm->_debugLevel == 4)
				_vm->updateIdleTimer();
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(20) + 60;
				if (_vm->_gnap->_actionStatus < 0) {
					if (_vm->getRandom(3) != 0)
						_nextJanitorSequenceId = 0xCB;
					else
						_nextJanitorSequenceId = 0xCF;
				}
			}
			if (!_vm->_timers[7]) {
				_vm->_timers[7] = _vm->getRandom(100) + 300;
				if (_vm->_gnap->_actionStatus < 0)
					gameSys.insertSequence(0xD4, 120, 0, 0, kSeqNone, 0, 0, 0);
			}
			_vm->playSoundB();
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

void Scene27::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (_vm->_gnap->_actionStatus) {
		case kAS27TalkJanitor:
			switch (_vm->getRandom(3)) {
			case 0:
				_nextJanitorSequenceId = 0xCC;
				break;
			case 1:
				_nextJanitorSequenceId = 0xCD;
				break;
			case 2:
				_nextJanitorSequenceId = 0xCE;
				break;
			}
			break;
		case kAS27GrabBucket:
			_vm->playGnapPullOutDevice(0, 0);
			_vm->playGnapUseDevice(0, 0);
			_vm->_hotspots[kHS27Bucket]._flags = SF_DISABLED;
			_vm->invAdd(kItemEmptyBucket);
			_vm->setFlag(kGFUnk13);
			gameSys.setAnimation(0xD2, 39, 0);
			gameSys.insertSequence(0xD2, 39, 211, 39, kSeqSyncWait, 0, 0, 0);
			_vm->_gnap->_actionStatus = kAS27GrabBucketDone;
			break;
		case kAS27GrabBucketDone:
			_vm->setGrabCursorSprite(kItemEmptyBucket);
			_vm->_gnap->_actionStatus = -1;
			break;
		case kAS27ShowPictureToJanitor:
			_nextJanitorSequenceId = 0xD0;
			break;
		case kAS27TryEnterClownTent:
			_nextJanitorSequenceId = 0xD1;
			gameSys.insertSequence(0xD1, 39, _currJanitorSequenceId, 39, kSeqSyncExists, 0, 0, 0);
			gameSys.setAnimation(_nextJanitorSequenceId, 39, 3);
			gameSys.setAnimation(_nextJanitorSequenceId, 39, 0);
			_currJanitorSequenceId = _nextJanitorSequenceId;
			_nextJanitorSequenceId = -1;
			_vm->_gnap->_actionStatus = kAS27TryEnterClownTentDone;
			break;
		case kAS27TryEnterClownTentDone:
			_vm->_hotspots[kHS27WalkArea1]._flags |= SF_WALKABLE;
			_vm->gnapWalkTo(_vm->_hotspotsWalkPos[7].x, 9, -1, 0x107BC, 1);
			_vm->_hotspots[kHS27WalkArea1]._flags &= ~SF_WALKABLE;
			_vm->_gnap->_actionStatus = -1;
			break;
		case kAS27EnterClownTent:
			_vm->gnapWalkTo(_vm->_gnap->_pos, 0, 0x107B2, 1);
			_vm->_gnap->_actionStatus = kAS27LeaveScene;
			break;
		case kAS27LeaveScene:
			_vm->_sceneDone = true;
			break;
		}
	}

	if (gameSys.getAnimationStatus(3) == 2) {
		switch (_nextJanitorSequenceId) {
		case -1:
			_nextJanitorSequenceId = 0xCB;
			gameSys.insertSequence(0xCB, 39, _currJanitorSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(_nextJanitorSequenceId, 39, 3);
			_currJanitorSequenceId = _nextJanitorSequenceId;
			_nextJanitorSequenceId = -1;
			break;
		case 0xCC:
		case 0xCD:
		case 0xCE:
			_vm->_gnap->_actionStatus = -1;
			gameSys.insertSequence(_nextJanitorSequenceId, 39, _currJanitorSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(_nextJanitorSequenceId, 39, 3);
			gameSys.setAnimation(_nextJanitorSequenceId, 39, 0);
			_currJanitorSequenceId = _nextJanitorSequenceId;
			_nextJanitorSequenceId = -1;
			break;
		case 0xD0:
			// Show picture to janitor
			_vm->playGnapPullOutDevice(0, 0);
			_vm->playGnapUseDevice(0, 0);
			gameSys.insertSequence(_nextJanitorSequenceId, 39, _currJanitorSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(_nextJanitorSequenceId, 39, 0);
			_vm->_gnap->_actionStatus = kAS27EnterClownTent;
			_currJanitorSequenceId = _nextJanitorSequenceId;
			_nextJanitorSequenceId = -1;
			_vm->setFlag(kGFPlatypus);
			_vm->setGrabCursorSprite(-1);
			_vm->invRemove(kItemPicture);
			_vm->_newSceneNum = 28;
			break;
		default:
			gameSys.insertSequence(_nextJanitorSequenceId, 39, _currJanitorSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(_nextJanitorSequenceId, 39, 3);
			_currJanitorSequenceId = _nextJanitorSequenceId;
			_nextJanitorSequenceId = -1;
			break;
		}
	}
}

/*****************************************************************************/

Scene28::Scene28(GnapEngine *vm) : Scene(vm) {
	_currClownSequenceId = -1;
	_nextClownSequenceId = -1;
	_clownTalkCtr = 0;
}

int Scene28::init() {
	return 0x125;
}

void Scene28::updateHotspots() {
	_vm->setHotspot(kHS28Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS28Horn, 148, 352, 215, 383, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 1, 7);
	_vm->setHotspot(kHS28Clown, 130, 250, 285, 413, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 5);
	_vm->setHotspot(kHS28ExitOutsideClown, 660, 190, 799, 400, SF_EXIT_R_CURSOR, 9, 6);
	_vm->setHotspot(kHS28EmptyBucket, 582, 421, 643, 478, SF_WALKABLE | SF_DISABLED, 9, 7);
	_vm->setHotspot(kHS28WalkArea1, 0, 0, 799, 523);
	_vm->setHotspot(kHS28WalkArea2, 0, 0, 0, 0, 7, SF_DISABLED);
	_vm->setDeviceHotspot(kHS28Device, -1, -1, -1, -1);
	if (_vm->invHas(kItemHorn))
		_vm->_hotspots[kHS28Horn]._flags = SF_DISABLED;
	if (_vm->isFlag(kGFUnk22))
		_vm->_hotspots[kHS28EmptyBucket]._flags = SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	_vm->_hotspotsCount = 8;
}

void Scene28::run() {
	GameSys& gameSys = *_vm->_gameSys;

	_vm->playSound(0x1093C, true);
	_nextClownSequenceId = -1;
	_vm->queueInsertDeviceIcon();
	gameSys.insertSequence(0x124, 255, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->isFlag(kGFUnk22))
		gameSys.insertSequence(0x112, 99, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->isFlag(kGFMudTaken)) {
		if (_vm->isFlag(kGFUnk21)) {
			gameSys.setAnimation(0x11C, 39, 3);
			gameSys.insertSequence(0x11C, 39, 0, 0, kSeqNone, 0, 0, 0);
			if (!_vm->invHas(kItemHorn))
				gameSys.insertSequence(0x118, 59, 0, 0, kSeqNone, 0, 0, 0);
			_currClownSequenceId = 0x11C;
		} else {
			_currClownSequenceId = 0x11B;
			gameSys.setAnimation(0x11B, 39, 3);
			gameSys.insertSequence(_currClownSequenceId, 39, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_timers[4] = _vm->getRandom(20) + 80;
		}
		_vm->initGnapPos(8, 8, kDirBottomLeft);
		_vm->initPlatypusPos(9, 8, kDirUnk4);
		_vm->endSceneInit();
	} else {
		gameSys.insertSequence(0x11B, 39, 0, 0, kSeqNone, 0, 0, 0);
		_vm->initGnapPos(8, 8, kDirBottomLeft);
		_vm->initPlatypusPos(9, 8, kDirUnk4);
		_vm->endSceneInit();
		_vm->playSequences(0xF7, 0x121, 0x122, 0x123);
		_currClownSequenceId = 0x115;
		_vm->setFlag(kGFMudTaken);
		gameSys.setAnimation(0x115, 39, 3);
		gameSys.insertSequence(_currClownSequenceId, 39, 0x11B, 39, kSeqSyncWait, 0, 0, 0);
		_nextClownSequenceId = -1;
		_vm->_timers[4] = _vm->getRandom(20) + 80;
		_vm->_gnap->_actionStatus = kAS28GnapWaiting;
		while (gameSys.getAnimationStatus(3) != 2) {
			_vm->gameUpdateTick();
			_vm->updateMouseCursor();
		}
		_vm->_gnap->_actionStatus = -1;
	}

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS28Device:
			if (_vm->_gnap->_actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS28Platypus:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemJoint) {
					_vm->gnapUseJointOnPlatypus();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapImpossible(0, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapMoan1(_vm->_plat->_pos.x, _vm->_plat->_pos.y);
						break;
					case GRAB_CURSOR:
						_vm->gnapKissPlatypus(0);
						break;
					case TALK_CURSOR:
						_vm->playGnapBrainPulsating(_vm->_plat->_pos.x, _vm->_plat->_pos.y);
						_vm->playPlatypusSequence(_vm->getPlatypusSequenceId());
						break;
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS28Horn:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(2, 8, 3, 4);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(2, 4);
						break;
					case GRAB_CURSOR:
						if (_vm->isFlag(kGFUnk21)) {
							if (!_vm->invHas(kItemHorn)) {
								_vm->gnapWalkTo(_vm->_gnap->_pos, 0, _vm->getGnapSequenceId(gskIdle, _vm->_hotspotsWalkPos[kHS28Horn].x, _vm->_hotspotsWalkPos[kHS28Horn].y) | 0x10000, 1);
								_vm->_gnap->_actionStatus = kAS28GrabHornSuccess;
							}
						} else {
							_vm->_gnap->_idleFacing = kDirUpLeft;
							_vm->gnapWalkTo(2, 8, 0, 0x107BB, 1);
							_vm->_hotspots[kHS28WalkArea1]._flags |= SF_WALKABLE;
							_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS28Horn].x, _vm->_hotspotsWalkPos[kHS28Horn].y, 0, 0x107BB, 1);
							_vm->_hotspots[kHS28WalkArea1]._flags &= ~SF_WALKABLE;
							_vm->_gnap->_actionStatus = kAS28GrabHornFails;
						}
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS28Clown:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_vm->isFlag(kGFUnk21)) {
					if (_vm->_verbCursor == LOOK_CURSOR)
						_vm->playGnapScratchingHead(5, 2);
					else
						_vm->playGnapImpossible(0, 0);
				} else if (_vm->_grabCursorSpriteIndex == kItemBucketWithBeer) {
					_vm->_gnap->_idleFacing = kDirUpLeft;
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS28Clown].x, _vm->_hotspotsWalkPos[kHS28Clown].y, 0, 0x107BC, 1);
					_vm->playGnapPullOutDevice(0, 0);
					_vm->playGnapUseDevice(0, 0);
					_vm->_gnap->_actionStatus = kAS28UseBeerBucketWithClown;
				} else if (_vm->_grabCursorSpriteIndex == kItemBucketWithPill) {
					_vm->_gnap->_idleFacing = kDirUpLeft;
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS28Clown].x, _vm->_hotspotsWalkPos[kHS28Clown].y, 0, 0x107BC, 1);
					_vm->playGnapPullOutDevice(0, 0);
					_vm->playGnapUseDevice(0, 0);
					_vm->_gnap->_actionStatus = kAS28UsePillBucketWithClown;
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHS28Clown].x, _vm->_hotspotsWalkPos[kHS28Clown].y, 2, 4);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(5, 2);
						break;
					case TALK_CURSOR:
						_vm->_gnap->_idleFacing = kDirUpLeft;
						_vm->gnapWalkTo(5, 8, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_vm->_gnap->_actionStatus = kAS28TalkClown;
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS28ExitOutsideClown:
			if (_vm->_gnap->_actionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 27;
				_vm->_hotspots[kHS28WalkArea1]._flags |= SF_WALKABLE;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS28ExitOutsideClown].x, _vm->_hotspotsWalkPos[kHS28ExitOutsideClown].y, 0, 0x107BF, 1);
				_vm->_gnap->_actionStatus = kAS28LeaveScene;
				_vm->_hotspots[kHS28WalkArea1]._flags &= ~SF_WALKABLE;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS28ExitOutsideClown].x - 1, _vm->_hotspotsWalkPos[kHS28ExitOutsideClown].y, -1, 0x107C2, 1);
			}
			break;

		case kHS28EmptyBucket:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, 8, 6);
				} else if (_vm->isFlag(kGFUnk21)) {
					_vm->playGnapImpossible(8, 6);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(8, 6);
						break;
					case GRAB_CURSOR:
						_vm->gnapWalkTo(_vm->_gnap->_pos, 0, _vm->getGnapSequenceId(gskIdle, _vm->_hotspotsWalkPos[kHS28EmptyBucket].x, _vm->_hotspotsWalkPos[kHS28EmptyBucket].y) | 0x10000, 1);
						_vm->_gnap->_actionStatus = kAS28GrabEmptyBucket;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS28WalkArea1:
		case kHS28WalkArea2:
			if (_vm->_gnap->_actionStatus < 0)
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			break;

		default:
			if (_vm->_mouseClickState._left && _vm->_gnap->_actionStatus < 0) {
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				_vm->_mouseClickState._left = false;
			}
			break;
		}

		updateAnimations();

		if (!_vm->isSoundPlaying(0x1093C))
			_vm->playSound(0x1093C, true);

		if (!_vm->_isLeavingScene) {
			_vm->updatePlatypusIdleSequence();
			_vm->updateGnapIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(20) + 80;
				if (_vm->_gnap->_actionStatus < 0 && !_vm->isFlag(kGFUnk21))
					_nextClownSequenceId = 0x114;
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

void Scene28::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (_vm->_gnap->_actionStatus) {
		case kAS28UseBeerBucketWithClown:
			_vm->setFlag(kGFUnk22);
			_nextClownSequenceId = 0x113;
			_vm->invRemove(kItemBucketWithBeer);
			updateHotspots();
			break;
		case kAS28UsePillBucketWithClown:
			_nextClownSequenceId = 0x116;
			_vm->invRemove(kItemBucketWithPill);
			_vm->setFlag(kGFUnk22);
			_vm->setFlag(kGFUnk21);
			updateHotspots();
			break;
		case kAS28GrabHornFails:
			_nextClownSequenceId = 0x119;
			break;
		case kAS28GrabHornSuccess:
			_vm->playGnapPullOutDevice(0, 0);
			_vm->playGnapUseDevice(0, 0);
			gameSys.setAnimation(0x117, 59, 0);
			gameSys.insertSequence(0x117, 59, 280, 59, kSeqSyncWait, 0, 0, 0);
			_vm->_gnap->_actionStatus = kAS28GrabHornSuccessDone;
			break;
		case kAS28GrabHornSuccessDone:
			_vm->hideCursor();
			_vm->setGrabCursorSprite(-1);
			_vm->addFullScreenSprite(0xF6, 255);
			gameSys.setAnimation(0x120, 256, 0);
			gameSys.insertSequence(0x120, 256, 0, 0, kSeqNone, 0, 0, 0);
			while (gameSys.getAnimationStatus(0) != 2)
				_vm->gameUpdateTick();
			_vm->removeFullScreenSprite();
			_vm->showCursor();
			_vm->setGrabCursorSprite(kItemHorn);
			_vm->invAdd(kItemHorn);
			updateHotspots();
			_vm->_gnap->_actionStatus = -1;
			break;
		case kAS28GrabEmptyBucket:
			_vm->playGnapPullOutDevice(0, 0);
			_vm->playGnapUseDevice(0, 0);
			gameSys.setAnimation(0x111, 99, 0);
			gameSys.insertSequence(0x111, 99, 274, 99, kSeqSyncWait, 0, 0, 0);
			_vm->_gnap->_actionStatus = kAS28GrabEmptyBucketDone;
			break;
		case kAS28GrabEmptyBucketDone:
			_vm->setGrabCursorSprite(kItemEmptyBucket);
			_vm->clearFlag(kGFUnk22);;
			updateHotspots();
			_vm->invAdd(kItemEmptyBucket);
			_vm->_gnap->_actionStatus = -1;
			break;
		case kAS28GrabHornFailsDone:
			gameSys.insertSequence(0x107B5, _vm->_gnapId, 281, 39, kSeqSyncWait, 0, 75 * _vm->_gnap->_pos.x - _vm->_gnapGridX, 48 * _vm->_gnap->_pos.y - _vm->_gnapGridY);
			_vm->_gnapSequenceId = 0x7B5;
			_vm->_gnapSequenceDatNum = 1;
			gameSys.insertSequence(0x11B, 39, 0, 0, kSeqNone, 0, 0, 0);
			_currClownSequenceId = 0x11B;
			_nextClownSequenceId = -1;
			_vm->_gnap->_actionStatus = -1;
			_vm->gnapWalkTo(2, 8, -1, 0x107BB, 1);
			break;
		case kAS28TalkClown:
			_clownTalkCtr = (_clownTalkCtr + 1) % 2;
			if (_clownTalkCtr == 0)
				_nextClownSequenceId = 0x11D;
			else if (_clownTalkCtr == 1)
				_nextClownSequenceId = 0x11E;
			else if (_clownTalkCtr == 2)
				_nextClownSequenceId = 0x11F; // NOTE CHECKME Never set, bug in the original?
			break;
		case kAS28GnapWaiting:
			_vm->_gnap->_actionStatus = -1;
			break;
		case kAS28LeaveScene:
			_vm->_sceneDone = true;
			break;
		}
	}

	if (gameSys.getAnimationStatus(3) == 2) {
		switch (_nextClownSequenceId) {
		case 0x113:
			_vm->setGrabCursorSprite(-1);
			gameSys.setAnimation(_nextClownSequenceId, 39, 0);
			gameSys.insertSequence(0x112, 99, 0, 0, kSeqNone, _vm->getSequenceTotalDuration(_nextClownSequenceId), 0, 0);
			gameSys.insertSequence(_nextClownSequenceId, 39, _currClownSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0x11B, 39, _nextClownSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			_currClownSequenceId = 0x11B;
			_nextClownSequenceId = -1;
			_vm->_gnap->_actionStatus = kAS28GnapWaiting;
			break;
		case 0x116:
			_vm->setGrabCursorSprite(-1);
			gameSys.setAnimation(_nextClownSequenceId, 39, 0);
			gameSys.insertSequence(0x112, 99, 0, 0, kSeqNone, _vm->getSequenceTotalDuration(_nextClownSequenceId), 0, 0);
			gameSys.insertSequence(_nextClownSequenceId, 39, _currClownSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0x11C, 39, _nextClownSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0x118, 59, 0, 0, kSeqNone, _vm->getSequenceTotalDuration(_nextClownSequenceId), 0, 0);
			_currClownSequenceId = _nextClownSequenceId;
			_nextClownSequenceId = -1;
			_vm->_gnap->_actionStatus = kAS28GnapWaiting;
			break;
		case 0x11D:
		case 0x11E:
		case 0x11F:
			_vm->_gnap->_actionStatus = -1;
			break;
		case 0x119:
			gameSys.insertSequence(_nextClownSequenceId, 39, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(_nextClownSequenceId, 39, 0);
			gameSys.removeSequence(_currClownSequenceId, 39, true);
			_vm->_gnap->_actionStatus = kAS28GrabHornFailsDone;
			_vm->_gnapSequenceId = _nextClownSequenceId;
			_vm->_gnapSequenceDatNum = 0;
			_nextClownSequenceId = -1;
			break;
		}
		if (_nextClownSequenceId != -1) {
			gameSys.insertSequence(_nextClownSequenceId, 39, _currClownSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(_nextClownSequenceId, 39, 3);
			_currClownSequenceId = _nextClownSequenceId;
			_nextClownSequenceId = -1;
		}
	}
}

/*****************************************************************************/

Scene29::Scene29(GnapEngine *vm) : Scene(vm) {
	_currMonkeySequenceId = -1;
	_nextMonkeySequenceId = -1;
	_currManSequenceId = -1;
	_nextManSequenceId = -1;
}

int Scene29::init() {
	return 0xF6;
}

void Scene29::updateHotspots() {
	_vm->setHotspot(kHS29Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS29Monkey, 410, 374, 518, 516, SF_WALKABLE | SF_DISABLED, 3, 7);
	_vm->setHotspot(kHS29ExitCircus, 150, 585, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
	_vm->setHotspot(kHS29ExitOutsideClown, 785, 0, 800, 600, SF_EXIT_R_CURSOR | SF_WALKABLE, 11, 9);
	_vm->setHotspot(kHS29Aracde, 88, 293, 155, 384, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 8);
	_vm->setHotspot(kHS29WalkArea1, 0, 0, 800, 478);
	_vm->setDeviceHotspot(kHS29Device, -1, -1, -1, -1);
	if (_vm->invHas(kItemHorn))
		_vm->_hotspots[kHS29Monkey]._flags = SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	_vm->_hotspotsCount = 7;
}

void Scene29::run() {
	GameSys& gameSys = *_vm->_gameSys;

	_vm->playSound(0x1093B, true);
	_vm->startSoundTimerB(6);
	_vm->queueInsertDeviceIcon();

	if (_vm->invHas(kItemHorn)) {
		_currMonkeySequenceId = 0xE8;
		_nextMonkeySequenceId = -1;
		gameSys.setAnimation(0xE8, 159, 4);
		gameSys.insertSequence(_currMonkeySequenceId, 159, 0, 0, kSeqNone, 0, 0, 0);
		gameSys.insertSequence(0xED, 39, 0, 0, kSeqNone, 0, 0, 0);
		_currManSequenceId = 0xED;
		_nextManSequenceId = -1;
		gameSys.setAnimation(0xED, 39, 3);
		_vm->_timers[4] = _vm->getRandom(20) + 60;
	} else {
		gameSys.insertSequence(0xF4, 19, 0, 0, kSeqNone, 0, 0, 0);
		gameSys.setAnimation(0, 0, 4);
		gameSys.insertSequence(0xED, 39, 0, 0, kSeqNone, 0, 0, 0);
		gameSys.setAnimation(0, 0, 3);
	}

	gameSys.insertSequence(0xF3, 39, 0, 0, kSeqLoop, 0, 0, 0);
	gameSys.insertSequence(0xF5, 38, 0, 0, kSeqLoop, 0, 0, 0);

	if (_vm->_prevSceneNum == 27) {
		_vm->initGnapPos(12, 7, kDirBottomRight);
		_vm->initPlatypusPos(12, 8, kDirNone);
		_vm->endSceneInit();
		_vm->gnapWalkTo(8, 7, -1, 0x107B9, 1);
		_vm->platypusWalkTo(8, 8, -1, 0x107C2, 1);
	} else {
		_vm->initGnapPos(-1, 7, kDirBottomRight);
		_vm->initPlatypusPos(-2, 7, kDirNone);
		_vm->endSceneInit();
		_vm->gnapWalkTo(2, 7, -1, 0x107B9, 1);
		_vm->platypusWalkTo(1, 7, -1, 0x107C2, 1);
	}

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS29Device:
			if (_vm->_gnap->_actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS29Platypus:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemJoint) {
					_vm->gnapUseJointOnPlatypus();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapImpossible(0, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapMoan1(_vm->_plat->_pos.x, _vm->_plat->_pos.y);
						break;
					case GRAB_CURSOR:
						_vm->gnapKissPlatypus(0);
						break;
					case TALK_CURSOR:
						_vm->playGnapBrainPulsating(_vm->_plat->_pos.x, _vm->_plat->_pos.y);
						_vm->playPlatypusSequence(_vm->getPlatypusSequenceId());
						break;
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS29Monkey:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemBanana) {
					_vm->_gnap->_idleFacing = kDirBottomRight;
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS29Monkey].x, _vm->_hotspotsWalkPos[kHS29Monkey].y, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
					_vm->_gnap->_actionStatus = kAS29UseBananaWithMonkey;
					_vm->_newSceneNum = 51;
					_vm->_isLeavingScene = true;
					_vm->setGrabCursorSprite(-1);
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHS29Monkey].x, _vm->_hotspotsWalkPos[kHS29Monkey].y, 5, 6);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(5, 6);
						break;
					case TALK_CURSOR:
						_vm->playGnapBrainPulsating(_vm->_hotspotsWalkPos[kHS29Monkey].x, _vm->_hotspotsWalkPos[kHS29Monkey].y);
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS29ExitCircus:
			if (_vm->_gnap->_actionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 26;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS29ExitCircus].x, _vm->_hotspotsWalkPos[kHS29ExitCircus].y, 0, 0x107AE, 1);
				_vm->_gnap->_actionStatus = kAS29LeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS29ExitCircus].x + 1, _vm->_hotspotsWalkPos[kHS29ExitCircus].y, -1, -1, 1);
			}
			break;

		case kHS29ExitOutsideClown:
			if (_vm->_gnap->_actionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 27;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS29ExitOutsideClown].x, _vm->_hotspotsWalkPos[kHS29ExitOutsideClown].y, 0, 0x107AB, 1);
				_vm->_gnap->_actionStatus = kAS29LeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS29ExitOutsideClown].x, _vm->_hotspotsWalkPos[kHS29ExitOutsideClown].y - 1, -1, 0x107CD, 1);
			}
			break;

		case kHS29Aracde:
			if (_vm->_gnap->_actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemDiceQuarterHole) {
					_vm->setGrabCursorSprite(-1);
					_vm->_isLeavingScene = true;
					_vm->_newSceneNum = 52;
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS29Aracde].x, _vm->_hotspotsWalkPos[kHS29Aracde].y, 0, -1, 1);
					_vm->playGnapIdle(_vm->_hotspotsWalkPos[kHS29Aracde].x, _vm->_hotspotsWalkPos[kHS29Aracde].y);
					_vm->_gnap->_actionStatus = kAS29LeaveScene;
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHS29Aracde].x, _vm->_hotspotsWalkPos[kHS29Aracde].y, 2, 3);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapMoan2(0, 0);
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

		case kHS29WalkArea1:
			if (_vm->_gnap->_actionStatus < 0)
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

		if (!_vm->isSoundPlaying(0x1093B))
			_vm->playSound(0x1093B, true);

		if (!_vm->_isLeavingScene) {
			if (_vm->_gnap->_actionStatus < 0) {
				_vm->updateGnapIdleSequence();
				_vm->updatePlatypusIdleSequence();
			}
			if (!_vm->_timers[4]) {
				if (_vm->invHas(kItemHorn)) {
					_vm->_timers[4] = _vm->getRandom(20) + 60;
					if (_vm->_gnap->_actionStatus < 0) {
						switch (_vm->getRandom(5)) {
						case 0:
							_nextManSequenceId = 0xED;
							break;
						case 1:
							_nextManSequenceId = 0xEE;
							break;
						case 2:
							_nextManSequenceId = 0xEF;
							break;
						case 3:
							_nextManSequenceId = 0xF0;
							break;
						case 4:
							_nextManSequenceId = 0xF1;
							break;
						}
					}
				}
			}
			_vm->playSoundB();
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

void Scene29::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (_vm->_gnap->_actionStatus) {
		case kAS29UseBananaWithMonkey:
			_nextMonkeySequenceId = 0xE5;
			break;
		case kAS29LeaveScene:
			_vm->_sceneDone = true;
			break;
		}
	}

	if (gameSys.getAnimationStatus(3) == 2 && _nextManSequenceId != -1) {
		gameSys.insertSequence(_nextManSequenceId, 39, _currManSequenceId, 39, kSeqSyncWait, 0, 0, 0);
		gameSys.setAnimation(_nextManSequenceId, 39, 3);
		_currManSequenceId = _nextManSequenceId;
		_nextManSequenceId = -1;
	}

	if (gameSys.getAnimationStatus(4) == 2) {
		if (_nextMonkeySequenceId == 0xE5) {
			gameSys.insertSequence(0xF2, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapSequenceId = 0xF2;
			gameSys.setAnimation(0xE6, 159, 0);
			gameSys.setAnimation(0, 159, 4);
			gameSys.insertSequence(_nextMonkeySequenceId, 159, _currMonkeySequenceId, 159, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0xE6, 159, _nextMonkeySequenceId, 159, kSeqSyncWait, 0, 0, 0);
			_vm->_gnap->_actionStatus = kAS29LeaveScene;
			_currMonkeySequenceId = 0xE6;
			_nextMonkeySequenceId = -1;
			_vm->_timers[5] = 30;
			while (_vm->_timers[5])
				_vm->gameUpdateTick();
			_vm->platypusWalkTo(0, 8, 1, 0x107CF, 1);
			while (gameSys.getAnimationStatus(1) != 2)
				_vm->gameUpdateTick();
		} else if (_nextMonkeySequenceId == -1) {
			switch (_vm->getRandom(6)) {
			case 0:
				_nextMonkeySequenceId = 0xE8;
				break;
			case 1:
				_nextMonkeySequenceId = 0xE9;
				break;
			case 2:
				_nextMonkeySequenceId = 0xEA;
				break;
			case 3:
				_nextMonkeySequenceId = 0xEB;
				break;
			case 4:
				_nextMonkeySequenceId = 0xEC;
				break;
			case 5:
				_nextMonkeySequenceId = 0xE7;
				break;
			}
			gameSys.insertSequence(_nextMonkeySequenceId, 159, _currMonkeySequenceId, 159, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(_nextMonkeySequenceId, 159, 4);
			_currMonkeySequenceId = _nextMonkeySequenceId;
			_nextMonkeySequenceId = -1;
		} else {
			gameSys.insertSequence(_nextMonkeySequenceId, 159, _currMonkeySequenceId, 159, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(_nextMonkeySequenceId, 159, 4);
			_currMonkeySequenceId = _nextMonkeySequenceId;
			_nextMonkeySequenceId = -1;
		}
	}
}

} // End of namespace Gnap
