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
#include "gnap/scenes/scene31.h"

namespace Gnap {

enum {
	kHSPlatypus			= 0,
	kHSMeasuringClown	= 1,
	kHSBeerBarrel		= 2,
	kHSDevice			= 3,
	kHSExitCircus		= 4,
	kHSExitOutsideClown	= 5,
	kHSWalkArea1		= 6
};

enum {
	kASUseBeerBarrel				= 1,
	kASFillEmptyBucketWithBeer		= 2,
	kASFillEmptyBucketWithBeerDone	= 3,
	kASPlatMeasuringClown			= 4,
	kASUseMeasuringClown			= 5,
	kASLeaveScene					= 6
};

Scene31::Scene31(GnapEngine *vm) : Scene(vm) {
	_s31_beerGuyDistracted = false;
	_s31_currClerkSequenceId = -1;
	_s31_nextClerkSequenceId = -1;
	_s31_clerkMeasureCtr = -1;
	_s31_clerkMeasureMaxCtr = 3;
}

int Scene31::init() {
	return 0x105;
}

void Scene31::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSMeasuringClown, 34, 150, 256, 436, SF_WALKABLE | SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 0, 6);
	_vm->setHotspot(kHSBeerBarrel, 452, 182, 560, 306, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 7);
	_vm->setHotspot(kHSExitCircus, 150, 585, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
	_vm->setHotspot(kHSExitOutsideClown, 0, 0, 15, 600, SF_EXIT_L_CURSOR | SF_WALKABLE, 0, 8);
	_vm->setHotspot(kHSWalkArea1, 0, 0, 800, 480);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_vm->_hotspotsCount = 7;
}

void Scene31::run() {
	_vm->playSound(0x1093B, true);
	_vm->startSoundTimerB(6);

	_vm->queueInsertDeviceIcon();

	_s31_beerGuyDistracted = false;
	_vm->_gameSys->insertSequence(0xFB, 39, 0, 0, kSeqNone, 0, 0, 0);

	_s31_currClerkSequenceId = 0xFB;
	_s31_nextClerkSequenceId = -1;

	_vm->_gameSys->setAnimation(0xFB, 39, 3);

	_vm->_timers[4] = _vm->getRandom(20) + 60;
	_vm->_timers[5] = _vm->getRandom(50) + 180;

	if (_vm->_prevSceneNum == 27) {
		_vm->initGnapPos(-1, 8, kDirBottomLeft);
		_vm->initPlatypusPos(-1, 9, kDirUnk4);
		_vm->endSceneInit();
		_vm->gnapWalkTo(3, 8, -1, 0x107BA, 1);
		_vm->platypusWalkTo(3, 9, -1, 0x107D2, 1);
	} else {
		_vm->initGnapPos(7, 12, kDirBottomRight);
		_vm->initPlatypusPos(6, 12, kDirNone);
		_vm->endSceneInit();
		_vm->gnapWalkTo(7, 8, -1, 0x107BA, 1);
		_vm->platypusWalkTo(6, 8, -1, 0x107D2, 1);
	}

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHSDevice:
			if (_vm->_gnapActionStatus < 0 || _vm->_gnapActionStatus == kASPlatMeasuringClown) {
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

		case kHSMeasuringClown:
			if (_vm->_gnapActionStatus < 0 || _vm->_gnapActionStatus == kASPlatMeasuringClown) {
				if (_vm->_gnapActionStatus == kASPlatMeasuringClown) {
					if (_vm->_verbCursor == LOOK_CURSOR)
						_vm->playGnapScratchingHead(2, 2);
					else
						_vm->playGnapImpossible(0, 0);
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHSMeasuringClown].x, _vm->_hotspotsWalkPos[kHSMeasuringClown].y + 1, 2, 2);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(2, 2);
						break;
					case GRAB_CURSOR:
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSMeasuringClown].x, _vm->_hotspotsWalkPos[kHSMeasuringClown].y + 1, -1, -1, 1);
						_vm->_hotspots[kHSWalkArea1]._flags |= SF_WALKABLE;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSMeasuringClown].x, _vm->_hotspotsWalkPos[kHSMeasuringClown].y, 0, 0x107B9, 1);
						_vm->_hotspots[kHSWalkArea1]._flags &= ~SF_WALKABLE;
						_vm->_gnapActionStatus = kASUseMeasuringClown;
						_vm->_timers[4] = 300;
						break;
					case TALK_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					case PLAT_CURSOR:
						if (!_vm->invHas(kItemBucketWithBeer)) {
							_vm->gnapUseDeviceOnPlatypuss();
							_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSMeasuringClown].x, _vm->_hotspotsWalkPos[kHSMeasuringClown].y + 1, 1, 0x107C2, 1);
							_vm->_hotspots[kHSWalkArea1]._flags |= SF_WALKABLE;
							_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSMeasuringClown].x, _vm->_hotspotsWalkPos[kHSMeasuringClown].y, 1, 0x107C2, 1);
							_vm->_hotspots[kHSWalkArea1]._flags &= ~SF_WALKABLE;
							_vm->_platypusActionStatus = kASPlatMeasuringClown;
							_vm->_gnapActionStatus = kASPlatMeasuringClown;
							_vm->_timers[4] = 300;
						} else
							_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSBeerBarrel:
			if (_vm->_gnapActionStatus < 0 || _vm->_gnapActionStatus == kASPlatMeasuringClown) {
				if (_vm->_grabCursorSpriteIndex == kItemEmptyBucket && _s31_beerGuyDistracted) {
					_vm->setGrabCursorSprite(-1);
					_vm->gnapWalkTo(_vm->_gnapX, _vm->_gnapY, -1, _vm->getGnapSequenceId(gskIdle, _vm->_hotspotsWalkPos[kHSBeerBarrel].x, _vm->_hotspotsWalkPos[kHSBeerBarrel].y) | 0x10000, 1);
					_s31_clerkMeasureMaxCtr += 5;
					_vm->_gameSys->insertSequence(0xF8, 59, 0, 0, kSeqNone, 0, 0, 0);
					_vm->playGnapPullOutDevice(6, 8);
					_vm->playGnapUseDevice(0, 0);
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSBeerBarrel].x, _vm->_hotspotsWalkPos[kHSBeerBarrel].y, 0, 0x107BC, 1);
					_vm->_gnapActionStatus = kASFillEmptyBucketWithBeer;
					_vm->_timers[4] = 300;
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHSBeerBarrel].x, _vm->_hotspotsWalkPos[kHSBeerBarrel].y, 6, 2);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(6, 2);
						break;
					case GRAB_CURSOR:
						if (_s31_beerGuyDistracted) {
							_vm->playGnapScratchingHead(6, 2);
						} else {
							_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSBeerBarrel].x, _vm->_hotspotsWalkPos[kHSBeerBarrel].y, 0, 0x107BC, 1);
							_vm->_gnapActionStatus = kASUseBeerBarrel;
							_vm->_gnapIdleFacing = kDirUpLeft;
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

		case kHSExitCircus:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 26;
				_vm->gnapWalkTo(-1, _vm->_hotspotsWalkPos[kHSExitCircus].y, 0, 0x107AE, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitCircus].x + 1, _vm->_hotspotsWalkPos[kHSExitCircus].y, -1, -1, 1);
			}
			break;

		case kHSExitOutsideClown:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 27;
				_vm->gnapWalkTo(-1, _vm->_hotspotsWalkPos[kHSExitOutsideClown].y, 0, 0x107AF, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitOutsideClown].x, _vm->_hotspotsWalkPos[kHSExitOutsideClown].y + 1, -1, 0x107CF, 1);
			}
			break;

		case kHSWalkArea1:
			if (_vm->_gnapActionStatus < 0)
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
			if (_vm->_platypusActionStatus < 0)
				_vm->updatePlatypusIdleSequence();
			if (_vm->_gnapActionStatus < 0)
				_vm->updateGnapIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(20) + 60;
				if (_vm->_gnapActionStatus < 0 && _s31_nextClerkSequenceId == -1) {
					switch (_vm->getRandom(6)){
					case 0:
						_s31_nextClerkSequenceId = 0xFF;
						break;
					case 1:
						_s31_nextClerkSequenceId = 0x100;
						break;
					case 2:
						_s31_nextClerkSequenceId = 0x101;
						break;
					default:
						_s31_nextClerkSequenceId = 0xFB;
						break;
					}
				}
			}
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(50) + 180;
				if (_vm->_gnapActionStatus < 0) {
					if (_vm->getRandom(2) != 0)
						_vm->_gameSys->insertSequence(0x104, 20, 0, 0, kSeqNone, 0, 0, 0);
					else
						_vm->_gameSys->insertSequence(0x103, 20, 0, 0, kSeqNone, 0, 0, 0);
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

void Scene31::updateAnimations() {
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kASUseBeerBarrel:
			_s31_nextClerkSequenceId = 0xFE;
			break;
		case kASFillEmptyBucketWithBeer:
			_vm->_gameSys->setAnimation(0x102, 59, 0);
			_vm->_gameSys->insertSequence(0x102, 59, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapX = 5;
			_vm->_gnapY = 7;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapSequenceId = 0x102;
			_vm->_gnapId = 59;
			_vm->_gnapActionStatus = kASFillEmptyBucketWithBeerDone;
			break;
		case kASFillEmptyBucketWithBeerDone:
			_vm->_gnapIdleFacing = kDirBottomLeft;
			_vm->playGnapPullOutDevice(0, 0);
			_vm->playGnapUseDevice(0, 0);
			_vm->_gameSys->insertSequence(0xF9, 59, 0xF8, 59, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapActionStatus = -1;
			_vm->invAdd(kItemBucketWithBeer);
			_vm->invRemove(kItemEmptyBucket);
			_vm->setGrabCursorSprite(kItemBucketWithBeer);
			break;
		case kASUseMeasuringClown:
			_s31_nextClerkSequenceId = 0xFA;
			_s31_clerkMeasureMaxCtr = 1;
			break;
		case kASLeaveScene:
			_vm->_sceneDone = true;
			_vm->_gnapActionStatus = -1;
			break;
		}
	}

	if (_vm->_gameSys->getAnimationStatus(1) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 1);
		if (_vm->_platypusActionStatus == kASPlatMeasuringClown) {
			_vm->_sceneWaiting = true;
			_s31_beerGuyDistracted = true;
			_s31_nextClerkSequenceId = 0xFA;
		}
	}

	if (_vm->_gameSys->getAnimationStatus(3) == 2) {
		switch (_s31_nextClerkSequenceId) {
		case 0xFA:
			_vm->_gameSys->insertSequence(_s31_nextClerkSequenceId, 39, _s31_currClerkSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0xFC, 39, _s31_nextClerkSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(0xFC, 39, 3);
			_s31_currClerkSequenceId = 0xFC;
			_s31_nextClerkSequenceId = 0xFC;
			_s31_clerkMeasureCtr = 0;
			break;
		case 0xFC:
			++_s31_clerkMeasureCtr;
			if (_s31_clerkMeasureCtr >= _s31_clerkMeasureMaxCtr) {
				if (_vm->_gnapActionStatus != 5)
					_vm->_platypusActionStatus = -1;
				_vm->_timers[0] = 40;
				_vm->_gameSys->insertSequence(0xFD, 39, _s31_currClerkSequenceId, 39, kSeqSyncWait, 0, 0, 0);
				_s31_currClerkSequenceId = 0xFD;
				_s31_nextClerkSequenceId = -1;
				if (_vm->_gnapActionStatus != kASFillEmptyBucketWithBeerDone && _vm->_gnapActionStatus != kASFillEmptyBucketWithBeer)
					_vm->_gnapActionStatus = -1;
				_s31_beerGuyDistracted = false;
				_s31_clerkMeasureMaxCtr = 3;
				_vm->_gameSys->setAnimation(0xFD, 39, 3);
				_vm->_sceneWaiting = false;
			} else {
				_vm->_gameSys->insertSequence(_s31_nextClerkSequenceId, 39, _s31_currClerkSequenceId, 39, kSeqSyncWait, 0, 0, 0);
				_s31_currClerkSequenceId = _s31_nextClerkSequenceId;
				_s31_nextClerkSequenceId = 0xFC;
				_vm->_gameSys->setAnimation(0xFC, 39, 3);
			}
			break;
		case 0xFE:
			_vm->_gameSys->insertSequence(_s31_nextClerkSequenceId, 39, _s31_currClerkSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(_s31_nextClerkSequenceId, 39, 3);
			_s31_currClerkSequenceId = _s31_nextClerkSequenceId;
			_s31_nextClerkSequenceId = -1;
			_vm->_gnapActionStatus = -1;
			break;
		default:
			if (_s31_nextClerkSequenceId != -1) {
				_vm->_gameSys->insertSequence(_s31_nextClerkSequenceId, 39, _s31_currClerkSequenceId, 39, kSeqSyncWait, 0, 0, 0);
				_vm->_gameSys->setAnimation(_s31_nextClerkSequenceId, 39, 3);
				_s31_currClerkSequenceId = _s31_nextClerkSequenceId;
				_s31_nextClerkSequenceId = -1;
			}
			break;
		}
	}
}

} // End of namespace Gnap
