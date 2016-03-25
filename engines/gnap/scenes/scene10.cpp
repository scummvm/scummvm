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
	kHSExitBar		= 1,
	kHSExitBackdoor	= 2,
	kHSCook			= 3,
	kHSTongs		= 4,
	kHSBox			= 5,
	kHSOven			= 6,
	kHSWalkArea1	= 7,
	kHSDevice		= 8,
	kHSWalkArea2	= 9,
	kHSWalkArea3	= 10,
	kHSWalkArea4	= 11
};

enum {
	kASLeaveScene		= 0,
	kASAnnoyCook		= 1,
	kASPlatWithBox		= 4
};

int GnapEngine::scene10_init() {
	_gameSys->setAnimation(0, 0, 0);
	_gameSys->setAnimation(0, 0, 1);
	_gameSys->setAnimation(0, 0, 2);
	return 0x10F;
}

void GnapEngine::scene10_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSExitBar, 0, 75, 85, 455, SF_EXIT_NW_CURSOR);
	setHotspot(kHSExitBackdoor, 75, 590, 500, 599, SF_EXIT_D_CURSOR | SF_WALKABLE);
	setHotspot(kHSCook, 370, 205, 495, 460, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSTongs, 250, 290, 350, 337, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSBox, 510, 275, 565, 330, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSOven, 690, 280, 799, 420, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSWalkArea1, 59, 0, 495, 460);
	setHotspot(kHSWalkArea2, 495, 0, 650, 420);
	setHotspot(kHSWalkArea3, 651, 0, 725, 400);
	setHotspot(kHSWalkArea4, 725, 0, 799, 441);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_hotspotsCount = 12;
}

void GnapEngine::scene10_run() {

	_s10_currCookSequenceId = 0x103;
	
	_gameSys->setAnimation(0x103, 100, 2);
	_gameSys->insertSequence(0x103, 100, 0, 0, kSeqNone, 0, 0, 0);
	
	_s10_nextCookSequenceId = 0x106;
	if (!isFlag(1))
		_gameSys->insertSequence(0x107, 100, 0, 0, kSeqNone, 0, 0, 0);
	
	queueInsertDeviceIcon();
	
	if (_prevSceneNum == 9) {
		initGnapPos(11, 8, 3);
		initBeaverPos(12, 7, 4);
		endSceneInit();
		gnapWalkTo(9, 8, -1, 0x107BA, 1);
		platypusWalkTo(9, 7, -1, 0x107D2, 1);
	} else {
		initGnapPos(-1, 7, 1);
		initBeaverPos(-2, 8, 0);
		endSceneInit();
		gnapWalkTo(1, 7, -1, 0x107B9, 1);
		platypusWalkTo(1, 8, -1, 0x107C2, 1);
	}

	_timers[4] = getRandom(80) + 150;
	_timers[5] = getRandom(100) + 100;

	while (!_sceneDone) {
	
		if (!isSoundPlaying(0x1091E))
			playSound(0x1091E, 1);
	
		if (!isSoundPlaying(0x1091A))
			playSound(0x1091A, 1);
	
		updateMouseCursor();
	
		testWalk(0, 0, -1, -1, -1, -1);
	
		updateCursorByHotspot();
	
		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);
	
		switch (_sceneClickedHotspot) {

		case kHSPlatypus:
			if (_grabCursorSpriteIndex >= 0) {
				playGnapImpossible(0, 0);
			} else {
				switch (_verbCursor) {
				case LOOK_CURSOR:
					if (isFlag(1))
						playGnapMoan1(_platX, _platY);
					else
						playGnapScratchingHead(_platX, _platY);
					break;
				case GRAB_CURSOR:
					gnapKissPlatypus(10);
					break;
				case TALK_CURSOR:
					playGnapBrainPulsating(_platX, _platY);
					playBeaverSequence(getBeaverSequenceId(0, 0, 0) | 0x10000);
					break;
				case PLAT_CURSOR:
					break;
				}
			}
			break;

		case kHSExitBar:
			_isLeavingScene = 1;
			gnapActionIdle(0x10C);
			gnapWalkTo(0, 7, 0, 0x107AF, 1);
			_gnapActionStatus = kASLeaveScene;
			platypusWalkTo(0, 7, -1, 0x107CF, 1);
			_newSceneNum = 11;
			break;

		case kHSExitBackdoor:
			_isLeavingScene = 1;
			gnapActionIdle(0x10C);
			gnapWalkTo(2, 9, 0, 0x107AE, 1);
			_gnapActionStatus = kASLeaveScene;
			platypusWalkTo(3, 9, -1, 0x107C7, 1);
			_newSceneNum = 9;
			break;

		case kHSCook:
			if (_grabCursorSpriteIndex >= 0) {
				playGnapShowCurrItem(4, 8, 6, 0);
				_gameSys->setAnimation(makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, 0);
				_gnapActionStatus = kASAnnoyCook;
			} else {
				switch (_verbCursor) {
				case LOOK_CURSOR:
					playGnapScratchingHead(6, 0);
					break;
				case GRAB_CURSOR:
					playGnapImpossible(0, 0);
					_gnapIdleFacing = 1;
					break;
				case TALK_CURSOR:
					_gnapIdleFacing = 7;
					gnapActionIdle(0x10C);
					gnapWalkTo(4, 8, 0, getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
					_gnapActionStatus = kASAnnoyCook;
					break;
				case PLAT_CURSOR:
					gnapActionIdle(0x10C);
					gnapUseDeviceOnBeaver();
					platypusWalkTo(4, 6, -1, -1, 1);
					gnapWalkTo(4, 8, 0, 0x107BB, 1);
					_gnapActionStatus = kASAnnoyCook;
					break;
				}
			}
			break;

		case kHSTongs:
			if (_grabCursorSpriteIndex >= 0) {
				playGnapShowCurrItem(3, 7, 4, 0);
			} else {
				switch (_verbCursor) {
				case LOOK_CURSOR:
					if (isFlag(1))
						playGnapMoan2(-1, -1);
					else
						playGnapScratchingHead(4, 3);
					break;
				case GRAB_CURSOR:
					if (isFlag(1))
						playGnapMoan2(-1, -1);
					else {
						gnapActionIdle(0x10C);
						gnapWalkTo(4, 8, 0, 0x107BB, 1);
						_gnapActionStatus = kASAnnoyCook;
					}
					break;
				case TALK_CURSOR:
					playGnapImpossible(0, 0);
					break;
				case PLAT_CURSOR:
					if (isFlag(1))
						playGnapMoan2(-1, -1);
					else {
						gnapActionIdle(0x10C);
						gnapUseDeviceOnBeaver();
						platypusWalkTo(3, 7, -1, -1, 1);
						gnapWalkTo(4, 8, 0, 0x107BB, 1);
						_gnapActionStatus = kASAnnoyCook;
					}
					break;
				}
			}
			break;

		case kHSBox:
			if (_grabCursorSpriteIndex >= 0) {
				playGnapShowCurrItem(7, 6, 6, 0);
			} else {
				switch (_verbCursor) {
				case LOOK_CURSOR:
					playGnapScratchingHead(7, 3);
					break;
				case GRAB_CURSOR:
					gnapActionIdle(0x10C);
					gnapWalkTo(4, 8, 0, 0x107BB, 1);
					_gnapActionStatus = kASAnnoyCook;
					break;
				case TALK_CURSOR:
					playGnapImpossible(0, 0);
					break;
				case PLAT_CURSOR:
					if (isFlag(1))
						playGnapMoan2(-1, -1);
					else {
						invAdd(kItemTongs);
						setFlag(1);
						gnapActionIdle(0x10C);
						gnapUseDeviceOnBeaver();
						platypusWalkTo(7, 6, 1, 0x107D2, 1);
						_beaverActionStatus = kASPlatWithBox;
						_beaverFacing = 4;
						_largeSprite = _gameSys->createSurface(0xC3);
						playGnapIdle(7, 6);
					}
					break;
				}
			}
			break;

		case kHSOven:
			if (_grabCursorSpriteIndex >= 0) {
				playGnapShowCurrItem(9, 6, 10, 0);
			} else {
				switch (_verbCursor) {
				case LOOK_CURSOR:
					playGnapSequence(getGnapSequenceId(gskDeflect, 10, 5) | 0x10000);
					break;
				case GRAB_CURSOR:
					gnapActionIdle(0x10C);
					gnapWalkTo(9, 6, 0, 0x107BB, 1);
					_gameSys->insertSequence(0x10E, 120, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
					_gnapSequenceId = 0x10E;
					_gnapId = 120;
					_gnapIdleFacing = 7;
					_gnapSequenceDatNum = 0;
					_gnapX = 9;
					_gnapY = 6;
					_timers[2] = 360;
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
		case kHSWalkArea3:
		case kHSWalkArea4:
			gnapActionIdle(0x10C);
			gnapWalkTo(-1, -1, -1, -1, 1);
			break;

		case kHSDevice:
			if (_gnapActionStatus < 0) {
				runMenu();
				scene10_updateHotspots();
			}
			break;
			
		default:
			if (_mouseClickState._left) {
				gnapActionIdle(0x10C);
				gnapWalkTo(-1, -1, -1, -1, 1);
				_mouseClickState._left = false;
			}
			break;

		}
	
		scene10_updateAnimations();
	
		if (!_isLeavingScene) {
			updateBeaverIdleSequence();
			updateGnapIdleSequence();
			if (!_timers[4]) {
				_timers[4] = getRandom(80) + 150;
				playSound(0x12B, 0);
			}
			if (!_timers[5]) {
				_timers[5] = getRandom(100) + 100;
				int _gnapRandomValue = getRandom(4);
				if (_gnapRandomValue) {
					int sequenceId;
					if (_gnapRandomValue == 1) {
						sequenceId = 0x8A5;
					} else if (_gnapRandomValue == 2) {
						sequenceId = 0x8A6;
					} else {
						sequenceId = 0x8A7;
					}
					_gameSys->insertSequence(sequenceId | 0x10000, 179, 0, 0, kSeqNone, 0, 0, 0);
				}
			}
		}
	
		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene10_updateHotspots();
		}
		
		gameUpdateTick();
		
	}

}

void GnapEngine::scene10_updateAnimations() {

	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		switch (_gnapActionStatus) {
		case kASLeaveScene:
			_sceneDone = true;
			break;
		case kASAnnoyCook:
			_s10_nextCookSequenceId = 0x105;
			break;
		}
	}
	
	if (_gameSys->getAnimationStatus(1) == 2) {
		_gameSys->setAnimation(0, 0, 1);
		switch (_beaverActionStatus) {
		case kASPlatWithBox:
			_s10_nextCookSequenceId = 0x109;
			break;
		}
	}
	
	if (_gameSys->getAnimationStatus(2) == 2 && _s10_nextCookSequenceId != -1) {
		
		switch (_s10_nextCookSequenceId) {
		case 0x109:
			_platX = 4;
			_platY = 8;
			_gameSys->insertSequence(0x109, 100, _s10_currCookSequenceId, 100, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0x107C9, 160,
				_beaverSequenceId | (_beaverSequenceDatNum << 16), _beaverId,
				kSeqSyncWait, getSequenceTotalDuration(0x109) + getSequenceTotalDuration(0x10A) + getSequenceTotalDuration(0x10843),
				75 * _platX - _platGridX, 48 * _platY - _platGridY);
			_gameSys->removeSequence(0x107, 100, true);
			_s10_currCookSequenceId = 0x109;
			_s10_nextCookSequenceId = 0x843;
			_beaverSequenceId = 0x7C9;
			_beaverId = 160;
			_beaverFacing = 0;
			_beaverSequenceDatNum = 1;
			break;
		case 0x843:
			hideCursor();
			_gameSys->insertSpriteDrawItem(_largeSprite, 0, 0, 300);
			_gameSys->insertSequence(0x10843, 301, _s10_currCookSequenceId, 100, kSeqSyncWait, 0, 0, 0);
			_s10_currCookSequenceId = 0x843;
			_s10_nextCookSequenceId = 0x10A;
			break;
		case 0x10A:
			_gameSys->insertSequence(_s10_nextCookSequenceId, 100, 0x10843, 301, kSeqSyncWait, 0, 0, 0);
			_s10_currCookSequenceId = _s10_nextCookSequenceId;
			_s10_nextCookSequenceId = 0x104;
			showCursor();
			_gameSys->removeSpriteDrawItem(_largeSprite, 300);
			delayTicksCursor(5);
			deleteSurface(&_largeSprite);
			setGrabCursorSprite(kItemTongs);
			if (_beaverActionStatus == kASPlatWithBox)
				_beaverActionStatus = -1;
			if (_gnapX == 4 && _gnapY == 8)
				gnapWalkStep();
			break;
		default:
			_gameSys->insertSequence(_s10_nextCookSequenceId, 100, _s10_currCookSequenceId, 100, kSeqSyncWait, 0, 0, 0);
			_s10_currCookSequenceId = _s10_nextCookSequenceId;
			break;
		}
	
		switch (_s10_currCookSequenceId) {
		case 0x106: {
			// TODO: Refactor into a if + a switch
			int rnd = getRandom(7);
			if (_gnapActionStatus >= 0 || _beaverActionStatus >= 0)
				_s10_nextCookSequenceId = 0x106;
			else if (rnd == 0)
				_s10_nextCookSequenceId = 0x104;
			else if (rnd == 1)
				_s10_nextCookSequenceId = 0x103;
			else if (rnd == 2) {
				_s10_nextCookSequenceId = 0x106;
				_gameSys->insertSequence(0x10D, 1, 0, 0, kSeqNone, 0, 0, 0);
			} else
				_s10_nextCookSequenceId = 0x106;
			}
			break;
		case 0x103:
			if (_gnapActionStatus >= 0 || _beaverActionStatus >= 0)
				_s10_nextCookSequenceId = 0x106;
			else if (getRandom(7) == 0)
				_s10_nextCookSequenceId = 0x104;
			else
				_s10_nextCookSequenceId = 0x106;
			break;
		case 0x104:
			if (_gnapActionStatus >= 0 || _beaverActionStatus >= 0)
				_s10_nextCookSequenceId = 0x106;
			else if (getRandom(7) == 0)
				_s10_nextCookSequenceId = 0x103;
			else
				_s10_nextCookSequenceId = 0x106;
			break;
		case 0x105: {
			// TODO: Refactor into a if + a switch
			int rnd = getRandom(7);
			if (_gnapActionStatus >= 0 || _beaverActionStatus >= 0)
				_s10_nextCookSequenceId = 0x106;
			else if (rnd == 0)
				_s10_nextCookSequenceId = 0x104;
			else if (rnd == 1)
				_s10_nextCookSequenceId = 0x103;
			else
				_s10_nextCookSequenceId = 0x106;
			_timers[2] = getRandom(30) + 20;
			_timers[3] = 300;
			_gameSys->insertSequence(0x10C, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gnapSequenceId = 0x10C;
			_gnapIdleFacing = 7;
			_gnapSequenceDatNum = 0;
			_gnapActionStatus = -1;
			_beaverActionStatus = -1;
			}
			break;
		}
		if (_s10_currCookSequenceId == 0x843)
			_gameSys->setAnimation(_s10_currCookSequenceId | 0x10000, 301, 2);
		else
			_gameSys->setAnimation(_s10_currCookSequenceId, 100, 2);
	}
}

void GnapEngine::scene10_updateAnimationsCb() {
	if (_gameSys->getAnimationStatus(2) == 2) {
		_gameSys->setAnimation(_s10_nextCookSequenceId, 100, 2);
		_gameSys->insertSequence(_s10_nextCookSequenceId, 100, _s10_currCookSequenceId, 100, kSeqSyncWait, 0, 0, 0);
		_s10_currCookSequenceId = _s10_nextCookSequenceId;
		_s10_nextCookSequenceId = 0x106;
	}
}

} // End of namespace Gnap
