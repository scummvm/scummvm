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
#include "gnap/scenes/scene39.h"

namespace Gnap {

enum {
	kHSPlatypus			= 0,
	kHSExitInsideHouse	= 1,
	kHSExitUfoParty		= 2,
	kHSSign				= 3,
	kHSDevice			= 4,
	kHSWalkArea1		= 5,
	kHSWalkArea2		= 6
};

enum {
	kASLeaveScene				= 0
};

Scene39::Scene39(GnapEngine *vm) : Scene(vm) {
	_s39_currGuySequenceId = -1;
	_s39_nextGuySequenceId = -1;
}

int Scene39::init() {
	_vm->_gameSys->setAnimation(0, 0, 0);
	_vm->_gameSys->setAnimation(0, 0, 1);
	return 0x35;
}

void Scene39::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSExitInsideHouse, 0, 0, 140, 206, SF_EXIT_U_CURSOR, 4, 8);
	_vm->setHotspot(kHSExitUfoParty, 360, 204, 480, 430, SF_EXIT_R_CURSOR, 6, 8);
	_vm->setHotspot(kHSSign, 528, 232, 607, 397, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 7, 3);
	_vm->setHotspot(kHSWalkArea1, 0, 0, 800, 466);
	_vm->setHotspot(kHSWalkArea2, 502, 466, 800, 600);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_vm->_hotspotsCount = 7;
}

void Scene39::run() {
	// Bug in the original? Timer was never initialized.
	_vm->_timers[5] = 0;
	
	_vm->queueInsertDeviceIcon();
	_s39_currGuySequenceId = 0x33;

	_vm->_gameSys->setAnimation(0x33, 21, 3);
	_vm->_gameSys->insertSequence(_s39_currGuySequenceId, 21, 0, 0, kSeqNone, 0, 0, 0);
	_vm->_gameSys->insertSequence(0x34, 21, 0, 0, kSeqLoop, 0, 0, 0);

	_s39_nextGuySequenceId = -1;
	if (_vm->_prevSceneNum == 38) {
		_vm->initGnapPos(3, 7, kDirUpRight);
		_vm->initPlatypusPos(2, 7, kDirUpLeft);
		_vm->endSceneInit();
	} else {
		_vm->initGnapPos(4, 7, kDirBottomRight);
		_vm->initPlatypusPos(5, 7, kDirNone);
		_vm->endSceneInit();
	}
	
	while (!_vm->_sceneDone) {
		if (!_vm->isSoundPlaying(0x1094B)) {
			_vm->playSound(0x1094B, true);
			_vm->setSoundVolume(0x1094B, 60);
		}
		
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
	
		_vm->testWalk(0, 0, -1, -1, -1, -1);

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHSDevice:
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[5] = _vm->getRandom(20) + 50;
			break;
	
		case kHSPlatypus:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapImpossible(_vm->_platX, _vm->_platY);
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
						_vm->playGnapImpossible(_vm->_platX, _vm->_platY);
						break;
					}
				}
			}
			break;
	
		case kHSExitUfoParty:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_sceneDone = true;
				_vm->gnapWalkTo(_vm->_gnapX, _vm->_gnapY, 0, 0x107AB, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->_newSceneNum = 40;
			}
			break;
	
		case kHSSign:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapImpossible(0, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSSign].x, _vm->_hotspotsWalkPos[kHSSign].y, 0, -1, 1);
						_vm->playGnapIdle(_vm->_hotspotsWalkPos[kHSSign].x, _vm->_hotspotsWalkPos[kHSSign].y);
						_vm->showFullScreenSprite(0x1C);
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
	
		case kHSExitInsideHouse:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_sceneDone = true;
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 38;
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
	
		if (!_vm->_isLeavingScene) {
			if (_vm->_platypusActionStatus < 0)
				_vm->updatePlatypusIdleSequence();
			if (_vm->_gnapActionStatus < 0)
				_vm->updateGnapIdleSequence();
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(20) + 50;
				switch (_vm->getRandom(4)) {
				case 0:
					_s39_nextGuySequenceId = 0x30;
					break;
				case 1:
					_s39_nextGuySequenceId = 0x31;
					break;
				case 2:
					_s39_nextGuySequenceId = 0x32;
					break;
				case 3:
					_s39_nextGuySequenceId = 0x33;
					break;
				}
			}
		}
	
		_vm->checkGameKeys();
	
		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[5] = _vm->getRandom(20) + 50;
		}
		
		_vm->gameUpdateTick();
	}
}

void Scene39::updateAnimations() {
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		if (_vm->_gnapActionStatus == kASLeaveScene)
			_vm->_sceneDone = true;
		else
			_vm->_gnapActionStatus = -1;
	}
	
	if (_vm->_gameSys->getAnimationStatus(3) == 2 && _s39_nextGuySequenceId != -1) {
		_vm->_gameSys->setAnimation(_s39_nextGuySequenceId, 21, 3);
		_vm->_gameSys->insertSequence(_s39_nextGuySequenceId, 21, _s39_currGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
		_s39_currGuySequenceId = _s39_nextGuySequenceId;
		_s39_nextGuySequenceId = -1;
	}
}

} // End of namespace Gnap
