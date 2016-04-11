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

int GnapEngine::scene40_init() {
	_gameSys->setAnimation(0, 0, 0);
	_gameSys->setAnimation(0, 0, 1);
	return isFlag(kGFUnk23) ? 0x01 : 0x00;
}

void GnapEngine::scene40_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_DISABLED | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSExitCave, 169, 510, 264, 600, SF_EXIT_D_CURSOR, 0, 8);
	setHotspot(kHSExitToyStand, 238, 297, 328, 376, SF_EXIT_L_CURSOR, 0, 8);
	setHotspot(kHSExitBBQ, 328, 220, 401, 306, SF_EXIT_L_CURSOR, 0, 8);
	setHotspot(kHSExitUfo, 421, 215, 501, 282, SF_EXIT_U_CURSOR, 0, 8);
	setHotspot(kHSExitKissinBooth, 476, 284, 556, 345, SF_EXIT_R_CURSOR, 0, 8);
	setHotspot(kHSExitDancefloor, 317, 455, 445, 600, SF_EXIT_D_CURSOR, 0, 8);
	setHotspot(kHSExitShoe, 455, 346, 549, 417, SF_EXIT_D_CURSOR, 0, 8);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_hotspotsCount = 9;
}

void GnapEngine::scene40_run() {

	queueInsertDeviceIcon();
	endSceneInit();

	while (!_sceneDone) {

		if (!isSoundPlaying(0x1094B))
			playSound(0x1094B, true);

		updateMouseCursor();
		updateCursorByHotspot();
	
		testWalk(0, 0, -1, -1, -1, -1);

		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);
	
		switch (_sceneClickedHotspot) {

		case kHSDevice:
			runMenu();
			scene40_updateHotspots();
			break;
			
		case kHSPlatypus:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapImpossible(_platX, _platY);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapMoan1(_platX, _platY);
						break;
					case GRAB_CURSOR:
						gnapKissPlatypus(0);
						break;
					case TALK_CURSOR:
						playGnapBrainPulsating(_platX, _platY);
						playBeaverSequence(getBeaverSequenceId(0, 0, 0) | 0x10000);
						break;
					case PLAT_CURSOR:
						playGnapImpossible(_platX, _platY);
						break;
					}
				}
			}
			break;
	
		case kHSExitCave:
			if (_gnapActionStatus < 0) {
				_newSceneNum = 39;
				_sceneDone = true;
			}
			break;
	
		case kHSExitToyStand:
			if (_gnapActionStatus < 0) {
				_newSceneNum = 41;
				_sceneDone = true;
			}
			break;
	
		case kHSExitBBQ:
			if (_gnapActionStatus < 0) {
				_newSceneNum = 42;
				_sceneDone = true;
			}
			break;
	
		case kHSExitUfo:
			if (_gnapActionStatus < 0) {
				_newSceneNum = 43;
				_sceneDone = true;
			}
			break;
	
		case kHSExitKissinBooth:
			if (_gnapActionStatus < 0) {
				_newSceneNum = 44;
				_sceneDone = true;
			}
			break;
	
		case kHSExitDancefloor:
			if (_gnapActionStatus < 0) {
				_newSceneNum = 45;
				_sceneDone = true;
			}
			break;
	
		case kHSExitShoe:
			if (_gnapActionStatus < 0) {
				_newSceneNum = 46;
				_sceneDone = true;
			}
			break;
	
		default:
			if (_mouseClickState._left && _gnapActionStatus < 0)
				_mouseClickState._left = false;
			break;
		
		}
	
		scene40_updateAnimations();
		
		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene40_updateHotspots();
		}
		
		gameUpdateTick();
	
	}

}

void GnapEngine::scene40_updateAnimations() {
	
	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		if (_gnapActionStatus)
			_gnapActionStatus = -1;
		else
			_sceneDone = true;
	}

}

} // End of namespace Gnap
