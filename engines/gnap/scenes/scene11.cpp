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
#include "gnap/scenes/scene11.h"

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

Scene11::Scene11(GnapEngine *vm) : Scene(vm) {
	_s11_billardBallCtr = 0;
	_s11_nextHookGuySequenceId = -1;
	_s11_currHookGuySequenceId = -1;
	_s11_nextGoggleGuySequenceId = -1;
	_s11_currGoggleGuySequenceId = -1;
}

int Scene11::init() {
	_vm->_gameSys->setAnimation(0, 0, 0);
	_vm->_gameSys->setAnimation(0, 0, 3);
	_vm->_gameSys->setAnimation(0, 0, 2);
	if (_vm->_prevSceneNum == 10 || _vm->_prevSceneNum == 13) {
		_vm->playSound(0x108EC, false);
		_vm->playSound(0x10928, false);
	}
	return 0x209;
}

void Scene11::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSExitKitchen, 420, 140, 520, 345, SF_EXIT_U_CURSOR);
	_vm->setHotspot(kHSExitToilet, 666, 130, 740, 364, SF_EXIT_R_CURSOR);
	_vm->setHotspot(kHSExitLeft, 0, 350, 10, 599, SF_EXIT_L_CURSOR | SF_WALKABLE);
	_vm->setHotspot(kHSGoggleGuy, 90, 185, 185, 340, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSHookGuy, 210, 240, 340, 430, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSBillard, 640, 475, 700, 530, SF_WALKABLE | SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSWalkArea1, 0, 0, 365, 453);
	_vm->setHotspot(kHSWalkArea2, 0, 0, 629, 353);
	_vm->setHotspot(kHSWalkArea3, 629, 0, 799, 364);
	_vm->setHotspot(kHSWalkArea4, 735, 0, 799, 397);
	_vm->setHotspot(kHSWalkArea5, 510, 540, 799, 599);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_vm->_hotspotsCount = 13;
}

void Scene11::run() {
	bool flag = true;
	
	_vm->_timers[7] = 50;
	_vm->_hotspots[kHSBillard]._flags |= SF_DISABLED;
	
	_s11_currGoggleGuySequenceId = 0x1F9;
	_s11_currHookGuySequenceId = 0x201;
	
	switch (_vm->_prevSceneNum) {
	case 13:
		_vm->initGnapPos(8, 5, kDirBottomLeft);
		_vm->initPlatypusPos(9, 6, kDirUnk4);
		break;
	case 47:
		_vm->initGnapPos(8, 5, kDirBottomLeft);
		_vm->initPlatypusPos(9, 5, kDirUnk4);
		_s11_currGoggleGuySequenceId = 0x1FA;
		_s11_currHookGuySequenceId = 0x1FF;
		_vm->_timers[7] = 180;
		break;
	case 12:
		_vm->initGnapPos(-1, 9, kDirBottomRight);
		_vm->initPlatypusPos(-2, 8, kDirNone);
		break;
	default:
		_vm->initGnapPos(6, 6, kDirBottomLeft);
		_vm->initPlatypusPos(6, 5, kDirUnk4);
		break;
	}
	
	_vm->queueInsertDeviceIcon();
	
	_vm->_gameSys->insertSequence(_s11_currHookGuySequenceId, 120, 0, 0, kSeqNone, 0, 0, 0);
	
	_s11_nextHookGuySequenceId = -1;
	
	_vm->_gameSys->setAnimation(_s11_currHookGuySequenceId, 120, 3);
	_vm->_gameSys->insertSequence(_s11_currGoggleGuySequenceId, 121, 0, 0, kSeqNone, 0, 0, 0);
	
	_s11_nextGoggleGuySequenceId = -1;
	
	_vm->_gameSys->setAnimation(_s11_currGoggleGuySequenceId, 121, 2);
	
	_vm->_timers[5] = _vm->getRandom(100) + 75;
	_vm->_timers[4] = _vm->getRandom(40) + 20;
	_vm->_timers[6] = _vm->getRandom(100) + 100;
	_vm->endSceneInit();
	
	if (_vm->_prevSceneNum == 12) {
		_vm->gnapWalkTo(2, 8, -1, 0x107B9, 1);
		_vm->platypusWalkTo(1, 8, -1, 0x107C2, 1);
	}

	_vm->_gameSys->insertSequence(0x208, 256, 0, 0, kSeqNone, 40, 0, 0);
	
	while (!_vm->_sceneDone) {
		_vm->testWalk(0, 0, -1, -1, -1, -1);
	
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
	
		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);
	
		switch (_vm->_sceneClickedHotspot) {
		case kHSPlatypus:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapImpossible(0, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->playGnapMoan1(_vm->_platX, _vm->_platY);
					break;
				case GRAB_CURSOR:
					_vm->gnapKissPlatypus(0);
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

		case kHSExitKitchen:
			_vm->_isLeavingScene = true;
			_vm->gnapWalkTo(6, 5, 0, 0x107BF, 1);
			_vm->_gnapActionStatus = kASLeaveScene;
			_vm->platypusWalkTo(6, 6, -1, -1, 1);
			_vm->_newSceneNum = 10;
			break;

		case kHSExitToilet:
			_vm->_isLeavingScene = true;
			_vm->gnapWalkTo(8, 5, 0, 0x107BF, 1);
			_vm->_gnapActionStatus = kASLeaveScene;
			_vm->platypusWalkTo(8, 6, -1, -1, 1);
			_vm->_newSceneNum = 13;
			break;

		case kHSExitLeft:
			_vm->_isLeavingScene = true;
			_vm->gnapWalkTo(-1, 8, 0, 0x107AF, 1);
			_vm->_gnapActionStatus = kASLeaveScene;
			_vm->platypusWalkTo(-1, 9, -1, 0x107CF, 1);
			_vm->_newSceneNum = 12;
			break;

		case kHSGoggleGuy:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemMagazine) {
					_vm->gnapWalkTo(3, 7, 0, 0x107BC, 1);
					_vm->_gnapActionStatus = kASShowMagazineToGoggleGuy;
					_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, 2, 0);
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(3, 7, 2, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(1, 6);
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					case TALK_CURSOR:
						_vm->_gnapIdleFacing = kDirUpLeft;
						_vm->gnapWalkTo(3, 7, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_vm->_gnapActionStatus = kASTalkGoggleGuy;
						break;
					}
				}
			}
			break;

		case kHSHookGuy:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_gnapIdleFacing = kDirUpRight;
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->gnapWalkTo(5, 6, 0, 0x107BC, 9);
					_vm->_gnapActionStatus = kASShowItemToHookGuy;
					_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, 4, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapSequence(_vm->getGnapSequenceId(gskDeflect, 3, 6) | 0x10000);
						break;
					case GRAB_CURSOR:
						_vm->gnapWalkTo(5, 6, 0, 0x107BC, 1);
						_vm->_gnapActionStatus = kASGrabHookGuy;
						break;
					case TALK_CURSOR:
						_vm->_gnapIdleFacing = kDirBottomLeft;
						_vm->gnapWalkTo(5, 6, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_vm->_gnapActionStatus = kASTalkHookGuy;
						break;
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSBillard:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapImpossible(9, 8);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(9, 8);
						break;
					case GRAB_CURSOR:
						_vm->gnapWalkTo(9, 8, 0, 0x107BA, 1);
						_vm->_gnapActionStatus = kASGrabBillardBall;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(9, 8);
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
			if (_vm->_gnapActionStatus < 0)
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			break;

		case kHSDevice:
			if (_vm->_gnapActionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;
			
		default:
			if (_vm->_mouseClickState._left && _vm->_gnapActionStatus < 0) {
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				_vm->_mouseClickState._left = false;
			}
			break;
			
		}

		updateAnimations();
	
		if (!_vm->_isLeavingScene) {
			if (flag && !_vm->_timers[7]) {
				flag = false;
				_vm->_gameSys->setAnimation(0x207, 257, 4);
				_vm->_gameSys->insertSequence(0x207, 257, 0, 0, kSeqNone, 0, 0, 0);
			}
			_vm->platypusSub426234();
			_vm->updateGnapIdleSequence2();
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(100) + 75;
				if (_vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0 && _s11_nextGoggleGuySequenceId == -1) {
					if (_vm->getRandom(2))
						_s11_nextGoggleGuySequenceId = 0x1F6;
					else
						_s11_nextGoggleGuySequenceId = 0x1F9;
				}
			}
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(40) + 20;
				if (_vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0 && _s11_nextHookGuySequenceId == -1) {
					if (_s11_currHookGuySequenceId == 0x201) {
						switch (_vm->getRandom(7)) {
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
			if (!_vm->_timers[6]) {
				_vm->_timers[6] = _vm->getRandom(100) + 100;
				int _gnapRandomValue = _vm->getRandom(3);
				switch (_gnapRandomValue) {
				case 0:
					_vm->_gameSys->insertSequence(0x8A5 | 0x10000, 179, 0, 0, kSeqNone, 0, 0, 0);
					break;
				case 1:
					_vm->_gameSys->insertSequence(0x8A7 | 0x10000, 179, 0, 0, kSeqNone, 0, 0, 0);
					break;
				case 2:
					_vm->_gameSys->insertSequence(0x8A6 | 0x10000, 179, 0, 0, kSeqNone, 0, 0, 0);
					break;
				}
			}
		}
	
		_vm->checkGameKeys();
	
		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[5] = _vm->getRandom(50) + 75;
			_vm->_timers[4] = _vm->getRandom(40) + 20;
		}
		
		_vm->gameUpdateTick();
	}
}

void Scene11::updateAnimations() {
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		if (_vm->_gnapActionStatus != kASGrabBillardBall)
			_vm->_gameSys->setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kASLeaveScene:
			_vm->_sceneDone = true;
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
			if (_vm->_gameSys->getAnimationStatus(2) == 2 && _vm->_gameSys->getAnimationStatus(3) == 2) {
				_vm->_gameSys->setAnimation(0, 0, 0);
				_vm->_timers[2] = _vm->getRandom(30) + 20;
				_vm->_timers[3] = _vm->getRandom(50) + 200;
				_vm->_gameSys->insertSequence(0x1F4, 255, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
				_vm->_gnapSequenceId = 0x1F4;
				_vm->_gnapId = 255;
				_vm->_gnapSequenceDatNum = 0;
				_vm->_gameSys->removeSequence(0x207, 257, true);
				_vm->_gameSys->removeSequence(0x208, 256, true);
				_s11_nextGoggleGuySequenceId = 0x1F8;
				_vm->_timers[5] = _vm->getRandom(100) + 75;
				_vm->_gameSys->insertSequence(_s11_nextGoggleGuySequenceId, 121, _s11_currGoggleGuySequenceId, 121, kSeqSyncWait, 0, 0, 0);
				_vm->_gameSys->setAnimation(_s11_nextGoggleGuySequenceId, 121, 2);
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
				_vm->_gameSys->insertSequence(_s11_nextHookGuySequenceId, 120, _s11_currHookGuySequenceId, 120, kSeqSyncWait, 0, 0, 0);
				_vm->_gameSys->setAnimation(_s11_nextHookGuySequenceId, 120, 3);
				_s11_currHookGuySequenceId = _s11_nextHookGuySequenceId;
				_s11_nextHookGuySequenceId = -1;
				_vm->_timers[4] = _vm->getRandom(40) + 20;
				_vm->_gameSys->insertSequence(0x208, 256, 0, 0, kSeqNone, _vm->getSequenceTotalDuration(0x1F4) - 5, 0, 0);
				_vm->_hotspots[kHSBillard]._flags |= SF_DISABLED;
				_vm->_gameSys->setAnimation(0x207, 257, 4);
				_vm->_gameSys->insertSequence(0x207, 257, 0, 0, kSeqNone, _vm->getSequenceTotalDuration(0x1FE), 0, 0);
				_vm->_gnapActionStatus = -1;
			}
			break;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(2) == 2 && _s11_nextGoggleGuySequenceId != -1) {
		_vm->_timers[5] = _vm->getRandom(100) + 75;
		_vm->_gameSys->insertSequence(_s11_nextGoggleGuySequenceId, 121, _s11_currGoggleGuySequenceId, 121, kSeqSyncWait, 0, 0, 0);
		_vm->_gameSys->setAnimation(_s11_nextGoggleGuySequenceId, 121, 2);
		_s11_currGoggleGuySequenceId = _s11_nextGoggleGuySequenceId;
		_s11_nextGoggleGuySequenceId = -1;
		if (_vm->_gnapActionStatus >= 1 && _vm->_gnapActionStatus <= 4)
			_vm->_gnapActionStatus = -1;
	}
	
	if (_vm->_gameSys->getAnimationStatus(3) == 2) {
		if (_s11_nextHookGuySequenceId == 0x204) {
			_vm->_gameSys->setAnimation(_s11_nextHookGuySequenceId, 120, 3);
			_vm->_gameSys->insertSequence(0x204, 120, _s11_currHookGuySequenceId, 120, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x1F5, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_s11_currHookGuySequenceId = 0x204;
			_s11_nextHookGuySequenceId = -1;
			_vm->_gnapSequenceId = 0x1F5;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_timers[4] = _vm->getRandom(40) + 20;
			_vm->_timers[2] = _vm->getRandom(20) + 70;
			_vm->_timers[3] = _vm->getRandom(50) + 200;
			if (_vm->_gnapActionStatus == kASGrabHookGuy)
				_vm->_gnapActionStatus = -1;
		} else if (_s11_nextHookGuySequenceId != -1) {
			_vm->_gameSys->insertSequence(_s11_nextHookGuySequenceId, 120, _s11_currHookGuySequenceId, 120, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(_s11_nextHookGuySequenceId, 120, 3);
			_s11_currHookGuySequenceId = _s11_nextHookGuySequenceId;
			_s11_nextHookGuySequenceId = -1;
			_vm->_timers[4] = _vm->getRandom(40) + 20;
			if (_vm->_gnapActionStatus >= 6 && _vm->_gnapActionStatus <= 9)
				_vm->_gnapActionStatus = -1;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(4) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 4);
		_vm->_hotspots[kHSBillard]._flags &= ~SF_DISABLED;
	}
}

} // End of namespace Gnap
