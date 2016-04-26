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
#include "gnap/scenes/scene06.h"

namespace Gnap {

enum {
	kHS06Platypus			= 0,
	kHS06Gas				= 1,
	kHS06Ladder				= 2,
	kHS06Horse				= 3,
	kHS06ExitOutsideBarn	= 4,
	kHS06Device				= 5,
	kHS06WalkArea1			= 6,
	kHS06WalkArea2			= 7,
	kHS06WalkArea3			= 8,
	kHS06WalkArea4			= 9,
	kHS06WalkArea5			= 10
};

enum {
	kAS06TryToGetGas			= 0,
	kAS06TryToClimbLadder		= 1,
	kAS06TryToClimbLadderDone	= 2,
	kAS06TalkToHorse			= 3,
	kAS06UseTwigOnHorse			= 4,
	kAS06LeaveScene				= 5
};

Scene06::Scene06(GnapEngine *vm) : Scene(vm) {
	_horseTurnedBack = false;;
	_nextPlatSequenceId = -1;
	_nextHorseSequenceId = -1;
	_currHorseSequenceId = -1;
}

int Scene06::init() {
	_vm->_gameSys->setAnimation(0, 0, 0);
	_vm->_gameSys->setAnimation(0, 0, 1);
	_vm->_gameSys->setAnimation(0, 0, 2);
	if (_vm->isFlag(kGFSceneFlag1)) {
		_vm->playSound(0x11B, false);
		_vm->clearFlag(kGFSceneFlag1);
	}
	return 0x101;
}

void Scene06::updateHotspots() {
	_vm->setHotspot(kHS06Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS06Gas, 300, 120, 440, 232, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 4, 7);
	_vm->setHotspot(kHS06Ladder, 497, 222, 614, 492, SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 8);
	_vm->setHotspot(kHS06Horse, 90, 226, 259, 376, SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 4, 7);
	_vm->setHotspot(kHS06ExitOutsideBarn, 226, 580, 688, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 10);
	_vm->setHotspot(kHS06WalkArea1, 0, 0, 200, 515);
	_vm->setHotspot(kHS06WalkArea2, 200, 0, 285, 499);
	_vm->setHotspot(kHS06WalkArea3, 688, 0, 800, 499);
	_vm->setHotspot(kHS06WalkArea4, 475, 469, 800, 505);
	_vm->setHotspot(kHS06WalkArea5, 0, 0, 800, 504);
	_vm->setDeviceHotspot(kHS06Device, -1, -1, -1, -1);
    if (_vm->isFlag(kGFGasTaken)) 
    	_vm->_hotspots[kHS06Ladder]._flags = SF_DISABLED;
	if (_vm->_cursorValue == 4) {
		_vm->_hotspots[kHS06Ladder]._flags = SF_DISABLED;
		_vm->_hotspots[kHS06Gas]._flags = SF_DISABLED;
	}
	_vm->_hotspotsCount = 11;
}

void Scene06::run() {
	bool triedDeviceOnGas = false;
	
	_vm->startSoundTimerC(7);

	_horseTurnedBack = false;
	_vm->_gameSys->insertSequence(0xF1, 120, 0, 0, kSeqNone, 0, 0, 0);

	_currHorseSequenceId = 0xF1;
	_nextHorseSequenceId = -1;

	_vm->_gameSys->setAnimation(0xF1, 120, 2);
	_vm->_timers[4] = _vm->getRandom(40) + 25;

	if (_vm->isFlag(kGFUnk04))
		_vm->_gameSys->insertSequence(0xF7, 20, 0, 0, kSeqNone, 0, 0, 0);
	else
		_vm->_gameSys->insertSequence(0xF8, 20, 0, 0, kSeqNone, 0, 0, 0);
	
	if (!_vm->isFlag(kGFGasTaken) && _vm->_cursorValue != 4)
		_vm->_gameSys->insertSequence(0xFE, 20, 0, 0, kSeqNone, 0, 0, 0);
	
	_vm->queueInsertDeviceIcon();
	
	_vm->initGnapPos(5, 12, kDirBottomRight);
	_vm->initPlatypusPos(6, 12, kDirNone);
	_vm->endSceneInit();
	
	_vm->platypusWalkTo(6, 8, -1, 0x107C2, 1);
	_vm->gnapWalkTo(5, 8, -1, 0x107B9, 1);
	
	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
		
		_vm->testWalk(0, 5, -1, -1, -1, -1);
	
		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);
	
		switch (_vm->_sceneClickedHotspot) {
		case kHS06Device:
			if (_vm->_gnapActionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;
		
		case kHS06Platypus:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemDisguise) {
					_vm->gnapUseDisguiseOnPlatypus();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapImpossible(0, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						if (_vm->isFlag(kGFKeysTaken))
							_vm->playGnapMoan1(_vm->_platX, _vm->_platY);
						else
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

		case kHS06Gas:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].x, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].y, 5, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(5, 0);
						break;
					case GRAB_CURSOR:
						if (_vm->isFlag(kGFUnk04)) {
							_vm->playGnapImpossible(0, 0);
						} else if (triedDeviceOnGas) {
							_vm->_hotspots[kHS06WalkArea5]._flags |= SF_WALKABLE;
							_vm->gnapWalkTo(_vm->_hotspotsWalkPos[1].x, _vm->_hotspotsWalkPos[1].y, 0, 0x107BC, 1);
							_vm->_hotspots[kHS06WalkArea5]._flags &= ~SF_WALKABLE;
							_vm->_gnapActionStatus = kAS06TryToGetGas;
						} else {
							triedDeviceOnGas = true;
							_vm->playGnapPullOutDeviceNonWorking(_vm->_hotspotsWalkPos[1].x, _vm->_hotspotsWalkPos[1].y);
						}
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						if (_vm->isFlag(kGFUnk04))
							_vm->playGnapImpossible(0, 0);
						else
							_vm->playGnapScratchingHead(5, 0);
						break;
					}
				}
			}
			break;

		case kHS06Ladder:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].x, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].y, 8, 4);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(8, 4);
						break;
					case GRAB_CURSOR:
						if (_vm->isFlag(kGFGasTaken))
							_vm->playGnapImpossible(0, 0);
						else {
							_vm->gnapWalkTo(_vm->_hotspotsWalkPos[2].x, _vm->_hotspotsWalkPos[2].y, 0, 0x107BB, 1);
							_vm->_gnapActionStatus = kAS06TryToClimbLadder;
							_vm->setFlag(kGFGasTaken);
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

		case kHS06Horse:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemTwig && _horseTurnedBack) {
					_vm->_hotspots[kHS06WalkArea5]._flags |= SF_WALKABLE;
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[3].x, _vm->_hotspotsWalkPos[3].y, 0, 0x107BC, 1);
					_vm->_hotspots[kHS06WalkArea5]._flags &= ~SF_WALKABLE;
					_vm->_gnapIdleFacing = kDirUpLeft;
					_vm->platypusWalkTo(6, 8, 1, 0x107C2, 1);
					_vm->_platypusFacing = kDirNone;
					_vm->_gnapActionStatus = kAS06UseTwigOnHorse;
					_vm->setGrabCursorSprite(-1);
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].x, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].y, 3, 2);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(3, 2);
						break;
					case TALK_CURSOR:
						if (_horseTurnedBack) {
							_vm->gnapWalkTo(_vm->_hotspotsWalkPos[3].x, _vm->_hotspotsWalkPos[3].y, 0, _vm->getGnapSequenceId(gskBrainPulsating, 3, 2) | 0x10000, 1);
						} else {
							_vm->_gnapIdleFacing = kDirBottomLeft;
							_vm->_hotspots[kHS06WalkArea5]._flags |= SF_WALKABLE;
							_vm->gnapWalkTo(_vm->_hotspotsWalkPos[3].x, _vm->_hotspotsWalkPos[3].y, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
							_vm->_hotspots[kHS06WalkArea5]._flags &= ~SF_WALKABLE;
							_vm->_gnapActionStatus = kAS06TalkToHorse;
						}
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS06ExitOutsideBarn:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[4].x, _vm->_hotspotsWalkPos[4].y, 0, 0x107AE, 1);
				_vm->_gnapActionStatus = kAS06LeaveScene;
				if (_vm->_cursorValue == 1)
					_vm->_newSceneNum = 5;
				else
					_vm->_newSceneNum = 35;
			}
			break;

		case kHS06WalkArea1:
		case kHS06WalkArea2:
		case kHS06WalkArea3:
		case kHS06WalkArea4:
		case kHS06WalkArea5:
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
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(40) + 25;
				if (_vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0 && _nextHorseSequenceId == -1) {
					if (_horseTurnedBack) {
						_nextHorseSequenceId = 0xF5;
					} else {
						switch (_vm->getRandom(5)) {
						case 0:
						case 1:
						case 2:
							_nextHorseSequenceId = 0xF1;
							break;
						case 3:
							_nextHorseSequenceId = 0xF3;
							break;
						case 4:
							_nextHorseSequenceId = 0xF4;
							break;
						}
					}
				}
			}
			_vm->playSoundC();
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

void Scene06::updateAnimations() {
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kAS06LeaveScene:
			_vm->_sceneDone = true;
			_vm->_gnapActionStatus = -1;
			break;
		case kAS06TryToGetGas:
			_vm->_gameSys->insertSequence(0xFC, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceId = 0xFC;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapActionStatus = -1;
			break;
		case kAS06TryToClimbLadder:
			_vm->_gameSys->insertSequence(0xFF, 20, 0xFE, 20, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(0xFD, _vm->_gnapId, 0);
			_vm->_gameSys->insertSequence(0xFD, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceId = 0xFD;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapActionStatus = kAS06TryToClimbLadderDone;
			break;
		case kAS06TryToClimbLadderDone:
			_vm->_gnapX = 6;
			_vm->_gnapY = 7;
			_vm->_gnapActionStatus = -1;
			break;
		case kAS06TalkToHorse:
			_nextHorseSequenceId = 0xF6;
			break;
		case kAS06UseTwigOnHorse:
			_nextPlatSequenceId = 0xFB;
			break;
		default:
			_vm->_gnapActionStatus = -1;
			break;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(1) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 1);
		if (_vm->_platypusSequenceId == 0xFA) {
			_vm->_gameSys->setAnimation(0, 0, 1);
			_vm->invAdd(kItemGas);
			_vm->setFlag(kGFGasTaken);
			_vm->_hotspots[kHS06Ladder]._flags = SF_DISABLED;
			_vm->setGrabCursorSprite(kItemGas);
			_vm->_platypusActionStatus = -1;
			_vm->_platX = 6;
			_vm->_platY = 8;
			_vm->_gameSys->insertSequence(0x107C1, _vm->_platypusId, 0, 0, kSeqNone, 0, 450 - _vm->_platGridX, 384 - _vm->_platGridY);
			_vm->_platypusSequenceId = 0x7C1;
			_vm->_platypusSequenceDatNum = 1;
			_vm->setFlag(kGFUnk04);
			_vm->_gnapActionStatus = -1;
			_vm->showCursor();
		}
		if (_nextPlatSequenceId == 0xFB) {
			_vm->_gameSys->setAnimation(0, 0, 1);
			_nextHorseSequenceId = 0xF2;
			_vm->_platypusActionStatus = 6;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(2) == 2 && _nextHorseSequenceId != -1) {
		switch (_nextHorseSequenceId) {
		case 0xF2:
			_vm->setGrabCursorSprite(-1);
			_vm->hideCursor();
			_vm->_gameSys->setAnimation(0xFA, 256, 1);
			_vm->_gameSys->insertSequence(0xF2, 120, _currHorseSequenceId, 120, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x100, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0xF7, 20, 0xF8, 20, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0xFB, _vm->_platypusId, _vm->_platypusSequenceId | (_vm->_platypusSequenceDatNum << 16), _vm->_platypusId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0xFA, 256, 0xFB, _vm->_platypusId, kSeqSyncWait, 0, 0, 0);
			_vm->_platypusSequenceId = 0xFA;
			_vm->_platypusSequenceDatNum = 0;
			_vm->_gameSys->insertSequence(0x107B7, _vm->_gnapId, 0x100, _vm->_gnapId,
				kSeqSyncWait, 0, 75 * _vm->_gnapX - _vm->_gnapGridX, 48 * _vm->_gnapY - _vm->_gnapGridY);
			_vm->_gnapSequenceId = 0x7B7;
			_vm->_gnapSequenceDatNum = 1;
			_currHorseSequenceId = _nextHorseSequenceId;
			_nextHorseSequenceId = -1;
			_nextPlatSequenceId = -1;
			_vm->invRemove(kItemTwig);
			break;
		case 0xF6:
			_vm->_gameSys->setAnimation(_nextHorseSequenceId, 120, 2);
			_vm->_gameSys->insertSequence(0xF6, 120, _currHorseSequenceId, 120, kSeqSyncWait, 0, 0, 0);
			_horseTurnedBack = true;
			_currHorseSequenceId = _nextHorseSequenceId;
			_nextHorseSequenceId = -1;
			_vm->_gnapActionStatus = -1;
			break;
		default:
			_vm->_gameSys->setAnimation(_nextHorseSequenceId, 120, 2);
			_vm->_gameSys->insertSequence(_nextHorseSequenceId, 120, _currHorseSequenceId, 120, kSeqSyncWait, 0, 0, 0);
			_currHorseSequenceId = _nextHorseSequenceId;
			_nextHorseSequenceId = -1;
			break;
		}
	}
}

} // End of namespace Gnap
