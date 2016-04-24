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
#include "gnap/scenes/scene08.h"

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

Scene08::Scene08(GnapEngine *vm) : Scene(vm) {
	_s08_nextDogSequenceId = -1;
	_s08_currDogSequenceId = -1;
	_s08_nextManSequenceId = -1;
	_s08_currManSequenceId = -1;
}

int Scene08::init() {
	return 0x150;
}

void Scene08::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSExitBackdoor, 0, 280, 10, 400, SF_EXIT_L_CURSOR | SF_WALKABLE);
	_vm->setHotspot(kHSExitCrash, 200, 590, 400, 599, SF_EXIT_D_CURSOR | SF_WALKABLE);
	_vm->setHotspot(kHSMan, 510, 150, 610, 380, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSDoor, 350, 170, 500, 410, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSMeat, 405, 450, 480, 485, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSBone, 200, 405, 270, 465, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSToy, 540, 430, 615, 465, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSWalkArea1, 290, 340, -1, -1);
	_vm->setHotspot(kHSWalkArea2, 0, 0, 799, 420);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	if (_vm->isFlag(kGFBarnPadlockOpen))
		_vm->_hotspots[kHSMeat]._flags = SF_WALKABLE | SF_DISABLED;
	if (_vm->isFlag(kGFTruckFilledWithGas))
		_vm->_hotspots[kHSBone]._flags = SF_WALKABLE | SF_DISABLED;
	if (_vm->isFlag(kGFTruckKeysUsed))
		_vm->_hotspots[kHSToy]._flags = SF_WALKABLE | SF_DISABLED;
	_vm->_hotspotsCount = 11;
}

void Scene08::updateAnimationsCb() {
	if (_vm->_gameSys->getAnimationStatus(3) == 2) {
		_vm->_gameSys->setAnimation(_s08_nextDogSequenceId, 100, 3);
		_vm->_gameSys->insertSequence(_s08_nextDogSequenceId, 100, _s08_currDogSequenceId, 100, kSeqSyncWait, 0, 0, 0);
		_s08_currDogSequenceId = _s08_nextDogSequenceId;
		if ( _s08_nextDogSequenceId != 0x135 )
			_s08_nextDogSequenceId = 0x134;
	}
}

void Scene08::run() {
	_vm->queueInsertDeviceIcon();

	_vm->_gameSys->insertSequence(0x14F, 1, 0, 0, kSeqLoop, 0, 0, 0);
	_vm->_gameSys->insertSequence(0x14E, 256, 0, 0, kSeqNone, 0, 0, 0);
	
	_s08_currDogSequenceId = 0x135;
	_s08_nextDogSequenceId = 0x135;
	
	_vm->_gameSys->setAnimation(0x135, 100, 3);
	_vm->_gameSys->insertSequence(_s08_currDogSequenceId, 100, 0, 0, kSeqNone, 0, 0, 0);
	
	_s08_currManSequenceId = 0x140;
	_s08_nextManSequenceId = -1;
	
	_vm->_gameSys->setAnimation(0x140, 100, 2);
	_vm->_gameSys->insertSequence(_s08_currManSequenceId, 100, 0, 0, kSeqNone, 0, 0, 0);
	
	_vm->_timers[4] = _vm->getRandom(50) + 75;
	
	if (!_vm->isFlag(kGFBarnPadlockOpen))
		_vm->_gameSys->insertSequence(0x144, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	if (!_vm->isFlag(kGFTruckFilledWithGas))
		_vm->_gameSys->insertSequence(0x145, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	if (!_vm->isFlag(kGFTruckKeysUsed))
		_vm->_gameSys->insertSequence(0x146, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	_vm->initGnapPos(-1, 8, kDirBottomRight);
	_vm->initPlatypusPos(-1, 7, kDirNone);
	
	_vm->endSceneInit();
	
	_vm->gnapWalkTo(1, 8, -1, 0x107B9, 1);
	_vm->platypusWalkTo(1, 7, -1, 0x107C2, 1);
	
	_vm->_timers[5] = _vm->getRandom(40) + 50;
	
	while (!_vm->_sceneDone) {	
		if (!_vm->isSoundPlaying(0x10919))
			_vm->playSound(0x10919, true);
	
		_vm->testWalk(0, 0, -1, -1, -1, -1);
		
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);
	
		switch (_vm->_sceneClickedHotspot) {
		case kHSDevice:
			if (_vm->_gnapActionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(50) + 75;
				_vm->_timers[5] = _vm->getRandom(40) + 50;
			}
			break;

		case kHSPlatypus:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapImpossible(0, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					if (_vm->isFlag(kGFSceneFlag1))
						_vm->playGnapMoan1(_vm->_platX, _vm->_platY);
					else
						_vm->playGnapScratchingHead(_vm->_platX, _vm->_platY);
					break;
				case GRAB_CURSOR:
					_vm->gnapActionIdle(0x14D);
					_vm->gnapKissPlatypus(8);
					break;
				case TALK_CURSOR:
					_vm->playGnapBrainPulsating(_vm->_platX, _vm->_platY);
					_vm->playPlatypusSequence(_vm->getPlatypusSequenceId());
					break;
				case PLAT_CURSOR:
					break;
				}
			}
			break;

		case kHSExitBackdoor:
			_vm->_isLeavingScene = true;
			_vm->gnapActionIdle(0x14D);
			_vm->gnapWalkTo(0, 6, 0, 0x107AF, 1);
			_vm->_gnapActionStatus = kASLeaveScene;
			_vm->platypusWalkTo(0, 7, 1, 0x107CF, 1);
			_vm->_newSceneNum = 9;
			break;

		case kHSExitCrash:
			_vm->_isLeavingScene = true;
			_vm->gnapActionIdle(0x14D);
			_vm->gnapWalkTo(3, 9, 0, 0x107AE, 1);
			_vm->_gnapActionStatus = kASLeaveScene;
			_vm->platypusWalkTo(4, 9, 1, 0x107C1, 1);
			_vm->_newSceneNum = 7;
			break;

		case kHSMan:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapShowCurrItem(6, 6, 7, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->gnapActionIdle(0x14D);
					_vm->gnapWalkTo(6, 6, 0, 0x107BB, 1);
					_vm->_gnapActionStatus = kASLookMan;
					_vm->_gnapIdleFacing = kDirUpRight;
					break;
				case GRAB_CURSOR:
					_vm->playGnapImpossible(0, 0);
					break;
				case TALK_CURSOR:
					_vm->_gnapIdleFacing = kDirUpLeft;
					_vm->gnapActionIdle(0x14D);
					_vm->gnapWalkTo(8, 6, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
					_vm->_gnapActionStatus = kASTalkMan;
					break;
				case PLAT_CURSOR:
					_vm->gnapActionIdle(0x14D);
					_vm->gnapUseDeviceOnPlatypuss();
					_vm->platypusWalkTo(6, 6, 1, 0x107C2, 1);
					_vm->_platypusActionStatus = kASPlatWithMan;
					_vm->_platypusFacing = kDirNone;
					_vm->playGnapIdle(6, 6);
					break;
				}
			}
			break;

		case kHSDoor:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapShowCurrItem(4, 7, 5, 0);
				_vm->_gameSys->setAnimation(makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, 0);
				_vm->_gnapActionStatus = kASGrabDog;
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->playGnapScratchingHead(6, 0);
					_vm->_gameSys->setAnimation(makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, 0);
					_vm->_gnapActionStatus = kASLookDog;
					break;
				case GRAB_CURSOR:
					_vm->gnapWalkTo(4, 7, 0, 0x107BB, 1);
					_vm->_gnapActionStatus = kASGrabDog;
					_vm->_gnapIdleFacing = kDirUpRight;
					break;
				case TALK_CURSOR:
					_vm->_gnapIdleFacing = kDirUpRight;
					_vm->gnapActionIdle(0x14D);
					_vm->gnapWalkTo(4, 7, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
					_vm->_gnapActionStatus = kASTalkDog;
					break;
				case PLAT_CURSOR:
					_vm->setFlag(kGFSceneFlag1);
					_vm->gnapActionIdle(0x14D);
					_vm->gnapUseDeviceOnPlatypuss();
					_vm->platypusWalkTo(3, 7, 1, 0x107C2, 1);
					_vm->_platypusActionStatus = kASPlatWithDog;
					_vm->_platypusFacing = kDirNone;
					_vm->playGnapIdle(3, 7);
					break;
				}
			}
			break;

		case kHSMeat:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapShowCurrItem(6, 8, 5, 6);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->playGnapScratchingHead(6, 7);
					break;
				case GRAB_CURSOR:
					if (_s08_currDogSequenceId == 0x135) {
						_vm->playGnapScratchingHead(6, 7);
					} else {
						_vm->gnapActionIdle(0x14D);
						_vm->playGnapPullOutDevice(6, 7);
						_vm->playGnapUseDevice(0, 0);
						_s08_nextDogSequenceId = 0x149;
					}
					break;
				case TALK_CURSOR:
				case PLAT_CURSOR:
					_vm->playGnapImpossible(0, 0);
					break;
				}
			}
			break;

		case kHSBone:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapShowCurrItem(2, 7, 3, 6);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->playGnapScratchingHead(3, 6);
					break;
				case GRAB_CURSOR:
					if (_s08_currDogSequenceId == 0x135) {
						_vm->playGnapScratchingHead(3, 6);
					} else {
						_vm->gnapActionIdle(0x14D);
						_vm->playGnapPullOutDevice(3, 6);
						_vm->playGnapUseDevice(0, 0);
						_s08_nextDogSequenceId = 0x14A;
					}
					break;
				case TALK_CURSOR:
				case PLAT_CURSOR:
					_vm->playGnapImpossible(0, 0);
					break;
				}
			}
			break;

		case kHSToy:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapShowCurrItem(8, 7, 7, 6);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->playGnapScratchingHead(7, 6);
					break;
				case GRAB_CURSOR:
					if (_s08_currDogSequenceId == 0x135) {
						_vm->playGnapScratchingHead(7, 6);
					} else {
						_vm->gnapActionIdle(0x14D);
						_vm->playGnapPullOutDevice(7, 6);
						_vm->playGnapUseDevice(0, 0);
						_s08_nextDogSequenceId = 0x14B;
					}
					break;
				case TALK_CURSOR:
				case PLAT_CURSOR:
					_vm->playGnapImpossible(0, 0);
					break;
				}
			}
			break;

		case kHSWalkArea1:
		case kHSWalkArea2:
			_vm->gnapActionIdle(0x14D);
			_vm->gnapWalkTo(-1, 6, -1, -1, 1);
			break;

		default:
			if (_vm->_mouseClickState._left) {
				_vm->gnapActionIdle(0x14D);
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				_vm->_mouseClickState._left = false;
			}
			break;
		}
	
		updateAnimations();

		if (!_vm->_isLeavingScene) {
			_vm->updatePlatypusIdleSequence();
			_vm->updateGnapIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(50) + 125;
				if (_vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0 && _s08_nextManSequenceId == -1 &&
					(_s08_currDogSequenceId == 0x134 || _s08_currDogSequenceId == 0x135)) {
					int _gnapRandomValue = _vm->getRandom(4);
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
			playRandomSound(5);
		}
	
		_vm->checkGameKeys();
	
		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[4] = _vm->getRandom(50) + 75;
			_vm->_timers[5] = _vm->getRandom(40) + 50;
		}
		
		_vm->gameUpdateTick();
	}
}

void Scene08::updateAnimations() {
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kASLeaveScene:
			_vm->_sceneDone = true;
			_vm->_gnapActionStatus = -1;
			break;
		case kASTalkMan:
			_s08_nextManSequenceId = 0x13F;
			_vm->_gnapActionStatus = -1;
			break;
		case kASLookMan:
			_s08_nextManSequenceId = 0x140;
			_vm->_gnapActionStatus = -1;
			break;
		case kASLookDog:
			_s08_nextManSequenceId = 0x137;
			_vm->_gnapActionStatus = -1;
			break;
		case kASGrabDog:
			if (_s08_currDogSequenceId == 0x135)
				_s08_nextDogSequenceId = 0x133;
			else
				_s08_nextDogSequenceId = 0x13C;
			_vm->_gnapActionStatus = -1;
			break;
		case kASTalkDog:
			if (_s08_currDogSequenceId == 0x135)
				_s08_nextDogSequenceId = 0x133;
			else
				_s08_nextDogSequenceId = 0x13C;
			_vm->_gnapActionStatus = -1;
			break;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(1) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 1);
		switch (_vm->_platypusActionStatus) {
		case kASPlatWithDog:
			_s08_nextDogSequenceId = 0x147;
			break;
		case kASPlatWithMan:
			_s08_nextManSequenceId = 0x140;
			_vm->_platypusActionStatus = -1;
			break;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(2) == 2 && _s08_nextManSequenceId != -1) {
		_vm->_gameSys->setAnimation(_s08_nextManSequenceId, 100, 2);
		_vm->_gameSys->insertSequence(_s08_nextManSequenceId, 100, _s08_currManSequenceId, 100, kSeqSyncWait, 0, 0, 0);
		_s08_currManSequenceId = _s08_nextManSequenceId;
		_s08_nextManSequenceId = -1;
	}
	
	if (_vm->_gameSys->getAnimationStatus(3) == 2) {
		if (_s08_currDogSequenceId == 0x147)
			_vm->_platypusActionStatus = -1;
		if (_s08_currDogSequenceId == 0x149 || _s08_currDogSequenceId == 0x14A || _s08_currDogSequenceId == 0x14B) {
			if (_vm->getRandom(2) != 0)
				_s08_nextManSequenceId = 0x13D;
			else
				_s08_nextManSequenceId = 0x13E;
		} else if (_s08_currDogSequenceId == 0x133)
			_s08_nextManSequenceId = 0x139;
		if (_s08_nextDogSequenceId == 0x149 || _s08_nextDogSequenceId == 0x14A || _s08_nextDogSequenceId == 0x14B) {
			_vm->_gameSys->setAnimation(_s08_nextDogSequenceId, 100, 3);
			_vm->_gameSys->insertSequence(_s08_nextDogSequenceId, 100, _s08_currDogSequenceId, 100, kSeqSyncWait, 0, 0, 0);
			switch (_s08_nextDogSequenceId) {
			case 0x149:
				_vm->setFlag(kGFBarnPadlockOpen);
				_vm->_hotspots[kHSMeat]._flags = SF_DISABLED | SF_WALKABLE;
				_vm->_gameSys->removeSequence(0x144, 1, true);
				break;
			case 0x14A:
				_vm->setFlag(kGFTruckFilledWithGas);
				_vm->_hotspots[kHSBone]._flags = SF_DISABLED | SF_WALKABLE;
				_vm->_gameSys->removeSequence(0x145, 1, true);
				break;
			case 0x14B:
				_vm->setFlag(kGFTruckKeysUsed);
				_vm->_hotspots[kHSToy]._flags = SF_DISABLED | SF_WALKABLE;
				_vm->_gameSys->removeSequence(0x146, 1, true);
				break;
			}
			_s08_currDogSequenceId = _s08_nextDogSequenceId;
			_s08_nextDogSequenceId = 0x134;
		} else if (_s08_nextDogSequenceId == 0x147) {
			_vm->_gameSys->setAnimation(_s08_nextDogSequenceId, 100, 3);
			_vm->_gameSys->insertSequence(_s08_nextDogSequenceId, 100, _s08_currDogSequenceId, 100, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x148, 160, _vm->_platypusSequenceId | (_vm->_platypusSequenceDatNum << 16), _vm->_platypusId, kSeqSyncWait, 0, 0, 0);
			_s08_currDogSequenceId = _s08_nextDogSequenceId;
			_s08_nextDogSequenceId = 0x134;
			_vm->_platX = 1;
			_vm->_platY = 8;
			_vm->_platypusId = 160;
			_vm->_platypusSequenceId = 0x148;
			_vm->_platypusFacing = kDirUnk4;
			_vm->_platypusSequenceDatNum = 0;
			if (_vm->_gnapX == 1 && _vm->_gnapY == 8)
				_vm->gnapWalkStep();
		} else if (_s08_nextDogSequenceId != -1) {
			_vm->_gameSys->setAnimation(_s08_nextDogSequenceId, 100, 3);
			_vm->_gameSys->insertSequence(_s08_nextDogSequenceId, 100, _s08_currDogSequenceId, 100, kSeqSyncWait, 0, 0, 0);
			_s08_currDogSequenceId = _s08_nextDogSequenceId;
			if (_s08_nextDogSequenceId != 0x135)
				_s08_nextDogSequenceId = 0x134;
			if (_s08_currDogSequenceId == 0x133) {
				_vm->_timers[2] = _vm->getRandom(30) + 20;
				_vm->_timers[3] = _vm->getRandom(50) + 200;
				_vm->_gameSys->insertSequence(0x14D, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
				_vm->_gnapSequenceId = 0x14D;
				_vm->_gnapIdleFacing = kDirUpRight;
				_vm->_gnapSequenceDatNum = 0;
				_vm->_gnapActionStatus = -1;
			}
		}
	}
}

} // End of namespace Gnap
