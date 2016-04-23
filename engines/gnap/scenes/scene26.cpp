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
#include "gnap/scenes/scene26.h"

namespace Gnap {

enum {
	kHSPlatypus					= 0,
	kHSExitOutsideCircusWorld	= 1,
	kHSExitOutsideClown			= 2,
	kHSExitArcade				= 3,
	kHSExitElephant				= 4,
	kHSExitBeerStand			= 5,
	kHSDevice					= 6,
	kHSWalkArea1				= 7,
	kHSWalkArea2				= 8
};

enum {
	kASLeaveScene					= 0
};

Scene26::Scene26(GnapEngine *vm) : Scene(vm) {
	_s26_currKidSequenceId = -1;
	_s26_nextKidSequenceId = -1;
}

int Scene26::init() {
	return _vm->isFlag(kGFUnk23) ? 0x61 : 0x60;
}

void Scene26::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSExitOutsideCircusWorld, 0, 590, 300, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 1, 10);
	_vm->setHotspot(kHSExitOutsideClown, 200, 265, 265, 350, SF_EXIT_U_CURSOR, 3, 8);
	_vm->setHotspot(kHSExitArcade, 0, 295, 150, 400, SF_EXIT_NW_CURSOR, 2, 8);
	_vm->setHotspot(kHSExitElephant, 270, 290, 485, 375, SF_EXIT_U_CURSOR, 5, 8);
	_vm->setHotspot(kHSExitBeerStand, 530, 290, 620, 350, SF_EXIT_NE_CURSOR, 5, 8);
	_vm->setHotspot(kHSWalkArea1, 0, 0, 800, 500);
	_vm->setHotspot(kHSWalkArea2, 281, 0, 800, 600);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_vm->_hotspotsCount = 9;
}

void Scene26::run() {
	_vm->startSoundTimerB(7);
	_vm->playSound(0x1093B, true);

	_s26_currKidSequenceId = 0x5B;
	_s26_nextKidSequenceId = -1;
	_vm->_gameSys->setAnimation(0x5B, 160, 3);
	_vm->_gameSys->insertSequence(_s26_currKidSequenceId, 160, 0, 0, kSeqNone, 0, 0, 0);

	_vm->_timers[5] = _vm->getRandom(20) + 50;
	_vm->_timers[4] = _vm->getRandom(20) + 50;
	_vm->_timers[6] = _vm->getRandom(50) + 100;

	_vm->queueInsertDeviceIcon();

	_vm->_gameSys->insertSequence(0x58, 40, 0, 0, kSeqLoop, 0, 0, 0);
	_vm->_gameSys->insertSequence(0x5C, 40, 0, 0, kSeqLoop, 0, 0, 0);
	_vm->_gameSys->insertSequence(0x5D, 40, 0, 0, kSeqLoop, 0, 0, 0);
	_vm->_gameSys->insertSequence(0x5E, 40, 0, 0, kSeqLoop, 0, 0, 0);
	
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

		case kHSExitOutsideCircusWorld:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 25;
				_vm->gnapWalkTo(-1, _vm->_hotspotsWalkPos[kHSExitOutsideCircusWorld].y, 0, 0x107AE, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
			}
			break;

		case kHSExitOutsideClown:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 27;
				_vm->gnapWalkTo(-1, _vm->_hotspotsWalkPos[kHSExitOutsideClown].y, 0, 0x107BC, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
			}
			break;

		case kHSExitArcade:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 29;
				_vm->gnapWalkTo(-1, _vm->_hotspotsWalkPos[kHSExitArcade].y, 0, 0x107BC, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
			}
			break;

		case kHSExitElephant:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 30;
				_vm->gnapWalkTo(-1, _vm->_hotspotsWalkPos[4].y, 0, 0x107BC, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
			}
			break;

		case kHSExitBeerStand:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 31;
				_vm->gnapWalkTo(-1, _vm->_hotspotsWalkPos[kHSExitBeerStand].y, 0, 0x107BB, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
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

		if (!_vm->isSoundPlaying(0x1093B))
			_vm->playSound(0x1093B, true);

		if (!_vm->_isLeavingScene) {
			_vm->updatePlatypusIdleSequence();
			_vm->updateGnapIdleSequence();
			if (!_vm->_timers[5] && _s26_nextKidSequenceId == -1) {
				_vm->_timers[5] = _vm->getRandom(20) + 50;
				if (_vm->getRandom(5) != 0)
					_s26_nextKidSequenceId = 0x5B;
				else
					_s26_nextKidSequenceId = 0x5A;
			}
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(20) + 130;
				_vm->_gameSys->insertSequence(0x59, 40, 0, 0, kSeqNone, 0, 0, 0);
			}
			if (!_vm->_timers[6]) {
				_vm->_timers[6] = _vm->getRandom(50) + 100;
				_vm->_gameSys->insertSequence(0x5F, 40, 0, 0, kSeqNone, 0, 0, 0);
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
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		if (_vm->_gnapActionStatus == kASLeaveScene)
			_vm->_sceneDone = true;
		_vm->_gnapActionStatus = -1;
	}

	if (_vm->_gameSys->getAnimationStatus(3) == 2 && _s26_nextKidSequenceId != -1) {
		_vm->_gameSys->insertSequence(_s26_nextKidSequenceId, 160, _s26_currKidSequenceId, 160, kSeqSyncWait, 0, 0, 0);
		_vm->_gameSys->setAnimation(_s26_nextKidSequenceId, 160, 3);
		_s26_currKidSequenceId = _s26_nextKidSequenceId;
		_s26_nextKidSequenceId = -1;
	}
	
}

} // End of namespace Gnap
