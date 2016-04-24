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
#include "gnap/scenes/scene33.h"

namespace Gnap {

enum {
	kHSPlatypus		= 0,
	kHSChicken		= 1,
	kHSDevice		= 2,
	kHSExitHouse	= 3,
	kHSExitBarn		= 4,
	kHSExitCreek	= 5,
	kHSExitPigpen	= 6,
	kHSWalkArea1	= 7,
	kHSWalkArea2	= 8
};

enum {
	kASLeaveScene			= 0,
	kASTalkChicken			= 1,
	kASUseChicken			= 2,
	kASUseChickenDone		= 3
};

Scene33::Scene33(GnapEngine *vm) : Scene(vm) {
	_s33_currChickenSequenceId = -1;
	_s33_nextChickenSequenceId = -1;
}

int Scene33::init() {
	return _vm->isFlag(kGFPlatypusTalkingToAssistant) ? 0x84 : 0x85;
}

void Scene33::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSChicken, 606, 455, 702, 568, SF_WALKABLE | SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 7, 8);
	_vm->setHotspot(kHSExitHouse, 480, 120, 556, 240, SF_EXIT_U_CURSOR, 7, 3);
	_vm->setHotspot(kHSExitBarn, 610, 75, 800, 164, SF_EXIT_U_CURSOR, 10, 3);
	_vm->setHotspot(kHSExitCreek, 780, 336, 800, 556, SF_EXIT_R_CURSOR | SF_WALKABLE, 10, 8);
	_vm->setHotspot(kHSExitPigpen, 0, 300, 20, 600, SF_EXIT_L_CURSOR | SF_WALKABLE, 0, 8);
	_vm->setHotspot(kHSWalkArea1, 120, 0, 514, 458);
	_vm->setHotspot(kHSWalkArea2, 0, 0, 800, 452);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_vm->_hotspotsCount = 9;
}

void Scene33::run() {
	_vm->playSound(0x1091C, true);
	_vm->startSoundTimerC(6);
	_vm->queueInsertDeviceIcon();

	_s33_currChickenSequenceId = 0x7E;
	_vm->_gameSys->setAnimation(0x7E, 179, 2);
	_vm->_gameSys->insertSequence(_s33_currChickenSequenceId, 179, 0, 0, kSeqNone, 0, 0, 0);
	_s33_nextChickenSequenceId = -1;
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
		case kHSDevice:
			if (_vm->_gnapActionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
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

		case kHSChicken:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(7, 9, 9, 8);
				} else {
					switch (_vm->_verbCursor) {
					case GRAB_CURSOR:
						_vm->_gnapIdleFacing = kDirBottomRight;
						if (_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSChicken].x, _vm->_hotspotsWalkPos[kHSChicken].y, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1))
							_vm->_gnapActionStatus = kASUseChicken;
						else
							_vm->_gnapActionStatus = -1;
						break;
					case TALK_CURSOR:
						_vm->_gnapIdleFacing = kDirBottomRight;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSChicken].x, _vm->_hotspotsWalkPos[kHSChicken].y, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_vm->_gnapActionStatus = kASTalkChicken;
						break;
					case LOOK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSExitHouse:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->_newSceneNum = 37;
				if (_vm->_gnapX > 6)
					_vm->gnapWalkTo(_vm->_gnapX, _vm->_gnapY, 0, 0x107AD, 1);
				else
					_vm->gnapWalkTo(6, 7, 0, 0x107B1, 1);
			}
			break;

		case kHSExitBarn:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->_newSceneNum = 35;
				if (_vm->_gnapX > 7)
					_vm->gnapWalkTo(_vm->_gnapX, _vm->_gnapY, 0, 0x107AD, 1);
				else
					_vm->gnapWalkTo(7, 7, 0, 0x107B1, 1);
			}
			break;

		case kHSExitCreek:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitCreek].x, _vm->_hotspotsWalkPos[kHSExitCreek].y, 0, 0x107AB, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitCreek].x, _vm->_hotspotsWalkPos[kHSExitCreek].y, -1, 0x107CD, 1);
				_vm->_newSceneNum = 34;
			}
			break;

		case kHSExitPigpen:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitPigpen].x, _vm->_hotspotsWalkPos[kHSExitPigpen].y, 0, 0x107AF, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitPigpen].x, _vm->_hotspotsWalkPos[kHSExitPigpen].y, -1, 0x107CF, 1);
				_vm->_newSceneNum = 32;
			}
			break;

		case kHSWalkArea1:
		case kHSWalkArea2:
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
					_vm->_gameSys->insertSequence(0x83, 256, 0, 0, kSeqNone, 0, 0, 0);
				else
					_vm->_gameSys->insertSequence(0x82, 256, 0, 0, kSeqNone, 0, 0, 0);
			}
			if (!_vm->_timers[5] && _s33_nextChickenSequenceId == -1 && _vm->_gnapActionStatus != kASTalkChicken && _vm->_gnapActionStatus != kASUseChicken) {
				if (_vm->getRandom(6) != 0) {
					_s33_nextChickenSequenceId = 0x7E;
					_vm->_timers[5] = _vm->getRandom(20) + 30;
				} else {
					_s33_nextChickenSequenceId = 0x80;
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
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		switch (_vm->_gnapActionStatus) {
		case kASLeaveScene:
			_vm->_sceneDone = true;
			break;
		case kASTalkChicken:
			_s33_nextChickenSequenceId = 0x7F;
			break;
		case kASUseChicken:
			_s33_nextChickenSequenceId = 0x81;
			_vm->_timers[2] = 100;
			break;
		case kASUseChickenDone:
			_vm->_gameSys->insertSequence(0x107B5, _vm->_gnapId, 0x81, 179, kSeqSyncWait, 0, 75 * _vm->_gnapX - _vm->_gnapGridX, 48 * _vm->_gnapY - _vm->_gnapGridY);
			_vm->_gnapSequenceId = 0x7B5;
			_vm->_gnapSequenceDatNum = 1;
			_s33_currChickenSequenceId = 0x7E;
			_vm->_gameSys->setAnimation(0x7E, 179, 2);
			_vm->_gameSys->insertSequence(_s33_currChickenSequenceId, 179, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_gnapActionStatus = -1;
			_vm->_timers[5] = 30;
			break;
		default:
			_vm->_gnapActionStatus = -1;
			break;
		}
	}

	if (_vm->_gameSys->getAnimationStatus(2) == 2) {
		if (_s33_nextChickenSequenceId == 0x81) {
			_vm->_gameSys->setAnimation(_s33_nextChickenSequenceId, 179, 0);
			_vm->_gameSys->insertSequence(_s33_nextChickenSequenceId, 179, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->removeSequence(_s33_currChickenSequenceId, 179, true);
			_s33_nextChickenSequenceId = -1;
			_s33_currChickenSequenceId = -1;
			_vm->_gnapActionStatus = kASUseChickenDone;
			_vm->_timers[5] = 500;
		} else if (_s33_nextChickenSequenceId == 0x7F) {
			_vm->_gameSys->setAnimation(_s33_nextChickenSequenceId, 179, 2);
			_vm->_gameSys->insertSequence(_s33_nextChickenSequenceId, 179, _s33_currChickenSequenceId, 179, kSeqSyncWait, 0, 0, 0);
			_s33_currChickenSequenceId = _s33_nextChickenSequenceId;
			_s33_nextChickenSequenceId = -1;
			_vm->_gnapActionStatus = -1;
		} else if (_s33_nextChickenSequenceId != -1) {
			_vm->_gameSys->setAnimation(_s33_nextChickenSequenceId, 179, 2);
			_vm->_gameSys->insertSequence(_s33_nextChickenSequenceId, 179, _s33_currChickenSequenceId, 179, kSeqSyncWait, 0, 0, 0);
			_s33_currChickenSequenceId = _s33_nextChickenSequenceId;
			_s33_nextChickenSequenceId = -1;
		}
	}
}

} // End of namespace Gnap
