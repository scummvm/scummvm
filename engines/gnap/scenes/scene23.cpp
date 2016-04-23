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
#include "gnap/scenes/scene23.h"

namespace Gnap {

enum {
	kHSPlatypus				= 0,
	kHSExitFrontGrubCity	= 1,
	kHSDevice				= 2,
	kHSCereals				= 3,
	kHSWalkArea1			= 4,
	kHSWalkArea2			= 5
};

enum {
	kASLookCereals			= 0,
	kASGrabCereals			= 1,
	kASGrabCerealsDone		= 2,
	kASLeaveScene			= 3
};

Scene23::Scene23(GnapEngine *vm) : Scene(vm) {
	_s23_currStoreClerkSequenceId = -1;
	_s23_nextStoreClerkSequenceId = -1;
}

int Scene23::init() {
	return 0xC0;
}

void Scene23::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSExitFrontGrubCity, 0, 250, 15, 550, SF_EXIT_L_CURSOR | SF_WALKABLE, 0, 7);
	_vm->setHotspot(kHSCereals, 366, 332, 414, 408, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 4, 7);
	_vm->setHotspot(kHSWalkArea1, 0, 0, 340, 460);
	_vm->setHotspot(kHSWalkArea2, 340, 0, 800, 501);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_vm->_hotspotsCount = 6;
}

void Scene23::run() {
	_vm->_timers[4] = _vm->getRandom(100) + 200;
	_vm->_timers[5] = _vm->getRandom(100) + 200;
	
	_s23_currStoreClerkSequenceId = 0xB4;
	_s23_nextStoreClerkSequenceId = -1;
	
	_vm->_gameSys->setAnimation(0xB4, 1, 4);
	_vm->_gameSys->insertSequence(_s23_currStoreClerkSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	_vm->queueInsertDeviceIcon();
	
	_vm->initGnapPos(-1, 7, kDirBottomRight);
	_vm->initPlatypusPos(-2, 7, kDirNone);
	_vm->_gameSys->insertSequence(0xBD, 255, 0, 0, kSeqNone, 0, 0, 0);
	_vm->_gameSys->insertSequence(0xBF, 2, 0, 0, kSeqNone, 0, 0, 0);
	_vm->endSceneInit();
	
	_vm->platypusWalkTo(1, 7, -1, 0x107C2, 1);

	if (_vm->isFlag(kGFUnk24)) {
		_vm->gnapWalkTo(2, 7, -1, 0x107B9, 1);
	} else {
		_vm->gnapWalkTo(2, 7, 0, 0x107B9, 1);
		while (_vm->_gameSys->getAnimationStatus(0) != 2)
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
		
		case kHSCereals:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHSCereals].x, _vm->_hotspotsWalkPos[kHSCereals].y, 5, 4);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						if (_vm->isFlag(kGFSceneFlag1))
							_vm->playGnapMoan2(0, 0);
						else {
							_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSCereals].x, _vm->_hotspotsWalkPos[kHSCereals].y,
								0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
							_vm->_gnapActionStatus = kASLookCereals;
						}
						break;
					case GRAB_CURSOR:
						if (_vm->isFlag(kGFSceneFlag1))
							_vm->playGnapImpossible(0, 0);
						else {
							_vm->_gnapIdleFacing = kDirBottomRight;
							_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSCereals].x, _vm->_hotspotsWalkPos[kHSCereals].y,
								0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
							_vm->setFlag(kGFSceneFlag1);
							_vm->_gnapActionStatus = kASGrabCereals;
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
		
		case kHSExitFrontGrubCity:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 22;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitFrontGrubCity].x, _vm->_hotspotsWalkPos[kHSExitFrontGrubCity].y, 0, 0x107AF, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitFrontGrubCity].x, _vm->_hotspotsWalkPos[kHSExitFrontGrubCity].y - 1, -1, 0x107C2, 1);
			}
			break;
		
		case kHSWalkArea1:
		case kHSWalkArea2:
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
	
		if (!_vm->_isLeavingScene) {
			_vm->updatePlatypusIdleSequence();
			_vm->updateGnapIdleSequence();
			if (!_vm->_timers[4] && _vm->_gnapActionStatus == -1) {
				_vm->_timers[4] = _vm->getRandom(100) + 200;
				switch (_vm->getRandom(4)) {
				case 0:
					_vm->_gameSys->insertSequence(0xB7, 256, 0, 0, kSeqNone, 0, 0, 0);
					break;
				case 1:
					_vm->_gameSys->insertSequence(0xB8, 256, 0, 0, kSeqNone, 0, 0, 0);
					break;
				case 2:
				case 3:
					_vm->_gameSys->insertSequence(0xB9, 256, 0, 0, kSeqNone, 0, 0, 0);
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
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kASLookCereals:
			_vm->showFullScreenSprite(0x48);
			_vm->_gnapActionStatus = -1;
			break;
		case kASGrabCereals:
			_vm->_gameSys->setAnimation(0xBE, _vm->_gnapId, 0);
			_vm->_gameSys->insertSequence(0xBE, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->requestRemoveSequence(0xBF, 2);
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapSequenceId = 0xBE;
			_vm->_gnapActionStatus = kASGrabCerealsDone;
			break;
		case kASGrabCerealsDone:
			_vm->setGrabCursorSprite(kItemCereals);
			_vm->_gnapActionStatus = -1;
			break;
		case kASLeaveScene:
			_vm->_sceneDone = true;
			break;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(4) == 2 && _s23_nextStoreClerkSequenceId == -1) {
		switch (_vm->getRandom(8)) {
		case 0:
		case 1:
		case 2:
			_s23_nextStoreClerkSequenceId = 0xB4;
			break;
		case 3:
		case 4:
		case 5:
			_s23_nextStoreClerkSequenceId = 0xB5;
			break;
		default:
			_s23_nextStoreClerkSequenceId = 0xB6;
			break;
		}
		_vm->_gameSys->setAnimation(_s23_nextStoreClerkSequenceId, 1, 4);
		_vm->_gameSys->insertSequence(_s23_nextStoreClerkSequenceId, 1, _s23_currStoreClerkSequenceId, 1, kSeqSyncWait, 0, 0, 0);
		_s23_currStoreClerkSequenceId = _s23_nextStoreClerkSequenceId;
		_s23_nextStoreClerkSequenceId = -1;
	}
}

} // End of namespace Gnap
