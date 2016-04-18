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
	kHSPlatypus				= 0,
	kHSExitCircusWorld		= 1,
	kHSExitOutsideGrubCity	= 2,
	kHSDevice				= 3,
	kHSWalkArea1			= 4,
	kHSWalkArea2			= 5,
	kHSWalkArea3			= 6
};

enum {
	kASLeaveScene			= 0
};

int GnapEngine::scene24_init() {
	return 0x3B;
}

void GnapEngine::scene24_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSExitCircusWorld, 785, 128, 800, 600, SF_EXIT_R_CURSOR, 8, 7);
	setHotspot(kHSExitOutsideGrubCity, 0, 213, 91, 600, SF_EXIT_NW_CURSOR, 1, 8);
	setHotspot(kHSWalkArea1, 0, 0, 0, 0);
	setHotspot(kHSWalkArea2, 530, 0, 800, 600);
	setHotspot(kHSWalkArea3, 0, 0, 800, 517);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_hotspotsCount = 7;
}

void GnapEngine::scene24_run() {
	int counter = 0;
	
	playSound(0x10940, true);
	
	startSoundTimerA(9);
	
	_timers[7] = getRandom(100) + 100;
	
	_gameSys->insertSequence(0x2F, 256, 0, 0, kSeqNone, 0, 0, 0);
	
	_timers[4] = getRandom(20) + 50;
	_timers[5] = getRandom(20) + 40;
	_timers[6] = getRandom(50) + 30;
	
	_gameSys->insertSequence(0x36, 20, 0, 0, kSeqNone, 0, 0, 0);
	_gameSys->insertSequence(0x30, 20, 0, 0, kSeqNone, 0, 0, 0);
	_gameSys->insertSequence(0x35, 20, 0, 0, kSeqNone, 0, 0, 0);
	
	_s24_currWomanSequenceId = 0x35;
	_s24_girlSequenceId = 0x36;
	_s24_boySequenceId = 0x30;
	
	if (_debugLevel == 4)
		startIdleTimer(8);
	
	queueInsertDeviceIcon();
	
	if (_prevSceneNum == 20) {
		initGnapPos(1, 8, kDirBottomRight);
		initPlatypusPos(2, 8, kDirNone);
		endSceneInit();
		gnapWalkTo(1, 9, -1, 0x107B9, 1);
		platypusWalkTo(2, 9, -1, 0x107C2, 1);
	} else {
		initGnapPos(8, 8, kDirBottomLeft);
		initPlatypusPos(8, 8, kDirUnk4);
		endSceneInit();
		gnapWalkTo(2, 8, -1, 0x107BA, 1);
		platypusWalkTo(3, 8, -1, 0x107C2, 1);
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
				scene24_updateHotspots();
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
						playGnapScratchingHead(_platX, _platY);
						break;
					case GRAB_CURSOR:
						gnapKissPlatypus(0);
						break;
					case TALK_CURSOR:
						playGnapBrainPulsating(_platX, _platY);
						playPlatypusSequence(getPlatypusSequenceId());
						break;
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;
		
		case kHSExitCircusWorld:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = true;
				_newSceneNum = 25;
				gnapWalkTo(_hotspotsWalkPos[kHSExitCircusWorld].x, _hotspotsWalkPos[kHSExitCircusWorld].y, 0, 0x107AB, 1);
				_gnapActionStatus = kASLeaveScene;
				platypusWalkTo(_hotspotsWalkPos[kHSExitCircusWorld].x + 1, _hotspotsWalkPos[kHSExitCircusWorld].y, -1, 0x107C2, 1);
			}
			break;
		
		case kHSExitOutsideGrubCity:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = true;
				_newSceneNum = 20;
				_gnapIdleFacing = kDirUpRight;
				gnapWalkTo(_hotspotsWalkPos[kHSExitOutsideGrubCity].x, _hotspotsWalkPos[kHSExitOutsideGrubCity].y, 0, getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
				_gnapActionStatus = kASLeaveScene;
				platypusWalkTo(_hotspotsWalkPos[kHSExitOutsideGrubCity].x + 1, _hotspotsWalkPos[kHSExitOutsideGrubCity].y, -1, 0x107C2, 1);
			}
			break;
		
		case kHSWalkArea1:
		case kHSWalkArea2:
		case kHSWalkArea3:
			if (_gnapActionStatus == -1)
				gnapWalkTo(-1, -1, -1, -1, 1);
			break;
		
		default:
			if (_mouseClickState._left) {
				gnapWalkTo(-1, -1, -1, -1, 1);
				_mouseClickState._left = false;
			}
			break;
		
		}
	
		scene24_updateAnimations();
	
		if (!isSoundPlaying(0x10940))
			playSound(0x10940, true);
	
		if (!_isLeavingScene) {
			updatePlatypusIdleSequence();
			updateGnapIdleSequence();
			if (!_timers[4]) {
				_timers[4] = getRandom(20) + 50;
				_gameSys->insertSequence(0x37, 20, _s24_girlSequenceId, 20, kSeqSyncWait, 0, 0, 0);
				_s24_girlSequenceId = 0x37;
			}
			if (!_timers[5]) {
				_timers[5] = getRandom(20) + 40;
				_gameSys->insertSequence(0x31, 20, _s24_boySequenceId, 20, kSeqSyncWait, 0, 0, 0);
				_s24_boySequenceId = 0x31;
			}
			if (!_timers[6]) {
				_timers[6] = getRandom(50) + 30;
				counter = (counter + 1) % 3;
				switch (counter) {
				case 0:
					_s24_nextWomanSequenceId = 0x32;
					break;
				case 1:
					_s24_nextWomanSequenceId = 0x33;
					break;
				case 2:
					_s24_nextWomanSequenceId = 0x34;
					break;
				}
				_gameSys->insertSequence(_s24_nextWomanSequenceId, 20, _s24_currWomanSequenceId, 20, kSeqSyncWait, 0, 0, 0);
				_s24_currWomanSequenceId = _s24_nextWomanSequenceId;
			}
			if (!_timers[7]) {
				_timers[7] = getRandom(100) + 100;
				switch (getRandom(3)) {
				case 0:
					_gameSys->insertSequence(0x38, 253, 0, 0, kSeqNone, 0, 0, 0);
					break;
				case 1:
					_gameSys->insertSequence(0x39, 253, 0, 0, kSeqNone, 0, 0, 0);
					break;
				case 2:
					_gameSys->insertSequence(0x3A, 253, 0, 0, kSeqNone, 0, 0, 0);
					break;
				}
			}
			if (_debugLevel == 4)
				updateIdleTimer();
			playSoundA();
		}
	
		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene24_updateHotspots();
		}
		
		gameUpdateTick();
	
	}

}

void GnapEngine::scene24_updateAnimations() {

	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		if (_gnapActionStatus == kASLeaveScene)
			_sceneDone = true;
		_gnapActionStatus = -1;
	}

}

} // End of namespace Gnap
