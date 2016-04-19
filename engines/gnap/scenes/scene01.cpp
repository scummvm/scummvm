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
#include "gnap/scenes/scene01.h"

namespace Gnap {

enum {
	kHSPlatypus		= 0,
	kHSExitTruck	= 1,
	kHSMud			= 2,
	kHSPigs			= 3,
	kHSSpaceship	= 4,
	kHSDevice		= 5,
	kHSWalkArea1	= 6,
	kHSWalkArea2	= 7,
	kHSWalkArea3	= 8,
	kHSWalkArea4	= 9,
	kHSWalkArea5	= 10,
	kHSWalkArea6	= 11,
	kHSWalkArea7	= 12,
	kHSWalkArea8	= 13
};

enum {
	kASLookSpaceship		= 1,
	kASLookSpaceshipDone	= 2,
	kASLeaveScene			= 3,
	kASTakeMud				= 5,
	kASLookPigs				= 6,
	kASUsePigs				= 7
};

Scene01::Scene01(GnapEngine *vm) : Scene(vm) {
	_s01_pigsIdCtr = 0;
	_s01_smokeIdCtr = 0;
	_s01_spaceshipSurface = nullptr;
}

int Scene01::init() {
	_vm->_gameSys->setAnimation(0, 0, 0);
	_vm->_gameSys->setAnimation(0, 0, 3);
	return 0x88;
}

void Scene01::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_DISABLED | SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSExitTruck, 780, 226, 800, 455, SF_EXIT_R_CURSOR | SF_WALKABLE, 10, 6);
	_vm->setHotspot(kHSMud, 138, 282, 204, 318, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 5);
	_vm->setHotspot(kHSPigs, 408, 234, 578, 326, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 4);
	_vm->setHotspot(kHSSpaceship, 0, 200, 94, 292, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 1, 6);
	_vm->setHotspot(kHSWalkArea1, 0, 0, 162, 426);
	_vm->setHotspot(kHSWalkArea2, 162, 0, 237, 396);
	_vm->setHotspot(kHSWalkArea3, 237, 0, 319, 363);
	_vm->setHotspot(kHSWalkArea4, 520, 0, 800, 404);
	_vm->setHotspot(kHSWalkArea5, 300, 447, 800, 600);
	_vm->setHotspot(kHSWalkArea6, 678, 0, 800, 404);
	_vm->setHotspot(kHSWalkArea7, 0, 0, 520, 351);
	_vm->setHotspot(kHSWalkArea8, 0, 546, 300, 600);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	if (_vm->isFlag(kGFPlatypus))
		_vm->_hotspots[kHSPlatypus]._flags = SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	if (_vm->isFlag(kGFMudTaken))
		_vm->_hotspots[kHSMud]._flags = SF_WALKABLE | SF_DISABLED;
	_vm->_hotspotsCount = 14;
}

void Scene01::run() {
	// NOTE Removed _s01_dword_474380 which was set when the mud was taken
	// which is also set in the global game flags.
	
	_vm->playSound(0x1091C, true);
	_vm->startSoundTimerC(5);

	_vm->_gameSys->setAnimation(134, 20, 4);
	_vm->_gameSys->insertSequence(134, 20, 0, 0, kSeqNone, 0, 0, 0);

	_vm->_gameSys->setAnimation(0x7F, 40, 2);
	_vm->_gameSys->insertSequence(0x7F, 40, 0, 0, kSeqNone, 0, 0, 0);

	_vm->_timers[4] = _vm->getRandom(100) + 300;

	if (!_vm->isFlag(kGFMudTaken))
		_vm->_gameSys->insertSequence(129, 40, 0, 0, kSeqNone, 0, 0, 0);

	_vm->queueInsertDeviceIcon();

	if (_vm->_prevSceneNum == 2) {
		_vm->initGnapPos(11, 6, kDirBottomLeft);
		if (_vm->isFlag(kGFPlatypus))
			_vm->initPlatypusPos(12, 6, kDirUnk4);
		_vm->endSceneInit();
		if (_vm->isFlag(kGFPlatypus))
			_vm->platypusWalkTo(9, 6, -1, 0x107C2, 1);
		_vm->gnapWalkTo(8, 6, -1, 0x107B9, 1);
	} else {
		_vm->initGnapPos(1, 6, kDirBottomRight);
		if (_vm->isFlag(kGFPlatypus))
			_vm->initPlatypusPos(1, 7, kDirNone);
		_vm->endSceneInit();
	}

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
		_vm->testWalk(0, 3, -1, -1, -1, -1);

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		
		case kHSDevice:
			if (_vm->_gnapActionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHSPlatypus:
			if (_vm->_gnapActionStatus < 0 && _vm->isFlag(kGFPlatypus)) {
				if (_vm->_grabCursorSpriteIndex == kItemDisguise) {
					_vm->gnapUseDisguiseOnPlatypus();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapImpossible(0, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						if (_vm->isFlag(kGFKeysTaken))
							_vm->playGnapMoan1(_vm->_platX, _vm->_platY);
						else
							_vm->playGnapScratchingHead(_vm->_platX, _vm->_platY);
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

		case kHSSpaceship:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[4].x, _vm->_hotspotsWalkPos[4].y, 0, 2);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->_gnapIdleFacing = kDirUpLeft;
						if (_vm->gnapWalkTo(_vm->_hotspotsWalkPos[4].x, _vm->_hotspotsWalkPos[4].y, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1))
							_vm->_gnapActionStatus = kASLookSpaceship;
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

		case kHSMud:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[2].x, _vm->_hotspotsWalkPos[2].y, 2, 3);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(3, 3);
						break;
					case GRAB_CURSOR:
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[2].x, _vm->_hotspotsWalkPos[2].y, 0, _vm->getGnapSequenceId(gskIdle, 2, 3) | 0x10000, 1);
						_vm->_gnapActionStatus = kASTakeMud;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSPigs:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[3].x, _vm->_hotspotsWalkPos[3].y, 7, 2);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->_gnapIdleFacing = kDirUpRight;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[3].x, _vm->_hotspotsWalkPos[3].y, 0, _vm->getGnapSequenceId(gskIdle, 7, 2) | 0x10000, 1);
						_vm->_gnapActionStatus = kASLookPigs;
						break;
					case GRAB_CURSOR:
						_vm->_gnapIdleFacing = kDirUpRight;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[3].x, _vm->_hotspotsWalkPos[3].y, 0, _vm->getGnapSequenceId(gskIdle, 7, 2) | 0x10000, 1);
						_vm->_gnapActionStatus = kASUsePigs;
						break;
					case TALK_CURSOR:
						_vm->_gnapIdleFacing = kDirUpRight;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[3].x, _vm->_hotspotsWalkPos[3].y, 0, _vm->getGnapSequenceId(gskBrainPulsating, 7, 2) | 0x10000, 1);
						_vm->_gnapActionStatus = kASLookPigs;
						break;
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSExitTruck:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[1].x, _vm->_hotspotsWalkPos[1].y, 0, 0x107AB, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				if (_vm->isFlag(kGFPlatypus))
					_vm->platypusWalkTo(_vm->_hotspotsWalkPos[1].x, _vm->_hotspotsWalkPos[1].y + 1, -1, 0x107CD, 1);
				_vm->_newSceneNum = 2;
			}
			break;

		case kHSWalkArea1:
		case kHSWalkArea2:
		case kHSWalkArea3:
		case kHSWalkArea4:
		case kHSWalkArea5:
		case kHSWalkArea6:
		case kHSWalkArea7:
		case kHSWalkArea8:
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
			if (_vm->_platypusActionStatus < 0 && _vm->isFlag(kGFPlatypus))
				_vm->updatePlatypusIdleSequence();
			if (_vm->_gnapActionStatus < 0)
				_vm->updateGnapIdleSequence();
			if (_vm->_timers[4] == 0) {
				// Update bird animation
				_vm->_timers[4] = _vm->getRandom(100) + 300;
				if (_vm->getRandom(1) == 0)
					_vm->_gameSys->insertSequence(0x84, 180, 0, 0, kSeqNone, 0, 0, 0);
				else
					_vm->_gameSys->insertSequence(0x83, 180, 0, 0, kSeqNone, 0, 0, 0);
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

void Scene01::updateAnimations() {	
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kASLookSpaceship:
			_s01_spaceshipSurface = _vm->_gameSys->createSurface(47);
			_vm->_gameSys->insertSpriteDrawItem(_s01_spaceshipSurface, 0, 0, 255);
			_vm->_gameSys->setAnimation(133, 256, 0);
			_vm->_gameSys->insertSequence(133, 256, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_gnapActionStatus = kASLookSpaceshipDone;
			break;

		case kASLookSpaceshipDone:
			_vm->_gameSys->removeSequence(133, 256, true);
			_vm->_gameSys->removeSpriteDrawItem(_s01_spaceshipSurface, 255);
			_vm->deleteSurface(&_s01_spaceshipSurface);
			_vm->_gnapActionStatus = -1;
			break;

		case kASLeaveScene:
			_vm->_sceneDone = true;
			break;

		case kASTakeMud:
			_vm->playGnapPullOutDevice(2, 3);
			_vm->playGnapUseDevice(0, 0);
			_vm->_gameSys->insertSequence(128, 40, 129, 40, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(128, 40, 3);
			_vm->_gnapActionStatus = -1;
			break;

		case kASLookPigs:
			_vm->playSound(138, false);
			_vm->playSound(139, false);
			_vm->playSound(140, false);
			_vm->_gnapActionStatus = -1;
			break;

		case kASUsePigs:
			_vm->playGnapPullOutDevice(7, 2);
			_vm->playGnapUseDevice(0, 0);
			_vm->_gameSys->insertSequence(135, 39, 0, 0, kSeqNone, 25, _vm->getRandom(140) - 40, 0);
			_vm->_gnapActionStatus = -1;
			break;
		}
	}

	if (_vm->_gameSys->getAnimationStatus(3) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 3);
		_vm->invAdd(kItemMud);
		_vm->setGrabCursorSprite(kItemMud);
		_vm->setFlag(kGFMudTaken);
		updateHotspots();
	}

	if (_vm->_gameSys->getAnimationStatus(4) == 2) {
		_s01_smokeIdCtr = (_s01_smokeIdCtr + 1) % 2;
		_vm->_gameSys->setAnimation(0x86, _s01_smokeIdCtr + 20, 4);
		_vm->_gameSys->insertSequence(0x86, _s01_smokeIdCtr + 20,
			0x86, (_s01_smokeIdCtr + 1) % 2 + 20,
			kSeqSyncWait, 0, 0, 0);
	}

	if (_vm->_gameSys->getAnimationStatus(2) == 2) {
		_s01_pigsIdCtr = (_s01_pigsIdCtr + 1) % 2;
		_vm->_gameSys->setAnimation(0x7F, _s01_pigsIdCtr + 40, 2);
		_vm->_gameSys->insertSequence(0x7F, _s01_pigsIdCtr + 40,
			0x7F, (_s01_pigsIdCtr + 1) % 2 + 40,
			kSeqSyncWait, 0, 0, 0);
	}
}

} // End of namespace Gnap
