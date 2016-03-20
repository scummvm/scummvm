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
	kHSPlatypus			= 0,
	kHSDevice			= 1,
	kHSExitUfoParty		= 2,
	kHSExitBBQ			= 3,
	kHSExitKissinBooth	= 4,
	kHSTwoHeadedGuy		= 5,
	kHSKey				= 6,
	kHSUfo				= 7,
	kHSWalkArea1		= 8,
	kHSWalkArea2		= 9
};

enum {
	kHSUfoExitLeft	= 1,
	kHSUfoExitRight	= 2,
	kHSUfoKey		= 3,
	kHSUfoBucket	= 4,
	kHSUfoDevice	= 5
};

int GnapEngine::scene43_init() {
	_gameSys->setAnimation(0, 0, 0);
	_gameSys->setAnimation(0, 0, 1);
	_gameSys->setAnimation(0, 0, 2);
	return 0x13F;
}

void GnapEngine::scene43_updateHotspots() {
	if (isFlag(12)) {
		setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_DISABLED);
		setHotspot(kHSUfoExitLeft, 0, 0, 10, 599, SF_EXIT_L_CURSOR);
		setHotspot(kHSUfoExitRight, 790, 0, 799, 599, SF_EXIT_R_CURSOR);
		setHotspot(kHSUfoKey, 140, 170, 185, 260, SF_GRAB_CURSOR);
		setHotspot(kHSUfoBucket, 475, 290, 545, 365, SF_DISABLED);
		setDeviceHotspot(kHSUfoDevice, -1, 534, -1, 599);
		if (isFlag(19))
			_hotspots[kHSUfoBucket]._flags = SF_GRAB_CURSOR;
		// NOTE Bug in the original. Key hotspot wasn't disabled.
		if (isFlag(14))
			_hotspots[kHSUfoKey]._flags = SF_DISABLED;
		_hotspotsCount = 6;
	} else {
		setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		setHotspot(kHSExitUfoParty, 150, 580, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
		setHotspot(kHSExitBBQ, 0, 100, 10, 599, SF_EXIT_L_CURSOR, 0, 8);
		setHotspot(kHSExitKissinBooth, 790, 100, 799, 599, SF_EXIT_R_CURSOR, 10, 8);
		setHotspot(kHSTwoHeadedGuy, 470, 240, 700, 470, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		setHotspot(kHSKey, 140, 170, 185, 260, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		setHotspot(kHSUfo, 110, 0, 690, 350, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		setHotspot(kHSWalkArea1, 0, 0, 800, 445);
		setHotspot(kHSWalkArea2, 465, 0, 800, 493);
		setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
        if (isFlag(14))
			_hotspots[kHSKey]._flags = SF_DISABLED;
		_hotspotsCount = 10;
	}
}

void GnapEngine::scene43_run() {

	queueInsertDeviceIcon();

	if (!isFlag(14))
		_gameSys->insertSequence(0x1086F, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	_currTwoHeadedGuySequenceId = 0x13C;
	_nextTwoHeadedGuySequenceId = -1;
	
	_gameSys->setAnimation(0x13C, 1, 2);
	_gameSys->insertSequence(_currTwoHeadedGuySequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	if (isFlag(12)) {
		_toyUfoId = 0;
		_toyUfoActionStatus = -1;
		_toyUfoSequenceId = toyUfoGetSequenceId();
		_toyUfoNextSequenceId = _toyUfoSequenceId;
		if (_prevSceneNum == 42)
			_toyUfoX = 30;
		else
			_toyUfoX = 770;
		_gameSys->setAnimation(_toyUfoSequenceId | 0x10000, _toyUfoId, 3);
		_gameSys->insertSequence(_toyUfoSequenceId | 0x10000, _toyUfoId, 0, 0, kSeqNone, 0, _toyUfoX - 274, _toyUfoY - 128);
		endSceneInit();
	} else {
		switch (_prevSceneNum) {
		case 42:
			initGnapPos(-1, 8, 7);
			initBeaverPos(-1, 9, 5);
			endSceneInit();
			gnapWalkTo(2, 8, -1, 0x107B9, 1);
			platypusWalkTo(1, 8, -1, 0x107C2, 1);
			break;
		case 44:
			initGnapPos(11, 8, 7);
			initBeaverPos(11, 9, 5);
			endSceneInit();
			gnapWalkTo(8, 8, -1, 0x107BA, 1);
			platypusWalkTo(9, 8, -1, 0x107D2, 1);
			break;
		case 54:
			initGnapPos(4, 7, 3);
			initBeaverPos(11, 8, 5);
			endSceneInit();
			platypusWalkTo(9, 8, -1, 0x107D2, 1);
			break;
		default:
			initGnapPos(5, 11, 7);
			initBeaverPos(6, 11, 5);
			endSceneInit();
			gnapWalkTo(5, 8, -1, 0x107BA, 1);
			platypusWalkTo(6, 8, -1, 0x107C2, 1);
			break;
		}
	}
	
	while (!_sceneDone) {
	
		if (!isSoundPlaying(0x1094B))
			playSound(0x1094B, 1);
	
		updateMouseCursor();
		updateCursorByHotspot();
	
		testWalk(0, 0, -1, -1, -1, -1);
	
		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);
	
		if (isFlag(12)) {

			switch (_sceneClickedHotspot) {

			case kHSUfoDevice:
				runMenu();
				scene43_updateHotspots();
				_timers[4] = getRandom(100) + 100;
				break;
			
			case kHSUfoExitLeft:
				if (_toyUfoActionStatus < 0) {
					_isLeavingScene = 1;
					_toyUfoActionStatus = 4;
					_newSceneNum = 42;
					toyUfoFlyTo(-35, -1, -35, 799, 0, 300, 3);
				}
				break;
			
			case kHSUfoExitRight:
				if (_toyUfoActionStatus < 0) {
					_isLeavingScene = 1;
					_toyUfoActionStatus = 4;
					_newSceneNum = 44;
					toyUfoFlyTo(835, -1, 0, 835, 0, 300, 3);
				}
				break;
			
			case kHSUfoKey:
				if (isFlag(17)) {
					_toyUfoActionStatus = 6;
					toyUfoFlyTo(163, 145, 0, 799, 0, 300, 3);
				} else {
					_toyUfoActionStatus = 5;
					toyUfoFlyTo(-1, -1, 0, 799, 0, 300, 3);
				}
				break;
			
			case kHSUfoBucket:
				_toyUfoActionStatus = 7;
				toyUfoFlyTo(497, 143, 0, 799, 0, 300, 3);
				_timers[9] = 600;
				break;
			}

		} else {

			switch (_sceneClickedHotspot) {

			case kHSDevice:
				runMenu();
				scene43_updateHotspots();
				_timers[4] = getRandom(100) + 100;
				break;

			case kHSPlatypus:
				if (_gnapActionStatus < 0) {
					if (_grabCursorSpriteIndex >= 0) {
						playGnapImpossible(_platX, _platY);
					} else {
						switch (_verbCursor) {
						case LOOK_CURSOR:
							playGnapMoan1(_platX, _platY);
							break;
						case GRAB_CURSOR:
							gnapKissPlatypus(0);
							break;
						case TALK_CURSOR:
							playGnapBrainPulsating(_platX, _platY);
							playBeaverSequence(getBeaverSequenceId(0, 0, 0) | 0x10000);
							break;
						case PLAT_CURSOR:
							playGnapImpossible(_platX, _platY);
							break;
						}
					}
				}
				break;

			case kHSExitUfoParty:
				_isLeavingScene = 1;
				gnapWalkTo(_hotspotsWalkPos[kHSExitUfoParty].x, _hotspotsWalkPos[kHSExitUfoParty].y, 0, 0x107AE, 1);
				_gnapActionStatus = 0;
				platypusWalkTo(_hotspotsWalkPos[kHSExitUfoParty].x, _hotspotsWalkPos[kHSExitUfoParty].y, -1, 0x107C7, 1);
				_newSceneNum = 40;
				break;

			case kHSExitBBQ:
				_isLeavingScene = 1;
				gnapWalkTo(_hotspotsWalkPos[kHSExitBBQ].x, _gnapY, 0, 0x107AF, 1);
				_gnapActionStatus = 0;
				platypusWalkTo(_hotspotsWalkPos[kHSExitBBQ].x, _hotspotsWalkPos[kHSExitBBQ].y, -1, 0x107CF, 1);
				_newSceneNum = 42;
				break;

			case kHSExitKissinBooth:
				_isLeavingScene = 1;
				gnapWalkTo(_hotspotsWalkPos[kHSExitKissinBooth].x, _gnapY, 0, 0x107AB, 1);
				_gnapActionStatus = 0;
				platypusWalkTo(_hotspotsWalkPos[kHSExitKissinBooth].x, _hotspotsWalkPos[kHSExitKissinBooth].y, -1, 0x107CD, 1);
				_newSceneNum = 44;
				break;

			case kHSTwoHeadedGuy:
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(6, 8, 7, 0);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(7, 0);
						break;
					case TALK_CURSOR:
							_gnapIdleFacing = 7;
							gnapWalkTo(5, 8, 0, getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
							_gnapActionStatus = 2;
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
				break;

			case kHSKey:
			case kHSUfo:
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(3, 7, 2, 0);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(0, 0);
						break;
					case GRAB_CURSOR:
						_gnapIdleFacing = 7;
						gnapWalkTo(3, 7, 0, 67515, 1);
						_gnapActionStatus = 1;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
				break;

			case kHSWalkArea1:
			case kHSWalkArea2:
				if (_gnapActionStatus < 0)
					gnapWalkTo(-1, -1, -1, -1, 1);
				break;

			}

		}
	
		if (_mouseClickState._left && _gnapActionStatus < 0) {
			_mouseClickState._left = false;
			if (isFlag(12) && (_toyUfoActionStatus == 5 || _toyUfoActionStatus == -1)) {
				_toyUfoActionStatus = 5;
				toyUfoFlyTo(-1, -1, 0, 799, 0, 300, 3);
			} else {
				gnapWalkTo(-1, -1, -1, -1, 1);
			}
		}
	
		scene43_updateAnimations();
	
		toyUfoCheckTimer();
	
		if (!_isLeavingScene) {
			if (_beaverActionStatus < 0 && !isFlag(12))
				updateBeaverIdleSequence();
			if (_gnapActionStatus < 0 && !isFlag(12))
				updateGnapIdleSequence();
			if (!_timers[4] && (!isFlag(12) || !isFlag(19))) {
				_timers[4] = getRandom(100) + 100;
				if (_gnapActionStatus < 0 && _beaverActionStatus < 0 && _nextTwoHeadedGuySequenceId == -1) {
					_gnapRandomValue = getRandom(5);
					switch (_gnapRandomValue) {
					case 0:
						_nextTwoHeadedGuySequenceId = 0x13C;
						break;
					case 1:
						_nextTwoHeadedGuySequenceId = 0x134;
						break;
					case 2:
						_nextTwoHeadedGuySequenceId = 0x135;
						break;
					case 3:
						_nextTwoHeadedGuySequenceId = 0x136;
						break;
					case 4:
						_nextTwoHeadedGuySequenceId = 0x13A;
						break;
					}
					if (_nextTwoHeadedGuySequenceId == _currTwoHeadedGuySequenceId)
						_nextTwoHeadedGuySequenceId = -1;
				}
			}
		}
	
		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene43_updateHotspots();
			_timers[4] = getRandom(100) + 100;
		}
		
		gameUpdateTick();
	
	}
	
	if (_newSceneNum == 54)
		clearFlag(12);

}

void GnapEngine::scene43_updateAnimations() {
	if (_gameSys->getAnimationStatus(0) == 2) {
		switch (_gnapActionStatus) {
		case 0:
			_gameSys->setAnimation(0, 0, 0);
			_sceneDone = true;
			break;

		case 1:
			if (_gameSys->getAnimationStatus(2) == 2) {
				_timers[2] = getRandom(30) + 20;
				_timers[3] = getRandom(50) + 200;
				_gameSys->insertSequence(0x13D, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
				_gnapSequenceId = 0x13D;
				_gnapSequenceDatNum = 0;
				_gameSys->setAnimation(0x13D, _gnapId, 0);
				_nextTwoHeadedGuySequenceId = 0x13B;
				_gameSys->insertSequence(0x13B, 1, _currTwoHeadedGuySequenceId, 1, kSeqSyncWait, 0, 0, 0);
				_gameSys->setAnimation(_nextTwoHeadedGuySequenceId, 1, 2);
				_currTwoHeadedGuySequenceId = _nextTwoHeadedGuySequenceId;
				_nextTwoHeadedGuySequenceId = -1;
				_timers[4] = getRandom(100) + 100;
				_gnapActionStatus = -1;
			}
			break;

		default:
			_gameSys->setAnimation(0, 0, 0);
			_gnapActionStatus = -1;
			break;
		}
	}
	
	if (_gameSys->getAnimationStatus(2) == 2) {
		if (_currTwoHeadedGuySequenceId == 0x13A) {
			if (isFlag(19)) {
				_nextTwoHeadedGuySequenceId = 0x13E;
				stopSound(0x108F6);
			} else if (getRandom(2) != 0) {
				_nextTwoHeadedGuySequenceId = 0x137;
			} else {
				_nextTwoHeadedGuySequenceId = 0x138;
			}
		} else if (_currTwoHeadedGuySequenceId == 0x13E) {
			_sceneDone = true;
			_newSceneNum = 54;
		}
		if (_nextTwoHeadedGuySequenceId != -1) {
			_gameSys->insertSequence(_nextTwoHeadedGuySequenceId, 1, _currTwoHeadedGuySequenceId, 1, kSeqSyncWait, 0, 0, 0);
			_gameSys->setAnimation(_nextTwoHeadedGuySequenceId, 1, 2);
			_currTwoHeadedGuySequenceId = _nextTwoHeadedGuySequenceId;
			_nextTwoHeadedGuySequenceId = -1;
			_timers[4] = getRandom(100) + 100;
		}
	}
	
	if (_gameSys->getAnimationStatus(3) == 2) {
		switch (_toyUfoActionStatus) {
		case 4:
			_sceneDone = true;
			_toyUfoActionStatus = -1;
			break;
		case 6:
			_gameSys->insertSequence(0x10871, _toyUfoId, _toyUfoSequenceId | 0x10000, _toyUfoId, kSeqSyncWait, 0, 0, 0);
			_gameSys->removeSequence(0x1086F, 1, true);
			setFlag(14);
			scene43_updateHotspots();
			toyUfoSetStatus(18);
			_toyUfoSequenceId = 0x871;
			_gameSys->setAnimation(0x10871, _toyUfoId, 3);
			_toyUfoActionStatus = -1;
			_toyUfoX = 96;
			_toyUfoY = 131;
			break;
		case 7:
			_gameSys->insertSequence(0x10874, _toyUfoId, _toyUfoSequenceId | 0x10000, _toyUfoId, kSeqSyncWait, 0, 0, 0);
			_toyUfoSequenceId = 0x874;
			_gameSys->setAnimation(0x10874, _toyUfoId, 3);
			_toyUfoActionStatus = 8;
			setFlag(17);
			_gnapActionStatus = 3;
			break;
		case 8:
			_nextTwoHeadedGuySequenceId = 0x13A;
			_toyUfoX = 514;
			_toyUfoY = 125;
			toyUfoFlyTo(835, 125, 0, 835, 0, 300, 3);
			_toyUfoActionStatus = 9;
			break;
		case 9:
			// Nothing
			break;
		default:
			_toyUfoNextSequenceId = toyUfoGetSequenceId();
			_gameSys->insertSequence(_toyUfoNextSequenceId | 0x10000, _toyUfoId + 1,
				_toyUfoSequenceId | 0x10000, _toyUfoId,
				kSeqSyncWait, 0, _toyUfoX - 274, _toyUfoY - 128);
			_toyUfoSequenceId = _toyUfoNextSequenceId;
			++_toyUfoId;
			_gameSys->setAnimation(_toyUfoNextSequenceId | 0x10000, _toyUfoId, 3);
			_toyUfoActionStatus = -1;
			break;
		}
	}

}

} // End of namespace Gnap
