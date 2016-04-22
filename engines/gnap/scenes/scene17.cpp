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
#include "gnap/scenes/scene17.h"

namespace Gnap {

enum {
	kHSPlatypus		= 0,
	kHSPhone1		= 1,
	kHSPhone2		= 2,
	kHSExitGrubCity	= 3,
	kHSDevice		= 4,
	kHSExitToyStore	= 5,
	kHSWrench		= 6,
	kHSWalkArea1	= 7,
	kHSWalkArea2	= 8,
	kHSWalkArea3	= 9
};

enum {
	kASTryGetWrench				= 0,
	kASGetWrench2				= 1,
	kASGetWrenchDone			= 2,
	kASGetWrench1				= 3,
	kASPlatUsePhone				= 4,
	kASPutCoinIntoPhone			= 5,
	kASGetCoinFromPhone			= 6,
	kASGetCoinFromPhoneDone		= 7,
	kASPutCoinIntoPhoneDone		= 8,
	kASGnapUsePhone				= 9,
	kASGetWrenchGnapReady		= 10,
	kASGnapHangUpPhone			= 11,
	kASPlatPhoningAssistant		= 12,
	kASPlatHangUpPhone			= 14,
	kASLeaveScene				= 15
};

Scene17::Scene17(GnapEngine *vm) : Scene(vm) {
	_s17_platTryGetWrenchCtr = 0;
	_s17_wrenchCtr = 2;
	_s17_nextCarWindowSequenceId = -1;
	_s17_nextWrenchSequenceId = -1;
	_s17_canTryGetWrench = true;
	_s17_platPhoneCtr = 0;
	_s17_nextPhoneSequenceId = -1;
	_s17_currPhoneSequenceId = -1;
}

int Scene17::init() {
	return 0x263;
}

void Scene17::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 1, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSPhone1, 61, 280, 97, 322, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 1, 7);
	_vm->setHotspot(kHSPhone2, 80, 204, 178, 468, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 1, 7);
	_vm->setHotspot(kHSExitGrubCity, 196, 207, 280, 304, SF_EXIT_U_CURSOR, 3, 5);
	_vm->setHotspot(kHSExitToyStore, 567, 211, 716, 322, SF_EXIT_U_CURSOR, 5, 6);
	_vm->setHotspot(kHSWrench, 586, 455, 681, 547, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 5, 7);
	_vm->setHotspot(kHSWalkArea1, 0, 0, 800, 434);
	_vm->setHotspot(kHSWalkArea2, 541, 0, 800, 600);
	_vm->setHotspot(kHSWalkArea3, 0, 204, 173, 468);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	if (_vm->isFlag(kGFGrassTaken))
		_vm->_hotspots[kHSWrench]._flags = SF_NONE;
	if (_vm->isFlag(kGFPlatypusTalkingToAssistant)) {
		_vm->_hotspots[kHSDevice]._flags = SF_DISABLED;
		_vm->_hotspots[kHSPlatypus]._flags = SF_DISABLED;
	}
	_vm->_hotspotsCount = 10;
}

void Scene17::update() {
	_vm->gameUpdateTick();
	_vm->updateMouseCursor();
	_vm->updateGrabCursorSprite(0, 0);
	if (_vm->_mouseClickState._left) {
		_vm->gnapWalkTo(-1, -1, -1, -1, 1);
		_vm->_mouseClickState._left = false;
	}
}

void Scene17::platHangUpPhone() {
	int savedGnapActionStatus = _vm->_gnapActionStatus;

	if (_vm->_platypusActionStatus == kASPlatPhoningAssistant) {
		_vm->_gnapActionStatus = kASPlatHangUpPhone;
		_vm->updateMouseCursor();
		_s17_platPhoneCtr = 0;
		_vm->_platypusActionStatus = -1;
		_vm->_gameSys->setAnimation(0x257, 254, 4);
		_vm->_gameSys->insertSequence(0x257, 254, _s17_currPhoneSequenceId, 254, kSeqSyncExists, 0, 0, 0);
		while (_vm->_gameSys->getAnimationStatus(4) != 2)
			_vm->gameUpdateTick();
		_vm->_gameSys->setAnimation(0x25B, _vm->_platypusId, 1);
		_vm->_gameSys->insertSequence(0x25B, _vm->_platypusId, _vm->_platypusSequenceId | (_vm->_platypusSequenceDatNum << 16), _vm->_platypusId, kSeqSyncWait, 0, 0, 0);
		_vm->_platypusSequenceId = 0x25B;
		_vm->_platypusSequenceDatNum = 0;
		_s17_currPhoneSequenceId = -1;
		_s17_nextPhoneSequenceId = -1;
		_vm->clearFlag(kGFPlatypusTalkingToAssistant);
		while (_vm->_gameSys->getAnimationStatus(1) != 2)
			_vm->gameUpdateTick();
		_vm->_gnapActionStatus = savedGnapActionStatus;
		_vm->updateMouseCursor();
	}
	updateHotspots();
}

void Scene17::run() {
	_vm->playSound(0x10940, true);
	_vm->startSoundTimerA(8);
	_vm->_sceneWaiting = false;
	_vm->_timers[4] = _vm->getRandom(100) + 200;
	_vm->_timers[3] = 200;
	_vm->_timers[5] = _vm->getRandom(30) + 80;
	_vm->_timers[6] = _vm->getRandom(30) + 200;
	_vm->_timers[7] = _vm->getRandom(100) + 100;

	if (_vm->isFlag(kGFTruckKeysUsed)) {
		_vm->_gameSys->insertSequence(0x25F, 20, 0, 0, kSeqNone, 0, 0, 0);
	} else {
		if (_vm->_s18_garbageCanPos >= 8) {
			_vm->_gameSys->insertSequence(0x260, 20, 0, 0, kSeqNone, 0, 97, 1);
		} else if (_vm->_s18_garbageCanPos >= 6) {
			_vm->_gameSys->insertSequence(0x260, 20, 0, 0, kSeqNone, 0, 68, 2);
		} else if (_vm->_s18_garbageCanPos >= 5) {
			_vm->_gameSys->insertSequence(0x260, 20, 0, 0, kSeqNone, 0, 23, -1);
		} else if (_vm->_s18_garbageCanPos >= 4) {
			_vm->_gameSys->insertSequence(0x260, 20, 0, 0, kSeqNone, 0, -11, -5);
		} else {
			_vm->_gameSys->insertSequence(0x260, 20, 0, 0, kSeqNone, 0, -54, -8);
		}
	}

	if (_vm->isFlag(kGFGroceryStoreHatTaken))
		_vm->_gameSys->insertSequence(0x262, 1, 0, 0, kSeqNone, 0, 0, 0);

	_vm->queueInsertDeviceIcon();

	if (_vm->isFlag(kGFGrassTaken))
		_s17_currWrenchSequenceId = 0x22D;
	else
		_s17_currWrenchSequenceId = 0x22F;

	_s17_currCarWindowSequenceId = 0x244;

	if (_vm->isFlag(kGFUnk14))
		_vm->_gameSys->insertSequence(0x261, 1, 0, 0, kSeqNone, 0, 0, 0);

	_vm->_gameSys->setAnimation(_s17_currWrenchSequenceId, 40, 2);
	_vm->_gameSys->insertSequence(_s17_currWrenchSequenceId, 40, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->isFlag(kGFGrassTaken)) {
		_vm->_gameSys->setAnimation(0, 0, 3);
	} else {
		_vm->_gameSys->setAnimation(_s17_currCarWindowSequenceId, 40, 3);
		_vm->_gameSys->insertSequence(_s17_currCarWindowSequenceId, 40, 0, 0, kSeqNone, 0, 0, 0);
	}

	_s17_canTryGetWrench = true;

	if (_vm->isFlag(kGFUnk18))
		_vm->_gameSys->insertSequence(0x24F, 100, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->_prevSceneNum == 53 || _vm->_prevSceneNum == 18 || _vm->_prevSceneNum == 20 || _vm->_prevSceneNum == 19) {
		if (_vm->_prevSceneNum == 20) {
			_vm->initGnapPos(4, 6, kDirBottomRight);
			_vm->initPlatypusPos(5, 6, kDirNone);
			_vm->endSceneInit();
			_vm->platypusWalkTo(5, 9, -1, 0x107C2, 1);
			_vm->gnapWalkTo(4, 8, -1, 0x107B9, 1);
		} else if (_vm->isFlag(kGFUnk27)) {
			_vm->initGnapPos(3, 9, kDirUpLeft);
			_vm->_platX = _vm->_hotspotsWalkPos[2].x;
			_vm->_platY = _vm->_hotspotsWalkPos[2].y;
			_vm->_platypusId = 20 * _vm->_hotspotsWalkPos[2].y;
			_vm->_gameSys->insertSequence(0x25A, 20 * _vm->_hotspotsWalkPos[2].y, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x257, 254, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_platypusSequenceId = 0x25A;
			_vm->_platypusSequenceDatNum = 0;
			_vm->endSceneInit();
			_vm->clearFlag(kGFSpringTaken);
			_vm->clearFlag(kGFUnk16);
			_vm->_platypusActionStatus = kASPlatPhoningAssistant;
			platHangUpPhone();
			_vm->_gameSys->setAnimation(0, 0, 4);
			_vm->clearFlag(kGFPlatypusTalkingToAssistant);
			_vm->clearFlag(kGFUnk27);
			updateHotspots();
		} else if (_vm->isFlag(kGFUnk25)) {
			_vm->clearFlag(kGFSpringTaken);
			_vm->clearFlag(kGFUnk16);
			_vm->initPlatypusPos(7, 9, kDirNone);
			_vm->_gnapX = _vm->_hotspotsWalkPos[2].x;
			_vm->_gnapY = _vm->_hotspotsWalkPos[2].y;
			_vm->_gnapId = 20 * _vm->_hotspotsWalkPos[2].y;
			_vm->_gameSys->insertSequence(601, 20 * _vm->_hotspotsWalkPos[2].y, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapSequenceId = 601;
			_vm->_gnapActionStatus = kASGnapHangUpPhone;
			_vm->clearFlag(kGFUnk25);
			_vm->_gameSys->insertSequence(0x251, 254, 0, 0, kSeqNone, 0, 0, 0);
			_vm->endSceneInit();
			_vm->_gameSys->setAnimation(0x257, 254, 0);
			_vm->_gameSys->insertSequence(0x257, 254, 0x251, 254, kSeqSyncWait, 0, 0, 0);
		} else if (_vm->isFlag(kGFPlatypusTalkingToAssistant)) {
			_vm->clearFlag(kGFSpringTaken);
			_vm->clearFlag(kGFUnk16);
			_vm->_sceneWaiting = true;
			_vm->initGnapPos(3, 9, kDirUpLeft);
			_vm->_platX = _vm->_hotspotsWalkPos[2].x;
			_vm->_platY = _vm->_hotspotsWalkPos[2].y;
			_vm->_platypusId = 20 * _vm->_hotspotsWalkPos[2].y;
			_s17_currPhoneSequenceId = 0x251;
			_vm->_gameSys->insertSequence(0x25A, 20 * _vm->_hotspotsWalkPos[2].y, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_gameSys->insertSequence(_s17_currPhoneSequenceId, 254, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_platypusSequenceId = 0x25A;
			_vm->_platypusSequenceDatNum = 0;
			_vm->endSceneInit();
			_vm->_gameSys->setAnimation(_s17_currPhoneSequenceId, 254, 1);
			_vm->_platypusActionStatus = kASPlatPhoningAssistant;
			updateHotspots();
		} else if (_vm->_prevSceneNum == 18) {
			_vm->initGnapPos(6, 6, kDirBottomRight);
			_vm->initPlatypusPos(5, 6, kDirNone);
			_vm->endSceneInit();
			_vm->platypusWalkTo(5, 9, -1, 0x107C2, 1);
			_vm->gnapWalkTo(4, 8, -1, 0x107B9, 1);
		} else {
			if (_vm->isFlag(kGFSpringTaken)) {
				_vm->initGnapPos(_vm->_hotspotsWalkPos[2].x, _vm->_hotspotsWalkPos[2].y, kDirBottomRight);
				_vm->initPlatypusPos(1, 9, kDirNone);
				_vm->endSceneInit();
			} else {
				_vm->initGnapPos(3, 7, kDirBottomRight);
				_vm->initPlatypusPos(1, 7, kDirNone);
				_vm->endSceneInit();
			}
			_vm->clearFlag(kGFSpringTaken);
			_vm->clearFlag(kGFUnk16);
			_vm->endSceneInit();
		}
	} else {
		_vm->_gnapX = 3;
		_vm->_gnapY = 6;
		_vm->_gnapId = 120;
		_vm->_gnapSequenceId = 0x23D;
		_vm->_gnapSequenceDatNum = 0;
		_vm->_gnapIdleFacing = kDirBottomRight;
		_vm->_gameSys->insertSequence(makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, 0, 0, kSeqNone, 0, 0, 0);
		_vm->_platX = -1;
		_vm->_platY = 8;
		_vm->_platypusId = 160;
		_vm->_gameSys->insertSequence(0x241, 160, 0, 0, kSeqNone, 0, 0, 0);
		_vm->_gameSys->insertSequence(0x107C1, _vm->_platypusId, 0x241, _vm->_platypusId,
			kSeqScale | kSeqSyncWait, 0, 75 * _vm->_platX - _vm->_platGridX, 48 * _vm->_platY - _vm->_platGridY);
		_vm->_gameSys->insertSequence(0x22C, 2, 0, 0, kSeqNone, 0, 0, 0);
		// TODO delayTicksA(2, 9);
		_vm->endSceneInit();
		_vm->_platypusSequenceId = 0x7C1;
		_vm->_platypusSequenceDatNum = 1;
		_vm->_platypusFacing = kDirBottomRight;
		_vm->platypusWalkTo(2, 9, -1, 0x107C2, 1);
	}

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);
	
		switch (_vm->_sceneClickedHotspot) {
		case kHSDevice:
			if (_vm->_gnapActionStatus < 0 || _vm->_gnapActionStatus == 3) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHSPlatypus:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemJoint) {
					if (_vm->isFlag(kGFGrassTaken)) {
						_vm->gnapUseJointOnPlatypus();
					} else {
						_vm->gnapUseDeviceOnPlatypuss();
						_vm->platypusWalkTo(_vm->_hotspotsWalkPos[6].x, _vm->_hotspotsWalkPos[6].y, 1, 0x107C2, 1);
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[6].x + 1, _vm->_hotspotsWalkPos[6].y, 0, 0x107BA, 1);
						_vm->_platypusActionStatus = kASGetWrench1;
						_vm->_gnapActionStatus = kASGetWrench1;
						_vm->_timers[5] = _vm->getRandom(30) + 80;
						_vm->setGrabCursorSprite(-1);
						_vm->invRemove(kItemJoint);
					}
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapScratchingHead(_vm->_platX, _vm->_platY);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
					case GRAB_CURSOR:
						_vm->playGnapScratchingHead(_vm->_platX, _vm->_platY);
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

		case kHSWrench:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->isFlag(kGFGrassTaken)) {
					_vm->playGnapImpossible(0, 0);
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].x, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].y, 8, 7);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
					case GRAB_CURSOR:
						_vm->playGnapScratchingHead(8, 7);
						break;
					case TALK_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					case PLAT_CURSOR:
						if (_s17_canTryGetWrench) {
							platHangUpPhone();
							_vm->gnapUseDeviceOnPlatypuss();
							_vm->platypusWalkTo(_vm->_hotspotsWalkPos[6].x + 1, _vm->_hotspotsWalkPos[6].y, 1, 0x107C2, 1);
							_vm->_platypusActionStatus = kASTryGetWrench;
							_vm->_gnapActionStatus = kASTryGetWrench;
							_vm->_timers[5] = _vm->getRandom(30) + 80;
						} else
							_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;
	
		case kHSPhone1:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemDiceQuarterHole) {
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[2].x, _vm->_hotspotsWalkPos[2].y, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
					_vm->_gnapActionStatus = kASPutCoinIntoPhone;
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[2].x, _vm->_hotspotsWalkPos[2].y, 1, 3);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(1, 3);
						break;
					case GRAB_CURSOR:
						if (_vm->isFlag(kGFUnk18)) {
							platHangUpPhone();
							_vm->gnapWalkTo(_vm->_gnapX, _vm->_gnapY, 0, _vm->getGnapSequenceId(gskIdle, _vm->_hotspotsWalkPos[2].x, _vm->_hotspotsWalkPos[2].y) | 0x10000, 1);
							_vm->_gnapActionStatus = kASGetCoinFromPhone;
						} else
							_vm->playGnapImpossible(0, 0);
						break;
					case TALK_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					case PLAT_CURSOR:
						if (_vm->isFlag(kGFUnk18)) {
							platHangUpPhone();
							_vm->_isLeavingScene = true;
							_vm->gnapUseDeviceOnPlatypuss();
							_vm->_platypusFacing = kDirUpLeft;
							_vm->platypusWalkTo(_vm->_hotspotsWalkPos[2].x, _vm->_hotspotsWalkPos[2].y, 1, 0x107C2, 1);
							_vm->setFlag(kGFUnk16);
							_vm->_platypusActionStatus = kASPlatUsePhone;
							_vm->_gnapActionStatus = kASPlatUsePhone;
						} else
							_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSPhone2:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemDiceQuarterHole) {
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[2].x, _vm->_hotspotsWalkPos[2].y, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
					_vm->_gnapActionStatus = kASPutCoinIntoPhone;
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[2].x, _vm->_hotspotsWalkPos[2].y, 1, 3);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(1, 3);
						break;
					case GRAB_CURSOR:
						if (_vm->isFlag(kGFUnk18)) {
							platHangUpPhone();
							_vm->_isLeavingScene = true;
							_vm->_gnapIdleFacing = kDirUpLeft;
							_vm->gnapWalkTo(_vm->_hotspotsWalkPos[2].x, _vm->_hotspotsWalkPos[2].y, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
							_vm->_gnapActionStatus = kASGnapUsePhone;
							_vm->setFlag(kGFSpringTaken);
						} else
							_vm->playGnapImpossible(0, 0);
						break;
					case TALK_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					case PLAT_CURSOR:
						if (_vm->isFlag(kGFUnk18)) {
							platHangUpPhone();
							_vm->_isLeavingScene = true;
							_vm->gnapUseDeviceOnPlatypuss();
							_vm->_platypusFacing = kDirUpLeft;
							_vm->platypusWalkTo(_vm->_hotspotsWalkPos[2].x, _vm->_hotspotsWalkPos[2].y, 1, 0x107C2, 1);
							_vm->setFlag(kGFUnk16);
							_vm->_platypusActionStatus = kASPlatUsePhone;
							_vm->_gnapActionStatus = kASPlatUsePhone;
						} else
							_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;
	
		case kHSExitToyStore:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 18;
				_vm->_gnapIdleFacing = kDirUpRight;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[5].x, _vm->_hotspotsWalkPos[5].y, 0, 0x107BB, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				if (_vm->_platypusActionStatus != kASPlatPhoningAssistant)
					_vm->platypusWalkTo(_vm->_hotspotsWalkPos[5].x - 1, _vm->_hotspotsWalkPos[5].y, -1, 0x107C2, 1);
			}
			break;
	
		case kHSExitGrubCity:
			if (_vm->_gnapActionStatus < 0) {
				platHangUpPhone();
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 20;
				_vm->_gnapIdleFacing = kDirUpLeft;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[3].x, _vm->_hotspotsWalkPos[3].y, 0, 0x107BC, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[3].x + 1, _vm->_hotspotsWalkPos[3].y, -1, 0x107C2, 1);
			}
			break;
	
		case kHSWalkArea1:
		case kHSWalkArea2:
		case kHSWalkArea3:
			if (_vm->_gnapActionStatus < 0)
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			break;
	
		default:
			if (_vm->_mouseClickState._left && _vm->_gnapActionStatus < 0) {
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				_vm->_mouseClickState._left = 0;
			}
			break;
		}
	
		updateAnimations();
		
		if (!_vm->isSoundPlaying(0x10940))
			_vm->playSound(0x10940, true);
	
		if (!_vm->_isLeavingScene) {
			if (_vm->_platypusActionStatus < 0)
				_vm->platypusSub426234();
			_vm->updateGnapIdleSequence2();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(100) + 200;
				if (_vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0)
					_vm->_gameSys->insertSequence(0x22B, 21, 0, 0, kSeqNone, 0, 0, 0);
			}
			if (!_vm->_timers[7]) {
				_vm->_timers[7] = _vm->getRandom(100) + 100;
				if (_vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0) {
					switch (_vm->getRandom(3)) {
					case 0:
						_vm->_gameSys->insertSequence(0x25C, 255, 0, 0, kSeqNone, 0, 0, 0);
						break;
					case 1:
						_vm->_gameSys->insertSequence(0x25D, 255, 0, 0, kSeqNone, 0, 0, 0);
						break;
					case 2:
						_vm->_gameSys->insertSequence(0x25E, 255, 0, 0, kSeqNone, 0, 0, 0);
						break;
					}
				}
			}
			if (_vm->_platypusActionStatus < 0 && !_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(30) + 80;
				if (_vm->isFlag(kGFGrassTaken) && _s17_nextWrenchSequenceId == -1) {
					_s17_nextWrenchSequenceId = 0x236;
				} else if (_s17_canTryGetWrench) {
					switch (_vm->getRandom(6)) {
					case 0:
						_s17_nextWrenchSequenceId = 0x231;
						break;
					case 1:
						_s17_nextWrenchSequenceId = 0x232;
						break;
					case 2:
					case 3:
						_s17_nextWrenchSequenceId = 0x23C;
						break;
					case 4:
					case 5:
						_s17_nextWrenchSequenceId = 0x22E;
						break;
					}
				} else {
					--_s17_wrenchCtr;
					if (_s17_wrenchCtr) {
						switch (_vm->getRandom(6)) {
						case 0:
							_s17_nextWrenchSequenceId = 0x237;
							break;
						case 1:
							_s17_nextWrenchSequenceId = 0x238;
							break;
						case 2:
							_s17_nextWrenchSequenceId = 0x239;
							break;
						case 3:
							_s17_nextWrenchSequenceId = 0x23A;
							break;
						case 4:
							_s17_nextWrenchSequenceId = 0x23B;
							break;
						case 5:
							_s17_nextWrenchSequenceId = 0x235;
							break;
						}
					} else {
						_s17_wrenchCtr = 2;
						_s17_nextWrenchSequenceId = 0x235;
					}
				}
			}
			if (!_vm->_timers[6]) {
				_vm->_timers[6] = _vm->getRandom(30) + 200;
				if (_s17_nextCarWindowSequenceId == -1 && !_vm->isFlag(kGFGrassTaken))
					_s17_nextCarWindowSequenceId = 0x246;
			}
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

void Scene17::updateAnimations() {
	static const int kPlatPhoneSequenceIds[] = {
		0x251, 0x252, 0x253, 0x254, 0x255, 0x256, 0x257
	};

	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kASGetWrench1:
			_vm->_gnapActionStatus = kASGetWrenchGnapReady;
			break;
		case kASGetCoinFromPhone:
			_vm->playGnapPullOutDevice(1, 3);
			_vm->playGnapUseDevice(0, 0);
			_vm->_gameSys->setAnimation(0x250, 100, 0);
			_vm->_gameSys->insertSequence(0x250, 100, 591, 100, kSeqSyncWait, 0, 0, 0);
			_vm->invAdd(kItemDiceQuarterHole);
			_vm->clearFlag(kGFUnk18);
			_vm->_gnapActionStatus = kASGetCoinFromPhoneDone;
			break;
		case kASGetCoinFromPhoneDone:
			_vm->setGrabCursorSprite(kItemDiceQuarterHole);
			_vm->_gnapActionStatus = -1;
			break;
		case kASPutCoinIntoPhone:
			_vm->_gameSys->setAnimation(0x24C, _vm->_gnapId, 0);
			_vm->_gameSys->insertSequence(0x24C, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapSequenceId = 0x24C;
			_vm->invRemove(kItemDiceQuarterHole);
			_vm->setGrabCursorSprite(-1);
			_vm->setFlag(kGFUnk18);
			_vm->_gnapActionStatus = kASPutCoinIntoPhoneDone;
			break;
		case kASPutCoinIntoPhoneDone:
			_vm->_gameSys->insertSequence(0x24F, 100, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_gnapActionStatus = -1;
			break;
		case kASGnapUsePhone:
			_vm->_gameSys->setAnimation(0x24D, _vm->_gnapId, 0);
			_vm->_gameSys->insertSequence(0x24D, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapActionStatus = kASLeaveScene;
			_vm->_newSceneNum = 53;
			break;
		case kASGnapHangUpPhone:
			_vm->_gameSys->insertSequence(0x258, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapSequenceId = 0x258;
			_vm->_gnapActionStatus = -1;
			break;
		case kASLeaveScene:
			_vm->_sceneDone = true;
			break;
		}
	}

	if (_vm->_gameSys->getAnimationStatus(1) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 1);
		switch (_vm->_platypusActionStatus) {
		case kASTryGetWrench:
			_vm->_platypusActionStatus = -1;
			++_s17_platTryGetWrenchCtr;
			if (_s17_platTryGetWrenchCtr % 2 != 0)
				_s17_nextWrenchSequenceId = 0x233;
			else
				_s17_nextWrenchSequenceId = 0x234;
			_s17_canTryGetWrench = false;
			break;
		case kASGetWrench1:
			_s17_nextWrenchSequenceId = 0x230;
			break;
		case kASGetWrench2:
			_s17_nextCarWindowSequenceId = 0x249;
			break;
		case kASGetWrenchDone:
			_vm->_platypusActionStatus = -1;
			_vm->invAdd(kItemWrench);
			_vm->setGrabCursorSprite(kItemWrench);
			break;
		case kASPlatUsePhone:
			_vm->_gameSys->setAnimation(0x24E, _vm->_platypusId, 1);
			_vm->_gameSys->insertSequence(0x24E, _vm->_platypusId, _vm->_platypusSequenceId | (_vm->_platypusSequenceDatNum << 16), _vm->_platypusId, kSeqSyncWait, 0, 0, 0);
			_vm->_platypusSequenceDatNum = 0;
			_vm->_platypusSequenceId = 0x24E;
			_vm->_platypusActionStatus = kASLeaveScene;
			_vm->_newSceneNum = 53;
			break;
		case kASPlatPhoningAssistant:
			++_s17_platPhoneCtr;
			if (_s17_platPhoneCtr >= 7) {
				_s17_platPhoneCtr = 0;
				_s17_nextPhoneSequenceId = -1;
				_s17_currPhoneSequenceId = -1;
				_vm->_gameSys->insertSequence(0x25B, _vm->_platypusId, 0x25A, _vm->_platypusId, kSeqSyncWait, 0, 0, 0);
				_vm->_platypusSequenceDatNum = 0;
				_vm->_platypusSequenceId = 0x25B;
				_vm->_platypusActionStatus = -1;
				_vm->clearFlag(kGFPlatypusTalkingToAssistant);
				_vm->_sceneWaiting = false;
				updateHotspots();
			} else {
				_s17_nextPhoneSequenceId = kPlatPhoneSequenceIds[_s17_platPhoneCtr];
				_vm->_gameSys->setAnimation(_s17_nextPhoneSequenceId, 254, 1);
				_vm->_gameSys->insertSequence(_s17_nextPhoneSequenceId, 254, _s17_currPhoneSequenceId, 254, kSeqSyncWait, 0, 0, 0);
				_vm->_gameSys->insertSequence(0x25A, _vm->_platypusId, 0x25A, _vm->_platypusId, kSeqSyncWait, 0, 0, 0);
				_vm->_platypusSequenceDatNum = 0;
				_vm->_platypusSequenceId = 0x25A;
				_s17_currPhoneSequenceId = _s17_nextPhoneSequenceId;
			}
			break;
		case kASLeaveScene:
			_vm->_sceneDone = true;
			break;
		}
	}

	if (_vm->_gameSys->getAnimationStatus(2) == 2) {
		switch (_s17_nextWrenchSequenceId) {
		case 0x233:
			_vm->_gnapActionStatus = -1;
			_vm->_gameSys->insertSequence(0x243, _vm->_platypusId,
				_vm->_platypusSequenceId | (_vm->_platypusSequenceDatNum << 16), _vm->_platypusId,
				kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(_s17_nextWrenchSequenceId, 40, _s17_currWrenchSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			_s17_currWrenchSequenceId = _s17_nextWrenchSequenceId;
			_s17_nextWrenchSequenceId = -1;
			_vm->_platypusSequenceId = 0x243;
			_vm->_platypusSequenceDatNum = 0;
			_vm->_gameSys->setAnimation(0x243, _vm->_platypusId, 1);
			break;
		case 0x234:
			_vm->_gnapActionStatus = -1;
			_vm->_gameSys->insertSequence(0x242, _vm->_platypusId,
				_vm->_platypusSequenceId | (_vm->_platypusSequenceDatNum << 16), _vm->_platypusId,
				kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(_s17_nextWrenchSequenceId, 40, _s17_currWrenchSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			_s17_currWrenchSequenceId = _s17_nextWrenchSequenceId;
			_s17_nextWrenchSequenceId = -1;
			_vm->_platypusSequenceId = 0x242;
			_vm->_platypusSequenceDatNum = 0;
			_vm->_gameSys->setAnimation(0x242, _vm->_platypusId, 1);
			break;
		case 0x231:
			if (_vm->getRandom(2) != 0)
				_s17_nextCarWindowSequenceId = 0x245;
			else
				_s17_nextCarWindowSequenceId = 0x248;
			_vm->_gameSys->setAnimation(0, 0, 2);
			break;
		case 0x232:
			_s17_nextCarWindowSequenceId = 0x247;
			_vm->_gameSys->setAnimation(0, 0, 2);
			break;
		case 0x22E:
		case 0x235:
			if (_s17_nextWrenchSequenceId == 0x235)
				_vm->_hotspots[kHSWrench]._flags &= ~SF_DISABLED;
			else
				_vm->_hotspots[kHSWrench]._flags |= SF_DISABLED;
			_s17_canTryGetWrench = !_s17_canTryGetWrench;
			_vm->_gameSys->setAnimation(_s17_nextWrenchSequenceId, 40, 2);
			_vm->_gameSys->insertSequence(_s17_nextWrenchSequenceId, 40, _s17_currWrenchSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			_s17_currWrenchSequenceId = _s17_nextWrenchSequenceId;
			_s17_nextWrenchSequenceId = -1;
			break;
		case 0x230:
			if (_vm->_gnapActionStatus == kASGetWrenchGnapReady) {
				_vm->_gameSys->setAnimation(0, 0, 2);
				if (_s17_canTryGetWrench) {
					_vm->_gameSys->insertSequence(0x22E, 40, _s17_currWrenchSequenceId, 40, kSeqSyncWait, 0, 0, 0);
					_s17_currWrenchSequenceId = 0x22E;
					_s17_canTryGetWrench = false;
				}
				_vm->_gameSys->setAnimation(0x23F, _vm->_platypusId, 1);
				_vm->_gameSys->insertSequence(0x10875, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
				_vm->_gameSys->insertSequence(0x23F, _vm->_platypusId,
					_vm->_platypusSequenceId | (_vm->_platypusSequenceDatNum << 16), _vm->_platypusId,
					kSeqSyncWait, 0, 0, 0);
				_vm->_gnapSequenceDatNum = 1;
				_vm->_platypusSequenceDatNum = 0;
				_vm->_gnapSequenceId = 0x875;
				_vm->_platypusSequenceId = 0x23F;
				_vm->gnapWalkTo(3, 8, -1, 0x107B9, 1);
				_vm->_platypusActionStatus = kASGetWrench2;
			}
			break;
		default:
			if (_s17_nextWrenchSequenceId != -1) {
				_vm->_gameSys->setAnimation(_s17_nextWrenchSequenceId, 40, 2);
				_vm->_gameSys->insertSequence(_s17_nextWrenchSequenceId, 40, _s17_currWrenchSequenceId, 40, kSeqSyncWait, 0, 0, 0);
				_s17_currWrenchSequenceId = _s17_nextWrenchSequenceId;
				_s17_nextWrenchSequenceId = -1;
			}
			break;
		}
	}

	if (_vm->_gameSys->getAnimationStatus(3) == 2) {
		switch (_s17_nextCarWindowSequenceId) {
		case 0x246:
			_vm->_gameSys->setAnimation(_s17_nextCarWindowSequenceId, 40, 3);
			_vm->_gameSys->insertSequence(_s17_nextCarWindowSequenceId, 40, _s17_currCarWindowSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			_s17_currCarWindowSequenceId = _s17_nextCarWindowSequenceId;
			_s17_nextCarWindowSequenceId = -1;
			break;
		case 0x245:
		case 0x247:
		case 0x248:
			_vm->_gameSys->setAnimation(_s17_nextWrenchSequenceId, 40, 2);
			_vm->_gameSys->insertSequence(_s17_nextWrenchSequenceId, 40, _s17_currWrenchSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			while (_vm->_gameSys->getAnimationStatus(2) != 2)
				update();
			_vm->_gameSys->setAnimation(_s17_nextCarWindowSequenceId, 40, 3);
			_vm->_gameSys->insertSequence(_s17_nextCarWindowSequenceId, 40, _s17_currCarWindowSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			_s17_currCarWindowSequenceId = _s17_nextCarWindowSequenceId;
			_s17_nextCarWindowSequenceId = -1;
			_s17_currWrenchSequenceId = _s17_nextWrenchSequenceId;
			_s17_nextWrenchSequenceId = -1;
			break;
		case 0x249:
			_vm->_gameSys->setAnimation(0x230, 40, 2);
			_vm->_gameSys->setAnimation(0x240, _vm->_platypusId, 1);
			_vm->_gameSys->insertSequence(0x230, 40, _s17_currWrenchSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(_s17_nextCarWindowSequenceId, 40, _s17_currCarWindowSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x240, _vm->_platypusId, _vm->_platypusSequenceId, _vm->_platypusId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x23E, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceId = 0x23E;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_platypusSequenceId = 0x240;
			_vm->_platypusSequenceDatNum = 0;
			_vm->_gameSys->setAnimation(0x24A, 40, 3);
			_vm->_gameSys->insertSequence(0x24A, 40, _s17_nextCarWindowSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			while (_vm->_gameSys->getAnimationStatus(2) != 2) {
				update();
				if (_vm->_gameSys->getAnimationStatus(3) == 2) {
					_vm->_gameSys->setAnimation(0x24A, 40, 3);
					_vm->_gameSys->insertSequence(0x24A, 40, 586, 40, kSeqSyncWait, 0, 0, 0);
				}
			}
			_vm->_gameSys->insertSequence(0x22D, 40, 560, 40, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(0x24B, 40, 3);
			_vm->_gameSys->insertSequence(0x24B, 40, 586, 40, kSeqSyncWait, 0, 0, 0);
			_s17_currCarWindowSequenceId = 0x24B;
			_s17_nextCarWindowSequenceId = -1;
			_s17_currWrenchSequenceId = 0x22D;
			_s17_nextWrenchSequenceId = -1;
			_vm->setFlag(kGFGrassTaken);
			_vm->_gnapActionStatus = -1;
			_vm->_platypusActionStatus = 2;
			updateHotspots();
			_vm->_timers[5] = _vm->getRandom(30) + 80;
			break;
		}
	}

}

} // End of namespace Gnap
