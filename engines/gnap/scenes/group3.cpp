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
#include "gnap/scenes/group3.h"

namespace Gnap {

Scene30::Scene30(GnapEngine *vm) : Scene(vm) {
	_kidSequenceId = -1;
}

int Scene30::init() {
	return _vm->isFlag(kGFUnk23) ? 0x10B : 0x10A;
}

void Scene30::updateHotspots() {
	_vm->setHotspot(kHS30Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS30PillMachine, 598, 342, 658, 426, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 9, 7);
	_vm->setHotspot(kHS30ExitCircus, 100, 590 - _vm->_deviceY1, 700, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
	_vm->setHotspot(kHS30WalkArea1, 0, 0, 800, 514);
	_vm->setDeviceHotspot(kHS30Device, -1, -1, -1, -1);
	_vm->_hotspotsCount = 5;
}

void Scene30::run() {
	GameSys& gameSys = *_vm->_gameSys;
	bool hasTakenPill = false;

	_vm->playSound(0x1093B, true);
	_vm->startSoundTimerB(6);

	_vm->queueInsertDeviceIcon();

	if (_vm->isFlag(kGFUnk23))
		gameSys.insertSequence(0x106, 1, 0, 0, kSeqNone, 0, 0, 0);

	if (!_vm->isFlag(kGFUnk13))
		gameSys.insertSequence(0x107, 1, 0, 0, kSeqNone, 0, 0, 0);
	_vm->_timers[5] = _vm->getRandom(50) + 180;

	gameSys.insertSequence(0x101, 40, 0, 0, kSeqNone, 0, 0, 0);
	_vm->_timers[4] = _vm->getRandom(100) + 300;

	_kidSequenceId = 0x101;
	_vm->initGnapPos(7, 12, kDirBottomRight);
	_vm->initPlatypusPos(6, 12, kDirNone);
	_vm->endSceneInit();
	_vm->gnapWalkTo(7, 8, -1, 0x107B9, 1);
	_vm->platypusWalkTo(6, 8, -1, 0x107C2, 1);

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS30Device:
			if (_vm->_gnapActionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS30Platypus:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemJoint) {
					_vm->gnapUseJointOnPlatypus();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
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
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS30PillMachine:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemDiceQuarterHole && !_vm->isFlag(kGFUnk23)) {
					_vm->_hotspots[kHS30WalkArea1]._flags |= SF_WALKABLE;
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS30PillMachine].x, _vm->_hotspotsWalkPos[kHS30PillMachine].y, 0, 0x107BC, 1);
					_vm->_hotspots[kHS30WalkArea1]._flags &= ~SF_WALKABLE;
					_vm->_gnapActionStatus = kAS30UsePillMachine;
					hasTakenPill = true;
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHS30PillMachine].x, _vm->_hotspotsWalkPos[kHS30PillMachine].y, 8, 5);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->gnapWalkTo(9, 8, 0, 0x107BC, 1);
						_vm->_gnapActionStatus = kAS30LookPillMachine;
						break;
					case GRAB_CURSOR:
						_vm->playGnapScratchingHead(8, 5);
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(8, 5);
						break;
					}
				}
			}
			break;

		case kHS30ExitCircus:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				if (hasTakenPill)
					_vm->_newSceneNum = 47;
				else
					_vm->_newSceneNum = 26;
				_vm->gnapWalkTo(-1, _vm->_hotspotsWalkPos[kHS30ExitCircus].y, 0, 0x107AE, 1);
				_vm->_gnapActionStatus = kAS30LeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS30ExitCircus].x + 1, _vm->_hotspotsWalkPos[kHS30ExitCircus].y, -1, 0x107C2, 1);
			}
			break;

		case kHS30WalkArea1:
			if (_vm->_gnapActionStatus < 0)
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			break;

		default:
			if (_vm->_mouseClickState._left) {
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				_vm->_mouseClickState._left = false;
			}
			break;
		}

		updateAnimations();

		if (!_vm->isSoundPlaying(0x1093B))
			_vm->playSound(0x1093B, true);

		if (!_vm->_isLeavingScene) {
			_vm->updatePlatypusIdleSequence();
			if (_vm->_gnapActionStatus < 0)
				_vm->updateGnapIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(100) + 300;
				if (_vm->_gnapActionStatus < 0) {
					if (_vm->getRandom(5) == 1) {
						gameSys.insertSequence(0xFF, 40, 0, 0, kSeqNone, 0, 0, 0);
						gameSys.insertSequence(0x100, 40, _kidSequenceId, 40, kSeqSyncWait, 0, 0, 0);
						_kidSequenceId = 0x100;
					} else {
						gameSys.insertSequence(0xFE, 40, 0, 0, kSeqNone, 0, 0, 0);
					}
				}
			}
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(50) + 180;
				if (_vm->_gnapActionStatus < 0) {
					if (!_vm->isFlag(kGFUnk23) || hasTakenPill)
						gameSys.insertSequence(0x109, 20, 0, 0, kSeqNone, 0, 0, 0);
					else
						gameSys.insertSequence(0x108, 20, 0, 0, kSeqNone, 0, 0, 0);
				}
			}
			_vm->playSoundB();
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

void Scene30::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kAS30LeaveScene:
			_vm->_sceneDone = true;
			break;
		case kAS30UsePillMachine:
			_vm->setGrabCursorSprite(-1);
			gameSys.setAnimation(0x105, _vm->_gnapId, 0);
			gameSys.insertSequence(0x105, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceId = 0x105;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapActionStatus = kAS30UsePillMachine2;
			break;
		case kAS30UsePillMachine2:
			_vm->hideCursor();
			_vm->setGrabCursorSprite(-1);
			_vm->addFullScreenSprite(0x3F, 255);
			gameSys.removeSequence(0x105, _vm->_gnapId, true);
			gameSys.setAnimation(0x102, 256, 0);
			gameSys.insertSequence(0x102, 256, 0, 0, kSeqNone, 0, 0, 0);
			while (gameSys.getAnimationStatus(0) != 2)
				_vm->gameUpdateTick();
			gameSys.setAnimation(0x103, _vm->_gnapId, 0);
			gameSys.insertSequence(0x103, _vm->_gnapId, 0, 0, kSeqNone, 0, 0, 0);
			_vm->removeFullScreenSprite();
			_vm->showCursor();
			_vm->_gnapActionStatus = kAS30UsePillMachine3;
			_vm->invAdd(kItemPill);
			_vm->setFlag(kGFUnk23);
			break;
		case kAS30UsePillMachine3:
			gameSys.setAnimation(0x104, _vm->_gnapId, 0);
			gameSys.insertSequence(0x104, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, 0x103), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceId = 0x104;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapActionStatus = kAS30UsePillMachine4;
			_vm->setGrabCursorSprite(kItemDiceQuarterHole);
			break;
		case kAS30UsePillMachine4:
			gameSys.insertSequence(0x106, 1, 0, 0, kSeqNone, 0, 0, 0);
			_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS30PillMachine].x, _vm->_hotspotsWalkPos[kHS30PillMachine].y + 1, -1, 0x107BC, 1);
			_vm->_gnapActionStatus = -1;
			break;
		case kAS30LookPillMachine:
			if (_vm->isFlag(kGFUnk23))
				_vm->showFullScreenSprite(0xE3);
			else
				_vm->showFullScreenSprite(0xE2);
			_vm->_gnapActionStatus = -1;
			break;
		}
	}
}

/*****************************************************************************/

Scene31::Scene31(GnapEngine *vm) : Scene(vm) {
	_beerGuyDistracted = false;
	_currClerkSequenceId = -1;
	_nextClerkSequenceId = -1;
	_clerkMeasureCtr = -1;
	_clerkMeasureMaxCtr = 3;
}

int Scene31::init() {
	return 0x105;
}

void Scene31::updateHotspots() {
	_vm->setHotspot(kHS31Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS31MeasuringClown, 34, 150, 256, 436, SF_WALKABLE | SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 0, 6);
	_vm->setHotspot(kHS31BeerBarrel, 452, 182, 560, 306, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 7);
	_vm->setHotspot(kHS31ExitCircus, 150, 585, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
	_vm->setHotspot(kHS31ExitOutsideClown, 0, 0, 15, 600, SF_EXIT_L_CURSOR | SF_WALKABLE, 0, 8);
	_vm->setHotspot(kHS31WalkArea1, 0, 0, 800, 480);
	_vm->setDeviceHotspot(kHS31Device, -1, -1, -1, -1);
	_vm->_hotspotsCount = 7;
}

void Scene31::run() {
	GameSys& gameSys = *_vm->_gameSys;

	_vm->playSound(0x1093B, true);
	_vm->startSoundTimerB(6);
	_vm->queueInsertDeviceIcon();

	_beerGuyDistracted = false;
	gameSys.insertSequence(0xFB, 39, 0, 0, kSeqNone, 0, 0, 0);

	_currClerkSequenceId = 0xFB;
	_nextClerkSequenceId = -1;

	gameSys.setAnimation(0xFB, 39, 3);

	_vm->_timers[4] = _vm->getRandom(20) + 60;
	_vm->_timers[5] = _vm->getRandom(50) + 180;

	if (_vm->_prevSceneNum == 27) {
		_vm->initGnapPos(-1, 8, kDirBottomLeft);
		_vm->initPlatypusPos(-1, 9, kDirUnk4);
		_vm->endSceneInit();
		_vm->gnapWalkTo(3, 8, -1, 0x107BA, 1);
		_vm->platypusWalkTo(3, 9, -1, 0x107D2, 1);
	} else {
		_vm->initGnapPos(7, 12, kDirBottomRight);
		_vm->initPlatypusPos(6, 12, kDirNone);
		_vm->endSceneInit();
		_vm->gnapWalkTo(7, 8, -1, 0x107BA, 1);
		_vm->platypusWalkTo(6, 8, -1, 0x107D2, 1);
	}

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS31Device:
			if (_vm->_gnapActionStatus < 0 || _vm->_gnapActionStatus == kAS31PlatMeasuringClown) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS31Platypus:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemJoint) {
					_vm->gnapUseJointOnPlatypus();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
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
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS31MeasuringClown:
			if (_vm->_gnapActionStatus < 0 || _vm->_gnapActionStatus == kAS31PlatMeasuringClown) {
				if (_vm->_gnapActionStatus == kAS31PlatMeasuringClown) {
					if (_vm->_verbCursor == LOOK_CURSOR)
						_vm->playGnapScratchingHead(2, 2);
					else
						_vm->playGnapImpossible(0, 0);
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHS31MeasuringClown].x, _vm->_hotspotsWalkPos[kHS31MeasuringClown].y + 1, 2, 2);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(2, 2);
						break;
					case GRAB_CURSOR:
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS31MeasuringClown].x, _vm->_hotspotsWalkPos[kHS31MeasuringClown].y + 1, -1, -1, 1);
						_vm->_hotspots[kHS31WalkArea1]._flags |= SF_WALKABLE;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS31MeasuringClown].x, _vm->_hotspotsWalkPos[kHS31MeasuringClown].y, 0, 0x107B9, 1);
						_vm->_hotspots[kHS31WalkArea1]._flags &= ~SF_WALKABLE;
						_vm->_gnapActionStatus = kAS31UseMeasuringClown;
						_vm->_timers[4] = 300;
						break;
					case TALK_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					case PLAT_CURSOR:
						if (!_vm->invHas(kItemBucketWithBeer)) {
							_vm->gnapUseDeviceOnPlatypus();
							_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS31MeasuringClown].x, _vm->_hotspotsWalkPos[kHS31MeasuringClown].y + 1, 1, 0x107C2, 1);
							_vm->_hotspots[kHS31WalkArea1]._flags |= SF_WALKABLE;
							_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS31MeasuringClown].x, _vm->_hotspotsWalkPos[kHS31MeasuringClown].y, 1, 0x107C2, 1);
							_vm->_hotspots[kHS31WalkArea1]._flags &= ~SF_WALKABLE;
							_vm->_platypusActionStatus = kAS31PlatMeasuringClown;
							_vm->_gnapActionStatus = kAS31PlatMeasuringClown;
							_vm->_timers[4] = 300;
						} else
							_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS31BeerBarrel:
			if (_vm->_gnapActionStatus < 0 || _vm->_gnapActionStatus == kAS31PlatMeasuringClown) {
				if (_vm->_grabCursorSpriteIndex == kItemEmptyBucket && _beerGuyDistracted) {
					_vm->setGrabCursorSprite(-1);
					_vm->gnapWalkTo(_vm->_gnapX, _vm->_gnapY, -1, _vm->getGnapSequenceId(gskIdle, _vm->_hotspotsWalkPos[kHS31BeerBarrel].x, _vm->_hotspotsWalkPos[kHS31BeerBarrel].y) | 0x10000, 1);
					_clerkMeasureMaxCtr += 5;
					gameSys.insertSequence(0xF8, 59, 0, 0, kSeqNone, 0, 0, 0);
					_vm->playGnapPullOutDevice(6, 8);
					_vm->playGnapUseDevice(0, 0);
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS31BeerBarrel].x, _vm->_hotspotsWalkPos[kHS31BeerBarrel].y, 0, 0x107BC, 1);
					_vm->_gnapActionStatus = kAS31FillEmptyBucketWithBeer;
					_vm->_timers[4] = 300;
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHS31BeerBarrel].x, _vm->_hotspotsWalkPos[kHS31BeerBarrel].y, 6, 2);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(6, 2);
						break;
					case GRAB_CURSOR:
						if (_beerGuyDistracted) {
							_vm->playGnapScratchingHead(6, 2);
						} else {
							_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS31BeerBarrel].x, _vm->_hotspotsWalkPos[kHS31BeerBarrel].y, 0, 0x107BC, 1);
							_vm->_gnapActionStatus = kAS31UseBeerBarrel;
							_vm->_gnapIdleFacing = kDirUpLeft;
						}
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS31ExitCircus:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 26;
				_vm->gnapWalkTo(-1, _vm->_hotspotsWalkPos[kHS31ExitCircus].y, 0, 0x107AE, 1);
				_vm->_gnapActionStatus = kAS31LeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS31ExitCircus].x + 1, _vm->_hotspotsWalkPos[kHS31ExitCircus].y, -1, -1, 1);
			}
			break;

		case kHS31ExitOutsideClown:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 27;
				_vm->gnapWalkTo(-1, _vm->_hotspotsWalkPos[kHS31ExitOutsideClown].y, 0, 0x107AF, 1);
				_vm->_gnapActionStatus = kAS31LeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS31ExitOutsideClown].x, _vm->_hotspotsWalkPos[kHS31ExitOutsideClown].y + 1, -1, 0x107CF, 1);
			}
			break;

		case kHS31WalkArea1:
			if (_vm->_gnapActionStatus < 0)
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			break;

		default:
			if (_vm->_mouseClickState._left) {
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				_vm->_mouseClickState._left = false;
			}
			break;
		}

		updateAnimations();

		if (!_vm->isSoundPlaying(0x1093B))
			_vm->playSound(0x1093B, true);

		if (!_vm->_isLeavingScene) {
			if (_vm->_platypusActionStatus < 0)
				_vm->updatePlatypusIdleSequence();
			if (_vm->_gnapActionStatus < 0)
				_vm->updateGnapIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(20) + 60;
				if (_vm->_gnapActionStatus < 0 && _nextClerkSequenceId == -1) {
					switch (_vm->getRandom(6)){
					case 0:
						_nextClerkSequenceId = 0xFF;
						break;
					case 1:
						_nextClerkSequenceId = 0x100;
						break;
					case 2:
						_nextClerkSequenceId = 0x101;
						break;
					default:
						_nextClerkSequenceId = 0xFB;
						break;
					}
				}
			}
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(50) + 180;
				if (_vm->_gnapActionStatus < 0) {
					if (_vm->getRandom(2) != 0)
						gameSys.insertSequence(0x104, 20, 0, 0, kSeqNone, 0, 0, 0);
					else
						gameSys.insertSequence(0x103, 20, 0, 0, kSeqNone, 0, 0, 0);
				}
			}
			_vm->playSoundB();
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

void Scene31::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kAS31UseBeerBarrel:
			_nextClerkSequenceId = 0xFE;
			break;
		case kAS31FillEmptyBucketWithBeer:
			gameSys.setAnimation(0x102, 59, 0);
			gameSys.insertSequence(0x102, 59, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapX = 5;
			_vm->_gnapY = 7;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapSequenceId = 0x102;
			_vm->_gnapId = 59;
			_vm->_gnapActionStatus = kAS31FillEmptyBucketWithBeerDone;
			break;
		case kAS31FillEmptyBucketWithBeerDone:
			_vm->_gnapIdleFacing = kDirBottomLeft;
			_vm->playGnapPullOutDevice(0, 0);
			_vm->playGnapUseDevice(0, 0);
			gameSys.insertSequence(0xF9, 59, 0xF8, 59, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapActionStatus = -1;
			_vm->invAdd(kItemBucketWithBeer);
			_vm->invRemove(kItemEmptyBucket);
			_vm->setGrabCursorSprite(kItemBucketWithBeer);
			break;
		case kAS31UseMeasuringClown:
			_nextClerkSequenceId = 0xFA;
			_clerkMeasureMaxCtr = 1;
			break;
		case kAS31LeaveScene:
			_vm->_sceneDone = true;
			_vm->_gnapActionStatus = -1;
			break;
		}
	}

	if (gameSys.getAnimationStatus(1) == 2) {
		gameSys.setAnimation(0, 0, 1);
		if (_vm->_platypusActionStatus == kAS31PlatMeasuringClown) {
			_vm->_sceneWaiting = true;
			_beerGuyDistracted = true;
			_nextClerkSequenceId = 0xFA;
		}
	}

	if (gameSys.getAnimationStatus(3) == 2) {
		switch (_nextClerkSequenceId) {
		case 0xFA:
			gameSys.insertSequence(_nextClerkSequenceId, 39, _currClerkSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0xFC, 39, _nextClerkSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(0xFC, 39, 3);
			_currClerkSequenceId = 0xFC;
			_nextClerkSequenceId = 0xFC;
			_clerkMeasureCtr = 0;
			break;
		case 0xFC:
			++_clerkMeasureCtr;
			if (_clerkMeasureCtr >= _clerkMeasureMaxCtr) {
				if (_vm->_gnapActionStatus != 5)
					_vm->_platypusActionStatus = -1;
				_vm->_timers[0] = 40;
				gameSys.insertSequence(0xFD, 39, _currClerkSequenceId, 39, kSeqSyncWait, 0, 0, 0);
				_currClerkSequenceId = 0xFD;
				_nextClerkSequenceId = -1;
				if (_vm->_gnapActionStatus != kAS31FillEmptyBucketWithBeerDone && _vm->_gnapActionStatus != kAS31FillEmptyBucketWithBeer)
					_vm->_gnapActionStatus = -1;
				_beerGuyDistracted = false;
				_clerkMeasureMaxCtr = 3;
				gameSys.setAnimation(0xFD, 39, 3);
				_vm->_sceneWaiting = false;
			} else {
				gameSys.insertSequence(_nextClerkSequenceId, 39, _currClerkSequenceId, 39, kSeqSyncWait, 0, 0, 0);
				_currClerkSequenceId = _nextClerkSequenceId;
				_nextClerkSequenceId = 0xFC;
				gameSys.setAnimation(0xFC, 39, 3);
			}
			break;
		case 0xFE:
			gameSys.insertSequence(_nextClerkSequenceId, 39, _currClerkSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(_nextClerkSequenceId, 39, 3);
			_currClerkSequenceId = _nextClerkSequenceId;
			_nextClerkSequenceId = -1;
			_vm->_gnapActionStatus = -1;
			break;
		default:
			if (_nextClerkSequenceId != -1) {
				gameSys.insertSequence(_nextClerkSequenceId, 39, _currClerkSequenceId, 39, kSeqSyncWait, 0, 0, 0);
				gameSys.setAnimation(_nextClerkSequenceId, 39, 3);
				_currClerkSequenceId = _nextClerkSequenceId;
				_nextClerkSequenceId = -1;
			}
			break;
		}
	}
}

/*****************************************************************************/

Scene32::Scene32(GnapEngine *vm) : Scene(vm) {}

int Scene32::init() {
	_vm->_gameSys->setAnimation(0, 0, 0);
	return _vm->isFlag(kGFPlatypusTalkingToAssistant) ? 0xF : 0x10;
}

void Scene32::updateHotspots() {
	_vm->setHotspot(kHS32Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS32ExitTruck, 780, 226, 800, 455, SF_EXIT_R_CURSOR | SF_WALKABLE, 10, 6);
	_vm->setHotspot(kHS32WalkArea1, 0, 0, 162, 426);
	_vm->setHotspot(kHS32WalkArea2, 162, 0, 237, 396);
	_vm->setHotspot(kHS32WalkArea3, 237, 0, 319, 363);
	_vm->setHotspot(kHS32WalkArea4, 520, 0, 800, 404);
	_vm->setHotspot(kHS32WalkArea5, 300, 447, 800, 600);
	_vm->setHotspot(kHS32WalkArea6, 678, 0, 800, 404);
	_vm->setHotspot(kHS32WalkArea7, 0, 0, 520, 351);
	_vm->setHotspot(kHS32WalkArea8, 0, 546, 300, 600);
	_vm->setDeviceHotspot(kHS32Device, -1, -1, -1, -1);
	_vm->_hotspotsCount = 11;
}

void Scene32::run() {
	GameSys& gameSys = *_vm->_gameSys;

	_vm->playSound(0x1091C, true);
	_vm->startSoundTimerC(5);
	_vm->queueInsertDeviceIcon();
	_vm->_timers[4] = _vm->getRandom(100) + 300;

	if (_vm->_prevSceneNum == 33) {
		_vm->initGnapPos(11, 6, kDirBottomLeft);
		_vm->initPlatypusPos(12, 6, kDirUnk4);
		_vm->endSceneInit();
		_vm->platypusWalkTo(9, 6, -1, 0x107D2, 1);
		_vm->gnapWalkTo(8, 6, -1, 0x107BA, 1);
	} else {
		_vm->initGnapPos(1, 6, kDirBottomRight);
		_vm->initPlatypusPos(1, 7, kDirNone);
		_vm->endSceneInit();
	}

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
		_vm->testWalk(0, 0, -1, -1, -1, -1);

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS32Device:
			if (_vm->_gnapActionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS32Platypus:
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

		case kHS32ExitTruck:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->setGrabCursorSprite(-1);
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS32ExitTruck].x, _vm->_hotspotsWalkPos[kHS32ExitTruck].y, 0, 0x107AB, 1);
				_vm->_gnapActionStatus = kAS32LeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS32ExitTruck].x, _vm->_hotspotsWalkPos[kHS32ExitTruck].y + 1, -1, 0x107CD, 1);
				_vm->_newSceneNum = 33;
			}
			break;

		case kHS32WalkArea1:
		case kHS32WalkArea2:
		case kHS32WalkArea3:
		case kHS32WalkArea4:
		case kHS32WalkArea5:
		case kHS32WalkArea6:
		case kHS32WalkArea7:
		case kHS32WalkArea8:
			if (_vm->_gnapActionStatus < 0)
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			break;

		}

		if (_vm->_mouseClickState._left && _vm->_gnapActionStatus < 0) {
			_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			_vm->_mouseClickState._left = 0;
		}

		updateAnimations();

		if (!_vm->isSoundPlaying(0x1091C))
			_vm->playSound(0x1091C, true);

		if (!_vm->_isLeavingScene) {
			if (_vm->_platypusActionStatus < 0)
				_vm->updatePlatypusIdleSequence();
			if (_vm->_gnapActionStatus < 0)
				_vm->updateGnapIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(100) + 300;
				if (_vm->getRandom(2) != 0)
					gameSys.insertSequence(0x0E, 180, 0, 0, kSeqNone, 0, 0, 0);
				else
					gameSys.insertSequence(0x0D, 180, 0, 0, kSeqNone, 0, 0, 0);
			}
			_vm->playSoundC();
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

void Scene32::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		if (_vm->_gnapActionStatus == kAS32LeaveScene)
			_vm->_sceneDone = true;
	}
}

/*****************************************************************************/

Scene33::Scene33(GnapEngine *vm) : Scene(vm) {
	_currChickenSequenceId = -1;
	_nextChickenSequenceId = -1;
}

int Scene33::init() {
	return _vm->isFlag(kGFPlatypusTalkingToAssistant) ? 0x84 : 0x85;
}

void Scene33::updateHotspots() {
	_vm->setHotspot(kHS33Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS33Chicken, 606, 455, 702, 568, SF_WALKABLE | SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 7, 8);
	_vm->setHotspot(kHS33ExitHouse, 480, 120, 556, 240, SF_EXIT_U_CURSOR, 7, 3);
	_vm->setHotspot(kHS33ExitBarn, 610, 75, 800, 164, SF_EXIT_U_CURSOR, 10, 3);
	_vm->setHotspot(kHS33ExitCreek, 780, 336, 800, 556, SF_EXIT_R_CURSOR | SF_WALKABLE, 10, 8);
	_vm->setHotspot(kHS33ExitPigpen, 0, 300, 20, 600, SF_EXIT_L_CURSOR | SF_WALKABLE, 0, 8);
	_vm->setHotspot(kHS33WalkArea1, 120, 0, 514, 458);
	_vm->setHotspot(kHS33WalkArea2, 0, 0, 800, 452);
	_vm->setDeviceHotspot(kHS33Device, -1, -1, -1, -1);
	_vm->_hotspotsCount = 9;
}

void Scene33::run() {
	GameSys& gameSys = *_vm->_gameSys;

	_vm->playSound(0x1091C, true);
	_vm->startSoundTimerC(6);
	_vm->queueInsertDeviceIcon();

	_currChickenSequenceId = 0x7E;
	gameSys.setAnimation(0x7E, 179, 2);
	gameSys.insertSequence(_currChickenSequenceId, 179, 0, 0, kSeqNone, 0, 0, 0);
	_nextChickenSequenceId = -1;
	_vm->_timers[5] = _vm->getRandom(20) + 30;
	_vm->_timers[4] = _vm->getRandom(100) + 300;

	switch (_vm->_prevSceneNum) {
	case 34:
		_vm->initGnapPos(11, 7, kDirBottomLeft);
		_vm->initPlatypusPos(12, 7, kDirUnk4);
		_vm->endSceneInit();
		_vm->gnapWalkTo(8, 7, -1, 0x107BA, 1);
		_vm->platypusWalkTo(9, 7, -1, 0x107D2, 1);
		break;
	case 37:
		_vm->initGnapPos(7, 7, kDirBottomRight);
		_vm->initPlatypusPos(8, 7, kDirNone);
		_vm->endSceneInit();
		break;
	case 32:
		_vm->initGnapPos(-1, 6, kDirBottomRight);
		_vm->initPlatypusPos(-1, 7, kDirNone);
		_vm->endSceneInit();
		_vm->platypusWalkTo(2, 7, -1, 0x107C2, 1);
		_vm->gnapWalkTo(2, 8, -1, 0x107B9, 1);
		break;
	default:
		_vm->initGnapPos(3, 7, kDirBottomRight);
		_vm->initPlatypusPos(2, 7, kDirNone);
		_vm->endSceneInit();
		break;
	}

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
		_vm->testWalk(0, 0, 7, 6, 8, 6);

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS33Device:
			if (_vm->_gnapActionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS33Platypus:
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

		case kHS33Chicken:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(7, 9, 9, 8);
				} else {
					switch (_vm->_verbCursor) {
					case GRAB_CURSOR:
						_vm->_gnapIdleFacing = kDirBottomRight;
						if (_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS33Chicken].x, _vm->_hotspotsWalkPos[kHS33Chicken].y, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1))
							_vm->_gnapActionStatus = kAS33UseChicken;
						else
							_vm->_gnapActionStatus = -1;
						break;
					case TALK_CURSOR:
						_vm->_gnapIdleFacing = kDirBottomRight;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS33Chicken].x, _vm->_hotspotsWalkPos[kHS33Chicken].y, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_vm->_gnapActionStatus = kAS33TalkChicken;
						break;
					case LOOK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS33ExitHouse:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_gnapActionStatus = kAS33LeaveScene;
				_vm->_newSceneNum = 37;
				if (_vm->_gnapX > 6)
					_vm->gnapWalkTo(_vm->_gnapX, _vm->_gnapY, 0, 0x107AD, 1);
				else
					_vm->gnapWalkTo(6, 7, 0, 0x107B1, 1);
			}
			break;

		case kHS33ExitBarn:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_gnapActionStatus = kAS33LeaveScene;
				_vm->_newSceneNum = 35;
				if (_vm->_gnapX > 7)
					_vm->gnapWalkTo(_vm->_gnapX, _vm->_gnapY, 0, 0x107AD, 1);
				else
					_vm->gnapWalkTo(7, 7, 0, 0x107B1, 1);
			}
			break;

		case kHS33ExitCreek:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS33ExitCreek].x, _vm->_hotspotsWalkPos[kHS33ExitCreek].y, 0, 0x107AB, 1);
				_vm->_gnapActionStatus = kAS33LeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS33ExitCreek].x, _vm->_hotspotsWalkPos[kHS33ExitCreek].y, -1, 0x107CD, 1);
				_vm->_newSceneNum = 34;
			}
			break;

		case kHS33ExitPigpen:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS33ExitPigpen].x, _vm->_hotspotsWalkPos[kHS33ExitPigpen].y, 0, 0x107AF, 1);
				_vm->_gnapActionStatus = kAS33LeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS33ExitPigpen].x, _vm->_hotspotsWalkPos[kHS33ExitPigpen].y, -1, 0x107CF, 1);
				_vm->_newSceneNum = 32;
			}
			break;

		case kHS33WalkArea1:
		case kHS33WalkArea2:
			if (_vm->_gnapActionStatus < 0)
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			break;

		default:
			if (_vm->_mouseClickState._left && _vm->_gnapActionStatus < 0) {
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				_vm->_mouseClickState._left = false;
			}
			break;
		}

		updateAnimations();

		if (!_vm->isSoundPlaying(0x1091C))
			_vm->playSound(0x1091C, true);

		if (!_vm->_isLeavingScene) {
			if (_vm->_platypusActionStatus < 0)
				_vm->updatePlatypusIdleSequence();
			if (_vm->_gnapActionStatus < 0)
				_vm->updateGnapIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(100) + 300;
				if (_vm->getRandom(2) != 0)
					gameSys.insertSequence(0x83, 256, 0, 0, kSeqNone, 0, 0, 0);
				else
					gameSys.insertSequence(0x82, 256, 0, 0, kSeqNone, 0, 0, 0);
			}
			if (!_vm->_timers[5] && _nextChickenSequenceId == -1 && _vm->_gnapActionStatus != kAS33TalkChicken && _vm->_gnapActionStatus != kAS33UseChicken) {
				if (_vm->getRandom(6) != 0) {
					_nextChickenSequenceId = 0x7E;
					_vm->_timers[5] = _vm->getRandom(20) + 30;
				} else {
					_nextChickenSequenceId = 0x80;
					_vm->_timers[5] = _vm->getRandom(20) + 50;
				}
			}
			_vm->playSoundC();
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

void Scene33::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(0) == 2) {
		switch (_vm->_gnapActionStatus) {
		case kAS33LeaveScene:
			_vm->_sceneDone = true;
			break;
		case kAS33TalkChicken:
			_nextChickenSequenceId = 0x7F;
			break;
		case kAS33UseChicken:
			_nextChickenSequenceId = 0x81;
			_vm->_timers[2] = 100;
			break;
		case kAS33UseChickenDone:
			gameSys.insertSequence(0x107B5, _vm->_gnapId, 0x81, 179, kSeqSyncWait, 0, 75 * _vm->_gnapX - _vm->_gnapGridX, 48 * _vm->_gnapY - _vm->_gnapGridY);
			_vm->_gnapSequenceId = 0x7B5;
			_vm->_gnapSequenceDatNum = 1;
			_currChickenSequenceId = 0x7E;
			gameSys.setAnimation(0x7E, 179, 2);
			gameSys.insertSequence(_currChickenSequenceId, 179, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_gnapActionStatus = -1;
			_vm->_timers[5] = 30;
			break;
		default:
			_vm->_gnapActionStatus = -1;
			break;
		}
	}

	if (gameSys.getAnimationStatus(2) == 2) {
		if (_nextChickenSequenceId == 0x81) {
			gameSys.setAnimation(_nextChickenSequenceId, 179, 0);
			gameSys.insertSequence(_nextChickenSequenceId, 179, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			gameSys.removeSequence(_currChickenSequenceId, 179, true);
			_nextChickenSequenceId = -1;
			_currChickenSequenceId = -1;
			_vm->_gnapActionStatus = kAS33UseChickenDone;
			_vm->_timers[5] = 500;
		} else if (_nextChickenSequenceId == 0x7F) {
			gameSys.setAnimation(_nextChickenSequenceId, 179, 2);
			gameSys.insertSequence(_nextChickenSequenceId, 179, _currChickenSequenceId, 179, kSeqSyncWait, 0, 0, 0);
			_currChickenSequenceId = _nextChickenSequenceId;
			_nextChickenSequenceId = -1;
			_vm->_gnapActionStatus = -1;
		} else if (_nextChickenSequenceId != -1) {
			gameSys.setAnimation(_nextChickenSequenceId, 179, 2);
			gameSys.insertSequence(_nextChickenSequenceId, 179, _currChickenSequenceId, 179, kSeqSyncWait, 0, 0, 0);
			_currChickenSequenceId = _nextChickenSequenceId;
			_nextChickenSequenceId = -1;
		}
	}
}

/*****************************************************************************/

Scene38::Scene38(GnapEngine *vm) : Scene(vm) {
}

int Scene38::init() {
	GameSys& gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 1);
	return 0xA5;
}

void Scene38::updateHotspots() {
	_vm->setHotspot(kHS38Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS38ExitHouse, 150, 585, 650, 600, SF_EXIT_D_CURSOR, 0, 8);
	_vm->setHotspot(kHS38ExitCave, 430, 440, 655, 470, SF_WALKABLE, 0, 8);
	_vm->setHotspot(kHS38TrapDoorLid1, 525, 265, 640, 350, SF_DISABLED);
	_vm->setHotspot(kHS38TrapDoorLid2, 555, 350, 670, 430, SF_DISABLED);
	_vm->setHotspot(kHS38HuntingTrophy, 170, 85, 250, 190, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 0, 8);
	_vm->setHotspot(kHS38WalkArea1, 330, 270, 640, 380, SF_DISABLED | SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 0, 8);
	_vm->setHotspot(kHS38WalkArea2, 0, 0, 799, 396);
	_vm->setHotspot(kHS38WalkArea3, 0, 585, 799, 599, SF_WALKABLE | SF_DISABLED);
	_vm->setHotspot(kHS38WalkArea4, 0, 0, 97, 445);
	_vm->setHotspot(kHS38WalkArea5, 770, 0, 799, 445);
	_vm->setHotspot(kHS38WalkArea6, 393, 0, 698, 445, SF_WALKABLE | SF_DISABLED);
	_vm->setDeviceHotspot(kHS38Device, -1, -1, -1, -1);
	if (_vm->_platypusActionStatus == kAS38PlatypusHoldingTrapDoor)
		_vm->_hotspots[kHS38Platypus]._flags = SF_WALKABLE | SF_DISABLED;
	if (_vm->_platypusActionStatus == kAS38PlatypusHoldingTrapDoor)
		_vm->_hotspots[kHS38ExitCave]._flags = SF_EXIT_D_CURSOR;
	else if (_vm->_gnapActionStatus == kAS38HoldingHuntingTrophy)
		_vm->_hotspots[kHS38ExitCave]._flags = SF_EXIT_D_CURSOR;
	if (_vm->_platypusActionStatus == kAS38PlatypusHoldingTrapDoor)
		_vm->_hotspots[kHS38TrapDoorLid1]._flags = SF_DISABLED;
	else if (_vm->_gnapActionStatus == kAS38HoldingHuntingTrophy)
		_vm->_hotspots[kHS38TrapDoorLid1]._flags = SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	if (_vm->_platypusActionStatus == kAS38PlatypusHoldingTrapDoor)
		_vm->_hotspots[kHS38TrapDoorLid2]._flags = SF_DISABLED;
	else if (_vm->_gnapActionStatus == kAS38HoldingHuntingTrophy)
		_vm->_hotspots[kHS38TrapDoorLid2]._flags = SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	if (_vm->_platypusActionStatus == kAS38PlatypusHoldingTrapDoor)
		_vm->_hotspots[kHS38WalkArea6]._flags = SF_NONE;
	_vm->_hotspotsCount = 13;
}

void Scene38::run() {
	_vm->queueInsertDeviceIcon();
	_vm->_gameSys->insertSequence(0x9B, 0, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->_prevSceneNum == 39) {
		_vm->initGnapPos(3, 7, kDirBottomLeft);
		_vm->initPlatypusPos(4, 7, kDirUnk4);
	} else {
		_vm->initGnapPos(3, 8, kDirBottomRight);
		_vm->initPlatypusPos(4, 8, kDirNone);
	}
	_vm->endSceneInit();

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS38Device:
			_vm->runMenu();
			updateHotspots();
			break;

		case kHS38Platypus:
			if (_vm->_gnapActionStatus == kAS38HoldingHuntingTrophy) {
				_vm->_gnapActionStatus = kAS38ReleaseHuntingTrophy;
			} else if (_vm->_grabCursorSpriteIndex >= 0) {
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
			break;

		case kHS38ExitHouse:
			if (_vm->_gnapActionStatus == kAS38HoldingHuntingTrophy) {
				_vm->_gnapActionStatus = kAS38ReleaseHuntingTrophy;
			} else {
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(-1, -1, 0, 0x107AE, 1);
				_vm->_gnapActionStatus = kAS38LeaveScene;
				_vm->_newSceneNum = 37;
			}
			break;

		case kHS38ExitCave:
			if (_vm->_gnapActionStatus == kAS38HoldingHuntingTrophy) {
				_vm->_gnapActionStatus = kAS38ReleaseHuntingTrophy;
				if (_vm->_platypusActionStatus == kAS38PlatypusHoldingTrapDoor)
					_vm->_isLeavingScene = true;
			} else if (_vm->_platypusActionStatus == kAS38PlatypusHoldingTrapDoor) {
				_vm->_sceneWaiting = false;
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(5, 7, 0, 0x107BB, 1);
				_vm->_newSceneNum = 39;
				_vm->_gnapActionStatus = kAS38ExitCave;
			}
			break;

		case kHS38TrapDoorLid1:
		case kHS38TrapDoorLid2:
			if (_vm->_gnapActionStatus == kAS38HoldingHuntingTrophy) {
				if (_vm->_verbCursor == PLAT_CURSOR && _vm->_platypusActionStatus != kAS38PlatypusHoldingTrapDoor)
					_vm->_gnapActionStatus = kAS38UsePlatypusWithTrapDoor;
				else
					_vm->_gnapActionStatus = kAS38ReleaseHuntingTrophy;
			}
			break;

		case kHS38HuntingTrophy:
			if (_vm->_gnapActionStatus != kAS38HoldingHuntingTrophy) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(3, 6, 2, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(0, 0);
						break;
					case GRAB_CURSOR:
						if (_vm->_platypusActionStatus == kAS38PlatypusHoldingTrapDoor)
							_vm->playGnapImpossible(0, 0);
						else {
							_vm->gnapWalkTo(3, 6, 0, 0x107BB, 1);
							_vm->platypusWalkTo(4, 8, -1, -1, 1);
							_vm->_gnapActionStatus = kAS38UseHuntingTrophy;
						}
						break;
					case TALK_CURSOR:
						_vm->playGnapBrainPulsating(2, 0);
						break;
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS38WalkArea1:
			// Nothing
			break;

		case kHS38WalkArea2:
		case kHS38WalkArea3:
		case kHS38WalkArea4:
		case kHS38WalkArea5:
		case kHS38WalkArea6:
			if (_vm->_gnapActionStatus == kAS38HoldingHuntingTrophy)
				_vm->_gnapActionStatus = kAS38ReleaseHuntingTrophy;
			else if (_vm->_gnapActionStatus < 0)
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			break;

		default:
			if (_vm->_mouseClickState._left) {
				if (_vm->_gnapActionStatus == kAS38HoldingHuntingTrophy)
					_vm->_gnapActionStatus = kAS38ReleaseHuntingTrophy;
				else if (_vm->_gnapActionStatus < 0)
					_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				_vm->_mouseClickState._left = false;
			}
			break;
		}

		updateAnimations();

		if (!_vm->_isLeavingScene) {
			_vm->updatePlatypusIdleSequence();
			_vm->updateGnapIdleSequence();
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

void Scene38::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kAS38LeaveScene:
			_vm->_sceneDone = true;
			break;
		case kAS38ExitCave:
			gameSys.removeSequence(_vm->_platypusSequenceId | (_vm->_platypusSequenceDatNum << 16), _vm->_platypusId, true);
			gameSys.insertSequence(0xA3, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceId = 0xA3;
			_vm->_gnapSequenceDatNum = 0;
			gameSys.setAnimation(0xA3, _vm->_gnapId, 0);
			_vm->_gnapActionStatus = kAS38LeaveScene;
			break;
		case kAS38UseHuntingTrophy:
			gameSys.removeSequence(0x9B, 0, true);
			gameSys.insertSequence(0x9C, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceId = 0x9C;
			_vm->_gnapSequenceDatNum = 0;
			gameSys.setAnimation(0x9C, _vm->_gnapId, 0);
			_vm->_gnapActionStatus = kAS38HoldingHuntingTrophy;
			updateHotspots();
			break;
		case kAS38HoldingHuntingTrophy:
			if (_vm->_platypusActionStatus != kAS38PlatypusHoldingTrapDoor)
				_vm->_sceneWaiting = true;
			if (_vm->_gnapSequenceId == 0xA4) {
				gameSys.insertSequence(0x9D, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
				_vm->_gnapSequenceId = 0x9D;
			} else {
				gameSys.insertSequence(0xA4, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
				_vm->_gnapSequenceId = 0xA4;
			}
			_vm->_gnapSequenceDatNum = 0;
			gameSys.setAnimation(_vm->_gnapSequenceId, _vm->_gnapId, 0);
			break;
		case kAS38ReleaseHuntingTrophy:
			if (_vm->_gnapSequenceId == 0x9E) {
				gameSys.insertSequence(0x9B, 0, 0, 0, kSeqNone, 0, 0, 0);
				_vm->_gnapActionStatus = -1;
			} else if (_vm->_platypusActionStatus == kAS38PlatypusHoldingTrapDoor) {
				gameSys.insertSequence(0xA0, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
				_vm->_gnapSequenceId = 0xA0;
				_vm->_gnapSequenceDatNum = 0;
				_vm->_gnapX = 3;
				_vm->_gnapY = 6;
				_vm->_gnapIdleFacing = kDirBottomRight;
				if (_vm->_isLeavingScene) {
					_vm->_sceneWaiting = false;
					_vm->gnapWalkTo(5, 7, 0, 0x107BB, 1);
					_vm->_newSceneNum = 39;
					_vm->_gnapActionStatus = kAS38ExitCave;
				} else {
					_vm->_gnapActionStatus = -1;
				}
			} else {
				gameSys.insertSequence(0x9E, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
				_vm->_gnapSequenceId = 0x9E;
				_vm->_gnapSequenceDatNum = 0;
				_vm->_gnapX = 3;
				_vm->_gnapY = 6;
				_vm->_gnapIdleFacing = kDirBottomRight;
				gameSys.setAnimation(0x9E, _vm->_gnapId, 0);
				_vm->_sceneWaiting = false;
				updateHotspots();
			}
			break;
		case kAS38UsePlatypusWithTrapDoor:
			_vm->_sceneWaiting = false;
			gameSys.insertSequence(0x9F, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceId = 0x9F;
			_vm->_gnapSequenceDatNum = 0;
			gameSys.setAnimation(0x9F, _vm->_gnapId, 0);
			_vm->_gnapActionStatus = kAS38HoldingHuntingTrophy;
			if (_vm->_platypusFacing != kDirNone)
				_vm->playPlatypusSequence(0x107D5);
			else
				_vm->playPlatypusSequence(0x107D4);
			_vm->platypusWalkTo(8, 7, -1, 0x107D2, 1);
			gameSys.insertSequence(0xA1, _vm->_gnapId + 1, _vm->_platypusSequenceId | (_vm->_platypusSequenceDatNum << 16), _vm->_platypusId, kSeqSyncWait, 0, 0, 0);
			_vm->_platypusSequenceId = 0xA1;
			_vm->_platypusSequenceDatNum = 0;
			_vm->_platypusId = _vm->_gnapId + 1;
			gameSys.setAnimation(0xA1, _vm->_gnapId + 1, 1);
			_vm->_platypusActionStatus = kAS38PlatypusHoldingTrapDoor;
			updateHotspots();
			break;
		}
	}

	if (gameSys.getAnimationStatus(1) == 2) {
		gameSys.setAnimation(0, 0, 1);
		if (_vm->_platypusActionStatus == kAS38PlatypusHoldingTrapDoor) {
			gameSys.insertSequence(0xA2, _vm->_platypusId, _vm->_platypusSequenceId | (_vm->_platypusSequenceDatNum << 16), _vm->_platypusId, kSeqSyncWait, 0, 0, 0);
			_vm->_platypusSequenceId = 0xA2;
			_vm->_platypusSequenceDatNum = 0;
			updateHotspots();
			_vm->_sceneWaiting = true;
		}
	}
}

/*****************************************************************************/

Scene39::Scene39(GnapEngine *vm) : Scene(vm) {
	_currGuySequenceId = -1;
	_nextGuySequenceId = -1;
}

int Scene39::init() {
	GameSys& gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 1);
	return 0x35;
}

void Scene39::updateHotspots() {
	_vm->setHotspot(kHS39Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS39ExitInsideHouse, 0, 0, 140, 206, SF_EXIT_U_CURSOR, 4, 8);
	_vm->setHotspot(kHS39ExitUfoParty, 360, 204, 480, 430, SF_EXIT_R_CURSOR, 6, 8);
	_vm->setHotspot(kHS39Sign, 528, 232, 607, 397, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 7, 3);
	_vm->setHotspot(kHS39WalkArea1, 0, 0, 800, 466);
	_vm->setHotspot(kHS39WalkArea2, 502, 466, 800, 600);
	_vm->setDeviceHotspot(kHS39Device, -1, -1, -1, -1);
	_vm->_hotspotsCount = 7;
}

void Scene39::run() {
	GameSys& gameSys = *_vm->_gameSys;

	// Bug in the original? Timer was never initialized.
	_vm->_timers[5] = 0;

	_vm->queueInsertDeviceIcon();
	_currGuySequenceId = 0x33;

	gameSys.setAnimation(0x33, 21, 3);
	gameSys.insertSequence(_currGuySequenceId, 21, 0, 0, kSeqNone, 0, 0, 0);
	gameSys.insertSequence(0x34, 21, 0, 0, kSeqLoop, 0, 0, 0);

	_nextGuySequenceId = -1;
	if (_vm->_prevSceneNum == 38) {
		_vm->initGnapPos(3, 7, kDirUpRight);
		_vm->initPlatypusPos(2, 7, kDirUpLeft);
		_vm->endSceneInit();
	} else {
		_vm->initGnapPos(4, 7, kDirBottomRight);
		_vm->initPlatypusPos(5, 7, kDirNone);
		_vm->endSceneInit();
	}

	while (!_vm->_sceneDone) {
		if (!_vm->isSoundPlaying(0x1094B)) {
			_vm->playSound(0x1094B, true);
			_vm->setSoundVolume(0x1094B, 60);
		}

		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->testWalk(0, 0, -1, -1, -1, -1);

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS39Device:
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[5] = _vm->getRandom(20) + 50;
			break;

		case kHS39Platypus:
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

		case kHS39ExitUfoParty:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_sceneDone = true;
				_vm->gnapWalkTo(_vm->_gnapX, _vm->_gnapY, 0, 0x107AB, 1);
				_vm->_gnapActionStatus = kAS39LeaveScene;
				_vm->_newSceneNum = 40;
			}
			break;

		case kHS39Sign:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapImpossible(0, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS39Sign].x, _vm->_hotspotsWalkPos[kHS39Sign].y, 0, -1, 1);
						_vm->playGnapIdle(_vm->_hotspotsWalkPos[kHS39Sign].x, _vm->_hotspotsWalkPos[kHS39Sign].y);
						_vm->showFullScreenSprite(0x1C);
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

		case kHS39ExitInsideHouse:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_sceneDone = true;
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 38;
			}
			break;

		case kHS39WalkArea1:
		case kHS39WalkArea2:
			if (_vm->_gnapActionStatus < 0)
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
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
			if (_vm->_platypusActionStatus < 0)
				_vm->updatePlatypusIdleSequence();
			if (_vm->_gnapActionStatus < 0)
				_vm->updateGnapIdleSequence();
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(20) + 50;
				switch (_vm->getRandom(4)) {
				case 0:
					_nextGuySequenceId = 0x30;
					break;
				case 1:
					_nextGuySequenceId = 0x31;
					break;
				case 2:
					_nextGuySequenceId = 0x32;
					break;
				case 3:
					_nextGuySequenceId = 0x33;
					break;
				}
			}
		}

		_vm->checkGameKeys();

		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[5] = _vm->getRandom(20) + 50;
		}

		_vm->gameUpdateTick();
	}
}

void Scene39::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		if (_vm->_gnapActionStatus == kAS39LeaveScene)
			_vm->_sceneDone = true;
		else
			_vm->_gnapActionStatus = -1;
	}

	if (gameSys.getAnimationStatus(3) == 2 && _nextGuySequenceId != -1) {
		gameSys.setAnimation(_nextGuySequenceId, 21, 3);
		gameSys.insertSequence(_nextGuySequenceId, 21, _currGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
		_currGuySequenceId = _nextGuySequenceId;
		_nextGuySequenceId = -1;
	}
}

} // End of namespace Gnap
