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
#include "gnap/scenes/scene02.h"

namespace Gnap {

enum {
	kHSPlatypus		= 0,
	kHSChicken		= 1,
	kHSTruck1		= 2,
	kHSTruck2		= 3,
	kHSTruckGrill	= 4,
	kHSDevice		= 5,
	kHSExitHouse	= 6,
	kHSExitBarn		= 7,
	kHSExitCreek	= 8,
	kHSExitPigpen	= 9,
	kHSWalkArea1	= 10,
	kHSWalkArea2	= 11,
	kHSWalkArea3	= 12,
	kHSWalkArea4	= 13
};

enum {
	kASUseTruckNoKeys			= 0,
	kASUseGasWithTruck			= 1,
	kASUseTruckGas				= 2,
	kASUseTruckNoGas			= 3,
	kASGrabTruckGrill			= 5,
	kASLeaveScene				= 6,
	kASTalkChicken				= 7,
	kASGrabChicken				= 8,
	kASGrabChickenDone			= 9,
	kASUseTruckNoKeysDone		= 11,
	kASUseGasWithTruckDone		= 12,
	kASUseTwigWithChicken		= 16
};

Scene02::Scene02(GnapEngine *vm) : Scene(vm) {
	_s02_truckGrillCtr = 0;
	_s02_nextChickenSequenceId = 0;
	_s02_currChickenSequenceId = 0;
	_s02_gnapTruckSequenceId = 0;
}

int Scene02::init() {
	_vm->_gameSys->setAnimation(0, 0, 0);
	return _vm->isFlag(kGFTruckKeysUsed) ? 0x15A : 0x15B;
}

void Scene02::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_DISABLED | SF_WALKABLE);
	_vm->setHotspot(kHSChicken, 606, 455, 702, 568, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 7, 8);
	_vm->setHotspot(kHSTruck1, 385, 258, 464, 304, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 5);
	_vm->setHotspot(kHSTruck2, 316, 224, 390, 376, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 5, 6);
	_vm->setHotspot(kHSTruckGrill, 156, 318, 246, 390, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 2, 7);
	_vm->setHotspot(kHSExitHouse, 480, 120, 556, 240, SF_EXIT_U_CURSOR, 7, 5);
	_vm->setHotspot(kHSExitBarn, 610, 0, 800, 164, SF_EXIT_U_CURSOR, 10, 5);
	_vm->setHotspot(kHSExitCreek, 780, 336, 800, 556, SF_EXIT_R_CURSOR | SF_WALKABLE, 10, 8);
	_vm->setHotspot(kHSExitPigpen, 0, 300, 20, 600, SF_EXIT_L_CURSOR | SF_WALKABLE, 0, 8);
	_vm->setHotspot(kHSWalkArea1, 92, 140, 304, 430, 0, 3, 1);
	_vm->setHotspot(kHSWalkArea2, 0, 0, 800, 380);
	_vm->setHotspot(kHSWalkArea3, 0, 0, 386, 445);
	_vm->setHotspot(kHSWalkArea4, 386, 0, 509, 410);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	if (_vm->isFlag(kGFPlatypus))
		_vm->_hotspots[kHSPlatypus]._flags = SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	_vm->_hotspotsCount = 14;
}

void Scene02::run() {
	_vm->playSound(0x1091C, true);
	_vm->startSoundTimerC(6);

	_s02_currChickenSequenceId = 0x14B;
	_vm->_gameSys->setAnimation(0x14B, 179, 2);
	_vm->_gameSys->insertSequence(0x14B, 179, 0, 0, kSeqNone, 0, 0, 0);

	_s02_nextChickenSequenceId = -1;
	_vm->_timers[5] = _vm->getRandom(20) + 30;
	_vm->_timers[4] = _vm->getRandom(100) + 300;
	
	_vm->queueInsertDeviceIcon();
	
	switch (_vm->_prevSceneNum) {
	case 3:
		_vm->initGnapPos(11, 6, kDirBottomLeft);
		if (_vm->isFlag(kGFPlatypus))
			_vm->initPlatypusPos(12, 6, kDirUnk4);
		_vm->endSceneInit();
		if (_vm->isFlag(kGFPlatypus))
			_vm->platypusWalkTo(9, 6, -1, 0x107C2, 1);
		_vm->gnapWalkTo(8, 6, -1, 0x107BA, 1);
		break;
	case 4:
		_vm->initGnapPos(_vm->_hotspotsWalkPos[6].x, _vm->_hotspotsWalkPos[6].y, kDirBottomLeft);
		if (_vm->isFlag(kGFPlatypus))
			_vm->initPlatypusPos(_vm->_hotspotsWalkPos[6].x + 1, _vm->_hotspotsWalkPos[6].y, kDirUnk4);
		_vm->endSceneInit();
		_vm->gnapWalkTo(7, 6, 0, 0x107B9, 1);
		if (_vm->isFlag(kGFPlatypus))
			_vm->platypusWalkTo(8, 6, 1, 0x107C2, 1);
		updateHotspots();
		_vm->_gameSys->waitForUpdate();
		break;
	case 47:
		_vm->clearFlag(kGFUnk25);
		_vm->initGnapPos(5, 6, kDirBottomLeft);
		_vm->initPlatypusPos(6, 7, kDirUnk4);
		_vm->endSceneInit();
		break;
	case 49:
		_vm->initGnapPos(5, 6, kDirBottomRight);
		if (_vm->isFlag(kGFPlatypus))
			_vm->initPlatypusPos(6, 7, kDirNone);
		_vm->endSceneInit();
		break;
	default:
		_vm->initGnapPos(-1, 6, kDirBottomRight);
		if (_vm->isFlag(kGFPlatypus))
			_vm->initPlatypusPos(-1, 7, kDirNone);
		_vm->endSceneInit();
		if (_vm->isFlag(kGFPlatypus))
			_vm->platypusWalkTo(2, 7, -1, 0x107C2, 1);
		_vm->gnapWalkTo(2, 8, -1, 0x107B9, 1);
		break;
	}
	
	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();	
		_vm->updateCursorByHotspot();
	
		_vm->testWalk(0, 6, 7, 6, 8, 6);
	
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

		case kHSChicken:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemTwig) {
					_vm->_gnapIdleFacing = kDirUpRight;
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].x, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].y + 1,
						0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
					_vm->_gnapActionStatus = kASUseTwigWithChicken;
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[1].x, _vm->_hotspotsWalkPos[1].y + 1, 9, 8);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapMoan2(9, 8);
						break;
					case GRAB_CURSOR:
						_vm->_gnapIdleFacing = kDirBottomRight;
						if (_vm->gnapWalkTo(_vm->_hotspotsWalkPos[1].x, _vm->_hotspotsWalkPos[1].y, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1))
							_vm->_gnapActionStatus = kASGrabChicken;
						else
							_vm->_gnapActionStatus = -1;
						break;
					case TALK_CURSOR:
						_vm->_gnapIdleFacing = kDirBottomRight;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[1].x, _vm->_hotspotsWalkPos[1].y, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_vm->_gnapActionStatus = kASTalkChicken;
						break;
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSTruck1:
		case kHSTruck2:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemKeys) {
					if (_vm->gnapWalkTo(_vm->_hotspotsWalkPos[3].x, _vm->_hotspotsWalkPos[3].y, 0, _vm->getGnapSequenceId(gskIdle, 2, 2) | 0x10000, 1)) {
						_vm->setGrabCursorSprite(-1);
						_vm->invRemove(kItemKeys);
						if (_vm->isFlag(kGFTruckFilledWithGas))
							_vm->_gnapActionStatus = kASUseTruckGas;
						else
							_vm->_gnapActionStatus = kASUseTruckNoGas;
					}
				} else if (_vm->_grabCursorSpriteIndex == kItemGas) {
					_vm->_hotspots[kHSWalkArea4]._flags |= SF_WALKABLE;
					if (_vm->gnapWalkTo(_vm->_hotspotsWalkPos[2].x, _vm->_hotspotsWalkPos[2].y, 0, _vm->getGnapSequenceId(gskIdle, 2, 2) | 0x10000, 1))
						_vm->_gnapActionStatus = kASUseGasWithTruck;
					_vm->_hotspots[kHSWalkArea4]._flags &= ~SF_WALKABLE;
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[2].x, _vm->_hotspotsWalkPos[2].y, 2, 2);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(2, 2);
						break;
					case GRAB_CURSOR:
						if (_vm->isFlag(kGFTruckKeysUsed)) {
							if (_vm->gnapWalkTo(_vm->_hotspotsWalkPos[3].x, _vm->_hotspotsWalkPos[3].y, 0, _vm->getGnapSequenceId(gskIdle, 2, 2) | 0x10000, 1)) {
								if (_vm->isFlag(kGFTruckFilledWithGas))
									_vm->_gnapActionStatus = kASUseTruckGas;
								else
									_vm->_gnapActionStatus = kASUseTruckNoGas;
							}
						} else {
							_vm->_gnapIdleFacing = kDirUnk4;
							if (_vm->gnapWalkTo(_vm->_hotspotsWalkPos[3].x, _vm->_hotspotsWalkPos[3].y, 0, _vm->getGnapSequenceId(gskIdle, 2, 2) | 0x10000, 1))
								_vm->_gnapActionStatus = kASUseTruckNoKeys;
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

		case kHSTruckGrill:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[4].x, _vm->_hotspotsWalkPos[4].y, 2, 4);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapMoan2(2, 4);
						break;
					case GRAB_CURSOR:
						_vm->_gnapIdleFacing = kDirUpRight;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[4].x, _vm->_hotspotsWalkPos[4].y, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
						_vm->_gnapActionStatus = kASGrabTruckGrill;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSExitHouse:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[6].x, _vm->_hotspotsWalkPos[6].y, 0, 0x107AD, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				if (_vm->isFlag(kGFPlatypus))
					_vm->platypusWalkTo(_vm->_hotspotsWalkPos[6].x + 1, _vm->_hotspotsWalkPos[6].y, -1, 0x107C1, 1);
				updateHotspots();
				_vm->_newSceneNum = 4;
			}
			break;

		case kHSExitBarn:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[7].x, _vm->_hotspotsWalkPos[7].y, 0, 0x107AD, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				if (_vm->isFlag(kGFPlatypus))
					_vm->platypusWalkTo(_vm->_hotspotsWalkPos[7].x + 1, _vm->_hotspotsWalkPos[7].y, -1, 0x107C1, 1);
				updateHotspots();
				_vm->_newSceneNum = 5;
			}
			break;

		case kHSExitCreek:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[8].x, _vm->_hotspotsWalkPos[8].y, 0, 0x107AB, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				if (_vm->isFlag(kGFPlatypus))
					_vm->platypusWalkTo(_vm->_hotspotsWalkPos[8].x, _vm->_hotspotsWalkPos[8].y, -1, 0x107CD, 1);
				_vm->_newSceneNum = 3;
			}
			break;

		case kHSExitPigpen:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[9].x, _vm->_hotspotsWalkPos[9].y, 0, 0x107AF, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				if (_vm->isFlag(kGFPlatypus))
					_vm->platypusWalkTo(_vm->_hotspotsWalkPos[9].x, _vm->_hotspotsWalkPos[9].y, -1, 0x107CF, 1);
				_vm->_newSceneNum = 1;
			}
			break;

		case kHSWalkArea1:
		case kHSWalkArea2:
		case kHSWalkArea3:
		case kHSWalkArea4:
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
				_vm->updatePlatypusIdleSequence();
			if (_vm->_gnapActionStatus < 0)
				_vm->updateGnapIdleSequence();
			if (!_vm->_timers[4]) {
				// Update bird animation
				_vm->_timers[4] = _vm->getRandom(100) + 300;
				if (_vm->getRandom(2) != 0)
					_vm->_gameSys->insertSequence(0x156, 256, 0, 0, kSeqNone, 0, 0, 0);
				else
					_vm->_gameSys->insertSequence(0x154, 256, 0, 0, kSeqNone, 0, 0, 0);
			}
			if (!_vm->_timers[5] && _s02_nextChickenSequenceId == -1 && _vm->_gnapActionStatus != 7 && _vm->_gnapActionStatus != 8) {
				if (_vm->getRandom(6) != 0) {
					_s02_nextChickenSequenceId = 0x14B;
					_vm->_timers[5] = _vm->getRandom(20) + 30;
				} else {
					_s02_nextChickenSequenceId = 0x14D;
					_vm->_timers[5] = _vm->getRandom(20) + 50;
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

void Scene02::updateAnimations() {
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		switch (_vm->_gnapActionStatus) {
		case kASUseTruckNoKeys:
			_vm->_gameSys->insertSequence(0x14E, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(0x14E, _vm->_gnapId, 0);
			_vm->_gnapSequenceId = 0x14E;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapActionStatus = kASUseTruckNoKeysDone;
			break;
		case kASUseGasWithTruck:
			_vm->_gameSys->insertSequence(0x151, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(0x151, _vm->_gnapId, 0);
			_vm->_gnapSequenceId = 0x151;
			_vm->_gnapSequenceDatNum = 0;
			_vm->invRemove(kItemGas);
			_vm->setGrabCursorSprite(-1);
			_vm->setFlag(kGFTruckFilledWithGas);
			_vm->_gnapActionStatus = kASUseGasWithTruckDone;
			break;
		case kASUseTruckGas:
			_vm->_timers[5] = 9999;
			_vm->_timers[4] = 9999;
			_vm->hideCursor();
			_vm->setGrabCursorSprite(-1);
			if (!_vm->isFlag(kGFTruckKeysUsed)) {
				_vm->_gameSys->insertSequence(0x14F, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
				_vm->_gameSys->waitForUpdate();
				_vm->setFlag(kGFTruckKeysUsed);
				_vm->_gnapSequenceId = 0x14F;
				_vm->_gnapSequenceDatNum = 0;
				_vm->invRemove(kItemKeys);
				_vm->setGrabCursorSprite(-1);
			}
			_vm->_newSceneNum = 47;
			_vm->_sceneDone = true;
			break;
		case kASUseTruckNoGas:
			_vm->hideCursor();
			_vm->setGrabCursorSprite(-1);
			_vm->_timers[4] = 250;
			if (!_vm->isFlag(kGFTruckKeysUsed)) {
				_vm->_gameSys->insertSequence(0x14F, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
				_vm->_gameSys->waitForUpdate();
				_vm->setFlag(kGFTruckKeysUsed);
				_vm->_gnapSequenceId = 0x14F;
				_vm->_gnapSequenceDatNum = 0;
				_vm->invRemove(kItemKeys);
				_vm->setGrabCursorSprite(-1);
			}
			_vm->_newSceneNum = 47;
			_vm->_sceneDone = true;
			_vm->setFlag(kGFUnk25);
			break;
		case kASGrabTruckGrill:
			switch (_s02_truckGrillCtr) {
			case 0:
				_s02_gnapTruckSequenceId = 0x158;
				break;
			case 1:
				_s02_gnapTruckSequenceId = 0x159;
				break;
			case 2:
				_s02_gnapTruckSequenceId = 0x157;
				break;
			}
			_s02_truckGrillCtr = (_s02_truckGrillCtr + 1) % 3;
			_vm->_gameSys->insertSequence(_s02_gnapTruckSequenceId, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(_s02_gnapTruckSequenceId, _vm->_gnapId, 0);
			_vm->_gnapSequenceId = _s02_gnapTruckSequenceId;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapActionStatus = -1;
			break;
		case kASLeaveScene:
			_vm->_sceneDone = true;
			break;
		case kASTalkChicken:
			_s02_nextChickenSequenceId = 0x14C;
			break;
		case kASGrabChicken:
			_s02_nextChickenSequenceId = 0x150;
			_vm->_timers[2] = 100;
			break;
		case kASGrabChickenDone:
			_vm->_gameSys->insertSequence(0x107B5, _vm->_gnapId, 0x150, 179, kSeqSyncWait, 0, 75 * _vm->_gnapX - _vm->_gnapGridX, 48 * _vm->_gnapY - _vm->_gnapGridY);
			_vm->_gnapSequenceId = 0x7B5;
			_vm->_gnapSequenceDatNum = 1;
			_s02_currChickenSequenceId = 0x14B;
			_vm->_gameSys->setAnimation(0x14B, 179, 2);
			_vm->_gameSys->insertSequence(_s02_currChickenSequenceId, 179, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_gnapActionStatus = -1;
			_vm->_timers[5] = 30;
			break;
		case kASUseTwigWithChicken:
			_vm->playGnapShowItem(5, 0, 0);
			_vm->_gameSys->insertSequence(0x155, 179, _s02_currChickenSequenceId, 179, kSeqSyncExists, 0, 0, 0);
			_s02_currChickenSequenceId = 0x155;
			_s02_nextChickenSequenceId = -1;
			_vm->_gnapActionStatus = -1;
			break;
		case kASUseTruckNoKeysDone:
		case kASUseGasWithTruckDone:
		default:
			_vm->_gnapActionStatus = -1;
			break;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(2) == 2) {
		if (_s02_nextChickenSequenceId == 0x150) {
			_vm->_gameSys->setAnimation(_s02_nextChickenSequenceId, 179, 0);
			_vm->_gameSys->insertSequence(_s02_nextChickenSequenceId, 179, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->removeSequence(_s02_currChickenSequenceId, 179, true);
			_s02_nextChickenSequenceId = -1;
			_s02_currChickenSequenceId = -1;
			_vm->_gnapActionStatus = kASGrabChickenDone;
			_vm->_timers[5] = 500;
		} else if (_s02_nextChickenSequenceId == 0x14C) {
			_vm->_gameSys->setAnimation(_s02_nextChickenSequenceId, 179, 2);
			_vm->_gameSys->insertSequence(_s02_nextChickenSequenceId, 179, _s02_currChickenSequenceId, 179, kSeqSyncWait, 0, 0, 0);
			_s02_currChickenSequenceId = _s02_nextChickenSequenceId;
			_s02_nextChickenSequenceId = -1;
			_vm->_gnapActionStatus = -1;
		} else if (_s02_nextChickenSequenceId != -1) {
			_vm->_gameSys->setAnimation(_s02_nextChickenSequenceId, 179, 2);
			_vm->_gameSys->insertSequence(_s02_nextChickenSequenceId, 179, _s02_currChickenSequenceId, 179, kSeqSyncWait, 0, 0, 0);
			_s02_currChickenSequenceId = _s02_nextChickenSequenceId;
			_s02_nextChickenSequenceId = -1;
		}
	}
}

} // End of namespace Gnap
