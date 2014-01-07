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
	kHSExitKitchen	= 1,
	kHSExitToilet	= 2,
	kHSExitLeft		= 3,
	kHSGoggleGuy	= 4,
	kHSHookGuy		= 5,
	kHSBillard		= 6,
	kHSWalkArea1	= 7,
	kHSDevice		= 8,
	kHSWalkArea2	= 9,
	kHSWalkArea3	= 10,
	kHSWalkArea4	= 11,
	kHSWalkArea5	= 12
};

enum {
	kASLeaveScene				= 0,
	kASShowMagazineToGoggleGuy	= 3,
	kASTalkGoggleGuy			= 4,
	kASGrabHookGuy				= 6,
	kASShowItemToHookGuy		= 8,
	kASTalkHookGuy				= 9,
	kASGrabBillardBall			= 11
};

int GnapEngine::scene11_init() {
	_gameSys->setAnimation(0, 0, 0);
	_gameSys->setAnimation(0, 0, 3);
	_gameSys->setAnimation(0, 0, 2);
	if (_prevSceneNum == 10 || _prevSceneNum == 13) {
		playSound(0x108EC, 0);
		playSound(0x10928, 0);
	}
	return 0x209;
}

void GnapEngine::scene11_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSExitKitchen, 420, 140, 520, 345, SF_EXIT_U_CURSOR);
	setHotspot(kHSExitToilet, 666, 130, 740, 364, SF_EXIT_R_CURSOR);
	setHotspot(kHSExitLeft, 0, 350, 10, 599, SF_EXIT_L_CURSOR | SF_WALKABLE);
	setHotspot(kHSGoggleGuy, 90, 185, 185, 340, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSHookGuy, 210, 240, 340, 430, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSBillard, 640, 475, 700, 530, SF_WALKABLE | SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSWalkArea1, 0, 0, 365, 453);
	setHotspot(kHSWalkArea2, 0, 0, 629, 353);
	setHotspot(kHSWalkArea3, 629, 0, 799, 364);
	setHotspot(kHSWalkArea4, 735, 0, 799, 397);
	setHotspot(kHSWalkArea5, 510, 540, 799, 599);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_hotspotsCount = 13;
}

void GnapEngine::scene11_run() {
	bool flag = true;
	
	_timers[7] = 50;
	
	_hotspots[kHSBillard].flags |= SF_DISABLED;
	
	_s11_currGoggleGuySequenceId = 0x1F9;
	
	_s11_currHookGuySequenceId = 0x201;
	
	switch (_prevSceneNum) {
	case 13:
		initGnapPos(8, 5, 3);
		initBeaverPos(9, 6, 4);
		break;
	case 47:
		initGnapPos(8, 5, 3);
		initBeaverPos(9, 5, 4);
		_s11_currGoggleGuySequenceId = 0x1FA;
		_s11_currHookGuySequenceId = 0x1FF;
		_timers[7] = 180;
		break;
	case 12:
		initGnapPos(-1, 9, 1);
		initBeaverPos(-2, 8, 0);
		break;
	default:
		initGnapPos(6, 6, 3);
		initBeaverPos(6, 5, 4);
		break;
	}
	
	queueInsertDeviceIcon();
	
	_gameSys->insertSequence(_s11_currHookGuySequenceId, 120, 0, 0, kSeqNone, 0, 0, 0);
	
	_s11_nextHookGuySequenceId = -1;
	
	_gameSys->setAnimation(_s11_currHookGuySequenceId, 120, 3);
	_gameSys->insertSequence(_s11_currGoggleGuySequenceId, 121, 0, 0, kSeqNone, 0, 0, 0);
	
	_s11_nextGoggleGuySequenceId = -1;
	
	_gameSys->setAnimation(_s11_currGoggleGuySequenceId, 121, 2);
	
	_timers[5] = getRandom(100) + 75;
	_timers[4] = getRandom(40) + 20;
	_timers[6] = getRandom(100) + 100;
	endSceneInit();
	
	if (_prevSceneNum == 12) {
		gnapWalkTo(2, 8, -1, 0x107B9, 1);
		platypusWalkTo(1, 8, -1, 0x107C2, 1);
	}

	_gameSys->insertSequence(0x208, 256, 0, 0, kSeqNone, 40, 0, 0);
	
	while (!_sceneDone) {
	
		testWalk(0, 0, -1, -1, -1, -1);
	
		updateMouseCursor();
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
			}
			break;

		case kHSExitKitchen:
			_isLeavingScene = true;
			gnapWalkTo(6, 5, 0, 0x107BF, 1);
			_gnapActionStatus = kASLeaveScene;
			platypusWalkTo(6, 6, -1, -1, 1);
			_newSceneNum = 10;
			break;

		case kHSExitToilet:
			_isLeavingScene = true;
			gnapWalkTo(8, 5, 0, 0x107BF, 1);
			_gnapActionStatus = kASLeaveScene;
			platypusWalkTo(8, 6, -1, -1, 1);
			_newSceneNum = 13;
			break;

		case kHSExitLeft:
			_isLeavingScene = true;
			gnapWalkTo(-1, 8, 0, 0x107AF, 1);
			_gnapActionStatus = kASLeaveScene;
			platypusWalkTo(-1, 9, -1, 0x107CF, 1);
			_newSceneNum = 12;
			break;

		case kHSGoggleGuy:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex == kItemMagazine) {
					gnapWalkTo(3, 7, 0, 0x107BC, 1);
					_gnapActionStatus = kASShowMagazineToGoggleGuy;
					playGnapShowItem(_grabCursorSpriteIndex, 2, 0);
				} else if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(3, 7, 2, 0);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(1, 6);
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					case TALK_CURSOR:
						_gnapIdleFacing = 5;
						gnapWalkTo(3, 7, 0, getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_gnapActionStatus = kASTalkGoggleGuy;
						break;
					}
				}
			}
			break;

		case kHSHookGuy:
			if (_gnapActionStatus < 0) {
				_gnapIdleFacing = 7;
				if (_grabCursorSpriteIndex >= 0) {
					gnapWalkTo(5, 6, 0, 0x107BC, 9);
					_gnapActionStatus = kASShowItemToHookGuy;
					playGnapShowItem(_grabCursorSpriteIndex, 4, 0);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapSequence(getGnapSequenceId(gskDeflect, 3, 6) | 0x10000);
						break;
					case GRAB_CURSOR:
						gnapWalkTo(5, 6, 0, 0x107BC, 1);
						_gnapActionStatus = kASGrabHookGuy;
						break;
					case TALK_CURSOR:
						_gnapIdleFacing = 3;
						gnapWalkTo(5, 6, 0, getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_gnapActionStatus = kASTalkHookGuy;
						break;
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSBillard:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapImpossible(9, 8);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(9, 8);
						break;
					case GRAB_CURSOR:
						gnapWalkTo(9, 8, 0, 0x107BA, 1);
						_gnapActionStatus = kASGrabBillardBall;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(9, 8);
						break;
					}
				}
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

		case kHSDevice:
			if (_gnapActionStatus < 0) {
				runMenu();
				scene11_updateHotspots();
			}
			break;
			
		default:
			if (_mouseClickState.left && _gnapActionStatus < 0) {
				gnapWalkTo(-1, -1, -1, -1, 1);
				_mouseClickState.left = false;
			}
			break;
			
		}

		scene11_updateAnimations();
	
		if (!_isLeavingScene) {
			if (flag && !_timers[7]) {
				flag = false;
				_gameSys->setAnimation(0x207, 257, 4);
				_gameSys->insertSequence(0x207, 257, 0, 0, kSeqNone, 0, 0, 0);
			}
			beaverSub426234();
			updateGnapIdleSequence2();
			if (!_timers[5]) {
				_timers[5] = getRandom(100) + 75;
				if (_gnapActionStatus < 0 && _beaverActionStatus < 0 && _s11_nextGoggleGuySequenceId == -1) {
					_gnapRandomValue = getRandom(2);
					if (_gnapRandomValue)
						_s11_nextGoggleGuySequenceId = 0x1F6;
					else
						_s11_nextGoggleGuySequenceId = 0x1F9;
				}
			}
			if (!_timers[4]) {
				_timers[4] = getRandom(40) + 20;
				if (_gnapActionStatus < 0 && _beaverActionStatus < 0 && _s11_nextHookGuySequenceId == -1) {
					if (_s11_currHookGuySequenceId == 0x201) {
						_gnapRandomValue = getRandom(7);
						switch (_gnapRandomValue) {
						case 0:
							_s11_nextHookGuySequenceId = 0x200;
							break;
						case 1:
							_s11_nextHookGuySequenceId = 0x205;
							break;
						case 2:
							_s11_nextHookGuySequenceId = 0x202;
							break;
						default:
							_s11_nextHookGuySequenceId = 0x201;
							break;
						}
					} else {
						_s11_nextHookGuySequenceId = 0x201;
					}
				}
			}
			if (!_timers[6]) {
				_timers[6] = getRandom(100) + 100;
				_gnapRandomValue = getRandom(3);
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
			scene11_updateHotspots();
			_timers[5] = getRandom(50) + 75;
			_timers[4] = getRandom(40) + 20;
		}
		
		gameUpdateTick();
	
	}
  
}

void GnapEngine::scene11_updateAnimations() {

	if (_gameSys->getAnimationStatus(0) == 2) {
		if (_gnapActionStatus != kASGrabBillardBall)
			_gameSys->setAnimation(0, 0, 0);
		switch (_gnapActionStatus) {
		case kASLeaveScene:
			_sceneDone = true;
			break;
		case kASShowMagazineToGoggleGuy:
			_s11_nextGoggleGuySequenceId = 0x1F7;
			break;
		case kASTalkGoggleGuy:
			_s11_nextGoggleGuySequenceId = 0x1FB;
			break;
		case kASGrabHookGuy:
			_s11_nextHookGuySequenceId = 0x204;
			break;
		case kASShowItemToHookGuy:
			_s11_nextHookGuySequenceId = 0x203;
			break;
		case kASTalkHookGuy:
			_s11_nextHookGuySequenceId = 0x206;
			break;
		case kASGrabBillardBall:
			if (_gameSys->getAnimationStatus(2) == 2 && _gameSys->getAnimationStatus(3) == 2) {
				_gameSys->setAnimation(0, 0, 0);
				_timers[2] = getRandom(30) + 20;
				_timers[3] = getRandom(50) + 200;
				_gameSys->insertSequence(0x1F4, 255, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
				_gnapSequenceId = 0x1F4;
				_gnapId = 255;
				_gnapSequenceDatNum = 0;
				_gameSys->removeSequence(0x207, 257, 1);
				_gameSys->removeSequence(0x208, 256, 1);
				_s11_nextGoggleGuySequenceId = 0x1F8;
				_timers[5] = getRandom(100) + 75;
				_gameSys->insertSequence(_s11_nextGoggleGuySequenceId, 121, _s11_currGoggleGuySequenceId, 121, kSeqSyncWait, 0, 0, 0);
				_gameSys->setAnimation(_s11_nextGoggleGuySequenceId, 121, 2);
				_s11_currGoggleGuySequenceId = _s11_nextGoggleGuySequenceId;
				_s11_nextGoggleGuySequenceId = -1;
				switch (_s11_billardBallCtr) {
				case 0:
					_s11_nextHookGuySequenceId = 0x1FC;
					break;
				case 1:
					_s11_nextHookGuySequenceId = 0x1FD;
					break;
				default:
					_s11_nextHookGuySequenceId = 0x1FE;
					break;
				}
				++_s11_billardBallCtr;
				_gameSys->insertSequence(_s11_nextHookGuySequenceId, 120, _s11_currHookGuySequenceId, 120, kSeqSyncWait, 0, 0, 0);
				_gameSys->setAnimation(_s11_nextHookGuySequenceId, 120, 3);
				_s11_currHookGuySequenceId = _s11_nextHookGuySequenceId;
				_s11_nextHookGuySequenceId = -1;
				_timers[4] = getRandom(40) + 20;
				_gameSys->insertSequence(0x208, 256, 0, 0, kSeqNone, getSequenceTotalDuration(0x1F4) - 5, 0, 0);
				_hotspots[kHSBillard].flags |= SF_DISABLED;
				_gameSys->setAnimation(0x207, 257, 4);
				_gameSys->insertSequence(0x207, 257, 0, 0, kSeqNone, getSequenceTotalDuration(0x1FE), 0, 0);
				_gnapActionStatus = -1;
			}
			break;
		}
	}
	
	if (_gameSys->getAnimationStatus(2) == 2 && _s11_nextGoggleGuySequenceId != -1) {
		_timers[5] = getRandom(100) + 75;
		_gameSys->insertSequence(_s11_nextGoggleGuySequenceId, 121, _s11_currGoggleGuySequenceId, 121, kSeqSyncWait, 0, 0, 0);
		_gameSys->setAnimation(_s11_nextGoggleGuySequenceId, 121, 2);
		_s11_currGoggleGuySequenceId = _s11_nextGoggleGuySequenceId;
		_s11_nextGoggleGuySequenceId = -1;
		if (_gnapActionStatus >= 1 && _gnapActionStatus <= 4)
			_gnapActionStatus = -1;
	}
	
	if (_gameSys->getAnimationStatus(3) == 2) {
		if (_s11_nextHookGuySequenceId == 0x204) {
			_gameSys->setAnimation(_s11_nextHookGuySequenceId, 120, 3);
			_gameSys->insertSequence(0x204, 120, _s11_currHookGuySequenceId, 120, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0x1F5, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_s11_currHookGuySequenceId = 0x204;
			_s11_nextHookGuySequenceId = -1;
			_gnapSequenceId = 0x1F5;
			_gnapSequenceDatNum = 0;
			_timers[4] = getRandom(40) + 20;
			_timers[2] = getRandom(20) + 70;
			_timers[3] = getRandom(50) + 200;
			if (_gnapActionStatus == kASGrabHookGuy)
				_gnapActionStatus = -1;
		} else if (_s11_nextHookGuySequenceId != -1) {
			_gameSys->insertSequence(_s11_nextHookGuySequenceId, 120, _s11_currHookGuySequenceId, 120, kSeqSyncWait, 0, 0, 0);
			_gameSys->setAnimation(_s11_nextHookGuySequenceId, 120, 3);
			_s11_currHookGuySequenceId = _s11_nextHookGuySequenceId;
			_s11_nextHookGuySequenceId = -1;
			_timers[4] = getRandom(40) + 20;
			if (_gnapActionStatus >= 6 && _gnapActionStatus <= 9)
				_gnapActionStatus = -1;
		}
	}
	
	if (_gameSys->getAnimationStatus(4) == 2) {
		_gameSys->setAnimation(0, 0, 4);
		_hotspots[kHSBillard].flags &= ~SF_DISABLED;
	}

}

} // End of namespace Gnap
