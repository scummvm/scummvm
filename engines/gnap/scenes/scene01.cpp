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
	kHSMud			= 2,
	kHSPigs			= 3,
	kHSSpaceship	= 4,
	kHSDevice		= 5,
	kHSWalkArea1	= 6,
	kHSWalkArea2	= 7,
	kHSWalkArea3	= 8,
	kHSWalkArea4	= 9,
	kHSWalkArea5	= 10,
	kHSWalkArea6	= 11,
	kHSWalkArea7	= 12,
	kHSWalkArea8	= 13
};

enum {
	kASLookSpaceship		= 1,
	kASLookSpaceshipDone	= 2,
	kASLeaveScene			= 3,
	kASTakeMud				= 5,
	kASLookPigs				= 6,
	kASUsePigs				= 7
};

int GnapEngine::scene01_init() {
	_gameSys->setAnimation(0, 0, 0);
	_gameSys->setAnimation(0, 0, 3);
	return 0x88;
}

void GnapEngine::scene01_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_DISABLED | SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSExitTruck, 780, 226, 800, 455, SF_EXIT_R_CURSOR | SF_WALKABLE, 10, 6);
	setHotspot(kHSMud, 138, 282, 204, 318, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 5);
	setHotspot(kHSPigs, 408, 234, 578, 326, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 4);
	setHotspot(kHSSpaceship, 0, 200, 94, 292, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 1, 6);
	setHotspot(kHSWalkArea1, 0, 0, 162, 426);
	setHotspot(kHSWalkArea2, 162, 0, 237, 396);
	setHotspot(kHSWalkArea3, 237, 0, 319, 363);
	setHotspot(kHSWalkArea4, 520, 0, 800, 404);
	setHotspot(kHSWalkArea5, 300, 447, 800, 600);
	setHotspot(kHSWalkArea6, 678, 0, 800, 404);
	setHotspot(kHSWalkArea7, 0, 0, 520, 351);
	setHotspot(kHSWalkArea8, 0, 546, 300, 600);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
    if (isFlag(0))
		_hotspots[kHSPlatypus]._flags = SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
    if (isFlag(1))
		_hotspots[kHSMud]._flags = SF_WALKABLE | SF_DISABLED;
	_hotspotsCount = 14;
}

void GnapEngine::scene01_run() {

	// NOTE Removed _s01_dword_474380 which was set when the mud was taken
	// which is also set in the global game flags.
	
	playSound(0x1091C, 1);
	startSoundTimerC(5);

	_gameSys->setAnimation(134, 20, 4);
	_gameSys->insertSequence(134, 20, 0, 0, kSeqNone, 0, 0, 0);

	_gameSys->setAnimation(0x7F, 40, 2);
	_gameSys->insertSequence(0x7F, 40, 0, 0, kSeqNone, 0, 0, 0);

	_timers[4] = getRandom(100) + 300;

	if (!isFlag(1))
		_gameSys->insertSequence(129, 40, 0, 0, kSeqNone, 0, 0, 0);

	queueInsertDeviceIcon();

	if (_prevSceneNum == 2) {
		initGnapPos(11, 6, 3);
		if (isFlag(0))
			initBeaverPos(12, 6, 4);
		endSceneInit();
		if (isFlag(0))
			platypusWalkTo(9, 6, -1, 0x107C2, 1);
		gnapWalkTo(8, 6, -1, 0x107B9, 1);
	} else {
		initGnapPos(1, 6, 1);
		if (isFlag(0))
			initBeaverPos(1, 7, 0);
		endSceneInit();
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
				scene01_updateHotspots();
			}
			break;

		case kHSPlatypus:
			if (_gnapActionStatus < 0 && isFlag(0)) {
				if (_grabCursorSpriteIndex == kItemDisguise) {
					gnapUseDisguiseOnPlatypus();
				} else if (_grabCursorSpriteIndex >= 0) {
					playGnapImpossible(0, 0);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						if (isFlag(5))
							playGnapMoan1(_platX, _platY);
						else
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

		case kHSSpaceship:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[4].x, _hotspotsWalkPos[4].y, 0, 2);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						_gnapIdleFacing = 5;
						if (gnapWalkTo(_hotspotsWalkPos[4].x, _hotspotsWalkPos[4].y, 0, getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1))
							_gnapActionStatus = kASLookSpaceship;
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

		case kHSMud:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[2].x, _hotspotsWalkPos[2].y, 2, 3);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(3, 3);
						break;
					case GRAB_CURSOR:
						gnapWalkTo(_hotspotsWalkPos[2].x, _hotspotsWalkPos[2].y, 0, getGnapSequenceId(gskIdle, 2, 3) | 0x10000, 1);
						_gnapActionStatus = kASTakeMud;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSPigs:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[3].x, _hotspotsWalkPos[3].y, 7, 2);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						_gnapIdleFacing = 7;
						gnapWalkTo(_hotspotsWalkPos[3].x, _hotspotsWalkPos[3].y, 0, getGnapSequenceId(gskIdle, 7, 2) | 0x10000, 1);
						_gnapActionStatus = kASLookPigs;
						break;
					case GRAB_CURSOR:
						_gnapIdleFacing = 7;
						gnapWalkTo(_hotspotsWalkPos[3].x, _hotspotsWalkPos[3].y, 0, getGnapSequenceId(gskIdle, 7, 2) | 0x10000, 1);
						_gnapActionStatus = kASUsePigs;
						break;
					case TALK_CURSOR:
						_gnapIdleFacing = 7;
						gnapWalkTo(_hotspotsWalkPos[3].x, _hotspotsWalkPos[3].y, 0, getGnapSequenceId(gskBrainPulsating, 7, 2) | 0x10000, 1);
						_gnapActionStatus = kASLookPigs;
						break;
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSExitTruck:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = true;
				gnapWalkTo(_hotspotsWalkPos[1].x, _hotspotsWalkPos[1].y, 0, 0x107AB, 1);
				_gnapActionStatus = kASLeaveScene;
				if (isFlag(0))
					platypusWalkTo(_hotspotsWalkPos[1].x, _hotspotsWalkPos[1].y + 1, -1, 0x107CD, 1);
				_newSceneNum = 2;
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

		default:
			if (_mouseClickState._left && _gnapActionStatus < 0) {
				gnapWalkTo(-1, -1, -1, -1, 1);
				_mouseClickState._left = false;
			}
			break;

		}

		scene01_updateAnimations();

		if (!isSoundPlaying(0x1091C))
			playSound(0x1091C, 1);

		if (!_isLeavingScene) {
			if (_beaverActionStatus < 0 && isFlag(0))
				updateBeaverIdleSequence();
			if (_gnapActionStatus < 0)
				updateGnapIdleSequence();
			if (_timers[4] == 0) {
				// Update bird animation
				_timers[4] = getRandom(100) + 300;
				if (getRandom(1) == 0)
					_gameSys->insertSequence(0x84, 180, 0, 0, kSeqNone, 0, 0, 0);
				else
					_gameSys->insertSequence(0x83, 180, 0, 0, kSeqNone, 0, 0, 0);
			}
			playSoundC();
		}

		checkGameKeys();
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene01_updateHotspots();
		}
		
		gameUpdateTick();

	}

}

void GnapEngine::scene01_updateAnimations() {
	
	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		switch (_gnapActionStatus) {
		case kASLookSpaceship:
			_s01_spaceshipSurface = _gameSys->createSurface(47);
			_gameSys->insertSpriteDrawItem(_s01_spaceshipSurface, 0, 0, 255);
			_gameSys->setAnimation(133, 256, 0);
			_gameSys->insertSequence(133, 256, 0, 0, kSeqNone, 0, 0, 0);
			_gnapActionStatus = kASLookSpaceshipDone;
			break;
		case kASLookSpaceshipDone:
			_gameSys->removeSequence(133, 256, true);
			_gameSys->removeSpriteDrawItem(_s01_spaceshipSurface, 255);
			deleteSurface(&_s01_spaceshipSurface);
			_gnapActionStatus = -1;
			break;
		case kASLeaveScene:
			_sceneDone = true;
			break;
		case kASTakeMud:
			playGnapPullOutDevice(2, 3);
			playGnapUseDevice(0, 0);
			_gameSys->insertSequence(128, 40, 129, 40, kSeqSyncWait, 0, 0, 0);
			_gameSys->setAnimation(128, 40, 3);
			_gnapActionStatus = -1;
			break;
		case kASLookPigs:
			playSound(138, 0);
			playSound(139, 0);
			playSound(140, 0);
			_gnapActionStatus = -1;
			break;
		case kASUsePigs:
			playGnapPullOutDevice(7, 2);
			playGnapUseDevice(0, 0);
			_gameSys->insertSequence(135, 39, 0, 0, kSeqNone, 25, getRandom(140) - 40, 0);
			_gnapActionStatus = -1;
			break;
		}
	}

	if (_gameSys->getAnimationStatus(3) == 2) {
		_gameSys->setAnimation(0, 0, 3);
		invAdd(kItemMud);
		setGrabCursorSprite(kItemMud);
		setFlag(1);
		scene01_updateHotspots();
	}

	if (_gameSys->getAnimationStatus(4) == 2) {
		_s01_smokeIdCtr = (_s01_smokeIdCtr + 1) % 2;
		_gameSys->setAnimation(0x86, _s01_smokeIdCtr + 20, 4);
		_gameSys->insertSequence(0x86, _s01_smokeIdCtr + 20,
			0x86, (_s01_smokeIdCtr + 1) % 2 + 20,
			kSeqSyncWait, 0, 0, 0);
	}

	if (_gameSys->getAnimationStatus(2) == 2) {
		_s01_pigsIdCtr = (_s01_pigsIdCtr + 1) % 2;
		_gameSys->setAnimation(0x7F, _s01_pigsIdCtr + 40, 2);
		_gameSys->insertSequence(0x7F, _s01_pigsIdCtr + 40,
			0x7F, (_s01_pigsIdCtr + 1) % 2 + 40,
			kSeqSyncWait, 0, 0, 0);
	}

}

} // End of namespace Gnap
