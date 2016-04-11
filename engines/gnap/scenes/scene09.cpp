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
	kHSExitKitchen	= 1,
	kHSExitHouse	= 2,
	kHSTrash		= 3,
	kHSDevice		= 4,
	kHSWalkArea1	= 5,
	kHSWalkArea2	= 6,
	kHSWalkArea3	= 7
};

enum {
	kASLeaveScene		= 0,
	kASSearchTrash		= 1,
	kASSearchTrashDone	= 2
};

int GnapEngine::scene09_init() {
	return 0x4E;
}

void GnapEngine::scene09_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 200, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSExitKitchen, 280, 200, 380, 400, SF_EXIT_U_CURSOR);
	setHotspot(kHSExitHouse, 790, 200, 799, 450, SF_EXIT_R_CURSOR | SF_WALKABLE);
	setHotspot(kHSTrash, 440, 310, 680, 420, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSWalkArea1, 0, 0, 799, 400);
	setHotspot(kHSWalkArea2, 0, 0, 630, 450);
	setHotspot(kHSWalkArea2, 0, 0, 175, 495);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_hotspotsCount = 8;
}

void GnapEngine::scene09_run() {
	
	queueInsertDeviceIcon();
	
	_gameSys->insertSequence(0x4D, 1, 0, 0, kSeqLoop, 0, 0, 0);
	_gameSys->insertSequence(0x4B, 2, 0, 0, kSeqNone, 0, 0, 0);
	
	if (_prevSceneNum == 8) {
		initGnapPos(11, 8, kDirBottomLeft);
		initBeaverPos(12, 7, kDirUnk4);
		endSceneInit();
		gnapWalkTo(9, 8, -1, 0x107BA, 1);
		platypusWalkTo(9, 7, -1, 0x107D2, 1);
	} else {
		initGnapPos(4, 7, kDirBottomRight);
		initBeaverPos(5, 7, kDirNone);
		endSceneInit();
	}

	_timers[4] = getRandom(150) + 50;
	_timers[5] = getRandom(40) + 50;

	while (!_sceneDone) {
	
		if (!isSoundPlaying(0x10919))
			playSound(0x10919, true);
	
		testWalk(0, 0, -1, -1, -1, -1);
	
		updateMouseCursor();
		updateCursorByHotspot();
	
		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);
	
		switch (_sceneClickedHotspot) {

		case kHSDevice:
			if (_gnapActionStatus < 0) {
				runMenu();
				scene09_updateHotspots();
				_timers[4] = getRandom(150) + 50;
				_timers[5] = getRandom(40) + 50;
			}
			break;

		case kHSPlatypus:
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
				break;
			}
			break;

		case kHSExitKitchen:
			_isLeavingScene = 1;
			_newSceneNum = 10;
			gnapWalkTo(4, 7, 0, 0x107BF, 1);
			_gnapActionStatus = kASLeaveScene;
			platypusWalkTo(4, 8, -1, 0x107D2, 1);
			_beaverFacing = kDirUnk4;
			break;

		case kHSExitHouse:
			_isLeavingScene = 1;
			_newSceneNum = 8;
			gnapWalkTo(10, -1, 0, 0x107AB, 1);
			_gnapActionStatus = kASLeaveScene;
			platypusWalkTo(10, -1, -1, 0x107CD, 1);
			_beaverFacing = kDirUnk4;
			break;

		case kHSTrash:
			if (_grabCursorSpriteIndex >= 0) {
				playGnapShowCurrItem(9, 6, 8, 0);
			} else {
				switch (_verbCursor) {
				case LOOK_CURSOR:
					playGnapScratchingHead(8, 3);
					break;
				case GRAB_CURSOR:
					_gnapActionStatus = kASSearchTrash;
					gnapWalkTo(9, 6, 0, 0x107BC, 1);
					break;
				case TALK_CURSOR:
				case PLAT_CURSOR:
					playGnapImpossible(0, 0);
					break;
				}
			}
			break;

		case kHSWalkArea1:
		case kHSWalkArea2:
		case kHSWalkArea3:
			gnapWalkTo(-1, -1, -1, -1, 1);
			break;

		default:
			if (_mouseClickState._left) {
				gnapWalkTo(-1, -1, -1, -1, 1);
				_mouseClickState._left = false;
			}
			break;

		}
	
		scene09_updateAnimations();
	
		if (!_isLeavingScene && _gnapActionStatus != 1 && _gnapActionStatus != 2) {
			updateBeaverIdleSequence();
			updateGnapIdleSequence();
			if (!_timers[4]) {
				_timers[4] = getRandom(150) + 100;
				if (_timers[4] & 1)
					_gameSys->insertSequence(0x49, 1, 0, 0, kSeqNone, 0, 0, 0);
				else
					_gameSys->insertSequence(0x4A, 1, 0, 0, kSeqNone, 0, 0, 0);
			}
			sceneXX_playRandomSound(5);
		}
	
		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene09_updateHotspots();
			_timers[4] = getRandom(150) + 50;
			_timers[5] = getRandom(40) + 50;
		}
		
		gameUpdateTick();
		
	}
  
}

void GnapEngine::scene09_updateAnimations() {

	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		switch (_gnapActionStatus) {
		case kASLeaveScene:
			_sceneDone = true;
			_gnapActionStatus = -1;
			break;
		case kASSearchTrash:
			_gameSys->setAnimation(0x4C, 120, 0);
			_gameSys->insertSequence(0x4C, 120, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gameSys->removeSequence(0x4B, 2, true);
			_gnapSequenceId = 0x4C;
			_gnapId = 120;
			_gnapIdleFacing = kDirUpLeft;
			_gnapSequenceDatNum = 0;
			_gnapX = 9;
			_gnapY = 6;
			_gnapActionStatus = kASSearchTrashDone;
			break;
		case kASSearchTrashDone:
			_gameSys->insertSequence(0x4B, 2, 0, 0, kSeqNone, 0, 0, 0);
			_timers[2] = 360;
			_timers[4] = getRandom(150) + 100;
			_gnapActionStatus = -1;
			break;
		}
	}
  
}

} // End of namespace Gnap
