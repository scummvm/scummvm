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
	kHSPlatypus					= 0,
	kHSExitOutsideCircusWorld	= 1,
	kHSExitOutsideClown			= 2,
	kHSExitArcade				= 3,
	kHSExitElephant				= 4,
	kHSExitBeerStand			= 5,
	kHSDevice					= 6,
	kHSWalkArea1				= 7,
	kHSWalkArea2				= 8
};

enum {
	kASLeaveScene					= 0
};

int GnapEngine::scene26_init() {
	return isFlag(23) ? 0x61 : 0x60;
}

void GnapEngine::scene26_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSExitOutsideCircusWorld, 0, 590, 300, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 1, 10);
	setHotspot(kHSExitOutsideClown, 200, 265, 265, 350, SF_EXIT_U_CURSOR, 3, 8);
	setHotspot(kHSExitArcade, 0, 295, 150, 400, SF_EXIT_NW_CURSOR, 2, 8);
	setHotspot(kHSExitElephant, 270, 290, 485, 375, SF_EXIT_U_CURSOR, 5, 8);
	setHotspot(kHSExitBeerStand, 530, 290, 620, 350, SF_EXIT_NE_CURSOR, 5, 8);
	setHotspot(kHSWalkArea1, 0, 0, 800, 500);
	setHotspot(kHSWalkArea2, 281, 0, 800, 600);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_hotspotsCount = 9;
}

void GnapEngine::scene26_run() {
	
	startSoundTimerB(7);
	playSound(0x1093B, 1);

	_s26_currKidSequenceId = 0x5B;
	_s26_nextKidSequenceId = -1;
	_gameSys->setAnimation(0x5B, 160, 3);
	_gameSys->insertSequence(_s26_currKidSequenceId, 160, 0, 0, kSeqNone, 0, 0, 0);

	_timers[5] = getRandom(20) + 50;
	_timers[4] = getRandom(20) + 50;
	_timers[6] = getRandom(50) + 100;

	queueInsertDeviceIcon();

	_gameSys->insertSequence(0x58, 40, 0, 0, kSeqLoop, 0, 0, 0);
	_gameSys->insertSequence(0x5C, 40, 0, 0, kSeqLoop, 0, 0, 0);
	_gameSys->insertSequence(0x5D, 40, 0, 0, kSeqLoop, 0, 0, 0);
	_gameSys->insertSequence(0x5E, 40, 0, 0, kSeqLoop, 0, 0, 0);
	
	if (_prevSceneNum == 25) {
		initGnapPos(-1, 8, kDirBottomRight);
		initBeaverPos(-2, 8, 0);
		endSceneInit();
		gnapWalkTo(2, 8, -1, 0x107B9, 1);
		platypusWalkTo(1, 8, -1, 0x107C2, 1);
	} else {
		initGnapPos(2, 8, kDirBottomRight);
		initBeaverPos(3, 8, 0);
		endSceneInit();
	}

	while (!_sceneDone) {

		updateMouseCursor();
		updateCursorByHotspot();

		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);

		switch (_sceneClickedHotspot) {

		case kHSDevice:
			if (_gnapActionStatus < 0) {
				runMenu();
				scene26_updateHotspots();
			}
			break;

		case kHSPlatypus:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex == kItemJoint) {
					gnapUseJointOnPlatypus();
				} else if (_grabCursorSpriteIndex >= 0) {
					playGnapImpossible(0, 0);
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
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSExitOutsideCircusWorld:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = true;
				_newSceneNum = 25;
				gnapWalkTo(-1, _hotspotsWalkPos[kHSExitOutsideCircusWorld].y, 0, 0x107AE, 1);
				_gnapActionStatus = kASLeaveScene;
			}
			break;

		case kHSExitOutsideClown:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = true;
				_newSceneNum = 27;
				gnapWalkTo(-1, _hotspotsWalkPos[kHSExitOutsideClown].y, 0, 0x107BC, 1);
				_gnapActionStatus = kASLeaveScene;
			}
			break;

		case kHSExitArcade:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = true;
				_newSceneNum = 29;
				gnapWalkTo(-1, _hotspotsWalkPos[kHSExitArcade].y, 0, 0x107BC, 1);
				_gnapActionStatus = kASLeaveScene;
			}
			break;

		case kHSExitElephant:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = true;
				_newSceneNum = 30;
				gnapWalkTo(-1, _hotspotsWalkPos[4].y, 0, 0x107BC, 1);
				_gnapActionStatus = kASLeaveScene;
			}
			break;

		case kHSExitBeerStand:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = true;
				_newSceneNum = 31;
				gnapWalkTo(-1, _hotspotsWalkPos[kHSExitBeerStand].y, 0, 0x107BB, 1);
				_gnapActionStatus = kASLeaveScene;
			}
			break;

		case kHSWalkArea1:
		case kHSWalkArea2:
			if (_gnapActionStatus < 0)
				gnapWalkTo(-1, -1, -1, -1, 1);
			break;
			
		default:
			if (_mouseClickState._left) {
				gnapWalkTo(-1, -1, -1, -1, 1);
				_mouseClickState._left = false;
			}
			break;
			
		}

		scene26_updateAnimations();

		if (!isSoundPlaying(0x1093B))
			playSound(0x1093B, 1);

		if (!_isLeavingScene) {
			updateBeaverIdleSequence();
			updateGnapIdleSequence();
			if (!_timers[5] && _s26_nextKidSequenceId == -1) {
				_timers[5] = getRandom(20) + 50;
				if (getRandom(5) != 0)
					_s26_nextKidSequenceId = 0x5B;
				else
					_s26_nextKidSequenceId = 0x5A;
			}
			if (!_timers[4]) {
				_timers[4] = getRandom(20) + 130;
				_gameSys->insertSequence(0x59, 40, 0, 0, kSeqNone, 0, 0, 0);
			}
			if (!_timers[6]) {
				_timers[6] = getRandom(50) + 100;
				_gameSys->insertSequence(0x5F, 40, 0, 0, kSeqNone, 0, 0, 0);
			}
			playSoundB();
		}

		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene26_updateHotspots();
		}
		
		gameUpdateTick();

	}
	
}

void GnapEngine::scene26_updateAnimations() {

	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		if (_gnapActionStatus == kASLeaveScene)
			_sceneDone = true;
		_gnapActionStatus = -1;
	}

	if (_gameSys->getAnimationStatus(3) == 2 && _s26_nextKidSequenceId != -1) {
		_gameSys->insertSequence(_s26_nextKidSequenceId, 160, _s26_currKidSequenceId, 160, kSeqSyncWait, 0, 0, 0);
		_gameSys->setAnimation(_s26_nextKidSequenceId, 160, 3);
		_s26_currKidSequenceId = _s26_nextKidSequenceId;
		_s26_nextKidSequenceId = -1;
	}
	
}

} // End of namespace Gnap
