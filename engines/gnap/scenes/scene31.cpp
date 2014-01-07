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
	kHSMeasuringClown	= 1,
	kHSBeerBarrel		= 2,
	kHSDevice			= 3,
	kHSExitCircus		= 4,
	kHSExitOutsideClown	= 5,
	kHSWalkArea1		= 6
};

int GnapEngine::scene31_init() {
	return 0x105;
}

void GnapEngine::scene31_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSMeasuringClown, 34, 150, 256, 436, SF_WALKABLE | SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 0, 6);
	setHotspot(kHSBeerBarrel, 452, 182, 560, 306, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 7);
	setHotspot(kHSExitCircus, 150, 585, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
	setHotspot(kHSExitOutsideClown, 0, 0, 15, 600, SF_EXIT_L_CURSOR | SF_WALKABLE, 0, 8);
	setHotspot(kHSWalkArea1, 0, 0, 800, 480);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_hotspotsCount = 7;
}

void GnapEngine::scene31_run() {
	
	playSound(0x1093B, 1);
	startSoundTimerB(6);

	queueInsertDeviceIcon();

	_s31_beerGuyDistracted = false;
	_gameSys->insertSequence(0xFB, 39, 0, 0, kSeqNone, 0, 0, 0);

	_s31_dword_47EAA8 = 0xFB;
	_s31_dword_47EAAC = -1;

	_gameSys->setAnimation(0xFB, 39, 3);

	_timers[4] = getRandom(20) + 60;
	_timers[5] = getRandom(50) + 180;

	if (_prevSceneNum == 27) {
		initGnapPos(-1, 8, 3);
		initBeaverPos(-1, 9, 4);
		endSceneInit();
		gnapWalkTo(3, 8, -1, 0x107BA, 1);
		platypusWalkTo(3, 9, -1, 0x107D2, 1);
	} else {
		initGnapPos(7, 12, 1);
		initBeaverPos(6, 12, 0);
		endSceneInit();
		gnapWalkTo(7, 8, -1, 0x107BA, 1);
		platypusWalkTo(6, 8, -1, 0x107D2, 1);
	}

	while (!_sceneDone) {

		updateMouseCursor();
		updateCursorByHotspot();

		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);

		switch (_sceneClickedHotspot) {

		case kHSDevice:
			if (_gnapActionStatus < 0 || _gnapActionStatus == 4) {
				runMenu();
				scene31_updateHotspots();
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
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSMeasuringClown:
			if (_gnapActionStatus < 0 || _gnapActionStatus == 4) {
				if (_gnapActionStatus == 4) {
					if (_verbCursor == LOOK_CURSOR)
						playGnapScratchingHead(2, 2);
					else
						playGnapImpossible(0, 0);
				} else if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[kHSMeasuringClown].x, _hotspotsWalkPos[kHSMeasuringClown].y + 1, 2, 2);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(2, 2);
						break;
					case GRAB_CURSOR:
						gnapWalkTo(_hotspotsWalkPos[kHSMeasuringClown].x, _hotspotsWalkPos[kHSMeasuringClown].y + 1, -1, -1, 1);
						_hotspots[kHSWalkArea1].flags |= SF_WALKABLE;
						gnapWalkTo(_hotspotsWalkPos[kHSMeasuringClown].x, _hotspotsWalkPos[kHSMeasuringClown].y, 0, 0x107B9, 1);
						_hotspots[kHSWalkArea1].flags &= ~SF_WALKABLE;
						_gnapActionStatus = 5;
						_timers[4] = 300;
						break;
					case TALK_CURSOR:
						playGnapImpossible(0, 0);
						break;
					case PLAT_CURSOR:
						if (!invHas(kItemBucketWithBeer)) {
							gnapUseDeviceOnBeaver();
							platypusWalkTo(_hotspotsWalkPos[kHSMeasuringClown].x, _hotspotsWalkPos[kHSMeasuringClown].y + 1, 1, 0x107C2, 1);
							_hotspots[kHSWalkArea1].flags |= SF_WALKABLE;
							platypusWalkTo(_hotspotsWalkPos[kHSMeasuringClown].x, _hotspotsWalkPos[kHSMeasuringClown].y, 1, 0x107C2, 1);
							_hotspots[kHSWalkArea1].flags &= ~SF_WALKABLE;
							_beaverActionStatus = 4;
							_gnapActionStatus = 4;
							_timers[4] = 300;
						} else
							playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSBeerBarrel:
			if (_gnapActionStatus < 0 || _gnapActionStatus == 4) {
				if (_grabCursorSpriteIndex == kItemEmptyBucket && _s31_beerGuyDistracted) {
					setGrabCursorSprite(-1);
					gnapWalkTo(_gnapX, _gnapY, -1, getGnapSequenceId(gskIdle, _hotspotsWalkPos[kHSBeerBarrel].x, _hotspotsWalkPos[kHSBeerBarrel].y) | 0x10000, 1);
					_s31_dword_474940 += 5;
					_gameSys->insertSequence(0xF8, 59, 0, 0, kSeqNone, 0, 0, 0);
					playGnapPullOutDevice(6, 8);
					playGnapUseDevice(0, 0);
					gnapWalkTo(_hotspotsWalkPos[kHSBeerBarrel].x, _hotspotsWalkPos[kHSBeerBarrel].y, 0, 0x107BC, 1);
					_gnapActionStatus = 2;
					_timers[4] = 300;
				} else if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[kHSBeerBarrel].x, _hotspotsWalkPos[kHSBeerBarrel].y, 6, 2);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(6, 2);
						break;
					case GRAB_CURSOR:
						if (_s31_beerGuyDistracted) {
							playGnapScratchingHead(6, 2);
						} else {
							gnapWalkTo(_hotspotsWalkPos[kHSBeerBarrel].x, _hotspotsWalkPos[kHSBeerBarrel].y, 0, 0x107BC, 1);
							_gnapActionStatus = 1;
							_gnapIdleFacing = 5;
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

		case kHSExitCircus:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = 1;
				_newSceneNum = 26;
				gnapWalkTo(-1, _hotspotsWalkPos[kHSExitCircus].y, 0, 0x107AE, 1);
				_gnapActionStatus = 6;
				platypusWalkTo(_hotspotsWalkPos[kHSExitCircus].x + 1, _hotspotsWalkPos[kHSExitCircus].y, -1, -1, 1);
			}
			break;

		case kHSExitOutsideClown:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = 1;
				_newSceneNum = 27;
				gnapWalkTo(-1, _hotspotsWalkPos[kHSExitOutsideClown].y, 0, 0x107AF, 1);
				_gnapActionStatus = 6;
				platypusWalkTo(_hotspotsWalkPos[kHSExitOutsideClown].x, _hotspotsWalkPos[kHSExitOutsideClown].y + 1, -1, 0x107CF, 1);
			}
			break;

		case kHSWalkArea1:
			if (_gnapActionStatus < 0)
				gnapWalkTo(-1, -1, -1, -1, 1);
			break;

		default:
			if (_mouseClickState.left) {
				gnapWalkTo(-1, -1, -1, -1, 1);
				_mouseClickState.left = false;
			}
			break;
		}
	
		scene31_updateAnimations();
	
		if (!isSoundPlaying(0x1093B))
			playSound(0x1093B, 1);
	
		if (!_isLeavingScene) {
			if (_beaverActionStatus < 0)
				updateBeaverIdleSequence();
			if (_gnapActionStatus < 0)
				updateGnapIdleSequence();
			if (!_timers[4]) {
				_timers[4] = getRandom(20) + 60;
				if (_gnapActionStatus < 0 && _s31_dword_47EAAC == -1) {
					switch (getRandom(6)){
					case 0:
						_s31_dword_47EAAC = 0xFF;
						break;
					case 1:
						_s31_dword_47EAAC = 0x100;
						break;
					case 2:
						_s31_dword_47EAAC = 0x101;
						break;
					default:
						_s31_dword_47EAAC = 0xFB;
						break;
					}
				}
			}
			if (!_timers[5]) {
				_timers[5] = getRandom(50) + 180;
				if (_gnapActionStatus < 0) {
					if (getRandom(2) != 0)
						_s31_dword_47EAB4 = 0x104;
					else
						_s31_dword_47EAB4 = 0x103;
					_gameSys->insertSequence(_s31_dword_47EAB4, 20, 0, 0, kSeqNone, 0, 0, 0);
				}
			}
			playSoundB();
		}

		checkGameKeys();

		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene31_updateHotspots();
		}
		
		gameUpdateTick();

	}
	
}

void GnapEngine::scene31_updateAnimations() {
	
	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		switch (_gnapActionStatus) {
		case 1:
			_s31_dword_47EAAC = 0xFE;
			break;
		case 2:
			_gameSys->setAnimation(0x102, 59, 0);
			_gameSys->insertSequence(0x102, 59, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gnapX = 5;
			_gnapY = 7;
			_gnapSequenceDatNum = 0;
			_gnapSequenceId = 0x102;
			_gnapId = 59;
			_gnapActionStatus = 3;
			break;
		case 3:
			_gnapIdleFacing = 3;
			playGnapPullOutDevice(0, 0);
			playGnapUseDevice(0, 0);
			_gameSys->insertSequence(0xF9, 59, 0xF8, 59, kSeqSyncWait, 0, 0, 0);
			_gnapActionStatus = -1;
			invAdd(kItemBucketWithBeer);
			invRemove(kItemEmptyBucket);
			setGrabCursorSprite(kItemBucketWithBeer);
			break;
		case 5:
			_s31_dword_47EAAC = 0xFA;
			_s31_dword_474940 = 1;
			break;
		case 6:
			_sceneDone = true;
			_gnapActionStatus = -1;
			break;
		}
	}

	if (_gameSys->getAnimationStatus(1) == 2) {
		_gameSys->setAnimation(0, 0, 1);
		if (_beaverActionStatus == 4) {
			_sceneWaiting = 1;
			_s31_beerGuyDistracted = true;
			_s31_dword_47EAAC = 0xFA;
		}
	}

	if (_gameSys->getAnimationStatus(3) == 2) {
		switch (_s31_dword_47EAAC) {
		case 0xFA:
			_gameSys->insertSequence(_s31_dword_47EAAC, 39, _s31_dword_47EAA8, 39, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0xFC, 39, _s31_dword_47EAAC, 39, kSeqSyncWait, 0, 0, 0);
			_gameSys->setAnimation(0xFC, 39, 3);
			_s31_dword_47EAA8 = 0xFC;
			_s31_dword_47EAAC = 0xFC;
			_s31_dword_47EAB0 = 0;
			break;
		case 0xFC:
			++_s31_dword_47EAB0;
			if (_s31_dword_47EAB0 >= _s31_dword_474940) {
				if (_gnapActionStatus != 5)
					_beaverActionStatus = -1;
				_timers[0] = 40;
				_gameSys->insertSequence(0xFD, 39, _s31_dword_47EAA8, 39, kSeqSyncWait, 0, 0, 0);
				_s31_dword_47EAA8 = 0xFD;
				_s31_dword_47EAAC = -1;
				if (_gnapActionStatus != 3 && _gnapActionStatus != 2)
					_gnapActionStatus = -1;
				_s31_beerGuyDistracted = false;
				_s31_dword_474940 = 3;
				_gameSys->setAnimation(0xFD, 39, 3);
				_sceneWaiting = 0;
			} else {
				_gameSys->insertSequence(_s31_dword_47EAAC, 39, _s31_dword_47EAA8, 39, kSeqSyncWait, 0, 0, 0);
				_s31_dword_47EAA8 = _s31_dword_47EAAC;
				_s31_dword_47EAAC = 0xFC;
				_gameSys->setAnimation(0xFC, 39, 3);
			}
			break;
		case 0xFE:
			_gameSys->insertSequence(_s31_dword_47EAAC, 39, _s31_dword_47EAA8, 39, kSeqSyncWait, 0, 0, 0);
			_gameSys->setAnimation(_s31_dword_47EAAC, 39, 3);
			_s31_dword_47EAA8 = _s31_dword_47EAAC;
			_s31_dword_47EAAC = -1;
			_gnapActionStatus = -1;
			break;
		default:
			if (_s31_dword_47EAAC != -1) {
				_gameSys->insertSequence(_s31_dword_47EAAC, 39, _s31_dword_47EAA8, 39, kSeqSyncWait, 0, 0, 0);
				_gameSys->setAnimation(_s31_dword_47EAAC, 39, 3);
				_s31_dword_47EAA8 = _s31_dword_47EAAC;
				_s31_dword_47EAAC = -1;
			}
			break;
		}
	}
	
}

} // End of namespace Gnap
