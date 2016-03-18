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

int GnapEngine::scene17_init() {
	return 0x263;
}

void GnapEngine::scene17_updateHotspots() {
	setHotspot(kHSPlatypus, 1, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSPhone1, 61, 280, 97, 322, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 1, 7);
	setHotspot(kHSPhone2, 80, 204, 178, 468, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 1, 7);
	setHotspot(kHSExitGrubCity, 196, 207, 280, 304, SF_EXIT_U_CURSOR, 3, 5);
	setHotspot(kHSExitToyStore, 567, 211, 716, 322, SF_EXIT_U_CURSOR, 5, 6);
	setHotspot(kHSWrench, 586, 455, 681, 547, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 5, 7);
	setHotspot(kHSWalkArea1, 0, 0, 800, 434);
	setHotspot(kHSWalkArea2, 541, 0, 800, 600);
	setHotspot(kHSWalkArea3, 0, 204, 173, 468);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	if (isFlag(6))
		_hotspots[kHSWrench]._flags = SF_NONE;
	if (isFlag(26)) {
		_hotspots[kHSDevice]._flags = SF_DISABLED;
		_hotspots[kHSPlatypus]._flags = SF_DISABLED;
	}
	_hotspotsCount = 10;
}

void GnapEngine::scene17_update() {
	gameUpdateTick();
	updateMouseCursor();
	updateGrabCursorSprite(0, 0);
	if (_mouseClickState._left) {
		gnapWalkTo(-1, -1, -1, -1, 1);
		_mouseClickState._left = false;
	}
}

void GnapEngine::scene17_platHangUpPhone() {
	int savedGnapActionStatus = _gnapActionStatus;

	if (_beaverActionStatus == kASPlatPhoningAssistant) {
		_gnapActionStatus = kASPlatHangUpPhone;
		updateMouseCursor();
		_s17_platPhoneCtr = 0;
		_beaverActionStatus = -1;
		_gameSys->setAnimation(0x257, 254, 4);
		_gameSys->insertSequence(0x257, 254, _s17_currPhoneSequenceId, 254, kSeqSyncExists, 0, 0, 0);
		while (_gameSys->getAnimationStatus(4) != 2)
			gameUpdateTick();
		_gameSys->setAnimation(0x25B, _beaverId, 1);
		_gameSys->insertSequence(0x25B, _beaverId, _beaverSequenceId | (_beaverSequenceDatNum << 16), _beaverId, kSeqSyncWait, 0, 0, 0);
		_beaverSequenceId = 0x25B;
		_beaverSequenceDatNum = 0;
		_s17_currPhoneSequenceId = -1;
		_s17_nextPhoneSequenceId = -1;
		clearFlag(26);
		while (_gameSys->getAnimationStatus(1) != 2)
			gameUpdateTick();
		_gnapActionStatus = savedGnapActionStatus;
		updateMouseCursor();
	}
	scene17_updateHotspots();
}

void GnapEngine::scene17_run() {

	playSound(0x10940, 1);
	startSoundTimerA(8);
	_sceneWaiting = false;
	_timers[4] = getRandom(100) + 200;
	_timers[3] = 200;
	_timers[5] = getRandom(30) + 80;
	_timers[6] = getRandom(30) + 200;
	_timers[7] = getRandom(100) + 100;

	if (isFlag(9)) {
		_gameSys->insertSequence(0x25F, 20, 0, 0, kSeqNone, 0, 0, 0);
	} else {
		if (_s18_garbageCanPos >= 8) {
			_gameSys->insertSequence(0x260, 20, 0, 0, kSeqNone, 0, 97, 1);
		} else if (_s18_garbageCanPos >= 6) {
			_gameSys->insertSequence(0x260, 20, 0, 0, kSeqNone, 0, 68, 2);
		} else if (_s18_garbageCanPos >= 5) {
			_gameSys->insertSequence(0x260, 20, 0, 0, kSeqNone, 0, 23, -1);
		} else if (_s18_garbageCanPos >= 4) {
			_gameSys->insertSequence(0x260, 20, 0, 0, kSeqNone, 0, -11, -5);
		} else {
			_gameSys->insertSequence(0x260, 20, 0, 0, kSeqNone, 0, -54, -8);
		}
	}

	if (isFlag(19))
		_gameSys->insertSequence(0x262, 1, 0, 0, kSeqNone, 0, 0, 0);

	queueInsertDeviceIcon();

	if (isFlag(6))
		_s17_currWrenchSequenceId = 0x22D;
	else
		_s17_currWrenchSequenceId = 0x22F;

	_s17_currCarWindowSequenceId = 0x244;

	if (isFlag(14))
		_gameSys->insertSequence(0x261, 1, 0, 0, kSeqNone, 0, 0, 0);

	_gameSys->setAnimation(_s17_currWrenchSequenceId, 40, 2);
	_gameSys->insertSequence(_s17_currWrenchSequenceId, 40, 0, 0, kSeqNone, 0, 0, 0);

	if (isFlag(6)) {
		_gameSys->setAnimation(0, 0, 3);
	} else {
		_gameSys->setAnimation(_s17_currCarWindowSequenceId, 40, 3);
		_gameSys->insertSequence(_s17_currCarWindowSequenceId, 40, 0, 0, kSeqNone, 0, 0, 0);
	}

	_s17_canTryGetWrench = true;

	if (isFlag(18))
		_gameSys->insertSequence(0x24F, 100, 0, 0, kSeqNone, 0, 0, 0);

	if (_prevSceneNum == 53 || _prevSceneNum == 18 || _prevSceneNum == 20 || _prevSceneNum == 19) {
		if (_prevSceneNum == 20) {
			initGnapPos(4, 6, 1);
			initBeaverPos(5, 6, 0);
			endSceneInit();
			platypusWalkTo(5, 9, -1, 0x107C2, 1);
			gnapWalkTo(4, 8, -1, 0x107B9, 1);
		} else if (isFlag(27)) {
			initGnapPos(3, 9, 5);
			_platX = _hotspotsWalkPos[2].x;
			_platY = _hotspotsWalkPos[2].y;
			_beaverId = 20 * _hotspotsWalkPos[2].y;
			_gameSys->insertSequence(0x25A, 20 * _hotspotsWalkPos[2].y, 0, 0, kSeqNone, 0, 0, 0);
			_gameSys->insertSequence(0x257, 254, 0, 0, kSeqNone, 0, 0, 0);
			_beaverSequenceId = 0x25A;
			_beaverSequenceDatNum = 0;
			endSceneInit();
			clearFlag(15);
			clearFlag(16);
			_beaverActionStatus = kASPlatPhoningAssistant;
			scene17_platHangUpPhone();
			_gameSys->setAnimation(0, 0, 4);
			clearFlag(26);
			clearFlag(27);
			scene17_updateHotspots();
		} else if (isFlag(25)) {
			clearFlag(15);
			clearFlag(16);
			initBeaverPos(7, 9, 0);
			_gnapX = _hotspotsWalkPos[2].x;
			_gnapY = _hotspotsWalkPos[2].y;
			_gnapId = 20 * _hotspotsWalkPos[2].y;
			_gameSys->insertSequence(601, 20 * _hotspotsWalkPos[2].y, 0, 0, kSeqNone, 0, 0, 0);
			_gnapSequenceDatNum = 0;
			_gnapSequenceId = 601;
			_gnapActionStatus = kASGnapHangUpPhone;
			clearFlag(25);
			_gameSys->insertSequence(0x251, 254, 0, 0, kSeqNone, 0, 0, 0);
			endSceneInit();
			_gameSys->setAnimation(0x257, 254, 0);
			_gameSys->insertSequence(0x257, 254, 0x251, 254, kSeqSyncWait, 0, 0, 0);
		} else if (isFlag(26)) {
			clearFlag(15);
			clearFlag(16);
			_sceneWaiting = true;
			initGnapPos(3, 9, 5);
			_platX = _hotspotsWalkPos[2].x;
			_platY = _hotspotsWalkPos[2].y;
			_beaverId = 20 * _hotspotsWalkPos[2].y;
			_s17_currPhoneSequenceId = 0x251;
			_gameSys->insertSequence(0x25A, 20 * _hotspotsWalkPos[2].y, 0, 0, kSeqNone, 0, 0, 0);
			_gameSys->insertSequence(_s17_currPhoneSequenceId, 254, 0, 0, kSeqNone, 0, 0, 0);
			_beaverSequenceId = 0x25A;
			_beaverSequenceDatNum = 0;
			endSceneInit();
			_gameSys->setAnimation(_s17_currPhoneSequenceId, 254, 1);
			_beaverActionStatus = kASPlatPhoningAssistant;
			scene17_updateHotspots();
		} else if (_prevSceneNum == 18) {
			initGnapPos(6, 6, 1);
			initBeaverPos(5, 6, 0);
			endSceneInit();
			platypusWalkTo(5, 9, -1, 0x107C2, 1);
			gnapWalkTo(4, 8, -1, 0x107B9, 1);
		} else {
			if (isFlag(15)) {
				initGnapPos(_hotspotsWalkPos[2].x, _hotspotsWalkPos[2].y, 1);
				initBeaverPos(1, 9, 0);
				endSceneInit();
			} else {
				initGnapPos(3, 7, 1);
				initBeaverPos(1, 7, 0);
				endSceneInit();
			}
			clearFlag(15);
			clearFlag(16);
			endSceneInit();
		}
	} else {
		_gnapX = 3;
		_gnapY = 6;
		_gnapId = 120;
		_gnapSequenceId = 0x23D;
		_gnapSequenceDatNum = 0;
		_gnapIdleFacing = 1;
		_gameSys->insertSequence(makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, 0, 0, kSeqNone, 0, 0, 0);
		_platX = -1;
		_platY = 8;
		_beaverId = 160;
		_gameSys->insertSequence(0x241, 160, 0, 0, kSeqNone, 0, 0, 0);
		// CHECKME: Check the value of the flag
		_gameSys->insertSequence(0x107C1, _beaverId, 0x241, _beaverId,
			9, 0, 75 * _platX - _platGridX, 48 * _platY - _platGridY);
		_gameSys->insertSequence(0x22C, 2, 0, 0, kSeqNone, 0, 0, 0);
		// TODO delayTicksA(2, 9);
		endSceneInit();
		_beaverSequenceId = 0x7C1;
		_beaverSequenceDatNum = 1;
		_beaverFacing = 1;
		platypusWalkTo(2, 9, -1, 0x107C2, 1);
	}

	while (!_sceneDone) {

		updateMouseCursor();
		updateCursorByHotspot();

		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);
	
		switch (_sceneClickedHotspot) {

		case kHSDevice:
			if (_gnapActionStatus < 0 || _gnapActionStatus == 3) {
				runMenu();
				scene17_updateHotspots();
			}
			break;

		case kHSPlatypus:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex == kItemJoint) {
					if (isFlag(6)) {
						gnapUseJointOnPlatypus();
					} else {
						gnapUseDeviceOnBeaver();
						platypusWalkTo(_hotspotsWalkPos[6].x, _hotspotsWalkPos[6].y, 1, 0x107C2, 1);
						gnapWalkTo(_hotspotsWalkPos[6].x + 1, _hotspotsWalkPos[6].y, 0, 0x107BA, 1);
						_beaverActionStatus = kASGetWrench1;
						_gnapActionStatus = kASGetWrench1;
						_timers[5] = getRandom(30) + 80;
						setGrabCursorSprite(-1);
						invRemove(kItemJoint);
					}
				} else if (_grabCursorSpriteIndex >= 0) {
					playGnapScratchingHead(_platX, _platY);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
					case GRAB_CURSOR:
						playGnapScratchingHead(_platX, _platY);
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

		case kHSWrench:
			if (_gnapActionStatus < 0) {
				if (isFlag(6)) {
					playGnapImpossible(0, 0);
				} else if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[_sceneClickedHotspot].x, _hotspotsWalkPos[_sceneClickedHotspot].y, 8, 7);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
					case GRAB_CURSOR:
						playGnapScratchingHead(8, 7);
						break;
					case TALK_CURSOR:
						playGnapImpossible(0, 0);
						break;
					case PLAT_CURSOR:
						if (_s17_canTryGetWrench) {
							scene17_platHangUpPhone();
							gnapUseDeviceOnBeaver();
							platypusWalkTo(_hotspotsWalkPos[6].x + 1, _hotspotsWalkPos[6].y, 1, 0x107C2, 1);
							_beaverActionStatus = kASTryGetWrench;
							_gnapActionStatus = kASTryGetWrench;
							_timers[5] = getRandom(30) + 80;
						} else
							playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;
	
		case kHSPhone1:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex == kItemDiceQuarterHole) {
					gnapWalkTo(_hotspotsWalkPos[2].x, _hotspotsWalkPos[2].y, 0, getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
					_gnapActionStatus = kASPutCoinIntoPhone;
				} else if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[2].x, _hotspotsWalkPos[2].y, 1, 3);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(1, 3);
						break;
					case GRAB_CURSOR:
						if (isFlag(18)) {
							scene17_platHangUpPhone();
							gnapWalkTo(_gnapX, _gnapY, 0, getGnapSequenceId(gskIdle, _hotspotsWalkPos[2].x, _hotspotsWalkPos[2].y) | 0x10000, 1);
							_gnapActionStatus = kASGetCoinFromPhone;
						} else
							playGnapImpossible(0, 0);
						break;
					case TALK_CURSOR:
						playGnapImpossible(0, 0);
						break;
					case PLAT_CURSOR:
						if (isFlag(18)) {
							scene17_platHangUpPhone();
							_isLeavingScene = true;
							gnapUseDeviceOnBeaver();
							_beaverFacing = 5;
							platypusWalkTo(_hotspotsWalkPos[2].x, _hotspotsWalkPos[2].y, 1, 0x107C2, 1);
							setFlag(16);
							_beaverActionStatus = kASPlatUsePhone;
							_gnapActionStatus = kASPlatUsePhone;
						} else
							playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSPhone2:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex == kItemDiceQuarterHole) {
					gnapWalkTo(_hotspotsWalkPos[2].x, _hotspotsWalkPos[2].y, 0, getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
					_gnapActionStatus = kASPutCoinIntoPhone;
				} else if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[2].x, _hotspotsWalkPos[2].y, 1, 3);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(1, 3);
						break;
					case GRAB_CURSOR:
						if (isFlag(18)) {
							scene17_platHangUpPhone();
							_isLeavingScene = true;
							_gnapIdleFacing = 5;
							gnapWalkTo(_hotspotsWalkPos[2].x, _hotspotsWalkPos[2].y, 0, getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
							_gnapActionStatus = kASGnapUsePhone;
							setFlag(15);
						} else
							playGnapImpossible(0, 0);
						break;
					case TALK_CURSOR:
						playGnapImpossible(0, 0);
						break;
					case PLAT_CURSOR:
						if (isFlag(18)) {
							scene17_platHangUpPhone();
							_isLeavingScene = true;
							gnapUseDeviceOnBeaver();
							_beaverFacing = 5;
							platypusWalkTo(_hotspotsWalkPos[2].x, _hotspotsWalkPos[2].y, 1, 0x107C2, 1);
							setFlag(16);
							_beaverActionStatus = kASPlatUsePhone;
							_gnapActionStatus = kASPlatUsePhone;
						} else
							playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;
	
		case kHSExitToyStore:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = true;
				_newSceneNum = 18;
				_gnapIdleFacing = 7;
				gnapWalkTo(_hotspotsWalkPos[5].x, _hotspotsWalkPos[5].y, 0, 0x107BB, 1);
				_gnapActionStatus = kASLeaveScene;
				if (_beaverActionStatus != kASPlatPhoningAssistant)
					platypusWalkTo(_hotspotsWalkPos[5].x - 1, _hotspotsWalkPos[5].y, -1, 0x107C2, 1);
			}
			break;
	
		case kHSExitGrubCity:
			if (_gnapActionStatus < 0) {
				scene17_platHangUpPhone();
				_isLeavingScene = true;
				_newSceneNum = 20;
				_gnapIdleFacing = 5;
				gnapWalkTo(_hotspotsWalkPos[3].x, _hotspotsWalkPos[3].y, 0, 0x107BC, 1);
				_gnapActionStatus = kASLeaveScene;
				platypusWalkTo(_hotspotsWalkPos[3].x + 1, _hotspotsWalkPos[3].y, -1, 0x107C2, 1);
			}
			break;
	
		case kHSWalkArea1:
		case kHSWalkArea2:
		case kHSWalkArea3:
			if (_gnapActionStatus < 0)
				gnapWalkTo(-1, -1, -1, -1, 1);
			break;
	
		default:
			if (_mouseClickState._left && _gnapActionStatus < 0) {
				gnapWalkTo(-1, -1, -1, -1, 1);
				_mouseClickState._left = 0;
			}
			break;
		}
	
		scene17_updateAnimations();
		
		if (!isSoundPlaying(0x10940))
			playSound(0x10940, 1);
	
		if (!_isLeavingScene) {
			if (_beaverActionStatus < 0)
				beaverSub426234();
			updateGnapIdleSequence2();
			if (!_timers[4]) {
				_timers[4] = getRandom(100) + 200;
				if (_gnapActionStatus < 0 && _beaverActionStatus < 0)
					_gameSys->insertSequence(0x22B, 21, 0, 0, kSeqNone, 0, 0, 0);
			}
			if (!_timers[7]) {
				_timers[7] = getRandom(100) + 100;
				if (_gnapActionStatus < 0 && _beaverActionStatus < 0) {
					switch (getRandom(3)) {
					case 0:
						_gameSys->insertSequence(0x25C, 255, 0, 0, kSeqNone, 0, 0, 0);
						break;
					case 1:
						_gameSys->insertSequence(0x25D, 255, 0, 0, kSeqNone, 0, 0, 0);
						break;
					case 2:
						_gameSys->insertSequence(0x25E, 255, 0, 0, kSeqNone, 0, 0, 0);
						break;
					}
				}
			}
			if (_beaverActionStatus < 0 && !_timers[5]) {
				_timers[5] = getRandom(30) + 80;
				if (isFlag(6) && _s17_nextWrenchSequenceId == -1) {
					_s17_nextWrenchSequenceId = 0x236;
				} else if (_s17_canTryGetWrench) {
					switch (getRandom(6)) {
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
						switch (getRandom(6)) {
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
			if (!_timers[6]) {
				_timers[6] = getRandom(30) + 200;
				if (_s17_nextCarWindowSequenceId == -1 && !isFlag(6))
					_s17_nextCarWindowSequenceId = 0x246;
			}
			playSoundA();
		}
	
		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene17_updateHotspots();
		}
		
		gameUpdateTick();
		
	}

}

void GnapEngine::scene17_updateAnimations() {

	static const int kPlatPhoneSequenceIds[] = {
		0x251, 0x252, 0x253, 0x254, 0x255, 0x256, 0x257
	};

	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		switch (_gnapActionStatus) {
		case kASGetWrench1:
			_gnapActionStatus = kASGetWrenchGnapReady;
			break;
		case kASGetCoinFromPhone:
			playGnapPullOutDevice(1, 3);
			playGnapUseDevice(0, 0);
			_gameSys->setAnimation(0x250, 100, 0);
			_gameSys->insertSequence(0x250, 100, 591, 100, kSeqSyncWait, 0, 0, 0);
			invAdd(kItemDiceQuarterHole);
			clearFlag(18);
			_gnapActionStatus = kASGetCoinFromPhoneDone;
			break;
		case kASGetCoinFromPhoneDone:
			setGrabCursorSprite(kItemDiceQuarterHole);
			_gnapActionStatus = -1;
			break;
		case kASPutCoinIntoPhone:
			_gameSys->setAnimation(0x24C, _gnapId, 0);
			_gameSys->insertSequence(0x24C, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gnapSequenceDatNum = 0;
			_gnapSequenceId = 0x24C;
			invRemove(kItemDiceQuarterHole);
			setGrabCursorSprite(-1);
			setFlag(18);
			_gnapActionStatus = kASPutCoinIntoPhoneDone;
			break;
		case kASPutCoinIntoPhoneDone:
			_gameSys->insertSequence(0x24F, 100, 0, 0, kSeqNone, 0, 0, 0);
			_gnapActionStatus = -1;
			break;
		case kASGnapUsePhone:
			_gameSys->setAnimation(0x24D, _gnapId, 0);
			_gameSys->insertSequence(0x24D, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gnapActionStatus = kASLeaveScene;
			_newSceneNum = 53;
			break;
		case kASGnapHangUpPhone:
			_gameSys->insertSequence(0x258, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gnapSequenceDatNum = 0;
			_gnapSequenceId = 0x258;
			_gnapActionStatus = -1;
			break;
		case kASLeaveScene:
			_sceneDone = true;
			break;
		}
	}

	if (_gameSys->getAnimationStatus(1) == 2) {
		_gameSys->setAnimation(0, 0, 1);
		switch (_beaverActionStatus) {
		case kASTryGetWrench:
			_beaverActionStatus = -1;
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
			_beaverActionStatus = -1;
			invAdd(kItemWrench);
			setGrabCursorSprite(kItemWrench);
			break;
		case kASPlatUsePhone:
			_gameSys->setAnimation(0x24E, _beaverId, 1);
			_gameSys->insertSequence(0x24E, _beaverId, _beaverSequenceId | (_beaverSequenceDatNum << 16), _beaverId, kSeqSyncWait, 0, 0, 0);
			_beaverSequenceDatNum = 0;
			_beaverSequenceId = 0x24E;
			_beaverActionStatus = kASLeaveScene;
			_newSceneNum = 53;
			break;
		case kASPlatPhoningAssistant:
			++_s17_platPhoneCtr;
			if (_s17_platPhoneCtr >= 7) {
				_s17_platPhoneCtr = 0;
				_s17_nextPhoneSequenceId = -1;
				_s17_currPhoneSequenceId = -1;
				_gameSys->insertSequence(0x25B, _beaverId, 0x25A, _beaverId, kSeqSyncWait, 0, 0, 0);
				_beaverSequenceDatNum = 0;
				_beaverSequenceId = 0x25B;
				_beaverActionStatus = -1;
				clearFlag(26);
				_sceneWaiting = false;
				scene17_updateHotspots();
			} else {
				_s17_nextPhoneSequenceId = kPlatPhoneSequenceIds[_s17_platPhoneCtr];
				_gameSys->setAnimation(_s17_nextPhoneSequenceId, 254, 1);
				_gameSys->insertSequence(_s17_nextPhoneSequenceId, 254, _s17_currPhoneSequenceId, 254, kSeqSyncWait, 0, 0, 0);
				_gameSys->insertSequence(0x25A, _beaverId, 0x25A, _beaverId, kSeqSyncWait, 0, 0, 0);
				_beaverSequenceDatNum = 0;
				_beaverSequenceId = 0x25A;
				_s17_currPhoneSequenceId = _s17_nextPhoneSequenceId;
			}
			break;
		case kASLeaveScene:
			_sceneDone = true;
			break;
		}
	}

	if (_gameSys->getAnimationStatus(2) == 2) {
		switch (_s17_nextWrenchSequenceId) {
		case 0x233:
			_gnapActionStatus = -1;
			_gameSys->insertSequence(0x243, _beaverId,
				_beaverSequenceId | (_beaverSequenceDatNum << 16), _beaverId,
				kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(_s17_nextWrenchSequenceId, 40, _s17_currWrenchSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			_s17_currWrenchSequenceId = _s17_nextWrenchSequenceId;
			_s17_nextWrenchSequenceId = -1;
			_beaverSequenceId = 0x243;
			_beaverSequenceDatNum = 0;
			_gameSys->setAnimation(0x243, _beaverId, 1);
			break;
		case 0x234:
			_gnapActionStatus = -1;
			_gameSys->insertSequence(0x242, _beaverId,
				_beaverSequenceId | (_beaverSequenceDatNum << 16), _beaverId,
				kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(_s17_nextWrenchSequenceId, 40, _s17_currWrenchSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			_s17_currWrenchSequenceId = _s17_nextWrenchSequenceId;
			_s17_nextWrenchSequenceId = -1;
			_beaverSequenceId = 0x242;
			_beaverSequenceDatNum = 0;
			_gameSys->setAnimation(0x242, _beaverId, 1);
			break;
		case 0x231:
			if (getRandom(2) != 0)
				_s17_nextCarWindowSequenceId = 0x245;
			else
				_s17_nextCarWindowSequenceId = 0x248;
			_gameSys->setAnimation(0, 0, 2);
			break;
		case 0x232:
			_s17_nextCarWindowSequenceId = 0x247;
			_gameSys->setAnimation(0, 0, 2);
			break;
		case 0x22E:
		case 0x235:
			if (_s17_nextWrenchSequenceId == 0x235)
				_hotspots[kHSWrench]._flags &= ~SF_DISABLED;
			else
				_hotspots[kHSWrench]._flags |= SF_DISABLED;
			_s17_canTryGetWrench = !_s17_canTryGetWrench;
			_gameSys->setAnimation(_s17_nextWrenchSequenceId, 40, 2);
			_gameSys->insertSequence(_s17_nextWrenchSequenceId, 40, _s17_currWrenchSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			_s17_currWrenchSequenceId = _s17_nextWrenchSequenceId;
			_s17_nextWrenchSequenceId = -1;
			break;
		case 0x230:
			if (_gnapActionStatus == kASGetWrenchGnapReady) {
				_gameSys->setAnimation(0, 0, 2);
				if (_s17_canTryGetWrench) {
					_gameSys->insertSequence(0x22E, 40, _s17_currWrenchSequenceId, 40, kSeqSyncWait, 0, 0, 0);
					_s17_currWrenchSequenceId = 0x22E;
					_s17_canTryGetWrench = false;
				}
				_gameSys->setAnimation(0x23F, _beaverId, 1);
				_gameSys->insertSequence(0x10875, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
				_gameSys->insertSequence(0x23F, _beaverId,
					_beaverSequenceId | (_beaverSequenceDatNum << 16), _beaverId,
					kSeqSyncWait, 0, 0, 0);
				_gnapSequenceDatNum = 1;
				_beaverSequenceDatNum = 0;
				_gnapSequenceId = 0x875;
				_beaverSequenceId = 0x23F;
				gnapWalkTo(3, 8, -1, 0x107B9, 1);
				_beaverActionStatus = kASGetWrench2;
			}
			break;
		default:
			if (_s17_nextWrenchSequenceId != -1) {
				_gameSys->setAnimation(_s17_nextWrenchSequenceId, 40, 2);
				_gameSys->insertSequence(_s17_nextWrenchSequenceId, 40, _s17_currWrenchSequenceId, 40, kSeqSyncWait, 0, 0, 0);
				_s17_currWrenchSequenceId = _s17_nextWrenchSequenceId;
				_s17_nextWrenchSequenceId = -1;
			}
			break;
		}
	}

	if (_gameSys->getAnimationStatus(3) == 2) {
		switch (_s17_nextCarWindowSequenceId) {
		case 0x246:
			_gameSys->setAnimation(_s17_nextCarWindowSequenceId, 40, 3);
			_gameSys->insertSequence(_s17_nextCarWindowSequenceId, 40, _s17_currCarWindowSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			_s17_currCarWindowSequenceId = _s17_nextCarWindowSequenceId;
			_s17_nextCarWindowSequenceId = -1;
			break;
		case 0x245:
		case 0x247:
		case 0x248:
			_gameSys->setAnimation(_s17_nextWrenchSequenceId, 40, 2);
			_gameSys->insertSequence(_s17_nextWrenchSequenceId, 40, _s17_currWrenchSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			while (_gameSys->getAnimationStatus(2) != 2)
				scene17_update();
			_gameSys->setAnimation(_s17_nextCarWindowSequenceId, 40, 3);
			_gameSys->insertSequence(_s17_nextCarWindowSequenceId, 40, _s17_currCarWindowSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			_s17_currCarWindowSequenceId = _s17_nextCarWindowSequenceId;
			_s17_nextCarWindowSequenceId = -1;
			_s17_currWrenchSequenceId = _s17_nextWrenchSequenceId;
			_s17_nextWrenchSequenceId = -1;
			break;
		case 0x249:
			_gameSys->setAnimation(0x230, 40, 2);
			_gameSys->setAnimation(0x240, _beaverId, 1);
			_gameSys->insertSequence(0x230, 40, _s17_currWrenchSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(_s17_nextCarWindowSequenceId, 40, _s17_currCarWindowSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0x240, _beaverId, _beaverSequenceId, _beaverId, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0x23E, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gnapSequenceId = 0x23E;
			_gnapSequenceDatNum = 0;
			_beaverSequenceId = 0x240;
			_beaverSequenceDatNum = 0;
			_gameSys->setAnimation(0x24A, 40, 3);
			_gameSys->insertSequence(0x24A, 40, _s17_nextCarWindowSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			while (_gameSys->getAnimationStatus(2) != 2) {
				scene17_update();
				if (_gameSys->getAnimationStatus(3) == 2) {
					_gameSys->setAnimation(0x24A, 40, 3);
					_gameSys->insertSequence(0x24A, 40, 586, 40, kSeqSyncWait, 0, 0, 0);
				}
			}
			_gameSys->insertSequence(0x22D, 40, 560, 40, kSeqSyncWait, 0, 0, 0);
			_gameSys->setAnimation(0x24B, 40, 3);
			_gameSys->insertSequence(0x24B, 40, 586, 40, kSeqSyncWait, 0, 0, 0);
			_s17_currCarWindowSequenceId = 0x24B;
			_s17_nextCarWindowSequenceId = -1;
			_s17_currWrenchSequenceId = 0x22D;
			_s17_nextWrenchSequenceId = -1;
			setFlag(6);
			_gnapActionStatus = -1;
			_beaverActionStatus = 2;
			scene17_updateHotspots();
			_timers[5] = getRandom(30) + 80;
			break;
		}
	}

}

} // End of namespace Gnap
