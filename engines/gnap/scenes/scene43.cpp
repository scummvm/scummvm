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
#include "gnap/scenes/scene43.h"

namespace Gnap {

enum {
	kHSPlatypus			= 0,
	kHSDevice			= 1,
	kHSExitUfoParty		= 2,
	kHSExitBBQ			= 3,
	kHSExitKissinBooth	= 4,
	kHSTwoHeadedGuy		= 5,
	kHSKey				= 6,
	kHSUfo				= 7,
	kHSWalkArea1		= 8,
	kHSWalkArea2		= 9
};

enum {
	kHSUfoExitLeft	= 1,
	kHSUfoExitRight	= 2,
	kHSUfoKey		= 3,
	kHSUfoBucket	= 4,
	kHSUfoDevice	= 5
};

Scene43::Scene43(GnapEngine *vm) : Scene(vm) {
	_s43_currTwoHeadedGuySequenceId = -1;
	_s43_nextTwoHeadedGuySequenceId = -1;
}

int Scene43::init() {
	_vm->_gameSys->setAnimation(0, 0, 0);
	_vm->_gameSys->setAnimation(0, 0, 1);
	_vm->_gameSys->setAnimation(0, 0, 2);
	return 0x13F;
}

void Scene43::updateHotspots() {
	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_DISABLED);
		_vm->setHotspot(kHSUfoExitLeft, 0, 0, 10, 599, SF_EXIT_L_CURSOR);
		_vm->setHotspot(kHSUfoExitRight, 790, 0, 799, 599, SF_EXIT_R_CURSOR);
		_vm->setHotspot(kHSUfoKey, 140, 170, 185, 260, SF_GRAB_CURSOR);
		_vm->setHotspot(kHSUfoBucket, 475, 290, 545, 365, SF_DISABLED);
		_vm->setDeviceHotspot(kHSUfoDevice, -1, 534, -1, 599);
		if (_vm->isFlag(kGFGroceryStoreHatTaken))
			_vm->_hotspots[kHSUfoBucket]._flags = SF_GRAB_CURSOR;
		// NOTE Bug in the original. Key hotspot wasn't disabled.
		if (_vm->isFlag(kGFUnk14))
			_vm->_hotspots[kHSUfoKey]._flags = SF_DISABLED;
		_vm->_hotspotsCount = 6;
	} else {
		_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHSExitUfoParty, 150, 580, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
		_vm->setHotspot(kHSExitBBQ, 0, 100, 10, 599, SF_EXIT_L_CURSOR, 0, 8);
		_vm->setHotspot(kHSExitKissinBooth, 790, 100, 799, 599, SF_EXIT_R_CURSOR, 10, 8);
		_vm->setHotspot(kHSTwoHeadedGuy, 470, 240, 700, 470, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHSKey, 140, 170, 185, 260, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHSUfo, 110, 0, 690, 350, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHSWalkArea1, 0, 0, 800, 445);
		_vm->setHotspot(kHSWalkArea2, 465, 0, 800, 493);
		_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
        if (_vm->isFlag(kGFUnk14))
			_vm->_hotspots[kHSKey]._flags = SF_DISABLED;
		_vm->_hotspotsCount = 10;
	}
}

void Scene43::run() {
	_vm->queueInsertDeviceIcon();

	if (!_vm->isFlag(kGFUnk14))
		_vm->_gameSys->insertSequence(0x1086F, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	_s43_currTwoHeadedGuySequenceId = 0x13C;
	_s43_nextTwoHeadedGuySequenceId = -1;
	
	_vm->_gameSys->setAnimation(0x13C, 1, 2);
	_vm->_gameSys->insertSequence(_s43_currTwoHeadedGuySequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->_toyUfoId = 0;
		_vm->_toyUfoActionStatus = -1;
		_vm->_toyUfoSequenceId = _vm->toyUfoGetSequenceId();
		_vm->_toyUfoNextSequenceId = _vm->_toyUfoSequenceId;
		if (_vm->_prevSceneNum == 42)
			_vm->_toyUfoX = 30;
		else
			_vm->_toyUfoX = 770;
		_vm->_gameSys->setAnimation(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 3);
		_vm->_gameSys->insertSequence(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 0, 0, kSeqNone, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
		_vm->endSceneInit();
	} else {
		switch (_vm->_prevSceneNum) {
		case 42:
			_vm->initGnapPos(-1, 8, kDirUpRight);
			_vm->initPlatypusPos(-1, 9, kDirUpLeft);
			_vm->endSceneInit();
			_vm->gnapWalkTo(2, 8, -1, 0x107B9, 1);
			_vm->platypusWalkTo(1, 8, -1, 0x107C2, 1);
			break;
		case 44:
			_vm->initGnapPos(11, 8, kDirUpRight);
			_vm->initPlatypusPos(11, 9, kDirUpLeft);
			_vm->endSceneInit();
			_vm->gnapWalkTo(8, 8, -1, 0x107BA, 1);
			_vm->platypusWalkTo(9, 8, -1, 0x107D2, 1);
			break;
		case 54:
			_vm->initGnapPos(4, 7, kDirBottomLeft);
			_vm->initPlatypusPos(11, 8, kDirUpLeft);
			_vm->endSceneInit();
			_vm->platypusWalkTo(9, 8, -1, 0x107D2, 1);
			break;
		default:
			_vm->initGnapPos(5, 11, kDirUpRight);
			_vm->initPlatypusPos(6, 11, kDirUpLeft);
			_vm->endSceneInit();
			_vm->gnapWalkTo(5, 8, -1, 0x107BA, 1);
			_vm->platypusWalkTo(6, 8, -1, 0x107C2, 1);
			break;
		}
	}
	
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
			case kHSUfoDevice:
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(100) + 100;
				break;
			
			case kHSUfoExitLeft:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = 4;
					_vm->_newSceneNum = 42;
					_vm->toyUfoFlyTo(-35, -1, -35, 799, 0, 300, 3);
				}
				break;
			
			case kHSUfoExitRight:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = 4;
					_vm->_newSceneNum = 44;
					_vm->toyUfoFlyTo(835, -1, 0, 835, 0, 300, 3);
				}
				break;
			
			case kHSUfoKey:
				if (_vm->isFlag(kGFJointTaken)) {
					_vm->_toyUfoActionStatus = 6;
					_vm->toyUfoFlyTo(163, 145, 0, 799, 0, 300, 3);
				} else {
					_vm->_toyUfoActionStatus = 5;
					_vm->toyUfoFlyTo(-1, -1, 0, 799, 0, 300, 3);
				}
				break;
			
			case kHSUfoBucket:
				_vm->_toyUfoActionStatus = 7;
				_vm->toyUfoFlyTo(497, 143, 0, 799, 0, 300, 3);
				_vm->_timers[9] = 600;
				break;
			}
		} else {
			switch (_vm->_sceneClickedHotspot) {
			case kHSDevice:
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(100) + 100;
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

			case kHSExitUfoParty:
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitUfoParty].x, _vm->_hotspotsWalkPos[kHSExitUfoParty].y, 0, 0x107AE, 1);
				_vm->_gnapActionStatus = 0;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitUfoParty].x, _vm->_hotspotsWalkPos[kHSExitUfoParty].y, -1, 0x107C7, 1);
				_vm->_newSceneNum = 40;
				break;

			case kHSExitBBQ:
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitBBQ].x, _vm->_gnapY, 0, 0x107AF, 1);
				_vm->_gnapActionStatus = 0;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitBBQ].x, _vm->_hotspotsWalkPos[kHSExitBBQ].y, -1, 0x107CF, 1);
				_vm->_newSceneNum = 42;
				break;

			case kHSExitKissinBooth:
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitKissinBooth].x, _vm->_gnapY, 0, 0x107AB, 1);
				_vm->_gnapActionStatus = 0;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitKissinBooth].x, _vm->_hotspotsWalkPos[kHSExitKissinBooth].y, -1, 0x107CD, 1);
				_vm->_newSceneNum = 44;
				break;

			case kHSTwoHeadedGuy:
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(6, 8, 7, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(7, 0);
						break;
					case TALK_CURSOR:
							_vm->_gnapIdleFacing = kDirUpRight;
							_vm->gnapWalkTo(5, 8, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
							_vm->_gnapActionStatus = 2;
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
				break;

			case kHSKey:
			case kHSUfo:
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(3, 7, 2, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(0, 0);
						break;
					case GRAB_CURSOR:
						_vm->_gnapIdleFacing = kDirUpRight;
						_vm->gnapWalkTo(3, 7, 0, 67515, 1);
						_vm->_gnapActionStatus = 1;
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
				if (_vm->_gnapActionStatus < 0)
					_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				break;
			}
		}
	
		if (_vm->_mouseClickState._left && _vm->_gnapActionStatus < 0) {
			_vm->_mouseClickState._left = false;
			if (_vm->isFlag(kGFGnapControlsToyUFO) && (_vm->_toyUfoActionStatus == 5 || _vm->_toyUfoActionStatus == -1)) {
				_vm->_toyUfoActionStatus = 5;
				_vm->toyUfoFlyTo(-1, -1, 0, 799, 0, 300, 3);
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
			if (!_vm->_timers[4] && (!_vm->isFlag(kGFGnapControlsToyUFO) || !_vm->isFlag(kGFGroceryStoreHatTaken))) {
				_vm->_timers[4] = _vm->getRandom(100) + 100;
				if (_vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0 && _s43_nextTwoHeadedGuySequenceId == -1) {
					switch (_vm->getRandom(5)) {
					case 0:
						_s43_nextTwoHeadedGuySequenceId = 0x13C;
						break;
					case 1:
						_s43_nextTwoHeadedGuySequenceId = 0x134;
						break;
					case 2:
						_s43_nextTwoHeadedGuySequenceId = 0x135;
						break;
					case 3:
						_s43_nextTwoHeadedGuySequenceId = 0x136;
						break;
					case 4:
						_s43_nextTwoHeadedGuySequenceId = 0x13A;
						break;
					}
					if (_s43_nextTwoHeadedGuySequenceId == _s43_currTwoHeadedGuySequenceId)
						_s43_nextTwoHeadedGuySequenceId = -1;
				}
			}
		}
	
		_vm->checkGameKeys();
	
		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[4] = _vm->getRandom(100) + 100;
		}
		
		_vm->gameUpdateTick();
	}
	
	if (_vm->_newSceneNum == 54)
		_vm->clearFlag(kGFGnapControlsToyUFO);
}

void Scene43::updateAnimations() {
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		switch (_vm->_gnapActionStatus) {
		case 0:
			_vm->_gameSys->setAnimation(0, 0, 0);
			_vm->_sceneDone = true;
			break;

		case 1:
			if (_vm->_gameSys->getAnimationStatus(2) == 2) {
				_vm->_timers[2] = _vm->getRandom(30) + 20;
				_vm->_timers[3] = _vm->getRandom(50) + 200;
				_vm->_gameSys->insertSequence(0x13D, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
				_vm->_gnapSequenceId = 0x13D;
				_vm->_gnapSequenceDatNum = 0;
				_vm->_gameSys->setAnimation(0x13D, _vm->_gnapId, 0);
				_s43_nextTwoHeadedGuySequenceId = 0x13B;
				_vm->_gameSys->insertSequence(0x13B, 1, _s43_currTwoHeadedGuySequenceId, 1, kSeqSyncWait, 0, 0, 0);
				_vm->_gameSys->setAnimation(_s43_nextTwoHeadedGuySequenceId, 1, 2);
				_s43_currTwoHeadedGuySequenceId = _s43_nextTwoHeadedGuySequenceId;
				_s43_nextTwoHeadedGuySequenceId = -1;
				_vm->_timers[4] = _vm->getRandom(100) + 100;
				_vm->_gnapActionStatus = -1;
			}
			break;

		default:
			_vm->_gameSys->setAnimation(0, 0, 0);
			_vm->_gnapActionStatus = -1;
			break;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(2) == 2) {
		if (_s43_currTwoHeadedGuySequenceId == 0x13A) {
			if (_vm->isFlag(kGFGroceryStoreHatTaken)) {
				_s43_nextTwoHeadedGuySequenceId = 0x13E;
				_vm->stopSound(0x108F6);
			} else if (_vm->getRandom(2) != 0) {
				_s43_nextTwoHeadedGuySequenceId = 0x137;
			} else {
				_s43_nextTwoHeadedGuySequenceId = 0x138;
			}
		} else if (_s43_currTwoHeadedGuySequenceId == 0x13E) {
			_vm->_sceneDone = true;
			_vm->_newSceneNum = 54;
		}
		if (_s43_nextTwoHeadedGuySequenceId != -1) {
			_vm->_gameSys->insertSequence(_s43_nextTwoHeadedGuySequenceId, 1, _s43_currTwoHeadedGuySequenceId, 1, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(_s43_nextTwoHeadedGuySequenceId, 1, 2);
			_s43_currTwoHeadedGuySequenceId = _s43_nextTwoHeadedGuySequenceId;
			_s43_nextTwoHeadedGuySequenceId = -1;
			_vm->_timers[4] = _vm->getRandom(100) + 100;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(3) == 2) {
		switch (_vm->_toyUfoActionStatus) {
		case 4:
			_vm->_sceneDone = true;
			_vm->_toyUfoActionStatus = -1;
			break;
		case 6:
			_vm->_gameSys->insertSequence(0x10871, _vm->_toyUfoId, _vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->removeSequence(0x1086F, 1, true);
			_vm->setFlag(kGFUnk14);
			updateHotspots();
			_vm->toyUfoSetStatus(kGFUnk18);
			_vm->_toyUfoSequenceId = 0x871;
			_vm->_gameSys->setAnimation(0x10871, _vm->_toyUfoId, 3);
			_vm->_toyUfoActionStatus = -1;
			_vm->_toyUfoX = 96;
			_vm->_toyUfoY = 131;
			break;
		case 7:
			_vm->_gameSys->insertSequence(0x10874, _vm->_toyUfoId, _vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, kSeqSyncWait, 0, 0, 0);
			_vm->_toyUfoSequenceId = 0x874;
			_vm->_gameSys->setAnimation(0x10874, _vm->_toyUfoId, 3);
			_vm->_toyUfoActionStatus = 8;
			_vm->setFlag(kGFJointTaken);
			_vm->_gnapActionStatus = 3;
			break;
		case 8:
			_s43_nextTwoHeadedGuySequenceId = 0x13A;
			_vm->_toyUfoX = 514;
			_vm->_toyUfoY = 125;
			_vm->toyUfoFlyTo(835, 125, 0, 835, 0, 300, 3);
			_vm->_toyUfoActionStatus = 9;
			break;
		case 9:
			// Nothing
			break;
		default:
			_vm->_toyUfoNextSequenceId = _vm->toyUfoGetSequenceId();
			_vm->_gameSys->insertSequence(_vm->_toyUfoNextSequenceId | 0x10000, _vm->_toyUfoId + 1,
				_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId,
				kSeqSyncWait, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
			_vm->_toyUfoSequenceId = _vm->_toyUfoNextSequenceId;
			++_vm->_toyUfoId;
			_vm->_gameSys->setAnimation(_vm->_toyUfoNextSequenceId | 0x10000, _vm->_toyUfoId, 3);
			_vm->_toyUfoActionStatus = -1;
			break;
		}
	}
}

} // End of namespace Gnap
