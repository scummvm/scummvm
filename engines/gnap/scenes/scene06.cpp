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
	kHSGas				= 1,
	kHSLadder			= 2,
	kHSHorse			= 3,
	kHSExitOutsideBarn	= 4,
	kHSDevice			= 5,
	kHSWalkArea1		= 6,
	kHSWalkArea2		= 7,
	kHSWalkArea3		= 8,
	kHSWalkArea4		= 9,
	kHSWalkArea5		= 10
};

enum {
	kASTryToGetGas			= 0,
	kASTryToClimbLadder		= 1,
	kASTryToClimbLadderDone	= 2,
	kASTalkToHorse			= 3,
	kASUseTwigOnHorse		= 4,
	kASLeaveScene			= 5
};

int GnapEngine::scene06_init() {
	_gameSys->setAnimation(0, 0, 0);
	_gameSys->setAnimation(0, 0, 1);
	_gameSys->setAnimation(0, 0, 2);
	if (isFlag(11)) {
		playSound(0x11B, 0);
		clearFlag(11);
	}
	return 0x101;
}

void GnapEngine::scene06_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSGas, 300, 120, 440, 232, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 4, 7);
	setHotspot(kHSLadder, 497, 222, 614, 492, SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 8);
	setHotspot(kHSHorse, 90, 226, 259, 376, SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 4, 7);
	setHotspot(kHSExitOutsideBarn, 226, 580, 688, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 10);
	setHotspot(kHSWalkArea1, 0, 0, 200, 515);
	setHotspot(kHSWalkArea2, 200, 0, 285, 499);
	setHotspot(kHSWalkArea3, 688, 0, 800, 499);
	setHotspot(kHSWalkArea4, 475, 469, 800, 505);
	setHotspot(kHSWalkArea5, 0, 0, 800, 504);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
    if (isFlag(29)) 
    	_hotspots[kHSLadder]._flags = SF_DISABLED;
	if (_cursorValue == 4) {
		_hotspots[kHSLadder]._flags = SF_DISABLED;
		_hotspots[kHSGas]._flags = SF_DISABLED;
	}
	_hotspotsCount = 11;
}

void GnapEngine::scene06_run() {
	bool triedDeviceOnGas = false;
	
	startSoundTimerC(7);

	_s06_horseTurnedBack = false;
	_gameSys->insertSequence(0xF1, 120, 0, 0, kSeqNone, 0, 0, 0);

	_s06_currHorseSequenceId = 0xF1;
	_s06_nextHorseSequenceId = -1;

	_gameSys->setAnimation(0xF1, 120, 2);

	_timers[4] = getRandom(40) + 25;

	if (isFlag(4))
		_gameSys->insertSequence(0xF7, 20, 0, 0, kSeqNone, 0, 0, 0);
	else
		_gameSys->insertSequence(0xF8, 20, 0, 0, kSeqNone, 0, 0, 0);
	
	if (!isFlag(29) && _cursorValue != 4)
		_gameSys->insertSequence(0xFE, 20, 0, 0, kSeqNone, 0, 0, 0);
	
	queueInsertDeviceIcon();
	
	initGnapPos(5, 12, 1);
	initBeaverPos(6, 12, 0);
	endSceneInit();
	
	platypusWalkTo(6, 8, -1, 0x107C2, 1);
	gnapWalkTo(5, 8, -1, 0x107B9, 1);
	
	while (!_sceneDone) {
	
		updateMouseCursor();
		updateCursorByHotspot();
		
		testWalk(0, 5, -1, -1, -1, -1);
	
		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);
	
		switch (_sceneClickedHotspot) {

		case kHSDevice:
			if (_gnapActionStatus < 0) {
				runMenu();
				scene06_updateHotspots();
			}
			break;
		
		case kHSPlatypus:
			if (_gnapActionStatus < 0) {
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

		case kHSGas:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[_sceneClickedHotspot].x, _hotspotsWalkPos[_sceneClickedHotspot].y, 5, 0);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(5, 0);
						break;
					case GRAB_CURSOR:
						if (isFlag(4)) {
							playGnapImpossible(0, 0);
						} else if (triedDeviceOnGas) {
							_hotspots[kHSWalkArea5]._flags |= SF_WALKABLE;
							gnapWalkTo(_hotspotsWalkPos[1].x, _hotspotsWalkPos[1].y, 0, 0x107BC, 1);
							_hotspots[kHSWalkArea5]._flags &= ~SF_WALKABLE;
							_gnapActionStatus = kASTryToGetGas;
						} else {
							triedDeviceOnGas = true;
							playGnapPullOutDeviceNonWorking(_hotspotsWalkPos[1].x, _hotspotsWalkPos[1].y);
						}
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						if (isFlag(4))
							playGnapImpossible(0, 0);
						else
							playGnapScratchingHead(5, 0);
						break;
					}
				}
			}
			break;

		case kHSLadder:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[_sceneClickedHotspot].x, _hotspotsWalkPos[_sceneClickedHotspot].y, 8, 4);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(8, 4);
						break;
					case GRAB_CURSOR:
						if (isFlag(29))
							playGnapImpossible(0, 0);
						else {
							gnapWalkTo(_hotspotsWalkPos[2].x, _hotspotsWalkPos[2].y, 0, 0x107BB, 1);
							_gnapActionStatus = kASTryToClimbLadder;
							setFlag(29);
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

		case kHSHorse:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex == kItemTwig && _s06_horseTurnedBack) {
					_hotspots[kHSWalkArea5]._flags |= SF_WALKABLE;
					gnapWalkTo(_hotspotsWalkPos[3].x, _hotspotsWalkPos[3].y, 0, 0x107BC, 1);
					_hotspots[kHSWalkArea5]._flags &= ~SF_WALKABLE;
					_gnapIdleFacing = 5;
					platypusWalkTo(6, 8, 1, 0x107C2, 1);
					_beaverFacing = 0;
					_gnapActionStatus = kASUseTwigOnHorse;
					setGrabCursorSprite(-1);
				} else if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[_sceneClickedHotspot].x, _hotspotsWalkPos[_sceneClickedHotspot].y, 3, 2);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(3, 2);
						break;
					case TALK_CURSOR:
						if (_s06_horseTurnedBack) {
							gnapWalkTo(_hotspotsWalkPos[3].x, _hotspotsWalkPos[3].y, 0, getGnapSequenceId(gskBrainPulsating, 3, 2) | 0x10000, 1);
						} else {
							_gnapIdleFacing = 3;
							_hotspots[kHSWalkArea5]._flags |= SF_WALKABLE;
							gnapWalkTo(_hotspotsWalkPos[3].x, _hotspotsWalkPos[3].y, 0, getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
							_hotspots[kHSWalkArea5]._flags &= ~SF_WALKABLE;
							_gnapActionStatus = kASTalkToHorse;
						}
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSExitOutsideBarn:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = 1;
				gnapWalkTo(_hotspotsWalkPos[4].x, _hotspotsWalkPos[4].y, 0, 0x107AE, 1);
				_gnapActionStatus = kASLeaveScene;
				if (_cursorValue == 1)
					_newSceneNum = 5;
				else
					_newSceneNum = 35;
			}
			break;

		case kHSWalkArea1:
		case kHSWalkArea2:
		case kHSWalkArea3:
		case kHSWalkArea4:
		case kHSWalkArea5:
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
	
	
		scene06_updateAnimations();
	
		if (!_isLeavingScene) {
			if (_beaverActionStatus < 0)
				updateBeaverIdleSequence();
			if (_gnapActionStatus < 0)
				updateGnapIdleSequence();
			if (!_timers[4]) {
				_timers[4] = getRandom(40) + 25;
				if (_gnapActionStatus < 0 && _beaverActionStatus < 0 && _s06_nextHorseSequenceId == -1) {
					if (_s06_horseTurnedBack) {
						_s06_nextHorseSequenceId = 0xF5;
					} else {
						switch (getRandom(5)) {
						case 0:
						case 1:
						case 2:
							_s06_nextHorseSequenceId = 0xF1;
							break;
						case 3:
							_s06_nextHorseSequenceId = 0xF3;
							break;
						case 4:
							_s06_nextHorseSequenceId = 0xF4;
							break;
						}
					}
				}
			}
			playSoundC();
		}
	
		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene06_updateHotspots();
		}
		
		gameUpdateTick();
	
	}

}

void GnapEngine::scene06_updateAnimations() {

	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		switch (_gnapActionStatus) {
		case kASLeaveScene:
			_sceneDone = true;
			_gnapActionStatus = -1;
			break;
		case kASTryToGetGas:
			_gameSys->insertSequence(0xFC, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gnapSequenceId = 0xFC;
			_gnapSequenceDatNum = 0;
			_gnapActionStatus = -1;
			break;
		case kASTryToClimbLadder:
			_gameSys->insertSequence(0xFF, 20, 0xFE, 20, kSeqSyncWait, 0, 0, 0);
			_gameSys->setAnimation(0xFD, _gnapId, 0);
			_gameSys->insertSequence(0xFD, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gnapSequenceId = 0xFD;
			_gnapSequenceDatNum = 0;
			_gnapActionStatus = kASTryToClimbLadderDone;
			break;
		case kASTryToClimbLadderDone:
			_gnapX = 6;
			_gnapY = 7;
			_gnapActionStatus = -1;
			break;
		case kASTalkToHorse:
			_s06_nextHorseSequenceId = 0xF6;
			break;
		case kASUseTwigOnHorse:
			_s06_nextPlatSequenceId = 0xFB;
			break;
		default:
			_gnapActionStatus = -1;
			break;
		}
	}
	
	if (_gameSys->getAnimationStatus(1) == 2) {
		_gameSys->setAnimation(0, 0, 1);
		if (_beaverSequenceId == 0xFA) {
			_gameSys->setAnimation(0, 0, 1);
			invAdd(kItemGas);
			setFlag(29);
			_hotspots[kHSLadder]._flags = SF_DISABLED;
			setGrabCursorSprite(kItemGas);
			_beaverActionStatus = -1;
			_platX = 6;
			_platY = 8;
			_gameSys->insertSequence(0x107C1, _beaverId, 0, 0, kSeqNone, 0, 450 - _platGridX, 384 - _platGridY);
			_beaverSequenceId = 0x7C1;
			_beaverSequenceDatNum = 1;
			setFlag(4);
			_gnapActionStatus = -1;
			showCursor();
		}
		if (_s06_nextPlatSequenceId == 0xFB) {
			_gameSys->setAnimation(0, 0, 1);
			_s06_nextHorseSequenceId = 0xF2;
			_beaverActionStatus = 6;
		}
	}
	
	if (_gameSys->getAnimationStatus(2) == 2 && _s06_nextHorseSequenceId != -1) {
		switch (_s06_nextHorseSequenceId) {
		case 0xF2:
			setGrabCursorSprite(-1);
			hideCursor();
			_gameSys->setAnimation(0xFA, 256, 1);
			_gameSys->insertSequence(0xF2, 120, _s06_currHorseSequenceId, 120, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0x100, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0xF7, 20, 0xF8, 20, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0xFB, _beaverId, _beaverSequenceId | (_beaverSequenceDatNum << 16), _beaverId, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0xFA, 256, 0xFB, _beaverId, kSeqSyncWait, 0, 0, 0);
			_beaverSequenceId = 0xFA;
			_beaverSequenceDatNum = 0;
			_gameSys->insertSequence(0x107B7, _gnapId, 0x100, _gnapId,
				kSeqSyncWait, 0, 75 * _gnapX - _gnapGridX, 48 * _gnapY - _gnapGridY);
			_gnapSequenceId = 0x7B7;
			_gnapSequenceDatNum = 1;
			_s06_currHorseSequenceId = _s06_nextHorseSequenceId;
			_s06_nextHorseSequenceId = -1;
			_s06_nextPlatSequenceId = -1;
			invRemove(kItemTwig);
			break;
		case 0xF6:
			_gameSys->setAnimation(_s06_nextHorseSequenceId, 120, 2);
			_gameSys->insertSequence(0xF6, 120, _s06_currHorseSequenceId, 120, kSeqSyncWait, 0, 0, 0);
			_s06_horseTurnedBack = true;
			_s06_currHorseSequenceId = _s06_nextHorseSequenceId;
			_s06_nextHorseSequenceId = -1;
			_gnapActionStatus = -1;
			break;
		default:
			_gameSys->setAnimation(_s06_nextHorseSequenceId, 120, 2);
			_gameSys->insertSequence(_s06_nextHorseSequenceId, 120, _s06_currHorseSequenceId, 120, kSeqSyncWait, 0, 0, 0);
			_s06_currHorseSequenceId = _s06_nextHorseSequenceId;
			_s06_nextHorseSequenceId = -1;
			break;
		}
	}

}

} // End of namespace Gnap
