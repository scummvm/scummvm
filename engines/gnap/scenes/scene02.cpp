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

int GnapEngine::scene02_init() {
	_gameSys->setAnimation(0, 0, 0);
	return isFlag(9) ? 0x15A : 0x15B;
}

void GnapEngine::scene02_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_DISABLED | SF_WALKABLE);
	setHotspot(kHSChicken, 606, 455, 702, 568, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 7, 8);
	setHotspot(kHSTruck1, 385, 258, 464, 304, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 5);
	setHotspot(kHSTruck2, 316, 224, 390, 376, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 5, 6);
	setHotspot(kHSTruckGrill, 156, 318, 246, 390, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 2, 7);
	setHotspot(kHSExitHouse, 480, 120, 556, 240, SF_EXIT_U_CURSOR, 7, 5);
	setHotspot(kHSExitBarn, 610, 0, 800, 164, SF_EXIT_U_CURSOR, 10, 5);
	setHotspot(kHSExitCreek, 780, 336, 800, 556, SF_EXIT_R_CURSOR | SF_WALKABLE, 10, 8);
	setHotspot(kHSExitPigpen, 0, 300, 20, 600, SF_EXIT_L_CURSOR | SF_WALKABLE, 0, 8);
	setHotspot(kHSWalkArea1, 92, 140, 304, 430, 0, 3, 1);
	setHotspot(kHSWalkArea2, 0, 0, 800, 380);
	setHotspot(kHSWalkArea3, 0, 0, 386, 445);
	setHotspot(kHSWalkArea4, 386, 0, 509, 410);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	if (isFlag(0))
		_hotspots[kHSPlatypus].flags = SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	_hotspotsCount = 14;
}

void GnapEngine::scene02_run() {

	playSound(0x1091C, 1);
	startSoundTimerC(6);

	_s02_currChickenSequenceId = 0x14B;
	_gameSys->setAnimation(0x14B, 179, 2);
	_gameSys->insertSequence(0x14B, 179, 0, 0, kSeqNone, 0, 0, 0);

	_s02_nextChickenSequenceId = -1;
	_timers[5] = getRandom(20) + 30;
	_timers[4] = getRandom(100) + 300;
	
	queueInsertDeviceIcon();
	
	switch (_prevSceneNum) {
	case 3:
		initGnapPos(11, 6, 3);
		if (isFlag(0))
			initBeaverPos(12, 6, 4);
		endSceneInit();
		if (isFlag(0))
			platypusWalkTo(9, 6, -1, 0x107C2, 1);
		gnapWalkTo(8, 6, -1, 0x107BA, 1);
		break;
	case 4:
		initGnapPos(_hotspotsWalkPos[6].x, _hotspotsWalkPos[6].y, 3);
		if (isFlag(0))
			initBeaverPos(_hotspotsWalkPos[6].x + 1, _hotspotsWalkPos[6].y, 4);
		endSceneInit();
		gnapWalkTo(7, 6, 0, 0x107B9, 1);
		if (isFlag(0))
			platypusWalkTo(8, 6, 1, 0x107C2, 1);
		scene02_updateHotspots();
		_gameSys->waitForUpdate();
		break;
	case 47:
		clearFlag(25);
		initGnapPos(5, 6, 3);
		initBeaverPos(6, 7, 4);
		endSceneInit();
		break;
	case 49:
		initGnapPos(5, 6, 1);
		if (isFlag(0))
			initBeaverPos(6, 7, 0);
		endSceneInit();
		break;
	default:
		initGnapPos(-1, 6, 1);
		if (isFlag(0))
			initBeaverPos(-1, 7, 0);
		endSceneInit();
		if (isFlag(0))
			platypusWalkTo(2, 7, -1, 0x107C2, 1);
		gnapWalkTo(2, 8, -1, 0x107B9, 1);
		break;
	}
	
	while (!_sceneDone) {
	
		updateMouseCursor();
	
		updateCursorByHotspot();
	
		testWalk(0, 6, 7, 6, 8, 6);
	
		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);
	
		switch (_sceneClickedHotspot) {

		case kHSDevice:
			if (_gnapActionStatus < 0) {
				runMenu();
				scene02_updateHotspots();
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
						gnapKissPlatypus(0);
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

		case kHSChicken:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex == kItemTwig) {
					_gnapIdleFacing = 7;
					gnapWalkTo(_hotspotsWalkPos[_sceneClickedHotspot].x, _hotspotsWalkPos[_sceneClickedHotspot].y + 1,
						0, getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
					_gnapActionStatus = kASUseTwigWithChicken;
				} else if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[1].x, _hotspotsWalkPos[1].y + 1, 9, 8);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapMoan2(9, 8);
						break;
					case GRAB_CURSOR:
						_gnapIdleFacing = 1;
						if (gnapWalkTo(_hotspotsWalkPos[1].x, _hotspotsWalkPos[1].y, 0, getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1))
							_gnapActionStatus = kASGrabChicken;
						else
							_gnapActionStatus = -1;
						break;
					case TALK_CURSOR:
						_gnapIdleFacing = 1;
						gnapWalkTo(_hotspotsWalkPos[1].x, _hotspotsWalkPos[1].y, 0, getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_gnapActionStatus = kASTalkChicken;
						break;
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSTruck1:
		case kHSTruck2:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex == kItemKeys) {
					if (gnapWalkTo(_hotspotsWalkPos[3].x, _hotspotsWalkPos[3].y, 0, getGnapSequenceId(gskIdle, 2, 2) | 0x10000, 1)) {
						setGrabCursorSprite(-1);
						invRemove(kItemKeys);
						if (isFlag(8))
							_gnapActionStatus = kASUseTruckGas;
						else
							_gnapActionStatus = kASUseTruckNoGas;
					}
				} else if (_grabCursorSpriteIndex == kItemGas) {
					_hotspots[kHSWalkArea4].flags |= SF_WALKABLE;
					if (gnapWalkTo(_hotspotsWalkPos[2].x, _hotspotsWalkPos[2].y, 0, getGnapSequenceId(gskIdle, 2, 2) | 0x10000, 1))
						_gnapActionStatus = kASUseGasWithTruck;
					_hotspots[kHSWalkArea4].flags &= ~SF_WALKABLE;
				} else if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[2].x, _hotspotsWalkPos[2].y, 2, 2);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(2, 2);
						break;
					case GRAB_CURSOR:
						if (isFlag(9)) {
							if (gnapWalkTo(_hotspotsWalkPos[3].x, _hotspotsWalkPos[3].y, 0, getGnapSequenceId(gskIdle, 2, 2) | 0x10000, 1)) {
								if (isFlag(8))
									_gnapActionStatus = kASUseTruckGas;
								else
									_gnapActionStatus = kASUseTruckNoGas;
							}
						} else {
							_gnapIdleFacing = 4;
							if (gnapWalkTo(_hotspotsWalkPos[3].x, _hotspotsWalkPos[3].y, 0, getGnapSequenceId(gskIdle, 2, 2) | 0x10000, 1))
								_gnapActionStatus = kASUseTruckNoKeys;
						}
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSTruckGrill:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[4].x, _hotspotsWalkPos[4].y, 2, 4);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapMoan2(2, 4);
						break;
					case GRAB_CURSOR:
						_gnapIdleFacing = 7;
						gnapWalkTo(_hotspotsWalkPos[4].x, _hotspotsWalkPos[4].y, 0, getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
						_gnapActionStatus = kASGrabTruckGrill;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSExitHouse:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = 1;
				gnapWalkTo(_hotspotsWalkPos[6].x, _hotspotsWalkPos[6].y, 0, 0x107AD, 1);
				_gnapActionStatus = kASLeaveScene;
				if (isFlag(0))
					platypusWalkTo(_hotspotsWalkPos[6].x + 1, _hotspotsWalkPos[6].y, -1, 0x107C1, 1);
				scene02_updateHotspots();
				_newSceneNum = 4;
			}
			break;

		case kHSExitBarn:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = 1;
				gnapWalkTo(_hotspotsWalkPos[7].x, _hotspotsWalkPos[7].y, 0, 0x107AD, 1);
				_gnapActionStatus = kASLeaveScene;
				if (isFlag(0))
					platypusWalkTo(_hotspotsWalkPos[7].x + 1, _hotspotsWalkPos[7].y, -1, 0x107C1, 1);
				scene02_updateHotspots();
				_newSceneNum = 5;
			}
			break;

		case kHSExitCreek:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = 1;
				gnapWalkTo(_hotspotsWalkPos[8].x, _hotspotsWalkPos[8].y, 0, 0x107AB, 1);
				_gnapActionStatus = kASLeaveScene;
				if (isFlag(0))
					platypusWalkTo(_hotspotsWalkPos[8].x, _hotspotsWalkPos[8].y, -1, 0x107CD, 1);
				_newSceneNum = 3;
			}
			break;

		case kHSExitPigpen:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = 1;
				gnapWalkTo(_hotspotsWalkPos[9].x, _hotspotsWalkPos[9].y, 0, 0x107AF, 1);
				_gnapActionStatus = kASLeaveScene;
				if (isFlag(0))
					platypusWalkTo(_hotspotsWalkPos[9].x, _hotspotsWalkPos[9].y, -1, 0x107CF, 1);
				_newSceneNum = 1;
			}
			break;

		case kHSWalkArea1:
		case kHSWalkArea2:
		case kHSWalkArea3:
		case kHSWalkArea4:
			if (_gnapActionStatus < 0)
				gnapWalkTo(-1, -1, -1, -1, 1);
			break;

		default:
			if (_mouseClickState.left && _gnapActionStatus < 0) {
				gnapWalkTo(-1, -1, -1, -1, 1);
				_mouseClickState.left = false;
			}
			break;

		}
	
		scene02_updateAnimations();
	
		if (!isSoundPlaying(0x1091C))
			playSound(0x1091C, 1);
	
		if (!_isLeavingScene) {
			if (_beaverActionStatus < 0 && isFlag(0))
				updateBeaverIdleSequence();
			if (_gnapActionStatus < 0)
				updateGnapIdleSequence();
			if (!_timers[4]) {
				// Update bird animation
				_timers[4] = getRandom(100) + 300;
				if (getRandom(2) != 0)
					_gameSys->insertSequence(0x156, 256, 0, 0, kSeqNone, 0, 0, 0);
				else
					_gameSys->insertSequence(0x154, 256, 0, 0, kSeqNone, 0, 0, 0);
			}
			if (!_timers[5] && _s02_nextChickenSequenceId == -1 && _gnapActionStatus != 7 && _gnapActionStatus != 8) {
				if (getRandom(6) != 0) {
					_s02_nextChickenSequenceId = 0x14B;
					_timers[5] = getRandom(20) + 30;
				} else {
					_s02_nextChickenSequenceId = 0x14D;
					_timers[5] = getRandom(20) + 50;
				}
			}
			playSoundC();
		}
	
		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene02_updateHotspots();
		}
		
		gameUpdateTick();
	
	}

}

void GnapEngine::scene02_updateAnimations() {

	if (_gameSys->getAnimationStatus(0) == 2) {
		switch (_gnapActionStatus) {
		case kASUseTruckNoKeys:
			_gameSys->insertSequence(0x14E, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gameSys->setAnimation(0x14E, _gnapId, 0);
			_gnapSequenceId = 0x14E;
			_gnapSequenceDatNum = 0;
			_gnapActionStatus = kASUseTruckNoKeysDone;
			break;
		case kASUseGasWithTruck:
			_gameSys->insertSequence(0x151, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gameSys->setAnimation(0x151, _gnapId, 0);
			_gnapSequenceId = 0x151;
			_gnapSequenceDatNum = 0;
			invRemove(kItemGas);
			setGrabCursorSprite(-1);
			setFlag(8);
			_gnapActionStatus = kASUseGasWithTruckDone;
			break;
		case kASUseTruckGas:
			_timers[5] = 9999;
			_timers[4] = 9999;
			hideCursor();
			setGrabCursorSprite(-1);
			if (!isFlag(9)) {
				_gameSys->insertSequence(0x14F, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
				_gameSys->waitForUpdate();
				setFlag(9);
				_gnapSequenceId = 0x14F;
				_gnapSequenceDatNum = 0;
				invRemove(kItemKeys);
				setGrabCursorSprite(-1);
			}
			_newSceneNum = 47;
			_sceneDone = true;
			break;
		case kASUseTruckNoGas:
			hideCursor();
			setGrabCursorSprite(-1);
			_timers[4] = 250;
			if (!isFlag(9)) {
				_gameSys->insertSequence(0x14F, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
				_gameSys->waitForUpdate();
				setFlag(9);
				_gnapSequenceId = 0x14F;
				_gnapSequenceDatNum = 0;
				invRemove(kItemKeys);
				setGrabCursorSprite(-1);
			}
			_newSceneNum = 47;
			_sceneDone = true;
			setFlag(25);
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
			_gameSys->insertSequence(_s02_gnapTruckSequenceId, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gameSys->setAnimation(_s02_gnapTruckSequenceId, _gnapId, 0);
			_gnapSequenceId = _s02_gnapTruckSequenceId;
			_gnapSequenceDatNum = 0;
			_gnapActionStatus = -1;
			break;
		case kASLeaveScene:
			_sceneDone = true;
			break;
		case kASTalkChicken:
			_s02_nextChickenSequenceId = 0x14C;
			break;
		case kASGrabChicken:
			_s02_nextChickenSequenceId = 0x150;
			_timers[2] = 100;
			break;
		case kASGrabChickenDone:
			_gameSys->insertSequence(0x107B5, _gnapId, 0x150, 179, kSeqSyncWait, 0, 75 * _gnapX - _gnapGridX, 48 * _gnapY - _gnapGridY);
			_gnapSequenceId = 0x7B5;
			_gnapSequenceDatNum = 1;
			_s02_currChickenSequenceId = 0x14B;
			_gameSys->setAnimation(0x14B, 179, 2);
			_gameSys->insertSequence(_s02_currChickenSequenceId, 179, 0, 0, kSeqNone, 0, 0, 0);
			_gnapActionStatus = -1;
			_timers[5] = 30;
			break;
		case kASUseTwigWithChicken:
			playGnapShowItem(5, 0, 0);
			_gameSys->insertSequence(0x155, 179, _s02_currChickenSequenceId, 179, 32, 0, 0, 0);
			_s02_currChickenSequenceId = 0x155;
			_s02_nextChickenSequenceId = -1;
			_gnapActionStatus = -1;
			break;
		case kASUseTruckNoKeysDone:
		case kASUseGasWithTruckDone:
		default:
			_gnapActionStatus = -1;
			break;
		}
	}
	
	if (_gameSys->getAnimationStatus(2) == 2) {
		if (_s02_nextChickenSequenceId == 0x150) {
			_gameSys->setAnimation(_s02_nextChickenSequenceId, 179, 0);
			_gameSys->insertSequence(_s02_nextChickenSequenceId, 179, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gameSys->removeSequence(_s02_currChickenSequenceId, 179, true);
			_s02_nextChickenSequenceId = -1;
			_s02_currChickenSequenceId = -1;
			_gnapActionStatus = kASGrabChickenDone;
			_timers[5] = 500;
		} else if (_s02_nextChickenSequenceId == 0x14C) {
			_gameSys->setAnimation(_s02_nextChickenSequenceId, 179, 2);
			_gameSys->insertSequence(_s02_nextChickenSequenceId, 179, _s02_currChickenSequenceId, 179, kSeqSyncWait, 0, 0, 0);
			_s02_currChickenSequenceId = _s02_nextChickenSequenceId;
			_s02_nextChickenSequenceId = -1;
			_gnapActionStatus = -1;
		} else if (_s02_nextChickenSequenceId != -1) {
			_gameSys->setAnimation(_s02_nextChickenSequenceId, 179, 2);
			_gameSys->insertSequence(_s02_nextChickenSequenceId, 179, _s02_currChickenSequenceId, 179, kSeqSyncWait, 0, 0, 0);
			_s02_currChickenSequenceId = _s02_nextChickenSequenceId;
			_s02_nextChickenSequenceId = -1;
		}
	}

}

} // End of namespace Gnap
