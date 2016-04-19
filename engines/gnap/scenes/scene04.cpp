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
#include "gnap/scenes/scene04.h"

namespace Gnap {

enum {
	kHSPlatypus		= 0,
	kHSTwig			= 1,
	kHSDog			= 2,
	kHSAxe			= 3,
	kHSDoor			= 4,
	kHSExitTruck	= 5,
	kHSDevice		= 6,
	kHSWindow		= 7,
	kHSExitBarn		= 8,
	kHSWalkArea1	= 9,
	kHSWalkArea2	= 10
};

enum {
	kASOpenDoor				= 1,
	kASGetKeyFirst			= 2,
	kASGetKeyAnother		= 3,
	kASLeaveScene			= 4,
	kASGetKeyFirstDone		= 6,
	kASGetKeyFirst2			= 7,
	kASGetKeyAnother2		= 8,
	kASGetKeyAnotherDone	= 9,
	kASOpenDoorDone			= 10,
	kASGrabDog				= 12,
	kASGrabAxe				= 13
};

Scene04::Scene04(GnapEngine *vm) : Scene(vm) {
	_s04_dogIdCtr = 0;
	_s04_triedWindow = false;
	_s04_nextDogSequenceId = -1;
	_s04_currDogSequenceId = -1;
}

int Scene04::init() {
	_vm->_gameSys->setAnimation(0, 0, 0);
	_vm->_gameSys->setAnimation(0, 0, 1);
	_vm->_gameSys->setAnimation(0, 0, 2);
	return 0x214;
}

void Scene04::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_DISABLED | SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSTwig, 690, 394, 769, 452, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 9, 6);
	_vm->setHotspot(kHSDog, 550, 442, 680, 552, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 8);
	_vm->setHotspot(kHSAxe, 574, 342, 680, 412, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 7);
	_vm->setHotspot(kHSDoor, 300, 244, 386, 410, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 7);
	_vm->setHotspot(kHSExitTruck, 226, 580, 688, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
	_vm->setHotspot(kHSWindow, 121, 295, 237, 342, SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 7);
	_vm->setHotspot(kHSExitBarn, 585, 154, 800, 276, SF_EXIT_U_CURSOR, 10, 8);
	_vm->setHotspot(kHSWalkArea1, 0, 0, 562, 461);
	_vm->setHotspot(kHSWalkArea2, 562, 0, 800, 500);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	if (_vm->isFlag(kGFPlatypus))
		_vm->_hotspots[kHSPlatypus]._flags = SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	if (_vm->isFlag(kGFTwigTaken))
		_vm->_hotspots[kHSTwig]._flags = SF_WALKABLE | SF_DISABLED;
	if (_vm->isFlag(kGFPlatypusTalkingToAssistant) || _vm->_cursorValue == 1)
		_vm->_hotspots[kHSAxe]._flags = SF_DISABLED;
	_vm->_hotspotsCount = 11;
}

void Scene04::run() {
	_vm->playSound(0x1091C, true);
	_vm->startSoundTimerC(4);

	_vm->_gameSys->insertSequence(0x210, 139 - _s04_dogIdCtr, 0, 0, kSeqNone, 0, 0, 0);

	_s04_currDogSequenceId = 0x210;
	_s04_nextDogSequenceId = -1;
	
	_vm->_gameSys->setAnimation(0x210, 139 - _s04_dogIdCtr, 3);
	_s04_dogIdCtr = (_s04_dogIdCtr + 1) % 2;
	_vm->_timers[6] = _vm->getRandom(20) + 60;
	_vm->_timers[5] = _vm->getRandom(150) + 300;
	_vm->_timers[7] = _vm->getRandom(150) + 200;
	_vm->_timers[8] = _vm->getRandom(150) + 400;

	if (!_vm->isFlag(kGFPlatypusTalkingToAssistant) && _vm->_cursorValue == 4)
		_vm->_gameSys->insertSequence(0x212, 100, 0, 0, kSeqNone, 0, 0, 0);
	
	if (!_vm->isFlag(kGFTwigTaken))
		_vm->_gameSys->insertSequence(0x1FE, 100, 0, 0, kSeqNone, 0, 0, 0);
	
	_vm->queueInsertDeviceIcon();
	
	if (_vm->isFlag(kGFPlatyPussDisguised)) {
		_vm->_timers[3] = 300;
		// TODO setCursor((LPCSTR)IDC_WAIT);
		_vm->setGrabCursorSprite(kItemKeys);
		_vm->_gnapX = 4;
		_vm->_gnapY = 7;
		_vm->_gnapId = 140;
		_vm->_platX = 6;
		_vm->_platY = 7;
		_vm->_platypusId = 141;
		_vm->_gameSys->insertSequence(0x107B5, 140, 0, 0, kSeqNone, 0, 300 - _vm->_gnapGridX, 336 - _vm->_gnapGridY);
		_vm->_gameSys->insertSequence(0x20C, 141, 0, 0, kSeqNone, 0, 0, 0);
		_vm->_gameSys->insertSequence(0x208, 121, 0, 0, kSeqNone, 0, 0, 0);
		_vm->_gameSys->insertSequence(0x209, 121, 0x208, 121, kSeqSyncWait, 0, 0, 0);
		_vm->endSceneInit();
		_vm->invRemove(kItemDisguise);
		_vm->invAdd(kItemKeys);
		_vm->setFlag(kGFKeysTaken);
		_vm->clearFlag(kGFPlatyPussDisguised);
		_vm->_platypusSequenceId = 0x20C;
		_vm->_platypusSequenceDatNum = 0;
		_vm->_platypusFacing = kDirBottomRight;
		_vm->_gnapSequenceId = 0x7B5;
		_vm->_gnapSequenceDatNum = 1;
		_vm->_gameSys->waitForUpdate();
	} else {
		_vm->_gameSys->insertSequence(0x209, 121, 0, 0, kSeqNone, 0, 0, 0);
		if (_vm->_prevSceneNum == 2) {
			_vm->initGnapPos(5, 11, kDirUpRight);
			if (_vm->isFlag(kGFPlatypus))
				_vm->initPlatypusPos(6, 11, kDirUpLeft);
			_vm->endSceneInit();
			if (_vm->isFlag(kGFPlatypus))
				_vm->platypusWalkTo(5, 8, -1, 0x107C2, 1);
			_vm->gnapWalkTo(6, 9, -1, 0x107BA, 1);
		} else if (_vm->_prevSceneNum == 38) {
			_vm->initGnapPos(5, 7, kDirBottomRight);
			_vm->initPlatypusPos(4, 7, kDirNone);
			_vm->endSceneInit();
		} else {
			_vm->initGnapPos(12, 9, kDirBottomRight);
			if (_vm->isFlag(kGFPlatypus))
				_vm->initPlatypusPos(12, 8, kDirNone);
			_vm->endSceneInit();
			if (_vm->isFlag(kGFPlatypus))
				_vm->platypusWalkTo(9, 8, -1, 0x107C2, 1);
		_vm->gnapWalkTo(9, 9, -1, 0x107BA, 1);
		}
	}
	
	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
		
		_vm->testWalk(0, 4, -1, -1, -1, -1);
	
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
			if (_vm->_gnapActionStatus < 0 && _vm->isFlag(kGFPlatypus)) {
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
						if (_vm->_cursorValue == 4)
							_vm->gnapKissPlatypus(0);
						else
							_vm->playGnapMoan1(_vm->_platX, _vm->_platY);
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
		
		case kHSTwig:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, 9, 6);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(_vm->_hotspotsWalkPos[1].x, _vm->_hotspotsWalkPos[1].y);
						break;
					case GRAB_CURSOR:
						_vm->playGnapPullOutDevice(_vm->_hotspotsWalkPos[1].x, _vm->_hotspotsWalkPos[1].y);
						_vm->playGnapUseDevice(_vm->_hotspotsWalkPos[1].x, _vm->_hotspotsWalkPos[1].y);
						_vm->_gameSys->insertSequence(0x1FD, 100, 510, 100, kSeqSyncWait, 0, 0, 0);
						_vm->_gameSys->setAnimation(0x1FD, 100, 2);
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;
		
		case kHSAxe:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[3].x, _vm->_hotspotsWalkPos[3].y, 9, 5);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapMoan2(_vm->_hotspotsWalkPos[3].x, _vm->_hotspotsWalkPos[3].y);
						break;
					case GRAB_CURSOR:
						_vm->_gnapIdleFacing = kDirUpRight;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[3].x, _vm->_hotspotsWalkPos[3].y, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
						_vm->_gnapActionStatus = kASGrabAxe;
						_vm->setFlag(kGFPlatypusTalkingToAssistant);
						updateHotspots();
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;
		
		case kHSDog:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[2].x, _vm->_hotspotsWalkPos[2].y, 9, 7);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						if (_vm->gnapWalkTo(_vm->_gnapX, _vm->_gnapY, 0, -1, 1)) {
							_vm->playGnapMoan2(_vm->_hotspotsWalkPos[2].x, _vm->_hotspotsWalkPos[2].y);
							_s04_nextDogSequenceId = 0x20F;
						}
						break;
					case GRAB_CURSOR:
						_vm->_gnapIdleFacing = kDirBottomRight;
						if (_vm->gnapWalkTo(_vm->_hotspotsWalkPos[2].x, _vm->_hotspotsWalkPos[2].y, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1))
							_vm->_gnapActionStatus = kASGrabDog;
						break;
					case TALK_CURSOR:
						_vm->_gnapIdleFacing = kDirBottomRight;
						if (_vm->gnapWalkTo(_vm->_gnapX, _vm->_gnapY, 0, -1, 1)) {
							_vm->playGnapBrainPulsating(_vm->_hotspotsWalkPos[2].x, _vm->_hotspotsWalkPos[2].y);
							_s04_nextDogSequenceId = 0x20E;
						}
						break;
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;
		
		case kHSDoor:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].x, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].y, 4, 3);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapScratchingHead(4, 3);
						break;
					case GRAB_CURSOR:
						if (_vm->_cursorValue == 1) {
							_vm->gnapWalkTo(_vm->_hotspotsWalkPos[4].x, _vm->_hotspotsWalkPos[4].y, 0, 0x107BC, 1);
							_vm->_gnapActionStatus = kASOpenDoor;
							_vm->_timers[5] = 300;
							_vm->_gnapIdleFacing = kDirUpLeft;
						} else {
							_vm->_isLeavingScene = true;
							_vm->gnapWalkTo(_vm->_hotspotsWalkPos[4].x, _vm->_hotspotsWalkPos[4].y, 0, 0x107BC, 1);
							_vm->_gnapActionStatus = kASLeaveScene;
							_vm->_newSceneNum = 38;
						}
						break;
					case TALK_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;
		
		case kHSExitTruck:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[5].x, _vm->_hotspotsWalkPos[5].y, 0, 0x107AE, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				if (_vm->isFlag(kGFPlatypus))
					_vm->platypusWalkTo(_vm->_hotspotsWalkPos[5].x, _vm->_hotspotsWalkPos[5].y, -1, 0x107C7, 1);
				if (_vm->_cursorValue == 1)
					_vm->_newSceneNum = 2;
				else
					_vm->_newSceneNum = 33;
			}
			break;
		
		case kHSWindow:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].x, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].y, 2, 3);
				} else if (_vm->isFlag(kGFKeysTaken)) {
					_vm->playGnapImpossible(0, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						if (_vm->gnapWalkTo(_vm->_hotspotsWalkPos[7].x, _vm->_hotspotsWalkPos[7].y, 0, _vm->getGnapSequenceId(gskIdle, 10, 2) | 0x10000, 1)) {
							if (_s04_triedWindow) {
								_vm->_gnapActionStatus = kASGetKeyAnother;
							} else {
								_vm->_gnapActionStatus = kASGetKeyFirst;
								_s04_triedWindow = true;
							}
						}
						break;
					case GRAB_CURSOR:
						_vm->playGnapScratchingHead(_vm->_hotspotsWalkPos[7].x, _vm->_hotspotsWalkPos[7].y);
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;
		
		case kHSExitBarn:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[8].x, _vm->_hotspotsWalkPos[8].y, 0, 0x107AB, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				if (_vm->isFlag(kGFPlatypus))
					_vm->platypusWalkTo(_vm->_hotspotsWalkPos[8].x, _vm->_hotspotsWalkPos[8].y + 1, -1, 0x107C1, 1);
				if (_vm->_cursorValue == 1)
					_vm->_newSceneNum = 5;
				else
					_vm->_newSceneNum = 35;
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
	
		if (!_vm->isSoundPlaying(0x1091C))
			_vm->playSound(0x1091C, true);
	
		if (!_vm->_isLeavingScene) {
			if (_vm->_platypusActionStatus < 0 && _vm->isFlag(kGFPlatypus))
				_vm->platypusSub426234();
			if (_vm->_gnapActionStatus < 0)
				_vm->updateGnapIdleSequence2();
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(150) + 300;
				if (_vm->_gnapActionStatus < 0)
					_vm->_gameSys->insertSequence(0x20D, 79, 0, 0, kSeqNone, 0, 0, 0);
			}
			if (!_vm->_timers[7]) {
				_vm->_timers[7] = _vm->getRandom(150) + 200;
				_vm->_gameSys->insertSequence(0x1FC, 59, 0, 0, kSeqNone, 0, 0, 0);
			}
			if (!_vm->_timers[6]) {
				_vm->_timers[6] = _vm->getRandom(20) + 60;
				if (_s04_nextDogSequenceId == -1)
					_s04_nextDogSequenceId = 0x210;
			}
			if (!_vm->_timers[8]) {
				_vm->_timers[8] = _vm->getRandom(150) + 400;
				_vm->_gameSys->insertSequence(0x213, 20, 0, 0, kSeqNone, 0, 0, 0);
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

void Scene04::updateAnimations() {
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kASLeaveScene:
			_vm->_sceneDone = true;
			break;
		case kASOpenDoor:
			_vm->_gameSys->insertSequence(0x205, _vm->_gnapId, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x207, 121, 521, 121, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapX = 6;
			_vm->_gnapY = 7;
			_vm->_gameSys->insertSequence(0x107B5, _vm->_gnapId,
				makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId,
				kSeqSyncWait, _vm->getSequenceTotalDuration(0x205) - 1, 450 - _vm->_gnapGridX, 336 - _vm->_gnapGridY);
			_vm->_gameSys->setAnimation(0x107B5, _vm->_gnapId, 0);
			_vm->_gnapSequenceId = 0x7B5;
			_vm->_gnapSequenceDatNum = 1;
			_vm->_gnapActionStatus = kASOpenDoorDone;
			break;
		case kASOpenDoorDone:
			_vm->_gameSys->insertSequence(0x209, 121, 0x207, 121, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapActionStatus = -1;
			break;
		case kASGetKeyFirst:
			_vm->_gameSys->insertSequence(0x204, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(0x204, _vm->_gnapId, 0);
			_vm->_gnapSequenceId = 0x204;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapActionStatus = kASGetKeyFirst2;
			break;
		case kASGetKeyFirst2:
			_vm->_gameSys->insertSequence(0x206, 255, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x1FF, 256, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x20B, 256, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_gameSys->setAnimation(0x20B, 256, 0);
			_vm->_gnapSequenceId = 0x206;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapActionStatus = kASGetKeyFirstDone;
			break;
		case kASGetKeyFirstDone:
			_vm->_gameSys->requestRemoveSequence(0x1FF, 256);
			_vm->_gameSys->requestRemoveSequence(0x20B, 256);
			_vm->_gameSys->insertSequence(0x107B5, _vm->_gnapId,
				makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), 255,
				kSeqSyncWait, 0, 75 * _vm->_gnapX - _vm->_gnapGridX, 48 * _vm->_gnapY - _vm->_gnapGridY);
			_vm->_gnapIdleFacing = kDirBottomRight;
			_vm->_gnapSequenceId = 0x7B5;
			_vm->_gnapSequenceDatNum = 1;
			_vm->_gnapActionStatus = -1;
			break;
		case kASGetKeyAnother:
			_vm->_gameSys->insertSequence(0x202, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(0x202, _vm->_gnapId, 0);
			_vm->_gnapSequenceId = 0x202;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapActionStatus = kASGetKeyAnother2;
			break;
		case kASGetKeyAnother2:
			_vm->_gameSys->insertSequence(0x203, 255, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x1FF, 256, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x20A, 256, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_gameSys->setAnimation(0x20A, 256, 0);
			_vm->_gnapSequenceId = 0x203;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapActionStatus = kASGetKeyAnotherDone;
			break;
		case kASGetKeyAnotherDone:
			_vm->_gameSys->removeSequence(0x1FF, 256, true);
			_vm->_gameSys->removeSequence(0x20A, 256, true);
			_vm->_gameSys->insertSequence(0x107B5, _vm->_gnapId,
				makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), 255,
				kSeqSyncWait, 0, 75 * _vm->_gnapX - _vm->_gnapGridX, 48 * _vm->_gnapY - _vm->_gnapGridY);
			_vm->_gnapSequenceId = 0x7B5;
			_vm->_gnapSequenceDatNum = 1;
			_vm->_gnapIdleFacing = kDirBottomRight;
			_vm->_gnapActionStatus = -1;
			break;
		case kASGrabDog:
			_s04_nextDogSequenceId = 0x201;
			break;
		case kASGrabAxe:
			_vm->_gameSys->insertSequence(0x211, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->requestRemoveSequence(0x212, 100);
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapSequenceId = 0x211;
			_vm->_gnapActionStatus = -1;
			break;
		default:
			_vm->_gnapActionStatus = -1;
			break;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(2) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 2);
		_vm->invAdd(kItemTwig);
		_vm->setGrabCursorSprite(kItemTwig);
		_vm->setFlag(kGFTwigTaken);
		updateHotspots();
	}
	
	if (_vm->_gameSys->getAnimationStatus(3) == 2) {
		if (_s04_nextDogSequenceId == 0x201) {
			_vm->_gameSys->insertSequence(_s04_nextDogSequenceId, 139 - _s04_dogIdCtr,
				_s04_currDogSequenceId, 139 - (_s04_dogIdCtr + 1) % 2,
				kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x200, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(_s04_nextDogSequenceId, 139 - _s04_dogIdCtr, 3);
			_s04_dogIdCtr = (_s04_dogIdCtr + 1) % 2;
			_s04_currDogSequenceId = 0x201;
			_vm->_gnapSequenceId = 0x200;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapActionStatus = -1;
			_vm->_timers[6] = _vm->getRandom(20) + 60;
			_s04_nextDogSequenceId = -1;
		} else if (_s04_nextDogSequenceId != -1) {
			_vm->_gameSys->insertSequence(_s04_nextDogSequenceId, 139 - _s04_dogIdCtr,
				_s04_currDogSequenceId, 139 - (_s04_dogIdCtr + 1) % 2,
				kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(_s04_nextDogSequenceId, 139 - _s04_dogIdCtr, 3);
			_s04_dogIdCtr = (_s04_dogIdCtr + 1) % 2;
			_s04_currDogSequenceId = _s04_nextDogSequenceId;
			_s04_nextDogSequenceId = -1;
		}
	}
}

} // End of namespace Gnap
