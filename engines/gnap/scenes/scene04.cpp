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

int GnapEngine::scene04_init() {
	_gameSys->setAnimation(0, 0, 0);
	_gameSys->setAnimation(0, 0, 1);
	_gameSys->setAnimation(0, 0, 2);
	return 0x214;
}

void GnapEngine::scene04_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_DISABLED | SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSTwig, 690, 394, 769, 452, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 9, 6);
	setHotspot(kHSDog, 550, 442, 680, 552, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 8);
	setHotspot(kHSAxe, 574, 342, 680, 412, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 7);
	setHotspot(kHSDoor, 300, 244, 386, 410, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 7);
	setHotspot(kHSExitTruck, 226, 580, 688, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
	setHotspot(kHSWindow, 121, 295, 237, 342, SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 7);
	setHotspot(kHSExitBarn, 585, 154, 800, 276, SF_EXIT_U_CURSOR, 10, 8);
	setHotspot(kHSWalkArea1, 0, 0, 562, 461);
	setHotspot(kHSWalkArea2, 562, 0, 800, 500);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	if (isFlag(0))
		_hotspots[kHSPlatypus]._flags = SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	if (isFlag(3))
		_hotspots[kHSTwig]._flags = SF_WALKABLE | SF_DISABLED;
	if (isFlag(26) || _cursorValue == 1)
		_hotspots[kHSAxe]._flags = SF_DISABLED;
	_hotspotsCount = 11;
}

void GnapEngine::scene04_run() {

	playSound(0x1091C, 1);
	startSoundTimerC(4);

	_gameSys->insertSequence(0x210, 139 - _s04_dogIdCtr, 0, 0, kSeqNone, 0, 0, 0);

	_s04_currDogSequenceId = 0x210;
	_s04_nextDogSequenceId = -1;
	
	_gameSys->setAnimation(0x210, 139 - _s04_dogIdCtr, 3);
	_s04_dogIdCtr = (_s04_dogIdCtr + 1) % 2;
	_timers[6] = getRandom(20) + 60;
	_timers[5] = getRandom(150) + 300;
	_timers[7] = getRandom(150) + 200;
	_timers[8] = getRandom(150) + 400;

	if (!isFlag(26) && _cursorValue == 4)
		_gameSys->insertSequence(0x212, 100, 0, 0, kSeqNone, 0, 0, 0);
	
	if (!isFlag(3))
		_gameSys->insertSequence(0x1FE, 100, 0, 0, kSeqNone, 0, 0, 0);
	
	queueInsertDeviceIcon();
	
	if (isFlag(10)) {
		_timers[3] = 300;
		// TODO setCursor((LPCSTR)IDC_WAIT);
		setGrabCursorSprite(kItemKeys);
		_gnapX = 4;
		_gnapY = 7;
		_gnapId = 140;
		_platX = 6;
		_platY = 7;
		_beaverId = 141;
		_gameSys->insertSequence(0x107B5, 140, 0, 0, kSeqNone, 0, 300 - _gnapGridX, 336 - _gnapGridY);
		_gameSys->insertSequence(0x20C, 141, 0, 0, kSeqNone, 0, 0, 0);
		_gameSys->insertSequence(0x208, 121, 0, 0, kSeqNone, 0, 0, 0);
		_gameSys->insertSequence(0x209, 121, 0x208, 121, kSeqSyncWait, 0, 0, 0);
		endSceneInit();
		invRemove(kItemDisguise);
		invAdd(kItemKeys);
		setFlag(5);
		clearFlag(10);
		_beaverSequenceId = 0x20C;
		_beaverSequenceDatNum = 0;
		_beaverFacing = 1;
		_gnapSequenceId = 0x7B5;
		_gnapSequenceDatNum = 1;
		_gameSys->waitForUpdate();
	} else {
		_gameSys->insertSequence(0x209, 121, 0, 0, kSeqNone, 0, 0, 0);
		if (_prevSceneNum == 2) {
			initGnapPos(5, 11, 7);
			if (isFlag(0))
				initBeaverPos(6, 11, 5);
			endSceneInit();
			if (isFlag(0))
				platypusWalkTo(5, 8, -1, 0x107C2, 1);
			gnapWalkTo(6, 9, -1, 0x107BA, 1);
		} else if (_prevSceneNum == 38) {
			initGnapPos(5, 7, 1);
			initBeaverPos(4, 7, 0);
			endSceneInit();
		} else {
			initGnapPos(12, 9, 1);
			if (isFlag(0))
				initBeaverPos(12, 8, 0);
			endSceneInit();
			if (isFlag(0))
				platypusWalkTo(9, 8, -1, 0x107C2, 1);
		gnapWalkTo(9, 9, -1, 0x107BA, 1);
		}
	}
	
	while (!_sceneDone) {
	
		updateMouseCursor();
		updateCursorByHotspot();
		
		testWalk(0, 4, -1, -1, -1, -1);
	
		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);
	
		switch (_sceneClickedHotspot) {

		case kHSDevice:
			if (_gnapActionStatus < 0) {
				runMenu();
				scene04_updateHotspots();
			}
			break;

		case kHSPlatypus:
			if (_gnapActionStatus < 0 && isFlag(0)) {
				if (_grabCursorSpriteIndex == kItemDisguise) {
					gnapUseDisguiseOnPlatypus();
				} else if (_grabCursorSpriteIndex >= 0) {
					playGnapImpossible(0, 0);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						if (isFlag(5))
							playGnapMoan1(_platX, _platY);
						else
							playGnapScratchingHead(_platX, _platY);
						break;
					case GRAB_CURSOR:
						if (_cursorValue == 4)
							gnapKissPlatypus(0);
						else
							playGnapMoan1(_platX, _platY);
						break;
					case TALK_CURSOR:
						playGnapBrainPulsating(_platX, _platY);
						playBeaverSequence(getBeaverSequenceId(0, 0, 0) | 0x10000);
						break;
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;
		
		case kHSTwig:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowItem(_grabCursorSpriteIndex, 9, 6);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(_hotspotsWalkPos[1].x, _hotspotsWalkPos[1].y);
						break;
					case GRAB_CURSOR:
						playGnapPullOutDevice(_hotspotsWalkPos[1].x, _hotspotsWalkPos[1].y);
						playGnapUseDevice(_hotspotsWalkPos[1].x, _hotspotsWalkPos[1].y);
						_gameSys->insertSequence(0x1FD, 100, 510, 100, kSeqSyncWait, 0, 0, 0);
						_gameSys->setAnimation(0x1FD, 100, 2);
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;
		
		case kHSAxe:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[3].x, _hotspotsWalkPos[3].y, 9, 5);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapMoan2(_hotspotsWalkPos[3].x, _hotspotsWalkPos[3].y);
						break;
					case GRAB_CURSOR:
						_gnapIdleFacing = 7;
						gnapWalkTo(_hotspotsWalkPos[3].x, _hotspotsWalkPos[3].y, 0, getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
						_gnapActionStatus = kASGrabAxe;
						setFlag(26);
						scene04_updateHotspots();
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;
		
		case kHSDog:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[2].x, _hotspotsWalkPos[2].y, 9, 7);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						if (gnapWalkTo(_gnapX, _gnapY, 0, -1, 1)) {
							playGnapMoan2(_hotspotsWalkPos[2].x, _hotspotsWalkPos[2].y);
							_s04_nextDogSequenceId = 0x20F;
						}
						break;
					case GRAB_CURSOR:
						_gnapIdleFacing = 1;
						if (gnapWalkTo(_hotspotsWalkPos[2].x, _hotspotsWalkPos[2].y, 0, getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1))
							_gnapActionStatus = kASGrabDog;
						break;
					case TALK_CURSOR:
						_gnapIdleFacing = 1;
						if (gnapWalkTo(_gnapX, _gnapY, 0, -1, 1)) {
							playGnapBrainPulsating(_hotspotsWalkPos[2].x, _hotspotsWalkPos[2].y);
							_s04_nextDogSequenceId = 0x20E;
						}
						break;
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;
		
		case kHSDoor:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[_sceneClickedHotspot].x, _hotspotsWalkPos[_sceneClickedHotspot].y, 4, 3);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
					case PLAT_CURSOR:
						playGnapScratchingHead(4, 3);
						break;
					case GRAB_CURSOR:
						if (_cursorValue == 1) {
							gnapWalkTo(_hotspotsWalkPos[4].x, _hotspotsWalkPos[4].y, 0, 0x107BC, 1);
							_gnapActionStatus = kASOpenDoor;
							_timers[5] = 300;
							_gnapIdleFacing = 5;
						} else {
							_isLeavingScene = true;
							gnapWalkTo(_hotspotsWalkPos[4].x, _hotspotsWalkPos[4].y, 0, 0x107BC, 1);
							_gnapActionStatus = kASLeaveScene;
							_newSceneNum = 38;
						}
						break;
					case TALK_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;
		
		case kHSExitTruck:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = 1;
				gnapWalkTo(_hotspotsWalkPos[5].x, _hotspotsWalkPos[5].y, 0, 0x107AE, 1);
				_gnapActionStatus = kASLeaveScene;
				if (isFlag(0))
					platypusWalkTo(_hotspotsWalkPos[5].x, _hotspotsWalkPos[5].y, -1, 0x107C7, 1);
				if (_cursorValue == 1)
					_newSceneNum = 2;
				else
					_newSceneNum = 33;
			}
			break;
		
		case kHSWindow:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[_sceneClickedHotspot].x, _hotspotsWalkPos[_sceneClickedHotspot].y, 2, 3);
				} else if (isFlag(5)) {
					playGnapImpossible(0, 0);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						if (gnapWalkTo(_hotspotsWalkPos[7].x, _hotspotsWalkPos[7].y, 0, getGnapSequenceId(gskIdle, 10, 2) | 0x10000, 1)) {
							if (_s04_triedWindow) {
								_gnapActionStatus = kASGetKeyAnother;
							} else {
								_gnapActionStatus = kASGetKeyFirst;
								_s04_triedWindow = true;
							}
						}
						break;
					case GRAB_CURSOR:
						playGnapScratchingHead(_hotspotsWalkPos[7].x, _hotspotsWalkPos[7].y);
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;
		
		case kHSExitBarn:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = 1;
				gnapWalkTo(_hotspotsWalkPos[8].x, _hotspotsWalkPos[8].y, 0, 0x107AB, 1);
				_gnapActionStatus = kASLeaveScene;
				if (isFlag(0))
					platypusWalkTo(_hotspotsWalkPos[8].x, _hotspotsWalkPos[8].y + 1, -1, 0x107C1, 1);
				if (_cursorValue == 1)
					_newSceneNum = 5;
				else
					_newSceneNum = 35;
			}
			break;
		
		case kHSWalkArea1:
		case kHSWalkArea2:
			if (_gnapActionStatus < 0)
				gnapWalkTo(-1, -1, -1, -1, 1);
			break;
			
		default:
			if (_mouseClickState._left && _gnapActionStatus < 0) {
				gnapWalkTo(-1, -1, -1, -1, 1);
				_mouseClickState._left = false;
			}
			break;
			      	
		}
	
		scene04_updateAnimations();
	
		if (!isSoundPlaying(0x1091C))
			playSound(0x1091C, 1);
	
		if (!_isLeavingScene) {
			if (_beaverActionStatus < 0 && isFlag(0))
				beaverSub426234();
			if (_gnapActionStatus < 0)
				updateGnapIdleSequence2();
			if (!_timers[5]) {
				_timers[5] = getRandom(150) + 300;
				if (_gnapActionStatus < 0)
					_gameSys->insertSequence(0x20D, 79, 0, 0, kSeqNone, 0, 0, 0);
			}
			if (!_timers[7]) {
				_timers[7] = getRandom(150) + 200;
				_gameSys->insertSequence(0x1FC, 59, 0, 0, kSeqNone, 0, 0, 0);
			}
			if (!_timers[6]) {
				_timers[6] = getRandom(20) + 60;
				if (_s04_nextDogSequenceId == -1)
					_s04_nextDogSequenceId = 0x210;
			}
			if (!_timers[8]) {
				_timers[8] = getRandom(150) + 400;
				_gameSys->insertSequence(0x213, 20, 0, 0, kSeqNone, 0, 0, 0);
			}
			playSoundC();
		}
	
		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene04_updateHotspots();
		}
		
		gameUpdateTick();
	
	}

}

void GnapEngine::scene04_updateAnimations() {

	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		switch (_gnapActionStatus) {
		case kASLeaveScene:
			_sceneDone = true;
			break;
		case kASOpenDoor:
			_gameSys->insertSequence(0x205, _gnapId, 0, 0, kSeqNone, 0, 0, 0);
			_gameSys->insertSequence(0x207, 121, 521, 121, kSeqSyncWait, 0, 0, 0);
			_gnapX = 6;
			_gnapY = 7;
			_gameSys->insertSequence(0x107B5, _gnapId,
				makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId,
				kSeqSyncWait, getSequenceTotalDuration(0x205) - 1, 450 - _gnapGridX, 336 - _gnapGridY);
			_gameSys->setAnimation(0x107B5, _gnapId, 0);
			_gnapSequenceId = 0x7B5;
			_gnapSequenceDatNum = 1;
			_gnapActionStatus = kASOpenDoorDone;
			break;
		case kASOpenDoorDone:
			_gameSys->insertSequence(0x209, 121, 0x207, 121, kSeqSyncWait, 0, 0, 0);
			_gnapActionStatus = -1;
			break;
		case kASGetKeyFirst:
			_gameSys->insertSequence(0x204, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gameSys->setAnimation(0x204, _gnapId, 0);
			_gnapSequenceId = 0x204;
			_gnapSequenceDatNum = 0;
			_gnapActionStatus = kASGetKeyFirst2;
			break;
		case kASGetKeyFirst2:
			_gameSys->insertSequence(0x206, 255, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0x1FF, 256, 0, 0, kSeqNone, 0, 0, 0);
			_gameSys->insertSequence(0x20B, 256, 0, 0, kSeqNone, 0, 0, 0);
			_gameSys->setAnimation(0x20B, 256, 0);
			_gnapSequenceId = 0x206;
			_gnapSequenceDatNum = 0;
			_gnapActionStatus = kASGetKeyFirstDone;
			break;
		case kASGetKeyFirstDone:
			_gameSys->requestRemoveSequence(0x1FF, 256);
			_gameSys->requestRemoveSequence(0x20B, 256);
			_gameSys->insertSequence(0x107B5, _gnapId,
				makeRid(_gnapSequenceDatNum, _gnapSequenceId), 255,
				kSeqSyncWait, 0, 75 * _gnapX - _gnapGridX, 48 * _gnapY - _gnapGridY);
			_gnapIdleFacing = 1;
			_gnapSequenceId = 0x7B5;
			_gnapSequenceDatNum = 1;
			_gnapActionStatus = -1;
			break;
		case kASGetKeyAnother:
			_gameSys->insertSequence(0x202, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gameSys->setAnimation(0x202, _gnapId, 0);
			_gnapSequenceId = 0x202;
			_gnapSequenceDatNum = 0;
			_gnapActionStatus = kASGetKeyAnother2;
			break;
		case kASGetKeyAnother2:
			_gameSys->insertSequence(0x203, 255, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0x1FF, 256, 0, 0, kSeqNone, 0, 0, 0);
			_gameSys->insertSequence(0x20A, 256, 0, 0, kSeqNone, 0, 0, 0);
			_gameSys->setAnimation(0x20A, 256, 0);
			_gnapSequenceId = 0x203;
			_gnapSequenceDatNum = 0;
			_gnapActionStatus = kASGetKeyAnotherDone;
			break;
		case kASGetKeyAnotherDone:
			_gameSys->removeSequence(0x1FF, 256, true);
			_gameSys->removeSequence(0x20A, 256, true);
			_gameSys->insertSequence(0x107B5, _gnapId,
				makeRid(_gnapSequenceDatNum, _gnapSequenceId), 255,
				kSeqSyncWait, 0, 75 * _gnapX - _gnapGridX, 48 * _gnapY - _gnapGridY);
			_gnapSequenceId = 0x7B5;
			_gnapSequenceDatNum = 1;
			_gnapIdleFacing = 1;
			_gnapActionStatus = -1;
			break;
		case kASGrabDog:
			_s04_nextDogSequenceId = 0x201;
			break;
		case kASGrabAxe:
			_gameSys->insertSequence(0x211, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gameSys->requestRemoveSequence(0x212, 100);
			_gnapSequenceDatNum = 0;
			_gnapSequenceId = 0x211;
			_gnapActionStatus = -1;
			break;
		default:
			_gnapActionStatus = -1;
			break;
		}
	}
	
	if (_gameSys->getAnimationStatus(2) == 2) {
		_gameSys->setAnimation(0, 0, 2);
		invAdd(kItemTwig);
		setGrabCursorSprite(kItemTwig);
		setFlag(3);
		scene04_updateHotspots();
	}
	
	if (_gameSys->getAnimationStatus(3) == 2) {
		if (_s04_nextDogSequenceId == 0x201) {
			_gameSys->insertSequence(_s04_nextDogSequenceId, 139 - _s04_dogIdCtr,
				_s04_currDogSequenceId, 139 - (_s04_dogIdCtr + 1) % 2,
				kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0x200, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gameSys->setAnimation(_s04_nextDogSequenceId, 139 - _s04_dogIdCtr, 3);
			_s04_dogIdCtr = (_s04_dogIdCtr + 1) % 2;
			_s04_currDogSequenceId = 0x201;
			_gnapSequenceId = 0x200;
			_gnapSequenceDatNum = 0;
			_gnapActionStatus = -1;
			_timers[6] = getRandom(20) + 60;
			_s04_nextDogSequenceId = -1;
		} else if (_s04_nextDogSequenceId != -1) {
			_gameSys->insertSequence(_s04_nextDogSequenceId, 139 - _s04_dogIdCtr,
				_s04_currDogSequenceId, 139 - (_s04_dogIdCtr + 1) % 2,
				kSeqSyncWait, 0, 0, 0);
			_gameSys->setAnimation(_s04_nextDogSequenceId, 139 - _s04_dogIdCtr, 3);
			_s04_dogIdCtr = (_s04_dogIdCtr + 1) % 2;
			_s04_currDogSequenceId = _s04_nextDogSequenceId;
			_s04_nextDogSequenceId = -1;
		}
	}

}

} // End of namespace Gnap
