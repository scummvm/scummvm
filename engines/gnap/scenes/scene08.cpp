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
	kH08SPlatypus	= 0,
	kHS08ExitBackdoor	= 1,
	kHS08ExitCrash	= 2,
	kHS08Man		= 3,
	kHS08Door		= 4,
	kHS08Meat		= 5,
	kHS08Bone		= 6,
	kHS08Toy		= 7,
	kHS08WalkArea1	= 8,
	kHS08Device		= 9,
	kHS08WalkArea2	= 10
};

enum {
	kAS08LeaveScene		= 0,
	kAS08TalkMan		= 1,
	kAS08LookMan		= 2,
	kAS08LookDog		= 3,
	kAS08GrabDog		= 4,
	kAS08TalkDog		= 5,
	kAS08PlatWithMan	= 6,
	kAS08PlatWithDog	= 7
};

Scene08::Scene08(GnapEngine *vm) : Scene(vm) {
	_nextDogSequenceId = -1;
	_currDogSequenceId = -1;
	_nextManSequenceId = -1;
	_currManSequenceId = -1;
}

int Scene08::init() {
	return 0x150;
}

void Scene08::updateHotspots() {
	_vm->setHotspot(kH08SPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS08ExitBackdoor, 0, 280, 10, 400, SF_EXIT_L_CURSOR | SF_WALKABLE);
	_vm->setHotspot(kHS08ExitCrash, 200, 590, 400, 599, SF_EXIT_D_CURSOR | SF_WALKABLE);
	_vm->setHotspot(kHS08Man, 510, 150, 610, 380, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS08Door, 350, 170, 500, 410, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS08Meat, 405, 450, 480, 485, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS08Bone, 200, 405, 270, 465, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS08Toy, 540, 430, 615, 465, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS08WalkArea1, 290, 340, -1, -1);
	_vm->setHotspot(kHS08WalkArea2, 0, 0, 799, 420);
	_vm->setDeviceHotspot(kHS08Device, -1, -1, -1, -1);
	if (_vm->isFlag(kGFBarnPadlockOpen))
		_vm->_hotspots[kHS08Meat]._flags = SF_WALKABLE | SF_DISABLED;
	if (_vm->isFlag(kGFTruckFilledWithGas))
		_vm->_hotspots[kHS08Bone]._flags = SF_WALKABLE | SF_DISABLED;
	if (_vm->isFlag(kGFTruckKeysUsed))
		_vm->_hotspots[kHS08Toy]._flags = SF_WALKABLE | SF_DISABLED;
	_vm->_hotspotsCount = 11;
}

void Scene08::updateAnimationsCb() {
	if (_vm->_gameSys->getAnimationStatus(3) == 2) {
		_vm->_gameSys->setAnimation(_nextDogSequenceId, 100, 3);
		_vm->_gameSys->insertSequence(_nextDogSequenceId, 100, _currDogSequenceId, 100, kSeqSyncWait, 0, 0, 0);
		_currDogSequenceId = _nextDogSequenceId;
		if ( _nextDogSequenceId != 0x135 )
			_nextDogSequenceId = 0x134;
	}
}

void Scene08::run() {
	_vm->queueInsertDeviceIcon();

	_vm->_gameSys->insertSequence(0x14F, 1, 0, 0, kSeqLoop, 0, 0, 0);
	_vm->_gameSys->insertSequence(0x14E, 256, 0, 0, kSeqNone, 0, 0, 0);
	
	_currDogSequenceId = 0x135;
	_nextDogSequenceId = 0x135;
	
	_vm->_gameSys->setAnimation(0x135, 100, 3);
	_vm->_gameSys->insertSequence(_currDogSequenceId, 100, 0, 0, kSeqNone, 0, 0, 0);
	
	_currManSequenceId = 0x140;
	_nextManSequenceId = -1;
	
	_vm->_gameSys->setAnimation(0x140, 100, 2);
	_vm->_gameSys->insertSequence(_currManSequenceId, 100, 0, 0, kSeqNone, 0, 0, 0);
	
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
		case kHS08Device:
			if (_vm->_gnapActionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(50) + 75;
				_vm->_timers[5] = _vm->getRandom(40) + 50;
			}
			break;

		case kH08SPlatypus:
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

		case kHS08ExitBackdoor:
			_vm->_isLeavingScene = true;
			_vm->gnapActionIdle(0x14D);
			_vm->gnapWalkTo(0, 6, 0, 0x107AF, 1);
			_vm->_gnapActionStatus = kAS08LeaveScene;
			_vm->platypusWalkTo(0, 7, 1, 0x107CF, 1);
			_vm->_newSceneNum = 9;
			break;

		case kHS08ExitCrash:
			_vm->_isLeavingScene = true;
			_vm->gnapActionIdle(0x14D);
			_vm->gnapWalkTo(3, 9, 0, 0x107AE, 1);
			_vm->_gnapActionStatus = kAS08LeaveScene;
			_vm->platypusWalkTo(4, 9, 1, 0x107C1, 1);
			_vm->_newSceneNum = 7;
			break;

		case kHS08Man:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapShowCurrItem(6, 6, 7, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->gnapActionIdle(0x14D);
					_vm->gnapWalkTo(6, 6, 0, 0x107BB, 1);
					_vm->_gnapActionStatus = kAS08LookMan;
					_vm->_gnapIdleFacing = kDirUpRight;
					break;
				case GRAB_CURSOR:
					_vm->playGnapImpossible(0, 0);
					break;
				case TALK_CURSOR:
					_vm->_gnapIdleFacing = kDirUpLeft;
					_vm->gnapActionIdle(0x14D);
					_vm->gnapWalkTo(8, 6, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
					_vm->_gnapActionStatus = kAS08TalkMan;
					break;
				case PLAT_CURSOR:
					_vm->gnapActionIdle(0x14D);
					_vm->gnapUseDeviceOnPlatypuss();
					_vm->platypusWalkTo(6, 6, 1, 0x107C2, 1);
					_vm->_platypusActionStatus = kAS08PlatWithMan;
					_vm->_platypusFacing = kDirNone;
					_vm->playGnapIdle(6, 6);
					break;
				}
			}
			break;

		case kHS08Door:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapShowCurrItem(4, 7, 5, 0);
				_vm->_gameSys->setAnimation(makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, 0);
				_vm->_gnapActionStatus = kAS08GrabDog;
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->playGnapScratchingHead(6, 0);
					_vm->_gameSys->setAnimation(makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, 0);
					_vm->_gnapActionStatus = kAS08LookDog;
					break;
				case GRAB_CURSOR:
					_vm->gnapWalkTo(4, 7, 0, 0x107BB, 1);
					_vm->_gnapActionStatus = kAS08GrabDog;
					_vm->_gnapIdleFacing = kDirUpRight;
					break;
				case TALK_CURSOR:
					_vm->_gnapIdleFacing = kDirUpRight;
					_vm->gnapActionIdle(0x14D);
					_vm->gnapWalkTo(4, 7, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
					_vm->_gnapActionStatus = kAS08TalkDog;
					break;
				case PLAT_CURSOR:
					_vm->setFlag(kGFSceneFlag1);
					_vm->gnapActionIdle(0x14D);
					_vm->gnapUseDeviceOnPlatypuss();
					_vm->platypusWalkTo(3, 7, 1, 0x107C2, 1);
					_vm->_platypusActionStatus = kAS08PlatWithDog;
					_vm->_platypusFacing = kDirNone;
					_vm->playGnapIdle(3, 7);
					break;
				}
			}
			break;

		case kHS08Meat:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapShowCurrItem(6, 8, 5, 6);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->playGnapScratchingHead(6, 7);
					break;
				case GRAB_CURSOR:
					if (_currDogSequenceId == 0x135) {
						_vm->playGnapScratchingHead(6, 7);
					} else {
						_vm->gnapActionIdle(0x14D);
						_vm->playGnapPullOutDevice(6, 7);
						_vm->playGnapUseDevice(0, 0);
						_nextDogSequenceId = 0x149;
					}
					break;
				case TALK_CURSOR:
				case PLAT_CURSOR:
					_vm->playGnapImpossible(0, 0);
					break;
				}
			}
			break;

		case kHS08Bone:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapShowCurrItem(2, 7, 3, 6);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->playGnapScratchingHead(3, 6);
					break;
				case GRAB_CURSOR:
					if (_currDogSequenceId == 0x135) {
						_vm->playGnapScratchingHead(3, 6);
					} else {
						_vm->gnapActionIdle(0x14D);
						_vm->playGnapPullOutDevice(3, 6);
						_vm->playGnapUseDevice(0, 0);
						_nextDogSequenceId = 0x14A;
					}
					break;
				case TALK_CURSOR:
				case PLAT_CURSOR:
					_vm->playGnapImpossible(0, 0);
					break;
				}
			}
			break;

		case kHS08Toy:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapShowCurrItem(8, 7, 7, 6);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->playGnapScratchingHead(7, 6);
					break;
				case GRAB_CURSOR:
					if (_currDogSequenceId == 0x135) {
						_vm->playGnapScratchingHead(7, 6);
					} else {
						_vm->gnapActionIdle(0x14D);
						_vm->playGnapPullOutDevice(7, 6);
						_vm->playGnapUseDevice(0, 0);
						_nextDogSequenceId = 0x14B;
					}
					break;
				case TALK_CURSOR:
				case PLAT_CURSOR:
					_vm->playGnapImpossible(0, 0);
					break;
				}
			}
			break;

		case kHS08WalkArea1:
		case kHS08WalkArea2:
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
				if (_vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0 && _nextManSequenceId == -1 &&
					(_currDogSequenceId == 0x134 || _currDogSequenceId == 0x135)) {
					int _gnapRandomValue = _vm->getRandom(4);
					switch (_gnapRandomValue) {
					case 0:
						_nextManSequenceId = 0x138;
						break;
					case 1:
						_nextManSequenceId = 0x136;
						break;
					case 2:
						_nextManSequenceId = 0x13B;
						break;
					case 3:
						_nextManSequenceId = 0x13A;
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
		case kAS08LeaveScene:
			_vm->_sceneDone = true;
			_vm->_gnapActionStatus = -1;
			break;
		case kAS08TalkMan:
			_nextManSequenceId = 0x13F;
			_vm->_gnapActionStatus = -1;
			break;
		case kAS08LookMan:
			_nextManSequenceId = 0x140;
			_vm->_gnapActionStatus = -1;
			break;
		case kAS08LookDog:
			_nextManSequenceId = 0x137;
			_vm->_gnapActionStatus = -1;
			break;
		case kAS08GrabDog:
			if (_currDogSequenceId == 0x135)
				_nextDogSequenceId = 0x133;
			else
				_nextDogSequenceId = 0x13C;
			_vm->_gnapActionStatus = -1;
			break;
		case kAS08TalkDog:
			if (_currDogSequenceId == 0x135)
				_nextDogSequenceId = 0x133;
			else
				_nextDogSequenceId = 0x13C;
			_vm->_gnapActionStatus = -1;
			break;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(1) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 1);
		switch (_vm->_platypusActionStatus) {
		case kAS08PlatWithDog:
			_nextDogSequenceId = 0x147;
			break;
		case kAS08PlatWithMan:
			_nextManSequenceId = 0x140;
			_vm->_platypusActionStatus = -1;
			break;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(2) == 2 && _nextManSequenceId != -1) {
		_vm->_gameSys->setAnimation(_nextManSequenceId, 100, 2);
		_vm->_gameSys->insertSequence(_nextManSequenceId, 100, _currManSequenceId, 100, kSeqSyncWait, 0, 0, 0);
		_currManSequenceId = _nextManSequenceId;
		_nextManSequenceId = -1;
	}
	
	if (_vm->_gameSys->getAnimationStatus(3) == 2) {
		if (_currDogSequenceId == 0x147)
			_vm->_platypusActionStatus = -1;
		if (_currDogSequenceId == 0x149 || _currDogSequenceId == 0x14A || _currDogSequenceId == 0x14B) {
			if (_vm->getRandom(2) != 0)
				_nextManSequenceId = 0x13D;
			else
				_nextManSequenceId = 0x13E;
		} else if (_currDogSequenceId == 0x133)
			_nextManSequenceId = 0x139;
		if (_nextDogSequenceId == 0x149 || _nextDogSequenceId == 0x14A || _nextDogSequenceId == 0x14B) {
			_vm->_gameSys->setAnimation(_nextDogSequenceId, 100, 3);
			_vm->_gameSys->insertSequence(_nextDogSequenceId, 100, _currDogSequenceId, 100, kSeqSyncWait, 0, 0, 0);
			switch (_nextDogSequenceId) {
			case 0x149:
				_vm->setFlag(kGFBarnPadlockOpen);
				_vm->_hotspots[kHS08Meat]._flags = SF_DISABLED | SF_WALKABLE;
				_vm->_gameSys->removeSequence(0x144, 1, true);
				break;
			case 0x14A:
				_vm->setFlag(kGFTruckFilledWithGas);
				_vm->_hotspots[kHS08Bone]._flags = SF_DISABLED | SF_WALKABLE;
				_vm->_gameSys->removeSequence(0x145, 1, true);
				break;
			case 0x14B:
				_vm->setFlag(kGFTruckKeysUsed);
				_vm->_hotspots[kHS08Toy]._flags = SF_DISABLED | SF_WALKABLE;
				_vm->_gameSys->removeSequence(0x146, 1, true);
				break;
			}
			_currDogSequenceId = _nextDogSequenceId;
			_nextDogSequenceId = 0x134;
		} else if (_nextDogSequenceId == 0x147) {
			_vm->_gameSys->setAnimation(_nextDogSequenceId, 100, 3);
			_vm->_gameSys->insertSequence(_nextDogSequenceId, 100, _currDogSequenceId, 100, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x148, 160, _vm->_platypusSequenceId | (_vm->_platypusSequenceDatNum << 16), _vm->_platypusId, kSeqSyncWait, 0, 0, 0);
			_currDogSequenceId = _nextDogSequenceId;
			_nextDogSequenceId = 0x134;
			_vm->_platX = 1;
			_vm->_platY = 8;
			_vm->_platypusId = 160;
			_vm->_platypusSequenceId = 0x148;
			_vm->_platypusFacing = kDirUnk4;
			_vm->_platypusSequenceDatNum = 0;
			if (_vm->_gnapX == 1 && _vm->_gnapY == 8)
				_vm->gnapWalkStep();
		} else if (_nextDogSequenceId != -1) {
			_vm->_gameSys->setAnimation(_nextDogSequenceId, 100, 3);
			_vm->_gameSys->insertSequence(_nextDogSequenceId, 100, _currDogSequenceId, 100, kSeqSyncWait, 0, 0, 0);
			_currDogSequenceId = _nextDogSequenceId;
			if (_nextDogSequenceId != 0x135)
				_nextDogSequenceId = 0x134;
			if (_currDogSequenceId == 0x133) {
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
