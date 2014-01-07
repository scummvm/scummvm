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

int GnapEngine::scene32_init() {
	_gameSys->setAnimation(0, 0, 0);
	return isFlag(26) ? 0xF : 0x10;
}

void GnapEngine::scene32_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSExitTruck, 780, 226, 800, 455, SF_EXIT_R_CURSOR | SF_WALKABLE, 10, 6);
	setHotspot(kHSWalkArea1, 0, 0, 162, 426);
	setHotspot(kHSWalkArea2, 162, 0, 237, 396);
	setHotspot(kHSWalkArea3, 237, 0, 319, 363);
	setHotspot(kHSWalkArea4, 520, 0, 800, 404);
	setHotspot(kHSWalkArea5, 300, 447, 800, 600);
	setHotspot(kHSWalkArea6, 678, 0, 800, 404);
	setHotspot(kHSWalkArea7, 0, 0, 520, 351);
	setHotspot(kHSWalkArea8, 0, 546, 300, 600);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_hotspotsCount = 11;
}

void GnapEngine::scene32_run() {
	
	playSound(0x1091C, 1);
	startSoundTimerC(5);
	queueInsertDeviceIcon();
	_s32_dword_47EADC = -1;
	_timers[4] = getRandom(100) + 300;

	if (_prevSceneNum == 33) {
		initGnapPos(11, 6, 3);
		initBeaverPos(12, 6, 4);
		endSceneInit();
		platypusWalkTo(9, 6, -1, 0x107D2, 1);
		gnapWalkTo(8, 6, -1, 0x107BA, 1);
	} else {
		initGnapPos(1, 6, 1);
		initBeaverPos(1, 7, 0);
		endSceneInit();
	}
	
	while (!_sceneDone) {
	
		updateMouseCursor();
		updateCursorByHotspot();
		testWalk(0, 0, -1, -1, -1, -1);
		
		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);
	
		switch (_sceneClickedHotspot) {

		case kHSDevice:
			if (_gnapActionStatus < 0) {
				runMenu();
				scene32_updateHotspots();
			}
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

		case kHSExitTruck:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = 1;
				setGrabCursorSprite(-1);
				gnapWalkTo(_hotspotsWalkPos[kHSExitTruck].x, _hotspotsWalkPos[kHSExitTruck].y, 0, 0x107AB, 1);
				_gnapActionStatus = 0;
				platypusWalkTo(_hotspotsWalkPos[kHSExitTruck].x, _hotspotsWalkPos[kHSExitTruck].y + 1, -1, 0x107CD, 1);
				_newSceneNum = 33;
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
			if (_gnapActionStatus < 0)
				gnapWalkTo(-1, -1, -1, -1, 1);
			break;

		}

		if (_mouseClickState.left && _gnapActionStatus < 0) {
			gnapWalkTo(-1, -1, -1, -1, 1);
			_mouseClickState.left = 0;
		}

		scene32_updateAnimations();
		
		if (!isSoundPlaying(0x1091C))
			playSound(0x1091C, 1);
	
		if (!_isLeavingScene) {
			if (_beaverActionStatus < 0)
				updateBeaverIdleSequence();
			if (_gnapActionStatus < 0)
				updateGnapIdleSequence();
			if (!_timers[4]) {
				_timers[4] = getRandom(100) + 300;
				if (getRandom(2) != 0)
					_s32_dword_47EADC = 14;
				else
					_s32_dword_47EADC = 13;
				_gameSys->insertSequence(_s32_dword_47EADC, 180, 0, 0, kSeqNone, 0, 0, 0);
			}
			playSoundC();
		}
	
		checkGameKeys();
		
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene32_updateHotspots();
		}
		
		gameUpdateTick();
	
	}
	
}

void GnapEngine::scene32_updateAnimations() {

	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		if (_gnapActionStatus == 0)
			_sceneDone = true;
	}

}

} // End of namespace Gnap
					