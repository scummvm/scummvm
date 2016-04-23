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
#include "gnap/scenes/scene21.h"

namespace Gnap {

enum {
	kHSPlatypus				= 0,
	kHSBanana				= 1,
	kHSOldLady				= 2,
	kHSDevice				= 3,
	kHSExitOutsideGrubCity	= 4,
	kHSWalkArea1			= 5,
	kHSWalkArea2			= 6
};

enum {
	kASTalkOldLady				= 0,
	kASGrabBanana				= 1,
	kASGrabBananaDone			= 2,
	kASGrabOldLady				= 3,
	kASUseHatWithOldLady		= 4,
	kASUseHatWithOldLadyDone	= 5,
	kASLeaveScene				= 6
};

Scene21::Scene21(GnapEngine *vm) : Scene(vm) {
	_s21_currOldLadySequenceId = -1;
	_s21_nextOldLadySequenceId = -1;
}

int Scene21::init() {
	_vm->_gameSys->setAnimation(0, 0, 3);
	return _vm->isFlag(kGFTwigTaken) ? 0x94 : 0x93;
}

void Scene21::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSBanana, 94, 394, 146, 430, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 2, 6);
	_vm->setHotspot(kHSOldLady, 402, 220, 528, 430, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 4, 7);
	_vm->setHotspot(kHSExitOutsideGrubCity, 522, 498, 800, 600, SF_EXIT_SE_CURSOR | SF_WALKABLE, 5, 10);
	_vm->setHotspot(kHSWalkArea1, 0, 0, 800, 440);
	_vm->setHotspot(kHSWalkArea2, 698, 0, 800, 600);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	if (_vm->isFlag(kGFUnk04) || !_vm->isFlag(kGFTwigTaken))
		_vm->_hotspots[kHSBanana]._flags = SF_WALKABLE | SF_DISABLED;
	if (_vm->isFlag(kGFTwigTaken))
		_vm->_hotspots[kHSOldLady]._flags = SF_DISABLED;
	_vm->_hotspotsCount = 7;
}

void Scene21::run() {
	_vm->playSound(0x10940, true);
	_vm->startSoundTimerA(6);
	
	_vm->_timers[5] = _vm->getRandom(100) + 100;
	
	_vm->queueInsertDeviceIcon();
	
	if (_vm->isFlag(kGFTwigTaken)) {
		if (_vm->isFlag(kGFKeysTaken)) {
			_vm->initGnapPos(5, 8, kDirBottomRight);
			_vm->initPlatypusPos(6, 8, kDirNone);
			_vm->_gameSys->insertSequence(0x8E, 2, 0, 0, kSeqNone, 0, 0, 0);
			if (!_vm->isFlag(kGFUnk04))
				_vm->_gameSys->insertSequence(0x8D, 59, 0, 0, kSeqNone, 0, 0, 0);
			_vm->endSceneInit();
			_vm->clearFlag(kGFKeysTaken);
		} else {
			_vm->initGnapPos(5, 11, kDirBottomRight);
			_vm->initPlatypusPos(6, 11, kDirNone);
			if (!_vm->isFlag(kGFUnk04))
				_vm->_gameSys->insertSequence(0x8D, 59, 0, 0, kSeqNone, 0, 0, 0);
			_vm->endSceneInit();
			_vm->gnapWalkTo(5, 8, -1, 0x107B9, 1);
			_vm->platypusWalkTo(6, 8, -1, 0x107C2, 1);
		}
	} else {
		_vm->initGnapPos(5, 11, kDirBottomRight);
		_vm->initPlatypusPos(6, 11, kDirNone);
		_s21_currOldLadySequenceId = 0x89;
		_vm->_gameSys->setAnimation(0x89, 79, 3);
		_vm->_gameSys->insertSequence(_s21_currOldLadySequenceId, 79, 0, 0, kSeqNone, 0, 0, 0);
		_s21_nextOldLadySequenceId = -1;
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

		case kHSBanana:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, 2, 5);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(2, 5);
						break;
					case GRAB_CURSOR:
						_vm->gnapWalkTo(_vm->_gnapX, _vm->_gnapY, 0, _vm->getGnapSequenceId(gskIdle, _vm->_hotspotsWalkPos[kHSBanana].x, _vm->_hotspotsWalkPos[kHSBanana].y) | 0x10000, 1);
						_vm->playGnapPullOutDevice(2, 5);
						_vm->playGnapUseDevice(0, 0);
						_vm->_gnapActionStatus = kASGrabBanana;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						break;
					}
				}
			}
			break;

		case kHSOldLady:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemGroceryStoreHat) {
					_vm->_newSceneNum = 47;
					_vm->gnapWalkTo(4, 6, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
					_vm->_gnapActionStatus = kASUseHatWithOldLady;
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(4, 6, 7, 4);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(7, 4);
						break;
					case GRAB_CURSOR:
						_vm->_gnapIdleFacing = kDirUpLeft;
						_vm->_hotspots[kHSWalkArea1]._flags |= SF_WALKABLE;
						_vm->gnapWalkTo(7, 6, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
						_vm->_gnapActionStatus = kASGrabOldLady;
						_vm->_hotspots[kHSWalkArea1]._flags &= ~SF_WALKABLE;
						break;
					case TALK_CURSOR:
						_vm->_gnapIdleFacing = kDirUpRight;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSOldLady].x, _vm->_hotspotsWalkPos[kHSOldLady].y, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_vm->_gnapActionStatus = kASTalkOldLady;
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
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitOutsideGrubCity].x, _vm->_hotspotsWalkPos[kHSExitOutsideGrubCity].y, 0, 0x107B3, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitOutsideGrubCity].x + 1, _vm->_hotspotsWalkPos[kHSExitOutsideGrubCity].y, -1, 0x107C2, 1);
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
			_vm->updatePlatypusIdleSequence();
			_vm->updateGnapIdleSequence();
			if (!_vm->isFlag(kGFTwigTaken) && !_vm->_timers[4] && _s21_nextOldLadySequenceId == -1 && _vm->_gnapActionStatus == -1) {
				_vm->_timers[4] = _vm->getRandom(30) + 50;
				switch (_vm->getRandom(5)) {
				case 0:
					_s21_nextOldLadySequenceId = 0x88;
					break;
				case 1:
					_s21_nextOldLadySequenceId = 0x8A;
					break;
				default:
					_s21_nextOldLadySequenceId = 0x89;
					break;
				}
			}
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(100) + 100;
				_vm->_gameSys->insertSequence(0x92, 255, 0, 0, kSeqNone, 0, 0, 0);
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
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kASTalkOldLady:
			_s21_nextOldLadySequenceId = 0x8B;
			_vm->_gnapActionStatus = -1;
			break;
		case kASGrabBanana:
			_vm->_gameSys->setAnimation(0x8C, 59, 0);
			_vm->_gameSys->insertSequence(0x8C, 59, 141, 59, kSeqSyncWait, 0, 0, 0);
			_vm->setFlag(kGFUnk04);
			_vm->invAdd(kItemBanana);
			updateHotspots();
			_vm->_gnapActionStatus = kASGrabBananaDone;
			break;
		case kASGrabBananaDone:
			_vm->setGrabCursorSprite(kItemBanana);
			_vm->_gnapActionStatus = -1;
			break;
		case kASGrabOldLady:
			_vm->_timers[4] = _vm->getRandom(30) + 50;
			_s21_nextOldLadySequenceId = 0x87;
			break;
		case kASUseHatWithOldLady:
			_vm->_gameSys->setAnimation(0x8F, _vm->_gnapId, 0);
			_vm->_gameSys->insertSequence(0x8F, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapSequenceId = 0x8F;
			_vm->_gnapActionStatus = kASUseHatWithOldLadyDone;
			_vm->invAdd(kItemTickets);
			_vm->invRemove(kItemGroceryStoreHat);
			_vm->setGrabCursorSprite(-1);
			break;
		case kASUseHatWithOldLadyDone:
			_s21_nextOldLadySequenceId = 0x91;
			break;
		case kASLeaveScene:
			_vm->_sceneDone = true;
			break;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(3) == 2 && _s21_nextOldLadySequenceId != -1) {
		if (_s21_nextOldLadySequenceId == 0x87) {
			_vm->_gameSys->setAnimation(_s21_nextOldLadySequenceId, 79, 3);
			_vm->_gameSys->insertSequence(_s21_nextOldLadySequenceId, 79, _s21_currOldLadySequenceId, 79, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x86, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceId = 0x86;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapActionStatus = -1;
			_s21_currOldLadySequenceId = _s21_nextOldLadySequenceId;
			_s21_nextOldLadySequenceId = -1;
		} else if (_s21_nextOldLadySequenceId == 0x91) {
			_vm->_gameSys->setAnimation(0x91, 79, 0);
			_vm->_gameSys->insertSequence(_s21_nextOldLadySequenceId, 79, _s21_currOldLadySequenceId, 79, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapActionStatus = kASLeaveScene;
			_s21_currOldLadySequenceId = _s21_nextOldLadySequenceId;
			_s21_nextOldLadySequenceId = -1;
		} else {
			_vm->_gameSys->setAnimation(_s21_nextOldLadySequenceId, 79, 3);
			_vm->_gameSys->insertSequence(_s21_nextOldLadySequenceId, 79, _s21_currOldLadySequenceId, 79, kSeqSyncWait, 0, 0, 0);
			_s21_currOldLadySequenceId = _s21_nextOldLadySequenceId;
			_s21_nextOldLadySequenceId = -1;
		}
	}
}

} // End of namespace Gnap
