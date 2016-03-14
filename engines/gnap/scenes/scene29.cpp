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
	kHSMonkey			= 1,
	kHSDevice			= 2,
	kHSExitCircus		= 3,
	kHSExitOutsideClown	= 4,
	kHSAracde			= 5,
	kHSWalkArea1		= 6
};

enum {
	kASUseBananaWithMonkey		= 0,
	kASLeaveScene				= 2
};

int GnapEngine::scene29_init() {
	return 0xF6;
}

void GnapEngine::scene29_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSMonkey, 410, 374, 518, 516, SF_WALKABLE | SF_DISABLED, 3, 7);
	setHotspot(kHSExitCircus, 150, 585, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
	setHotspot(kHSExitOutsideClown, 785, 0, 800, 600, SF_EXIT_R_CURSOR | SF_WALKABLE, 11, 9);
	setHotspot(kHSAracde, 88, 293, 155, 384, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 8);
	setHotspot(kHSWalkArea1, 0, 0, 800, 478);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	if (invHas(kItemHorn))
		_hotspots[kHSMonkey]._flags = SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	_hotspotsCount = 7;
}

void GnapEngine::scene29_run() {
	
	playSound(0x1093B, 1);
	startSoundTimerB(6);
	queueInsertDeviceIcon();

	if (invHas(kItemHorn)) {
		_s28_currMonkeySequenceId = 0xE8;
		_s28_nextMonkeySequenceId = -1;
		_gameSys->setAnimation(0xE8, 159, 4);
		_gameSys->insertSequence(_s28_currMonkeySequenceId, 159, 0, 0, kSeqNone, 0, 0, 0);
		_gameSys->insertSequence(0xED, 39, 0, 0, kSeqNone, 0, 0, 0);
		_s28_currManSequenceId = 0xED;
		_s28_nextManSequenceId = -1;
		_gameSys->setAnimation(0xED, 39, 3);
		_timers[4] = getRandom(20) + 60;
	} else {
		_gameSys->insertSequence(0xF4, 19, 0, 0, kSeqNone, 0, 0, 0);
		_gameSys->setAnimation(0, 0, 4);
		_gameSys->insertSequence(0xED, 39, 0, 0, kSeqNone, 0, 0, 0);
		_gameSys->setAnimation(0, 0, 3);
	}
	
	_gameSys->insertSequence(0xF3, 39, 0, 0, kSeqLoop, 0, 0, 0);
	_gameSys->insertSequence(0xF5, 38, 0, 0, kSeqLoop, 0, 0, 0);

	if (_prevSceneNum == 27) {
		initGnapPos(12, 7, 1);
		initBeaverPos(12, 8, 0);
		endSceneInit();
		gnapWalkTo(8, 7, -1, 0x107B9, 1);
		platypusWalkTo(8, 8, -1, 0x107C2, 1);
	} else {
		initGnapPos(-1, 7, 1);
		initBeaverPos(-2, 7, 0);
		endSceneInit();
		gnapWalkTo(2, 7, -1, 0x107B9, 1);
		platypusWalkTo(1, 7, -1, 0x107C2, 1);
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
				scene29_updateHotspots();
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

		case kHSMonkey:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex == kItemBanana) {
					_gnapIdleFacing = 1;
					gnapWalkTo(_hotspotsWalkPos[kHSMonkey].x, _hotspotsWalkPos[kHSMonkey].y, 0, getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
					_gnapActionStatus = kASUseBananaWithMonkey;
					_newSceneNum = 51;
					_isLeavingScene = true;
					setGrabCursorSprite(-1);
				} else if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[kHSMonkey].x, _hotspotsWalkPos[kHSMonkey].y, 5, 6);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(5, 6);
						break;
					case TALK_CURSOR:
						playGnapBrainPulsating(_hotspotsWalkPos[kHSMonkey].x, _hotspotsWalkPos[kHSMonkey].y);
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSExitCircus:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = true;
				_newSceneNum = 26;
				gnapWalkTo(_hotspotsWalkPos[kHSExitCircus].x, _hotspotsWalkPos[kHSExitCircus].y, 0, 0x107AE, 1);
				_gnapActionStatus = kASLeaveScene;
				platypusWalkTo(_hotspotsWalkPos[kHSExitCircus].x + 1, _hotspotsWalkPos[kHSExitCircus].y, -1, -1, 1);
			}
			break;

		case kHSExitOutsideClown:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = true;
				_newSceneNum = 27;
				gnapWalkTo(_hotspotsWalkPos[kHSExitOutsideClown].x, _hotspotsWalkPos[kHSExitOutsideClown].y, 0, 0x107AB, 1);
				_gnapActionStatus = kASLeaveScene;
				platypusWalkTo(_hotspotsWalkPos[kHSExitOutsideClown].x, _hotspotsWalkPos[kHSExitOutsideClown].y - 1, -1, 0x107CD, 1);
			}
			break;

		case kHSAracde:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex == kItemDiceQuarterHole) {
					setGrabCursorSprite(-1);
					_isLeavingScene = true;
					_newSceneNum = 52;
					gnapWalkTo(_hotspotsWalkPos[kHSAracde].x, _hotspotsWalkPos[kHSAracde].y, 0, -1, 1);
					playGnapIdle(_hotspotsWalkPos[kHSAracde].x, _hotspotsWalkPos[kHSAracde].y);
					_gnapActionStatus = kASLeaveScene;
				} else if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[kHSAracde].x, _hotspotsWalkPos[kHSAracde].y, 2, 3);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapMoan2(0, 0);
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

		case kHSWalkArea1:
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
	
		scene29_updateAnimations();

		if (!isSoundPlaying(0x1093B))
			playSound(0x1093B, 1);

		if (!_isLeavingScene) {
			if (_gnapActionStatus < 0) {
				updateGnapIdleSequence();
				updateBeaverIdleSequence();
			}
			if (!_timers[4]) {
				if (invHas(kItemHorn)) {
					_timers[4] = getRandom(20) + 60;
					if (_gnapActionStatus < 0) {
						switch (getRandom(5)) {
						case 0:
							_s28_nextManSequenceId = 0xED;
							break;
						case 1:
							_s28_nextManSequenceId = 0xEE;
							break;
						case 2:
							_s28_nextManSequenceId = 0xEF;
							break;
						case 3:
							_s28_nextManSequenceId = 0xF0;
							break;
						case 4:
							_s28_nextManSequenceId = 0xF1;
							break;
						}
					}
				}
			}
			playSoundB();
		}
	
		checkGameKeys();

		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene29_updateHotspots();
		}
		
		gameUpdateTick();
		
	}		

}

void GnapEngine::scene29_updateAnimations() {
	
	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		switch (_gnapActionStatus) {
		case kASUseBananaWithMonkey:
			_s28_nextMonkeySequenceId = 0xE5;
			break;
		case kASLeaveScene:
			_sceneDone = true;
			break;
		}
	}

	if (_gameSys->getAnimationStatus(3) == 2 && _s28_nextManSequenceId != -1) {
		_gameSys->insertSequence(_s28_nextManSequenceId, 39, _s28_currManSequenceId, 39, kSeqSyncWait, 0, 0, 0);
		_gameSys->setAnimation(_s28_nextManSequenceId, 39, 3);
		_s28_currManSequenceId = _s28_nextManSequenceId;
		_s28_nextManSequenceId = -1;
	}

	if (_gameSys->getAnimationStatus(4) == 2) {
		if (_s28_nextMonkeySequenceId == 0xE5) {
			_gameSys->insertSequence(0xF2, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gnapSequenceDatNum = 0;
			_gnapSequenceId = 0xF2;
			_gameSys->setAnimation(0xE6, 159, 0);
			_gameSys->setAnimation(0, 159, 4);
			_gameSys->insertSequence(_s28_nextMonkeySequenceId, 159, _s28_currMonkeySequenceId, 159, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0xE6, 159, _s28_nextMonkeySequenceId, 159, kSeqSyncWait, 0, 0, 0);
			_gnapActionStatus = kASLeaveScene;
			_s28_currMonkeySequenceId = 0xE6;
			_s28_nextMonkeySequenceId = -1;
			_timers[5] = 30;
			while (_timers[5])
				gameUpdateTick();
			platypusWalkTo(0, 8, 1, 0x107CF, 1);
			while (_gameSys->getAnimationStatus(1) != 2)
				gameUpdateTick();
		} else if (_s28_nextMonkeySequenceId == -1) {
			switch (getRandom(6)) {
			case 0:
				_s28_nextMonkeySequenceId = 0xE8;
				break;
			case 1:
				_s28_nextMonkeySequenceId = 0xE9;
				break;
			case 2:
				_s28_nextMonkeySequenceId = 0xEA;
				break;
			case 3:
				_s28_nextMonkeySequenceId = 0xEB;
				break;
			case 4:
				_s28_nextMonkeySequenceId = 0xEC;
				break;
			case 5:
				_s28_nextMonkeySequenceId = 0xE7;
				break;
			}
			_gameSys->insertSequence(_s28_nextMonkeySequenceId, 159, _s28_currMonkeySequenceId, 159, kSeqSyncWait, 0, 0, 0);
			_gameSys->setAnimation(_s28_nextMonkeySequenceId, 159, 4);
			_s28_currMonkeySequenceId = _s28_nextMonkeySequenceId;
			_s28_nextMonkeySequenceId = -1;
		} else {
			_gameSys->insertSequence(_s28_nextMonkeySequenceId, 159, _s28_currMonkeySequenceId, 159, kSeqSyncWait, 0, 0, 0);
			_gameSys->setAnimation(_s28_nextMonkeySequenceId, 159, 4);
			_s28_currMonkeySequenceId = _s28_nextMonkeySequenceId;
			_s28_nextMonkeySequenceId = -1;
		}
	}
	
}

} // End of namespace Gnap
