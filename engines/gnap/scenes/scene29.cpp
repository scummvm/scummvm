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
#include "gnap/scenes/scene29.h"

namespace Gnap {

enum {
	kHSPlatypus			= 0,
	kHSMonkey			= 1,
	kHSDevice			= 2,
	kHSExitCircus		= 3,
	kHSExitOutsideClown	= 4,
	kHSAracde			= 5,
	kHSWalkArea1		= 6
};

enum {
	kASUseBananaWithMonkey		= 0,
	kASLeaveScene				= 2
};

Scene29::Scene29(GnapEngine *vm) : Scene(vm) {
	_s28_currMonkeySequenceId = -1;
	_s28_nextMonkeySequenceId = -1;
	_s28_currManSequenceId = -1;
	_s28_nextManSequenceId = -1;
}

int Scene29::init() {
	return 0xF6;
}

void Scene29::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSMonkey, 410, 374, 518, 516, SF_WALKABLE | SF_DISABLED, 3, 7);
	_vm->setHotspot(kHSExitCircus, 150, 585, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
	_vm->setHotspot(kHSExitOutsideClown, 785, 0, 800, 600, SF_EXIT_R_CURSOR | SF_WALKABLE, 11, 9);
	_vm->setHotspot(kHSAracde, 88, 293, 155, 384, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 8);
	_vm->setHotspot(kHSWalkArea1, 0, 0, 800, 478);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	if (_vm->invHas(kItemHorn))
		_vm->_hotspots[kHSMonkey]._flags = SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	_vm->_hotspotsCount = 7;
}

void Scene29::run() {
	_vm->playSound(0x1093B, true);
	_vm->startSoundTimerB(6);
	_vm->queueInsertDeviceIcon();

	if (_vm->invHas(kItemHorn)) {
		_s28_currMonkeySequenceId = 0xE8;
		_s28_nextMonkeySequenceId = -1;
		_vm->_gameSys->setAnimation(0xE8, 159, 4);
		_vm->_gameSys->insertSequence(_s28_currMonkeySequenceId, 159, 0, 0, kSeqNone, 0, 0, 0);
		_vm->_gameSys->insertSequence(0xED, 39, 0, 0, kSeqNone, 0, 0, 0);
		_s28_currManSequenceId = 0xED;
		_s28_nextManSequenceId = -1;
		_vm->_gameSys->setAnimation(0xED, 39, 3);
		_vm->_timers[4] = _vm->getRandom(20) + 60;
	} else {
		_vm->_gameSys->insertSequence(0xF4, 19, 0, 0, kSeqNone, 0, 0, 0);
		_vm->_gameSys->setAnimation(0, 0, 4);
		_vm->_gameSys->insertSequence(0xED, 39, 0, 0, kSeqNone, 0, 0, 0);
		_vm->_gameSys->setAnimation(0, 0, 3);
	}
	
	_vm->_gameSys->insertSequence(0xF3, 39, 0, 0, kSeqLoop, 0, 0, 0);
	_vm->_gameSys->insertSequence(0xF5, 38, 0, 0, kSeqLoop, 0, 0, 0);

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

		case kHSMonkey:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemBanana) {
					_vm->_gnapIdleFacing = kDirBottomRight;
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSMonkey].x, _vm->_hotspotsWalkPos[kHSMonkey].y, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
					_vm->_gnapActionStatus = kASUseBananaWithMonkey;
					_vm->_newSceneNum = 51;
					_vm->_isLeavingScene = true;
					_vm->setGrabCursorSprite(-1);
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHSMonkey].x, _vm->_hotspotsWalkPos[kHSMonkey].y, 5, 6);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(5, 6);
						break;
					case TALK_CURSOR:
						_vm->playGnapBrainPulsating(_vm->_hotspotsWalkPos[kHSMonkey].x, _vm->_hotspotsWalkPos[kHSMonkey].y);
						break;
					case GRAB_CURSOR:
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
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitCircus].x, _vm->_hotspotsWalkPos[kHSExitCircus].y, 0, 0x107AE, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitCircus].x + 1, _vm->_hotspotsWalkPos[kHSExitCircus].y, -1, -1, 1);
			}
			break;

		case kHSExitOutsideClown:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 27;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitOutsideClown].x, _vm->_hotspotsWalkPos[kHSExitOutsideClown].y, 0, 0x107AB, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitOutsideClown].x, _vm->_hotspotsWalkPos[kHSExitOutsideClown].y - 1, -1, 0x107CD, 1);
			}
			break;

		case kHSAracde:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemDiceQuarterHole) {
					_vm->setGrabCursorSprite(-1);
					_vm->_isLeavingScene = true;
					_vm->_newSceneNum = 52;
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSAracde].x, _vm->_hotspotsWalkPos[kHSAracde].y, 0, -1, 1);
					_vm->playGnapIdle(_vm->_hotspotsWalkPos[kHSAracde].x, _vm->_hotspotsWalkPos[kHSAracde].y);
					_vm->_gnapActionStatus = kASLeaveScene;
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHSAracde].x, _vm->_hotspotsWalkPos[kHSAracde].y, 2, 3);
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
			if (_vm->_gnapActionStatus < 0) {
				_vm->updateGnapIdleSequence();
				_vm->updatePlatypusIdleSequence();
			}
			if (!_vm->_timers[4]) {
				if (_vm->invHas(kItemHorn)) {
					_vm->_timers[4] = _vm->getRandom(20) + 60;
					if (_vm->_gnapActionStatus < 0) {
						switch (_vm->getRandom(5)) {
						case 0:
							_s28_nextManSequenceId = 0xED;
							break;
						case 1:
							_s28_nextManSequenceId = 0xEE;
							break;
						case 2:
							_s28_nextManSequenceId = 0xEF;
							break;
						case 3:
							_s28_nextManSequenceId = 0xF0;
							break;
						case 4:
							_s28_nextManSequenceId = 0xF1;
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
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kASUseBananaWithMonkey:
			_s28_nextMonkeySequenceId = 0xE5;
			break;
		case kASLeaveScene:
			_vm->_sceneDone = true;
			break;
		}
	}

	if (_vm->_gameSys->getAnimationStatus(3) == 2 && _s28_nextManSequenceId != -1) {
		_vm->_gameSys->insertSequence(_s28_nextManSequenceId, 39, _s28_currManSequenceId, 39, kSeqSyncWait, 0, 0, 0);
		_vm->_gameSys->setAnimation(_s28_nextManSequenceId, 39, 3);
		_s28_currManSequenceId = _s28_nextManSequenceId;
		_s28_nextManSequenceId = -1;
	}

	if (_vm->_gameSys->getAnimationStatus(4) == 2) {
		if (_s28_nextMonkeySequenceId == 0xE5) {
			_vm->_gameSys->insertSequence(0xF2, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapSequenceId = 0xF2;
			_vm->_gameSys->setAnimation(0xE6, 159, 0);
			_vm->_gameSys->setAnimation(0, 159, 4);
			_vm->_gameSys->insertSequence(_s28_nextMonkeySequenceId, 159, _s28_currMonkeySequenceId, 159, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0xE6, 159, _s28_nextMonkeySequenceId, 159, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapActionStatus = kASLeaveScene;
			_s28_currMonkeySequenceId = 0xE6;
			_s28_nextMonkeySequenceId = -1;
			_vm->_timers[5] = 30;
			while (_vm->_timers[5])
				_vm->gameUpdateTick();
			_vm->platypusWalkTo(0, 8, 1, 0x107CF, 1);
			while (_vm->_gameSys->getAnimationStatus(1) != 2)
				_vm->gameUpdateTick();
		} else if (_s28_nextMonkeySequenceId == -1) {
			switch (_vm->getRandom(6)) {
			case 0:
				_s28_nextMonkeySequenceId = 0xE8;
				break;
			case 1:
				_s28_nextMonkeySequenceId = 0xE9;
				break;
			case 2:
				_s28_nextMonkeySequenceId = 0xEA;
				break;
			case 3:
				_s28_nextMonkeySequenceId = 0xEB;
				break;
			case 4:
				_s28_nextMonkeySequenceId = 0xEC;
				break;
			case 5:
				_s28_nextMonkeySequenceId = 0xE7;
				break;
			}
			_vm->_gameSys->insertSequence(_s28_nextMonkeySequenceId, 159, _s28_currMonkeySequenceId, 159, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(_s28_nextMonkeySequenceId, 159, 4);
			_s28_currMonkeySequenceId = _s28_nextMonkeySequenceId;
			_s28_nextMonkeySequenceId = -1;
		} else {
			_vm->_gameSys->insertSequence(_s28_nextMonkeySequenceId, 159, _s28_currMonkeySequenceId, 159, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(_s28_nextMonkeySequenceId, 159, 4);
			_s28_currMonkeySequenceId = _s28_nextMonkeySequenceId;
			_s28_nextMonkeySequenceId = -1;
		}
	}
	
}

} // End of namespace Gnap
