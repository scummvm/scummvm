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
	kHSExitInsideHouse	= 1,
	kHSExitUfoParty		= 2,
	kHSSign				= 3,
	kHSDevice			= 4,
	kHSWalkArea1		= 5,
	kHSWalkArea2		= 6
};

enum {
	kASLeaveScene				= 0
};

int GnapEngine::scene39_init() {
	_gameSys->setAnimation(0, 0, 0);
	_gameSys->setAnimation(0, 0, 1);
	return 0x35;
}

void GnapEngine::scene39_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSExitInsideHouse, 0, 0, 140, 206, SF_EXIT_U_CURSOR, 4, 8);
	setHotspot(kHSExitUfoParty, 360, 204, 480, 430, SF_EXIT_R_CURSOR, 6, 8);
	setHotspot(kHSSign, 528, 232, 607, 397, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 7, 3);
	setHotspot(kHSWalkArea1, 0, 0, 800, 466);
	setHotspot(kHSWalkArea2, 502, 466, 800, 600);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_hotspotsCount = 7;
}

void GnapEngine::scene39_run() {

	_timers[5] = 0; // Bug in the original? Timer was never intiailized.
	
	queueInsertDeviceIcon();
	_s39_currGuySequenceId = 0x33;

	_gameSys->setAnimation(0x33, 21, 3);
	_gameSys->insertSequence(_s39_currGuySequenceId, 21, 0, 0, kSeqNone, 0, 0, 0);
	_gameSys->insertSequence(0x34, 21, 0, 0, kSeqLoop, 0, 0, 0);

	_s39_nextGuySequenceId = -1;
	if (_prevSceneNum == 38) {
		initGnapPos(3, 7, kDirUpRight);
		initPlatypusPos(2, 7, kDirUpLeft);
		endSceneInit();
	} else {
		initGnapPos(4, 7, kDirBottomRight);
		initPlatypusPos(5, 7, kDirNone);
		endSceneInit();
	}
	
	while (!_sceneDone) {

		if (!isSoundPlaying(0x1094B)) {
			playSound(0x1094B, true);
			setSoundVolume(0x1094B, 60);
		}
		
		updateMouseCursor();
		updateCursorByHotspot();
	
		testWalk(0, 0, -1, -1, -1, -1);

		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);

		switch (_sceneClickedHotspot) {

		case kHSDevice:
			runMenu();
			scene39_updateHotspots();
			_timers[5] = getRandom(20) + 50;
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
						playPlatypusSequence(getPlatypusSequenceId());
						break;
					case PLAT_CURSOR:
						playGnapImpossible(_platX, _platY);
						break;
					}
				}
			}
			break;
	
		case kHSExitUfoParty:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = true;
				_sceneDone = true;
				gnapWalkTo(_gnapX, _gnapY, 0, 0x107AB, 1);
				_gnapActionStatus = kASLeaveScene;
				_newSceneNum = 40;
			}
			break;
	
		case kHSSign:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapImpossible(0, 0);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						gnapWalkTo(_hotspotsWalkPos[kHSSign].x, _hotspotsWalkPos[kHSSign].y, 0, -1, 1);
						playGnapIdle(_hotspotsWalkPos[kHSSign].x, _hotspotsWalkPos[kHSSign].y);
						showFullScreenSprite(0x1C);
						break;
					case GRAB_CURSOR:
					case TALK_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;
	
		case kHSExitInsideHouse:
			if (_gnapActionStatus < 0) {
				_sceneDone = true;
				_isLeavingScene = true;
				_newSceneNum = 38;
			}
			break;
	
		case kHSWalkArea1:
		case kHSWalkArea2:
			if (_gnapActionStatus < 0)
				gnapWalkTo(-1, -1, -1, -1, 1);
			break;
			
		default:
			if (_mouseClickState._left && _gnapActionStatus < 0) {
				gnapWalkTo(-1, -1, -1, -1, 1);
				_mouseClickState._left = false;
			}
			break;
			
		}
	
		scene39_updateAnimations();
	
		if (!_isLeavingScene) {
			if (_platypusActionStatus < 0)
				updatePlatypusIdleSequence();
			if (_gnapActionStatus < 0)
				updateGnapIdleSequence();
			if (!_timers[5]) {
				_timers[5] = getRandom(20) + 50;
				switch (getRandom(4)) {
				case 0:
					_s39_nextGuySequenceId = 0x30;
					break;
				case 1:
					_s39_nextGuySequenceId = 0x31;
					break;
				case 2:
					_s39_nextGuySequenceId = 0x32;
					break;
				case 3:
					_s39_nextGuySequenceId = 0x33;
					break;
				}
			}
		}
	
		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene39_updateHotspots();
			_timers[5] = getRandom(20) + 50;
		}
		
		gameUpdateTick();
	
	}

}

void GnapEngine::scene39_updateAnimations() {
	
	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		if (_gnapActionStatus == kASLeaveScene)
			_sceneDone = true;
		else
			_gnapActionStatus = -1;
	}
	
	if (_gameSys->getAnimationStatus(3) == 2 && _s39_nextGuySequenceId != -1) {
		_gameSys->setAnimation(_s39_nextGuySequenceId, 21, 3);
		_gameSys->insertSequence(_s39_nextGuySequenceId, 21, _s39_currGuySequenceId, 21, kSeqSyncWait, 0, 0, 0);
		_s39_currGuySequenceId = _s39_nextGuySequenceId;
		_s39_nextGuySequenceId = -1;
	}

}

} // End of namespace Gnap
