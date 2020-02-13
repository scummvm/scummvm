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
#include "gnap/character.h"
#include "gnap/gamesys.h"
#include "gnap/resource.h"
#include "gnap/scenes/group0.h"

namespace Gnap {

Scene01::Scene01(GnapEngine *vm) : Scene(vm) {
	_pigsIdCtr = 0;
	_smokeIdCtr = 0;
	_spaceshipSurface = nullptr;
}

Scene01::~Scene01() {
	delete _spaceshipSurface;
}

int Scene01::init() {
	GameSys& gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 3);
	return 0x88;
}

void Scene01::updateHotspots() {
	_vm->setHotspot(kHS01Platypus, 0, 0, 0, 0, SF_DISABLED | SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS01ExitTruck, 780, 226, 800, 455, SF_EXIT_R_CURSOR | SF_WALKABLE, 10, 6);
	_vm->setHotspot(kHS01Mud, 138, 282, 204, 318, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 5);
	_vm->setHotspot(kHS01Pigs, 408, 234, 578, 326, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 4);
	_vm->setHotspot(kHS01Spaceship, 0, 200, 94, 292, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 1, 6);
	_vm->setHotspot(kHS01WalkArea1, 0, 0, 162, 426);
	_vm->setHotspot(kHS01WalkArea2, 162, 0, 237, 396);
	_vm->setHotspot(kHS01WalkArea3, 237, 0, 319, 363);
	_vm->setHotspot(kHS01WalkArea4, 520, 0, 800, 404);
	_vm->setHotspot(kHS01WalkArea5, 300, 447, 800, 600);
	_vm->setHotspot(kHS01WalkArea6, 678, 0, 800, 404);
	_vm->setHotspot(kHS01WalkArea7, 0, 0, 520, 351);
	_vm->setHotspot(kHS01WalkArea8, 0, 546, 300, 600);
	_vm->setDeviceHotspot(kHS01Device, -1, -1, -1, -1);
	if (_vm->isFlag(kGFPlatypus))
		_vm->_hotspots[kHS01Platypus]._flags = SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	if (_vm->isFlag(kGFMudTaken))
		_vm->_hotspots[kHS01Mud]._flags = SF_WALKABLE | SF_DISABLED;
	_vm->_hotspotsCount = 14;
}

void Scene01::run() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	_vm->playSound(0x1091C, true);
	_vm->startSoundTimerC(5);

	gameSys.setAnimation(134, 20, 4);
	gameSys.insertSequence(134, 20, 0, 0, kSeqNone, 0, 0, 0);

	gameSys.setAnimation(0x7F, 40, 2);
	gameSys.insertSequence(0x7F, 40, 0, 0, kSeqNone, 0, 0, 0);

	_vm->_timers[4] = _vm->getRandom(100) + 300;

	if (!_vm->isFlag(kGFMudTaken))
		gameSys.insertSequence(129, 40, 0, 0, kSeqNone, 0, 0, 0);

	_vm->queueInsertDeviceIcon();

	if (_vm->_prevSceneNum == 2) {
		gnap.initPos(11, 6, kDirBottomLeft);
		if (_vm->isFlag(kGFPlatypus))
			plat.initPos(12, 6, kDirIdleRight);
		_vm->endSceneInit();
		if (_vm->isFlag(kGFPlatypus))
			plat.walkTo(Common::Point(9, 6), -1, 0x107C2, 1);
		gnap.walkTo(Common::Point(8, 6), -1, 0x107B9, 1);
	} else {
		gnap.initPos(1, 6, kDirBottomRight);
		if (_vm->isFlag(kGFPlatypus))
			plat.initPos(1, 7, kDirIdleLeft);
		_vm->endSceneInit();
	}

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
		_vm->testWalk(0, 3, -1, -1, -1, -1);

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS01Device:
			if (gnap._actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS01Platypus:
			if (gnap._actionStatus < 0 && _vm->isFlag(kGFPlatypus)) {
				if (_vm->_grabCursorSpriteIndex == kItemDisguise) {
					gnap.useDisguiseOnPlatypus();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playImpossible();
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						if (_vm->isFlag(kGFKeysTaken))
							gnap.playMoan1(plat._pos);
						else
							gnap.playScratchingHead(plat._pos);
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

		case kHS01Spaceship:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[4], 0, 2);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap._idleFacing = kDirUpLeft;
						if (gnap.walkTo(_vm->_hotspotsWalkPos[4], 0, gnap.getSequenceId(kGSIdle, Common::Point(0, 0)) | 0x10000, 1))
							gnap._actionStatus = kAS01LookSpaceship;
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

		case kHS01Mud:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[2], 2, 3);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playScratchingHead(Common::Point(3, 3));
						break;
					case GRAB_CURSOR:
						gnap.walkTo(_vm->_hotspotsWalkPos[2], 0, gnap.getSequenceId(kGSIdle, Common::Point(2, 3)) | 0x10000, 1);
						gnap._actionStatus = kAS01TakeMud;
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

		case kHS01Pigs:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[3], 7, 2);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap._idleFacing = kDirUpRight;
						gnap.walkTo(_vm->_hotspotsWalkPos[3], 0, gnap.getSequenceId(kGSIdle, Common::Point(7, 2)) | 0x10000, 1);
						gnap._actionStatus = kAS01LookPigs;
						break;
					case GRAB_CURSOR:
						gnap._idleFacing = kDirUpRight;
						gnap.walkTo(_vm->_hotspotsWalkPos[3], 0, gnap.getSequenceId(kGSIdle, Common::Point(7, 2)) | 0x10000, 1);
						gnap._actionStatus = kAS01UsePigs;
						break;
					case TALK_CURSOR:
						gnap._idleFacing = kDirUpRight;
						gnap.walkTo(_vm->_hotspotsWalkPos[3], 0, gnap.getSequenceId(kGSBrainPulsating, Common::Point(7, 2)) | 0x10000, 1);
						gnap._actionStatus = kAS01LookPigs;
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

		case kHS01ExitTruck:
			if (gnap._actionStatus < 0) {
				_vm->_isLeavingScene = true;
				gnap.walkTo(_vm->_hotspotsWalkPos[1], 0, 0x107AB, 1);
				gnap._actionStatus = kAS01LeaveScene;
				if (_vm->isFlag(kGFPlatypus))
					plat.walkTo(_vm->_hotspotsWalkPos[1] + Common::Point(0, 1), -1, 0x107CD, 1);
				_vm->_newSceneNum = 2;
			}
			break;

		case kHS01WalkArea1:
		case kHS01WalkArea2:
		case kHS01WalkArea3:
		case kHS01WalkArea4:
		case kHS01WalkArea5:
		case kHS01WalkArea6:
		case kHS01WalkArea7:
		case kHS01WalkArea8:
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
			if (plat._actionStatus < 0 && _vm->isFlag(kGFPlatypus))
				plat.updateIdleSequence();
			if (gnap._actionStatus < 0)
				gnap.updateIdleSequence();
			if (_vm->_timers[4] == 0) {
				// Update bird animation
				_vm->_timers[4] = _vm->getRandom(100) + 300;
				if (_vm->getRandom(1) == 0)
					gameSys.insertSequence(0x84, 180, 0, 0, kSeqNone, 0, 0, 0);
				else
					gameSys.insertSequence(0x83, 180, 0, 0, kSeqNone, 0, 0, 0);
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

void Scene01::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (gnap._actionStatus) {
		case kAS01LookSpaceship:
			_spaceshipSurface = gameSys.createSurface(47);
			gameSys.insertSpriteDrawItem(_spaceshipSurface, 0, 0, 255);
			gameSys.setAnimation(133, 256, 0);
			gameSys.insertSequence(133, 256, 0, 0, kSeqNone, 0, 0, 0);
			gnap._actionStatus = kAS01LookSpaceshipDone;
			break;

		case kAS01LookSpaceshipDone:
			gameSys.removeSequence(133, 256, true);
			gameSys.removeSpriteDrawItem(_spaceshipSurface, 255);
			_vm->deleteSurface(&_spaceshipSurface);
			gnap._actionStatus = -1;
			break;

		case kAS01LeaveScene:
			_vm->_sceneDone = true;
			break;

		case kAS01TakeMud:
			gnap.playPullOutDevice(Common::Point(2, 3));
			gnap.playUseDevice();
			gameSys.insertSequence(128, 40, 129, 40, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(128, 40, 3);
			gnap._actionStatus = -1;
			break;

		case kAS01LookPigs:
			_vm->playSound(0x8A, false);
			_vm->playSound(0x8B, false);
			_vm->playSound(0x8C, false);
			gnap._actionStatus = -1;
			break;

		case kAS01UsePigs:
			gnap.playPullOutDevice(Common::Point(7, 2));
			gnap.playUseDevice();
			gameSys.insertSequence(135, 39, 0, 0, kSeqNone, 25, _vm->getRandom(140) - 40, 0);
			gnap._actionStatus = -1;
			break;

		default:
			break;
		}
	}

	if (gameSys.getAnimationStatus(3) == 2) {
		gameSys.setAnimation(0, 0, 3);
		_vm->invAdd(kItemMud);
		_vm->setGrabCursorSprite(kItemMud);
		_vm->setFlag(kGFMudTaken);
		updateHotspots();
	}

	if (gameSys.getAnimationStatus(4) == 2) {
		_smokeIdCtr = (_smokeIdCtr + 1) % 2;
		gameSys.setAnimation(0x86, _smokeIdCtr + 20, 4);
		gameSys.insertSequence(0x86, _smokeIdCtr + 20,
			0x86, (_smokeIdCtr + 1) % 2 + 20,
			kSeqSyncWait, 0, 0, 0);
	}

	if (gameSys.getAnimationStatus(2) == 2) {
		_pigsIdCtr = (_pigsIdCtr + 1) % 2;
		gameSys.setAnimation(0x7F, _pigsIdCtr + 40, 2);
		gameSys.insertSequence(0x7F, _pigsIdCtr + 40,
			0x7F, (_pigsIdCtr + 1) % 2 + 40,
			kSeqSyncWait, 0, 0, 0);
	}
}

/*****************************************************************************/

Scene02::Scene02(GnapEngine *vm) : Scene(vm) {
	_truckGrillCtr = 0;
	_nextChickenSequenceId = 0;
	_currChickenSequenceId = 0;
	_gnapTruckSequenceId = 0;
}

int Scene02::init() {
	_vm->_gameSys->setAnimation(0, 0, 0);
	return _vm->isFlag(kGFTruckKeysUsed) ? 0x15A : 0x15B;
}

void Scene02::updateHotspots() {
	_vm->setHotspot(kHS02Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	if (!_vm->isFlag(kGFPlatypus))
		_vm->_hotspots[kHS02Platypus]._flags |= SF_DISABLED;
	_vm->setHotspot(kHS02Chicken, 606, 455, 702, 568, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 7, 8);
	_vm->setHotspot(kHS02Truck1, 385, 258, 464, 304, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 5);
	_vm->setHotspot(kHS02Truck2, 316, 224, 390, 376, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 5, 6);
	_vm->setHotspot(kHS02TruckGrill, 156, 318, 246, 390, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 2, 7);
	_vm->setHotspot(kHS02ExitHouse, 480, 120, 556, 240, SF_EXIT_U_CURSOR, 7, 5);
	_vm->setHotspot(kHS02ExitBarn, 610, 0, 800, 164, SF_EXIT_U_CURSOR, 10, 5);
	_vm->setHotspot(kHS02ExitCreek, 780, 336, 800, 556, SF_EXIT_R_CURSOR | SF_WALKABLE, 10, 8);
	_vm->setHotspot(kHS02ExitPigpen, 0, 300, 20, 600, SF_EXIT_L_CURSOR | SF_WALKABLE, 0, 8);
	_vm->setHotspot(kHS02WalkArea1, 92, 140, 304, 430, SF_NONE, 3, 1);
	_vm->setHotspot(kHS02WalkArea2, 0, 0, 800, 380);
	_vm->setHotspot(kHS02WalkArea3, 0, 0, 386, 445);
	_vm->setHotspot(kHS02WalkArea4, 386, 0, 509, 410);
	_vm->setDeviceHotspot(kHS02Device, -1, -1, -1, -1);
	_vm->_hotspotsCount = 14;
}

void Scene02::run() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	_vm->playSound(0x1091C, true);
	_vm->startSoundTimerC(6);

	_currChickenSequenceId = 0x14B;
	gameSys.setAnimation(0x14B, 179, 2);
	gameSys.insertSequence(0x14B, 179, 0, 0, kSeqNone, 0, 0, 0);

	_nextChickenSequenceId = -1;
	_vm->_timers[5] = _vm->getRandom(20) + 30;
	_vm->_timers[4] = _vm->getRandom(100) + 300;

	_vm->queueInsertDeviceIcon();

	switch (_vm->_prevSceneNum) {
	case 3:
		gnap.initPos(11, 6, kDirBottomLeft);
		if (_vm->isFlag(kGFPlatypus))
			plat.initPos(12, 6, kDirIdleRight);
		_vm->endSceneInit();
		if (_vm->isFlag(kGFPlatypus))
			plat.walkTo(Common::Point(9, 6), -1, 0x107C2, 1);
		gnap.walkTo(Common::Point(8, 6), -1, 0x107BA, 1);
		break;
	case 4:
		gnap.initPos(_vm->_hotspotsWalkPos[6].x, _vm->_hotspotsWalkPos[6].y, kDirBottomLeft);
		if (_vm->isFlag(kGFPlatypus))
			plat.initPos(_vm->_hotspotsWalkPos[6].x + 1, _vm->_hotspotsWalkPos[6].y, kDirIdleRight);
		_vm->endSceneInit();
		gnap.walkTo(Common::Point(7, 6), 0, 0x107B9, 1);
		if (_vm->isFlag(kGFPlatypus))
			plat.walkTo(Common::Point(8, 6), 1, 0x107C2, 1);
		updateHotspots();
		gameSys.waitForUpdate();
		break;
	case 47:
		_vm->clearFlag(kGFUnk25);
		gnap.initPos(5, 6, kDirBottomLeft);
		plat.initPos(6, 7, kDirIdleRight);
		_vm->endSceneInit();
		break;
	case 49:
		gnap.initPos(5, 6, kDirBottomRight);
		if (_vm->isFlag(kGFPlatypus))
			plat.initPos(6, 7, kDirIdleLeft);
		_vm->endSceneInit();
		break;
	default:
		gnap.initPos(-1, 6, kDirBottomRight);
		if (_vm->isFlag(kGFPlatypus))
			plat.initPos(-1, 7, kDirIdleLeft);
		_vm->endSceneInit();
		if (_vm->isFlag(kGFPlatypus))
			plat.walkTo(Common::Point(2, 7), -1, 0x107C2, 1);
		gnap.walkTo(Common::Point(2, 8), -1, 0x107B9, 1);
		break;
	}

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->testWalk(0, 6, 7, 6, 8, 6);

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS02Device:
			if (gnap._actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS02Platypus:
			if (gnap._actionStatus < 0 && _vm->isFlag(kGFPlatypus)) {
				if (_vm->_grabCursorSpriteIndex == kItemDisguise) {
					gnap.useDisguiseOnPlatypus();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playImpossible();
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						if (_vm->isFlag(kGFKeysTaken))
							gnap.playMoan1(plat._pos);
						else
							gnap.playScratchingHead(plat._pos);
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

		case kHS02Chicken:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemTwig) {
					gnap._idleFacing = kDirUpRight;
					Common::Point destPos = _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot] + Common::Point(0, 1);
					gnap.walkTo(destPos, 0, gnap.getSequenceId(kGSIdle, Common::Point(0, 0)) | 0x10000, 1);
					gnap._actionStatus = kAS02UseTwigWithChicken;
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[1] + Common::Point(0, 1), 9, 8);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playMoan2(Common::Point(9, 8));
						break;
					case GRAB_CURSOR:
						gnap._idleFacing = kDirBottomRight;
						if (gnap.walkTo(_vm->_hotspotsWalkPos[1], 0, gnap.getSequenceId(kGSIdle, Common::Point(0, 0)) | 0x10000, 1))
							gnap._actionStatus = kAS02GrabChicken;
						else
							gnap._actionStatus = -1;
						break;
					case TALK_CURSOR:
						gnap._idleFacing = kDirBottomRight;
						gnap.walkTo(_vm->_hotspotsWalkPos[1], 0, gnap.getSequenceId(kGSBrainPulsating, Common::Point(0, 0)) | 0x10000, 1);
						gnap._actionStatus = kAS02TalkChicken;
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

		case kHS02Truck1:
		case kHS02Truck2:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemKeys) {
					if (gnap.walkTo(_vm->_hotspotsWalkPos[3], 0, gnap.getSequenceId(kGSIdle, Common::Point(2, 2)) | 0x10000, 1)) {
						_vm->setGrabCursorSprite(-1);
						_vm->invRemove(kItemKeys);
						if (_vm->isFlag(kGFTruckFilledWithGas))
							gnap._actionStatus = kAS02UseTruckGas;
						else
							gnap._actionStatus = kAS02UseTruckNoGas;
					}
				} else if (_vm->_grabCursorSpriteIndex == kItemGas) {
					_vm->_hotspots[kHS02WalkArea4]._flags |= SF_WALKABLE;
					if (gnap.walkTo(_vm->_hotspotsWalkPos[2], 0, gnap.getSequenceId(kGSIdle, Common::Point(2, 2)) | 0x10000, 1))
						gnap._actionStatus = kAS02UseGasWithTruck;
					_vm->_hotspots[kHS02WalkArea4]._flags &= ~SF_WALKABLE;
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[2], 2, 2);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playScratchingHead(Common::Point(2, 2));
						break;
					case GRAB_CURSOR:
						if (_vm->isFlag(kGFTruckKeysUsed)) {
							if (gnap.walkTo(_vm->_hotspotsWalkPos[3], 0, gnap.getSequenceId(kGSIdle, Common::Point(2, 2)) | 0x10000, 1)) {
								if (_vm->isFlag(kGFTruckFilledWithGas))
									gnap._actionStatus = kAS02UseTruckGas;
								else
									gnap._actionStatus = kAS02UseTruckNoGas;
							}
						} else {
							gnap._idleFacing = kDirIdleRight;
							if (gnap.walkTo(_vm->_hotspotsWalkPos[3], 0, gnap.getSequenceId(kGSIdle, Common::Point(2, 2)) | 0x10000, 1))
								gnap._actionStatus = kAS02UseTruckNoKeys;
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

		case kHS02TruckGrill:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[4], 2, 4);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playMoan2(Common::Point(2, 4));
						break;
					case GRAB_CURSOR:
						gnap._idleFacing = kDirUpRight;
						gnap.walkTo(_vm->_hotspotsWalkPos[4], 0, gnap.getSequenceId(kGSIdle, Common::Point(0, 0)) | 0x10000, 1);
						gnap._actionStatus = kAS02GrabTruckGrill;
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

		case kHS02ExitHouse:
			if (gnap._actionStatus < 0) {
				_vm->_isLeavingScene = true;
				gnap.walkTo(_vm->_hotspotsWalkPos[6], 0, 0x107AD, 1);
				gnap._actionStatus = kAS02LeaveScene;
				if (_vm->isFlag(kGFPlatypus))
					plat.walkTo(_vm->_hotspotsWalkPos[6] + Common::Point(1, 0), -1, 0x107C1, 1);
				updateHotspots();
				_vm->_newSceneNum = 4;
			}
			break;

		case kHS02ExitBarn:
			if (gnap._actionStatus < 0) {
				_vm->_isLeavingScene = true;
				gnap.walkTo(_vm->_hotspotsWalkPos[7], 0, 0x107AD, 1);
				gnap._actionStatus = kAS02LeaveScene;
				if (_vm->isFlag(kGFPlatypus))
					plat.walkTo(_vm->_hotspotsWalkPos[7] + Common::Point(1, 0), -1, 0x107C1, 1);
				updateHotspots();
				_vm->_newSceneNum = 5;
			}
			break;

		case kHS02ExitCreek:
			if (gnap._actionStatus < 0) {
				_vm->_isLeavingScene = true;
				gnap.walkTo(_vm->_hotspotsWalkPos[8], 0, 0x107AB, 1);
				gnap._actionStatus = kAS02LeaveScene;
				if (_vm->isFlag(kGFPlatypus))
					plat.walkTo(_vm->_hotspotsWalkPos[8], -1, 0x107CD, 1);
				_vm->_newSceneNum = 3;
			}
			break;

		case kHS02ExitPigpen:
			if (gnap._actionStatus < 0) {
				_vm->_isLeavingScene = true;
				gnap.walkTo(_vm->_hotspotsWalkPos[9], 0, 0x107AF, 1);
				gnap._actionStatus = kAS02LeaveScene;
				if (_vm->isFlag(kGFPlatypus))
					plat.walkTo(_vm->_hotspotsWalkPos[9], -1, 0x107CF, 1);
				_vm->_newSceneNum = 1;
			}
			break;

		case kHS02WalkArea1:
		case kHS02WalkArea2:
		case kHS02WalkArea3:
		case kHS02WalkArea4:
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
			if (plat._actionStatus < 0 && _vm->isFlag(kGFPlatypus))
				plat.updateIdleSequence();
			if (gnap._actionStatus < 0)
				gnap.updateIdleSequence();
			if (!_vm->_timers[4]) {
				// Update bird animation
				_vm->_timers[4] = _vm->getRandom(100) + 300;
				if (_vm->getRandom(2) != 0)
					gameSys.insertSequence(0x156, 256, 0, 0, kSeqNone, 0, 0, 0);
				else
					gameSys.insertSequence(0x154, 256, 0, 0, kSeqNone, 0, 0, 0);
			}
			if (!_vm->_timers[5] && _nextChickenSequenceId == -1 && gnap._actionStatus != 7 && gnap._actionStatus != 8) {
				if (_vm->getRandom(6) != 0) {
					_nextChickenSequenceId = 0x14B;
					_vm->_timers[5] = _vm->getRandom(20) + 30;
				} else {
					_nextChickenSequenceId = 0x14D;
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

void Scene02::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;

	if (gameSys.getAnimationStatus(0) == 2) {
		switch (gnap._actionStatus) {
		case kAS02UseTruckNoKeys:
			gameSys.insertSequence(0x14E, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(0x14E, gnap._id, 0);
			gnap._sequenceId = 0x14E;
			gnap._sequenceDatNum = 0;
			gnap._actionStatus = kAS02UseTruckNoKeysDone;
			break;
		case kAS02UseGasWithTruck:
			gameSys.insertSequence(0x151, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(0x151, gnap._id, 0);
			gnap._sequenceId = 0x151;
			gnap._sequenceDatNum = 0;
			_vm->invRemove(kItemGas);
			_vm->setGrabCursorSprite(-1);
			_vm->setFlag(kGFTruckFilledWithGas);
			gnap._actionStatus = kAS02UseGasWithTruckDone;
			break;
		case kAS02UseTruckGas:
			_vm->_timers[5] = 9999;
			_vm->_timers[4] = 9999;
			_vm->hideCursor();
			_vm->setGrabCursorSprite(-1);
			if (!_vm->isFlag(kGFTruckKeysUsed)) {
				gameSys.insertSequence(0x14F, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
				gameSys.waitForUpdate();
				_vm->setFlag(kGFTruckKeysUsed);
				gnap._sequenceId = 0x14F;
				gnap._sequenceDatNum = 0;
				_vm->invRemove(kItemKeys);
				_vm->setGrabCursorSprite(-1);
			}
			_vm->_newSceneNum = 47;
			_vm->_sceneDone = true;
			break;
		case kAS02UseTruckNoGas:
			_vm->hideCursor();
			_vm->setGrabCursorSprite(-1);
			_vm->_timers[4] = 250;
			if (!_vm->isFlag(kGFTruckKeysUsed)) {
				gameSys.insertSequence(0x14F, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
				gameSys.waitForUpdate();
				_vm->setFlag(kGFTruckKeysUsed);
				gnap._sequenceId = 0x14F;
				gnap._sequenceDatNum = 0;
				_vm->invRemove(kItemKeys);
				_vm->setGrabCursorSprite(-1);
			}
			_vm->_newSceneNum = 47;
			_vm->_sceneDone = true;
			_vm->setFlag(kGFUnk25);
			break;
		case kAS02GrabTruckGrill:
			switch (_truckGrillCtr) {
			case 0:
				_gnapTruckSequenceId = 0x158;
				break;
			case 1:
				_gnapTruckSequenceId = 0x159;
				break;
			case 2:
				_gnapTruckSequenceId = 0x157;
				break;
			default:
				break;
			}
			_truckGrillCtr = (_truckGrillCtr + 1) % 3;
			gameSys.insertSequence(_gnapTruckSequenceId, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(_gnapTruckSequenceId, gnap._id, 0);
			gnap._sequenceId = _gnapTruckSequenceId;
			gnap._sequenceDatNum = 0;
			gnap._actionStatus = -1;
			break;
		case kAS02LeaveScene:
			_vm->_sceneDone = true;
			break;
		case kAS02TalkChicken:
			_nextChickenSequenceId = 0x14C;
			break;
		case kAS02GrabChicken:
			_nextChickenSequenceId = 0x150;
			_vm->_timers[2] = 100;
			break;
		case kAS02GrabChickenDone:
			gameSys.insertSequence(0x107B5, gnap._id, 0x150, 179, kSeqSyncWait, 0, 75 * gnap._pos.x - gnap._gridX, 48 * gnap._pos.y - gnap._gridY);
			gnap._sequenceId = 0x7B5;
			gnap._sequenceDatNum = 1;
			_currChickenSequenceId = 0x14B;
			gameSys.setAnimation(0x14B, 179, 2);
			gameSys.insertSequence(_currChickenSequenceId, 179, 0, 0, kSeqNone, 0, 0, 0);
			gnap._actionStatus = -1;
			_vm->_timers[5] = 30;
			break;
		case kAS02UseTwigWithChicken:
			gnap.playShowItem(5, 0, 0);
			gameSys.insertSequence(0x155, 179, _currChickenSequenceId, 179, kSeqSyncExists, 0, 0, 0);
			_currChickenSequenceId = 0x155;
			_nextChickenSequenceId = -1;
			gnap._actionStatus = -1;
			break;
		case kAS02UseTruckNoKeysDone:
		case kAS02UseGasWithTruckDone:
		default:
			gnap._actionStatus = -1;
			break;
		}
	}

	if (gameSys.getAnimationStatus(2) == 2) {
		if (_nextChickenSequenceId == 0x150) {
			gameSys.setAnimation(_nextChickenSequenceId, 179, 0);
			gameSys.insertSequence(_nextChickenSequenceId, 179, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gameSys.removeSequence(_currChickenSequenceId, 179, true);
			_nextChickenSequenceId = -1;
			_currChickenSequenceId = -1;
			gnap._actionStatus = kAS02GrabChickenDone;
			_vm->_timers[5] = 500;
		} else if (_nextChickenSequenceId == 0x14C) {
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

Scene03::Scene03(GnapEngine *vm) : Scene(vm) {
	_nextPlatSequenceId = -1;
	_platypusScared = false;
	_platypusHypnotized = false;
	_nextFrogSequenceId = -1;
	_currFrogSequenceId = -1;
}

int Scene03::init() {
	GameSys& gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 1);
	gameSys.setAnimation(0, 0, 5);
	return 0x1CC;
}

void Scene03::updateHotspots() {
	_vm->setHotspot(kHS03Platypus, 0, 0, 0, 0, SF_DISABLED | SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS03Grass, 646, 408, 722, 458, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 9, 6);
	_vm->setHotspot(kHS03ExitTruck, 218, 64, 371, 224, SF_EXIT_U_CURSOR | SF_WALKABLE, 4, 4);
	_vm->setHotspot(kHS03Creek, 187, 499, 319, 587, SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 7);
	_vm->setHotspot(kHS03TrappedPlatypus, 450, 256, 661, 414, SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 2, 5);
	_vm->setHotspot(kHS03WalkAreas1, 0, 500, 300, 600);
	_vm->setHotspot(kHS03WalkAreas2, 300, 447, 800, 600);
	_vm->setHotspot(kHS03PlatypusWalkArea, 235, 0, 800, 600);
	_vm->setHotspot(kHS03WalkAreas3, 0, 0, 800, 354);
	_vm->setDeviceHotspot(kHS03Device, -1, -1, -1, -1);
	if (_vm->isFlag(kGFPlatypus))
		_vm->_hotspots[kHS03Platypus]._flags = SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	if (_vm->isFlag(kGFGrassTaken))
		_vm->_hotspots[kHS03Grass]._flags = SF_WALKABLE | SF_DISABLED;
	if (_vm->isFlag(kGFPlatypus))
		_vm->_hotspots[kHS03TrappedPlatypus]._flags = SF_DISABLED;
	if (_vm->isFlag(kGFPlatypus) || _platypusHypnotized)
		_vm->_hotspots[kHS03PlatypusWalkArea]._flags |= SF_WALKABLE;
	_vm->_hotspotsCount = 10;
}

void Scene03::run() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	_vm->playSound(0x10925, true);
	_vm->startSoundTimerC(7);

	gameSys.insertSequence(0x1CA, 251, 0, 0, kSeqLoop, 0, 0, 0);
	gameSys.insertSequence(0x1CB, 251, 0, 0, kSeqLoop, 0, 0, 0);

	_platypusHypnotized = false;
	gnap.initPos(3, 4, kDirBottomRight);

	gameSys.insertSequence(0x1C6, 253, 0, 0, kSeqNone, 0, 0, 0);

	_currFrogSequenceId = 0x1C6;
	_nextFrogSequenceId = -1;
	gameSys.setAnimation(0x1C6, 253, 2);

	_vm->_timers[6] = _vm->getRandom(20) + 30;
	_vm->_timers[4] = _vm->getRandom(100) + 300;
	_vm->_timers[5] = _vm->getRandom(100) + 200;

	if (_vm->isFlag(kGFPlatypus)) {
		plat.initPos(5, 4, kDirIdleLeft);
	} else {
		_vm->_timers[1] = _vm->getRandom(40) + 20;
		gameSys.setAnimation(0x1C2, 99, 1);
		gameSys.insertSequence(0x1C2, 99, 0, 0, kSeqNone, 0, 0, 0);
		plat._sequenceId = 0x1C2;
		plat._sequenceDatNum = 0;
	}

	gameSys.insertSequence(0x1C4, 255, 0, 0, kSeqNone, 0, 0, 0);

	if (!_vm->isFlag(kGFGrassTaken))
		gameSys.insertSequence(0x1B2, 253, 0, 0, kSeqNone, 0, 0, 0);

	_vm->queueInsertDeviceIcon();

	_vm->endSceneInit();

	if (_vm->isFlag(kGFPlatypus))
		plat.walkTo(Common::Point(4, 7), -1, 0x107C2, 1);
	gnap.walkTo(Common::Point(3, 6), -1, 0x107B9, 1);

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS03Platypus:
			if (gnap._actionStatus < 0 && _vm->isFlag(kGFPlatypus)) {
				if (_vm->_grabCursorSpriteIndex == kItemDisguise) {
					gnap.useDisguiseOnPlatypus();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playImpossible();
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						if (_vm->isFlag(kGFKeysTaken))
							gnap.playMoan1(plat._pos);
						else
							gnap.playScratchingHead(plat._pos);
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

		case kHS03Grass:
			if (gnap._actionStatus < 0) {
				if (_vm->isFlag(kGFGrassTaken)) {
					gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowItem(_vm->_grabCursorSpriteIndex, 9, 6);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playScratchingHead(Common::Point(9, 6));
						break;
					case GRAB_CURSOR:
						gnap.playPullOutDevice(Common::Point(9, 6));
						gnap.playUseDevice();
						gameSys.insertSequence(0x1B3, 253, 0x1B2, 253, kSeqSyncWait, 0, 0, 0);
						gameSys.setAnimation(0x1B3, 253, 5);
						_vm->_hotspots[kHS03Grass]._flags |= SF_WALKABLE | SF_DISABLED;
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

		case kHS03ExitTruck:
			if (gnap._actionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_hotspots[kHS03PlatypusWalkArea]._flags |= SF_WALKABLE;
				gnap.walkTo(_vm->_hotspotsWalkPos[2], 0, 0x107AD, 1);
				gnap._actionStatus = kAS03LeaveScene;
				if (_vm->isFlag(kGFPlatypus))
					plat.walkTo(_vm->_hotspotsWalkPos[2], -1, 0x107C2, 1);
				_vm->_hotspots[kHS03PlatypusWalkArea]._flags &= ~SF_WALKABLE;
				if (_vm->_cursorValue == 1)
					_vm->_newSceneNum = 2;
				else
					_vm->_newSceneNum = 33;
			}
			break;

		case kHS03Creek:
			if (gnap._actionStatus == -1) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playImpossible();
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playMoan2(Common::Point(2, 8));
						break;
					case GRAB_CURSOR:
						if (!_vm->isFlag(kGFPlatypus))
							_vm->_hotspots[kHS03PlatypusWalkArea]._flags |= SF_WALKABLE;
						if (gnap.walkTo(_vm->_hotspotsWalkPos[3], 0, gnap.getSequenceId(kGSIdle, _vm->_hotspotsWalkPos[3] + Common::Point(1, 1)) | 0x10000, 1))
							gnap._actionStatus = kAS03GrabCreek;
						if (!_vm->isFlag(kGFPlatypus))
							_vm->_hotspots[kHS03PlatypusWalkArea]._flags &= ~SF_WALKABLE;
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

		case kHS03TrappedPlatypus:
			if (gnap._actionStatus < 0) {
				if (_vm->isFlag(kGFPlatypus)) {
					gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowItem(_vm->_grabCursorSpriteIndex, 8, 4);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playScratchingHead(Common::Point(8, 4));
						break;
					case GRAB_CURSOR:
						if (_platypusHypnotized) {
							gnap.walkTo(Common::Point(7, 6), 0, 0x107B5, 1);
							gnap._actionStatus = kAS03FreePlatypus;
						} else {
							gnap.walkTo(_vm->_hotspotsWalkPos[4], 0, gnap.getSequenceId(kGSIdle, Common::Point(0, 0)) | 0x10000, 1);
							if (_platypusScared)
								gnap._actionStatus = kAS03GrabScaredPlatypus;
							else
								gnap._actionStatus = kAS03GrabPlatypus;
						}
						break;
					case TALK_CURSOR:
						if (_platypusHypnotized) {
							gnap.playBrainPulsating(Common::Point(8, 4));
						} else {
							gnap._idleFacing = kDirBottomRight;
							gnap.walkTo(_vm->_hotspotsWalkPos[4], 0, gnap.getSequenceId(kGSIdle, Common::Point(0, 0)) | 0x10000, 1);
							if (_platypusScared)
								gnap._actionStatus = kAS03HypnotizeScaredPlat;
							else
								gnap._actionStatus = kAS03HypnotizePlat;
						}
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

		case kHS03Device:
			if (gnap._actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS03WalkAreas1:
		case kHS03WalkAreas2:
		case kHS03WalkAreas3:
			if (gnap._actionStatus < 0)
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
			break;

		case kHS03PlatypusWalkArea:
			if (gnap._actionStatus < 0) {
				if (_vm->isFlag(kGFPlatypus) || _platypusHypnotized) {
					gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
				} else {
					gnap.walkTo(_vm->_hotspotsWalkPos[4], 0, 0x107B5, 1);
					if (_platypusScared)
						gnap._actionStatus = kAS03GrabScaredPlatypus;
					else
						gnap._actionStatus = kAS03GrabPlatypus;
				}
			}
			break;

		default:
			if (_vm->_mouseClickState._left && gnap._actionStatus < 0) {
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
				_vm->_mouseClickState._left = false;
			}
			break;

		}

		updateAnimations();

		if (!_vm->isSoundPlaying(0x10925))
			_vm->playSound(0x10925, true);

		if (!_vm->_isLeavingScene) {
			if (plat._actionStatus < 0 && _vm->isFlag(kGFPlatypus))
				plat.updateIdleSequence();
			if (gnap._actionStatus < 0)
				gnap.updateIdleSequence();
			if (!_vm->_timers[1] && !_platypusScared) {
				_vm->_timers[1] = _vm->getRandom(40) + 20;
				if (gnap._actionStatus < 0 && plat._actionStatus < 0 && !_vm->isFlag(kGFPlatypus) && !_platypusHypnotized)
					_nextPlatSequenceId = 450;
			}
			if (!_vm->_timers[6]) {
				_vm->_timers[6] = _vm->getRandom(20) + 30;
				if (gnap._actionStatus < 0 && plat._actionStatus < 0 && _nextFrogSequenceId == -1) {
					if (_vm->getRandom(5) == 1)
						_nextFrogSequenceId = 0x1C6;
					else
						_nextFrogSequenceId = 0x1C7;
				}
			}
			if (!_vm->_timers[4]) {
				// Update bird animation
				_vm->_timers[4] = _vm->getRandom(100) + 300;
				if (gnap._actionStatus < 0 && plat._actionStatus < 0)
					gameSys.insertSequence(_vm->getRandom(2) != 0 ? 0x1C8 : 0x1C3, 253, 0, 0, kSeqNone, 0, 0, 0);
			}
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(100) + 200;
				if (gnap._actionStatus < 0 && plat._actionStatus < 0) {
					gameSys.setAnimation(0x1C5, 253, 4);
					gameSys.insertSequence(0x1C5, 253, 0, 0, kSeqNone, 0, 0, 0);
				}
			}
			_vm->playSoundC();
		}

		_vm->checkGameKeys();

		if (_vm->isKeyStatus1(Common::KEYCODE_BACKSPACE)) {
			_vm->clearKeyStatus1(Common::KEYCODE_BACKSPACE);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[5] = _vm->getRandom(100) + 200;
			_vm->_timers[4] = _vm->getRandom(100) + 300;
			_vm->_timers[6] = _vm->getRandom(20) + 30;
		}

		_vm->gameUpdateTick();
	}
}

void Scene03::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (gnap._actionStatus) {
		case kAS03LeaveScene:
			_vm->_sceneDone = true;
			break;
		case kAS03FreePlatypus:
			_nextPlatSequenceId = 0x1BC;
			break;
		case kAS03FreePlatypusDone:
			gnap._actionStatus = -1;
			plat._pos = Common::Point(6, 6);
			plat._idleFacing = kDirIdleRight;
			plat._id = 120;
			gameSys.insertSequence(0x107CA, plat._id, 0x1BC, 99,
				kSeqSyncWait, 0, 75 * plat._pos.x - plat._gridX, 48 * plat._pos.y - plat._gridY);
			gameSys.insertSequence(0x1B7, 99, 0, 0, kSeqNone, 0, 0, 0);
			plat._sequenceDatNum = 1;
			plat._sequenceId = 0x7CA;
			_vm->setFlag(kGFPlatypus);
			_nextPlatSequenceId = -1;
			updateHotspots();
			break;
		case kAS03HypnotizePlat:
			gnap.playBrainPulsating();
			_vm->addFullScreenSprite(0x106, 255);
			gameSys.setAnimation(0x1C9, 256, 1);
			gameSys.insertSequence(0x1C9, 256, 0, 0, kSeqNone, 0, 0, 0);
			while (gameSys.getAnimationStatus(1) != 2 && !_vm->_gameDone)
				_vm->gameUpdateTick();
			_vm->removeFullScreenSprite();
			gameSys.setAnimation(0x1BA, 99, 1);
			gameSys.insertSequence(0x1BA, 99, plat._sequenceId | (plat._sequenceDatNum << 16), 99, kSeqSyncExists, 0, 0, 0);
			plat._sequenceDatNum = 0;
			plat._sequenceId = 0x1BA;
			gnap._actionStatus = -1;
			_platypusHypnotized = true;
			updateHotspots();
			break;
		case kAS03HypnotizeScaredPlat:
			gnap.playBrainPulsating();
			gameSys.insertSequence(0x1BF, 99, plat._sequenceId | (plat._sequenceDatNum << 16), 99, kSeqSyncExists, 0, 0, 0);
			gameSys.setAnimation(0x1BF, 99, 1);
			while (gameSys.getAnimationStatus(1) != 2 && !_vm->_gameDone)
				_vm->gameUpdateTick();
			_vm->addFullScreenSprite(0x106, 255);
			gameSys.setAnimation(0x1C9, 256, 1);
			gameSys.insertSequence(0x1C9, 256, 0, 0, kSeqNone, 0, 0, 0);
			while (gameSys.getAnimationStatus(1) != 2 && !_vm->_gameDone)
				_vm->gameUpdateTick();
			_vm->removeFullScreenSprite();
			gameSys.setAnimation(0x1BA, 99, 1);
			gameSys.insertSequence(0x1BA, 99, 447, 99, kSeqSyncWait, 0, 0, 0);
			plat._sequenceDatNum = 0;
			plat._sequenceId = 0x1BA;
			gnap._actionStatus = -1;
			_platypusHypnotized = true;
			updateHotspots();
			break;
		case kAS03GrabPlatypus:
			_nextPlatSequenceId = 0x1BD;
			_platypusHypnotized = false;
			break;
		case kAS03GrabScaredPlatypus:
			_nextPlatSequenceId = 0x1C0;
			_platypusHypnotized = false;
			break;
		case kAS03GrabCreek:
			gameSys.insertSequence(0x1B4, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(0x1B4, gnap._id, 0);
			gnap._sequenceId = 0x1B4;
			gnap._sequenceDatNum = 0;
			gnap._actionStatus = kAS03GrabCreekDone;
			break;
		default:
			gnap._actionStatus = -1;
			break;
		}
	}

	if (gameSys.getAnimationStatus(1) == 2) {
		if (_nextPlatSequenceId == 0x1BD || _nextPlatSequenceId == 0x1C0) {
			gameSys.setAnimation(0, 0, 1);
			_platypusScared = true;
			gameSys.insertSequence(0x1B5, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(_nextPlatSequenceId, 99, plat._sequenceId | (plat._sequenceDatNum << 16), 99, kSeqSyncWait, 0, 0, 0);
			gnap._sequenceId = 0x1B5;
			gnap._sequenceDatNum = 0;
			gnap._idleFacing = kDirIdleLeft;
			plat._sequenceId = _nextPlatSequenceId;
			plat._sequenceDatNum = 0;
			gameSys.setAnimation(_nextPlatSequenceId, 99, 1);
			_nextPlatSequenceId = -1;
			gnap._actionStatus = -1;
		} else if (_nextPlatSequenceId == 0x1BC) {
			gnap._pos = Common::Point(3, 6);
			gameSys.insertSequence(0x1B6, 120, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0x1BC, 99, plat._sequenceId | (plat._sequenceDatNum << 16), 99, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(0x1BC, 99, 0);
			gnap._id = 20 * gnap._pos.y;
			gnap._sequenceId = 0x1B6;
			gnap._sequenceDatNum = 0;
			gnap._idleFacing = kDirIdleLeft;
			gnap._actionStatus = kAS03FreePlatypusDone;
			_nextPlatSequenceId = -1;
		} else if (_nextPlatSequenceId == 0x1C2 && !_platypusScared) {
			gameSys.setAnimation(0, 0, 1);
			gameSys.insertSequence(0x1C2, 99, plat._sequenceId | (plat._sequenceDatNum << 16), 99, kSeqSyncWait, 0, 0, 0);
			plat._sequenceId = 0x1C2;
			plat._sequenceDatNum = 0;
			gameSys.setAnimation(0x1C2, 99, 1);
			_nextPlatSequenceId = -1;
		} else if (_nextPlatSequenceId == -1 && _platypusScared && !_platypusHypnotized) {
			gameSys.setAnimation(0, 0, 1);
			gameSys.setAnimation(0x1BE, 99, 1);
			gameSys.insertSequence(0x1BE, 99, plat._sequenceId | (plat._sequenceDatNum << 16), 99, kSeqSyncWait, 0, 0, 0);
			plat._sequenceId = 0x1BE;
			plat._sequenceDatNum = 0;
			_nextPlatSequenceId = -1;
		}
	}

	if (gameSys.getAnimationStatus(2) == 2 && _nextFrogSequenceId != -1) {
		gameSys.setAnimation(_nextFrogSequenceId, 253, 2);
		gameSys.insertSequence(_nextFrogSequenceId, 253, _currFrogSequenceId, 253, kSeqSyncWait, 0, 0, 0);
		_currFrogSequenceId = _nextFrogSequenceId;
		_nextFrogSequenceId = -1;
	}

	if (gameSys.getAnimationStatus(5) == 2) {
		gameSys.setAnimation(0, 0, 5);
		_vm->invAdd(kItemGrass);
		_vm->setGrabCursorSprite(kItemGrass);
		_vm->setFlag(kGFGrassTaken);
		updateHotspots();
	}
}

/*****************************************************************************/

Scene04::Scene04(GnapEngine *vm) : Scene(vm) {
	_dogIdCtr = 0;
	_triedWindow = false;
	_nextDogSequenceId = -1;
	_currDogSequenceId = -1;
}

int Scene04::init() {
	GameSys& gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 1);
	gameSys.setAnimation(0, 0, 2);
	return 0x214;
}

void Scene04::updateHotspots() {
	_vm->setHotspot(kHS04Platypus, 0, 0, 0, 0, SF_DISABLED | SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS04Twig, 690, 394, 769, 452, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 9, 6);
	_vm->setHotspot(kHS04Dog, 550, 442, 680, 552, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 8);
	_vm->setHotspot(kHS04Axe, 574, 342, 680, 412, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 7);
	_vm->setHotspot(kHS04Door, 300, 244, 386, 410, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 7);
	_vm->setHotspot(kHS04ExitTruck, 226, 580, 688, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
	_vm->setHotspot(kHS04Window, 121, 295, 237, 342, SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 7);
	_vm->setHotspot(kHS04ExitBarn, 585, 154, 800, 276, SF_EXIT_U_CURSOR, 10, 8);
	_vm->setHotspot(kHS04WalkArea1, 0, 0, 562, 461);
	_vm->setHotspot(kHS04WalkArea2, 562, 0, 800, 500);
	_vm->setDeviceHotspot(kHS04Device, -1, -1, -1, -1);
	if (_vm->isFlag(kGFPlatypus))
		_vm->_hotspots[kHS04Platypus]._flags = SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	if (_vm->isFlag(kGFTwigTaken))
		_vm->_hotspots[kHS04Twig]._flags = SF_WALKABLE | SF_DISABLED;
	if (_vm->isFlag(kGFPlatypusTalkingToAssistant) || _vm->_cursorValue == 1)
		_vm->_hotspots[kHS04Axe]._flags = SF_DISABLED;
	_vm->_hotspotsCount = 11;
}

void Scene04::run() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	_vm->playSound(0x1091C, true);
	_vm->startSoundTimerC(4);

	gameSys.insertSequence(0x210, 139 - _dogIdCtr, 0, 0, kSeqNone, 0, 0, 0);

	_currDogSequenceId = 0x210;
	_nextDogSequenceId = -1;

	gameSys.setAnimation(0x210, 139 - _dogIdCtr, 3);
	_dogIdCtr = (_dogIdCtr + 1) % 2;
	_vm->_timers[6] = _vm->getRandom(20) + 60;
	_vm->_timers[5] = _vm->getRandom(150) + 300;
	_vm->_timers[7] = _vm->getRandom(150) + 200;
	_vm->_timers[8] = _vm->getRandom(150) + 400;

	if (!_vm->isFlag(kGFPlatypusTalkingToAssistant) && _vm->_cursorValue == 4)
		gameSys.insertSequence(0x212, 100, 0, 0, kSeqNone, 0, 0, 0);

	if (!_vm->isFlag(kGFTwigTaken))
		gameSys.insertSequence(0x1FE, 100, 0, 0, kSeqNone, 0, 0, 0);

	_vm->queueInsertDeviceIcon();

	if (_vm->isFlag(kGFPlatypusDisguised)) {
		_vm->_timers[3] = 300;
		_vm->setGrabCursorSprite(kItemKeys);
		gnap._pos = Common::Point(4, 7);
		gnap._id = 140;
		plat._pos = Common::Point(6, 7);
		plat._id = 141;
		gameSys.insertSequence(0x107B5, 140, 0, 0, kSeqNone, 0, 300 - gnap._gridX, 336 - gnap._gridY);
		gameSys.insertSequence(0x20C, 141, 0, 0, kSeqNone, 0, 0, 0);
		gameSys.insertSequence(0x208, 121, 0, 0, kSeqNone, 0, 0, 0);
		gameSys.insertSequence(0x209, 121, 0x208, 121, kSeqSyncWait, 0, 0, 0);
		_vm->endSceneInit();
		_vm->invRemove(kItemDisguise);
		_vm->invAdd(kItemKeys);
		_vm->setFlag(kGFKeysTaken);
		_vm->clearFlag(kGFPlatypusDisguised);
		plat._sequenceId = 0x20C;
		plat._sequenceDatNum = 0;
		plat._idleFacing = kDirBottomRight;
		gnap._sequenceId = 0x7B5;
		gnap._sequenceDatNum = 1;
		gameSys.waitForUpdate();
	} else {
		gameSys.insertSequence(0x209, 121, 0, 0, kSeqNone, 0, 0, 0);
		if (_vm->_prevSceneNum == 2) {
			gnap.initPos(5, 11, kDirUpRight);
			if (_vm->isFlag(kGFPlatypus))
				plat.initPos(6, 11, kDirUpLeft);
			_vm->endSceneInit();
			if (_vm->isFlag(kGFPlatypus))
				plat.walkTo(Common::Point(5, 8), -1, 0x107C2, 1);
			gnap.walkTo(Common::Point(6, 9), -1, 0x107BA, 1);
		} else if (_vm->_prevSceneNum == 38) {
			gnap.initPos(5, 7, kDirBottomRight);
			plat.initPos(4, 7, kDirIdleLeft);
			_vm->endSceneInit();
		} else {
			gnap.initPos(12, 9, kDirBottomRight);
			if (_vm->isFlag(kGFPlatypus))
				plat.initPos(12, 8, kDirIdleLeft);
			_vm->endSceneInit();
			if (_vm->isFlag(kGFPlatypus))
				plat.walkTo(Common::Point(9, 8), -1, 0x107C2, 1);
		gnap.walkTo(Common::Point(9, 9), -1, 0x107BA, 1);
		}
	}

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->testWalk(0, 4, -1, -1, -1, -1);

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS04Device:
			if (gnap._actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS04Platypus:
			if (gnap._actionStatus < 0 && _vm->isFlag(kGFPlatypus)) {
				if (_vm->_grabCursorSpriteIndex == kItemDisguise) {
					gnap.useDisguiseOnPlatypus();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playImpossible();
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						if (_vm->isFlag(kGFKeysTaken))
							gnap.playMoan1(plat._pos);
						else
							gnap.playScratchingHead(plat._pos);
						break;
					case GRAB_CURSOR:
						if (_vm->_cursorValue == 4)
							gnap.kissPlatypus(0);
						else
							gnap.playMoan1(plat._pos);
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

		case kHS04Twig:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowItem(_vm->_grabCursorSpriteIndex, 9, 6);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playScratchingHead(_vm->_hotspotsWalkPos[1]);
						break;
					case GRAB_CURSOR:
						gnap.playPullOutDevice(_vm->_hotspotsWalkPos[1]);
						gnap.playUseDevice(_vm->_hotspotsWalkPos[1]);
						gameSys.insertSequence(0x1FD, 100, 510, 100, kSeqSyncWait, 0, 0, 0);
						gameSys.setAnimation(0x1FD, 100, 2);
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

		case kHS04Axe:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[3], 9, 5);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playMoan2(_vm->_hotspotsWalkPos[3]);
						break;
					case GRAB_CURSOR:
						gnap._idleFacing = kDirUpRight;
						gnap.walkTo(_vm->_hotspotsWalkPos[3], 0, gnap.getSequenceId(kGSIdle, Common::Point(0, 0)) | 0x10000, 1);
						gnap._actionStatus = kAS04GrabAxe;
						_vm->setFlag(kGFPlatypusTalkingToAssistant);
						updateHotspots();
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

		case kHS04Dog:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[2], 9, 7);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						if (gnap.walkTo(gnap._pos, 0, -1, 1)) {
							gnap.playMoan2(_vm->_hotspotsWalkPos[2]);
							_nextDogSequenceId = 0x20F;
						}
						break;
					case GRAB_CURSOR:
						gnap._idleFacing = kDirBottomRight;
						if (gnap.walkTo(_vm->_hotspotsWalkPos[2], 0, gnap.getSequenceId(kGSIdle, Common::Point(0, 0)) | 0x10000, 1))
							gnap._actionStatus = kAS04GrabDog;
						break;
					case TALK_CURSOR:
						gnap._idleFacing = kDirBottomRight;
						if (gnap.walkTo(gnap._pos, 0, -1, 1)) {
							gnap.playBrainPulsating(_vm->_hotspotsWalkPos[2]);
							_nextDogSequenceId = 0x20E;
						}
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

		case kHS04Door:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot], 4, 3);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
					case PLAT_CURSOR:
						gnap.playScratchingHead(Common::Point(4, 3));
						break;
					case GRAB_CURSOR:
						if (_vm->_cursorValue == 1) {
							gnap.walkTo(_vm->_hotspotsWalkPos[4], 0, 0x107BC, 1);
							gnap._actionStatus = kAS04OpenDoor;
							_vm->_timers[5] = 300;
							gnap._idleFacing = kDirUpLeft;
						} else {
							_vm->_isLeavingScene = true;
							gnap.walkTo(_vm->_hotspotsWalkPos[4], 0, 0x107BC, 1);
							gnap._actionStatus = kAS04LeaveScene;
							_vm->_newSceneNum = 38;
						}
						break;
					case TALK_CURSOR:
						gnap.playImpossible();
						break;
					default:
						break;
					}
				}
			}
			break;

		case kHS04ExitTruck:
			if (gnap._actionStatus < 0) {
				_vm->_isLeavingScene = true;
				gnap.walkTo(_vm->_hotspotsWalkPos[5], 0, 0x107AE, 1);
				gnap._actionStatus = kAS04LeaveScene;
				if (_vm->isFlag(kGFPlatypus))
					plat.walkTo(_vm->_hotspotsWalkPos[5], -1, 0x107C7, 1);
				if (_vm->_cursorValue == 1)
					_vm->_newSceneNum = 2;
				else
					_vm->_newSceneNum = 33;
			}
			break;

		case kHS04Window:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot], 2, 3);
				} else if (_vm->isFlag(kGFKeysTaken)) {
					gnap.playImpossible();
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						if (gnap.walkTo(_vm->_hotspotsWalkPos[7], 0, gnap.getSequenceId(kGSIdle, Common::Point(10, 2)) | 0x10000, 1)) {
							if (_triedWindow) {
								gnap._actionStatus = kAS04GetKeyAnother;
							} else {
								gnap._actionStatus = kAS04GetKeyFirst;
								_triedWindow = true;
							}
						}
						break;
					case GRAB_CURSOR:
						gnap.playScratchingHead(_vm->_hotspotsWalkPos[7]);
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

		case kHS04ExitBarn:
			if (gnap._actionStatus < 0) {
				_vm->_isLeavingScene = true;
				gnap.walkTo(_vm->_hotspotsWalkPos[8], 0, 0x107AB, 1);
				gnap._actionStatus = kAS04LeaveScene;
				if (_vm->isFlag(kGFPlatypus))
					plat.walkTo(_vm->_hotspotsWalkPos[8] + Common::Point(0, 1), -1, 0x107C1, 1);
				if (_vm->_cursorValue == 1)
					_vm->_newSceneNum = 5;
				else
					_vm->_newSceneNum = 35;
			}
			break;

		case kHS04WalkArea1:
		case kHS04WalkArea2:
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
			if (plat._actionStatus < 0 && _vm->isFlag(kGFPlatypus))
				plat.updateIdleSequence2();
			if (gnap._actionStatus < 0)
				gnap.updateIdleSequence2();
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(150) + 300;
				if (gnap._actionStatus < 0)
					gameSys.insertSequence(0x20D, 79, 0, 0, kSeqNone, 0, 0, 0);
			}
			if (!_vm->_timers[7]) {
				_vm->_timers[7] = _vm->getRandom(150) + 200;
				gameSys.insertSequence(0x1FC, 59, 0, 0, kSeqNone, 0, 0, 0);
			}
			if (!_vm->_timers[6]) {
				_vm->_timers[6] = _vm->getRandom(20) + 60;
				if (_nextDogSequenceId == -1)
					_nextDogSequenceId = 0x210;
			}
			if (!_vm->_timers[8]) {
				_vm->_timers[8] = _vm->getRandom(150) + 400;
				gameSys.insertSequence(0x213, 20, 0, 0, kSeqNone, 0, 0, 0);
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

void Scene04::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (gnap._actionStatus) {
		case kAS04LeaveScene:
			_vm->_sceneDone = true;
			break;
		case kAS04OpenDoor:
			gameSys.insertSequence(0x205, gnap._id, 0, 0, kSeqNone, 0, 0, 0);
			gameSys.insertSequence(0x207, 121, 521, 121, kSeqSyncWait, 0, 0, 0);
			gnap._pos = Common::Point(6, 7);
			gameSys.insertSequence(0x107B5, gnap._id,
				makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id,
				kSeqSyncWait, _vm->getSequenceTotalDuration(0x205) - 1, 450 - gnap._gridX, 336 - gnap._gridY);
			gameSys.setAnimation(0x107B5, gnap._id, 0);
			gnap._sequenceId = 0x7B5;
			gnap._sequenceDatNum = 1;
			gnap._actionStatus = kAS04OpenDoorDone;
			break;
		case kAS04OpenDoorDone:
			gameSys.insertSequence(0x209, 121, 0x207, 121, kSeqSyncWait, 0, 0, 0);
			gnap._actionStatus = -1;
			break;
		case kAS04GetKeyFirst:
			gameSys.insertSequence(0x204, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(0x204, gnap._id, 0);
			gnap._sequenceId = 0x204;
			gnap._sequenceDatNum = 0;
			gnap._actionStatus = kAS04GetKeyFirst2;
			break;
		case kAS04GetKeyFirst2:
			gameSys.insertSequence(0x206, 255, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0x1FF, 256, 0, 0, kSeqNone, 0, 0, 0);
			gameSys.insertSequence(0x20B, 256, 0, 0, kSeqNone, 0, 0, 0);
			gameSys.setAnimation(0x20B, 256, 0);
			gnap._sequenceId = 0x206;
			gnap._sequenceDatNum = 0;
			gnap._actionStatus = kAS04GetKeyFirstDone;
			break;
		case kAS04GetKeyFirstDone:
			gameSys.requestRemoveSequence(0x1FF, 256);
			gameSys.requestRemoveSequence(0x20B, 256);
			gameSys.insertSequence(0x107B5, gnap._id,
				makeRid(gnap._sequenceDatNum, gnap._sequenceId), 255,
				kSeqSyncWait, 0, 75 * gnap._pos.x - gnap._gridX, 48 * gnap._pos.y - gnap._gridY);
			gnap._idleFacing = kDirBottomRight;
			gnap._sequenceId = 0x7B5;
			gnap._sequenceDatNum = 1;
			gnap._actionStatus = -1;
			break;
		case kAS04GetKeyAnother:
			gameSys.insertSequence(0x202, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(0x202, gnap._id, 0);
			gnap._sequenceId = 0x202;
			gnap._sequenceDatNum = 0;
			gnap._actionStatus = kAS04GetKeyAnother2;
			break;
		case kAS04GetKeyAnother2:
			gameSys.insertSequence(0x203, 255, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0x1FF, 256, 0, 0, kSeqNone, 0, 0, 0);
			gameSys.insertSequence(0x20A, 256, 0, 0, kSeqNone, 0, 0, 0);
			gameSys.setAnimation(0x20A, 256, 0);
			gnap._sequenceId = 0x203;
			gnap._sequenceDatNum = 0;
			gnap._actionStatus = kAS04GetKeyAnotherDone;
			break;
		case kAS04GetKeyAnotherDone:
			gameSys.removeSequence(0x1FF, 256, true);
			gameSys.removeSequence(0x20A, 256, true);
			gameSys.insertSequence(0x107B5, gnap._id,
				makeRid(gnap._sequenceDatNum, gnap._sequenceId), 255,
				kSeqSyncWait, 0, 75 * gnap._pos.x - gnap._gridX, 48 * gnap._pos.y - gnap._gridY);
			gnap._sequenceId = 0x7B5;
			gnap._sequenceDatNum = 1;
			gnap._idleFacing = kDirBottomRight;
			gnap._actionStatus = -1;
			break;
		case kAS04GrabDog:
			_nextDogSequenceId = 0x201;
			break;
		case kAS04GrabAxe:
			gameSys.insertSequence(0x211, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gameSys.requestRemoveSequence(0x212, 100);
			gnap._sequenceDatNum = 0;
			gnap._sequenceId = 0x211;
			gnap._actionStatus = -1;
			break;
		default:
			gnap._actionStatus = -1;
			break;
		}
	}

	if (gameSys.getAnimationStatus(2) == 2) {
		gameSys.setAnimation(0, 0, 2);
		_vm->invAdd(kItemTwig);
		_vm->setGrabCursorSprite(kItemTwig);
		_vm->setFlag(kGFTwigTaken);
		updateHotspots();
	}

	if (gameSys.getAnimationStatus(3) == 2) {
		if (_nextDogSequenceId == 0x201) {
			gameSys.insertSequence(_nextDogSequenceId, 139 - _dogIdCtr,
				_currDogSequenceId, 139 - (_dogIdCtr + 1) % 2,
				kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0x200, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(_nextDogSequenceId, 139 - _dogIdCtr, 3);
			_dogIdCtr = (_dogIdCtr + 1) % 2;
			_currDogSequenceId = 0x201;
			gnap._sequenceId = 0x200;
			gnap._sequenceDatNum = 0;
			gnap._actionStatus = -1;
			_vm->_timers[6] = _vm->getRandom(20) + 60;
			_nextDogSequenceId = -1;
		} else if (_nextDogSequenceId != -1) {
			gameSys.insertSequence(_nextDogSequenceId, 139 - _dogIdCtr,
				_currDogSequenceId, 139 - (_dogIdCtr + 1) % 2,
				kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(_nextDogSequenceId, 139 - _dogIdCtr, 3);
			_dogIdCtr = (_dogIdCtr + 1) % 2;
			_currDogSequenceId = _nextDogSequenceId;
			_nextDogSequenceId = -1;
		}
	}
}

/*****************************************************************************/

Scene05::Scene05(GnapEngine *vm) : Scene(vm) {
	_nextChickenSequenceId = -1;
	_currChickenSequenceId = -1;
}

int Scene05::init() {
	GameSys& gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 1);
	gameSys.setAnimation(0, 0, 3);
	return _vm->isFlag(kGFBarnPadlockOpen) ? 0x151 : 0x150;
}

void Scene05::updateHotspots() {
	_vm->setHotspot(kHS05Platypus, 0, 0, 0, 0, SF_DISABLED | SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS05Haystack, 236, 366, 372, 442, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 5, 7);
	_vm->setHotspot(kHS05Padlock, 386, 230, 626, 481, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 5, 7);
	_vm->setHotspot(kHS05Ladder, 108, 222, 207, 444, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 4, 7);
	_vm->setHotspot(kHS05ExitHouse, 0, 395, 20, 600, SF_EXIT_L_CURSOR | SF_WALKABLE, 0, 8);
	_vm->setHotspot(kHS05Chicken, 612, 462, 722, 564, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 7, 8);
	_vm->setHotspot(kHS05WalkArea1, 104, 0, 421, 480);
	_vm->setHotspot(kHS05WalkArea2, 422, 0, 800, 487);
	_vm->setHotspot(kHS05WalkArea3, 0, 0, 104, 499);
	_vm->setDeviceHotspot(kHS05Device, -1, -1, -1, -1);
	if (_vm->isFlag(kGFPlatypus))
		_vm->_hotspots[kHS05Platypus]._flags = SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	if (_vm->isFlag(kGFBarnPadlockOpen))
		_vm->_hotspots[kHS05Padlock]._flags = SF_EXIT_U_CURSOR;
	_vm->_hotspotsCount = 10;
}

void Scene05::run() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	_vm->playSound(0x1091C, true);
	_vm->startSoundTimerC(7);

	_currChickenSequenceId = 0x142;
	gameSys.setAnimation(0x142, 100, 3);
	gameSys.insertSequence(0x142, 100, 0, 0, kSeqNone, 0, 0, 0);

	_nextChickenSequenceId = -1;

	_vm->_timers[5] = _vm->getRandom(10) + 30;
	_vm->_timers[6] = _vm->getRandom(150) + 300;

	if (_vm->isFlag(kGFBarnPadlockOpen))
		gameSys.insertSequence(0x14A, 141, 0, 0, kSeqNone, 0, 0, 0);

	_vm->queueInsertDeviceIcon();

	if (_vm->_prevSceneNum != 6 && _vm->_prevSceneNum != 36) {
		gnap.initPos(-1, 8, kDirBottomRight);
		if (_vm->isFlag(kGFPlatypus))
			plat.initPos(-1, 9, kDirIdleLeft);
		_vm->endSceneInit();
		if (_vm->isFlag(kGFPlatypus))
			plat.walkTo(Common::Point(2, 8), -1, 0x107C2, 1);
		gnap.walkTo(Common::Point(2, 9), -1, 0x107B9, 1);
	} else {
		gnap.initPos(6, 8, kDirBottomRight);
		if (_vm->isFlag(kGFPlatypus))
			plat.initPos(7, 9, kDirIdleLeft);
		_vm->endSceneInit();
	}

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->testWalk(0, 12, -1, -1, -1, -1);

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS05Device:
			if (gnap._actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS05Platypus:
			if (gnap._actionStatus < 0 && _vm->isFlag(kGFPlatypus)) {
				if (_vm->_grabCursorSpriteIndex == kItemDisguise) {
					gnap.useDisguiseOnPlatypus();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playImpossible();
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						if (_vm->isFlag(kGFKeysTaken))
							gnap.playMoan1(plat._pos);
						else
							gnap.playScratchingHead(plat._pos);
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

		case kHS05Haystack:
			if (gnap._actionStatus < 0 && plat._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[1] + Common::Point(-2, 0), 4, 5);
				} else if (_vm->isFlag(kGFNeedleTaken)) {
					gnap.playImpossible();
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playScratchingHead(_vm->_hotspotsWalkPos[1] + Common::Point(0, -1));
						break;
					case GRAB_CURSOR:
					case TALK_CURSOR:
						gnap.playImpossible();
						break;
					case PLAT_CURSOR:
						if (_vm->isFlag(kGFPlatypus)) {
							gnap.useDeviceOnPlatypus();
							if (plat.walkTo(_vm->_hotspotsWalkPos[1], 1, 0x107C2, 1)) {
								plat._actionStatus = kAS05PlatSearchHaystack;
								plat._idleFacing = kDirIdleRight;
							}
							if (gnap._pos.x == 4 && (gnap._pos.y == 8 || gnap._pos.y == 7))
								gnap.walkStep();
							gnap.playIdle(plat._pos);
						}
						break;
					default:
						break;
					}
				}
			}
			break;

		case kHS05Chicken:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemTwig) {
					gnap._idleFacing = kDirUpRight;
					Common::Point checkPt = _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot] + Common::Point(0, 1);
					gnap.walkTo(checkPt, 0, gnap.getSequenceId(kGSIdle, Common::Point(0, 0)) | 0x10000, 1);
					gnap._actionStatus = kAS05UseTwigWithChicken;
				} else if (_vm->_grabCursorSpriteIndex >= 0)
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot] + Common::Point(0, 1), 9, 7);
				else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playMoan2(Common::Point(9, 7));
						break;
					case GRAB_CURSOR:
						gnap._idleFacing = kDirBottomRight;
						gnap.walkTo(_vm->_hotspotsWalkPos[5], 0, gnap.getSequenceId(kGSIdle, Common::Point(0, 0)) | 0x10000, 1);
						gnap._actionStatus = kAS05GrabChicken;
						break;
					case TALK_CURSOR:
						gnap._idleFacing = kDirBottomRight;
						gnap.walkTo(_vm->_hotspotsWalkPos[5], 0, gnap.getSequenceId(kGSBrainPulsating, Common::Point(0, 0)) | 0x10000, 1);
						gnap._actionStatus = kAS05TalkChicken;
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

		case kHS05Ladder:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot], 2, 5);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playMoan2(Common::Point(2, 4));
						break;
					case GRAB_CURSOR:
						gnap._idleFacing = kDirBottomLeft;
						gnap.walkTo(_vm->_hotspotsWalkPos[3], 0, gnap.getSequenceId(kGSIdle, Common::Point(0, 0)) | 0x10000, 1);
						gnap._actionStatus = kAS05GrabLadder;
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

		case kHS05Padlock:
			if (_vm->isFlag(kGFBarnPadlockOpen)) {
				_vm->_isLeavingScene = true;
				Common::Point destPt = _vm->_hotspotsWalkPos[2] + Common::Point(- 1, 1);
				gnap.walkTo(destPt, 0, -1, 1);
				gnap._actionStatus = kAS05EnterBarn;
				if (_vm->_cursorValue == 1)
					_vm->_newSceneNum = 6;
				else
					_vm->_newSceneNum = 36;
			} else if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemNeedle) {
					if (gnap.walkTo(_vm->_hotspotsWalkPos[2], 0,
						gnap.getSequenceId(kGSIdle, _vm->_hotspotsWalkPos[2]) | 0x10000, 1))
						gnap._actionStatus = kAS05PickPadlock;
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[2], 7, 4);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playScratchingHead(Common::Point(7, 4));
						break;
					case GRAB_CURSOR:
						gnap._idleFacing = kDirUpRight;
						gnap.walkTo(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot], 0, gnap.getSequenceId(kGSIdle, Common::Point(0, 0)) | 0x10000, 1);
						gnap._actionStatus = kAS05TryPickPadlock;
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

		case kHS05ExitHouse:
			if (gnap._actionStatus < 0) {
				_vm->_isLeavingScene = true;
				gnap.walkTo(_vm->_hotspotsWalkPos[4], 0, 0x107AF, 1);
				gnap._actionStatus = kAS05LeaveScene;
				if (_vm->isFlag(kGFPlatypus))
					plat.walkTo(_vm->_hotspotsWalkPos[4] + Common::Point(0, 1), -1, 0x107C7, 1);
				if (_vm->_cursorValue == 1)
					_vm->_newSceneNum = 4;
				else
					_vm->_newSceneNum = 37;
			}
			break;

		case kHS05WalkArea1:
		case kHS05WalkArea2:
			if (gnap._actionStatus < 0)
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
			break;

		case kHS05WalkArea3:
			// Nothing
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
			if (_vm->isFlag(kGFPlatypus))
				plat.updateIdleSequence();
			gnap.updateIdleSequence();
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(20) + 30;
				if (gnap._actionStatus != kAS05TalkChicken && _nextChickenSequenceId == -1) {
					if (_vm->getRandom(4) != 0)
						_nextChickenSequenceId = 0x142;
					else
						_nextChickenSequenceId = 0x143;
				}
			}
			if (!_vm->_timers[6]) {
				_vm->_timers[6] = _vm->getRandom(150) + 300;
				if (gnap._actionStatus < 0)
					gameSys.insertSequence(0x149, 39, 0, 0, kSeqNone, 0, 0, 0);
			}
			_vm->playSoundC();
		}

		_vm->checkGameKeys();

		if (_vm->isKeyStatus1(Common::KEYCODE_BACKSPACE)) {
			_vm->clearKeyStatus1(Common::KEYCODE_BACKSPACE);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[5] = _vm->getRandom(20) + 30;
		}

		_vm->gameUpdateTick();
	}
}

void Scene05::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (gnap._actionStatus) {
		case kAS05LeaveScene:
			_vm->_sceneDone = true;
			gnap._actionStatus = -1;
			break;
		case kAS05TryPickPadlock:
			gameSys.insertSequence(0x148, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gnap._sequenceId = 0x148;
			gnap._sequenceDatNum = 0;
			gnap._actionStatus = -1;
			break;
		case kAS05PickPadlock:
			gameSys.setAnimation(0x147, gnap._id, 0);
			gameSys.insertSequence(0x147, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gnap._sequenceId = 0x147;
			gnap._sequenceDatNum = 0;
			_vm->setFlag(kGFBarnPadlockOpen);
			_vm->setFlag(kGFSceneFlag1);
			_vm->setGrabCursorSprite(-1);
			_vm->_newSceneNum = 6;
			_vm->_timers[2] = 100;
			_vm->invRemove(kItemNeedle);
			gnap._actionStatus = kAS05LeaveScene;
			break;
		case kAS05TalkChicken:
			_nextChickenSequenceId = 0x144;
			gnap._actionStatus = -1;
			break;
		case kAS05GrabChicken:
			_nextChickenSequenceId = 0x14B;
			break;
		case kAS05GrabLadder:
			while (gameSys.isSequenceActive(0x149, 39) && !_vm->_gameDone)
				_vm->gameUpdateTick();
			gameSys.insertSequence(0x14E, gnap._id + 1, 0, 0, kSeqNone, 0, 0, 0);
			gameSys.insertSequence(0x14D, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gnap._sequenceId = 0x14D;
			gnap._sequenceDatNum = 0;
			_vm->_timers[2] = 200;
			_vm->_timers[6] = 300;
			gnap._actionStatus = -1;
			break;
		case kAS05EnterBarn:
			gameSys.insertSequence(0x107B1, 1,
				makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id,
				kSeqSyncWait, 0, 75 * gnap._pos.x - gnap._gridX, 48 * gnap._pos.y - gnap._gridY);
			gameSys.setAnimation(0x107B1, 1, 0);
			gnap._actionStatus = kAS05LeaveScene;
			break;
		case kAS05UseTwigWithChicken:
			gnap.playShowItem(5, 0, 0);
			_nextChickenSequenceId = 0x14F;
			gnap._actionStatus = -1;
			break;
		default:
			break;
		}
	}

	if (gameSys.getAnimationStatus(1) == 2) {
		PlayerPlat& plat = *_vm->_plat;
		if (plat._sequenceId == 0x146) {
			plat._pos = Common::Point(4, 8);
			gameSys.insertSequence(0x107C1, 160, 0x146, 256, kSeqSyncWait, 0, 300 - plat._gridX, 384 - plat._gridY);
			plat._sequenceId = 0x7C1;
			plat._sequenceDatNum = 1;
			plat._id = 20 * plat._pos.y;
			_vm->invAdd(kItemNeedle);
			_vm->setFlag(kGFNeedleTaken);
			_vm->setGrabCursorSprite(kItemNeedle);
			_vm->showCursor();
			_vm->_timers[1] = 30;
			plat._actionStatus = -1;
		}
		if (plat._actionStatus == kAS05PlatSearchHaystack) {
			gameSys.setAnimation(0, 0, 1);
			gameSys.insertSequence(0x145, plat._id, plat._sequenceId | (plat._sequenceDatNum << 16), plat._id, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0x146, 256, 0x145, plat._id, kSeqSyncWait, 0, 0, 0);
			_vm->hideCursor();
			_vm->setGrabCursorSprite(-1);
			plat._sequenceId = 0x146;
			plat._sequenceDatNum = 0;
			gameSys.setAnimation(0x146, 256, 1);
			_vm->_timers[1] = 300;
		}
	}

	if (gameSys.getAnimationStatus(3) == 2) {
		if (_nextChickenSequenceId == 0x14B) {
			gameSys.setAnimation(_nextChickenSequenceId, 100, 3);
			gameSys.insertSequence(_nextChickenSequenceId, 100, _currChickenSequenceId, 100, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0x14C, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gnap._sequenceDatNum = 0;
			gnap._sequenceId = 0x14C;
			_currChickenSequenceId = _nextChickenSequenceId;
			_nextChickenSequenceId = -1;
			gnap._actionStatus = -1;
		} else if (_nextChickenSequenceId != -1) {
			gameSys.setAnimation(_nextChickenSequenceId, 100, 3);
			gameSys.insertSequence(_nextChickenSequenceId, 100, _currChickenSequenceId, 100, kSeqSyncWait, 0, 0, 0);
			_currChickenSequenceId = _nextChickenSequenceId;
			_nextChickenSequenceId = -1;
		}
	}
}

/*****************************************************************************/

Scene06::Scene06(GnapEngine *vm) : Scene(vm) {
	_horseTurnedBack = false;
	_nextPlatSequenceId = -1;
	_nextHorseSequenceId = -1;
	_currHorseSequenceId = -1;
}

int Scene06::init() {
	GameSys& gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 1);
	gameSys.setAnimation(0, 0, 2);
	if (_vm->isFlag(kGFSceneFlag1)) {
		_vm->playSound(0x11B, false);
		_vm->clearFlag(kGFSceneFlag1);
	}
	return 0x101;
}

void Scene06::updateHotspots() {
	_vm->setHotspot(kHS06Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS06Gas, 300, 120, 440, 232, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 4, 7);
	_vm->setHotspot(kHS06Ladder, 497, 222, 614, 492, SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 8);
	_vm->setHotspot(kHS06Horse, 90, 226, 259, 376, SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 4, 7);
	_vm->setHotspot(kHS06ExitOutsideBarn, 226, 580, 688, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 10);
	_vm->setHotspot(kHS06WalkArea1, 0, 0, 200, 515);
	_vm->setHotspot(kHS06WalkArea2, 200, 0, 285, 499);
	_vm->setHotspot(kHS06WalkArea3, 688, 0, 800, 499);
	_vm->setHotspot(kHS06WalkArea4, 475, 469, 800, 505);
	_vm->setHotspot(kHS06WalkArea5, 0, 0, 800, 504);
	_vm->setDeviceHotspot(kHS06Device, -1, -1, -1, -1);
	if (_vm->isFlag(kGFGasTaken))
		_vm->_hotspots[kHS06Ladder]._flags = SF_DISABLED;
	if (_vm->_cursorValue == 4) {
		_vm->_hotspots[kHS06Ladder]._flags = SF_DISABLED;
		_vm->_hotspots[kHS06Gas]._flags = SF_DISABLED;
	}
	_vm->_hotspotsCount = 11;
}

void Scene06::run() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	bool triedDeviceOnGas = false;

	_vm->startSoundTimerC(7);

	_horseTurnedBack = false;
	gameSys.insertSequence(0xF1, 120, 0, 0, kSeqNone, 0, 0, 0);

	_currHorseSequenceId = 0xF1;
	_nextHorseSequenceId = -1;

	gameSys.setAnimation(0xF1, 120, 2);
	_vm->_timers[4] = _vm->getRandom(40) + 25;

	if (_vm->isFlag(kGFUnk04))
		gameSys.insertSequence(0xF7, 20, 0, 0, kSeqNone, 0, 0, 0);
	else
		gameSys.insertSequence(0xF8, 20, 0, 0, kSeqNone, 0, 0, 0);

	if (!_vm->isFlag(kGFGasTaken) && _vm->_cursorValue != 4)
		gameSys.insertSequence(0xFE, 20, 0, 0, kSeqNone, 0, 0, 0);

	_vm->queueInsertDeviceIcon();

	gnap.initPos(5, 12, kDirBottomRight);
	plat.initPos(6, 12, kDirIdleLeft);
	_vm->endSceneInit();

	plat.walkTo(Common::Point(6, 8), -1, 0x107C2, 1);
	gnap.walkTo(Common::Point(5, 8), -1, 0x107B9, 1);

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->testWalk(0, 5, -1, -1, -1, -1);

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS06Device:
			if (gnap._actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS06Platypus:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemDisguise) {
					gnap.useDisguiseOnPlatypus();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playImpossible();
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						if (_vm->isFlag(kGFKeysTaken))
							gnap.playMoan1(plat._pos);
						else
							gnap.playScratchingHead(plat._pos);
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

		case kHS06Gas:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot], 5, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playScratchingHead(Common::Point(5, 0));
						break;
					case GRAB_CURSOR:
						if (_vm->isFlag(kGFUnk04)) {
							gnap.playImpossible();
						} else if (triedDeviceOnGas) {
							_vm->_hotspots[kHS06WalkArea5]._flags |= SF_WALKABLE;
							gnap.walkTo(_vm->_hotspotsWalkPos[1], 0, 0x107BC, 1);
							_vm->_hotspots[kHS06WalkArea5]._flags &= ~SF_WALKABLE;
							gnap._actionStatus = kAS06TryToGetGas;
						} else {
							triedDeviceOnGas = true;
							gnap.playPullOutDeviceNonWorking(_vm->_hotspotsWalkPos[1]);
						}
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						if (_vm->isFlag(kGFUnk04))
							gnap.playImpossible();
						else
							gnap.playScratchingHead(Common::Point(5, 0));
						break;
					default:
						break;
					}
				}
			}
			break;

		case kHS06Ladder:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot], 8, 4);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playScratchingHead(Common::Point(8, 4));
						break;
					case GRAB_CURSOR:
						if (_vm->isFlag(kGFGasTaken))
							gnap.playImpossible();
						else {
							gnap.walkTo(_vm->_hotspotsWalkPos[2], 0, 0x107BB, 1);
							gnap._actionStatus = kAS06TryToClimbLadder;
							_vm->setFlag(kGFGasTaken);
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

		case kHS06Horse:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemTwig && _horseTurnedBack) {
					_vm->_hotspots[kHS06WalkArea5]._flags |= SF_WALKABLE;
					gnap.walkTo(_vm->_hotspotsWalkPos[3], 0, 0x107BC, 1);
					_vm->_hotspots[kHS06WalkArea5]._flags &= ~SF_WALKABLE;
					gnap._idleFacing = kDirUpLeft;
					plat.walkTo(Common::Point(6, 8), 1, 0x107C2, 1);
					plat._idleFacing = kDirIdleLeft;
					gnap._actionStatus = kAS06UseTwigOnHorse;
					_vm->setGrabCursorSprite(-1);
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot], 3, 2);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playScratchingHead(Common::Point(3, 2));
						break;
					case TALK_CURSOR:
						if (_horseTurnedBack) {
							gnap.walkTo(_vm->_hotspotsWalkPos[3], 0, gnap.getSequenceId(kGSBrainPulsating, Common::Point(3, 2)) | 0x10000, 1);
						} else {
							gnap._idleFacing = kDirBottomLeft;
							_vm->_hotspots[kHS06WalkArea5]._flags |= SF_WALKABLE;
							gnap.walkTo(_vm->_hotspotsWalkPos[3], 0, gnap.getSequenceId(kGSBrainPulsating, Common::Point(0, 0)) | 0x10000, 1);
							_vm->_hotspots[kHS06WalkArea5]._flags &= ~SF_WALKABLE;
							gnap._actionStatus = kAS06TalkToHorse;
						}
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						gnap.playImpossible();
						break;
					default:
						break;
					}
				}
			}
			break;

		case kHS06ExitOutsideBarn:
			if (gnap._actionStatus < 0) {
				_vm->_isLeavingScene = true;
				gnap.walkTo(_vm->_hotspotsWalkPos[4], 0, 0x107AE, 1);
				gnap._actionStatus = kAS06LeaveScene;
				if (_vm->_cursorValue == 1)
					_vm->_newSceneNum = 5;
				else
					_vm->_newSceneNum = 35;
			}
			break;

		case kHS06WalkArea1:
		case kHS06WalkArea2:
		case kHS06WalkArea3:
		case kHS06WalkArea4:
		case kHS06WalkArea5:
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
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(40) + 25;
				if (gnap._actionStatus < 0 && plat._actionStatus < 0 && _nextHorseSequenceId == -1) {
					if (_horseTurnedBack) {
						_nextHorseSequenceId = 0xF5;
					} else {
						switch (_vm->getRandom(5)) {
						case 0:
						case 1:
						case 2:
							_nextHorseSequenceId = 0xF1;
							break;
						case 3:
							_nextHorseSequenceId = 0xF3;
							break;
						case 4:
							_nextHorseSequenceId = 0xF4;
							break;
						default:
							break;
						}
					}
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

void Scene06::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (gnap._actionStatus) {
		case kAS06LeaveScene:
			_vm->_sceneDone = true;
			gnap._actionStatus = -1;
			break;
		case kAS06TryToGetGas:
			gameSys.insertSequence(0xFC, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gnap._sequenceId = 0xFC;
			gnap._sequenceDatNum = 0;
			gnap._actionStatus = -1;
			break;
		case kAS06TryToClimbLadder:
			gameSys.insertSequence(0xFF, 20, 0xFE, 20, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(0xFD, gnap._id, 0);
			gameSys.insertSequence(0xFD, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gnap._sequenceId = 0xFD;
			gnap._sequenceDatNum = 0;
			gnap._actionStatus = kAS06TryToClimbLadderDone;
			break;
		case kAS06TryToClimbLadderDone:
			gnap._pos = Common::Point(6, 7);
			gnap._actionStatus = -1;
			break;
		case kAS06TalkToHorse:
			_nextHorseSequenceId = 0xF6;
			break;
		case kAS06UseTwigOnHorse:
			_nextPlatSequenceId = 0xFB;
			break;
		default:
			gnap._actionStatus = -1;
			break;
		}
	}

	if (gameSys.getAnimationStatus(1) == 2) {
		gameSys.setAnimation(0, 0, 1);
		if (plat._sequenceId == 0xFA) {
			gameSys.setAnimation(0, 0, 1);
			_vm->invAdd(kItemGas);
			_vm->setFlag(kGFGasTaken);
			_vm->_hotspots[kHS06Ladder]._flags = SF_DISABLED;
			_vm->setGrabCursorSprite(kItemGas);
			plat._actionStatus = -1;
			plat._pos = Common::Point(6, 8);
			gameSys.insertSequence(0x107C1, plat._id, 0, 0, kSeqNone, 0, 450 - plat._gridX, 384 - plat._gridY);
			plat._sequenceId = 0x7C1;
			plat._sequenceDatNum = 1;
			_vm->setFlag(kGFUnk04);
			gnap._actionStatus = -1;
			_vm->showCursor();
		}
		if (_nextPlatSequenceId == 0xFB) {
			gameSys.setAnimation(0, 0, 1);
			_nextHorseSequenceId = 0xF2;
			plat._actionStatus = 6;
		}
	}

	if (gameSys.getAnimationStatus(2) == 2 && _nextHorseSequenceId != -1) {
		switch (_nextHorseSequenceId) {
		case 0xF2:
			_vm->setGrabCursorSprite(-1);
			_vm->hideCursor();
			gameSys.setAnimation(0xFA, 256, 1);
			gameSys.insertSequence(0xF2, 120, _currHorseSequenceId, 120, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0x100, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0xF7, 20, 0xF8, 20, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0xFB, plat._id, plat._sequenceId | (plat._sequenceDatNum << 16), plat._id, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0xFA, 256, 0xFB, plat._id, kSeqSyncWait, 0, 0, 0);
			plat._sequenceId = 0xFA;
			plat._sequenceDatNum = 0;
			gameSys.insertSequence(0x107B7, gnap._id, 0x100, gnap._id,
				kSeqSyncWait, 0, 75 * gnap._pos.x - gnap._gridX, 48 * gnap._pos.y - gnap._gridY);
			gnap._sequenceId = 0x7B7;
			gnap._sequenceDatNum = 1;
			_currHorseSequenceId = _nextHorseSequenceId;
			_nextHorseSequenceId = -1;
			_nextPlatSequenceId = -1;
			_vm->invRemove(kItemTwig);
			break;
		case 0xF6:
			gameSys.setAnimation(_nextHorseSequenceId, 120, 2);
			gameSys.insertSequence(0xF6, 120, _currHorseSequenceId, 120, kSeqSyncWait, 0, 0, 0);
			_horseTurnedBack = true;
			_currHorseSequenceId = _nextHorseSequenceId;
			_nextHorseSequenceId = -1;
			gnap._actionStatus = -1;
			break;
		default:
			gameSys.setAnimation(_nextHorseSequenceId, 120, 2);
			gameSys.insertSequence(_nextHorseSequenceId, 120, _currHorseSequenceId, 120, kSeqSyncWait, 0, 0, 0);
			_currHorseSequenceId = _nextHorseSequenceId;
			_nextHorseSequenceId = -1;
			break;
		}
	}
}

/*****************************************************************************/

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
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	_vm->queueInsertDeviceIcon();
	gameSys.insertSequence(0x8C, 1, 0, 0, kSeqLoop, 0, 0, 0);
	gameSys.insertSequence(0x90, 1, 0, 0, kSeqLoop, 0, 0, 0);

	_vm->invRemove(kItemGas);
	_vm->invRemove(kItemNeedle);

	if (!_vm->isFlag(kGFPlatypus))
		gameSys.insertSequence(0x8D, 1, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->_prevSceneNum == 8) {
		gnap.initPos(7, 7, kDirBottomLeft);
		plat.initPos(9, 7, kDirIdleRight);
		_vm->endSceneInit();
	} else {
		gnap._pos = Common::Point(6, 7);
		gnap._id = 140;
		gnap._sequenceId = 0x8F;
		gnap._sequenceDatNum = 0;
		gnap._idleFacing = kDirBottomRight;
		gameSys.insertSequence(0x8F, 140, 0, 0, kSeqNone, 0, 0, 0);
		gameSys.setAnimation(makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, 0);
		gnap._actionStatus = kAS07Wait;
		plat._pos = Common::Point(3, 8);
		plat._id = 160;
		plat._sequenceId = 0x91;
		plat._sequenceDatNum = 0;
		plat._idleFacing = kDirIdleLeft;
		gameSys.insertSequence(0x91, 160, 0, 0, kSeqNone, 0, 0, 0);
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
				break;
			default:
				break;
			}
			break;

		case kHS07ExitHouse:
			_vm->_isLeavingScene = true;
			if (gnap._pos.x > 8)
				gnap.walkTo(Common::Point(gnap._pos.x, 7), 0, 0x107AD, 1);
			else
				gnap.walkTo(Common::Point(8, 7), 0, 0x107AD, 1);
			gnap._actionStatus = kAS07LeaveScene;
			break;

		case kHS07Dice:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				gnap.playShowCurrItem(Common::Point(4, 8), 3, 3);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					break;
				case GRAB_CURSOR:
					_vm->setFlag(kGFPlatypus);
					_vm->invAdd(kItemDice);
					updateHotspots();
					gnap.playPullOutDevice(Common::Point(3, 3));
					gameSys.setAnimation(0x8E, 1, 2);
					gameSys.insertSequence(0x8E, 1, 141, 1, kSeqSyncWait, 0, 0, 0);
					gameSys.insertSequence(gnap.getSequenceId(kGSUseDevice, Common::Point(0, 0)) | 0x10000, gnap._id,
						makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id,
						kSeqSyncWait, 0, 75 * gnap._pos.x - gnap._gridX, 48 * gnap._pos.y - gnap._gridY);
					gnap._sequenceId = gnap.getSequenceId(kGSUseDevice, Common::Point(0, 0));
					gnap._sequenceDatNum = 1;
					break;
				case TALK_CURSOR:
				case PLAT_CURSOR:
					gnap.playImpossible();
					break;
				default:
					break;
				}
			}
			break;

		case kHS07Device:
			if (gnap._actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(40) + 50;
			}
			break;

		case kHS07WalkArea1:
		case kHS07WalkArea2:
			gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
			break;

		case kHS07WalkArea3:
			// Nothing
			break;

		default:
			if (_vm->_mouseClickState._left) {
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
				_vm->_mouseClickState._left = false;
			}
			break;
		}

		updateAnimations();

		if (!_vm->_isLeavingScene) {
			gnap.updateIdleSequence();
			if (plat._actionStatus < 0 && gnap._actionStatus < 0) {
				if (_vm->_timers[0]) {
					if (!_vm->_timers[1]) {
						_vm->_timers[1] = _vm->getRandom(20) + 30;
						int gnapRandomValue = _vm->getRandom(20);
						if (plat._idleFacing != kDirIdleLeft) {
							if (gnapRandomValue == 0 && plat._sequenceId == 0x7CA)
								plat.playSequence(0x107CC);
							else if (gnapRandomValue == 1 && plat._sequenceId == 0x7CA)
								plat.playSequence(0x10845);
							else if (plat._pos.y == 9)
								plat.playSequence(0x107CA);
						} else if (gnapRandomValue == 0 && plat._sequenceId == 0x7C9)
								plat.playSequence(0x107CB);
						else if (gnapRandomValue == 1 && plat._sequenceId == 0x7C9)
								plat.playSequence(0x10844);
						else if (plat._pos.y == 9)
								plat.playSequence(0x107C9);
						gameSys.setAnimation(plat._sequenceId | (plat._sequenceDatNum << 16), plat._id, 1);
					}
				} else {
					_vm->_timers[0] = _vm->getRandom(75) + 75;
					plat.makeRoom();
				}
			} else {
				_vm->_timers[0] = 100;
				_vm->_timers[1] = 35;
			}
			playRandomSound(4);
		}

		_vm->checkGameKeys();

		if (_vm->isKeyStatus1(Common::KEYCODE_BACKSPACE)) {
			_vm->clearKeyStatus1(Common::KEYCODE_BACKSPACE);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[4] = _vm->getRandom(40) + 50;
		}
		_vm->gameUpdateTick();
	}
}

void Scene07::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (gnap._actionStatus) {
		case kAS07LeaveScene:
			_vm->_newSceneNum = 8;
			_vm->_sceneDone = true;
			break;
		default:
			break;
		}
		gnap._actionStatus = -1;
	}

	if (gameSys.getAnimationStatus(2) == 2) {
		gameSys.setAnimation(0, 0, 2);
		_vm->setGrabCursorSprite(kItemDice);
	}
}

/*****************************************************************************/

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
	_vm->setHotspot(kHS08WalkArea1, 0, 0, 290, 340);
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
	GameSys& gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(3) == 2) {
		gameSys.setAnimation(_nextDogSequenceId, 100, 3);
		gameSys.insertSequence(_nextDogSequenceId, 100, _currDogSequenceId, 100, kSeqSyncWait, 0, 0, 0);
		_currDogSequenceId = _nextDogSequenceId;
		if ( _nextDogSequenceId != 0x135 )
			_nextDogSequenceId = 0x134;
	}
}

void Scene08::run() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	_vm->queueInsertDeviceIcon();

	gameSys.insertSequence(0x14F, 1, 0, 0, kSeqLoop, 0, 0, 0);
	gameSys.insertSequence(0x14E, 256, 0, 0, kSeqNone, 0, 0, 0);

	_currDogSequenceId = 0x135;
	_nextDogSequenceId = 0x135;

	gameSys.setAnimation(0x135, 100, 3);
	gameSys.insertSequence(_currDogSequenceId, 100, 0, 0, kSeqNone, 0, 0, 0);

	_currManSequenceId = 0x140;
	_nextManSequenceId = -1;

	gameSys.setAnimation(0x140, 100, 2);
	gameSys.insertSequence(_currManSequenceId, 100, 0, 0, kSeqNone, 0, 0, 0);

	_vm->_timers[4] = _vm->getRandom(50) + 75;

	if (!_vm->isFlag(kGFBarnPadlockOpen))
		gameSys.insertSequence(0x144, 1, 0, 0, kSeqNone, 0, 0, 0);

	if (!_vm->isFlag(kGFTruckFilledWithGas))
		gameSys.insertSequence(0x145, 1, 0, 0, kSeqNone, 0, 0, 0);

	if (!_vm->isFlag(kGFTruckKeysUsed))
		gameSys.insertSequence(0x146, 1, 0, 0, kSeqNone, 0, 0, 0);

	gnap.initPos(-1, 8, kDirBottomRight);
	plat.initPos(-1, 7, kDirIdleLeft);

	_vm->endSceneInit();

	gnap.walkTo(Common::Point(1, 8), -1, 0x107B9, 1);
	plat.walkTo(Common::Point(1, 7), -1, 0x107C2, 1);

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
			if (gnap._actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(50) + 75;
				_vm->_timers[5] = _vm->getRandom(40) + 50;
			}
			break;

		case kH08SPlatypus:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				gnap.playImpossible();
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					if (_vm->isFlag(kGFSceneFlag1))
						gnap.playMoan1(plat._pos);
					else
						gnap.playScratchingHead(plat._pos);
					break;
				case GRAB_CURSOR:
					gnap.actionIdle(0x14D);
					gnap.kissPlatypus(8);
					break;
				case TALK_CURSOR:
					gnap.playBrainPulsating(plat._pos);
					plat.playSequence(plat.getSequenceId());
					break;
				case PLAT_CURSOR:
					break;
				default:
					break;
				}
			}
			break;

		case kHS08ExitBackdoor:
			_vm->_isLeavingScene = true;
			gnap.actionIdle(0x14D);
			gnap.walkTo(Common::Point(0, 6), 0, 0x107AF, 1);
			gnap._actionStatus = kAS08LeaveScene;
			plat.walkTo(Common::Point(0, 7), 1, 0x107CF, 1);
			_vm->_newSceneNum = 9;
			break;

		case kHS08ExitCrash:
			_vm->_isLeavingScene = true;
			gnap.actionIdle(0x14D);
			gnap.walkTo(Common::Point(3, 9), 0, 0x107AE, 1);
			gnap._actionStatus = kAS08LeaveScene;
			plat.walkTo(Common::Point(4, 9), 1, 0x107C1, 1);
			_vm->_newSceneNum = 7;
			break;

		case kHS08Man:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				gnap.playShowCurrItem(Common::Point(6, 6), 7, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					gnap.actionIdle(0x14D);
					gnap.walkTo(Common::Point(6, 6), 0, 0x107BB, 1);
					gnap._actionStatus = kAS08LookMan;
					gnap._idleFacing = kDirUpRight;
					break;
				case GRAB_CURSOR:
					gnap.playImpossible();
					break;
				case TALK_CURSOR:
					gnap._idleFacing = kDirUpLeft;
					gnap.actionIdle(0x14D);
					gnap.walkTo(Common::Point(8, 6), 0, gnap.getSequenceId(kGSBrainPulsating, Common::Point(0, 0)) | 0x10000, 1);
					gnap._actionStatus = kAS08TalkMan;
					break;
				case PLAT_CURSOR:
					gnap.actionIdle(0x14D);
					gnap.useDeviceOnPlatypus();
					plat.walkTo(Common::Point(6, 6), 1, 0x107C2, 1);
					plat._actionStatus = kAS08PlatWithMan;
					plat._idleFacing = kDirIdleLeft;
					gnap.playIdle(Common::Point(6, 6));
					break;
				default:
					break;
				}
			}
			break;

		case kHS08Door:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				gnap.playShowCurrItem(Common::Point(4, 7), 5, 0);
				gameSys.setAnimation(makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, 0);
				gnap._actionStatus = kAS08GrabDog;
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					gnap.playScratchingHead(Common::Point(6, 0));
					gameSys.setAnimation(makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, 0);
					gnap._actionStatus = kAS08LookDog;
					break;
				case GRAB_CURSOR:
					gnap.walkTo(Common::Point(4, 7), 0, 0x107BB, 1);
					gnap._actionStatus = kAS08GrabDog;
					gnap._idleFacing = kDirUpRight;
					break;
				case TALK_CURSOR:
					gnap._idleFacing = kDirUpRight;
					gnap.actionIdle(0x14D);
					gnap.walkTo(Common::Point(4, 7), 0, gnap.getSequenceId(kGSBrainPulsating, Common::Point(0, 0)) | 0x10000, 1);
					gnap._actionStatus = kAS08TalkDog;
					break;
				case PLAT_CURSOR:
					_vm->setFlag(kGFSceneFlag1);
					gnap.actionIdle(0x14D);
					gnap.useDeviceOnPlatypus();
					plat.walkTo(Common::Point(3, 7), 1, 0x107C2, 1);
					plat._actionStatus = kAS08PlatWithDog;
					plat._idleFacing = kDirIdleLeft;
					gnap.playIdle(Common::Point(3, 7));
					break;
				default:
					break;
				}
			}
			break;

		case kHS08Meat:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				gnap.playShowCurrItem(Common::Point(6, 8), 5, 6);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					gnap.playScratchingHead(Common::Point(6, 7));
					break;
				case GRAB_CURSOR:
					if (_currDogSequenceId == 0x135) {
						gnap.playScratchingHead(Common::Point(6, 7));
					} else {
						gnap.actionIdle(0x14D);
						gnap.playPullOutDevice(Common::Point(6, 7));
						gnap.playUseDevice();
						_nextDogSequenceId = 0x149;
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
			break;

		case kHS08Bone:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				gnap.playShowCurrItem(Common::Point(2, 7), 3, 6);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					gnap.playScratchingHead(Common::Point(3, 6));
					break;
				case GRAB_CURSOR:
					if (_currDogSequenceId == 0x135) {
						gnap.playScratchingHead(Common::Point(3, 6));
					} else {
						gnap.actionIdle(0x14D);
						gnap.playPullOutDevice(Common::Point(3, 6));
						gnap.playUseDevice();
						_nextDogSequenceId = 0x14A;
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
			break;

		case kHS08Toy:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				gnap.playShowCurrItem(Common::Point(8, 7), 7, 6);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					gnap.playScratchingHead(Common::Point(7, 6));
					break;
				case GRAB_CURSOR:
					if (_currDogSequenceId == 0x135) {
						gnap.playScratchingHead(Common::Point(7, 6));
					} else {
						gnap.actionIdle(0x14D);
						gnap.playPullOutDevice(Common::Point(7, 6));
						gnap.playUseDevice();
						_nextDogSequenceId = 0x14B;
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
			break;

		case kHS08WalkArea1:
		case kHS08WalkArea2:
			gnap.actionIdle(0x14D);
			gnap.walkTo(Common::Point(-1, 6), -1, -1, 1);
			break;

		default:
			if (_vm->_mouseClickState._left) {
				gnap.actionIdle(0x14D);
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
				_vm->_mouseClickState._left = false;
			}
			break;
		}

		updateAnimations();

		if (!_vm->_isLeavingScene) {
			plat.updateIdleSequence();
			gnap.updateIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(50) + 125;
				if (gnap._actionStatus < 0 && plat._actionStatus < 0 && _nextManSequenceId == -1 &&
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
					default:
						break;
					}
				}
			}
			playRandomSound(5);
		}

		_vm->checkGameKeys();

		if (_vm->isKeyStatus1(Common::KEYCODE_BACKSPACE)) {
			_vm->clearKeyStatus1(Common::KEYCODE_BACKSPACE);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[4] = _vm->getRandom(50) + 75;
			_vm->_timers[5] = _vm->getRandom(40) + 50;
		}

		_vm->gameUpdateTick();
	}
}

void Scene08::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (gnap._actionStatus) {
		case kAS08LeaveScene:
			_vm->_sceneDone = true;
			gnap._actionStatus = -1;
			break;
		case kAS08TalkMan:
			_nextManSequenceId = 0x13F;
			gnap._actionStatus = -1;
			break;
		case kAS08LookMan:
			_nextManSequenceId = 0x140;
			gnap._actionStatus = -1;
			break;
		case kAS08LookDog:
			_nextManSequenceId = 0x137;
			gnap._actionStatus = -1;
			break;
		case kAS08GrabDog:
			if (_currDogSequenceId == 0x135)
				_nextDogSequenceId = 0x133;
			else
				_nextDogSequenceId = 0x13C;
			gnap._actionStatus = -1;
			break;
		case kAS08TalkDog:
			if (_currDogSequenceId == 0x135)
				_nextDogSequenceId = 0x133;
			else
				_nextDogSequenceId = 0x13C;
			gnap._actionStatus = -1;
			break;
		default:
			break;
		}
	}

	if (gameSys.getAnimationStatus(1) == 2) {
		gameSys.setAnimation(0, 0, 1);
		switch (plat._actionStatus) {
		case kAS08PlatWithDog:
			_nextDogSequenceId = 0x147;
			break;
		case kAS08PlatWithMan:
			_nextManSequenceId = 0x140;
			plat._actionStatus = -1;
			break;
		default:
			break;
		}
	}

	if (gameSys.getAnimationStatus(2) == 2 && _nextManSequenceId != -1) {
		gameSys.setAnimation(_nextManSequenceId, 100, 2);
		gameSys.insertSequence(_nextManSequenceId, 100, _currManSequenceId, 100, kSeqSyncWait, 0, 0, 0);
		_currManSequenceId = _nextManSequenceId;
		_nextManSequenceId = -1;
	}

	if (gameSys.getAnimationStatus(3) == 2) {
		if (_currDogSequenceId == 0x147)
			plat._actionStatus = -1;
		if (_currDogSequenceId == 0x149 || _currDogSequenceId == 0x14A || _currDogSequenceId == 0x14B) {
			if (_vm->getRandom(2) != 0)
				_nextManSequenceId = 0x13D;
			else
				_nextManSequenceId = 0x13E;
		} else if (_currDogSequenceId == 0x133)
			_nextManSequenceId = 0x139;
		if (_nextDogSequenceId == 0x149 || _nextDogSequenceId == 0x14A || _nextDogSequenceId == 0x14B) {
			gameSys.setAnimation(_nextDogSequenceId, 100, 3);
			gameSys.insertSequence(_nextDogSequenceId, 100, _currDogSequenceId, 100, kSeqSyncWait, 0, 0, 0);
			switch (_nextDogSequenceId) {
			case 0x149:
				_vm->setFlag(kGFBarnPadlockOpen);
				_vm->_hotspots[kHS08Meat]._flags = SF_DISABLED | SF_WALKABLE;
				gameSys.removeSequence(0x144, 1, true);
				break;
			case 0x14A:
				_vm->setFlag(kGFTruckFilledWithGas);
				_vm->_hotspots[kHS08Bone]._flags = SF_DISABLED | SF_WALKABLE;
				gameSys.removeSequence(0x145, 1, true);
				break;
			case 0x14B:
				_vm->setFlag(kGFTruckKeysUsed);
				_vm->_hotspots[kHS08Toy]._flags = SF_DISABLED | SF_WALKABLE;
				gameSys.removeSequence(0x146, 1, true);
				break;
			default:
				break;
			}
			_currDogSequenceId = _nextDogSequenceId;
			_nextDogSequenceId = 0x134;
		} else if (_nextDogSequenceId == 0x147) {
			gameSys.setAnimation(_nextDogSequenceId, 100, 3);
			gameSys.insertSequence(_nextDogSequenceId, 100, _currDogSequenceId, 100, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0x148, 160, plat._sequenceId | (plat._sequenceDatNum << 16), plat._id, kSeqSyncWait, 0, 0, 0);
			_currDogSequenceId = _nextDogSequenceId;
			_nextDogSequenceId = 0x134;
			plat._pos = Common::Point(1, 8);
			plat._id = 160;
			plat._sequenceId = 0x148;
			plat._idleFacing = kDirIdleRight;
			plat._sequenceDatNum = 0;
			if (gnap._pos == Common::Point(1, 8))
				gnap.walkStep();
		} else if (_nextDogSequenceId != -1) {
			gameSys.setAnimation(_nextDogSequenceId, 100, 3);
			gameSys.insertSequence(_nextDogSequenceId, 100, _currDogSequenceId, 100, kSeqSyncWait, 0, 0, 0);
			_currDogSequenceId = _nextDogSequenceId;
			if (_nextDogSequenceId != 0x135)
				_nextDogSequenceId = 0x134;
			if (_currDogSequenceId == 0x133) {
				_vm->_timers[2] = _vm->getRandom(30) + 20;
				_vm->_timers[3] = _vm->getRandom(50) + 200;
				gameSys.insertSequence(0x14D, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
				gnap._sequenceId = 0x14D;
				gnap._idleFacing = kDirUpRight;
				gnap._sequenceDatNum = 0;
				gnap._actionStatus = -1;
			}
		}
	}
}

/*****************************************************************************/

Scene09::Scene09(GnapEngine *vm) : Scene(vm) {
}

int Scene09::init() {
	return 0x4E;
}

void Scene09::updateHotspots() {
	_vm->setHotspot(kHS09Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS09ExitKitchen, 280, 200, 380, 400, SF_EXIT_U_CURSOR);
	_vm->setHotspot(kHS09ExitHouse, 790, 200, 799, 450, SF_EXIT_R_CURSOR | SF_WALKABLE);
	_vm->setHotspot(kHS09Trash, 440, 310, 680, 420, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS09WalkArea1, 0, 0, 799, 400);
	_vm->setHotspot(kHS09WalkArea2, 0, 0, 630, 450);
	_vm->setHotspot(kHS09WalkArea2, 0, 0, 175, 495);
	_vm->setDeviceHotspot(kHS09Device, -1, -1, -1, -1);
	_vm->_hotspotsCount = 8;
}

void Scene09::run() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	_vm->queueInsertDeviceIcon();

	gameSys.insertSequence(0x4D, 1, 0, 0, kSeqLoop, 0, 0, 0);
	gameSys.insertSequence(0x4B, 2, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->_prevSceneNum == 8) {
		gnap.initPos(11, 8, kDirBottomLeft);
		plat.initPos(12, 7, kDirIdleRight);
		_vm->endSceneInit();
		gnap.walkTo(Common::Point(9, 8), -1, 0x107BA, 1);
		plat.walkTo(Common::Point(9, 7), -1, 0x107D2, 1);
	} else {
		gnap.initPos(4, 7, kDirBottomRight);
		plat.initPos(5, 7, kDirIdleLeft);
		_vm->endSceneInit();
	}

	_vm->_timers[4] = _vm->getRandom(150) + 50;
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
		case kHS09Device:
			if (gnap._actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(150) + 50;
				_vm->_timers[5] = _vm->getRandom(40) + 50;
			}
			break;

		case kHS09Platypus:
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
				break;
			default:
				break;
			}
			break;

		case kHS09ExitKitchen:
			_vm->_isLeavingScene = true;
			_vm->_newSceneNum = 10;
			gnap.walkTo(Common::Point(4, 7), 0, 0x107BF, 1);
			gnap._actionStatus = kAS09LeaveScene;
			plat.walkTo(Common::Point(4, 8), -1, 0x107D2, 1);
			plat._idleFacing = kDirIdleRight;
			break;

		case kHS09ExitHouse:
			_vm->_isLeavingScene = true;
			_vm->_newSceneNum = 8;
			gnap.walkTo(Common::Point(10, -1), 0, 0x107AB, 1);
			gnap._actionStatus = kAS09LeaveScene;
			plat.walkTo(Common::Point(10, -1), -1, 0x107CD, 1);
			plat._idleFacing = kDirIdleRight;
			break;

		case kHS09Trash:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				gnap.playShowCurrItem(Common::Point(9, 6), 8, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					gnap.playScratchingHead(Common::Point(8, 3));
					break;
				case GRAB_CURSOR:
					gnap._actionStatus = kAS09SearchTrash;
					gnap.walkTo(Common::Point(9, 6), 0, 0x107BC, 1);
					break;
				case TALK_CURSOR:
				case PLAT_CURSOR:
					gnap.playImpossible();
					break;
				default:
					break;
				}
			}
			break;

		case kHS09WalkArea1:
		case kHS09WalkArea2:
		case kHS09WalkArea3:
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

		if (!_vm->_isLeavingScene && gnap._actionStatus != 1 && gnap._actionStatus != 2) {
			plat.updateIdleSequence();
			gnap.updateIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(150) + 100;
				if (_vm->_timers[4] & 1)
					gameSys.insertSequence(0x49, 1, 0, 0, kSeqNone, 0, 0, 0);
				else
					gameSys.insertSequence(0x4A, 1, 0, 0, kSeqNone, 0, 0, 0);
			}
			playRandomSound(5);
		}

		_vm->checkGameKeys();

		if (_vm->isKeyStatus1(Common::KEYCODE_BACKSPACE)) {
			_vm->clearKeyStatus1(Common::KEYCODE_BACKSPACE);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[4] = _vm->getRandom(150) + 50;
			_vm->_timers[5] = _vm->getRandom(40) + 50;
		}

		_vm->gameUpdateTick();
	}
}

void Scene09::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (gnap._actionStatus) {
		case kAS09LeaveScene:
			_vm->_sceneDone = true;
			gnap._actionStatus = -1;
			break;
		case kAS09SearchTrash:
			gameSys.setAnimation(0x4C, 120, 0);
			gameSys.insertSequence(0x4C, 120, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gameSys.removeSequence(0x4B, 2, true);
			gnap._sequenceId = 0x4C;
			gnap._id = 120;
			gnap._idleFacing = kDirUpLeft;
			gnap._sequenceDatNum = 0;
			gnap._pos = Common::Point(9, 6);
			gnap._actionStatus = kAS09SearchTrashDone;
			break;
		case kAS09SearchTrashDone:
			gameSys.insertSequence(0x4B, 2, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_timers[2] = 360;
			_vm->_timers[4] = _vm->getRandom(150) + 100;
			gnap._actionStatus = -1;
			break;
		default:
			break;
		}
	}
}

} // End of namespace Gnap
