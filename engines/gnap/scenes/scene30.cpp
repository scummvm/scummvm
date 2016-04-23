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
#include "gnap/scenes/scene30.h"

namespace Gnap {

enum {
	kHSPlatypus		= 0,
	kHSPillMachine	= 1,
	kHSDevice		= 2,
	kHSExitCircus	= 3,
	kHSWalkArea1	= 4
};

enum {
	kASLeaveScene				= 0,
	kASUsePillMachine			= 1,
	kASUsePillMachine2			= 2,
	kASLookPillMachine			= 3,
	kASUsePillMachine3			= 4,
	kASUsePillMachine4			= 5
};

Scene30::Scene30(GnapEngine *vm) : Scene(vm) {
	_s30_kidSequenceId = -1;
}

int Scene30::init() {
	return _vm->isFlag(kGFUnk23) ? 0x10B : 0x10A;
}

void Scene30::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSPillMachine, 598, 342, 658, 426, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 9, 7);
	_vm->setHotspot(kHSExitCircus, 100, 590 - _vm->_deviceY1, 700, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
	_vm->setHotspot(kHSWalkArea1, 0, 0, 800, 514);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_vm->_hotspotsCount = 5;
}

void Scene30::run() {
	bool hasTakenPill = false;

	_vm->playSound(0x1093B, true);
	_vm->startSoundTimerB(6);

	_vm->queueInsertDeviceIcon();

	if (_vm->isFlag(kGFUnk23))
		_vm->_gameSys->insertSequence(0x106, 1, 0, 0, kSeqNone, 0, 0, 0);

	if (!_vm->isFlag(kGFUnk13))
		_vm->_gameSys->insertSequence(0x107, 1, 0, 0, kSeqNone, 0, 0, 0);
	_vm->_timers[5] = _vm->getRandom(50) + 180;

	_vm->_gameSys->insertSequence(0x101, 40, 0, 0, kSeqNone, 0, 0, 0);
	_vm->_timers[4] = _vm->getRandom(100) + 300;

	_s30_kidSequenceId = 0x101;
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
		case kHSDevice:
			if (_vm->_gnapActionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHSPlatypus:
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

		case kHSPillMachine:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemDiceQuarterHole && !_vm->isFlag(kGFUnk23)) {
					_vm->_hotspots[kHSWalkArea1]._flags |= SF_WALKABLE;
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSPillMachine].x, _vm->_hotspotsWalkPos[kHSPillMachine].y, 0, 0x107BC, 1);
					_vm->_hotspots[kHSWalkArea1]._flags &= ~SF_WALKABLE;
					_vm->_gnapActionStatus = kASUsePillMachine;
					hasTakenPill = true;
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHSPillMachine].x, _vm->_hotspotsWalkPos[kHSPillMachine].y, 8, 5);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->gnapWalkTo(9, 8, 0, 0x107BC, 1);
						_vm->_gnapActionStatus = kASLookPillMachine;
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

		case kHSExitCircus:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				if (hasTakenPill)
					_vm->_newSceneNum = 47;
				else
					_vm->_newSceneNum = 26;
				_vm->gnapWalkTo(-1, _vm->_hotspotsWalkPos[kHSExitCircus].y, 0, 0x107AE, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitCircus].x + 1, _vm->_hotspotsWalkPos[kHSExitCircus].y, -1, 0x107C2, 1);
			}
			break;

		case kHSWalkArea1:
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
						_vm->_gameSys->insertSequence(0xFF, 40, 0, 0, kSeqNone, 0, 0, 0);
						_vm->_gameSys->insertSequence(0x100, 40, _s30_kidSequenceId, 40, kSeqSyncWait, 0, 0, 0);
						_s30_kidSequenceId = 0x100;
					} else {
						_vm->_gameSys->insertSequence(0xFE, 40, 0, 0, kSeqNone, 0, 0, 0);
					}
				}
			}
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(50) + 180;
				if (_vm->_gnapActionStatus < 0) {
					if (!_vm->isFlag(kGFUnk23) || hasTakenPill)
						_vm->_gameSys->insertSequence(0x109, 20, 0, 0, kSeqNone, 0, 0, 0);
					else
						_vm->_gameSys->insertSequence(0x108, 20, 0, 0, kSeqNone, 0, 0, 0);
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
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kASLeaveScene:
			_vm->_sceneDone = true;
			break;
		case kASUsePillMachine:
			_vm->setGrabCursorSprite(-1);
			_vm->_gameSys->setAnimation(0x105, _vm->_gnapId, 0);
			_vm->_gameSys->insertSequence(0x105, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceId = 0x105;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapActionStatus = kASUsePillMachine2;
			break;
		case kASUsePillMachine2:
			_vm->hideCursor();
			_vm->setGrabCursorSprite(-1);
			_vm->addFullScreenSprite(0x3F, 255);
			_vm->_gameSys->removeSequence(0x105, _vm->_gnapId, true);
			_vm->_gameSys->setAnimation(0x102, 256, 0);
			_vm->_gameSys->insertSequence(0x102, 256, 0, 0, kSeqNone, 0, 0, 0);
			while (_vm->_gameSys->getAnimationStatus(0) != 2)
				_vm->gameUpdateTick();
			_vm->_gameSys->setAnimation(0x103, _vm->_gnapId, 0);
			_vm->_gameSys->insertSequence(0x103, _vm->_gnapId, 0, 0, kSeqNone, 0, 0, 0);
			_vm->removeFullScreenSprite();
			_vm->showCursor();
			_vm->_gnapActionStatus = kASUsePillMachine3;
			_vm->invAdd(kItemPill);
			_vm->setFlag(kGFUnk23);
			break;
		case kASUsePillMachine3:
			_vm->_gameSys->setAnimation(0x104, _vm->_gnapId, 0);
			_vm->_gameSys->insertSequence(0x104, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, 0x103), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceId = 0x104;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapActionStatus = kASUsePillMachine4;
			_vm->setGrabCursorSprite(kItemDiceQuarterHole);
			break;
		case kASUsePillMachine4:
			_vm->_gameSys->insertSequence(0x106, 1, 0, 0, kSeqNone, 0, 0, 0);
			_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSPillMachine].x, _vm->_hotspotsWalkPos[kHSPillMachine].y + 1, -1, 0x107BC, 1);
			_vm->_gnapActionStatus = -1;
			break;
		case kASLookPillMachine:
			if (_vm->isFlag(kGFUnk23))
				_vm->showFullScreenSprite(0xE3);
			else
				_vm->showFullScreenSprite(0xE2);
			_vm->_gnapActionStatus = -1;
			break;
		}
	}
}

} // End of namespace Gnap
