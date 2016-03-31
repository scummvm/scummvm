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
	kHSExitFrontGrubCity	= 1,
	kHSDevice				= 2,
	kHSCereals				= 3,
	kHSWalkArea1			= 4,
	kHSWalkArea2			= 5
};

enum {
	kASLookCereals			= 0,
	kASGrabCereals			= 1,
	kASGrabCerealsDone		= 2,
	kASLeaveScene			= 3
};

int GnapEngine::scene23_init() {
	return 0xC0;
}

void GnapEngine::scene23_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSExitFrontGrubCity, 0, 250, 15, 550, SF_EXIT_L_CURSOR | SF_WALKABLE, 0, 7);
	setHotspot(kHSCereals, 366, 332, 414, 408, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 4, 7);
	setHotspot(kHSWalkArea1, 0, 0, 340, 460);
	setHotspot(kHSWalkArea2, 340, 0, 800, 501);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_hotspotsCount = 6;
}

void GnapEngine::scene23_run() {
	
	_timers[4] = getRandom(100) + 200;
	_timers[5] = getRandom(100) + 200;
	
	_s23_currStoreClerkSequenceId = 0xB4;
	_s23_nextStoreClerkSequenceId = -1;
	
	_gameSys->setAnimation(0xB4, 1, 4);
	_gameSys->insertSequence(_s23_currStoreClerkSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	queueInsertDeviceIcon();
	
	initGnapPos(-1, 7, kDirBottomRight);
	initBeaverPos(-2, 7, 0);
	_gameSys->insertSequence(0xBD, 255, 0, 0, kSeqNone, 0, 0, 0);
	_gameSys->insertSequence(0xBF, 2, 0, 0, kSeqNone, 0, 0, 0);
	endSceneInit();
	
	platypusWalkTo(1, 7, -1, 0x107C2, 1);

	if (isFlag(24)) {
		gnapWalkTo(2, 7, -1, 0x107B9, 1);
	} else {
		gnapWalkTo(2, 7, 0, 0x107B9, 1);
		while (_gameSys->getAnimationStatus(0) != 2)
			gameUpdateTick();
		playSequences(0x48, 0xBA, 0xBB, 0xBC);
		setFlag(24);
	}
	
	while (!_sceneDone) {
	
		updateMouseCursor();
		updateCursorByHotspot();
	
		testWalk(0, 3, -1, -1, -1, -1);
	
		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);
	
		switch (_sceneClickedHotspot) {

		case kHSDevice:
			if (_gnapActionStatus < 0) {
				runMenu();
				scene23_updateHotspots();
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
						playBeaverSequence(getBeaverSequenceId(0, 0, 0) | 0x10000);
						break;
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;
		
		case kHSCereals:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[kHSCereals].x, _hotspotsWalkPos[kHSCereals].y, 5, 4);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						if (isFlag(11))
							playGnapMoan2(0, 0);
						else {
							gnapWalkTo(_hotspotsWalkPos[kHSCereals].x, _hotspotsWalkPos[kHSCereals].y,
								0, getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
							_gnapActionStatus = kASLookCereals;
						}
						break;
					case GRAB_CURSOR:
						if (isFlag(11))
							playGnapImpossible(0, 0);
						else {
							_gnapIdleFacing = kDirBottomRight;
							gnapWalkTo(_hotspotsWalkPos[kHSCereals].x, _hotspotsWalkPos[kHSCereals].y,
								0, getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
							setFlag(11);
							_gnapActionStatus = kASGrabCereals;
							invAdd(kItemCereals);
						}
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;
		
		case kHSExitFrontGrubCity:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = true;
				_newSceneNum = 22;
				gnapWalkTo(_hotspotsWalkPos[kHSExitFrontGrubCity].x, _hotspotsWalkPos[kHSExitFrontGrubCity].y, 0, 0x107AF, 1);
				_gnapActionStatus = kASLeaveScene;
				platypusWalkTo(_hotspotsWalkPos[kHSExitFrontGrubCity].x, _hotspotsWalkPos[kHSExitFrontGrubCity].y - 1, -1, 0x107C2, 1);
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
	
		scene23_updateAnimations();
	
		if (!_isLeavingScene) {
			updateBeaverIdleSequence();
			updateGnapIdleSequence();
			if (!_timers[4] && _gnapActionStatus == -1) {
				_timers[4] = getRandom(100) + 200;
				switch (getRandom(4)) {
				case 0:
					_gameSys->insertSequence(0xB7, 256, 0, 0, kSeqNone, 0, 0, 0);
					break;
				case 1:
					_gameSys->insertSequence(0xB8, 256, 0, 0, kSeqNone, 0, 0, 0);
					break;
				case 2:
				case 3:
					_gameSys->insertSequence(0xB9, 256, 0, 0, kSeqNone, 0, 0, 0);
					break;
				}
			}
			if (!_timers[5]) {
				_timers[5] = getRandom(100) + 200;
				switch (getRandom(3)) {
				case 0:
					playSound(0xCE, 0);
					break;
				case 1:
					playSound(0xD0, 0);
					break;
				case 2:
					playSound(0xCF, 0);
					break;
				}
			}
		}
	
		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene23_updateHotspots();
		}
		
		gameUpdateTick();
	
	}

}

void GnapEngine::scene23_updateAnimations() {

	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		switch (_gnapActionStatus) {
		case kASLookCereals:
			showFullScreenSprite(0x48);
			_gnapActionStatus = -1;
			break;
		case kASGrabCereals:
			_gameSys->setAnimation(0xBE, _gnapId, 0);
			_gameSys->insertSequence(0xBE, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gameSys->requestRemoveSequence(0xBF, 2);
			_gnapSequenceDatNum = 0;
			_gnapSequenceId = 0xBE;
			_gnapActionStatus = kASGrabCerealsDone;
			break;
		case kASGrabCerealsDone:
			setGrabCursorSprite(kItemCereals);
			_gnapActionStatus = -1;
			break;
		case kASLeaveScene:
			_sceneDone = true;
			break;
		}
	}
	
	if (_gameSys->getAnimationStatus(4) == 2 && _s23_nextStoreClerkSequenceId == -1) {
		switch (getRandom(8)) {
		case 0:
		case 1:
		case 2:
			_s23_nextStoreClerkSequenceId = 0xB4;
			break;
		case 3:
		case 4:
		case 5:
			_s23_nextStoreClerkSequenceId = 0xB5;
			break;
		default:
			_s23_nextStoreClerkSequenceId = 0xB6;
			break;
		}
		_gameSys->setAnimation(_s23_nextStoreClerkSequenceId, 1, 4);
		_gameSys->insertSequence(_s23_nextStoreClerkSequenceId, 1, _s23_currStoreClerkSequenceId, 1, kSeqSyncWait, 0, 0, 0);
		_s23_currStoreClerkSequenceId = _s23_nextStoreClerkSequenceId;
		_s23_nextStoreClerkSequenceId = -1;
	}

}

} // End of namespace Gnap
