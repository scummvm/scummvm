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
	kHSGrass			= 1,
	kHSExitTruck		= 2,
	kHSCreek			= 3,
	kHSTrappedPlatypus	= 4,
	kHSDevice			= 5,
	kHSWalkAreas1		= 6,
	kHSWalkAreas2		= 7,
	kHSPlatypusWalkArea	= 8,
	kHSWalkAreas3		= 9
};

enum {
	kASLeaveScene			= 0,
	kASFreePlatypus			= 1,
	kASHypnotizePlat		= 2,
	kASHypnotizeScaredPlat	= 3,
	kASFreePlatypusDone		= 4,
	kASGrabPlatypus			= 5,
	kASGrabCreek			= 6,
	kASGrabCreekDone		= 7,
	kASGrabScaredPlatypus	= 8
};

int GnapEngine::scene03_init() {
	_gameSys->setAnimation(0, 0, 0);
	_gameSys->setAnimation(0, 0, 1);
	_gameSys->setAnimation(0, 0, 5);
	return 0x1CC;
}

void GnapEngine::scene03_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_DISABLED | SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSGrass, 646, 408, 722, 458, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 9, 6);
	setHotspot(kHSExitTruck, 218, 64, 371, 224, SF_EXIT_U_CURSOR | SF_WALKABLE, 4, 4);
	setHotspot(kHSCreek, 187, 499, 319, 587, SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 7);
	setHotspot(kHSTrappedPlatypus, 450, 256, 661, 414, SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 2, 5);
	setHotspot(kHSWalkAreas1, 0, 500, 300, 600);
	setHotspot(kHSWalkAreas2, 300, 447, 800, 600);
	setHotspot(kHSPlatypusWalkArea, 235, 0, 800, 600);
	setHotspot(kHSWalkAreas3, 0, 0, 800, 354);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	if (isFlag(0))
		_hotspots[kHSPlatypus]._flags = SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	if (isFlag(6))
		_hotspots[kHSGrass]._flags = SF_WALKABLE | SF_DISABLED;
	if (isFlag(0))
		_hotspots[kHSTrappedPlatypus]._flags = SF_DISABLED;
	if (isFlag(0) || _s03_platypusHypnotized)
		_hotspots[kHSPlatypusWalkArea]._flags |= SF_WALKABLE;
	_hotspotsCount = 10;
}

void GnapEngine::scene03_run() {

	playSound(0x10925, 1);
	
	startSoundTimerC(7);
	
	_gameSys->insertSequence(0x1CA, 251, 0, 0, kSeqLoop, 0, 0, 0);
	_gameSys->insertSequence(0x1CB, 251, 0, 0, kSeqLoop, 0, 0, 0);
	
	_s03_platypusHypnotized = false;
	
	initGnapPos(3, 4, 1);
	
	_gameSys->insertSequence(0x1C6, 253, 0, 0, kSeqNone, 0, 0, 0);
	
	_s03_currFrogSequenceId = 0x1C6;
	_s03_nextFrogSequenceId = -1;
	_gameSys->setAnimation(0x1C6, 253, 2);
	
	_timers[6] = getRandom(20) + 30;
	_timers[4] = getRandom(100) + 300;
	_timers[5] = getRandom(100) + 200;
	
	if (isFlag(0)) {
		initBeaverPos(5, 4, 0);
	} else {
		_timers[1] = getRandom(40) + 20;
		_gameSys->setAnimation(0x1C2, 99, 1);
		_gameSys->insertSequence(0x1C2, 99, 0, 0, kSeqNone, 0, 0, 0);
		_beaverSequenceId = 0x1C2;
		_beaverSequenceDatNum = 0;
	}
	
	_gameSys->insertSequence(0x1C4, 255, 0, 0, kSeqNone, 0, 0, 0);
	
	if (!isFlag(6))
		_gameSys->insertSequence(0x1B2, 253, 0, 0, kSeqNone, 0, 0, 0);
	
	queueInsertDeviceIcon();
	
	endSceneInit();
	
	if (isFlag(0))
		platypusWalkTo(4, 7, -1, 0x107C2, 1);
	gnapWalkTo(3, 6, -1, 0x107B9, 1);
	
	while (!_sceneDone) {
	
		updateMouseCursor();
		updateCursorByHotspot();
	
		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);
	
		switch (_sceneClickedHotspot) {

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

		case kHSGrass:
			if (_gnapActionStatus < 0) {
				if (isFlag(6)) {
					gnapWalkTo(-1, -1, -1, -1, 1);
				} else if (_grabCursorSpriteIndex >= 0) {
					playGnapShowItem(_grabCursorSpriteIndex, 9, 6);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(9, 6);
						break;
					case GRAB_CURSOR:
						playGnapPullOutDevice(9, 6);
						playGnapUseDevice(0, 0);
						_gameSys->insertSequence(0x1B3, 253, 0x1B2, 253, kSeqSyncWait, 0, 0, 0);
						_gameSys->setAnimation(0x1B3, 253, 5);
						_hotspots[kHSGrass]._flags |= SF_WALKABLE | SF_DISABLED;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSExitTruck:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = 1;
				_hotspots[kHSPlatypusWalkArea]._flags |= SF_WALKABLE;
				gnapWalkTo(_hotspotsWalkPos[2].x, _hotspotsWalkPos[2].y, 0, 0x107AD, 1);
				_gnapActionStatus = kASLeaveScene;
				if (isFlag(0))
					platypusWalkTo(_hotspotsWalkPos[2].x, _hotspotsWalkPos[2].y, -1, 0x107C2, 1);
				_hotspots[kHSPlatypusWalkArea]._flags &= ~SF_WALKABLE;
				if (_cursorValue == 1)
					_newSceneNum = 2;
				else
					_newSceneNum = 33;
			}
			break;

		case kHSCreek:
			if (_gnapActionStatus == -1) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapImpossible(0, 0);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapMoan2(2, 8);
						break;
					case GRAB_CURSOR:
						if (!isFlag(0))
							_hotspots[kHSPlatypusWalkArea]._flags |= SF_WALKABLE;
						if (gnapWalkTo(_hotspotsWalkPos[3].x, _hotspotsWalkPos[3].y, 0, getGnapSequenceId(gskIdle, _hotspotsWalkPos[3].x + 1, _hotspotsWalkPos[3].y + 1) | 0x10000, 1))
							_gnapActionStatus = kASGrabCreek;
						if (!isFlag(0))
							_hotspots[kHSPlatypusWalkArea]._flags &= ~SF_WALKABLE;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSTrappedPlatypus:
			if (_gnapActionStatus < 0) {
				if (isFlag(0)) {
					gnapWalkTo(-1, -1, -1, -1, 1);
				} else if (_grabCursorSpriteIndex >= 0) {
					playGnapShowItem(_grabCursorSpriteIndex, 8, 4);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(8, 4);
						break;
					case GRAB_CURSOR:
						if (_s03_platypusHypnotized) {
							gnapWalkTo(7, 6, 0, 0x107B5, 1);
							_gnapActionStatus = kASFreePlatypus;
						} else {
							gnapWalkTo(_hotspotsWalkPos[4].x, _hotspotsWalkPos[4].y, 0, getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
							if (_s03_platypusScared)
								_gnapActionStatus = kASGrabScaredPlatypus;
							else
								_gnapActionStatus = kASGrabPlatypus;
						}
						break;
					case TALK_CURSOR:
						if (_s03_platypusHypnotized) {
							playGnapBrainPulsating(8, 4);
						} else {
							_gnapIdleFacing = 1;
							gnapWalkTo(_hotspotsWalkPos[4].x, _hotspotsWalkPos[4].y, 0, getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
							if (_s03_platypusScared)
								_gnapActionStatus = kASHypnotizeScaredPlat;
							else
								_gnapActionStatus = kASHypnotizePlat;
						}
						break;
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSDevice:
			if (_gnapActionStatus < 0) {
				runMenu();
				scene03_updateHotspots();
			}
			break;

		case kHSWalkAreas1:
		case kHSWalkAreas2:
		case kHSWalkAreas3:
			if (_gnapActionStatus < 0)
				gnapWalkTo(-1, -1, -1, -1, 1);
			break;

		case kHSPlatypusWalkArea:
			if (_gnapActionStatus < 0) {
				if (isFlag(0) || _s03_platypusHypnotized) {
					gnapWalkTo(-1, -1, -1, -1, 1);
				} else {
					gnapWalkTo(_hotspotsWalkPos[4].x, _hotspotsWalkPos[4].y, 0, 0x107B5, 1);
					if (_s03_platypusScared)
						_gnapActionStatus = kASGrabScaredPlatypus;
					else
						_gnapActionStatus = kASGrabPlatypus;
				}
			}
			break;
			
		default:
			if (_mouseClickState._left && _gnapActionStatus < 0) {
				gnapWalkTo(-1, -1, -1, -1, 1);
				_mouseClickState._left = false;
			}
			break;
			
		}

		scene03_updateAnimations();
	
		if (!isSoundPlaying(0x10925))
			playSound(0x10925, 1);
	
		if (!_isLeavingScene) {
			if (_beaverActionStatus < 0 && isFlag(0))
				updateBeaverIdleSequence();
			if (_gnapActionStatus < 0)
				updateGnapIdleSequence();
			if (!_timers[1] && !_s03_platypusScared) {
				_timers[1] = getRandom(40) + 20;
				if (_gnapActionStatus < 0 && _beaverActionStatus < 0 && !isFlag(0) && !_s03_platypusHypnotized)
					_s03_nextPlatSequenceId = 450;
			}
			if (!_timers[6]) {
				_timers[6] = getRandom(20) + 30;
				if (_gnapActionStatus < 0 && _beaverActionStatus < 0 && _s03_nextFrogSequenceId == -1) {
					if (getRandom(5) == 1)
						_s03_nextFrogSequenceId = 0x1C6;
					else
						_s03_nextFrogSequenceId = 0x1C7;
				}
			}
			if (!_timers[4]) {
				// Update bird animation
				_timers[4] = getRandom(100) + 300;
				if (_gnapActionStatus < 0 && _beaverActionStatus < 0)
					_gameSys->insertSequence(getRandom(2) != 0 ? 0x1C8 : 0x1C3, 253, 0, 0, kSeqNone, 0, 0, 0);
			}
			if (!_timers[5]) {
				_timers[5] = getRandom(100) + 200;
				if (_gnapActionStatus < 0 && _beaverActionStatus < 0) {
					_gameSys->setAnimation(0x1C5, 253, 4);
					_gameSys->insertSequence(0x1C5, 253, 0, 0, kSeqNone, 0, 0, 0);
				}
			}
			playSoundC();
		}
	
		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene03_updateHotspots();
			_timers[5] = getRandom(100) + 200;
			_timers[4] = getRandom(100) + 300;
			_timers[6] = getRandom(20) + 30;
		}
		
		gameUpdateTick();
	
	}

}

void GnapEngine::scene03_updateAnimations() {

	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		switch (_gnapActionStatus) {
		case kASLeaveScene:
			_sceneDone = true;
			break;
		case kASFreePlatypus:
			_s03_nextPlatSequenceId = 0x1BC;
			break;
		case kASFreePlatypusDone:
			_gnapActionStatus = -1;
			_platX = 6;
			_platY = 6;
			_beaverFacing = 4;
			_beaverId = 120;
			_gameSys->insertSequence(0x107CA, _beaverId, 0x1BC, 99,
				kSeqSyncWait, 0, 75 * _platX - _platGridX, 48 * _platY - _platGridY);
			_gameSys->insertSequence(0x1B7, 99, 0, 0, kSeqNone, 0, 0, 0);
			_beaverSequenceDatNum = 1;
			_beaverSequenceId = 0x7CA;
			setFlag(0);
			_s03_nextPlatSequenceId = -1;
			scene03_updateHotspots();
			break;
		case kASHypnotizePlat:
			playGnapBrainPulsating(0, 0);
			addFullScreenSprite(0x106, 255);
			_gameSys->setAnimation(0x1C9, 256, 1);
			_gameSys->insertSequence(0x1C9, 256, 0, 0, kSeqNone, 0, 0, 0);
			while (_gameSys->getAnimationStatus(1) != 2)
				gameUpdateTick();
			removeFullScreenSprite();
			_gameSys->setAnimation(0x1BA, 99, 1);
			_gameSys->insertSequence(0x1BA, 99, _beaverSequenceId | (_beaverSequenceDatNum << 16), 99, 32, 0, 0, 0);
			_beaverSequenceDatNum = 0;
			_beaverSequenceId = 0x1BA;
			_gnapActionStatus = -1;
			_s03_platypusHypnotized = true;
			scene03_updateHotspots();
			break;
		case kASHypnotizeScaredPlat:
			playGnapBrainPulsating(0, 0);
			_gameSys->insertSequence(0x1BF, 99, _beaverSequenceId | (_beaverSequenceDatNum << 16), 99, 32, 0, 0, 0);
			_gameSys->setAnimation(0x1BF, 99, 1);
			while (_gameSys->getAnimationStatus(1) != 2)
				gameUpdateTick();
			addFullScreenSprite(0x106, 255);
			_gameSys->setAnimation(0x1C9, 256, 1);
			_gameSys->insertSequence(0x1C9, 256, 0, 0, kSeqNone, 0, 0, 0);
			while (_gameSys->getAnimationStatus(1) != 2)
				gameUpdateTick();
			removeFullScreenSprite();
			_gameSys->setAnimation(0x1BA, 99, 1);
			_gameSys->insertSequence(0x1BA, 99, 447, 99, kSeqSyncWait, 0, 0, 0);
			_beaverSequenceDatNum = 0;
			_beaverSequenceId = 0x1BA;
			_gnapActionStatus = -1;
			_s03_platypusHypnotized = true;
			scene03_updateHotspots();
			break;
		case kASGrabPlatypus:
			_s03_nextPlatSequenceId = 0x1BD;
			_s03_platypusHypnotized = false;
			break;
		case kASGrabScaredPlatypus:
			_s03_nextPlatSequenceId = 0x1C0;
			_s03_platypusHypnotized = false;
			break;
		case kASGrabCreek:
			_gameSys->insertSequence(0x1B4, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gameSys->setAnimation(0x1B4, _gnapId, 0);
			_gnapSequenceId = 0x1B4;
			_gnapSequenceDatNum = 0;
			_gnapActionStatus = kASGrabCreekDone;
			break;
		default:
			_gnapActionStatus = -1;
			break;
		}
	}
	
	if (_gameSys->getAnimationStatus(1) == 2) {
		if (_s03_nextPlatSequenceId == 0x1BD || _s03_nextPlatSequenceId == 0x1C0) {
			_gameSys->setAnimation(0, 0, 1);
			_s03_platypusScared = true;
			_gameSys->insertSequence(0x1B5, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(_s03_nextPlatSequenceId, 99, _beaverSequenceId | (_beaverSequenceDatNum << 16), 99, kSeqSyncWait, 0, 0, 0);
			_gnapSequenceId = 0x1B5;
			_gnapSequenceDatNum = 0;
			_gnapIdleFacing = 0;
			_beaverSequenceId = _s03_nextPlatSequenceId;
			_beaverSequenceDatNum = 0;
			_gameSys->setAnimation(_s03_nextPlatSequenceId, 99, 1);
			_s03_nextPlatSequenceId = -1;
			_gnapActionStatus = -1;
		} else if (_s03_nextPlatSequenceId == 0x1BC) {
			_gnapX = 3;
			_gnapY = 6;
			_gameSys->insertSequence(0x1B6, 120, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0x1BC, 99, _beaverSequenceId | (_beaverSequenceDatNum << 16), 99, kSeqSyncWait, 0, 0, 0);
			_gameSys->setAnimation(0x1BC, 99, 0);
			_gnapId = 20 * _gnapY;
			_gnapSequenceId = 0x1B6;
			_gnapSequenceDatNum = 0;
			_gnapIdleFacing = 0;
			_gnapActionStatus = kASFreePlatypusDone;
			_s03_nextPlatSequenceId = -1;
		} else if (_s03_nextPlatSequenceId == 0x1C2 && !_s03_platypusScared) {
			_gameSys->setAnimation(0, 0, 1);
			_gameSys->insertSequence(0x1C2, 99, _beaverSequenceId | (_beaverSequenceDatNum << 16), 99, kSeqSyncWait, 0, 0, 0);
			_beaverSequenceId = 0x1C2;
			_beaverSequenceDatNum = 0;
			_gameSys->setAnimation(0x1C2, 99, 1);
			_s03_nextPlatSequenceId = -1;
		} else if (_s03_nextPlatSequenceId == -1 && _s03_platypusScared && !_s03_platypusHypnotized) {
			_gameSys->setAnimation(0, 0, 1);
			_gameSys->setAnimation(0x1BE, 99, 1);
			_gameSys->insertSequence(0x1BE, 99, _beaverSequenceId | (_beaverSequenceDatNum << 16), 99, kSeqSyncWait, 0, 0, 0);
			_beaverSequenceId = 0x1BE;
			_beaverSequenceDatNum = 0;
			_s03_nextPlatSequenceId = -1;
		}
	}
	
	if (_gameSys->getAnimationStatus(2) == 2 && _s03_nextFrogSequenceId != -1) {
		_gameSys->setAnimation(_s03_nextFrogSequenceId, 253, 2);
		_gameSys->insertSequence(_s03_nextFrogSequenceId, 253, _s03_currFrogSequenceId, 253, kSeqSyncWait, 0, 0, 0);
		_s03_currFrogSequenceId = _s03_nextFrogSequenceId;
		_s03_nextFrogSequenceId = -1;
	}
	
	if (_gameSys->getAnimationStatus(5) == 2) {
		_gameSys->setAnimation(0, 0, 5);
		invAdd(kItemGrass);
		setGrabCursorSprite(kItemGrass);
		setFlag(6);
		scene03_updateHotspots();
	}

}

} // End of namespace Gnap
