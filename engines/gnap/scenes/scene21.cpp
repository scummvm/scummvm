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
	kHSPlatypus				= 0,
	kHSBanana				= 1,
	kHSOldLady				= 2,
	kHSDevice				= 3,
	kHSExitOutsideGrubCity	= 4,
	kHSWalkArea1			= 5,
	kHSWalkArea2			= 6
};

enum {
	kASTalkOldLady				= 0,
	kASGrabBanana				= 1,
	kASGrabBananaDone			= 2,
	kASGrabOldLady				= 3,
	kASUseHatWithOldLady		= 4,
	kASUseHatWithOldLadyDone	= 5,
	kASLeaveScene				= 6
};

int GnapEngine::scene21_init() {
	_gameSys->setAnimation(0, 0, 3);
	return isFlag(kGFTwigTaken) ? 0x94 : 0x93;
}

void GnapEngine::scene21_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSBanana, 94, 394, 146, 430, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 2, 6);
	setHotspot(kHSOldLady, 402, 220, 528, 430, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 4, 7);
	setHotspot(kHSExitOutsideGrubCity, 522, 498, 800, 600, SF_EXIT_SE_CURSOR | SF_WALKABLE, 5, 10);
	setHotspot(kHSWalkArea1, 0, 0, 800, 440);
	setHotspot(kHSWalkArea2, 698, 0, 800, 600);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	if (isFlag(kGFUnk04) || !isFlag(kGFTwigTaken))
		_hotspots[kHSBanana]._flags = SF_WALKABLE | SF_DISABLED;
	if (isFlag(kGFTwigTaken))
		_hotspots[kHSOldLady]._flags = SF_DISABLED;
	_hotspotsCount = 7;
}

void GnapEngine::scene21_run() {
	playSound(0x10940, true);
	startSoundTimerA(6);
	
	_timers[5] = getRandom(100) + 100;
	
	queueInsertDeviceIcon();
	
	if (isFlag(kGFTwigTaken)) {
		if (isFlag(kGFKeysTaken)) {
			initGnapPos(5, 8, kDirBottomRight);
			initPlatypusPos(6, 8, kDirNone);
			_gameSys->insertSequence(0x8E, 2, 0, 0, kSeqNone, 0, 0, 0);
			if (!isFlag(kGFUnk04))
				_gameSys->insertSequence(0x8D, 59, 0, 0, kSeqNone, 0, 0, 0);
			endSceneInit();
			clearFlag(kGFKeysTaken);
		} else {
			initGnapPos(5, 11, kDirBottomRight);
			initPlatypusPos(6, 11, kDirNone);
			if (!isFlag(kGFUnk04))
				_gameSys->insertSequence(0x8D, 59, 0, 0, kSeqNone, 0, 0, 0);
			endSceneInit();
			gnapWalkTo(5, 8, -1, 0x107B9, 1);
			platypusWalkTo(6, 8, -1, 0x107C2, 1);
		}
	} else {
		initGnapPos(5, 11, kDirBottomRight);
		initPlatypusPos(6, 11, kDirNone);
		_s21_currOldLadySequenceId = 0x89;
		_gameSys->setAnimation(0x89, 79, 3);
		_gameSys->insertSequence(_s21_currOldLadySequenceId, 79, 0, 0, kSeqNone, 0, 0, 0);
		_s21_nextOldLadySequenceId = -1;
		_timers[4] = getRandom(30) + 50;
		endSceneInit();
		gnapWalkTo(5, 8, -1, 0x107B9, 1);
		platypusWalkTo(6, 8, -1, 0x107C2, 1);
	}
	
	while (!_sceneDone) {
	
		updateMouseCursor();
		updateCursorByHotspot();

		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);
	
		switch (_sceneClickedHotspot) {

		case kHSDevice:
			if (_gnapActionStatus < 0) {
				runMenu();
				scene21_updateHotspots();
			}
			break;

		case kHSPlatypus:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex == kItemJoint) {
					gnapUseJointOnPlatypus();
				} else if (_grabCursorSpriteIndex >= 0) {
					playGnapImpossible(0, 0);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(_platX, _platY);
						break;
					case GRAB_CURSOR:
						gnapKissPlatypus(0);
						break;
					case TALK_CURSOR:
						playGnapBrainPulsating(_platX, _platY);
						playPlatypusSequence(getPlatypusSequenceId());
						break;
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSBanana:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowItem(_grabCursorSpriteIndex, 2, 5);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(2, 5);
						break;
					case GRAB_CURSOR:
						gnapWalkTo(_gnapX, _gnapY, 0, getGnapSequenceId(gskIdle, _hotspotsWalkPos[kHSBanana].x, _hotspotsWalkPos[kHSBanana].y) | 0x10000, 1);
						playGnapPullOutDevice(2, 5);
						playGnapUseDevice(0, 0);
						_gnapActionStatus = kASGrabBanana;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						break;
					}
				}
			}
			break;

		case kHSOldLady:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex == kItemGroceryStoreHat) {
					_newSceneNum = 47;
					gnapWalkTo(4, 6, 0, getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
					_gnapActionStatus = kASUseHatWithOldLady;
				} else if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(4, 6, 7, 4);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(7, 4);
						break;
					case GRAB_CURSOR:
						_gnapIdleFacing = kDirUpLeft;
						_hotspots[kHSWalkArea1]._flags |= SF_WALKABLE;
						gnapWalkTo(7, 6, 0, getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
						_gnapActionStatus = kASGrabOldLady;
						_hotspots[kHSWalkArea1]._flags &= ~SF_WALKABLE;
						break;
					case TALK_CURSOR:
						_gnapIdleFacing = kDirUpRight;
						gnapWalkTo(_hotspotsWalkPos[kHSOldLady].x, _hotspotsWalkPos[kHSOldLady].y, 0, getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_gnapActionStatus = kASTalkOldLady;
						break;
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;
		
		case kHSExitOutsideGrubCity:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = true;
				_newSceneNum = 20;
				gnapWalkTo(_hotspotsWalkPos[kHSExitOutsideGrubCity].x, _hotspotsWalkPos[kHSExitOutsideGrubCity].y, 0, 0x107B3, 1);
				_gnapActionStatus = kASLeaveScene;
				platypusWalkTo(_hotspotsWalkPos[kHSExitOutsideGrubCity].x + 1, _hotspotsWalkPos[kHSExitOutsideGrubCity].y, -1, 0x107C2, 1);
			}
			break;
		
		case kHSWalkArea1:
		case kHSWalkArea2:
			gnapWalkTo(-1, -1, -1, -1, 1);
			break;
		
		default:
			if (_mouseClickState._left) {
				gnapWalkTo(-1, -1, -1, -1, 1);
				_mouseClickState._left = false;
			}
			break;
		
		}
	
		scene21_updateAnimations();
	
		if (!isSoundPlaying(0x10940))
			playSound(0x10940, true);
	
		if (!_isLeavingScene) {
			updatePlatypusIdleSequence();
			updateGnapIdleSequence();
			if (!isFlag(kGFTwigTaken) && !_timers[4] && _s21_nextOldLadySequenceId == -1 && _gnapActionStatus == -1) {
				_timers[4] = getRandom(30) + 50;
				switch (getRandom(5)) {
				case 0:
					_s21_nextOldLadySequenceId = 0x88;
					break;
				case 1:
					_s21_nextOldLadySequenceId = 0x8A;
					break;
				default:
					_s21_nextOldLadySequenceId = 0x89;
					break;
				}
			}
			if (!_timers[5]) {
				_timers[5] = getRandom(100) + 100;
				_gameSys->insertSequence(0x92, 255, 0, 0, kSeqNone, 0, 0, 0);
			}
			playSoundA();
		}
	
		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene21_updateHotspots();
		}
		
		gameUpdateTick();
	}
}

void GnapEngine::scene21_updateAnimations() {
	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		switch (_gnapActionStatus) {
		case kASTalkOldLady:
			_s21_nextOldLadySequenceId = 0x8B;
			_gnapActionStatus = -1;
			break;
		case kASGrabBanana:
			_gameSys->setAnimation(0x8C, 59, 0);
			_gameSys->insertSequence(0x8C, 59, 141, 59, kSeqSyncWait, 0, 0, 0);
			setFlag(kGFUnk04);
			invAdd(kItemBanana);
			scene21_updateHotspots();
			_gnapActionStatus = kASGrabBananaDone;
			break;
		case kASGrabBananaDone:
			setGrabCursorSprite(kItemBanana);
			_gnapActionStatus = -1;
			break;
		case kASGrabOldLady:
			_timers[4] = getRandom(30) + 50;
			_s21_nextOldLadySequenceId = 0x87;
			break;
		case kASUseHatWithOldLady:
			_gameSys->setAnimation(0x8F, _gnapId, 0);
			_gameSys->insertSequence(0x8F, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gnapSequenceDatNum = 0;
			_gnapSequenceId = 0x8F;
			_gnapActionStatus = kASUseHatWithOldLadyDone;
			invAdd(kItemTickets);
			invRemove(kItemGroceryStoreHat);
			setGrabCursorSprite(-1);
			break;
		case kASUseHatWithOldLadyDone:
			_s21_nextOldLadySequenceId = 0x91;
			break;
		case kASLeaveScene:
			_sceneDone = true;
			break;
		}
	}
	
	if (_gameSys->getAnimationStatus(3) == 2 && _s21_nextOldLadySequenceId != -1) {
		if (_s21_nextOldLadySequenceId == 0x87) {
			_gameSys->setAnimation(_s21_nextOldLadySequenceId, 79, 3);
			_gameSys->insertSequence(_s21_nextOldLadySequenceId, 79, _s21_currOldLadySequenceId, 79, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0x86, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gnapSequenceId = 0x86;
			_gnapSequenceDatNum = 0;
			_gnapActionStatus = -1;
			_s21_currOldLadySequenceId = _s21_nextOldLadySequenceId;
			_s21_nextOldLadySequenceId = -1;
		} else if (_s21_nextOldLadySequenceId == 0x91) {
			_gameSys->setAnimation(0x91, 79, 0);
			_gameSys->insertSequence(_s21_nextOldLadySequenceId, 79, _s21_currOldLadySequenceId, 79, kSeqSyncWait, 0, 0, 0);
			_gnapActionStatus = kASLeaveScene;
			_s21_currOldLadySequenceId = _s21_nextOldLadySequenceId;
			_s21_nextOldLadySequenceId = -1;
		} else {
			_gameSys->setAnimation(_s21_nextOldLadySequenceId, 79, 3);
			_gameSys->insertSequence(_s21_nextOldLadySequenceId, 79, _s21_currOldLadySequenceId, 79, kSeqSyncWait, 0, 0, 0);
			_s21_currOldLadySequenceId = _s21_nextOldLadySequenceId;
			_s21_nextOldLadySequenceId = -1;
		}
	}

}

} // End of namespace Gnap
