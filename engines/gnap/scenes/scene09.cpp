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
#include "gnap/scenes/scene09.h"

namespace Gnap {

enum {
	kHSPlatypus		= 0,
	kHSExitKitchen	= 1,
	kHSExitHouse	= 2,
	kHSTrash		= 3,
	kHSDevice		= 4,
	kHSWalkArea1	= 5,
	kHSWalkArea2	= 6,
	kHSWalkArea3	= 7
};

enum {
	kASLeaveScene		= 0,
	kASSearchTrash		= 1,
	kASSearchTrashDone	= 2
};

Scene09::Scene09(GnapEngine *vm) : Scene(vm) {
}

int Scene09::init() {
	return 0x4E;
}

void Scene09::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 200, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSExitKitchen, 280, 200, 380, 400, SF_EXIT_U_CURSOR);
	_vm->setHotspot(kHSExitHouse, 790, 200, 799, 450, SF_EXIT_R_CURSOR | SF_WALKABLE);
	_vm->setHotspot(kHSTrash, 440, 310, 680, 420, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSWalkArea1, 0, 0, 799, 400);
	_vm->setHotspot(kHSWalkArea2, 0, 0, 630, 450);
	_vm->setHotspot(kHSWalkArea2, 0, 0, 175, 495);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_vm->_hotspotsCount = 8;
}

void Scene09::run() {	
	_vm->queueInsertDeviceIcon();
	
	_vm->_gameSys->insertSequence(0x4D, 1, 0, 0, kSeqLoop, 0, 0, 0);
	_vm->_gameSys->insertSequence(0x4B, 2, 0, 0, kSeqNone, 0, 0, 0);
	
	if (_vm->_prevSceneNum == 8) {
		_vm->initGnapPos(11, 8, kDirBottomLeft);
		_vm->initPlatypusPos(12, 7, kDirUnk4);
		_vm->endSceneInit();
		_vm->gnapWalkTo(9, 8, -1, 0x107BA, 1);
		_vm->platypusWalkTo(9, 7, -1, 0x107D2, 1);
	} else {
		_vm->initGnapPos(4, 7, kDirBottomRight);
		_vm->initPlatypusPos(5, 7, kDirNone);
		_vm->endSceneInit();
	}

	_vm->_timers[4] = _vm->getRandom(150) + 50;
	_vm->_timers[5] = _vm->getRandom(40) + 50;

	while (!_vm->_sceneDone) {
		if (!_vm->isSoundPlaying(0x10919))
			_vm->playSound(0x10919, true);
	
		_vm->testWalk(0, 0, -1, -1, -1, -1);
	
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
	
		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);
	
		switch (_vm->_sceneClickedHotspot) {
		case kHSDevice:
			if (_vm->_gnapActionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(150) + 50;
				_vm->_timers[5] = _vm->getRandom(40) + 50;
			}
			break;

		case kHSPlatypus:
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
				break;
			}
			break;

		case kHSExitKitchen:
			_vm->_isLeavingScene = true;
			_vm->_newSceneNum = 10;
			_vm->gnapWalkTo(4, 7, 0, 0x107BF, 1);
			_vm->_gnapActionStatus = kASLeaveScene;
			_vm->platypusWalkTo(4, 8, -1, 0x107D2, 1);
			_vm->_platypusFacing = kDirUnk4;
			break;

		case kHSExitHouse:
			_vm->_isLeavingScene = true;
			_vm->_newSceneNum = 8;
			_vm->gnapWalkTo(10, -1, 0, 0x107AB, 1);
			_vm->_gnapActionStatus = kASLeaveScene;
			_vm->platypusWalkTo(10, -1, -1, 0x107CD, 1);
			_vm->_platypusFacing = kDirUnk4;
			break;

		case kHSTrash:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapShowCurrItem(9, 6, 8, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->playGnapScratchingHead(8, 3);
					break;
				case GRAB_CURSOR:
					_vm->_gnapActionStatus = kASSearchTrash;
					_vm->gnapWalkTo(9, 6, 0, 0x107BC, 1);
					break;
				case TALK_CURSOR:
				case PLAT_CURSOR:
					_vm->playGnapImpossible(0, 0);
					break;
				}
			}
			break;

		case kHSWalkArea1:
		case kHSWalkArea2:
		case kHSWalkArea3:
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
	
		if (!_vm->_isLeavingScene && _vm->_gnapActionStatus != 1 && _vm->_gnapActionStatus != 2) {
			_vm->updatePlatypusIdleSequence();
			_vm->updateGnapIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(150) + 100;
				if (_vm->_timers[4] & 1)
					_vm->_gameSys->insertSequence(0x49, 1, 0, 0, kSeqNone, 0, 0, 0);
				else
					_vm->_gameSys->insertSequence(0x4A, 1, 0, 0, kSeqNone, 0, 0, 0);
			}
			playRandomSound(5);
		}
	
		_vm->checkGameKeys();
	
		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[4] = _vm->getRandom(150) + 50;
			_vm->_timers[5] = _vm->getRandom(40) + 50;
		}
		
		_vm->gameUpdateTick();
	}
}

void Scene09::updateAnimations() {
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kASLeaveScene:
			_vm->_sceneDone = true;
			_vm->_gnapActionStatus = -1;
			break;
		case kASSearchTrash:
			_vm->_gameSys->setAnimation(0x4C, 120, 0);
			_vm->_gameSys->insertSequence(0x4C, 120, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->removeSequence(0x4B, 2, true);
			_vm->_gnapSequenceId = 0x4C;
			_vm->_gnapId = 120;
			_vm->_gnapIdleFacing = kDirUpLeft;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapX = 9;
			_vm->_gnapY = 6;
			_vm->_gnapActionStatus = kASSearchTrashDone;
			break;
		case kASSearchTrashDone:
			_vm->_gameSys->insertSequence(0x4B, 2, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_timers[2] = 360;
			_vm->_timers[4] = _vm->getRandom(150) + 100;
			_vm->_gnapActionStatus = -1;
			break;
		}
	}
}

} // End of namespace Gnap
