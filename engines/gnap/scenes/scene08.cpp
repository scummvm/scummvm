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
	kHSExitBackdoor	= 1,
	kHSExitCrash	= 2,
	kHSMan			= 3,
	kHSDoor			= 4,
	kHSMeat			= 5,
	kHSBone			= 6,
	kHSToy			= 7,
	kHSWalkArea1	= 8,
	kHSDevice		= 9,
	kHSWalkArea2	= 10
};

enum {
	kASLeaveScene		= 0,
	kASTalkMan			= 1,
	kASLookMan			= 2,
	kASLookDog			= 3,
	kASGrabDog			= 4,
	kASTalkDog			= 5,
	kASPlatWithMan		= 6,
	kASPlatWithDog		= 7
};

int GnapEngine::scene08_init() {
	return 0x150;
}

void GnapEngine::scene08_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSExitBackdoor, 0, 280, 10, 400, SF_EXIT_L_CURSOR | SF_WALKABLE);
	setHotspot(kHSExitCrash, 200, 590, 400, 599, SF_EXIT_D_CURSOR | SF_WALKABLE);
	setHotspot(kHSMan, 510, 150, 610, 380, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSDoor, 350, 170, 500, 410, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSMeat, 405, 450, 480, 485, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSBone, 200, 405, 270, 465, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSToy, 540, 430, 615, 465, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSWalkArea1, 290, 340, -1, -1);
	setHotspot(kHSWalkArea2, 0, 0, 799, 420);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	if (isFlag(kGFBarnPadlockOpen))
		_hotspots[kHSMeat]._flags = SF_WALKABLE | SF_DISABLED;
	if (isFlag(kGFTruckFilledWithGas))
		_hotspots[kHSBone]._flags = SF_WALKABLE | SF_DISABLED;
	if (isFlag(kGFTruckKeysUsed))
		_hotspots[kHSToy]._flags = SF_WALKABLE | SF_DISABLED;
	_hotspotsCount = 11;
}

void GnapEngine::scene08_updateAnimationsCb() {
	if (_gameSys->getAnimationStatus(3) == 2) {
		_gameSys->setAnimation(_s08_nextDogSequenceId, 100, 3);
		_gameSys->insertSequence(_s08_nextDogSequenceId, 100, _s08_currDogSequenceId, 100, kSeqSyncWait, 0, 0, 0);
		_s08_currDogSequenceId = _s08_nextDogSequenceId;
		if ( _s08_nextDogSequenceId != 0x135 )
			_s08_nextDogSequenceId = 0x134;
	}
}

void GnapEngine::scene08_run() {
	
	queueInsertDeviceIcon();

	_gameSys->insertSequence(0x14F, 1, 0, 0, kSeqLoop, 0, 0, 0);
	_gameSys->insertSequence(0x14E, 256, 0, 0, kSeqNone, 0, 0, 0);
	
	_s08_currDogSequenceId = 0x135;
	_s08_nextDogSequenceId = 0x135;
	
	_gameSys->setAnimation(0x135, 100, 3);
	_gameSys->insertSequence(_s08_currDogSequenceId, 100, 0, 0, kSeqNone, 0, 0, 0);
	
	_s08_currManSequenceId = 0x140;
	_s08_nextManSequenceId = -1;
	
	_gameSys->setAnimation(0x140, 100, 2);
	_gameSys->insertSequence(_s08_currManSequenceId, 100, 0, 0, kSeqNone, 0, 0, 0);
	
	_timers[4] = getRandom(50) + 75;
	
	if (!isFlag(kGFBarnPadlockOpen))
		_gameSys->insertSequence(0x144, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	if (!isFlag(kGFTruckFilledWithGas))
		_gameSys->insertSequence(0x145, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	if (!isFlag(kGFTruckKeysUsed))
		_gameSys->insertSequence(0x146, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	initGnapPos(-1, 8, kDirBottomRight);
	initPlatypusPos(-1, 7, kDirNone);
	
	endSceneInit();
	
	gnapWalkTo(1, 8, -1, 0x107B9, 1);
	platypusWalkTo(1, 7, -1, 0x107C2, 1);
	
	_timers[5] = getRandom(40) + 50;
	
	while (!_sceneDone) {
	
		if (!isSoundPlaying(0x10919))
			playSound(0x10919, true);
	
		testWalk(0, 0, -1, -1, -1, -1);
		
		updateMouseCursor();
		updateCursorByHotspot();

		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);
	
		switch (_sceneClickedHotspot) {

		case kHSDevice:
			if (_gnapActionStatus < 0) {
				runMenu();
				scene08_updateHotspots();
				_timers[4] = getRandom(50) + 75;
				_timers[5] = getRandom(40) + 50;
			}
			break;

		case kHSPlatypus:
			if (_grabCursorSpriteIndex >= 0) {
				playGnapImpossible(0, 0);
			} else {
				switch (_verbCursor) {
				case LOOK_CURSOR:
					if (isFlag(kGFSceneFlag1))
						playGnapMoan1(_platX, _platY);
					else
						playGnapScratchingHead(_platX, _platY);
					break;
				case GRAB_CURSOR:
					gnapActionIdle(0x14D);
					gnapKissPlatypus(8);
					break;
				case TALK_CURSOR:
					playGnapBrainPulsating(_platX, _platY);
					playPlatypusSequence(getPlatypusSequenceId());
					break;
				case PLAT_CURSOR:
					break;
				}
			}
			break;

		case kHSExitBackdoor:
			_isLeavingScene = 1;
			gnapActionIdle(0x14D);
			gnapWalkTo(0, 6, 0, 0x107AF, 1);
			_gnapActionStatus = kASLeaveScene;
			platypusWalkTo(0, 7, 1, 0x107CF, 1);
			_newSceneNum = 9;
			break;

		case kHSExitCrash:
			_isLeavingScene = 1;
			gnapActionIdle(0x14D);
			gnapWalkTo(3, 9, 0, 0x107AE, 1);
			_gnapActionStatus = kASLeaveScene;
			platypusWalkTo(4, 9, 1, 0x107C1, 1);
			_newSceneNum = 7;
			break;

		case kHSMan:
			if (_grabCursorSpriteIndex >= 0) {
				playGnapShowCurrItem(6, 6, 7, 0);
			} else {
				switch (_verbCursor) {
				case LOOK_CURSOR:
					gnapActionIdle(0x14D);
					gnapWalkTo(6, 6, 0, 0x107BB, 1);
					_gnapActionStatus = kASLookMan;
					_gnapIdleFacing = kDirUpRight;
					break;
				case GRAB_CURSOR:
					playGnapImpossible(0, 0);
					break;
				case TALK_CURSOR:
					_gnapIdleFacing = kDirUpLeft;
					gnapActionIdle(0x14D);
					gnapWalkTo(8, 6, 0, getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
					_gnapActionStatus = kASTalkMan;
					break;
				case PLAT_CURSOR:
					gnapActionIdle(0x14D);
					gnapUseDeviceOnPlatypuss();
					platypusWalkTo(6, 6, 1, 0x107C2, 1);
					_platypusActionStatus = kASPlatWithMan;
					_platypusFacing = kDirNone;
					playGnapIdle(6, 6);
					break;
				}
			}
			break;

		case kHSDoor:
			if (_grabCursorSpriteIndex >= 0) {
				playGnapShowCurrItem(4, 7, 5, 0);
				_gameSys->setAnimation(makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, 0);
				_gnapActionStatus = kASGrabDog;
			} else {
				switch (_verbCursor) {
				case LOOK_CURSOR:
					playGnapScratchingHead(6, 0);
					_gameSys->setAnimation(makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, 0);
					_gnapActionStatus = kASLookDog;
					break;
				case GRAB_CURSOR:
					gnapWalkTo(4, 7, 0, 0x107BB, 1);
					_gnapActionStatus = kASGrabDog;
					_gnapIdleFacing = kDirUpRight;
					break;
				case TALK_CURSOR:
					_gnapIdleFacing = kDirUpRight;
					gnapActionIdle(0x14D);
					gnapWalkTo(4, 7, 0, getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
					_gnapActionStatus = kASTalkDog;
					break;
				case PLAT_CURSOR:
					setFlag(kGFSceneFlag1);
					gnapActionIdle(0x14D);
					gnapUseDeviceOnPlatypuss();
					platypusWalkTo(3, 7, 1, 0x107C2, 1);
					_platypusActionStatus = kASPlatWithDog;
					_platypusFacing = kDirNone;
					playGnapIdle(3, 7);
					break;
				}
			}
			break;

		case kHSMeat:
			if (_grabCursorSpriteIndex >= 0) {
				playGnapShowCurrItem(6, 8, 5, 6);
			} else {
				switch (_verbCursor) {
				case LOOK_CURSOR:
					playGnapScratchingHead(6, 7);
					break;
				case GRAB_CURSOR:
					if (_s08_currDogSequenceId == 0x135) {
						playGnapScratchingHead(6, 7);
					} else {
						gnapActionIdle(0x14D);
						playGnapPullOutDevice(6, 7);
						playGnapUseDevice(0, 0);
						_s08_nextDogSequenceId = 0x149;
					}
					break;
				case TALK_CURSOR:
				case PLAT_CURSOR:
					playGnapImpossible(0, 0);
					break;
				}
			}
			break;

		case kHSBone:
			if (_grabCursorSpriteIndex >= 0) {
				playGnapShowCurrItem(2, 7, 3, 6);
			} else {
				switch (_verbCursor) {
				case LOOK_CURSOR:
					playGnapScratchingHead(3, 6);
					break;
				case GRAB_CURSOR:
					if (_s08_currDogSequenceId == 0x135) {
						playGnapScratchingHead(3, 6);
					} else {
						gnapActionIdle(0x14D);
						playGnapPullOutDevice(3, 6);
						playGnapUseDevice(0, 0);
						_s08_nextDogSequenceId = 0x14A;
					}
					break;
				case TALK_CURSOR:
				case PLAT_CURSOR:
					playGnapImpossible(0, 0);
					break;
				}
			}
			break;

		case kHSToy:
			if (_grabCursorSpriteIndex >= 0) {
				playGnapShowCurrItem(8, 7, 7, 6);
			} else {
				switch (_verbCursor) {
				case LOOK_CURSOR:
					playGnapScratchingHead(7, 6);
					break;
				case GRAB_CURSOR:
					if (_s08_currDogSequenceId == 0x135) {
						playGnapScratchingHead(7, 6);
					} else {
						gnapActionIdle(0x14D);
						playGnapPullOutDevice(7, 6);
						playGnapUseDevice(0, 0);
						_s08_nextDogSequenceId = 0x14B;
					}
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
			gnapActionIdle(0x14D);
			gnapWalkTo(-1, 6, -1, -1, 1);
			break;

		default:
			if (_mouseClickState._left) {
				gnapActionIdle(0x14D);
				gnapWalkTo(-1, -1, -1, -1, 1);
				_mouseClickState._left = false;
			}
			break;

		}
	
		scene08_updateAnimations();
	
		if (!_isLeavingScene) {
			updatePlatypusIdleSequence();
			updateGnapIdleSequence();
			if (!_timers[4]) {
				_timers[4] = getRandom(50) + 125;
				if (_gnapActionStatus < 0 && _platypusActionStatus < 0 && _s08_nextManSequenceId == -1 &&
					(_s08_currDogSequenceId == 0x134 || _s08_currDogSequenceId == 0x135)) {
					int _gnapRandomValue = getRandom(4);
					switch (_gnapRandomValue) {
					case 0:
						_s08_nextManSequenceId = 0x138;
						break;
					case 1:
						_s08_nextManSequenceId = 0x136;
						break;
					case 2:
						_s08_nextManSequenceId = 0x13B;
						break;
					case 3:
						_s08_nextManSequenceId = 0x13A;
						break;
					}
				}
			}
			sceneXX_playRandomSound(5);
		}
	
		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene08_updateHotspots();
			_timers[4] = getRandom(50) + 75;
			_timers[5] = getRandom(40) + 50;
		}
		
		gameUpdateTick();
	}
}

void GnapEngine::scene08_updateAnimations() {

	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		switch (_gnapActionStatus) {
		case kASLeaveScene:
			_sceneDone = true;
			_gnapActionStatus = -1;
			break;
		case kASTalkMan:
			_s08_nextManSequenceId = 0x13F;
			_gnapActionStatus = -1;
			break;
		case kASLookMan:
			_s08_nextManSequenceId = 0x140;
			_gnapActionStatus = -1;
			break;
		case kASLookDog:
			_s08_nextManSequenceId = 0x137;
			_gnapActionStatus = -1;
			break;
		case kASGrabDog:
			if (_s08_currDogSequenceId == 0x135)
				_s08_nextDogSequenceId = 0x133;
			else
				_s08_nextDogSequenceId = 0x13C;
			_gnapActionStatus = -1;
			break;
		case kASTalkDog:
			if (_s08_currDogSequenceId == 0x135)
				_s08_nextDogSequenceId = 0x133;
			else
				_s08_nextDogSequenceId = 0x13C;
			_gnapActionStatus = -1;
			break;
		}
	}
	
	if (_gameSys->getAnimationStatus(1) == 2) {
		_gameSys->setAnimation(0, 0, 1);
		switch (_platypusActionStatus) {
		case kASPlatWithDog:
			_s08_nextDogSequenceId = 0x147;
			break;
		case kASPlatWithMan:
			_s08_nextManSequenceId = 0x140;
			_platypusActionStatus = -1;
			break;
		}
	}
	
	if (_gameSys->getAnimationStatus(2) == 2 && _s08_nextManSequenceId != -1) {
		_gameSys->setAnimation(_s08_nextManSequenceId, 100, 2);
		_gameSys->insertSequence(_s08_nextManSequenceId, 100, _s08_currManSequenceId, 100, kSeqSyncWait, 0, 0, 0);
		_s08_currManSequenceId = _s08_nextManSequenceId;
		_s08_nextManSequenceId = -1;
	}
	
	if (_gameSys->getAnimationStatus(3) == 2) {
		if (_s08_currDogSequenceId == 0x147)
			_platypusActionStatus = -1;
		if (_s08_currDogSequenceId == 0x149 || _s08_currDogSequenceId == 0x14A || _s08_currDogSequenceId == 0x14B) {
			if (getRandom(2) != 0)
				_s08_nextManSequenceId = 0x13D;
			else
				_s08_nextManSequenceId = 0x13E;
		} else if (_s08_currDogSequenceId == 0x133)
			_s08_nextManSequenceId = 0x139;
		if (_s08_nextDogSequenceId == 0x149 || _s08_nextDogSequenceId == 0x14A || _s08_nextDogSequenceId == 0x14B) {
			_gameSys->setAnimation(_s08_nextDogSequenceId, 100, 3);
			_gameSys->insertSequence(_s08_nextDogSequenceId, 100, _s08_currDogSequenceId, 100, kSeqSyncWait, 0, 0, 0);
			switch (_s08_nextDogSequenceId) {
			case 0x149:
				setFlag(kGFBarnPadlockOpen);
				_hotspots[kHSMeat]._flags = SF_DISABLED | SF_WALKABLE;
				_gameSys->removeSequence(0x144, 1, true);
				break;
			case 0x14A:
				setFlag(kGFTruckFilledWithGas);
				_hotspots[kHSBone]._flags = SF_DISABLED | SF_WALKABLE;
				_gameSys->removeSequence(0x145, 1, true);
				break;
			case 0x14B:
				setFlag(kGFTruckKeysUsed);
				_hotspots[kHSToy]._flags = SF_DISABLED | SF_WALKABLE;
				_gameSys->removeSequence(0x146, 1, true);
				break;
			}
			_s08_currDogSequenceId = _s08_nextDogSequenceId;
			_s08_nextDogSequenceId = 0x134;
		} else if (_s08_nextDogSequenceId == 0x147) {
			_gameSys->setAnimation(_s08_nextDogSequenceId, 100, 3);
			_gameSys->insertSequence(_s08_nextDogSequenceId, 100, _s08_currDogSequenceId, 100, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0x148, 160, _platypusSequenceId | (_platypusSequenceDatNum << 16), _platypusId, kSeqSyncWait, 0, 0, 0);
			_s08_currDogSequenceId = _s08_nextDogSequenceId;
			_s08_nextDogSequenceId = 0x134;
			_platX = 1;
			_platY = 8;
			_platypusId = 160;
			_platypusSequenceId = 0x148;
			_platypusFacing = kDirUnk4;
			_platypusSequenceDatNum = 0;
			if (_gnapX == 1 && _gnapY == 8)
				gnapWalkStep();
		} else if (_s08_nextDogSequenceId != -1) {
			_gameSys->setAnimation(_s08_nextDogSequenceId, 100, 3);
			_gameSys->insertSequence(_s08_nextDogSequenceId, 100, _s08_currDogSequenceId, 100, kSeqSyncWait, 0, 0, 0);
			_s08_currDogSequenceId = _s08_nextDogSequenceId;
			if (_s08_nextDogSequenceId != 0x135)
				_s08_nextDogSequenceId = 0x134;
			if (_s08_currDogSequenceId == 0x133) {
				_timers[2] = getRandom(30) + 20;
				_timers[3] = getRandom(50) + 200;
				_gameSys->insertSequence(0x14D, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
				_gnapSequenceId = 0x14D;
				_gnapIdleFacing = kDirUpRight;
				_gnapSequenceDatNum = 0;
				_gnapActionStatus = -1;
			}
		}
	}
}

} // End of namespace Gnap
