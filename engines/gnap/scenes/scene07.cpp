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

#include "gnap/scenes/scene07.h"

namespace Gnap {

enum {
	kHS07Platypus	= 0,
	kHS07ExitHouse	= 1,
	kHS07Dice		= 2,
	kHS07Device		= 3,
	kHS07WalkArea1	= 4,
	kHS07WalkArea2	= 5,
	kHS07WalkArea3	= 6
};

enum {
	kAS07Wait		= 0,
	kAS07LeaveScene	= 1
};

Scene07::Scene07(GnapEngine *vm) : Scene(vm) {
}

int Scene07::init() {
	return 0x92;
}

void Scene07::updateHotspots() {
	_vm->setHotspot(kHS07Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS07ExitHouse, 700, 125, 799, 290, SF_EXIT_NE_CURSOR);
	_vm->setHotspot(kHS07Dice, 200, 290, 270, 360, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS07WalkArea1, 0, 0, 325, 445);
	_vm->setHotspot(kHS07WalkArea2, 325, 0, 799, 445, _vm->_isLeavingScene ? SF_WALKABLE : SF_NONE);
	_vm->setHotspot(kHS07WalkArea3, 160, 0, 325, 495);
	_vm->setDeviceHotspot(kHS07Device, -1, -1, -1, -1);
	if (_vm->isFlag(kGFPlatypus))
		_vm->_hotspots[kHS07Dice]._flags = SF_DISABLED;
	_vm->_hotspotsCount = 7;
}

void Scene07::run() {
	_vm->queueInsertDeviceIcon();
	_vm->_gameSys->insertSequence(0x8C, 1, 0, 0, kSeqLoop, 0, 0, 0);
	_vm->_gameSys->insertSequence(0x90, 1, 0, 0, kSeqLoop, 0, 0, 0);

	_vm->invRemove(kItemGas);
	_vm->invRemove(kItemNeedle);
	
	if (!_vm->isFlag(kGFPlatypus))
		_vm->_gameSys->insertSequence(0x8D, 1, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->_prevSceneNum == 8) {
		_vm->initGnapPos(7, 7, kDirBottomLeft);
		_vm->initPlatypusPos(9, 7, kDirUnk4);
		_vm->endSceneInit();
	} else {
		_vm->_gnapX = 6;
		_vm->_gnapY = 7;
		_vm->_gnapId = 140;
		_vm->_gnapSequenceId = 0x8F;
		_vm->_gnapSequenceDatNum = 0;
		_vm->_gnapIdleFacing = kDirBottomRight;
		_vm->_gameSys->insertSequence(0x8F, 140, 0, 0, kSeqNone, 0, 0, 0);
		_vm->_gameSys->setAnimation(makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, 0);
		_vm->_gnapActionStatus = kAS07Wait;
		_vm->_platX = 3;
		_vm->_platY = 8;
		_vm->_platypusId = 160;
		_vm->_platypusSequenceId = 0x91;
		_vm->_platypusSequenceDatNum = 0;
		_vm->_platypusFacing = kDirNone;
		_vm->_gameSys->insertSequence(0x91, 160, 0, 0, kSeqNone, 0, 0, 0);
		_vm->endSceneInit();
	}

	_vm->_timers[3] = 600;
	_vm->_timers[4] = _vm->getRandom(40) + 50;

	while (!_vm->_sceneDone) {
		if (!_vm->isSoundPlaying(0x10919))
			_vm->playSound(0x10919, true);

		if (_vm->testWalk(0, 1, 8, 7, 6, 7))
			updateHotspots();

		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS07Platypus:
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
			break;

		case kHS07ExitHouse:
			_vm->_isLeavingScene = true;
			if (_vm->_gnapX > 8)
				_vm->gnapWalkTo(_vm->_gnapX, 7, 0, 0x107AD, 1);
			else
				_vm->gnapWalkTo(8, 7, 0, 0x107AD, 1);
			_vm->_gnapActionStatus = kAS07LeaveScene;
			break;

		case kHS07Dice:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapShowCurrItem(4, 8, 3, 3);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					break;
				case GRAB_CURSOR:
					_vm->setFlag(kGFPlatypus);
					_vm->invAdd(kItemDice);
					updateHotspots();
					_vm->playGnapPullOutDevice(3, 3);
					_vm->_gameSys->setAnimation(0x8E, 1, 2);
					_vm->_gameSys->insertSequence(0x8E, 1, 141, 1, kSeqSyncWait, 0, 0, 0);
					_vm->_gameSys->insertSequence(_vm->getGnapSequenceId(gskUseDevice, 0, 0) | 0x10000, _vm->_gnapId,
						makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId,
						kSeqSyncWait, 0, 75 * _vm->_gnapX - _vm->_gnapGridX, 48 * _vm->_gnapY - _vm->_gnapGridY);
					_vm->_gnapSequenceId = _vm->getGnapSequenceId(gskUseDevice, 0, 0);
					_vm->_gnapSequenceDatNum = 1;
					break;
				case TALK_CURSOR:
				case PLAT_CURSOR:
					_vm->playGnapImpossible(0, 0);
					break;
				}
			}
			break;

		case kHS07Device:
			if (_vm->_gnapActionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(40) + 50;
			}
			break;

		case kHS07WalkArea1:
		case kHS07WalkArea2:
			_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			break;
			
		case kHS07WalkArea3:
			// Nothing
			break;

		default:
			if (_vm->_mouseClickState._left) {
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				_vm->_mouseClickState._left = false;
			}
			break;
		}

		updateAnimations();

		if (!_vm->_isLeavingScene) {
			_vm->updateGnapIdleSequence();
			if (_vm->_platypusActionStatus < 0 && _vm->_gnapActionStatus < 0) {
				if (_vm->_timers[0]) {
					if (!_vm->_timers[1]) {
						_vm->_timers[1] = _vm->getRandom(20) + 30;
						int gnapRandomValue = _vm->getRandom(20);
						// TODO Cleanup
						if (_vm->_platypusFacing != kDirNone) {
							if (gnapRandomValue != 0 || _vm->_platypusSequenceId != 0x7CA) {
								if (gnapRandomValue != 1 || _vm->_platypusSequenceId != 0x7CA) {
									if (_vm->_platY == 9)
										_vm->playPlatypusSequence(0x107CA);
								} else {
									_vm->playPlatypusSequence(0x10845);
								}
							} else {
								_vm->playPlatypusSequence(0x107CC);
							}
						} else if (gnapRandomValue != 0 || _vm->_platypusSequenceId != 0x7C9) {
							if (gnapRandomValue != 1 || _vm->_platypusSequenceId != 0x7C9) {
								if (_vm->_platY == 9)
									_vm->playPlatypusSequence(0x107C9);
							} else {
								_vm->playPlatypusSequence(0x10844);
							}
						} else {
							_vm->playPlatypusSequence(0x107CB);
						}
						_vm->_gameSys->setAnimation(_vm->_platypusSequenceId | (_vm->_platypusSequenceDatNum << 16), _vm->_platypusId, 1);
					}
				} else {
					_vm->_timers[0] = _vm->getRandom(75) + 75;
					_vm->platypusMakeRoom();
				}
			} else {
				_vm->_timers[0] = 100;
				_vm->_timers[1] = 35;
			}
			playRandomSound(4);
		}

		_vm->checkGameKeys();

		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[4] = _vm->getRandom(40) + 50;
		}
		_vm->gameUpdateTick();
	}
}

void Scene07::updateAnimations() {
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kAS07LeaveScene:
			_vm->_newSceneNum = 8;
			_vm->_sceneDone = true;
			break;
		}
		_vm->_gnapActionStatus = -1;
	}

	if (_vm->_gameSys->getAnimationStatus(2) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 2);
		_vm->setGrabCursorSprite(kItemDice);
	}
}

} // End of namespace Gnap
