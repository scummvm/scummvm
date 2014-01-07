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
	kHSHaystack		= 1,
	kHSPadlock		= 2,
	kHSLadder		= 3,
	kHSExitHouse	= 4,
	kHSChicken		= 5,
	kHSDevice		= 6,
	kHSWalkArea1	= 7,
	kHSWalkArea2	= 8,
	kHSWalkArea3	= 9
};

enum {
	kASPlatSearchHaystack		= 0,
	kASTryPickPadlock			= 1,
	kASPickPadlock				= 2,
	kASTalkChicken				= 3,
	kASGrabChicken				= 4,
	kASGrabLadder				= 5,
	kASEnterBarn				= 6,
	kASUseTwigWithChicken		= 11,
	kASLeaveScene				= 12
};

int GnapEngine::scene05_init() {
	_gameSys->setAnimation(0, 0, 0);
	_gameSys->setAnimation(0, 0, 1);
	_gameSys->setAnimation(0, 0, 3);
	return isFlag(7) ? 0x151 : 0x150;
}

void GnapEngine::scene05_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_DISABLED | SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSHaystack, 236, 366, 372, 442, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 5, 7);
	setHotspot(kHSPadlock, 386, 230, 626, 481, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 5, 7);
	setHotspot(kHSLadder, 108, 222, 207, 444, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 4, 7);
	setHotspot(kHSExitHouse, 0, 395, 20, 600, SF_EXIT_L_CURSOR | SF_WALKABLE, 0, 8);
	setHotspot(kHSChicken, 612, 462, 722, 564, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 7, 8);
	setHotspot(kHSWalkArea1, 104, 0, 421, 480);
	setHotspot(kHSWalkArea2, 422, 0, 800, 487);
	setHotspot(kHSWalkArea3, 0, 0, 104, 499);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	if (isFlag(0))
		_hotspots[kHSPlatypus].flags = SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	if (isFlag(7))
		_hotspots[kHSPadlock].flags = SF_EXIT_U_CURSOR;
	_hotspotsCount = 10;
}

void GnapEngine::scene05_run() {

	playSound(0x1091C, 1);
	startSoundTimerC(7);
	
	_s05_currChickenSequenceId = 0x142;
	_gameSys->setAnimation(0x142, 100, 3);
	_gameSys->insertSequence(0x142, 100, 0, 0, kSeqNone, 0, 0, 0);
	
	_s05_nextChickenSequenceId = -1;

	_timers[5] = getRandom(10) + 30;
	_timers[6] = getRandom(150) + 300;

	if (isFlag(7))
		_gameSys->insertSequence(0x14A, 141, 0, 0, kSeqNone, 0, 0, 0);
	
	queueInsertDeviceIcon();
	
	if (_prevSceneNum != 6 && _prevSceneNum != 36) {
		initGnapPos(-1, 8, 1);
		if (isFlag(0))
			initBeaverPos(-1, 9, 0);
		endSceneInit();
		if (isFlag(0))
			platypusWalkTo(2, 8, -1, 0x107C2, 1);
		gnapWalkTo(2, 9, -1, 0x107B9, 1);
	} else {
		initGnapPos(6, 8, 1);
		if (isFlag(0))
			initBeaverPos(7, 9, 0);
		endSceneInit();
	}
	
	while (!_sceneDone) {
	
		updateMouseCursor();
		updateCursorByHotspot();
	
		testWalk(0, 12, -1, -1, -1, -1);
	
		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);
	
		switch (_sceneClickedHotspot) {

		case kHSDevice:
			if (_gnapActionStatus < 0) {
				runMenu();
				scene05_updateHotspots();
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

		case kHSHaystack:
			if (_gnapActionStatus < 0 && _beaverActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[1].x - 2, _hotspotsWalkPos[1].y, 4, 5);
				} else if (isFlag(2)) {
					playGnapImpossible(0, 0);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(_hotspotsWalkPos[1].x, _hotspotsWalkPos[1].y - 1);
						break;
					case GRAB_CURSOR:
					case TALK_CURSOR:
						playGnapImpossible(0, 0);
						break;
					case PLAT_CURSOR:
						if (isFlag(0)) {
							gnapUseDeviceOnBeaver();
							if (platypusWalkTo(_hotspotsWalkPos[1].x, _hotspotsWalkPos[1].y, 1, 0x107C2, 1)) {
								_beaverActionStatus = kASPlatSearchHaystack;
								_beaverFacing = 4;
							}
							if (_gnapX == 4 && (_gnapY == 8 || _gnapY == 7))
								gnapWalkStep();
							playGnapIdle(_platX, _platY);
						}
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
					playGnapShowCurrItem(_hotspotsWalkPos[_sceneClickedHotspot].x, _hotspotsWalkPos[_sceneClickedHotspot].y + 1, 9, 7);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapMoan2(9, 7);
						break;
					case GRAB_CURSOR:
						_gnapIdleFacing = 1;
						gnapWalkTo(_hotspotsWalkPos[5].x, _hotspotsWalkPos[5].y, 0, getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
						_gnapActionStatus = kASGrabChicken;
						break;
					case TALK_CURSOR:
						_gnapIdleFacing = 1;
						gnapWalkTo(_hotspotsWalkPos[5].x, _hotspotsWalkPos[5].y, 0, getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_gnapActionStatus = kASTalkChicken;
						break;
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSLadder:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[_sceneClickedHotspot].x, _hotspotsWalkPos[_sceneClickedHotspot].y, 2, 5);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapMoan2(2, 4);
						break;
					case GRAB_CURSOR:
						_gnapIdleFacing = 3;
						gnapWalkTo(_hotspotsWalkPos[3].x, _hotspotsWalkPos[3].y, 0, getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
						_gnapActionStatus = kASGrabLadder;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSPadlock:
			if (isFlag(7)) {
				_isLeavingScene = true;
				gnapWalkTo(_hotspotsWalkPos[2].x - 1, _hotspotsWalkPos[2].y + 1, 0, -1, 1);
				_gnapActionStatus = kASEnterBarn;
				if (_cursorValue == 1)
					_newSceneNum = 6;
				else
					_newSceneNum = 36;
			} else if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex == kItemNeedle) {
					if (gnapWalkTo(_hotspotsWalkPos[2].x, _hotspotsWalkPos[2].y, 0,
						getGnapSequenceId(gskIdle, _hotspotsWalkPos[2].x, _hotspotsWalkPos[2].y) | 0x10000, 1))
						_gnapActionStatus = kASPickPadlock;
				} else if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[2].x, _hotspotsWalkPos[2].y, 7, 4);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(7, 4);
						break;
					case GRAB_CURSOR:
						_gnapIdleFacing = 7;
						gnapWalkTo(_hotspotsWalkPos[_sceneClickedHotspot].x, _hotspotsWalkPos[_sceneClickedHotspot].y,
							0, getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
						_gnapActionStatus = kASTryPickPadlock;
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
				_isLeavingScene = true;
				gnapWalkTo(_hotspotsWalkPos[4].x, _hotspotsWalkPos[4].y, 0, 0x107AF, 1);
				_gnapActionStatus = kASLeaveScene;
				if (isFlag(0))
					platypusWalkTo(_hotspotsWalkPos[4].x, _hotspotsWalkPos[4].y + 1, -1, 0x107C7, 1);
				if (_cursorValue == 1)
					_newSceneNum = 4;
				else
					_newSceneNum = 37;
			}
			break;

		case kHSWalkArea1:
		case kHSWalkArea2:
			if (_gnapActionStatus < 0)
				gnapWalkTo(-1, -1, -1, -1, 1);
			break;
			
		case kHSWalkArea3:
			// Nothing
			break;

		default:
			if (_mouseClickState.left && _gnapActionStatus < 0) {
				gnapWalkTo(-1, -1, -1, -1, 1);
				_mouseClickState.left = false;
			}
			break;
		
		}
	
		scene05_updateAnimations();
	
		if (!isSoundPlaying(0x1091C))
			playSound(0x1091C, 1);
	
		if (!_isLeavingScene) {
			if (isFlag(0))
				updateBeaverIdleSequence();
			updateGnapIdleSequence();
			if (!_timers[5]) {
				_timers[5] = getRandom(20) + 30;
				if (_gnapActionStatus != kASTalkChicken && _s05_nextChickenSequenceId == -1) {
					if (getRandom(4) != 0)
						_s05_nextChickenSequenceId = 0x142;
					else
						_s05_nextChickenSequenceId = 0x143;
				}
			}
			if (!_timers[6]) {
				_timers[6] = getRandom(150) + 300;
				if (_gnapActionStatus < 0)
					_gameSys->insertSequence(0x149, 39, 0, 0, kSeqNone, 0, 0, 0);
			}
			playSoundC();
		}
	
		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene05_updateHotspots();
			_timers[5] = getRandom(20) + 30;
		}
		
		gameUpdateTick();
	
	}

}

void GnapEngine::scene05_updateAnimations() {

	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		switch (_gnapActionStatus) {
		case kASLeaveScene:
			_sceneDone = true;
			_gnapActionStatus = -1;
			break;
		case kASTryPickPadlock:
			_gameSys->insertSequence(0x148, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gnapSequenceId = 0x148;
			_gnapSequenceDatNum = 0;
			_gnapActionStatus = -1;
			break;
		case kASPickPadlock:
			_gameSys->setAnimation(0x147, _gnapId, 0);
			_gameSys->insertSequence(0x147, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gnapSequenceId = 0x147;
			_gnapSequenceDatNum = 0;
			setFlag(7);
			setFlag(11);
			setGrabCursorSprite(-1);
			_newSceneNum = 6;
			_timers[2] = 100;
			invRemove(kItemNeedle);
			_gnapActionStatus = kASLeaveScene;
			break;
		case kASTalkChicken:
			_s05_nextChickenSequenceId = 0x144;
			_gnapActionStatus = -1;
			break;
		case kASGrabChicken:
			_s05_nextChickenSequenceId = 0x14B;
			break;
		case kASGrabLadder:
			while (_gameSys->isSequenceActive(0x149, 39))
				gameUpdateTick();
			_gameSys->insertSequence(0x14E, _gnapId + 1, 0, 0, kSeqNone, 0, 0, 0);
			_gameSys->insertSequence(0x14D, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gnapSequenceId = 0x14D;
			_gnapSequenceDatNum = 0;
			_timers[2] = 200;
			_timers[6] = 300;
			_gnapActionStatus = -1;
			break;
		case kASEnterBarn:
			_gameSys->insertSequence(0x107B1, 1,
				makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId,
				kSeqSyncWait, 0, 75 * _gnapX - _gnapGridX, 48 * _gnapY - _gnapGridY);
			_gameSys->setAnimation(0x107B1, 1, 0);
			_gnapActionStatus = kASLeaveScene;
			break;
		case kASUseTwigWithChicken:
			playGnapShowItem(5, 0, 0);
			_s05_nextChickenSequenceId = 0x14F;
			_gnapActionStatus = -1;
			break;
		}
	}
	
	if (_gameSys->getAnimationStatus(1) == 2) {
		if (_beaverSequenceId == 0x146) {
			_platX = 4;
			_platY = 8;
			_gameSys->insertSequence(0x107C1, 160, 0x146, 256, kSeqSyncWait, 0, 300 - _platGridX, 384 - _platGridY);
			_beaverSequenceId = 0x7C1;
			_beaverSequenceDatNum = 1;
			_beaverId = 20 * _platY;
			invAdd(kItemNeedle);
			setFlag(2);
			setGrabCursorSprite(kItemNeedle);
			showCursor();
			_timers[1] = 30;
			_beaverActionStatus = -1;
		}
		if (_beaverActionStatus == kASPlatSearchHaystack) {
			_gameSys->setAnimation(0, 0, 1);
			_gameSys->insertSequence(0x145, _beaverId, _beaverSequenceId | (_beaverSequenceDatNum << 16), _beaverId, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0x146, 256, 0x145, _beaverId, kSeqSyncWait, 0, 0, 0);
			hideCursor();
			setGrabCursorSprite(-1);
			_beaverSequenceId = 0x146;
			_beaverSequenceDatNum = 0;
			_gameSys->setAnimation(0x146, 256, 1);
			_timers[1] = 300;
		}
	}
	
	if (_gameSys->getAnimationStatus(3) == 2) {
		if (_s05_nextChickenSequenceId == 0x14B) {
			_gameSys->setAnimation(_s05_nextChickenSequenceId, 100, 3);
			_gameSys->insertSequence(_s05_nextChickenSequenceId, 100, _s05_currChickenSequenceId, 100, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0x14C, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gnapSequenceDatNum = 0;
			_gnapSequenceId = 0x14C;
			_s05_currChickenSequenceId = _s05_nextChickenSequenceId;
			_s05_nextChickenSequenceId = -1;
			_gnapActionStatus = -1;
		} else if (_s05_nextChickenSequenceId != -1) {
			_gameSys->setAnimation(_s05_nextChickenSequenceId, 100, 3);
			_gameSys->insertSequence(_s05_nextChickenSequenceId, 100, _s05_currChickenSequenceId, 100, kSeqSyncWait, 0, 0, 0);
			_s05_currChickenSequenceId = _s05_nextChickenSequenceId;
			_s05_nextChickenSequenceId = -1;
		}
	}

}

} // End of namespace Gnap
