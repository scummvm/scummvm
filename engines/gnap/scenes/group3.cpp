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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

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
	gnap.initPos(7, 12, kDirBottomRight);
	plat.initPos(6, 12, kDirIdleLeft);
	_vm->endSceneInit();
	gnap.walkTo(Common::Point(7, 8), -1, 0x107B9, 1);
	plat.walkTo(Common::Point(6, 8), -1, 0x107C2, 1);

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS30Device:
			if (gnap._actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS30Platypus:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemJoint) {
					gnap.useJointOnPlatypus();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playImpossible();
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playMoan1(plat._pos);
						break;
					case GRAB_CURSOR:
						gnap.kissPlatypus(0);
						break;
					case TALK_CURSOR:
						gnap.playBrainPulsating(plat._pos);
						plat.playSequence(plat.getSequenceId());
						break;
					case PLAT_CURSOR:
						gnap.playImpossible();
						break;
					default:
						break;
					}
				}
			}
			break;

		case kHS30PillMachine:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemDiceQuarterHole && !_vm->isFlag(kGFUnk23)) {
					_vm->_hotspots[kHS30WalkArea1]._flags |= SF_WALKABLE;
					gnap.walkTo(_vm->_hotspotsWalkPos[kHS30PillMachine], 0, 0x107BC, 1);
					_vm->_hotspots[kHS30WalkArea1]._flags &= ~SF_WALKABLE;
					gnap._actionStatus = kAS30UsePillMachine;
					hasTakenPill = true;
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[kHS30PillMachine], 8, 5);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.walkTo(Common::Point(9, 8), 0, 0x107BC, 1);
						gnap._actionStatus = kAS30LookPillMachine;
						break;
					case GRAB_CURSOR:
						gnap.playScratchingHead(Common::Point(8, 5));
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						gnap.playImpossible(Common::Point(8, 5));
						break;
					default:
						break;
					}
				}
			}
			break;

		case kHS30ExitCircus:
			if (gnap._actionStatus < 0) {
				_vm->_isLeavingScene = true;
				if (hasTakenPill)
					_vm->_newSceneNum = 47;
				else
					_vm->_newSceneNum = 26;
				gnap.walkTo(Common::Point(-1, _vm->_hotspotsWalkPos[kHS30ExitCircus].y), 0, 0x107AE, 1);
				gnap._actionStatus = kAS30LeaveScene;
				plat.walkTo(_vm->_hotspotsWalkPos[kHS30ExitCircus] + Common::Point(1, 0), -1, 0x107C2, 1);
			}
			break;

		case kHS30WalkArea1:
			if (gnap._actionStatus < 0)
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
			break;

		default:
			if (_vm->_mouseClickState._left) {
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
				_vm->_mouseClickState._left = false;
			}
			break;
		}

		updateAnimations();

		if (!_vm->isSoundPlaying(0x1093B))
			_vm->playSound(0x1093B, true);

		if (!_vm->_isLeavingScene) {
			plat.updateIdleSequence();
			if (gnap._actionStatus < 0)
				gnap.updateIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(100) + 300;
				if (gnap._actionStatus < 0) {
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
				if (gnap._actionStatus < 0) {
					if (!_vm->isFlag(kGFUnk23) || hasTakenPill)
						gameSys.insertSequence(0x109, 20, 0, 0, kSeqNone, 0, 0, 0);
					else
						gameSys.insertSequence(0x108, 20, 0, 0, kSeqNone, 0, 0, 0);
				}
			}
			_vm->playSoundB();
		}

		_vm->checkGameKeys();

		if (_vm->isKeyStatus1(Common::KEYCODE_BACKSPACE)) {
			_vm->clearKeyStatus1(Common::KEYCODE_BACKSPACE);
			_vm->runMenu();
			updateHotspots();
		}

		_vm->gameUpdateTick();
	}
}

void Scene30::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (gnap._actionStatus) {
		case kAS30LeaveScene:
			_vm->_sceneDone = true;
			break;
		case kAS30UsePillMachine:
			_vm->setGrabCursorSprite(-1);
			gameSys.setAnimation(0x105, gnap._id, 0);
			gameSys.insertSequence(0x105, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gnap._sequenceId = 0x105;
			gnap._sequenceDatNum = 0;
			gnap._actionStatus = kAS30UsePillMachine2;
			break;
		case kAS30UsePillMachine2:
			_vm->hideCursor();
			_vm->setGrabCursorSprite(-1);
			_vm->addFullScreenSprite(0x3F, 255);
			gameSys.removeSequence(0x105, gnap._id, true);
			gameSys.setAnimation(0x102, 256, 0);
			gameSys.insertSequence(0x102, 256, 0, 0, kSeqNone, 0, 0, 0);
			while (gameSys.getAnimationStatus(0) != 2 && !_vm->_gameDone)
				_vm->gameUpdateTick();
			gameSys.setAnimation(0x103, gnap._id, 0);
			gameSys.insertSequence(0x103, gnap._id, 0, 0, kSeqNone, 0, 0, 0);
			_vm->removeFullScreenSprite();
			_vm->showCursor();
			gnap._actionStatus = kAS30UsePillMachine3;
			_vm->invAdd(kItemPill);
			_vm->setFlag(kGFUnk23);
			break;
		case kAS30UsePillMachine3:
			gameSys.setAnimation(0x104, gnap._id, 0);
			gameSys.insertSequence(0x104, gnap._id, makeRid(gnap._sequenceDatNum, 0x103), gnap._id, kSeqSyncWait, 0, 0, 0);
			gnap._sequenceId = 0x104;
			gnap._sequenceDatNum = 0;
			gnap._actionStatus = kAS30UsePillMachine4;
			_vm->setGrabCursorSprite(kItemDiceQuarterHole);
			break;
		case kAS30UsePillMachine4:
			gameSys.insertSequence(0x106, 1, 0, 0, kSeqNone, 0, 0, 0);
			gnap.walkTo(_vm->_hotspotsWalkPos[kHS30PillMachine] + Common::Point(0, 1), -1, 0x107BC, 1);
			gnap._actionStatus = -1;
			break;
		case kAS30LookPillMachine:
			if (_vm->isFlag(kGFUnk23))
				_vm->showFullScreenSprite(0xE3);
			else
				_vm->showFullScreenSprite(0xE2);
			gnap._actionStatus = -1;
			break;
		default:
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
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

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
		gnap.initPos(-1, 8, kDirBottomLeft);
		plat.initPos(-1, 9, kDirIdleRight);
		_vm->endSceneInit();
		gnap.walkTo(Common::Point(3, 8), -1, 0x107BA, 1);
		plat.walkTo(Common::Point(3, 9), -1, 0x107D2, 1);
	} else {
		gnap.initPos(7, 12, kDirBottomRight);
		plat.initPos(6, 12, kDirIdleLeft);
		_vm->endSceneInit();
		gnap.walkTo(Common::Point(7, 8), -1, 0x107BA, 1);
		plat.walkTo(Common::Point(6, 8), -1, 0x107D2, 1);
	}

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS31Device:
			if (gnap._actionStatus < 0 || gnap._actionStatus == kAS31PlatMeasuringClown) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS31Platypus:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemJoint) {
					gnap.useJointOnPlatypus();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playImpossible();
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playMoan1(plat._pos);
						break;
					case GRAB_CURSOR:
						gnap.kissPlatypus(0);
						break;
					case TALK_CURSOR:
						gnap.playBrainPulsating(plat._pos);
						plat.playSequence(plat.getSequenceId());
						break;
					case PLAT_CURSOR:
						gnap.playImpossible();
						break;
					default:
						break;
					}
				}
			}
			break;

		case kHS31MeasuringClown:
			if (gnap._actionStatus < 0 || gnap._actionStatus == kAS31PlatMeasuringClown) {
				if (gnap._actionStatus == kAS31PlatMeasuringClown) {
					if (_vm->_verbCursor == LOOK_CURSOR)
						gnap.playScratchingHead(Common::Point(2, 2));
					else
						gnap.playImpossible();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[kHS31MeasuringClown] + Common::Point(0, 1), 2, 2);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playScratchingHead(Common::Point(2, 2));
						break;
					case GRAB_CURSOR:
						gnap.walkTo(_vm->_hotspotsWalkPos[kHS31MeasuringClown] + Common::Point(0, 1), -1, -1, 1);
						_vm->_hotspots[kHS31WalkArea1]._flags |= SF_WALKABLE;
						gnap.walkTo(_vm->_hotspotsWalkPos[kHS31MeasuringClown], 0, 0x107B9, 1);
						_vm->_hotspots[kHS31WalkArea1]._flags &= ~SF_WALKABLE;
						gnap._actionStatus = kAS31UseMeasuringClown;
						_vm->_timers[4] = 300;
						break;
					case TALK_CURSOR:
						gnap.playImpossible();
						break;
					case PLAT_CURSOR:
						if (!_vm->invHas(kItemBucketWithBeer)) {
							gnap.useDeviceOnPlatypus();
							plat.walkTo(_vm->_hotspotsWalkPos[kHS31MeasuringClown] + Common::Point(0, 1), 1, 0x107C2, 1);
							_vm->_hotspots[kHS31WalkArea1]._flags |= SF_WALKABLE;
							plat.walkTo(_vm->_hotspotsWalkPos[kHS31MeasuringClown], 1, 0x107C2, 1);
							_vm->_hotspots[kHS31WalkArea1]._flags &= ~SF_WALKABLE;
							plat._actionStatus = kAS31PlatMeasuringClown;
							gnap._actionStatus = kAS31PlatMeasuringClown;
							_vm->_timers[4] = 300;
						} else
							gnap.playImpossible();
						break;
					default:
						break;
					}
				}
			}
			break;

		case kHS31BeerBarrel:
			if (gnap._actionStatus < 0 || gnap._actionStatus == kAS31PlatMeasuringClown) {
				if (_vm->_grabCursorSpriteIndex == kItemEmptyBucket && _beerGuyDistracted) {
					_vm->setGrabCursorSprite(-1);
					gnap.walkTo(gnap._pos, -1, gnap.getSequenceId(kGSIdle, _vm->_hotspotsWalkPos[kHS31BeerBarrel]) | 0x10000, 1);
					_clerkMeasureMaxCtr += 5;
					gameSys.insertSequence(0xF8, 59, 0, 0, kSeqNone, 0, 0, 0);
					gnap.playPullOutDevice(Common::Point(6, 8));
					gnap.playUseDevice();
					gnap.walkTo(_vm->_hotspotsWalkPos[kHS31BeerBarrel], 0, 0x107BC, 1);
					gnap._actionStatus = kAS31FillEmptyBucketWithBeer;
					_vm->_timers[4] = 300;
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[kHS31BeerBarrel], 6, 2);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playScratchingHead(Common::Point(6, 2));
						break;
					case GRAB_CURSOR:
						if (_beerGuyDistracted) {
							gnap.playScratchingHead(Common::Point(6, 2));
						} else {
							gnap.walkTo(_vm->_hotspotsWalkPos[kHS31BeerBarrel], 0, 0x107BC, 1);
							gnap._actionStatus = kAS31UseBeerBarrel;
							gnap._idleFacing = kDirUpLeft;
						}
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						gnap.playImpossible();
						break;
					default:
						break;
					}
				}
			}
			break;

		case kHS31ExitCircus:
			if (gnap._actionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 26;
				gnap.walkTo(Common::Point(-1, _vm->_hotspotsWalkPos[kHS31ExitCircus].y), 0, 0x107AE, 1);
				gnap._actionStatus = kAS31LeaveScene;
				plat.walkTo(_vm->_hotspotsWalkPos[kHS31ExitCircus] + Common::Point(1, 0), -1, -1, 1);
			}
			break;

		case kHS31ExitOutsideClown:
			if (gnap._actionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 27;
				gnap.walkTo(Common::Point(-1, _vm->_hotspotsWalkPos[kHS31ExitOutsideClown].y), 0, 0x107AF, 1);
				gnap._actionStatus = kAS31LeaveScene;
				plat.walkTo(_vm->_hotspotsWalkPos[kHS31ExitOutsideClown] + Common::Point(0, 1), -1, 0x107CF, 1);
			}
			break;

		case kHS31WalkArea1:
			if (gnap._actionStatus < 0)
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
			break;

		default:
			if (_vm->_mouseClickState._left) {
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
				_vm->_mouseClickState._left = false;
			}
			break;
		}

		updateAnimations();

		if (!_vm->isSoundPlaying(0x1093B))
			_vm->playSound(0x1093B, true);

		if (!_vm->_isLeavingScene) {
			if (plat._actionStatus < 0)
				plat.updateIdleSequence();
			if (gnap._actionStatus < 0)
				gnap.updateIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(20) + 60;
				if (gnap._actionStatus < 0 && _nextClerkSequenceId == -1) {
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
				if (gnap._actionStatus < 0) {
					if (_vm->getRandom(2) != 0)
						gameSys.insertSequence(0x104, 20, 0, 0, kSeqNone, 0, 0, 0);
					else
						gameSys.insertSequence(0x103, 20, 0, 0, kSeqNone, 0, 0, 0);
				}
			}
			_vm->playSoundB();
		}

		_vm->checkGameKeys();

		if (_vm->isKeyStatus1(Common::KEYCODE_BACKSPACE)) {
			_vm->clearKeyStatus1(Common::KEYCODE_BACKSPACE);
			_vm->runMenu();
			updateHotspots();
		}

		_vm->gameUpdateTick();
	}
}

void Scene31::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (gnap._actionStatus) {
		case kAS31UseBeerBarrel:
			_nextClerkSequenceId = 0xFE;
			break;
		case kAS31FillEmptyBucketWithBeer:
			gameSys.setAnimation(0x102, 59, 0);
			gameSys.insertSequence(0x102, 59, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gnap._pos = Common::Point(5, 7);
			gnap._sequenceDatNum = 0;
			gnap._sequenceId = 0x102;
			gnap._id = 59;
			gnap._actionStatus = kAS31FillEmptyBucketWithBeerDone;
			break;
		case kAS31FillEmptyBucketWithBeerDone:
			gnap._idleFacing = kDirBottomLeft;
			gnap.playPullOutDevice();
			gnap.playUseDevice();
			gameSys.insertSequence(0xF9, 59, 0xF8, 59, kSeqSyncWait, 0, 0, 0);
			gnap._actionStatus = -1;
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
			gnap._actionStatus = -1;
			break;
		default:
			break;
		}
	}

	if (gameSys.getAnimationStatus(1) == 2) {
		gameSys.setAnimation(0, 0, 1);
		if (plat._actionStatus == kAS31PlatMeasuringClown) {
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
				if (gnap._actionStatus != 5)
					plat._actionStatus = -1;
				_vm->_timers[0] = 40;
				gameSys.insertSequence(0xFD, 39, _currClerkSequenceId, 39, kSeqSyncWait, 0, 0, 0);
				_currClerkSequenceId = 0xFD;
				_nextClerkSequenceId = -1;
				if (gnap._actionStatus != kAS31FillEmptyBucketWithBeerDone && gnap._actionStatus != kAS31FillEmptyBucketWithBeer)
					gnap._actionStatus = -1;
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
			gnap._actionStatus = -1;
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
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	_vm->playSound(0x1091C, true);
	_vm->startSoundTimerC(5);
	_vm->queueInsertDeviceIcon();
	_vm->_timers[4] = _vm->getRandom(100) + 300;

	if (_vm->_prevSceneNum == 33) {
		gnap.initPos(11, 6, kDirBottomLeft);
		plat.initPos(12, 6, kDirIdleRight);
		_vm->endSceneInit();
		plat.walkTo(Common::Point(9, 6), -1, 0x107D2, 1);
		gnap.walkTo(Common::Point(8, 6), -1, 0x107BA, 1);
	} else {
		gnap.initPos(1, 6, kDirBottomRight);
		plat.initPos(1, 7, kDirIdleLeft);
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
			if (gnap._actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS32Platypus:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playImpossible(plat._pos);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playMoan1(plat._pos);
						break;
					case GRAB_CURSOR:
						gnap.kissPlatypus(0);
						break;
					case TALK_CURSOR:
						gnap.playBrainPulsating(plat._pos);
						plat.playSequence(plat.getSequenceId());
						break;
					case PLAT_CURSOR:
						gnap.playImpossible(plat._pos);
						break;
					default:
						break;
					}
				}
			}
			break;

		case kHS32ExitTruck:
			if (gnap._actionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->setGrabCursorSprite(-1);
				gnap.walkTo(_vm->_hotspotsWalkPos[kHS32ExitTruck], 0, 0x107AB, 1);
				gnap._actionStatus = kAS32LeaveScene;
				plat.walkTo(_vm->_hotspotsWalkPos[kHS32ExitTruck] + Common::Point(0, 1), -1, 0x107CD, 1);
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
			if (gnap._actionStatus < 0)
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
			break;

		default:
			break;
		}

		if (_vm->_mouseClickState._left && gnap._actionStatus < 0) {
			gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
			_vm->_mouseClickState._left = 0;
		}

		updateAnimations();

		if (!_vm->isSoundPlaying(0x1091C))
			_vm->playSound(0x1091C, true);

		if (!_vm->_isLeavingScene) {
			if (plat._actionStatus < 0)
				plat.updateIdleSequence();
			if (gnap._actionStatus < 0)
				gnap.updateIdleSequence();
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

		if (_vm->isKeyStatus1(Common::KEYCODE_BACKSPACE)) {
			_vm->clearKeyStatus1(Common::KEYCODE_BACKSPACE);
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
		if (_vm->_gnap->_actionStatus == kAS32LeaveScene)
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
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

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
		gnap.initPos(11, 7, kDirBottomLeft);
		plat.initPos(12, 7, kDirIdleRight);
		_vm->endSceneInit();
		gnap.walkTo(Common::Point(8, 7), -1, 0x107BA, 1);
		plat.walkTo(Common::Point(9, 7), -1, 0x107D2, 1);
		break;
	case 37:
		gnap.initPos(7, 7, kDirBottomRight);
		plat.initPos(8, 7, kDirIdleLeft);
		_vm->endSceneInit();
		break;
	case 32:
		gnap.initPos(-1, 6, kDirBottomRight);
		plat.initPos(-1, 7, kDirIdleLeft);
		_vm->endSceneInit();
		plat.walkTo(Common::Point(2, 7), -1, 0x107C2, 1);
		gnap.walkTo(Common::Point(2, 8), -1, 0x107B9, 1);
		break;
	default:
		gnap.initPos(3, 7, kDirBottomRight);
		plat.initPos(2, 7, kDirIdleLeft);
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
			if (gnap._actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS33Platypus:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playImpossible(plat._pos);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playMoan1(plat._pos);
						break;
					case GRAB_CURSOR:
						gnap.kissPlatypus(0);
						break;
					case TALK_CURSOR:
						gnap.playBrainPulsating(plat._pos);
						plat.playSequence(plat.getSequenceId());
						break;
					case PLAT_CURSOR:
						gnap.playImpossible(plat._pos);
						break;
					default:
						break;
					}
				}
			}
			break;

		case kHS33Chicken:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(Common::Point(7, 9), 9, 8);
				} else {
					switch (_vm->_verbCursor) {
					case GRAB_CURSOR:
						gnap._idleFacing = kDirBottomRight;
						if (gnap.walkTo(_vm->_hotspotsWalkPos[kHS33Chicken], 0, gnap.getSequenceId(kGSIdle, Common::Point(0, 0)) | 0x10000, 1))
							gnap._actionStatus = kAS33UseChicken;
						else
							gnap._actionStatus = -1;
						break;
					case TALK_CURSOR:
						gnap._idleFacing = kDirBottomRight;
						gnap.walkTo(_vm->_hotspotsWalkPos[kHS33Chicken], 0, gnap.getSequenceId(kGSBrainPulsating, Common::Point(0, 0)) | 0x10000, 1);
						gnap._actionStatus = kAS33TalkChicken;
						break;
					case LOOK_CURSOR:
					case PLAT_CURSOR:
						gnap.playImpossible();
						break;
					default:
						break;
					}
				}
			}
			break;

		case kHS33ExitHouse:
			if (gnap._actionStatus < 0) {
				_vm->_isLeavingScene = true;
				gnap._actionStatus = kAS33LeaveScene;
				_vm->_newSceneNum = 37;
				if (gnap._pos.x > 6)
					gnap.walkTo(gnap._pos, 0, 0x107AD, 1);
				else
					gnap.walkTo(Common::Point(6, 7), 0, 0x107B1, 1);
			}
			break;

		case kHS33ExitBarn:
			if (gnap._actionStatus < 0) {
				_vm->_isLeavingScene = true;
				gnap._actionStatus = kAS33LeaveScene;
				_vm->_newSceneNum = 35;
				if (gnap._pos.x > 7)
					gnap.walkTo(gnap._pos, 0, 0x107AD, 1);
				else
					gnap.walkTo(Common::Point(7, 7), 0, 0x107B1, 1);
			}
			break;

		case kHS33ExitCreek:
			if (gnap._actionStatus < 0) {
				_vm->_isLeavingScene = true;
				gnap.walkTo(_vm->_hotspotsWalkPos[kHS33ExitCreek], 0, 0x107AB, 1);
				gnap._actionStatus = kAS33LeaveScene;
				plat.walkTo(_vm->_hotspotsWalkPos[kHS33ExitCreek], -1, 0x107CD, 1);
				_vm->_newSceneNum = 34;
			}
			break;

		case kHS33ExitPigpen:
			if (gnap._actionStatus < 0) {
				_vm->_isLeavingScene = true;
				gnap.walkTo(_vm->_hotspotsWalkPos[kHS33ExitPigpen], 0, 0x107AF, 1);
				gnap._actionStatus = kAS33LeaveScene;
				plat.walkTo(_vm->_hotspotsWalkPos[kHS33ExitPigpen], -1, 0x107CF, 1);
				_vm->_newSceneNum = 32;
			}
			break;

		case kHS33WalkArea1:
		case kHS33WalkArea2:
			if (gnap._actionStatus < 0)
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
			break;

		default:
			if (_vm->_mouseClickState._left && gnap._actionStatus < 0) {
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
				_vm->_mouseClickState._left = false;
			}
			break;
		}

		updateAnimations();

		if (!_vm->isSoundPlaying(0x1091C))
			_vm->playSound(0x1091C, true);

		if (!_vm->_isLeavingScene) {
			if (plat._actionStatus < 0)
				plat.updateIdleSequence();
			if (gnap._actionStatus < 0)
				gnap.updateIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(100) + 300;
				if (_vm->getRandom(2) != 0)
					gameSys.insertSequence(0x83, 256, 0, 0, kSeqNone, 0, 0, 0);
				else
					gameSys.insertSequence(0x82, 256, 0, 0, kSeqNone, 0, 0, 0);
			}
			if (!_vm->_timers[5] && _nextChickenSequenceId == -1 && gnap._actionStatus != kAS33TalkChicken && gnap._actionStatus != kAS33UseChicken) {
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

		if (_vm->isKeyStatus1(Common::KEYCODE_BACKSPACE)) {
			_vm->clearKeyStatus1(Common::KEYCODE_BACKSPACE);
			_vm->runMenu();
			updateHotspots();
		}
		_vm->gameUpdateTick();
	}
}

void Scene33::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;

	if (gameSys.getAnimationStatus(0) == 2) {
		switch (gnap._actionStatus) {
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
			gameSys.insertSequence(0x107B5, gnap._id, 0x81, 179, kSeqSyncWait, 0, 75 * gnap._pos.x - gnap._gridX, 48 * gnap._pos.y - gnap._gridY);
			gnap._sequenceId = 0x7B5;
			gnap._sequenceDatNum = 1;
			_currChickenSequenceId = 0x7E;
			gameSys.setAnimation(0x7E, 179, 2);
			gameSys.insertSequence(_currChickenSequenceId, 179, 0, 0, kSeqNone, 0, 0, 0);
			gnap._actionStatus = -1;
			_vm->_timers[5] = 30;
			break;
		default:
			gnap._actionStatus = -1;
			break;
		}
	}

	if (gameSys.getAnimationStatus(2) == 2) {
		if (_nextChickenSequenceId == 0x81) {
			gameSys.setAnimation(_nextChickenSequenceId, 179, 0);
			gameSys.insertSequence(_nextChickenSequenceId, 179, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gameSys.removeSequence(_currChickenSequenceId, 179, true);
			_nextChickenSequenceId = -1;
			_currChickenSequenceId = -1;
			gnap._actionStatus = kAS33UseChickenDone;
			_vm->_timers[5] = 500;
		} else if (_nextChickenSequenceId == 0x7F) {
			gameSys.setAnimation(_nextChickenSequenceId, 179, 2);
			gameSys.insertSequence(_nextChickenSequenceId, 179, _currChickenSequenceId, 179, kSeqSyncWait, 0, 0, 0);
			_currChickenSequenceId = _nextChickenSequenceId;
			_nextChickenSequenceId = -1;
			gnap._actionStatus = -1;
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
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

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
	if (plat._actionStatus == kAS38PlatypusHoldingTrapDoor)
		_vm->_hotspots[kHS38Platypus]._flags = SF_WALKABLE | SF_DISABLED;
	if (plat._actionStatus == kAS38PlatypusHoldingTrapDoor)
		_vm->_hotspots[kHS38ExitCave]._flags = SF_EXIT_D_CURSOR;
	else if (gnap._actionStatus == kAS38HoldingHuntingTrophy)
		_vm->_hotspots[kHS38ExitCave]._flags = SF_EXIT_D_CURSOR;
	if (plat._actionStatus == kAS38PlatypusHoldingTrapDoor)
		_vm->_hotspots[kHS38TrapDoorLid1]._flags = SF_DISABLED;
	else if (gnap._actionStatus == kAS38HoldingHuntingTrophy)
		_vm->_hotspots[kHS38TrapDoorLid1]._flags = SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	if (plat._actionStatus == kAS38PlatypusHoldingTrapDoor)
		_vm->_hotspots[kHS38TrapDoorLid2]._flags = SF_DISABLED;
	else if (gnap._actionStatus == kAS38HoldingHuntingTrophy)
		_vm->_hotspots[kHS38TrapDoorLid2]._flags = SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	if (plat._actionStatus == kAS38PlatypusHoldingTrapDoor)
		_vm->_hotspots[kHS38WalkArea6]._flags = SF_NONE;
	_vm->_hotspotsCount = 13;
}

void Scene38::run() {
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	_vm->queueInsertDeviceIcon();
	_vm->_gameSys->insertSequence(0x9B, 0, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->_prevSceneNum == 39) {
		gnap.initPos(3, 7, kDirBottomLeft);
		plat.initPos(4, 7, kDirIdleRight);
	} else {
		gnap.initPos(3, 8, kDirBottomRight);
		plat.initPos(4, 8, kDirIdleLeft);
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
			if (gnap._actionStatus == kAS38HoldingHuntingTrophy) {
				gnap._actionStatus = kAS38ReleaseHuntingTrophy;
			} else if (_vm->_grabCursorSpriteIndex >= 0) {
				gnap.playImpossible(plat._pos);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					gnap.playMoan1(plat._pos);
					break;
				case GRAB_CURSOR:
					gnap.kissPlatypus(0);
					break;
				case TALK_CURSOR:
					gnap.playBrainPulsating(plat._pos);
					plat.playSequence(plat.getSequenceId());
					break;
				case PLAT_CURSOR:
					gnap.playImpossible(plat._pos);
					break;
				default:
					break;
				}
			}
			break;

		case kHS38ExitHouse:
			if (gnap._actionStatus == kAS38HoldingHuntingTrophy) {
				gnap._actionStatus = kAS38ReleaseHuntingTrophy;
			} else {
				_vm->_isLeavingScene = true;
				gnap.walkTo(Common::Point(-1, -1), 0, 0x107AE, 1);
				gnap._actionStatus = kAS38LeaveScene;
				_vm->_newSceneNum = 37;
			}
			break;

		case kHS38ExitCave:
			if (gnap._actionStatus == kAS38HoldingHuntingTrophy) {
				gnap._actionStatus = kAS38ReleaseHuntingTrophy;
				if (plat._actionStatus == kAS38PlatypusHoldingTrapDoor)
					_vm->_isLeavingScene = true;
			} else if (plat._actionStatus == kAS38PlatypusHoldingTrapDoor) {
				_vm->_sceneWaiting = false;
				_vm->_isLeavingScene = true;
				gnap.walkTo(Common::Point(5, 7), 0, 0x107BB, 1);
				_vm->_newSceneNum = 39;
				gnap._actionStatus = kAS38ExitCave;
			}
			break;

		case kHS38TrapDoorLid1:
		case kHS38TrapDoorLid2:
			if (gnap._actionStatus == kAS38HoldingHuntingTrophy) {
				if (_vm->_verbCursor == PLAT_CURSOR && plat._actionStatus != kAS38PlatypusHoldingTrapDoor)
					gnap._actionStatus = kAS38UsePlatypusWithTrapDoor;
				else
					gnap._actionStatus = kAS38ReleaseHuntingTrophy;
			}
			break;

		case kHS38HuntingTrophy:
			if (gnap._actionStatus != kAS38HoldingHuntingTrophy) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(Common::Point(3, 6), 2, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playScratchingHead();
						break;
					case GRAB_CURSOR:
						if (plat._actionStatus == kAS38PlatypusHoldingTrapDoor)
							gnap.playImpossible();
						else {
							gnap.walkTo(Common::Point(3, 6), 0, 0x107BB, 1);
							plat.walkTo(Common::Point(4, 8), -1, -1, 1);
							gnap._actionStatus = kAS38UseHuntingTrophy;
						}
						break;
					case TALK_CURSOR:
						gnap.playBrainPulsating(Common::Point(2, 0));
						break;
					case PLAT_CURSOR:
						gnap.playImpossible();
						break;
					default:
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
			if (gnap._actionStatus == kAS38HoldingHuntingTrophy)
				gnap._actionStatus = kAS38ReleaseHuntingTrophy;
			else if (gnap._actionStatus < 0)
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
			break;

		default:
			if (_vm->_mouseClickState._left) {
				if (gnap._actionStatus == kAS38HoldingHuntingTrophy)
					gnap._actionStatus = kAS38ReleaseHuntingTrophy;
				else if (gnap._actionStatus < 0)
					gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
				_vm->_mouseClickState._left = false;
			}
			break;
		}

		updateAnimations();

		if (!_vm->_isLeavingScene) {
			plat.updateIdleSequence();
			gnap.updateIdleSequence();
		}

		_vm->checkGameKeys();

		if (_vm->isKeyStatus1(Common::KEYCODE_BACKSPACE)) {
			_vm->clearKeyStatus1(Common::KEYCODE_BACKSPACE);
			_vm->runMenu();
			updateHotspots();
		}

		_vm->gameUpdateTick();
	}
}

void Scene38::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (gnap._actionStatus) {
		case kAS38LeaveScene:
			_vm->_sceneDone = true;
			break;
		case kAS38ExitCave:
			gameSys.removeSequence(plat._sequenceId | (plat._sequenceDatNum << 16), plat._id, true);
			gameSys.insertSequence(0xA3, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gnap._sequenceId = 0xA3;
			gnap._sequenceDatNum = 0;
			gameSys.setAnimation(0xA3, gnap._id, 0);
			gnap._actionStatus = kAS38LeaveScene;
			break;
		case kAS38UseHuntingTrophy:
			gameSys.removeSequence(0x9B, 0, true);
			gameSys.insertSequence(0x9C, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gnap._sequenceId = 0x9C;
			gnap._sequenceDatNum = 0;
			gameSys.setAnimation(0x9C, gnap._id, 0);
			gnap._actionStatus = kAS38HoldingHuntingTrophy;
			updateHotspots();
			break;
		case kAS38HoldingHuntingTrophy:
			if (plat._actionStatus != kAS38PlatypusHoldingTrapDoor)
				_vm->_sceneWaiting = true;
			if (gnap._sequenceId == 0xA4) {
				gameSys.insertSequence(0x9D, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
				gnap._sequenceId = 0x9D;
			} else {
				gameSys.insertSequence(0xA4, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
				gnap._sequenceId = 0xA4;
			}
			gnap._sequenceDatNum = 0;
			gameSys.setAnimation(gnap._sequenceId, gnap._id, 0);
			break;
		case kAS38ReleaseHuntingTrophy:
			if (gnap._sequenceId == 0x9E) {
				gameSys.insertSequence(0x9B, 0, 0, 0, kSeqNone, 0, 0, 0);
				gnap._actionStatus = -1;
			} else if (plat._actionStatus == kAS38PlatypusHoldingTrapDoor) {
				gameSys.insertSequence(0xA0, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
				gnap._sequenceId = 0xA0;
				gnap._sequenceDatNum = 0;
				gnap._pos = Common::Point(3, 6);
				gnap._idleFacing = kDirBottomRight;
				if (_vm->_isLeavingScene) {
					_vm->_sceneWaiting = false;
					gnap.walkTo(Common::Point(5, 7), 0, 0x107BB, 1);
					_vm->_newSceneNum = 39;
					gnap._actionStatus = kAS38ExitCave;
				} else {
					gnap._actionStatus = -1;
				}
			} else {
				gameSys.insertSequence(0x9E, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
				gnap._sequenceId = 0x9E;
				gnap._sequenceDatNum = 0;
				gnap._pos = Common::Point(3, 6);
				gnap._idleFacing = kDirBottomRight;
				gameSys.setAnimation(0x9E, gnap._id, 0);
				_vm->_sceneWaiting = false;
				updateHotspots();
			}
			break;
		case kAS38UsePlatypusWithTrapDoor:
			_vm->_sceneWaiting = false;
			gameSys.insertSequence(0x9F, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gnap._sequenceId = 0x9F;
			gnap._sequenceDatNum = 0;
			gameSys.setAnimation(0x9F, gnap._id, 0);
			gnap._actionStatus = kAS38HoldingHuntingTrophy;
			if (plat._idleFacing != kDirIdleLeft)
				plat.playSequence(0x107D5);
			else
				plat.playSequence(0x107D4);
			plat.walkTo(Common::Point(8, 7), -1, 0x107D2, 1);
			gameSys.insertSequence(0xA1, gnap._id + 1, plat._sequenceId | (plat._sequenceDatNum << 16), plat._id, kSeqSyncWait, 0, 0, 0);
			plat._sequenceId = 0xA1;
			plat._sequenceDatNum = 0;
			plat._id = gnap._id + 1;
			gameSys.setAnimation(0xA1, gnap._id + 1, 1);
			plat._actionStatus = kAS38PlatypusHoldingTrapDoor;
			updateHotspots();
			break;
		default:
			break;
		}
	}

	if (gameSys.getAnimationStatus(1) == 2) {
		gameSys.setAnimation(0, 0, 1);
		if (plat._actionStatus == kAS38PlatypusHoldingTrapDoor) {
			gameSys.insertSequence(0xA2, plat._id, plat._sequenceId | (plat._sequenceDatNum << 16), plat._id, kSeqSyncWait, 0, 0, 0);
			plat._sequenceId = 0xA2;
			plat._sequenceDatNum = 0;
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
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	// Bug in the original? Timer was never initialized.
	_vm->_timers[5] = 0;

	_vm->queueInsertDeviceIcon();
	_currGuySequenceId = 0x33;

	gameSys.setAnimation(0x33, 21, 3);
	gameSys.insertSequence(_currGuySequenceId, 21, 0, 0, kSeqNone, 0, 0, 0);
	gameSys.insertSequence(0x34, 21, 0, 0, kSeqLoop, 0, 0, 0);

	_nextGuySequenceId = -1;
	if (_vm->_prevSceneNum == 38) {
		gnap.initPos(3, 7, kDirUpRight);
		plat.initPos(2, 7, kDirUpLeft);
		_vm->endSceneInit();
	} else {
		gnap.initPos(4, 7, kDirBottomRight);
		plat.initPos(5, 7, kDirIdleLeft);
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
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playImpossible(plat._pos);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playMoan1(plat._pos);
						break;
					case GRAB_CURSOR:
						gnap.kissPlatypus(0);
						break;
					case TALK_CURSOR:
						gnap.playBrainPulsating(plat._pos);
						plat.playSequence(plat.getSequenceId());
						break;
					case PLAT_CURSOR:
						gnap.playImpossible(plat._pos);
						break;
					default:
						break;
					}
				}
			}
			break;

		case kHS39ExitUfoParty:
			if (gnap._actionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_sceneDone = true;
				gnap.walkTo(gnap._pos, 0, 0x107AB, 1);
				gnap._actionStatus = kAS39LeaveScene;
				_vm->_newSceneNum = 40;
			}
			break;

		case kHS39Sign:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playImpossible();
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.walkTo(_vm->_hotspotsWalkPos[kHS39Sign], 0, -1, 1);
						gnap.playIdle(_vm->_hotspotsWalkPos[kHS39Sign]);
						_vm->showFullScreenSprite(0x1C);
						break;
					case GRAB_CURSOR:
					case TALK_CURSOR:
					case PLAT_CURSOR:
						gnap.playImpossible();
						break;
					default:
						break;
					}
				}
			}
			break;

		case kHS39ExitInsideHouse:
			if (gnap._actionStatus < 0) {
				_vm->_sceneDone = true;
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 38;
			}
			break;

		case kHS39WalkArea1:
		case kHS39WalkArea2:
			if (gnap._actionStatus < 0)
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
			break;

		default:
			if (_vm->_mouseClickState._left && gnap._actionStatus < 0) {
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
				_vm->_mouseClickState._left = false;
			}
			break;
		}

		updateAnimations();

		if (!_vm->_isLeavingScene) {
			if (plat._actionStatus < 0)
				plat.updateIdleSequence();
			if (gnap._actionStatus < 0)
				gnap.updateIdleSequence();
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
				default:
					break;
				}
			}
		}

		_vm->checkGameKeys();

		if (_vm->isKeyStatus1(Common::KEYCODE_BACKSPACE)) {
			_vm->clearKeyStatus1(Common::KEYCODE_BACKSPACE);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[5] = _vm->getRandom(20) + 50;
		}

		_vm->gameUpdateTick();
	}
}

void Scene39::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		if (gnap._actionStatus == kAS39LeaveScene)
			_vm->_sceneDone = true;
		else
			gnap._actionStatus = -1;
	}

	if (gameSys.getAnimationStatus(3) == 2 && _nextGuySequenceId != -1) {
		gameSys.setAnimation(_nextGuySequenceId, 21, 3);
		gameSys.insertSequence(_nextGuySequenceId, 21, _currGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
		_currGuySequenceId = _nextGuySequenceId;
		_nextGuySequenceId = -1;
	}
}

} // End of namespace Gnap
