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
#include "gnap/scenes/scene32.h"

namespace Gnap {

enum {
	kHSPlatypus		= 0,
	kHSExitTruck	= 1,
	kHSDevice		= 2,
	kHSWalkArea1	= 3,
	kHSWalkArea2	= 4,
	kHSWalkArea3	= 5,
	kHSWalkArea4	= 6,
	kHSWalkArea5	= 7,
	kHSWalkArea6	= 8,
	kHSWalkArea7	= 9,
	kHSWalkArea8	= 10
};

enum {
	kASLeaveScene					= 0
};

Scene32::Scene32(GnapEngine *vm) : Scene(vm) {

}

int Scene32::init() {
	_vm->_gameSys->setAnimation(0, 0, 0);
	return _vm->isFlag(kGFPlatypusTalkingToAssistant) ? 0xF : 0x10;
}

void Scene32::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSExitTruck, 780, 226, 800, 455, SF_EXIT_R_CURSOR | SF_WALKABLE, 10, 6);
	_vm->setHotspot(kHSWalkArea1, 0, 0, 162, 426);
	_vm->setHotspot(kHSWalkArea2, 162, 0, 237, 396);
	_vm->setHotspot(kHSWalkArea3, 237, 0, 319, 363);
	_vm->setHotspot(kHSWalkArea4, 520, 0, 800, 404);
	_vm->setHotspot(kHSWalkArea5, 300, 447, 800, 600);
	_vm->setHotspot(kHSWalkArea6, 678, 0, 800, 404);
	_vm->setHotspot(kHSWalkArea7, 0, 0, 520, 351);
	_vm->setHotspot(kHSWalkArea8, 0, 546, 300, 600);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_vm->_hotspotsCount = 11;
}

void Scene32::run() {
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

		case kHSExitTruck:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->setGrabCursorSprite(-1);
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitTruck].x, _vm->_hotspotsWalkPos[kHSExitTruck].y, 0, 0x107AB, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitTruck].x, _vm->_hotspotsWalkPos[kHSExitTruck].y + 1, -1, 0x107CD, 1);
				_vm->_newSceneNum = 33;
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
					_vm->_gameSys->insertSequence(0x0E, 180, 0, 0, kSeqNone, 0, 0, 0);
				else
					_vm->_gameSys->insertSequence(0x0D, 180, 0, 0, kSeqNone, 0, 0, 0);
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
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		if (_vm->_gnapActionStatus == kASLeaveScene)
			_vm->_sceneDone = true;
	}
}

} // End of namespace Gnap
