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
	kHSExitUfoParty		= 1,
	kHSExitKissinBooth	= 2,
	kHSExitDisco		= 3,
	kHSSackGuy			= 4,
	kHSItchyGuy			= 5,
	kHSDevice			= 6,
	kHSWalkArea1		= 7
};

enum {
	kHSUfoExitLeft	= 1,
	kHSUfoExitRight	= 2,
	kHSUfoDevice	= 3
};

int GnapEngine::scene46_init() {
	_gameSys->setAnimation(0, 0, 0);
	_gameSys->setAnimation(0, 0, 1);
	_gameSys->setAnimation(0, 0, 2);
	_gameSys->setAnimation(0, 0, 3);
	_gameSys->setAnimation(0, 0, 4);
	return 0x4E;
}

void GnapEngine::scene46_updateHotspots() {
	if (isFlag(12)) {
		setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_DISABLED);
		setHotspot(kHSUfoExitLeft, 0, 0, 10, 599, SF_EXIT_L_CURSOR);
		setHotspot(kHSUfoExitRight, 790, 0, 799, 599, SF_EXIT_R_CURSOR);
		setDeviceHotspot(kHSUfoDevice, -1, 534, -1, 599);
		_hotspotsCount = 4;
	} else {
		setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		setHotspot(kHSExitUfoParty, 150, 580, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
		setHotspot(kHSExitKissinBooth, 0, 100, 10, 599, SF_EXIT_L_CURSOR, 0, 8);
		setHotspot(kHSExitDisco, 790, 100, 799, 599, SF_EXIT_R_CURSOR, 10, 8);
		setHotspot(kHSSackGuy, 180, 370, 235, 490, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 8);
		setHotspot(kHSItchyGuy, 535, 210, 650, 480, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 8);
		setHotspot(kHSWalkArea1, 0, 0, 800, 485);
		setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
		_hotspotsCount = 8;
	}
}

void GnapEngine::scene46_run() {

	queueInsertDeviceIcon();

	_gameSys->insertSequence(0x4D, 0, 0, 0, kSeqLoop, 0, 0, 0);
	
	_s46_currSackGuySequenceId = 0x4B;
	_s46_nextSackGuySequenceId = -1;
	_gameSys->setAnimation(0x4B, 1, 3);
	_gameSys->insertSequence(_s46_currSackGuySequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	_s46_currItchyGuySequenceId = 0x47;
	_s46_nextItchyGuySequenceId = -1;
	_gameSys->setAnimation(0x47, 1, 4);
	_gameSys->insertSequence(_s46_currItchyGuySequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	if (isFlag(12)) {
		_toyUfoId = 0;
		_toyUfoActionStatus = -1;
		_toyUfoSequenceId = toyUfoGetSequenceId();
		_toyUfoNextSequenceId = _toyUfoSequenceId;
		if (_prevSceneNum == 44)
			_toyUfoX = 30;
		else
			_toyUfoX = 770;
		_gameSys->setAnimation(_toyUfoSequenceId | 0x10000, _toyUfoId, 2);
		_gameSys->insertSequence(_toyUfoSequenceId | 0x10000, _toyUfoId, 0, 0, kSeqNone, 0, _toyUfoX - 274, _toyUfoY - 128);
		endSceneInit();
	} else if (_prevSceneNum == 44) {
		initGnapPos(-1, 8, 7);
		initBeaverPos(-1, 8, 5);
		endSceneInit();
		platypusWalkTo(1, 8, -1, 0x107C2, 1);
		gnapWalkTo(2, 8, -1, 0x107B9, 1);
	} else if (_prevSceneNum == 45) {
		initGnapPos(11, 8, 7);
		initBeaverPos(12, 8, 5);
		endSceneInit();
		gnapWalkTo(8, 8, -1, 0x107BA, 1);
		platypusWalkTo(9, 8, -1, 0x107D2, 1);
	} else {
		initGnapPos(5, 11, 7);
		initBeaverPos(6, 11, 5);
		endSceneInit();
		platypusWalkTo(5, 8, -1, 0x107C2, 1);
		gnapWalkTo(6, 8, -1, 0x107BA, 1);
	}

	_timers[4] = getRandom(50) + 80;
	_timers[5] = getRandom(50) + 80;

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

			case kHSUfoExitLeft:
				if (_toyUfoActionStatus < 0) {
					_isLeavingScene = 1;
					_toyUfoActionStatus = 3;
					_newSceneNum = 44;
					toyUfoFlyTo(-35, -1, -35, 799, 0, 300, 2);
				}
				break;

			case kHSUfoExitRight:
				if (_toyUfoActionStatus < 0) {
					_isLeavingScene = 1;
					_toyUfoActionStatus = 3;
					_newSceneNum = 45;
					toyUfoFlyTo(835, -1, 0, 835, 0, 300, 2);
				}
				break;

			case kHSUfoDevice:
				runMenu();
				scene46_updateHotspots();
				break;
			}
		} else {
			switch (_sceneClickedHotspot) {
			case kHSDevice:
				runMenu();
				scene46_updateHotspots();
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

			case kHSSackGuy:
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[kHSSackGuy].x, _hotspotsWalkPos[kHSSackGuy].y, 2, 0);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapMoan1(_hotspotsWalkPos[kHSSackGuy].x + 1, 0);
						break;
					case TALK_CURSOR:
						_gnapIdleFacing = 5;
						gnapWalkTo(_hotspotsWalkPos[kHSSackGuy].x, _hotspotsWalkPos[kHSSackGuy].y, 0, getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_gnapActionStatus = 2;
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
				break;

			case kHSItchyGuy:
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[kHSItchyGuy].x, _hotspotsWalkPos[kHSItchyGuy].y, 7, 0);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapMoan1(_hotspotsWalkPos[kHSItchyGuy].x - 1, 0);
						break;
					case TALK_CURSOR:
						_gnapIdleFacing = 7;
						gnapWalkTo(_hotspotsWalkPos[kHSItchyGuy].x, _hotspotsWalkPos[kHSItchyGuy].y, 0, getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_gnapActionStatus = 1;
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
				break;

			case kHSExitUfoParty:
				_isLeavingScene = 1;
				gnapWalkTo(_gnapX, _hotspotsWalkPos[kHSExitUfoParty].y, 0, 0x107AE, 1);
				_gnapActionStatus = 0;
				_newSceneNum = 40;
				break;

			case kHSExitKissinBooth:
				_isLeavingScene = 1;
				gnapWalkTo(_hotspotsWalkPos[kHSExitKissinBooth].x, _gnapY, 0, 0x107AF, 1);
				_gnapActionStatus = 0;
				platypusWalkTo(_hotspotsWalkPos[kHSExitKissinBooth].x, _platY, -1, 0x107CF, 1);
				_newSceneNum = 44;
				break;

			case kHSExitDisco:
				_isLeavingScene = 1;
				gnapWalkTo(_hotspotsWalkPos[kHSExitDisco].x, _gnapY, 0, 0x107AB, 1);
				_gnapActionStatus = 0;
				platypusWalkTo(_hotspotsWalkPos[kHSExitDisco].x, _platY, -1, 0x107CD, 1);
				_newSceneNum = 45;
				break;

			case kHSWalkArea1:
				if (_gnapActionStatus < 0)
					gnapWalkTo(-1, -1, -1, -1, 1);
					break;
			}
		}
	
		if (_mouseClickState._left && _gnapActionStatus < 0) {
			_mouseClickState._left = false;
			if (isFlag(12)) {
				_toyUfoActionStatus = 4;
				toyUfoFlyTo(-1, -1, 0, 799, 0, 300, 2);
			} else {
				gnapWalkTo(-1, -1, -1, -1, 1);
			}
		}
	
		scene46_updateAnimations();
		
		toyUfoCheckTimer();
	
		if (!_isLeavingScene) {
			if (_beaverActionStatus < 0 && !isFlag(12))
				updateBeaverIdleSequence();
			if (_gnapActionStatus < 0 && !isFlag(12))
				updateGnapIdleSequence();
			if (!_timers[4]) {
				_timers[4] = getRandom(50) + 80;
				if (_gnapActionStatus < 0 && _beaverActionStatus < 0 && _s46_nextItchyGuySequenceId == -1) {
					_gnapRandomValue = getRandom(2);
					if (_gnapRandomValue != 0)
						_s46_nextItchyGuySequenceId = 0x49;
					else
						_s46_nextItchyGuySequenceId = 0x48;
				}
			}
			if (!_timers[5]) {
				_timers[5] = getRandom(50) + 80;
				if (_gnapActionStatus < 0 && _beaverActionStatus < 0 && _s46_nextSackGuySequenceId == -1)
					_s46_nextSackGuySequenceId = 0x4C;
			}
		}
	
		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene46_updateHotspots();
		}
		
		gameUpdateTick();
	}
}

void GnapEngine::scene46_updateAnimations() {
	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		switch (_gnapActionStatus) {
		case 0:
			_sceneDone = true;
			break;
		case 1:
			_s46_nextItchyGuySequenceId = 0x46;
			break;
		case 2:
			_s46_nextSackGuySequenceId = 0x4A;
			break;
		}
		_gnapActionStatus = -1;
	}
	
	if (_gameSys->getAnimationStatus(3) == 2 && _s46_nextSackGuySequenceId != -1) {
		_gameSys->insertSequence(_s46_nextSackGuySequenceId, 1, _s46_currSackGuySequenceId, 1, kSeqSyncWait, 0, 0, 0);
		_gameSys->setAnimation(_s46_nextSackGuySequenceId, 1, 3);
		_s46_currSackGuySequenceId = _s46_nextSackGuySequenceId;
		_s46_nextSackGuySequenceId = -1;
		_timers[5] = getRandom(50) + 80;
	}
	
	if (_gameSys->getAnimationStatus(4) == 2 && _s46_nextItchyGuySequenceId != -1) {
		_gameSys->insertSequence(_s46_nextItchyGuySequenceId, 1, _s46_currItchyGuySequenceId, 1, kSeqSyncWait, 0, 0, 0);
		_gameSys->setAnimation(_s46_nextItchyGuySequenceId, 1, 4);
		_s46_currItchyGuySequenceId = _s46_nextItchyGuySequenceId;
		_s46_nextItchyGuySequenceId = -1;
		_timers[4] = getRandom(50) + 80;
	}
	
	if (_gameSys->getAnimationStatus(2) == 2) {
		switch (_toyUfoActionStatus) {
		case 3:
			_sceneDone = true;
			break;
		default:
			_toyUfoNextSequenceId = toyUfoGetSequenceId();
			_gameSys->insertSequence(_toyUfoNextSequenceId | 0x10000, _toyUfoId + 1,
				_toyUfoSequenceId | 0x10000, _toyUfoId,
				kSeqSyncWait, 0, _toyUfoX - 274, _toyUfoY - 128);
			_toyUfoSequenceId = _toyUfoNextSequenceId;
			++_toyUfoId;
			_gameSys->setAnimation(_toyUfoNextSequenceId | 0x10000, _toyUfoId, 2);
			break;
		}
		_toyUfoActionStatus = -1;
	}
}

} // End of namespace Gnap
