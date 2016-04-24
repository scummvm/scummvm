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
#include "gnap/scenes/scene40.h"

namespace Gnap {

enum {
	kHSPlatypus			= 0,
	kHSExitCave			= 1,
	kHSExitToyStand		= 2,
	kHSExitBBQ			= 3,
	kHSExitUfo			= 4,
	kHSExitKissinBooth	= 5,
	kHSExitDancefloor	= 6,
	kHSExitShoe			= 7,
	kHSDevice			= 8
};

Scene40::Scene40(GnapEngine *vm) : Scene(vm) {
}

int Scene40::init() {
	_vm->_gameSys->setAnimation(0, 0, 0);
	_vm->_gameSys->setAnimation(0, 0, 1);
	return _vm->isFlag(kGFUnk23) ? 0x01 : 0x00;
}

void Scene40::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_DISABLED | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSExitCave, 169, 510, 264, 600, SF_EXIT_D_CURSOR, 0, 8);
	_vm->setHotspot(kHSExitToyStand, 238, 297, 328, 376, SF_EXIT_L_CURSOR, 0, 8);
	_vm->setHotspot(kHSExitBBQ, 328, 220, 401, 306, SF_EXIT_L_CURSOR, 0, 8);
	_vm->setHotspot(kHSExitUfo, 421, 215, 501, 282, SF_EXIT_U_CURSOR, 0, 8);
	_vm->setHotspot(kHSExitKissinBooth, 476, 284, 556, 345, SF_EXIT_R_CURSOR, 0, 8);
	_vm->setHotspot(kHSExitDancefloor, 317, 455, 445, 600, SF_EXIT_D_CURSOR, 0, 8);
	_vm->setHotspot(kHSExitShoe, 455, 346, 549, 417, SF_EXIT_D_CURSOR, 0, 8);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_vm->_hotspotsCount = 9;
}

void Scene40::run() {
	_vm->queueInsertDeviceIcon();
	_vm->endSceneInit();

	while (!_vm->_sceneDone) {
		if (!_vm->isSoundPlaying(0x1094B))
			_vm->playSound(0x1094B, true);

		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
	
		_vm->testWalk(0, 0, -1, -1, -1, -1);

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);
	
		switch (_vm->_sceneClickedHotspot) {
		case kHSDevice:
			_vm->runMenu();
			updateHotspots();
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
	
		case kHSExitCave:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_newSceneNum = 39;
				_vm->_sceneDone = true;
			}
			break;
	
		case kHSExitToyStand:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_newSceneNum = 41;
				_vm->_sceneDone = true;
			}
			break;
	
		case kHSExitBBQ:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_newSceneNum = 42;
				_vm->_sceneDone = true;
			}
			break;
	
		case kHSExitUfo:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_newSceneNum = 43;
				_vm->_sceneDone = true;
			}
			break;
	
		case kHSExitKissinBooth:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_newSceneNum = 44;
				_vm->_sceneDone = true;
			}
			break;
	
		case kHSExitDancefloor:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_newSceneNum = 45;
				_vm->_sceneDone = true;
			}
			break;
	
		case kHSExitShoe:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_newSceneNum = 46;
				_vm->_sceneDone = true;
			}
			break;
	
		default:
			if (_vm->_mouseClickState._left && _vm->_gnapActionStatus < 0)
				_vm->_mouseClickState._left = false;
			break;
		
		}
	
		updateAnimations();
		_vm->checkGameKeys();
	
		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
		}
		
		_vm->gameUpdateTick();
	}
}

void Scene40::updateAnimations() {
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		if (_vm->_gnapActionStatus)
			_vm->_gnapActionStatus = -1;
		else
			_vm->_sceneDone = true;
	}
}

} // End of namespace Gnap
