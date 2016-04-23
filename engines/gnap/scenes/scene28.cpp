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
#include "gnap/scenes/scene28.h"

namespace Gnap {

enum {
	kHSPlatypus			= 0,
	kHSHorn				= 1,
	kHSClown			= 2,
	kHSExitOutsideClown	= 3,
	kHSEmptyBucket		= 4,
	kHSDevice			= 5,
	kHSWalkArea1		= 6,
	kHSWalkArea2		= 7
};

enum {
	kASUseBeerBucketWithClown	= 0,
	kASUsePillBucketWithClown	= 1,
	kASGrabHornFails			= 2,
	kASGrabEmptyBucket			= 3,
	kASGrabHornSuccess			= 4,
	kASGrabHornSuccessDone		= 5,
	kASGrabEmptyBucketDone		= 6,
	kASGrabHornFailsDone		= 7,
	kASTalkClown				= 8,
	kASGnapWaiting				= 9,
	kASLeaveScene				= 10
};

Scene28::Scene28(GnapEngine *vm) : Scene(vm) {
	_s28_currClownSequenceId = -1;
	_s28_nextClownSequenceId = -1;
	_s28_clownTalkCtr = 0;
}

int Scene28::init() {
	return 0x125;
}

void Scene28::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSHorn, 148, 352, 215, 383, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 1, 7);
	_vm->setHotspot(kHSClown, 130, 250, 285, 413, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 5);
	_vm->setHotspot(kHSExitOutsideClown, 660, 190, 799, 400, SF_EXIT_R_CURSOR, 9, 6);
	_vm->setHotspot(kHSEmptyBucket, 582, 421, 643, 478, SF_WALKABLE | SF_DISABLED, 9, 7);
	_vm->setHotspot(kHSWalkArea1, 0, 0, 799, 523);
	_vm->setHotspot(kHSWalkArea2, 0, 0, 0, 0, 7, SF_DISABLED);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	if (_vm->invHas(kItemHorn))
		_vm->_hotspots[kHSHorn]._flags = SF_DISABLED;
	if (_vm->isFlag(kGFUnk22))
		_vm->_hotspots[kHSEmptyBucket]._flags = SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	_vm->_hotspotsCount = 8;
}

void Scene28::run() {
	_vm->playSound(0x1093C, true);
	_s28_nextClownSequenceId = -1;
	_vm->queueInsertDeviceIcon();
	_vm->_gameSys->insertSequence(0x124, 255, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->isFlag(kGFUnk22))
		_vm->_gameSys->insertSequence(0x112, 99, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->isFlag(kGFMudTaken)) {
		if (_vm->isFlag(kGFUnk21)) {
			_vm->_gameSys->setAnimation(0x11C, 39, 3);
			_vm->_gameSys->insertSequence(0x11C, 39, 0, 0, kSeqNone, 0, 0, 0);
			if (!_vm->invHas(kItemHorn))
				_vm->_gameSys->insertSequence(0x118, 59, 0, 0, kSeqNone, 0, 0, 0);
			_s28_currClownSequenceId = 0x11C;
		} else {
			_s28_currClownSequenceId = 0x11B;
			_vm->_gameSys->setAnimation(0x11B, 39, 3);
			_vm->_gameSys->insertSequence(_s28_currClownSequenceId, 39, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_timers[4] = _vm->getRandom(20) + 80;
		}
		_vm->initGnapPos(8, 8, kDirBottomLeft);
		_vm->initPlatypusPos(9, 8, kDirUnk4);
		_vm->endSceneInit();
	} else {
		_vm->_gameSys->insertSequence(0x11B, 39, 0, 0, kSeqNone, 0, 0, 0);
		_vm->initGnapPos(8, 8, kDirBottomLeft);
		_vm->initPlatypusPos(9, 8, kDirUnk4);
		_vm->endSceneInit();
		_vm->playSequences(0xF7, 0x121, 0x122, 0x123);
		_s28_currClownSequenceId = 0x115;
		_vm->setFlag(kGFMudTaken);
		_vm->_gameSys->setAnimation(0x115, 39, 3);
		_vm->_gameSys->insertSequence(_s28_currClownSequenceId, 39, 0x11B, 39, kSeqSyncWait, 0, 0, 0);
		_s28_nextClownSequenceId = -1;
		_vm->_timers[4] = _vm->getRandom(20) + 80;
		_vm->_gnapActionStatus = kASGnapWaiting;
		while (_vm->_gameSys->getAnimationStatus(3) != 2) {
			_vm->gameUpdateTick();
			_vm->updateMouseCursor();
		}
		_vm->_gnapActionStatus = -1;
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
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSHorn:
			if (_vm->_gnapActionStatus < 0) {
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
								_vm->gnapWalkTo(_vm->_gnapX, _vm->_gnapY, 0, _vm->getGnapSequenceId(gskIdle, _vm->_hotspotsWalkPos[kHSHorn].x, _vm->_hotspotsWalkPos[kHSHorn].y) | 0x10000, 1);
								_vm->_gnapActionStatus = kASGrabHornSuccess;
							}
						} else {
							_vm->_gnapIdleFacing = kDirUpLeft;
							_vm->gnapWalkTo(2, 8, 0, 0x107BB, 1);
							_vm->_hotspots[kHSWalkArea1]._flags |= SF_WALKABLE;
							_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSHorn].x, _vm->_hotspotsWalkPos[kHSHorn].y, 0, 0x107BB, 1);
							_vm->_hotspots[kHSWalkArea1]._flags &= ~SF_WALKABLE;
							_vm->_gnapActionStatus = kASGrabHornFails;
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

		case kHSClown:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->isFlag(kGFUnk21)) {
					if (_vm->_verbCursor == LOOK_CURSOR)
						_vm->playGnapScratchingHead(5, 2);
					else
						_vm->playGnapImpossible(0, 0);
				} else if (_vm->_grabCursorSpriteIndex == kItemBucketWithBeer) {
					_vm->_gnapIdleFacing = kDirUpLeft;
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSClown].x, _vm->_hotspotsWalkPos[kHSClown].y, 0, 0x107BC, 1);
					_vm->playGnapPullOutDevice(0, 0);
					_vm->playGnapUseDevice(0, 0);
					_vm->_gnapActionStatus = kASUseBeerBucketWithClown;
				} else if (_vm->_grabCursorSpriteIndex == kItemBucketWithPill) {
					_vm->_gnapIdleFacing = kDirUpLeft;
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSClown].x, _vm->_hotspotsWalkPos[kHSClown].y, 0, 0x107BC, 1);
					_vm->playGnapPullOutDevice(0, 0);
					_vm->playGnapUseDevice(0, 0);
					_vm->_gnapActionStatus = kASUsePillBucketWithClown;
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHSClown].x, _vm->_hotspotsWalkPos[kHSClown].y, 2, 4);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(5, 2);
						break;
					case TALK_CURSOR:
						_vm->_gnapIdleFacing = kDirUpLeft;
						_vm->gnapWalkTo(5, 8, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_vm->_gnapActionStatus = kASTalkClown;
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSExitOutsideClown:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 27;
				_vm->_hotspots[kHSWalkArea1]._flags |= SF_WALKABLE;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitOutsideClown].x, _vm->_hotspotsWalkPos[kHSExitOutsideClown].y, 0, 0x107BF, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->_hotspots[kHSWalkArea1]._flags &= ~SF_WALKABLE;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitOutsideClown].x - 1, _vm->_hotspotsWalkPos[kHSExitOutsideClown].y, -1, 0x107C2, 1);
			}
			break;

		case kHSEmptyBucket:
			if (_vm->_gnapActionStatus < 0) {
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
						_vm->gnapWalkTo(_vm->_gnapX, _vm->_gnapY, 0, _vm->getGnapSequenceId(gskIdle, _vm->_hotspotsWalkPos[kHSEmptyBucket].x, _vm->_hotspotsWalkPos[kHSEmptyBucket].y) | 0x10000, 1);
						_vm->_gnapActionStatus = kASGrabEmptyBucket;
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

		if (!_vm->isSoundPlaying(0x1093C))
			_vm->playSound(0x1093C, true);

		if (!_vm->_isLeavingScene) {
			_vm->updatePlatypusIdleSequence();
			_vm->updateGnapIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(20) + 80;
				if (_vm->_gnapActionStatus < 0 && !_vm->isFlag(kGFUnk21))
					_s28_nextClownSequenceId = 0x114;
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
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kASUseBeerBucketWithClown:
			_vm->setFlag(kGFUnk22);
			_s28_nextClownSequenceId = 0x113;
			_vm->invRemove(kItemBucketWithBeer);
			updateHotspots();
			break;
		case kASUsePillBucketWithClown:
			_s28_nextClownSequenceId = 0x116;
			_vm->invRemove(kItemBucketWithPill);
			_vm->setFlag(kGFUnk22);
			_vm->setFlag(kGFUnk21);
			updateHotspots();
			break;
		case kASGrabHornFails:
			_s28_nextClownSequenceId = 0x119;
			break;
		case kASGrabHornSuccess:
			_vm->playGnapPullOutDevice(0, 0);
			_vm->playGnapUseDevice(0, 0);
			_vm->_gameSys->setAnimation(0x117, 59, 0);
			_vm->_gameSys->insertSequence(0x117, 59, 280, 59, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapActionStatus = kASGrabHornSuccessDone;
			break;
		case kASGrabHornSuccessDone:
			_vm->hideCursor();
			_vm->setGrabCursorSprite(-1);
			_vm->addFullScreenSprite(0xF6, 255);
			_vm->_gameSys->setAnimation(0x120, 256, 0);
			_vm->_gameSys->insertSequence(0x120, 256, 0, 0, kSeqNone, 0, 0, 0);
			while (_vm->_gameSys->getAnimationStatus(0) != 2)
				_vm->gameUpdateTick();
			_vm->removeFullScreenSprite();
			_vm->showCursor();
			_vm->setGrabCursorSprite(kItemHorn);
			_vm->invAdd(kItemHorn);
			updateHotspots();
			_vm->_gnapActionStatus = -1;
			break;
		case kASGrabEmptyBucket:
			_vm->playGnapPullOutDevice(0, 0);
			_vm->playGnapUseDevice(0, 0);
			_vm->_gameSys->setAnimation(0x111, 99, 0);
			_vm->_gameSys->insertSequence(0x111, 99, 274, 99, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapActionStatus = kASGrabEmptyBucketDone;
			break;
		case kASGrabEmptyBucketDone:
			_vm->setGrabCursorSprite(kItemEmptyBucket);
			_vm->clearFlag(kGFUnk22);;
			updateHotspots();
			_vm->invAdd(kItemEmptyBucket);
			_vm->_gnapActionStatus = -1;
			break;
		case kASGrabHornFailsDone:
			_vm->_gameSys->insertSequence(0x107B5, _vm->_gnapId, 281, 39, kSeqSyncWait, 0, 75 * _vm->_gnapX - _vm->_gnapGridX, 48 * _vm->_gnapY - _vm->_gnapGridY);
			_vm->_gnapSequenceId = 0x7B5;
			_vm->_gnapSequenceDatNum = 1;
			_vm->_gameSys->insertSequence(0x11B, 39, 0, 0, kSeqNone, 0, 0, 0);
			_s28_currClownSequenceId = 0x11B;
			_s28_nextClownSequenceId = -1;
			_vm->_gnapActionStatus = -1;
			_vm->gnapWalkTo(2, 8, -1, 0x107BB, 1);
			break;
		case kASTalkClown:
			_s28_clownTalkCtr = (_s28_clownTalkCtr + 1) % 2;
			if (_s28_clownTalkCtr == 0)
				_s28_nextClownSequenceId = 0x11D;
			else if (_s28_clownTalkCtr == 1)
				_s28_nextClownSequenceId = 0x11E;
			else if (_s28_clownTalkCtr == 2)
				_s28_nextClownSequenceId = 0x11F; // NOTE CHECKME Never set, bug in the original?
			break;
		case kASGnapWaiting:
			_vm->_gnapActionStatus = -1;
			break;
		case kASLeaveScene:
			_vm->_sceneDone = true;
			break;
		}
	}

	if (_vm->_gameSys->getAnimationStatus(3) == 2) {
		switch (_s28_nextClownSequenceId) {
		case 0x113:
			_vm->setGrabCursorSprite(-1);
			_vm->_gameSys->setAnimation(_s28_nextClownSequenceId, 39, 0);
			_vm->_gameSys->insertSequence(0x112, 99, 0, 0, kSeqNone, _vm->getSequenceTotalDuration(_s28_nextClownSequenceId), 0, 0);
			_vm->_gameSys->insertSequence(_s28_nextClownSequenceId, 39, _s28_currClownSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x11B, 39, _s28_nextClownSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			_s28_currClownSequenceId = 0x11B;
			_s28_nextClownSequenceId = -1;
			_vm->_gnapActionStatus = kASGnapWaiting;
			break;
		case 0x116:
			_vm->setGrabCursorSprite(-1);
			_vm->_gameSys->setAnimation(_s28_nextClownSequenceId, 39, 0);
			_vm->_gameSys->insertSequence(0x112, 99, 0, 0, kSeqNone, _vm->getSequenceTotalDuration(_s28_nextClownSequenceId), 0, 0);
			_vm->_gameSys->insertSequence(_s28_nextClownSequenceId, 39, _s28_currClownSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x11C, 39, _s28_nextClownSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x118, 59, 0, 0, kSeqNone, _vm->getSequenceTotalDuration(_s28_nextClownSequenceId), 0, 0);
			_s28_currClownSequenceId = _s28_nextClownSequenceId;
			_s28_nextClownSequenceId = -1;
			_vm->_gnapActionStatus = kASGnapWaiting;
			break;
		case 0x11D:
		case 0x11E:
		case 0x11F:
			_vm->_gnapActionStatus = -1;
			break;
		case 0x119:
			_vm->_gameSys->insertSequence(_s28_nextClownSequenceId, 39, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(_s28_nextClownSequenceId, 39, 0);
			_vm->_gameSys->removeSequence(_s28_currClownSequenceId, 39, true);
			_vm->_gnapActionStatus = kASGrabHornFailsDone;
			_vm->_gnapSequenceId = _s28_nextClownSequenceId;
			_vm->_gnapSequenceDatNum = 0;
			_s28_nextClownSequenceId = -1;
			break;
		}
		if (_s28_nextClownSequenceId != -1) {
			_vm->_gameSys->insertSequence(_s28_nextClownSequenceId, 39, _s28_currClownSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(_s28_nextClownSequenceId, 39, 3);
			_s28_currClownSequenceId = _s28_nextClownSequenceId;
			_s28_nextClownSequenceId = -1;
		}
	}
}

} // End of namespace Gnap
