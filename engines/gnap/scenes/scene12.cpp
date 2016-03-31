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
	kHSExitRight	= 1,
	kHSToothGuy		= 2,
	kHSBarkeeper	= 3,
	kHSBeardGuy		= 4,
	kHSJukebox		= 5,
	kHSWalkArea1	= 6,
	kHSDevice		= 7,
	kHSWalkArea2	= 8,
	kHSWalkArea3	= 9,
	kHSWalkArea4	= 10
};

enum {
	kASLeaveScene					= 0,
	kASQuarterToToothGuyDone		= 1,
	kASTalkToothGuy					= 2,
	kASGrabToothGuy					= 4,
	kASShowItemToToothGuy			= 5,
	kASQuarterWithHoleToToothGuy	= 6,
	kASQuarterToToothGuy			= 7,
	kASTalkBeardGuy					= 8,
	kASLookBeardGuy					= 9,
	kASGrabBeardGuy					= 10,
	kASShowItemToBeardGuy			= 11,
	kASTalkBarkeeper				= 12,
	kASLookBarkeeper				= 13,
	kASShowItemToBarkeeper			= 15,
	kASQuarterWithBarkeeper			= 16,
	kASPlatWithBarkeeper			= 17,
	kASPlatWithToothGuy				= 18,
	kASPlatWithBeardGuy				= 19
};

int GnapEngine::scene12_init() {
	return 0x209;
}

void GnapEngine::scene12_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSExitRight, 790, 360, 799, 599, SF_EXIT_R_CURSOR);
	setHotspot(kHSToothGuy, 80, 180, 160, 380, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSBarkeeper, 490, 175, 580, 238, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSBeardGuy, 620, 215, 720, 350, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSJukebox, 300, 170, 410, 355, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSWalkArea1, 0, 0, 260, 460);
	setHotspot(kHSWalkArea2, 0, 0, 380, 410);
	setHotspot(kHSWalkArea3, 0, 0, 799, 395);
	setHotspot(kHSWalkArea4, 585, 0, 799, 455);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_hotspotsCount = 11;
}

void GnapEngine::scene12_run() {
	int v18 = 1;

	queueInsertDeviceIcon();

	_gameSys->insertSequence(0x207, 256, 0, 0, kSeqNone, 0, 0, 0);
	_gameSys->insertSequence(0x200, 50, 0, 0, kSeqNone, 0, 0, 0);
	
	_s12_currToothGuySequenceId = 0x200;
	_s12_nextToothGuySequenceId = -1;
	
	_gameSys->setAnimation(0x200, 50, 2);
	_gameSys->insertSequence(0x202, 50, 0, 0, kSeqNone, 0, 0, 0);
	
	_s12_currBeardGuySequenceId = 0x202;
	_s12_nextBeardGuySequenceId = -1;
	
	_gameSys->setAnimation(0x202, 50, 4);
	_gameSys->insertSequence(0x203, 50, 0, 0, kSeqNone, 0, 0, 0);
	
	_s12_currBarkeeperSequenceId = 0x203;
	_s12_nextBarkeeperSequenceId = -1;
	
	_gameSys->setAnimation(0x203, 50, 3);
	
	_timers[4] = 30;
	_timers[6] = getRandom(30) + 20;
	_timers[5] = getRandom(30) + 20;
	_timers[7] = getRandom(100) + 100;
	
	if (_prevSceneNum == 15) {
		initGnapPos(5, 6, kDirBottomRight);
		initBeaverPos(3, 7, 0);
		endSceneInit();
	} else {
		initGnapPos(11, 8, kDirBottomLeft);
		initBeaverPos(12, 8, 4);
		endSceneInit();
		gnapWalkTo(8, 8, -1, 0x107BA, 1);
		platypusWalkTo(9, 8, -1, 0x107D2, 1);
	}

	while (!_sceneDone) {
	
		testWalk(0, 0, -1, -1, -1, -1);
	
		updateMouseCursor();
		updateCursorByHotspot();
		updateGrabCursorSprite(0, 0);
	
		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);
	
		switch (_sceneClickedHotspot) {

		case kHSDevice:
			if (_gnapActionStatus < 0) {
				runMenu();
				scene12_updateHotspots();
			}
			break;

		case kHSPlatypus:
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
				break;
			}
			break;

		case kHSExitRight:
			_isLeavingScene = 1;
			gnapWalkTo(10, -1, 0, 0x107AB, 1);
			_gnapActionStatus = kASLeaveScene;
			platypusWalkTo(10, -1, -1, -1, 1);
			_newSceneNum = 11;
			break;

		case kHSToothGuy:
			if (_grabCursorSpriteIndex == kItemQuarter) {
				_largeSprite = _gameSys->createSurface(0x141);
				gnapWalkTo(3, 7, 0, 0x107BC, 9);
				_gnapIdleFacing = kDirUpLeft;
				_gnapActionStatus = kASQuarterToToothGuy;
				playGnapShowItem(_grabCursorSpriteIndex, 2, 0);
				setGrabCursorSprite(-1);
			} else if (_grabCursorSpriteIndex == kItemQuarterWithHole) {
				gnapWalkTo(3, 7, 0, 0x107BC, 9);
				_gnapIdleFacing = kDirUpLeft;
				_gnapActionStatus = kASQuarterWithHoleToToothGuy;
				playGnapShowItem(_grabCursorSpriteIndex, 2, 0);
			} else if (_grabCursorSpriteIndex >= 0) {
				gnapWalkTo(3, 7, 0, 0x107BC, 9);
				_gnapIdleFacing = kDirUpLeft;
				_gnapActionStatus = kASShowItemToToothGuy;
				playGnapShowItem(_grabCursorSpriteIndex, 2, 0);
			} else {
				switch (_verbCursor) {
				case LOOK_CURSOR:
					playGnapScratchingHead(1, 2);
					break;
				case GRAB_CURSOR:
					gnapWalkTo(3, 7, 0, 0x107BC, 1);
					_gnapActionStatus = kASGrabToothGuy;
					break;
				case TALK_CURSOR:
					_gnapIdleFacing = kDirUpLeft;
					gnapWalkTo(3, 7, 0, getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
					_gnapActionStatus = kASTalkToothGuy;
					break;
				case PLAT_CURSOR:
					gnapUseDeviceOnBeaver();
					platypusWalkTo(3, 7, 1, 0x107D2, 1);
					_beaverActionStatus = kASPlatWithToothGuy;
					_beaverFacing = 4;
					playGnapIdle(2, 7);
					break;
				}
			}
			break;

		case kHSBarkeeper:
			if (_grabCursorSpriteIndex == kItemQuarter || _grabCursorSpriteIndex == kItemQuarterWithHole) {
				gnapWalkTo(6, 6, 0, 0x107BB, 9);
				_gnapIdleFacing = kDirUpRight;
				_gnapActionStatus = kASQuarterWithBarkeeper;
				playGnapShowItem(_grabCursorSpriteIndex, 7, 0);
			} else if (_grabCursorSpriteIndex >= 0) {
				gnapWalkTo(6, 6, 0, 0x107BB, 9);
				_gnapIdleFacing = kDirUpRight;
				_gnapActionStatus = kASShowItemToBarkeeper;
				playGnapShowItem(_grabCursorSpriteIndex, 7, 0);
			} else {
				switch (_verbCursor) {
				case LOOK_CURSOR:
					gnapWalkTo(6, 6, 0, 0x107BB, 1);
					_gnapIdleFacing = kDirUpRight;
					_gnapActionStatus = kASLookBarkeeper;
					break;
				case GRAB_CURSOR:
					playGnapImpossible(0, 0);
					break;
				case TALK_CURSOR:
					_gnapIdleFacing = kDirUpRight;
					gnapWalkTo(6, 6, 0, getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
					_gnapActionStatus = kASTalkBarkeeper;
					break;
				case PLAT_CURSOR:
					playGnapPullOutDevice(_platX, _platY);
					_gameSys->setAnimation(makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, 0);
					_gnapActionStatus = kASPlatWithBarkeeper;
					break;
				}
			}
			break;

		case kHSBeardGuy:
			if (_grabCursorSpriteIndex >= 0) {
				gnapWalkTo(7, 6, 0, 0x107BB, 9);
				_gnapIdleFacing = kDirUpRight;
				_gnapActionStatus = kASShowItemToBeardGuy;
				playGnapShowItem(_grabCursorSpriteIndex, 8, 0);
			} else {
				switch (_verbCursor) {
				case LOOK_CURSOR:
					gnapWalkTo(7, 6, 0, 0x107BB, 1);
					_gnapIdleFacing = kDirUpRight;
					_gnapActionStatus = kASLookBeardGuy;
					break;
				case GRAB_CURSOR:
					// NOTE Bug in the original. It has 9 as flags which seems wrong here.
					gnapWalkTo(7, 6, 0, 0x107BB, 1);
					_gnapIdleFacing = kDirUpRight;
					_gnapActionStatus = kASGrabBeardGuy;
					break;
				case TALK_CURSOR:
					_gnapIdleFacing = kDirUpRight;
					gnapWalkTo(7, 6, 0, getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
					_gnapActionStatus = kASTalkBeardGuy;
					break;
				case PLAT_CURSOR:
					gnapUseDeviceOnBeaver();
					platypusWalkTo(7, 6, 1, 0x107C2, 1);
					_beaverActionStatus = kASPlatWithBeardGuy;
					_beaverFacing = 0;
					playGnapIdle(7, 6);
					break;
				}
			}
			break;

		case kHSJukebox:
			_newSceneNum = 15;
			_isLeavingScene = 1;
			gnapWalkTo(5, 6, 0, 0x107BC, 1);
			_gnapActionStatus = kASLeaveScene;
			break;

		case kHSWalkArea1:
		case kHSWalkArea2:
		case kHSWalkArea3:
		case kHSWalkArea4:
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
	
		scene12_updateAnimations();
	
		if (!_isLeavingScene) {
			updateBeaverIdleSequence();
			updateGnapIdleSequence();
			if (!_timers[4]) {
				_timers[4] = 15;
				if (_s12_nextToothGuySequenceId == -1) {
					if (v18 == 0 && _s12_currBeardGuySequenceId == 0x202 && _s12_currBarkeeperSequenceId == 0x203 && _gnapActionStatus < 0 && _beaverActionStatus < 0) {
						if (getRandom(2) != 0)
							_s12_nextToothGuySequenceId = 0x1EC;
						else
							_s12_nextToothGuySequenceId = 0x204;
					} else if (_s12_currToothGuySequenceId != 0x200)
						_s12_nextToothGuySequenceId = 0x200;
					v18 = (v18 + 1) % 15;
				}
			}
			if (!_timers[5]) {
				_timers[5] = getRandom(30) + 20;
				if (_s12_nextBarkeeperSequenceId == -1 && _gnapActionStatus < 0 && _beaverActionStatus < 0) {
					if (v18 == 0 && _s12_currToothGuySequenceId == 0x200 && _s12_currBeardGuySequenceId == 0x202 && _gnapActionStatus < 0 && _beaverActionStatus < 0) {
						if (getRandom(2) != 0)
							_s12_nextBarkeeperSequenceId = 0x208;
						else
							_s12_nextBarkeeperSequenceId = 0x1FB;
					} else
						_s12_nextBarkeeperSequenceId = 0x203;
					v18 = (v18 + 1) % 15;
				}
			}
			if (!_timers[6]) {
				_timers[6] = getRandom(30) + 15;
				if (_s12_nextBeardGuySequenceId == -1 && _gnapActionStatus < 0 && _beaverActionStatus < 0) {
					if (v18 == 0 && _s12_currToothGuySequenceId == 0x200 && _s12_currBarkeeperSequenceId == 0x203 && _gnapActionStatus < 0 && _beaverActionStatus < 0)
						_s12_nextBeardGuySequenceId = 0x1F2;
					else
						_s12_nextBeardGuySequenceId = 0x202;
					v18 = (v18 + 1) % 15;
				}
			}
			if (!_timers[7]) {
				_timers[7] = getRandom(100) + 100;
				int _gnapRandomValue = getRandom(3);
				switch (_gnapRandomValue) {
				case 0:
					_gameSys->insertSequence(0x8A5 | 0x10000, 179, 0, 0, kSeqNone, 0, 0, 0);
					break;
				case 1:
					_gameSys->insertSequence(0x8A7 | 0x10000, 179, 0, 0, kSeqNone, 0, 0, 0);
					break;
				case 2:
					_gameSys->insertSequence(0x8A6 | 0x10000, 179, 0, 0, kSeqNone, 0, 0, 0);
					break;
				}
			}
		}
	
		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene12_updateHotspots();
			_timers[4] = 30;
			_timers[5] = getRandom(30) + 20;
			_timers[6] = getRandom(30) + 20;
		}
		
		gameUpdateTick();
	
	}

}

void GnapEngine::scene12_updateAnimations() {

	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		switch (_gnapActionStatus) {
		case kASLeaveScene:
			_sceneDone = true;
			break;
		case kASTalkToothGuy:
			if (isKeyStatus1(Common::KEYCODE_j)) {
				// Easter egg
				clearKeyStatus1(Common::KEYCODE_j);
				_s12_nextToothGuySequenceId = 0x206;
			} else {
				_s12_nextToothGuySequenceId = 0x1EE;
			}
			break;
		case 3:
			break;
		case kASGrabToothGuy:
			if (isKeyStatus1(Common::KEYCODE_j)) {
				clearKeyStatus1(Common::KEYCODE_j);
				_s12_nextToothGuySequenceId = 0x206;
			} else {
				_s12_nextToothGuySequenceId = 0x1EF;
			}
			break;
		case kASShowItemToToothGuy:
			if (isKeyStatus1(Common::KEYCODE_j)) {
				clearKeyStatus1(Common::KEYCODE_j);
				_s12_nextToothGuySequenceId = 0x206;
			} else {
				_s12_nextToothGuySequenceId = 0x1ED;
			}
			break;
		case kASQuarterWithHoleToToothGuy:
			if (isKeyStatus1(Common::KEYCODE_j)) {
				clearKeyStatus1(Common::KEYCODE_j);
				_s12_nextToothGuySequenceId = 0x206;
			} else {
				_s12_nextToothGuySequenceId = 0x1EA;
			}
			break;
		case kASQuarterToToothGuy:
			if (isKeyStatus1(Common::KEYCODE_j)) {
				clearKeyStatus1(Common::KEYCODE_j);
				_s12_nextToothGuySequenceId = 0x206;
			} else {
				_s12_nextToothGuySequenceId = 0x1E9;
			}
			break;
		case kASQuarterToToothGuyDone:
			_gnapActionStatus = -1;
			showCursor();
			_gameSys->removeSpriteDrawItem(_largeSprite, 300);
			deleteSurface(&_largeSprite);
			setGrabCursorSprite(kItemQuarterWithHole);
			break;
		case kASTalkBeardGuy:
			_s12_nextBeardGuySequenceId = 0x1F4;
			break;
		case kASLookBeardGuy:
			_s12_nextBeardGuySequenceId = 0x1F3;
			break;
		case kASGrabBeardGuy:
			_s12_nextBeardGuySequenceId = 0x1F1;
			break;
		case kASShowItemToBeardGuy:
			_s12_nextBeardGuySequenceId = 0x1F0;
			break;
		case kASTalkBarkeeper:
			if (getRandom(2) != 0)
				_s12_nextBarkeeperSequenceId = 0x1FD;
			else
				_s12_nextBarkeeperSequenceId = 0x1FF;
			break;
		case kASLookBarkeeper:
			_s12_nextBarkeeperSequenceId = 0x1F8;
			break;
		case 14:
			_s12_nextBarkeeperSequenceId = 0x1F6;
			break;
		case kASShowItemToBarkeeper:
			_s12_nextBarkeeperSequenceId = 0x1F5;
			break;
		case kASQuarterWithBarkeeper:
			_s12_nextBarkeeperSequenceId = 0x1FA;
			break;
		case kASPlatWithBarkeeper:
			_s12_nextBarkeeperSequenceId = 0x1F9;
			break;
		}
	}
	
	if (_gameSys->getAnimationStatus(1) == 2) {
		_gameSys->setAnimation(0, 0, 1);
		switch (_beaverActionStatus) {
		case kASPlatWithToothGuy:
			_s12_nextToothGuySequenceId = 0x1EB;
			break;
		case kASPlatWithBeardGuy:
			_s12_nextBeardGuySequenceId = 0x1F3;
			break;
		}
	}
	
	if (_gameSys->getAnimationStatus(2) == 2) {
		if (_s12_currToothGuySequenceId == 0x1E9) {
			_gameSys->setAnimation(0, 0, 2);
			hideCursor();
			_gameSys->setAnimation(0x10843, 301, 0);
			_gnapActionStatus = kASQuarterToToothGuyDone;
			_gameSys->insertSpriteDrawItem(_largeSprite, 0, 0, 300);
			_gameSys->insertSequence(0x10843, 301, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0x107B7, _gnapId, 0x10843, 301,
				kSeqSyncWait, 0, 75 * _gnapX - _gnapGridX, 48 * _gnapY - _gnapGridY);
			_gnapSequenceId = 0x7B7;
			_gnapSequenceDatNum = 1;
			setFlag(3);
			invAdd(kItemQuarterWithHole);
			invRemove(kItemQuarter);
		}
		if (_s12_nextToothGuySequenceId == 0x1EF) {
			_gameSys->setAnimation(_s12_nextToothGuySequenceId, 50, 2);
			_gameSys->insertSequence(_s12_nextToothGuySequenceId, 50, _s12_currToothGuySequenceId, 50, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0x205, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_s12_currToothGuySequenceId = _s12_nextToothGuySequenceId;
			_s12_nextToothGuySequenceId = -1;
			_gnapSequenceId = 0x205;
			_gnapSequenceDatNum = 0;
			_timers[4] = 40;
			_timers[2] = getRandom(20) + 70;
			_timers[3] = getRandom(50) + 200;
			if (_gnapActionStatus == kASGrabToothGuy)
				_gnapActionStatus = -1;
		} else if (_s12_nextToothGuySequenceId != -1) {
			_gameSys->insertSequence(_s12_nextToothGuySequenceId, 50, _s12_currToothGuySequenceId, 50, kSeqSyncWait, 0, 0, 0);
			_gameSys->setAnimation(_s12_nextToothGuySequenceId, 50, 2);
			_s12_currToothGuySequenceId = _s12_nextToothGuySequenceId;
			_s12_nextToothGuySequenceId = -1;
			_timers[4] = 50;
			if (_gnapActionStatus >= kASTalkToothGuy && _gnapActionStatus <= kASQuarterToToothGuy && _s12_currToothGuySequenceId != 0x1E9 &&
				_s12_currToothGuySequenceId != 0x1EC && _s12_currToothGuySequenceId != 0x200)
				_gnapActionStatus = -1;
			if (_beaverActionStatus == kASPlatWithToothGuy)
				_beaverActionStatus = -1;
		}
	}
	
	if (_gameSys->getAnimationStatus(3) == 2) {
		if (_gnapActionStatus == kASPlatWithBarkeeper && _s12_currBarkeeperSequenceId == 0x1F9) {
			_gnapActionStatus = -1;
			playGnapIdle(7, 6);
			_timers[5] = 0;
		}
		if (_s12_nextBarkeeperSequenceId != -1) {
			_gameSys->insertSequence(_s12_nextBarkeeperSequenceId, 50, _s12_currBarkeeperSequenceId, 50, kSeqSyncWait, 0, 0, 0);
			_gameSys->setAnimation(_s12_nextBarkeeperSequenceId, 50, 3);
			_s12_currBarkeeperSequenceId = _s12_nextBarkeeperSequenceId;
			_s12_nextBarkeeperSequenceId = -1;
			_timers[5] = getRandom(30) + 20;
			if (_gnapActionStatus >= kASTalkBarkeeper && _gnapActionStatus <= kASQuarterWithBarkeeper && _s12_currBarkeeperSequenceId != 0x203 &&
				_s12_currBarkeeperSequenceId != 0x1FB && _s12_currBarkeeperSequenceId != 0x208)
				_gnapActionStatus = -1;
		}
	}
	
	if (_gameSys->getAnimationStatus(4) == 2 && _s12_nextBeardGuySequenceId != -1) {
		_gameSys->insertSequence(_s12_nextBeardGuySequenceId, 50, _s12_currBeardGuySequenceId, 50, kSeqSyncWait, 0, 0, 0);
		_gameSys->setAnimation(_s12_nextBeardGuySequenceId, 50, 4);
		_s12_currBeardGuySequenceId = _s12_nextBeardGuySequenceId;
		_s12_nextBeardGuySequenceId = -1;
		_timers[6] = getRandom(30) + 20;
		if (_gnapActionStatus >= kASTalkBeardGuy && _gnapActionStatus <= kASShowItemToBeardGuy && _s12_currBeardGuySequenceId != 0x202 && _s12_currBeardGuySequenceId != 0x1F2)
			_gnapActionStatus = -1;
		if (_beaverActionStatus == kASPlatWithBeardGuy)
			_beaverActionStatus = -1;
	}

}

} // End of namespace Gnap
