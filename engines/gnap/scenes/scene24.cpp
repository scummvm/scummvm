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
#include "gnap/scenes/scene24.h"

namespace Gnap {

enum {
	kHSPlatypus				= 0,
	kHSExitCircusWorld		= 1,
	kHSExitOutsideGrubCity	= 2,
	kHSDevice				= 3,
	kHSWalkArea1			= 4,
	kHSWalkArea2			= 5,
	kHSWalkArea3			= 6
};

enum {
	kASLeaveScene			= 0
};

Scene24::Scene24(GnapEngine *vm) : Scene(vm) {
	_s24_currWomanSequenceId = -1;
	_s24_nextWomanSequenceId = -1;
	_s24_boySequenceId = -1;
	_s24_girlSequenceId = -1;
}

int Scene24::init() {
	return 0x3B;
}

void Scene24::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSExitCircusWorld, 785, 128, 800, 600, SF_EXIT_R_CURSOR, 8, 7);
	_vm->setHotspot(kHSExitOutsideGrubCity, 0, 213, 91, 600, SF_EXIT_NW_CURSOR, 1, 8);
	_vm->setHotspot(kHSWalkArea1, 0, 0, 0, 0);
	_vm->setHotspot(kHSWalkArea2, 530, 0, 800, 600);
	_vm->setHotspot(kHSWalkArea3, 0, 0, 800, 517);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_vm->_hotspotsCount = 7;
}

void Scene24::run() {
	int counter = 0;
	
	_vm->playSound(0x10940, true);
	_vm->startSoundTimerA(9);
	
	_vm->_timers[7] = _vm->getRandom(100) + 100;
	
	_vm->_gameSys->insertSequence(0x2F, 256, 0, 0, kSeqNone, 0, 0, 0);
	
	_vm->_timers[4] = _vm->getRandom(20) + 50;
	_vm->_timers[5] = _vm->getRandom(20) + 40;
	_vm->_timers[6] = _vm->getRandom(50) + 30;
	
	_vm->_gameSys->insertSequence(0x36, 20, 0, 0, kSeqNone, 0, 0, 0);
	_vm->_gameSys->insertSequence(0x30, 20, 0, 0, kSeqNone, 0, 0, 0);
	_vm->_gameSys->insertSequence(0x35, 20, 0, 0, kSeqNone, 0, 0, 0);
	
	_s24_currWomanSequenceId = 0x35;
	_s24_girlSequenceId = 0x36;
	_s24_boySequenceId = 0x30;
	
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
		
		case kHSExitCircusWorld:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 25;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitCircusWorld].x, _vm->_hotspotsWalkPos[kHSExitCircusWorld].y, 0, 0x107AB, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitCircusWorld].x + 1, _vm->_hotspotsWalkPos[kHSExitCircusWorld].y, -1, 0x107C2, 1);
			}
			break;
		
		case kHSExitOutsideGrubCity:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 20;
				_vm->_gnapIdleFacing = kDirUpRight;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitOutsideGrubCity].x, _vm->_hotspotsWalkPos[kHSExitOutsideGrubCity].y, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitOutsideGrubCity].x + 1, _vm->_hotspotsWalkPos[kHSExitOutsideGrubCity].y, -1, 0x107C2, 1);
			}
			break;
		
		case kHSWalkArea1:
		case kHSWalkArea2:
		case kHSWalkArea3:
			if (_vm->_gnapActionStatus == -1)
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
				_vm->_gameSys->insertSequence(0x37, 20, _s24_girlSequenceId, 20, kSeqSyncWait, 0, 0, 0);
				_s24_girlSequenceId = 0x37;
			}
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(20) + 40;
				_vm->_gameSys->insertSequence(0x31, 20, _s24_boySequenceId, 20, kSeqSyncWait, 0, 0, 0);
				_s24_boySequenceId = 0x31;
			}
			if (!_vm->_timers[6]) {
				_vm->_timers[6] = _vm->getRandom(50) + 30;
				counter = (counter + 1) % 3;
				switch (counter) {
				case 0:
					_s24_nextWomanSequenceId = 0x32;
					break;
				case 1:
					_s24_nextWomanSequenceId = 0x33;
					break;
				case 2:
					_s24_nextWomanSequenceId = 0x34;
					break;
				}
				_vm->_gameSys->insertSequence(_s24_nextWomanSequenceId, 20, _s24_currWomanSequenceId, 20, kSeqSyncWait, 0, 0, 0);
				_s24_currWomanSequenceId = _s24_nextWomanSequenceId;
			}
			if (!_vm->_timers[7]) {
				_vm->_timers[7] = _vm->getRandom(100) + 100;
				switch (_vm->getRandom(3)) {
				case 0:
					_vm->_gameSys->insertSequence(0x38, 253, 0, 0, kSeqNone, 0, 0, 0);
					break;
				case 1:
					_vm->_gameSys->insertSequence(0x39, 253, 0, 0, kSeqNone, 0, 0, 0);
					break;
				case 2:
					_vm->_gameSys->insertSequence(0x3A, 253, 0, 0, kSeqNone, 0, 0, 0);
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
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		if (_vm->_gnapActionStatus == kASLeaveScene)
			_vm->_sceneDone = true;
		_vm->_gnapActionStatus = -1;
	}

}

} // End of namespace Gnap
