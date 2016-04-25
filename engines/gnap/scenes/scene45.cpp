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
#include "gnap/scenes/scene45.h"

namespace Gnap {

enum {
	kHSPlatypus			= 0,
	kHSExitUfoParty		= 1,
	kHSExitShoe			= 2,
	kHSExitRight		= 3,
	kHSExitDiscoBall	= 4,
	kHSDiscoBall		= 5,
	kHSDevice			= 6,
	kHSWalkArea1		= 7
};

enum {
	kHSUfoExitLeft	= 1,
	kHSUfoExitRight	= 2,
	kHSUfoDevice	= 3
};

Scene45::Scene45(GnapEngine *vm) : Scene(vm) {
	_currDancerSequenceId = -1;
}

int Scene45::init() {
	GameSys gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 1);
	gameSys.setAnimation(0, 0, 2);
	gameSys.setAnimation(0, 0, 3);
	gameSys.setAnimation(0, 0, 4);
	gameSys.setAnimation(0, 0, 5);
	return _vm->isFlag(kGFUnk23) ? 0xA2 : 0xA1;
}

void Scene45::updateHotspots() {
	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_DISABLED);
		_vm->setHotspot(kHSUfoExitLeft, 0, 0, 10, 599, SF_EXIT_L_CURSOR);
		_vm->setHotspot(kHSUfoExitRight, 794, 0, 799, 599, SF_EXIT_R_CURSOR | SF_DISABLED);
		_vm->setDeviceHotspot(kHSUfoDevice, -1, 534, -1, 599);
		_vm->_hotspotsCount = 4;
	} else {
		_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHSExitUfoParty, 150, 580, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
		_vm->setHotspot(kHSExitShoe, 0, 100, 10, 599, SF_EXIT_L_CURSOR, 0, 8);
		_vm->setHotspot(kHSExitRight, 794, 100, 799, 599, SF_EXIT_R_CURSOR | SF_DISABLED, 10, 8);
		_vm->setHotspot(kHSExitDiscoBall, 200, 0, 600, 10, SF_DISABLED);
		_vm->setHotspot(kHSDiscoBall, 370, 10, 470, 125, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 7);
		_vm->setHotspot(kHSWalkArea1, 0, 0, 800, 472);
		_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
		if (_vm->isFlag(kGFUnk22)) {
			_vm->_hotspots[kHSPlatypus]._flags = SF_DISABLED;
			_vm->_hotspots[kHSExitUfoParty]._flags = SF_DISABLED;
			_vm->_hotspots[kHSExitShoe]._flags = SF_DISABLED;
			_vm->_hotspots[kHSExitRight]._flags = SF_DISABLED;
			_vm->_hotspots[kHSExitDiscoBall]._flags = SF_EXIT_U_CURSOR;
		}
		if (_vm->isFlag(kGFUnk23) || _vm->isFlag(kGFUnk22))
			_vm->_hotspots[kHSDiscoBall]._flags = SF_DISABLED;
		_vm->_hotspotsCount = 8;
	}
}

void Scene45::run() {
	GameSys gameSys = *_vm->_gameSys;

	if (!_vm->isSoundPlaying(0x1094A))
		_vm->playSound(0x1094A, true);
	
	_vm->queueInsertDeviceIcon();

	gameSys.insertSequence(0x96, 1, 0, 0, kSeqNone, 0, 0, 0);
	gameSys.setAnimation(0x96, 1, 3);
	gameSys.insertSequence(0x99, 1, 0, 0, kSeqNone, 0, 0, 0);
	gameSys.setAnimation(0x99, 1, 4);
	_currDancerSequenceId = 0x8F;
	gameSys.setAnimation(_currDancerSequenceId, 1, 2);
	gameSys.insertSequence(_currDancerSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->_toyUfoId = 0;
		_vm->_toyUfoActionStatus = -1;
		_vm->_toyUfoSequenceId = _vm->toyUfoGetSequenceId();
		_vm->_toyUfoNextSequenceId = _vm->_toyUfoSequenceId;
		if (_vm->_prevSceneNum == 46)
			_vm->_toyUfoX = 30;
		else
			_vm->_toyUfoX = 770;
		gameSys.setAnimation(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 5);
		gameSys.insertSequence(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 0, 0, kSeqNone, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
		_vm->endSceneInit();
	} else if (_vm->isFlag(kGFUnk22)) {
		_vm->_gnapSequenceId = 0x9E;
		_vm->_gnapSequenceDatNum = 0;
		_vm->_gnapId = 1;
		gameSys.setAnimation(0x9E, 1, 0);
		_vm->_gnapActionStatus = 1;
		gameSys.insertSequence(_vm->_gnapSequenceId, _vm->_gnapId, 0, 0, kSeqNone, 0, 0, 0);
		_vm->initPlatypusPos(4, 8, kDirNone);
		_vm->endSceneInit();
	} else if (_vm->_prevSceneNum == 46) {
		_vm->initGnapPos(-1, 8, kDirUpRight);
		_vm->initPlatypusPos(-1, 9, kDirUpLeft);
		_vm->endSceneInit();
		_vm->platypusWalkTo(4, 8, -1, 0x107C2, 1);
		_vm->gnapWalkTo(2, 7, -1, 0x107B9, 1);
	} else if (_vm->_prevSceneNum == 41) {
		_vm->initGnapPos(11, 8, kDirUpRight);
		_vm->initPlatypusPos(11, 9, kDirUpLeft);
		_vm->endSceneInit();
		_vm->platypusWalkTo(4, 8, -1, 0x107D2, 1);
		_vm->gnapWalkTo(10, 9, -1, 0x107BA, 1);
	} else {
		_vm->initGnapPos(2, 11, kDirUpRight);
		_vm->initPlatypusPos(6, 11, kDirUpLeft);
		_vm->endSceneInit();
		_vm->platypusWalkTo(4, 8, -1, 0x107C2, 1);
		_vm->gnapWalkTo(2, 7, -1, 0x107B9, 1);
	}
	
	if (!_vm->isFlag(kGFUnk21) && !_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->setFlag(kGFUnk21);
		_vm->setGrabCursorSprite(-1);
		gameSys.setAnimation(0x9D, _vm->_gnapId, 0);
		gameSys.insertSequence(0x9D, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
		while (gameSys.getAnimationStatus(0) != 2) {
			_vm->gameUpdateTick();
			if (gameSys.getAnimationStatus(2) == 2) {
				gameSys.setAnimation(0, 0, 2);
				int newSeqId = _vm->getRandom(7) + 0x8F;
				gameSys.insertSequence(newSeqId, 1, _currDancerSequenceId, 1, kSeqSyncWait, 0, 0, 0);
				gameSys.setAnimation(newSeqId, 1, 2);
				_currDancerSequenceId = newSeqId;
			}
			if (gameSys.getAnimationStatus(3) == 2 && gameSys.getAnimationStatus(4) == 2) {
				gameSys.insertSequence(0x96, 1, 0x96, 1, kSeqSyncWait, 0, 0, 0);
				gameSys.setAnimation(0x96, 1, 3);
				gameSys.insertSequence(0x99, 1, 0x99, 1, kSeqSyncWait, 0, 0, 0);
				gameSys.setAnimation(0x99, 1, 4);
			}
		}
		_vm->_gnapSequenceId = 0x9D;
		_vm->_gnapSequenceDatNum = 0;
		_vm->hideCursor();
		_vm->addFullScreenSprite(0x8A, 255);
		gameSys.setAnimation(0xA0, 256, 0);
		gameSys.insertSequence(0xA0, 256, 0, 0, kSeqNone, 0, 0, 0);
		while (gameSys.getAnimationStatus(0) != 2)
			_vm->gameUpdateTick();
		gameSys.setAnimation(0x107BD, _vm->_gnapId, 0);
		gameSys.insertSequence(0x107BD, _vm->_gnapId,
			makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId,
			kSeqSyncWait, 0, 75 * _vm->_gnapX - _vm->_gnapGridX, 48 * _vm->_gnapY - _vm->_gnapGridY);
		_vm->removeFullScreenSprite();
		_vm->showCursor();
		_vm->_gnapSequenceId = 0x7BD;
		_vm->_gnapSequenceDatNum = 1;
	}
	
	_vm->playPlatypusSequence(0x9A);
	gameSys.setAnimation(_vm->_platypusSequenceId, _vm->_platypusId, 1);
	
	while (!_vm->_sceneDone) {
		if (!_vm->isSoundPlaying(0x1094A))
			_vm->playSound(0x1094A, true);
	
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
					_vm->_toyUfoActionStatus = 2;
					_vm->_newSceneNum = 46;
					_vm->toyUfoFlyTo(-35, -1, -35, 799, 0, 300, 5);
				}
				break;

			case kHSUfoExitRight:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = 2;
					_vm->_newSceneNum = 41;
					_vm->toyUfoFlyTo(835, -1, 0, 835, 0, 300, 5);
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
							_vm->playPlatypusSequence(0x9A);
							gameSys.setAnimation(_vm->_platypusSequenceId, _vm->_platypusId, 1);
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

			case kHSExitUfoParty:
				if (_vm->_gnapActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->gnapWalkTo(_vm->_gnapX, _vm->_hotspotsWalkPos[kHSExitUfoParty].y, 0, 0x107AE, 1);
					_vm->_gnapActionStatus = 0;
					_vm->_newSceneNum = 40;
				}
				break;
			
			case kHSExitShoe:
				if (_vm->_gnapActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitShoe].x, _vm->_gnapY, 0, 0x107AF, 1);
					_vm->_gnapActionStatus = 0;
					_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitShoe].x, _vm->_platY, -1, 0x107CF, 1);
					_vm->_newSceneNum = 46;
				}
				break;
			
			case kHSExitRight:
				if (_vm->_gnapActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitRight].x, _vm->_gnapY, 0, 0x107AB, 1);
					_vm->_gnapActionStatus = 0;
					_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitRight].x, _vm->_platY, -1, 0x107CD, 1);
					_vm->_newSceneNum = 41;
				}
				break;

			case kHSExitDiscoBall:
				_vm->clearFlag(kGFUnk22);
				_vm->setFlag(kGFUnk23);
				_vm->_sceneDone = true;
				_vm->_newSceneNum = 54;
				break;

			case kHSDiscoBall:
				if (_vm->_gnapActionStatus < 0) {
					if (_vm->_grabCursorSpriteIndex == kItemSpring) {
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSDiscoBall].x, _vm->_hotspotsWalkPos[kHSDiscoBall].y, 0, 0x9F, 5);
						_vm->_gnapActionStatus = 1;
						_vm->setGrabCursorSprite(-1);
						_vm->invRemove(kItemSpring);
					} else if (_vm->_grabCursorSpriteIndex >= 0) {
						_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, 5, 0);
					} else {
						switch (_vm->_verbCursor) {
						case LOOK_CURSOR:
							_vm->playGnapScratchingHead(5, 0);
							break;
						case GRAB_CURSOR:
						case TALK_CURSOR:
						case PLAT_CURSOR:
							_vm->playGnapImpossible(0, 0);
							break;
						}
					}
				}
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
				_vm->_toyUfoActionStatus = 3;
				_vm->toyUfoFlyTo(-1, -1, 0, 799, 0, 300, 5);
			} else {
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			}
		}
	
		updateAnimations();
		_vm->toyUfoCheckTimer();
	
		if (!_vm->_isLeavingScene && _vm->_gnapActionStatus < 0 && !_vm->isFlag(kGFGnapControlsToyUFO))
			_vm->updateGnapIdleSequence();
	
		_vm->checkGameKeys();
	
		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
		}
	
		_vm->gameUpdateTick();
	}

	_vm->_sceneWaiting = false;
}

void Scene45::updateAnimations() {
	GameSys gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case 0:
			_vm->_sceneDone = true;
			break;
		case 1:
			_vm->_sceneWaiting = true;
			_vm->setFlag(kGFUnk22);
			updateHotspots();
			gameSys.insertSequence(0x9E, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceId = 0x9E;
			_vm->_gnapSequenceDatNum = 0;
			gameSys.setAnimation(0x9E, _vm->_gnapId, 0);
			break;
		default:
			_vm->_gnapActionStatus = -1;
			break;
		}
	}
	
	if (gameSys.getAnimationStatus(1) == 2) {
		gameSys.setAnimation(0, 0, 1);
		if (_vm->getRandom(2) != 0)
			_vm->playPlatypusSequence(0x9B);
		else
			_vm->playPlatypusSequence(0x9C);
		gameSys.setAnimation(_vm->_platypusSequenceId, _vm->_platypusId, 1);
	}
	
	if (gameSys.getAnimationStatus(2) == 2) {
		gameSys.setAnimation(0, 0, 2);
		int newSeqId = _vm->getRandom(7) + 0x8F;
		gameSys.insertSequence(newSeqId, 1, _currDancerSequenceId, 1, kSeqSyncWait, 0, 0, 0);
		gameSys.setAnimation(newSeqId, 1, 2);
		_currDancerSequenceId = newSeqId;
	}
	
	if (gameSys.getAnimationStatus(3) == 2 && gameSys.getAnimationStatus(4) == 2) {
		gameSys.insertSequence(0x96, 1, 0x96, 1, kSeqSyncWait, 0, 0, 0);
		gameSys.setAnimation(0x96, 1, 3);
		gameSys.insertSequence(0x99, 1, 0x99, 1, kSeqSyncWait, 0, 0, 0);
		gameSys.setAnimation(0x99, 1, 4);
	}
	
	if (gameSys.getAnimationStatus(5) == 2) {
		switch (_vm->_toyUfoActionStatus) {
		case 2:
			_vm->_sceneDone = true;
			break;
		default:
			_vm->_toyUfoNextSequenceId = _vm->toyUfoGetSequenceId();
			gameSys.insertSequence(_vm->_toyUfoNextSequenceId | 0x10000, _vm->_toyUfoId + 1,
				_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId,
				kSeqSyncWait, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
			_vm->_toyUfoSequenceId = _vm->_toyUfoNextSequenceId;
			++_vm->_toyUfoId;
			gameSys.setAnimation(_vm->_toyUfoNextSequenceId | 0x10000, _vm->_toyUfoId, 5);
			break;
		}
		_vm->_toyUfoActionStatus = -1;
	}
}

} // End of namespace Gnap
