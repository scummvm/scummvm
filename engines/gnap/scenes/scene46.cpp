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
#include "gnap/scenes/scene46.h"

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

Scene46::Scene46(GnapEngine *vm) : Scene(vm) {
	_currSackGuySequenceId = -1;
	_nextItchyGuySequenceId = -1;
	_nextSackGuySequenceId = -1;
	_currItchyGuySequenceId = -1;
}

int Scene46::init() {
	GameSys gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 1);
	gameSys.setAnimation(0, 0, 2);
	gameSys.setAnimation(0, 0, 3);
	gameSys.setAnimation(0, 0, 4);
	return 0x4E;
}

void Scene46::updateHotspots() {
	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_DISABLED);
		_vm->setHotspot(kHSUfoExitLeft, 0, 0, 10, 599, SF_EXIT_L_CURSOR);
		_vm->setHotspot(kHSUfoExitRight, 790, 0, 799, 599, SF_EXIT_R_CURSOR);
		_vm->setDeviceHotspot(kHSUfoDevice, -1, 534, -1, 599);
		_vm->_hotspotsCount = 4;
	} else {
		_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHSExitUfoParty, 150, 580, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
		_vm->setHotspot(kHSExitKissinBooth, 0, 100, 10, 599, SF_EXIT_L_CURSOR, 0, 8);
		_vm->setHotspot(kHSExitDisco, 790, 100, 799, 599, SF_EXIT_R_CURSOR, 10, 8);
		_vm->setHotspot(kHSSackGuy, 180, 370, 235, 490, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 8);
		_vm->setHotspot(kHSItchyGuy, 535, 210, 650, 480, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 8);
		_vm->setHotspot(kHSWalkArea1, 0, 0, 800, 485);
		_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
		_vm->_hotspotsCount = 8;
	}
}

void Scene46::run() {
	GameSys gameSys = *_vm->_gameSys;

	_vm->queueInsertDeviceIcon();
	gameSys.insertSequence(0x4D, 0, 0, 0, kSeqLoop, 0, 0, 0);
	
	_currSackGuySequenceId = 0x4B;
	_nextSackGuySequenceId = -1;
	gameSys.setAnimation(0x4B, 1, 3);
	gameSys.insertSequence(_currSackGuySequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	_currItchyGuySequenceId = 0x47;
	_nextItchyGuySequenceId = -1;
	gameSys.setAnimation(0x47, 1, 4);
	gameSys.insertSequence(_currItchyGuySequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->_toyUfoId = 0;
		_vm->_toyUfoActionStatus = -1;
		_vm->_toyUfoSequenceId = _vm->toyUfoGetSequenceId();
		_vm->_toyUfoNextSequenceId = _vm->_toyUfoSequenceId;
		if (_vm->_prevSceneNum == 44)
			_vm->_toyUfoX = 30;
		else
			_vm->_toyUfoX = 770;
		gameSys.setAnimation(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 2);
		gameSys.insertSequence(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 0, 0, kSeqNone, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
		_vm->endSceneInit();
	} else if (_vm->_prevSceneNum == 44) {
		_vm->initGnapPos(-1, 8, kDirUpRight);
		_vm->initPlatypusPos(-1, 8, kDirUpLeft);
		_vm->endSceneInit();
		_vm->platypusWalkTo(1, 8, -1, 0x107C2, 1);
		_vm->gnapWalkTo(2, 8, -1, 0x107B9, 1);
	} else if (_vm->_prevSceneNum == 45) {
		_vm->initGnapPos(11, 8, kDirUpRight);
		_vm->initPlatypusPos(12, 8, kDirUpLeft);
		_vm->endSceneInit();
		_vm->gnapWalkTo(8, 8, -1, 0x107BA, 1);
		_vm->platypusWalkTo(9, 8, -1, 0x107D2, 1);
	} else {
		_vm->initGnapPos(5, 11, kDirUpRight);
		_vm->initPlatypusPos(6, 11, kDirUpLeft);
		_vm->endSceneInit();
		_vm->platypusWalkTo(5, 8, -1, 0x107C2, 1);
		_vm->gnapWalkTo(6, 8, -1, 0x107BA, 1);
	}

	_vm->_timers[4] = _vm->getRandom(50) + 80;
	_vm->_timers[5] = _vm->getRandom(50) + 80;

	while (!_vm->_sceneDone) {
		if (!_vm->isSoundPlaying(0x1094B))
			_vm->playSound(0x1094B, true);
	
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
	
		_vm->testWalk(0, 0, -1, -1, -1, -1);
	
		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);
	
		if (_vm->isFlag(kGFGnapControlsToyUFO)) {
			switch (_vm->_sceneClickedHotspot) {
			case kHSUfoExitLeft:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = 3;
					_vm->_newSceneNum = 44;
					_vm->toyUfoFlyTo(-35, -1, -35, 799, 0, 300, 2);
				}
				break;

			case kHSUfoExitRight:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = 3;
					_vm->_newSceneNum = 45;
					_vm->toyUfoFlyTo(835, -1, 0, 835, 0, 300, 2);
				}
				break;

			case kHSUfoDevice:
				_vm->runMenu();
				updateHotspots();
				break;
			}
		} else {
			switch (_vm->_sceneClickedHotspot) {
			case kHSDevice:
				_vm->runMenu();
				updateHotspots();
				break;

			case kHSPlatypus:
				if (_vm->_gnapActionStatus < 0) {
					if (_vm->_grabCursorSpriteIndex >= 0) {
						_vm->playGnapImpossible(_vm->_platX, _vm->_platY);
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
							_vm->playGnapImpossible(_vm->_platX, _vm->_platY);
							break;
						}
					}
				}
				break;

			case kHSSackGuy:
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHSSackGuy].x, _vm->_hotspotsWalkPos[kHSSackGuy].y, 2, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapMoan1(_vm->_hotspotsWalkPos[kHSSackGuy].x + 1, 0);
						break;
					case TALK_CURSOR:
						_vm->_gnapIdleFacing = kDirUpLeft;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSSackGuy].x, _vm->_hotspotsWalkPos[kHSSackGuy].y, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_vm->_gnapActionStatus = 2;
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
				break;

			case kHSItchyGuy:
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHSItchyGuy].x, _vm->_hotspotsWalkPos[kHSItchyGuy].y, 7, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapMoan1(_vm->_hotspotsWalkPos[kHSItchyGuy].x - 1, 0);
						break;
					case TALK_CURSOR:
						_vm->_gnapIdleFacing = kDirUpRight;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSItchyGuy].x, _vm->_hotspotsWalkPos[kHSItchyGuy].y, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_vm->_gnapActionStatus = 1;
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
				break;

			case kHSExitUfoParty:
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_gnapX, _vm->_hotspotsWalkPos[kHSExitUfoParty].y, 0, 0x107AE, 1);
				_vm->_gnapActionStatus = 0;
				_vm->_newSceneNum = 40;
				break;

			case kHSExitKissinBooth:
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitKissinBooth].x, _vm->_gnapY, 0, 0x107AF, 1);
				_vm->_gnapActionStatus = 0;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitKissinBooth].x, _vm->_platY, -1, 0x107CF, 1);
				_vm->_newSceneNum = 44;
				break;

			case kHSExitDisco:
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitDisco].x, _vm->_gnapY, 0, 0x107AB, 1);
				_vm->_gnapActionStatus = 0;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitDisco].x, _vm->_platY, -1, 0x107CD, 1);
				_vm->_newSceneNum = 45;
				break;

			case kHSWalkArea1:
				if (_vm->_gnapActionStatus < 0)
					_vm->gnapWalkTo(-1, -1, -1, -1, 1);
					break;
			}
		}
	
		if (_vm->_mouseClickState._left && _vm->_gnapActionStatus < 0) {
			_vm->_mouseClickState._left = false;
			if (_vm->isFlag(kGFGnapControlsToyUFO)) {
				_vm->_toyUfoActionStatus = 4;
				_vm->toyUfoFlyTo(-1, -1, 0, 799, 0, 300, 2);
			} else {
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			}
		}
	
		updateAnimations();
		_vm->toyUfoCheckTimer();
	
		if (!_vm->_isLeavingScene) {
			if (_vm->_platypusActionStatus < 0 && !_vm->isFlag(kGFGnapControlsToyUFO))
				_vm->updatePlatypusIdleSequence();
			if (_vm->_gnapActionStatus < 0 && !_vm->isFlag(kGFGnapControlsToyUFO))
				_vm->updateGnapIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(50) + 80;
				if (_vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0 && _nextItchyGuySequenceId == -1) {
					if (_vm->getRandom(2) != 0)
						_nextItchyGuySequenceId = 0x49;
					else
						_nextItchyGuySequenceId = 0x48;
				}
			}
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(50) + 80;
				if (_vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0 && _nextSackGuySequenceId == -1)
					_nextSackGuySequenceId = 0x4C;
			}
		}
	
		_vm->checkGameKeys();
	
		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
		}
		
		_vm->gameUpdateTick();
	}
}

void Scene46::updateAnimations() {
	GameSys gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case 0:
			_vm->_sceneDone = true;
			break;
		case 1:
			_nextItchyGuySequenceId = 0x46;
			break;
		case 2:
			_nextSackGuySequenceId = 0x4A;
			break;
		}
		_vm->_gnapActionStatus = -1;
	}
	
	if (gameSys.getAnimationStatus(3) == 2 && _nextSackGuySequenceId != -1) {
		gameSys.insertSequence(_nextSackGuySequenceId, 1, _currSackGuySequenceId, 1, kSeqSyncWait, 0, 0, 0);
		gameSys.setAnimation(_nextSackGuySequenceId, 1, 3);
		_currSackGuySequenceId = _nextSackGuySequenceId;
		_nextSackGuySequenceId = -1;
		_vm->_timers[5] = _vm->getRandom(50) + 80;
	}
	
	if (gameSys.getAnimationStatus(4) == 2 && _nextItchyGuySequenceId != -1) {
		gameSys.insertSequence(_nextItchyGuySequenceId, 1, _currItchyGuySequenceId, 1, kSeqSyncWait, 0, 0, 0);
		gameSys.setAnimation(_nextItchyGuySequenceId, 1, 4);
		_currItchyGuySequenceId = _nextItchyGuySequenceId;
		_nextItchyGuySequenceId = -1;
		_vm->_timers[4] = _vm->getRandom(50) + 80;
	}
	
	if (gameSys.getAnimationStatus(2) == 2) {
		switch (_vm->_toyUfoActionStatus) {
		case 3:
			_vm->_sceneDone = true;
			break;
		default:
			_vm->_toyUfoNextSequenceId = _vm->toyUfoGetSequenceId();
			gameSys.insertSequence(_vm->_toyUfoNextSequenceId | 0x10000, _vm->_toyUfoId + 1,
				_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId,
				kSeqSyncWait, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
			_vm->_toyUfoSequenceId = _vm->_toyUfoNextSequenceId;
			++_vm->_toyUfoId;
			gameSys.setAnimation(_vm->_toyUfoNextSequenceId | 0x10000, _vm->_toyUfoId, 2);
			break;
		}
		_vm->_toyUfoActionStatus = -1;
	}
}

} // End of namespace Gnap
