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

#include "gnap/scenes/scene14.h"

namespace Gnap {

enum {
	kHSPlatypus		= 0,
	kHSExit			= 1,
	kHSCoin			= 2,
	kHSToilet		= 3,
	kHSDevice		= 4
};

Scene14::Scene14(GnapEngine *vm) : Scene(vm) {
}

int Scene14::init() {
	_vm->_gameSys->setAnimation(0, 0, 0);
	_vm->_gameSys->setAnimation(0, 0, 1);
	return 0x27;
}

void Scene14::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0);
	_vm->setHotspot(kHSExit, 0, 590, 799, 599, SF_EXIT_D_CURSOR);
	_vm->setHotspot(kHSCoin, 330, 390, 375, 440, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSToilet, 225, 250, 510, 500, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	if (_vm->isFlag(kGFNeedleTaken))
		_vm->_hotspots[kHSCoin]._flags = SF_DISABLED;
	_vm->_hotspotsCount = 5;
}

void Scene14::run() {
	_vm->_largeSprite = nullptr;

	_vm->queueInsertDeviceIcon();
	
	if (!_vm->isFlag(kGFNeedleTaken))
		_vm->_gameSys->insertSequence(0x23, 10, 0, 0, kSeqNone, 0, 0, 0);
	
	_vm->endSceneInit();
	
	if (!_vm->isFlag(kGFNeedleTaken) && _vm->invHas(kItemTongs))
		_vm->_largeSprite = _vm->_gameSys->createSurface(1);

	if (!_vm->isFlag(kGFNeedleTaken)) {
		_vm->_gameSys->insertSequence(0x24, 10, 0x23, 10, kSeqSyncWait, 0, 0, 0);
		_vm->_gnapSequenceId = 0x24;
		_vm->_timers[2] = _vm->getRandom(40) + 50;
	}
	
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

		case kHSExit:
			_vm->_sceneDone = true;
			_vm->_newSceneNum = 13;
			break;

		case kHSCoin:
			if (_vm->_grabCursorSpriteIndex == kItemTongs) {
				_vm->invAdd(kItemQuarter);
				_vm->setFlag(kGFNeedleTaken);
				_vm->setGrabCursorSprite(-1);
				_vm->hideCursor();
				_vm->_gameSys->setAnimation(0x26, 10, 0);
				_vm->_gameSys->insertSequence(0x26, 10, _vm->_gnapSequenceId, 10, kSeqSyncWait, 0, 0, 0);
			} else if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playSound(0x108E9, false);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->playSound(0x108E9, false);
					break;
				case GRAB_CURSOR:
					_vm->_gameSys->insertSequence(0x25, 10, _vm->_gnapSequenceId, 10, kSeqSyncWait, 0, 0, 0);
					_vm->_gameSys->insertSequence(0x23, 10, 0x25, 10, kSeqSyncWait, 0, 0, 0);
					_vm->_gnapSequenceId = 0x23;
					break;
				case TALK_CURSOR:
					_vm->playSound((_vm->getRandom(5) + 0x8D5) | 0x10000, false);
					break;
				case PLAT_CURSOR:
					_vm->_gameSys->insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - _vm->_gnapGridX, 576 - _vm->_gnapGridY);
					break;
				}
			}
			break;

		case kHSToilet:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->_gameSys->insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - _vm->_gnapGridX, 576 - _vm->_gnapGridY);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
				case GRAB_CURSOR:
					_vm->playSound(0x108B1, false);
					break;
				case TALK_CURSOR:
					_vm->playSound((_vm->getRandom(5) + 0x8D5) | 0x10000, false);
					break;
				case PLAT_CURSOR:
					_vm->_gameSys->insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - _vm->_gnapGridX, 576 - _vm->_gnapGridY);
					break;
				}
			}
			break;

		default:
			_vm->_mouseClickState._left = false;
			break;
		}
	
		updateAnimations();
		_vm->checkGameKeys();
	
		if (!_vm->isFlag(kGFNeedleTaken) && !_vm->_timers[2]) {
			_vm->_gameSys->insertSequence(0x24, 10, _vm->_gnapSequenceId, 10, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceId = 0x24;
			_vm->_timers[2] = _vm->getRandom(40) + 50;
		}
	
		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
		}
		
		_vm->gameUpdateTick();
	}
	
	if (_vm->_largeSprite)
		_vm->deleteSurface(&_vm->_largeSprite);
}

void Scene14::updateAnimations() {	
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		_vm->_gameSys->insertSpriteDrawItem(_vm->_largeSprite, 0, 0, 300);
		_vm->_gameSys->setAnimation(0x10843, 301, 1);
		_vm->_gameSys->insertSequence(0x10843, 301, 0x26, 10, kSeqSyncWait, 0, 0, 0);
	}
	
	if (_vm->_gameSys->getAnimationStatus(1) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 1);
		_vm->_sceneDone = true;
		_vm->_newSceneNum = 13;
		_vm->_grabCursorSpriteIndex = kItemQuarter;
	}
}

} // End of namespace Gnap
