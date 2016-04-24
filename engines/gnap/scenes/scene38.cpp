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
#include "gnap/scenes/scene38.h"

namespace Gnap {

enum {
	kHSPlatypus			= 0,
	kHSExitHouse		= 1,
	kHSExitCave			= 2,
	kHSTrapDoorLid1		= 3,
	kHSTrapDoorLid2		= 4,
	kHSHuntingTrophy	= 5,
	kHSWalkArea1		= 6,
	kHSDevice			= 7,
	kHSWalkArea2		= 8,
	kHSWalkArea3		= 9,
	kHSWalkArea4		= 10,
	kHSWalkArea5		= 11,
	kHSWalkArea6		= 12
};

enum {
	kASLeaveScene				= 0,
	kASExitCave					= 1,
	kASUseHuntingTrophy			= 2,
	kASHoldingHuntingTrophy		= 3,
	kASReleaseHuntingTrophy		= 4,
	kASUsePlatypusWithTrapDoor	= 5,
	kASPlatypusHoldingTrapDoor	= 6
};

Scene38::Scene38(GnapEngine *vm) : Scene(vm) {
}

int Scene38::init() {
	_vm->_gameSys->setAnimation(0, 0, 0);
	_vm->_gameSys->setAnimation(0, 0, 1);
	return 0xA5;
}

void Scene38::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSExitHouse, 150, 585, 650, 600, SF_EXIT_D_CURSOR, 0, 8);
	_vm->setHotspot(kHSExitCave, 430, 440, 655, 470, SF_WALKABLE, 0, 8);
	_vm->setHotspot(kHSTrapDoorLid1, 525, 265, 640, 350, SF_DISABLED);
	_vm->setHotspot(kHSTrapDoorLid2, 555, 350, 670, 430, SF_DISABLED);
	_vm->setHotspot(kHSHuntingTrophy, 170, 85, 250, 190, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 0, 8);
	_vm->setHotspot(kHSWalkArea1, 330, 270, 640, 380, SF_DISABLED | SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 0, 8);
	_vm->setHotspot(kHSWalkArea2, 0, 0, 799, 396);
	_vm->setHotspot(kHSWalkArea3, 0, 585, 799, 599, SF_WALKABLE | SF_DISABLED);
	_vm->setHotspot(kHSWalkArea4, 0, 0, 97, 445);
	_vm->setHotspot(kHSWalkArea5, 770, 0, 799, 445);
	_vm->setHotspot(kHSWalkArea6, 393, 0, 698, 445, SF_WALKABLE | SF_DISABLED);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	if (_vm->_platypusActionStatus == kASPlatypusHoldingTrapDoor)
		_vm->_hotspots[kHSPlatypus]._flags = SF_WALKABLE | SF_DISABLED;
	if (_vm->_platypusActionStatus == kASPlatypusHoldingTrapDoor)
		_vm->_hotspots[kHSExitCave]._flags = SF_EXIT_D_CURSOR;
	else if (_vm->_gnapActionStatus == kASHoldingHuntingTrophy)
		_vm->_hotspots[kHSExitCave]._flags = SF_EXIT_D_CURSOR;
	if (_vm->_platypusActionStatus == kASPlatypusHoldingTrapDoor)
		_vm->_hotspots[kHSTrapDoorLid1]._flags = SF_DISABLED;
	else if (_vm->_gnapActionStatus == kASHoldingHuntingTrophy)
		_vm->_hotspots[kHSTrapDoorLid1]._flags = SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	if (_vm->_platypusActionStatus == kASPlatypusHoldingTrapDoor)
		_vm->_hotspots[kHSTrapDoorLid2]._flags = SF_DISABLED;
	else if (_vm->_gnapActionStatus == kASHoldingHuntingTrophy)
		_vm->_hotspots[kHSTrapDoorLid2]._flags = SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	if (_vm->_platypusActionStatus == kASPlatypusHoldingTrapDoor)
		_vm->_hotspots[kHSWalkArea6]._flags = SF_NONE;
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
		case kHSDevice:
			_vm->runMenu();
			updateHotspots();
			break;

		case kHSPlatypus:
			if (_vm->_gnapActionStatus == kASHoldingHuntingTrophy) {
				_vm->_gnapActionStatus = kASReleaseHuntingTrophy;
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

		case kHSExitHouse:
			if (_vm->_gnapActionStatus == kASHoldingHuntingTrophy) {
				_vm->_gnapActionStatus = kASReleaseHuntingTrophy;
			} else {
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(-1, -1, 0, 0x107AE, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->_newSceneNum = 37;
			}
			break;

		case kHSExitCave:
			if (_vm->_gnapActionStatus == kASHoldingHuntingTrophy) {
				_vm->_gnapActionStatus = kASReleaseHuntingTrophy;
				if (_vm->_platypusActionStatus == kASPlatypusHoldingTrapDoor)
					_vm->_isLeavingScene = true;
			} else if (_vm->_platypusActionStatus == kASPlatypusHoldingTrapDoor) {
				_vm->_sceneWaiting = false;
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(5, 7, 0, 0x107BB, 1);
				_vm->_newSceneNum = 39;
				_vm->_gnapActionStatus = kASExitCave;
			}
			break;

		case kHSTrapDoorLid1:
		case kHSTrapDoorLid2:
			if (_vm->_gnapActionStatus == kASHoldingHuntingTrophy) {
				if (_vm->_verbCursor == PLAT_CURSOR && _vm->_platypusActionStatus != kASPlatypusHoldingTrapDoor)
					_vm->_gnapActionStatus = kASUsePlatypusWithTrapDoor;
				else
					_vm->_gnapActionStatus = kASReleaseHuntingTrophy;
			}
			break;

		case kHSHuntingTrophy:
			if (_vm->_gnapActionStatus != kASHoldingHuntingTrophy) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(3, 6, 2, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(0, 0);
						break;
					case GRAB_CURSOR:
						if (_vm->_platypusActionStatus == kASPlatypusHoldingTrapDoor)
							_vm->playGnapImpossible(0, 0);
						else {
							_vm->gnapWalkTo(3, 6, 0, 0x107BB, 1);
							_vm->platypusWalkTo(4, 8, -1, -1, 1);
							_vm->_gnapActionStatus = kASUseHuntingTrophy;
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

		case kHSWalkArea1:
			// Nothing
			break;

		case kHSWalkArea2:
		case kHSWalkArea3:
		case kHSWalkArea4:
		case kHSWalkArea5:
		case kHSWalkArea6:
			if (_vm->_gnapActionStatus == kASHoldingHuntingTrophy)
				_vm->_gnapActionStatus = kASReleaseHuntingTrophy;
			else if (_vm->_gnapActionStatus < 0)
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			break;

		default:
			if (_vm->_mouseClickState._left) {
				if (_vm->_gnapActionStatus == kASHoldingHuntingTrophy)
					_vm->_gnapActionStatus = kASReleaseHuntingTrophy;
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
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kASLeaveScene:
			_vm->_sceneDone = true;
			break;
		case kASExitCave:
			_vm->_gameSys->removeSequence(_vm->_platypusSequenceId | (_vm->_platypusSequenceDatNum << 16), _vm->_platypusId, true);
			_vm->_gameSys->insertSequence(0xA3, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceId = 0xA3;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gameSys->setAnimation(0xA3, _vm->_gnapId, 0);
			_vm->_gnapActionStatus = kASLeaveScene;
			break;
		case kASUseHuntingTrophy:
			_vm->_gameSys->removeSequence(0x9B, 0, true);
			_vm->_gameSys->insertSequence(0x9C, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceId = 0x9C;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gameSys->setAnimation(0x9C, _vm->_gnapId, 0);
			_vm->_gnapActionStatus = kASHoldingHuntingTrophy;
			updateHotspots();
			break;
		case kASHoldingHuntingTrophy:
			if (_vm->_platypusActionStatus != kASPlatypusHoldingTrapDoor)
				_vm->_sceneWaiting = true;
			if (_vm->_gnapSequenceId == 0xA4) {
				_vm->_gameSys->insertSequence(0x9D, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
				_vm->_gnapSequenceId = 0x9D;
			} else {
				_vm->_gameSys->insertSequence(0xA4, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
				_vm->_gnapSequenceId = 0xA4;
			}
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gameSys->setAnimation(_vm->_gnapSequenceId, _vm->_gnapId, 0);
			break;
		case kASReleaseHuntingTrophy:
			if (_vm->_gnapSequenceId == 0x9E) {
				_vm->_gameSys->insertSequence(0x9B, 0, 0, 0, kSeqNone, 0, 0, 0);
				_vm->_gnapActionStatus = -1;
			} else if (_vm->_platypusActionStatus == kASPlatypusHoldingTrapDoor) {
				_vm->_gameSys->insertSequence(0xA0, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
				_vm->_gnapSequenceId = 0xA0;
				_vm->_gnapSequenceDatNum = 0;
				_vm->_gnapX = 3;
				_vm->_gnapY = 6;
				_vm->_gnapIdleFacing = kDirBottomRight;
				if (_vm->_isLeavingScene) {
					_vm->_sceneWaiting = false;
					_vm->gnapWalkTo(5, 7, 0, 0x107BB, 1);
					_vm->_newSceneNum = 39;
					_vm->_gnapActionStatus = kASExitCave;
				} else {
					_vm->_gnapActionStatus = -1;
				}
			} else {
				_vm->_gameSys->insertSequence(0x9E, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
				_vm->_gnapSequenceId = 0x9E;
				_vm->_gnapSequenceDatNum = 0;
				_vm->_gnapX = 3;
				_vm->_gnapY = 6;
				_vm->_gnapIdleFacing = kDirBottomRight;
				_vm->_gameSys->setAnimation(0x9E, _vm->_gnapId, 0);
				_vm->_sceneWaiting = false;
				updateHotspots();
			}
			break;
		case kASUsePlatypusWithTrapDoor:
			_vm->_sceneWaiting = false;
			_vm->_gameSys->insertSequence(0x9F, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceId = 0x9F;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gameSys->setAnimation(0x9F, _vm->_gnapId, 0);
			_vm->_gnapActionStatus = kASHoldingHuntingTrophy;
			if (_vm->_platypusFacing != kDirNone)
				_vm->playPlatypusSequence(0x107D5);
			else
				_vm->playPlatypusSequence(0x107D4);
			_vm->platypusWalkTo(8, 7, -1, 0x107D2, 1);
			_vm->_gameSys->insertSequence(0xA1, _vm->_gnapId + 1, _vm->_platypusSequenceId | (_vm->_platypusSequenceDatNum << 16), _vm->_platypusId, kSeqSyncWait, 0, 0, 0);
			_vm->_platypusSequenceId = 0xA1;
			_vm->_platypusSequenceDatNum = 0;
			_vm->_platypusId = _vm->_gnapId + 1;
			_vm->_gameSys->setAnimation(0xA1, _vm->_gnapId + 1, 1);
			_vm->_platypusActionStatus = kASPlatypusHoldingTrapDoor;
			updateHotspots();
			break;
		}
	}

	if (_vm->_gameSys->getAnimationStatus(1) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 1);
		if (_vm->_platypusActionStatus == kASPlatypusHoldingTrapDoor) {
			_vm->_gameSys->insertSequence(0xA2, _vm->_platypusId, _vm->_platypusSequenceId | (_vm->_platypusSequenceDatNum << 16), _vm->_platypusId, kSeqSyncWait, 0, 0, 0);
			_vm->_platypusSequenceId = 0xA2;
			_vm->_platypusSequenceDatNum = 0;
			updateHotspots();
			_vm->_sceneWaiting = true;
		}
	}
}

} // End of namespace Gnap
